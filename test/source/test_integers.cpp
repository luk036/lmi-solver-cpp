#include <doctest/doctest.h>

#include <limits>       // import numric_limits
#include <type_traits>  // import make_signed

using namespace std;

template <typename T> static void check_unsigned() {
    auto MAX = numeric_limits<T>::max();

    auto a = T{MAX};
    auto v = T{MAX / 2};
    auto z = T{0};
    auto a2 = make_signed_t<T>(a - v);
    auto z2 = make_signed_t<T>(z - v);
    auto a3 = -a2;
    auto z3 = -z2;
    auto a4 = -a3;
    auto z4 = -z3;
    auto a5 = T(a4 + v);
    auto z5 = T(z4 + v);

    CHECK(a == a5);
    CHECK(z == z5);
}

TEST_CASE("test unsigned long long") { check_unsigned<unsigned long long>(); }

TEST_CASE("test unsigned long") { check_unsigned<unsigned long>(); }
