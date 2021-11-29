// -*- coding: utf-8 -*-
#include <lmisolver/ldlt_ext.hpp>
#include <stdexcept>
#include <xtensor/xtensor_config.hpp>

/**
 * @brief witness that certifies $A$ is not
 * symmetric positive definite (spd)
 *
 * @return auto
 */
auto ldlt_ext::witness() -> double {
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
auto ldlt_ext::sym_quad(const Vec& A) const -> double {
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

/**
 * @brief Return upper triangular matrix $R$ where $A = R^T R$
 *
 * @return Mat
 */
auto ldlt_ext::sqrt() -> Mat {
    assert(this->is_spd());

    auto M = zeros({this->_n, this->_n});
    for (auto i = 0U; i != this->_n; ++i) {
        M(i, i) = std::sqrt(this->T(i, i));
        for (auto j = i + 1; j != this->_n; ++j) {
            M(i, j) = this->T(j, i) * M(i, i);
        }
    }
    return M;
}
