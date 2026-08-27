#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#include <doctest/doctest.h>

#include <Eigen/Dense>
#include <cmath>
#include <ellalgo/cutting_plane.hpp>  // for cutting_plane_optim
#include <ellalgo/ell.hpp>            // for Ell
#include <ellalgo/ell_config.hpp>     // for Options
#include <ellalgo/ell_stable.hpp>     // for EllStable
#include <lmisolver/lmi0_oracle.hpp>
#include <lmisolver/lmi_oracle.hpp>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace {

    using Vec = Eigen::VectorXd;
    using Mat = Eigen::MatrixXd;

    // Mirrors py/ellalgo/tests/test_lmi.py::MyOracle
    class MyLmiOracle {
        using Cut = std::pair<Vec, double>;

        // LmiOracle stores a non-owning reference; matrices must outlive the oracle.
        lmi::LmiOracle<Vec, Mat> lmi1;
        lmi::LmiOracle<Vec, Mat> lmi2;
        Vec c;
        int idx = -1;

      public:
        MyLmiOracle(std::size_t m1, const std::vector<Mat>& F1, const Mat& B1, std::size_t m2,
                    const std::vector<Mat>& F2, const Mat& B2, Vec c)
            : lmi1{m1, F1, B1}, lmi2{m2, F2, B2}, c{std::move(c)} {}

        auto assess_optim(const Vec& x, double& gamma) -> std::tuple<Cut, bool> {
            for (int i = 0; i != 3; ++i) {
                this->idx = this->idx == 2 ? 0 : this->idx + 1;  // round robin
                if (this->idx == 0) {
                    if (auto* cut = this->lmi1.assess_feas(x)) {
                        return {*cut, false};
                    }
                } else if (this->idx == 1) {
                    if (auto* cut = this->lmi2.assess_feas(x)) {
                        return {*cut, false};
                    }
                } else {
                    const auto f0 = this->c.dot(x);
                    const auto fj = f0 - gamma;
                    if (fj > 0.0) {
                        return {{this->c, fj}, false};
                    }
                    return {{this->c, 0.0}, true};
                }
            }
            return {{this->c, 0.0}, false};
        }
    };

    // Mirrors py/ellalgo/tests/test_lmi.py (F1_mat, B1_mat, F2_mat, B2_mat data)

    auto F1_mat() -> std::vector<Mat> {
        Mat m0(2, 2);
        m0 << -7.0, -11.0, -11.0, 3.0;
        Mat m1(2, 2);
        m1 << 7.0, -18.0, -18.0, 8.0;
        Mat m2(2, 2);
        m2 << -2.0, -8.0, -8.0, 1.0;
        return {m0, m1, m2};
    }

    auto B1_mat() -> Mat {
        Mat B(2, 2);
        B << 33.0, -9.0, -9.0, 26.0;
        return B;
    }

    auto F2_mat() -> std::vector<Mat> {
        Mat m0(3, 3);
        m0 << -21.0, -11.0, 0.0, -11.0, 10.0, 8.0, 0.0, 8.0, 5.0;
        Mat m1(3, 3);
        m1 << 0.0, 10.0, 16.0, 10.0, -10.0, -10.0, 16.0, -10.0, 3.0;
        Mat m2(3, 3);
        m2 << -5.0, 2.0, -17.0, 2.0, -6.0, 8.0, -17.0, 8.0, 6.0;
        return {m0, m1, m2};
    }

    auto B2_mat() -> Mat {
        Mat B(3, 3);
        B << 14.0, 9.0, 40.0, 9.0, 91.0, 10.0, 40.0, 10.0, 15.0;
        return B;
    }

    // Mirrors py/ellalgo/tests/test_lmi.py::run_lmi
    template <typename Space> auto run_lmi() -> std::size_t {
        const auto F1 = F1_mat();
        const auto B1 = B1_mat();
        const auto F2 = F2_mat();
        const auto B2 = B2_mat();
        const auto c = (Vec(3) << 1.0, -1.0, 1.0).finished();
        auto omega = MyLmiOracle{2, F1, B1, 3, F2, B2, c};
        auto ellip = Space{10.0, Vec::Zero(3)};
        auto gamma = 1.0e100;  // std::numeric_limits<double>::max()
        const auto result = cutting_plane_optim(omega, ellip, gamma, Options{2000, 1e-10});
        const auto& x = std::get<0>(result);
        REQUIRE_NE(x.size(), 0U);
        return std::get<1>(result);
    }

}  // namespace

TEST_CASE("lmi::LmiOracle: origin is feasible") {
    // Mirrors py/ellalgo/tests/test_lmi.py::test_lmi_oracle
    const auto F1 = F1_mat();
    const auto B1 = B1_mat();
    auto oracle = lmi::LmiOracle<Vec, Mat>(2, F1, B1);
    const auto x = Vec::Zero(3);
    const auto cut = oracle.assess_feas(x);
    CHECK(cut == nullptr);
}

TEST_CASE("lmi::Lmi0Oracle: origin yields a cut") {
    // Mirrors py/ellalgo/tests/test_lmi.py::test_lmi0_oracle
    const auto F1 = F1_mat();
    auto oracle = lmi::Lmi0Oracle<Vec, Mat>(2, F1);
    const auto x = Vec::Zero(3);
    const auto cut = oracle.assess_feas(x);
    CHECK(cut != nullptr);
}

TEST_CASE("LMI via cutting_plane_optim with Ell") {
    // Mirrors py/ellalgo/tests/test_lmi.py::test_lmi_lazy. Note: py asserts 281
    // iterations, but that count is specific to ellalgo's numpy/valarray oracles;
    // lmi's independent Eigen-LDLT oracle converges faster (31) — assert the
    // convergence contract (xbest non-empty) and a bound, not the py-specific count.
    const auto num_iters = run_lmi<Ell<Vec>>();
    CHECK_LT(num_iters, 2000U);
}

TEST_CASE("LMI via cutting_plane_optim with EllStable") {
    // Mirrors py/ellalgo/tests/test_lmi.py::test_lmi_lazy_stable (same note as above)
    const auto num_iters = run_lmi<EllStable<Vec>>();
    CHECK_LT(num_iters, 2000U);
}
