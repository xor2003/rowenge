/*
	 Battle of Britain
	 Copyright (C) 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at bob@rowansoftware.com a
better place to ask is http://www.simhq.com/ or even :-
http://www.simhq.com/cgi-bin/boards/cgi-bin/forumdisplay.cgi?action=topics&forum=Battle+of+Britain&number=40&DaysPrune=20&LastLogin=
*/

//------------------------------------------------------------------------------
//Filename       tilemake.cpp
//System         
//Author         Paul.   
//Date           Wed 15 Jul 1998
//Description    
//------------------------------------------------------------------------------

//#define _REDO_ME3_

#define F_GRAFIX
#define F_SOUNDS
#define F_BATTLE

#include "dosdefs.h"
#include "fileman.h"
#include "files.g"
#include "ImageMap.h"
#include "ImageMap.g"
#include "display.h"
#include "worldinc.h"
#include "MigLand.h"

#include "3dcode.h"
#include "mymath.h"
#include "TileMake.h"
#include "gameset.h"
#include "world.h"
#include "shapes.h"
#include "shpinstr.h"
#include "savegame.h"
#include "areatype.h"
#include "overlay.h"
#include "shapenum.g"
#include "fastmath.h"
#include "../lib3d/lib3d.h"

const	SWord	SHIFT_StartIndex		=	4;
const	SWord	SHIFT_MultIndex			=	8;

const R3DVALUE DIV_SCALE = D3DVALUE(1.f/256.f);

#define MAKE_SHIFT(p1) SWord(SHIFT_StartIndex+((p1)*(SHIFT_MultIndex+SHIFT_StartIndex)))

const	SWord	SHIFT_MajorRiver		=	MAKE_SHIFT ( 1 );
const	SWord	SHIFT_MinorRiver		=	MAKE_SHIFT ( 1 );
const	SWord	SHIFT_Tributary			=	MAKE_SHIFT ( 1 );
const	SWord	SHIFT_MinorRoad			=	MAKE_SHIFT ( 2 );
const	SWord	SHIFT_FootPath			=	MAKE_SHIFT ( 2 );
const	SWord	SHIFT_MajorRail			=	MAKE_SHIFT ( 3 );
const	SWord	SHIFT_MinorRail			=	MAKE_SHIFT ( 3 );
const	SWord	SHIFT_ARoad				=	MAKE_SHIFT ( 4 );
const	SWord	SHIFT_BRoad				=	MAKE_SHIFT ( 4 );
const	SWord	SHIFT_AirFieldRT		=	MAKE_SHIFT ( 9 );
const	SWord	SHIFT_BlendGrassOther	=	MAKE_SHIFT ( 5 );
const	SWord	SHIFT_BlendSeaLand		=	MAKE_SHIFT ( 6 );
const	SWord	SHIFT_BlendSeaBeach		=	MAKE_SHIFT ( 7 );
const	SWord	SHIFT_CliffTopBaseBlend	=	MAKE_SHIFT ( 8 );

TMake* tMake=NULL;													  //JIM 29/03/99


namespace TileMake
{


const SLong _wholeAreaSizeMAX=33;
const SLong _wholeAreaSizeMIN=17;
const SLong _cornerLengthMAX=_wholeAreaSizeMAX+1;
const SLong _cornerLengthMIN=_wholeAreaSizeMIN+1;
const SLong _areaBufferSizeMAX=_wholeAreaSizeMAX*_wholeAreaSizeMAX;
const SLong _areaBufferSizeMIN=_wholeAreaSizeMIN*_wholeAreaSizeMIN;
const SLong _cornerBufferSizeMAX=_cornerLengthMAX*_cornerLengthMAX;
const SLong _cornerBufferSizeMIN=_cornerLengthMIN*_cornerLengthMIN;

const SLong Grid_Width=131072;
const ULong Grid_Mask=~ULong(Grid_Width-1);

const SLong DataGridSizeMAX=33;
const SLong DataGridSizeMIN=17;

const SLong Max_TextureLevelsMAX=6;
const SLong Max_TextureLevelsMIN=5;

const SLong Max_Edges=256;
const SLong Indexable_Edges=Max_Edges;
const SLong Real_Max_Edges=Max_Edges+2048;	//1024;

const SLong Max_Textures=32;
const SLong Max_Scales=4;

const FileNum BaseTexture=FIL_IMG0000;
//DEAD const FileNum LoRezBase=FIL_IMG20000;

const SLong TextureWidth=256;
const SWord SpanMin=0;
const SWord SSpanMax=SWord(TextureWidth-1);
const SWord ESpanMax=SWord(TextureWidth);

const UByte cf_null=0;
const UByte cf_top=1;
const UByte cf_bot=2;
const UByte cf_lft=4;
const UByte cf_rgt=8;
const UByte cf_all=cf_top+cf_bot+cf_lft+cf_rgt;

const UWord _offleft=0x0001;
const UWord _offright=0x0002;
const UWord _offtop=0x0004;
const UWord _offbottom=0x0008;

static ShapeNum noShadowShapes[]=
{
TREE1,
TREE2,
SHCKWV,
EMPTY,
EMPTY2,
CTRPTC,
CRKPTC
};

static ShapeNum noShadowGroups[]=
{
// Stuff tall&thin groups here...
R_CL_1,	// radar sites
R_CL_2,
R_CL_3,
R_CHL_1,
R_CHL_2,
R_CHL_3,
EMPTY
};

const SLong noShadowShapeEntries=sizeof(noShadowShapes)/sizeof(ShapeNum);
const SLong noShadowGroupEntries=sizeof(noShadowGroups)/sizeof(ShapeNum);

inline bool groupHasShadow(ShapeNum grp)
{
 	for (int i=noShadowGroupEntries-1;i>=0;i--)
		if (noShadowGroups[i]==grp)
			return false;
	return true;
}

inline bool shapeHasShadow(ShapeNum shp)
{
 	for (int i=noShadowShapeEntries-1;i>=0;i--)
		if (noShadowShapes[i]==shp)
			return false;
	return true;
}

template<class t> inline t Min(const t a,const t b) 
							{return (a<b)?a:b;}
template<class t> inline t Max(const t a,const t b) 
							{return (a>b)?a:b;}
template<class t> inline t Abs(const t a) 
							{return (a>t(0))?a:-a;}
template<class t> inline t Clamp(const t a,const t min,const t max)
							{return (a<min)?min:(a>max)?max:a;}
template<class t> inline t Absolute(const t i,const t j,t& k)
							{return ((i-j)*(k=((i-j)<0?-1:1)));}

template<class t>
inline
void ClipResult(const t x,const t z,const Rect& r,UWord& flags,UWord& edges)
{
	if (x<r.l) 		flags=_offleft;
	else if (x>r.r) flags=_offright;
	else 			flags=0;

	if (z<r.t) 		flags+=_offtop;
	else if (z>r.b) flags+=_offbottom;

	if (x==r.l) 		edges=_offleft;
	else if (x==r.r)	edges=_offright;
	else				edges=0;
		
	if (z==r.t)			edges+=_offtop;
	else if (z==r.b)	edges+=_offbottom;
}

typedef struct EdgeTag
{
	SLong 	ymin,
			ymax;
	SLong 	xi,
			dxi;
}
Edge,*EdgePtr;

typedef struct EdgeTag2
{
	Edge	tri;
	Edge	texture;
}
Edge2,Edge2Ptr;
	
typedef struct VDataTag
{
	SWord v;
	SWord x;
	SWord y;
}
VData,*VDataPtr;

typedef struct VTriTag
{
	VData v1,v2,v3;
}
VTri,*VTriPtr;



typedef struct XDataTag : VData
{
	UByte cf;
}
XData,*XDataPtr;

typedef struct BoxTag
{
	SLong left,top,right,bottom;
}
Box,*BoxPtr;

//DEAD fileblock*	fileData[Max_Textures];	//*Max_Scales];
//DEAD fileblock*	fileData2[Max_Textures];	//*Max_Scales];
UByte* 		imageData[Max_Textures]; //*Max_Scales];
ImageMapDesc* imagePtrs[Max_Textures];
//DEAD UByte* 		imageData2[Max_Textures]; //*Max_Scales];
PointDef* 	pointdef;
//SWord		byteMask;



SLong gridRecordBase_x;
SLong gridRecordBase_z;


DataRecord*	gridRecords;



struct vector2d
{
	float x;
	float z;
};

WorldStuff* worldPtr;
static SLong shadowDX,shadowDZ;
static float shadowDXf,shadowDZf;

static bool 	loadedTextures=false;
static CLib3D*	g_lpLib3d=NULL;


const	float	SHAD_CUT_OFF = 18. / float(SUNELEVDIST);

const	float	shadNearMin = 110;
const	float	shadNearMax = 128;
const	ULong	shadColour = 0x101010;

static	ULong	shadNearAlpha = shadNearMin;
static	ULong	shadFarAlpha = shadNearMin;
static	ULong	shadColourNear = shadColour;
static	ULong	shadColourFar = shadColour;












//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DoFreeTextures
//Author		Paul.   
//Date			Sat 17 Apr 1999
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DoAllocateTexture
//Author		Paul.   
//Date			Sat 17 Apr 1999
//------------------------------------------------------------------------------

inline void ClearTexturePointers();
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetTexturePointers
//Author		Paul.   
//Date			Wed 15 Jul 1998
//------------------------------------------------------------------------------
void SetTexturePointers()
{
	SWord fileNumOffset;
	SWord i,j;
	FileNum fn;
	fileblockptr fbp;

	if (loadedTextures) ClearTexturePointers();		

	loadedTextures=true;

	fileNumOffset=SWord(FIL_IMG0000H-FIL_IMG0000);


	for (j=0;j<Max_Textures;j++)
	{
		fn=FileNum(j+BaseTexture+fileNumOffset);
		fileblock fb(fn);
		imagePtrs[j]=new ImageMapDesc;
		imagePtrs[j]->FixLbmImageMap(&fb);
		imagePtrs[j]->hTextureMap=0;
#pragma warnmsg ( "rob - why do I need this???" )
		imagePtrs[j]->isMasked=0; // dunno why this should be needed...
		imageData[j]=imagePtrs[j]->body;

	}
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetImageFromArea
//Author		Paul.   
//Date			Fri 28 Jan 2000
//
//------------------------------------------------------------------------------
ImageMapDescPtr GetImageFromArea(UByte at)
{
 	ImageMapDescPtr imgp;
	imgp=imagePtrs[(at&0x1F)];
	return imgp;
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ClearTexturePointers
//Author		Paul.   
//Date			Thu 16 Jul 1998
//------------------------------------------------------------------------------
inline void ClearTexturePointers()
{
	SWord i;

	if (loadedTextures)
	{
		for (i=0;i<Max_Textures;i++) delete imagePtrs[i],imagePtrs[i]=NULL;
		loadedTextures=false;
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ClearRecords
//Author		Paul.   
//Date			Wed 15 Jul 1998
//------------------------------------------------------------------------------




//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ScaledTextureWidth
//Author		Paul.   
//Date			Wed 15 Jul 1998
//------------------------------------------------------------------------------







//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Render2Map
//Author		Paul.   
//Date			Wed 15 Jul 1998
//------------------------------------------------------------------------------



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetLandBaseDetail
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ScrollTexture
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		WaitForTextureScrollComplete
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		MakeDataGridIndex
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
inline SLong MakeDataGridIndex(const UWord offset)
{
	return (SLong)offset;
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		RenderShadow
//Author		Jon   
//Date			Fri 14 Apr 2000
//
//						
//------------------------------------------------------------------------------
inline void RenderShadow(CLib3D* lib3d, vector2d* corners, float& shapeDX, float& shapeDZ )
{

	for ( int i = 0; i<4; i++ )
	{
		if ( corners[i].x < 1 || corners[i].z < 1 ||
			 corners[i].x > 254 || corners[i].z > 254 )
		{
			NOP;
			return;
		}

	}
	
	Lib3DPoint* verts = lib3d->BeginPoly( NULL_MATERIAL, 6 );

	verts[0].setPosition( corners[3].x,			corners[3].z, 0.f );
	verts[1].setPosition( corners[0].x,			corners[0].z, 0.f );
	verts[2].setPosition( corners[1].x,			corners[1].z, 0.f );
	verts[3].setPosition( corners[1].x-shapeDX,	corners[1].z-shapeDZ, 0.f );
	verts[4].setPosition( corners[2].x-shapeDX,	corners[2].z-shapeDZ, 0.f );
	verts[5].setPosition( corners[3].x-shapeDX,	corners[3].z-shapeDZ, 0.f );

// Set colour and alpha for shadow...

	verts[0].setVertexCol(D3DCOLOR(shadColourNear));			//RJS 19Oct00
	verts[1].setVertexCol(D3DCOLOR(shadColourNear));
	verts[2].setVertexCol(D3DCOLOR(shadColourNear));
	verts[3].setVertexCol(D3DCOLOR(shadColourFar));
	verts[4].setVertexCol(D3DCOLOR(shadColourFar));
	verts[5].setVertexCol(D3DCOLOR(shadColourFar));

		

	lib3d->EndPoly();
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		RenderObjectShadow
//Author		Jon   
//Date			Fri 14 Apr 2000
//
//						
//------------------------------------------------------------------------------
inline void RenderObjectShadow(CLib3D* lib3d,ShapeDescPtr sdp,SLong& x,SLong& z, SWord& hdg)
{
	if ( sdp->sy>>5 <1 )
		return;
	const float SHAPESCALE = 1.f/float( 1<<5 );
	float shapeRadx = sdp->sx*SHAPESCALE;
	float shapeHeight = sdp->sy*(SHAPESCALE*2);
	float shapeRadz = sdp->sz*SHAPESCALE;
	float shapeDX = shapeHeight*shadowDXf;
	float shapeDZ = shapeHeight*shadowDZf;						//RJS 05May00

	vector2d corners[8];

	if ( hdg == 0 )
	{
		corners[0].x = -shapeRadx;
		corners[0].z = +shapeRadz;
		corners[1].x = +shapeRadx;
		corners[1].z = +shapeRadz;
		corners[2].x = +shapeRadx;
		corners[2].z = -shapeRadz;
		corners[3].x = -shapeRadx;
		corners[3].z = -shapeRadz;

		

	}
	else
	{
		double sinAng,cosAng;

		ANGLES hdgAng=(Angles)hdg;

		Math_Lib.high_sin_cos(hdgAng,sinAng,cosAng);

		float rotMat[4];
		rotMat[0]=rotMat[3]=cosAng;
		rotMat[1]=sinAng;
		rotMat[2]=-rotMat[1];
		
		corners[0].x = (-shapeRadx*rotMat[0])+(shapeRadz*rotMat[1]);
		corners[0].z = (-shapeRadx*rotMat[2])+(shapeRadz*rotMat[3]);
		corners[1].x = (shapeRadx*rotMat[0])+(shapeRadz*rotMat[1]);
		corners[1].z = (shapeRadx*rotMat[2])+(shapeRadz*rotMat[3]);
		corners[2].x = (shapeRadx*rotMat[0])+(-shapeRadz*rotMat[1]);
		corners[2].z = (shapeRadx*rotMat[2])+(-shapeRadz*rotMat[3]);
		corners[3].x = (-shapeRadx*rotMat[0])+(-shapeRadz*rotMat[1]);
		corners[3].z = (-shapeRadx*rotMat[2])+(-shapeRadz*rotMat[3]);


		
	}

	// find biggest dot product ( closest point to direction of shadow)
	float max = 0.0;
	int index, maxI;
	for ( index = 0; index < 4; index++ )
	{
		float cur = corners[index].x*shapeDX + corners[index].z*shapeDZ;
		if ( cur > max )
		{
			max = cur;
			maxI = index;
		}
		corners[index].x+=x;
		corners[index].z+=z;
	}
	memcpy( corners+4, corners, sizeof( corners[0] ) *4 );
	RenderShadow( lib3d, corners+maxI, shapeDX, shapeDZ );
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PatchColorData
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		UpdateColorData
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		UpdateGridRecords
//Author		Paul.   
//Date			Mon 20 Jul 1998
//------------------------------------------------------------------------------
inline void UpdateGridRecords(const SLong x,const SLong z)
{
	DataRecord* pdr=gridRecords;
	SLong ox,oz,ix;
	SLong dataGridSize;

	dataGridSize=DataGridSizeMIN;

	ox=(x&Grid_Mask)-((dataGridSize>>1)*Grid_Width);
	oz=(z&Grid_Mask)-((dataGridSize>>1)*Grid_Width);

	SLong dx=Abs(gridRecordBase_x-ox);
	SLong dz=Abs(gridRecordBase_z-oz);

	if (dx!=0 || dz!=0)
	{
		gridRecordBase_x=ox;
		gridRecordBase_z=oz;

		for (int j=0;j<dataGridSize;j++)
		{
			ix=ox;
			for (int k=0;k<dataGridSize;k++)
			{
				*pdr++=Three_Dee.pMigLand->GetData(ix,oz);
				ix+=Grid_Width;
			}
			oz+=Grid_Width;
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		InitGridRecords
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------

void InitGridRecords(const SLong x,const SLong z)
{
	SLong ox,oz,ix;
	SLong dataGridSize;
	DataRecord* pdr;

//	dataGridSize=Save_Data.detail_3d[DETAIL3D_HORIZONDIST]?
//					DataGridSizeMAX:DataGridSizeMIN;
	dataGridSize=DataGridSizeMIN;

	pdr=gridRecords;
	ox=(x&Grid_Mask)-((dataGridSize>>1)*Grid_Width);
	oz=(z&Grid_Mask)-((dataGridSize>>1)*Grid_Width);

	gridRecordBase_x=ox;
	gridRecordBase_z=oz;

	for (int j=0;j<dataGridSize;j++)
	{
		ix=ox;
		for (int k=0;k<dataGridSize;k++)
		{
			*pdr++=Three_Dee.pMigLand->GetData(ix,oz);
			ix+=Grid_Width;
		}
		oz+=Grid_Width;
	} 
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ProcessLandUpdateRequests
//Author		Paul.   
//Date			Fri 17 Jul 1998
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetBlock
//Author		Paul.   
//Date			Wed 22 Jul 1998
//------------------------------------------------------------------------------
inline DataRecord GetBlock(const SLong x,const SLong z)
{
//	SLong dataGridSize=Save_Data.detail_3d[DETAIL3D_HORIZONDIST]?DataGridSizeMAX:DataGridSizeMIN;
	SLong dataGridSize=DataGridSizeMIN;
	SLong ix=(x&Grid_Mask)-gridRecordBase_x;
	SLong iz=(z&Grid_Mask)-gridRecordBase_z;
	ix/=Grid_Width;
	iz/=Grid_Width;
	return gridRecords[(iz*dataGridSize)+ix];
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AllocateBuffers
//Author		Paul.   
//Date			Tue 6 Oct 1998
//------------------------------------------------------------------------------
void AllocateBuffers()
{
//	if (Save_Data.detail_3d[DETAIL3D_HORIZONDIST])
//	{
		//full horizon

//		land_x=new SLong[Max_TextureLevelsMAX];
//		land_z=new SLong[Max_TextureLevelsMAX];
//		update_req=new bool[Max_TextureLevelsMAX];
 //		gridRecords=new DataRecord[DataGridSizeMAX*DataGridSizeMAX];
//		shadowRecords=new ShadowStruc[DataGridSizeMAX*DataGridSizeMAX];
//		clipRects=new Rect[5*(Max_TextureLevelsMAX-1)+1];
//		gridClipper=new UByte[_areaBufferSizeMAX];
//		wholeAreaClips=new SquareClips[_areaBufferSizeMAX];
//	}
//	else
	{
		//reduced horizon

  		gridRecords=new DataRecord[DataGridSizeMIN*DataGridSizeMIN];
	}

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ReleaseBuffers
//Author		Paul.   
//Date			Tue 6 Oct 1998
//------------------------------------------------------------------------------
inline void ReleaseBuffers()
{
	delete[]gridRecords,gridRecords=NULL;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FrameInit
//Author		Jon
//Date			Fri 14 Apr	2000
//------------------------------------------------------------------------------
inline void FrameInit(WorldStuff *wP,LightVec& sunLight)
{
	float	zStretch = SHAD_CUT_OFF;									//RJS 19Oct00
	worldPtr=wP;

	if (sunLight.nj.i > 0.f)									//RJS 19Oct00
	{
		zStretch = 1.f/float(sunLight.nj.i);		//MS 05May00
		if (zStretch > SHAD_CUT_OFF)	zStretch = SHAD_CUT_OFF;
	}

	float	shadScale = zStretch / SHAD_CUT_OFF;

	shadNearAlpha = shadNearMin + (shadScale * (shadNearMax-shadNearMin));
	shadFarAlpha = shadNearAlpha - ((shadNearAlpha * shadScale));

	if ( zStretch < 0 )
		zStretch = -zStretch;

	// generate the shadow dx and sz's.
	shadowDXf = float(sunLight.ni.i)*zStretch;
	shadowDZf = float(sunLight.nk.i)*zStretch;

//TempCode JON 25Sep00 	const float MAX_SHADOW_MULT = 1000.f;
//TempCode JON 25Sep00 	if ( shadowDXf > MAX_SHADOW_MULT )
//TempCode JON 25Sep00 		shadowDXf = MAX_SHADOW_MULT;
//TempCode JON 25Sep00 	else if ( shadowDXf < -MAX_SHADOW_MULT )
//TempCode JON 25Sep00 		shadowDXf = -MAX_SHADOW_MULT;
//TempCode JON 25Sep00 
//TempCode JON 25Sep00 	if ( shadowDZf > MAX_SHADOW_MULT )
//TempCode JON 25Sep00 		shadowDZf = MAX_SHADOW_MULT;
//TempCode JON 25Sep00 	else if ( shadowDZf < -MAX_SHADOW_MULT )
//TempCode JON 25Sep00 		shadowDZf = -MAX_SHADOW_MULT;

}





//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		RenderTile2Surface
//Author		Paul.   
//Date			Fri 6 Nov 1998
//------------------------------------------------------------------------------



inline SWord RenderEdgeStrip(SWord x1,SWord y1,SWord x2,SWord y2,SWord right_ix,SWord shift)
{ 
	SWord xx[4],yy[4];
	SWord dx,dy;
	SWord left_ix;
	dy=(y2-y1);
	dx=(x2-x1);
	SWord len=fastMath.FastLen(dx,dy);
	if (len)
	{
		SWord tmp=dx;
		dx=-(dy<<2)/len;
		dy=(tmp<<2)/len;
		xx[0]=x1+dx;
		yy[0]=y1+dy;
		xx[1]=x2+dx;
		yy[1]=y2+dy;
		xx[2]=x2-dx;
		yy[2]=y2-dy;
		xx[3]=x1-dx;
		yy[3]=y1-dy;
		left_ix=right_ix&0xFF;
		right_ix=left_ix+len;

//		return right_ix;



		Lib3DPoint *pRVert = g_lpLib3d->BeginPoly(HMATERIAL(Image_Map.GetImageMapPtr(BLENDMAPNO)),4);

		pRVert->setPosition( xx[0], yy[0], 0.f );
		pRVert->setIMapCoords( left_ix, shift );
		pRVert++;

		pRVert->setPosition( xx[1], yy[1], 0.f );
		pRVert->setIMapCoords( right_ix, shift );
		pRVert++;

		pRVert->setPosition( xx[2], yy[2], 0.f );
		pRVert->setIMapCoords( right_ix, 8+shift );
		pRVert++;

		pRVert->setPosition( xx[3], yy[3], 0.f );
		pRVert->setIMapCoords( left_ix, 8+shift );

		g_lpLib3d->EndPoly();
		
	}
	return right_ix;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Render2Surface
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------

static ULong DebugRecs[256*256];


inline bool HasSheep(UByte at)
{
	if ( at<RT_Lowest )
	{
		ULong at_shift = 1UL<<(at&AT_MASK);
		const ULong at_shift_mask =
					(1UL<<(AT_spread1)) |
					(1UL<<(AT_spread2)) |
					(1UL<<(AT_spread3)) |
					(1UL<<(AT_spread4)) |
					(1UL<<(AT_spread5)) |
					(1UL<<(AT_rough)) |
					(1UL<<(AT_roughGrass)) |
					(1UL<<(AT_patchyGrass));
		return ( (at_shift&at_shift_mask) != 0 );
	} else return false;
}
inline bool HasCows(UByte at)
{
	if ( at<RT_Lowest )
	{
		ULong at_shift = 1UL<<(at&AT_MASK);
		const ULong at_shift_mask =
					(1UL<<(AT_midGrass)) |
					(1UL<<(AT_darkGrass));
		return ( (at_shift&at_shift_mask) != 0 );
	} else return false;

}
inline bool HasHayStacks(UByte at)
{
	if ( at<RT_Lowest )
	{
		ULong at_shift = 1UL<<(at&AT_MASK);
		const ULong at_shift_mask =
					(1UL<<(AT_field1)) |
					(1UL<<(AT_field2));
		return ( (at_shift&at_shift_mask) != 0 );
	} else return false;
}

//#pragma warnmsg ( "inline me" )
inline void Render2Surface(const HTEXT& hTexture,UByte rezTexture,UWord offset)
{
	
	SLong dgi=MakeDataGridIndex(offset);

	DataRecord& currentData=gridRecords[dgi];
	UByte* pdata=Three_Dee.pMigLand->MakeIData(currentData,0);//rezTexture>>1);	

//	D3DVECTOR tv={-.1f,-.1f,0.f};
//	D3DVECTOR preTrans={.5f,.5f,0.f };
	static D3DVECTOR preTrans={-0.49f,0.49f,0.f };
//	static D3DVECTOR preScale={(255.f/256.f),(255.f/256.f),1.f};
	D3DVECTOR tv={-.5f,-.5f,0.f};
	D3DVECTOR sf={2.f,2.f,1.f};

	g_lpLib3d->BeginScene( HRENDERTARGET( RENDERTARGET_LANDSCAPE, UWord(hTexture.textureHandle) ) );
//	g_lpLib3d->PushMatrix(MATRIX_OBJECT);
//	g_lpLib3d->PushMatrix(MATRIX_VIEWER);
//	g_lpLib3d->PushMatrix(MATRIX_PROJECTION);
	g_lpLib3d->LoadIdentity(MATRIX_OBJECT);

//	g_lpLib3d->Scale(MATRIX_OBJECT,preScale);
	g_lpLib3d->Translate(MATRIX_OBJECT,preTrans);
	g_lpLib3d->Scale(MATRIX_OBJECT,(DIV_SCALE));  // this could be optimised to make a constant matrix
	g_lpLib3d->Translate(MATRIX_OBJECT,tv);		// to be set to....
	g_lpLib3d->Scale(MATRIX_OBJECT,sf);

	g_lpLib3d->LoadIdentity(MATRIX_VIEWER);
	g_lpLib3d->LoadIdentity(MATRIX_PROJECTION); 

	g_lpLib3d->GiveHint( HINT_2DRENDER );

	g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, 5 );
	
	AltPointsHdr*	aph;
	DrawDataHdr*	ddh;

	aph=(AltPointsHdr*)pdata;

	if (rezTexture<4)
	{
		SLong vc=aph->vertexCount-1;
		for (SLong h=vc;h>=0;h--)
			for (SLong w=vc,hi=h<<8;w>=0;w--) DebugRecs[hi++]=0UL;
	}

	SWord edge_points=aph->leftEdgePoints+aph->rightEdgePoints+aph->topEdgePoints+aph->bottomEdgePoints+4;

	pdata+=sizeof(AltPointsHdr);

	pointdef=(PointDef*)pdata;

//TEMPCODE JON 5/26/00 	// right copy the pointdefs into the point buffer					//JON 5/26/00
//TEMPCODE JON 5/26/00 	DoPointStruc* pPoint = SHAPE.newco;
//TEMPCODE JON 5/26/00 	for ( int i=vertexCoint; i!=0; i-- )
//TEMPCODE JON 5/26/00 	{
//TEMPCODE JON 5/26/00 		*pPoint->setPosition( pointdef->x, 256-pointdef->z );
//TEMPCODE JON 5/26/00 		pPoint++;
//TEMPCODE JON 5/26/00 		pointdef++;
//TEMPCODE JON 5/26/00 	}
//TEMPCODE JON 5/26/00 	pPoint = SHAPE.newco;
//TEMPCODE JON 5/26/00 	pointdef=(PointDef*)pdata;

	pdata+=aph->vertexCount*sizeof(PointDef);

	ddh=(DrawDataHdr*)pdata;
	pdata+=sizeof(DrawDataHdr);

	SWord primCount=ddh->noOfPolys;

	UByte *savedata=pdata;

//#define DO_TEST_TILE
#ifdef DO_TEST_TILE
	Lib3DPoint *pRVert = g_lpLib3d->BeginFan( HMATERIAL(imagePtrs[2]),4);
	for ( int v = 0; v<4; v++ )
	{
		pRVert->setIMapCoords( pointdef[v].x, 256-pointdef[v].z );
		pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
		pRVert++;

	}
	g_lpLib3d->EndFan();
#else
	while (primCount--)
	{
		ULong at,atMask;
		int rotation;
		at=*pdata;
		HMATERIAL hMat;
//TempCode JON 25Oct00 		rotation = AT_ROT0;
//TempCode JON 25Oct00 		if ( HasSheep(at) )
//TempCode JON 25Oct00 		{
//TempCode JON 25Oct00 			at&=AT_MASK;
//TempCode JON 25Oct00 			hMat = HMATERIAL( 120 );//red
//TempCode JON 25Oct00 		} else if ( HasCows( at ) )
//TempCode JON 25Oct00 		{
//TempCode JON 25Oct00 			at&=AT_MASK;
//TempCode JON 25Oct00 			hMat = HMATERIAL( 191 );//green
//TempCode JON 25Oct00 		} else if ( HasHayStacks( at ) )
//TempCode JON 25Oct00 		{
//TempCode JON 25Oct00 			at&=AT_MASK;
//TempCode JON 25Oct00 			hMat = HMATERIAL( 144 );//blue
//TempCode JON 25Oct00 		} else 
//TempCode JON 25Oct00 		{
//TempCode JON 25Oct00 			at&=AT_MASK;
//TempCode JON 25Oct00 			hMat = HMATERIAL( 159 );//white
//TempCode JON 25Oct00 		}
		// now check to see if the areatype 4 this is one of the widened routes types
		if ( at<RT_Lowest )
		{ // it isn't
			rotation = (at|AT_MASK);
			at&=AT_MASK;
			hMat=HMATERIAL(imagePtrs[at]);
		} else {
			UByte ra = at - RT_Lowest;
			ra&=AT_ROT0;
			rotation = AT_ROT0;
			switch ( ra )
			{
				// road types
				case RA_AROAD:
				case RA_BROAD:
				case RA_ROADJUNCTION:
				case RA_CROSSING3:
				case RA_RIVAROADJUNCT:
				case RA_RIVBROADJUNCT:
					g_lpLib3d->SetFontColour( 0x424242 ); //grey
					hMat=HMATERIAL( 8 ); // grey
				break;
				// rail types
				case RA_MAJORRAIL:
				case RA_MINORRAIL:
				case RA_RAILJUNCTION:
				case RA_MAJRAILROADJUNCT:
				case RA_MINRAILROADJUNCT:
				case RA_CROSSING1:
				case RA_CROSSING2:
				case RA_CROSSING4:
				case RA_RIVMINRAILJUNCT:
				case RA_RIVMAJRAILJUNCT:
					g_lpLib3d->SetFontColour( 0x3C3C2C );
					hMat=HMATERIAL( 47 ); // GREY/BROWN
				break;
				//river types
				case RA_MAJORRIVER:
				case RA_MINORRIVER:
					g_lpLib3d->SetFontColour( 0x192942 );
					hMat=HMATERIAL( 131 ); // grey/blue
				break;
				default:
//					g_lpLib3d->SetFontColour( 0xFF0000 );
//					hMat=HMATERIAL( 120 ); // red
					at = AT_midGrass;
					hMat=HMATERIAL(imagePtrs[at]);
				break;
			}
			at=AT_cityBase;
		}
		atMask=1<<at;
		pdata++;

		UByte fanCount=*pdata++;

		hMat.isMasked = MSK_NONE;  // shouldn't be needed but city types seem to want...

//		rotation = AT_ROT0;
		switch (rotation)
		{
		case AT_ROT0:
			while (fanCount--)
			{
				ULong triCount=2+*pdata++;
				Lib3DPoint *pRVert = g_lpLib3d->BeginFan(hMat,triCount);

				SWord v,last_v,first_v;
				v=first_v=*pdata++;
				triCount--;

				pRVert->setIMapCoords( pointdef[v].x, 256-pointdef[v].z );
				if ( IsHirez(at) )
					pRVert->setIMapCoords( pRVert->getIMapX()<<1, pRVert->getIMapY()<<1 );
				pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
				pRVert++;
				while (triCount--)
				{
					last_v=v;
					v=*pdata++;
					if (v>=edge_points || last_v>=edge_points)
					{
						DebugRecs[(v<<8)+last_v]|=atMask;
						DebugRecs[(last_v<<8)+v]|=atMask;
					}

					pRVert->setIMapCoords( pointdef[v].x, 256-pointdef[v].z );
					if ( IsHirez(at) )
						pRVert->setIMapCoords( pRVert->getIMapX()<<1, pRVert->getIMapY()<<1 );
					pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
					pRVert++;
				}

				g_lpLib3d->EndFan();

				if (v>=edge_points || first_v>=edge_points)
				{
					DebugRecs[(v<<8)+first_v]|=atMask;
					DebugRecs[(first_v<<8)+v]|=atMask;
				}
			}
			break;
		case AT_ROT1:
			while (fanCount--)
			{
				ULong triCount=2+*pdata++;
				Lib3DPoint *pRVert = g_lpLib3d->BeginFan(hMat,triCount);

				SWord v,last_v,first_v;
				v=first_v=*pdata++;
				triCount--;
				pRVert->setIMapCoords( 256-pointdef[v].z, 256-pointdef[v].x );
				if ( IsHirez(at) )
					pRVert->setIMapCoords( pRVert->getIMapX()<<1, pRVert->getIMapY()<<1 );
				pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
				pRVert++;
				while (triCount--)
				{
					last_v=v;
					v=*pdata++;
					if (v>=edge_points || last_v>=edge_points)
					{
						DebugRecs[(v<<8)+last_v]|=atMask;
						DebugRecs[(last_v<<8)+v]|=atMask;
					}
					pRVert->setIMapCoords( 256-pointdef[v].z, 256-pointdef[v].x );
					if ( IsHirez(at) )
						pRVert->setIMapCoords( pRVert->getIMapX()<<1, pRVert->getIMapY()<<1 );
					pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
					pRVert++;
				}
				g_lpLib3d->EndFan();
				if (v>=edge_points || first_v>=edge_points)
				{
					DebugRecs[(v<<8)+first_v]|=atMask;
					DebugRecs[(first_v<<8)+v]|=atMask;
				}
			}
			break;
		case AT_ROT2:
			while (fanCount--)
			{
				ULong triCount=2+*pdata++;
				Lib3DPoint *pRVert = g_lpLib3d->BeginFan(hMat,triCount);

				SWord v,last_v,first_v;
				v=first_v=*pdata++;
				triCount--;
				pRVert->setIMapCoords( 256-pointdef[v].x, pointdef[v].z );
				if ( IsHirez(at) )
					pRVert->setIMapCoords( pRVert->getIMapX()<<1, pRVert->getIMapY()<<1 );
				pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
				pRVert++;
				while (triCount--)
				{
					last_v=v;
					v=*pdata++;
					if (v>=edge_points || last_v>=edge_points)
					{
						DebugRecs[(v<<8)+last_v]|=atMask;
						DebugRecs[(last_v<<8)+v]|=atMask;
					}
					pRVert->setIMapCoords( 256-pointdef[v].x, pointdef[v].z );
					if ( IsHirez(at) )
						pRVert->setIMapCoords( pRVert->getIMapX()<<1, pRVert->getIMapY()<<1 );
					pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
					pRVert++;
				}
				g_lpLib3d->EndFan();

				if (v>=edge_points || first_v>=edge_points)
				{
					DebugRecs[(v<<8)+first_v]|=atMask;
					DebugRecs[(first_v<<8)+v]|=atMask;
				}
			}
			break;
		case AT_ROT3:
			while (fanCount--)
			{
				ULong triCount=2+*pdata++;
				Lib3DPoint *pRVert = g_lpLib3d->BeginFan(hMat,triCount);

				SWord v,last_v,first_v;
				v=first_v=*pdata++;
				triCount--;
				pRVert->setIMapCoords( pointdef[v].x, pointdef[v].z );
				if ( IsHirez(at) )
					pRVert->setIMapCoords( pRVert->getIMapX()<<1, pRVert->getIMapY()<<1 );
				pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
				pRVert++;
				while (triCount--)
				{
					last_v=v;
					v=*pdata++;
					if (v>=edge_points || last_v>=edge_points)
					{
						DebugRecs[(v<<8)+last_v]|=atMask;
						DebugRecs[(last_v<<8)+v]|=atMask;
					}
					pRVert->setIMapCoords( pointdef[v].x, pointdef[v].z );
					if ( IsHirez(at) )
						pRVert->setIMapCoords( pRVert->getIMapX()<<1, pRVert->getIMapY()<<1 );
					pRVert->setPosition( pointdef[v].x, 256-pointdef[v].z, 0.f );
					pRVert++;
				}
				g_lpLib3d->EndFan();
				if (v>=edge_points || first_v>=edge_points)
				{
					DebugRecs[(v<<8)+first_v]|=atMask;
					DebugRecs[(first_v<<8)+v]|=atMask;
				}
			}
			break;
			NODEFAULT;
		}

	}

	g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, 4 );

	// draw in routes
//TempCode JON 25Aug00 	g_lpLib3d->GiveHint( HINT_CLIPPING_ON );						//JON 25Aug00

	primCount=(rezTexture<4 && Save_Data.detail_3d[DETAIL3D_ROUTES])?ddh->noOfLines:0;

//	g_lpLib3d->ClearTargetZ();
	while (primCount--)
	{
		UByte lineCol(0);
		SWord shift;

		switch (*pdata++)
		{
		case RT_Cliff:
			shift=SHIFT_CliffTopBaseBlend;
			break;
		case RT_AirField:
			shift=SHIFT_AirFieldRT;
			break;
		case RT_MajorRiver:
			shift=SHIFT_MajorRiver;
			break;
		case RT_MinorRiver:
			shift=SHIFT_MinorRiver;
			break;
		case RT_Tributary:
			shift=SHIFT_Tributary;
			break;
		case RT_MinorRoad:
			shift=SHIFT_MinorRoad;
			break;
		case RT_Footpath:
			shift=SHIFT_FootPath;
			break;
		case RT_MajorRail:
			shift=SHIFT_MajorRail;
			break;
		case RT_MinorRail:
			shift=SHIFT_MinorRail;
			break;
		case RT_ARoad:
			shift=SHIFT_ARoad;
			break;
		case RT_BRoad:
			shift=SHIFT_BRoad;
			break;
		case RT_HedgerowThick:
		case RT_HedgerowThin:
			shift=SHIFT_BlendGrassOther;
			break;
		default:
			{
				lineCol=ARTWORKMASK;
				UByte sectionCount=*pdata++;
				pdata+=sectionCount;
				continue;
			}
			break;
		}

		UByte sectionCount=*pdata++;

		{
			SWord v,last_v;
			SWord right_ix=0;
			v=*pdata++;
			SLong x2,y2;
			x2=pointdef[v].x;
			y2=256-pointdef[v].z;
			while (--sectionCount!=0)
			{
				SLong x1,y1;
				x1=x2;
				y1=y2;
				last_v=v;
				v=*pdata++;
				DebugRecs[(v<<8)+last_v]=
				DebugRecs[(last_v<<8)+v]=0UL;
				x2=pointdef[v].x;
				y2=256-pointdef[v].z;
				right_ix=RenderEdgeStrip(x1,y1,x2,y2,right_ix,shift);
			}
		}
	}


	g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, 3 );

	// draw in the edge anitaliasing
	primCount=rezTexture<4?ddh->noOfPolys:0;
	const ULong grassyMask=	
							(1<<AT_darkGrass)+
							(1<<AT_midGrass)+
							(1<<AT_lightGrass)+
							(1<<AT_garden)+
							(1<<AT_golfCourse)+
							(1<<AT_field1)+
							(1<<AT_field2)+
							(1<<AT_field3)+
							(1<<AT_field4)+
							(1<<AT_field5)+
							(1<<AT_spread1)+
							(1<<AT_spread2)+
							(1<<AT_spread3)+
							(1<<AT_spread4)+
							(1<<AT_spread5)+
							(1<<AT_swamp)+
							(1<<AT_rough)+
							(1<<AT_roughGrass)+
							(1<<AT_patchyGrass);	
	const ULong grassMask=
							(1<<AT_darkGrass)+
							(1<<AT_midGrass)+
							(1<<AT_lightGrass)+
							(1<<AT_garden)+
							(1<<AT_golfCourse);							//JON 2Oct00
	
	while (primCount--)
	{
		ULong at,atMask;
		at=*savedata;
		if ( at<RT_Lowest )
		{
			at&=0x1F;
		} else
		{
			at=AT_cityBase;
		}
		atMask=1<<at;
		savedata++;
		UByte fanCount=*savedata++;

		SWord right_ix=0;
		
		while (fanCount--)
		{
			R3DVERTEX *pRVert;
			ULong triCount=2+*savedata++;
			
			for (int this_i=0;this_i<triCount;this_i++)
			{
				int next_i=this_i+1;
				if (next_i==triCount) next_i=0;

				int last_v=savedata[this_i];
				int next_v=savedata[next_i];

				ULong mask=DebugRecs[(next_v<<8)+last_v];

				//only consider edge if both of the points are connected to something other than the
				//current material
				//also don't consider the edge if a route is already running along the edge
				if (mask!=0)
				{
					SWord shift=0;

/*					if ( IsPointCliff[last_v] && isPointCliff[next_v] && IsCliffBottom( at )  )
					{ // cliff antialising here

					} else */ if (at==AT_sea)
					{
						if ((mask&(1<<AT_sand))!=0)
							shift=SHIFT_BlendSeaBeach;
						else if ((mask&~(1<<AT_sea))!=0)
							shift=SHIFT_BlendSeaLand;
					}
					else if ((grassyMask&atMask)!=0)
					{
						if (
							 (((mask&~atMask)&grassyMask)!=0) && // are they of different types?
							 ((mask&(~grassMask)) || (atMask&(~grassMask)) ) //one of them isn't plain grass(for airfields, ta guys) 
							)
							shift=SHIFT_BlendGrassOther;
					}
					if (shift)
					{
						DebugRecs[(next_v<<8)+last_v]=
							DebugRecs[(last_v<<8)+next_v]=0UL;
						SLong x2,y2;
						x2=pointdef[last_v].x;
						y2=256-pointdef[last_v].z;
						SLong x1,y1;
						x1=x2;
						y1=y2;
						x2=pointdef[next_v].x;
						y2=256-pointdef[next_v].z;
						right_ix=RenderEdgeStrip(x1,y1,x2,y2,right_ix,shift);
					}
				}
			}
			savedata+=triCount;
		}
	}

	//check for shadow rendering to this tile
	if (rezTexture<3 && Save_Data.detail_3d[DETAIL3D_ITEMSHADOWS])
	{
		g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, 2 );
 		g_lpLib3d->SetGlobal( TOGGLE_GLOBALALPHA, 100 );

// Set up shadow info...

		LIGHTFLAG	useLightFlag = LF_VERTEX;							//RJS 19Oct00
		g_lpLib3d->SetObjectLighting(useLightFlag);

		shadColourNear = (shadColour & 0xFFFFFF) | (shadNearAlpha << 24);
		shadColourFar = (shadColour & 0xFFFFFF) | (shadFarAlpha << 24);

		//currentData contains world coordinates for sector search
		item dummyItem,*itemPtr;
		UWord sec_x,sec_y;
		dummyItem.World.X=currentData.x+(Grid_Width>>1);
		dummyItem.World.Z=currentData.z+(Grid_Width>>1);
		worldPtr->getsectornos(&dummyItem,sec_x,sec_y);
		ULong sec_index=worldPtr->makeindex(sec_x,sec_y);
		itemPtr=worldPtr->getfirstitem(sec_index);

		while (itemPtr)
		{
			const SLong gridSx=currentData.x;
			const SLong gridSz=currentData.z;
			const SLong gridEx=gridSx+Grid_Width;
			const SLong gridEz=gridSz+Grid_Width;
			const SLong world2Pix=9;	//+scale;

		 	if (itemPtr->Status.size<=RotatedSize)
			{
				ShapeDescPtr shapePtr=SHAPESTUFF.GetShapePtr(itemPtr->shape);

				if (SHAPESTUFF.GetShapeScale(shapePtr)==SHP_GRP)
				{
					if (groupHasShadow(itemPtr->shape))
					{
						UByte* inst_ptr=(UByte*)shapePtr+shapePtr->liveshpref;

						for (SWord inst=*inst_ptr++;inst==dogroupno;inst=*inst_ptr++)
						{
							DOGROUP_PTR gptr=(DOGROUP_PTR)inst_ptr;

							SLong cx=itemPtr->World.X+gptr->xoffset;
							SLong cz=itemPtr->World.Z+gptr->zoffset;

							if (cx>gridSx && cx<gridEx && cz>gridSz && cz<gridEz && shapeHasShadow((ShapeNum)gptr->shapenum))
							{
								SWord hdg=gptr->angle;
								cx=(cx-gridSx)>>world2Pix;
								cz=(cz-gridSz)>>world2Pix;

								ShapeDescPtr groupPtr=SHAPESTUFF.GetShapePtr((ShapeNum)gptr->shapenum);

								RenderObjectShadow(g_lpLib3d,groupPtr,cx,cz,hdg);
							}
							inst_ptr+=sizeof(DOGROUP);
						}
					}
				}
				else if (	itemPtr->World.X>gridSx &&
							itemPtr->World.X<gridEx &&
							itemPtr->World.Z>gridSz &&
							itemPtr->World.Z<gridEz &&
							shapeHasShadow(itemPtr->shape))
				{
					SWord hdg;
					SLong tx,tz;
					hdg=itemPtr->Status.size<HdgSize?0:((hdgitem*)itemPtr)->hdg;

					tx=(itemPtr->World.X-gridSx)>>world2Pix;
					tz=(itemPtr->World.Z-gridSz)>>world2Pix;
					RenderObjectShadow(g_lpLib3d,shapePtr,tx,tz,hdg);

				}
			}
			itemPtr=itemPtr->Next;
		}

		g_lpLib3d->SetObjectLighting(useLightFlag);							//RJS 19Oct00
	}
#endif
//	g_lpLib3d->PopMatrix(MATRIX_PROJECTION);
//	g_lpLib3d->PopMatrix(MATRIX_VIEWER);
//	g_lpLib3d->PopMatrix(MATRIX_OBJECT);
	g_lpLib3d->EndScene();
	g_lpLib3d->UploadTexture();
}

bool CheckCache()
{
	for (int i=DataGridSizeMIN*DataGridSizeMIN-1;i>=0;i--)
	{
		AltPointsHdr *aph=(AltPointsHdr*)gridRecords[i].pData;
		if (!aph->CheckChecksum())
			return false;
	}
	return true;
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetMinMaxAlt
//Author		Paul.   
//Date			Wed 16 Dec 1998
//------------------------------------------------------------------------------
void GetMinMaxAlt(const SLong x,const SLong z,const SWord dir,SLong& min,SLong& max)
{
	SLong dataGridSize=DataGridSizeMIN;
	SLong ix=(x&Grid_Mask)-gridRecordBase_x;
	SLong iz=(z&Grid_Mask)-gridRecordBase_z;
	ix/=Grid_Width;
	iz/=Grid_Width;

	DataRecord* dr=&gridRecords[(iz*dataGridSize)+ix];
	AltPointsHdr* aph=(AltPointsHdr*)dr->pData;

	min=aph->minAlt;
	max=aph->maxAlt;

	if (dir==0)			iz++;
	else if (dir==1)	ix++;
	else if (dir==2)	iz--;
	else if (dir==3)	ix--;

	dr=&gridRecords[(iz*dataGridSize)+ix];
	aph=(AltPointsHdr*)dr->pData;

	if (aph->minAlt<min) min=aph->minAlt;
	if (aph->maxAlt>max) max=aph->maxAlt;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetMinMaxAlt
//Author		Paul.   
//Date			Wed 16 Dec 1998
//------------------------------------------------------------------------------
void GetMinMaxAlt(const SLong x,const SLong z,SLong& min,SLong& max)
{
//	SLong dataGridSize=Save_Data.detail_3d[DETAIL3D_HORIZONDIST]?DataGridSizeMAX:DataGridSizeMIN;
	SLong dataGridSize=DataGridSizeMIN;
	SLong ix=(x&Grid_Mask)-gridRecordBase_x;
	SLong iz=(z&Grid_Mask)-gridRecordBase_z;
	ix/=Grid_Width;
	iz/=Grid_Width;
	if (ix<0 || iz<0 || ix>=dataGridSize || iz>=dataGridSize)
	{
		min=0;
		max=257000/_altitudeScale;
	}
	else
	{
		DataRecord* dr=&gridRecords[(iz*dataGridSize)+ix];
		AltPointsHdr* aph=(AltPointsHdr*)dr->pData;
		min=aph->minAlt;
		max=aph->maxAlt;
	}
}

}

using namespace TileMake;

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Init
//Author		Paul.   
//Date			Thu 16 Jul 1998
//------------------------------------------------------------------------------

void TMake::Reinit(const SLong x,const SLong z,CLib3D* lib3d) const
{
	g_lpLib3d=lib3d;
	InitGridRecords(x,z);
}

void TMake::Init(const SLong x,const SLong z,CLib3D* lib3d) const
{
	OverLay.LoaderScreen((1<<8)+96);
	g_lpLib3d=lib3d;
	SetTexturePointers();
	OverLay.LoaderScreen((1<<8)+128);
	OverLay.LoaderScreen((1<<8)+128+32);
	InitGridRecords(x,z);
	OverLay.LoaderScreen((1<<8)+128+64);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Exit
//Author		Paul.   
//Date			Thu 16 Jul 1998
//------------------------------------------------------------------------------
void TMake::Exit() const
{
	ClearTexturePointers();
	g_lpLib3d=NULL;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ChkUpdate
//Author		Paul.   
//Date			Mon 20 Jul 1998
//------------------------------------------------------------------------------
void TMake::ChkUpdate(const SLong x,const SLong z) const
{
	UpdateGridRecords(x,z);
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FrameInit
//Author		Jon   
//Date			Fri 14 Apr 2000
//------------------------------------------------------------------------------
void TMake::FrameInit(WorldStuff* wp, LightVec& sunLight)
{
	TileMake::FrameInit( wp, sunLight );
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetBlock
//Author		Paul.   
//Date			Wed 22 Jul 1998
//------------------------------------------------------------------------------
DataRecord TMake::GetBlock(const SLong x,const SLong z) const
{
	return TileMake::GetBlock(x,z);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetSquares
//Author		Paul.   
//Date			Wed 22 Jul 1998
//
//Description	Debug code that returns a series of boxes showing the area
//				of the view covered by each texture map level
//
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetClipRegion
//Author		Paul.   
//Date			Mon 27 Jul 1998
//------------------------------------------------------------------------------


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		TMake
//Author		Paul.   
//Date			Thu 6 Aug 1998
//------------------------------------------------------------------------------
TMake::TMake()
{
	AllocateBuffers();
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		TMake
//Author		Paul.   
//Date			Tue 6 Oct 1998
//------------------------------------------------------------------------------
TMake::~TMake()
{
 	ReleaseBuffers();
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Render2Surface
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void TMake::Render2Surface(const HTEXT& hTexture,UByte rezTexture,UWord offset)
{
	TileMake::Render2Surface(hTexture,rezTexture,offset);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetMinMaxAlt
//Author		Paul.   
//Date			Wed 16 Dec 1998
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetMinMaxAlt
//Author		Paul.   
//Date			Wed 16 Dec 1998
//------------------------------------------------------------------------------
void TMake::GetMinMaxAlt(const SLong x,const SLong z,const SWord dir,SLong& min,SLong& max)
{
	TileMake::GetMinMaxAlt(x,z,dir,min,max);
}






bool TMake::CheckCache()
{
	return TileMake::CheckCache();
}
ImageMapDescPtr TMake::GetImageFromArea(UByte at)
{
	return TileMake::GetImageFromArea(at);
}

