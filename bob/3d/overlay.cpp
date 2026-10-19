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
//Filename       overlay.cpp
//System
//Author         Paul.
//Date           Thu 4 Jun 1998
//Description
//------------------------------------------------------------------------------

//#define _REDO_ME_

#ifndef	NDEBUG
//#define	_XYCHEAT_
#define _SHOWCURTILENUM
//#define _SHOWDIGITIME
#define _SHOWFRAMETIME
#define _SHOW_FREE_RAM
#define _SHOW_RAM_PERCENT
//#define _SHOW_RECENT_FRAME_TIMES
#endif


//#define _SHOWFRAMETIME
//#define _SHOW_RECENT_FRAME_TIMES


#define SCREENSHOT_STILLS 0
#define SCREENSHOT_VIDEO 1
#define SCREENSHOT_PANORAMIC 2

#define SCREENSHOT_MODE SCREENSHOT_STILLS
//#define SCREENSHOT_MODE SCREENSHOT_VIDEO
//#define SCREENSHOT_MODE SCREENSHOT_PANORAMIC

#define	OFFLEFT	0x00000001
#define	OFFRIGHT 0x00000002
#define OFFTOP 0x00000004
#define OFFBOTTOM 0x00000008

// get rid of this later...
//#define	_NOTFORDEMO_

#include "3ddefs.h"
enum	ImageMapNumber;
// x0r ??? #include <afxctl.h>
// x0r ??? #include	<afxwin.h>
#include "dosdefs.h"
#define	F_BATTLE
#include	"files.g"
#include "fileman.h"
#include "polygon.h"
#include "imagemap.g"
#include "viewsel.h"
#include "overlay.h"
#include "keytest.h"
#include	"savegame.h"
#include <stdio.h>
#include "../mfc/resource.h"
#include "analogue.h"
#include "CString.h"
#include "flymodel.h"
#include "landscap.h"
#include	"rchatter.h"
#include	"messengn.h"
#include	"cstring.h"
#include <SDL_ttf.h>
const char* RowanLang();	//MFC_stub.cpp - optional UI language override
static unsigned char NextOverlayChar(const char*& p);	//fwd: defined by PatchFontTable
#include "ai.h"
#include "Gameset.h"
#include "grid.h"
#include "bstream.h"
#include "stub3d.h"
#include	<mbstring.h>
#include "3dcode.h"
#include "3dcom.h"
#include	"winmove.h"
#include	"persons2.h"
#include	"replay.h"
#include	"fastmath.h"
#include	"transite.h"

const ULong ONE_SECOND_TIME = 1000;
extern	CDC g_OffScreenDC;

class CFont;
											//RJS 1Dec00
extern CFont* (g_AllFonts[][4]);

int strcasecmp( const char *s1, const char *s2 )
{
  while (1)
    {
     int c1 = tolower( (unsigned char) *s1++ );
     int c2 = tolower( (unsigned char) *s2++ );
     if (c1 == 0 || c1 != c2) return c1 - c2;
    }
}

static ULong vsets[16]=
{
	0x00000000,0x000000ff,0x0000ff00,0x0000ffff,
	0x00ff0000,0x00ff00ff,0x00ffff00,0x00ffffff,
	0xff000000,0xff0000ff,0xff00ff00,0xff00ffff,
	0xffff0000,0xffff00ff,0xffffff00,0xffffffff
};



extern bool fNot16bit;

const ULong NO_BACKGROUND_WIPE=0x00000000;
const ULong WIPE_ALPHA_MASK=0xFF000000;
const ULong GLASS_GREY_COLOUR=0x7F1F1F1F;
const ULong GLASS_COLOUR=GLASS_GREY_COLOUR;	//0x7F1F7F1F;
const ULong TEXT_NORMAL=0xFF00FF00;
const ULong TEXT_HI=0xFFFF0000;
const ULong TEXT_GREYED=0xFF1F7F1F;
const ULong TEXT_MAP_BLACK=0xFF000000;
const ULong TEXT_MAP_WHITE=0xFFFFFFFF;
const ULong ICON_BACK_COLOUR=0xFFFFFFFF;

const ULong INFO_BAR_COL=0x404040;
const UByte INFO_BAR_ALPHA=200;

#define SET_WIPE_TEXTURE(p1) ULong((p1)&0xFFFF)
#define GET_WIPE_TEXTURE(p1) SWord((p1)&0xFFFF)

static int doneonce=0;
//TempCode JON 5Nov00 static bool nofontkludge=true;
SWord scanX,scanY;


#ifndef	NDEBUG
static	char	shpdebug[256];
#endif

struct OString
{
	char* text;
	ULong resID;
	OString(ULong id=-1L) {text=NULL;resID=id;}
};


inline Colour MAKE_SOFT_COL(ULong inp)
{
	if (inp==TEXT_NORMAL || inp==TEXT_GREYED || inp==TEXT_MAP_WHITE || inp==ICON_BACK_COLOUR)
		return WHITE;
	if (inp==TEXT_HI)
		return RED;
	if (inp==TEXT_MAP_BLACK)
		return BLACK;
	return WHITE;
}

static OString OSpeedText(IDS_INFOPANEL_SPEED);
static OString OAltitudeText(IDS_ALT);
static OString OMachText(IDS_MACH);
static OString OAmmoText(IDS_GUNAMMO);
static OString OHdgText(IDS_HDG);
static OString OThrustText(IDS_THRUST);
static OString OFrameTimeText(IDS_FRAMES_SEC);
static OString ORangeText(IDS_INFOPANEL_RANGE);
static OString ORelBrgText(IDS_RELBRG);
static OString ORelAltText(IDS_RELALT);
static OString OUnitsMeters(IDS_M_SH);
static OString OUnitsFeet(IDS_FT_ABBR);
static OString OViewee(IDS_VIEWEE);
static OString OViewType(IDS_INFOPANEL_VIEW);
static OString OViewInfo2(IDS_VIEWREQUESTEDNOTAVAILABLE);
static OString OViewResetView(IDS_INFOPANEL_PLAYER);
static OString OViewEnemyView(IDS_ENEMY);
static OString OViewFrndView(IDS_FRIENDLY);
static OString OViewGrndTView(IDS_INFOPANEL_TARGET);
static OString OViewWayptView(IDS_INFOPANEL_WAYPOINT);
static OString OViewMessage(IDS_MESSAGE);
static OString OViewResetEnemyView=OViewEnemyView;
static OString OViewResetFrndView=OViewFrndView;
static OString OViewResetGrndTView=OViewGrndTView;
static OString OViewResetWayptView=OViewWayptView;
static OString OViewResetMessage=OViewMessage;
static OString OViewInsidePadTog(IDS_PADLOCK);
static OString OViewInsideFwdTog(IDS_INSIDELOOKFORWARDS);
static OString OViewInsideTog(IDS_INSIDE);
static OString OViewOutsidePadTog=OViewInsidePadTog;
static OString OViewOutsideTog(IDS_TRACK);
static OString OViewOutsideRevPadTog(IDS_OUTSIDEREVERSEDPADLOCK);
static OString OViewSatelliTog(IDS_SATELLITE);
static OString OViewNrSatelliTog(IDS_NEARSATELLITE);
static OString OViewFlyByTog(IDS_FLYBY);
static OString OViewChaseTog(IDS_CHASE);
static OString OViewCheatTog(IDS_NOCOCKPIT);


inline bool IsWide(SLong pwdth) {return pwdth<640?false:true;}
inline bool IsExtraWide(SLong pwidth) {return pwidth<=800?false:true;}

static OString* OInfoPanelText[]=
{
	&OSpeedText,
	&OAltitudeText,
	&OMachText,
	&OAmmoText,
	&OHdgText,
	&OThrustText,
	&OFrameTimeText,
	&ORangeText,
	&ORelBrgText,
	&ORelAltText,
	&OUnitsMeters,
	&OUnitsFeet,
	&OViewee,
	&OViewType,
	&OViewInfo2,
	&OViewResetView,
	&OViewEnemyView,
	&OViewFrndView,
	&OViewGrndTView,
	&OViewWayptView,
	&OViewMessage,
	&OViewResetEnemyView,
	&OViewResetFrndView,
	&OViewResetGrndTView,
	&OViewResetWayptView,
	&OViewResetMessage,
	&OViewInsidePadTog,
	&OViewInsideFwdTog,
	&OViewInsideTog,
	&OViewOutsidePadTog,
	&OViewOutsideTog,
	&OViewOutsideTog,
	&OViewSatelliTog,
	&OViewNrSatelliTog,
	&OViewFlyByTog,
	&OViewChaseTog,
	&OViewCheatTog
};

const SLong _NumInfoTexts=sizeof(OInfoPanelText)/sizeof(OString*);

OString* listViewee[]=
{
	&OViewResetView,
	&OViewEnemyView,
	&OViewFrndView,
	&OViewGrndTView,
	&OViewWayptView,
	&OViewMessage,
	&OViewResetEnemyView,
	&OViewResetFrndView,
	&OViewResetGrndTView,
	&OViewResetWayptView,
	&OViewResetMessage
};

OString* listViewType[]=
{
	&OViewInsidePadTog,
	&OViewInsideTog,
	&OViewOutsidePadTog,
	&OViewOutsideTog,
	&OViewOutsideTog,
	&OViewSatelliTog,
	&OViewNrSatelliTog,
	&OViewFlyByTog,
	&OViewChaseTog,
	&OViewCheatTog,
	&OViewCheatTog,
	&OViewCheatTog,
	&OViewCheatTog
};

const SWord VIEWEE_LIST_SIZE=sizeof(listViewee)/sizeof(OString*);
const SWord VIEWTYPE_LIST_SIZE=sizeof(listViewType)/sizeof(OString*);

class COverlay OverLay;

struct COORDS2D
{
	SWord x,y;
};

struct RECT2D
{
	SWord x,y,w,h;
};

struct IMAGEDEF
{
  	ImageMapNumber imageNo;
	RECT2D mappingCoords;
};

struct ICON
{
	RECT2D hitBox;
	IMAGEDEF unclicked;
	IMAGEDEF clicked;
};

const SWord HALF_WIDTH_640=320;
const SWord HALF_HEIGHT_640=320;
const Float FHALF_WIDTH=Float(HALF_WIDTH_640);

//const SWord INFO_LINE_HEIGHT=8;	//info bar height in pixels @ 640x480
const SWord REPLAY_BAR_WIDTH=640;	//replay bar width & height
const SWord REPLAY_BAR_HEIGHT=64;	//in pixels @ 640x480

const Colour REPLAY_BAR_COLOR=Colour(4);

//COORDS2D infoLine1[]=
//{
//	{0,0},{639,0},{639,INFO_LINE_HEIGHT-1},{0,INFO_LINE_HEIGHT-1}
//};

//COORDS2D infoLine2[]=
//{
//	{0,0},{639,0},{639,2*INFO_LINE_HEIGHT-1},{0,2*INFO_LINE_HEIGHT-1}
//};

//COORDS2D infoLine3[]=
//{
//	{0,0},{639,0},{639,3*INFO_LINE_HEIGHT-1},{0,3*INFO_LINE_HEIGHT-1}
//};

//COORDS2D replayBar[]=
//{
//	{0,0},{REPLAY_BAR_WIDTH-1,0},
//	{REPLAY_BAR_WIDTH-1,REPLAY_BAR_HEIGHT-1},{0,REPLAY_BAR_HEIGHT-1}
//};

const SWord ICON_WIDTH=32;
const SWord ICON_HEIGHT=32;
const SWord ICON_SPACING=ICON_WIDTH+2;

ICON replayPanel=
{	{ICON_SPACING*11,0,ICON_WIDTH*4,ICON_HEIGHT},
	{RPLYICNSNO,{0,ICON_HEIGHT*3,(4*ICON_WIDTH)-1,ICON_HEIGHT-1}},
	{RPLYICNSNO,{0,ICON_HEIGHT*3,(4*ICON_WIDTH)-1,ICON_HEIGHT-1}}
};

ICON replayIcon[]=
{
	//Mark start
	{	{0,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{0,0,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{0,0,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//goto start
	{	{ICON_SPACING,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{ICON_WIDTH,0,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{ICON_WIDTH,0,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//rewind
	{	{ICON_SPACING*2,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{2*ICON_WIDTH,0,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{2*ICON_WIDTH,0,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//pause
	{	{ICON_SPACING*3,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{3*ICON_WIDTH,0,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{3*ICON_WIDTH,0,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//fast fwd
	{	{ICON_SPACING*4,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{ICON_WIDTH,ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{ICON_WIDTH,ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//goto end
	{	{ICON_SPACING*5,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{2*ICON_WIDTH,2*ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{2*ICON_WIDTH,2*ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//mark end
	{	{ICON_SPACING*6,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{2*ICON_WIDTH,ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{ICON_WIDTH,2*ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//load
	{	{ICON_SPACING*7,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{3*ICON_WIDTH,ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{3*ICON_WIDTH,ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//save
	{	{ICON_SPACING*8,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{0,2*ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{0,2*ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}}
	},
	//eject
	{	{ICON_SPACING*9,0,ICON_WIDTH,ICON_HEIGHT},
		{RPLYICNSNO,{ICON_WIDTH,2*ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}},
		{RPLYICNSNO,{ICON_WIDTH,2*ICON_HEIGHT,ICON_WIDTH-1,ICON_HEIGHT-1}}
	}
};

bool fMouseDisabled;
int active_lines;

const SWord POINTER_WIDTH=32;
const SWord POINTER_HEIGHT=32;


//const SWord INFO_POINT_COUNT=sizeof(infoLine1)/sizeof(COORDS2D);
//const SWord REPLAY_POINT_COUNT=sizeof(replayBar)/sizeof(COORDS2D);
//const SWord REPLAY_ICON_COUNT=sizeof(replayIcon)/sizeof(ICON);

struct TEXTDEF
{
 	COORDS2D pos;
	char* text;
};

//const SWord TOPLINE_Y=0;
SWord TOPLINE_Y=0;
SWord TOPLINE_Y2=0;
SWord TOPLINE_Y3=0;
SWord TOPLINE_YY=0;
const SWord ALTITUDE_X	=  0;
const SWord HDG_X		= 1;
const SWord SPEED_X		= 2;
const SWord THRUST_X	= 3;
const SWord AMMO_X		= 4;
const SWord MACH_X		= 5;

typedef SWord	LangXOffsets[6];

enum	LangNames
{
	LI_ENGLISH	=	0,
	LI_FRENCH,
	LI_GERMAN,
	LI_SPANISH,
	LI_ITALIAN,
	LI_RUSSIAN,
	LI_JAPANESE,
	LI_MAX
};

static	char*	lang_name[] =
{
	"english",
	"french",
	"german",
	"spanish",
	"italian",
	"russian",
	"japanese"
};

int	g_languageIndex = LI_ENGLISH;									//RJS 7Dec00

static	LangXOffsets	x_offsets[] =
{
	{10,110,190,310,410,0},			//english
	{10,110,190,310,410,0},			//french
	{10,110,190,310,420,0},			//german
	{4,130,214,340,415,0},			//spanish
	{10,120,210,350,450,0},			//italian
	{10,110,190,310,410,0},			//russian
	{10,110,190,310,410,0}			//japanese
//wid10,100,80,120,100
//hex0A,64, 50,78,64

};

const SWord CHAR_WH=16;
const SWord CHAR_SPACING=8;

const SWord MESSAGE_X=0;
//const SWord MESSAGE_Y=INFO_LINE_HEIGHT;

const SWord VIEWEE_X=0;
const SWord VIEWTYPE_X=212-20;
//const SWord VIEWLINE_Y=2*INFO_LINE_HEIGHT;

const SWord VIEWEETXT_X=56;
const SWord VIEWTYPETXT_X=VIEWTYPE_X+40;

const SWord TRGRNGTXT_X=VIEWTYPETXT_X+60;
const SWord TRGBRGTXT_X=TRGRNGTXT_X+100;
const SWord TRGALTTXT_X=TRGBRGTXT_X+100;


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		FindLanguage
//Author		Robert Slater
//Date			Thu 7 Dec 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	COverlay::FindLanguage()
{
	char*	resourcelanguagewidths=LdStr(IDS_INFOPANELWIDTHS);
	if (resourcelanguagewidths && resourcelanguagewidths[0])
	{
		g_languageIndex=0;
		int curroffset=0;
		for (int i=0;i<MACH_X;i++)
		{
			curroffset+=(UByte)resourcelanguagewidths[i];
			x_offsets[0][i]=curroffset;
		}
	}
	else
	{
		char*	theName = File_Man.namenumberedfile(FIL_RCHAT_SCRIPT_TABLE);
		if (theName)
		{
			char	tmpname[256];
			int		stringlen = strlen(theName);

			if (stringlen > 18)
			{
				char*	langname = tmpname;

				strcpy(tmpname,theName);

				tmpname[stringlen-18] = 0;
				{
				for (int i=stringlen-19; i > 0; i--)
				{
					if (	(tmpname[i] == '\\')
						||	(tmpname[i] == ':')	)
					{
						langname = &tmpname[i+1];
						break;
					}
				}
				}
				for (int i=0; i < LI_MAX; i++)
				{
					if (strcasecmp(lang_name[i],langname)==0)
					{
						g_languageIndex = i;
						break;
					}
				}
			}
		}
	}
	delete [] resourcelanguagewidths;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DrawWholeScreen
//Author		Paul
//Date			Thu 4 Jun 1998
//------------------------------------------------------------------------------
void COverlay::DrawWholeScreen()
{

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		GetSurfaceDimensions
//Author		Paul.
//
//------------------------------------------------------------------------------
void COverlay::GetSurfaceDimensions(SLong& w,SLong& h)
{
	ROWANSURFACEDESC rsd;
	rsd.dwSize=sizeof(rsd);
	g_lpLib3d->GetSurfaceDesc(&rsd);
	w=SLong(rsd.dwWidth);
	h=SLong(rsd.dwHeight);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DrawInfoBar
//Author		Paul
//Date			Thu 4 Jun 1998
//------------------------------------------------------------------------------
void COverlay::DrawInfoBar()
{
	active_lines=0;

	if (pCurScr==&mapViewScr)
		return;

	if (Key_Tests.KeyPress3d(HUDTOGGLE))
	{
		if (Save_Data.gamedifficulty[GD_HUDINSTACTIVE])
			Save_Data.gamedifficulty%=GD_HUDINSTACTIVE;
		else
			Save_Data.gamedifficulty|=GD_HUDINSTACTIVE;
	}

	g_lpLib3d->PushMatrix(MATRIX_OBJECT);
	g_lpLib3d->PushMatrix(MATRIX_VIEWER);
	g_lpLib3d->PushMatrix(MATRIX_PROJECTION);

	g_lpLib3d->LoadIdentity(MATRIX_OBJECT);
	g_lpLib3d->LoadIdentity(MATRIX_VIEWER);
	g_lpLib3d->LoadIdentity(MATRIX_PROJECTION);

	g_lpLib3d->GiveHint(HINT_2DRENDER);

	if (Manual_Pilot.ControlledAC2)
	{
		if (Save_Data.gamedifficulty[GD_HUDINSTACTIVE])
		{
			DoThreat();
			DoArtHoriz();
		}
		DoCheatBox();
	}

	CrossHair();

	if (!(Save_Data.infoLineCount==0 && messageTimer==0))
	{
		messageTimer-=pvp->RealFrameTime();
		if (messageTimer<0)
			messageTimer=0;

		COORDS2D* pointList;
		Save_Data.infoLineCount%=INFOLINESMAX;


		SLong box_height;

		GetSurfaceDimensions(physicalWidth,physicalHeight);


		if (!fIsWide)	box_height=10;
		else			box_height=18;

		longDistUnitStr=LdStr(Save_Data.dist.longabbr);
		shortDistUnitStr=LdStr(Save_Data.dist.mediumabbr);
		heightUnitStr=LdStr(Save_Data.alt.mediumabbr);
		speedUnitStr=LdStr(Save_Data.speed.perhrabbr);

		PreFormatViewText();

		if (TestMessageText())
		{
			SWord local_box_height=0;

			if (Save_Data.infoLineCount==1)
			{
				local_box_height+=box_height;
			}
			else if (Save_Data.infoLineCount==2)
			{
				if (fTwoLines)
					local_box_height+=box_height<<1;
				else
					local_box_height+=box_height;
			}

			if (messageTimer && (Save_Data.gamedifficulty[GD_DISPLAYMESSAGES] || !otherText->IsEmpty()))
			{
				local_box_height+=box_height;
				if (otherText->IsEmpty() && !msgText[1].IsEmpty())	local_box_height+=box_height;
			}
//TempCode JON 14Jul00 			static ULong zDepth = 1;
//TempCode JON 14Jul00 			static ULong bCol = INFO_BAR_COL;
//TempCode JON 14Jul00 			static UByte bAlpha = INFO_BAR_ALPHA;
//TempCode JON 14Jul00 			static ULong tCol = TEXT_MAP_WHITE;
//TempCode JON 14Jul00 			g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, zDepth );
			g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, 1 );
			SmokedGlassBox(0,physicalHeight-local_box_height,physicalWidth,local_box_height,INFO_BAR_COL,INFO_BAR_ALPHA);
//TempCode JON 14Jul00 			SmokedGlassBox(0,physicalHeight-local_box_height,physicalWidth+1,local_box_height+1,bCol,bAlpha);
			g_lpLib3d->SetFontColour(TEXT_MAP_WHITE);
//TempCode JON 14Jul00 			g_lpLib3d->SetFontColour(tCol);
		}

//TempCode JON 14Jul00 		static ULong zDepth1 = 0;
//TempCode JON 14Jul00 		g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, zDepth1 );
		g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, ULong(0) );

		TOPLINE_Y=physicalHeight-box_height+1;
		TOPLINE_Y2=TOPLINE_Y-box_height;
		TOPLINE_Y3=TOPLINE_Y2-box_height;

		if (Save_Data.infoLineCount==2 && fTwoLines)
		{
			TOPLINE_YY=TOPLINE_Y;
			TOPLINE_Y-=box_height;
			TOPLINE_Y2-=box_height;
			TOPLINE_Y3-=box_height;
		}
		active_lines=Save_Data.infoLineCount!=0?1:0;

		if (messageTimer && (Save_Data.gamedifficulty[GD_DISPLAYMESSAGES] || !otherText->IsEmpty()))
		{
			if (!active_lines)
			{
				TOPLINE_Y3=TOPLINE_Y2;
				TOPLINE_Y2=TOPLINE_Y;
			}
			DrawMessageText();
			active_lines+=msgText[1].IsEmpty()?1:2;
		}

		switch (Save_Data.infoLineCount)
		{
			case 2:
			DrawViewText();
			if (fTwoLines) active_lines++;
			break;
			case 1:
			DrawTopText();
			break;
		}
		delete[]longDistUnitStr;
		delete[]shortDistUnitStr;
		delete[]heightUnitStr;
		delete[]speedUnitStr;
		active_lines*=box_height;
	}
	g_lpLib3d->GiveHint(HINT_3DRENDER);
	g_lpLib3d->PopMatrix(MATRIX_OBJECT);
	g_lpLib3d->PopMatrix(MATRIX_VIEWER);
	g_lpLib3d->PopMatrix(MATRIX_PROJECTION);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DrawTopText
//Author		Paul
//Date			Thu 4 Jun 1998
//------------------------------------------------------------------------------
void COverlay::DrawTopText()
{
	//flight model code sets speed=>mph & height=>ft

	SLong speed2,altitude2; float speedey;

	altitude2=(altitude*305)/Save_Data.alt.mediummm;
	speedey = float(speed) * Save_Data.speed.mmpcs2perhr;	//CSB 23/06/99
	speed2=SLong(speedey);

//DEAD	if (!fIsWide)	doubleSpace>>=1;
	char temp[128];
	SWord xpos=0;

#ifndef	NDEBUG
	if (Three_Dee.shapecheat || diagnosticstring[0])
	{
		if (!diagnosticstring[0])
			PrintAt2(xpos,TOPLINE_Y,shpdebug);
		else
			PrintAt2(xpos,TOPLINE_Y,diagnosticstring);
	}
	else
	{
#endif
#ifndef	_XYCHEAT_

	sprintf(temp, "%s %5d%s", OAltitudeText.text, altitude2, heightUnitStr);
	xpos = x_offsets[g_languageIndex][ALTITUDE_X];						//RJS 7Dec00
	PrintAt2(xpos, TOPLINE_Y, temp);

	sprintf(temp, "%s %3d", OHdgText.text, heading);
	xpos = x_offsets[g_languageIndex][HDG_X];						//RJS 7Dec00;
	PrintAt2(xpos, TOPLINE_Y, temp);

	sprintf(temp, "%s %3d%s", OSpeedText.text, speed2, speedUnitStr);
	xpos = x_offsets[g_languageIndex][SPEED_X];						//RJS 7Dec00;
	PrintAt2(xpos, TOPLINE_Y, temp);

	sprintf(temp, "%s %3d", OThrustText.text, thrust);
	xpos = x_offsets[g_languageIndex][THRUST_X];						//RJS 7Dec00;
	PrintAt2(xpos, TOPLINE_Y, temp);

	sprintf(temp, "%s %4d", OAmmoText.text, ammocount);
	xpos = x_offsets[g_languageIndex][AMMO_X];						//RJS 7Dec00;
	PrintAt2(xpos, TOPLINE_Y, temp);

#else
	PrintAt2(xpos,TOPLINE_Y,hitlinestr);
#endif
#ifndef	NDEBUG
	}
#endif

}

inline bool COverlay::TestMessageText()
{
	if (!otherText->IsEmpty()) return true;
	if (Save_Data.gamedifficulty[GD_DISPLAYMESSAGES])
	{
		if (messageTimer!=0 && !msgText[0].IsEmpty()) return true;
	}
 	if (Save_Data.infoLineCount!=0) return true;
 	return false;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DrawMessageText
//Author		Paul
//Date			Thu 4 Jun 1998
//------------------------------------------------------------------------------
void COverlay::DrawMessageText()
{
	SWord xpos=MESSAGE_X;
	if (!otherText->IsEmpty())
	{
		PrintAt2(xpos,TOPLINE_Y2,const_cast<char*>(LPCTSTR(otherText[0])));
	}
	else if (!_MsgBuffer.nothingatall)
	{
		if (!msgText[1].IsEmpty())
		{
			PrintAt2(xpos,TOPLINE_Y3,const_cast<char*>(LPCTSTR(msgText[0])));
			xpos=MESSAGE_X;
			PrintAt2(xpos,TOPLINE_Y2,const_cast<char*>(LPCTSTR(msgText[1])));
		}
		else
			PrintAt2(xpos,TOPLINE_Y2,const_cast<char*>(LPCTSTR(msgText[0])));
	}
	else if (messageTimer!=0) messageTimer=0;
}

void COverlay::PreFormatViewText()
{
	GetSurfaceDimensions(physicalWidth,physicalHeight);

	SLong	vieweeLen=0,
			viewTypeLen=0,
			i,j,
			totalLen;

	fTwoLines=false;

	if (Save_Data.infoLineCount!=2)
		return;

	const	SLong bigSpaceLen=StrPixelWidthPadded("      ")>>1;
	const	SLong smallSpaceLen=bigSpaceLen>>1;
	const	SLong vieweeTextLen=StrPixelWidthPadded(OViewee.text);
	const	SLong viewTypeTextLen=StrPixelWidthPadded(OViewType.text);
	const	SLong rangeTextLen=StrPixelWidthPadded(ORangeText.text);
	const	SLong relBrgTextLen=StrPixelWidthPadded(ORelBrgText.text);
	const	SLong relAltTextLen=StrPixelWidthPadded(ORelAltText.text);
	const	SLong rangeUnitsLen=(i=StrPixelWidthPadded(shortDistUnitStr))>
								(j=StrPixelWidthPadded(longDistUnitStr))?
								i:j;
	const	SLong relBrgUnitsLen=0;
	const	SLong relAltUnitsLen=StrPixelWidthPadded(heightUnitStr);
	const	SLong rangeDigitLen=StrPixelWidthPadded("00000000");
	const	SLong relBrgDigitLen=StrPixelWidthPadded("0000");
	const	SLong relAltDigitLen=StrPixelWidthPadded("00000000");

	//find the longest viewer & viewee strings

	for (i=VIEWEE_LIST_SIZE-1;i>=0;i--)
		if ((j=StrPixelWidthPadded(listViewee[i]->text))>vieweeLen)
			vieweeLen=j;

	for (i=VIEWTYPE_LIST_SIZE-1;i>=0;i--)
		if ((j=StrPixelWidthPadded(listViewType[i]->text))>viewTypeLen)
			viewTypeLen=j;


	totalLen =	vieweeTextLen+
				smallSpaceLen+
				vieweeLen+
				bigSpaceLen+
				viewTypeTextLen+
				smallSpaceLen+
				viewTypeLen;

	if (pvp->trackeditem2!=NULL && targRange!=0)
	{
		totalLen+=	bigSpaceLen+
					rangeTextLen+
					smallSpaceLen+
					rangeDigitLen+
					rangeUnitsLen+
					bigSpaceLen+
					relAltTextLen+
					smallSpaceLen+
					relAltDigitLen+
					relAltUnitsLen+
					bigSpaceLen+
					relBrgTextLen+
					smallSpaceLen+
					relBrgDigitLen+
					relBrgUnitsLen;

		if (totalLen>physicalWidth)
			fTwoLines=true;
	}
	viewee_x=0;

	viewtype_x=vieweeTextLen+smallSpaceLen+vieweeLen+bigSpaceLen;

	viewrange_x=fTwoLines?0:viewtype_x+viewTypeTextLen+smallSpaceLen+viewTypeLen+bigSpaceLen;

	viewalt_x=viewrange_x+rangeTextLen+smallSpaceLen+rangeDigitLen+rangeUnitsLen+bigSpaceLen;

	viewbrg_x=viewalt_x+relAltTextLen+smallSpaceLen+relAltDigitLen+relAltUnitsLen+bigSpaceLen;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DrawViewText
//Author		Paul.
//Date			Fri 5 Jun 1998
//------------------------------------------------------------------------------
void COverlay::DrawViewText()
{
	char str[256];
	SWord pos_y=TOPLINE_Y;

	sprintf(str,"%s %s",OViewee.text,listViewee[pvp->viewnum.viewtarg]->text);
	PrintAt2(viewee_x,pos_y,str);

	sprintf(str,"%s %s",OViewType.text,listViewType[pvp->viewnum.viewmode]->text);
	PrintAt2(viewtype_x,pos_y,str);

	if (pvp->trackeditem2!=NULL && targRange!=0)
	{
		if (fTwoLines) pos_y=TOPLINE_YY;

		//display target range,rel bearing & rel alt
		SLong trange,talt,tbrg;
		char *distUnits=longDistUnitStr;

		//try converting range to large units (km or miles)
		trange=targRange/Save_Data.dist.longcm;

		if (trange<2)
		{
			//try meters or yards instead
			distUnits=shortDistUnitStr;
			trange=(targRange*10)/Save_Data.dist.mediummm;
			sprintf(str,"%s: %d%s",ORangeText.text,trange,distUnits);
		}
		else if (trange>=100)
		{
			sprintf(str,"%s: %d%s",ORangeText.text,trange,distUnits);
		}
		else
		{
			trange=(targRange*10)/Save_Data.dist.longcm;
			sprintf(str,"%s: %.1f%s",ORangeText.text,float(trange)/10,distUnits);
		}
		PrintAt2(viewrange_x,pos_y,str);

		talt=(targRelAlt*10)/Save_Data.alt.mediummm;
		tbrg=((SLong(targRelBrg)&0xFFFF)*360)>>16;

		sprintf(str,"%s: %03d",ORelBrgText.text,tbrg);
		PrintAt2(viewbrg_x,pos_y,str);

		sprintf(str,"%s: %d%s",ORelAltText.text,talt,heightUnitStr);
		PrintAt2(viewalt_x,pos_y,str);
	}
}



//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		RenderIcon
//Author		Paul
//Date			Thu 4 Jun 1998
//------------------------------------------------------------------------------

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		ReplayIconHitTest
//Author		Paul
//Date			Thu 4 Jun 1998
//------------------------------------------------------------------------------
//SWord COverlay::ReplayIconHitTest(SWord x,SWord y)
//{
//	ICON* iconList=replayIcon;
//	for (int i=0;i<REPLAY_ICON_COUNT;iconList++,i++)
//	{
//		if (x>=iconList->hitBox.x &&
//			x<iconList->hitBox.x+iconList->hitBox.w &&
//			y>=iconList->hitBox.y &&
//			y<iconList->hitBox.y+iconList->hitBox.h)
//			return SWord(i);
//	}
//	return SWord(-1);
//}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PrintAt
//Author		Paul
//Date			Thu 4 Jun 1998
//------------------------------------------------------------------------------
void COverlay::PrintAt(SWord& x,SWord& y,const char* str, const ULong& theCol)
{
	if (nameTableCnt < MaxNames)
	{
		NameTable[nameTableCnt].x = x;
		NameTable[nameTableCnt].y = y;
		NameTable[nameTableCnt].colour = theCol;

		strcpy(NameTable[nameTableCnt++].text,str);
	}
}

void COverlay::PrintAt2(SWord& x,SWord& y,const char* str, bool noWrap)
{
	LIGHTFLAG lf = LF_FONT;
	g_lpLib3d->SetObjectLighting( lf );
	if (str)
	{
		char c;
		if (!noWrap)
		{
			while (*str!=0 && x<physicalWidth) PutC(x,y,NextOverlayChar(str));
		}
		else
		{
			while (*str!=0 && x<physicalWidth) PutC2(x,y,NextOverlayChar(str));
		}
	}
	if ( lf != LF_FONT )
		g_lpLib3d->SetObjectLighting( lf );
}


static UByte bigWidths[]=
{8,	2,	5,	8,	5,	8,	9,	2,	//punc
2,	2,	7,	7,	2,	7,	2,	10,
6,	3,	7,	6,	8,	6,	6,	7,	//digits
6,	6,	2,	2,	5,	6,	5,	6,

10,	10,	7,	7,	8,	7,	6,	8,	//uppercase
7,	2,	4,	7,	7,	9,	8,	8,
7,	8,	8,	7,	8,	8,	9,	11,
11,	7,	10,	3,	10,	3,	7,	7,

2,	7,	7,	6,	7,	6,	6,	7,	//lowercase
7,	2,	2,	7,	2,	12,	7,	7,
7,	9,	5,	6,	4,	7,	7,	12,
8,	7,	7,	5,	2,	5,	3,	5,

1,	1,	1,	1,	1,	1,	1,	1,	//generally unused extras
1,	1,	1,	1,	1,	1,	1,	1,
1,	1,	1,	1,	1,	1,	1,	1,
1,	1,	1,	1,	1,	1,	1,	1,
1,	2,	1,	1,	1,	1,	1,	1,

1,	9,	1,	1,	1,	1,	1,	1,	//various extras
1,	1,	1,	1,	1,	1,	1,	1,
1,	1,	1,	1,	1,	1,	1,	6,
10,	10,	11,	12,	10,	10,	11,	7,
6,	6,	6,	6,	3,	3,	3,	3,

9,	8,	8,	8,	8,	8,	8,	6,	//uppercase extras
8,	8,	8,	8,	8,	7,	7,	7,
7,	7,	7,	9,	7,	7,	11,	6,
6,	6,	6,	6,	3,	2,	3,	3,
7,	7,	7,	7,	7,	7,	7,	6,

7,	7,	7,	7,	7,	7,	7,	7,	//lowercase extras
1,	1,	1,	1,	1,	1,	1,	1,
1,	1,	1,	1,	1,	1,	1,	1,
1,	1,	1,	1,	1,	1,	1,	1,
1,	1,	1,	1,	1,	1,	1,	1};



//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		StrPixelLen
//Author		Paul.
//Date			Thu 17 Dec 1998
//------------------------------------------------------------------------------

void COverlay::ChopStr(CString& in,CString& out)
{
	out="";
	GetSurfaceDimensions(physicalWidth,physicalHeight);
	in.TrimRight();
	in.TrimLeft();

	while (StrPixelWidth(const_cast<char*>(LPCTSTR(in)))>=physicalWidth)
	{
		int l=in.GetLength();
		int i=in.ReverseFind(' ');
		if (i>=0)
		{
			out=in.Right(l-i)+out;
			in=in.Left(i);
			in.TrimRight();
		}
		else break;
	}
	if (!out.IsEmpty()) out.TrimLeft();
}

void COverlay::MakeDotDotDot(CString& in)
{
	GetSurfaceDimensions(physicalWidth,physicalHeight);
	in.TrimLeft();
	in.TrimRight();
	if (StrPixelWidth(const_cast<char*>(LPCTSTR(in)))>=physicalWidth)
	{
		CString temp;
		do
		{
			in=in.Left(in.GetLength()-1);
			in.TrimRight();
			temp=in+"...";
		}
		while (StrPixelWidth(const_cast<char*>(LPCTSTR(temp)))>=physicalWidth);
		in=temp;
	}
}

SWord COverlay::CalcTargIndex(CString& in)
{
	GetSurfaceDimensions(physicalWidth,physicalHeight);
	in.TrimLeft();
	in.TrimRight();

	if (StrPixelWidth(const_cast<char*>(LPCTSTR(in)))<physicalWidth)
		return 0;
    SLong i;
	for (i=0;i<in.GetLength();i++)
	{
		CString temp=in.Right(in.GetLength()-i);
		if (StrPixelWidth(const_cast<char*>(LPCTSTR(temp)))<physicalWidth)
		{
			if (i+3<in.GetLength()) i+=3;
			break;
		}
	}
	return i<in.GetLength()?i:0;
	return 0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PutC
//Author		Paul
//Date			Thu 4 Jun 1998
//------------------------------------------------------------------------------
void COverlay::PutC(SWord& x,SWord& y,unsigned char c)
{
	//lo res version only displays upper case chars

	UByte tWidth=8;
	SWord this_y;

	if (!fIsWide && c>='a' && c<='z') c=c-'a'+'A';
	else tWidth=bigWidths[c-32]+1/*+extra*/;

 	c-=32;		//1st 32 chars not included

 	SWord ou,ov;
 	SWord cWidth,pWidth,cHeight,pHeight;
 	ImageMapNumber fontNo;

	ImageMapDesc* pmap;

	this_y=y;

	if (!fIsWide)
	{
 		ou=(SWord(c)&0x0F)<<3;
		ou++;
 		ov=(SWord(c)&0xF0)>>1;
 		cWidth=7/*+extra*/;		//8;
 		cHeight=8/*+extra*/;
 		pWidth=cWidth;
		pHeight=cHeight;
		fontNo=FONTNO;
		bool patchit=Image_Map.GetImageMapPtrDontLoad(fontNo)==NULL;
		pmap=Image_Map.GetImageMapPtr(fontNo);
		if (patchit)	PatchFontTable(pmap);
	}
	else
	{
 		ou=(SWord(c)&0x0F)<<4;
		ou++;
 		ov=(SWord(c)&0xF0);
 		cWidth=tWidth;
 		cHeight=17/*+extra*/;	//16;
 		pWidth=cWidth;
		pHeight=cHeight;
		fontNo=FONTNO;
		fNot16bit=true;
		bool patchit=Image_Map.GetImageMapPtrDontLoad(fontNo)==NULL;
		pmap=Image_Map.GetImageMapPtr(fontNo);
		if (patchit)	PatchFontTable(pmap);
		fNot16bit=false;
	}

	R3DVALUE sx,sy;
	SWord hx,hy;
	hx=physicalWidth>>1;
	hy=physicalHeight>>1;
	sx=1.f/R3DVALUE(hx);
	sy=-1.f/R3DVALUE(hy);

	Lib3DPoint *dp = g_lpLib3d->BeginPoly(HMATERIAL(pmap),4);
//TEMPCODE JON 6/1/00 	DoPointStruc *dp = SHAPE.newco;

	dp[0].setPosition(
		R3DVALUE(x-hx)*sx,
		R3DVALUE(this_y-hy)*sy,
		0.f
	);

	dp[1].setPosition(
		R3DVALUE((x+pWidth-1)-hx)*sx,
//		R3DVALUE(this_y-hy)*sy,
		dp[0].getPosY(),
		0.f
	);

	dp[2].setPosition(
//		R3DVALUE((x+pWidth-1)-hx)*sx,
		dp[1].getPosX(),
		R3DVALUE((this_y+pHeight-1)-hy)*sy,
		0.f
	);

	dp[3].setPosition(
//		R3DVALUE(x-hx)*sx,
		dp[0].getPosX(),
//		R3DVALUE((this_y+pHeight-1)-hy)*sy,
		dp[2].getPosY(),
		0.f
	);

	dp[0].setIMapCoords(
		ou,
		ov
	);

	dp[1].setIMapCoords(
		ou+cWidth-1,
		ov
	);

	dp[2].setIMapCoords(
		ou+cWidth-1,
		ov+cHeight-1
	);

	dp[3].setIMapCoords(
		ou,
		ov+cHeight-1
	);

//TEMPCODE JON 6/1/00 	Lib3DPoint *dp1 = g_lpLib3d->BeginPoly(HMATERIAL(pmap),4);
//TEMPCODE JON 6/1/00 	*dp1++=*dp++;
//TEMPCODE JON 6/1/00 	*dp1++=*dp++;
//TEMPCODE JON 6/1/00 	*dp1++=*dp++;
//TEMPCODE JON 6/1/00 	*dp1=*dp;
	g_lpLib3d->EndPoly();


 	x+=pWidth;

 	if (x>=physicalWidth) x=0,y+=pHeight;
 	if (y>=physicalHeight) y=0;

	if (doneonce==0)
	{
		doneonce=1;
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PutC2
//Author		Robert Slater
//Date			Tue 22 Aug 2000
//
//Description	with letter clip!
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
//	The 3D font atlas holds a single-byte codepage of glyphs (cells for
//	chars 32..255).  Under a Cyrillic ROWAN_LANG the resource strings are
//	UTF-8, so decode one UTF-8 sequence per step and map Cyrillic
//	codepoints onto the cp1251 cells that PatchFontTableCyrillic()
//	rasterizes into the atlas at font-load time.
static bool OverlayFontIsUTF8()
{
	static const char* lang = RowanLang();
	static const bool is = lang &&
		(strstr(lang,"ru") || strstr(lang,"uk") || strstr(lang,"be") || strstr(lang,"bg"));
	return is;
}

static unsigned char NextOverlayChar(const char*& p)
{
	unsigned char c = (unsigned char)*p++;
	if (!OverlayFontIsUTF8() || c < 0x80)
		return c;
	if ((c & 0xE0) == 0xC0 && (*p & 0xC0) == 0x80)
	{
		unsigned cp = ((c & 0x1F) << 6) | (((unsigned char)*p++) & 0x3F);
		if (cp >= 0x410 && cp <= 0x44F)	return (unsigned char)(cp - 0x350);	//cp1251 C0..FF
		if (cp == 0x401)	return 0xA8;	//IO
		if (cp == 0x451)	return 0xB8;	//io
		return '?';
	}
	if (c >= 0xE0)					//3/4-byte heads: skip continuations
	{
		int cont = (c >= 0xF0) ? 3 : 2;
		while (cont-- > 0 && (*p & 0xC0) == 0x80) p++;
	}
	return '?';
}

//	Rasterize the Cyrillic half of the 8-bit font atlas (cp1251 cells
//	A8, B8, C0-FF) via SDL_ttf.  The shipped FONT.X8 only covers Latin;
//	this replaces the Win32 GetGlyphOutline path that the retail
//	x0r-disabled PatchFontTable body used.
static void PatchFontTableCyrillic(ImageMapDesc* imap)
{
	if (!imap || !imap->alpha || !OverlayFontIsUTF8())
		return;

	const int wh = imap->w;
	if (wh != 128 && wh != 256)
		return;
	const int cell = (wh == 128) ? 8 : 16;
	const int ouShift = (wh == 128) ? 3 : 4;	//ou = (c&0xF)<<ouShift
	const int ovShift = (wh == 128) ? 1 : 0;	//ov = (c&0xF0)>>ovShift

	TTF_Font* font = NULL;
	static const char* paths[] = {
		"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
		"/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
		"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
		"./INTEL.TTF",
		"./Intel.ttf",
	};
	for (int i = 0; i < 5 && !font; i++)
		font = TTF_OpenFont(paths[i], (wh == 128) ? 10 : 20);
	if (!font)
		return;

	//match the atlas' own glyph intensity (nibble- or byte-scale)
	int maxv = 0;
	{
		const int ac = 'A' - 32;
		const int ax = (ac & 0xF) << ouShift, ay = (ac & 0xF0) >> ovShift;
		for (int yy = ay; yy < ay + cell && yy < imap->h; yy++)
			for (int xx = ax; xx < ax + cell && xx < wh; xx++)
				if (imap->alpha[yy * wh + xx] > maxv)
					maxv = imap->alpha[yy * wh + xx];
	}
	if (maxv < 8)
		maxv = 15;

	const int baseline = (wh == 128) ? 5 : 11;
	const SDL_Color white = {255, 255, 255, 255};

	for (int cp1251 = 0xA8; cp1251 <= 0xFF; cp1251++)
	{
		unsigned cp;
		if (cp1251 == 0xA8)			cp = 0x401;
		else if (cp1251 == 0xB8)	cp = 0x451;
		else if (cp1251 >= 0xC0)	cp = cp1251 + 0x350;
		else						continue;	//A9-B7, B9-BF hold no Russian letters

		char u8[3] = {char(0xC0 | (cp >> 6)), char(0x80 | (cp & 0x3F)), 0};
		SDL_Surface* g = TTF_RenderUTF8_Blended(font, u8, white);
		if (!g)
			continue;

		int minx = 0, maxx = 0, miny = 0, maxy = 0, adv = 0;
		TTF_GlyphMetrics32(font, (Uint32)cp, &minx, &maxx, &miny, &maxy, &adv);

		const int cellX = (cp1251 & 0xF) << ouShift;
		const int cellY = (cp1251 & 0xF0) >> ovShift;
		const int top = baseline - maxy;	//maxy is above-baseline height

		for (int gy = 0; gy < g->h; gy++)
		{
			const int dy = top + gy;
			if (dy < 0 || dy >= cell || cellY + dy >= imap->h)
				continue;
			for (int gx = 0; gx < g->w; gx++)
			{
				const int dx = (minx > 0 ? 0 : -minx) + gx;	//keep left sidebearing
				if (dx < 0 || dx >= cell || cellX + dx >= wh)
					continue;
				const Uint32 px = ((const Uint32*)g->pixels)[gy * (g->pitch >> 2) + gx];
				const int a = (px >> 24) & 0xFF;
				imap->alpha[(cellY + dy) * wh + cellX + dx] = (UByte)((a * maxv + 127) / 255);
			}
		}
		if (adv > 0)
			bigWidths[cp1251 - 32] = (UByte)(adv < cell - 1 ? adv : cell - 1);
		SDL_FreeSurface(g);
	}
	TTF_CloseFont(font);
}

VOID	COverlay::PatchFontTable(ImageMapDesc* imap)
{/* x0r !!!
    	//This routine is ONLY for use on an 8-bit font to generate all 256 chracters

	if (COverlay::languagesetbyresources>2)
	{
		const int wh=imap->w;
		const int	pixperchar=(wh==128)	?8	:16	,
					MAX_PIXWIDE=(wh==128)	?6	:15	,
					MAX_LONGSWIDE=(wh==128)	?2	:4	,
					BASELINE=(wh==128)		?5	:11	,
					MAX_ROWS=(wh==128)		?7	:15	,
					ROW_JUMP=(wh==128)		?30	:60	,
					SCALE_HI=(wh==128)		?0	:0	,
					SCALE_LO=(wh==128)		?18000:36000;




		CFont* oldfont=g_OffScreenDC.SelectObject(::g_AllFonts[4][0]);
		for (int y=2,sy=1,cr=32;y<16;sy+=pixperchar,y++)
			for (int x=0,sx=1;x<16;x++,sx+=pixperchar,cr++)
			{
				int c=cr;
				if (wh==128 && ((c>='a' && c<='z') || (c>='à' && c<='ÿ')))
					c-=32;
				GLYPHMETRICS gm={0};
				MAT2		mat2={{SCALE_LO,SCALE_HI},{0,0},{0,0},{SCALE_LO,SCALE_HI}};
				enum	{MAX_BYTES_GLYPH=512};

				assert(GetGlyphOutline(	  g_OffScreenDC,             // handle to DC
										  c,          // character to query
										  GGO_GRAY8_BITMAP,//GGO_GRAY8_BITMAP,        // data format
										  &gm, // glyph metrics
										  0,      // size of data buffer
										  NULL,    // data buffer
										  &mat2   // transformation matrix
										) < MAX_BYTES_GLYPH);

				char buffer[MAX_BYTES_GLYPH];

				DWORD ggo2= GetGlyphOutline(
				  g_OffScreenDC,             // handle to DC
				  c,          // character to query
				  GGO_GRAY8_BITMAP,        // data format L
				  &gm, // glyph metrics
				  MAX_BYTES_GLYPH,      // size of data buffer
				  buffer,    // data buffer
				  &mat2   // transformation matrix
				);

				int longswide=(gm.gmBlackBoxX+3)>>2;
				ULong* src=(ULong*)buffer;
				ULong* trg=(ULong*)(imap->alpha+sx+sy*wh);
				if (cr=='1' && longswide==1)
					trg+=MAX_LONGSWIDE;
				int	srcskip=longswide-4;


				if (gm.gmBlackBoxX>MAX_PIXWIDE)
					longswide=MAX_LONGSWIDE;
				else
					srcskip=0;
				int starty=BASELINE-gm.gmptGlyphOrigin.y;
				if (starty<0)	starty=0;
				if (starty>MAX_ROWS)	starty=MAX_ROWS;
				int gmBlackBoxY=gm.gmBlackBoxY+starty;
				if (gmBlackBoxY>MAX_ROWS)
					gmBlackBoxY=MAX_ROWS;

				bigWidths[cr-32]= gm.gmCellIncX + 1;
				{//assume(longswide>=0 && longswide<=4);
					int y;
					for (y=0;y<starty;y++)
					{
						for (int i=0;i<MAX_LONGSWIDE;i++)
							*trg++=0;
						trg+=ROW_JUMP;
					}
					for (;y<gmBlackBoxY;y++)
					{
						for (int x=0;x<longswide;x++)
						{
							ULong v=*src++;
							ULong t=v&0x40404040;
							v-=t;
			//converting byte seperatyed flags to nibble.
							ULong u=t>>14;
							t+=u;
							u=t>>7;
							t+=u;
			//deleting crap
							t&=0x3C0;
			//putting nibble in right place
							t>>=6;
							v<<=2;
							v|=vsets[t];//hoping optimiser will reduce shift count.
							*trg++=v;
						}
						for (;x<MAX_LONGSWIDE;x++)
						{
							*trg++=0;
						}
						trg+=ROW_JUMP;
					}
					for (;y<=MAX_ROWS;y++)
					{
						for (int i=0;i<MAX_LONGSWIDE;i++)
							*trg++=0;
							trg+=ROW_JUMP;
					}
				}
			}
		g_OffScreenDC.SelectObject(oldfont);
	}
	int maxwid=bigWidths['-'-32];
	{for (int c='0'-32;c<='9'-32;c++)
		if (bigWidths[c]>maxwid)
			maxwid=bigWidths[c];
	}
	{for (int c='0'-32;c<='9'-32;c++)
		bigWidths[c]=maxwid;
	}
	bigWidths[' '-32]=maxwid;
	bigWidths['-'-32]=maxwid;
*/
	PatchFontTableCyrillic(imap);
}

void COverlay::PutC2(SWord& x,SWord& y,unsigned char c)
{
	//lo res version only displays upper case chars

	UByte tWidth=8;
	SWord this_y;

	if (!fIsWide && c>='a' && c<='z')
		c=c-'a'+'A';
	else
		tWidth=bigWidths[c-32]+1/*+extra*/;

 	c-=32;		//1st 32 chars not included

 	SWord ou,ov;
 	SWord cWidth,pWidth,cHeight,pHeight;
 	ImageMapNumber fontNo;

	ImageMapDesc* pmap;

	this_y=y;

	if (!fIsWide)
	{
 		ou=(SWord(c)&0x0F)<<3;
		ou++;
 		ov=(SWord(c)&0xF0)>>1;
 		cWidth=7./*+extra*/;		//8;
 		cHeight=8/*+extra*/;
 		pWidth=cWidth;
		pHeight=cHeight;
		fontNo=FONTNO;
		bool patchit=Image_Map.GetImageMapPtrDontLoad(fontNo)==NULL;
		pmap=Image_Map.GetImageMapPtr(fontNo);
		if (patchit)	PatchFontTable(pmap);
	}
	else
	{
 		ou=(SWord(c)&0x0F)<<4;
		ou++;
 		ov=(SWord(c)&0xF0);
 		cWidth=tWidth;
 		cHeight=17/*+extra*/;	//16;
 		pWidth=cWidth;
		pHeight=cHeight;
		fontNo=FONTNO;
		fNot16bit=true;
		bool patchit=Image_Map.GetImageMapPtrDontLoad(fontNo)==NULL;
		pmap=Image_Map.GetImageMapPtr(fontNo);
		if (patchit)	PatchFontTable(pmap);
		fNot16bit=false;
	}

	if (	(x > 0)
		&&	((x+pWidth) < physicalWidth)
		&&	(y > 0)
		&&	((y+pHeight) < physicalHeight)	)
	{
		R3DVALUE sx,sy;
		SWord hx,hy;
		hx=physicalWidth>>1;
		hy=physicalHeight>>1;
		sx=1.f/R3DVALUE(hx);
		sy=-1.f/R3DVALUE(hy);

		Lib3DPoint *dp = g_lpLib3d->BeginPoly(HMATERIAL(pmap),4);

		dp[0].setPosition(
			R3DVALUE(x-hx)*sx,
			R3DVALUE(this_y-hy)*sy,
			0.f
		);

		dp[1].setPosition(
			R3DVALUE((x+pWidth-1)-hx)*sx,
			dp[0].getPosY(),
			0.f
		);

		dp[2].setPosition(
			dp[1].getPosX(),
			R3DVALUE((this_y+pHeight-1)-hy)*sy,
			0.f
		);

		dp[3].setPosition(
			dp[0].getPosX(),
			dp[2].getPosY(),
			0.f
		);

		dp[0].setIMapCoords(
			ou,
			ov
		);

		dp[1].setIMapCoords(
			ou+cWidth-1,
			ov
		);

		dp[2].setIMapCoords(
			ou+cWidth-1,
			ov+cHeight-1
		);

		dp[3].setIMapCoords(
			ou,
			ov+cHeight-1
		);

		g_lpLib3d->EndPoly();

		if (!doneonce)
			doneonce=1;
	}

	x+=pWidth;
}


inline R3DVALUE scaledScreenX( const R3DVALUE x )
{
	return (-1.0)+( x*(1.0/320.0) );
}

inline R3DVALUE scaledScreenY( const R3DVALUE y )
{
	return (1.0)-( y*(1.0/200.0) );
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PrintIcon
//Author		Paul.
//Date			Wed 17 Feb 1999
//------------------------------------------------------------------------------
void COverlay::PrintIcon(SLong sx,SLong sy,SLong sw,SLong sh,UWord iconIndex)
{
 	SWord ou,ov;
 	SWord cWidth,cHeight;
	R3DVALUE scx,scy;
	SLong hx,hy;
	hx=physicalWidth>>1;
	hy=physicalHeight>>1;
	scx=1.f/R3DVALUE(hx);
	scy=-1.f/R3DVALUE(hy);

	ImageMapNumber iconNo=(ImageMapNumber)currIcons[iconIndex].imageMapNo;

	ImageMapDesc* pmap;

	ou=currIcons[iconIndex].x;
	ov=currIcons[iconIndex].y;
	cWidth=currIcons[iconIndex].w;
	cHeight=currIcons[iconIndex].h;
	pmap=Image_Map.GetImageMapPtr(iconNo);
	Lib3DPoint *dp = g_lpLib3d->BeginPoly(HMATERIAL(pmap),4);
	dp[0].setPosition(
		R3DVALUE(sx-hx)*scx,
		R3DVALUE(sy-hy)*scy,
		0.f
	);

	dp[1].setPosition(
		R3DVALUE((sx+sw)-hx)*scx,
		dp[0].getPosY(),
		0.f
	);


	dp[2].setPosition(
		dp[1].getPosX(),
		R3DVALUE((sy+sh)-hy)*scy,
		0.f
	);


	dp[3].setPosition(
		dp[0].getPosX(),
		dp[2].getPosY(),
		0.f
	);

	dp[0].setIMapCoords(
		ou,
		ov
	);

	dp[1].setIMapCoords(
		ou+cWidth,
		ov
	);

	dp[2].setIMapCoords(
		ou+cWidth,
		ov+cHeight
	);

	dp[3].setIMapCoords(
		ou,
		ov+cHeight
	);


	g_lpLib3d->EndPoly();
}

////////////////////////////////////////////////////////////////////////////////
//
//	CODE STARTS HERE FOR 2D SCREENS...
//
////////////////////////////////////////////////////////////////////////////////

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		RenderInfoPanel
//Author		Paul.
//Date			Fri 5 Jun 1998
//------------------------------------------------------------------------------
void COverlay::RenderInfoPanel()
{
	if (pvp==NULL) return;

	DrawInfoBar();

#ifndef	NDEBUG
	diagnosticstring[0]=0;
#endif
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		RenderMapPad
//Author		Paul.
//Date			Wed 18 Nov 1998
//------------------------------------------------------------------------------
void COverlay::RenderMapPad()
{
	if (pvp==NULL||Save_Data.infoLineCount==0) return;

	DrawMapPad();
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DrawMapPad
//Author		Paul.
//Date			Wed 18 Nov 1998
//------------------------------------------------------------------------------
void COverlay::DrawMapPad()
{
}

COORDS3D last;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DisplayProfileData
//Author		Paul.
//Date			Wed 18 Nov 1998
//------------------------------------------------------------------------------
void COverlay::DisplayProfileData(bool fDisplay)
{
/*	static char profileText[]="Profile Data";
	static char minText[]="Max Frames/sec: %-#6.2f";
	static char maxText[]="Min Frames/sec: %-#6.2f";
	static char aveText[]="Ave Frames/sec: %-#6.2f";
	static char curText[]="Cur Frames/sec: %-#6.2f";
	static char textRemadeText[]="Textures remade: %4d";

	const int max_frames=64;
	static int frames=0;
	static int curr_insert_pos=0;
	static int frec[max_frames];

	static char tempStr[128];

	int this_frame_time=pvp->RealFrameTime();
	frec[curr_insert_pos++]=this_frame_time;
	if (curr_insert_pos==max_frames) curr_insert_pos=0;
	if (frames<max_frames) frames++;

	//calculate min,max,average frame time over last 'max_frames' frames

	int min_frame,max_frame,ave_frame;

	min_frame=10000;
	max_frame=0;
	ave_frame=0;

	for (int i=0;i<frames;i++)
	{
		ave_frame+=frec[i];
		if (frec[i]<min_frame) min_frame=frec[i];
		if (frec[i]>max_frame) max_frame=frec[i];
	}

	float fp_ave=100./(float(ave_frame)/float(frames));
	float fp_min=100./float(min_frame);
	float fp_max=100./float(max_frame);
	float fp_cur=100./float(this_frame_time);

	if (curr_insert_pos==0) averageFrameRate=SLong(100.*fp_ave);

	if (fDisplay)
	{
		//prepare to display the data

		GetSurfaceDimensions(physicalWidth,physicalHeight);

		fIsWide=IsWide(physicalWidth);

		pw->DoSetGlobalAlpha(8);
		pw->DoSmokedGlassBox((physicalWidth>>2),(physicalHeight>>2),(physicalWidth>>1),(physicalHeight>>1),GLASS_COLOUR);
		pw->DoSetGlobalAlpha(255);

		int lineHeight=fIsWide?16:8;

		SWord cx=(physicalWidth>>2)+lineHeight;
		SWord cy=fIsWide?(physicalHeight>>3)+lineHeight:(physicalHeight>>2)+lineHeight;

		//PrintAt(cx,cy,profileText);
		cy+=lineHeight+lineHeight;
		sprintf(tempStr,minText,fp_min);
		//PrintAt(cx,cy,tempStr);
		cy+=lineHeight;
		sprintf(tempStr,maxText,fp_max);
		//PrintAt(cx,cy,tempStr);
		cy+=lineHeight;
		sprintf(tempStr,aveText,fp_ave);
		//PrintAt(cx,cy,tempStr);
		cy+=lineHeight;
		sprintf(tempStr,curText,fp_cur);
		//PrintAt(cx,cy,tempStr);

	}*/
}




//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		UpdateSelections
//Author		Paul.
//Date			Fri 20 Nov 1998
//------------------------------------------------------------------------------

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetViewVals
//Author		Paul
//Date			Mon 23 Nov 1998
//------------------------------------------------------------------------------
void COverlay::SetViewVals(SLong rng,SLong relB,SLong relA)
{
	targRange=rng;
	targRelBrg=relB;
	targRelAlt=relA;
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetInfoLines
//Author		Dave Whiteside
//Date			Mon 23 Nov 1998
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void COverlay::SetInfoLines(SWord l)
{
	Save_Data.infoLineCount=l%INFOLINESMAX;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		StepInfoBarUp
//Author		Dave Whiteside
//Date			Mon 23 Nov 1998
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void COverlay::StepInfoBarUp()
{
	Save_Data.infoLineCount=(Save_Data.infoLineCount+1)%INFOLINESMAX;
	if (messageTimer) messageTimer=0;		//remove current message when the i key is pressed
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetInfoLineVals
//Author		Paul
//Date			Mon 23 Nov 1998
//------------------------------------------------------------------------------
void COverlay::SetInfoLineVals(SLong s,SLong m,SLong a, SLong h,SLong t)
{
	speed=s;													//PD 04Jan99
	ammocount=m;
	altitude = a>0?a:0;
	heading=h;
	thrust=t;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		InitInfoTexts
//Author		Paul.
//Date			Wed 16 Dec 1998
//------------------------------------------------------------------------------
void COverlay::InitInfoTexts()
{
	for (SLong i=0;i<_NumInfoTexts;i++)
	{
		OString* str=OInfoPanelText[i];
		if ( str->text == NULL )
			str->text=LdStr(str->resID);
		if ( i==29 )
			NOP;
	}


	blankMsgText=new CString[2];
	blankMsgText[0]=blankMsgText[1]="";
	blankOtherText=new CString("");
	target_item=NULL;
	msgText=blankMsgText;
	otherText=blankOtherText;
}


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		ReleaseInfoTexts
//Author		Paul.
//Date			Wed 16 Dec 1998
//------------------------------------------------------------------------------
void COverlay::ReleaseInfoTexts()
{
	for (SLong i=0;i<_NumInfoTexts;i++)
	{
		OString* str=OInfoPanelText[i];
		if ( str->text != NULL )
			delete[]str->text,str->text=NULL;
	}
	delete[]blankMsgText;
	delete blankOtherText;
	blankMsgText=NULL;
	blankOtherText=NULL;
}

void COverlay::SetToContinueQuitScr()
{
	if (_DPlay.Implemented)
		return; // dont show this screen in multiplayer

	Kludge();
	if (!Manual_Pilot.ControlledAC2->Status.deadtime &&
		!pCurScr)
	{
		displayTimeLimit=0;
		CancelAccel();
		SetToUIScreen(&continueQuitScr);
	}
}


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetToMapViewScreen
//Author		Paul.
//Date			Thu 11 Mar 1999
//------------------------------------------------------------------------------
void COverlay::SetToMapViewScreen()
{
	Kludge();
	currIcons=mapViewIconTable;
 	SetToUIScreen(&mapViewScr);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetToOrdersScreen
//Author		Paul.
//Date			Mon 22 Feb 1999
//------------------------------------------------------------------------------
void COverlay::SetToOrdersScreen()
{
	Kludge();
	if (!Manual_Pilot.ControlledAC2->Status.deadtime &&
		!pCurScr)
	{
		displayTimeLimit=0;
		CancelAccel();
		SetToUIScreen(&orders3dScr);
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetToMapScreen
//Author		Paul.
//Date			Tue 16 Feb 1999
//------------------------------------------------------------------------------
void COverlay::SetToMapScreen()
{
	Kludge();
	if (Manual_Pilot.ControlledAC2->Status.deadtime)
		OverLay.pvp->ReturnFromMap();
	else
	{
		if ( Manual_Pilot.ControlledAC2 && Manual_Pilot.ControlledAC2->classtype->aerobaticfactor == AEROBATIC_LOW )
			SetToUIScreen(&firstMapScrBomber);
		else
			SetToUIScreen(&firstMapScr);
	}
}

void COverlay::SetToRadioScreen()
{
	Kludge();
	if (!Manual_Pilot.ControlledAC2->Status.deadtime)
	{
		if (_DPlay.Implemented)	SetToUIScreen(&userMsgScrCOMMS);
		else
		{																				//CSB 23/02/00

			bool toweronly = false;
			if(		((Persons2::PlayerSeenAC->movecode == AUTO_TRAININGTAKEOFF)	|| (Persons2::PlayerSeenAC->movecode == AUTO_TRAININGLANDING))
				||	(Art_Int.CountFormationSize(Persons2::PlayerGhostAC) == 1)	)
			toweronly = true;

			if(toweronly)
				SetToUIScreen(&userMsgScrTower);											//CSB 23/02/00
			else
			{
				if(Manual_Pilot.ControlledAC2->formpos == 0)								//CSB 23/02/00
					if(		(Manual_Pilot.ControlledAC2->classtype->visible == JU87)		//CSB 23/02/00
						||	(Manual_Pilot.ControlledAC2->classtype->visible == ME110)	)	//CSB 23/02/00
						SetToUIScreen(&userMsgScrLeadBomb);									//CSB 23/02/00
					else																	//CSB 23/02/00
						SetToUIScreen(&userMsgScrLead);										//CSB 23/02/00
				else																		//CSB 23/02/00
					SetToUIScreen(&userMsgScrFoll);											//CSB 23/02/00
			}
		}
	}
}

void COverlay::SetToCommsChatScr()
{
	Kludge();
	if (!Manual_Pilot.ControlledAC2->Status.deadtime)
	{
		SetToUIScreen(&commsChatDialog);
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetToMapAccelScreen
//Author		Paul.
//Date			Wed 3 Mar 1999
//------------------------------------------------------------------------------
void COverlay::SetToMapAccelScreen()
{
	Kludge();
	if (Manual_Pilot.ControlledAC2->Status.deadtime)// ||
//		Manual_Pilot.ControlledAC2->movecode!=AUTO_FOLLOWWP)
		OverLay.pvp->ReturnFromMap();
	else
		SetToUIScreen(&accelMapScr);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetToReplayScreen
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
void COverlay::SetToReplayScreen()
{
	Kludge();
	currIcons=replayIconTable;
	SetToUIScreen(&replayScr);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DisplayCommsQuitDialog
//Author		Paul.
//Date			Fri 19 Feb 1999
//				returns 00x000==no selection
//						0x0001==okay selected
//						0xFFFF==cancel selected
//------------------------------------------------------------------------------
UWord COverlay::DisplayCommsQuitDialog()
{
	Kludge();
 	SetToUIScreen(&commsDialog);
	return commsDialReturn;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetToUIScreen
//Author		Paul.
//Date			Tue 16 Feb 1999
//------------------------------------------------------------------------------
bool COverlay::SetToUIScreen(MapScr* pns)
{
	if (pScrollyText)
	{
		delete pScrollyText;
		pScrollyText=NULL;
	}

	if (pCurScr!=NULL)
		return false;		//reject a change if a screen is already on display
	InitMousePos();
	pNewScr=pns;
	return true;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		InitMousePos
//Author		Paul
//Date			Sat 20 Feb 1999
//------------------------------------------------------------------------------
void COverlay::InitMousePos()
{
	mouseX=320;
	mouseY=240;
	lastMouseX=lastMouseY-1;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		UpdateMousePos
//Author		Paul
//Date			Sat 20 Feb 1999
//------------------------------------------------------------------------------
bool COverlay::UpdateMousePos()
{
	_Analogue.PollPosition();
	int newMouseX=(SWord)_Analogue.ReadPosition(AU_UI_X);
	fMouseDisabled=false;

	if (newMouseX==-0x8000) fMouseDisabled=true;


		if (!(newMouseX==-0x8000))
		{
			if (_Analogue.tune[AU_UI_X].aliased[AA_TT_REVERSE])
			{
				newMouseX=-newMouseX;
			}
			newMouseX+=32768;											//JIM 15Sep00
		}

//	if (newMouseX!=-0x8000)
//	{
	mouseX=newMouseX*(physicalWidth-10)/65536;

//TEMPCODE RDH 20/05/99 	newMouseX>>=4;
//TEMPCODE RDH 20/05/99 		if (lastMouseX!=-1)	mouseX+=(newMouseX-lastMouseX);
//TEMPCODE RDH 20/05/99
//TEMPCODE RDH 20/05/99 		if (mouseX>=0 && mouseX<=physicalWidth)
//TEMPCODE RDH 20/05/99 		{
//TEMPCODE RDH 20/05/99 			lastMouseX=newMouseX;
//TEMPCODE RDH 20/05/99 		}
//TEMPCODE RDH 20/05/99 		else if (mouseX<0)
//TEMPCODE RDH 20/05/99 		{
//TEMPCODE RDH 20/05/99 			mouseX=0;
//TEMPCODE RDH 20/05/99 			lastMouseX=-1;
//TEMPCODE RDH 20/05/99 		}
//TEMPCODE RDH 20/05/99 		else if (mouseX>physicalWidth)
//TEMPCODE RDH 20/05/99 		{
//TEMPCODE RDH 20/05/99 			mouseX=physicalWidth;
//TEMPCODE RDH 20/05/99 			lastMouseX=-1;
//TEMPCODE RDH 20/05/99 		}
//	}
	int newMouseY=(SWord)_Analogue.ReadPosition(AU_UI_Y);
 	if (newMouseY==-0x8000) fMouseDisabled=true;

	if (
		!(newMouseY==-0x8000))
	{

		if (_Analogue.tune[AU_UI_Y].aliased[AA_TT_REVERSE])
		{
			newMouseY=-newMouseY;
		}
		newMouseY+=32768;
	}

//	if (newMouseY!=-0x8000)
//	{
	mouseY=newMouseY*(physicalHeight-10)/65536;
//TEMPCODE RDH 20/05/99 	newMouseY>>=4;
//TEMPCODE RDH 20/05/99 		if (lastMouseY!=-1) mouseY+=(newMouseY-lastMouseY);
//TEMPCODE RDH 20/05/99
//TEMPCODE RDH 20/05/99 		if (mouseY>=0 && mouseY<=physicalHeight)
//TEMPCODE RDH 20/05/99 		{
//TEMPCODE RDH 20/05/99 			lastMouseY=newMouseY;
//TEMPCODE RDH 20/05/99 		}
//TEMPCODE RDH 20/05/99 		else if (mouseY<0)
//TEMPCODE RDH 20/05/99 		{
//TEMPCODE RDH 20/05/99 			mouseY=0;
//TEMPCODE RDH 20/05/99 			lastMouseY=-1;
//TEMPCODE RDH 20/05/99 		}
//TEMPCODE RDH 20/05/99 		else if (mouseY>physicalHeight)
//TEMPCODE RDH 20/05/99 		{
//TEMPCODE RDH 20/05/99 			mouseY=physicalHeight;
//TEMPCODE RDH 20/05/99 			lastMouseY=-1;
//TEMPCODE RDH 20/05/99 		}
//TEMPCODE RDH 20/05/99 //	}
	bool retval;

	SHAPE.LogMousePos(mouseX,mouseY,physicalWidth,physicalHeight,fMouseDisabled);	//RJS 23Nov99

	if (Key_Tests.KeyPress3d(MENUSELECT))
		retval=true;
	else
		retval=false;
	return retval;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		LoaderScreen
//Author		Paul.
//Date			Mon 22 Feb 1999
//------------------------------------------------------------------------------
void COverlay::LoaderScreen(int stage)
{

	if (pball)															//JON 13Jun00
		delete[](UByte*)pball;
	pball=NULL;

	GetSurfaceDimensions(physicalWidth,physicalHeight);

	// initialise the palettes for the 3d... and do the font width calculatioin
	if ( stage == 0 )
	{
		Three_Dee.InitPalette( g_lpLib3d );  // Jon
		Land_Scape.InitTextures( g_lpLib3d );							//JON 9Aug00
		fIsWide=IsWide(physicalWidth);
	}

	g_lpLib3d->BeginScene(RENDERTARGET_PRIMARY);
	g_lpLib3d->LoadIdentity(MATRIX_VIEWER);
	g_lpLib3d->LoadIdentity(MATRIX_OBJECT);
	g_lpLib3d->LoadIdentity(MATRIX_PROJECTION);

	g_lpLib3d->GiveHint(HINT_2DRENDER);

	int realStage,subStage;
	realStage=stage>>8;
	subStage=stage&0xFF;
	FileNum loaderArt=FIL_LOADING;
	fileblockptr fbp=new fileblock(loaderArt);
	g_lpLib3d->LoadBMP( (UByte*)fbp->getdata() );
	char* str=LdStr(IDS_LOADERSTAGE00+realStage);

	SWord boxWidth=0;
	for (int j=4;j>=0;j--)
	{
		char* stmp=LdStr(IDS_LOADERSTAGE00+j);
		SWord tmp=StrPixelWidthPadded(stmp);
		if (tmp>boxWidth)
			boxWidth=tmp;
		delete[]stmp;
	}
	boxWidth+=fIsWide?20:16;

	SWord boxHeight=fIsWide?20:16;

	SWord textx=(physicalWidth-boxWidth+2)>>1;
	SWord texty=(physicalHeight-boxHeight+2)>>1;

	SWord boxX=textx-2;
	SWord boxY=texty-2;

	SWord totBoxWidth = boxWidth-2;
	SmokedGlassBox(boxX, boxY, boxWidth, 1, 0x8F8F8F, 255 );
	SmokedGlassBox(boxX, boxY, 1, boxHeight, 0x8F8F8F, 255 );
	SmokedGlassBox(boxX, boxY+boxHeight-1, boxWidth, 1, 0x8F8F8F, 255 );
	SmokedGlassBox(boxX+boxWidth-1, boxY, 1, boxHeight, 0x8F8F8F, 255 );
	boxX+=1;
	boxY+=1;
	boxHeight-=2;
	boxWidth-=2;
	boxWidth=(boxWidth*subStage)>>8;

	SmokedGlassBox(boxX+boxWidth, boxY, totBoxWidth-boxWidth, boxHeight, 0x8F8FFF, 255 );
	SmokedGlassBox(boxX, boxY, boxWidth, boxHeight, 0xFF0000, 255 );
	g_lpLib3d->SetFontColour(0xFF000000);
	// put this in to force the font imagemap to be made at full quality - should never be thrown away...
	ULong curQual = ULong( HINT_FULL_RES_TEXTURE );							//JON 19Jul00
	g_lpLib3d->SetGlobal( TOGGLE_TEXTURE_QUALITY, &curQual );				//JON 19Jul00
	g_lpLib3d->GiveHint( HINT_BILINEAR );									//JON 19Jul00
	PrintAt2(textx,texty,str);
	g_lpLib3d->SetGlobal( TOGGLE_TEXTURE_QUALITY, &curQual );				//JON 19Jul00
	g_lpLib3d->GiveHint( HINT_TRILINEAR ); //note won't go above max avail  //JON 19Jul00

	textx = 10;
	texty = 10;
	char* diagStr;
	g_lpLib3d->GetDiagnosticString( diagStr );
	if ( diagStr != NULL )
		PrintAt2( textx,texty, diagStr );
	diagStr = NULL;

	if (doneonce==1)
	{
		doneonce++;
	}
//	_asm {int 3}

	delete[]str;
	delete fbp;

	g_lpLib3d->EndScene();
	g_lpLib3d->ScreenSwap();
}

void COverlay::CommsWaitingScreen(int stage)
{
	g_lpLib3d->BeginScene(RENDERTARGET_PRIMARY);
	g_lpLib3d->LoadIdentity(MATRIX_VIEWER);
	g_lpLib3d->LoadIdentity(MATRIX_OBJECT);
	g_lpLib3d->LoadIdentity(MATRIX_PROJECTION);

	g_lpLib3d->GiveHint(HINT_2DRENDER);

	int realStage,subStage;
	realStage=stage>>8;
	subStage=stage&0xFF;
	char* str=LdStr(IDS_LOADERSTAGE03);
	GetSurfaceDimensions(physicalWidth,physicalHeight);

	SWord boxWidth=StrPixelWidthPadded(str);

	boxWidth+=fIsWide?20:16;

	SWord boxHeight=fIsWide?20:16;

	SWord textx=(physicalWidth-boxWidth+2)>>1;
	SWord texty=(physicalHeight-boxHeight+2)>>1;

	SWord boxX=textx-2;
	SWord boxY=texty-2;

	SWord totBoxWidth = boxWidth-2;
	SmokedGlassBox(boxX, boxY, boxWidth, 1, 0x8F8F8F, 255 );
	SmokedGlassBox(boxX, boxY, 1, boxHeight, 0x8F8F8F, 255 );
	SmokedGlassBox(boxX, boxY+boxHeight-1, boxWidth, 1, 0x8F8F8F, 255 );
	SmokedGlassBox(boxX+boxWidth-1, boxY, 1, boxHeight, 0x8F8F8F, 255 );
	boxX+=1;
	boxY+=1;
	boxHeight-=2;
	boxWidth-=2;
	boxWidth=(boxWidth*subStage)>>8;

	SmokedGlassBox(boxX+boxWidth, boxY, totBoxWidth-boxWidth, boxHeight, 0x8F8FFF, 255 );
	SmokedGlassBox(boxX, boxY, boxWidth, boxHeight, 0xFF0000, 255 );
	g_lpLib3d->SetFontColour(0xFF000000);
	PrintAt2(textx,texty,str);

	textx = 10;
	texty = 10;
	char* diagStr;
	g_lpLib3d->GetDiagnosticString( diagStr );
	if ( diagStr != NULL )
		PrintAt2( textx,texty, diagStr );
	diagStr = NULL;

	if (doneonce==1)
	{
		doneonce++;
	}

	delete[]str;

	g_lpLib3d->EndScene();
	g_lpLib3d->ScreenSwap();
/*

	str=LdStr(IDS_COMMSWAITING);

	GetSurfaceDimensions(physicalWidth,physicalHeight);
	fIsWide=IsWide(physicalWidth);

	SWord textx=SWord(Float(LoaderTextX)*Float(physicalWidth)/Float(640));
	SWord texty=SWord(Float(LoaderTextY)*Float(physicalHeight)/Float(480));

	SWord boxX=textx-2;
	SWord boxY=texty-2;

	SWord boxWidth=0;
	{
		char* stmp; //=LdStr(IDS_LOADERSTAGE03);

		if (_DPlay.resyncbar)
			stmp=LdStr(IDS_PHRASE_RESYNCHING);
		else
			stmp=LdStr(IDS_LOADERSTAGE03);

		SWord tmp=StrPixelLen2(stmp);
		if (tmp>boxWidth) boxWidth=tmp;
		delete[]stmp;
	}
	boxWidth+=fIsWide?20:16;

	SWord boxHeight=fIsWide?20:16;

	boxX+=1;
	boxY+=1;
	boxHeight-=2;
	boxWidth-=2;
	boxWidth=(boxWidth*subStage)>>8;
	PrintAt2(textx,texty,str);

	delete[]str;
*/
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DisplayMouseCursor
//Author		Paul
//Date			Sat 20 Feb 1999
//------------------------------------------------------------------------------
void COverlay::DisplayMouseCursor()
{
	if (fMouseDisabled)
		return;

	//LoaderScreen();

 	SWord ou,ov;
 	SWord cw,ch;
	SWord sw,sh;
	SLong sx,sy;


	ImageMapDesc* pmap=Image_Map.GetImageMapPtr(SQUEAKNO);

	sx=mouseX;
	sy=mouseY;
	ou=
	ov=0;
	cw=sw=pmap->w;
	ch=sh=pmap->h;

	g_lpLib3d->SetFontColour(0xFFFFFFFF);
//TEMPCODE RDH 20/05/99 	sx=SLong(Float(sx)*Float(physicalWidth)/Float(640));
//TEMPCODE RDH 20/05/99 	sy=SLong(Float(sy)*Float(physicalHeight)/Float(480));

 	Lib3DPoint dp[4];

	dp[0].setPosition( sx, sy, 0.f );
	dp[0].setIMapCoords( ou, ov );
	dp[1].setPosition( sx+sw, sy, 0.f );
	dp[1].setIMapCoords( ou+cw, ov );
	dp[2].setPosition( sx+cw, sy+sh, 0.f );
	dp[2].setIMapCoords( ou+cw, ov+cw );
	dp[3].setPosition( sx, sy+sh, 0.f );
	dp[3].setIMapCoords( ou, ov+ch );

	PutC(pmap,dp);

}

void COverlay::SetGridDimensions(MapScr *pscr)
{
	char* prefixTable[]={"1.","2.","3.","4.","5.","6.","7.","8.","9.","0.","  ","  ","  ","  ","  ","  ","  ","  "};
	//find the longest string & count number of lines @ the same time
	MapScr::OptionList *popt=pscr->optionList;
	SLong maxChars=0;
	SLong oc;
	for (oc=0;popt->key!=SEL_EOL;oc++,popt++)
	{
		if (popt->resIDOffset!=0xFF &&
			(popt->resID<IDS_MAPICON00LO || popt->resID>IDS_MAPICON15HI))
		{
			CString cs=prefixTable[popt->key];
			//check for comms message strings...
			if (popt->resID>=IDS_INDIRECT00 &&
				popt->resID<=IDS_INDIRECT10)
				cs+=_DPlay.Messages[_DPlay.mySlot][popt->resID-IDS_INDIRECT00];
			else
			{
				char *pstr=LdStr(popt->resID+popt->resIDOffset);
				cs+=pstr;
				delete[]pstr;
			}
			//count pixel width of 'cs'
			SLong thisLen=StrPixelWidthPadded(const_cast<char*>(LPCTSTR(cs)));
			if (thisLen>maxChars)
				maxChars=thisLen;
		}
	}
	pscr->rowHeight=fIsWide?20:10;
	oc=(oc+2)*pscr->rowHeight;
	pscr->windowWidth=pscr->columnWidth=UWord(maxChars+(2*pscr->rowHeight));
	pscr->windowHeight=UWord(oc);
	pscr->gridXOffset=
	pscr->gridYOffset=pscr->rowHeight;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		ProcessUIScreen
//Author		Paul.
//Date			Fri 12 Feb 1999
//------------------------------------------------------------------------------
void COverlay::ProcessUIScreen()
{

	if ( !Three_Dee.IsPaused() )										//JON 13Jul00
		Land_Scape.BadVision();											//JON 13Jul00

	GetSurfaceDimensions(physicalWidth,physicalHeight);
	//eat map key presses
//jon	Key_Tests.KeyPress3d(GOTOMAPKEY);
	bool mouseClicked=UpdateMousePos();

	char* prefixTable[]={"1.","2.","3.",
						"4.","5.","6.",
						"7.","8.","9.",
						"0.","  ","  ",
						"  ","  ","  ",
						"  ","  ","  "};

	g_lpLib3d->PushMatrix(MATRIX_OBJECT);			//RJS 16Mar00
	g_lpLib3d->PushMatrix(MATRIX_VIEWER);
	g_lpLib3d->PushMatrix(MATRIX_PROJECTION);

	g_lpLib3d->LoadIdentity(MATRIX_OBJECT);
	g_lpLib3d->LoadIdentity(MATRIX_VIEWER);
	g_lpLib3d->LoadIdentity(MATRIX_PROJECTION);

	g_lpLib3d->GiveHint(HINT_2DRENDER);

#ifdef _SHOWFRAMETIME
	{
		SWord posx = 10, posy = 10;
		char frameTimeStr[64];
		sprintf( frameTimeStr, "%3.2f fps\n", 1000.f/float( pvp->getFrameTimeMSecs() ) );
		static ULong tottime = 0;
		tottime++;
		posx+=tottime%100;
		PrintAt2( posx, posy, frameTimeStr );
	}
#endif
#ifdef _SHOW_FREE_RAM
	{
		SWord posx = 10, posy = 22;
		char tmpStr[64];
		ULong curFree= g_lpLib3d->GetFreeVideoMemory();
		sprintf( tmpStr, "%i Bytes\n", curFree );
		PrintAt2( posx, posy, tmpStr );
#ifdef _SHOW_RAM_PERCENT
		posx = 10;
		posy = 34;
		ULong totFree;
		g_lpLib3d->GetTextureRAM( totFree );
		sprintf( tmpStr, "%3.2fpc Free\n", float( ( float(curFree)/float(totFree) )*100.f ) );
		PrintAt2( posx, posy, tmpStr );
#endif
	}
#endif
#ifdef _SHOW_RECENT_FRAME_TIMES
	{
		const int NUMLINES = 50;
		const int MAXCHARS = 24;
		static char frameTimeStr[MAXCHARS*NUMLINES];
		static int curLine=-1;
		static	int	ftime = 0;

		if ( curLine == -1 )
		{
			curLine = 0;
			memset( frameTimeStr, 0, MAXCHARS*NUMLINES );
		}
		char* curStr = frameTimeStr+MAXCHARS*curLine++;
		curLine%=NUMLINES;
//		sprintf( curStr, "%3.2f\n", 1000.f/float( pvp->getFrameTimeMSecs() ) );
		ftime = pvp->getFrameTimeMSecs();
		if (ftime > 30)
			sprintf( curStr, "***  %d    %d  ***\n", ftime,Three_Dee.View_Dt());
		else
			sprintf( curStr, "%d    %d\n", ftime,Three_Dee.View_Dt());

		curStr = frameTimeStr;

		SWord posx = 5, posy = 5;
		for ( int i = 0; i< NUMLINES; i++, curStr+=MAXCHARS )
		{
			if (i==curLine)
				g_lpLib3d->SetFontColour(0xFF5050);

			PrintAt2( posx, posy, curStr );
			posx=5;
			posy+=12;

			g_lpLib3d->SetFontColour(0xFFFFFF);
		}

	}
#endif
#ifdef _SHOWDIGITIME
	{
		SWord posx = 10, posy = 25;
		char dayTimeStr[20];
		int tod = pvp->TimeOfDay()/100; // time is in seconds
		int hours = tod/(60*60);
		int mins = (tod/60)%60;
		int secs = tod%60;
		sprintf( dayTimeStr, "%i : %i : %i\n", hours, mins, secs );
		PrintAt2( posx, posy, dayTimeStr );
	}
#endif
#ifdef _SHOWCURTILENUM
	{
		g_lpLib3d->SetFontColour( 0xFF0000 );
		ULong x = pvp->World.X >> Grid_Base::WORLDSPACEMOD;
		ULong z = pvp->World.Z >> Grid_Base::WORLDSPACEMOD;
		char blockStr[7];
		blockStr[6] = char(0);
		blockStr[5] = '1'+char(z%8);
		blockStr[3] = '1'+char((z/8)%8);
		blockStr[1] = '1'+char((z/64)%8);
		blockStr[4] = '1'+char(x%8);
		blockStr[2] = '1'+char((x/8)%8);
		blockStr[0] = '1'+char((x/64)%8);
		SWord posx = 10, posy = 40;
		PrintAt2( posx, posy, blockStr );
	}
#endif
	g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, 1 );

	if (pNewScr!=pCurScr)
	{
		pCurScr=pNewScr;
		highTimer=0;
		if (pCurScr!=NULL && pCurScr->initRtn!=NULL)
		{
			Key k=(pCurScr->*(pCurScr->initRtn))();
			hilight=k;
		}
	}
	else if (pCurScr!=NULL)
	{
		SetUp3DUIMouse();


		//check for hilight/dehilight counter

		if (highTimer>0 && (highTimer-=5)<=0)
		{
			highTimer=0;
			hilight=SEL_NONE;
		}

		//count options...

		MapScr::OptionList* pCurOpt=pCurScr->optionList;

		SLong optionCount=0;

		while (pCurOpt->key!=SEL_EOL) {pCurOpt++;optionCount++;}

		if (MapScr::SCALE_DYNAMICSIZE&pCurScr->gridFlags)
			SetGridDimensions(pCurScr);

		//display current screen options && test for input @ the same time

		pCurOpt=pCurScr->optionList;

		SLong 	posx,posy,				//scaled screen coordinates
										//of the top left corner of the
										//grid
				width,height,			//scaled screen coordinates of the
										//width and height of a single grid
										//element
				scaledMouseX,scaledMouseY,

				gridXOff,gridYOff,

				rowIndex,columnIndex;

//		if (mouseClicked)
//		{
			scaledMouseX=SLong(Float(mouseX));//*Float(physicalWidth)/Float(640));
			scaledMouseY=SLong(Float(mouseY));//*Float(physicalHeight)/Float(480));
//		}

		if (MapScr::SCALE_GRIDORIGIN&pCurScr->gridFlags)
		{
			posx=SLong(Float(pCurScr->windowLeft)*Float(physicalWidth)/Float(640));
			posy=SLong(Float(pCurScr->windowTop)*Float(physicalHeight)/Float(480));
		}
		else
		{
			posx=pCurScr->windowLeft;
			posy=pCurScr->windowTop;
		}

		//wipe the background if neccessary

		if (pCurScr->wipeColour!=NO_BACKGROUND_WIPE)
		{
			ULong	oldZfiddle=3;
			g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, &oldZfiddle );
			SLong winWidth,winHeight;

			if (MapScr::SCALE_GRIDWH&pCurScr->gridFlags)
			{
				winWidth=SLong(Float(pCurScr->windowWidth)*Float(physicalWidth)/Float(640));
				winHeight=SLong(Float(pCurScr->windowHeight)*Float(physicalHeight)/Float(480));
			}
			else
			{
				winWidth=pCurScr->windowWidth;
				winHeight=pCurScr->windowHeight;
			}
			if (MapScr::JUSTIFY_CENTRE_SX&pCurScr->gridFlags)
				posx=(physicalWidth>>1)-(winWidth>>1);

			if (MapScr::JUSTIFY_CENTRE_SY&pCurScr->gridFlags)
				posy=(physicalHeight>>1)-(winHeight>>1);

			if ((pCurScr->wipeColour&WIPE_ALPHA_MASK)==0)
			{
				ImageMapNumber imno=(ImageMapNumber)GET_WIPE_TEXTURE(pCurScr->wipeColour);
				ImageMapDesc* pmap=Image_Map.GetImageMapPtr(imno);
			 	Lib3DPoint dp[4];

				dp[0].setPosition( posx, posy, 0.f );
				dp[0].setIMapCoords( 0, 0 );
				dp[1].setPosition( posx+winWidth, posy, 0.f );
				dp[1].setIMapCoords( pmap->w, 0 );
				dp[2].setPosition( posx+winWidth, posy+winHeight, 0.f );
				dp[2].setIMapCoords( pmap->w, pmap->h );
				dp[3].setPosition( posx, posy+winHeight, 0.f );
				dp[3].setIMapCoords( 0, pmap->h );
				PutC(pmap,dp);
			}
			else
			{
				SmokedGlassBox( posx, posy, winWidth, winHeight, pCurScr->wipeColour, 8 );
			}
			g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, &oldZfiddle );
		}

		if (MapScr::SCALE_GRIDOFFSETS&pCurScr->gridFlags)
		{
			gridXOff=SLong(Float(pCurScr->gridXOffset)*Float(physicalWidth)/Float(640));
			gridYOff=SLong(Float(pCurScr->gridYOffset)*Float(physicalHeight)/Float(480));
		}
		else
		{
			gridXOff=pCurScr->gridXOffset;
			gridYOff=pCurScr->gridYOffset;
		}

		posx+=gridXOff;
		posy+=gridYOff;

		if (MapScr::SCALE_GRIDELEMENTS&pCurScr->gridFlags)
		{
			width=SLong(Float(pCurScr->columnWidth)*Float(physicalWidth)/Float(640));
			height=SLong(Float(pCurScr->rowHeight)*Float(physicalHeight)/Float(480));
		}
		else
		{
			width=pCurScr->columnWidth;
			height=pCurScr->rowHeight;

			if (MapScr::SCALE_DOUBLEIFGT800&pCurScr->gridFlags)
			{
				if (IsExtraWide(physicalWidth))
				{
					width<<=1;
					height<<=1;
				}
			}
		}

		rowIndex=columnIndex=0;

		for (SLong i=0;i<optionCount;i++)
		{
			//check for a blank entry first

			SWord curx,cury;
			SWord savex,savey;

			savex=curx=posx+columnIndex*width;
			savey=cury=posy+rowIndex*height;

			if (pCurOpt->resIDOffset!=0xFF)
			{
				ULong	oldZfiddle=2;
				g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, &oldZfiddle );

				//check for icons instead of text
				if (pCurOpt->resID>=IDS_MAPICON00LO &&
					pCurOpt->resID<=IDS_MAPICON15HI)
				{
					//handle replay icons by printing them on screen as single
					//characters using the text system

					g_lpLib3d->SetFontColour(pCurScr->textColour);

					UWord iconIndex=pCurOpt->resID-IDS_MAPICON00LO;

					if (pCurOpt->key==hilight)
						iconIndex+=pCurOpt->resIDOffset;

					PrintIcon(curx,cury,width,height,iconIndex);
				}
				else if (pCurOpt->resID>=IDS_INDIRECT00 &&
						 pCurOpt->resID<=IDS_INDIRECT10)
				{
// if indirectStringTable is not used for anything other than comms
// then can just access players messages
					SWord index=pCurOpt->resID-IDS_INDIRECT00;
					if (pCurOpt->key==hilight)
						g_lpLib3d->SetFontColour(pCurScr->hilightColour);
					else
						g_lpLib3d->SetFontColour(pCurScr->textColour);
					char* theString;
					theString=prefixTable[pCurOpt->key];
					PrintAt2(curx,cury,theString);
					theString=_DPlay.Messages[_DPlay.mySlot][index];
					PrintAt2(curx,cury,theString);
				}
				else
				{
					if (pCurOpt->key==hilight)
						g_lpLib3d->SetFontColour(pCurScr->hilightColour);
					else
						g_lpLib3d->SetFontColour(pCurScr->textColour);

					char* theString;
					theString=prefixTable[pCurOpt->key];
					PrintAt2(curx,cury,theString);
					theString=LdStr(pCurOpt->resID+pCurOpt->resIDOffset);
					PrintAt2(curx,cury,theString);
					delete[]theString;
				}

				g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, oldZfiddle );
			}

			//check this option for keyboard && mouse input

			if (pCurOpt->key!=SEL_BLANK)
			{
				KeyVal3D thisKey;
				KeyVal3D keyTable[]={	RPM_10,RPM_20,RPM_30,RPM_40,
										RPM_50,RPM_60,RPM_70,RPM_80,
										RPM_90,RPM_00,RPM_UP,RPM_DOWN,PADLOCKTOG,
										RESETVIEW,ROTUP,ROTDOWN,ROTLEFT,ROTRIGHT,
										PAUSEKEY,EXITKEY};

				thisKey=keyTable[pCurOpt->key];
				if (Key_Tests.KeyPress3d(thisKey) &&
					pCurOpt->selRtn!=NULL)
				{
					highTimer=pCurScr->highTimer;
					pNewScr=(pCurScr->*(pCurOpt->selRtn))(pCurOpt->resIDOffset,(Key)pCurOpt->key);
					Kludge();
				}


				else if (	scaledMouseX>=savex && scaledMouseX<(savex+width) &&
							scaledMouseY>=savey && scaledMouseY<(savey+height)	)
				{
					if (mouseClicked && pCurOpt->selRtn!=NULL)
					{
						highTimer=pCurScr->highTimer;
						pNewScr=(pCurScr->*(pCurOpt->selRtn))(pCurOpt->resIDOffset,(Key)pCurOpt->key);
						Kludge();
					}
					else if (MapScr::FLAG_MOUSEOVERHI&pCurScr->gridFlags)
					{
						hilight=(Key)pCurOpt->key;
					}
				}
			}

			//update grid coordinates to the next entry

			if (++columnIndex>=pCurScr->numColumns)
			{
				columnIndex=0;
				if (++rowIndex>pCurScr->numRows) INT3;
			}
			pCurOpt++;
		}

		//extra piece of code called each frame. Currently used to update the
		//messages displayed in the window on the map screen

		g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, ULong(0) );

		if (pCurScr!=NULL && pCurScr->extraRtn!=NULL)
			(pCurScr->*(pCurScr->extraRtn))();

		g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, 1 );

		DisplayMouseCursor();

		if (_DPlay.SimulateExitKey)
			pNewScr=pCurScr=NULL;
	}
	else if (pNewScr==NULL && _MsgBuffer.thereIsANewMessage)
	{
		_MsgBuffer.thereIsANewMessage=false;
		_MsgBuffer.InitMessageWindow();
//Dead		msgText[0]=_MsgBuffer.Callsign(1)+_MsgBuffer.MessageBody(1);
		msgText[0]=_MsgBuffer.Callsign(1)+*_MsgBuffer.lateststring;			//RJS 15Jun99
		otherText[0]="";
		ChopStr(msgText[0],msgText[1]);
		messageTimer=5*ONE_SECOND_TIME;	//5 seconds of message display			  //RDH 19/06/99
	}
//dead	if (pvp) pvp->BlockTick(FALSE);

	if (doInteractiveCockpit)										//RJS 16Mar00
		DisplayMouseCursor();

	if (pCurScr==NULL)
	{
		RestoreMouse();
	}

// Aircaft names...

	SWord	tmpX,tmpY;													//MS 28/04/00
	for (int namecnt=0;namecnt < nameTableCnt;namecnt++)
	{
		g_lpLib3d->SetFontColour(NameTable[namecnt].colour);			//RJS 09May00

		tmpX = NameTable[namecnt].x;									//MS 28/04/00
		tmpY = NameTable[namecnt].y;									//MS 28/04/00
		PrintAt2(tmpX,tmpY,NameTable[namecnt].text,true);				//RJS 31Aug00
	}

	nameTableCnt = 0;

// Perihperal blobs...

	DrawPeripheralBlobs();												//RJS 1Sep00

	g_lpLib3d->GiveHint(HINT_3DRENDER);
	g_lpLib3d->SetGlobal( TOGGLE_GLOBAL2DZDEPTH, ULong(0) );
	g_lpLib3d->PopMatrix(MATRIX_OBJECT);
	g_lpLib3d->PopMatrix(MATRIX_VIEWER);
	g_lpLib3d->PopMatrix(MATRIX_PROJECTION);
}

/**********************
**	FIRST MAP SCREEN **
**********************/


const UWord MapScreenX=370;
const UWord MapScreenY=34;
const UWord MapScreenWidth=241;
const UWord MapScreenHeight=272;
const UWord MapGridXOffset=10;
const UWord MapGridYOffset=10;
const UWord MapColumnWidth=MapScreenWidth;
const UWord MapRowHeight=10;
const UWord MapGridNumCols=1;
const UWord MapGridNumRows=10;

const UWord RadioScreenWidth=160;
const UWord RadioScreenHeight=200;

const UWord CommsChatWidth=480;
const UWord CommsChatHeight=160;

inline void MakeTimeString(char* str,int time)
{
	time/=6000;
	int mins=time%60;
	time=time/60;
	time%=24;
	sprintf(str,"%2d:%02d",time,mins);
}


inline WayPointPtr FindNextWP(UNIQUE_ID uid,WayPointPtr wp)
{
	WayPointPtr nwp=wp;
	//return straight away if no waypoint
	if (nwp==NULL)
		return nwp;
	//skip forwards to the next valid waypoint
	nwp=nwp->next;
	while (nwp!=NULL && (uid.count<nwp->skipunder || uid.count>nwp->skipover))
		nwp=nwp->next;
	return nwp;
}

inline WayPointPtr FindPrevWP(UNIQUE_ID uid,WayPointPtr wp)
{
	WayPointPtr nwp=wp;
	//return straight away if no waypoint
	if (nwp==NULL)
		return nwp;
	//skip backwards to the next valid waypoint
	nwp=nwp->prev;
	while (nwp!=NULL && (uid.count<nwp->skipunder || uid.count>nwp->skipover))
		nwp=nwp->prev;
	return nwp;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		UpdateMessageDisplay
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
void MapScr::UpdateWaypointDisplay()
{
/*			resid=this_name+IDS_L_MAIN_WP_GAP;
			if (!Save_Data.fSoftware)	OverLay.pw->DoSetFontColour(messageColours[i]);
			else						POLYGON.SetFontColour(softMessageColours[i]);
			char temp[]="1234567890";
			char* theString=OverLay.LdStr(resid);
			cs=theString;
			if (wpp->wpnum!=0)
			{
				sprintf(temp," (%d)",wpp->wpnum);
				cs+=temp;
			}
			delete[]theString;
			saveX=localX;
			saveY=localY;
			OverLay.PrintAt2(localX,localY,const_cast<char*>(LPCTSTR(cs)));
			localX=saveX+8*32;
			localY=saveY;
			COORDS3D pos=*(wpp->FindDesPos());
			pos.Y=(pos.Y*10)/Save_Data.alt.mediummm;
			pos.Y+=50;
			pos.Y-=(pos.Y%100);
			sprintf(temp,"%d",pos.Y);
			cs=temp;
			resid=Save_Data.alt.mediumabbr;
			theString=OverLay.LdStr(resid);
			cs+=theString;
			delete[]theString;
			saveX=localX;
			saveY=localY;
			OverLay.PrintAt2(localX,localY,const_cast<char*>(LPCTSTR(cs)));
			localX=saveX+8*24;
			localY=saveY;
			pos.Y=wpp->ETA;
			MakeTimeString(temp,pos.Y);
			cs=temp;
			OverLay.PrintAt2(localX,localY,const_cast<char*>(LPCTSTR(cs)));
		}
	}
	HandleUpDnLftRgtToggleKeys();
	OverLay.message_caller=NULL;
	OverLay.message_callee=NULL;
	OverLay.message_target=NULL;

	if (!OverLay.fZoomedMap) ShowItemIcons(true);
	else					 ShowItemIconsZoomed(true);

	messageX=16+SWord(Float(8)*Float(OverLay.physicalWidth)/Float(640));
	messageY=SWord(Float(3)*Float(OverLay.physicalHeight)/Float(480));

	//display place name and time here
	CString placeName=_MsgBuffer.GetPlaceName(Manual_Pilot.ControlledAC2->World);
	int time=OverLay.pvp->TimeOfDay();
	char timeStr[]="hh:mm\0";
	MakeTimeString(timeStr,time);
	CString csTime=timeStr;
	csTime+="        "+placeName;
	OverLay.PrintAt2(messageX,messageY,const_cast<char*>(LPCTSTR(csTime)));

	//allow user to step through waypoints here

	WayPointPtr newWP;
	if (Key_Tests.KeyPress3d(ZOOMIN) &&
		(newWP=FindPrevWP(Manual_Pilot.ControlledAC2->uniqueID,OverLay.curr_waypoint))!=NULL)
		OverLay.curr_waypoint=newWP;
	else if (Key_Tests.KeyPress3d(ZOOMOUT) &&
		(newWP=FindNextWP(Manual_Pilot.ControlledAC2->uniqueID,OverLay.curr_waypoint))!=NULL)
		OverLay.curr_waypoint=newWP;
		*/
}

void MapScr::UpdateMessageDisplay()
{
	SWord messageX=SWord(Float(1)*Float(OverLay.physicalWidth)/Float(640));
	SWord messageY=SWord(Float(438)*Float(OverLay.physicalHeight)/Float(480));
	SWord messageH=SWord(Float(14)*Float(OverLay.physicalHeight)/Float(480));

	if (Manual_Pilot.ControlledAC2 &&
		Manual_Pilot.ControlledAC2->waypoint)
		OverLay.curr_waypoint=Manual_Pilot.ControlledAC2->waypoint;

	ULong messageColours[3]={TEXT_MAP_WHITE,TEXT_HI,TEXT_MAP_WHITE};

	for (SLong i=2;i>=0;i--)
	{
		SWord localX,localY;
		localX=messageX;
		localY=messageY;
		messageY+=messageH;
		CString cs=_MsgBuffer.Callsign(i)+_MsgBuffer.MessageBody(i);
		OverLay.g_lpLib3d->SetFontColour(messageColours[i]);
		if (i==1)
		{
			if (!OverLay.pScrollyText)
			{
				SWord targetIndex=OverLay.CalcTargIndex(cs);
				OverLay.pScrollyText=new COverlay::ScrollyText(targetIndex);
			}
			cs=OverLay.pScrollyText->Scroll(cs,OverLay.pvp->RealFrameTime());
		}
		OverLay.MakeDotDotDot(cs);
		OverLay.PrintAt2(localX,localY,const_cast<char*>(LPCTSTR(cs)));
	}
	HandleUpDnLftRgtToggleKeys();
	_MsgBuffer.MessageParticipants(1,OverLay.message_caller,OverLay.message_callee,OverLay.message_target);
	if (!OverLay.fZoomedMap) ShowItemIcons(false);
	else					 ShowItemIconsZoomed(false);

	//display world coordinates & cd block position of the piloted a/c
	//on the map

	messageX=16+SWord(Float(8)*Float(OverLay.physicalWidth)/Float(640));
	messageY=SWord(Float(3)*Float(OverLay.physicalHeight)/Float(480));
	if (Save_Data.gamedifficulty[GD_VISIBLEMIGSCHEAT])						//CSB 27Jul00
	{
		SLong viewer_x=Manual_Pilot.ControlledAC2->World.X;
		SLong viewer_z=Manual_Pilot.ControlledAC2->World.Z;
		char block[128];
		sprintf(block,"WX: %d WZ: %d",viewer_x,viewer_z);
		OverLay.PrintAt2(messageX,messageY,block);
		messageX=16+SWord(Float(8)*Float(OverLay.physicalWidth)/Float(640));
		messageY=SWord(Float(3)*Float(OverLay.physicalHeight)/Float(480))+16;
		block[4]=char(0);
		SLong xof=(viewer_x&0x1FFFFF)>>10;	//0..2048
		SLong val=viewer_x>>(17+3);
		block[2]=char(val&0x7)+'1';		//x (lo)
		val>>=3;
		block[0]=char(val)+'1';			//x (hi)
		SLong zof=2047-((viewer_z&0x1FFFFF)>>10);
		val=viewer_z>>(17+3);
		block[3]=char(val&0x7)+'1';		//z (lo)
		val>>=3;
		block[1]=char(val)+'1';			//z (hi)
		OverLay.PrintAt2(messageX,messageY,block);
		sprintf(block," xoff %d ",xof);
		OverLay.PrintAt2(messageX,messageY,block);
		sprintf(block,"zoff %d ",zof);
		OverLay.PrintAt2(messageX,messageY,block);
	//#ifdef _SHOWCURTILENUM
		{
			ULong x = Manual_Pilot.ControlledAC2->World.X >> Grid_Base::WORLDSPACEMOD;
			ULong z = Manual_Pilot.ControlledAC2->World.Z >> Grid_Base::WORLDSPACEMOD;
			char blockStr[7];
			blockStr[6] = char(0);
			blockStr[5] = '1'+char(z%8);
			blockStr[3] = '1'+char((z/8)%8);
			blockStr[1] = '1'+char((z/64)%8);
			blockStr[4] = '1'+char(x%8);
			blockStr[2] = '1'+char((x/8)%8);
			blockStr[0] = '1'+char((x/64)%8);
			SWord posx = 10, posy = 40;
			OverLay.PrintAt2( posx, posy, blockStr );
		}
	//#endif
	}
	else																	//CSB 27Jul00
	{
		//display place name and time here
		CString placeName=_MsgBuffer.GetPlaceName(Manual_Pilot.ControlledAC2->World);
		int time=OverLay.pvp->TimeOfDay();
		char timeStr[]="hh:mm\0";
		MakeTimeString(timeStr,time);
		CString csTime=timeStr;
		csTime+="        "+placeName;
		OverLay.PrintAt2(messageX,messageY,const_cast<char*>(LPCTSTR(csTime)));
	}
	if (Key_Tests.KeyPress3d(ZOOMIN))
	{
		_MsgBuffer.DecMessageWindow();
		delete OverLay.pScrollyText;
		OverLay.pScrollyText=NULL;
	}
	else if (Key_Tests.KeyPress3d(ZOOMOUT))
	{
		_MsgBuffer.IncMessageWindow();
		delete OverLay.pScrollyText;
		OverLay.pScrollyText=NULL;
	}
}

//map is made up of two 256x256 texture maps giving a map size of 256x512
//the map must be clipped to a window defined by the following eqautes

#define MAP_SCREEN_ORIGIN_X_640	0										//RJS 27Jun00
#define MAP_SCREEN_ORIGIN_Y_640 0										//RJS 27Jun00
#define MAP_SCREEN_WIDTH_640	640										//RJS 27Jun00
#define MAP_SCREEN_HEIGHT_640	480										//RJS 27Jun00

//the shift factor used to zoom the map (1==2x zoom)

#define MAP_SCALE				1
#define MAP_TEXTURE_WIDTH		(256<<MAP_SCALE)
#define MAP_TEXTURE_HEIGHT		(512<<MAP_SCALE)

inline SWord WorldXToTu(SLong wx)
{
	const SLong grid_scale=8;
 	const SLong grid_size=1<<(21-grid_scale);
	const SLong grid_start=11*grid_size;
	const SLong grid_width=37*grid_size;
	wx>>=grid_scale;
	wx-=grid_start;
	wx<<=(8+MAP_SCALE);
	wx/=grid_width;
	return SWord(wx+(8<<MAP_SCALE));
}

inline SWord WorldZToTv(SLong wz)
{
	const SLong grid_scale=8;
 	const SLong grid_size=1<<(21-grid_scale);
	const SLong grid_start=0;
	const SLong grid_width=64*grid_size;
	wz>>=grid_scale;
	wz-=grid_start;
	wz<<=(9+MAP_SCALE);
	wz/=grid_width;
	wz=(512<<MAP_SCALE)-wz;
	return SWord(wz);
}

const float XSCALEFUDGE=1.15;

inline SWord WorldXToSx640(SLong wx,SWord *x)
{
	SWord texel_u=WorldXToTu(wx);
	Float frac_u=Float(texel_u-x[0])/Float(x[1]-x[0]);
	SWord screen_x=MAP_SCREEN_ORIGIN_X_640+SWord(XSCALEFUDGE*frac_u*Float(MAP_SCREEN_WIDTH_640));
	return screen_x;
}

inline SWord WorldZToSy640(SLong wz,SWord *y)
{
	SWord texel_v=WorldZToTv(wz);
	Float frac_v=Float(texel_v-y[0])/Float(y[1]-y[0]);
	SWord screen_y=MAP_SCREEN_ORIGIN_Y_640+SWord(frac_v*Float(MAP_SCREEN_HEIGHT_640));
	return screen_y;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		WorldToSxy
//Author		Robert Slater
//Date			Mon 5 Jun 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
inline	void	WorldToSxy(Coords3D& pos,SWord& screenx, SWord& screeny)
{
	float	rhw = 1./float(Three_Dee.ViewerY());

	screenx = (float(pos.X - Three_Dee.ViewerX())*rhw*320.)+320.;
	screeny = (float(pos.Z - Three_Dee.ViewerZ())*-rhw*240.)+240.;
}

inline SWord WorldXToSx640Zoomed(SLong wx,SLong *x)
{
	Float frac_u=Float(wx-x[0])/Float(x[1]-x[0]);
	SWord screen_x=SWord(frac_u*Float(640));
	return screen_x;
}

inline SWord WorldZToSy640Zoomed(SLong wz,SLong *y)
{
	Float frac_v=Float(wz-y[0])/Float(y[1]-y[0]);
	SWord screen_y=SWord(frac_v*Float(480));
	return screen_y;
}


inline UWord MapClipFlags(SWord ix,SWord iy,SWord *x,SWord *y)
{
	UWord clipResult=0;
	if (ix<x[0])		clipResult|=OFFLEFT;
	else if (ix>x[1])	clipResult|=OFFRIGHT;
	if (iy<y[0])		clipResult|=OFFTOP;
	else if (iy>y[1])	clipResult|=OFFBOTTOM;
	return clipResult;
}

inline UWord MapClipFlagsZoomed(SLong ix,SLong iy,SLong *x,SLong *y)
{
	UWord clipResult=0;
	if (ix<x[0])		clipResult|=OFFLEFT;
	else if (ix>x[1])	clipResult|=OFFRIGHT;
	if (iy<y[0])		clipResult|=OFFTOP;
	else if (iy>y[1])	clipResult|=OFFBOTTOM;
	return clipResult;
}

inline bool ClipPoint(SWord& x,SWord& y,UWord f,SWord *xc,SWord *yc)
{
	D3DVALUE frac;
	SWord temp;
	UWord temp2;

	if (f&OFFTOP)
	{
		y = yc[0];

		f=MapClipFlags(x,y,xc,yc);
	}

	if (f&OFFBOTTOM)
	{
		y = yc[1];

		f=MapClipFlags(x,y,xc,yc);
	}

	if (f&OFFLEFT)
	{
		x = xc[0];

		f=MapClipFlags(x,y,xc,yc);
	}

	if (f&OFFRIGHT)
	{
		x=xc[1];
		f=MapClipFlags(x,y,xc,yc);
	}
	return f==0?true:false;
}

inline bool ClipLine(SWord *x,SWord *y,UWord *f,SWord *xc,SWord *yc)
{
	Float frac;
	SWord temp;
	UWord temp2;

	if ((f[0]|f[1])&OFFTOP)
	{
		if (f[0]&OFFTOP)
		{
			temp=x[0]; x[0]=x[1]; x[1]=temp;
			temp=y[0]; y[0]=y[1]; y[1]=temp;
			temp2=f[0]; f[0]=f[1]; f[1]=temp2;
		}
		frac=Float(yc[0]-y[0])/Float(y[1]-y[0]);
		y[1]=yc[0];
		x[1]=x[0]+SWord(frac*Float(x[1]-x[0]));
		f[1]=MapClipFlags(x[1],y[1],xc,yc);
	}

	if ((f[0]|f[1])&OFFBOTTOM)
	{
		if (f[0]&OFFBOTTOM)
		{
			temp=x[0]; x[0]=x[1]; x[1]=temp;
			temp=y[0]; y[0]=y[1]; y[1]=temp;
			temp2=f[0]; f[0]=f[1]; f[1]=temp2;
		}
		frac=Float(yc[1]-y[0])/Float(y[1]-y[0]);
		y[1]=yc[1];
		x[1]=x[0]+SWord(frac*Float(x[1]-x[0]));
		f[1]=MapClipFlags(x[1],y[1],xc,yc);
	}

	if ((f[0]|f[1])&OFFLEFT)
	{
		if (f[0]&OFFLEFT)
		{
			temp=x[0]; x[0]=x[1]; x[1]=temp;
			temp=y[0]; y[0]=y[1]; y[1]=temp;
			temp2=f[0]; f[0]=f[1]; f[1]=temp2;
		}
		frac=Float(xc[0]-x[0])/Float(x[1]-x[0]);
		x[1]=xc[0];
		y[1]=y[0]+SWord(frac*Float(y[1]-y[0]));
		f[1]=MapClipFlags(x[1],y[1],xc,yc);
	}

	if ((f[0]|f[1])&OFFRIGHT)
	{
		if (f[0]&OFFRIGHT)
		{
			temp=x[0]; x[0]=x[1]; x[1]=temp;
			temp=y[0]; y[0]=y[1]; y[1]=temp;
			temp2=f[0]; f[0]=f[1]; f[1]=temp2;
		}
		frac=Float(xc[1]-x[0])/Float(x[1]-x[0]);
		x[1]=xc[1];
		y[1]=y[0]+SWord(frac*Float(y[1]-y[0]));
		f[1]=MapClipFlags(x[1],y[1],xc,yc);
	}
	return (f[0]|f[1])==0?true:false;
	return true;
}

inline bool ClipLineZoomed(SLong *x,SLong *y,UWord *f,SLong *xc,SLong *yc)
{
	Float frac;
	SLong temp;
	UWord temp2;

	if ((f[0]|f[1])&OFFTOP)
	{
		if (f[0]&OFFTOP)
		{
			temp=x[0]; x[0]=x[1]; x[1]=temp;
			temp=y[0]; y[0]=y[1]; y[1]=temp;
			temp2=f[0]; f[0]=f[1]; f[1]=temp2;
		}
		frac=Float(yc[0]-y[0])/Float(y[1]-y[0]);
		y[1]=yc[0];
		x[1]=x[0]+SLong(frac*Float(x[1]-x[0]));
		f[1]=MapClipFlagsZoomed(x[1],y[1],xc,yc);
	}

	if ((f[0]|f[1])&OFFBOTTOM)
	{
		if (f[0]&OFFBOTTOM)
		{
			temp=x[0]; x[0]=x[1]; x[1]=temp;
			temp=y[0]; y[0]=y[1]; y[1]=temp;
			temp2=f[0]; f[0]=f[1]; f[1]=temp2;
		}
		frac=Float(yc[1]-y[0])/Float(y[1]-y[0]);
		y[1]=yc[1];
		x[1]=x[0]+SLong(frac*Float(x[1]-x[0]));
		f[1]=MapClipFlagsZoomed(x[1],y[1],xc,yc);
	}

	if ((f[0]|f[1])&OFFLEFT)
	{
		if (f[0]&OFFLEFT)
		{
			temp=x[0]; x[0]=x[1]; x[1]=temp;
			temp=y[0]; y[0]=y[1]; y[1]=temp;
			temp2=f[0]; f[0]=f[1]; f[1]=temp2;
		}
		frac=Float(xc[0]-x[0])/Float(x[1]-x[0]);
		x[1]=xc[0];
		y[1]=y[0]+SLong(frac*Float(y[1]-y[0]));
		f[1]=MapClipFlagsZoomed(x[1],y[1],xc,yc);
	}

	if ((f[0]|f[1])&OFFRIGHT)
	{
		if (f[0]&OFFRIGHT)
		{
			temp=x[0]; x[0]=x[1]; x[1]=temp;
			temp=y[0]; y[0]=y[1]; y[1]=temp;
			temp2=f[0]; f[0]=f[1]; f[1]=temp2;
		}
		frac=Float(xc[1]-x[0])/Float(x[1]-x[0]);
		x[1]=xc[1];
		y[1]=y[0]+SLong(frac*Float(y[1]-y[0]));
		f[1]=MapClipFlagsZoomed(x[1],y[1],xc,yc);
	}
	return (f[0]|f[1])==0?true:false;
	return true;
}


void WorldToScreenXY( double& sx, double& sy, COORDS3D& pos )				//JON 8Nov00
{
	double	rhw = 1.f/double(Three_Dee.ViewerY());
	const double	sxScale = double(OverLay.physicalWidth)*.5f;
	const double	syScale = double(OverLay.physicalHeight)*.5f;

	sx = (double(pos.X - Three_Dee.ViewerX())*rhw*sxScale)+sxScale;
	sy = (double(pos.Z - Three_Dee.ViewerZ())*-rhw*syScale)+syScale;
}

// returns true if visible
inline bool ClipLineToScreen( double& sx, double& sy, double& ex, double& ey )		//JON 8Nov00
{
	bool retVal = false;

	const double sw = OverLay.physicalWidth;
	const double sh = OverLay.physicalHeight;

	// first generate a clipcode...
	const ULong CLIP_LEFT = 1;
	const ULong CLIP_RIGHT = 2;
	const ULong CLIP_TOP = 4;
	const ULong CLIP_BOTTOM = 8;

	ULong ccs = 0;
	ULong cce = 0;
	if ( sx < 0.f )
	{ // clip to left
		ccs+=CLIP_LEFT;
	}
	if ( ex < 0.f )
	{ // clip to left
		cce+=CLIP_LEFT;
	}
	if ( sx > sw )
	{ // clip to right
		ccs+=CLIP_RIGHT;
	}
	if ( ex > sw )
	{ // clip to right
		cce+=CLIP_RIGHT;
	}
	if ( sy < 0.f )
	{ // clip to top
		ccs+=CLIP_TOP;
	}
	if ( ey < 0.f )
	{ // clip to top
		cce+=CLIP_TOP;
	}
	if ( sy > sh )
	{ // clip to bottom
		ccs+=CLIP_BOTTOM;
	}
	if ( ey > sh )
	{ // clip to bottom
		cce+=CLIP_BOTTOM;
	}

	// right does the line appear at all
	if ( (cce&ccs) == 0 )
	{ // the line is visble
		retVal = true;
		if ( (cce|ccs) != 0 )
		{ // the line needs clipping
			//		y = mx+c
			//  =>	m = y/x
			//	=>	c = y-mx;
			//	=>	x =	(y-c)/m
			//	=>	x = (y-c)*rm
			const double m	= (ey-sy)/(ex-sx);
			const double rm	= 1.f/m;
			const double c	= sy - sx*m;

			if ( ccs&CLIP_LEFT )
			{ // clip to left
				sx = 0.f;
				sy = m*0.f+c;
			}
			if ( cce&CLIP_LEFT )
			{ // clip to left
				ex = 0.f;
				ey = m*0.f+c;
			}
			if ( ccs&CLIP_RIGHT )
			{ // clip to right
				sx = sw;
				sy = m*sw+c;
			}
			if ( cce&CLIP_RIGHT )
			{ // clip to right
				ex = sw;
				ey = m*sw+c;
			}
			if ( ccs&CLIP_TOP )
			{ // clip to top
				sy = 0.f;
				sx = (-c)*rm;
			}
			if ( cce&CLIP_TOP )
			{ // clip to top
				ey = 0.f;
				ex = (-c)*rm;
			}
			if ( ccs&CLIP_BOTTOM )
			{ // clip to bottom
				sy = sh;
				sx = (sh-c)*rm;
			}
			if ( cce&CLIP_BOTTOM )
			{ // clip to bottom
				ey = sh;
				ex = (sh-c)*rm;
			}
		}
	}
	return retVal;
}




//TEMPCODE JON 5/22/00 	struct SAircraftIconDef{
//TEMPCODE JON 5/22/00 		ImageMapNumber mapno;
//TEMPCODE JON 5/22/00 		SWord x,y,w,h;
//TEMPCODE JON 5/22/00 	};
//TEMPCODE JON 5/22/00 	static SAircraftIconDef targ_icon_def={MAPCHAPNO,0,16,16,16};
//TEMPCODE JON 5/22/00
//TEMPCODE JON 5/22/00 	const Float p_w=OverLay.physicalWidth;
//TEMPCODE JON 5/22/00 	const Float p_h=OverLay.physicalHeight;
//TEMPCODE JON 5/22/00
//TEMPCODE JON 5/22/00 	//generate clipping values in physical pixels
//TEMPCODE JON 5/22/00 	FRect clip;
//TEMPCODE JON 5/22/00
//TEMPCODE JON 5/22/00 	clip.left=Float(MAP_SCREEN_ORIGIN_X_640)*p_w/Float(640);
//TEMPCODE JON 5/22/00 	clip.right=Float(MAP_SCREEN_ORIGIN_X_640+MAP_SCREEN_WIDTH_640-1)*p_w/Float(640);
//TEMPCODE JON 5/22/00 	clip.top=Float(MAP_SCREEN_ORIGIN_Y_640)*p_h/Float(480);
//TEMPCODE JON 5/22/00 	clip.bottom=Float(MAP_SCREEN_ORIGIN_Y_640+MAP_SCREEN_HEIGHT_640-1)*p_h/Float(480);
//TEMPCODE JON 5/22/00
//TEMPCODE JON 5/22/00 	//convert input pixels 640 to physical screen pixels
//TEMPCODE JON 5/22/00
//TEMPCODE JON 5/22/00 	Float p_x,p_y;
//TEMPCODE JON 5/22/00
//TEMPCODE JON 5/22/00 	p_x=Float(x)*p_w/Float(640);
//TEMPCODE JON 5/22/00 	p_y=Float(y)*p_h/Float(480);
//TEMPCODE JON 5/22/00
//TEMPCODE JON 5/22/00 	//select icon to render
//TEMPCODE JON 5/22/00
//TEMPCODE JON 5/22/00 	R3DVERTEX dp[4];
//TEMPCODE JON 5/22/00 	ImageMapDesc* pmap;
//TEMPCODE JON 5/22/00 	SAircraftIconDef* selectedIcon=&targ_icon_def;
//TEMPCODE JON 5/22/00 	pmap=Image_Map.GetImageMapPtr(selectedIcon->mapno);
//TEMPCODE JON 5/22/00 	const SWord map_w=selectedIcon->w;
//TEMPCODE JON 5/22/00 	const SWord map_h=selectedIcon->h;
//TEMPCODE JON 5/22/00 	Float f_w_2,f_h_2;
//TEMPCODE JON 5/22/00 	f_w_2=Float(map_w>>1);
//TEMPCODE JON 5/22/00 	f_h_2=Float(map_h>>1);
//TEMPCODE JON 5/22/00 	dp[0].bodyx.f=dp[3].bodyx.f=int(p_x-f_w_2);
//TEMPCODE JON 5/22/00 	dp[0].ix=dp[3].ix=selectedIcon->x;
//TEMPCODE JON 5/22/00 	dp[1].bodyx.f=dp[2].bodyx.f=dp[3].bodyx.f+map_w;
//TEMPCODE JON 5/22/00 	dp[1].ix=dp[2].ix=selectedIcon->x+map_w;
//TEMPCODE JON 5/22/00 	dp[0].bodyy.f=dp[1].bodyy.f=int(p_y-f_h_2);
//TEMPCODE JON 5/22/00 	dp[0].iy=dp[1].iy=selectedIcon->y;
//TEMPCODE JON 5/22/00 	dp[2].bodyy.f=dp[3].bodyy.f=dp[1].bodyy.f+map_h;
//TEMPCODE JON 5/22/00 	dp[2].iy=dp[3].iy=selectedIcon->y+map_h;
//TEMPCODE JON 5/22/00 	OverLay.ClippedPutC(dp,pmap,clip);


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		ShowItemIcons
//Author		Paul.
//Date			Tue 2 Mar 1999
//------------------------------------------------------------------------------
void MapScr::ShowItemIcons(bool flag)
{
	//flag==true if on waypoint screen & false if not

	//display the map background and generate the
	//parameters needed to calculate the screen coordinates
	//for anything else that needs to go on the map

//#pragma message(__HERE__"Comment this block in when Rod defines the key")
	if (Key_Tests.KeyPress3d(SEEMIGS))
	{
			Save_Data.gamedifficulty^=GD_VISIBLEMIGSCHEAT;
	}

}

void MapScr::ShowItemIconsZoomed(bool flag)
{
//	ShowTargetLocation(OverLay.target_item,x,y);
}

MESSAGE_STRUC* localMsg;
DecisionAI* localDec;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		ShowPositionIndicators
//Author		Paul.   / Jon
//				re written by Jon 11Aug00
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
void MapScr::ShowPositionIndicators()
{
	//3 position indicators (1) block start,
	//						(2) current position
	//						(3) block end



	SWord _TopY=40;
	SWord _LeftX=8;
	SWord _Width=368;
	SWord _Height=16;
	SWord _Height_2=_Height>>1;
	SWord _Height_3_2=_Height_2+(_Height_2>>1);

	OverLay.startMarker=_Replay.BeginMarkPos;					//AMM 24Feb99
	OverLay.endMarker=_Replay.EndMarkPos;						//AMM 24Feb99
	OverLay.curPos=_Replay.CurrPos;								//AMM 24Feb99

	SLong startBlockX=OverLay.startMarker;
	SLong endBlockX=OverLay.endMarker;
	SLong currentPosX=OverLay.curPos;

	startBlockX=((startBlockX*_Width)>>8)+_LeftX;
	endBlockX=((endBlockX*_Width)>>8)+_LeftX;
	currentPosX=((currentPosX*_Width)>>8)+_LeftX;


	float scx = float(OverLay.physicalWidth)/640.0;
	float scy = float(OverLay.physicalHeight)/480.0;
#define SCALE_ME_X( arg ) arg=float(float(arg)*scx)
#define SCALE_ME_Y( arg ) arg=float(float(arg)*scy)

	SCALE_ME_Y( _TopY );
	SCALE_ME_Y( _Height );
	SCALE_ME_Y( _Height_2 );
	SCALE_ME_X( startBlockX );
	SCALE_ME_X( currentPosX );
	SCALE_ME_X( endBlockX );

	OverLay.DrawLine(startBlockX,_TopY+_Height-_Height_3_2,startBlockX,_TopY+_Height,Colour(21));
	OverLay.DrawLine(endBlockX,_TopY+_Height-_Height_3_2,endBlockX,_TopY+_Height,Colour(21));
	OverLay.DrawLine(currentPosX,_TopY+_Height-_Height_2,currentPosX,_TopY+_Height,Colour(31));

//TempCode JON 31Oct00 	SWord x = currentPosX;
//TempCode JON 31Oct00 	SWord y = _TopY+_Height;
//TempCode JON 31Oct00 	OverLay.PrintAt2( x, y, "Pos" );
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		HandleUpDnLftRgtToggleKeys
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
void MapScr::HandleUpDnLftRgtToggleKeys()
{
	//find the currently hilighted option

	bool abort=false;

	OptionList* pCurOpt=optionList;

	while (pCurOpt->key!=SEL_EOL && pCurOpt->key!=OverLay.hilight) pCurOpt++;

	if (pCurOpt->key==SEL_EOL || OverLay.hilight==SEL_NONE)
	{
		pCurOpt=optionList;
		while (pCurOpt->key!=SEL_EOL && pCurOpt->key!=SEL_BLANK) pCurOpt++;
		if (pCurOpt->key==SEL_EOL)
			return;
	}

	if (Key_Tests.KeyPress3d(ROTUP) ||
		Key_Tests.KeyPress3d(ROTLEFT))
	{
		//move the selected hilight option up

		if (uintptr_t(--pCurOpt)<uintptr_t(optionList))
		{
			pCurOpt=optionList;
			while (pCurOpt[1].key!=SEL_EOL) pCurOpt++;
		}
		abort=false;
		while (pCurOpt->key==SEL_BLANK)
		{
			if (uintptr_t(--pCurOpt)<uintptr_t(optionList))
			{
				if (abort) return;
				pCurOpt=optionList;
				while (pCurOpt[1].key!=SEL_EOL) pCurOpt++;
				abort=true;
			}
		}
		OverLay.hilight=(Key)pCurOpt->key;
	}
	else if (Key_Tests.KeyPress3d(ROTDOWN) ||
			 Key_Tests.KeyPress3d(ROTRIGHT))
	{
		//move the selected hilight option down

		pCurOpt++;
		if (pCurOpt->key==SEL_EOL || pCurOpt->key==SEL_BLANK)
		{
			if (pCurOpt->key==SEL_EOL)	pCurOpt=optionList;

			while (pCurOpt->key!=SEL_BLANK)
			{
				pCurOpt++;
				if (pCurOpt->key==SEL_EOL)
				{
					if (abort) return;
					abort=true;
					pCurOpt=optionList;
				}
			}
		}
		OverLay.hilight=(Key)pCurOpt->key;
	}
	else if (Key_Tests.KeyPress3d(SHOOT))
	{
		//call the select routine for the current hilighted option
		if (pCurOpt->selRtn!=NULL)
			OverLay.pNewScr=(this->*(pCurOpt->selRtn))(pCurOpt->resIDOffset,(Key)pCurOpt->key);
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		FirstMapInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::FirstMapInit()
{
	OverLay.fZoomedMap=false;
	if (Manual_Pilot.ControlledAC2 &&
		Manual_Pilot.ControlledAC2->waypoint)
		OverLay.curr_waypoint=Manual_Pilot.ControlledAC2->waypoint;
	else
		OverLay.curr_waypoint=NULL;
	_MsgBuffer.InitMessageWindow();
	if (!_DPlay.Implemented)
		OverLay.keyFlags=KF_PAUSEON;
	return SEL_0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromFirstMap
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromFirstMap(UByte&,Key k)
{
	MapScr* retval=this;

	switch (k)
	{
	case SEL_1:
		retval=&COverlay::accelMapScr;
		OverLay.fZoomedMap=false;
		break;
	case SEL_2:
		OverLay.SetPossibleLandscapeResfresh();
		retval=&COverlay::waypointMapScr;
		break;
	case SEL_3:
		if ( Manual_Pilot.ControlledAC2 && Manual_Pilot.ControlledAC2->classtype->aerobaticfactor != AEROBATIC_LOW )
			retval=&COverlay::radioMapScr;
		break;
	case SEL_4:
		OverLay.DoPossibleLandscapeRefresh();
		OverLay.fZoomedMap=OverLay.fZoomedMap?false:true;
		break;
	case SEL_0:
	case SEL_ESC:
		OverLay.SetPossibleLandscapeResfresh();							//JON 9Nov00
		OverLay.keyFlags=KF_PAUSEOFF|KF_ACCELOFF;
		OverLay.pvp->ReturnFromMap();
		retval=NULL;
		break;
	default:
		break;
	}
	return retval;
}

MapScr COverlay::firstMapScr=
{
	MapScr::SCALE_GRIDORIGIN+/*MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE+
	MapScr::FLAG_MOUSEOVERHI,
	MapScreenX,MapScreenY,
	MapScreenWidth,MapScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),//NO_BACKGROUND_WIPE,//if the alpha part of this ulong is null then it'll draw it as a colour
	// otherwise it'll assume it is a imagemap filenum
	TEXT_MAP_BLACK,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::FirstMapInit,
	MYPREFIX MapScr::UpdateMessageDisplay,
	{
		{IDS_MAP_ACCEL,0,SEL_1,MYPREFIX MapScr::SelectFromFirstMap},
		{IDS_MAP_WAYPOINTS,0,SEL_2,MYPREFIX MapScr::SelectFromFirstMap},
		{IDS_MAP_RADIO,0,SEL_3,MYPREFIX MapScr::SelectFromFirstMap},
		{IDS_MAP_ZOOM,0,SEL_4,MYPREFIX MapScr::SelectFromFirstMap},
		MSBlankLine,
		MSEscapeLine(MYPREFIX MapScr::SelectFromFirstMap),
		{IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromFirstMap},
		MSListEnd
	}
};


MapScr COverlay::firstMapScrBomber=
{
	MapScr::SCALE_GRIDORIGIN+/*MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE+
	MapScr::FLAG_MOUSEOVERHI,
	MapScreenX,MapScreenY,
	MapScreenWidth,MapScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),//NO_BACKGROUND_WIPE,//if the alpha part of this ulong is null then it'll draw it as a colour
	// otherwise it'll assume it is a imagemap filenum
	TEXT_MAP_BLACK,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::FirstMapInit,
	MYPREFIX MapScr::UpdateMessageDisplay,
	{
		{IDS_MAP_ACCEL,0,SEL_1,MYPREFIX MapScr::SelectFromFirstMap},
		{IDS_MAP_WAYPOINTS,0,SEL_2,MYPREFIX MapScr::SelectFromFirstMap},
//		{IDS_MAP_RADIO,0,SEL_3,MYPREFIX MapScr::SelectFromFirstMap},
		MSBlankLine,
		{IDS_MAP_ZOOM,0,SEL_4,MYPREFIX MapScr::SelectFromFirstMap},
		MSBlankLine,
		MSEscapeLine(MYPREFIX MapScr::SelectFromFirstMap),
		{IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromFirstMap},
		MSListEnd
	}
};


/*******************
**	ORDERS SCREEN **
*******************/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SendMessageAndCall
//Author		Paul.
//Date			Thu 1 Apr 1999
//------------------------------------------------------------------------------
void COverlay::SendMessageAndCall(MESSAGE_STRUC* msg,DecisionAI* dec)
{
	if (msg->callee==Persons2::PlayerSeenAC)					  //RDH 17/05/99
		msg->callee=Persons2::PlayerGhostAC;					  //RDH 17/05/99
	if (msg->caller==Persons2::PlayerSeenAC)					  //RDH 17/05/99
		msg->caller=Persons2::PlayerGhostAC;					  //RDH 17/05/99
	if (msg->target==Persons2::PlayerSeenAC)					  //RDH 17/05/99
		msg->target=Persons2::PlayerGhostAC;					  //RDH 17/05/99

	if (dec)
		dec->UserMsgDecision((AirStruc*)msg->callee,msg->target,(AirStruc*)msg->callee,&Art_Int,msg);
	msg->SetPriority(PRI_MAX);
	_Radio.TriggerMsg(*msg);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		MsgOptionSelected
//Author		Paul.
//Date			Thu 1 Apr 1999
//------------------------------------------------------------------------------
void COverlay::MsgOptionSelected(MESSAGE_STRUC* msg,SLong op)
{
	DecisionAI* dec=msg->decisionhandler;

	DecisionMessage(dec,op,msg->callee,msg->target,msg->callee);

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		ActionMessage
//Author		Paul
//Date			Tue 9 Mar 1999
//------------------------------------------------------------------------------
void COverlay::ActionMessage(MESSAGE_STRUC* msg)
{
//TempCode JON 22Sep00 #pragma warnmsg ("RadioChat: Action Message commented out. UMM... NOW ITS NOT. PLEASE REMOVE THESE MESSAGES WHEN YOU HAVE FINISHED WITH THEM!")

	MapScr::OptionList termOpt=MSListEnd;
	MapScr::OptionList escOpt=MSEscapeLine(MYPREFIX MapScr::SelectFrom3DOrders);
	MapScr::OptionList blankOpt=MSBlankLine;
	MapScr::OptionList miscOpt={IDS_INDIRECT00,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders};

	Kludge();
	if (!Manual_Pilot.ControlledAC2->Status.deadtime &&
		!pCurScr)
	{
		localMsg=msg;

		if (msg->decisionhandler)
		{
			DecisionAI* dec=msg->decisionhandler;
			DecisionAI::OptionRef* opt=dec->GetMsgOptions();
			MapScr* map=&orders3dScr;
			MapScr::OptionList* ol=map->optionList;
			SLong oc=0;
			while (opt->optionmsg!=PHRASE_NULL && oc<9)
			{
				miscOpt.resID=_Radio.PanelPhraseToResource(opt->optionmsg);	//RJS 18Sep00
				*ol++=miscOpt;
				miscOpt.key++;
				opt++;
				oc++;
			}
			while (oc++<9)	*ol++=blankOpt;
			*ol++=escOpt;
			*ol=termOpt;

			//trigger UI display @ end of the current frame
			SetToOrdersScreen();
		}
	}
}

EXTERNAI(GroupInfoMsg,		 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(GroupInfoMsgFolRaf, AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(GroupInfoMsgFolLuf, AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(PreCombatMsg,		 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(PreCombatMsgLuf,	 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(CombatMsg,			 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(CombatMsgFolRaf,	 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(CombatMsgFolLuf,	 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(PostCombatMsg,		 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(TowerMsg,			 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(Strike,			 AirStrucPtr);							  //CSB 11/02/00
EXTERNAI(QuitContinue,		 AirStrucPtr);							  //CSB 11/02/00

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		UserMsgInit
//------------------------------------------------------------------------------
Key MapScr::UserMsgInit()
{
	return SEL_0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromUserMsg
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromUserMsg(UByte&,Key k)
{
	MapScr* retval=NULL;	//close on selection of an option
	localDec=NULL;

	AirStrucPtr player = Persons2::PlayerGhostAC;

	bool toweronly = false;
		if(		((Persons2::PlayerSeenAC->movecode == AUTO_TRAININGTAKEOFF)	|| (Persons2::PlayerSeenAC->movecode == AUTO_TRAININGLANDING))
			||	(Art_Int.CountFormationSize(Persons2::PlayerGhostAC) == 1)	)
		toweronly = true;

	switch (k)
	{
		case SEL_1:
			localDec = NULL;
			if(!toweronly)
			{
				if(player->formpos == 0)				localDec = GroupInfoMsg;
				else
					if(player->nationality == NAT_RAF)	localDec = GroupInfoMsgFolRaf;
					else								localDec = GroupInfoMsgFolLuf;
			}
			break;

		case SEL_2:
			localDec = NULL;
			if(!toweronly)
			{
				if(player->formpos == 0)
					if(player->nationality == NAT_RAF)	localDec = PreCombatMsg;
					else								localDec = PreCombatMsgLuf;
			}
			break;

		case SEL_3:
			localDec = NULL;
			if(!toweronly)
			{
				if(player->formpos == 0)				localDec = CombatMsg;
				else
					if(player->nationality == NAT_RAF)	localDec = CombatMsgFolRaf;
					else								localDec = CombatMsgFolLuf;
			}
			break;

		case SEL_4:
			localDec = NULL;
			if(!toweronly)
				if(player->formpos == 0)				localDec = PostCombatMsg;
			break;

		case SEL_5:
			localDec = TowerMsg;
		break;

		case SEL_6:
			localDec = NULL;
			if(!toweronly)
				if((player->formpos == 0) && ((player->classtype->visible == JU87) || (player->classtype->visible == ME110)))
					localDec = Strike;
		break;

		case SEL_7:

			if (_DPlay.Implemented)//AMM20May
				retval=&COverlay::commsRecipientOrdersScr;
			localDec = NULL;
			break;

		case SEL_8:
			localDec=NULL;
			if (_DPlay.Implemented)//AMM20May
				retval = &COverlay::commsMsgOrdersScr;					//AMM 14May99
			break;

		default:
			break;
	}

	if (localDec!=NULL)
	{
		retval=&COverlay::userOptionsScr;
	}
	return retval;
}

MapScr COverlay::commsChatDialog=
{
	MapScr::JUSTIFY_CENTRE_SX+MapScr::SCALE_GRIDWH,
	MapScreenX,MapScreenY,
	CommsChatWidth,CommsChatHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	2,5,
	0x0000,
	MYPREFIX MapScr::CommsTextInit,
	MYPREFIX MapScr::CommsTextUpdateDisplay,
	{
		MSEscapeLine(MYPREFIX MapScr::SelectCommsChat),
		MSEscapeLine(MYPREFIX MapScr::SelectCommsChat),
		MSEscapeLine(MYPREFIX MapScr::SelectCommsChat),
		MSEscapeLine(MYPREFIX MapScr::SelectCommsChat),
		MSEscapeLine(MYPREFIX MapScr::SelectCommsChat),
		MSEscapeLine(MYPREFIX MapScr::SelectCommsChat),
		MSEscapeLine(MYPREFIX MapScr::SelectCommsChat),
		MSEscapeLine(MYPREFIX MapScr::SelectCommsChat),
		{IDS_COMMSDIALOK,		0,	SEL_ENTER,	MYPREFIX MapScr::SelectCommsChat},
		{IDS_COMMSDIALCANCEL,	0,	SEL_0,		MYPREFIX MapScr::SelectCommsChat},
		MSListEnd
	}
};

MapScr COverlay::userMsgScrLead=										//CSB 23/02/00
{
	MapScr::JUSTIFY_CENTRE_SX+ /*MapScr::SCALE_GRIDWH+*/
/*	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::Orders3DInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		{IDS_UM_GROUPINFO,	0,	SEL_1,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_PRECOMBAT,	0,	SEL_2,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_COMBAT,		0,	SEL_3,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_POSTCOMBAT,	0,	SEL_4,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_TOWER,		0,	SEL_5,	MYPREFIX MapScr::SelectFromUserMsg},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		{IDS_MAP_EXIT,		0,	SEL_0,	MYPREFIX MapScr::SelectFromUserMsg},
		MSListEnd
	}
};

MapScr COverlay::userMsgScrLeadBomb=									//CSB 23/02/00
{
	MapScr::JUSTIFY_CENTRE_SX+ /*MapScr::SCALE_GRIDWH+*/
/*	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::Orders3DInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		{IDS_UM_GROUPINFO,	0,	SEL_1,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_PRECOMBAT,	0,	SEL_2,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_COMBAT,		0,	SEL_3,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_POSTCOMBAT,	0,	SEL_4,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_TOWER,		0,	SEL_5,	MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_FAC,		0,	SEL_6,	MYPREFIX MapScr::SelectFromUserMsg},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		{IDS_MAP_EXIT,		0,	SEL_0,	MYPREFIX MapScr::SelectFromUserMsg},
		MSListEnd
	}
};

MapScr COverlay::userMsgScrFoll=										//CSB 23/02/00
{
	MapScr::JUSTIFY_CENTRE_SX+ /*MapScr::SCALE_GRIDWH+*/
/*	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::Orders3DInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		{IDS_UM_GROUPINFO,	0,	SEL_1,	MYPREFIX MapScr::SelectFromUserMsg},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		{IDS_UM_COMBAT,		0,	SEL_3,	MYPREFIX MapScr::SelectFromUserMsg},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		{IDS_UM_TOWER,		0,	SEL_5,	MYPREFIX MapScr::SelectFromUserMsg},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		{IDS_MAP_EXIT,		0,	SEL_0,	MYPREFIX MapScr::SelectFromUserMsg},
		MSListEnd
	}
};

MapScr COverlay::userMsgScrTower=										//CSB 23/02/00
{
	MapScr::JUSTIFY_CENTRE_SX+ /*MapScr::SCALE_GRIDWH+*/
/*	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::Orders3DInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		{IDS_UM_TOWER,		0,	SEL_5,	MYPREFIX MapScr::SelectFromUserMsg},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		{IDS_MAP_EXIT,		0,	SEL_0,	MYPREFIX MapScr::SelectFromUserMsg},
		MSListEnd
	}
};

MapScr COverlay::userMsgScrCOMMS=
{
	MapScr::JUSTIFY_CENTRE_SX+/*MapScr::SCALE_GRIDWH+
	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::Orders3DInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		{IDS_UM_GROUPINFO,0,SEL_1,MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_PRECOMBAT,0,SEL_2,MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_COMBAT,0,SEL_3,MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_POSTCOMBAT,0,SEL_4,MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_TOWER,0,SEL_5,MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_FAC,0,SEL_6,MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_COMMSPLAYER,0,SEL_7,MYPREFIX MapScr::SelectFromUserMsg},
		{IDS_UM_COMMSMSG,0,SEL_8,MYPREFIX MapScr::SelectFromUserMsg},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserMsg),
		{IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromUserMsg},
		MSListEnd
	}
};

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		UserOptionsInit
//------------------------------------------------------------------------------
Key MapScr::UserOptionsInit()
{
//TempCode JON 22Sep00 	#pragma warnmsg ("RadioChat: UserOptionsInit commented out")
	MapScr::OptionList termOpt=MSListEnd;
	MapScr::OptionList escOpt=MSEscapeLine(MYPREFIX MapScr::SelectFromUserOptions);
	MapScr::OptionList blankOpt=MSBlankLine;
	MapScr::OptionList quitOpt={IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromUserOptions};
	MapScr::OptionList miscOpt={IDS_INDIRECT00,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions};

	DecisionAI* dec=localDec;
	DecisionAI::OptionRef* opt=dec->GetMsgOptions();
	MapScr* map=this;
	MapScr::OptionList* ol=map->optionList;

	SLong oc=0;
	while (opt->optionmsg!=PHRASE_NULL && oc<9)			//RJS 02Feb00
	{
		miscOpt.resID=_Radio.PanelPhraseToResource(opt->optionmsg);		//RJS 18Sep00
		*ol++=miscOpt;
		miscOpt.key++;
		opt++;
		oc++;
	}
	while (oc++<9)	*ol++=blankOpt;
	*ol++=escOpt;
	*ol++=quitOpt;
	*ol=termOpt;

	return SEL_0;

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		UserOptsMapInit
//------------------------------------------------------------------------------
Key MapScr::UserOptsMapInit()
{
	MapScr::OptionList termOpt=MSListEnd;
	MapScr::OptionList escOpt=MSEscapeLine(MYPREFIX MapScr::SelectFromUserOptsMap);
	MapScr::OptionList quitOpt={IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromUserOptsMap};
	MapScr::OptionList miscOpt={IDS_INDIRECT00,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap};

	DecisionAI* dec=localDec;
	DecisionAI::OptionRef* opt=dec->GetMsgOptions();
	MapScr* map=this;
	MapScr::OptionList* ol=map->optionList;

	SLong oc=0;
	while (opt->optionmsg!=PHRASE_NULL && oc<9)			//RJS 02Feb00
	{
		miscOpt.resID=_Radio.PanelPhraseToResource(opt->optionmsg);		//RJS 18Sep00
		*ol++=miscOpt;
		miscOpt.key++;
		opt++;
		oc++;
	}
//DEAD	while (oc++<9)	*ol++=blankOpt;
	*ol++=escOpt;
	*ol++=quitOpt;
	*ol=termOpt;

		return SEL_0;

}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromUserOptions
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromUserOptsMap(UByte& a,Key k)
{
	SelectFromUserOptions(a,k);

	if ( Manual_Pilot.ControlledAC2 && Manual_Pilot.ControlledAC2->classtype->aerobaticfactor == AEROBATIC_LOW )
		return &COverlay::firstMapScrBomber;
	else
		return &COverlay::firstMapScr;


}

MapScr COverlay::userOptsMapScr=
{
	MapScr::SCALE_GRIDORIGIN+/*MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE+
	MapScr::FLAG_MOUSEOVERHI,
	MapScreenX,MapScreenY,
	MapScreenWidth,MapScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),//NO_BACKGROUND_WIPE,//if the alpha part of this ulong is null then it'll draw it as a colour
	// otherwise it'll assume it is a imagemap filenum
	TEXT_MAP_BLACK,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows+2,
	0x0000,
	MYPREFIX MapScr::UserOptsMapInit,
	MYPREFIX MapScr::UpdateMessageDisplay,
	{
		{IDS_INDIRECT00,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		{IDS_INDIRECT01,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		{IDS_INDIRECT02,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		{IDS_INDIRECT03,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		{IDS_INDIRECT04,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		{IDS_INDIRECT05,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		{IDS_INDIRECT06,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		{IDS_INDIRECT07,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		{IDS_INDIRECT08,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptsMap},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserOptsMap),
		{IDS_DEFAULT,0,SEL_0,MYPREFIX MapScr::SelectFromUserOptsMap},
		MSListEnd
	}
};

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromUserOptions
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromUserOptions(UByte&,Key k)
{
	MapScr* retval=NULL;	//close on selection of an option
	DecisionAI* dec=NULL;

	if (localMsg && localMsg->decisionhandler)	dec=localMsg->decisionhandler;
	else if (localDec)							dec=localDec;

	localMsg=NULL;
	localDec=NULL;

	if (dec)
	{

		if (k>=SEL_1 && k<=SEL_9)
		{
				OverLay.DecisionMessage(dec,
								(SWord)(k-SEL_1),
								Persons2::PlayerGhostAC,
								Persons2::PlayerGhostAC->ai.unfriendly,
								Persons2::PlayerGhostAC);

			retval=NULL;
		}
	}
	return retval;
}

MapScr COverlay::userOptionsScr=
{
	MapScr::JUSTIFY_CENTRE_SX+/*MapScr::SCALE_GRIDWH+
	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	1,15,
	0x0000,
	MYPREFIX MapScr::UserOptionsInit,
	NULL,
	{
		{IDS_INDIRECT00,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		{IDS_INDIRECT01,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		{IDS_INDIRECT02,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		{IDS_INDIRECT03,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		{IDS_INDIRECT04,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		{IDS_INDIRECT05,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		{IDS_INDIRECT06,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		{IDS_INDIRECT07,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		{IDS_INDIRECT08,0,SEL_1,MYPREFIX MapScr::SelectFromUserOptions},
		MSEscapeLine(MYPREFIX MapScr::SelectFromUserOptions),
		{IDS_DEFAULT,0,SEL_0,MYPREFIX MapScr::SelectFromUserOptions},
		MSListEnd
	}
};

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		Orders3DInit
//Author		Paul.
//Date			Mon 22 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::Orders3DInit()
{
	OverLay.displayTimeLimit=8*ONE_SECOND_TIME;
 	return SEL_0;
}

Key MapScr::CommsTextInit()
{
	return SEL_0;
}

void MapScr::CommsTextUpdateDisplay()
{

}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		TimeLimitedDisplay
//Author		Paul.
//Date			Mon 22 Feb 1999
//------------------------------------------------------------------------------
void MapScr::TimeLimitedDisplay()
{
	//decrement timer and select default
	//option if time has expired

 	OverLay.displayTimeLimit-=OverLay.pvp->getFrameTimeMSecs();

	if (OverLay.displayTimeLimit<=0)
	{
		OptionList* pCurOpt=optionList;

		while (pCurOpt->key!=SEL_0 && pCurOpt->key!=SEL_ESC && pCurOpt->key!=SEL_EOL)
			pCurOpt++;

		if (pCurOpt->key==SEL_ESC && pCurOpt->selRtn!=NULL)
			OverLay.pNewScr=(this->*(pCurOpt->selRtn))(pCurOpt->resIDOffset,SEL_ESC);
		else if (pCurOpt->key==SEL_0 && pCurOpt->selRtn!=NULL)
			OverLay.pNewScr=(this->*(pCurOpt->selRtn))(pCurOpt->resIDOffset,SEL_0);
		else if (pCurOpt->key==SEL_ESC && pCurOpt->selRtn!=NULL)
			OverLay.pNewScr=(this->*(pCurOpt->selRtn))(pCurOpt->resIDOffset,SEL_ESC);

		OverLay.displayTimeLimit=0;
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFrom3DOrders
//Author		Paul.
//Date			Mon 22 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFrom3DOrders(UByte&,Key k)
{
	MapScr* retval=this;	//close on selection of an option
	DecisionAI* dec=NULL;

	MESSAGE_STRUC* theMsg=localMsg;
	localMsg=NULL;

	if (theMsg && theMsg->decisionhandler)
		dec=theMsg->decisionhandler;

	if (dec)
	{

		if (k>=SEL_1 && k<=SEL_9)
		{
			OverLay.MsgOptionSelected(theMsg,SLong(k-SEL_1));
			retval=NULL;
		}
		else
		{
			DecisionAI* dec=theMsg->decisionhandler;
			DecisionAI::OptionRef* opt=dec->GetMsgOptions();
			PhraseTables defPh=dec->GetDefaultPhrase(theMsg->callee,theMsg->target);
			SLong sel=0;
			while (opt->optionmsg!=PHRASE_NULL &&			//RJS 02Feb00
					opt->optionmsg!=defPh && sel<9)
			{
				opt++;
				sel++;
			}

			if (sel<9 && opt->optionmsg!=PHRASE_NULL)		//RJS 02Feb00
			{
				OverLay.MsgOptionSelected(theMsg,sel);
				retval=NULL;
			}
		}
	}
	else retval=NULL;
	return retval;
}

MapScr COverlay::orders3dScr=
{
	MapScr::JUSTIFY_CENTRE_SX+/*MapScr::SCALE_GRIDWH+
	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::Orders3DInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		{IDS_INDIRECT00,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		{IDS_INDIRECT01,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		{IDS_INDIRECT02,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		{IDS_INDIRECT03,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		{IDS_INDIRECT04,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		{IDS_INDIRECT05,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		{IDS_INDIRECT06,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		{IDS_INDIRECT07,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		{IDS_INDIRECT08,0,SEL_1,MYPREFIX MapScr::SelectFrom3DOrders},
		MSEscapeLine(MYPREFIX MapScr::SelectFrom3DOrders),
		{IDS_DEFAULT,0,SEL_0,MYPREFIX MapScr::SelectFrom3DOrders},
		MSListEnd
	}
};

// continue quit screen 												//JON 31Oct00
MapScr COverlay::continueQuitScr=
{
	MapScr::JUSTIFY_CENTRE_SX+/*MapScr::SCALE_GRIDWH+
	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::ContinueQuitInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		{IDS_CONTINUE,0,SEL_1,MYPREFIX MapScr::SelectFromContinueQuit},
		{IDS_L_CPS_QUIT,0,SEL_2,MYPREFIX MapScr::SelectFromContinueQuit},
		MSEscapeLine(MYPREFIX MapScr::SelectFromContinueQuit),
	//	{IDS_DEFAULT,0,SEL_0,MYPREFIX MapScr::SelectFrom3DOrders},
		MSListEnd
	}
};

Key MapScr::ContinueQuitInit()
{
	OverLay.displayTimeLimit=10*ONE_SECOND_TIME;
//	OverLay.hilight=;
	return SEL_1;
}

MapScr* MapScr::SelectFromContinueQuit(UByte&,Key k)
{
	MapScr* retVal;
	switch ( k )
	{
		case SEL_2:
			_DPlay.SimulateExitKey = TRUE; // quit the 3d...
		case SEL_1:case SEL_ESC:
			retVal = NULL;
			break;
		default:
			retVal = this;
	}
	return retVal;
}


/**********************
**	ACCEL MAP SCREEN **
**********************/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		COverlay.CancelAccel
//Author		Paul.
//Date			Thu 4 Mar 1999
//------------------------------------------------------------------------------
void COverlay::CancelAccel()
{
	if (!pvp || !pvp->Accel	  ())
		return;
	DoPossibleLandscapeRefresh();
	accelcountdown=0;												  //JIM 12/07/99
	UByte dummy;
	if (pCurScr) pCurScr=pNewScr=accelMapScr.SelectFromAccelMap(dummy,SEL_0);
	else
	{
		OverLay.keyFlags|=KF_ACCELOFF;
		Manual_Pilot.AutoToggle(ManualPilot::MANUAL);
	}
	if (otherText[0]!="")
	{
		otherText[0]="";
		messageTimer=0;
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		AccelMapInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::AccelMapInit()
{
	//build list of display options based on whether movecode is autofollowwp or not
	MapScr::OptionList termOpt=MSListEnd;
	MapScr::OptionList escOpt=MSEscapeLine(MYPREFIX MapScr::SelectFromAccelMap);
	MapScr::OptionList blankOpt=MSBlankLine;
	MapScr::OptionList quitOpt={IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromAccelMap};
	MapScr::OptionList fwdOpt={IDS_MAP_FORWARD,0,SEL_1,MYPREFIX MapScr::SelectFromAccelMap};
	MapScr::OptionList wayptOpt={IDS_MAP_NEXTWAYPOINT,0,SEL_2,MYPREFIX MapScr::SelectFromAccelMap};
	MapScr::OptionList msubjOpt={IDS_MAP_MESSAGESUBJECT,0,SEL_3,MYPREFIX MapScr::SelectFromAccelMap};
	MapScr::OptionList missipOpt={IDS_MAP_MISSIONIP,0,SEL_4,MYPREFIX MapScr::SelectFromAccelMap};
	MapScr::OptionList homeOpt={IDS_MAP_HOME,0,SEL_5,MYPREFIX MapScr::SelectFromAccelMap};

	bool allowall=true;
	bool allowip=true;

//	if (Manual_Pilot.ControlledAC2)
//	{
	if (Persons2::PlayerGhostAC && Persons2::PlayerGhostAC->movecode!=AUTO_FOLLOWWP)
			allowall=false;
	else
	if (Manual_Pilot.ControlledAC2 && Manual_Pilot.ControlledAC2->waypoint==NULL)//JIM 14Jul99
			allowall=false;														//JIM 14Jul99
	else																		//JIM 14Jul99
	if (Manual_Pilot.ControlledAC2 && Manual_Pilot.ControlledAC2->waypoint &&
			Manual_Pilot.ControlledAC2->waypoint->wpname>WPNAME_IP)
			allowip=false;
//	}
	if (_DPlay.Implemented) allowall=false;

	OptionList* ol=this->optionList;
	*ol++=fwdOpt;
	*ol++=(allowall)?wayptOpt:blankOpt;
	*ol++=(allowall)?msubjOpt:blankOpt;
	*ol++=(allowall && allowip)?missipOpt:blankOpt;
	*ol++=(allowall)?homeOpt:blankOpt;
	*ol++=blankOpt;
	*ol++=blankOpt;
	*ol++=blankOpt;
	*ol++=escOpt;
	*ol++=quitOpt;
	*ol++=termOpt;

	if (_DPlay.Implemented)
		OverLay.SetPossibleLandscapeResfresh();
	OverLay.fZoomedMap=false;
	return SEL_0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromAccelMap
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromAccelMap(UByte&,Key k)
{
	MapScr* retval=this;

	Manual_Pilot.acceltochanged=NULL;								  //JIM 18/04/99
	switch (k)
	{
	case SEL_0:
	case SEL_ESC:
		UByte dummy;
		if (!_DPlay.Implemented)
			OverLay.keyFlags=KF_PAUSEON+KF_SLOWACCEL;

		OverLay.keyFlags|=KF_ACCELOFF;
		Manual_Pilot.AutoToggle(ManualPilot::MANUAL);
		retval=SelectFromFirstMap(dummy,k);
		break;
	case SEL_2:	//to next waypoint
		OverLay.AccelSelection(k,DPlay::ACCEL_NEXTWP);
		break;
	case SEL_1:	//forwards
		//force the selected option to be hilighted
		OverLay.AccelSelection(k,DPlay::ACCEL_FORWARD);
		break;
	case SEL_3:	//message subject
		//force the selected option to be hilighted
		OverLay.AccelSelection(k,DPlay::ACCEL_MESSAGESUBJECT);
		break;
	case SEL_4:	//mission IP
		//force the selected option to be hilighted
		OverLay.AccelSelection(k,DPlay::ACCEL_MISSIONIP);
		break;
	case SEL_5:	//home
		//force the selected option to be hilighted
		OverLay.AccelSelection(k,DPlay::ACCEL_HOME);
		break;
	default:
		break;
	}
	return retval;
}

MapScr COverlay::accelMapScr=
{
	MapScr::SCALE_GRIDORIGIN+/*MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE+
	MapScr::FLAG_MOUSEOVERHI,
	MapScreenX,MapScreenY,
	MapScreenWidth,MapScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_BLACK,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::AccelMapInit,
	MYPREFIX MapScr::UpdateMessageDisplay,
	{
		{IDS_MAP_FORWARD,0,SEL_1,MYPREFIX MapScr::SelectFromAccelMap},
		{IDS_MAP_NEXTWAYPOINT,0,SEL_2,MYPREFIX MapScr::SelectFromAccelMap},
		{IDS_MAP_MESSAGESUBJECT,0,SEL_3,MYPREFIX MapScr::SelectFromAccelMap},
		{IDS_MAP_MISSIONIP,0,SEL_4,MYPREFIX MapScr::SelectFromAccelMap},
		{IDS_MAP_HOME,0,SEL_5,MYPREFIX MapScr::SelectFromAccelMap},
		MSBlankLine,
		MSBlankLine,
		MSBlankLine,
		MSEscapeLine(MYPREFIX MapScr::SelectFromAccelMap),
		{IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromAccelMap},
		MSListEnd
	}
};


/*************************
**	WAYPOINT MAP SCREEN **
*************************/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		WaypointMapInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::WaypointMapInit()
{

	return SEL_0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromWaypointMap
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromWaypointMap(UByte&,Key k)
{
	MapScr* retval=this;

	switch (k)
	{
	case SEL_1:	//set next waypoint to one stored in OverLay.curr_waypoint
		OverLay.hilight=k;
		if (Manual_Pilot.ControlledAC2)
		{
 			if (_DPlay.Implemented || _Replay.Record)
			{
				UniqueID wp=OverLay.curr_waypoint->uniqueID.count;

				_DPlay.NewWPMessage(wp);
			}
			else
			{
				Manual_Pilot.ControlledAC2->waypoint=OverLay.curr_waypoint;
				Persons2::PlayerGhostAC->waypoint=OverLay.curr_waypoint;
			}
		}
		break;
	case SEL_2:	//accel to hilighted waypoint

	OverLay.AccelSelection(k,DPlay::ACCEL_WP_NEXTWP);


		break;
//dead	case SEL_3:	//accel to hilighted waypoint
//dead		OverLay.hilight=k;
//dead		OverLay.fZoomedMap=false;
//dead		OverLay.keyFlags=KF_ACCELON+KF_FASTACCEL;
//dead		Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_WAYPT);
//dead		Manual_Pilot.acceltochanged=OverLay.curr_waypoint;
//dead		refreshLandscape=true;
//dead		break;
	case SEL_3:
	{
		WayPointPtr newWP;
		UNIQUE_ID tmp = Manual_Pilot.ControlledAC2->uniqueID;
		if ( Manual_Pilot.ControlledAC2->fly.expandedsag )
			tmp = Manual_Pilot.ControlledAC2->fly.expandedsag->uniqueID;
		if ( (newWP=FindNextWP(tmp,OverLay.curr_waypoint))!=NULL)
			OverLay.curr_waypoint=newWP;

		OverLay.SetPossibleLandscapeResfresh();
		break;
	}
	case SEL_4:
	{
		WayPointPtr newWP;
		UNIQUE_ID tmp = Manual_Pilot.ControlledAC2->uniqueID;
		if ( Manual_Pilot.ControlledAC2->fly.expandedsag )
			tmp = Manual_Pilot.ControlledAC2->fly.expandedsag->uniqueID;
		if ( (newWP=FindPrevWP(tmp,OverLay.curr_waypoint))!=NULL)
			OverLay.curr_waypoint=newWP;

		OverLay.SetPossibleLandscapeResfresh();
		break;
	}
	case SEL_5:
		OverLay.fZoomedMap=OverLay.fZoomedMap?false:true;
		OverLay.DoPossibleLandscapeRefresh();
		break;
	case SEL_0:
	case SEL_ESC:
		UByte dummy;
		OverLay.SetPossibleLandscapeResfresh();
		if (OverLay.pvp && OverLay.pvp->Accel())
		{
			OverLay.keyFlags|=KF_ACCELOFF;
			Manual_Pilot.AutoToggle(ManualPilot::MANUAL);
			retval=SelectFromFirstMap(dummy,k);
		} else
		{
			if ( Manual_Pilot.ControlledAC2 && Manual_Pilot.ControlledAC2->classtype->aerobaticfactor == AEROBATIC_LOW )
				retval=&COverlay::firstMapScrBomber;
			else
				retval=&COverlay::firstMapScr;
		}
		break;
	default:
		break;
	}
	return retval;
}

MapScr COverlay::waypointMapScr=
{
	MapScr::SCALE_GRIDORIGIN+/*MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE+
	MapScr::FLAG_MOUSEOVERHI,
	MapScreenX,MapScreenY,
	MapScreenWidth,MapScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_BLACK,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::WaypointMapInit,
	MYPREFIX MapScr::UpdateWaypointDisplay,
	{
		{IDS_MAP_SETNEXTWP,0,SEL_1,MYPREFIX MapScr::SelectFromWaypointMap},
		{IDS_MAP_ACCELTONEXTWP,0,SEL_2,MYPREFIX MapScr::SelectFromWaypointMap},
		{IDS_NEXT,0,SEL_3,MYPREFIX MapScr::SelectFromWaypointMap},
		{IDS_PREV,0,SEL_4,MYPREFIX MapScr::SelectFromWaypointMap},
		{IDS_MAP_ZOOM,0,SEL_5,MYPREFIX MapScr::SelectFromWaypointMap},
		MSEscapeLine(MYPREFIX MapScr::SelectFromWaypointMap),
		{IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromWaypointMap},
		MSListEnd
	}
};

/**********************
**	RADIO MAP SCREEN **
**********************/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		RadioMapInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::RadioMapInit()
{
	return SEL_0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromRadioMap
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromRadioMap(UByte&,Key k)
{
	localDec=NULL;
	MapScr* retval;

	AirStrucPtr player = Persons2::PlayerGhostAC;

	bool toweronly = false;
		if(		((Persons2::PlayerSeenAC->movecode == AUTO_TRAININGTAKEOFF)	|| (Persons2::PlayerSeenAC->movecode == AUTO_TRAININGLANDING))
			||	(Art_Int.CountFormationSize(Persons2::PlayerGhostAC) == 1)	)
		toweronly = true;

	switch (k)
	{
		case SEL_1:
			localDec = NULL;
			if(!toweronly)
			{
				if(player->formpos == 0)				localDec = GroupInfoMsg;
				else
					if(player->nationality == NAT_RAF)	localDec = GroupInfoMsgFolRaf;
					else								localDec = GroupInfoMsgFolLuf;
			}
			break;

		case SEL_2:
			localDec = NULL;
			if(!toweronly)
			{
				if(player->formpos == 0)
					if(player->nationality == NAT_RAF)	localDec = PreCombatMsg;
					else								localDec = PreCombatMsgLuf;
			}
			break;

		case SEL_3:
			localDec = NULL;
			if(!toweronly)
			{
				if(player->formpos == 0)				localDec = CombatMsg;
				else
					if(player->nationality == NAT_RAF)	localDec = CombatMsgFolRaf;
					else								localDec = CombatMsgFolLuf;
			}
			break;

		case SEL_4:
			localDec = NULL;
			if(!toweronly)
				if(player->formpos == 0)				localDec = PostCombatMsg;
			break;

		case SEL_5:
			localDec = TowerMsg;
		break;

		case SEL_6:
			localDec = NULL;
			if(!toweronly)
				if((player->formpos == 0) && ((player->classtype->visible == JU87) || (player->classtype->visible == ME110)))
					localDec = Strike;
		break;

		default:
			break;
	}


	if (localDec!=NULL)
	{
		retval=&COverlay::userOptsMapScr;
	}
	else retval=&COverlay::firstMapScr;

	return retval;
}

MapScr COverlay::radioMapScr=
{
	MapScr::SCALE_GRIDORIGIN+/*MapScr::SCALE_GRIDOFFSETS+*/
	MapScr::SCALE_DYNAMICSIZE+
	MapScr::FLAG_MOUSEOVERHI,
	MapScreenX,MapScreenY,
	MapScreenWidth,MapScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_BLACK,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	1,11,			//MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::RadioMapInit,
	MYPREFIX MapScr::UpdateMessageDisplay,
	{
		{IDS_UM_GROUPINFO,0,SEL_1,MYPREFIX MapScr::SelectFromRadioMap},
		{IDS_UM_PRECOMBAT,0,SEL_2,MYPREFIX MapScr::SelectFromRadioMap},
		{IDS_UM_COMBAT,0,SEL_3,MYPREFIX MapScr::SelectFromRadioMap},
		{IDS_UM_POSTCOMBAT,0,SEL_4,MYPREFIX MapScr::SelectFromRadioMap},
		{IDS_UM_TOWER,0,SEL_5,MYPREFIX MapScr::SelectFromRadioMap},
		{IDS_UM_FAC,0,SEL_6,MYPREFIX MapScr::SelectFromRadioMap},
		MSBlankLine,
		MSEscapeLine(MYPREFIX MapScr::SelectFromRadioMap),
		{IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromRadioMap},
		MSListEnd
	}
};



/*************************
**	COMMSMSG MAP SCREEN **
*************************/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		CommsMsgMapInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::CommsMsgMapInit()
{
	return SEL_0;
}


/****************************
**	COMMSMSG ORDERS SCREEN **
****************************/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		CommsMsgMapInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::CommsMsgOrdersInit()
{
	return SEL_0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromCommsMsgMap
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromCommsMsgOrders(UByte&,Key k)
{
	MapScr* retval=NULL;

	switch (k)
	{
	case SEL_1:
	case SEL_2:
	case SEL_3:
	case SEL_4:
	case SEL_5:
	case SEL_6:
	case SEL_7:
	case SEL_8:
		_DPlay.NewPlayerMessage(k);
		break;
	case SEL_0:
	case SEL_ESC:
		break;
	default:
		break;
	}
	return retval;
}

MapScr COverlay::commsMsgOrdersScr=
{
	MapScr::JUSTIFY_CENTRE_SX+/*MapScr::SCALE_GRIDWH+
	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::CommsMsgOrdersInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		{IDS_INDIRECT00,0,SEL_1,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		{IDS_INDIRECT01,0,SEL_2,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		{IDS_INDIRECT02,0,SEL_3,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		{IDS_INDIRECT03,0,SEL_4,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		{IDS_INDIRECT04,0,SEL_5,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		{IDS_INDIRECT05,0,SEL_6,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		{IDS_INDIRECT06,0,SEL_7,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		{IDS_INDIRECT07,0,SEL_8,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		MSEscapeLine(MYPREFIX MapScr::SelectFromCommsMsgOrders),
		{IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromCommsMsgOrders},
		MSListEnd
	}
};

/*******************************
**	COMMSRECIPIENT MAP SCREEN **
*******************************/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		CommsRecipientMapInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::CommsRecipientMapInit()
{
	return SEL_0;
}



/*******************************
**	COMMSRECIPIENT ORDERS SCREEN **
*******************************/

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		CommsRecipientMapInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::CommsRecipientOrdersInit()
{
	return SEL_0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromCommsRecipientMap
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromCommsRecipientOrders(UByte&,Key k)
{
	MapScr* retval=NULL;

	switch (k)
	{
	case SEL_1:
		_DPlay.MsgRecipientAll=true;
		break;
	case SEL_2:
		_DPlay.MsgRecipientAll=false;
		break;
	case SEL_0:
	case SEL_ESC:
		break;
	default:
		break;
	}
	return retval;
}

MapScr COverlay::commsRecipientOrdersScr=
{
	MapScr::JUSTIFY_CENTRE_SX+/*MapScr::SCALE_GRIDWH+
	MapScr::SCALE_GRIDORIGIN+MapScr::SCALE_GRIDOFFSETS*/
	MapScr::SCALE_DYNAMICSIZE,
	MapScreenX,MapScreenY,
	RadioScreenWidth,RadioScreenHeight,
	MapGridXOffset,MapGridYOffset,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_WHITE,
	TEXT_HI,
	MapColumnWidth,MapRowHeight,
	MapGridNumCols,MapGridNumRows,
	0x0000,
	MYPREFIX MapScr::CommsRecipientOrdersInit,
	MYPREFIX MapScr::TimeLimitedDisplay,
	{
		{IDS_EVERYBODY,0,SEL_1,MYPREFIX MapScr::SelectFromCommsRecipientOrders},
		{IDS_MYSIDEONLY,0,SEL_2,MYPREFIX MapScr::SelectFromCommsRecipientOrders},
		MSBlankLine,
		MSBlankLine,
		MSBlankLine,
		MSBlankLine,
		MSBlankLine,
		MSBlankLine,
		MSEscapeLine(MYPREFIX MapScr::SelectFromCommsRecipientOrders),
		{IDS_MAP_EXIT,0,SEL_0,MYPREFIX MapScr::SelectFromCommsRecipientOrders},
		MSListEnd
	}
};

/*********************
* REPLAY ICON SCREEN *
*********************/

struct IconEntry COverlay::replayIconTable[]=
{
{RPLYICNSNO,0,0,32,32},			//IDS_MAPICON00LO
{RPLYICN2NO,0,0,32,32},			//IDS_MAPICON00HI
{RPLYICNSNO,32,0,32,32},		//IDS_MAPICON01LO
{RPLYICN2NO,32,0,32,32},		//IDS_MAPICON01HI
{RPLYICNSNO,64,0,32,32},		//IDS_MAPICON02LO
{RPLYICN2NO,64,0,32,32},		//IDS_MAPICON02HI
{RPLYICNSNO,96,0,32,32},		//IDS_MAPICON03LO
{RPLYICN2NO,96,0,32,32},		//IDS_MAPICON03HI

{RPLYICNSNO,0,32,32,32},		//IDS_MAPICON04LO
{RPLYICN2NO,0,32,32,32},		//IDS_MAPICON04HI
{RPLYICNSNO,32,32,32,32},		//IDS_MAPICON05LO
{RPLYICN2NO,32,32,32,32},		//IDS_MAPICON05HI
{RPLYICNSNO,64,32,32,32},		//IDS_MAPICON06LO
{RPLYICN2NO,64,32,32,32},		//IDS_MAPICON06HI
{RPLYICNSNO,96,32,32,32},		//IDS_MAPICON07LO
{RPLYICN2NO,96,32,32,32},		//IDS_MAPICON07HI

{RPLYICNSNO,0,64,32,32},		//IDS_MAPICON08LO
{RPLYICN2NO,0,64,32,32},		//IDS_MAPICON08HI
{RPLYICNSNO,32,64,32,32},		//IDS_MAPICON09LO
{RPLYICN2NO,32,64,32,32},		//IDS_MAPICON09HI
{RPLYICNSNO,64,64,32,32},		//IDS_MAPICON09LO
{RPLYICN2NO,64,64,32,32},		//IDS_MAPICON09HI
{RPLYICNSNO,96,64,32,32},		//IDS_MAPICON09LO
{RPLYICN2NO,96,64,32,32},		//IDS_MAPICON09HI

{RPLYICNSNO,0,96,32,32},		//IDS_MAPICON09LO
{RPLYICN2NO,0,96,32,32},		//IDS_MAPICON09HI
{RPLYICNSNO,32,96,32,32},		//IDS_MAPICON09LO
{RPLYICN2NO,32,96,32,32},		//IDS_MAPICON09HI
{RPLYICNSNO,64,96,32,32},		//IDS_MAPICON09LO
{RPLYICN2NO,64,96,32,32},		//IDS_MAPICON09HI
{RPLYICNSNO,96,96,32,32},		//IDS_MAPICON09LO
{RPLYICN2NO,96,96,32,32}		//IDS_MAPICON09HI
};

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		ReplayScreenInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::ReplayScreenInit()
{
	return SEL_NONE;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromReplayScreen
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromReplayScreen(UByte&,Key k)
{
	MapScr* retval=this;

	if (_Replay.ReplayFlag==RF_NONE)
	{
// if there is already an operation that needs doing then ignore new ones

		switch (k)
		{
		case SEL_1:		//skip back
// in debug version disable first replay icon so I dont accidently click
// on it whilst debugging.
#ifdef NDEBUG
			OverLay.hilight=k;

			_Replay.ReplayFlag=RF_SKIPTOPREVIOUSMARK;
			_Replay.SkipToPreviousMark();

// if we are not going to forward any, then update position cursor otherwise
// it will not move until play begins

			if (!_Replay.framestoskip)
				_Replay.CalcCurrPos();

			_Replay.SetPreviousPos();
#endif
			break;
		case SEL_2:		//back a block
			OverLay.hilight=k;

			if (false)
			{
				_Replay.Rewind30secs();
			}
			else
			{
				_Replay.BlockRewind();
			}

			if (!_Replay.framestoskip)
				_Replay.CalcCurrPos();

			_Replay.SetPreviousPos();

			break;
		case SEL_3:		//back a frame
			OverLay.hilight=k;
			if (false)
			{
				_Replay.Rewind1sec();
				if (!_Replay.framestoskip)
					_Replay.CalcCurrPos();
			}
			else
			{
				_Replay.ReplayFlag=RF_FRAMEREWIND;
			}

			_Replay.SetPreviousPos();
			break;
		case SEL_PAUSE:
		case SEL_4:		//pause/play
			//force a screen change to toggle the icons
			if (uintptr_t(this)==uintptr_t(&OverLay.replayPlayingScr))
			{
				//was playing but not now
				OverLay.pCurScr=OverLay.pNewScr=retval=&OverLay.replayScr;
				_Replay.PlaybackPaused=TRUE;
				_Replay.setpaused=1;
			}
			else
			{
				//was paused but not now
				if (_Replay.AtEnd())
				{
					_Replay.SkipToBeginning();
				}
				OverLay.pCurScr=OverLay.pNewScr=retval=&OverLay.replayPlayingScr;
				_Replay.PlaybackPaused=FALSE;
				_Replay.setpaused=-1;
			}
			OverLay.hilight=k;
			break;
		case SEL_5:		//forward a frame
			OverLay.hilight=k;
			if (false)
			{
				_Replay.Forward1sec();
				if (!_Replay.framestoskip)
					_Replay.CalcCurrPos();
			}
			else
			{
				_Replay.ReplayFlag=RF_FRAMEFORWARD;
			}

			_Replay.SetPreviousPos();
			break;
		case SEL_6:		//forward a block
			OverLay.hilight=k;
			if (false)
			{
				_Replay.Forward30secs();
			}
			else
			{
				_Replay.BlockForward();
			}

			if (!_Replay.framestoskip)
				_Replay.CalcCurrPos();

			_Replay.SetPreviousPos();

			break;
		case SEL_7:		//skip forward
			OverLay.hilight=k;

			_Replay.ReplayFlag=RF_SKIPTONEXTMARK;
			_Replay.SkipToNextMark();
			_Replay.SetPreviousPos();

			break;
		case SEL_8:		//save
			OverLay.hilight=k;
			if (_Replay.SaveBlock())
			{
				_Replay.BackupPlaybackPosition();
				_DPlay.SimulateExitKey=TRUE;
			}
			break;
		case SEL_9:		//clear markers
			OverLay.hilight=k;
			_Replay.RemoveMarks();
			break;
		case SEL_QUIT:	//alt-X to exit
		case SEL_0:		//eject
// escape is used for reset view which we need, so dont have as exit
//		case SEL_ESC:
			OverLay.hilight=k;
			retval=NULL;
			_DPlay.SimulateExitKey=TRUE;
			break;
		case SEL_MINUS:	//mark start:
			OverLay.hilight=k;
			_Replay.MarkBlockBeginning();
			break;
		case SEL_PLUS:	//mark end
			OverLay.hilight=k;
			_Replay.MarkBlockEnd();
			break;
		default:
			break;
		}

		_DPlay.BackupGhost();
		_DPlay.BackupSeen();
	}

	return retval;
}

MapScr COverlay::replayPlayingScr=
{
	MapScr::SCALE_GRIDELEMENTS,
	0,0,
	320,32,
	0,0,
	NO_BACKGROUND_WIPE,
	ICON_BACK_COLOUR,
	ICON_BACK_COLOUR,
	32,32,
	12,2,
	0x00C8,
	MYPREFIX MapScr::ReplayScreenInit,
	MYPREFIX MapScr::ShowPositionIndicators,
	{
		{IDS_MAPICON01LO,1,SEL_1,MYPREFIX MapScr::SelectFromReplayScreen},		//skip back
		{IDS_MAPICON02LO,1,SEL_2,MYPREFIX MapScr::SelectFromReplayScreen},		//back a block
		{IDS_MAPICON12LO,1,SEL_3,MYPREFIX MapScr::SelectFromReplayScreen},		//back a frame
		{IDS_MAPICON04LO,1,SEL_4,MYPREFIX MapScr::SelectFromReplayScreen},		//play
		{IDS_MAPICON11LO,1,SEL_5,MYPREFIX MapScr::SelectFromReplayScreen},		//forward a frame
		{IDS_MAPICON05LO,1,SEL_6,MYPREFIX MapScr::SelectFromReplayScreen},		//forward a block
		{IDS_MAPICON10LO,1,SEL_7,MYPREFIX MapScr::SelectFromReplayScreen},		//skip forward
		{IDS_MAPICON07LO,1,SEL_8,MYPREFIX MapScr::SelectFromReplayScreen},		//save
		{IDS_MAPICON06LO,1,SEL_9,MYPREFIX MapScr::SelectFromReplayScreen},		//clear markers
		{IDS_MAPICON00LO,1,SEL_0,MYPREFIX MapScr::SelectFromReplayScreen},		//eject
		{IDS_MAPICON08LO,1,SEL_MINUS,MYPREFIX MapScr::SelectFromReplayScreen},	//mark start
		{IDS_MAPICON09LO,1,SEL_PLUS,MYPREFIX MapScr::SelectFromReplayScreen},	//mark end
		{IDS_MAPICON13LO,0,SEL_BLANK,NULL},								//progress bar start
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON15LO,0,SEL_BLANK,NULL},								//progress bar end
		MSAltXLine(MYPREFIX MapScr::SelectFromReplayScreen),
		MSPauseLine(MYPREFIX MapScr::SelectFromReplayScreen),
		MSEscapeLine(MYPREFIX MapScr::SelectFromReplayScreen),					//escape (eject)
		MSListEnd
	}
};

MapScr COverlay::replayScr=
{
	MapScr::SCALE_GRIDELEMENTS,
	0,0,
	320,32,
	0,0,
	NO_BACKGROUND_WIPE,
	ICON_BACK_COLOUR,
	ICON_BACK_COLOUR,
	32,32,
	12,2,
	0x0014,
	MYPREFIX MapScr::ReplayScreenInit,
	MYPREFIX MapScr::ShowPositionIndicators,
	{
		{IDS_MAPICON01LO,1,SEL_1,MYPREFIX MapScr::SelectFromReplayScreen},		//skip back
		{IDS_MAPICON02LO,1,SEL_2,MYPREFIX MapScr::SelectFromReplayScreen},		//back a block
		{IDS_MAPICON12LO,1,SEL_3,MYPREFIX MapScr::SelectFromReplayScreen},		//back a frame
		{IDS_MAPICON03LO,1,SEL_4,MYPREFIX MapScr::SelectFromReplayScreen},		//pause
		{IDS_MAPICON11LO,1,SEL_5,MYPREFIX MapScr::SelectFromReplayScreen},		//forward a frame
		{IDS_MAPICON05LO,1,SEL_6,MYPREFIX MapScr::SelectFromReplayScreen},		//forward a block
		{IDS_MAPICON10LO,1,SEL_7,MYPREFIX MapScr::SelectFromReplayScreen},		//skip forward
		{IDS_MAPICON07LO,1,SEL_8,MYPREFIX MapScr::SelectFromReplayScreen},		//save
		{IDS_MAPICON06LO,1,SEL_9,MYPREFIX MapScr::SelectFromReplayScreen},		//clear markers
		{IDS_MAPICON00LO,1,SEL_0,MYPREFIX MapScr::SelectFromReplayScreen},		//eject
		{IDS_MAPICON08LO,1,SEL_MINUS,MYPREFIX MapScr::SelectFromReplayScreen},	//mark start
		{IDS_MAPICON09LO,1,SEL_PLUS,MYPREFIX MapScr::SelectFromReplayScreen},	//mark end
		{IDS_MAPICON13LO,0,SEL_BLANK,NULL},								//progress bar start
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON14LO,0,SEL_BLANK,NULL},								//progress bar middle
		{IDS_MAPICON15LO,0,SEL_BLANK,NULL},								//progress bar end
		MSAltXLine(MYPREFIX MapScr::SelectFromReplayScreen),
		MSPauseLine(MYPREFIX MapScr::SelectFromReplayScreen),
		MSEscapeLine(MYPREFIX MapScr::SelectFromReplayScreen),					//escape (eject)
		MSListEnd
	}
};

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		CommsDialogInit
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
Key MapScr::CommsDialogInit()
{
	OverLay.commsDialReturn=0;
	return SEL_ENTER;
}

MapScr* MapScr::SelectCommsChat(UByte&,Key k)
{
	MapScr* retval=NULL;
	switch (k)
	{
	case SEL_0:
	case SEL_ESC:
		break;
	case SEL_ENTER:
		break;
	default:
		break;
	}
	return retval;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromCommsDialog
//Author		Paul.
//Date			Fri 19 Feb 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromCommsDialog(UByte&,Key k)
{
	MapScr* retval=this;

	switch (k)
	{
	case SEL_ESC:
		OverLay.commsDialReturn=0xFFFF;
		break;
	case SEL_ENTER:
		OverLay.commsDialReturn=1;
		break;
	default:
		break;
	}
	return retval;
}

MapScr COverlay::commsDialog=
{
	MapScr::JUSTIFY_CENTRE_SX+MapScr::JUSTIFY_CENTRE_SY,
	0,0,
	320,32,
	0,0,
	SET_WIPE_TEXTURE(RADIONO),
	TEXT_MAP_BLACK,
	TEXT_MAP_BLACK,
	32,32,
	10,1,
	0x0000,
	MYPREFIX MapScr::CommsDialogInit,
	NULL,
	{
		{IDS_COMMSDIALMSG,0,SEL_NONE,NULL},
		MSBlankLine,
		{IDS_COMMSDIALOK,0,SEL_ENTER,MYPREFIX MapScr::SelectFromCommsDialog},
		{IDS_COMMSDIALCANCEL,0,SEL_ESC,MYPREFIX MapScr::SelectFromCommsDialog},
		MSListEnd
	}
};

/***********************
* MAP VIEW ICON SCREEN *
***********************/

struct IconEntry COverlay::mapViewIconTable[]=
{
{MAPICNSNO,0,0,32,32},		//IDS_MAPICON00LO
{MAPICN2NO,0,0,32,32},		//IDS_MAPICON00HI
{MAPICNSNO,32,0,32,32},		//IDS_MAPICON01LO
{MAPICN2NO,32,0,32,32},		//IDS_MAPICON01HI
{MAPICNSNO,64,0,32,32},		//IDS_MAPICON02LO
{MAPICN2NO,64,0,32,32},		//IDS_MAPICON02HI
{MAPICNSNO,96,0,32,32},		//IDS_MAPICON03LO
{MAPICN2NO,96,0,32,32},		//IDS_MAPICON03HI

{MAPICNSNO,0,32,32,32},		//IDS_MAPICON04LO
{MAPICN2NO,0,32,32,32},		//IDS_MAPICON04HI
{MAPICNSNO,32,32,32,32},	//IDS_MAPICON05LO
{MAPICN2NO,32,32,32,32},	//IDS_MAPICON05HI
{MAPICNSNO,64,32,32,32},	//IDS_MAPICON06LO
{MAPICN2NO,64,32,32,32},	//IDS_MAPICON06HI
{MAPICNSNO,96,32,32,32},	//IDS_MAPICON07LO
{MAPICN2NO,96,32,32,32},	//IDS_MAPICON07HI

{MAPICNSNO,0,64,32,32},		//IDS_MAPICON08LO
{MAPICN2NO,0,64,32,32},		//IDS_MAPICON08HI
{MAPICNSNO,32,64,32,32},	//IDS_MAPICON09LO
{MAPICN2NO,32,64,32,32},	//IDS_MAPICON09HI
{MAPICNSNO,64,64,32,32},	//IDS_MAPICON09LO
{MAPICN2NO,64,64,32,32},	//IDS_MAPICON09HI
{MAPICNSNO,96,64,32,32},	//IDS_MAPICON09LO
{MAPICN2NO,96,64,32,32},	//IDS_MAPICON09HI

{MAPICNSNO,0,96,32,32},		//IDS_MAPICON09LO
{MAPICN2NO,0,96,32,32},		//IDS_MAPICON09HI
{MAPICNSNO,32,96,32,32},	//IDS_MAPICON09LO
{MAPICN2NO,32,96,32,32},	//IDS_MAPICON09HI
{MAPICNSNO,64,96,32,32},	//IDS_MAPICON09LO
{MAPICN2NO,64,96,32,32},	//IDS_MAPICON09HI
{MAPICNSNO,96,96,32,32},	//IDS_MAPICON09LO
{MAPICN2NO,96,96,32,32}		//IDS_MAPICON09HI
};

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		MapViewScreenInit
//Author		Paul.
//Date			Thu 11 Mar 1999
//------------------------------------------------------------------------------
Key MapScr::MapViewScreenInit()
{
	return SEL_ENTER;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SelectFromMapViewScreen
//Author		Paul.
//Date			Thu 11 Mar 1999
//------------------------------------------------------------------------------
MapScr* MapScr::SelectFromMapViewScreen(UByte&,Key k)
{
	MapScr* retval=this;

	switch (k)
	{
	case SEL_1:		//rotate left
		if (k!=OverLay.hilight)	OverLay.hilight=k;
		else					OverLay.hilight=SEL_NONE;
		break;
	case SEL_2:		//rotate right
		if (k!=OverLay.hilight)	OverLay.hilight=k;
		else					OverLay.hilight=SEL_NONE;
		break;
	case SEL_3:		//zoom out
		if (k!=OverLay.hilight)	OverLay.hilight=k;
		else					OverLay.hilight=SEL_NONE;
		break;
	case SEL_4:		//zoom in
		if (k!=OverLay.hilight)	OverLay.hilight=k;
		else					OverLay.hilight=SEL_NONE;
		break;
	case SEL_5:		//reset
		OverLay.highTimer=100;
		OverLay.hilight=k;
		break;
	case SEL_0:		//exit
	case SEL_ESC:
		_DPlay.SimulateExitKey=TRUE;
		delete OverLay.targName;
		OverLay.targName=NULL;
		retval=NULL;
		break;
	default:
		break;
	}
	return retval;
}

MapScr COverlay::mapViewScr=
{
	MapScr::SCALE_DOUBLEIFGT800,
	0,0,
	320,32,
	0,0,
	NO_BACKGROUND_WIPE,
	ICON_BACK_COLOUR,
	ICON_BACK_COLOUR,
	32,32,
	6,1,
	0x0000,
	MYPREFIX MapScr::MapViewScreenInit,
	MYPREFIX MapScr::DisplayTargName,
	{
		{IDS_MAPICON00LO,1,SEL_1,MYPREFIX MapScr::SelectFromMapViewScreen},		//rotate left
		{IDS_MAPICON01LO,1,SEL_2,MYPREFIX MapScr::SelectFromMapViewScreen},		//rotate right
		{IDS_MAPICON02LO,1,SEL_3,MYPREFIX MapScr::SelectFromMapViewScreen},		//zoom out
		{IDS_MAPICON03LO,1,SEL_4,MYPREFIX MapScr::SelectFromMapViewScreen},		//zoom in
		{IDS_MAPICON04LO,1,SEL_5,MYPREFIX MapScr::SelectFromMapViewScreen},		//reset
		{IDS_MAPICON05LO,1,SEL_0,MYPREFIX MapScr::SelectFromMapViewScreen},		//exit
		MSEscapeLine(MYPREFIX MapScr::SelectFromMapViewScreen),					//exit
		MSListEnd
	}
};

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetTargName
//Author		Paul.
//Date			Fri 12 Mar 1999
//------------------------------------------------------------------------------
void COverlay::SetTargName(char* tn)
{
	targName=new CString(LPCTSTR(tn));
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DisplayTargName
//Author		Paul.
//Date			Fri 12 Mar 1999
//------------------------------------------------------------------------------
void MapScr::DisplayTargName() {OverLay.DisplayTargName();}
void COverlay::DisplayTargName()
{
	//handle icon based zoom & rotate requests here

	pvp->ExecuteMapViewCommand(Key(hilight));

	if (targName)
	{
		LPCTSTR ps=LPCTSTR(*targName);


		GetSurfaceDimensions(physicalWidth,physicalHeight);
		SWord text_width=StrPixelWidth((char*)ps);

		SWord text_x=(physicalWidth-text_width)>>1;
		if (text_x<0) text_x=0;
		SWord text_y=physicalHeight-(fIsWide?32:16);
		PrintAt2(text_x,text_y,(char*)ps);
	}
}

R3DVALUE AbsRoot(R3DVALUE f)
{
	ULong sign=(*(ULong*)&f)&0x80000000;
	*(ULong*)&f-=sign;
	f=fastMath.FastSqrt(f);
	*(ULong*)&f|=sign;
	return f;
}


inline R3DVALUE Sx640ToFrac(int sx)
{
	return (R3DVALUE(sx)/320.f);
}
inline R3DVALUE Sy480ToFrac(int sx)
{
	return (R3DVALUE(sx)/240.f);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DoThreat
//Author		Paul.
//Date			Fri 26 Mar 1999
//------------------------------------------------------------------------------
void COverlay::DoThreat()
{
//#pragma message ( __HERE__ "DoThreat" )
	//display threat indicator in the top left corner of the screen
	const SWord MAX_THREATS=320;

	GetSurfaceDimensions(physicalWidth,physicalHeight);

	enum
	{
		TOP_LEFT_X = 10,
		TOP_LEFT_Y = 10,
		RADIUS = 50,
		Y_RADIUS = 10
	};

	const D3DVALUE SCX = D3DVALUE(physicalWidth)/640.;
	const D3DVALUE SCY = D3DVALUE(physicalHeight)/640.;
	const D3DVALUE sTOP_LEFT_X = D3DVALUE(TOP_LEFT_X)*SCX;
	const D3DVALUE sTOP_LEFT_Y = D3DVALUE(TOP_LEFT_Y)*SCY;
	const D3DVALUE sRADIUS = D3DVALUE(RADIUS)*SCX;
	const D3DVALUE sY_RADIUS = D3DVALUE(Y_RADIUS)*SCY;

	const SLong THREAT_RANGE= 8*100*1000;//MaxVisibleRange;	//viewsel.h
	const SLong THREAT_ALT= 8*100*1000;//MaxVisibleRange;

//	GetSurfaceDimensions(physicalWidth,physicalHeight);

	ImageMapDesc* pmap=Image_Map.GetImageMapPtr(THREAT01NO);



	Lib3DPoint dp[4];
	dp[0].setPosition( sTOP_LEFT_X, sTOP_LEFT_Y, 0.f );
	dp[0].setIMapCoords( 0, 0 );

	dp[1].setPosition( sRADIUS*2+sTOP_LEFT_X, sTOP_LEFT_Y, 0.f );
	dp[1].setIMapCoords( 128, 0 );

	dp[2].setPosition( sRADIUS*2+sTOP_LEFT_X, sRADIUS*2+sTOP_LEFT_Y, 0.f );
	dp[2].setIMapCoords( 128, 128 );

	dp[3].setPosition( sTOP_LEFT_X, sRADIUS*2+sTOP_LEFT_Y, 0.f );
	dp[3].setIMapCoords( 0, 128 );

	PutC( pmap, dp );

//	DrawLine( TOP_LEFT_X, RADIUS+TOP_LEFT_Y-Y_RADIUS, RADIUS*2+TOP_LEFT_X, RADIUS+TOP_LEFT_Y-Y_RADIUS, RED );
	//check for a/c within THREAT_RANGE cm
	COORDS3D* crd=new COORDS3D[MAX_THREATS];
	UByte* col=new UByte[MAX_THREATS];

	SWord cnt=MAX_THREATS;

	pvp->GetThreatData(cnt,crd,col,THREAT_RANGE);

	if (++cnt>0)
	{
		for (int i=0;i<cnt;i++)
		{
			//generate the screen coordinates for each
			//aircraft on the threat indicator.

			{

				SWord baseStickX, baseStickY, stickHeight; // screen coords and height in pixels

				baseStickX = ((crd[i].X*sRADIUS)/THREAT_RANGE) + sRADIUS+sTOP_LEFT_X;
				baseStickY = -((crd[i].Z*sY_RADIUS)/THREAT_RANGE) + sRADIUS+sTOP_LEFT_Y;
				stickHeight = -(crd[i].Y*sRADIUS)/THREAT_ALT;

				if (col[i])
				{
					DrawLine(baseStickX,baseStickY,baseStickX,baseStickY+stickHeight,0xFF0000);
					DrawLine(baseStickX,baseStickY+stickHeight,baseStickX+3,baseStickY+stickHeight,0xFF0000);
					DrawLine(baseStickX,baseStickY,sRADIUS+sTOP_LEFT_X,sRADIUS+sTOP_LEFT_Y,0xFF0000);
				}
				else
				{
					DrawLine( baseStickX,baseStickY,baseStickX,baseStickY+stickHeight,0xFF );
					DrawLine( baseStickX,baseStickY+stickHeight,baseStickX+3,baseStickY+stickHeight,0xFF);
					DrawLine(baseStickX,baseStickY,sRADIUS+sTOP_LEFT_X,sRADIUS+sTOP_LEFT_Y,0xFF);
				}
			}
		}


	}
	delete[]col;
	delete[]crd;
}


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetupCheatBox
//Author		Jon.
//Date			Tue 11 Apr 2000
//------------------------------------------------------------------------------
void COverlay::SetupCheatBox()
{
	cheatBoxX = -1.f;													//RJS 28Jul00
	if (Save_Data.detail_3d[DETAIL3D_PADLOCKCHEAT]	&& pvp)					//JIM 15Sep00
	{
		if(	((pvp->viewnum.viewmode == VM_OutPadlock) || (pvp->viewnum.viewmode == VM_InPadlock) || (pvp->viewnum.viewmode == VM_InsideCheat))
			&& pvp && pvp->trackeditem2	)
		{																//RJS 22Aug00
// Ignore things that are dead, or invisible...
			if (	pvp->trackeditem2
				&&	pvp->trackeditem2!=pvp
				&&	pvp->trackeditem2->Status.size >= ITEMSIZE
				&&	pvp->trackeditem2->Anim != NULL	)					//MS 29Sep00
			{
				MinAnimData*	mad = (MinAnimData*)pvp->trackeditem2->Anim;
				if (	(mad->itemstate == DEAD)
					||	(mad->IsInvisible == 1)	)
					return;
			}


			R3DVERTEX pos;
			if (pvp->trackeditem2==pvp) // is the thing looked at the camera??
			{
				pos.x=pvp->trackedItem2World.X-pvp->World.X;
				pos.y=pvp->trackedItem2World.Y-pvp->World.Y;
				pos.z=pvp->trackedItem2World.Z-pvp->World.Z;

			} else
			{
				pos.x = pvp->trackeditem2->World.X-pvp->World.X;
				pos.y = pvp->trackeditem2->World.Y-pvp->World.Y;
				pos.z = pvp->trackeditem2->World.Z-pvp->World.Z;

				if(pvp->trackeditem2->Status.size >= MOBILESIZE) // if tracked item2 is moving
				{
					MovingItem*	mip = (MovingItem*)pvp->trackeditem2;

					pos.x += float(mip->vel_x + Three_Dee.wind_x)*Three_Dee.fView_dt_frac;
					pos.y += float(mip->vel_y + Three_Dee.wind_y)*Three_Dee.fView_dt_frac;
					pos.z += float(mip->vel_z + Three_Dee.wind_z)*Three_Dee.fView_dt_frac;
				}
				else
				if (pvp->trackeditem2->Status.size==WAYPOINTSIZE && WayPointPtr(pvp->trackeditem2)->target)
				{ // is it a waypoint.
					pos.x+=WayPointPtr(pvp->trackeditem2)->target->World.X;
					pos.y+=WayPointPtr(pvp->trackeditem2)->target->World.Y;
					pos.z+=WayPointPtr(pvp->trackeditem2)->target->World.Z;

				}
			}

			g_lpLib3d->PushMatrix(MATRIX_OBJECT);
			g_lpLib3d->LoadIdentity(MATRIX_OBJECT);
			g_lpLib3d->BodyToScreen( pos, cheatBoxX, cheatBoxY, cheatBoxZ, &cheatBoxScale );
			g_lpLib3d->PopMatrix(MATRIX_OBJECT);

#ifndef NDEBUG
			if(pvp->trackeditem2->Status.size == AIRSTRUCSIZE)
			{
				pos.x = pvp->trackeditem2->World.X - pvp->World.X;
				pos.y = pvp->trackeditem2->World.Y - pvp->World.Y;
				pos.z = pvp->trackeditem2->World.Z - pvp->World.Z;
				MovingItem*	mip = (MovingItem*)pvp->trackeditem2;
				SLong dt = Three_Dee.View_Dt();
				float range = Persons2::PlayerGhostAC->Distance3D(&pvp->trackeditem2->World);
				if(range < 100000)
				{
					dt += range * 1e5 / float(Persons2::PlayerGhostAC->classtype->muzzlevel);
					pos.x += SLong((mip->vel_x + Three_Dee.wind_x) * dt) / 1e5;
					pos.y += SLong((mip->vel_y + Three_Dee.wind_y) * dt) / 1e5;
					pos.z += SLong((mip->vel_z + Three_Dee.wind_z) * dt) / 1e5;
					g_lpLib3d->PushMatrix(MATRIX_OBJECT);
					g_lpLib3d->LoadIdentity(MATRIX_OBJECT);
					g_lpLib3d->BodyToScreen( pos, cheatBox2X, cheatBox2Y, cheatBox2Z, &cheatBoxScale);
					g_lpLib3d->PopMatrix(MATRIX_OBJECT);
				}
				else
					cheatBox2X = cheatBox2Y = cheatBox2Z = 0;
			}
			else
				cheatBox2X = cheatBox2Y = cheatBox2Z = 0;
#endif
		}
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DoCheatBox
//Author		Paul.
//Date			Wed 5 May 1999
//------------------------------------------------------------------------------
void COverlay::DoCheatBox()
{
	const float CHEATBOXSIZEMINSIZE = 10.f;
// hmmmm, see if cheat box works properly in comms yet, probably not...

	{
		if (	( cheatBoxX > -0.5f )
			&&	( cheatBoxZ > 1.f)	)
		{
			float boxRadius = 30.f * 100.f * cheatBoxScale; // should be 30 metres at that distance

			if( boxRadius < CHEATBOXSIZEMINSIZE )
				boxRadius = CHEATBOXSIZEMINSIZE;

			DrawLine( cheatBoxX - boxRadius, cheatBoxY, cheatBoxX, cheatBoxY-boxRadius, Colour(120) );
			DrawLine( cheatBoxX - boxRadius, cheatBoxY, cheatBoxX, cheatBoxY+boxRadius, Colour(120) );
			DrawLine( cheatBoxX + boxRadius, cheatBoxY, cheatBoxX, cheatBoxY-boxRadius, Colour(120) );
			DrawLine( cheatBoxX + boxRadius, cheatBoxY, cheatBoxX, cheatBoxY+boxRadius, Colour(120) );

	#ifndef NDEBUG
			if(cheatBox2X)
			{
				boxRadius /= 4;
				DrawLine(cheatBoxX, cheatBoxY, cheatBox2X, cheatBox2Y, Colour(120));
				DrawLine(cheatBox2X - boxRadius, cheatBox2Y - boxRadius, cheatBox2X + boxRadius, cheatBox2Y + boxRadius, Colour(120));
				DrawLine(cheatBox2X + boxRadius, cheatBox2Y - boxRadius, cheatBox2X - boxRadius, cheatBox2Y + boxRadius, Colour(120));
			}

#define DRAWDAMAGE
#ifdef  DRAWDAMAGE
			if(pvp->trackeditem2->Status.size == AIRSTRUCSIZE)
			{
				AirStrucPtr ac = AirStrucPtr(pvp->trackeditem2);
				if(!ac->fly.numinsag)
				{
					GetSurfaceDimensions(physicalWidth, physicalHeight);
					float sx = physicalWidth;
					float sy = physicalHeight;

					AircraftAnimData* adptr = (AircraftAnimData*)ac->Anim;

					Colour col;
					if(adptr->LEFTWINGOUT == 255)	col = Colour(120);	else col = Colour(192 + adptr->LEFTWINGOUT / 16);
					DrawLine(0.750 * sx, 0.050 * sy, 0.775 * sx, 0.050 * sy, col);
					DrawLine(0.750 * sx, 0.050*sy+1, 0.775 * sx, 0.050*sy+1, col);
					if(adptr->LEFTWINGIN == 255)	col = Colour(120);	else col = Colour(192 + adptr->LEFTWINGIN / 16);
					DrawLine(0.775 * sx, 0.050 * sy, 0.800 * sx, 0.050 * sy, col);
					DrawLine(0.775 * sx, 0.050*sy+1, 0.800 * sx, 0.050*sy+1, col);
					if(adptr->RIGHTWINGIN == 255)	col = Colour(120);	else col = Colour(192 + adptr->RIGHTWINGIN / 16);
					DrawLine(0.800 * sx, 0.050 * sy, 0.825 * sx, 0.050 * sy, col);
					DrawLine(0.800 * sx, 0.050*sy+1, 0.825 * sx, 0.050*sy+1, col);
					if(adptr->RIGHTWINGOUT == 255)	col = Colour(120);	else col = Colour(192 + adptr->RIGHTWINGOUT / 16);
					DrawLine(0.825 * sx, 0.050 * sy, 0.850 * sx, 0.050 * sy, col);
					DrawLine(0.825 * sx, 0.050*sy+1, 0.850 * sx, 0.050*sy+1, col);
					if(adptr->TAIL == 255)			col = Colour(120);	else col = Colour(192 + adptr->TAIL / 16);
					DrawLine(0.785 * sx, 0.110 * sy, 0.815 * sx, 0.110 * sy, col);
					DrawLine(0.785 * sx, 0.110*sy+1, 0.815 * sx, 0.110*sy+1, col);

					switch(ac->classtype->visible)
					{
						case SPIT:
						case HURR:
						case ME109:
						case JU87:
						case DEF:
						{
							if(adptr->ENGINELEFT == 255)	col = Colour(120);	else col = Colour(192 + adptr->ENGINELEFT / 16);
							DrawLine(0.800 * sx, 0.020 * sy, 0.800 * sx, 0.040 * sy, col);
							DrawLine(0.800*sx+1, 0.020 * sy, 0.800*sx+1, 0.040 * sy, col);
							if(adptr->CANOPY == 255)		col = Colour(120);	else col = Colour(192 + adptr->CANOPY / 16);
							DrawLine(0.800 * sx, 0.040 * sy, 0.800 * sx, 0.060 * sy, col);
							DrawLine(0.800*sx+1, 0.040 * sy, 0.800*sx+1, 0.060 * sy, col);
							if(adptr->OTHER == 255)			col = Colour(120);	else col = Colour(192 + adptr->OTHER / 16);
							DrawLine(0.800 * sx, 0.060 * sy, 0.800 * sx, 0.100 * sy, col);
							DrawLine(0.800*sx+1, 0.060 * sy, 0.800*sx+1, 0.100 * sy, col);
							if(adptr->FINLEFT == 255)		col = Colour(120);	else col = Colour(192 + adptr->FINLEFT / 16);
							DrawLine(0.800 * sx, 0.100 * sy, 0.800 * sx, 0.120 * sy, col);
							DrawLine(0.800*sx+1, 0.100 * sy, 0.800*sx+1, 0.120 * sy, col);
							break;
						}
						case ME110:
						case JU88:
						case HE11:
						case DO17:
						case BRISTO:
						{
							if(adptr->ENGINELEFT == 255)	col = Colour(120);	else col = Colour(192 + adptr->ENGINELEFT / 16);
							DrawLine(0.775 * sx, 0.035 * sy, 0.775 * sx, 0.055 * sy, col);
							DrawLine(0.775*sx+1, 0.035 * sy, 0.775*sx+1, 0.055 * sy, col);
							if(adptr->ENGINERIGHT == 255)	col = Colour(120);	else col = Colour(192 + adptr->ENGINERIGHT / 16);
							DrawLine(0.825 * sx, 0.035 * sy, 0.825 * sx, 0.055 * sy, col);
							DrawLine(0.825*sx+1, 0.035 * sy, 0.825*sx+1, 0.055 * sy, col);

							if(adptr->CANOPY == 255)		col = Colour(120);	else col = Colour(192 + adptr->CANOPY / 16);
							DrawLine(0.800 * sx, 0.020 * sy, 0.800 * sx, 0.035 * sy, col);
							DrawLine(0.800*sx+1, 0.020 * sy, 0.800*sx+1, 0.035 * sy, col);
							if(adptr->OTHER == 255)			col = Colour(120);	else col = Colour(192 + adptr->OTHER / 16);
							DrawLine(0.800 * sx, 0.035 * sy, 0.800 * sx, 0.110 * sy, col);
							DrawLine(0.800*sx+1, 0.035 * sy, 0.800*sx+1, 0.110 * sy, col);

							if(adptr->FINLEFT == 255)		col = Colour(120);	else col = Colour(192 + adptr->FINLEFT / 16);
							DrawLine(0.785 * sx, 0.105 * sy, 0.785 * sx, 0.120 * sy, col);
							DrawLine(0.785*sx+1, 0.105 * sy, 0.785*sx+1, 0.120 * sy, col);
							if(adptr->FINRIGHT == 255)		col = Colour(120);	else col = Colour(192 + adptr->FINRIGHT / 16);
							DrawLine(0.815 * sx, 0.105 * sy, 0.815 * sx, 0.120 * sy, col);
							DrawLine(0.815*sx+1, 0.105 * sy, 0.815*sx+1, 0.120 * sy, col);
							break;
						}
					}

					WeapAnimData*	weapon;
					SLong			xpos, ypos, zpos;
					ULong			mvel;
					UWord			index, mdelay, mburst;
					int	totalammo = 0;
					for(index = 0; index < 6; index++)
					{
						weapon = SHAPE.GetWeaponLauncher(ac, index, xpos, ypos, zpos, mvel, mdelay, mburst, LT_BULLET);
						if((weapon)	&& (weapon->LoadedStores > 0))
							totalammo += weapon->LoadedStores;
					}
					DrawLine(0.86 * sx, (0.12 - 3.6e-5 * totalammo) * sy, 0.86 * sx, 0.12 * sy, Colour(120));
				}
			}

			GetSurfaceDimensions(physicalWidth, physicalHeight);
			float sx = physicalWidth;
			float sy = physicalHeight;

			AirStrucPtr attacker = NULL;
			float y = 0.02;
			for(AirStrucPtr ld = Persons2::PlayerGhostAC->FindFormpos0(); ld; ld = ld->fly.nextflight)
			{
				float x = 0.65;
				for(AirStrucPtr ac = ld; ac; ac = ac->Follower())
				{
					if(ac == Persons2::PlayerGhostAC)
						ac = Persons2::PlayerSeenAC;
					Colour accol = Colour(0);	if(ac->AircraftDamaged())	accol = Colour(120);
					DrawLine((x - 0.006) * sx, y * sy, (x + 0.006) * sx, y * sy, accol);
					DrawLine(x * sx, (y - 0.004) * sy, x * sx, (y + 0.008) * sy, accol);
					DrawLine((x-0.003)*sx,(y+0.008)*sy,(x+0.003)*sx,(y+0.008)*sy, accol);
					if(ItemPtr(ac) == Trans_Obj.View_Object->trackeditem2)
					{
						DrawLine(x * sx, (y - 0.01) * sy, (x - 0.01) * sx, y * sy, accol);
						DrawLine(x * sx, (y - 0.01) * sy, (x + 0.01) * sx, y * sy, accol);
						DrawLine(x * sx, (y + 0.01) * sy, (x - 0.01) * sx, y * sy, accol);
						DrawLine(x * sx, (y + 0.01) * sy, (x + 0.01) * sx, y * sy, accol);
					}

					if(ac->ai.attacker)
						attacker = ac->ai.attacker;
					x += 0.02;
				}
				y += 0.02;
			}

			y += 0.02;
			if(attacker)
				for(AirStrucPtr ld = attacker->FindFormpos0(); ld; ld = ld->fly.nextflight)
				{
					float x = 0.65;
					for(AirStrucPtr ac = ld; ac; ac = ac->Follower())
					{
						Colour accol = Colour(0);	if(ac->ShouldGoHome())	accol = Colour(120);
						DrawLine((x - 0.006) * sx, y * sy, (x + 0.006) * sx, y * sy, accol);
						DrawLine(x * sx, (y - 0.004) * sy, x * sx, (y + 0.008) * sy, accol);
						DrawLine((x-0.003)*sx,(y+0.008)*sy,(x+0.003)*sx,(y+0.008)*sy, accol);
						if(ItemPtr(ac) == Trans_Obj.View_Object->trackeditem2)
						{
							DrawLine(x * sx, (y - 0.01) * sy, (x - 0.01) * sx, y * sy, accol);
							DrawLine(x * sx, (y - 0.01) * sy, (x + 0.01) * sx, y * sy, accol);
							DrawLine(x * sx, (y + 0.01) * sy, (x - 0.01) * sx, y * sy, accol);
							DrawLine(x * sx, (y + 0.01) * sy, (x + 0.01) * sx, y * sy, accol);
						}
						x += 0.02;
					}
					y += 0.02;
				}

#endif

	#endif
		}
	}

/*	const SWord DIAMOND_WH=16;

	if (Save_Data.detail_3d[DETAIL3D_PADLOCKCHEAT] &&
		pvp &&
		pvp->trackeditem2 &&
		pvp->trackeditem2->Status.size!=WayPointSize &&
		(pvp->viewnum.viewmode==VM_OutPadlock || pvp->viewnum.viewmode==VM_InPadlock))
	{
		GetSurfaceDimensions(physicalWidth,physicalHeight);
		const Float scaled_w_2=Float(DIAMOND_WH)*Float(physicalWidth)/(2.*640.);
		const Float scaled_h_2=Float(DIAMOND_WH)*Float(physicalHeight)/(2.*480.);

		FPMATRIX tmat,*tmatp=&tmat;
		_matrix.Generate(pvp->hdg,pvp->pitch,pvp->roll,tmatp);
		DoPointStruc dp;
		COORDS3D itemPos;
		if (!Three_Dee.GetPosRec(pvp->trackeditem2,itemPos))
			itemPos=pvp->trackeditem2->World;
		dp.bodyx.i=itemPos.X-pvp->World.X;
		dp.bodyy.i=itemPos.Y-pvp->World.Y;
		dp.bodyz.i=itemPos.Z-pvp->World.Z;
		if (_matrix.transform(tmatp,dp.bodyx,dp.bodyy,dp.bodyz)==0)
		{
			pw->DoBody2Screen(dp);
			dp.screenx.f+=(physicalWidth>>1);
			dp.screeny.f+=(physicalHeight>>1);
			SWord x1,y1,x2,y2;
			x1=SWord(dp.screenx.f-scaled_w_2);
			y1=SWord(dp.screeny.f);
			x2=SWord(dp.screenx.f);
			y2=SWord(dp.screeny.f-scaled_h_2);
			DoClippedLine(x1,y1,x2,y2,RED,0,0,physicalWidth,physicalHeight);
			x1=x2;
			y1=y2;
			x2=SWord(dp.screenx.f+scaled_w_2);
			y2=SWord(dp.screeny.f);
			DoClippedLine(x1,y1,x2,y2,RED,0,0,physicalWidth,physicalHeight);
			x1=x2;
			y1=y2;
			x2=SWord(dp.screenx.f);
			y2=SWord(dp.screeny.f+scaled_h_2);
			DoClippedLine(x1,y1,x2,y2,RED,0,0,physicalWidth,physicalHeight);
			x1=x2;
			y1=y2;
			x2=SWord(dp.screenx.f-scaled_w_2);
			y2=SWord(dp.screeny.f);
			DoClippedLine(x1,y1,x2,y2,RED,0,0,physicalWidth,physicalHeight);
		}
	}
	*/
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DoClippedLine
//Author		Paul.
//Date			Wed 5 May 1999
//------------------------------------------------------------------------------
void COverlay::DoClippedLine(SWord x1,SWord y1,SWord x2,SWord y2,Colour col,
							SWord left,SWord top,SWord right,SWord bottom)
{
	UWord cf1=NULL,cf2=NULL;
	if (x1<left)		cf1|=OFFLEFT;
	else if (x1>right)	cf1|=OFFRIGHT;
	if (y1<top)			cf1|=OFFTOP;
	else if (y1>bottom)	cf1|=OFFBOTTOM;
	if (x2<left)		cf2|=OFFLEFT;
	else if (x2>right)	cf2|=OFFRIGHT;
	if (y2<top)			cf2|=OFFTOP;
	else if (y2>bottom)	cf2|=OFFBOTTOM;
	if ((cf1&cf2)==0)
	{
		if ((cf1|cf2)!=0)
		{
			if ((cf1|cf2)&OFFTOP)
			{
				if (cf1&OFFTOP) {SWord q=x1;x1=x2;x2=q;q=y1;y1=y2;y2=q;}
				Float frac=Float(top-y1)/Float(y2-y1);
				x2=x1+SWord(frac*Float(x2-x1));
				y2=top;
				if (x2<left)		cf2=OFFLEFT;
				else if (x2>right)	cf2=OFFRIGHT;
				else				cf2=NULL;
			}
			if ((cf1|cf2)&OFFBOTTOM)
			{
				if (cf1&OFFBOTTOM) {SWord q=x1;x1=x2;x2=q;q=y1;y1=y2;y2=q;}
				Float frac=Float(bottom-y1)/Float(y2-y1);
				x2=x1+SWord(frac*(x2-x1));
				y2=bottom;
				if (x2<left)		cf2=OFFLEFT;
				else if (x2>right)	cf2=OFFRIGHT;
				else				cf2=NULL;
			}
			if ((cf1|cf2)&OFFLEFT)
			{
				if (cf1&OFFLEFT) {SWord q=x1;x1=x2;x2=q;q=y1;y1=y2;y2=q;}
				Float frac=Float(left-x1)/Float(x2-x1);
				y2=y1+SWord(frac*Float(y2-y1));
				x2=left;
				cf2=NULL;
			}
			if ((cf1|cf2)&OFFRIGHT)
			{
				if (cf1&OFFRIGHT) {SWord q=x1;x1=x2;x2=q;q=y1;y1=y2;y2=q;}
				Float frac=Float(right-x1)/Float(x2-x1);
				y2=y1+SWord(frac*Float(y2-y1));
				x2=right;
				cf2=NULL;
			}
		}
		if ((cf1|cf2)==0)
			DrawLine(x1,y1,x2,y2,col);
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DoArtHoriz
//Author		Paul.
//Date			Wed 5 May 1999
//------------------------------------------------------------------------------
void COverlay::DoArtHoriz()
{
	//display artificial horizon in the top right corner of the screen
	const SWord THREAT_X=640-64-10;
	const SWord THREAT_Y=10;
	const SWord THREAT_W=64;
	const SWord THREAT_H=64;

	GetSurfaceDimensions(physicalWidth,physicalHeight);
	const Float scx=Float(physicalWidth)/640.;
	const Float scy=Float(physicalHeight)/480.;
	const Float scaled_x=scx*Float(THREAT_X);
	const Float scaled_w=scx*Float(THREAT_W);
	const Float scaled_y=scy*Float(THREAT_Y);
	const Float scaled_h=scy*Float(THREAT_H);



	//make ball center image

	ImageMapDesc* pOuterImage=Image_Map.GetImageMapPtr(BALL03NO);	//128x128
	ImageMapDesc* pMaskImage=Image_Map.GetImageMapPtr(BALL02NO);	//128x128
	ImageMapDesc* pCentralImage=Image_Map.GetImageMapPtr(BALL01NO);	//256x256

	if (pball==NULL)
	{
		lastArtHorizOffset=0x7fffffff;
		pball=(ImageMapDesc*)(new UByte[sizeof(ImageMapDesc)+pMaskImage->w*pMaskImage->h*2]);
		pball->w=pMaskImage->w;
		pball->h=pMaskImage->h;
		pball->body=((UByte*)pball)+sizeof(ImageMapDesc);
		pball->alpha=pball->body+pball->w*pball->h;
		pball->palette=pMaskImage->palette;
		pball->paletteindex=pMaskImage->paletteindex;
		pball->isMasked=pMaskImage->isMasked;
		pball->hTextureMap=0UL;
		pball->blendType=BLEND_AVERAGEALPHA;
		UByte* pBallAlpha=pball->alpha;
		UByte* pMaskAlpha=pMaskImage->body;
		for (int w=0;w<pball->w*pball->h;w++)
			*pBallAlpha++=(*pMaskAlpha++==UByte(ARTWORKMASK))?0:0x7F;
	}

	//use a/c pitch to fill the ball image

	UByte *pDst,*pSrc,*pMsk,*pSrcEnd;

	pSrc=pCentralImage->body;
	pSrcEnd=pSrc+pCentralImage->w*pCentralImage->h;
	pDst=pball->body;
	pMsk=pMaskImage->body;

	pSrc+=pCentralImage->w>>2;

	int offset=(int(SWord(-Manual_Pilot.ControlledAC2->pitch))<<6)/int(ANGLES_90Deg)+64;

	if (offset!=lastArtHorizOffset)
	{
		pSrc+=offset*pCentralImage->w;

		for (int y=0;y<pball->w;y++)
		{
			UByte* pDst2=pDst;
			UByte* pMsk2=pMsk;
			UByte* pSrc2=pSrc;
			for (int x=0;x<pball->h;x++)
			{
				if (*pMsk2==UByte(ARTWORKMASK))	*pDst2=UByte(ARTWORKMASK);
				else							*pDst2=*pSrc2;
				pMsk2++;
				pDst2++;
				pSrc2++;
			}
			pDst+=pball->w;
			pMsk+=pball->w;
			pSrc+=pCentralImage->w;
			if (uintptr_t(pSrc)>=uintptr_t(pSrcEnd))
				pSrc-=pCentralImage->w*pCentralImage->h;
		}

	}

	SWord sinAng,cosAng;

	Math_Lib.high_sin_cos(Manual_Pilot.ControlledAC2->roll,sinAng,cosAng);

	Float m[4];

	//generate 2D rotation matrix

	m[0]=m[3]=Float(cosAng)/Float(ANGLES_FRACT);
	m[1]=Float(sinAng)/Float(ANGLES_FRACT);
	m[2]=-Float(sinAng)/Float(ANGLES_FRACT);

 	Lib3DPoint dp[4];
	dp[0].setPosition( scaled_x, scaled_y, 0.f );
	dp[0].setIMapCoords( 0, 0 );

	dp[1].setPosition( scaled_x+scaled_w, scaled_y, 0.f );
	dp[1].setIMapCoords( 128, 0 );

	dp[2].setPosition( scaled_x+scaled_w, scaled_y+scaled_h, 0.f );
	dp[2].setIMapCoords( 128, 128 );

	dp[3].setPosition( scaled_x, scaled_y+scaled_h, 0.f );
	dp[3].setIMapCoords( 0, 128 );

	PutC(pOuterImage,dp);

	//apply a/c roll to rotate the ball poly coordinates

	Float ori_x=0,ori_y=0;
	{
	for (int i=0;i<4;i++)
	{
		ori_x+=dp[i].getPosX();
		ori_y+=dp[i].getPosY();
	}
	}
	ori_x/=4;
	ori_y/=4;

	for (int i=0;i<4;i++)
	{
		Float tx=dp[i].getPosX()-ori_x;
		Float tz=dp[i].getPosY()-ori_y;
		dp[i].setPosition(
			m[0]*tx+m[1]*tz+ori_x,
			m[2]*tx+m[3]*tz+ori_y,
			0.f
		);
	}

	PutC(pball,dp,offset!=lastArtHorizOffset?true:false);


	lastArtHorizOffset=offset;
}

void COverlay::CrossHair()
{
	if((pvp) && (pvp->viewnum.viewmode == VM_InsideCheat))
	{
		GetSurfaceDimensions(physicalWidth, physicalHeight);
		Colour colour = Colour(127);
		FP halfwidth  = physicalWidth / 2;
		FP halfheight = physicalHeight / 2;

		DrawLine((1.0 - 0.15) * halfwidth, halfheight, (1.0 - 0.05) * halfwidth, halfheight, colour);
		DrawLine((1.0 + 0.15) * halfwidth, halfheight, (1.0 + 0.05) * halfwidth, halfheight, colour);
		DrawLine(halfwidth, (1.0 - 0.2) * halfheight, halfwidth, (1.0 - 0.07) * halfheight, colour);
		DrawLine(halfwidth, (1.0 + 0.2) * halfheight, halfwidth, (1.0 + 0.07) * halfheight, colour);

		FP radx = 0.1;
		FP rady = 0.13;
		FP x1, x2, y1, y2;
		x1 = halfwidth;
		y1 = halfheight * (1.0 + rady);
		const FP nolines = 8;
		const FP pi = M_PI;
		for(int i = 1; i <= nolines; i++)
		{
			x2 = (1.0 + radx * FSin(i * pi * (0.5 / nolines))) * halfwidth;
			y2 = (1.0 + rady * FCos(i * pi * (0.5 / nolines))) * halfheight;

			DrawLine(x1, y1, x2, y2, colour);
			DrawLine(physicalWidth - x1, y1, physicalWidth - x2, y2, colour);
			DrawLine(x1, physicalHeight - y1, x2, physicalHeight - y2, colour);
			DrawLine(physicalWidth - x1, physicalHeight - y1, physicalWidth - x2, physicalHeight - y2, colour);
			x1 = x2;
			y1 = y2;
		}
		DrawLine(halfwidth - 1, halfheight, halfwidth + 2, halfheight, colour);
		DrawLine(halfwidth, halfheight - 1, halfwidth, halfheight + 2, colour);
	}

}

void COverlay::DrawLine(SWord sx,SWord sy,SWord ex,SWord ey,Colour c)
{
	DoPointStruc start,finish;
	start.setPosition( sx, sy, 0.f );
	start.setRHW( 1.f );
	finish.setPosition( ex, ey, 0.f );
	finish.setRHW( 1.f );
	g_lpLib3d->DrawTransformedLine(HMATERIAL(c),start,finish);
}


void COverlay::DrawLine(SWord sx,SWord sy,SWord ex,SWord ey,ULong c)
{

	DoPointStruc start,finish;
	start.setPosition( sx, sy, 0.f );
	start.setRHW( 1.f );
	finish.setPosition( ex, ey, 0.f );
	finish.setRHW( 1.f );
	g_lpLib3d->SetFontColour( c );
	g_lpLib3d->DrawTransformedLine(FONTCOL_MATERIAL,start,finish);
}

void COverlay::TriggerMessage(COverlay::MessageType mt,UWord val)
{
	char* theString;
	char localText[64];
	switch (mt)
	{
	case NOVIEWMESS:
		theString=LdStr(IDS_VIEWNOTAVAILABLE);
		sprintf(localText,"%s",theString);
		delete[]theString;
		*otherText=CString(localText);
		messageTimer=5*ONE_SECOND_TIME;
		break;
	case IMPACTONMESS:
		theString=LdStr(IDUS_IMPACTTOGGLEON);
		sprintf(localText,"%s",theString);
		delete[]theString;
		*otherText=CString(localText);
		messageTimer=5*ONE_SECOND_TIME;
		break;
	case IMPACTOFFMESS:
		theString=LdStr(IDUS_IMPACTTOGGLEOFF);
		sprintf(localText,"%s",theString);
		delete[]theString;
		*otherText=CString(localText);
		messageTimer=5*ONE_SECOND_TIME;
		break;
	case SENSMESS:
		theString=LdStr(IDS_KEY0);									  //RDH 19/05/99
		sprintf(localText,"%s %d",theString,val);
		delete[]theString;
		*otherText=CString(localText);
		messageTimer=5*ONE_SECOND_TIME;
		break;
	case PAUSEMESS:
		*otherText=CString("");
		messageTimer=5*ONE_SECOND_TIME;
		break;
	case ACCELMESS:
		theString=LdStr(IDS_MAP_ACCEL);
		sprintf(localText,"%s X4",theString);
		delete[]theString;
		*otherText=CString(localText);
		messageTimer=0x7FFF;
		break;
	case CLEARMESS:
		if (!otherText->IsEmpty())
		{
			*otherText=CString("");
			messageTimer=0;
		}
		break;
	}
}

void COverlay::HotKeyTriggerMenu(SWord sel)
{
	if (pCurScr==NULL)
	{
		Key k=(Key)sel;
		localDec=NULL;

		AirStrucPtr player = Persons2::PlayerGhostAC;

		bool toweronly = false;
		if(		((Persons2::PlayerSeenAC->movecode == AUTO_TRAININGTAKEOFF)	|| (Persons2::PlayerSeenAC->movecode == AUTO_TRAININGLANDING))
			||	(Art_Int.CountFormationSize(Persons2::PlayerGhostAC) == 1)	)
			toweronly = true;

		switch (k)
		{
			case SEL_1:
				localDec = NULL;
				if(!toweronly)
				{
					if(player->formpos == 0)				localDec = GroupInfoMsg;
					else
						if(player->nationality == NAT_RAF)	localDec = GroupInfoMsgFolRaf;
						else								localDec = GroupInfoMsgFolLuf;
				}
				break;

			case SEL_2:
				localDec = NULL;
				if(!toweronly)
				{
					if(player->formpos == 0)
						if(player->nationality == NAT_RAF)	localDec = PreCombatMsg;
						else								localDec = PreCombatMsgLuf;
				}
				break;

			case SEL_3:
				localDec = NULL;
				if(!toweronly)
				{
					if(player->formpos == 0)				localDec = CombatMsg;
					else
						if(player->nationality == NAT_RAF)	localDec = CombatMsgFolRaf;
						else								localDec = CombatMsgFolLuf;
				}
				break;

			case SEL_4:
				localDec = NULL;
				if(!toweronly)
					if(player->formpos == 0)				localDec = PostCombatMsg;
				break;

			case SEL_5:
				localDec = TowerMsg;
			break;

			case SEL_6:
				localDec = NULL;
				if(!toweronly)
					if((player->formpos == 0) && ((player->classtype->visible == JU87) || (player->classtype->visible == ME110)))
						localDec = Strike;
			break;

			default:
				break;
		}
		if (localDec!=NULL)	SetToUIScreen(&userOptionsScr);
	}
}

void COverlay::HotKeyTriggerMessage(SWord sel1,SWord sel2)
{
	if (pCurScr==NULL)
	{
		DecisionAI* dec=NULL;
		Key k=(Key)sel1;
		localDec=NULL;

		AirStrucPtr player = Persons2::PlayerGhostAC;

		bool toweronly = false;
		if(		((Persons2::PlayerSeenAC->movecode == AUTO_TRAININGTAKEOFF)	|| (Persons2::PlayerSeenAC->movecode == AUTO_TRAININGLANDING))
			||	(Art_Int.CountFormationSize(Persons2::PlayerGhostAC) == 1)	)
			toweronly = true;

		switch (k)
		{
			case SEL_1:
				dec = NULL;
				if(!toweronly)
				{
					if(player->formpos == 0)				dec = GroupInfoMsg;
					else
						if(player->nationality == NAT_RAF)	dec = GroupInfoMsgFolRaf;
						else								dec = GroupInfoMsgFolLuf;
				}
				break;

			case SEL_2:
				dec = NULL;
				if(!toweronly)
				{
					if(player->formpos == 0)
						if(player->nationality == NAT_RAF)	dec = PreCombatMsg;
						else								dec = PreCombatMsgLuf;
				}
				break;

			case SEL_3:
				dec = NULL;
				if(!toweronly)
				{
					if(player->formpos == 0)				dec = CombatMsg;
					else
						if(player->nationality == NAT_RAF)	dec = CombatMsgFolRaf;
						else								dec = CombatMsgFolLuf;
				}
				break;

			case SEL_4:
				dec = NULL;
				if(!toweronly)
					if(player->formpos == 0)				dec = PostCombatMsg;
				break;

			case SEL_5:
				dec = TowerMsg;
			break;

			case SEL_6:
				dec = NULL;
				if(!toweronly)
					if((player->formpos == 0) && ((player->classtype->visible == JU87) || (player->classtype->visible == ME110)))
						dec = Strike;
			break;

			default:
				break;
		}
		if (dec!=NULL)
		{
			k=(Key)sel2;
			if (k>=SEL_1 && k<=SEL_9)
			{
				DecisionMessage(dec,
								(SWord)(k-SEL_1),
								Persons2::PlayerGhostAC,
								Persons2::PlayerGhostAC,
								Persons2::PlayerGhostAC);
			}
		}
	}
}

void COverlay::Kludge()
{
	KeyVal3D keyTable[]=
	{
	RPM_10,
	RPM_20,
	RPM_30,
	RPM_40,
	RPM_50,
	RPM_60,
	RPM_70,
	RPM_80,
	RPM_90,
	RPM_00,
	PADLOCKTOG,
	RESETVIEW,
	ROTUP,
	ROTDOWN,
	ROTLEFT,
	ROTRIGHT,
	PAUSEKEY,
	EXITKEY
	};
	const SLong keyCount=sizeof(keyTable)/sizeof(KeyVal3D);
	for (SLong i=keyCount-1;i>=0;i--) Key_Tests.KeyPress3d(keyTable[i]);
}

CString COverlay::ScrollyText::Scroll(CString& in,SWord ft)
{
	const SWord scrollDivision = 10;
	timer-=ft;
	if (timer<=0)
	{
		switch (state)
		{
		case WaitingToScroll:		state=ScrollingToEnd;		timer=ScrollToEndTime;		break;
		case ScrollingToEnd:		state=WaitingToScrollBack;	timer=WaitToScrollBackTime;	break;
		case WaitingToScrollBack:	state=ScrollingToFront;		timer=ScrollToFrontTime;	break;
		case ScrollingToFront:		state=WaitingToScroll;		timer=WaitToScrollTime;		break;
		}
	}
	switch (state)
	{
	case WaitingToScrollBack:
	case WaitingToScroll:
	break;
	case ScrollingToEnd:
	{
		static SWord timeCount = 0;
		timeCount+=ft;
		SWord timeMs = timeCount/scrollDivision;
		if ( timeMs >= 1 )
		{
			timeCount = 0;
			index+=CharsToShuffle(timeMs);
			if (index>targetIndex)
			{
				index=targetIndex;
				state=WaitingToScrollBack;
				timer=WaitToScrollBackTime;
			}
		}
	}
	break;
	case ScrollingToFront:
	{
		static SWord timeCount = 0;
		timeCount+=ft;
		SWord timeMs = timeCount/scrollDivision;
		if ( timeMs >= 1 )
		{
			timeCount = 0;
			index-=CharsToShuffle(timeMs);
			if (index<0)
			{
				index=0;
				state=WaitingToScroll;
				timer=WaitToScrollTime;
			}
		}
	}
	break;
	}
	return in.Right(in.GetLength()-index);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		DecisionMessage
//Author		Andy McMaster
//Date			Fri 21 May 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void COverlay::DecisionMessage(DecisionAI* dec,SWord option,AirStrucPtr callee,ItemBasePtr target,AirStrucPtr caller,Bool sayvox)
{
	if (_DPlay.Implemented || _Replay.Record)
	{
		UByte decision=dec->optionnumber;
		UniqueID trg;

		if (target)
			trg=target->uniqueID.count;
		else
			trg=UID_Null;

		_DPlay.NewWingmanCommand(decision,option,trg);
	}
	else
	{
	 	DecisionAI::OptionRef* opt=dec->GetMsgOptions();
	 	opt+=option;
//Poo		int		script;												//RJS 09Jun99
//Poo		if (sayvox)													//RJS 09Jun99
//Poo			script = opt->saidmsg;									//RJS 09Jun99
//Poo		else														//RJS 09Jun99
//Poo			script = 0;												//RJS 09Jun99

		MESSAGE_STRUC newMsg(opt->saidmsg,							//RJS 17Jun99
							MSG_HIPRIORITY,
							callee,
							target,
							caller);
		SendMessageAndCall(&newMsg,opt->airesult[0]);
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		AccelSelection
//Author		Andy McMaster
//Date			Thu 10 Jun 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	COverlay::AccelSelection(UByte hilight,ULong acceltype)
{
	_DPlay.AccelComms=(DPlay::AccelTypeComms)acceltype;

// No accel in comms quick missions allowed

	if (_DPlay.Implemented)											  //AMM 02/07/99
		return;														  //AMM 02/07/99


 	OverLay.hilight=(Key)hilight;
	OverLay.keyFlags=KF_ACCELON+KF_FASTACCEL;
	OverLay.SetPossibleLandscapeResfresh();

	switch (_DPlay.AccelComms)
	{
	case DPlay::ACCEL_FORWARD:
		Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_FWD);
		break;

	case DPlay::ACCEL_NEXTWP:
		Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_WAYPT);
		Manual_Pilot.acceltochanged=Manual_Pilot.ControlledAC2->waypoint;					  //JIM 18/04/99
		break;

	case DPlay::ACCEL_MESSAGESUBJECT:
		Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_MSG);
		break;

	case DPlay::ACCEL_MISSIONIP:
		Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_MISS);
		Manual_Pilot.acceltochanged=Manual_Pilot.ControlledAC2->waypoint->FindWP(WPNAME_Target);
		break;

	case DPlay::ACCEL_HOME:
		Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_MISS);
		Manual_Pilot.acceltochanged=Manual_Pilot.ControlledAC2->waypoint->FindWP(WPNAME_EscDisperse);
		break;

	case DPlay::ACCEL_WP_NEXTWP:
		OverLay.fZoomedMap=false;
		Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_WAYPT);
		Manual_Pilot.acceltochanged=Manual_Pilot.ControlledAC2->waypoint;
		OverLay.curr_waypoint=Manual_Pilot.ControlledAC2->waypoint;
		break;

	case DPlay::ACCEL_WP_HIGHLIGHTED_WP:
		OverLay.fZoomedMap=false;
		Manual_Pilot.AutoToggle(ManualPilot::AUTOACCEL_WAYPT);
		Manual_Pilot.acceltochanged=OverLay.curr_waypoint;
		break;
	}
}

void	COverlay::HitLine(Float mx,Float my,Float hx1,Float hx2,Float hy1,Float hy2)
{
#ifdef	_XYCHEAT_
	sprintf(hitlinestr,"%d , %d   --> %d , %d -> %d , %d",int(mx),int(my),int(hx1),int(hy1),int(hx2),int(hy2));
#endif
}
void COverlay::PutC(ImageMapDesc *pmap,const Lib3DPoint* dp,bool forceReload)
{

	Lib3DPoint *pRVerts;
	if (forceReload)
		g_lpLib3d->UnloadTexture(pmap);

	R3DVALUE hx=R3DVALUE(physicalWidth>>1);
	R3DVALUE hy=R3DVALUE(physicalHeight>>1);
	R3DVALUE sx=1.f/hx;
	R3DVALUE sy=-1.f/hy;

	pRVerts = g_lpLib3d->BeginPoly(HMATERIAL(pmap),4);
	*pRVerts=dp[0];
	pRVerts->setPosition( (pRVerts->getPosX()-hx)*sx, (pRVerts->getPosY()-hy)*sy, 0.f );
	*(++pRVerts)=dp[1];
	pRVerts->setPosition( (pRVerts->getPosX()-hx)*sx, (pRVerts->getPosY()-hy)*sy, 0.f );
	*(++pRVerts)=dp[2];
	pRVerts->setPosition( (pRVerts->getPosX()-hx)*sx, (pRVerts->getPosY()-hy)*sy, 0.f );
	*(++pRVerts)=dp[3];
	pRVerts->setPosition( (pRVerts->getPosX()-hx)*sx, (pRVerts->getPosY()-hy)*sy, 0.f );
	g_lpLib3d->EndPoly();

}

void COverlay::SmokedGlassBox(SLong x,SLong y,SLong width,SLong height)
{

	Lib3DPoint *dp;
	SLong hx=physicalWidth>>1;
	SLong hy=physicalHeight>>1;
	R3DVALUE sx=1.f/R3DVALUE(hx);
	R3DVALUE sy=-1.f/R3DVALUE(hy);
	dp = g_lpLib3d->BeginPoly(HMATERIAL(Image_Map.GetImageMapPtr(RADIONO)),4);
	dp[0].setPosition(
		R3DVALUE(x-hx)*sx,
 		R3DVALUE(y-hy)*sy,
		0.f
	);
	dp[0].setIMapCoords( 8, 8 );

	dp[1].setPosition(
		R3DVALUE(x+width-hx)*sx,
 		R3DVALUE(y-hy)*sy,
		0.f
	);
	dp[1].setIMapCoords( 16, 8 );

	dp[2].setPosition(
		R3DVALUE(x+width-hx)*sx,
		R3DVALUE(y+height-hy)*sy,
		0.f
	);
	dp[2].setIMapCoords( 16, 16 );

	dp[3].setPosition(
		R3DVALUE(x-hx)*sx,
		R3DVALUE(y+height-hy)*sy,
		0.f
	);
	dp[3].setIMapCoords( 8, 16 );

	g_lpLib3d->EndPoly();

}


inline void COverlay::SmokedGlassBox(const SLong x,const SLong y,const SLong width,const SLong height,const ULong col, ULong alpha)
{
	g_lpLib3d->SetGlobal( TOGGLE_GLOBALALPHA, &alpha );
	g_lpLib3d->SetFontColour(col);
	LIGHTFLAG lf = LF_FONT;
	g_lpLib3d->SetObjectLighting( lf );

	Lib3DPoint *dp;
	SLong hx=physicalWidth>>1;
	SLong hy=physicalHeight>>1;
	R3DVALUE sx=1.f/R3DVALUE(hx);
	R3DVALUE sy=-1.f/R3DVALUE(hy);
	dp = g_lpLib3d->BeginPoly(UByte(0),4); // used only in 2d render therefore draw in font col
	dp[0].setPosition(
		R3DVALUE(x-hx)*sx,
 		R3DVALUE(y-hy)*sy,
		0.f
	);

	dp[1].setPosition(
		R3DVALUE(x+width-hx)*sx,
 		R3DVALUE(y-hy)*sy,
		0.f
	);

	dp[2].setPosition(
		R3DVALUE(x+width-hx)*sx,
		R3DVALUE(y+height-hy)*sy,
		0.f
	);

	dp[3].setPosition(
		R3DVALUE(x-hx)*sx,
		R3DVALUE(y+height-hy)*sy,
		0.f
	);

	g_lpLib3d->EndPoly();

	g_lpLib3d->SetObjectLighting( lf );
	g_lpLib3d->SetGlobal( TOGGLE_GLOBALALPHA, alpha );  // set globalpha back

}


void	COverlay::ShapeCheat(char*	shpdebugline)
{
#ifndef NDEBUG
	strcpy(shpdebug,shpdebugline);
#endif
}
//////////////////////////////////////////////////////////////////////
//
// Function:    CharPixelWidth
// Date:		10/10/00
// Author:		JON
//
//Description: returns the width of a char in pixels
//
//////////////////////////////////////////////////////////////////////
inline SLong COverlay::CharPixelWidth( const char theChar )
{
	return (!fIsWide)?7/*+extra*/:bigWidths[theChar-32]+1/*+extra*/;
}



//////////////////////////////////////////////////////////////////////
//
// Function:    StrPixelWidthPadded

// Date:		10/10/00
// Author:		JON
//
//Description:  returns a padded verion of the pixel width
//
//////////////////////////////////////////////////////////////////////
inline SLong		COverlay::StrPixelWidthPadded(const char*	theString)
{
	SLong pixelwidth;

	if ( !fIsWide )
	{
		pixelwidth = SLong(strlen( theString )*(8/*+extra*/));
	} else
	{
		const unsigned char*	str =(const unsigned char*)theString;
		unsigned char	c;
		pixelwidth=0;

		while (*str!=0)
		{
			c=NextOverlayChar((const char*&)str);
			if (c>=32)
 				pixelwidth+=bigWidths[c-32]+1/*+extra*/+1;
		}
	}

	return pixelwidth;
}


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PixelWidth
//Author		Robert Slater
//Date			Tue 18 Apr 2000
//
//Description	return width of a string in pixels
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
inline SLong		COverlay::StrPixelWidth(const char*	theString)
{
	SLong pixelwidth;

	if ( !fIsWide )
	{
		pixelwidth = SLong(strlen( theString )*(7/*+extra*/));
	} else
	{
		const char*	str = theString;
		char	c;
		pixelwidth=0;

		while (*str!=0)
		{
			unsigned char uc=NextOverlayChar(str);
			if (uc>=32)
 				pixelwidth+=bigWidths[uc-32]+1/*+extra*/;
		}
	}

	return pixelwidth;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PrintCentered
//Author		Robert Slater
//Date			Tue 18 Apr 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	COverlay::PrintCentered(float&	sx, float& sy, const char* theString, const ULong& theCol)
{
	PrintCentered(SWord(sx),SWord(sy),theString,theCol);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PrintCentered
//Author		Robert Slater
//Date			Tue 18 Apr 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	COverlay::PrintCentered(SWord	sx, SWord sy, const char* theString, const ULong& theCol)
{
	if (nameTableCnt < MaxNames)
	{
		int	stringwidth = StrPixelWidth(theString);

		NameTable[nameTableCnt].x = sx - (stringwidth>>1);
		NameTable[nameTableCnt].y = sy;
		NameTable[nameTableCnt].colour = theCol;
		strcpy(NameTable[nameTableCnt++].text,theString);
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PrintAt
//Author		Robert Slater
//Date			Tue 18 Apr 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	COverlay::PrintAt(Coords3D&	pos, CString&	theString, const ULong& theCol)
{

	R3DVERTEX	avertex;
	D3DVALUE	sx,sy,sz,zscale;

	avertex.x = pos.X - pvp->World.X;
	avertex.y = pos.Y - pvp->World.Y;
	avertex.z = pos.Z - pvp->World.Z;

	g_lpLib3d->BodyToScreen(avertex,sx,sy,sz,&zscale);
	if (	(sx < R3DVALUE(physicalWidth))
		&&	(sx > 0.0)
		&&	(sy < R3DVALUE(physicalHeight))
		&&	(sy > 0.0)											//RJS 10Aug00
		&&	(sz > 1.)						)					//RJS 10Aug00
	{
		const char*		theText = (const char*)theString;

		PrintCentered(sx,sy,theText,theCol);
	}
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PrintAt
//Author		Robert Slater
//Date			Tue 18 Apr 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	COverlay::PrintAt(ItemPtr	theItem, CString&	theString, const ULong& theCol)
{
	Coords3D	pos;

	pos = theItem->World;

	if(theItem->Status.size >= MOBILESIZE)
	{
		MobileItemPtr 	mobitem = MobileItemPtr(theItem);
		SLong			dx,dy,dz;

		fastMath.FloatToInt(&dx,float(Float(mobitem->vel_x + Three_Dee.wind_x)*Three_Dee.fView_dt_frac));
		fastMath.FloatToInt(&dy,float(Float(mobitem->vel_y + Three_Dee.wind_y)*Three_Dee.fView_dt_frac));
		fastMath.FloatToInt(&dz,float(Float(mobitem->vel_z + Three_Dee.wind_z)*Three_Dee.fView_dt_frac));

		pos.X += dx;
		pos.Y += dy;
		pos.Z += dz;
	}

	PrintAt(pos,theString,theCol);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		PrintID
//Author		Robert Slater
//Date			Tue 2 May 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	COverlay::PrintID(ItemPtr	theItem)
{
#ifndef	_NOTFORDEMO_
	if (
		(theItem != Persons2::PlayerSeenAC)			)
	{
		if (	!theItem->Status.deadtime								//RJS 5Oct00
			&&	(theItem->Anim != NULL)		)
		{
			MinAnimData*	mad = (MinAnimData*)theItem->Anim;
			if (!mad->IsInvisible)
			{
				AirStrucPtr		ac = AirStrucPtr(theItem);
				CString			acID = "";
				PhraseBuffer	phrasebuff;
				PhraseTables	thePhrase;
				ULong			theCol;

				if (ac->uniqueID.commsmove && _DPlay.Implemented)		//CSB 12Jul00
				{
					_Radio.ExpandMessage(phrasebuff,MESSAGE_STRUC(PHRASE_COMMSTARGET,MSG_HIPRIORITY,VOICE_PLAYER,ac,ac));

					switch (_DPlay.GameType)
					{
					case DPlay::DEATHMATCH:
						theCol = 0x0000FF00;		//green
						break;

					case DPlay::TEAMPLAY:
					case DPlay::COMMSQUICKMISSION:
						switch (ac->classtype->planetext)
						{
						case PT_SPIT_A:
						case PT_SPIT_B:
						case PT_HURR_A:
						case PT_HURR_B:
							theCol = 0x0000FF00;		//green
							break;

						case PT_ME109:
						case PT_ME110:
						case PT_JU87:
							theCol = 0x00FFFF; // yellow
							break;
						}
						break;
					}
					_MsgBuffer.PhraseToString(acID,phrasebuff);
				}
				else
				{
					if (ac->nationality == Persons2::PlayerSeenAC->nationality)
					{
						_Radio.ExpandCallsign(phrasebuff,ac,VOICE_PLAYER);
						_MsgBuffer.PhraseToString(acID,phrasebuff);
						theCol = 0x00FFFFFF;
					}
					else
					{
						//Radio code can't say blenheim
						char* str=LdStr(IDS_L_SPIT_A+ac->classtype->planetext);
						acID=str;
						delete []str;
						theCol = 0x00FF0000;
					}
				}


				PrintAt(ac,acID,theCol);
			}
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Function:    SaveScreenShot
// Date:		22/06/00
// Author:		JIM
//
//Description:
//
//////////////////////////////////////////////////////////////////////
bool	ScreenShot::SaveScreenShot(CLib3D* g_lpLib3d)
{
	struct BitMapWithMasks
	{
		BITMAPINFOHEADER header;
		ULong	extraspace[30];
	};

	BitMapWithMasks	bmi=
	{sizeof(BITMAPINFOHEADER),0,0,1,0,BI_RGB,0,100000,100000,0,0};
	int	headersize=g_lpLib3d->ScreenShot(&bmi.header,NULL);
	char* buffer=new char [((bmi.header.biWidth*bmi.header.biBitCount/8+3)&-4)*bmi.header.biHeight];
	if (buffer==NULL)
		return false;
	int	datasize=g_lpLib3d->ScreenShot(&bmi.header,buffer);
	bmi.header.biSizeImage=datasize;
	BITMAPFILEHEADER fileheader	={'MB',sizeof(BITMAPFILEHEADER)+headersize+datasize,0,0,sizeof(BITMAPFILEHEADER)+headersize};
	char dirname[80];
	char filename[]={'s','h','o','t','_','0'+(Save_Data.screenshotframe/100)%10,'0'+(Save_Data.screenshotframe/10)%10,'0'+(Save_Data.screenshotframe/1)%10,'.','b','m','p',0};
	Save_Data.screenshotframe++;
	BOStream bos(File_Man.NameNumberedFile(File_Man.fakefile(FIL_STILLS_START_DIR,filename),dirname));
	bos.write(UByteP(&fileheader),sizeof(fileheader));
	bos.write(UByteP(&bmi),headersize);
	bos.write(buffer,datasize);
	delete [] buffer;
	return true;
}

//////////////////////////////////////////////////////////////////////
//
// Function:    SaveVideoStream
// Date:		22/06/00
// Author:		JIM
//
//Description:
//
//////////////////////////////////////////////////////////////////////
bool	ScreenShot::SaveVideoStream(CLib3D* g_lpLib3d)
{
	if (screenformat)
	{
		int datasize=((screenformat->biWidth*screenformat->biBitCount/8+3)&-4)*screenformat->biHeight;
		screenformat->biSizeImage=datasize;
	 	BITMAPFILEHEADER fileheader	={'MB',sizeof(BITMAPFILEHEADER)+headersize+datasize,0,0,sizeof(BITMAPFILEHEADER)+headersize};
		int i=currframe+1;
		do
		{
			if (framebuffer[i])
			{
 				char dirname[80];
				char filename[]={'s','h','o','t','_','0'+(Save_Data.screenshotframe/100)%10,'0'+(Save_Data.screenshotframe/10)%10,'0'+(Save_Data.screenshotframe/1)%10,'.','b','m','p','\0'};
				Save_Data.screenshotframe++;
				BOStream bos(File_Man.NameNumberedFile(File_Man.fakefile(FIL_STILLS_START_DIR,filename),dirname));
				bos.write(UByteP(&fileheader),sizeof(fileheader));
				bos.write(UByteP(screenformat),headersize);
				bos.write(framebuffer[i],datasize);
			}
			if (++i==maxframe)	i=0;
		}while (i!=currframe+1);
		return true;
	}
	else
		return SaveScreenShot(g_lpLib3d);
}

#if SCREENSHOT_MODE == SCREENSHOT_STILLS
CON	ScreenShot::ScreenShot()	//STILLS
{
		angledelta=pixeldelta=currpixeloffset=0;
	framebuffer=NULL;currframe=maxframe=0;screenformat=NULL;minframetime=maxfametime=0;
}
#endif

#if SCREENSHOT_MODE == SCREENSHOT_VIDEO
CON	ScreenShot::ScreenShot()	//VIDEOS
{
	angledelta=pixeldelta=currpixeloffset=0;
	framebuffer=NULL;maxframe=50;currframe=0;screenformat=NULL;minframetime=maxfametime=80;
}
#endif

#if SCREENSHOT_MODE == SCREENSHOT_PANORAMIC
CON	ScreenShot::ScreenShot()	//PANORAMIC
{
	angledelta=-10;		//the delta to the camera heading per frame
	pixeldelta=1;		//the  pixels written per frame
	currpixeloffset=0;	//this means recording is ongoing!
	framebuffer=NULL;maxframe=50;currframe=0;screenformat=NULL;
	minframetime=maxfametime=0;
}
#endif

//////////////////////////////////////////////////////////////////////
//
// Function:    VideoShot
// Date:		22/06/00
// Author:		JIM
//
//Description:
//
//////////////////////////////////////////////////////////////////////
bool	ScreenShot::VideoShot(CLib3D* g_lpLib3d)
{
	if (maxframe)
	{
		if (!framebuffer)
		{
			framebuffer=new UByte*[maxframe];
			for (int i=0;i<maxframe;i++)
				framebuffer[i]=0;
		}
		if (screenformat==NULL)
		{
			struct BitMapWithMasks
			{
				BITMAPINFOHEADER header;
				ULong	extraspace[30];
			};

			static BitMapWithMasks	bmi=
			{sizeof(BITMAPINFOHEADER),0,0,1,0,BI_RGB,0,100000,100000,0,0};
			screenformat=&bmi.header;
			headersize=g_lpLib3d->ScreenShot(screenformat,NULL);
		}
		if (framebuffer[currframe]==NULL)
			framebuffer[currframe]=new UByte[((screenformat->biWidth*screenformat->biBitCount/8+3)&-4)*screenformat->biHeight];


	int	oldwid=screenformat->biWidth;
	if (pixeldelta)
	{
		screenformat->biWidth=pixeldelta;
	}
	g_lpLib3d->ScreenShot(screenformat,framebuffer[currframe]+currpixeloffset*screenformat->biBitCount/8);

	currpixeloffset+=pixeldelta;
	if (pixeldelta==0 || currpixeloffset>=oldwid)
	{
		currframe++;
		currpixeloffset=0;
	}

	screenformat->biWidth=oldwid;



		if (currframe==maxframe)
			currframe=0;
		return true;
	}
	else
		return false;

}

void	COverlay::SetUp3DUIMouse()
{
	if (!MouseSet)
	{
		MouseSet=true;
		MouseRestore=true;
	}
}

void	COverlay::RestoreMouse()
{
	if (MouseRestore)
	{
	}
}

void	COverlay::AddPeripheralBlob(const float	sx, const float sy, bool	nat)
{
	if (pBlobCnt < 32)
	{
		pBlob[pBlobCnt].sx = sx;
		if (!pBlob[pBlobCnt].sx)	pBlob[pBlobCnt].sx = 1;

		pBlob[pBlobCnt].sy = sy;
		if (!pBlob[pBlobCnt].sy)	pBlob[pBlobCnt].sy = 1;

		pBlob[pBlobCnt++].nat = nat;
	}
}

void	COverlay::DrawPeripheralBlobs()
{
	SWord xc[2],yc[2];
	SWord	blob_x, blob_y;
	UWord cf;
	SWord	halfWidth = OverLay.physicalWidth>>1;
	SWord	halfHeight = OverLay.physicalHeight>>1;

	xc[0]=1;
	xc[1]=OverLay.physicalWidth-1;
	yc[0]=1;
	yc[1]=OverLay.physicalHeight-1;

	for (int index = 0; index < pBlobCnt; index++)
	{
		cf=MapClipFlags(pBlob[index].sx,pBlob[index].sy,xc,yc);
		ClipPoint(pBlob[index].sx,pBlob[index].sy,cf,xc,yc);

		blob_x = pBlob[index].sx;
		blob_y = pBlob[index].sy;

		if (blob_x > halfWidth)	blob_x -= 20;							//RJS 6Sep00
		if (blob_y > halfHeight)	blob_y -= 20;						//RJS 6Sep00

		if (pBlob[index].nat)
			DrawPoly(MAPCHAPNO,blob_x,blob_y,20,20,16,33,15,14);		//RJS 6Sep00
		else
			DrawPoly(MAPCHAPNO,blob_x,blob_y,20,20,16,1,15,14);			//RJS 6Sep00
	}

	pBlobCnt = 0;
}

#ifndef	NDEBUG
void	COverlay::Diagnostic(char*	dstr)
{
	strcpy(diagnosticstring,dstr);
}
#endif

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
//Procedure		SetInteractiveMode
//Author		Robert Slater
//Date			Wed 23 Aug 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	COverlay::SetInteractiveMode()
{
	disableInteractiveMouse = false;

	if (pCurScr!=pNewScr)
	{
		if (pNewScr)
			disableInteractiveMouse = true;
	}
	else if (pCurScr)
	{
		disableInteractiveMouse = true;
	}
}

void	COverlay::DrawPoly(const int theMap, const SWord& xpos, const SWord& ypos,
							const SWord& width, const SWord& height,
							const SWord& ix0, const SWord& iy0,
							const SWord& iwidth, const SWord& iheight	)
{
	ImageMapDesc* pmap=Image_Map.GetImageMapPtr(ImageMapNumber(theMap));

	g_lpLib3d->SetFontColour(0xFFFFFFFF);

 	Lib3DPoint dp[4];

	dp[0].setPosition( xpos, ypos, 0.f );
	dp[0].setIMapCoords( ix0, iy0 );
	dp[1].setPosition( xpos+width, ypos, 0.f );
	dp[1].setIMapCoords( ix0+iwidth, iy0 );
	dp[2].setPosition( xpos+width, ypos+height, 0.f );
	dp[2].setIMapCoords( ix0+iwidth, iy0+iheight );
	dp[3].setPosition( xpos, ypos+height, 0.f );
	dp[3].setIMapCoords( ix0, iy0+iheight );

	PutC(pmap,dp);
}

void	COverlay::PreLoadImageMaps()
{
	Image_Map.AddImageMapToGlbList(UWord(SPLUME1NO));
	Image_Map.AddImageMapToGlbList(UWord(SPLUME2NO));
	Image_Map.AddImageMapToGlbList(UWord(SPLUME3NO));
	Image_Map.AddImageMapToGlbList(UWord(SPLUME4NO));
	Image_Map.AddImageMapToGlbList(UWord(PLUME2NO));
	Image_Map.AddImageMapToGlbList(UWord(PLUME3NO));
}
