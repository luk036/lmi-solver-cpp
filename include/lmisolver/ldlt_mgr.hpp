#pragma once

/** @file ldlt_mgr.hpp
 *  @brief LDL^T factorization manager for symmetric positive-definite matrices.
 */

#include <Eigen/Dense>
#include <cassert>
#include <cstddef>
#include <utility>

namespace lmi {
    namespace detail {
        template <typename V>
        concept VecConcept = requires(V& v, const V& cv, std::size_t i) {
            { v.size() } -> std::convertible_to<std::size_t>;
            { cv[i] };
            { v[i] = double{} };
            V{cv};
            V{std::move(v)};
        };

        template <typename M>
        concept MatConcept = requires(M& m, std::size_t i, std::size_t j) {
            { m(i, j) } -> std::convertible_to<double>;
        };
    }  // namespace detail

    /**
     * @brief LDL^T factorization manager for checking symmetric positive-definiteness.
     *
     * Provides LDL^T decomposition (without square roots) of symmetric matrices.
     * Used inside LMI oracles to detect indefiniteness and compute witness vectors
     * for cutting-plane optimization.
     *
     * @details The factorization stops early when a non-positive pivot is encountered,
     * recording the position where the matrix ceases to be positive-definite. This
     * position is used to compute a witness direction for the cutting-plane oracle.
     */
    class LDLTMgr {
      public:
        using Vec = Eigen::VectorXd;                      ///< Vector type for witness vectors
        using Rng = std::pair<std::size_t, std::size_t>;  ///< (start, stop) position range

      public:
        Rng pos{0U, 0U};  ///< Current (start, stop) position in factorization
        Vec witness_vec;  ///< Witness vector for cutting-plane computation
        std::size_t _n;   ///< Matrix dimension

      private:
        Eigen::MatrixXd T;

      public:
        /**
         * @brief Construct an LDLT manager for N x N matrices.
         * @param[in] N Matrix dimension.
         */
        explicit LDLTMgr(std::size_t N) : witness_vec(N), _n{N}, T(N, N) {}
        LDLTMgr(const LDLTMgr&) = delete;
        LDLTMgr& operator=(const LDLTMgr&) = delete;
        LDLTMgr(LDLTMgr&&) = default;
        LDLTMgr& operator=(LDLTMgr&&) = delete;
        ~LDLTMgr() = default;

        /**
         * @brief Factorize a matrix A via LDL^T decomposition.
         *
         * @f[
         *     A = LDL^T
         * @f]
         *
         * @tparam Mat Matrix type with operator()(i, j) returning double.
         * @param[in] A The matrix to factorize.
         * @return true if A is symmetric positive-definite, false otherwise.
         */
        template <typename Mat> auto factorize(const Mat& A) -> bool {
            return this->factor([&A](std::size_t i, std::size_t j) { return A(i, j); });
        }

        /**
         * @brief Perform LDL^T factorization using a callback for matrix elements.
         *
         * Factorizes the matrix row by row. Stops early when a non-positive pivot
         * is encountered and records the position in `pos`.
         *
         * @f[
         *     A = LDL^T
         * @f]
         *
         * @tparam Fn Callable type: Fn(std::size_t i, std::size_t j) -> double.
         * @param[in] get_matrix_elem Callback returning matrix element A(i, j).
         * @return true if the matrix is symmetric positive-definite, false otherwise.
         */
        template <typename Fn> auto factor(Fn&& get_matrix_elem) -> bool {
            this->pos = {0U, 0U};
            auto const& start = this->pos.first;
            auto& stop = this->pos.second;
            for (auto i = 0U; i != this->_n; ++i) {
                auto d = get_matrix_elem(i, start);
                for (auto j = start; j != i; ++j) {
                    this->T(j, i) = d;
                    this->T(i, j) = d / this->T(j, j);
                    auto s = j + 1;
                    d = get_matrix_elem(i, s);
                    for (auto k = start; k != s; ++k) d -= this->T(i, k) * this->T(k, s);
                }
                this->T(i, i) = d;
                if (d <= 0.0) {
                    stop = i + 1;
                    break;
                }
            }
            return this->is_spd();
        }

        /**
         * @brief Perform LDL^T factorization allowing semi-definite matrices.
         *
         * Identical to factor() but treats zero pivots as valid (semi-definite case)
         * rather than stopping. The zero-pivot position advances the start index.
         *
         * @f[
         *     A = LDL^T
         * @f]
         *
         * @tparam Fn Callable type: Fn(std::size_t i, std::size_t j) -> double.
         * @param[in] get_matrix_elem Callback returning matrix element A(i, j).
         * @return true if the matrix is positive-(semi)definite, false otherwise.
         */
        template <typename Fn> auto factor_with_allow_semidefinite(Fn&& get_matrix_elem) -> bool {
            this->pos = {0U, 0U};
            auto& start = this->pos.first;
            auto& stop = this->pos.second;
            for (auto i = 0U; i != this->_n; ++i) {
                auto d = get_matrix_elem(i, start);
                for (auto j = start; j != i; ++j) {
                    this->T(j, i) = d;
                    this->T(i, j) = d / this->T(j, j);
                    auto s = j + 1;
                    d = get_matrix_elem(i, s);
                    for (auto k = start; k != s; ++k) d -= this->T(i, k) * this->T(k, s);
                }
                this->T(i, i) = d;
                if (d < 0.0) {
                    stop = i + 1;
                    break;
                }
                if (d == 0.0) start = i + 1;
            }
            return this->is_spd();
        }

        /**
         * @brief Check whether the last factorized matrix is SPD.
         * @return true if symmetric positive-definite, false otherwise.
         */
        constexpr auto is_spd() const noexcept -> bool { return this->pos.second == 0; }

        /**
         * @brief Compute a witness vector for the cutting-plane oracle.
         * @details Uses the stopped factorization position to solve for the
         *          direction that violates positive-definiteness.
         * @return The witness value (ep for the cutting plane).
         */
        auto witness() -> double;

        /**
         * @brief Copy the internal witness vector into an external array.
         * @tparam Arr036 Array-like type with operator[].
         * @param[out] v Destination array to receive witness vector values.
         */
        template <typename Arr036> auto set_witness_vec(Arr036& v) const -> void {
            for (auto i = 0U; i != this->_n; ++i) v[i] = this->witness_vec[i];
        }

        /**
         * @brief Compute the symmetric quadratic form v^T A v using the witness vector.
         *
         * @f[
         *     v^T A v = \sum_{i} \sum_{j} v_i A_{ij} v_j
         * @f]
         *
         * @tparam Mat Matrix type with operator()(i, j).
         * @param[in] A The matrix for the quadratic form.
         * @return The value of v^T A v restricted to the active [start, stop) range.
         */
        template <typename Mat> auto sym_quad(const Mat& A) const -> double {
            auto res = double{};
            const auto& v = this->witness_vec;
            const auto& start = this->pos.first;
            const auto& stop = this->pos.second;
            for (auto i = start; i != stop; ++i) {
                auto s = 0.0;
                for (auto j = i + 1; j != stop; ++j) s += A(i, j) * v[j];
                res += v[i] * (A(i, i) * v[i] + 2.0 * s);
            }
            return res;
        }

        /**
         * @brief Compute the Cholesky factor (lower triangular) from the LDL^T decomposition.
         *
         * @f[
         *     M = L \quad\text{where}\quad LL^T = LDL^T
         * @f]
         *
         * @tparam Mat Matrix type with operator()(i, j).
         * @param[out] M Output matrix receiving the lower triangular Cholesky factor.
         * @pre The matrix must be SPD (is_spd() == true).
         */
        template <typename Mat> auto sqrt(Mat& M) -> void {
            assert(this->is_spd());
            for (auto i = 0U; i != this->_n; ++i) {
                M(i, i) = std::sqrt(this->T(i, i));
                for (auto j = i + 1; j != this->_n; ++j) {
                    M(i, j) = this->T(j, i) * M(i, i);
                    M(j, i) = 0.0;
                }
            }
        }
    };

}  // namespace lmi
