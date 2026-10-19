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
//Filename       msgai.cpp
//System
//Author         Jim Taylor
//Date           Tue 28 May 1996
//Description    Beginning of a new AI version.
//
//In this version, the aircraft are processed by stepping through a sparse array,
//rather than a packed array so that their moment of action can seem
//nondeterministic
//
//
//jobs
//1	Consider AddMessage for playerac
//
//
//
//
//
//------------------------------------------------------------------------------
#include	"dosdefs.h"
	#define F_COMMON
	#define F_GRAFIX
	#define F_BATTLE
#include	"files.g"
#include	"worldinc.h"
#include	"ai.h"
#include	"myerror.h"
#include	"mymath.h"
#include	"landscap.h"
#include	"ranges.h"
#include	"planetyp.h"
#include	"mytime.h"
#include	"flymodel.h"
#include	"persons2.h"
#include	"savegame.h"
#include	"miles.h"
#include	"3dinstr.h"
#include	"winmove.h"											//ARM 06Dec96
//TEMPCODE DAW 13/04/00 #include	"textref.h"
#include "rchatter.h"
#include	"replay.h"
#include	"overlay.h"
#include	"grid.h"
#include	"fileman.h"
#include	"shapes.h"
#include	"impact.h"													//RJS 12Sep00

#include	"radio.g"													//RJS 12Sep00
#undef assert
#include <cassert>
ArtInt	Art_Int;
AirStrucPtr	ArtInt::ACArray[ACARRAYSIZE]={0};
AirStrucPtr	ArtInt::MemberList[MEMBERLISTSIZE];
float		ArtInt::Dist2List[MEMBERLISTSIZE];

AirStrucPtr	ArtInt::CollideCheckList[COLLIDECHECKLISTSIZE];
AirStrucPtr	ArtInt::CollideAgainstList[COLLIDEAGAINSTLISTSIZE];

//rdh	enum	{DANGERRANGE  =METRES500,		//must defend self
//		SPOTRANGE  		=METRES5000,		//is it a bird, is it a plane?
//		RECOGNISERANGE  =METRES3000,		//no, its superman
//		ATTACKRANGE  	=MILES01,		//I guess he's comming for me, so I'll go for him
//		ENGAGERANGE  	=METRES1000};		//I'm going for him.

//////////////////////////////////////////////////////////////////////
//
// Function:    GetGroundSpotter
// Date:		17/10/00
// Author:		JIM
//
//Description:
//
//////////////////////////////////////////////////////////////////////

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GroundVisible
//Author		Jon Booth
//Date			Mon 17th Jan 2000
//
//Description	is the target plane visible from the ground/radar?
//
//Inputs	target plane
//
//Returns	uid of what it is visible from - returns illegal band if spotter corps, 0 if nothing
//			and CHRadarBand if above a low alt radar
//
//------------------------------------------------------------------------------

int		ArtInt::GroundVisible(Coords3D& trg, int* failedspotter)
{
	*failedspotter=UID_NULL;
	if ( !&trg || trg.X < 0 || trg.X > 8*8*8*256*512 || trg.Z < 0 || trg.Z > 8*8*8*256*512)
		return 0; // the target is null!
	// get the grid data
	Grid_Base* radarGrid = (Grid_Base*) Grid_Base::gridfiles[Grid_Base::GF_RADAR].getdata();

	int v = radarGrid->getWorld( trg.X, trg.Z );
	switch (v)
	{
	case 0: // we are over uk - spotter corps
	{
		UByte prob =0;	//probability of NOT being seen in range from 0-255

#pragma warnmsg ("ground spotting AI: make work with multiple cloud layers, at correct heights" )
//		if (MMC.Sky.Layer[0].AltBase < trg->World.Y ) // target is above clouds -NOTE: IGNORING CLOUD HOLES NOW
		if ( trg.Y > FT_5000 )
		{//above clouds
			Grid_Long* colD = (Grid_Long*) Grid_Base::gridfiles[Grid_Base::GF_CLOUDLAYER0].getdata();
			prob = colD->getWorld( trg.X, trg.Z );
		}
		*failedspotter=IllegalBAND;
		//So, what are the chances?
		if (Math_Lib.rnd(255) > prob)
			return IllegalBAND;
		else
			return 0;
		break;
	}
	case 1: // over an uncovered area - invisible
		return 0;
		break;
	default:
		// inside a radar scan area.
		if ( v<32 ) // low level radar
		{
			int uid = v + CLRadarBAND - 1;
			*failedspotter=uid;
			if ( trg.Y < FT_5000 )
			{
				// need to check if site is dead
				ItemBase* myitem = Persons2::ConvertPtrUID((UniqueID) uid);
				if (!myitem)
					bobassert(myitem,": See Jon");
				if (myitem->Status.deaded == TRUE )
				{
					return 0;
				}
				else
					return uid;
			}
			else
				return 0;//CHRadarBAND; // assume caught by one of these - dunno which	//MS 15Oct00
		} else // high level radar
		{
			int origv = v;
			// calculate the height band.
			int uid = v>>4; // get v/16
			uid += CHRadarBAND+1;
			v&=15; // get v%16
			v+=4;
			int bandbase = (v*FT_1000);
			if ( trg.Y > bandbase )
			{
				// need to check if site is dead
				ItemBase* myitem = Persons2::ConvertPtrUID((UniqueID) uid);
				if (!myitem)
				bobassert(myitem,": See Jon");
				*failedspotter=uid;
				if (myitem->Status.deaded == TRUE )
					return 0;
				else
					return uid;
			}
			else
				return 0;
		}
	}
}

static	UniqueID Associate_Spotters[][3]=
{
	{UID_CL_RameHead,		UID_CH_HawksTor},
	{UID_CL_WestPrawle,		UID_CH_HawksTor,UID_CH_Ventnor},
	{UID_CL_Worth,			UID_CH_HawksTor,UID_CH_Ventnor},
	{UID_CH_Ventnor,		UID_CH_HawksTor,UID_CH_Pevensey},
	{UID_CL_Poling,			UID_CH_Ventnor,UID_CH_Pevensey},
	{UID_CL_Truleigh,		UID_CH_Ventnor,UID_CH_Pevensey},
	{UID_CL_BeachyHead,		UID_CH_Ventnor,UID_CH_Pevensey},
	{UID_CH_Pevensey,		UID_CH_Ventnor,UID_CH_Rye},
	{UID_CL_FairLight,		UID_CH_Pevensey,UID_CH_Rye},
	{UID_CH_Rye,			UID_CH_Pevensey,UID_CH_Dover},
	{UID_CH_Dover,			UID_CH_Rye,UID_CH_Dunkirk},
	{UID_CL_Foreness,		UID_CH_Dover,UID_CH_Dunkirk},
	{UID_CL_Whitstable,		UID_CH_Canewdon,UID_CH_Dunkirk},
	{UID_CH_Canewdon,		UID_CH_Dunkirk,UID_CH_Bromley},
	{UID_CH_Bromley,		UID_CH_Canewdon,UID_CH_Bawdsey},
	{UID_CL_Walton,			UID_CH_Bromley,UID_CH_Bawdsey},
	{UID_CH_Bawdsey,		UID_CH_Bromley,UID_CH_Darsham},
	{UID_CH_Darsham,		UID_CH_Bawdsey,UID_CH_StokeHolyCross},
	{UID_CL_Dunwich,		UID_CH_Darsham,UID_CH_StokeHolyCross},
	{UID_CH_StokeHolyCross,	UID_CH_Darsham,UID_CH_WestBeckham},
	{UID_CL_Happisburgh,	UID_CH_StokeHolyCross,UID_CH_WestBeckham},
	{UID_CH_WestBeckham,	UID_CH_StokeHolyCross,UID_CH_Steingot},
	{UID_CL_Ingoldmeis,		UID_CH_WestBeckham,UID_CH_Steingot},
	{UID_CH_Steingot,		UID_CH_WestBeckham},

	UID_NULL
};
static UniqueID	AlternateCHL[32][2]={UID_NULL};
static UniqueID	AlternateCH[32][2]={UID_NULL};

void	BuildSpotters()
{
	if (Associate_Spotters[0][0])
	{
		for (int i=0;Associate_Spotters[i][0];i++)
			if (Associate_Spotters[i][0]>=CHRadarBAND)
			{
				AlternateCH[Associate_Spotters[i][0]-CHRadarBAND][0]=Associate_Spotters[i][1];
				AlternateCH[Associate_Spotters[i][0]-CHRadarBAND][1]=Associate_Spotters[i][2];
			}
			else
			{
				AlternateCHL[Associate_Spotters[i][0]-CLRadarBAND][0]=Associate_Spotters[i][1];
				AlternateCHL[Associate_Spotters[i][0]-CLRadarBAND][1]=Associate_Spotters[i][2];

			}
		Associate_Spotters[0][0]=UID_NULL;
	}
}
UniqueID	ArtInt::GetSecondarySpotter(Coords3D& trg,UniqueID primary,int slot)
{
	if (primary==IllegalBAND || primary==UID_NULL)
		return primary;
	BuildSpotters();
	if (primary>=CHRadarBAND)
		primary=AlternateCH[primary-CHRadarBAND][slot];
	else
		primary=AlternateCHL[primary-CLRadarBAND][slot];
	if (primary)
	{
		ItemBasePtr spot=Persons2::ConvertPtrUID(primary);
		float rangesq=Math_Lib.DistanceSquared(trg.X-spot->World.X,trg.Z-spot->World.Z);
		float altsq=float(trg.Y)*trg.Y;
		rangesq/=(MILES03*MILES04);			//1000 ft in 3.5 miles
		altsq/=(FT_1000*FT_1000);
		if (rangesq>altsq)
			primary=UID_NULL;
	}
	return primary;
}
UniqueID	ArtInt::GetSecondarySpotter(Coords3D& target,UniqueID primary)
{
	if (primary==IllegalBAND || primary==UID_NULL)
		return primary;
	for (int slot=0;slot<2;slot++)
	{
		UniqueID probreturn=GetSecondarySpotter(target,primary,slot);
		if (probreturn)
		{
			ItemBasePtr p=Persons2::ConvertPtrUID(probreturn);
			if (p->Status.deadtime==0)
				return probreturn;
		}
	}
	return UID_NULL;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SpottedFromGround
//Author		Jon Booth
//Date			Mon 17th Jan 2000
//
//Description	What to do if a plane is spotted from the ground
//
//Inputs	target plane
//
//Returns
//
//------------------------------------------------------------------------------


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CheckSequenceAudible
//Author		Jim Taylor
//Date			Thu 14 Nov 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool	FormationItem::PlayerSequenceAudible(FileNum f)
{
	if (	formpos==0	||	Manual_Pilot.ControlledAC2==this)
	{
		_Miles.SequenceAudible(f);
		return(true);
	}
	return(false);
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CleanUp
//Author		Jim Taylor
//Date			Tue 18 Jun 1996
//
//Description	reset AI system for next mission.
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	ArtInt::CleanUp()
{
	{
	for (int i=0;i<ACARRAYSIZE;i++)
		ACArray[i]=NULL;
	}
	gunstimer=0;
	exittimer=0;

	ACArrayInd = 0;
	MemberListInd = 0;
	CollideListInd = 0;
	for(int i = 0; i < MEMBERLISTSIZE; i++)
		MemberList[i] = 0;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Add
//Author		Jim Taylor
//Date			Tue 28 May 1996
//
//Description	Adds ac to ai ac array
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool	ArtInt::Add(AirStrucPtr ac)
{
	if (Remove(ac))
		_Error.EmitSysErr("Added AC already known");
	int	ind=Math_Lib.rnd(ACARRAYSIZE);
	while (ACArray[ind])
		if (--ind<0)
			break;
	if (ind<0)
	{
		ind=ACARRAYSIZE-1;
		while (ACArray[ind])
			if (--ind<0)
				_Error.EmitSysErr("Aircraft array is full!");
	}
	ACArray[ind]=ac;
	return(true);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Remove
//Author		Jim Taylor
//Date			Tue 28 May 1996
//
//Description	Removes ac from ai ac array
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool	ArtInt::Remove(AirStrucPtr ac)
{
 	int ind=ACARRAYSIZE-1;
	do
		if (ACArray[ind]==ac)
		{
			ACArray[ind]=NULL;
			return(true);
		}
	while	(ind--);
	return(false);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Visible
//Author		Jim Taylor
//Date			Tue 4 Jun 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool	ArtInt::Visible(AirStrucPtr eye,AirStrucPtr trg)
{
	Coords3D* eyepos=_DPlay.GetGoodACPosition(eye);
	Coords3D* trgpos=_DPlay.GetGoodACPosition(trg);
	//loads of quick throwouts for target
	if (trg->Status.deadtime)
		return(false);											//JIM 17Oct96
	if (trg->leader && trg->information)
		return(false);
	if (trg->nationality==eye->nationality)
		if((eye->movecode == AUTO_COMBAT) && (eye->ai.unfriendly))
			return(false);
		else;
	else
		if((trg->leader) && ((trg->movecode == AUTO_COMBAT) || (trg->movecode == AUTO_COMBAT)) && (trg->ai.unfriendly))
			return(false);
	if (trg->ai.attacker && trg->ai.attacker->ai.unfriendly==trg && trg->ai.attacker->movecode>=AUTO_PRECOMBAT)
		return(false);
	int	delta=trgpos->X-eyepos->X;
	int delta2=trgpos->Z-eyepos->Z;
	int	visiblerange=VISIBLERANGE;
	if (eye->nationality==NAT_RED && trg->nationality==NAT_BLUE)
		if (trg->ai.radiosilent)
			visiblerange=METRES30KM;
		else
			visiblerange=METRES100KM;

	if (	delta<visiblerange && delta>-visiblerange
		&&	delta2<visiblerange && delta2>-visiblerange
		)
	{
		Item::InterceptandRange(eyepos,trgpos);
		if (eye->Range<VISIBLERANGE)
			return(true);
	}
	//finally do proper hdg-intercept
	if (eye->ai.unfriendly==trg)										//JIM 22Jun00
		eye->ai.unfriendly=NULL;
	return(false);
}



bool	ArtInt::Seen(AirStrucPtr eye,AirStrucPtr trg)
{
	if(eye->Distance3DSquared(&trg->World) > FP(VISIBLERANGE) * FP(VISIBLERANGE))
		return(false);
	else
	{
		if(eye->ai.unfriendly != trg)
		{
			//build up probabilities
			enum	CentWord	{	PCW_1=655,PCW_2=1300,PCW_5=3277,
									PCW_10=6553,PCW_20=13107,
									PCW_25=16384,PCW_50=32768,PCW_75=49100,
									PCW_80=52430,PCW_90=59000,
									PCW_95=62250,PCW_99=64900,PCW_100=65535
									};
			ULong	prob=PCW_100;	//NOTE: ALL MATH IS UNSIGNED!!!!
			ULong	tprob;

// THE SUN - CAN'T SEE AIRCRAFT ATTACKING FROM THE SUN				  //CSB 12/01/00
			if(trg->World.Y > eye->World.Y)
			{
				tprob = PCW_100;
				WorldStuff*	worldptr = mobileitem::currworld;
				FP dotprod =  (trg->World.X - eye->World.X) * (worldptr->sunpos.X - eye->World.X)
							+ (trg->World.Y - eye->World.Y) * (worldptr->sunpos.Y - eye->World.Y)
							+ (trg->World.Z - eye->World.Z) * (worldptr->sunpos.Z - eye->World.Z);
				if(dotprod > 0)
				{
					dotprod = (dotprod * dotprod)
								/ (eye->Distance3DSquared(&worldptr->sunpos) * eye->Distance3DSquared(&trg->World));
					if(dotprod > 0.9)
						tprob *= (1 - dotprod) / 0.1;
				}

				prob = (prob * tprob) >> 16;
			}

			eye->InterceptandRange(trg);
// RANGE - AIRCRAFT FURTHER AWAY ARE HARDER TO SEE
			tprob = FP(PCW_100);
			FP range100 = VISIBLERANGE * 0.2;
			if(eye->Range > range100)
				tprob *= range100 * range100 / (FP(eye->Range) * FP(eye->Range));

			prob = (prob * tprob) >> 16;

// POSITION - ASSUME BOMBERS HAVE PERFECT 360x360 FIELD OF VIEW - FIGHTERS CAN'T SEE REARWARDS OR BELOW - 2-SEATERS CAN'T SEE BELOW
			tprob = PCW_10;
			SWord abshdg = AbsAngle(SWord(item::HdgIntercept - eye->hdg));
			SWord dpitch = SWord(item::PitchIntercept - eye->pitch);
			switch(eye->classtype->visible)
			{
				case SPIT:				// Rel Heading		  0 - 45	 45 - 90	 90 - 135	135 - 180	Everwhere
				case HURR:				// Rel Pitch		-10 - 90	-10 - 90	-45 - 90	-10 - 90	  Else
				case ME109:				// Probability			100%		75%			50%			25%			10%
				{
					if(abshdg <= ANGLES_45Deg)
					{
						if(dpitch > SWord(ANGLES_330Deg))
							tprob = PCW_100;
					}
					else if(abshdg <= ANGLES_90Deg)
					{
						if(dpitch > SWord(ANGLES_330Deg))
							tprob = PCW_75;
					}
					else if(abshdg <= ANGLES_135Deg)
					{
						if(dpitch > SWord(ANGLES_315Deg))
							tprob = PCW_50;
					}
					else
					{
						if(dpitch > SWord(ANGLES_350Deg))
							tprob = PCW_25;
					}
					break;
				}

				case BRISTO:			// Rel Heading		  0 - 45	 45 - 90	 90 - 135	135 - 180	Everwhere
				case DEF:				// Rel Pitch		-10 - 90	-10 - 90	-45 - 90	-10 - 90	  Else
				case ME110:				// Probability			100%		75%			75%			100%		10%
				case JU87:
				{
					if(abshdg <= ANGLES_45Deg)
					{
						if(dpitch > SWord(ANGLES_330Deg))
							if((eye != Persons2::PlayerGhostAC) && (eye != Persons2::PlayerSeenAC))
								tprob = PCW_100;
							else
								tprob = PCW_10;
					}
					else if(abshdg <= ANGLES_90Deg)
					{
						if(dpitch > SWord(ANGLES_350Deg))
							if((eye != Persons2::PlayerGhostAC) && (eye != Persons2::PlayerSeenAC))
								tprob = PCW_75;
							else
								tprob = PCW_25;
					}
					else if(abshdg <= ANGLES_135Deg)
					{
						if(dpitch > SWord(ANGLES_315Deg))
							tprob = PCW_75;
					}
					else
					{
						if(dpitch > SWord(ANGLES_350Deg))
							tprob = PCW_100;
					}
					break;
				}

				case JU88:
				case HE11:
				case DO17:
				{
					tprob = PCW_100;
					break;
				}
				break;
			}
			prob = (prob * tprob) >> 16;

// CLOUDS - IF EITHER AC IS IN CLOUD, CHANCES OF SPOT ARE REDUCED					//CSB 12/01/00
//			This is crude and doesn't account for SPECIFIC holes in cloud layers	//CSB 12/01/00
			tprob = PCW_100;
			for(SWord i = 0; i < 3; i++)
			{
				if((eye->World.Y > MissManCampSky().Layer[i].AltBase) && (eye->World.Y < MissManCampSky().Layer[i].AltTop))
					tprob = (tprob * (255 - MissManCampSky().Layer[i].Cover)) >> 8;

				if((trg->World.Y > MissManCampSky().Layer[i].AltBase) && (trg->World.Y < MissManCampSky().Layer[i].AltTop))
					tprob = (tprob * (255 - MissManCampSky().Layer[i].Cover)) >> 8;

				if((eye->World.Y < MissManCampSky().Layer[i].AltBase) && (trg->World.Y > MissManCampSky().Layer[i].AltTop))
					tprob = (tprob * (255 - MissManCampSky().Layer[i].Cover)) >> 8;

				if((trg->World.Y < MissManCampSky().Layer[i].AltBase) && (eye->World.Y > MissManCampSky().Layer[i].AltTop))
					tprob = (tprob * (255 - MissManCampSky().Layer[i].Cover)) >> 8;
			}
			prob = (prob * tprob) >> 16;


			if(		(Persons2::PlayerGhostAC == eye)
				&&	(item::Range < COMBATRANGE)
				&&	(!Save_Data.gamedifficulty[GD_AUTOVECTORING])	)
  				prob = PCW_100;


			int numeyes = eye->fly.numinsag;		if(numeyes == 0)	numeyes = 1;
			int numtrgs = trg->fly.numinsag;		if(numtrgs == 0)	numtrgs = 1;
			for(int ne = 1; ne <= numeyes; ne++)
				for(int nt = 1; nt <= numtrgs; nt++)
					if(Math_Lib.rnd() < prob)
						return(true);

			return(false);
		}															  //RDH 23/06/99
		else
			return(true);
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		VisibleCheck
//Author		Jim Taylor
//Date			Mon 3 Jun 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	ArtInt::VisibleCheck()
{
	VisibleAcCheck();
	VisibleAAACheck();

	if(!(ACArrayInd & 0x7f) && (MemberListInd == 0))			//RJS 13Sep00
	{
		mobileitem*	tmpac = mobileitem::ACList;
		while (tmpac)
		{
			MinAnimData*	mad = (MinAnimData*)tmpac->Anim;
			if (	(mad->itemstate < DYING)
				&&	(mad->IsInvisible == 0)
				&&	(tmpac->movecode != AUTO_SPAREAC)	)
				Fake_Damage.TestForBalloon(tmpac);

			tmpac = tmpac->nextmobile;
		}
	}

	if(gunstimer > 0)
	{
		gunstimer -= Timer_Code.FRAMETIME;
		if(gunstimer <= 0)
		{
			AirStrucPtr player = Persons2::PlayerGhostAC;
			SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, player, player, player), RealTestGuns);
			gunstimer = 0;
		}
	}

	if(exittimer > 0)
	{
		exittimer -= Timer_Code.FRAMETIME;
		if(exittimer <= 0)
		{
			AirStrucPtr player = Persons2::PlayerGhostAC;
			SendMsgAndCall(MESSAGE_STRUC(PHRASE_THINKING, MSG_CLEAR, player, player, player), SetupQuitContinue);
			exittimer = 0;
		}
	}

	if(		(mobileitem::timeofday >= GR_OkToQuit3DNow)	)
	{
		AirStrucPtr player = Persons2::PlayerGhostAC;
		GR_OkToQuit3DNow = GR_OkToQuit3DNow+MINS100*3;
		OverLay.CancelAccel();
		SendMsgAndCall(MESSAGE_STRUC(PHRASE_THINKING, MSG_CLEAR, player, player, player), SetupQuitContinue);
	}


	AirStrucPtr player = Persons2::PlayerGhostAC;
	if(		(player->nationality == NAT_RAF) && (!(mobileitem::timeofday & 0x01ff))
		&&	((!_DPlay.Implemented) || (_DPlay.Implemented && _DPlay.GameType==DPlay::COMMSQUICKMISSION))
		&&	((Persons2::PlayerGhostAC->movecode != AUTO_TRAININGTAKEOFF) && (Persons2::PlayerGhostAC->movecode != AUTO_TRAININGLANDING))	)
	{
		if(player->World.X - player->World.Z > METRES100KM)
		{
			Grid_Base* cornerAlt = (Grid_Base*)Grid_Base::gridfiles[Grid_Base::GF_CORNERALT].getdata();
			int Y = cornerAlt->getWorld(player->World.X, player->World.Z );
			if((Y > 0) && (player->World.Y > Y + FT_500))
			{
				OverLay.CancelAccel();
				WorldStuff*	worldptr = mobileitem::currworld;
				Trans_Obj.LaunchFlak(player,BFLAK,*worldptr);			//RJS 20Oct00
				if(Save_Data.gamedifficulty[GD_VULNERABLE])
				{
					AirStrucPtr seenac = Persons2::PlayerSeenAC;
					AircraftAnimData* adptr = (AircraftAnimData*)seenac->Anim;
					switch(Math_Lib.rnd(16))
					{
						case 0:		SHAPE.ForceDamage(seenac, seenac, &adptr->LEFTWINGIN,  BS_DEAD);	break;
						case 1:		SHAPE.ForceDamage(seenac, seenac, &adptr->RIGHTWINGIN, BS_DEAD);	break;
						default:	break;
					}
				}
			}
		}
	}

#ifndef NDEBUG

//#define PRINT_SQUADRON
#ifdef  PRINT_SQUADRON
	int line = 0;
	for(AirStrucPtr ld = Persons2::PlayerGhostAC->FindFormpos0(); ld; ld = ld->fly.nextflight)
		for(AirStrucPtr ac = ld; ac; ac = ac->Follower())
		{
			PrintVar(0, line, "Pos %.0f ", FP(ac->formpos));
			PrintVar(7, line, "Move %.0f ", FP(ac->movecode));
			PrintVar(15, line, "Step %.0f ", FP(ac->ai.ManStep));
			line++;
		}
#endif


//#define PRINT_SQUADRONS
#ifdef  PRINT_SQUADRONS
	if(!(ACArrayInd & 0x0f) && (MemberListInd == 0))
	{
		for(SWord y = 0; y < 25; y++)
			PrintString(0, y, "                                   ");
		y = 0;
		for(int i = 0; i < ACARRAYSIZE; i++)
			if(		((ACArray[i]) && (ACArray[i]->movecode != AUTOSAG_WAITTAKEOFF))
				&&	(	((!ACArray[i]->fly.leadflight) || (ACArray[i]->waypoint != ACArray[i]->fly.leadflight->waypoint))
					||	((ACArray[i]->movecode != AUTO_FOLLOWWP) && (ACArray[i]->movecode != AUTOSAG_TRACKEXPFOLLOW))
					||	(ACArray[i]->ai.unfriendly) || (ACArray[i]->ai.attacker)	)	)
			{
				AirStrucPtr group = ACArray[i];
				switch(group->classtype->visible)
				{
					case SPIT:		PrintString(0, y, "SPIT ");	break;
					case HURR:		PrintString(0, y, "HURR ");	break;
					case ME109:		PrintString(0, y, "M109 ");	break;
					case ME110:		PrintString(0, y, "M110 ");	break;
					case JU87:		PrintString(0, y, "JU87 ");	break;
					case DEF:		PrintString(0, y, "DEFI ");	break;
					case BRISTO:	PrintString(0, y, "BLEN ");	break;
					case JU88:		PrintString(0, y, "JU88 ");	break;
					case HE11:		PrintString(0, y, "H111 ");	break;
					case DO17:		PrintString(0, y, "DO17 ");	break;
					case JU52:		PrintString(0, y, "HE59 ");	break;
				}

				AirStrucPtr leadac = group->GetLeadAc();
				if(leadac->fly.numinsag)
					PrintString(5, y, "sag");
				else
					PrintString(5, y, "EXP");

				if(group->ai.attacker)
					PrintString(8, y, "@");
				else
					PrintString(8, y, " ");

				PrintVar(9, y, "%.0f", FP(CountSquadronSize(leadac)));

				if(group->ai.unfriendly)
				{
					if(leadac->classtype->aerobaticfactor == AEROBATIC_LOW)
						PrintString(11, y, "-");
					else
						PrintString(11, y, "*");
				}
				else
					PrintString(11, y, " ");

				switch(leadac->movecode)
				{
					case AUTO_FOLLOWWP:				PrintString(12, y, "AUTO_FOLLOWWP ");			break;
					case AUTO_NOPPILOT:				PrintString(12, y, "AUTO_NOPPILOT ");			break;
					case AUTO_WAIT4TIME:			PrintString(12, y, "AUTO_WAIT4TIME ");			break;
					case AUTO_TAXI:					PrintString(12, y, "AUTO_TAXI ");				break;
					case AUTO_TAKEOFF:				PrintString(12, y, "AUTO_TAKEOFF ");			break;
					case GROUND_TAXI:				PrintString(12, y, "GROUND_TAXI ");				break;
					case AUTO_TELLLEADER:			PrintString(12, y, "AUTO_TELLLEADER ");			break;
					case AUTO_FOLLOWTHELEADER:		PrintString(12, y, "AUTO_FOLLOWTHELEADER ");	break;
					case AUTO_SPIRAL2GROUND:		PrintString(12, y, "AUTO_SPIRAL2GROUND ");		break;
					case AUTO_RESURRECT:			PrintString(12, y, "AUTO_RESURRECT ");			break;
					case AUTO_DEATHSEQUENCE:		PrintString(12, y, "AUTO_DEATHSEQUENCE ");		break;
					case AUTO_TRAININGTAKEOFF:		PrintString(12, y, "AUTO_TRAININGTAKEOFF ");	break;
					case AUTO_TRAININGLANDING:		PrintString(12, y, "AUTO_TRAININGLANDING ");	break;
					case AUTO_TRAININGFORMATION:	PrintString(12, y, "AUTO_TRAININGFORMATION ");	break;
					case AUTO_CRASHSKID:			PrintString(12, y, "AUTO_CRASHSKID ");			break;
					case AUTO_CRASHROLL:			PrintString(12, y, "AUTO_CRASHROLL ");			break;
					case AUTO_CRASHONEWHEEL:		PrintString(12, y, "AUTO_CRASHONEWHEEL ");		break;
					case AUTO_CRASHNOSE:			PrintString(12, y, "AUTO_CRASHNOSE ");			break;
					case AUTO_CRASHFLIP:			PrintString(12, y, "AUTO_CRASHFLIP ");			break;
					case AUTO_CRASHTUMBLE:			PrintString(12, y, "AUTO_CRASHTUMBLE ");		break;
					case AUTO_BOMB:					PrintString(12, y, "AUTO_BOMB ");				break;
					case AUTO_COVER:				PrintString(12, y, "AUTO_COVER ");				break;
					case AUTO_STRAFFE:				PrintString(12, y, "AUTO_STRAFFE ");			break;
					case AUTO_BALLOON:				PrintString(12, y, "AUTO_BALLOON ");			break;
					case AUTO_RECONN:				PrintString(12, y, "AUTO_RECONN ");				break;
					case AUTO_ARTSPOT:				PrintString(12, y, "AUTO_ARTSPOT ");			break;
					case AUTO_HITWATER:				PrintString(12, y, "AUTO_HITWATER ");			break;
					case AUTO_TRACKINGBOGIE:		PrintString(12, y, "AUTO_TRACKINGBOGIE ");		break;
					case AUTO_TRACKINGBANDIT:		PrintString(12, y, "AUTO_TRACKINGBANDIT ");		break;
					case AUTO_PRECOMBAT:			PrintString(12, y, "AUTO_PRECOMBAT ");			break;
					case AUTO_LANDING:				PrintString(12, y, "AUTO_LANDING ");			break;
					case AUTO_FAC:					PrintString(12, y, "AUTO_FAC ");				break;
					case AUTO_CAS:					PrintString(12, y, "AUTO_CAS ");				break;
					case AUTO_ACCELERATE:			PrintString(12, y, "AUTO_ACCELERATE ");			break;
					case AUTO_MISSIONDICETHROW:		PrintString(12, y, "AUTO_MISSIONDICETHROW ");	break;
					case AUTO_COMMSACCEL:			PrintString(12, y, "AUTO_COMMSACCEL ");			break;
					case GROUND_TOGLOBREF:			PrintString(12, y, "GROUND_TOGLOBREF ");		break;
					case GROUND_BLOKE:				PrintString(12, y, "GROUND_BLOKE ");			break;
					case GROUND_EMERGENCY:			PrintString(12, y, "GROUND_EMERGENCY ");		break;
					case GROUND_HALTTRUCK:			PrintString(12, y, "GROUND_HALTTRUCK ");		break;
					case AUTO_LAST:					PrintString(12, y, "AUTO_LAST ");				break;
					case AUTOSAG_FIRST:				PrintString(12, y, "AUTOSAG_FIRST ");			break;
					case AUTOSAG_FOLLOWWPHOME:		PrintString(12, y, "AUTOSAG_FOLLOWWPHOME ");	break;
					case AUTOSAG_REFUELLING:		PrintString(12, y, "AUTOSAG_REFUELLING ");		break;
					case AUTOSAG_COMBAT:			PrintString(12, y, "AUTOSAG_COMBAT ");			break;
					case AUTOSAG_COMBATCIRCLES:		PrintString(12, y, "AUTOSAG_COMBATCIRCLES ");	break;
					case AUTOSAG_POSTCOMBAT:		PrintString(12, y, "AUTOSAG_POSTCOMBAT ");		break;
					case AUTOSAG_BOMBAPPROACH:		PrintString(12, y, "AUTOSAG_BOMBAPPROACH ");	break;
					case AUTOSAG_TRACKEXPFOLLOW:	PrintString(12, y, "AUTOSAG_TRACKEXPFOLLOW ");	break;
					case AUTOSAG_TRACKEXPCOMBAT:	PrintString(12, y, "AUTOSAG_TRACKEXPCOMBAT ");	break;
					case AUTOSAG_DESTROYED:			PrintString(12, y, "AUTOSAG_DESTROYED ");		break;
					case AUTOSAG_DEATHGLIDE:		PrintString(12, y, "AUTOSAG_DEATHGLIDE ");		break;
					case AUTOSAG_LAST:				PrintString(12, y, "AUTOSAG_LAST ");			break;
				}
				PrintVar(26, y, " %.0f ", FP(leadac->waypoint->wpname));

				int fuel = 0;
				for(int i = 0; i < 4; i++)
					fuel += leadac->fly.fuel_content[i];

				PrintVar(28, y, " %.0f ", FP(FP(fuel) / (10.0 * FP(leadac->classtype->maxintfuel))));





				y++;
			}


	}

 	COORDS3D AcPos;
	AcPos.X = Persons2::PlayerSeenAC->World.X;
	if(Persons2::PlayerSeenAC->World.Y > 5000)
		AcPos.Y = Persons2::PlayerSeenAC->World.Y - 5000;
	else AcPos.Y = 0;
	AcPos.Z = Persons2::PlayerSeenAC->World.Z;
	int TempGroundAlt = _Collide.GroundAltitude(AcPos);

#endif

//#define PRINT_COMBAT
#ifdef  PRINT_COMBAT

	for(int i = 0; i < ACARRAYSIZE; i++)
		if((ACArray[i]) && ((ACArray[i]->movecode == AUTO_COMBAT) || (ACArray[i]->movecode == AUTO_PRECOMBAT)) )
		{
			AirStrucPtr leadac = ACArray[i]->FindFormpos0();

			WeapAnimData*	weapon;
			SLong			xpos, ypos, zpos;
			UWord			index;
			ULong			mvel;
			UWord			mdelay,mburst;

			SWord y = 10;
			SWord dx = 0;
			for(AirStrucPtr lead = leadac; lead; lead = lead->fly.nextflight)
				for(AirStrucPtr foll = lead; foll; foll = foll->Follower())
				{
					if(foll->nationality == NAT_LUF)
						dx = 40;

					if(foll == Persons2::PlayerGhostAC)
						foll = Persons2::PlayerSeenAC;

					if(foll->movecode == AUTO_COMBAT)
						PrintString(0 + dx, y, "COM ");
					else if(foll->movecode == AUTO_PRECOMBAT)
						PrintString(0 + dx, y, "PRE ");
					else
						PrintString(0 + dx, y, "??? ");

					switch(foll->ai.manoeuvre)
					{
						case MANOEUVRE_SELECT:				PrintString(4 + dx, y, "SELECT ");				break;
						case MANOEUVRE_LOOKROUND:			PrintString(4 + dx, y, "LOOKROUND ");			break;
						case MANOEUVRE_WELDEDWINGMAN:		PrintString(4 + dx, y, "WELDEDWINGMAN ");		break;
						case MANOEUVRE_BAILOUT:				PrintString(4 + dx, y, "BAILOUT ");				break;
						case MANOEUVRE_TOPCOVER:			PrintString(4 + dx, y, "TOPCOVER ");			break;
						case MANOEUVRE_FORCETOPCOVER:		PrintString(4 + dx, y, "FORCETOPCOVER ");		break;
						case MANOEUVRE_LINEABREAST:			PrintString(4 + dx, y, "LINEABREAST ");			break;
						case MANOEUVRE_PINCER:				PrintString(4 + dx, y, "PINCER ");				break;
						case MANOEUVRE_MULTIWAVE:			PrintString(4 + dx, y, "MULTIWAVE ");			break;
						case MANOEUVRE_DIVEANDZOOM:			PrintString(4 + dx, y, "DIVEANDZOOM ");			break;
						case MANOEUVRE_LEADPURSUIT:			PrintString(4 + dx, y, "LEADPURSUIT ");			break;
						case MANOEUVRE_LAGPURSUIT:			PrintString(4 + dx, y, "LAGPURSUIT ");			break;
						case MANOEUVRE_SPLITMANOEUVRE:		PrintString(4 + dx, y, "SPLITMANOEUVRE ");		break;
						case MANOEUVRE_HEADON:				PrintString(4 + dx, y, "HEADON ");				break;
						case MANOEUVRE_LINEASTERN:			PrintString(4 + dx, y, "LINEASTERN ");			break;
						case MANOEUVRE_VICATTACK:			PrintString(4 + dx, y, "VICATTACK ");			break;
						case MANOEUVRE_BARRELROLLATTACK:	PrintString(4 + dx, y, "BARRELROLLATTACK ");	break;
						case MANOEUVRE_SCISSORS:			PrintString(4 + dx, y, "SCISSORS ");			break;
						case MANOEUVRE_MILDSCISSORS:		PrintString(4 + dx, y, "MILDSCISSORS ");		break;
						case MANOEUVRE_TURNINGFIGHT:		PrintString(4 + dx, y, "TURNINGFIGHT ");		break;
						case MANOEUVRE_SPLITS:				PrintString(4 + dx, y, "SPLITS ");				break;
						case MANOEUVRE_ZOOMANDDROP:			PrintString(4 + dx, y, "ZOOMANDDROP ");			break;
						case MANOEUVRE_STRAIGHTANDLEVEL:	PrintString(4 + dx, y, "STRAIGHTANDLEVEL ");	break;
						case MANOEUVRE_SPINOUT:				PrintString(4 + dx, y, "SPINOUT ");				break;
						case MANOEUVRE_DIVEFORHOME:			PrintString(4 + dx, y, "DIVEFORHOME ");			break;
						case MANOEUVRE_GOHOME:				PrintString(4 + dx, y, "GOHOME ");				break;
						case MANOEUVRE_MAKEFORFRIENDLY:		PrintString(4 + dx, y, "MAKEFORFRIENDLY ");		break;
						case MANOEUVRE_MOVEAWAY:			PrintString(4 + dx, y, "MOVEAWAY ");			break;
						case MANOEUVRE_ACCIDENTALSPINOUT:	PrintString(4 + dx, y, "ACCIDENTALSPINOUT ");	break;
						case MANOEUVRE_VERTICALLOOP:		PrintString(4 + dx, y, "VERTICALLOOP ");		break;
						case MANOEUVRE_IMMELMANNTURN:		PrintString(4 + dx, y, "IMMELMANNTURN ");		break;
						case MANOEUVRE_IMMELMANN:			PrintString(4 + dx, y, "IMMELMANN ");			break;
						case MANOEUVRE_STAYWITHPREY:		PrintString(4 + dx, y, "STAYWITHPREY ");		break;
						case MANOEUVRE_CLIMBFORHOME:		PrintString(4 + dx, y, "CLIMBFORHOME ");		break;
						case MANOEUVRE_STRAIGHTDIVE:		PrintString(4 + dx, y, "STRAIGHTDIVE ");		break;
						case MANOEUVRE_STANDONTAIL:			PrintString(4 + dx, y, "STANDONTAIL ");			break;
						case MANOEUVRE_SHOOTTOFRIGHTEN:		PrintString(4 + dx, y, "SHOOTTOFRIGHTEN ");		break;
						case MANOEUVRE_SHOOTTOOEARLY:		PrintString(4 + dx, y, "SHOOTTOOEARLY ");		break;
						case MANOEUVRE_GAINHEIGHT:			PrintString(4 + dx, y, "GAINHEIGHT ");			break;
						case MANOEUVRE_LAGROLL:				PrintString(4 + dx, y, "LAGROLL ");				break;
						case MANOEUVRE_EXTENSION:			PrintString(4 + dx, y, "EXTENSION ");			break;
						case MANOEUVRE_DIVINGSPIN:			PrintString(4 + dx, y, "DIVINGSPIN ");			break;
						case MANOEUVRE_REVERSETURN:			PrintString(4 + dx, y, "REVERSETURN ");			break;
						case MANOEUVRE_SELFASBAIT:			PrintString(4 + dx, y, "SELFASBAIT ");			break;
						case MANOEUVRE_JINK:				PrintString(4 + dx, y, "JINK ");				break;
						case MANOEUVRE_BREAKTURN:			PrintString(4 + dx, y, "BREAKTURN ");			break;
						case MANOEUVRE_LAZYTURN:			PrintString(4 + dx, y, "LAZYTURN ");			break;
						case MANOEUVRE_BREAKLOW:			PrintString(4 + dx, y, "BREAKLOW ");			break;
						case MANOEUVRE_BREAKHIGH:			PrintString(4 + dx, y, "BREAKHIGH ");			break;
						case MANOEUVRE_BREAK90:				PrintString(4 + dx, y, "BREAK90 ");				break;
						case MANOEUVRE_HIGBARRELROLL:		PrintString(4 + dx, y, "HIGBARRELROLL ");		break;
						case MANOEUVRE_PANICTURN:			PrintString(4 + dx, y, "PANICTURN ");			break;
						case MANOEUVRE_UNBALANCEDFLIGHT:	PrintString(4 + dx, y, "UNBALANCEDFLIGHT ");	break;
						case MANOEUVRE_LOWALT:				PrintString(4 + dx, y, "LOWALT ");				break;
						case MANOEUVRE_TURNTOHDGANDPITCH:	PrintString(4 + dx, y, "TURNTOHDGANDPITCH ");	break;
						case MANOEUVRE_ZOOM:				PrintString(4 + dx, y, "ZOOM ");				break;
						case MANOEUVRE_INTERCEPTHIGH:		PrintString(4 + dx, y, "INTERCEPTHIGH ");		break;
						case MANOEUVRE_INTERCEPTLOW:		PrintString(4 + dx, y, "INTERCEPTLOW ");		break;
						case MANOEUVRE_GAINSPEED:			PrintString(4 + dx, y, "GAINSPEED ");			break;
						case MANOEUVRE_HEADONATTACK:		PrintString(4 + dx, y, "HEADONATTACK ");		break;
						case MANOEUVRE_LUFBERRY:			PrintString(4 + dx, y, "LUFBERRY ");			break;
						case MANOEUVRE_STEEPDIVE:			PrintString(4 + dx, y, "STEEPDIVE ");			break;
						case MANOEUVRE_UPANDUNDER:			PrintString(4 + dx, y, "UPANDUNDER ");			break;
						case MANOEUVRE_STALLTURN:			PrintString(4 + dx, y, "STALLTURN ");			break;
						case MANOEUVRE_SLASHINGATTACK:		PrintString(4 + dx, y, "SLASHINGATTACK ");		break;
						case MANOEUVRE_CLIMBTURN:			PrintString(4 + dx, y, "CLIMBTURN ");			break;
						case MANOEUVRE_ROLL360:				PrintString(4 + dx, y, "ROLL360 ");				break;
						case MANOEUVRE_STRAFFE:				PrintString(4 + dx, y, "STRAFFE ");				break;
						case MANOEUVRE_FLYTHROUGHCLOUD:		PrintString(4 + dx, y, "FLYTHROUGHCLOUD ");		break;
						case MANOEUVRE_REGROUP:				PrintString(4 + dx, y, "REGROUP ");				break;
						case MANOEUVRE_SPINRECOVERY:		PrintString(4 + dx, y, "SPINRECOVERY ");		break;
						case MANOEUVRE_COLLISIONAVOIDANCE:	PrintString(4 + dx, y, "COLLISIONAVOIDANCE ");	break;
						case MANOEUVRE_COLLIDE:				PrintString(4 + dx, y, "COLLIDE ");				break;
						case MANOEUVRE_DISENGAGED:			PrintString(4 + dx, y, "DISENGAGED ");			break;
					}

					if((foll->ai.unfriendly) && (foll->ai.unfriendly->Status.size == AIRSTRUCSIZE))
					{
						switch(AirStrucPtr(foll->ai.unfriendly)->classtype->visible)
						{
							case SPIT:		PrintString(20 + dx, y, " SPIT ");	break;
							case HURR:		PrintString(20 + dx, y, " HURR ");	break;
							case ME109:		PrintString(20 + dx, y, " M109 ");	break;
							case ME110:		PrintString(20 + dx, y, " M110 ");	break;
							case JU87:		PrintString(20 + dx, y, " JU87 ");	break;
							case DEF:		PrintString(20 + dx, y, " DEFI ");	break;
							case BRISTO:	PrintString(20 + dx, y, " BLEN ");	break;
							case JU88:		PrintString(20 + dx, y, " JU88 ");	break;
							case HE11:		PrintString(20 + dx, y, " H111 ");	break;
							case DO17:		PrintString(20 + dx, y, " DO17 ");	break;
							case JU52:		PrintString(20 + dx, y, " HE59 ");	break;
						}
						PrintVar(25 + dx, y, " %.0f ", FP(AirStrucPtr(foll->ai.unfriendly)->formpos));
					}
					else
						PrintString(20 + dx, y, " ---- -- ");

					if((Trans_Obj.View_Object) && (ItemPtr(foll) == Trans_Obj.View_Object->trackeditem2))
						PrintString(28 + dx, y, "* ");
					else
						PrintString(28 + dx, y, "  ");

					int totalammo = 0;

					for(index = 0; index < 6; index++)
					{
						weapon = SHAPE.GetWeaponLauncher(foll,index,xpos,ypos,zpos,mvel,mdelay,mburst,LT_BULLET);
						if((weapon)	&& (weapon->LoadedStores > 0))
							totalammo += weapon->LoadedStores;
					}
					PrintVar(30 + dx, y, "%.0f ", FP(totalammo));

					y++;
				}
				PrintString(dx, y, "                                ");

		}
#endif

//#define PRINT_ROUTE
#ifdef PRINT_ROUTE
	if(!(Persons2::PlayerSeenAC->fly.pModel->FrameCount & 0x0f))
	{

		AirStrucPtr plyr = Persons2::PlayerSeenAC;

		int hour = plyr->timeofday / (60 * 60 * 100);
		int mins = (plyr->timeofday - hour * 60 * 60 * 100) / (60 * 100);
		int secs = (plyr->timeofday - hour * 60 * 60 * 100 - mins * 60 * 100) / 100;
		PrintVar(70, 0, " %.0f::", FP(hour));
		PrintVar(74, 0,  "%.0f::", FP(mins));
		PrintVar(77, 0,  "%.0f ", FP(secs));

		if((plyr->waypoint) && (plyr->waypoint->ETA))
		{
			plyr->InterceptandRangeFindDesPosAndVel();
			PrintVar(50, 2, "%.2f ", FP(plyr->waypoint->ETA / 360000.0));
			PrintVar(55, 2, "%.0f ", FP(plyr->Range / 100));
			PrintVar(62, 2, "%.0f ", FP(plyr->waypoint->wpname));
			PrintVar(65, 2, "%.0f ", FP(UWord(plyr->HdgIntercept) / 182));
			PrintVar(70, 2, "%.0f ", FP((plyr->waypoint->ETA - plyr->timeofday) / 100));
			if(plyr->waypoint->ETA != plyr->timeofday)
				PrintVar(75, 2, "%.0f ", FP(plyr->Range / (plyr->waypoint->ETA - plyr->timeofday)));

			if(plyr->waypoint->next)
			{
				plyr->InterceptandRange(&plyr->waypoint->World, &plyr->waypoint->next->World);
				PrintVar(50, 3, "%.2f ", FP(plyr->waypoint->next->ETA / 360000.0));
				PrintVar(55, 3, "%.0f ", FP(plyr->Range / 100));
				PrintVar(62, 3, "%.0f ", FP(plyr->waypoint->next->wpname));
				PrintVar(65, 3, "%.0f ", FP(UWord(plyr->HdgIntercept) / 182));
				PrintVar(70, 3, "%.0f ", FP((plyr->waypoint->next->ETA - plyr->waypoint->ETA) / 100));
				if(plyr->waypoint->next->ETA != plyr->waypoint->ETA)
					PrintVar(75, 3, "%.0f ", FP(plyr->Range / (plyr->waypoint->next->ETA - plyr->waypoint->ETA)));

				if(plyr->waypoint->next->next)
				{
					plyr->InterceptandRange(&plyr->waypoint->next->World, &plyr->waypoint->next->next->World);
					PrintVar(50, 4, "%.2f ", FP(plyr->waypoint->next->next->ETA / 360000.0));
					PrintVar(55, 4, "%.0f ", FP(plyr->Range / 100));
					PrintVar(62, 4, "%.0f ", FP(plyr->waypoint->next->next->wpname));
					PrintVar(65, 4, "%.0f ", FP(UWord(plyr->HdgIntercept) / 182));
					PrintVar(70, 4, "%.0f ", FP((plyr->waypoint->next->next->ETA - plyr->waypoint->next->ETA) / 100));
					if(plyr->waypoint->next->next->ETA - plyr->waypoint->next->ETA)
						PrintVar(75, 4, "%.0f ", FP(plyr->Range / (plyr->waypoint->next->next->ETA - plyr->waypoint->next->ETA)));
				}
			}
		}
		int count = 0;
		for(AirStrucPtr ac = AirStrucPtr(plyr->ACList); ac; ac = AirStrucPtr(ac->nextmobile))
			if(ac->movecode == AUTO_NOPPILOT)
				count++;
		PrintVar(65, 15, "A_NOP %.0f ", FP(count));
		PrintVar(65, 16, "Total %.0f ", FP(Persons3::SagExpansionCounter::totalexpanded));
		PrintVar(65, 17, "SptA %.0f ", FP(Persons3::sagexpcounts[PT_SPIT_A].numactive));
		PrintVar(74, 17, "%.0f ", FP(Persons3::sagexpcounts[PT_SPIT_A].currenttriggerrange * 0.01));
		PrintVar(65, 18, "SptB %.0f ", FP(Persons3::sagexpcounts[PT_SPIT_B].numactive));
		PrintVar(74, 18, "%.0f ", FP(Persons3::sagexpcounts[PT_SPIT_B].currenttriggerrange * 0.01));
		PrintVar(65, 19, "HurA %.0f ", FP(Persons3::sagexpcounts[PT_HURR_A].numactive));
		PrintVar(74, 19, "%.0f ", FP(Persons3::sagexpcounts[PT_HURR_A].currenttriggerrange * 0.01));
		PrintVar(65, 20, "HurB %.0f ", FP(Persons3::sagexpcounts[PT_HURR_B].numactive));
		PrintVar(74, 20, "%.0f ", FP(Persons3::sagexpcounts[PT_HURR_B].currenttriggerrange * 0.01));
		PrintVar(65, 21, "B109 %.0f ", FP(Persons3::sagexpcounts[PT_ME109].numactive));
		PrintVar(74, 21, "%.0f ", FP(Persons3::sagexpcounts[PT_ME109].currenttriggerrange * 0.01));
		PrintVar(65, 22, "B110 %.0f ", FP(Persons3::sagexpcounts[PT_ME110].numactive));
		PrintVar(74, 22, "%.0f ", FP(Persons3::sagexpcounts[PT_ME110].currenttriggerrange * 0.01));
		PrintVar(65, 23, "Ju87 %.0f ", FP(Persons3::sagexpcounts[PT_JU87].numactive));
		PrintVar(74, 23, "%.0f ", FP(Persons3::sagexpcounts[PT_JU87].currenttriggerrange * 0.01));
		PrintVar(65, 24, "Bomb %.0f ", FP(Persons3::sagexpcounts[PT_DO17].numactive));
		PrintVar(74, 24, "%.0f ", FP(Persons3::sagexpcounts[PT_DO17].currenttriggerrange * 0.01));

		int numRAF = 0;		int comRAF = 0;
		int numLUF = 0;		int comLUF = 0;
		for(int i = 0; i < ACARRAYSIZE; i++)
			if(ACArray[i])
				if(ACArray[i]->nationality == NAT_RAF)
				{
					numRAF += CountSquadronSize(ACArray[i]);
					if((ACArray[i]->movecode == AUTO_COMBAT) || (ACArray[i]->movecode == AUTO_PRECOMBAT) || (ACArray[i]->movecode == AUTOSAG_TRACKEXPCOMBAT))
						comRAF += CountSquadronSize(ACArray[i]);
				}
				else
				{
					numLUF += CountSquadronSize(ACArray[i]);
					if((ACArray[i]->movecode == AUTO_COMBAT) || (ACArray[i]->movecode == AUTO_PRECOMBAT) || (ACArray[i]->movecode == AUTOSAG_TRACKEXPCOMBAT))
						comLUF += CountSquadronSize(ACArray[i]);
				}

		PrintVar(50, 6, "RAF %.0f ", FP(numRAF));
		PrintVar(60, 6, "LUF %.0f ", FP(numLUF));
		PrintVar(50, 7, "Com %.0f ", FP(comRAF));
		PrintVar(60, 7, "Com %.0f ", FP(comLUF));
	}
#endif


//#define PRINT_DAMAGE
#ifdef PRINT_DAMAGE
	AirStrucPtr plyr2 = Persons2::PlayerSeenAC;
	AircraftAnimData *adptr = (AircraftAnimData*)plyr2->Anim;
	PrintVar(72,  9, "%.0f ", FP((adptr->ENGINELEFT) / 2.55));
	PrintVar(72, 10, "%.0f ", FP((adptr->CANOPY) / 2.55));
	PrintVar(70, 11, "%.0f ", FP((adptr->LEFTWINGIN) / 2.55));
	PrintVar(74, 11, "%.0f ", FP((adptr->RIGHTWINGIN) / 2.55));
	PrintVar(68, 12, "%.0f ", FP((adptr->LEFTWINGOUT) / 2.55));
	PrintVar(76, 12, "%.0f ", FP((adptr->RIGHTWINGOUT) / 2.55));
	PrintVar(72, 13, "%.0f ", FP((adptr->OTHER) / 2.55));
	PrintVar(72, 14, "%.0f ", FP((adptr->FINLEFT) / 2.55));
	PrintVar(72, 15, "%.0f ", FP((adptr->TAIL) / 2.55));

	if(		(Trans_Obj.View_Object) && (Trans_Obj.View_Object->trackeditem2)
		&&	(Trans_Obj.View_Object->trackeditem2->Status.size == AIRSTRUCSIZE)
		&&	(!Trans_Obj.View_Object->trackeditem2->Status.deadtime)	)
	{
		AirStrucPtr ac = AirStrucPtr(Trans_Obj.View_Object->trackeditem2);
		if(!ac->fly.numinsag)
		{
			adptr = (AircraftAnimData*)ac->Anim;
			if(		(ac->classtype->visible == ME110) || (ac->classtype->visible == JU88) || (ac->classtype->visible == HE11)
				||	(ac->classtype->visible == DO17) || (ac->classtype->visible == BRISTO)	)
			{
				PrintVar(72, 17, "%.0f ", FP((adptr->CANOPY) / 2.55));
				PrintVar(69, 18, "%.0f ", FP((adptr->ENGINELEFT) / 2.55));
				PrintVar(75, 18, "%.0f ", FP((adptr->ENGINERIGHT) / 2.55));
				PrintVar(70, 19, "%.0f ", FP((adptr->LEFTWINGIN) / 2.55));
				PrintVar(74, 19, "%.0f ", FP((adptr->RIGHTWINGIN) / 2.55));
				PrintVar(68, 20, "%.0f ", FP((adptr->LEFTWINGOUT) / 2.55));
				PrintVar(76, 20, "%.0f ", FP((adptr->RIGHTWINGOUT) / 2.55));
				PrintVar(72, 21, "%.0f ", FP((adptr->OTHER) / 2.55));
				PrintVar(72, 22, "%.0f ", FP((adptr->TAIL) / 2.55));
				PrintVar(70, 23, "%.0f ", FP((adptr->FINLEFT) / 2.55));
				PrintVar(74, 23, "%.0f ", FP((adptr->FINRIGHT) / 2.55));
			}
			else
			{
				PrintVar(72, 17, "%.0f ", FP((adptr->ENGINELEFT) / 2.55));
				PrintVar(72, 18, "%.0f ", FP((adptr->CANOPY) / 2.55));
				PrintVar(70, 19, "%.0f ", FP((adptr->LEFTWINGIN) / 2.55));
				PrintVar(74, 19, "%.0f ", FP((adptr->RIGHTWINGIN) / 2.55));
				PrintVar(68, 20, "%.0f ", FP((adptr->LEFTWINGOUT) / 2.55));
				PrintVar(76, 20, "%.0f ", FP((adptr->RIGHTWINGOUT) / 2.55));
				PrintVar(72, 21, "%.0f ", FP((adptr->OTHER) / 2.55));
				PrintVar(72, 22, "%.0f ", FP((adptr->FINLEFT) / 2.55));
				PrintVar(72, 23, "%.0f ", FP((adptr->TAIL) / 2.55));
			}
		}
	}
#endif


//#define PRINT_BOMBING
#ifdef  PRINT_BOMBING

	int x = 0;
	for(int i = 0; i < ACARRAYSIZE; i++)
		if((ACArray[i]) && (ACArray[i]->GetLeadAc()->movecode == AUTO_BOMB))
		{
			x += 5;
			int y = 2;

			for(AirStrucPtr leadac = ACArray[i]->GetLeadAc(); leadac; leadac = leadac->fly.nextflight)
				for(AirStrucPtr ac = leadac; ac; ac = ac->Follower())
				{
					PrintVar(x, y, " %.0f ", FP(ac->ai.ManStep));
					y++;
				}
		}

#endif

//#define PRINT_COMBATSQUADRONS
#ifdef  PRINT_COMBATSQUADRONS
		int y = 0;
		for(int i = 0; i < ACARRAYSIZE; i++)
			if((ACArray[i]) && (ACArray[i]->ai.unfriendly) && (ACArray[i]->ai.unfriendly->Status.size == AIRSTRUCSIZE))// && ((ACArray[i]->movecode == AUTO_COMBAT) || (ACArray[i]->movecode == AUTO_PRECOMBAT)))
			{
				PrintVar(0, y, "%.0f  ", FP(i));
				AirStrucPtr group = ACArray[i];
				switch(group->classtype->visible)
				{
					case SPIT:		PrintString(4, y, "SPIT ");	break;
					case HURR:		PrintString(4, y, "HURR ");	break;
					case ME109:		PrintString(4, y, "M109 ");	break;
					case ME110:		PrintString(4, y, "M110 ");	break;
					case JU87:		PrintString(4, y, "JU87 ");	break;
					case DEF:		PrintString(4, y, "DEFI ");	break;
					case BRISTO:	PrintString(4, y, "BLEN ");	break;
					case JU88:		PrintString(4, y, "JU88 ");	break;
					case HE11:		PrintString(4, y, "H111 ");	break;
					case DO17:		PrintString(4, y, "DO17 ");	break;
					case JU52:		PrintString(4, y, "HE59 ");	break;
				}
				PrintVar(9, y, "%.0f  ", FP(CountSquadronSize(group)));
				for(int j = 0; j < ACARRAYSIZE; j++)
					if(ACArray[j] == group->ai.unfriendly)
					{
						PrintVar(12, y, "%.0f  ", FP(j));
						break;
					}

				AirStrucPtr target = AirStrucPtr(group->ai.unfriendly);
				switch(target->classtype->visible)
				{
					case SPIT:		PrintString(16, y, "SPIT ");	break;
					case HURR:		PrintString(16, y, "HURR ");	break;
					case ME109:		PrintString(16, y, "M109 ");	break;
					case ME110:		PrintString(16, y, "M110 ");	break;
					case JU87:		PrintString(16, y, "JU87 ");	break;
					case DEF:		PrintString(16, y, "DEFI ");	break;
					case BRISTO:	PrintString(16, y, "BLEN ");	break;
					case JU88:		PrintString(16, y, "JU88 ");	break;
					case HE11:		PrintString(16, y, "H111 ");	break;
					case DO17:		PrintString(16, y, "DO17 ");	break;
					case JU52:		PrintString(16, y, "HE59 ");	break;
				}

				PrintVar(21, y, "%.0f  ", FP(CountSquadronSize(target)));
				y++;
			}
		PrintString(0, y, "____________________");
#endif

//#define PRINT_TRACKEDITEM2
#ifdef PRINT_TRACKEDITEM2
	if((Trans_Obj.View_Object) && (Trans_Obj.View_Object->trackeditem2) && (Trans_Obj.View_Object->trackeditem2->Status.size == AIRSTRUCSIZE))
	{
		AirStrucPtr ac = AirStrucPtr(Trans_Obj.View_Object->trackeditem2);
		if(ac->movecode == AUTO_COMBAT)
			PrintString(0, 24, "COMBAT");
		else if(ac->movecode == AUTO_FOLLOWWP)
			PrintString(0, 24, "FOLLOW");
		else if(ac->movecode == AUTO_PRECOMBAT)
			PrintString(0, 24, "PRECOM");
		else
			PrintString(0, 24, "??????");

		switch(ac->ai.manoeuvre)
		{
			case MANOEUVRE_SELECT:				PrintString(10, 24, "SELECT ");				break;
			case MANOEUVRE_LOOKROUND:			PrintString(10, 24, "LOOKROUND ");			break;
			case MANOEUVRE_WELDEDWINGMAN:		PrintString(10, 24, "WELDEDWINGMAN ");		break;
			case MANOEUVRE_BAILOUT:				PrintString(10, 24, "BAILOUT ");				break;
			case MANOEUVRE_TOPCOVER:			PrintString(10, 24, "TOPCOVER ");			break;
			case MANOEUVRE_FORCETOPCOVER:		PrintString(10, 24, "FORCETOPCOVER ");		break;
			case MANOEUVRE_LINEABREAST:			PrintString(10, 24, "LINEABREAST ");			break;
			case MANOEUVRE_PINCER:				PrintString(10, 24, "PINCER ");				break;
			case MANOEUVRE_MULTIWAVE:			PrintString(10, 24, "MULTIWAVE ");			break;
			case MANOEUVRE_DIVEANDZOOM:			PrintString(10, 24, "DIVEANDZOOM ");			break;
			case MANOEUVRE_LEADPURSUIT:			PrintString(10, 24, "LEADPURSUIT ");			break;
			case MANOEUVRE_LAGPURSUIT:			PrintString(10, 24, "LAGPURSUIT ");			break;
			case MANOEUVRE_SPLITMANOEUVRE:		PrintString(10, 24, "SPLITMANOEUVRE ");		break;
			case MANOEUVRE_HEADON:				PrintString(10, 24, "HEADON ");				break;
			case MANOEUVRE_LINEASTERN:			PrintString(10, 24, "LINEASTERN ");			break;
			case MANOEUVRE_VICATTACK:			PrintString(10, 24, "VICATTACK ");			break;
			case MANOEUVRE_BARRELROLLATTACK:	PrintString(10, 24, "BARRELROLLATTACK ");	break;
			case MANOEUVRE_SCISSORS:			PrintString(10, 24, "SCISSORS ");			break;
			case MANOEUVRE_MILDSCISSORS:		PrintString(10, 24, "MILDSCISSORS ");		break;
			case MANOEUVRE_TURNINGFIGHT:		PrintString(10, 24, "TURNINGFIGHT ");		break;
			case MANOEUVRE_SPLITS:				PrintString(10, 24, "SPLITS ");				break;
			case MANOEUVRE_ZOOMANDDROP:			PrintString(10, 24, "ZOOMANDDROP ");			break;
			case MANOEUVRE_STRAIGHTANDLEVEL:	PrintString(10, 24, "STRAIGHTANDLEVEL ");	break;
			case MANOEUVRE_SPINOUT:				PrintString(10, 24, "SPINOUT ");				break;
			case MANOEUVRE_DIVEFORHOME:			PrintString(10, 24, "DIVEFORHOME ");			break;
			case MANOEUVRE_GOHOME:				PrintString(10, 24, "GOHOME ");				break;
			case MANOEUVRE_MAKEFORFRIENDLY:		PrintString(10, 24, "MAKEFORFRIENDLY ");		break;
			case MANOEUVRE_MOVEAWAY:			PrintString(10, 24, "MOVEAWAY ");			break;
			case MANOEUVRE_ACCIDENTALSPINOUT:	PrintString(10, 24, "ACCIDENTALSPINOUT ");	break;
			case MANOEUVRE_VERTICALLOOP:		PrintString(10, 24, "VERTICALLOOP ");		break;
			case MANOEUVRE_IMMELMANNTURN:		PrintString(10, 24, "IMMELMANNTURN ");		break;
			case MANOEUVRE_IMMELMANN:			PrintString(10, 24, "IMMELMANN ");			break;
			case MANOEUVRE_STAYWITHPREY:		PrintString(10, 24, "STAYWITHPREY ");		break;
			case MANOEUVRE_CLIMBFORHOME:		PrintString(10, 24, "CLIMBFORHOME ");		break;
			case MANOEUVRE_STRAIGHTDIVE:		PrintString(10, 24, "STRAIGHTDIVE ");		break;
			case MANOEUVRE_STANDONTAIL:			PrintString(10, 24, "STANDONTAIL ");			break;
			case MANOEUVRE_SHOOTTOFRIGHTEN:		PrintString(10, 24, "SHOOTTOFRIGHTEN ");		break;
			case MANOEUVRE_SHOOTTOOEARLY:		PrintString(10, 24, "SHOOTTOOEARLY ");		break;
			case MANOEUVRE_GAINHEIGHT:			PrintString(10, 24, "GAINHEIGHT ");			break;
			case MANOEUVRE_LAGROLL:				PrintString(10, 24, "LAGROLL ");				break;
			case MANOEUVRE_EXTENSION:			PrintString(10, 24, "EXTENSION ");			break;
			case MANOEUVRE_DIVINGSPIN:			PrintString(10, 24, "DIVINGSPIN ");			break;
			case MANOEUVRE_REVERSETURN:			PrintString(10, 24, "REVERSETURN ");			break;
			case MANOEUVRE_SELFASBAIT:			PrintString(10, 24, "SELFASBAIT ");			break;
			case MANOEUVRE_JINK:				PrintString(10, 24, "JINK ");				break;
			case MANOEUVRE_BREAKTURN:			PrintString(10, 24, "BREAKTURN ");			break;
			case MANOEUVRE_LAZYTURN:			PrintString(10, 24, "LAZYTURN ");			break;
			case MANOEUVRE_BREAKLOW:			PrintString(10, 24, "BREAKLOW ");			break;
			case MANOEUVRE_BREAKHIGH:			PrintString(10, 24, "BREAKHIGH ");			break;
			case MANOEUVRE_BREAK90:				PrintString(10, 24, "BREAK90 ");				break;
			case MANOEUVRE_HIGBARRELROLL:		PrintString(10, 24, "HIGBARRELROLL ");		break;
			case MANOEUVRE_PANICTURN:			PrintString(10, 24, "PANICTURN ");			break;
			case MANOEUVRE_UNBALANCEDFLIGHT:	PrintString(10, 24, "UNBALANCEDFLIGHT ");	break;
			case MANOEUVRE_LOWALT:				PrintString(10, 24, "LOWALT ");				break;
			case MANOEUVRE_TURNTOHDGANDPITCH:	PrintString(10, 24, "TURNTOHDGANDPITCH ");	break;
			case MANOEUVRE_ZOOM:				PrintString(10, 24, "ZOOM ");				break;
			case MANOEUVRE_INTERCEPTHIGH:		PrintString(10, 24, "INTERCEPTHIGH ");		break;
			case MANOEUVRE_INTERCEPTLOW:		PrintString(10, 24, "INTERCEPTLOW ");		break;
			case MANOEUVRE_GAINSPEED:			PrintString(10, 24, "GAINSPEED ");			break;
			case MANOEUVRE_HEADONATTACK:		PrintString(10, 24, "HEADONATTACK ");		break;
			case MANOEUVRE_LUFBERRY:			PrintString(10, 24, "LUFBERRY ");			break;
			case MANOEUVRE_STEEPDIVE:			PrintString(10, 24, "STEEPDIVE ");			break;
			case MANOEUVRE_UPANDUNDER:			PrintString(10, 24, "UPANDUNDER ");			break;
			case MANOEUVRE_STALLTURN:			PrintString(10, 24, "STALLTURN ");			break;
			case MANOEUVRE_SLASHINGATTACK:		PrintString(10, 24, "SLASHINGATTACK ");		break;
			case MANOEUVRE_CLIMBTURN:			PrintString(10, 24, "CLIMBTURN ");			break;
			case MANOEUVRE_ROLL360:				PrintString(10, 24, "ROLL360 ");				break;
			case MANOEUVRE_STRAFFE:				PrintString(10, 24, "STRAFFE ");				break;
			case MANOEUVRE_FLYTHROUGHCLOUD:		PrintString(10, 24, "FLYTHROUGHCLOUD ");		break;
			case MANOEUVRE_REGROUP:				PrintString(10, 24, "REGROUP ");				break;
			case MANOEUVRE_SPINRECOVERY:		PrintString(10, 24, "SPINRECOVERY ");		break;
			case MANOEUVRE_COLLISIONAVOIDANCE:	PrintString(10, 24, "COLLISIONAVOIDANCE ");	break;
			case MANOEUVRE_COLLIDE:				PrintString(10, 24, "COLLIDE ");				break;
			case MANOEUVRE_DISENGAGED:			PrintString(10, 24, "DISENGAGED ");			break;
		}
	}
#endif

#endif

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CollisionCheck
//Author		Craig Beeston
//Date			Thu 03 Feb 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void ArtInt::CollisionCheck()
{
	if(_DPlay.Implemented)
		return;

	if(CollideListInd <= 0)
	{
		MakeCollisionLists();
		CollideListInd = COLLIDECHECKLISTSIZE;
	}

	CollideListInd--;
	AirStrucPtr ac = CollideCheckList[CollideListInd];
	for(SWord i = 0; i < COLLIDEAGAINSTLISTSIZE; i++)
	{
		AirStrucPtr ac2 = CollideAgainstList[i];
		if((ac) && (ac2) && (ac2 != ac))
			CollisionAvoidance(ac, ac2);
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		MakeCollisionLists
//Author		Craig Beeston
//Date			Thu 03 Feb 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void ArtInt::MakeCollisionLists()
{
	if(_DPlay.Implemented)
		return;

	SWord i;
	for(i = 0; i < COLLIDECHECKLISTSIZE; i++)
		CollideCheckList[i] = NULL;

	for(i = 0; i < COLLIDEAGAINSTLISTSIZE; i++)
		CollideAgainstList[i] = NULL;

	AirStrucPtr player = Persons2::PlayerGhostAC;
	MobileItemPtr entry;
	SWord CheckListInd = 0;
	CollideAgainstList[0] = player;
	SWord AgainstListInd = 1;

	for(entry = mobileitem::ACList; entry; entry = entry->nextmobile)
		if(		(entry != player) && (entry != Persons2::PlayerSeenAC)
			&&	(entry->Distance3DSquared(&player->World) < 50000.0 * 50000.0)	)
		{
			AirStrucPtr ac = AirStrucPtr(entry);

			AgainstListInd++;
			if(AgainstListInd < COLLIDEAGAINSTLISTSIZE)
				CollideAgainstList[AgainstListInd] = ac;

			if(		(ac->movecode == AUTO_COMBAT)
				&&	(ac->ai.manoeuvre != MANOEUVRE_FORCETOPCOVER)
				&&	(ac->ai.manoeuvre != MANOEUVRE_COLLIDE)		)
			{
				CheckListInd++;
				if(CheckListInd < COLLIDECHECKLISTSIZE)
					CollideCheckList[CheckListInd] = ac;
			}

			if((CheckListInd >= COLLIDECHECKLISTSIZE) && (AgainstListInd >= COLLIDEAGAINSTLISTSIZE))
				break;
		}

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CollisionAvoidance
//Author		Craig Beeston
//Date			Wed 02 Feb 2000
//
//Description	Checks if aircraft 1 are likely to collide with aircraft 2 and takes evasive action
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void ArtInt::CollisionAvoidance(AirStrucPtr ac1, AirStrucPtr ac2)
{
	if(ac1->movecode != AUTO_COMBAT)
		return;

	FP crudedist = ac1->Distance3DSquared(&ac2->World);
	if(crudedist > 40000 * 40000)
		return;			 //min 1.5 secs until collision

	FP facing =	  ac1->vel_x * (FP(ac2->World.X) - ac1->World.X)
				+ ac1->vel_y * (FP(ac2->World.Y) - ac1->World.Y)
				+ ac1->vel_z * (FP(ac2->World.Z) - ac1->World.Z);
	if(facing < 0)
		return;

	FP closure =  -	(FP(ac1->vel_x) - ac2->vel_x) * (FP(ac1->World.X) - ac2->World.X)
				  - (FP(ac1->vel_y) - ac2->vel_y) * (FP(ac1->World.Y) - ac2->World.Y)
				  - (FP(ac1->vel_z) - ac2->vel_z) * (FP(ac1->World.Z) - ac2->World.Z);
	if(closure < 0)
		return;

	crudedist = FSqrt(crudedist);
	FP crudedist2 = crudedist - (SHAPESTUFF.GetShapePtr(ac1->shape)->Size + SHAPESTUFF.GetShapePtr(ac2->shape)->Size) * 8;
	if(ac2 == Persons2::PlayerGhostAC)
		crudedist2 *= 4;

	FP timetocollide = 10000.0 * crudedist2 * crudedist / closure;
	if(timetocollide > 128)
		return;

	{

		ac1->InterceptandRange(ac2);

		SWord breakhigh  = ac1->pitch - ac1->PitchIntercept;
		SWord breakright = ac1->hdg - ac1->HdgIntercept;

		ac1->ai.desiredpitch = ac1->pitch;
		ac1->ai.desiredhdg   = ac1->hdg;

		if(breakhigh > SWord(ANGLES_355Deg))
			ac1->ai.desiredpitch += ANGLES_30Deg;
		else
			ac1->ai.desiredpitch -= ANGLES_30Deg;

		if(breakright > ANGLES_0Deg)
			ac1->ai.desiredhdg += ANGLES_90Deg;
		else
			ac1->ai.desiredhdg -= ANGLES_90Deg;

		ac1->ai.manoeuvre = MANOEUVRE_COLLISIONAVOIDANCE;
		ac1->ai.ManStep = 0;
		if(ac2->classtype->aerobaticfactor == AEROBATIC_LOW)
		{
			ac1->SetManoeuvreTime(400);
			if(ac1->nationality != ac2->nationality)
				ac1->ai.firsttactic = TACTIC_NULL;
		}
		else
			ac1->SetManoeuvreTime(50);
	}
}


void	ArtInt::ExpandMigs(AirStrucPtr placeholder)
{
	//Comms interface goes here and does this:
//TempCode JIM 13May99 T::F_C7_RED,
//TempCode JIM 13May99 T::F_C7_UN,

// dont need to send a message for replay because expandmigs uses ghost AC position
// to expand Migs. In time accel for replay the Migs could get expanded but upon
// resuming record the block header will record all their positions.
		placeholder->Status.Drawn=TRUE;							//AMM 12Apr99
}

void	ArtInt::ReallyExpandMigs(AirStrucPtr placeholder)
{	//GUARANTEED 256/50=5 SECOND DELAY BETWEEN THESE 2 ROUTINES!!!
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		VisibleAcCheck
//Author		R. Hyde
//Date			Fri 27 Feb 1998
//
//Description	All aircraft including medium bombrs come through here
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////
//
// Function:    VisibleAcCheck
// Date:        17/02/99
// Author:      RDH
//
// Description:
//
////////////////////////////////////////////////////////////////////////
void	ArtInt::VisibleAcCheck()
{
	{for (int ai=0;ai<ACARRAYSIZE;ai++)
	{
		AirStruc* Eye=ACArray[ai];
		assert (!Eye||!Eye->fly.numinsag||!Eye->fly.nextflight);
		if(Eye)
		{
			if((Eye->fly.numinsag < 0) || (Eye->fly.numinsag > 16))
				INT3;
			if((Eye->ai.unfriendly) && (Eye->ai.unfriendly->Status.size == AIRSTRUCSIZE))
				NOP;													//JIM 20Oct00
		}
	}}

	if(MemberListInd <= 0)		//Frame 1 of 5
	{
		ACArrayInd++;
		if(ACArrayInd >= ACARRAYSIZE)
			ACArrayInd = 0;
		MakeMemberList(ACArray[ACArrayInd]);
		MemberListInd = MEMBERLISTSIZE;

		AirStruc* eye=ACArray[ACArrayInd];
		if(eye)
			if(GroundVisible(eye))  // jon				  //CSB 04/02/00
				eye->ai.beenseen = TRUE;								//CSB 07/03/00
			else														//CSB 07/03/00
				eye->ai.beenseen = FALSE;								//CSB 07/03/00
	}
	else
	{							//Frames 2,3,4,5 of 5
		AirStrucPtr Eye = ACArray[ACArrayInd];
//		assert (Eye==NULL || Eye->fly.nextflight==NULL);
		MemberListInd--;
		AirStrucPtr Trg = MemberList[MemberListInd];
			assert (!Eye||!Eye->fly.numinsag||!Eye->fly.nextflight);
			assert (!Trg||!Trg->fly.numinsag||!Trg->fly.nextflight);

		if((Eye) && (Trg) && (Eye->nationality != Trg->nationality))
		{
			//assert (Trg->fly.nextflight==NULL);
			AirStrucPtr LEye = Eye;


			if(/*(Eye->fly.numinsag) && */(Eye->fly.expandedsag))
				LEye = Eye->fly.expandedsag;		//if SAG is expanded - find squadron leader
			AirStrucPtr LTrg = Trg;
			if(/*(Trg->fly.numinsag) && */(Trg->fly.expandedsag))
				LTrg = Trg->fly.expandedsag;

			if((LEye) && (LTrg))
			{
				bool breakout = false;
				for(AirStrucPtr leye = LEye; (leye) && (!breakout); leye = leye->fly.nextflight)
					for(AirStrucPtr eye = leye; (eye) && (!breakout); eye = eye->Follower())
						if(		(!eye->Status.deadtime)	&& (eye->movecode != AUTO_NOPPILOT)
							&&	(eye->movecode != AUTOSAG_DESTROYED) && (!eye->AircraftDamaged())
							&&	((eye->movecode != AUTO_COMBAT) && (eye->movecode != AUTO_PRECOMBAT))// || (eye->ai.unfriendly == NULL))
							&&	(eye->vel_ > 3 * eye->classtype->minvel / 2)
							&&	((!eye->fly.pModel) || (eye->World.Y > eye->fly.pModel->GroundHeight + FT_100))	)
						{
							if(		((eye != Persons2::PlayerGhostAC) && (eye != Persons2::PlayerSeenAC))
								||	((eye->classtype->visible == ME110) || (eye->classtype->visible == JU87))	)
							{
								for(AirStrucPtr ltrg = LTrg; (ltrg) && (!breakout); ltrg = ltrg->fly.nextflight)
									for(AirStrucPtr trg = ltrg; (trg) && (!breakout); trg = trg->Follower())
										if((!trg->Status.deadtime) && (trg->movecode != AUTO_NOPPILOT) && (trg->movecode != AUTOSAG_DESTROYED))
											if(Seen(eye, trg))
											{
												if (Save_Data.gamedifficulty[GD_TACTICALRANGE])
													OverLay.CancelAccel();

												if((eye != Persons2::PlayerGhostAC) && (eye != Persons2::PlayerSeenAC))
												{
														SpottedUnfriendly(eye, trg);
												}
												else
												{
													VoiceType	theVoice;	//RJS 12Sep00
													if (eye->nationality == NAT_RAF)
														theVoice = VOICE_REST;
													else
														theVoice = VOICE_LWREST;

													_Radio.TriggerMsg(MESSAGE_STRUC(ChooseSpottedScript(eye, trg), MSG_SPOT, eye, trg, eye).SetVoice(theVoice));	//RJS 12Sep00
												}

												breakout = true;
											}
							}
						}
			}
		}
	}

}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		MakeMemberList
//Author		Craig Beeston
//Date			Tue 01 Feb 2000
//
//Description	Find the 4 appropriate aircraft groups to do spotting checks against
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void ArtInt::MakeMemberList(AirStrucPtr ac)
{
	for(SWord i = 0; i < MEMBERLISTSIZE; i++)
	{
		MemberList[i] = NULL;
		Dist2List[i]  = -1;
	}

	if(ac == NULL)
		return;

	AirStrucPtr LeadAc = ac;
	if((ac->fly.numinsag == 0) && (ac->fly.expandedsag))
		LeadAc = ac->fly.expandedsag;

	if(LeadAc->fly.numinsag)					//UnExpanded SAG
	{
		if(LeadAc->ai.attacker)
			AddToMemberList(LeadAc, LeadAc->ai.attacker, 0);
	}
	else										//Squadron Leader
		for(AirStrucPtr lac = LeadAc; lac; lac = lac->fly.nextflight)
			for(AirStrucPtr fac = lac; fac; fac = fac->Follower())
			{
				if(fac->ai.attacker)
					AddToMemberList(LeadAc, fac->ai.attacker->GetLeadAc(), 0);
			}

	bool slotsavailable = false;
	{
	for(SWord i = 0; (i < MEMBERLISTSIZE) && (!slotsavailable); i++)
		if(Dist2List[i] != 0)
			slotsavailable = true;
	}

	if(slotsavailable)
	{
		for(SWord i = 0; i < ACARRAYSIZE; i++)
		{
			AirStrucPtr trg = ACArray[i];
			if((trg) && (trg->nationality != ac->nationality))
				if((!trg->ai.attacker) || (!trg->IsOutNumberedBy(1.0)))	//CSB 11Oct00
					if((trg->vel_ > 100000) || (!LeadAc->CheckForWeapon(LT_BOMB)))
					{																																			//CSB 4Aug00
						float dist2 = ac->Distance3DSquared(&trg->World);
						if(dist2 < 1500000.0 * 1500000.0)
							AddToMemberList(LeadAc, trg, dist2);
					}
		}
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AddToMemberList
//Author		Craig Beeston
//Date			Tue 01 Feb 2000
//
//Description	Adds aircraft to the member list if it is closer than an existing one in the list
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void ArtInt::AddToMemberList(AirStrucPtr eye, AirStrucPtr ac, float dist2)
{
	for(SWord i = 0; i < MEMBERLISTSIZE; i++)
		if(ac == MemberList[i])								//ac is already in the list
			return;

	AirStrucPtr grouplead = eye;
	while(grouplead->fly.leadflight)
		grouplead = grouplead->fly.leadflight;
	if((!grouplead->fly.numinsag) && (grouplead->fly.expandedsag))
		grouplead = grouplead->fly.expandedsag;

	AirStrucPtr trglead = ac;																												//CSB 4Aug00
	while((trglead->fly.leadflight) && (trglead->classtype->aerobaticfactor == trglead->fly.leadflight->classtype->aerobaticfactor))		//CSB 4Aug00
		trglead = trglead->fly.leadflight;																									//CSB 4Aug00
	if((!trglead->fly.numinsag) && (trglead->fly.expandedsag))																				//CSB 4Aug00
		trglead = trglead->fly.expandedsag;																									//CSB 4Aug00
																																						//CSB 4Aug00
	if(		(grouplead->ai.spottedunfriendly[0] == trglead) || (grouplead->ai.spottedunfriendly[1] == trglead)								//CSB 4Aug00
		||	(grouplead->ai.spottedunfriendly[2] == trglead) || (grouplead->ai.spottedunfriendly[3] == trglead)	)							//CSB 4Aug00
		return;

	if(((Dist2List[0] == -1) || (dist2 < Dist2List[0])) && (!ac->IsOutNumberedBy(1.0)))//if there is an empty slot or ac is closer than furthest in list
	{
		MemberList[0] = ac;									//replace furthest
		Dist2List[0] = dist2;

		for(SWord i = 1; (i < MEMBERLISTSIZE) && (Dist2List[0] != -1); i++)
			if((Dist2List[i] == -1) || (Dist2List[i] > Dist2List[0]))
			{
				AirStrucPtr tempac = MemberList[0];			//and bring furthest of empty slot into list[0]
				float		tempfl = Dist2List[0];
				MemberList[0] = MemberList[i];
				Dist2List[0]  = Dist2List[i];
				MemberList[i] = tempac;
				Dist2List[i]  = tempfl;
			}
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LeadersGroupInCombat
//Author		 //RDH 13/06/99
//Date
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AttackSpareInLeadersGroup
//Author		 //RDH 13/06/99
//Date
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool ArtInt::AttackSpareInLeadersGroup(AirStruc* eye)
{
	AirStrucPtr ldr;
	AirStrucPtr ac;
	AirStrucPtr unf = NULL;

	//find an unfriendly engaged by this group
	for(ldr = eye->FindFormpos0(); ldr && !unf; ldr = ldr->fly.nextflight)
		for(ac = ldr; ac && !unf; ac = ac->Follower())
			if(		(ac->ai.unfriendly)
				&&	(ac->ai.unfriendly->Status.size == AIRSTRUCSIZE)
				&&	(AirStrucPtr(ac->ai.unfriendly)->nationality != ac->nationality))
					unf = AirStrucPtr(ac->ai.unfriendly);

	if(!unf)
		return(false);

	if((unf->fly.numinsag) && (unf->fly.expandedsag))
		unf = unf->fly.expandedsag;

	{
		//unfldr is a GroupLeader
		AirStrucPtr unfldr = unf->FindFormpos0();

		if(unfldr->classtype->aerobaticfactor == AEROBATIC_LOW)	//Choose a target in the back flight;
			while((unfldr->fly.nextflight) && (unfldr->fly.nextflight->follower))
				unfldr = unfldr->fly.nextflight;

		unf = NULL;
		for(ldr = unfldr; ldr && !unf; ldr = ldr->fly.nextflight)
			for(ac = ldr; ac && !unf; ac = ac->Follower())
				if((ac->ai.attacker == NULL) && (!ac->Status.deadtime))
					unf = ac;

		if(!unf)
		{
			SLong closestrange;
			for(ldr = unfldr; ldr; ldr = ldr->fly.nextflight)
				for(ac = ldr; ac; ac = ac->Follower())
				{
					SLong range = eye->Distance3D(&ac->World);
					if((ac == unfldr) || (range < closestrange))
					{
						unf = ac;
						closestrange = range;
					}
				}
		}
	}

	SetEngage(eye, unf, MANOEUVRE_SELECT, ANGLES_0Deg, ANGLES_0Deg, FALSE);	//CSB 07/07/99

	if((unf->ai.unfriendly) && (unf->ai.unfriendly->Status.size == AIRSTRUCSIZE))
	{
		AirStrucPtr trgunf = AirStrucPtr(unf->ai.unfriendly);
//x0r		if( trgunf->FindFormpos0 == eye->FindFormpos0)
		if( trgunf->FindFormpos0() == eye->FindFormpos0())
		{
			if(unf->Distance3DSquared(&trgunf->World) < FP(DANGERRANGE) * FP(DANGERRANGE))
				if(		FP(trgunf->vel_x) * FP(unf->World.X - trgunf->World.X)
					+	FP(trgunf->vel_y) * FP(unf->World.Y - trgunf->World.Y)
					+	FP(trgunf->vel_z) * FP(unf->World.Z - trgunf->World.Z) > 0)
				{
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ENGAGETAIL, MSG_STATUSREPORT, eye, unf, trgunf));
					if((trgunf->ai.morale < MORALE_MEDIUM) && (!Math_Lib.rnd(2)))
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ENGAGETAIL_REPLY, MSG_STATUSREPORT, trgunf, NULL, eye));
				}
				else
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ENGAGEBANDIT, MSG_STATUSREPORT, eye, unf, trgunf));
		}
	}
	return(true);
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GuessFormationSize
//Author		Jim Taylor
//Date			Mon 10 Jun 1996
//
//Description	If the aircraft is on its own then return a total number of a/c
//				spotted so-far this frame.
//				If the aircraft is in formation then count up number of other
//				a/c in the formation.
//				GuessFormationSize will guess if a singleton is spotted
//				CountFormationSize will say "1"
//
//Inputs		A/C formation member
//
//Returns		Integer count
//
//------------------------------------------------------------------------------
int		ArtInt::GuessFormationSize(AirStrucPtr caller, AirStrucPtr trg)
{
	return(CountFormationSize(trg));
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CountFormationSize
//Author		Jim Taylor
//Date			Mon 10 Jun 1996
//
//Description	Start with the spotted aircraft.
//				If aircraft is in formation find the squadron leader and
//

//If the aircraft is in formation then count up number of other
//				a/c in the formation.
//				if a singleton is spotted	CountFormationSize will say "1"
//
//Inputs		A/C formation member
//
//Returns		Integer count
//
//------------------------------------------------------------------------------
int ArtInt::CountFormationSize(AirStrucPtr trg)
{
	if((trg->fly.expandedsag) && (!trg->fly.numinsag))
		trg = trg->fly.expandedsag;

	while((trg->fly.leadflight) && (trg->Distance3DSquared(&trg->fly.leadflight->World) < FP(HALFVISIBLERANGE) * FP(HALFVISIBLERANGE)))
		trg = trg->fly.leadflight;

	int num = 0;
	for(int i = 0; i < ACARRAYSIZE; i++)
		if((ACArray[i]) && (ACArray[i]->nationality == trg->nationality))
		{
			AirStrucPtr as = ACArray[i];
			while((as->fly.leadflight) && (as->Distance3DSquared(&as->fly.leadflight->World) < FP(HALFVISIBLERANGE) * FP(HALFVISIBLERANGE)))
				as = as->fly.leadflight;

			if(as == trg)
				num += CountSquadronSize(as);			//Add number in escorting squadrons
		}

	return(num);

}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CountSquadronSize
//Author		Craig Beeston
//Date			Thu 20 Jan 2000
//
//Description	Counts the number of aircraft in a squadron -
//				assuming that if you've seen one, you've seen all of them.
//				If it is a SAG, returns the number in the SAG
//
//Inputs		A/C formation member
//
//Returns		Integer count
//
//------------------------------------------------------------------------------
int ArtInt::CountSquadronSize(AirStrucPtr trg)
{
	trg = trg->FindFormpos0();
	int num = 0;
	if((trg->fly.numinsag) && (!trg->fly.expandedsag))
		num = trg->SAGCountSquadronSize();
	else
		for(AirStrucPtr ldr = trg->GetLeadAc(); ldr; ldr = ldr->fly.nextflight)
			for(AirStrucPtr ac = ldr; ac; ac = ac->Follower())
					num++;

	return(num);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SameFlight
//Author		Jim Taylor
//Date			Tue 25 Jun 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool	ArtInt::SameFlight(AirStrucPtr a,AirStrucPtr b)
{
	if (a && a->Status.size==AIRSTRUCSIZE)
	{
		if (a->leader)	a=*a->leader;
		if (a->fly.leadflight)	a=a->fly.leadflight;
	}
	if (b && a->Status.size==AIRSTRUCSIZE)
	{
		if (b->leader)	b=*b->leader;
		if (b->fly.leadflight)	b=b->fly.leadflight;
	}
	return  (a==b);
}

void	ArtInt::ClearFollowerFormation(AirStrucPtr ac)
{
	if (ac)
		while (ac)
		{
			AirStrucPtr fol=ac;
			while (fol)
			{
				fol->information=IF_OUT_POS;
				fol=*fol->follower;
			}
			ac=ac->fly.nextflight;
		}

}


void	ArtInt::AllEngage(AirStrucPtr leadac)
{
	AutoMoveCodeTypeSelect	m=leadac->movecode;
	AirStrucPtr	w=leadac;
	while(w)
	{
		AirStrucPtr	f=w;
		while (f)
		{
			if(f->movecode == m)
			if((!f->AircraftDamaged()) && (f->classtype->aerobaticfactor != AEROBATIC_LOW))								//JIM 29Aug96
				Art_Int.AttackSpareInLeadersGroup(f);
			f=*f->follower;
		}
		w=w->fly.nextflight;
	}
}


void	ArtInt::AllBreakOff(AirStrucPtr leadac)
{
	if(leadac->IsUnexpandedSAG())
	{
		Art_Int.BreakOff(leadac);
		return;
	}

	leadac = leadac->FindFormpos0();
	AutoMoveCodeTypeSelect leadersoriginalmovecode = leadac->movecode;

	if(leadac->movecode == AUTO_COMBAT)
	{
		for(AirStrucPtr acflight = leadac; acflight; acflight = acflight->fly.nextflight)
			for(AirStrucPtr ac = acflight; ac; ac = ac->Follower())
				if((ac->movecode == AUTO_COMBAT) && (ac->ai.manoeuvre <= MANOEUVRE_TOPCOVER))
					Art_Int.BreakOff(ac);
	}
	else
	{
		for(AirStrucPtr acflight = leadac; acflight; acflight = acflight->fly.nextflight)
			for(AirStrucPtr ac = acflight; ac; ac = ac->Follower())
				if((ac->movecode == AUTO_COMBAT) || (ac->movecode == leadersoriginalmovecode))
					Art_Int.BreakOff(ac);
	}
}


void ArtInt::BreakOff(AirStrucPtr ac)
{
// ac that break off have no owner until they engage again
	if (_DPlay.Implemented)//AMM 12Mar99
		if (ac->CommsOwner==Persons2::PlayerSeenAC->uniqueID.count)		//AMM 12Mar99
			_DPlay.AddAIACToPositionList(ac);			//AMM 12Mar99

	ac->movecode = AUTO_FOLLOWWP;
	ac->information = IF_OUT_POS;
	ac->manoeuvretime = 0;

	ac->SetUnfriendly();												//CSB 1Aug00
}


bool	ArtInt::OkToAccel()
{
	//need to test for piloted A/C
	AirStrucPtr tmp;
	if (Manual_Pilot.ControlledAC2->waypoint==NULL)
	{
//TEMP	 	Persons_2.AddMessage(UserMsg::TIME,TEXT_ENTERLANDING,Manual_Pilot.ControlledAC2->ai.homebase);//JIM 06Dec96
		return(false);
	}
	if (Manual_Pilot.ControlledAC2->Status.deadtime)
	{
		return(false);
	}
	for (int i=0;i<ACARRAYSIZE;i++)
	{
		tmp = ACArray[i];
		if(		(tmp != 0)
			&&	(tmp->movecode == AUTO_COMBAT)
			&&	(tmp->ai.unfriendly)
			&&	(Manual_Pilot.ControlledAC2 != tmp)	)
		{
			tmp->Range = tmp->Distance3D(&Manual_Pilot.ControlledAC2->World);
			if(tmp->Range < 4000000)
				return(false);
		}
	}
	return(true);
}


void ArtInt::PersonalThreatMsg(AirStrucPtr trg, AirStrucPtr agg)
{
	if(		(trg->leader)
		&&	(trg->ai.elementtactics == WELDEDWING)
		&&	(trg->Leader()->ai.unfriendly)
		&&	(trg->Leader()->ai.unfriendly->Status.size==AIRSTRUCSIZE)
		&&	(trg->ai.unfriendly)
		&&	(trg->ai.unfriendly->Status.size==AIRSTRUCSIZE)
		&&	(trg->ai.Unfriendly() == trg->Leader()->ai.Unfriendly())
	)	//can't cover any longer
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_NOTCLEAR, MSG_CLEAR, trg, NULL, trg->Leader())); //RDH 17/06/99
	else
		if(!trg->AcIsPlayer())
		{
			RndVal rndpc = (RndVal)Math_Lib.rnd(RndValMAX);
			AirStruc* buddy = trg->FindBuddy();
			if((!buddy) && (trg->nationality == Manual_Pilot.ControlledAC2->nationality) && (Math_Lib.rnd(RndValMAX) < RND10PC))
				buddy = Manual_Pilot.ControlledAC2;

			if(buddy)
				switch(Math_Lib.rnd(3))
				{
					case 0:
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CLOSEANDFIRING, MSG_PERSONALTHREAT, trg, agg, buddy));	break;
					case 1:
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_THEYARESHOOTING, MSG_PERSONALTHREAT, trg, agg, buddy));	break;
					case 2:
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_AMUNDERATTACK, MSG_PERSONALTHREAT, trg, agg, buddy));	break;
				}
		}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PersonalThreat
//Author		Modified -	Craig Beeston
//Date						Thu 20 Jan 2000
//
//Description
//								   SCRIPT_CLOSEANDFIRING
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool ArtInt::PersonalThreat(AirStrucPtr trg, AirStrucPtr agg, bool process)
{
	if(trg == Manual_Pilot.ControlledAC2)
	{
		OverLay.CancelAccel();	//if trg is player come out of accel
		return(false);
	}

	if((trg->vel_ < trg->classtype->minvel) || ((trg->fly.pModel) && (trg->World.Y < trg->fly.pModel->GroundHeight + FT_100)))
		return(false);

	if(agg == Manual_Pilot.ControlledAC2)
		agg = Persons2::PlayerGhostAC;

	if(		(!agg) || (!trg)															//No aggressor or target
		||	((trg->Status.size != AIRSTRUCSIZE) || (agg->Status.size != AIRSTRUCSIZE))	//Both aren't aircraft
		||	((trg->nationality) == (agg->nationality))									//Friendly fire
		||	(trg->Status.deadtime) || (agg->Status.deadtime)							//Target already dead
		||	((agg != Persons2::PlayerGhostAC) && (agg->ai.unfriendly != trg))			//Accidental shot
		)
		return(false);

	if(trg->classtype->aerobaticfactor == AEROBATIC_LOW)	//Heavy bomber
	{
		trg = trg->FindFormpos0();
		AirStrucPtr escortleader = FindEscort(trg);
		if(escortleader)
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_AMUNDERATTACK, MSG_PERSONALTHREAT, trg, trg, escortleader));//RJS 02Feb00
		return(false);
	}

	if(		(trg->movecode != AUTO_COMBAT) && (trg->movecode != AUTO_PRECOMBAT)
		&&	(agg->ai.attacker) && (agg->Distance3DSquared(&agg->ai.attacker->World) < FP(WEAPONSRANGE) * FP(WEAPONSRANGE))	)
		return(false);


	PersonalThreatMsg(trg, agg);

	if(agg->classtype->aerobaticfactor == AEROBATIC_LOW)	//Defensive fire
	{
		if(Math_Lib.rnd(256) < trg->ai.combatskill)
			return(false);

		if((trg->ai.unfriendly) && (trg->ai.unfriendly->Status.size == AIRSTRUCSIZE) && (agg->ai.attacker))
			agg = AirStrucPtr(trg->ai.unfriendly);
	}

	MANOEUVRE manoeuvre = ChooseEvasiveManoeuvre(trg, agg);

	if(trg->ai.manoeuvre != manoeuvre)
	{
		if(trg->ai.unfriendly == agg)
		{
			trg->ai.manoeuvre = manoeuvre;
			trg->ai.ManStep   = 0;
			trg->SetManoeuvreTime(0);
		}
		else
			if((trg->movecode == AUTO_COMBAT) || (trg->movecode == AUTO_PRECOMBAT) || (!SpottedUnfriendly(trg, agg)))
			SetEngage(trg, agg, manoeuvre, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	}

	return(true);
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ChooseEvasiveManoeuvre
//Author		Craig Beeston
//Date			Wed 19 Jan 2000
//
//Description	Choose an evasive manoeuvre when fired upon.
//				Manouevre depends on skill, and aircraft type
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
MANOEUVRE ArtInt::ChooseEvasiveManoeuvre(AirStrucPtr trg, AirStrucPtr agg)
{
	if(		(trg->ai.manoeuvre == MANOEUVRE_BAILOUT)
		||	(trg->ai.manoeuvre == MANOEUVRE_PANICTURN)
		||	(trg->ai.manoeuvre == MANOEUVRE_MILDSCISSORS)
		||	(trg->ai.manoeuvre == MANOEUVRE_JINK)
		||	(trg->ai.manoeuvre == MANOEUVRE_HIGBARRELROLL)
		||	(trg->ai.manoeuvre == MANOEUVRE_REVERSETURN)
		||	(trg->ai.manoeuvre == MANOEUVRE_SCISSORS)
		||	(trg->ai.manoeuvre == MANOEUVRE_BREAKTURN)
		||	(trg->ai.manoeuvre == MANOEUVRE_BREAKHIGH)
		||	(trg->ai.manoeuvre == MANOEUVRE_BREAKLOW)
		||	(trg->ai.manoeuvre == MANOEUVRE_BREAK90)
		||	(trg->ai.manoeuvre == MANOEUVRE_DIVINGSPIN)
		||	(trg->ai.manoeuvre == MANOEUVRE_SPLITS)
		||	(trg->ai.manoeuvre == MANOEUVRE_CLIMBTURN)
		||	(trg->ai.manoeuvre == MANOEUVRE_UNBALANCEDFLIGHT)
		||	(trg->ai.manoeuvre == MANOEUVRE_STEEPDIVE)
		||	(trg->ai.manoeuvre == MANOEUVRE_ZOOM)
		||	(trg->ai.manoeuvre == MANOEUVRE_STALLTURN)
		||	(trg->ai.manoeuvre == MANOEUVRE_LAZYTURN)
		||	(trg->ai.manoeuvre == MANOEUVRE_STRAIGHTDIVE)	)
		return(trg->ai.manoeuvre);

	if(trg->ai.combatskill < 30)
		return(trg->ai.manoeuvre);

	bool choosegood = false;
	bool choosebad  = false;

	int rnd = Math_Lib.rnd(65536);
	int goodval = 65536 - trg->ai.combatskill * trg->ai.combatskill;
	int badval  = (256 - trg->ai.combatskill) * (256 - trg->ai.combatskill);

	if(rnd > goodval)
		choosegood = true;
	else if(rnd < badval)
		choosebad = true;

	if(agg->classtype->aerobaticfactor == AEROBATIC_LOW)
	{
		switch(Math_Lib.rnd(4))
		{
			case 0:	return(MANOEUVRE_HIGBARRELROLL);
			case 1:	return(MANOEUVRE_BREAKHIGH);
			case 2:	return(MANOEUVRE_BREAKLOW);
			case 3:	return(MANOEUVRE_BREAK90);
		}
		INT3;
	}

	if((trg->classtype->visible == SPIT) || (trg->classtype->visible == HURR))
	{
		if(choosegood)
			switch(Math_Lib.rnd(5))
			{
				case 0:	return(MANOEUVRE_DIVINGSPIN);
				case 1:	return(MANOEUVRE_HIGBARRELROLL);
				case 2:	return(MANOEUVRE_SPLITS);
				case 3:
				case 4:	return(MANOEUVRE_CLIMBTURN);
			}
		else if(choosebad)
			switch(Math_Lib.rnd(4))
			{
				case 0:	return(MANOEUVRE_SCISSORS);
				case 1:	return(MANOEUVRE_REVERSETURN);
				case 2:	return(MANOEUVRE_JINK);
				case 3:	return(MANOEUVRE_BREAKTURN);
			}
		else
			switch(Math_Lib.rnd(5))
			{
				case 0:	return(MANOEUVRE_BREAKLOW);
				case 1:	return(MANOEUVRE_BREAKHIGH);
				case 2:	return(MANOEUVRE_BREAK90);
				case 3:	return(MANOEUVRE_PANICTURN);
				case 4:	return(MANOEUVRE_UNBALANCEDFLIGHT);
			}
		INT3;
	}

	if((trg->classtype->visible == ME109) || (trg->classtype->visible == ME110))
	{
		if(choosegood)
			switch(Math_Lib.rnd(3))
			{
				case 0:
				case 1:	if(agg->World.Y > trg->World.Y)		return(MANOEUVRE_ZOOM);
						else								return(MANOEUVRE_STEEPDIVE);
				case 2:	return(MANOEUVRE_HIGBARRELROLL);
			}
		else if(choosebad)
			switch(Math_Lib.rnd(5))
			{
				case 0:	return(MANOEUVRE_SCISSORS);
				case 1:	return(MANOEUVRE_REVERSETURN);
				case 2:	return(MANOEUVRE_JINK);
				case 3:	return(MANOEUVRE_BREAKTURN);
				case 4:	return(MANOEUVRE_STALLTURN);
			}
		else
			switch(Math_Lib.rnd(5))
			{
				case 0:	return(MANOEUVRE_BREAKLOW);
				case 1:	return(MANOEUVRE_BREAKHIGH);
				case 2:	return(MANOEUVRE_BREAK90);
				case 3:	return(MANOEUVRE_PANICTURN);
				case 4:	return(MANOEUVRE_STEEPDIVE);//MANOEUVRE_UNBALANCEDFLIGHT);
			}
		INT3;
	}

	//Two seaters
	{
		if(choosegood)
			switch(Math_Lib.rnd(2))
			{
				case 0:	return(MANOEUVRE_DIVEFORHOME);
				case 1:	return(MANOEUVRE_STEEPDIVE);
			}
		else if(choosebad)
			switch(Math_Lib.rnd(3))
			{
				case 0:	return(MANOEUVRE_LAZYTURN);
				case 1:	return(MANOEUVRE_STRAIGHTDIVE);
				case 2:	return(MANOEUVRE_BAILOUT);
			}
		else
			switch(Math_Lib.rnd(3))
			{
				case 0:	return(MANOEUVRE_MILDSCISSORS);
				case 1:	return(MANOEUVRE_STALLTURN);
				case 2:	return(MANOEUVRE_REVERSETURN);
			}
		INT3;
	}

}


void	FormationItem::BreakForm()
{


	FormationItemPtr	replacement=NULL;
	replacement=follower;
	bool	gottrain=false;//(Persons2::getbandfromUID(uniqueID.count)==RedTrainBAND);//DAW 22Jun99
	FormationItemPtr	user=NULL;
	if (leader)
	{
		user=leader;

		while (user && user->follower!=this)
			user=user->follower;
		if (!user && movecode==AUTO_NOPPILOT)
			return;				//hopefully already dead so don't remove again!
		assert(user&&"not member of leaders convoy!");
		user->follower=replacement;
		if (replacement && gottrain)
			while (replacement)
			{
				replacement->movecode=AUTO_NOPPILOT;
				replacement=replacement->follower;
			}
	}
	else
	{
		if	(replacement)
		{
			replacement->formpos=formpos;
			replacement->leader=NULL;
			leader=replacement;
		}
		user=follower;
		while (user)
		{
			if (user->leader==this)
				user->leader=replacement;
			if (gottrain)
				user->movecode=AUTO_NOPPILOT;
			user=user->follower;
		}
	}
	movecode=AUTO_NOPPILOT;										//JIM 15Oct96
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		BreakForm
//Author		Jim Taylor
//Date			Thu 3 Jun 1999
//
//Description	Break aircaft out because he is dead usually.
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	AirStruc::BreakForm()
{
	DoAllSAGChecks();													//CSB 1Aug00

	if(fly.numinsag)
	{
		INT3;
		SAGBreakForm();
		DoAllSAGChecks();													//CSB 1Aug00
		return;
	}

	if(this == Manual_Pilot.ControlledAC2)
		OverLay.CancelAccel();

	if(this == Persons2::PlayerSeenAC)
		Persons2::PlayerGhostAC->BreakForm();

	if(ShouldDumpAndRun(false, true))
		DumpAndRun();


	AirStrucPtr	replacement = NULL;
	if(fly.nextflight)
		replacement = fly.nextflight;
	if(follower)
		replacement = *follower;

	if(replacement)
	{
		if(replacement->leader == this)
			replacement->leader = NULL;
		if(replacement->fly.leadflight == this)
			replacement->fly.leadflight = NULL;
		if((!formpos) || (((formpos & 0xf0) == 0) && ((replacement->formpos & 0xf0) != 0)))
			replacement->formpos = formpos;							  //JIM 09/07/99
		if((replacement->fly.nextflight == NULL) && (fly.nextflight!=replacement))
			replacement->fly.nextflight = fly.nextflight;
		replacement->information = IF_OUT_POS;							//CSB 22Aug00
		replacement->manoeuvretime = 0;
	}

	AirStrucPtr newtarg = NULL;
	AirStrucPtr atarg   = NULL;
	for(AirStrucPtr lead = FindFormpos0(); (lead) && (!newtarg); lead = lead->fly.nextflight)
		for(AirStrucPtr foll = lead; (foll) && (!newtarg); foll = foll->Follower())
			if(foll != this)
				if((!foll->ai.attacker) && (!foll->Status.deadtime))
					newtarg = foll;
				else
					atarg = foll;

	if(!newtarg)
		newtarg = atarg;

//SORT OUT MEMBERS OF THIS SQUADRON
	for(AirStrucPtr nf = FindFormpos0(); nf; nf = nf->fly.nextflight)
		for(AirStrucPtr ac = nf; ac; ac = ac->Follower())
		{
			if(ac->leader == this)
			{
				ac->leader = replacement;
				ac->information = IF_OUT_POS;
				ac->manoeuvretime = 0;
			}

			if(ac->fly.leadflight == this)
			{
				ac->fly.leadflight = replacement;
				ac->information = IF_OUT_POS;
				ac->manoeuvretime = 0;
			}

			if(ac->follower == this)
				ac->follower = replacement;

			if(ac->fly.nextflight == this)
				ac->fly.nextflight = replacement;
		}

	leader          = NULL;												//CSB 2Aug00
	follower        = NULL;												//CSB 2Aug00
	fly.leadflight  = NULL;												//CSB 2Aug00
	fly.nextflight  = NULL;												//CSB 2Aug00
	if(ai.unfriendly)
	{
		if((ai.unfriendly->Status.size == AIRSTRUCSIZE) && (AirStrucPtr(ai.unfriendly)->ai.attacker == this))
			AirStrucPtr(ai.unfriendly)->ai.attacker = NULL;
		ai.unfriendly = NULL;
	}
	if(ai.attacker)
	{
		ai.attacker->ai.unfriendly = NULL;
		ai.attacker = NULL;
	}

//SORT OUT EVERYONE ELSE
	{
	for(int i = 0; i < ArtInt::ACARRAYSIZE; i++)
		if(ArtInt::ACArray[i])
		{
			AirStrucPtr leadac = ArtInt::ACArray[i];
			AirStrucPtr realleadac = leadac;
			if((leadac->fly.numinsag) && (leadac->fly.expandedsag))
				realleadac = leadac->fly.expandedsag;

			for(AirStrucPtr nf = realleadac; nf; nf = nf->fly.nextflight)
				for(AirStrucPtr ac = nf; ac; ac = ac->Follower())
				{
					if(ac->fly.leadflight == this)
					{
						ac->fly.leadflight = replacement;
						ac->information = IF_OUT_POS;
						ac->manoeuvretime = 0;
					}

					if(ac->ai.unfriendly == this)
						ac->ai.unfriendly = NULL;
				}
		}
	}
//SORT OUT ACARRAY
	for(int i = 0; i < ArtInt::ACARRAYSIZE; i++)
		if(ArtInt::ACArray[i])
		{
			AirStrucPtr leadac = ArtInt::ACArray[i];
			AirStrucPtr realleadac = leadac;
			if((leadac->fly.numinsag) && (leadac->fly.expandedsag))
				realleadac = leadac->fly.expandedsag;

			if(formpos == 0)
				if(this == realleadac)
				{
					if(realleadac == leadac)
						ArtInt::ACArray[i] = replacement;
					else
					{
						leadac->fly.expandedsag = replacement;
						leadac->information = IF_OUT_POS;				//CSB 22Aug00
						leadac->manoeuvretime = 0;

						if(replacement == NULL)								//MS 7Jul00
						{
							fly.expandedsag = NULL;
							leadac->SAGDeathSequenceOverride(AUTOSAG_DESTROYED);	//RJS 26Sep00
						}

						assert(replacement||(leadac->movecode&-2)!=AUTOSAG_TRACKEXPFOLLOW);	//MS 7Jul00
					}
					if((realleadac != leadac) && (!leadac->fly.expandedsag) && (leadac->movecode != AUTOSAG_DESTROYED))
						INT3;
				}
		}

	fly.expandedsag = NULL;												//CSB 2Aug00
	formpos = FormationIndex(0);
	DoAllSAGChecks();													//CSB 1Aug00

//THIS DOES NOT WORK AS ONLY GROUP LEADERS ARE BEING CHECKED			//CSB 10/03/00
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SAGBreakForm
//Author		Craig Beeston
//Date			Wed 13 Sep 2000
//
//Description	Break SAG out because he is dead usually.
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::SAGBreakForm()
{
	DoAllSAGChecks();													//CSB 1Aug00

	if(!fly.numinsag)
	{
		INT3;
		BreakForm();
		DoAllSAGChecks();													//CSB 1Aug00
		return;
	}
	{
	for(int i = 0; i < ArtInt::ACARRAYSIZE; i++)
		if(ArtInt::ACArray[i])
		{
			AirStrucPtr sag = ArtInt::ACArray[i];
			if(sag->ai.unfriendly == this)
			{
				sag->ai.unfriendly = NULL;
				if(!sag->fly.expandedsag)
				{
					sag->movecode = AUTOSAG_FOLLOWWP;
					sag->information = IF_OUT_POS;
					sag->manoeuvretime = 0;
				}
			}
			if(sag->ai.attacker == this)
				sag->ai.attacker = NULL;
			if(sag->fly.leadflight == this)
			{
				sag->fly.leadflight = fly.leadflight;
				sag->information = IF_OUT_POS;							//CSB 22Aug00
				if(sag->fly.expandedsag)
					if(sag->fly.expandedsag->fly.leadflight == this)
					{
						sag->fly.expandedsag->fly.leadflight = fly.leadflight;
						sag->fly.expandedsag->information = IF_OUT_POS;	//CSB 22Aug00
					}
			}

			for(AirStrucPtr l = sag->fly.expandedsag; l; l = l->fly.nextflight)
				for(AirStrucPtr f = l; f; f = f->Follower())
					if(f->ai.unfriendly == this)
						f->ai.unfriendly = NULL;
		}
	}
	ai.unfriendly = NULL;
	for(int i = 0; i < 4; i++)
		ai.spottedunfriendly[i] = NULL;
	ai.attacker = NULL;
	fly.leadflight = NULL;
	fly.nextflight = NULL;

	DoAllSAGChecks();
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ShouldDumpAndRun
//Author		Craig Beeston
//Date			Wed 18 Oct 2000
//
//Description	When a bomber is killed, check to see if the rest of the group should chicken out and run home
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool AirStruc::ShouldDumpAndRun(bool FromSpot, bool FromKill, AirStrucPtr Enemy)
{
	if(!Persons2::PlayerGhostAC->fly.expandedsag)
		return(false);

	bool retval = false;

	if(FromSpot)
	{
		if((classtype->visible == JU87) && (Enemy))
		{
			AirStrucPtr lead = this;
			while(lead->fly.leadflight)
				lead = lead->fly.leadflight;
			if((lead->fly.expandedsag) && (!lead->fly.numinsag))
				lead = lead->fly.expandedsag;

			bool gotescort = false;
			for(int i = 0; i < ArtInt::ACARRAYSIZE; i++)
				if(		(ArtInt::ACArray[i]) && (ArtInt::ACArray[i]->fly.leadflight == lead)
					&&	(ArtInt::ACArray[i]->classtype->aerobaticfactor == AEROBATIC_HIGH)	)
				{
					gotescort = true;
					break;
				}

			if(gotescort)
			{
				FP pointingtowards =	FP(Enemy->vel_x) * FP(World.X - Enemy->World.X)
									+	FP(Enemy->vel_y) * FP(World.Y - Enemy->World.Y)
									+	FP(Enemy->vel_z) * FP(World.Z - Enemy->World.Z);
				if(pointingtowards > 0.0)
				{
					FP chancestay = Distance3D(&Enemy->World) / 8888.0 + ai.morale / 51.0;
					if(Math_Lib.rnd(100) > chancestay)
						retval = true;
				}
			}
		}
	}
	else if(FromKill)
	{
		if(classtype->aerobaticfactor == AEROBATIC_LOW)
		{
			WayPointPtr ip = waypoint->FindWP(WPNAME_IP);
			if((ip) && (ip->wpname == WPNAME_IP) && (ip->target))
			{
				AirStrucPtr lead = FindFormpos0();
				if(lead->fly.leadflight)
					lead = lead->fly.leadflight;
				else if(!lead->fly.numinsag)
					lead = lead->fly.expandedsag;

				int numstart = lead->fly.numinsag;
				int numleft = Art_Int.CountSquadronSize(lead);
				for(int i = 0; i < ArtInt::ACARRAYSIZE; i++)
					if((ArtInt::ACArray[i]) && (ArtInt::ACArray[i]->fly.leadflight == lead))
					{
						numstart += ArtInt::ACArray[i]->fly.numinsag;
						numleft  += Art_Int.CountSquadronSize(ArtInt::ACArray[i]);
					}

				FP percentstay = 70.0 + Distance3D(&ip->target->World) / 8000.0 - ai.morale / 25.5;
				if(100.0 * numleft > percentstay * numstart)
					retval = true;
			}
		}
		else if(classtype->visible == JU87)
		{
			WayPointPtr ip = waypoint->FindWP(WPNAME_IP);
			if((ip) && (ip->wpname == WPNAME_IP) && (ip->target))
			{
				AirStrucPtr lead = FindFormpos0();
				if(lead->fly.leadflight)
					lead = lead->fly.leadflight;
				else if(!lead->fly.numinsag)
					lead = lead->fly.expandedsag;

				int numstart = lead->fly.numinsag;
				int numleft = Art_Int.CountSquadronSize(lead);
				for(int i = 0; i < ArtInt::ACARRAYSIZE; i++)
					if((ArtInt::ACArray[i]) && (ArtInt::ACArray[i]->fly.leadflight == lead))
					{
						numstart += ArtInt::ACArray[i]->fly.numinsag;
						numleft  += Art_Int.CountSquadronSize(ArtInt::ACArray[i]);
					}

				FP percentstay = 70.0 + Distance3D(&ip->target->World) / 8000.0 - ai.morale / 25.5;
				if(100.0 * numleft > percentstay * numstart)
					retval = true;
			}
		}
	}

	return(retval);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DumpAndRun
//Author		Craig Beeston
//Date			Wed 18 Oct 2000
//
//Description	group chicken out and run home
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::DumpAndRun()
{
	AirStrucPtr lead = this;
	while(lead->fly.leadflight)
		lead = lead->fly.leadflight;
	if((lead->fly.expandedsag) && (!lead->fly.numinsag))
		lead = lead->fly.expandedsag;

	AirStrucPtr callee = NULL;

	for(int i = 0; i < ArtInt::ACARRAYSIZE; i++)
		if((ArtInt::ACArray[i]) && ((ArtInt::ACArray[i] == lead) || (ArtInt::ACArray[i]->fly.leadflight == lead)))
		{
			AirStrucPtr leader = ArtInt::ACArray[i];
			if(leader->fly.expandedsag)
				leader = leader->fly.expandedsag;

			for(AirStrucPtr ld = leader; ld; ld = ld->fly.nextflight)
				for(AirStrucPtr ac = ld; ac; ac = ac->Follower())
				{
					WorldStuff*	worldptr = mobileitem::currworld;
					Trans_Obj.DropAllStores((mobileitem*)ac, *worldptr);
					ac->waypoint = ac->waypoint->FindWP(WPNAME_EscDisperse);
					if(!callee)
						callee = ac;
				}
		}

	_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_DROPBOMBS_L, MSG_FACTOOHOT, lead, NULL, callee));
	_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ALLGOHOME, MSG_TACTOOHOT, lead, NULL, callee));
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PlayerHitConvoy
//Author		R. Hyde
//Date			Thu 12 Dec 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ProcessWPEvents
//Author		R. Hyde
//Date			Fri 27 Feb 1998
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LaunchMiGs
//Author		R. Hyde
//Date			Fri 27 Feb 1998
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		VisibleContrailsCheck
//Author		R. Hyde
//Date			Tue 3 Mar 1998
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------


void	ArtInt::VisibleAAACheck()
{
}


//All new:
////////////////////////////////////////////////////////////////////////
//
// Function:    SendMsgAndCall
// Date:        15/02/99
// Author:      RDH
//
// Description:
//
////////////////////////////////////////////////////////////////////////
inline const MESSAGE_STRUC& MESSAGE_STRUC::CheckCalleePlayer() const
{
	if (callee==Persons2::PlayerSeenAC)
		callee=Persons2::PlayerGhostAC;
	return *this;
}
inline const MESSAGE_STRUC& MESSAGE_STRUC::CheckCallerPlayer() const
{
	if (caller==Persons2::PlayerSeenAC)
		caller=Persons2::PlayerGhostAC;
	return *this;
}
inline const MESSAGE_STRUC& MESSAGE_STRUC::CheckTargetPlayer() const
{
	if (target==Persons2::PlayerSeenAC)
		target=Persons2::PlayerGhostAC;
	return *this;
}

bool	ArtInt::SendMsgAndCall(const MESSAGE_STRUC& msg,DecisionAI* processor) //JIM 22/01/99
{
#ifndef NDEBUG
if(		((msg.caller) && (msg.caller->Status.deadtime))
   ||	((msg.callee) && (msg.callee->Status.deadtime))
   ||	((msg.target) && (msg.target->Status.deadtime))		)
   INT3;
#endif

	//if	autodecisions not enabled					//autodecisions does everything...
	//	and message recient is player interested		//other groups do own thing
	//	and	sender does not equal player [interested]	//myflight told myself to engage!
	msg.CheckCalleePlayer().CheckCallerPlayer().CheckTargetPlayer();

	if (blocking)
	{
		assert (blocking==DoNothing);
		blocking = processor;
		return(true);
	}
	else if((processor->GetMsgOptions() == NULL) || (Save_Data.gamedifficulty[GD_AUTOVECTORING]))	  //RDH 18/04/99
	{
		_Radio.TriggerMsg(msg);
		return(processor->AutoMsgDecision(*msg.callee, msg.target, *msg.caller, this));
	}
	else if (!msg.callee->AcIsPlayer())
	{//automatic decision making
		_Radio.TriggerMsg(msg);

// want to block messages to remote players
		if((msg.callee) && (!msg.callee->uniqueID.commsmove))				//AMM 20Apr99
			return(processor->AutoMsgDecision(*msg.callee, msg.target, *msg.caller, this));
		else
			return(true);
	}
	else
	{//decision required from user
		OverLay.CancelAccel();
		_Radio.TriggerMsg(msg.SetDecision(processor));
		return(true);
	}
}


bool	DecisionAI::UserMsgDecision(AirStrucPtr callee,ItemBasePtr trg,			  //JIM 22/01/99
		AirStrucPtr caller,ArtInt* artint,MESSAGE_STRUC* PriorityMessage)
{
	return AutoMsgDecision(callee,trg,caller,artint,PriorityMessage);
}


PhraseTables DecisionAI::GetDefaultPhrase(AirStrucPtr callee,ItemBasePtr trg)
{
		#pragma warnmsg ("RadioChat:code commented out")
	Art_Int.blocking=DoNothing;
	OptionRef* msgopts=GetMsgOptions();
	assert(msgopts);

	bool tookaction=AutoMsgDecision(callee,trg,callee,&Art_Int);
	DecisionAI* returnedscript=Art_Int.blocking;
	Art_Int.blocking=NULL;
	if (tookaction)
	{
		assert(returnedscript!=DoNothing && returnedscript!=NULL);
		for (;msgopts->optionmsg!=PHRASE_NULL;msgopts++)	//RJS 03May00
			if (*msgopts->airesult == returnedscript)
				return msgopts->optionmsg;
		if (*msgopts->airesult == returnedscript)					  //JIM 21/06/99
			return msgopts->optionmsg;
		msgopts++;
		for (;msgopts->optionmsg!=PHRASE_NULL;msgopts++)	//RJS 02Feb100
			if (*msgopts->airesult == returnedscript)
				return msgopts->optionmsg;

		assert(!"Default action taken does not match any user action");

		return	(OLDPHRASE_CONTINUE);
	}
	else
	{
		assert(returnedscript==DoNothing || returnedscript==NULL);
		for (;msgopts->optionmsg!=PHRASE_NULL;msgopts++)
			if (!msgopts->airesult || msgopts->airesult==&DoNothing)
				return msgopts->optionmsg;
		return	PHRASE_NULL;	//no user phrase to continue...
	}

	return PHRASE_NULL;

}

////////////////////////////////////////////////////////////////////////
//
// Function:    AcIsPlayer
// Date:        21/02/99
// Author:      JIM
//
// Description: A variety of routines to find message recipients
//
////////////////////////////////////////////////////////////////////////
AirStrucPtr	AirStruc::AcIsPlayer()
{
	if (this==NULL)	return this;
	if (this==Persons2::PlayerGhostAC)	return this;
	if (this==Persons2::PlayerSeenAC)	return this;
	//for comms to pick up other remote player aircraft:
	//I don't think we want this!
	//if (this->uniqueID.commsmove)		return	true;
	return NULL;
}


AirStrucPtr	AirStruc::InPlayersElement()
{
	AirStrucPtr lead = this;
	while(lead->leader)
		lead = lead->Leader();

	for(AirStrucPtr ac = lead; ac; ac = ac->Follower())
		if(ac->AcIsPlayer())
			return(ac);

	return(NULL);
}


AirStrucPtr		AirStruc::FindBuddy()
{
	if (!this)
		return this;
	if (leader)
		return Leader();
	if (follower)
		return	Follower();
	if (fly.nextflight)
		return fly.nextflight;
	if (fly.leadflight)
		return fly.leadflight;
	return NULL;
}


AirStrucPtr		AirStruc::PlayerInFlight()
{
	AirStrucPtr	rv;
	rv=InPlayersElement();
	if (rv)
		return	rv;
	switch (position())
	{
	case 0:
		if (fly.nextflight)
			rv=fly.nextflight->InPlayersElement();
		break;
	case 1:
		if (leader && !follower && Leader()->fly.nextflight)
			rv=Leader()->fly.nextflight->InPlayersElement();
		break;
	case 2:
		if (!leader)
			rv=(fly.leadelt()->InPlayersElement());
		break;
	case 3:
		if (leader && leader->follower==this)
		rv=(Leader()->fly.leadelt()->InPlayersElement());
	}
	return rv;
}


AirStrucPtr AirStruc::PlayerInGroup()
{
	for(AirStrucPtr lead = FindFormpos0(); lead; lead = lead->fly.nextflight)		//CSB 07/03/00
		for(AirStrucPtr ac = lead; ac; ac = ac->Follower())							//CSB 07/03/00
			if((ac == Persons2::PlayerGhostAC) || (ac == Persons2::PlayerSeenAC))	//CSB 07/03/00
				return(ac);															//CSB 07/03/00
	return(NULL);																	//CSB 07/03/00
}



AirStrucPtr		AirStruc::FindAirEscortLeader()
{
	INT3;																//CSB 18Oct00
	return(NULL);														//CSB 20Oct00

}


AirStrucPtr		AirStruc::AcCloseAttackingOneOfGroup(bool& unffound)
{
	SLong range = COMBATRANGE;
	AirStruc*	possiblevictim = NULL;
	AirStruc*	victim = NULL;
	AirStruc*	lead = FindGroupLeader();
	AirStruc*	eye;
	while (lead)
	{
		eye = lead;
		while (eye)
		{
			if (eye->ai.attacker)
			{
				Art_Int.InterceptRangeFromTo(eye, eye->ai.attacker);
				if (		(Range < COMBATRANGE)
						&&	(Range < range)
					)
				{
					 unffound = true;
					 possiblevictim = eye;
					if (		(eye != this)
							&&	(Art_Int.TargetFacingCaller(eye, eye->ai.attacker, ANGLES_40Deg))
							&&	(Art_Int.TargetOnCallerTail(eye, eye->ai.attacker, ANGLES_40Deg))
							&&	(Art_Int.AircraftAreClose(eye, this))
						)
					{
						 victim = eye;
	 					 range = Range;
					}
				}
			}
			eye=(AirStruc*)eye->follower;
		}
		lead=lead->fly.nextflight;
	}
	if (unffound)
	{
		if (victim)
			possiblevictim = victim;		//override possiblevictim  with victim if there is one
	}


	return possiblevictim;
}


AirStrucPtr		AirStruc::RemoveElement()
{//elements in MiG are considered to be flights in our system

	AirStruc*	leadf;
	if (fly.leadflight)
		leadf = fly.leadflight;
	else
		leadf = NULL;

	AirStruc*	nextf = this->fly.nextflight;

	AirStruc*	prevf = leadf;

	if (prevf)
	while (prevf->fly.nextflight!=this)
		prevf=prevf->fly.nextflight;



	if (prevf)
		prevf->fly.nextflight = nextf;

	if ((nextf)	&&	(position() == 0))
		if (nextf->position())
			nextf->formpos = formpos;
		else if (formpos == 0)
		{
			nextf->formpos = formpos;

			//following flights
			for (AirStruc* nf= nextf;nf;nf= nf->fly.nextflight) //everybody's lead flight is new leader
				for (AirStruc* wing= nf;wing;wing= wing->Follower()) //everybody else's lead flight is new leader
				{//following leaders and their wingmen
					wing->fly.leadflight = nextf;

				}
			nextf->fly.leadflight = leadf;		//overwrite using escortee for the new flight leader

		}



//removed element
	fly.leadflight = NULL;
	fly.nextflight = NULL;

	formpos = FormationIndexMIN;


	return NULL;
}


AirStrucPtr		AirStruc::FindABuddyWithPlayerGivenPriority()
{
	AirStruc*	callee = FindBuddy();
	if	(		(PlayerInGroup())
			&&	(!AcIsPlayer())
		)
			callee  = Manual_Pilot.ControlledAC2;

	return callee;
}


AirStrucPtr		AirStruc::FindABuddyNotPlayer()
{
		if (!this)
		return this;
	if (leader)
		if (!Leader()->AcIsPlayer())
			return Leader();
	if (follower)
		if (!Follower()->AcIsPlayer())
		return	Follower();
	if (fly.nextflight)
		if (!fly.nextflight->AcIsPlayer())
		return fly.nextflight;
	if (fly.leadflight)
		if (!fly.leadflight->AcIsPlayer())
		return fly.leadflight;
	return NULL;

}


AirStrucPtr		AirStruc::FindAcInGroup()
{
	AirStruc*	buddy = FindBuddy();
	if (!buddy)
	{
		if (leader)//RJS 01Apr99
		{
			if (((AirStruc*) leader)->fly.nextflight)
				buddy = ((AirStruc*) leader)->fly.nextflight;
			else if (((AirStruc*) leader)->fly.leadflight)
				buddy = ((AirStruc*) leader)->fly.leadflight;
		}
	}
	return (buddy);
}




AirStrucPtr AirStruc::FindBandit(int& count)
{
	FP range2 = FP(VISIBLERANGE) * FP(VISIBLERANGE);
	int nearesti = -1;
	count = 0;
	for(int i = 0; i < Art_Int.ACARRAYSIZE; i++)
		if((Art_Int.ACArray[i]) && (Art_Int.ACArray[i]->nationality != nationality) && (!Art_Int.ACArray[i]->Status.deadtime))
		{
			AirStrucPtr ac = Art_Int.ACArray[i];
			FP acrange2 = Distance3DSquared(&ac->World);
			if(acrange2 < FP(VISIBLERANGE) * FP(VISIBLERANGE))
			{
				count++;
				if(acrange2 < range2)
				{
					nearesti = i;
					range2 = acrange2;
				}
			}
		}

	if(nearesti == -1)
		return(NULL);
	else
	{
		Range = FSqrt(range2);
		return(Art_Int.ACArray[nearesti]->GetLeadAc());
	}

}







AirStrucPtr		AirStruc::FindAnyBandit(SLong& range, int& count)
{
	INT3;																//CSB 18Oct00
	return(NULL);

	return NULL;
}


AirStrucPtr AirStruc::FindBanditOnTail()
{
	for(AirStrucPtr ac = AirStrucPtr(mobileitem::ACList); ac; ac = AirStrucPtr(ac->nextmobile))
		if((ac->nationality != nationality) && (ac->movecode == AUTO_COMBAT))
			if(Distance3DSquared(&ac->World) < FP(ENGAGERANGE) * FP(ENGAGERANGE))
			{
				FP dotprod = FP(World.X - ac->World.X) * FP(vel_x) + FP(World.Y - ac->World.Y) * FP(vel_y) + FP(World.Z - ac->World.Z) * FP(vel_z);
				dotprod /= (FP(vel_) * FP(Distance3D(&ac->World)));
				if(dotprod > 0.7)
					return(ac);
			}

	return(NULL);

}


bool AirStruc::InSameWave(AirStruc* trg)
{
	AirStruc* caller = FindGroupLeader();
	trg = trg->FindGroupLeader();
	if	(	(		(trg)
				&&	(trg->fly.leadflight == caller)
			)
			||
			(		(caller)
				&&	(caller->fly.leadflight == trg)
			)
		)
		return (true);
	else
		return (false);
}


void	ArtInt::AllChangeMovecode(AirStrucPtr leadac, AirStrucPtr trg, AutoMoveCodeTypeSelect movecode)
{
	AirStrucPtr	w=leadac;
	while(w)
	{
		AirStrucPtr	f=w;
		while (f)
		{
			f->SetUnfriendly(trg);										//CSB 1Aug00
			f->movecode = movecode;
			f=*f->follower;
		}
		w=w->fly.nextflight;
	}
}


////////////////////////////////////////////////////////////////////////
//
// Function:    SplitUnit
// Date:        16/02/2000
// Author:      Craig Beeston
//
// Description: Calculates the size of the unit commanded by ac and
// returns the ac in command of the second half of the unit
//
////////////////////////////////////////////////////////////////////////
AirStrucPtr ArtInt::SplitUnit(AirStrucPtr ac, UnitSize& unitsize)
{
	AirStrucPtr ac2ic = NULL;
	if(ac->nationality == NAT_RAF)
	{
		if((!ac->follower) || (ac->Follower()->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_RAF_SINGLE;
			ac2ic = NULL;
		}
		else if((!ac->fly.nextflight) || (ac->fly.nextflight->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_RAF_SECTION;
			ac2ic = ac->Follower();
		}
		else if((!ac->fly.nextflight->fly.nextflight) || (ac->fly.nextflight->fly.nextflight->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_RAF_FLIGHT;
			ac2ic = ac->fly.nextflight;
		}
		else if((!FindEscort(ac)) || (FindEscort(ac)->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_RAF_SQUADRON;
			ac2ic = ac->fly.nextflight->fly.nextflight;
		}
		else
		{
			unitsize = UNIT_RAF_WING;
			ac2ic = FindEscort(ac);
		}
	}
	else if(ac->nationality == NAT_LUF)
	{
		if((!ac->follower) || (ac->Follower()->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_LUF_SINGLE;
			ac2ic = NULL;
		}
		else if((!ac->fly.nextflight) || (ac->fly.nextflight->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_LUF_ROTTE;
			ac2ic = ac->Follower();
		}
		else if((!ac->fly.nextflight->fly.nextflight) || (ac->fly.nextflight->fly.nextflight->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_LUF_SCHWARME;
			ac2ic = ac->fly.nextflight;
		}
		else if((!ac->fly.nextflight->fly.nextflight->fly.nextflight) || (ac->fly.nextflight->fly.nextflight->fly.nextflight->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_LUF_SCHWARME;
			ac2ic = ac->fly.nextflight->fly.nextflight;
		}
		else if((!ac->fly.nextflight->fly.nextflight->fly.nextflight->fly.nextflight) || (ac->fly.nextflight->fly.nextflight->fly.nextflight->fly.nextflight->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_LUF_SCHWARME;
			ac2ic = ac->fly.nextflight->fly.nextflight->fly.nextflight;
		}
		else if((!FindEscort(ac)) || (FindEscort(ac)->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_LUF_STAFFEL;
			ac2ic = ac->fly.nextflight->fly.nextflight->fly.nextflight->fly.nextflight;
		}
		else if((!0))//FindNextGruppen(ac)) && FindNextGruppen(ac)->ai.manoeuvre == MANOEUVRE_FORCETOPCOVER))
		{
			unitsize = UNIT_LUF_GRUPPEN;
			ac2ic = FindEscort(ac);
		}
	}
	else
		unitsize = UNIT_VOID;

	return(ac2ic);
}


////////////////////////////////////////////////////////////////////////
//
// Function:    GetLeadAc
// Date:        03/03/2000
// Author:      Craig Beeston
//
// Description: Returns the lead aircraft in the group.  If the ac is a SAG
//				returns the lead real aircraft if expanded or the SAG if not
//
////////////////////////////////////////////////////////////////////////
AirStrucPtr AirStruc::GetLeadAc()											//CSB 03/03/00
{
	if(fly.numinsag)
		if(fly.expandedsag)
			return(fly.expandedsag);
		else
			return(this);
	else
		if(formpos)
			return(FindFormpos0());
		else
			return(this);
}


////////////////////////////////////////////////////////////////////////
//
// Function:    IsUnexpandedSAG
// Date:        21/03/2000
// Author:      Craig Beeston
//
// Description:
//
////////////////////////////////////////////////////////////////////////
bool AirStruc::IsUnexpandedSAG()
{
	if((fly.numinsag) && (!fly.expandedsag))
		return(true);
	else
		return(false);
}


////////////////////////////////////////////////////////////////////////
//
// Function:    FindTailie
// Date:        13/03/00
// Author:      Craig Beeston
//
// Description: Returns the last aircraft in the group.
//
//
////////////////////////////////////////////////////////////////////////
AirStrucPtr ArtInt::FindTailie(AirStrucPtr ac)
{
	AirStrucPtr tail = ac->FindFormpos0();
	while(tail->fly.nextflight)	tail = tail->fly.nextflight;
	while(tail->follower)		tail = tail->Follower();

	return(tail);
}


////////////////////////////////////////////////////////////////////////
//
// Function:    CanAttackType
// Date:        Tue 21 Mar 2000
// Author:      Craig Beeston
//
// Description: Decides whether an aircraft can attack a target aircraft depending on its type
//
//
////////////////////////////////////////////////////////////////////////
bool ArtInt::CanAttackType(AirStrucPtr ac, AirStrucPtr trg)
{
	return(true);

}


////////////////////////////////////////////////////////////////////////
//
// Function:    IsOutNumberedBy
// Date:        31/07/00
// Author:      Craig Beeston
//
// Description:
//
////////////////////////////////////////////////////////////////////////
bool AirStruc::IsOutNumberedBy(FP factor)
{
	AirStrucPtr pos0 = FindFormpos0();

	int num_us = Art_Int.CountSquadronSize(this);
	int numattackers = 0;

	for(AirStrucPtr ac = AirStrucPtr(ACList); ac; ac = AirStrucPtr(ac->nextmobile))
		if((ac->nationality != nationality) && ((ac->movecode == AUTO_COMBAT) || (ac->movecode == AUTO_PRECOMBAT)))
			if((ac->ai.unfriendly) && (ac->ai.unfriendly->Status.size == AIRSTRUCSIZE))
				if(AirStrucPtr(ac->ai.unfriendly)->FindFormpos0() == pos0)
					numattackers++;

	if((!pos0->fly.numinsag) && (pos0->fly.expandedsag))
		pos0 = pos0->fly.expandedsag;

	for(AirStrucPtr sag = AirStrucPtr(SAGList); sag; sag = AirStrucPtr(sag->nextmobile))
		if((sag->nationality != nationality) && ((sag->movecode == AUTOSAG_COMBAT) || (sag->movecode == AUTOSAG_PRECOMBAT)))
			if((sag->ai.unfriendly) && (sag->ai.unfriendly->Status.size == AIRSTRUCSIZE))
				if(AirStrucPtr(sag->ai.unfriendly) == pos0)
					numattackers += Art_Int.CountSquadronSize(sag);

	if(FP(numattackers) >= FP(num_us) * factor)
		return(true);
	else
		return(false);
}


////////////////////////////////////////////////////////////////////////
//
// Function:    SetUnfriendly
// Date:        01/08/00
// Author:      Craig Beeston
//
// Description:
//
////////////////////////////////////////////////////////////////////////
void AirStruc::SetUnfriendly(AirStrucPtr newtrg)
{
	if((newtrg) && ((newtrg->Status.deadtime && !newtrg->uniqueID.commsmove) || (newtrg->movecode == AUTO_NOPPILOT) || (newtrg->movecode == AUTOSAG_DESTROYED)))
		INT3;

	DoAllSAGChecks();

	if((ai.unfriendly) && (ai.unfriendly->Status.size == AIRSTRUCSIZE) && (AirStrucPtr(ai.unfriendly)->ai.attacker == this))
		AirStrucPtr(ai.unfriendly)->ai.attacker = NULL;

	ai.unfriendly = newtrg;

	if((newtrg) && (newtrg->Status.size == AIRSTRUCSIZE) && (newtrg->ai.attacker == NULL))
		if((newtrg->fly.numinsag) && (!fly.numinsag) && (fly.expandedsag))
			fly.expandedsag->SetUnfriendly(newtrg);
		else
			newtrg->ai.attacker = this;

	DoAllSAGChecks();
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ShouldGoHome
//Author		Craig Beeston
//Date			Fri 25 Aug 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::ShouldGoHome()
{
	{
		if((nationality == NAT_RAF) && (World.X - World.Z > METRES80KM))
			return(TRUE);
	}

	{
		if(FuelShort(10 * 60 * 100))
			return(TRUE);
	}

	if((!fly.numinsag) && (classtype->visible != DEF))
	{
		int totalammo = 0;
		for(UWord index = 0; index < 6; index++)
		{
			SLong xpos, ypos, zpos;	ULong mvel;
			UWord mdelay, mburst;;
			WeapAnimData* weapon = SHAPE.GetWeaponLauncher(this, index, xpos, ypos, zpos, mvel, mdelay, mburst, LT_BULLET);
			if((weapon)	&& (weapon->LoadedStores > 0))
				totalammo += weapon->LoadedStores;
		}
		if(totalammo <= 0)
			return(TRUE);
	}

	if(AircraftDamaged())
		return(TRUE);

	return(FALSE);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GroupShouldGoHome
//Author		Craig Beeston
//Date			Mon 29 Aug 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::GroupShouldGoHome(FP fract)
{
	int numgohome = 0;
	int numinsquad = 0;
	for(AirStrucPtr ld = FindFormpos0(); ld; ld = ld->fly.nextflight)
		for(AirStrucPtr ac = ld; ac; ac = ac->Follower())
			if(ac != Persons2::PlayerGhostAC)
			{
				numinsquad++;
				if(ac->ShouldGoHome())
					numgohome++;
			}

	if(FP(numgohome) >= fract * FP(numinsquad))
		return(TRUE);
	else
		return(FALSE);
}


