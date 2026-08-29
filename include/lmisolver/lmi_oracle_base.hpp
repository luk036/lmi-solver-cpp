#pragma once

/** @file lmi_oracle_base.hpp
 *  @brief Shared LMI oracle skeleton (Template Method).
 */

#include <lmisolver/ldlt_mgr.hpp>
#include <memory>
#include <utility>
#include <vector>

namespace lmi {

    /**
     * @brief Shared skeleton for LMI feasibility oracles.
     *
     * The two LMI oracle flavors (LmiOracle, Lmi0Oracle) differ only in how
     * the matrix elements `A(i,j)` are assembled (a `getA` closure) and in the
     * `sym_quad` sign convention. The factorization / witness / cut-packing
     * pipeline is identical and lives here.
     *
     * @note Template Method pattern: `assess_impl` is the fixed algorithm
     *       skeleton (factor -> witness -> sym_quad -> pack cut); each concrete
     *       oracle supplies its own `getA` callable and sign via the parameter
     *       list. The LDL^T manager stays in the concrete oracle so every
     *       public constructor and member (e.g. `Lmi0Oracle::_mq`) is intact.
     *
     * @tparam Vec Vector type satisfying VecConcept.
     * @tparam Mat Matrix type satisfying MatConcept (default: Eigen::MatrixXd).
     */
    template <typename Vec, typename Mat = Eigen::MatrixXd>
        requires detail::VecConcept<Vec> && detail::MatConcept<Mat>
    class LmiOracleBase {
      protected:
        using Cut = std::pair<Vec, double>;

        const std::vector<Mat>& m_F;  ///< Constraint matrices F_k (non-owning)
        std::unique_ptr<Cut> cut = std::make_unique<Cut>();  ///< Cut buffer

        /**
         * @brief Construct the shared skeleton.
         * @param[in] F Vector of constraint matrices F_k (must outlive the oracle).
         */
        explicit LmiOracleBase(const std::vector<Mat>& F) : m_F{F} {}

        /**
         * @brief Shared assess_feas skeleton: factor, witness, sym_quad, pack cut.
         *
         * @tparam LDLT LDL^T manager type (LDLTMgr).
         * @tparam Fn   Callable with signature double(Eigen::Index, Eigen::Index).
         * @param[in,out] mgr  LDL^T factorization manager.
         * @param[in]     sign +1 or -1 convention for sym_quad.
         * @param[in]     x    Evaluation point.
         * @param[in]     getA Lazy accessor for matrix element A(i, j).
         * @return Pointer to the packed cut, or nullptr if feasible.
         */
        template <typename LDLT, typename Fn>
        auto assess_impl(LDLT& mgr, const int sign, const Vec& x, Fn&& getA) -> Cut* {
            const auto n = x.size();
            if (mgr.factor(std::forward<Fn>(getA))) return nullptr;
            const auto ep = mgr.witness();  // call before sym_quad() !!!
            Vec g{x};
            for (auto i = Eigen::Index{0}; i != n; ++i)
                g[i] = sign * mgr.sym_quad(this->m_F[static_cast<std::size_t>(i)]);
            this->cut->first = std::move(g);
            this->cut->second = std::move(ep);
            return this->cut.get();
        }
    };

}  // namespace lmi
