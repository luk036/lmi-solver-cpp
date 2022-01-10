/*
 *  Distributed under the MIT License (See accompanying file /LICENSE )
 */
#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase

#include <__tuple>                       // for tuple_element<>::type
#include <ellalgo/cutting_plane.hpp>     // for cutting_plane_dc
#include <ellalgo/ell.hpp>               // for ell
#include <ellalgo/ell_stable.hpp>        // for ell_stable
#include <gsl/span>                      // for span
#include <lmisolver/lmi_old_oracle.hpp>  // for lmi_old_oracle
#include <optional>                      // for optional
#include <tuple>                         // for tuple
#include <type_traits>                   // for move, add_const<>::type
#include <vector>                        // for vector
#include <xtensor/xaccessible.hpp>       // for xconst_accessible
#include <xtensor/xarray.hpp>            // for xarray_container
#include <xtensor/xlayout.hpp>           // for layout_type, layout_type::ro...
#include <xtensor/xmath.hpp>             // for sum
#include <xtensor/xoperation.hpp>        // for xfunction_type_t, operator*
#include <xtensor/xreducer.hpp>          // for xreducer
#include <xtensor/xtensor_forward.hpp>   // for xarray
// #include <xtensor-blas/xlinalg.hpp>

/**
 * @brief my_oracle
 *
 */
class my_oracle {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;
    using M_t = std::vector<Arr>;
    using Cut = std::tuple<Arr, double>;

  private:
    lmi_old_oracle lmi1;
    lmi_old_oracle lmi2;
    const Arr c;

  public:
    /**
     * @brief Construct a new my oracle object
     *
     * @param[in] F1
     * @param[in] B1
     * @param[in] F2
     * @param[in] B2
     * @param[in] c
     */
    my_oracle(gsl::span<const Arr> F1, Arr B1, gsl::span<const Arr> F2, Arr B2, Arr c)
        : lmi1{F1, std::move(B1)}, lmi2{F2, std::move(B2)}, c{std::move(c)} {}

    /**
     * @brief
     *
     * @param[in] x
     * @param[in,out] t the best-so-far optimal value
     * @return std::tuple<Cut, double>
     */
    auto operator()(const Arr& x, double& t) -> std::tuple<Cut, bool> {
        const auto cut1 = this->lmi1(x);
        if (cut1) {
            return {*cut1, false};
        }
        const auto cut2 = this->lmi2(x);
        if (cut2) {
            return {*cut2, false};
        }
        const auto f0 = xt::sum(this->c * x)();
        const auto fj = f0 - t;
        if (fj > 0.) {
            return {{this->c, fj}, false};
        }
        t = f0;
        return {{this->c, 0.}, true};
    }
};

TEST_CASE("LMI (old) test") {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;
    using M_t = std::vector<Arr>;

    // const auto c = Arr {1., -1., 1.};
    const auto F1
        = M_t{{{-7., -11.}, {-11., 3.}}, {{7., -18.}, {-18., 8.}}, {{-2., -8.}, {-8., 1.}}};
    auto B1 = Arr{{33., -9.}, {-9., 26.}};
    const auto F2 = M_t{{{-21., -11., 0.}, {-11., 10., 8.}, {0., 8., 5.}},
                        {{0., 10., 16.}, {10., -10., -10.}, {16., -10., 3.}},
                        {{-5., 2., -17.}, {2., -6., 8.}, {-17., 8., 6.}}};
    auto B2 = Arr{{14., 9., 40.}, {9., 91., 10.}, {40., 10., 15.}};
    auto c = Arr{1., -1., 1.};
    auto P = my_oracle(F1, std::move(B1), F2, std::move(B2), std::move(c));
    auto E = ell(10., Arr{0., 0., 0.});
    auto t = 1.e100;  // std::numeric_limits<double>::max()
    const auto [x, ell_info] = cutting_plane_dc(P, E, t);

    CHECK(x[0] < -0.3);
    CHECK(ell_info.feasible);
    CHECK(ell_info.num_iters == 113);
}

TEST_CASE("LMI (old) test (stable)") {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;
    using M_t = std::vector<Arr>;

    // const auto c = Arr {1., -1., 1.};
    const auto F1
        = M_t{{{-7., -11.}, {-11., 3.}}, {{7., -18.}, {-18., 8.}}, {{-2., -8.}, {-8., 1.}}};
    auto B1 = Arr{{33., -9.}, {-9., 26.}};
    const auto F2 = M_t{{{-21., -11., 0.}, {-11., 10., 8.}, {0., 8., 5.}},
                        {{0., 10., 16.}, {10., -10., -10.}, {16., -10., 3.}},
                        {{-5., 2., -17.}, {2., -6., 8.}, {-17., 8., 6.}}};
    auto B2 = Arr{{14., 9., 40.}, {9., 91., 10.}, {40., 10., 15.}};
    auto c = Arr{1., -1., 1.};
    auto P = my_oracle(F1, std::move(B1), F2, std::move(B2), std::move(c));
    auto E = ell_stable(10., Arr{0., 0., 0.});
    auto t = 1.e100;  // std::numeric_limits<double>::max()
    const auto [x, ell_info] = cutting_plane_dc(P, E, t);

    CHECK(ell_info.feasible);
    CHECK(ell_info.num_iters == 112);
}
