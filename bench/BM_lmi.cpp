#include <ellalgo/cutting_plane.hpp>     // for cutting_plane_dc
#include <ellalgo/ell.hpp>               // for ell
#include <gsl/span>                      // for span
#include <lmisolver/lmi_old_oracle.hpp>  // for lmi_old_oracle
#include <lmisolver/lmi_oracle.hpp>      // for lmi_oracle
#include <tuple>                         // for tuple
#include <type_traits>                   // for move
#include <vector>                        // for vector
#include <xtensor/xlayout.hpp>           // for layout_type, layout_type::ro...
#include <xtensor/xmath.hpp>             // for sum
#include <xtensor/xoperation.hpp>        // for operator*
#include <xtensor/xtensor_forward.hpp>   // for xarray

#include "benchmark/benchmark.h"  // for BENCHMARK, State, BENCHMARK_...

/**
 * @brief
 *
 * @tparam Oracle
 */
template <typename Oracle> class my_oracle {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;
    using Cut = std::tuple<Arr, double>;

  private:
    Oracle lmi1;
    Oracle lmi2;
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
    std::tuple<Cut, bool> operator()(const Arr& x, double& t) {
        const auto f0 = xt::sum(this->c * x)();
        const auto f1 = f0 - t;
        if (f1 > 0) {
            return {{this->c, f1}, false};
        }
        const auto cut1 = this->lmi1(x);
        if (cut1) {
            return {*cut1, false};
        }
        const auto cut2 = this->lmi2(x);
        if (cut2) {
            return {*cut2, false};
        }
        t = f0;
        return {{this->c, 0.0}, true};
    }
};

/**
 * @brief
 *
 * @param[in,out] state
 */
static void LMI_Lazy(benchmark::State& state) {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;

    // auto c = Arr {1.0, -1.0, 1.0};
    const auto F1 = std::vector<Arr>{
        {{-7.0, -11.0}, {-11.0, 3.0}}, {{7.0, -18.0}, {-18.0, 8.0}}, {{-2.0, -8.0}, {-8.0, 1.0}}};
    const auto B1 = Arr{{33.0, -9.0}, {-9.0, 26.0}};
    const auto F2 = std::vector<Arr>{{{-21.0, -11.0, 0.0}, {-11.0, 10.0, 8.0}, {0.0, 8.0, 5.0}},
                                     {{0.0, 10.0, 16.0}, {10.0, -10.0, -10.0}, {16.0, -10.0, 3.0}},
                                     {{-5.0, 2.0, -17.0}, {2.0, -6.0, 8.0}, {-17.0, 8.0, 6.0}}};
    const auto B2 = Arr{{14.0, 9.0, 40.0}, {9.0, 91.0, 10.0}, {40.0, 10.0, 15.0}};

    while (state.KeepRunning()) {
        auto P = my_oracle<lmi_oracle<Arr>>(F1, B1, F2, B2, Arr{1.0, -1.0, 1.0});
        auto E = ell(10.0, Arr{0.0, 0.0, 0.0});
        auto t = 1e100;  // std::numeric_limits<double>::max()
        [[maybe_unused]] const auto rslt = cutting_plane_dc(P, E, t);
    }
}

// Register the function as a benchmark
BENCHMARK(LMI_Lazy);

//~~~~~~~~~~~~~~~~

/**
 * @brief Define another benchmark
 *
 * @param[in,out] state
 */
static void LMI_old(benchmark::State& state) {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;

    // auto c = Arr {1.0, -1.0, 1.0};
    const auto F1 = std::vector<Arr>{
        {{-7.0, -11.0}, {-11.0, 3.0}}, {{7.0, -18.0}, {-18.0, 8.0}}, {{-2.0, -8.0}, {-8.0, 1.0}}};
    const auto B1 = Arr{{33.0, -9.0}, {-9.0, 26.0}};
    const auto F2 = std::vector<Arr>{{{-21.0, -11.0, 0.0}, {-11.0, 10.0, 8.0}, {0.0, 8.0, 5.0}},
                                     {{0.0, 10.0, 16.0}, {10.0, -10.0, -10.0}, {16.0, -10.0, 3.0}},
                                     {{-5.0, 2.0, -17.0}, {2.0, -6.0, 8.0}, {-17.0, 8.0, 6.0}}};
    const auto B2 = Arr{{14.0, 9.0, 40.0}, {9.0, 91.0, 10.0}, {40.0, 10.0, 15.0}};

    while (state.KeepRunning()) {
        auto P = my_oracle<lmi_old_oracle<Arr>>(F1, B1, F2, B2, Arr{1.0, -1.0, 1.0});
        auto E = ell(10.0, Arr{0.0, 0.0, 0.0});
        auto t = 1e100;  // std::numeric_limits<double>::max()
        [[maybe_unused]] const auto rslt = cutting_plane_dc(P, E, t);
    }
}
BENCHMARK(LMI_old);

/**
 * @brief
 *
 * @param[in,out] state
 */
static void LMI_No_Trick(benchmark::State& state) {
    using Arr = xt::xarray<double, xt::layout_type::row_major>;

    // const auto c = Arr {1.0, -1.0, 1.0};
    const auto F1 = std::vector<Arr>{
        {{-7.0, -11.0}, {-11.0, 3.0}}, {{7.0, -18.0}, {-18.0, 8.0}}, {{-2.0, -8.0}, {-8.0, 1.0}}};
    const auto B1 = Arr{{33.0, -9.0}, {-9.0, 26.0}};
    const auto F2 = std::vector<Arr>{{{-21.0, -11.0, 0.0}, {-11.0, 10.0, 8.0}, {0.0, 8.0, 5.0}},
                                     {{0.0, 10.0, 16.0}, {10.0, -10.0, -10.0}, {16.0, -10.0, 3.0}},
                                     {{-5.0, 2.0, -17.0}, {2.0, -6.0, 8.0}, {-17.0, 8.0, 6.0}}};
    const auto B2 = Arr{{14.0, 9.0, 40.0}, {9.0, 91.0, 10.0}, {40.0, 10.0, 15.0}};

    while (state.KeepRunning()) {
        auto P = my_oracle<lmi_oracle<Arr>>(F1, B1, F2, B2, Arr{1.0, -1.0, 1.0});
        auto E = ell(10.0, Arr{0.0, 0.0, 0.0});
        E.no_defer_trick = true;
        auto t = 1e100;  // std::numeric_limits<double>::max()
        [[maybe_unused]] const auto rslt = cutting_plane_dc(P, E, t);
    }
}

// Register the function as a benchmark
BENCHMARK(LMI_No_Trick);

BENCHMARK_MAIN();

/*
----------------------------------------------------------
Benchmark                Time             CPU   Iterations
----------------------------------------------------------
LMI_Lazy         131235 ns       131245 ns         4447
LMI_old          196694 ns       196708 ns         3548
LMI_No_Trick     129743 ns       129750 ns         5357
*/