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
//Filename	missinit.cpp
//System
//Author	 R. Hyde
//Date		Thu 7 Dec 1995
//Description
//------------------------------------------------------------------------------
#include	"dosdefs.h"											//DAW 29Feb96
#define F_SOUNDS												//DAW 05Aug96
#define F_COMMON
#include	"files.g"		//NON-GRAPHICS FILES				//JIM 14May96
//#include	"ui.h"
#include	"uniqueid.h"
//TEMPCODE DAW 13/04/00 #include	"briefing.h"												//JIM 30Apr96
#include	"package.h"
#include	"missman2.h"	//special double-include
#undef	MISSMAN2_Included	//special double-include
#include	"missman2.h"	//special double-include
#include	"mytime.h"
#include	"mymath.h"											//RDH 04Jan96
#include	"ranges.h"											//RDH 04Jan96
#include	"node.h"
#include	"ENUMBITS.M"
#include "savegame.h"
#include "persons2.h"
#include	"sqddiary.h"
#include "nodebob.h"

Bool	beentomap;												//RDH 29Nov96
MATHABLE	(SquadSelect);
ATMOSPHERE&	MissManCampSky()	{return Miss_Man.camp.Sky;}
int	MissManDawnTime()			{return Miss_Man.camp.dawntime;}
int	MissManDuskTime()			{return Miss_Man.camp.dusktime;}
int	MissManSunSouth()			{return Miss_Man.camp.sunsouth;}


//enum repeated here because one in missman2.h wasn't accessed by this file?
enum					{MAX_FLIGHT_NUM = 4,MAX_AC_IN_FLIGHT_NUM = 4};
typedef	Profile	AM;
typedef	Profile		BR;
typedef Profile	SM;
typedef	Directives::LW::Targ	Flags;
startinst(CampaignPackages, CampaignConvoy_Def)
	/*garbage*/		sizeof(CampaignPackages),
	/*introsmack*/		FIL_VID_PHASE1BEGINS,
	/*IntroQuarterArt*/	FIL_NULL,

	/*playerhomebase*/	UID_AF_Abbeville,							//MS 06Apr99
	/*playertakeoff*/	UID_AF_Abbeville,							//MS 06Apr99
	/*firsttime*/	true,
	/*startdate, in secs +BASECENTURY, 1-5-17*/	JUL10th1940,
	/*ripdate, in secs +BASECENTURY, 1-5-17*/	JUL10th1940,
	/*currdate, in secs +BASECENTURY, 1-5-17*/	JUL10th1940,
	/*lastdate*/								AUG11th1940,

	/*currtime*/		MORNINGPERIODSTART,				//can only be the start time
	/*currperiodtime*/	MORNINGPERIODSTART,
	/*dawntime*/		HR06,
	/*dusktime*/		HR20,
	/*sunsouth*/		SUNELEVDIST,
	/*sl_category*/		1,												//RDH 01/03/00
	/*playerrank*/		SECLIEUTENANT,
	/*MoraleType*/		SKILL_REGULAR1,
	/*acesavail*/		0,
	/*reservesavail*/	12,
	/*currentplayerslot*/	0,
	/*name*/				"Name",
	/*aircraftname*/		"Silver Eagle",
	/*front*/				{UID_NULL},//UID_SdBlPyongtaek,UID_SdRdWonju,UID_StKangnung},//MS 06Apr99
	/*Directives::LW::HistTypes*/	Directives::LW::LWHISTORICALPHASE1A,
	/*Directives::RAF::HistTypes*/	Directives::RAF::HISTORICALSCENARIO_PHASE1,
	/*camp_win_anim*/	 FIL_NULL,
	/*camp_lost_anim*/	 FIL_NULL,
	/*directives*/
	{
		{//LW
			/*userprofiles*/
			{
				{
					/*escortmethods*/		{0,4,32,32},		//MS 16Nov00
					/*acselectionrules*/	BR::BR_WIDESTPOSSIBLE,
					/*formations*/			{FTS_ASTERN, FTS_ASTERN, FTS_ASTERN, FTS_ASTERN},	//4
					/*bomberattackmethod*/	AM::AM_DECOYPATROL,
					/*bombers,attfighters,detfighters,retfighters*/
					12,12,12,0,
					/*NumTargets*/	1,
					/*mainalt*/	   FT_10000
				},
				{
					/*escortmethods*/		{0,4,32,32},		//MS 16Nov00
					/*acselectionrules*/	BR::BR_WIDESTPOSSIBLE,
					/*formations*/			{FTS_ASTERN, FTS_ASTERN, FTS_ASTERN, FTS_ASTERN},	//4
					/*bomberattackmethod*/	AM::AM_RECON,
					/*bombers,attfighters,detfighters,retfighters*/
					1,0,0,0,
					/*NumTargets*/		1,
					/*mainalt*/	   FT_15000

				},
				{
					/*escortmethods*/		{0,4,32,32},		//MS 16Nov00
					/*acselectionrules*/	BR::BR_WIDESTPOSSIBLE,
					/*formations*/			{FTS_ASTERN, FTS_ASTERN, FTS_ASTERN, FTS_ASTERN},	//4
					/*bomberattackmethod*/	AM::AM_LEVELBOMB,
					/*bombers,attfighters,detfighters,retfighters*/
					12,12,12,0,
					/*NumTargets*/	1,
					/*mainalt*/	   FT_15000

				}
			},
			/*current*/
			{
 				/*targetlines*/
				{		/*attached,detached, flags,			bombersperraid*/	/*acproportions*/
					{	0.0},

				},
				{0}
			},
			/*histdir*/
			{
				{//LWHISTORICALPHASE1A
//TempCode MS 30Aug00  						//targetlines
//TempCode MS 30Aug00 						{	/*attached,detached, flags,	  2ndtrg,	bombers*/		/*me109,me110, ju87, do17, ju88, he111,  */
//TempCode MS 30Aug00 /*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{1.0, 1.0, 1.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 1.0, 1.0, 1.0}},
//TempCode MS 30Aug00 						},
//TempCode MS 30Aug00 						/*reconn*/			{0,0,false,false,PT_ME110},
//TempCode MS 30Aug00 						/*morningquota*/	0.3,
//TempCode MS 30Aug00 						/*middayquota*/		0.4,
//TempCode MS 30Aug00 						/*escortmethods*/	 {0},		//AM_HIGH
//TempCode MS 30Aug00 						/*arrivalminsseperation*/	15,		//2=coordinated;15 is consecutive.
//TempCode MS 30Aug00 						/*ju87*/			Directives::LW::JU_ANY,
//TempCode MS 30Aug00 						/*escort*/			0.50
 						//targetlines
						{	/*attached,detached, flags,	  2ndtrg,	bombers*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{1.0, 1.0, 1.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 1.0, 1.0, 1.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.3,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},		//AM_HIGH
						/*arrivalminsseperation*/	15,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_ANY,
						/*escort*/			0.50
				},

				{//LWHISTORICALPHASE1B
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.2, 0.2, 0.2, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.8, 0.8, 0.8, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 1.0, 1.0, 1.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.3,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},		//AM_HIGH
						/*arrivalminsseperation*/	15,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_ANY,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE1C
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.5, 0.0, 0.0, 0.0, 0.0, 1.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.5, 0.0, 0.5, 0.0, 0.0}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.5, 0.5, 1.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.5, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.3,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},		//AM_HIGH
						/*arrivalminsseperation*/	15,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_ANY,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE1D
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{1.0, 0.0, 0.6, 0.0, 0.0, 0.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.5, 0.0, 0.5, 0.0, 0.0}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.5, 0.4, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.3,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},		//AM_HIGH
						/*arrivalminsseperation*/	15,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_ANY,
						/*escort*/			0.50
				},
//TempCode MS 30Aug00 				{//LWHISTORICALPHASE2A
//TempCode MS 30Aug00  						/*targetlines*/
//TempCode MS 30Aug00 						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
//TempCode MS 30Aug00 /*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{1.0, 1.0, 1.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 1.0, 1.0, 1.0}},
//TempCode MS 30Aug00 						},
//TempCode MS 30Aug00 						/*reconn*/			{0,0,false,false,PT_ME110},
//TempCode MS 30Aug00 						/*morningquota*/	0.3,
//TempCode MS 30Aug00 						/*middayquota*/		0.4,
//TempCode MS 30Aug00 						/*escortmethods*/	 {0},	//AM_CEILING
//TempCode MS 30Aug00 						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
//TempCode MS 30Aug00 						/*ju87*/			Directives::LW::JU_AM,
//TempCode MS 30Aug00 						/*escort*/			0.50
//TempCode MS 30Aug00 				},
//THIS ONE
				{//LWHISTORICALPHASE2A
 						/*targetlines*/
						{	/*attached,detached, flags,	secondarytrg,bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	1.0, 0.0,	  Flags::NONE,0,	1,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,		{0.3, 0.5, 0.5, 0.0, 0.0, 0.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,		{0.2, 0.0, 0.0, 0.0, 0.0, 0.2}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,		{0.3, 0.0, 0.5, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,		{0.4, 0.5, 0.0, 1.0, 1.0, 0.8}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.4,
						/*middayquota*/		0.3,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_ANY,
						/*escort*/			0.50
				},

//TempCode MS 30Aug00 				{//LWHISTORICALPHASE2B
//TempCode MS 30Aug00  						/*targetlines*/
//TempCode MS 30Aug00 						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
//TempCode MS 30Aug00 /*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,1,	30,			{0.5, 0.5, 0.0, 1.0, 1.0, 1.0}},
//TempCode MS 30Aug00 /*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.5, 0.5, 1.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 /*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 30Aug00 						},
//TempCode MS 30Aug00 						/*reconn*/			{0,0,false,false,PT_ME110},
//TempCode MS 30Aug00 						/*morningquota*/	0.3,
//TempCode MS 30Aug00 						/*middayquota*/		0.4,
//TempCode MS 30Aug00 						/*escortmethods*/	 {0},	//AM_CEILING
//TempCode MS 30Aug00 						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
//TempCode MS 30Aug00 						/*ju87*/			Directives::LW::JU_AM,
//TempCode MS 30Aug00 						/*escort*/			0.50
//TempCode MS 30Aug00 				},
				{//LWHISTORICALPHASE2B
 						/*targetlines*/
						{	/*attached,detached, flags,	secondarytrg,bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	1.0, 0.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.5, 0.5,	  Flags::NONE,0,	30,		{0.8, 0.5, 0.0, 1.0, 1.0, 0.5}},
/*DOCKS		*/				{	0.5, 0.5,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,		{0.2, 0.5, 1.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.5, 0.5,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.5, 0.5,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	0.5, 0.5,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.5, 0.5,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.5, 0.5,	  Flags::NONE,0,	30,		{0.0, 0.0, 0.0, 0.0, 0.0, 0.5}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.3,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	8,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_ANY,
						/*escort*/			0.50
				},

				{//LWHISTORICALPHASE2C NOTUSED!!!!!!!!!!!!!!!!!!
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.5, 0.5, 0.0, 1.0, 1.0, 1.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.5, 0.5, 1.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.4,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_AM,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE2D	NOTUSED!!!!!!!!!!!!!!!!!!
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
	/*RECON		*/			{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
	/*AIRFIELDS	*/			{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.5, 0.5, 0.0, 1.0, 1.0, 1.0}},
	/*DOCKS		*/			{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
	/*RADARS	*/			{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.5, 0.5, 1.0, 0.0, 0.0, 0.0}},
	/*CONVOYS	*/			{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
	/*LONDON	*/			{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
	/*FACTORIES	*/			{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
	/*SWEEPS	*/			{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
	/*RESTING	*/			{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.4,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_AM,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE3A
//TempCode MS 28Aug00  						/*targetlines*/
//TempCode MS 28Aug00 						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
//TempCode MS 28Aug00 /*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{1.0, 1.0, 0.0, 1.0, 1.0, 1.0}},
//TempCode MS 28Aug00 /*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 						},
//TempCode MS 28Aug00 						/*reconn*/			{0,0,false,false,PT_ME110},
//TempCode MS 28Aug00 						/*morningquota*/	0.4,
//TempCode MS 28Aug00 						/*middayquota*/		0.4,
//TempCode MS 28Aug00 						/*escortmethods*/	 {0},	//AM_CEILING
//TempCode MS 28Aug00 						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
//TempCode MS 28Aug00 						/*ju87*/			Directives::LW::JU_AM,
//TempCode MS 28Aug00 						/*escort*/			0.50
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	1.0, 0.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	1.0, 0.0,	  Flags::NONE,1,	30,					{0.9, 1.0, 0.0, 1.0, 0.7, 1.0}},
/*DOCKS		*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.1, 0.0, 0.0, 0.0, 0.2, 0.0}},
/*SWEEPS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.2,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	8,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_NONE,
						/*escort*/			0.50
				},
//TempCode MS 11Sep00 				{//LWHISTORICALPHASE3B
//TempCode MS 11Sep00  						/*targetlines*/
//TempCode MS 11Sep00 						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
//TempCode MS 11Sep00 /*RECON		*/				{	1.0, 0.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 11Sep00 /*AIRFIELDS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.8, 0.8, 0.0, 1.0, 1.0, 1.0}},
//TempCode MS 11Sep00 /*DOCKS		*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 11Sep00 /*RADARS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
//TempCode MS 11Sep00 /*CONVOYS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 11Sep00 /*LONDON	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 11Sep00 /*FACTORIES	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 11Sep00 /*SWEEPS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 11Sep00 /*RESTING	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 11Sep00 						},
//TempCode MS 11Sep00 						/*reconn*/			{0,0,false,false,PT_ME110},
//TempCode MS 11Sep00 						/*morningquota*/	0.4,
//TempCode MS 11Sep00 						/*middayquota*/		0.4,
//TempCode MS 11Sep00 						/*escortmethods*/	 {0},	//AM_CEILING
//TempCode MS 11Sep00 						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
//TempCode MS 11Sep00 						/*ju87*/			Directives::LW::JU_AM,
//TempCode MS 11Sep00 						/*escort*/			0.50
//TempCode MS 11Sep00 				},
				{//LWHISTORICALPHASE3B
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	1.0, 0.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	1.0, 0.0,	  Flags::NONE,1,	30,					{1.0, 0.5, 0.0, 1.0, 1.0, 1.0}},
/*DOCKS		*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.5, 1.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.2,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_AM,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE3C
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	1.0, 0.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	1.0, 0.0,	  Flags::NONE,0,	40,					{0.0, 1.0, 0.0, 0.0, 0.0, 0.5}},
/*DOCKS		*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	1.0, 0.0,	  Flags::NONE,0,	40,					{1.0, 0.0, 0.0, 1.0, 1.0, 0.5}},
/*SWEEPS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0  , 0  , 0  , 0  , 0  , 0  }},
/*RESTING	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.4,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_NONE,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE3D !!!!!!!!!!!!!!!!not used!!!!!!!!!!!!!!!!!!!!
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{1.0, 1.0, 1.0, 1.0, 1.0, 1.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.4,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_AM,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE4A
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
//TempCode MS 28Aug00 /*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*LONDON	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{1.0, 1.0, 0.0, 1.0, 1.0, 1.0}},
//TempCode MS 28Aug00 /*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
//TempCode MS 28Aug00 /*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
/*RECON		*/				{	1.0, 0.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*DOCKS		*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	1.0, 0.0,	  Flags::NONE,3,	30,					{1.0, 1.0, 0.0, 1.0, 1.0, 1.0}},
/*FACTORIES	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.2,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	8,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_NONE,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE4B	 //use for sep 15th
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	1.0, 0.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*DOCKS		*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	1.0, 0.0,	  Flags::NONE,5,	30,					{1.0, 1.0, 0.0, 1.0, 1.0, 1.0}},
/*FACTORIES	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	1.0, 0.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.1,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_NONE,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE4C	!!!not used!!!
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{1.0, 1.0, 0.0, 1.0, 1.0, 1.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.4,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_NONE,
						/*escort*/			0.50
				},
				{//LWHISTORICALPHASE4D	!!!not used!!!
 						/*targetlines*/
						{	/*attached,detached, flags,			secondarytrg,			bombersperraid*/		/*me109,me110, ju87, do17, ju88, he111,  */
/*RECON		*/				{	0.0, 1.0,	  Flags::NONE,0,	1,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*AIRFIELDS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*DOCKS		*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RADARS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*CONVOYS	*/				{	0.0, 1.0,	  Flags::FIGHTERSSTRAFFE,0,	30,			{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*LONDON	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{1.0, 1.0, 0.0, 1.0, 1.0, 1.0}},
/*FACTORIES	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*SWEEPS	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
/*RESTING	*/				{	0.0, 1.0,	  Flags::NONE,0,	30,					{0.0, 0.0, 1.0, 0.0, 0.0, 0.0}},
						},
						/*reconn*/			{0,0,false,false,PT_ME110},
						/*morningquota*/	0.4,
						/*middayquota*/		0.4,
						/*escortmethods*/	 {0},	//AM_CEILING
						/*arrivalminsseperation*/	2,		//2=coordinated;15 is consecutive.
						/*ju87*/			Directives::LW::JU_NONE,
						/*escort*/			0.50
				},
				},
		},
 		{//RAF
			{/*userprofiles*/
				{//PATROL
						/*acselectionrules*/ BR::BR_B_ANYRAF,
						/*packageattackmethod*/		 AM::AM_PATROL,
						/*mainalt*/			 FT_15000,
					{//hurr
						/*numac*/			0,
						/*what*/			SM::SM_NOTFIGHTERS,
						/*formation*/		FTW_RAFVIC
					},
					{//spit
						/*numac*/			6,
						/*what*/			SM::SM_NOTBOMBERS,
						/*formation*/		FTW_RAFVIC
					},
				},
				{//ANTISWEEP
						/*acselectionrules*/ BR::BR_B_ANYRAF,
						/*packageattackmethod*/		 AM::AM_INTERCEPT,
						/*mainalt*/			 FT_15000,
					{//hurr
						/*numac*/			0,
						/*what*/			SM::SM_NOTFIGHTERS,
						/*formation*/		FTW_RAFVIC
					},
					{//spit
						/*numac*/			12,
						/*what*/			SM::SM_NOTBOMBERS,
						/*formation*/		FTW_RAFVIC
					},
				},
				{//ANTISINGLE
						/*acselectionrules*/ BR::BR_B_ANYRAF,
						/*packageattackmethod*/		 AM::AM_INTERCEPT,
						/*mainalt*/			 FT_15000,
					{//hurr
						/*numac*/			3,
						/*what*/			SM::SM_NOTFIGHTERS,
						/*formation*/		FTW_RAFVIC
					},
					{//spit
						/*numac*/			0,
						/*what*/			SM::SM_NOTBOMBERS,
						/*formation*/		FTW_RAFVIC
					},
				},
				{//ANTIUNESC
						/*acselectionrules*/ BR::BR_B_ANYRAF,
						/*packageattackmethod*/		 AM::AM_INTERCEPT,
						/*mainalt*/			 FT_15000,
					{//hurr
						/*numac*/			12,
						/*what*/			SM::SM_NOTFIGHTERS,
						/*formation*/		FTW_RAFVIC
					},
					{//spit
						/*numac*/			0,
						/*what*/			SM::SM_NOTBOMBERS,
						/*formation*/		FTW_RAFVIC
					},
				},
				{//ANTIESC
						/*acselectionrules*/ BR::BR_B_ANYRAF,
						/*packageattackmethod*/		 AM::AM_INTERCEPT,
						/*mainalt*/			 FT_15000,
					{//hurr
						/*numac*/			12,
						/*what*/			SM::SM_NOTFIGHTERS,
						/*formation*/		FTW_RAFVIC
					},
					{//spit
						/*numac*/			0,	//12			//MS 12Nov00
						/*what*/			SM::SM_NOTBOMBERS,
						/*formation*/		FTW_RAFVIC
					},
			},
//TempCode MS 28Oct00 //WARNING - THIS IS A TEST
//TempCode MS 28Oct00 				{//ANTIESC
//TempCode MS 28Oct00 						/*acselectionrules*/ BR::BR_B_ANYRAF,
//TempCode MS 28Oct00 						/*packageattackmethod*/		 AM::AM_INTERCEPT,
//TempCode MS 28Oct00 						/*mainalt*/			 FT_17000,
//TempCode MS 28Oct00 					{//hurr
//TempCode MS 28Oct00 						/*numac*/			12,
//TempCode MS 28Oct00 						/*what*/			SM::SM_NOTFIGHTERS,
//TempCode MS 28Oct00 						/*formation*/		FTW_RAFVIC
//TempCode MS 28Oct00 					},
//TempCode MS 28Oct00 					{//spit
//TempCode MS 28Oct00 						/*numac*/			0,
//TempCode MS 28Oct00 						/*what*/			SM::SM_NOTBOMBERS,
//TempCode MS 28Oct00 						/*formation*/		FTW_RAFVIC
//TempCode MS 28Oct00 					},
//TempCode MS 28Oct00 				},
				{//ANTIBIG
						/*acselectionrules*/ BR::BR_B_ANYRAF,
						/*packageattackmethod*/		 AM::AM_INTERCEPT,
						/*mainalt*/			 FT_15000,
					{//hurr
//TempCode MS 28Aug00 						/*numac*/			12,
						/*numac*/			12,		//36,		//MS 12Nov00
						/*what*/			SM::SM_NOTFIGHTERS,
						/*formation*/		FTW_RAFVIC
					},
					{//spit
//TempCode MS 28Aug00 						/*numac*/			12,
						/*numac*/			0,		//24,		//MS 12Nov00
						/*what*/			SM::SM_NOTBOMBERS,
						/*formation*/		FTW_RAFVIC
					},
				},
//TempCode MS 28Oct00 //WARNING - THIS IS A TEST
//TempCode MS 28Oct00 				{//ANTIBIG
//TempCode MS 28Oct00 						/*acselectionrules*/ BR::BR_B_ANYRAF,
//TempCode MS 28Oct00 						/*packageattackmethod*/		 AM::AM_INTERCEPT,
//TempCode MS 28Oct00 						/*mainalt*/			 FT_17000,
//TempCode MS 28Oct00 					{//hurr
//TempCode MS 28Oct00 						/*numac*/			12,
//TempCode MS 28Oct00 						/*what*/			SM::SM_NOTFIGHTERS,
//TempCode MS 28Oct00 						/*formation*/		FTW_RAFVIC
//TempCode MS 28Oct00 					},
//TempCode MS 28Oct00 					{//spit
//TempCode MS 28Oct00 						/*numac*/			0,
//TempCode MS 28Oct00 						/*what*/			SM::SM_NOTBOMBERS,
//TempCode MS 28Oct00 						/*formation*/		FTW_RAFVIC
//TempCode MS 28Oct00 					},
//TempCode MS 28Oct00 				},
			},
			{/*current*/
				/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
				/*approach*/												AM::AM_INTERCEPT,
				/*restifcatbelow,restifavailbelow*/							2,6,
				/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
				/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
				/*convoylevel*/												1.0,
				/*interceptbeforetarget*/									true,
				/*unitsize*/												Directives::RAF::Dir::ONESQ,
			},
			{
				{/*HISTORICALSCENARIO_PHASE1*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.0,0.3,0.0,//MS 12Nov00
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.0,0.0,0.0,//MS 12Nov00
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_PHASE2*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.1,0.0,0.0,//MS 16Nov00
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.0,0.0,0.0,//MS 12Nov00
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_PHASE3*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,3,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.0,0.0,0.0,//MS 12Nov00
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.0,0.0,0.0,//MS 12Nov00
					/*convoylevel*/												0.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_PHASE4*/
//TempCode MS 28Aug00 					/*sanctioned*/												Directives::RAF::ANTIESC,
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,3,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.0,0.0,0.0,//MS 12Nov00
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.0,0.0,0.0,//MS 12Nov00
					/*convoylevel*/												0.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::BIGW,
				},
				{/*HISTORICALSCENARIO_A*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_B*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_C*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_D*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_E*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_F*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_G*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
				{/*HISTORICALSCENARIO_H*/
					/*sanctioned*/												Directives::RAF::MTB_ANTIALL-Directives::RAF::MTB_ANTISWEEP,
					/*approach*/												AM::AM_INTERCEPT,
					/*restifcatbelow,restifavailbelow*/							2,6,
					/*gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol*/		0.3,0.1,0.0,
					/*gr1012afpat,gr1012acfactpat,gr1012supfactpat*/			0.3,0.0,0.0,
					/*convoylevel*/												1.0,
					/*interceptbeforetarget*/									true,
					/*unitsize*/												Directives::RAF::Dir::ONESQ,
				},
			}
 		}
	},
endinst(CampaignPackages, CampaignConvoy_Def);
//------------------------------------------------------------------------------
startinst(CampaignStatic, CampaignRadar_Def)
	/*garbage*/		sizeof(CampaignStatic),
	/*introsmack*/		FIL_VID_PHASE2BEGINS,
	/*IntroQuarterArt*/	FIL_NULL,

	/*playerhomebase*/	UID_AF_Caffiers,							//MS 19Aug96
	/*playertakeoff*/	UID_AF_Caffiers,							//MS 19Aug96
	/*firsttime*/	true,
	/*startdate, in secs +BASECENTURY, 1-5-17*/	JUL10th1940,
	/*ripdate, in secs +BASECENTURY, 1-5-17*/	AUG12th1940,
	/*currdate, in secs +BASECENTURY, 1-5-17*/	AUG12th1940,
	/*lastdate*/		AUG23rd1940,
	/*currtime*/		MORNINGPERIODSTART,				//can only be the start time
	/*currperiodtime*/	MORNINGPERIODSTART,
	/*dawntime*/		HR06,
	/*dusktime*/		HR18,
	/*sunsouth*/		SUNELEVDIST,
	/*sl_category*/		1,												//RDH 01/03/00
	/*playerrank*/		SECLIEUTENANT,
	/*MoraleType*/		SKILL_REGULAR1,
	/*acesavail*/		0,
	/*reservesavail*/	12,
	/*currentplayerslot*/	0,
	/*name*/				"Name",
	/*aircraftname*/		"Silver Eagle",
//TempCode MS 06Apr99 	/*front*/				{UID_StKimchon,UID_StAndong,UID_StYonghae},//MS 22Apr99
/*front*/				{UID_NULL},//UID_StKumi,UID_StAndong,UID_StYonghae},//MS 22Apr99 //RJS 16/11/99
	/*Directives::LW::HistTypes*/	Directives::LW::LWHISTORICALPHASE2A,
	/*Directives::RAF::HistTypes*/	Directives::RAF::HISTORICALSCENARIO_PHASE2,
	/*camp_win_anim*/	 FIL_NULL,
	/*camp_lost_anim*/	 FIL_NULL,

endinst(CampaignStatic, CampaignRadar_Def);
//------------------------------------------------------------------------------
startinst(CampaignStatic, CampaignAirfields_Def)
	/*garbage*/		sizeof(CampaignStatic),
	/*introsmack*/		FIL_VID_PHASE3BEGINS,
	/*IntroQuarterArt*/	FIL_NULL,

	/*playerhomebase*/	UID_AF_Caffiers,							//MS 19Aug96
	/*playertakeoff*/	UID_AF_Caffiers,							//MS 19Aug96
	/*firsttime*/	true,
	/*startdate, in secs +BASECENTURY, 1-5-17*/	JUL10th1940,
	/*ripdate, in secs +BASECENTURY, 1-5-17*/	AUG24th1940,
	/*currdate, in secs +BASECENTURY, 1-5-17*/	AUG24th1940,
	/*lastdate*/		SEP6th1940,
	/*currtime*/		MORNINGPERIODSTART,				//can only be the start time
	/*currperiodtime*/	MORNINGPERIODSTART,
	/*dawntime*/		HR06,
	/*dusktime*/		HR18,
	/*sunsouth*/		SUNELEVDIST,
	/*sl_category*/		1,												//RDH 01/03/00
	/*playerrank*/		SECLIEUTENANT,
	/*MoraleType*/		SKILL_REGULAR1,
	/*acesavail*/		0,
	/*reservesavail*/	12,
	/*currentplayerslot*/	0,
	/*name*/				"Name",
	/*aircraftname*/		"Silver Eagle",
	/*front*/				{UID_NULL},//UID_MyBlSeoul,UID_StChuncheon,UID_StKansang},//MS 09Jun99
	/*Directives::LW::HistTypes*/	Directives::LW::LWHISTORICALPHASE3A,
	/*Directives::RAF::HistTypes*/	Directives::RAF::HISTORICALSCENARIO_PHASE3,
	/*camp_win_anim*/	 FIL_NULL,
	/*camp_lost_anim*/	 FIL_NULL,


endinst(CampaignStatic, CampaignAirfields_Def);
//------------------------------------------------------------------------------
startinst(CampaignStatic, CampaignBlitz_Def)
	/*garbage*/		sizeof(CampaignStatic),
	/*introsmack*/		FIL_VID_PHASE4BEGINS,
	/*IntroQuarterArt*/	FIL_NULL,

	/*playerhomebase*/	UID_AF_Caffiers,							//MS 19Aug96
	/*playertakeoff*/	UID_AF_Caffiers,							//MS 19Aug96
	/*firsttime*/	true,
	/*startdate, in secs +BASECENTURY, 1-5-17*/	JUL10th1940,
	/*ripdate, in secs +BASECENTURY, 1-5-17*/	SEP7th1940,
	/*currdate, in secs +BASECENTURY, 1-5-17*/	SEP7th1940,
	/*lastdate*/		SEP15th1940,
	/*currtime*/		MORNINGPERIODSTART,				//can only be the start time
	/*currperiodtime*/	MORNINGPERIODSTART,
	/*dawntime*/		HR06,
	/*dusktime*/		HR18,
	/*sunsouth*/		SUNELEVDIST,
	/*sl_category*/		1,												//RDH 01/03/00
	/*playerrank*/		SECLIEUTENANT,
	/*MoraleType*/		SKILL_REGULAR1,
	/*acesavail*/		0,
	/*reservesavail*/	12,
	/*currentplayerslot*/	0,
	/*name*/				"Name",
	/*aircraftname*/		"Silver Eagle",
	/*front*/				{UID_NULL},//UID_MyRdPyongyang,UID_StSamdung,UID_MyRlWonsan},//MS 14May99
	/*Directives::LW::HistTypes*/	Directives::LW::LWHISTORICALPHASE4A,
	/*Directives::RAF::HistTypes*/	Directives::RAF::HISTORICALSCENARIO_PHASE4,
	/*camp_win_anim*/	 FIL_NULL,
	/*camp_lost_anim*/	 FIL_NULL,

endinst(CampaignStatic, CampaignBlitz_Def);
//------------------------------------------------------------------------------

startinst(CampaignStatic, CampaignScramble_Def)
	/*garbage*/		sizeof(CampaignStatic),
	/*introsmack*/		FIL_VID_PHASE4BEGINS,
	/*IntroQuarterArt*/	FIL_NULL,

	/*playerhomebase*/	UID_AF_Caffiers,							//MS 19Aug96
	/*playertakeoff*/	UID_AF_Caffiers,							//MS 19Aug96
	/*firsttime*/	true,
	/*startdate, in secs +BASECENTURY, 1-5-17*/	JUL10th1940,
	/*ripdate, in secs +BASECENTURY, 1-5-17*/	SEP7th1940,
	/*currdate, in secs +BASECENTURY, 1-5-17*/	SEP7th1940,
	/*lastdate*/		SEP15th1940,
	/*currtime*/		MORNINGPERIODSTART,
	/*currperiodtime*/	MORNINGPERIODSTART,
	/*dawntime*/		HR06,
	/*dusktime*/		HR18,
	/*sunsouth*/		SUNELEVDIST,
	/*sl_category*/		1,												//RDH 01/03/00
	/*playerrank*/		SECLIEUTENANT,
	/*MoraleType*/		SKILL_REGULAR1,
	/*acesavail*/		0,
	/*reservesavail*/	12,
	/*currentplayerslot*/	0,
	/*name*/				"Name",
	/*aircraftname*/		"Silver Eagle",

	/*front*/				{UID_NULL},//UID_SdBlPyongtaek,UID_SdRdWonju,UID_SdRdYangyang},
	/*Directives::LW::HistTypes*/	Directives::LW::LWHISTORICALPHASE1A,
	/*Directives::RAF::HistTypes*/	Directives::RAF::HISTORICALSCENARIO_PHASE1,
	/*camp_win_anim*/	 FIL_NULL,
	/*camp_lost_anim*/	 FIL_NULL,

endinst(CampaignStatic, CampaignScramble_Def);



//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//nasty global!!!!!!!!!!!!!!!!!!!!!!!!!!!

//MARK, this is the list of events structures					//MS 04Mar99

//MARK, this is the list of events structures					//MS 04Mar99

#define FIL(f1,f2)		FIL_##f1,FIL_##f2
#define FIL2(f1)		FIL_##f1,FIL_##f1
#define FILTO(f1)		FIL_##f1,FIL_##f1##_TO

#define BAN(f1,f2)		BANDIT##f1,BANDIT##f2

//MissMan	Miss_Man ;
MissMan::CampaignName	MissMan::currcampaignnum	= MissMan::INTROCAMPAIGN;
CampaignBase			*MissMan::campaigntable[MAXCAMPAIGNNUM]
						={
							&CampaignConvoy_Def,						//RDH 03/04/00
							&CampaignRadar_Def,							//RDH 03/04/00
							&CampaignAirfields_Def,							//RDH 19/03/99
							&CampaignBlitz_Def,							//RDH 19/03/99
							&CampaignScramble_Def		//scramble
						 };


Campaign&	Campaign::operator=(CampaignBase*	b)
{
	char	*s=(char*)b,
			*t=(char*)this;

	int i;
	for ( i=b->garbage;i--;)
		*t++=*s++;
	s=(char*)&CampaignConvoy_Def + b->garbage;
	for ( i=CampaignConvoy_Def.garbage-b->garbage;i--;)
		*t++=*s++;
	for ( i=sizeof(Campaign)-CampaignConvoy_Def.garbage;i--;)
		*t++=0;
	garbage=sizeof(Campaign);
	CampaignInit();
	return *this;
}
Campaign&	Campaign::operator+=(CampaignBase*	b)
{
	char	*s=(char*)b,
			*t=(char*)this;

	for (int i=b->garbage;i--;)
		*t++=*s++;
	return *this;
}
Campaign&	Campaign::operator=(CampaignZero&	b)
	{
		return operator=(&b);
	}

Campaign&	Campaign::operator+=(CampaignZero&	b)
	{
		return operator+=(&b);
	}


void	Campaign::CampaignInit()
{
	Squad_Diary.EmptyDiary();											//RJS 9Oct00
	strncpy(MMC.PlayerName,Save_Data.CommsPlayerName,PLAYERNAMELEN-1); MMC.PlayerName[PLAYERNAMELEN-1]=0;

	InitIcons();		//also loads and initialises nodetree!
	curracceltype=ACCEL_RAIDSPD;
	accelrates[ACCEL_PAUSED]=0;
	accelrates[ACCEL_NORMAL]=1;
	accelrates[ACCEL_RAIDSPD]=20;	//600;
	accelrates[ACCEL_RECONNSPD]=40;
	accelrates[ACCEL_NONRAIDSPD]=300;
	accelrates[ACCEL_DIALOGSPEED]=1;
	{for (int i=SQ_BR_START+1;i<SQ_BR_END;i++)
			thisweekreview.Note(Node_Data[SquadNum(i)].AcType(),PeriodReview::AVAILSTART,Node_Data[SquadNum(i)].acavail);
	}
	{for (int i=SQ_LW_START+1;i<SQ_MAX;i++)
		if (GruppenPtr(Node_Data[SquadNum(i)])->periodsuntilready!=Gruppen::BUSY_IN_EUROPE)
			thisweekreview.Note(Node_Data[SquadNum(i)].AcType(),PeriodReview::AVAILSTART,Node_Data[SquadNum(i)].acavail);
	}
	fav.decelon=0;
	fav.flyon = 0;
	fav.flyon |= AS_SMALL_ENGAGEMENT;
	fav.flyon |= AS_LARGE_ENGAGEMENT;
	fav.flyon |= AS_LOSSES;
	fav.flyon |= AS_TARG;
	fav.flyonfav = 0;
	fav.geschwadertype = 4;	//WN_BETTERRULE;		//this is to get "ANY" to start with
	Dead_Stream.ClrWorldDead();
//TEMPCODE JIM 29/02/00 	Squad_Diary.EmptyDiary();

	Todays_Packages.WipeAll();

	LWDirectivesResults::SetUpLWDirectives(lwdir);
	RAFDirectivesResults::SetUpRAFDirectives(rafdir);

	if (Todays_Packages.localplayer==NAT_RAF)
	{
		Save_Data.mapfilters|=FILT_RAFINTERCEPTS;
		Save_Data.mapfilters|=FILT_RAFPATROLS;
		Save_Data.mapfilters|=FILT_LWARROWS;
		Save_Data.mapfilters%=	FILT_RECONN;
		Save_Data.mapfilters%=FILT_BOMBERS;


	}
	else
	{
		Save_Data.mapfilters%=FILT_RAFINTERCEPTS;
		Save_Data.mapfilters%=FILT_RAFPATROLS;
		Save_Data.mapfilters%=FILT_LWARROWS;
		Save_Data.mapfilters|=FILT_RECONN;
		Save_Data.mapfilters|=FILT_BOMBERS;
	}

	Save_Data.mapfilters|=FILT_LWTARGETLINES;

	Save_Data.mapfilters|=FILT_RAFSAG;
	Save_Data.mapfilters|=FILT_LWSAG;


										//DAW 15/05/00
	Sky.SetMissionConditions ();

	playeracnum = 0;
		playersquadron=0;
}

bool	Campaign::ChkEndCampaign()
{
	bool retval = false;
	wincamp = false;


	return(retval);
}
//////////////////////////////////////////////////////////////////////
//
// Function:    SetMissionConditions
// Date:		29/01/00
// Author:		DAW
//
//Description: 	Atmosphere stuff that reads the campaign structure directly
//
//////////////////////////////////////////////////////////////////////
void Atmosphere::SetMissionTemp (SWord month)
{
	int maxt[12] =	{3,5,8,12,20,25,25,25,23,15,10,5};
	int mint[12] =	{-10,-10,-5,0,2,8,10,10,8,6,0,-6};
	int maxtemp, mintemp;
	FP		delta;
	FP		hr12 = SECSPERDAY/2;



	maxtemp = maxt[month];
	mintemp = mint[month];

	delta = MMC.currtime - hr12;
	if (delta < 0 )
		delta = - delta;
	delta = hr12 - delta;
	Temp0 = (maxtemp - mintemp) * delta / hr12;
	FP var = Math_Lib.rnd (7);
	var = var - 3;
	TempVar = TempVar + var;
	if (TempVar > 10)
		TempVar = 10;
	if (TempVar < -10)
		TempVar = -10;
	Temp0 = 273 + Temp0 + TempVar + mintemp;

}
void Atmosphere::SetMissionPress (SWord month)
{



	FP delta;
	delta = month - 6;
	if (delta < 0)
		delta = - delta;
	if (delta < 3)
	{//Summer
		Press0 = 1001 + TempVar;
	}else
	{
		Press0 = 999 - TempVar;
	}

	ContrailBandCentre = FEET2CM(32000);		//CSB 10/06/99
	ContrailBandWidth = FEET2CM(4000);	//CSB 10/06/99

	FP ChanceBad = 0.5 + 0.5 * FCos(month * 6.2831853 / 12.0);		//CSB 02/07/99
	ChanceBad = 819.2 + 8192.0 * ChanceBad;						//CSB 02/07/99
	if(Math_Lib.rnd() > ChanceBad)									//CSB 02/07/99
	{
		Conditions = 0;
		Visibility = 4000000;

	}
	else
	{
		Visibility = 4000000;			//##lower vis possible here
		if (Temp0 < 273)
			Conditions = 2;
		else
			Conditions = 1;
	}

	if (		(Press0 < 980)
			&&	(MMC.currtime <  HR09)
		)
		MistInValley = true;
	else
		MistInValley = false;

//this will be a function of temp and press
	CloudLayer = 457200;
}
void Atmosphere::SetMissionConditions ()
{
	FCRD wind_at_zero, wind_at_alt;

  	SWord	month = 0;
	SWord	day = 0,year = 0;
	Math_Lib.DateFromSecs (MMC.currdate, day,month,year);

	SetMissionTemp (month);
	SetMissionPress (month);
	SetMissionDensity();
	SetMissionClouds();

	SetMissionWind(month);

	SWord Latitude = 51;												//RDH 25/02/00
	SetMissionDawnDusk(Latitude, MMC.currdate, MMC.dawntime, MMC.dusktime);
}

//////////////////////////////////////////////////////////////////////
//
// Function:    NextMission
// Date:		29/01/00
// Author:		DAW
//
//Description:
//
//////////////////////////////////////////////////////////////////////
void	Campaign::NextMission()
{
//init and test code



	Sky.SetMissionConditions ();



//handle debrief counters and data
//moved to EndBebrief to avoid date being wrong in debrief		  //RDH 12/06/99


//moved to EndBebrief to avoid date being wrong in debrief		  //RDH 12/06/99




}
void	Campaign::ModifySkill(int groupnum, int daysonduty)
{
}
void	Campaign::RotateFlightsCheck()
{
}
void	Campaign::ReplenishAcStock()
{
}
void	Campaign::RotateMiGSquadron(int squad)
{
}
void	Campaign::ChkRotateMiGSquadrons()
{

}
void	Campaign::ModifyMiGAggressionLevel(int squad)
{
}
void	Campaign::NextDay()
{
	currdate += SECSPERDAY;

//should be later
//	EndDebrief();


}




//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DecayingLossRate
//Author		Jim
//Date			Mon 5 Jun 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
float& LastWeekReview::DecayingLossRate(PlaneTypeSelect	actype)
{
	switch(actype)
	{
	case	PT_SPIT_A:
	case	PT_SPIT_B:	return rafloss[0];
	case	PT_HURR_A:
	case	PT_HURR_B:	return rafloss[1];
	case	PT_ME109:	return lwloss[0];
	case	PT_ME110:	return lwloss[1];
	case	PT_JU87:	return lwloss[2];
	case	PT_DO17:
	case	PT_JU88:
	case	PT_HE111:;
	}
	return lwloss[3];
}


//////////////////////////////////////////////////////////////////////
//
// Function:    Note
// Date:		25/05/00
// Author:		JIM
//
//Description: Returns current level of the stat
//
//////////////////////////////////////////////////////////////////////
void LastWeekReview::SetNote(enum PlaneTypeSelect pt,enum PeriodReview::ReviewType rv,int newvalue)
{
	TypeStats* record;
	switch(pt)
	{
	default_neverreferenced;
	case	PT_SPIT_A:
	case	PT_SPIT_B:	record=raf+0;	break;
	case	PT_HURR_A:
	case	PT_HURR_B:	record=raf+1;	break;
	case	PT_ME109:	record=lw+0;	break;
	case	PT_ME110:	record=lw+1;	break;
	case	PT_JU87:	record=lw+2;	break;
	case	PT_DO17:	record=lw+3;	break;
	case	PT_JU88:	record=lw+4;	break;
	case	PT_HE111:	record=lw+5;	break;
	}
	record->totalkills[rv]=newvalue;
}

int LastWeekReview::Note(enum PlaneTypeSelect pt,enum PeriodReview::ReviewType rv,int delta)
{
	if (delta==0)
		return 0;
	TypeStats* record=raf;int offset=0;
	switch(pt)
	{
	default_neverreferenced;
	case	PT_SPIT_A:
	case	PT_SPIT_B:	record=raf;offset=+0;		break;
	case	PT_HURR_A:
	case	PT_HURR_B:	record=raf;offset=+1;		break;
	case	PT_ME109:	record=lw;offset=+0;		break;
	case	PT_ME110:	record=lw;offset=+1;		break;
	case	PT_JU87:	record=lw;offset=+2;		break;
	case	PT_DO17:	record=lw;offset=+3;		break;
	case	PT_JU88:	record=lw;offset=+4;		break;
	case	PT_HE111:	record=lw;offset=+5;		break;
	}
	switch(rv)
	{
	case DESTROY:
		if (pt>PT_GER_FLYABLE)
		{
			lwtodaylosses+=delta;
			if (offset>2)
				lwloss[2]+=delta;
			else
				lwloss[offset]+=delta;

			lw[offset].totallosses += delta;							//RJS 12Nov00
		}
		else
		{
			raftodaylosses+=delta;
			if (pt>=PT_HURR_A)
				acproductiondelta[1]-=delta;
			else
				acproductiondelta[0]-=delta;
			rafloss[offset]+=delta;

			raf[offset].totallosses += delta;							//RJS 12Nov00
		}
	break;
	case CREATE:
		if (pt<PT_GER_FLYABLE)
			if (pt>=PT_HURR_A)
				acproductiondelta[1]+=delta;
			else
				acproductiondelta[0]+=delta;
	break;
	}
	return (record[offset].totalkills[rv]+=delta);
}

int LastWeekReview::Note(enum PlaneTypeSelect claimer,enum PlaneTypeSelect claim,int delta)
{
	if (delta==0)
		return 0;
	LastWeekReview::ReviewType rv=KILL2;
	switch(claim)
	{
	case	PT_ME109:
	case	PT_SPIT_A:
	case	PT_SPIT_B:	rv=KILL0;	break;
	case	PT_ME110:
	case	PT_HURR_A:
	case	PT_HURR_B:	rv=KILL1;	break;
	default:;
	}
	return Note(claimer,rv,delta);
}

void	LastWeekReview::ClearToday()
{
	if (lwtodaylosses>=lwrecordlosses)
		lwrecordlosses=lwtodaylosses+5;	//won't report until this delta exceeded
	if (raftodaylosses>=rafrecordlosses)
		rafrecordlosses=raftodaylosses+5;	//won't report until this delta exceeded
	lwtodaylosses=0;
	raftodaylosses=0;
	if (acproductiondelta[0]<0)
		dayssincedeltapos[0]++;
	if (acproductiondelta[1]<0)
		dayssincedeltapos[1]++;
	acproductiondelta[0]=acproductiondelta[1]=0;
	rafloss[0]*=0.90;
	rafloss[1]*=0.90;
	lwloss[0]*=0.90;
	lwloss[1]*=0.90;
	lwloss[2]*=0.90;
}
