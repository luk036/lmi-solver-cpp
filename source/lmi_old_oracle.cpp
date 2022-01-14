// #include <ellalgo/utility.hpp>           // for zeros
#include <gsl/span>                      // for span<>::reference, span, spa...
#include <lmisolver/lmi_old_oracle.hpp>  // for lmi_old_oracle::Arr, lmi_old...
#include <optional>                      // for optional
#include <tuple>                         // for tuple
#include <type_traits>                   // for move
#include <xtensor/xarray.hpp>            // for xarray_container
#include <xtensor/xcontainer.hpp>        // for xcontainer<>::const_reference
#include <xtensor/xlayout.hpp>           // for layout_type, layout_type::ro...
#include <xtensor/xoperation.hpp>        // for xfunction_type_t, operator-
#include <xtensor/xsemantic.hpp>         // for xsemantic_base
#include <xtensor/xtensor_forward.hpp>   // for xarray

#include "lmisolver/ldlt_ext.hpp"  // for ldlt_ext

// #include <xtensor-blas/xlinalg.hpp>

using Arr = xt::xarray<double, xt::layout_type::row_major>;
using Cut = std::tuple<Arr, double>;

/**
 * @brief
 *
 * @param[in] x
 * @return std::optional<Cut>
 */
auto lmi_old_oracle::operator()(const Arr& x) -> std::optional<Cut> {
    const auto n = x.size();

    auto A = Arr{this->_F0};
    for (auto k = 0U; k != n; ++k) {
        A -= this->_F[k] * x(k);
    }

    if (this->_Q.factorize(A)) {
        return {};
    }
    const auto ep = this->_Q.witness();
    Arr g = xt::zeros<double>(x);
    for (auto i = 0U; i != n; ++i) {
        g(i) = this->_Q.sym_quad(this->_F[i]);
    }
    return {{std::move(g), ep}};
}
