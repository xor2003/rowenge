//------------------------------------------------------------------------------
//Filename       modvec.h
//System
//Author         Andrew McRae
//Date           Mon 6 Jan 1997
//Description
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	MODVEC_Included
#define	MODVEC_Included

#include <math.h>
#define	DEFAULT_MODVEC 0

const double F1PIE2	=1.570796326795;
//const double F1PIE2	=M_PI_2;
const double FPIE	= M_PI;
const double F3PIE2=	4.712388980385;
const double F2PIE=	6.28318530718;

const double FANGLE_330	= 5.759586532;

//enum {MIN_TEMP = 255, MAX_TEMP = 310};
//typedef double FP, *PFP;
//class	FP
//{
//	Float	val;
//public:
//	FP(Float v=0.0)	{val=v;}
//	FP(Angles a)		{val=a;}
//	FP(ANGLES a)		{val=a;}
//	int operator=(int i)	{val=i;return i;}
//	Float operator=(FP f)	{val=f.val;return val;}
//	Float operator=(const Float f) {return val=f;}
//	Angles	operator=(Angles a) {val=(int)a;return a;}
//	ANGLES	operator=(ANGLES a) {val=(int)a;return a;}
//	operator Float& ()		{return val;}
//};

typedef float FP, *PFP;


typedef struct _fcrd {
			FP	x,y,z;
} FCRD, *PFCRD;

typedef struct _fcrdlong {
			Float x,y,z;
} FCRDlong, *PFCRDlong;

typedef struct _fori {
	_fcrd x,y,z;
} FORI, *PFORI;






// General conversion macros
#define RPM2RADSPERCSEC(spd) spd * 0.001047197
#define DEGS2ROWAN(ang) (SWord)((SLong)(ang * 182.04444444) & 0xffff)
#define DEGS2RADS(ang) ang / 57.29577951
#define RATEDTORQUE(p,s) ((p*1359.62*6000)/(s*F2PIE))
#define mBARS2PRESSURE(p) (p*0.01)
#define ROWAN2RADS(ang) ((FP)ang /  10430.37835047);
#define FEETPERMIN2MPS(val) (val * 0.00508)
#define FEET2CM(val) (val * 30.48)

void NullVec (FCRD&);
void NullVec (FCRDlong&);
void NullOri (FORI&);
void CopyVec (FCRD&, FCRD&);
void CopyOri (FORI&, FORI&);

#ifdef	__WATCOMC__

// Square Root Function
extern FP FSqrt(FP);
#pragma aux FSqrt parm [8087] = \
"fsqrt";

// Sine Function
extern FP FSin(FP);
#pragma aux FSin parm [8087] = \
"fsin";

// Cosine Function
extern FP FCos(FP);
#pragma aux FCos parm [8087] = \
"fcos";

// Partial Inverse Tangent Function
extern void FPATan(FP& opp, FP& adj, FP& ans);
#pragma aux	FPATan = 				\
"	fld		qword ptr ds:[eax]	"	\
"	fld		qword ptr ds:[ebx]	"	\
"	fpatan						"	\
"	fstp	qword ptr ds:[ecx]	"	\
parm	[eax] [ebx]	[ecx]			\

// X^Y Function
// only works when -1 < y.log2x < 1
extern void FXPowerY (FP& x, FP& y, FP& ans);
#pragma aux FXPowerY = 				\
"	fld		qword ptr ds:[ebx]	"	\
"	fld		qword ptr ds:[eax]	"	\
"	fyl2x						"	\
"	f2xm1						"	\
"	fld1						"	\
"	fadd						"	\
"	fstp	qword ptr ds:[ecx]	"	\
parm	[eax] [ebx] [ecx]			\

#else
#if defined(__MSVC__) || defined(__GNUC__) //RERUN: plain-C bodies work under GCC too

inline FP FSqrt(FP n)
{
	return sqrt(n);
/*	_asm
	{
		fld		n
		fsqrt
		fstp	n
	}
	return n;*/
}

inline FP FSin(FP n)
{
	return sin(n);
/*	_asm
	{
		fld		n
		fsin
		fstp	n
	}
	return n;*/
}
inline FP FCos(FP n)
{
	return cos(n);
/*	_asm
	{
		fld		n
		fcos
		fstp	n
	}
	return n;*/
}
/*
inline FP FPATan(FP opp,FP adj)
{
	Float	ans;
_asm
{
fld		opp
fld		adj
fpatan
fstp	ans
}
return ans;
return atan2(opp, adj);
}
*/
/*
inline FP FXPowerY(FP x, FP y)
{
	FP	ans;
	_asm
	{
		fld		x
		fld		y
		fyl2x
		f2xm1
		fld1
		fadd
		fstp	ans
	}
	return ans;
}
*/
#endif
#endif

SWord Rads2Rowan (FP);
FP Rads2Degs (FP);
SWord Degs2Rowan (FP);
FP Degs2Rads (FP);
FP Rowan2Rads (UWord);
FP Rowan2Degs (UWord);

FP Rpm2RadsPerCSec (FP);
SWord	RadPerCSec2RowanPerMin(FP);
SWord	RadPerCSec2RowanPerMin(FP	angle);
SWord	RadPerCSec2DegsPerMin(FP	angle);


FP CalcAngle (FP, FP);

FP VecLen2D (FP, FP);
FP VecLen (FCRD&);

Bool NrmVec2D (FP&, FP&);
Bool NrmVec (FCRD&, FCRD&);

void RotateVec2D (FP&, FP&, FP);

void RotVecXSC (FCRD&, FCRD&, FP, FP);
void RotVecYSC (FCRD&, FCRD&, FP, FP);
void RotVecZSC (FCRD&, FCRD&, FP, FP);

void TnsAxs (FCRD&, FCRD&, FORI&);
void TnsPnt (FCRD&, FCRD&, FORI&);

void AddVec (FCRD&, FCRD&, FCRD&);
void SubVec (FCRD&, FCRD&, FCRD&);

void AddVec (FCRDlong&, FCRDlong&, FCRDlong&);
void SubVec (FCRDlong&, FCRDlong&, FCRDlong&);

void CPrdX(FORI&);
void CPrdY(FORI&);
void CPrdZ(FORI&);
void CPrd (FCRD&, FCRD&, FCRD&);

FP DotPrd (FCRD&, FCRD&);

void RotOriXVec (FORI&, FP);
void RotOriYVec (FORI&, FP);
void RotOriZVec (FORI&, FP);

void SetOri (FORI&, FP, FP, FP);


#pragma pack(pop)
#endif
