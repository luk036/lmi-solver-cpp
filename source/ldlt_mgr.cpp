#include <lmisolver/ldlt_mgr.hpp>

namespace lmi {

auto LDLTMgr::witness() -> double {
    assert(!this->is_spd());
    const auto& start = this->pos.first;
    const auto& n = this->pos.second;
    auto m = n - 1;
    this->witness_vec[m] = 1.0;
    for (auto i = m; i > start; --i) {
        this->witness_vec[i - 1] = 0.0;
        for (auto k = i; k != n; ++k)
            this->witness_vec[i - 1] -= this->T(k, i - 1) * this->witness_vec[k];
    }
    return -this->T(m, m);
}

} // namespace lmi
