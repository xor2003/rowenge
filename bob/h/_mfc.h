#ifndef __MFC_INCLUDED
#define	__MFC_INCLUDED
#pragma optimize( "g", off )
#define	THIS_FILE_DEFINED
#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
#define THIS_FILE __FILE__
#endif

//x0r !!! MATHABLE(SquadSelect)
//#ifndef	THIS_FILE_DEFINED
//#endif
#ifndef _DEBUG
#pragma message(__HERE__ "Non-debug version: global optimisation enabled when safe!")
#pragma optimize( "g", on )
#endif
#define	REALISTIC_SPOTTING_COUNTS
#include "ranges.h"
#include "package.h"
#include "missman2.h"
#include "nodebob.h"
#endif



