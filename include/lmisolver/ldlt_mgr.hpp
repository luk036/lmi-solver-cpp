#pragma once

#include <Eigen/Dense>
#include <cassert>
#include <cstddef>
#include <utility>

/// LDLT factorization using Eigen storage.
///
/// LDLTMgr performs LDL^T factorization for a symmetric matrix.
/// The matrix is accessed lazily via a callable, avoiding explicit assembly.
/// Uses Eigen::MatrixXd for internal storage.
class LDLTMgr {
  public:
    using Vec = Eigen::VectorXd;
    using Rng = std::pair<std::size_t, std::size_t>;

  public:
    Rng pos{0U, 0U};
    Vec witness_vec;
    std::size_t _n;

  private:
    Eigen::MatrixXd T;

  public:
    explicit LDLTMgr(std::size_t N) : witness_vec(N), _n{N}, T(N, N) {}

    LDLTMgr(const LDLTMgr&) = delete;
    LDLTMgr& operator=(const LDLTMgr&) = delete;
    LDLTMgr(LDLTMgr&&) = default;
    LDLTMgr& operator=(LDLTMgr&&) = delete;
    ~LDLTMgr() = default;

    template <typename Mat> auto factorize(const Mat& A) -> bool {
        return this->factor([&A](std::size_t i, std::size_t j) { return A(i, j); });
    }

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
                for (auto k = start; k != s; ++k) {
                    d -= this->T(i, k) * this->T(k, s);
                }
            }
            this->T(i, i) = d;

            if (d <= 0.0) {
                stop = i + 1;
                break;
            }
        }

        return this->is_spd();
    }

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
                for (auto k = start; k != s; ++k) {
                    d -= this->T(i, k) * this->T(k, s);
                }
            }
            this->T(i, i) = d;

            if (d < 0.0) {
                stop = i + 1;
                break;
            }
            if (d == 0.0) {
                start = i + 1;
            }
        }
        return this->is_spd();
    }

    constexpr auto is_spd() const noexcept -> bool { return this->pos.second == 0; }

    auto witness() -> double;

    template <typename Arr036> auto set_witness_vec(Arr036& v) const -> void {
        for (auto i = 0U; i != this->_n; ++i) {
            v[i] = this->witness_vec[i];
        }
    }

    template <typename Mat> auto sym_quad(const Mat& A) const -> double {
        auto res = double{};
        const auto& v = this->witness_vec;
        const auto& start = this->pos.first;
        const auto& stop = this->pos.second;
        for (auto i = start; i != stop; ++i) {
            auto s = 0.0;
            for (auto j = i + 1; j != stop; ++j) {
                s += A(i, j) * v[j];
            }
            res += v[i] * (A(i, i) * v[i] + 2.0 * s);
        }
        return res;
    }

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
