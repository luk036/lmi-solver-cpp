#include <stddef.h>  // for size_t

// #include <ellalgo/utility.hpp>          // for zeros
#include <gsl/span>                      // for span, span<>::element_type
#include <lmisolver/lmi_old_oracle.hpp>  // for lmi_old_oracle<Arr036>::Arr, lmi_old_oracle<Arr036>::Cut
#include <optional>                      // for optional
#include <tuple>                         // for tuple
#include <type_traits>                   // for move
#include <xtensor/xarray.hpp>            // for xarray_container
#include <xtensor/xcontainer.hpp>        // for xcontainer
#include <xtensor/xlayout.hpp>           // for layout_type, layout_type::row...
#include <xtensor/xtensor_forward.hpp>  // for xarray

#include "lmisolver/ldlt_ext.hpp"  // for ldlt_ext
// #include <xtensor-blas/xlinalg.hpp>

/**
 * @brief Construct a new lmi oracle object
 *
 * @param[in] F
 * @param[in] B
 */
template <typename Arr036>
lmi_old_oracle<Arr036>::lmi_old_oracle(gsl::span<const Arr036> F, Arr036 B)
    : _F{F}, _F0{std::move(B)}, _Q{this->_F0.shape()[0]} {}

/**
 * @brief
 *
 * @param[in] x
 * @return std::optional<Cut>
 */
template <typename Arr036> auto lmi_old_oracle<Arr036>::operator()(const Arr036& x)
    -> std::optional<typename lmi_old_oracle<Arr036>::Cut> {
    const auto n = x.size();

    auto A = Arr036{this->_F0};
    for (auto k = 0U; k != n; ++k) {
        A -= this->_F[k] * x(k);
    }

    if (this->_Q.factorize(A)) {
        return {};
    }
    const auto ep = this->_Q.witness();

    auto g = Arr036{xt::zeros<double>({x.size()})};
    for (auto i = 0U; i != n; ++i) {
        g(i) = this->_Q.sym_quad(this->_F[i]);
    }
    return {{std::move(g), ep}};
}

using Arr = xt::xarray<double, xt::layout_type::row_major>;
template class lmi_old_oracle<Arr>;