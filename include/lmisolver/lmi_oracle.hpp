#pragma once

#include <lmisolver/ldlt_mgr.hpp>

#include <memory>
#include <vector>

namespace lmi {

template <typename Vec, typename Mat = Eigen::MatrixXd> class LmiOracle {
    using Cut = std::pair<Vec, double>;

    LDLTMgr _mgr;
    const std::vector<Mat>& m_F;
    Mat m_F0;
    std::unique_ptr<Cut> cut = std::make_unique<Cut>();

  public:
    LmiOracle(std::size_t ndim, const std::vector<Mat>& F, Mat B)
        : _mgr{ndim}, m_F{F}, m_F0{std::move(B)} {}

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

    auto operator()(const Vec& x) -> Cut* { return assess_feas(x); }
};

} // namespace lmi
