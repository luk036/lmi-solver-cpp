#include <Eigen/Dense>
#include <iostream>

#include <lmisolver/ldlt_mgr.hpp>
#include <lmisolver/lmi_oracle.hpp>

auto main() -> int {
    auto mgr = lmi::LDLTMgr(3);
    const auto I = Eigen::MatrixXd::Identity(3, 3);
    const auto ok = mgr.factorize(I);
    if (!ok || !mgr.is_spd()) {
        std::cerr << "LDLT factorization of identity failed\n";
        return 1;
    }
    std::cout << "LmiSolver installed smoke test passed\n";
    return 0;
}
