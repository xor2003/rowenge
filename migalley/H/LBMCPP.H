//------------------------------------------------------------------------------
//Filename       lbmcpp.h
//System
//Author         Jim Taylor
//Date           Mon 7 Apr 1997
//Description    LBM row unpack routine.
//
//				Was a textual-template include (#included inside function bodies
//				with macro-defined inputs TRANSFERVALUESIZE/LOADVALUE/COPYVALUE/
//				SAVEVALUE/DELTAVALUE/LBM_END_PTR, and it closed the caller's
//				brace itself). Every live instantiation used the identical UByte
//				configuration, so it is now a real function. Semantics
//				preserved exactly:
//				  - reads bounded at cend; past-end reads decode as a 1-byte
//				    COPY (b==0) so the state machine converges instead of
//				    running off the end of the fileblock. Pass
//				    (const UByte*)~0 for the historical unbounded mode.
//				  - data-derived stream skips saturate at [cstart,cend] rather
//				    than wrapping the pointer wild.
//				  - NULL stream or NULL destination decodes into a scratch sink
//				    instead of writing through a null base pointer.
//------------------------------------------------------------------------------

#ifndef LBMCPP_H_INCLUDED
#define LBMCPP_H_INCLUDED

#include <stdint.h>

//Inputs		Source data cursor c (moved past the row's bytes)
//				End of the compressed stream buffer
//				Destination image base + bytes per scanline
//				Length of line in data
//				start	x
//				start	y
//				physical min x
//				physical max x
//
//Returns		Data ptr moved
//------------------------------------------------------------------------------

static inline UByte* LBMUnpackNRow(UByte* c, const UByte* cend,
									UByte* logicalscreenptr,
									long BytesPerScanLine,
									SLong width, SLong x, SLong y,
									SLong minx, SLong maxx)
{
	static UByte	lbm_null_sink[65536];
	if (!c)
		c=(UByte*)lbm_null_sink;					//missing stream — decode zeros
	if (!cend)
		cend=(const UByte*)~0UL;					//historical unbounded mode
	const UByte* cstart=c;							//clamp floor for LBMCBACK
	UByte*	scraddr=	logicalscreenptr
						? (UByte*)(((long)BytesPerScanLine * y) + logicalscreenptr)
						: (UByte*)lbm_null_sink;
	UByte		b = 0;
	SLong		Count;
	UByte		TRANSFERVALUE;

	int	residue=0;
	Bool	skipplotting=FALSE;

	#define LBMREADB()	((c < (UByte*)cend) ? *(c++) : (UByte)0)
	#define LBMCFWD(n)	do { uintptr_t _c=(uintptr_t)(c), _e=(uintptr_t)cend; \
							if (_e<=_c || _e-_c < (uintptr_t)(n)) c=(UByte*)cend; else c+=(n); } while(0)
	#define LBMCBACK(n)	do { uintptr_t _c=(uintptr_t)(c), _s=(uintptr_t)cstart; \
							if (_s>=_c || _c-_s < (uintptr_t)(n)) c=(UByte*)cstart; else c-=(n); } while(0)
	#define LOADVALUE(v)	TRANSFERVALUE=(v)
	#define SAVEVALUE(t)	*(t)++=TRANSFERVALUE
	#define COPYVALUE(t,s)	*(t)++=(s)
	#define DELTAVALUE(t,d)	(t)+=(d)

	////// CASE 1: starting off left clip - clip necessary

	if (x<minx && x+width>=minx)
	{
		while (x<minx)
		{
			b=LBMREADB();
			if (b<0x80)
			{	//COPY
				Count = 1+(SByte) b;
				x+=Count;
				width-=Count;
					LBMCFWD(Count);
			}
			elseif (b==0x80)
			{	//SKIP
				Count= 1 + LBMREADB();
				x+=Count;
				width-=Count;
			}
			else
			{	//RUN
				Count = 1-(SByte)b;
				LBMCFWD(1);
				width-=Count;
				x+=Count;
			}
		}

	//////1.2 Handle overflow of segment overlapping left clip
		if (b<0x80)
		{
			residue=x-minx;
			LBMCBACK(residue);
			x=minx;
			DELTAVALUE(scraddr,x);			//set to starting pixel...
			x+=Count=residue;
			residue=0;
			if (x>maxx)
			{
				residue=x-maxx-1;
				Count=Count-residue;
				skipplotting=TRUE;
			}
			if (Count)
			do
			{
				COPYVALUE(scraddr,LBMREADB());
			}
			while (--Count!=0);
			LBMCFWD(residue);
		}
		else
		if (b!=0x80)
		{
			LOADVALUE(*(c-1));
			residue=x-minx;
			x=minx;
			DELTAVALUE(scraddr,x);			//set to starting pixel...
			x+=Count=residue;
			residue=0;
			if (x>maxx)
			{
				skipplotting=TRUE;
				residue=x-maxx-1;
				Count=Count-residue;
			}
			if (Count)
			do
			{
				SAVEVALUE(scraddr);
			}
			while (--Count!=0);
		}
		else
			{
				DELTAVALUE(scraddr,x);			//set to starting pixel...
				if (x>maxx)
					skipplotting=TRUE;
			}
	}
	else
		if (x>=minx && x<=maxx)
		{
			DELTAVALUE(scraddr,x);			//set to starting pixel...
		}
		else
			skipplotting=TRUE;

	residue=0;
	if (!skipplotting)
	///// CASE 3: MAIN ARTWORK UNPACK
		if (x+width<=maxx)
	/////CASE 3A: FAST UNPACK - CLIP TO WIDTH
			while (width)
			{
				b=LBMREADB();
				if (b<0x80)
				{	//COPY
					Count = 1+(SByte) b;
					width-=Count;
					do
					{
						COPYVALUE(scraddr,LBMREADB());
					}
					while (--Count!=0);
				}
				elseif (b==0x80)
				{	//SKIP
					Count= 1 + LBMREADB();
					width-=Count;
					DELTAVALUE(scraddr,Count);
				}
				else
				{	//RUN
					Count = 1-(SByte) b;
					width-=Count;
					LOADVALUE(LBMREADB());
					do
					{
						SAVEVALUE(scraddr);
					}
					while (--Count!=0);
				}
			}
		else
	/////CASE 3B: SLOW UNPACK - CLIP TO MAXX
		{
			while (x<=maxx)
			{
				b=LBMREADB();
				if (b<0x80)
				{	//COPY
					Count = 1+(SByte) b;
					x+=Count;
					width-=Count;
					if (x<=maxx)
						do
						{
							COPYVALUE(scraddr,LBMREADB());
						}
						while (--Count!=0);
				}
				elseif (b==0x80)
				{	//SKIP
					Count= 1 + LBMREADB();
					width-=Count;
					x+=Count;
					DELTAVALUE(scraddr,Count);
				}
				else
				{	//RUN
					Count = 1-(SByte) b;
					x+=Count;
					width-=Count;
					LOADVALUE(LBMREADB());
					if (x<=maxx)
						do
						{
							SAVEVALUE(scraddr);
						}
						while (--Count!=0);

				}
			}
			////// 3B2: Overflow of last unpack
			if (x>maxx)
			{
				if (b<0x80)
				{
					residue=x-maxx-1;
					Count-=residue;
					do
					{
						COPYVALUE(scraddr,LBMREADB());
					}
					while (--Count!=0);
					LBMCFWD(residue);
				}
				elseif (b!=0x80)
				{
					residue=x-maxx-1;
					Count-=residue;
					do
					{
						SAVEVALUE(scraddr);
					}
					while (--Count!=0);
				}
			}
		}

	///// CASE 4: RHS CLIPPED DATA - UP TO WIDTH
	if (width)
		while(width>0)
		{
			b=LBMREADB();
			if (b<0x80)
			{	//COPY
				Count = 1+(SByte) b;
				width-=Count;
				LBMCFWD(Count);
			}
			elseif (b==0x80)
			{	//SKIP
				Count= 1 + LBMREADB();
				width-=Count;
			}
			else
			{	//RUN
				Count = 1-(SByte)b;
				LBMCFWD(1);
				width-=Count;
			}
		}

	#undef LBMREADB
	#undef LBMCFWD
	#undef LBMCBACK
	#undef LOADVALUE
	#undef SAVEVALUE
	#undef COPYVALUE
	#undef DELTAVALUE

	return(c);
	//THAT'S YER LOT
}

#endif	//LBMCPP_H_INCLUDED
