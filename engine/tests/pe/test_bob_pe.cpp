// T105: BoB reference tests compiled for Win32-PE and run under Wine.
// Resurrected from the dead /* */ blocks in mfc/mig.cpp (#ifdef TESTS).
// Upstream compared asm-derived "old*" impls against ported C++; every
// old* body is itself dead upstream (MSVC _asm inside /* */), so the
// comparisons below check the live impls against independent references:
// x87 fsincos, libm, and C re-implementations of the documented semantics.
#include "dosdefs.h"
#include "mathasm.h"
#include "matrix.h"
#include "myangles.h"
#include "fastmath.h"
// RERUN: the real global (3dcom.cpp:170) — ctor runs BuildSqrtTable().
// GroundHeight/ArcCos dropped: collided.cpp pulls a 169-symbol game
// surface (Collide _Collide global), and both old* impls are dead upstream —
// the equivalence comparison upstream intended is impossible to run.
FastMath fastMath;
ULong FastMath::fastSqrtTable[0x10000];
SWord FastMath::tableCode1[65536];
SWord FastMath::tableCode2[257];
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <UnitTest++.h>

// ---------------------------------------------------------------------------
// Independent references (written from the semantics, not copied from impl)
// ---------------------------------------------------------------------------
static unsigned long abs32(SLong v) { return v < 0 ? -(unsigned long)v : (unsigned long)v; }

// bit position of highest set bit, 0 if none — independent of _BitScanReverse
static int ref_bsr(unsigned long v)
{
    int pos = 0;
    while (v >>= 1) ++pos;
    return v ? pos : pos; // pos already counts; for v==0 returns 0
}

// reference for ASMTransform: fixed-point matrix*vector with dynamic scaling
static SLong ref_asmtransform(MATRIX_PTR m, SLong& x, SLong& y, SLong& z)
{
    long long lx = x, ly = y, lz = z;
    unsigned long mag = abs32(x) | abs32(y) | abs32(z);
    int scale = 0;
    if ((long)mag >= 32767)
    {
        int sh = ref_bsr(mag) - 14;
        lx >>= sh; ly >>= sh; lz >>= sh;
        scale = sh;
    }
    long long bx = 2 * (lx * m->L11 + ly * m->L12 + lz * m->L13);
    long long by = 2 * (lx * m->L21 + ly * m->L22 + lz * m->L23);
    long long bz = 2 * (lx * m->L31 + ly * m->L32 + lz * m->L33);
    unsigned long hmag = abs32((SLong)(bx >> 32)) | abs32((SLong)(by >> 32)) | abs32((SLong)(bz >> 32));
    if (hmag > 0x7FFF)
    {
        int sh = ref_bsr(hmag) - 14;
        scale += sh;
        bx >>= (sh & 0x1F); by >>= (sh & 0x1F); bz >>= (sh & 0x1F);
    }
    x = (SLong)(bx & 0xffffffff);
    y = (SLong)(by & 0xffffffff);
    z = (SLong)(bz & 0xffffffff);
    return scale;
}

static SWord ref_getscale(SLong x, SLong y, SLong z)
{
    unsigned long mag = abs32(x) | abs32(y) | abs32(z);
    int r = ref_bsr(mag) - 14;
    return (SWord)(r < 0 ? 0 : r);
}

// ---------------------------------------------------------------------------

TEST(TestASMTransform)
{
	struct	matrix_ m = { -3333, 4444, -5555,
		11111, -2222, 3333
		- 6666, 7777, -8888 };
	SLong a = -2147373648, b = 2147373648, c = -2147373648;
	SLong aa = a, bb = b, cc = c;
	SLong s = ASMTransform(&m, aa, bb, cc);
	SLong rx = a, ry = b, rz = c;
	SLong rs = ref_asmtransform(&m, rx, ry, rz);
	CHECK(s == rs && aa == rx && bb == ry && cc == rz);
}

TEST(TestGetScale)
{
	SLong cases[][3] = {
		{ -3333333, 4444444, -5555555 },
		{ 0x10000, 0, 0 },
		{ 0x7FFF, 0x10, -0x20 },
		{ -0x7FFFFFFF, 1, 1 },
		{ 1, 2, 3 },
		{ 0x4000, -0x4000, 0x4000 },
	};
	for (auto& v : cases)
		CHECK(GetScale(v[0], v[1], v[2]) == ref_getscale(v[0], v[1], v[2]));
}

TEST(sincos)
{
	double 	ave_heading=1.5, ave_pitch=1.5;
	double 	sin_pitch, cos_pitch,
		sin_heading, cos_heading;

	// RERUN: MSVC _asm block -> GCC x87 constraints (same fsincos op)
	__asm__ volatile ("fsincos"
		: "=t" (cos_pitch), "=u" (sin_pitch)
		: "0" (ave_pitch));

	sin_heading = sin(ave_heading);
	cos_heading = cos(ave_heading);
	CHECK(sin_heading == sin_pitch && cos_pitch == cos_heading);
}


TEST(Round)
{
	SLong a,b,c,d,e,f;
	SLong aa, bb, cc, dd,ee,ff;
	R3DVALUE f1 = 2.3;
	R3DVALUE f2 = 3.8;
	R3DVALUE f3 = 5.5;
	R3DVALUE f4 = -2.3;
	R3DVALUE f5 = -4.7;
	R3DVALUE f6 = -5.5;

	fastMath.FloatToInt(&a, f1);
	fastMath.FloatToInt(&b, f2);
	fastMath.FloatToInt(&c, f3);
	fastMath.FloatToInt(&d, f4);
	fastMath.FloatToInt(&e, f5);
	fastMath.FloatToInt(&f, f6);
	aa = (SLong)round(f1);
	bb = (SLong)round(f2);
	cc = (SLong)round(f3);
	dd = (SLong)round(f4);
	ee = (SLong)round(f5);
	ff = (SLong)round(f6);

	CHECK(a==aa);
	CHECK(b == bb);
	CHECK(c == cc);
	CHECK(d == dd);
	CHECK(e == ee);
	CHECK(f== ff);
}

TEST(sincos2)
{
	// fpSin_Cos is live; oldfpSin_Cos is dead upstream. Check the ANGLES
	// encoding itself: angle * PI / 32768 — ANGLES_186Deg must yield
	// sin≈-0.1045, cos≈-0.9945.
	Float msin, mcos;
	fpSin_Cos(ANGLES(ANGLES_186Deg), msin, mcos);
	CHECK_CLOSE(msin, -0.104528, 1e-3);
	CHECK_CLOSE(mcos, -0.994522, 1e-3);
	// cardinal encodings: 0x4000 = 90deg, 0x8000 = 180deg
	fpSin_Cos(ANGLES((Angles)0x4000), msin, mcos);
	CHECK_CLOSE(msin, 1.0, 1e-9);
	CHECK_CLOSE(mcos, 0.0, 1e-9);
	fpSin_Cos(ANGLES((Angles)0x8000), msin, mcos);
	CHECK_CLOSE(msin, 0.0, 1e-9);
	CHECK_CLOSE(mcos, -1.0, 1e-9);
}

TEST(BitTest)
{
	char s[] = "               ";
	BITRESET(s, 74);
	CHECK(!BITTEST(s, 74));
	BITSET(s, 74);
	CHECK(BITTEST(s, 74));

	// RERUN: oldBITSET/oldBITRESET are dead /* */ blocks in mathasm.h —
	// reference impl is plain C bit ops (same semantics as the dead asm).
	((unsigned char*)s)[74 >> 3] &= ~(1 << (74 & 7));
	CHECK(!BITTEST(s, 74));
	((unsigned char*)s)[74 >> 3] |= 1 << (74 & 7);
	CHECK(BITTEST(s, 74));

	long i=0;
	CHECK(!BITTEST(&i, 18));
	BITSET(&i, 18);
	CHECK(BITTEST(&i, 18));

	i = 0;
	i |= 1 << 20;
	CHECK(BITTEST(&i, 20));
}
