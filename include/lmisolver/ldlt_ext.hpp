// -*- coding: utf-8 -*-
#pragma once

#include <cstddef>  // for size_t
#include <utility>  // for pair

/**
 * @brief LDLT factorization for LMI
 *
 *  - LDL^T square-root-free version
 *  - Option allow semidefinite
 *  - A matrix A in R^{m x m} is positive definite iff v' A v > 0
 *      for all v in R^n.
 *  - O(p^2) per iteration, independent of N
 */
template <typename Arr036> class ldlt_ext {
    using Vec = Arr036;
    using Mat = Arr036;
    using Rng = std::pair<size_t, size_t>;

  public:
    Rng p{0U, 0U};    //!< the rows where the process starts and stops
    Vec witness_vec;  //!< witness vector

  private:
    const size_t _n;  //!< dimension
    Mat T;            //!< temporary storage

    // static Vec zeros_vec(size_t n);
    // static Mat zeros_mat(size_t n);

  public:
    /**
     * @brief Construct a new ldlt ext object
     *
     * @param[in] N dimension
     */
    explicit ldlt_ext(size_t N);

    ldlt_ext(const ldlt_ext&) = delete;
    ldlt_ext& operator=(const ldlt_ext&) = delete;
    ldlt_ext(ldlt_ext&&) = default;

    /**
     * @brief Perform LDLT Factorization
     *
     * @param[in] A Symmetric Matrix
     *
     * If $A$ is positive definite, then $p$ is zero.
     * If it is not, then $p$ is a positive integer,
     * such that $v = R^-1 e_p$ is a certificate vector
     * to make $v'*A[:p,:p]*v < 0$
     */
    auto factorize(const Mat& A) -> bool {
        return this->factor([&](size_t i, size_t j) { return A(i, j); });
    }

    /**
     * @brief Perform LDLT Factorization (Lazy evaluation)
     *
     * @tparam Fn
     * @param[in] getA function to access the elements of A
     *
     * See also: factorize()
     */
    template <typename Callable, bool Allow_semidefinite = false> auto factor(Callable&& getA)
        -> bool {
        this->p = {0U, 0U};
        auto& [start, stop] = this->p;

        for (auto i = 0U; i != this->_n; ++i) {
            // auto j = start;
            auto d = getA(i, start);
            for (auto j = start; j != i; ++j) {
                this->T(j, i) = d;
                this->T(i, j) = d / this->T(j, j);  // note: T(j, i) here!
                auto s = j + 1;
                d = getA(i, s);
                for (auto k = start; k != s; ++k) {
                    d -= this->T(i, k) * this->T(k, s);
                }
            }
            this->T(i, i) = d;

            if constexpr (Allow_semidefinite) {
                if (d < 0.) {
                    // this->stop = i + 1;
                    stop = i + 1;
                    break;
                }
                if (d == 0.) {
                    start = i + 1;
                    // restart at i + 1, special as an LMI oracle
                }
            } else {  // not Allow_semidefinite
                if (d <= 0.) {
                    stop = i + 1;
                    break;
                }
            }
        }

        return this->is_spd();
    }

    /**
     * @brief Is $A$ symmetric positive definite (spd)
     *
     * @return true
     * @return false
     */
    auto is_spd() const noexcept -> bool { return this->p.second == 0; }

    /**
     * @brief witness that certifies $A$ is not
     * symmetric positive definite (spd)
     *
     * @return auto
     */
    auto witness() -> double;

    /**
     * @brief Calculate v'*{A}(p,p)*v
     *
     * @param[in] A
     * @return double
     */
    [[nodiscard]] auto sym_quad(const Vec& A) const -> double;

    auto sqrt() -> Mat;
};
