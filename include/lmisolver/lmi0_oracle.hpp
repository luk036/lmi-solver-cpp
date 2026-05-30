#pragma once

#include <lmisolver/ldlt_mgr.hpp>
#include <memory>
#include <vector>

namespace lmi {

    template <typename Vec, typename Mat = Eigen::MatrixXd>
        requires detail::VecConcept<Vec> && detail::MatConcept<Mat>
    class Lmi0Oracle {
        using Cut = std::pair<Vec, double>;

      public:
        LDLTMgr _mq;

      private:
        const std::vector<Mat>& m_F;
        std::unique_ptr<Cut> cut = std::make_unique<Cut>();

      public:
        Lmi0Oracle(std::size_t ndim, const std::vector<Mat>& F) : _mq(ndim), m_F{F} {}

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

        auto operator()(const Vec& x) -> Cut* { return assess_feas(x); }
    };

}  // namespace lmi
