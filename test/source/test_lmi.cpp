/*
 *  Distributed under the MIT License (See accompanying file /LICENSE )
 */
#include <doctest/doctest.h>  // for ResultBuilder, CHECK, TestCase

#include <ellalgo/cutting_plane.hpp>    // for cutting_plane_dc
#include <ellalgo/ell.hpp>              // for ell
#include <ellalgo/ell_stable.hpp>       // for ell_stable
#include <lmisolver/lmi_oracle.hpp>     // for lmi_oracle
#include <xtensor/xarray.hpp>           // for xarray_container
#include <xtensor/xlayout.hpp>          // for layout_type, layout_type::row...
#include <xtensor/xmath.hpp>            // for sum
#include <xtensor/xoperation.hpp>       // for xfunction_type_t, operator*
#include <xtensor/xreducer.hpp>         // for xreducer
#include <xtensor/xtensor_forward.hpp>  // for xarray
// #include <fmt/format.h>
#include <gsl/span>  // for span
// #include <spdlog/sinks/stdout_sinks.h>
// #include <spdlog/spdlog.h>
#include <optional>     // for optional
#include <tuple>        // for tuple
#include <type_traits>  // for move, add_const<>::type
#include <vector>       // for vector
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
    lmi_oracle<Arr> lmi1;
    lmi_oracle<Arr> lmi2;
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
    my_oracle(gsl::span<const Arr> F1, const Arr& B1, gsl::span<const Arr> F2, const Arr& B2, Arr c)
        : lmi1{F1, B1}, lmi2{F2, B2}, c{std::move(c)} {}

    /**
     * @brief
     *
     * @param[in] x
     * @param[in,out] t
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
        const auto f1 = f0 - t;
        if (f1 > 0) {
            return {{this->c, f1}, false};
        }
        t = f0;
        return {{this->c, 0.0}, true};
    }
};

TEST_CASE("LMI test (stable)") {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;
    using M_t = std::vector<Arr>;

    auto c = Arr{1.0, -1.0, 1.0};
    auto F1 = M_t{
        {{-7.0, -11.0}, {-11.0, 3.0}}, {{7.0, -18.0}, {-18.0, 8.0}}, {{-2.0, -8.0}, {-8.0, 1.0}}};
    auto B1 = Arr{{33.0, -9.0}, {-9.0, 26.0}};
    auto F2 = M_t{{{-21.0, -11.0, 0.0}, {-11.0, 10.0, 8.0}, {0.0, 8.0, 5.0}},
                  {{0.0, 10.0, 16.0}, {10.0, -10.0, -10.0}, {16.0, -10.0, 3.0}},
                  {{-5.0, 2.0, -17.0}, {2.0, -6.0, 8.0}, {-17.0, 8.0, 6.0}}};
    auto B2 = Arr{{14.0, 9.0, 40.0}, {9.0, 91.0, 10.0}, {40.0, 10.0, 15.0}};

    auto P = my_oracle(F1, B1, F2, B2, std::move(c));
    auto E = ell(10.0, Arr{0.0, 0.0, 0.0});

    auto t = 1e100;  // std::numeric_limits<double>::max()
    const auto [x, ell_info] = cutting_plane_dc(P, E, t);
    // fmt::print("{:f} {} {} \n", t, ell_info.num_iters, ell_info.feasible);
    // std::cout << "LMI xbest: " << xb << "\n";
    // std::cout << "LMI result: " << fb << ", " << niter << ", " << feasible <<
    // ", " << status
    //           << "\n";

    // create color multi threaded logger
    // auto console = spdlog::stdout_logger_mt("console");
    // auto err_logger = spdlog::stderr_logger_mt("stderr");
    // spdlog::get("console")->info("loggers can be retrieved from a global "
    //                              "registry using the
    //                              spdlog::get(logger_name)");

    CHECK(ell_info.feasible);
    CHECK(ell_info.num_iters == 113);
}

TEST_CASE("LMI test ") {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;
    using M_t = std::vector<Arr>;

    auto c = Arr{1.0, -1.0, 1.0};
    auto F1 = M_t{
        {{-7.0, -11.0}, {-11.0, 3.0}}, {{7.0, -18.0}, {-18.0, 8.0}}, {{-2.0, -8.0}, {-8.0, 1.0}}};
    auto B1 = Arr{{33.0, -9.0}, {-9.0, 26.0}};
    auto F2 = M_t{{{-21.0, -11.0, 0.0}, {-11.0, 10.0, 8.0}, {0.0, 8.0, 5.0}},
                  {{0.0, 10.0, 16.0}, {10.0, -10.0, -10.0}, {16.0, -10.0, 3.0}},
                  {{-5.0, 2.0, -17.0}, {2.0, -6.0, 8.0}, {-17.0, 8.0, 6.0}}};
    auto B2 = Arr{{14.0, 9.0, 40.0}, {9.0, 91.0, 10.0}, {40.0, 10.0, 15.0}};

    auto P = my_oracle(F1, B1, F2, B2, std::move(c));
    auto E = ell_stable(10.0, Arr{0.0, 0.0, 0.0});

    auto t = 1e100;  // std::numeric_limits<double>::max()
    const auto [x, ell_info] = cutting_plane_dc(P, E, t);

    CHECK(ell_info.feasible);
    CHECK(ell_info.num_iters == 112);
}
