#include <doctest/doctest.h>  // for ResultBuilder, TestCase, CHECK

#include <lmisolver/ldlt_ext.hpp>       // for ldlt_ext
#include <xtensor/xarray.hpp>           // for xarray_container
#include <xtensor/xcontainer.hpp>       // for xcontainer<>::inner_shape_type
#include <xtensor/xlayout.hpp>          // for layout_type, layout_type::row...
#include <xtensor/xtensor_forward.hpp>  // for xarray
// #include <xtensor/xarray.hpp>

using Arr = xt::xarray<double, xt::layout_type::row_major>;

TEST_CASE("Cholesky test 1") {
    const auto m1 = Arr{{25.0, 15.0, -5.0}, {15.0, 18.0, 0.0}, {-5.0, 0.0, 11.0}};
    auto Q1 = ldlt_ext<Arr>(m1.shape()[0]);
    CHECK(Q1.factorize(m1));
}

TEST_CASE("Cholesky test 2") {
    const auto m2 = Arr{{18.0, 22.0, 54.0, 42.0},
                        {22.0, -70.0, 86.0, 62.0},
                        {54.0, 86.0, -174.0, 134.0},
                        {42.0, 62.0, 134.0, -106.0}};
    auto Q2 = ldlt_ext<Arr>(m2.shape()[0]);
    Q2.factorize(m2);
    CHECK(!Q2.is_spd());
    // CHECK(Q2.p.second == 2);
}

TEST_CASE("Cholesky test 3") {
    const auto m3 = Arr{{0.0, 15.0, -5.0}, {15.0, 18.0, 0.0}, {-5.0, 0.0, 11.0}};
    auto Q3 = ldlt_ext<Arr>(m3.shape()[0]);
    Q3.factorize(m3);
    CHECK(!Q3.is_spd());
    const auto ep3 = Q3.witness();
    // CHECK(Q3.p.second == 1);
    CHECK(ep3 == 0.);
}
