#include <stddef.h>  // for size_t

#include <ellalgo/utility.hpp>          // for zeros
#include <gsl/span>                     // for span, span<>::element_type
#include <lmisolver/lmi_oracle.hpp>     // for lmi_oracle::Arr, lmi_oracle::Cut
#include <optional>                     // for optional
#include <tuple>                        // for tuple
#include <type_traits>                  // for move
#include <xtensor/xarray.hpp>           // for xarray_container
#include <xtensor/xcontainer.hpp>       // for xcontainer
#include <xtensor/xlayout.hpp>          // for layout_type, layout_type::row...
#include <xtensor/xtensor_forward.hpp>  // for xarray

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
auto lmi_oracle::operator()(const Arr& x) -> std::optional<Cut> {
    const auto n = x.size();

    auto getA = [&, this](size_t i, size_t j) -> double {
        auto a = this->_F0(i, j);
        for (auto k = 0U; k != n; ++k) {
            a -= this->_F[k](i, j) * x(k);
        }
        return a;
    };

    if (this->_Q.factor(getA)) {
        return {};
    }
    auto ep = this->_Q.witness();
    auto g = zeros({x.size()});
    for (auto i = 0U; i != n; ++i) {
        g(i) = this->_Q.sym_quad(this->_F[i]);
    }
    return {{std::move(g), ep}};
}
