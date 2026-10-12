// T107 family 5: terrain/collision — GroundHeight (terrain plane equation,
// shipped asm used x87 FISTP = round-to-nearest) and the ray/primitive
// predicates Collide::RayHitTriangle / RayHitParallelogram.
// Vector magnitude domain: normalise() squares components (SLong), so test
// geometry stays at ~10^2 units — |A x B|^2 must not overflow SLong.
#include "../harness.h"
#include "DOSDefs.h"
#include "myangles.h"
#include "mymath.h"
#include "myvector.h"
#include "collided.h"
#include <cstring>

// GroundHeight: h = -(a*x + c*z)/b — the C port truncates toward zero; the
// shipped x87 FISTP rounded to nearest. These tests pin rounding semantics.
REGISTER_TEST(test_bob_ground_height) {
    Coeffs pl;
    memset(&pl, 0, sizeof pl);

    // flat ground: b = -1 -> h = 0 everywhere
    pl.b = -1.0;
    SLong x = 5000, z = -3000;
    CHECK_EQ(GroundHeight(x, z, pl), 0);

    // tilted plane: a=1, b=-1 -> h = x  (height rises with x)
    pl.a = 1.0; pl.b = -1.0; pl.c = 0.0;
    CHECK_EQ(GroundHeight(x, z, pl), 5000);
    SLong x2 = -7777;
    CHECK_EQ(GroundHeight(x2, z, pl), -7777);

    // slope in z: a=0, b=-2, c=4 -> h = 4*z/2 = 2*z
    pl.a = 0.0; pl.b = -2.0; pl.c = 4.0;
    SLong z2 = 1234;
    CHECK_EQ(GroundHeight(x, z2, pl), 2468);

    // ROUNDING: a*x+c*z over b yields a .5 fraction.
    // a=1,b=-4: h = x/4. x = 6 -> 1.5 -> FISTP rounds to 2 (trunc gives 1).
    pl.a = 1.0; pl.b = -4.0; pl.c = 0.0;
    SLong xr = 6;
    CHECK_EQ(GroundHeight(xr, z, pl), 2);
    // negative half: x = -6 -> h = -1.5 -> FISTP rounds to -2 (trunc gives -1)
    SLong xn = -6;
    CHECK_EQ(GroundHeight(xn, z, pl), -2);
}

// RayHitTriangle(S, D, TO, TA, TB, P): ray S+t*D vs triangle with corner TO
// and edges TA-TO, TB-TO. Barycentric u,v in 0..0x100; inside iff u>=0, v>=0,
// u+v<=0x100. Note shipped semantic: t sign is NOT tested (line, not ray).
REGISTER_TEST(test_bob_ray_hit_triangle) {
    Collide col;
    vector P;

    // triangle in the y=0 plane: TO=origin, edges +x and +z (160 units —
    // large enough for the >>12 dot shifts to keep precision, small enough
    // that normalise's squared-magnitude stays inside SLong)
    vector TO(0, 0, 0), TA(160, 0, 0), TB(0, 0, 160);
    vector D(0, -256, 0);

    // straight-down ray through the triangle interior
    vector S(32, 1024, 32);
    CHECK(col.RayHitTriangle(S, D, TO, TA, TB, P) == TRUE);
    CHECK_EQ(P.a, 32);
    CHECK_EQ(P.b, 0);
    CHECK_EQ(P.c, 32);

    // miss: ray lands at x<0 (u < 0)
    vector Smiss(-16, 1024, 32);
    CHECK(col.RayHitTriangle(Smiss, D, TO, TA, TB, P) == FALSE);

    // miss: past the hypotenuse (u+v > 0x100)
    vector Shyp(150, 1024, 150);          // 150+150 = 300 > 160 edge
    CHECK(col.RayHitTriangle(Shyp, D, TO, TA, TB, P) == FALSE);

    // boundary: exactly on an edge (u == 0x100, v == 0) is INSIDE
    vector Sedge(160, 1024, 0);
    CHECK(col.RayHitTriangle(Sedge, D, TO, TA, TB, P) == TRUE);

    // ray pointing away from the plane still reports a hit: alpha is
    // unconstrained (shipped code tests the line, not the ray direction)
    vector Sup(32, -1024, 32);
    CHECK(col.RayHitTriangle(Sup, D, TO, TA, TB, P) == TRUE);
}

// RayHitParallelogram(S, D, C, A, B, P): edges A,B are VECTORS here (unlike
// the triangle variant's points). u,v independently in 0..0x100, and an
// explicit DdotN==0 (parallel ray) bail-out.
REGISTER_TEST(test_bob_ray_hit_parallelogram) {
    Collide col;
    vector P;
    vector C(0, 0, 0), A(64, 0, 0), B(0, 0, 64);
    vector D(0, -256, 0);

    // centre hit
    vector S(32, 1024, 32);
    CHECK(col.RayHitParallelogram(S, D, C, A, B, P) == TRUE);
    CHECK_EQ(P.b, 0);

    // corner boundary: u==v==0x100 exactly is INSIDE
    vector Scorner(64, 1024, 64);
    CHECK(col.RayHitParallelogram(Scorner, D, C, A, B, P) == TRUE);

    // inside the square but OUTSIDE the equivalent triangle — the square's
    // far half is in bounds (u+v may exceed 0x100)
    vector Shalf(60, 1024, 60);
    CHECK(col.RayHitParallelogram(Shalf, D, C, A, B, P) == TRUE);

    // just outside corner -> miss
    vector Sout(65, 1024, 64);
    CHECK(col.RayHitParallelogram(Sout, D, C, A, B, P) == FALSE);

    // degenerate: ray parallel to the plane -> FALSE (guarded)
    vector Spara(32, 1024, 32), Dpara(64, 0, 0);
    CHECK(col.RayHitParallelogram(Spara, Dpara, C, A, B, P) == FALSE);
}
