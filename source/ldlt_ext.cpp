// -*- coding: utf-8 -*-
#include <cassert>  // for assert
#include <cmath>    // for sqrt
#include <lmisolver/ldlt_ext.hpp>

/**
 * @brief witness that certifies $A$ is not
 * symmetric positive definite (spd)
 *
 * @return auto
 */
template <typename Arr036> auto ldlt_ext<Arr036>::witness() -> double {
    assert(!this->is_spd());

    // const auto& [start, n] = this->p;
    const auto& start = this->p.first;
    const auto& n = this->p.second;
    auto m = n - 1;  // assume stop > 0
    this->witness_vec(m) = 1.;
    for (auto i = m; i > start; --i) {
        this->witness_vec(i - 1) = 0.;
        for (auto k = i; k != n; ++k) {
            this->witness_vec(i - 1) -= this->T(k, i - 1) * this->witness_vec(k);
        }
    }
    return -this->T(m, m);
}

/**
 * @brief Calculate v'*{A}(p,p)*v
 *
 * @param[in] A
 * @return double
 */
template <typename Arr036>
auto ldlt_ext<Arr036>::sym_quad(const typename ldlt_ext<Arr036>::Vec& A) const -> double {
    auto res = double{};
    const auto& v = this->witness_vec;
    // const auto& [start, stop] = this->p;
    const auto& start = this->p.first;
    const auto& stop = this->p.second;
    for (auto i = start; i != stop; ++i) {
        auto s = double{};
        for (auto j = i + 1; j != stop; ++j) {
            s += A(i, j) * v(j);
        }
        res += v(i) * (A(i, i) * v(i) + 2 * s);
    }
    return res;
}

#include <xtensor/xarray.hpp>           // for xarray
#include <xtensor/xcontainer.hpp>       // for xcontainer
#include <xtensor/xlayout.hpp>          // for layout_type, layout_type::row...
#include <xtensor/xtensor_forward.hpp>  // for xarray

using Arr = xt::xarray<double, xt::layout_type::row_major>;

template <typename Arr036> ldlt_ext<Arr036>::ldlt_ext(size_t N)
    : witness_vec{xt::zeros<double>({N})}, _n{N}, T{xt::zeros<double>({N, N})} {}

/**
 * @brief Return upper triangular matrix $R$ where $A = R^T R$
 *
 * @return typename ldlt_ext<Arr036>::Mat
 */
template <typename Arr036> auto ldlt_ext<Arr036>::sqrt() -> typename ldlt_ext<Arr036>::Mat {
    assert(this->is_spd());

    ldlt_ext<Arr036>::Mat M = xt::zeros<double>({this->_n, this->_n});
    for (auto i = 0U; i != this->_n; ++i) {
        M(i, i) = std::sqrt(this->T(i, i));
        for (auto j = i + 1; j != this->_n; ++j) {
            M(i, j) = this->T(j, i) * M(i, i);
        }
    }
    return M;
}

template class ldlt_ext<Arr>;