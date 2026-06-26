#pragma once

/** @file lmi_oracle.hpp
 *  @brief Linear Matrix Inequality (LMI) oracle for feasibility optimization.
 */

#include <lmisolver/ldlt_mgr.hpp>
#include <memory>
#include <vector>

namespace lmi {

    /**
     * @brief Oracle for standard LMI feasibility problems.
     *
     * Evaluates the feasibility of a point x with respect to the LMI
     *   F0 - sum(x_k * F_k) ≻ 0
     * and returns a cutting-plane (subgradient, ep) when the LMI is violated.
     *
     * @tparam Vec Vector type satisfying VecConcept.
     * @tparam Mat Matrix type satisfying MatConcept (default: Eigen::MatrixXd).
     */
    template <typename Vec, typename Mat = Eigen::MatrixXd>
        requires detail::VecConcept<Vec> && detail::MatConcept<Mat>
    class LmiOracle {
        using Cut = std::pair<Vec, double>;

        LDLTMgr _mgr;
        const std::vector<Mat>& m_F;
        Mat m_F0;
        std::unique_ptr<Cut> cut = std::make_unique<Cut>();

      public:
        /**
         * @brief Construct an LMI oracle.
         * @param[in] ndim Dimension of the decision variable x.
         * @param[in] F Vector of constraint matrices F_k.
         * @param[in] B Constant matrix F0 (moved into the oracle).
         */
        LmiOracle(std::size_t ndim, const std::vector<Mat>& F, Mat B)
            : _mgr{ndim}, m_F{F}, m_F0{std::move(B)} {}

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
            auto getA = [&n, &x, this](std::size_t i, std::size_t j) -> double {
                auto a = this->m_F0(i, j);
                for (auto k = 0U; k != n; ++k) a -= this->m_F[k](i, j) * x[k];
                return a;
            };
            if (this->_mgr.factor(getA)) return nullptr;
            auto ep = this->_mgr.witness();
            Vec g{x};
            for (auto i = 0U; i != n; ++i) g[i] = this->_mgr.sym_quad(this->m_F[i]);
            this->cut->first = std::move(g);
            this->cut->second = std::move(ep);
            return this->cut.get();
        }

        /**
         * @brief Function-call operator (delegates to assess_feas).
         * @param[in] x Decision variable vector.
         * @return Pointer to a (gradient, ep) cut, or nullptr if feasible.
         */
        auto operator()(const Vec& x) -> Cut* { return assess_feas(x); }
    };

}  // namespace lmi
