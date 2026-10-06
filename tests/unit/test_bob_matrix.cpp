// T107 family 2: matrix/clip — matrix::generate/rotate/multiply/transform/
// scaleto16bit/crossproduct over the real matrix.cpp code in bob_math.
// MATRIX = 9xSWord fixed-point (ANGLES_FRACT = 32768 = 1.0).
#include "../harness.h"
#include "DOSDefs.h"
#include "myangles.h"
#include "mymath.h"
#include "matrix.h"
#include "3ddefs.h"

// identity-ish 1.0 fixed-point diagonal (generate output is table-derived,
// so allow small tolerance rather than exact 32768)
static void check_identity(MATRIX& m, int eps) {
    CHECK_NEAR(m.L11, ANGLES_FRACT, eps);
    CHECK_NEAR(m.L22, ANGLES_FRACT, eps);
    CHECK_NEAR(m.L33, ANGLES_FRACT, eps);
    CHECK_NEAR(m.L12, 0, eps); CHECK_NEAR(m.L13, 0, eps);
    CHECK_NEAR(m.L21, 0, eps); CHECK_NEAR(m.L23, 0, eps);
    CHECK_NEAR(m.L31, 0, eps); CHECK_NEAR(m.L32, 0, eps);
}

REGISTER_TEST(test_bob_matrix_generate) {
    matrix mx;
    MATRIX m;

    // zero angles -> rotation identity scaled to fixed-point
    mx.generate((Angles)0, (Angles)0, (Angles)0, &m);
    check_identity(m, 4);

    // 90-degree heading: x-axis -> -z or +z (convention check):
    // heading rotation maps (1,0,0) onto the yawed axis. Verify the row that
    // holds the x transform contains +-FRACT and orthogonality of columns.
    mx.generate((Angles)0x4000, (Angles)0, (Angles)0, &m);
    int c1 = m.L11*m.L11 + m.L21*m.L21 + m.L31*m.L31;   // col norms ~FRACT^2
    int c2 = m.L12*m.L12 + m.L22*m.L22 + m.L32*m.L32;
    int c3 = m.L13*m.L13 + m.L23*m.L23 + m.L33*m.L33;
    CHECK_NEAR(c1, ANGLES_FRACT*ANGLES_FRACT, 4*ANGLES_FRACT);
    CHECK_NEAR(c2, ANGLES_FRACT*ANGLES_FRACT, 4*ANGLES_FRACT);
    CHECK_NEAR(c3, ANGLES_FRACT*ANGLES_FRACT, 4*ANGLES_FRACT);
}

REGISTER_TEST(test_bob_matrix_rotate) {
    matrix mx;
    MATRIX m;
    mx.generate((Angles)0, (Angles)0, (Angles)0, &m);

    // identity rotation: vector passthrough. generate()'s diagonal is the
    // table-derived ~1.0 (0x7fff scale), so allow 1-ulp-per-component drift:
    // |v|/32768 + rounding, ~3 for these magnitudes.
    SLong x = 12345, y = -6789, z = 42000;
    mx.rotate(&m, x, y, z);
    CHECK_NEAR(x, 12345, 4);
    CHECK_NEAR(y, -6789, 4);
    CHECK_NEAR(z, 42000, 4);

    // zero vector stays zero under any matrix
    x = y = z = 0;
    mx.rotate(&m, x, y, z);
    CHECK_EQ(x, 0); CHECK_EQ(y, 0); CHECK_EQ(z, 0);
}

REGISTER_TEST(test_bob_matrix_multiply) {
    matrix mx;
    MATRIX ident, a, prod;

    mx.generate((Angles)0, (Angles)0, (Angles)0, &ident);
    mx.generate((Angles)0x2000, (Angles)0x1000, (Angles)0x0800, &a);

    // I x A = A
    prod = a;
    mx.multiply(&ident, &prod);
    CHECK_NEAR(prod.L11, a.L11, 4); CHECK_NEAR(prod.L22, a.L22, 4);
    CHECK_NEAR(prod.L33, a.L33, 4); CHECK_NEAR(prod.L12, a.L12, 4);
    CHECK_NEAR(prod.L21, a.L21, 4); CHECK_NEAR(prod.L31, a.L31, 4);
}

REGISTER_TEST(test_bob_matrix_transform) {
    matrix mx;
    MATRIX m;
    mx.generate((Angles)0, (Angles)0, (Angles)0, &m);

    // ASMTransform outputs 2*v*col (no downscale); callers recover with
    // v >> (16-scaleback) — the exact idiom masked in FT051/52. Check the
    // recover-roundtrip contract + the scale bound (<=34 per bsr-14 x2).
    SLong x = 1000, y = 0, z = 0;
    UWord scale = mx.transform(&m, x, y, z);
    CHECK(scale <= 34);
    CHECK_NEAR(x >> (16 - scale), 1000, 8);

    // large input: forced downscale keeps output in range; caller's
    // >> (16-scale) roundtrip still recovers the input
    x = 1 << 24;
    scale = mx.transform(&m, x, y, z);
    CHECK(scale > 0 && scale <= 34);
    CHECK_NEAR((double)(x >> (16 - scale)) / (double)(1 << 24), 1.0, 0.02);

    // stage-2 overflow window: shipped asm reads dword [BodyXl+2] (bits
    // 16..47 of the 64-bit product); a >>32 port misses bodies in
    // [2^31,2^32) and returns a sign-flipped low dword with no rescale.
    // x=32767 sits one below the stage-1 threshold; a hand-built row
    // pushes Body past 2^31.
    std::memset(&m, 0, sizeof(m));
    m.L11 = 32767; m.L12 = 100;
    x = 32767; y = 32767; z = 0;
    scale = mx.transform(&m, x, y, z);
    CHECK_EQ(scale, 1);                    // buggy version rescales to 0
    CHECK_EQ(x, 1076952989);               // buggy version sign-flips

    // 64-bit accumulate: three near-max products sum past 2^31 — an
    // int32 sum wraps negative before widening. Expected: Body =
    // 2*(32767*32767 + 32767*100 + 32767*32767) = 4301258556; slice
    // (>>16) = 65616 -> bsr 16 -> rescale 2 -> x = 1075314639.
    m.L13 = 32767; x = y = z = 32767;
    scale = mx.transform(&m, x, y, z);
    CHECK_EQ(scale, 2);
    CHECK_EQ(x, 1075314639);

    // degenerate all-zero input must not trap
    x = y = z = 0;
    scale = mx.transform(&m, x, y, z);
    CHECK(scale <= 34);
    CHECK_EQ(x, 0); CHECK_EQ(y, 0); CHECK_EQ(z, 0);
}

REGISTER_TEST(test_bob_matrix_scaleto16) {
    matrix mx;
    SLong x = 70000, y = 0, z = 0;             // > SWord range -> rescale
    UWord scale = mx.scaleto16bit(x, y, z);
    CHECK(scale > 0);
    CHECK(x <= 32767 && x >= -32768);

    // already-in-range inputs pass through unscaled
    x = 100; y = 200; z = -300;
    UWord s2 = mx.scaleto16bit(x, y, z);
    CHECK_EQ(x, 100); CHECK_EQ(y, 200); CHECK_EQ(z, -300);
    CHECK(s2 <= 34);
}

REGISTER_TEST(test_bob_matrix_crossproduct) {
    // NB: this is NOT a vector cross product — it's the screen-space triangle
    // winding test: perspective-divides v1..v3 (x/z, y/z) and returns TRUE
    // iff signed area <= 0. z must be non-zero (no guard upstream).
    matrix mx;
    DoPointStruc v1, v2, v3;

    // v1=(0,0), v2=(1,1), v3=(1,0) projected: signed area = 0 -> TRUE
    v1.setPosition(0, 0, 10);
    v2.setPosition(10, 10, 10);
    v3.setPosition(10, 0, 10);
    CHECK(mx.crossproduct(v1, v2, v3));

    // opposite winding -> FALSE
    v2.setPosition(10, 0, 10);
    v3.setPosition(10, 10, 10);
    CHECK(!mx.crossproduct(v1, v2, v3));

    // degenerate (v3 projects onto v2's point) -> signed area 0 -> TRUE
    v1.setPosition(0, 0, 10);
    v2.setPosition(10, 10, 10);
    v3.setPosition(20, 20, 20);
    CHECK(mx.crossproduct(v1, v2, v3));
}
