#pragma once

/** @file lmi_oracle.hpp
 *  @brief Linear Matrix Inequality (LMI) oracle for feasibility optimization.
 */

#include <lmisolver/ldlt_mgr.hpp>
#include <lmisolver/lmi_oracle_base.hpp>
#include <memory>
#include <utility>
#include <vector>

namespace lmi {

    /**
     * @brief Oracle for standard LMI feasibility problems.
     *
     * Evaluates the feasibility of a point x with respect to the LMI
     *   F0 - sum(x_k * F_k) ≻ 0
     * and returns a cutting-plane (subgradient, ep) when the LMI is violated.
     *
     * @note Concrete LMI oracle in the Template Method pattern: supplies a
     *       lazy `getA` accessor (F0 - Σ F_k x_k) with a positive `sym_quad`
     *       sign to the shared LmiOracleBase::assess_impl skeleton.
     *
     * @tparam Vec Vector type satisfying VecConcept.
     * @tparam Mat Matrix type satisfying MatConcept (default: Eigen::MatrixXd).
     */
    template <typename Vec, typename Mat = Eigen::MatrixXd>
        requires detail::VecConcept<Vec> && detail::MatConcept<Mat>
    class LmiOracle : public LmiOracleBase<Vec, Mat> {
        using Base = LmiOracleBase<Vec, Mat>;
        using Cut = std::pair<Vec, double>;

        LDLTMgr _mgr;
        Mat m_F0;

      public:
        /**
         * @brief Construct an LMI oracle.
         * @param[in] ndim Dimension of the decision variable x.
         * @param[in] F Vector of constraint matrices F_k.
         * @param[in] B Constant matrix F0 (moved into the oracle).
         */
        LmiOracle(std::size_t ndim, const std::vector<Mat>& F, Mat B)
            : Base{F}, _mgr{static_cast<Eigen::Index>(ndim)}, m_F0{std::move(B)} {}

        /**
         * @brief Assess feasibility of point x.
         *
         * Constructs A = F0 - sum(x_k * F_k) and checks if it is SPD.
         * If not, computes a witness direction (cut) for the cutting-plane method.
         *
         * @f[
         *     A(x) = F_0 - \sum_{k=1}^{n} x_k F_k \succ 0
         * @f]
         *
         * @param[in] x Decision variable vector.
         * @return Pointer to a (gradient, ep) cut, or nullptr if feasible.
         */
        auto assess_feas(const Vec& x) -> Cut* {
            const auto n = x.size();
            auto getA = [&n, &x, this](Eigen::Index i, Eigen::Index j) -> double {
                auto a = this->m_F0(i, j);
                for (auto k = Eigen::Index{0}; k != n; ++k)
                    a -= this->m_F[static_cast<std::size_t>(k)](i, j) * x[k];
                return a;
            };
            return this->assess_impl(this->_mgr, +1, x, getA);
        }

        /**
         * @brief Function-call operator (delegates to assess_feas).
         * @param[in] x Decision variable vector.
         * @return Pointer to a (gradient, ep) cut, or nullptr if feasible.
         */
        auto operator()(const Vec& x) -> Cut* { return assess_feas(x); }
    };

}  // namespace lmi
