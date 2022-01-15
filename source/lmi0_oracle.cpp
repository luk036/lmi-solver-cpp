#include <stddef.h>  // for size_t

#include <gsl/span>                     // for span, span<>::element_type
#include <lmisolver/lmi0_oracle.hpp>    // for lmi0_oracle::Arr, lmi0_oracle...
#include <optional>                     // for optional
#include <tuple>                        // for tuple
#include <type_traits>                  // for move
#include <xtensor/xarray.hpp>           // for xarray_container
#include <xtensor/xcontainer.hpp>       // for xcontainer
#include <xtensor/xlayout.hpp>          // for layout_type, layout_type::row...
#include <xtensor/xtensor_forward.hpp>  // for xarray

#include "lmisolver/ldlt_ext.hpp"  // for ldlt_ext
// #include <xtensor-blas/xlinalg.hpp>

/**
 * @brief Construct a new lmi0 oracle object
 *
 * @param[in] F
 */
template <typename Arr036> lmi0_oracle<Arr036>::lmi0_oracle(gsl::span<const Arr036> F)
    : _F{F}, _n{F[0].shape()[0]}, _Q(_n) {}

/**
 * @brief
 *
 * @param[in] x
 * @return auto
 */
template <typename Arr036> auto lmi0_oracle<Arr036>::operator()(const Arr036& x)
    -> std::optional<typename lmi0_oracle<Arr036>::Cut> {
    auto n = x.size();

    auto getA = [&, this](size_t i, size_t j) -> double {
        auto a = 0.;
        for (auto k = 0U; k != n; ++k) {
            a += this->_F[k](i, j) * x(k);
        }
        return a;
    };

    if (this->_Q.factor(getA)) {
        return {};
    }
    auto ep = this->_Q.witness();
    auto g = Arr036{xt::zeros<double>({n})};
    for (auto i = 0U; i != n; ++i) {
        g(i) = -_Q.sym_quad(this->_F[i]);
    }
    return {{std::move(g), ep}};
}

using Arr = xt::xarray<double, xt::layout_type::row_major>;
template class lmi0_oracle<Arr>;