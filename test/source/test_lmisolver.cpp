#define DOCTEST_CONFIG_USE_STD_HEADERS
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#include <doctest/doctest.h>

#include <Eigen/Dense>
#include <lmisolver/lmi0_oracle.hpp>
#include <lmisolver/lmi_oracle.hpp>

TEST_CASE("lmi::LDLTMgr: identity matrix is SPD") {
    auto mgr = lmi::LDLTMgr(3);

    auto I = Eigen::MatrixXd::Identity(3, 3);
    auto ok = mgr.factorize(I);

    CHECK(ok);
    CHECK(mgr.is_spd());
}

TEST_CASE("lmi::LDLTMgr: negative definite fails") {
    auto mgr = lmi::LDLTMgr(2);

    auto A = Eigen::MatrixXd(2, 2);
    A(0, 0) = -1.0;
    A(0, 1) = 0.0;
    A(1, 0) = 0.0;
    A(1, 1) = -2.0;

    auto ok = mgr.factorize(A);
    CHECK(!ok);
    CHECK(!mgr.is_spd());
    CHECK(mgr.witness() > 0.0);
}

TEST_CASE("lmi::LDLTMgr: witness and sym_quad") {
    auto mgr = lmi::LDLTMgr(2);

    // A = [[2, 0], [0, -1]] is not SPD
    auto A = Eigen::MatrixXd(2, 2);
    A(0, 0) = 2.0;
    A(0, 1) = 0.0;
    A(1, 0) = 0.0;
    A(1, 1) = -1.0;

    auto ok = mgr.factorize(A);
    CHECK(!ok);

    auto ep = mgr.witness();
    CHECK(ep > 0.0);

    // sym_quad should be positive for the violating part
    auto B = Eigen::MatrixXd(2, 2);
    B(0, 0) = 1.0;
    B(0, 1) = 0.0;
    B(1, 0) = 0.0;
    B(1, 1) = 1.0;
    auto sq = mgr.sym_quad(B);
    CHECK(sq >= 0.0);
}

TEST_CASE("lmi::Lmi0Oracle: infeasible point returns cut") {
    using Vec = Eigen::VectorXd;
    using Mat = Eigen::MatrixXd;

    // F = [I, I] -> F0 + x1*I + x2*I <= 0
    // At x = [-1, -1], A = -2I, which is <= 0 (feasible)
    // At x = [1, 1], A = 2I, which is > 0 (infeasible)
    auto F1 = Mat::Identity(2, 2);
    auto F2 = Mat::Identity(2, 2);
    auto F = std::vector<Mat>{F1, F2};
    auto oracle = lmi::Lmi0Oracle<Vec, Mat>(2, F);

    // At x=0, A=0 (zero matrix cannot be factorized as SPD)
    // so factor returns false, assess_feas proceeds to compute a cut
    // This is a known limitation for the zero-matrix case
    auto x0 = Vec::Zero(2);
    auto cut0 = oracle.assess_feas(x0);
    CHECK(cut0 != nullptr);
}

TEST_CASE("lmi::LmiOracle: two-constraint LMI problem") {
    using Vec = Eigen::VectorXd;
    using Mat = Eigen::MatrixXd;

    // Problem: find x s.t. (B - F*x) >= 0
    // Same data as ellalgo-cpp test_lmi.cpp

    // First LMI: 2x2 matrices
    auto m0F1 = Mat(2, 2);
    m0F1 << -7.0, -11.0, -11.0, 3.0;
    auto m1F1 = Mat(2, 2);
    m1F1 << 7.0, -18.0, -18.0, 8.0;
    auto m2F1 = Mat(2, 2);
    m2F1 << -2.0, -8.0, -8.0, 1.0;
    auto F1 = std::vector<Mat>{m0F1, m1F1, m2F1};

    auto B1 = Mat(2, 2);
    B1 << 33.0, -9.0, -9.0, 26.0;

    // Second LMI: 3x3 matrices
    auto m0F2 = Mat(3, 3);
    m0F2 << -21.0, -11.0, 0.0, -11.0, 10.0, 8.0, 0.0, 8.0, 5.0;
    auto m1F2 = Mat(3, 3);
    m1F2 << 0.0, 10.0, 16.0, 10.0, -10.0, -10.0, 16.0, -10.0, 3.0;
    auto m2F2 = Mat(3, 3);
    m2F2 << -5.0, 2.0, -17.0, 2.0, -6.0, 8.0, -17.0, 8.0, 6.0;
    auto F2 = std::vector<Mat>{m0F2, m1F2, m2F2};

    auto B2 = Mat(3, 3);
    B2 << 14.0, 9.0, 40.0, 9.0, 91.0, 10.0, 40.0, 10.0, 15.0;

    auto lmi1 = lmi::LmiOracle<Vec, Mat>(2, F1, B1);
    auto lmi2 = lmi::LmiOracle<Vec, Mat>(3, F2, B2);

    // Test an infeasible point
    auto x = Vec(3);
    x << 1.0, 1.0, 1.0;

    auto cut1 = lmi1(x);
    CHECK(cut1 != nullptr);
    CHECK(cut1->second > 0.0);

    // Test at origin (should be infeasible for at least one constraint)
    auto x0 = Vec::Zero(3);
    auto cut2 = lmi2(x0);
    CHECK(cut2 != nullptr);
}
