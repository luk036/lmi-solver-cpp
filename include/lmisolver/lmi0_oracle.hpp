#pragma once

/** @file lmi0_oracle.hpp
 *  @brief Homogeneous Linear Matrix Inequality (LMI) oracle for feasibility optimization.
 */

#include <lmisolver/ldlt_mgr.hpp>
#include <lmisolver/lmi_oracle_base.hpp>
#include <memory>
#include <utility>
#include <vector>

namespace lmi {

    /**
     * @brief Oracle for homogeneous LMI feasibility problems.
     *
     * Evaluates feasibility of x with respect to the homogeneous LMI
     *   sum(x_k * F_k) ≻ 0
     * and returns a cutting-plane when the LMI is violated.
     *
     * @note Concrete LMI oracle in the Template Method pattern: supplies a
     *       lazy `getA` accessor (Σ F_k x_k) with a negative `sym_quad` sign
     *       to the shared LmiOracleBase::assess_impl skeleton.
     *
     * @tparam Vec Vector type satisfying VecConcept.
     * @tparam Mat Matrix type satisfying MatConcept (default: Eigen::MatrixXd).
     */
    template <typename Vec, typename Mat = Eigen::MatrixXd>
        requires detail::VecConcept<Vec> && detail::MatConcept<Mat>
    class Lmi0Oracle : public LmiOracleBase<Vec, Mat> {
        using Base = LmiOracleBase<Vec, Mat>;
        using Cut = std::pair<Vec, double>;

      public:
        LDLTMgr _mq;  ///< LDL^T manager for the homogeneous matrix

      public:
        /**
         * @brief Construct a homogeneous LMI oracle.
         * @param[in] ndim Dimension of the decision variable x.
         * @param[in] F Vector of constraint matrices F_k.
         */
        Lmi0Oracle(std::size_t ndim, const std::vector<Mat>& F)
            : Base{F}, _mq(static_cast<Eigen::Index>(ndim)) {}

        /**
         * @brief Assess feasibility of point x for the homogeneous LMI.
         *
         * Constructs A = sum(x_k * F_k) and checks if it is SPD.
         * If not, computes a witness direction (cut) for the cutting-plane method.
         *
         * @f[
         *     A(x) = \sum_{k=1}^{n} x_k F_k \succ 0
         * @f]
         *
         * @param[in] x Decision variable vector.
         * @return Pointer to a (gradient, ep) cut, or nullptr if feasible.
         */
        auto assess_feas(const Vec& x) -> Cut* {
            const auto n = x.size();
            auto getA = [&n, &x, this](Eigen::Index i, Eigen::Index j) -> double {
                auto a = 0.0;
                for (auto k = Eigen::Index{0}; k != n; ++k)
                    a += this->m_F[static_cast<std::size_t>(k)](i, j) * x[k];
                return a;
            };
            return this->assess_impl(this->_mq, -1, x, getA);
        }

        /**
         * @brief Function-call operator (delegates to assess_feas).
         * @param[in] x Decision variable vector.
         * @return Pointer to a (gradient, ep) cut, or nullptr if feasible.
         */
        auto operator()(const Vec& x) -> Cut* { return assess_feas(x); }
    };

}  // namespace lmi
