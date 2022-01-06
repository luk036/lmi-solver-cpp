#include <ellalgo/utility.hpp>
#include <lmisolver/lmi0_oracle.hpp>
// #include <xtensor-blas/xlinalg.hpp>

using Arr = xt::xarray<double, xt::layout_type::row_major>;
using Cut = std::tuple<Arr, double>;

/**
 * @brief
 *
 * @param[in] x
 * @return auto
 */
auto lmi0_oracle::operator()(const Arr& x) -> std::optional<Cut> {
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
    auto g = zeros(x);
    for (auto i = 0U; i != n; ++i) {
        g(i) = -_Q.sym_quad(this->_F[i]);
    }
    return {{std::move(g), ep}};
}
