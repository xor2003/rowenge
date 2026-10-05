// BoB leaf tests: h/mathasm.h ported inline functions (T107 family 1:
// fixed-point/bit ops). Cases adapted from migalley tests/unit/unit_tests.cpp
// test_mathasm_{bits,muldiv,misc} — same function names and contracts, so the
// MiG fix evidence transfers directly. Any divergence here is a real BoB
// defect, not a test bug.
#include "../harness.h"
#include "DOSDefs.h"
#include "mathasm.h"
#include "myvector.h"

REGISTER_TEST(test_bob_mathasm_bits) {
    ULong w[2] = {0, 0};

    CHECK_EQ(BITSET(w, 5), 0);
    CHECK_EQ(w[0], 0x20);
    CHECK_EQ(BITSET(w, 5), 1);              // already set -> reports 1
    CHECK_EQ(BITSET(w, 63), 0);             // crosses into w[1]
    CHECK_EQ(w[1], 0x80000000UL);
    CHECK_EQ(BITRESET(w, 5), 1);
    CHECK_EQ(w[0], 0);
    CHECK_EQ(BITRESET(w, 63), 1);
    CHECK_EQ(w[1], 0);

    w[0] = 0x21;
    CHECK_EQ(BITTEST(w, 0), 1);
    CHECK_EQ(BITTEST(w, 5), 1);
    CHECK_EQ(BITTEST(w, 1), 0);
    CHECK_EQ(BITTEST(w, 32), 0);

    CHECK_EQ(BITCOMP(w, 3), 0);             // was clear -> sets
    CHECK_EQ(w[0], 0x29);
    CHECK_EQ(BITCOMP(w, 3), 1);             // was set -> clears
    CHECK_EQ(w[0], 0x21);

    CHECK_EQ(BITSETI(0, 7), 0x80);
    CHECK_EQ(BITRESETI(0xFF, 3), 0xF7);
    CHECK_EQ(BITCOMPI(0xFF, 3), 0xF7);
    CHECK_EQ(BITTESTI(0xFF, 3), 1);
    CHECK_EQ(BITTESTI(0xF7, 3), 0);
    CHECK_EQ(BITSETI(0, 39), 0x80);         // index masks to 5 bits

    CHECK_EQ(BITSCANLOWEST(0, 99), 99);
    CHECK_EQ(BITSCANLOWEST(0x28, 9), 3);
    CHECK_EQ(BITSCANHIGHEST(0, 77), 77);
    CHECK_EQ(BITSCANHIGHEST(0x28, 9), 5);
    CHECK_EQ(BITSCANHIGHEST(0x80000000UL, 0), 31);
}

REGISTER_TEST(test_bob_mathasm_muldiv) {
    CHECK_EQ(MULSHUNS(0x8000, 0x8000, 16), 0x4000UL);
    CHECK_EQ(MULSHUNS(0x40000, 0x40000, 16), 0x100000UL);
    CHECK_EQ((ULong)MULSHSIN(-0x8000, 0x8000, 16), 0xFFFFC000UL);
    CHECK_EQ((ULong)MULSHSIN(-1, 65536, 31), 0xFFFFFFFFUL);

    CHECK_EQ(SHDIVUNS(1, 16, 2), 0x8000UL);
    CHECK_EQ(SHDIVUNS(65536, 0, 65536), 1);
    CHECK_EQ(SHDIVUNS(12345, 5, 0), 0);          // div-0 guard (FT011 fix: was SIGFPE)
    CHECK_EQ(SHDIVUNS(1, 40, 7), (ULong)((1UL << (40 & 31)) / 7)); // shl masks to 5 bits
    CHECK_EQ((ULong)SHDIVSIN(-65536, 0, 2), (ULong)-32768);

    CHECK_EQ(MULDIVUNS(6, 7, 2), 21);
    CHECK_EQ(MULDIVUNS(0x8000, 0x8000, 3), 0x15555555UL);
    CHECK_EQ(MULDIVSIN(-6, 7, 2), -21);
    CHECK_EQ(MULDIVSIN(6, -7, 2), -21);
    CHECK_EQ(MULDIVSIN(-6, -7, 2), 21);
}

REGISTER_TEST(test_bob_mathasm_misc) {
    CHECK_EQ(mathlib_w_getsign(-5), -1);
    CHECK_EQ(mathlib_w_getsign(0), 0);
    CHECK_EQ(mathlib_w_getsign(5), 0);
    CHECK_EQ(mathlib_w_applysign(7, -1), -7);
    CHECK_EQ(mathlib_w_applysign(7, 0), 7);
    CHECK_EQ(mathlib_l_getsign(-9), -1);
    CHECK_EQ(mathlib_l_getsign(9), 0);
    CHECK_EQ(mathlib_l_applysign(7, -1), -7);
    CHECK_EQ(mathlib_l_applysign(-7, -1), 7);

    ULong src[4] = {1, 2, 3, 4}, dst[4] = {0, 0, 0, 0};
    repmovsd(src, dst, 4);
    CHECK_EQ(dst[0], 1); CHECK_EQ(dst[3], 4);

    // x87 control word (32-bit x87 exists on i386).
    UWord saved = GETFPCW();
    CHECK_EQ(saved, 0x37F);
    CHECK_EQ(GETPREC(), 3);
    SETPREC(2); CHECK_EQ(GETPREC(), 2);     // exercises the RERUN fpu_cw fix
    SETPREC(3); CHECK_EQ(GETPREC(), 3);
    SETFPCW(saved);
}

// FT042: vector::magnitude() swap bug — when |c| is the largest component the
// old code lost the previous max (min := new max), inflating the result.
// Reference: documented formula MAX + 11/32·MED + 1/4·MIN with floor shifts.
REGISTER_TEST(test_bob_vector_magnitude) {
    // z-dominant case that triggered the bug: a=100,b=50,c=300 ->
    // max=300 med=100 min=50 -> 300 + (25+6+3) + 12 = 346 (buggy gave 409)
    vector v(100, 50, 300);
    CHECK_EQ(v.magnitude(), 346);
    // x/y-dominant cases were correct even before the fix
    vector v2(300, 100, 50);
    CHECK_EQ(v2.magnitude(), 346);
    vector v3(50, 300, 100);
    CHECK_EQ(v3.magnitude(), 346);
    // pure axes / zero
    vector v4(0, 0, 0);
    CHECK_EQ(v4.magnitude(), 0);
    vector v5(0, 0, -64);
    CHECK_EQ(v5.magnitude(), 64);
    // negative components use abs values: a=-300 -> max=300 med=100 min=50
    vector v6(-300, 50, 100);
    CHECK_EQ(v6.magnitude(), 346);
}
