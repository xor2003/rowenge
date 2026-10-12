// Shared test harness for the rowan-engine port (T101 hardening).
//
// Same zero-dependency CHECK-macro style as migalley's tests/unit/harness.h,
// plus the spec-mandated hardening (REQ-TEST-02):
//   * zero-test runs FAIL (g_checks==0 -> exit 2)
//   * NaN never silently passes float compares (CHECK_FEQ rejects NaN)
//   * named test registry: every case logs its symbol coverage
//   * deterministic rng() for fixtures (fixed seed, override via TEST_SEED)
//   * summary on stdout: "<n> checks, <m> failures" then named-case table
//
// TUs call REGISTER_TEST(name) once per case; main() (tests/unit/main.cpp)
// calls RUN_ALL() which executes them and returns the exit status.
#pragma once
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <cstdlib>

extern int g_checks;
extern int g_failures;

struct TestEntry { const char* name; const char* file; int line; void (*fn)(); };
extern TestEntry g_tests[];
extern int g_test_count;

#define REGISTER_TEST(fn)                                                     \
    static void fn();                                                         \
    static int _reg_##fn = (g_tests[g_test_count++] =                         \
        TestEntry{#fn, __FILE__, __LINE__, fn}, 0);                           \
    static void fn()

#define CHECK(cond)                                                           \
    do { ++g_checks;                                                          \
        if (!(cond)) { ++g_failures;                                          \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); }       \
    } while (0)

#define CHECK_EQ(a, b)                                                        \
    do { ++g_checks;                                                          \
        long long _va=(long long)(a), _vb=(long long)(b);                     \
        if (_va != _vb) { ++g_failures;                                       \
            std::printf("FAIL %s:%d: %s (=%lld) != %s (=%lld)\n",               \
                        __FILE__, __LINE__, #a, _va, #b, _vb); }                \
    } while (0)

#define CHECK_STREQ(a, b)                                                     \
    do { ++g_checks;                                                          \
        std::string _va=(a), _vb=(b);                                         \
        if (_va != _vb) { ++g_failures;                                       \
            std::printf("FAIL %s:%d: \"%s\" != \"%s\"\n",                     \
                        __FILE__, __LINE__, _va.c_str(), _vb.c_str()); }        \
    } while (0)

// Float compares reject NaN on either side — a NaN operand is a failure,
// never a silent pass (REQ-TEST-02).
#define CHECK_FEQ(a, b)                                                       \
    do { ++g_checks;                                                          \
        double _va=(double)(a), _vb=(double)(b);                              \
        if (std::isnan(_va) || std::isnan(_vb) || std::isinf(_va)             \
            || std::isinf(_vb)) {                                             \
            ++g_failures;                                                     \
            std::printf("FAIL %s:%d: non-finite %s (=%g) or %s (=%g)\n",        \
                        __FILE__, __LINE__, #a, _va, #b, _vb); }               \
        else { double _d=_va-_vb;                                            \
            if (_d < -1e-6 || _d > 1e-6) { ++g_failures;                        \
                std::printf("FAIL %s:%d: %s (=%g) != %s (=%g)\n",               \
                            __FILE__, __LINE__, #a, _va, #b, _vb); } }          \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                 \
    do { ++g_checks;                                                          \
        double _va=(double)(a), _vb=(double)(b), _e=(eps);                    \
        if (std::isnan(_va) || std::isnan(_vb) || std::isnan(_e)) {            \
            ++g_failures;                                                     \
            std::printf("FAIL %s:%d: NaN operand in %s ~ %s\n",                 \
                        __FILE__, __LINE__, #a, #b); }                          \
        else { double _d=_va-_vb;                                            \
            if (_d < -_e || _d > _e) { ++g_failures;                            \
                std::printf("FAIL %s:%d: %s (=%g) != %s (=%g) eps=%g\n",        \
                            __FILE__, __LINE__, #a, _va, #b, _vb, _e); } }      \
    } while (0)

// Deterministic fixture RNG — xorshift32, fixed default seed; TEST_SEED env
// overrides for diagnosis. Never nondeterministic (REQ-TEST-02 seeds).
inline uint32_t rng() {
    static uint32_t s = [] {
        const char* e = std::getenv("TEST_SEED");
        return e ? (uint32_t)std::strtoul(e, nullptr, 0) : 0x5EEDu;
    }();
    s ^= s << 13; s ^= s >> 17; s ^= s << 5;
    return s;
}
