// BoB port-header tests: compile against the REAL bob-flight-sim h/mathasm.h
// (resolved via BobIncludes -> bob_overlay), NOT shared/math/mathasm.h which
// bob_leaf_tests picks up first. This is the header every bob_game TU sees.
// Cases here must reproduce the shipped x86 asm semantics:
//   SHDIVSIN: edx:eax = num1 << cl is a FULL signed 64-bit dividend
//   (edx = num1 copied before shl, then sar edx,32-cl supplies the high
//   dword); idiv num3 treats the divisor as SIGNED 32-bit. A port that
//   truncates the shift to 32 bits and then sign-extends loses every high
//   bit -- divergence starts at |num1 << num2| >= 2^32.
#include "../harness.h"
#include "DOSDefs.h"
#include "mathasm.h"

REGISTER_TEST(test_bob_port_mathasm_shdivsin) {
    // Low-range cases both readings get right.
    CHECK_EQ((ULong)SHDIVSIN(-65536, 0, 2), (ULong)-32768);
    CHECK_EQ(SHDIVSIN(1, 16, 0), 0);                       // div0 guard
    // Signed 32-bit divisor: 0xC0000000 -> -1073741824,
    // -2147483648 / -1073741824 = 2 (unsigned div would give 0).
    CHECK_EQ(SHDIVSIN((SLong)0x80000000, 0, 0xC0000000UL), 2);

    // High-range: dividend overflows 32 bits. Shipped asm keeps the high
    // dword via sar edx,32-cl; a truncating port returns 0 or wrong sign.
    // 1 << 31 = 0x80000000; shipped idiv gives +715827882, truncation
    // gives (int32_t)0x80000000 / 3 = -715827882.
    CHECK_EQ(SHDIVSIN(1, 31, 3), 715827882);
    // 0x10000 << 16 = 0x100000000; /3 = 0x55555555, truncation gives 0.
    CHECK_EQ(SHDIVSIN(0x10000, 16, 3), 0x55555555);
    // Negative dividend: -0x10000 << 16 = -0x100000000; /3 = -0x55555555.
    CHECK_EQ(SHDIVSIN(-0x10000, 16, 3), -0x55555555);
}
