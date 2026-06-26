#pragma once

/** @file lmi0_oracle.hpp
 *  @brief Homogeneous Linear Matrix Inequality (LMI) oracle for feasibility optimization.
 */

#include <lmisolver/ldlt_mgr.hpp>
#include <memory>
#include <vector>

namespace lmi {

    /**
     * @brief Oracle for homogeneous LMI feasibility problems.
     *
     * Evaluates feasibility of x with respect to the homogeneous LMI
     *   sum(x_k * F_k) ≻ 0
     * and returns a cutting-plane when the LMI is violated.
     *
     * @tparam Vec Vector type satisfying VecConcept.
     * @tparam Mat Matrix type satisfying MatConcept (default: Eigen::MatrixXd).
     */
    template <typename Vec, typename Mat = Eigen::MatrixXd>
        requires detail::VecConcept<Vec> && detail::MatConcept<Mat>
    class Lmi0Oracle {
        using Cut = std::pair<Vec, double>;

      public:
        LDLTMgr _mq;  ///< LDL^T manager for the homogeneous matrix

      private:
        const std::vector<Mat>& m_F;
        std::unique_ptr<Cut> cut = std::make_unique<Cut>();

      public:
        /**
         * @brief Construct a homogeneous LMI oracle.
         * @param[in] ndim Dimension of the decision variable x.
         * @param[in] F Vector of constraint matrices F_k.
         */
        Lmi0Oracle(std::size_t ndim, const std::vector<Mat>& F) : _mq(ndim), m_F{F} {}

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
            auto getA = [&n, &x, this](std::size_t i, std::size_t j) -> double {
                auto a = 0.0;
                for (auto k = 0U; k != n; ++k) a += this->m_F[k](i, j) * x[k];
                return a;
            };
            if (this->_mq.factor(getA)) return nullptr;
            auto ep = this->_mq.witness();
            Vec g{x};
            for (auto i = 0U; i != n; ++i) g[i] = -this->_mq.sym_quad(this->m_F[i]);
            cut->first = std::move(g);
            cut->second = std::move(ep);
            return cut.get();
        }

        /**
         * @brief Function-call operator (delegates to assess_feas).
         * @param[in] x Decision variable vector.
         * @return Pointer to a (gradient, ep) cut, or nullptr if feasible.
         */
        auto operator()(const Vec& x) -> Cut* { return assess_feas(x); }
    };

}  // namespace lmi
