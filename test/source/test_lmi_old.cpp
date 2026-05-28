/*
 *  Distributed under the MIT License (See accompanying file /LICENSE )
 */
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase

#include <ellalgo/cutting_plane.hpp>           // for cutting_plane_optim
#include <ellalgo/ell.hpp>                     // for Ell
#include <ellalgo/ell_matrix.hpp>              // for EllStable
#include <ellalgo/ell_stable.hpp>              // for EllStable
#include <ellalgo/oracles/lmi_old_oracle.hpp>  // for LmiOldOracle
// #include <span> // for span
// #include <spdlog/sinks/stdout_sinks.h>
// #include <spdlog/spdlog.h>
// #include <optional>    // for optional
#include <tuple>  // for tuple
#include <valarray>
#include <vector>  // for vector

/**
 * @brief MyOracle
 *
 */
class MyOldOracle {
    using Vec = std::valarray<double>;
    using M_t = std::vector<Matrix>;
    using Cut = std::pair<Vec, double>;

    LmiOldOracle<Vec, Matrix> lmi1;
    LmiOldOracle<Vec, Matrix> lmi2;
    Vec c;

  public:
    /**
     * The function is a constructor for a class called MyOldOracle that takes in several parameters
     * and initializes member variables.
     *
     * @param[in] m1 The parameter `m1` represents the number of constraints for the first
     * Linear-Matrix-Inequality (LMI).
     * @param[in] F1 F1 is a vector of matrices. It represents the matrices used in the first
     * Linear-Matrix-Inequality (LMI) problem.
     * @param[in] B1 B1 is a matrix.
     * @param[in] m2 The parameter `m2` represents the number of constraints in the second
     * Linear-Matrix-Inequality (LMI).
     * @param[in] F2 F2 is a vector of matrices.
     * @param[in] B2 The parameter B2 is a matrix.
     * @param[in] c The parameter `c` is a vector of type `Vec`. It is being moved into the
     * `MyLMIOracle` object.
     */
    MyOldOracle(size_t m1, const std::vector<Matrix>& F1, const Matrix& B1, size_t m2,
                const std::vector<Matrix>& F2, const Matrix& B2, Vec c)
        : lmi1{m1, F1, B1}, lmi2{m2, F2, B2}, c{std::move(c)} {}

    /**
     * The function assess_optim assesses the optimality of a given vector x by checking for certain
     * conditions and returning a tuple containing a cut and a boolean value.
     *
     * @param[in] x A vector representing the input values for the optimization problem.
     * @param[in,out] gamma The parameter `gamma` is a reference to a `double` variable. It is used
     * to store the value of `f0` in the last part of the function.
     *
     * @return The function `assess_optim` returns a tuple containing a `Cut` object and a boolean
     * value.
     */
    auto assess_optim(const Vec& x, double& gamma) -> std::tuple<Cut, bool> {
        auto* const cut1 = this->lmi1(x);
        if (cut1 != nullptr) {
            return {*cut1, false};
        }
        auto* const cut2 = this->lmi2(x);
        if (cut2 != nullptr) {
            return {*cut2, false};
        }
        const auto f0 = (this->c * x).sum();
        const auto f1 = f0 - gamma;
        if (f1 > 0.0) {
            return {{this->c, f1}, false};
        }
        gamma = f0;
        return {{this->c, 0.0}, true};
    }
};

TEST_CASE("LMI test (stable)") {
    using Vec = std::valarray<double>;
    using M_t = std::vector<Matrix>;

    auto c = Vec{1.0, -1.0, 1.0};

    auto m0F1 = Matrix(2);
    m0F1.row(0) = Vec{-7.0, -11.0};
    m0F1.row(1) = Vec{-11.0, 3.0};

    auto m1F1 = Matrix(2);
    m1F1.row(0) = Vec{7.0, -18.0};
    m1F1.row(1) = Vec{-18.0, 8.0};

    auto m2F1 = Matrix(2);
    m2F1.row(0) = Vec{-2.0, -8.0};
    m2F1.row(1) = Vec{-8.0, 1.0};

    auto F1 = M_t{m0F1, m1F1, m2F1};

    auto B1 = Matrix(2);
    B1.row(0) = Vec{33.0, -9.0};
    B1.row(1) = Vec{-9.0, 26.0};

    auto m0F2 = Matrix(3);
    m0F2.row(0) = Vec{-21.0, -11.0, 0.0};
    m0F2.row(1) = Vec{-11.0, 10.0, 8.0};
    m0F2.row(2) = Vec{0.0, 8.0, 5.0};

    auto m1F2 = Matrix(3);
    m1F2.row(0) = Vec{0.0, 10.0, 16.0};
    m1F2.row(1) = Vec{10.0, -10.0, -10.0};
    m1F2.row(2) = Vec{16.0, -10.0, 3.0};

    auto m2F2 = Matrix(3);
    m2F2.row(0) = Vec{-5.0, 2.0, -17.0};
    m2F2.row(1) = Vec{2.0, -6.0, 8.0};
    m2F2.row(2) = Vec{-17.0, 8.0, 6.0};

    auto F2 = M_t{m0F2, m1F2, m2F2};

    auto B2 = Matrix(3);
    B2.row(0) = Vec{14.0, 9.0, 40.0};
    B2.row(1) = Vec{9.0, 91.0, 10.0};
    B2.row(2) = Vec{40.0, 10.0, 15.0};

    auto omega = MyOldOracle(2, F1, B1, 3, F2, B2, std::move(c));
    auto ellip = Ell<Vec>(10.0, Vec{0.0, 0.0, 0.0});

    auto gamma = 1e100;  // should be std::numeric_limits<double>::max()
    const auto result = cutting_plane_optim(omega, ellip, gamma);
    auto x = std::get<0>(result);
    auto num_iters = std::get<1>(result);

    // create color multi threaded logger
    // auto console = spdlog::stdout_logger_mt("console");
    // auto err_logger = spdlog::stderr_logger_mt("stderr");
    // spdlog::get("console")->info("loggers can be retrieved from a global "
    //                              "registry using the
    //                              spdlog::get(logger_name)");

    CHECK_NE(x.size(), 0U);
    CHECK_EQ(num_iters, 281);
}

TEST_CASE("LMI test ") {
    using Vec = std::valarray<double>;
    using M_t = std::vector<Matrix>;

    auto c = Vec{1.0, -1.0, 1.0};

    auto m0F1 = Matrix(2);
    m0F1.row(0) = Vec{-7.0, -11.0};
    m0F1.row(1) = Vec{-11.0, 3.0};

    auto m1F1 = Matrix(2);
    m1F1.row(0) = Vec{7.0, -18.0};
    m1F1.row(1) = Vec{-18.0, 8.0};

    auto m2F1 = Matrix(2);
    m2F1.row(0) = Vec{-2.0, -8.0};
    m2F1.row(1) = Vec{-8.0, 1.0};

    auto F1 = M_t{m0F1, m1F1, m2F1};

    auto B1 = Matrix(2);
    B1.row(0) = Vec{33.0, -9.0};
    B1.row(1) = Vec{-9.0, 26.0};

    auto m0F2 = Matrix(3);
    m0F2.row(0) = Vec{-21.0, -11.0, 0.0};
    m0F2.row(1) = Vec{-11.0, 10.0, 8.0};
    m0F2.row(2) = Vec{0.0, 8.0, 5.0};

    auto m1F2 = Matrix(3);
    m1F2.row(0) = Vec{0.0, 10.0, 16.0};
    m1F2.row(1) = Vec{10.0, -10.0, -10.0};
    m1F2.row(2) = Vec{16.0, -10.0, 3.0};

    auto m2F2 = Matrix(3);
    m2F2.row(0) = Vec{-5.0, 2.0, -17.0};
    m2F2.row(1) = Vec{2.0, -6.0, 8.0};
    m2F2.row(2) = Vec{-17.0, 8.0, 6.0};

    auto F2 = M_t{m0F2, m1F2, m2F2};

    auto B2 = Matrix(3);
    B2.row(0) = Vec{14.0, 9.0, 40.0};
    B2.row(1) = Vec{9.0, 91.0, 10.0};
    B2.row(2) = Vec{40.0, 10.0, 15.0};

    auto omega = MyOldOracle(2, F1, B1, 3, F2, B2, std::move(c));
    auto ellip = EllStable<Vec>(10.0, Vec{0.0, 0.0, 0.0});

    auto gamma = 1e100;  // should be std::numeric_limits<double>::max()
    const auto result = cutting_plane_optim(omega, ellip, gamma);
    const auto& x = std::get<0>(result);
    const auto& num_iters = std::get<1>(result);

    CHECK_NE(x.size(), 0U);
    CHECK_EQ(num_iters, 281);
}
