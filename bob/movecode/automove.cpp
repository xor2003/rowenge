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

////////////////////////////////////////////////////////////////////// //DAW 04/06/99
//
// Module:      Automove.cpp
//
// Created:     22/03/99 by DAW
//
// Description: 			  TowerMsg
//
//////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
//Filename       automove.cpp
//System
//Author         R. Hyde
//Date           Tue 28 Nov 1995
//Description
//------------------------------------------------------------------------------
//THe aoa calc is only good for camels###

#ifndef	NDEBUG
#endif

#define F_GRAFIX												//RDH 24Sep96

#include	"dosdefs.h"
#include	"worldinc.h"

#include	"planetyp.h"
#include	"myangles.h"
#include	"enumbits.m"
#include	"flymodel.h"
//MATHABLE	(ANGLES);
BITABLE		(Bool);
//DELTABLE	(ANGLES,int);
#include	"mymath.h"
#include	"miles.h"
#include	"missman2.h"										//JIM 22Aug96
#include	"ranges.h"											//RDH 28Feb96
#include	"speed.h"											//RDH 28Feb96
#include	"mytime.h"											//PD 15Mar96
#include	"3dcom.h"											//PD 19Apr96
#include	"landscap.h"									//PD 15Mar96
#include	"viewsel.h"											//RDH 25Jun96
#include	"ai.h"
#include	"flyinit.h"
#include	"savegame.h"
#include	"collided.h"										//RDH 12Sep96
#include	"transite.h"										//RDH 12Sep96
#include	"contlist.h"										//RDH 12Sep96
#include	"persons2.h"										//RDH 12Sep96
#include	"winmove.h"											//ARM 06Dec96
#include	"monotxt.h"											//ARM 08Dec96
#include	"modvec.h"
#include	"model.h"											//RJS 24Feb98
#include	"globrefs.h"
#include	"areatype.h"										//RJS 24Aug98
#include	"3dcode.h"											//RJS 24Aug98
#include	"SHAPES.H"
#include	"movement.h"
#include	"replay.h"
#include	"node.h"
#include	"overlay.h"
#include	"rchatter.h"
#include	"../mfc/resource.h" //RERUN: path
#include	"airstruc.h"
#include	"animmove.h"										//RJS 11Jan00
#include	"fastmath.h"												//RJS 30Jun00
#include	"impact.h"
#include	"grid.h"

//extern	ULong	GR_Quit3DNow;
extern DPlay _DPlay;

#define TRACEMOVE

UWord AirStruc::airdensityratiosquareroot[] =
			{
				65535,
				64383,
				63357,
				62280,
				61387,
				60377,
				59282,
				58236,
				57172,
				56233,
				55203,
				54230,
				53239,
				52230,
				51200,
				50231,
				49243,
				48149,
				46943,
				45705,
				44525,
				43218,
				42162,
				41080
	};


static RequiredBankDataElement 	RequiredBankData[] =
			{
				{ANGLES_0Deg, ANGLES_0Deg},
				{ANGLES_ThirdDeg, ANGLES_2_5Deg},
				{ANGLES_1Deg, ANGLES_20Deg},
				{ANGLES_5Deg, ANGLES_35Deg},
				{ANGLES_30Deg, MAXROLL},
				{ANGLES_180BareDeg, MAXROLL}
			};

UWord AirStruc::vel_to_aoa[] =

			{
				ANGLES_40Deg,
				ANGLES_34Deg,
				ANGLES_25Deg,
				ANGLES_18Deg,
				ANGLES_10Deg,
				ANGLES_6Deg,
				ANGLES_4Deg,
				ANGLES_3Deg,
				ANGLES_2_5Deg,
				ANGLES_2Deg,
				ANGLES_1_67Deg,
				ANGLES_1_33Deg,
				ANGLES_1Deg,
				ANGLES_0Deg,
				ANGLES_0Deg,
				ANGLES_0Deg,
				ANGLES_0Deg,
				ANGLES_0Deg,
				ANGLES_0Deg

			};
SLong const 	MAXALTITUDE_CM 	=	2200000;
SLong const 	WORD_FRACT   	=	65536;


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		MoveWithLeader
//Author		R. Hyde
//Date			Tue 27 Feb 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
COORDS3D* AirStruc::PositionWRTLeader (AirStrucPtr lf)

{
 	SLong	x,y,z;


	if (formpos==0)
	{
		assert	(fly.leadflight && "Moving main leader wrt self!");
		GetEscort_xyz (formation);									  //AMM 18/01/00
	}
	else
	if (formpos&InWingMAX)
	{
		GetFollower_xyz ( formation, lf);
	}
	else
	if (formpos&InFormMAX)
	{
		GetFlightLeader_xyz (formation,lf);							  //AMM 18/01/00
	}
 									//JIM 02Aug96
	return (&despos);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PositionTakeOffWRTLeader
//Author		R. Hyde
//Date			Mon 17 Jun 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
COORDS3D* AirStruc::PositionTakeOffWRTLeader (AirStrucPtr leader)
{
 	SLong	x,y,z;


	if (formpos==0)
	{
		assert	(fly.leadflight && "Moving main leader wrt leader!");
		despos = fly.leadflight->World;
		despos.X +=1000;	//10m
	}
	else
	if (formpos&InWingMAX)
	{
		if(leader == NULL)
		{
			leader=*this->leader;
			if (!leader)
				leader=AirStrucPtr(*this)->fly.leadelt();
		}
		int formationpos = (formpos&InWingMAX) - 1;
		if (leader->follower==this && formationpos>0)
		{
			formationpos=formationpos/2+TakeOff_FormationWing.formationtype.numentries-1;	//pilot3=formatoionpos2 ==>4
		}
		Formation_xyz(leader->hdg,&TakeOff_FormationWing.formationtype,formationpos);
	}
	else
	if (formpos&InFormMAX)
	{
		if (leader==NULL)
			leader=fly.leadflight;
		Formation_xyz(leader->hdg,&TakeOff_FormationSquad.formationtype,(formpos&InFormMAX)-1);
	}
	despos += leader->World; 									//JIM 02Aug96
	return (&despos);

}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CopyLeaderSub
//Author		R. Hyde
//Date			Mon 17 Jun 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool	AirStruc::CopyLeaderSub (AirStrucPtr lf)									//RDH 11Jul96
{

	SWord deltahdg = SWord(lf->hdg) - SWord(hdg);
	if((deltahdg > ANGLES_5Deg) || (-deltahdg > ANGLES_5Deg))
	{
		information = IF_OUT_POS;
		return(TRUE);
	}
	SWord deltapitch = SWord(lf->pitch) - SWord(pitch);
	if((deltapitch > ANGLES_5Deg) || (-deltapitch > ANGLES_5Deg))
	{
		information = IF_OUT_POS;
		return(TRUE);
	}

	assert(lf&&"No leader in copy");
	pitch 	= lf->pitch;
	hdg 	= lf->hdg;
	roll 	= lf->roll;
	fly.cpitch 	= lf->fly.cpitch;
//	fly.aoa 	= lf->fly.aoa;

	vel_x =lf->vel_x;
	vel_y =lf->vel_y;
	vel_z =lf->vel_z;
	vel_ 	=lf->vel_;

	if((this != Persons2::PlayerSeenAC) && (this != Persons2::PlayerGhostAC))
	{
		fly.rudder = lf->fly.rudder;
		fly.aileron = lf->fly.aileron;
		fly.elevator = lf->fly.elevator;
	}

	if (roll==ANGLES_0Deg)										//RDH 11Jul96
		return TRUE;											//RDH 11Jul96
	else														//RDH 11Jul96
		return FALSE;											//RDH 11Jul96
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CopyLeader
//Author		R. Hyde
//Date			Tue 27 Feb 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::CopyLeader (AirStrucPtr lf)
{
	if (CopyLeaderSub (lf))		//if leader horizontal			//RDH 11Jul96
	{
		NewPosition();			//then even faster!				//RDH 11Jul96
		if(leader)
		{
			fly.pModel->FrameCount--;
			World.Y -= CalcWobbleDY();
			fly.pModel->FrameCount++;
		}
	}
	else														//RDH 11Jul96
	{															//RDH 11Jul96
		COORDS3D desworld = *PositionWRTLeader (lf);
	 	FP dist2 = Distance3DSquared(&desworld);
		if(dist2 > FP(1000) * FP(1000))
		{
			information = IF_OUT_POS;
			NewPosition();
		}
		else
			World = desworld;										//RDH 01Jun99
	}															//RDH 11Jul96

	if(leader)					  //Only wingmen will wobble.
		World.Y += CalcWobbleDY();	  //Flight leaders are always be placed correctly


	SetFlightParams ();
	uniqueID.changed = TRUE;									//JIM 12Jul96
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcWobbleDY
//Author		Craig Beeston
//Date			Tue 14 Dec 1999
//
//Description	Calculates the vertical wobble diplacement for aircraft in formation
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
SWord AirStruc::CalcWobbleDY()
{
	if(this == Persons2::PlayerGhostAC)
		return(0);

	SWord time =  1024 * uniqueID.count;						//AMM 01Feb100
	time       += 16 * Timer_Code.FRAMETIME * fly.pModel->FrameCount ;
	time       =  Math_Lib.K8sin128(time);
	return(time);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcWobbleVY
//Author		Craig Beeston
//Date			Tue 14 Dec 1999
//
//Description	Calculates the vertical wobble velocity for aircraft in formation
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
SWord AirStruc::CalcWobbleVY()
{
	if(this == Persons2::PlayerGhostAC)
		return(0);

	SWord time =  1024 * uniqueID.count;						//AMM 01Feb100
	time       += 16 * Timer_Code.FRAMETIME * fly.pModel->FrameCount;
	time	   = Math_Lib.K8cos(time)>>1;
	return(time);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoToGrndPitchRoll
//Author		Robert Slater
//Date			Thu 3 Oct 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	AirStruc::AutoToGrndPitchRoll(ANGLES grndroll,ANGLES grndpitch)
{
	if (((UWord) fly.cpitch) > ANGLES_180Deg)
		fly.cpitch = grndpitch;

	if (((UWord) pitch) > ANGLES_180Deg)
	{
		pitch = grndpitch;
	}
	else
	{
		if (((UWord) pitch) > (classtype->deckangle + grndpitch))
			pitch = classtype->deckangle+ grndpitch;
		else
			pitch = grndpitch;
	}

	if ((roll- grndroll) > 0)
		roll -= (Angles)Timer_Code.FRAMETIME;
	else
		roll += (Angles)Timer_Code.FRAMETIME;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		void
//Author		R. Hyde
//Date			Tue 10 Sep 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoDeathSequence()
{
	SLong	groundheight = Trans_Obj.GetGroundLevel(*this);		//RJS 14Apr99
																//RJS 14Apr99
	if (World.Y <= groundheight)								//RJS 14Apr99
	{															//RJS 14Apr99
		World.Y = groundheight;									//RJS 14Apr99
		Trans_Obj.LandedEffect(*this,_Collide.NOSE_TOUCH);		//RJS 14Apr99
	}															//RJS 14Apr99
	else
	{
		mobileitem* mob = MobileItemPtr(this);
		Trans_Obj.MobileMoveDeadAC(mob);
	}
	SetFlightParams ();
	UpdateDeathTime(DEADTIMEOUT_NORM,true);				//RJS 10Oct00

	if (this == Persons2::PlayerSeenAC)									//RJS 30Aug00
	{
		EngineInfo	left_engine;

		left_engine.rpm_starter = 0;
		left_engine.rattle = 0;
		left_engine.cockpitnobble = 0;
		left_engine.rpm_engine = fly.rpm;
		left_engine.velocity = vel_;

 		OverLay.SetInfoLineVals(SLong(float(vel_) * 0.001), 0, SLong(float(World.Y) * (3.2808 / 100.)), hdg / 182, fly.thrustpercent); 	//RJS 7Sep00

		_Miles.PlayEngine(this,left_engine,left_engine);	//RJS 13Jan00
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoDeathGlide
//Author		Craig Beeston
//Date			Tue 15 Aug 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoDeathGlide()
{
	SLong groundheight = Trans_Obj.GetGroundLevel(*this);

	PitchIntercept = ANGLES_350Deg;
	AutoCalcPitch();

	SWord reqroll = 0;
	if(ai.homebase)
	{
		InterceptandRange(ai.homebase);
		SWord reqdeltahdg = hdg - HdgIntercept;
 		reqroll = FindRequiredRoll(reqdeltahdg, RequiredBankData);
		SimpleMoveToRoll(reqroll);
	}

	if(World.Y <= groundheight)
	{
		World.Y = groundheight;
		if(!fly.numinsag)
			Trans_Obj.LandedEffect(*this,_Collide.NOSE_TOUCH);
		else
		{
			Trans_Obj.LaunchBigExplosion(this, *currworld, shape);
			DestroySag();
		}

		return;
	}

	hdg += CalcHdg();
	CalcVelAlt();
	CalcXYZVel();
	NewPosition();
	SetFlightParams ();
	if(!fly.numinsag)
		UpdateDeathTime(DEADTIMEOUT_NORM);								//RJS 10Oct00
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoResurrect
//Author		Andrew McRae
//Date			Thu 5 Dec 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoResurrect (AirStrucPtr ac)
{
	Bool Owner;
	ULong n;
	ULong MaxHeight;

	if (ac == Manual_Pilot.ControlledAC2)
	{
		if (_DPlay.FirstResurrectFrame)									//AMM 13Jul00
		{
// make sure ac is above ground before starting to resurrect

			if (ac->fly.pModel->GroundHeight>ac->World.Y)
				ac->World.Y=ac->fly.pModel->GroundHeight+FT_10;

// this aircraft belongs to this machine

			_DPlay.RepairAircraft (ac);
			_DPlay.AllowSuicide=FALSE;

// set aircraft orientation

			Manual_Pilot.controlmode=ManualPilot::PILOTDEAD;

			Manual_Pilot.ControlledAC2->roll = ANGLES_0Deg;
			Manual_Pilot.ControlledAC2->pitch = ANGLES_0Deg;
//		Manual_Pilot.ControlledAC2->hdg = ac->hdg;
//		Manual_Pilot.ControlledAC2->vel = MPH100;
//		Manual_Pilot.ControlledAC2->vel = MPH200;


// set vel to 0 so that it resurrect spiral, prediction is not so far off...

			Manual_Pilot.ControlledAC2->vel_=0;


// save sensitivity setting

//		UWord OldSensitivity;

//		OldSensitivity = Manual_Pilot.GetSensitivity();
//		Manual_Pilot.SetSensitivity (OldSensitivity);
			_DPlay.FirstResurrectFrame=false;							//AMM 13Jul00
		}

// move the aircraft
// a bit faster for jets!
// wow, back to prop AC for WW2. but quicker than WW1

		ac->World.Y+=FT_10;

//		ac->World.Y += FT_5;
		ac->hdg += ANGLES_4Deg;

// needed to update flight model which is still being processed even though its ignored at this stage

		Persons2::PlayerSeenAC->fly.pModel->UpdateModel(Persons2::PlayerSeenAC);	//AMM 8Aug00

// calc max height

		MaxHeight = 0;
		for (n=0; n<MAXPLAYERS; n++)
		{
			if (_DPlay.ActivePlayers&(1<<n))
			{
				AirStrucPtr ac2;
				ac2 = (AirStrucPtr)MobileItem::currworld->ConvertPtrUID ((UniqueID)_DPlay.AllocPacket [n]);
				if (ac2)
				{
					if (ac != ac2)
					{
						if (ac2->World.Y > MaxHeight)
							MaxHeight = ac2->World.Y;
					}
				}
			}
		}

// need MaxHeight a bit bigger than for biplanes!

		if (MaxHeight == 0)	// just in case no aircraft found
			MaxHeight=FT_10000;

		MaxHeight += FT_2000;

		if (MaxHeight > FT_10000)
			MaxHeight = FT_10000;


 		if (Key_Tests.KeyPress3d(RESURRECTKEY) || (ac->World.Y >= MaxHeight))
		{
// set velocity now that ac is alive and in game

			if (ac->fly.pModel->EngineList)
			{
				if (ac->fly.pModel->EngineList->Type==ET_JET)
				{
// jets go a bit quicker than biplanes then?

					Manual_Pilot.ControlledAC2->vel_ = (MPH400+MPH100);
				}
				else
				{
// mustangs a bit slower than jet?

					Manual_Pilot.ControlledAC2->vel_ = (MPH200+MPH100);
				}
			}
			else
			{
// what are you, a balloon or something????

				Manual_Pilot.ControlledAC2->vel_=0;
			}

			_DPlay.FirstResurrectFrame=true;

			ac->Status.deaded=FALSE;									//AMM 13Jul00
//			Manual_Pilot.ControlledAC2->vel = MPH100;
			Persons2::PlayerSeenAC->fly.pModel->UpdateModel(Persons2::PlayerSeenAC);	//AMM 8Aug00
			ac->movecode = AUTO_FOLLOWWP;	//0;					//AMM 10Dec96
			ac->information = IF_OUT_POS;					//CSB 22Aug00
			Manual_Pilot.controlmode=ManualPilot::MANUAL;	// PILOTDEAD
			for(int i = 0; i < 4; i++)							//AMM 08Aug00
				ac->fly.fuel_content[i] = ac->classtype->fueltankcap[i]*1000;	//AMM 5Sep00

// enable its death - not yet
//			for (n=0; n<MAXPLAYERS; n++)
//			{
//				if (_DPlay.AllocPacket [n] == (ac->uniqueID.count))
//					ac->Status.deaded=FALSE;
//			}

// Send message to tell other machines this aircraft is back online

			_DPlay.RepairAircraft(ac);							//AMM 12Jul99
			_DPlay.ResetCommsSurfaces();// reset gear etc now so that it is up when I resurrect

// set gear to not broken
			if (ac->fly.pModel)
			{
				ac->fly.pModel->EmergencyGearUsed = FALSE;
				PAERODEVICE pAeroDevice;
				for(pAeroDevice = ac->fly.pModel->DeviceList; pAeroDevice; pAeroDevice = pAeroDevice->List.NextItem())
				{
					{
						pAeroDevice->Broken = FALSE;
					}
				}
			}

			_DPlay.NewSpecial(DPlay::PIDC_RESURRECTEND);			//AMM 01Sep98
			Persons2::PlayerSeenAC->ai.radiosilent=FALSE;
			Persons2::PlayerGhostAC->ai.radiosilent=FALSE;
			_Miles.ResetSoundFlags();
			_DPlay.viewtime=true;
			_DPlay.AllowSuicide=TRUE;
			_DPlay.SuicideTime=0;
			_DPlay.resurrectend=true;
			Trans_Obj.View_Object->SetToPiloted((itemptr)Persons2::PlayerSeenAC,false);//AMM 12Jul99
		}
	}
	else
	{
// this aircraft belongs to another player

		_DPlay.RepairAircraft (ac);
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoFollowWpWing
//Author		R. Hyde					Craig Beeston
//Date			//RDH 27Feb96			23 Apr 1999
//
//Description
//
//Inputs
//
//Returns		//This routine has trouble with the inside aircraft when a big formation is turning.
//					His target position moves backwards faster thatn he can decellerate
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoFollowWpWing (AirStrucPtr MyLeader)
{
	const int CanSideSlip = 1;
	const int InFront     = 2;
	const int Beside      = 3;
	const int Behind      = 4;
	char  RelPos = 0;

	AirStrucPtr lf;
	if(MyLeader)
		lf = MyLeader;
	else														//RDH 21May99
		lf = FindLeadCandidate();

	SWord MaxRoll = ANGLES_60Deg;
	SWord MaxPitch = ANGLES_15Deg;
	SWord leadroll = SWord(lf->roll);		if(leadroll < 0)	leadroll *= -1;
	SWord leadpitch = SWord(lf->pitch);		if(leadpitch < 0)	leadpitch *= -1;

	if((!formpos) && (Distance3DSquared(&lf->World) > FP(15000) * FP(15000)))
		MaxRoll = ANGLES_15Deg;

	if(	((leadroll > MaxRoll) || (leadpitch > MaxPitch) || (lf->vel_ < lf->classtype->minvel))
		&& ((!currworld->vp) || (!currworld->vp->Accel()))	)
		information = IF_OUT_POS;

	if(World.Y < fly.pModel->GroundHeight + 5000)
		information = IF_OUT_POS;

	if(!(fly.pModel->FrameCount & 0xff))
		information = IF_OUT_POS;

	if(information == IF_IN_POS)
	{	//If we are slowed by a follower, then slow our leader
		//This approach leads to a chain of slowdowns and no false slowdowns:
		//wing->element->flight->group->escortee
		if (slowdownleader)											  //DAW 12/05/99
		{
			Bool DontSlowDown = FALSE;
			if(	   ((lf->classtype->visible == JU88) || (lf->classtype->visible == HE11) || (lf->classtype->visible == DO17))
				&& ((classtype->visible != JU88) && (classtype->visible != HE11) && (classtype->visible != DO17)) )
				DontSlowDown = TRUE;

			if(!DontSlowDown)
				lf->slowdownleader=TRUE;								  //DAW 12/05/99 //DAW 12/05/99
		}

		slowdownleader=FALSE;
		CopyLeader (lf);
	}
	else
	{
		if(!TimeLeft())
		{
			Bool GotIntoFormation = FALSE;
			Bool DontSlowDown = FALSE;
			if((lf->classtype->aerobaticfactor == AEROBATIC_LOW) && (classtype->aerobaticfactor != AEROBATIC_LOW))
				DontSlowDown = TRUE;


			despos = *PositionWRTLeader(lf);
#ifndef NDEBUG
if(despos.Y > lf->World.Y + 100000)
	INT3;
#endif
			if((leader) && (Distance3DSquared(&despos) < FP(1000) * FP(1000)))
				despos.Y += CalcWobbleDY();

			despos.X -= (lf->vel_x * Timer_Code.FRAMETIME) / 10000;
			despos.Y -= (lf->vel_y * Timer_Code.FRAMETIME) / 10000;
			despos.Z -= (lf->vel_z * Timer_Code.FRAMETIME) / 10000;

			if((World.Y < HIGHESTGROUND) && (World.Y < fly.pModel->GroundHeight + 5000))//(lf->vel_ < lf->classtype->minvel))
				despos.Y += 10000;

			COORDS3D GlobDelta, LocalDelta;
			GlobDelta.X = despos.X - World.X;
			GlobDelta.Y = despos.Y - World.Y;
			GlobDelta.Z = despos.Z - World.Z;
			RotateToHdg(GlobDelta, LocalDelta, lf->hdg);

			COORDS3D AbsD;
			AbsD.X = LocalDelta.X;	if(AbsD.X < 0) AbsD.X *= -1;
			AbsD.Y = LocalDelta.Y;	if(AbsD.Y < 0) AbsD.Y *= -1;
			AbsD.Z = LocalDelta.Z;	if(AbsD.Z < 0) AbsD.Z *= -1;

			FP RangeSq = Distance3DSquared(&despos);
			if(RangeSq > FP(METRES100 * METRES100))
				Status.Drawn=FALSE;

			if(RangeSq < FP(METRES50) * FP(METRES50))
				information = IF_NEAR_POS;
			else
				information = IF_OUT_POS;

			if(		(information >= IF_NEAR_POS)	//Can we go into sideslip
				&&	(vel_ > classtype->minvel)
				&&	((lf->hdg - hdg < ANGLES_5Deg) && (hdg - lf->hdg < ANGLES_5Deg))
				&&	((lf->roll <= ANGLES_60Deg) && (-lf->roll <= ANGLES_60Deg)) )
				RelPos = CanSideSlip;
			else
			{
				if(AbsD.X > AbsD.Z)			RelPos = Beside;
				else
				{
					if(LocalDelta.Z > 0)	RelPos = Behind;
					else					RelPos = InFront;
				}
			}

			if(RelPos == CanSideSlip)
			{
				Range = Distance3D(&despos);
				CalcVelWing(lf, LocalDelta.Z);
				GotIntoFormation = SideSlip (lf);

				if(	(!currworld->vp) ||	(currworld->vp->Accel()) )						//CSB 10/05/99
					GotIntoFormation = TRUE;											//CSB 10/05/99
			}
			else
			{
				Bool level = TRUE;
				FP leadtime = 0;

				COORDS3D GlobVel;
				COORDS3D LocalVel;
				GlobVel.X = vel_x;
				GlobVel.Y = vel_y;
				GlobVel.Z = vel_z;
				RotateToHdg(GlobVel, LocalVel, lf->hdg);

				switch(RelPos)
				{
					case Behind:
					{
						if(LocalVel.Z > lf->vel_)
							leadtime = (FP(LocalDelta.Z) * 20000.0) / FP(LocalVel.Z - lf->vel_);
						else leadtime = 60 * SECS100;
						level &= CalcVelWing(lf, LocalDelta.Z);
						break;
					}

					case Beside:
					{
						if(FP(LocalDelta.X) * FP(LocalVel.X) > 0)
						{
							leadtime = (FP(LocalDelta.X) * 20000.0) / LocalVel.X;
							FP maxtime = FP(LocalDelta.X) / 10.0;
							if(maxtime < 0) maxtime *= -1;
							if(leadtime > maxtime)
								leadtime = maxtime;
						}
						level &= CalcVelWing(lf, LocalDelta.Z);

						DontSlowDown = TRUE;
						break;
					}

					case InFront:
					{
						if(LocalDelta.Z < -50000)	//more than 500 metres infront	-> Turn back
							level &= CalcVelWing(lf, 100000);
						else
						{
							level &= CalcVelWing(lf, LocalDelta.Z);
							if(LocalVel.Z < lf->vel_)
								leadtime = (FP(LocalDelta.Z) * 20000.0) / FP(LocalVel.Z - lf->vel_);
							else
								leadtime = 20*SECS100;
						}

						DontSlowDown = TRUE;
						break;
					}
					break;

				}

				COORDS3D AimPos = despos;					//Position ahead of the target aircraft
				if(lf->vel_ < lf->classtype->landingspeed)
					AimPos.Y += FT_1000;

				int mintime = 256;
				if(classtype->aerobaticfactor == AEROBATIC_LOW)
					mintime = 1024;
				if(leadtime < mintime)
					leadtime = mintime;

				if(leadtime > 0)
				{
					MODMAXMIN(leadtime, 0, 30*SECS100);

					AimPos.X += (leadtime * FP(lf->vel_x)) / 10000.0;
					AimPos.Y += (leadtime * FP(lf->vel_y)) / 10000.0;
					if(AimPos.Y < fly.pModel->GroundHeight)
						AimPos.Y = (AimPos.Y + fly.pModel->GroundHeight) / 2;
					AimPos.Z += (leadtime * FP(lf->vel_z)) / 10000.0;
				}
				InterceptandRange(&AimPos);
				level &= AutoCalcPitch();
				SWord desroll = FindRequiredRoll(hdg - HdgIntercept, RequiredBankData);
				SimpleMoveToRoll(desroll);
				hdg += CalcHdg();

				CalcXYZVel ();

				if((level) && (SWord(roll) == 0) && (hdg == HdgIntercept))
				{
					if(RangeSq > FP(KM10) * FP(KM10))
						SetManoeuvreTime(THIRTYSECS100);
					else if(RangeSq > FP(KM1) * FP(KM1))
						SetManoeuvreTime(FIVESECS100);
				}
			}
																					//CSB 10/05/99
			if(GotIntoFormation)													//CSB 10/05/99
			{																		//CSB 10/05/99
				information = IF_IN_POS;
				World=despos;		//position correctly							//CSB 10/05/99
			}

			if(!DontSlowDown)
				lf->slowdownleader = TRUE;								  //DAW 12/05/99

			slowdownleader=FALSE;
		}
		NewPosition ();
		SetFlightParams ();
	}
}


//Movement.H
//enum	PhaseFAC	{
//	FAC_WAIT=0,			FAC_WAITRECONNECT,			FAC_WELCOME_LONGRANGE,		FAC_WINGWAGGLE,
//		FAC_WAITCLOSER,	FAC_WELCOME_CLOSE,			FAC_WAITTOUCHING,
//	FAC_TARGETAREA=16,	FAC_TARGETAREARECONNECT,	FAC_NEXTTARGASBOMBERCLIMBS,	FAC_DESCRIBETARGETS,
//		FAC_FLYTOAREA,	FAC_HOMEFORCES,				FAC_TARGETBMBCLOSER,		FAC_TARGETTOOCLOSE,
//		FAC_WATCHROCKETS,
//	FAC_ROCKETEER=32,	FAC_ROCKETEERRECONNECT,		FAC_GOINGIN,				FAC_SEEMYSMOKE,
//		FAC_CLIMBTOOBSERVE,	FAC_ADVISEATTACK,		FAC_OBSERVING,				FAC_MISSTIMEOUT,
//		FAC_ENOUGHHITS,	FAC_HIT,					FAC_DAMAGE,					FAC_HITFRIENDS,
//	FAC_EXTRAS=48,		FAC_REQUEST_LONGRANGE,		FAC_GOINGHOME,				FAC_MORETARGETS,
//		FAC_NOMORETARGETS,
//	FAC_CAS_SPECIAL=64,80,96,112
//
//
//
//	FAC_STAGEMASK=-16,FAC_SUBSTAGEMASK=15
//					};
//////////////////////////////////////////////////////////////////////
//
// Function:    AutoCAS
// Date:		30/03/99
// Author:		JIM
//
//Description:
//
//////////////////////////////////////////////////////////////////////
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FlyEscortTo
//Author		Jim
//Date			Apr 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
AirStrucPtr	AirStruc::FlyEscortTo()
{
///	if (	(fly.leadflight || formpos) &&	//don't remember why...


	AirStrucPtr candidate=NULL;
	candidate = FindLeadCandidate();

	if((candidate) && (Distance3DSquared(&candidate->World) < FP(5000000) * FP(5000000)))
		if (candidate->waypoint==waypoint)
		{
			if (movecode==AUTO_TELLLEADER)
				return candidate;
			if ((candidate->movecode&-2)==AUTOSAG_TRACKEXPFOLLOW)
				if (candidate->fly.expandedsag->movecode==movecode)
					return candidate->fly.expandedsag;
			if (movecode==candidate->movecode)
				return candidate;
		}
	return NULL;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindLeadCandidate
//Author		Craig Beeston
//Date			20 Apr 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
AirStrucPtr	AirStruc::FindLeadCandidate()
{
///	if (	(fly.leadflight || formpos) &&	//don't remember why...
	AirStrucPtr candidate = NULL;
	if(leader)
		candidate = Leader();
	else
	if(position() == 2)
		candidate = fly.leadelt();
	else if(fly.leadflight)
	{
		candidate = fly.leadflight->GetLeadAc();
		if(formpos == 0)
		{	//flying as escorter. Get range to escortee and waypoint.
			//If waypoint within 30 miles or escortee within 50 miles fly to escortee.
			if(Distance3DSquared(FindDesPos()) > FP(MILES30 + desposrange) * FP(MILES30 + desposrange))
				if(Distance3DSquared(&candidate->World) > FP(MILES30) * FP(MILES30))
					candidate = NULL;
		}
	}

	return(candidate);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindFormpos0
//Author		Craig Beeston
//Date			Thu 20 Jan 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
AirStrucPtr	AirStruc::FindFormpos0()
{
	if((fly.numinsag) && (fly.expandedsag))
		return(fly.expandedsag);

 	AirStrucPtr ac = this;

	if((ac->formpos) && (ac->leader))
		ac = ac->Leader();

	if((ac->formpos) && (ac->fly.leadflight))
		ac = ac->fly.leadflight;

	if((ac->formpos) && (!ac->Status.deadtime))
		INT3;
	return(ac);
}


//////////////////////////////////////////////////////////////////////
//
// Function:    AutoFollowWp
// Date:		18/04/99
// Author:		JIM
//
//Description:
//
//////////////////////////////////////////////////////////////////////
void MoveAirStruc::AutoFollowWp ()

{

	if((classtype->visible == ME110) && (!fly.numinsag) && (waypoint->wpname == WPNAME_PatrolS))
		int donothingforabreakpoint0 = 0;

	SWord flightpos = position();
	if (!waypoint)
	{
		_Error.EmitSysErr("Should never run out of waypoints while in autofollowwp");
		ExecuteWayPointAction ();
	}


	AirStrucPtr MyLeader;
	if((MyLeader = FlyEscortTo())!=NULL)
	{
		AutoFollowWpWing (MyLeader);
	}
	else
	{
		bool nth_frame = !TimeLeft();	//True when timer has runout


		if((information != IF_ALLGOOD) || (nth_frame))
		{
			Coords3D* dp;
			dp=InterceptandRangeFindDesPosAndVel();//InterceptandRange (dp=FindDesPos ()); //JIM 17/01/00


			if((!(information & IF_NOCLIMB)) || (nth_frame))
			{
				SWord LocalBestClimbPitch = CalcBestClimbPitch();	//CSB 28/09/98

				if (PitchIntercept > (Angles)LocalBestClimbPitch)	//CSB 28/09/98
			 		PitchIntercept = (Angles)LocalBestClimbPitch;		//RDH 26Jul96

				if (Range > MILES01)
				{
					if((SWord(PitchIntercept) > 0) && (SWord(PitchIntercept) < LocalBestClimbPitch))//CSB 28/09/98
					{
						ULong MinVel = CalcBestClimbSpeed();
						if(slowdownleader) MinVel -= 100000;
						if(vel_ >= MinVel)
							PitchIntercept = (Angles)LocalBestClimbPitch;	//RDH 26Jul96
					}
				}
				else if(((hdg-HdgIntercept)-ANGLES_90Deg)<<ANGLES_180Deg)
					PitchIntercept = ANGLES_0Deg;
				//travelling away from WP and within 1 mile
				//level out and hopefully get a despos push

				if(AutoCalcPitch())
					information = InFormation(UWord(information) | UWord(IF_NOCLIMB));
				else
				{
					information = InFormation(UWord(information) & (UWord(IF_ALLGOOD) - UWord(IF_NOCLIMB)));//Turn off NOCLIMB bit
					information = InFormation(UWord(information) & (UWord(IF_ALLGOOD) - UWord(IF_NOACCEL)));//Turn off NOACCEL bit
				}

			}


			if((!(information & IF_NOTURN)) || (nth_frame))
			{
				//travelling away from WP and within 1 mile
				//level out and hopefully get a despos push
				if((Range < MILES01) && (((hdg-HdgIntercept)-ANGLES_90Deg)<<ANGLES_180Deg))
					HdgIntercept = hdg;

				if(CalcHdgRollLimited())
					information = InFormation(UWord(information) | UWord(IF_NOTURN));
				else
					information = InFormation(UWord(information) & (UWord(IF_ALLGOOD) - UWord(IF_NOTURN)));//Turn off NOTURN bit

				SWord MaxAng = ANGLES_30Deg;
				if(((formation & FORMTYPE_WING) == FTW_SCWARM) || ((formation & FORMTYPE_WING) == FTW_BSCRWARM))
					MaxAng = ANGLES_60Deg;
				else
				{
					if(Range < 1000000)
						MaxAng = ANGLES_45Deg;
					if(Range < 600000)
						MaxAng = ANGLES_60Deg;
				}
				if(Range < 300000)
					MaxAng = ANGLES_75_5Deg;

				if(SWord(roll) > MaxAng)
					roll = Angles(MaxAng);
				if(SWord(roll) < -MaxAng)
					roll = Angles(-MaxAng);

			}


			if((!(information & IF_NOACCEL)) || (nth_frame))
			{
				if(CalcVelAlt())
					information = InFormation(UWord(information) | UWord(IF_NOACCEL));
				else
					information = InFormation(UWord(information) & (UWord(IF_ALLGOOD) - UWord(IF_NOACCEL)));//Turn off NOACCEL bit

			}


			if(nth_frame)
			{
				if((waypoint) && (!formpos) && (fly.leadflight) && (fly.leadflight->waypoint == waypoint->NextWP(this)))	//CSB 18Aug00
				{
					if(information)
						desposrange+=METRES10KM;
					else
						desposrange+=MILES01; //escortee gone through WP so allow increased range!
				}

				{
					Range -= dp->Y-World.Y;						//JIM 05Jul96
					if (ReachedDesPos ())								//JIM 03Jul96
					{
						ExecuteWayPointAction ();						//JIM 03Jul96
					}

				}

				if(Range > METRES5000)
					SetManoeuvreTime(10*SECS100);
				else
				{
					if(Range > METRES500)
						SetManoeuvreTime(SECS100);
					else
						SetManoeuvreTime(1);
				}
			}

			CalcXYZVel();
		}

		if(!(fly.pModel->FrameCount & 0x7ff))
			ai.DesEta() = 0;

		FP messagedist;
		FP olddist = 0.0;
		if(Range < METRES2500 + desposrange)
		{
			messagedist = vel_ * (1500.0 / 10000.0) + desposrange;
			olddist = Distance3DSquared(&despos);
			if(olddist < messagedist)
				olddist = 0.0;
		}

		NewPosition ();
		SetFlightParams ();

		if(		(olddist != 0.0)
			&&	(olddist > messagedist * messagedist)
			&&	(Distance3DSquared(&despos) < messagedist * messagedist)	)
			if(		(waypoint->NextWP(this))
				&&	((waypoint->action == wpacnoactionno) || (waypoint->action == wpstartloopno) || (waypoint->action == wpstoploopchkno))	)
			{
				AirStrucPtr callee = PlayerInGroup();
				if(!callee)
					callee = FindBuddy();
				if(callee)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_WARNNEWWAYPOINT, MSG_WARN_HEADING, *this, waypoint->NextWP(this), callee));
			}

	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCrossOverTurn
//Author		Craig Beeston
//Date			14 Dec 1999
//
//Description	Performs a cross over turn for a flight of 4 aircraft onto a new heading
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrossOverTurn()
{
	switch(ai.ManStep)
	{
		case PHASE0:
		{
			int flightpos = position();

			if(flightpos == 0)
			{
				manoeuvretime = 0;
				InterceptandRange(FindDesPos ());
				SWord dhdg = HdgIntercept - hdg;
				Float sdhdg, cdhdg;
				Math_Lib.high_sin_cos(Angles(dhdg), sdhdg, cdhdg);
				ai.desiredhdg = HdgIntercept;

				AirStrucPtr ac;

				for(ac = Follower(); ac; ac = ac->Follower())
				{
					SWord formationpos = (ac->formpos&InWingMAX) - 1;
					FP size = ((ANGLES_FRACT/5)+(ANGLES_FRACT*4*52/5)/SHAPESTUFF.GetShapePtr(ac->shape)->Size);
					size *= (1.0 / 32768.0);

					FormationTypeIndex oldformation = formation%FTI_SCWARM_SWAPPING_FLAG;

					ANGLES brg  = ANGLES_0Deg + Aircraft_Formations[oldformation]->wingpos [formationpos].bearing;
					SLong range = Aircraft_Formations[oldformation]->wingpos [formationpos].range;
					Float sin, cos;
					Math_Lib.high_sin_cos(brg, sin, cos);
					SLong dx = FP(range) * sin * size;
					SLong dz = FP(range) * cos * size;
					Float dt = (FP(dx) * (cdhdg + 1.0) / sdhdg + FP(dz)) * 10000.0 / FP(ac->vel_);
					if(dhdg < 0) dt *= -1;
					ac->manoeuvretime = ac->leader->manoeuvretime + dt;
				}

				SWord mintime = 0;
				for(ac = this; ac; ac = ac->Follower())
					if(SWord(ac->manoeuvretime) < mintime)
						mintime = SWord(ac->manoeuvretime);

 				for(ac = this; ac; ac = ac->Follower())
				{
					ac->manoeuvretime -= mintime;
					ac->ai.ManStep = PHASE1;
					if(ac->information)
						ac->information = IF_NEAR_POS;
				}
			}
			break;
		}

		case PHASE1:
		{
			PitchIntercept = ANGLES_0Deg;
			AutoCalcPitch();
			SimpleMoveToRoll(ANGLES_0Deg);
			hdg += Angles(CalcHdg() );
			CalcVelAlt();
			CalcXYZVel();
			if(!TimeLeft())
				ai.ManStep = PHASE2;
			break;
		}

		case PHASE2:
		{

			if(formpos)
				HdgIntercept = fly.leadflight->ai.desiredhdg;
			else
				HdgIntercept = ai.desiredhdg;

			SWord dHdg, dPitch;
			SLong dVel;
			WindTrackCorrection(vel_, HdgIntercept, PitchIntercept, dVel, dHdg, dPitch);
			HdgIntercept   += dHdg;
			PitchIntercept += dPitch;

			AutoCalcPitch();
			if(SWord(HdgIntercept - hdg) > 0)
				SimpleMoveToRoll(13653);	//ANGLES_75Deg);
			else
				SimpleMoveToRoll(-13653);	//ANGLES_285Deg);

			hdg += Angles(CalcHdg());
			CalcVelAlt();
			CalcXYZVel();

			SWord dhdg = HdgIntercept - hdg;
			if((dhdg < ANGLES_10Deg) && (-dhdg < ANGLES_10Deg))
				ai.ManStep = PHASE3;
			break;
		}

		case PHASE3:
		{
			if(!position())
			{
				if((formation & FORMTYPE_WING) == FTW_SCWARM)
					formation = ((formation % FORMTYPE_WING) | FTW_BSCRWARM) % FTI_SCWARM_SWAPPING_FLAG;
				else
					formation = ((formation % FORMTYPE_WING) | FTW_SCWARM) % FTI_SCWARM_SWAPPING_FLAG;
			}
			else
			{
				AirStrucPtr lead = Leader();
				if(!lead)
					lead = fly.leadflight;

				if(!(lead->formation & FTI_SCWARM_SWAPPING_FLAG))
					formation = lead->formation;

				PitchIntercept = ANGLES_0Deg;
				AutoCalcPitch();
				SimpleMoveToRoll(ANGLES_0Deg);
				hdg += Angles(CalcHdg() );
				CalcVelAlt();
				CalcXYZVel();
			}
			break;
		}
	}

	NewPosition ();
	SetFlightParams ();
}


//////////////////////////////////////////////////////////////////////
//
// Function:    MoveMissionDiceThrow
// Date:		08/06/99
// Author:		Jim
//
//Description:
//
//////////////////////////////////////////////////////////////////////
void	 MoveAirStruc::MoveMissionDiceThrow()
{
	movecode=AUTO_FOLLOWWP;
	information = IF_OUT_POS;					//CSB 22Aug00
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCommsAccel
//Author		Andy McMaster
//Date			Fri 11 Jun 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoCommsAccel()
{
// simulate AUTOACCEL_FWD

	Coords3D	*dpflyto;

	Float sinHdg,cosHdg;
	Range=METRES5000;
	Math_Lib.high_sin_cos(hdg,sinHdg,cosHdg);
	dpflyto=&despos;
	dpflyto->X=World.X + sinHdg * Range;
	dpflyto->Z=World.Z + cosHdg * Range;
	dpflyto->Y=World.Y;

	InterceptandRange (dpflyto);

	CalcXYZVel();
	NewPosition ();
}


//////////////////////////////////////////////////////////////////////
//
// Function:    AutoAccelerate
// Date:		03/10/00
// Author:		Craig Beeston
//
// Description:
//
//////////////////////////////////////////////////////////////////////
void MoveAirStruc::AutoAccelerate()
{
	assert(ULong(this)==ULong(Manual_Pilot.ControlledAC2)&&"AutoAccelerate is for the piloted ac only");

	switch(Manual_Pilot.controlmode)
	{
		case ManualPilot::AUTOACCEL_FWD:
		{
			SimpleMoveToRoll(ANGLES_0Deg);
			if(World.Y > Land_Scape.GetGroundLevel(World) + FT_1000)
				PitchIntercept = ANGLES_0Deg;
			else
				PitchIntercept = Angles(CalcBestClimbPitch());
			AutoCalcPitch();
			CalcVelAlt();
			CalcXYZVel();
			NewPosition();
			SetFlightParams();
			break;
		}

		case ManualPilot::AUTOACCEL_MSG:
		{
			if(OverLay.message_target)
			{
				InterceptandRange(&OverLay.message_target->World);
				SWord bestpitch = CalcBestClimbPitch();
				if(SWord(PitchIntercept) > bestpitch)
					PitchIntercept = Angles(bestpitch);
				AutoCalcPitch();
				SLong desroll = 5 * (HdgIntercept - hdg);
				MODLIMIT(desroll, ANGLES_45Deg);
				SimpleMoveToRoll(desroll);
				CalcHdg();
				CalcVelAlt();
				CalcXYZVel();
				NewPosition();
				SetFlightParams();
				if(Range < 100000)
					OverLay.CancelAccel();
			}
			else
			{
				NewPosition();
				SetFlightParams();
				OverLay.CancelAccel();
			}
			break;
		}

		case ManualPilot::AUTOACCEL_WAYPT:
		case ManualPilot::AUTOACCEL_MISS:
		case ManualPilot::AUTOACCEL_HOME:
		{
			if((vel_ < classtype->minvel) || (World.Y < fly.pModel->GroundHeight + FT_100))
			{
				if(vel_ < classtype->minvel)
					PitchIntercept = ANGLES_0Deg;
				else
					PitchIntercept = Angles(CalcBestClimbPitch());

				AutoCalcPitch();
				SimpleMoveToRoll(ANGLES_0Deg);
				CalcVelAlt();
				CalcXYZVel();
				NewPosition();
				SetFlightParams();
			}
			else
				AutoFollowWp();
			fly.pModel->Instruments(this);
			break;
		}
	}

	if((vel_ > classtype->minvel) || (World.Y < fly.pModel->GroundHeight + FT_100))
	{
		AircraftAnimData* adptr = (AircraftAnimData*)Anim;
		if((adptr->acleglowerl != 255) || (adptr->acleglowerr != 255))
			fly.pModel->GearUp(this);
	}

	if(movecode != AUTO_ACCELERATE)
		OverLay.CancelAccel();
}


//%%			if (level && Range>desposrange)
//%%			{
//%%				if (Range-desposrange>MILES10)		SetManoeuvreTime();
//%%				if (Range-desposrange>MILES05)		SetManoeuvreTime(THIRTYSECS100);
//%%				else if (Range-desposrange>MILES02)	SetManoeuvreTime(10*SECS100);
//%%				else								SetManoeuvreTime(FIVESECS100);
//%%			}
//%%	//		else
//%%		}
//%%	}
//%%	NewPosition ();
//%%	SetFlightParams ();
//%%}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Procedure
//Author		R. Hyde
//Date			Thu 12 Sep 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrashSkid()
{
	switch(ai.ManStep)
	{
		case 0:
		{
			fly.thrustpercent = 0;
			fly.rpm  = 0;
			fly.rpm1 = 0;
			AircraftAnimData* adptr = (AircraftAnimData*)Anim;
			adptr->acrpm  = 0;
			adptr->acrpm1 = 0;
			SHAPE.ForceDamage(*this, *this, &adptr->FRONT, BitStates(254));
			ai.ManStep = 1;
		}

		case 1:
		{
			vel_ -= 2500;
			SWord desroll = ANGLES_5Deg;
			if(SWord(roll) < 0)
				desroll = -ANGLES_5Deg;
			if(desroll > SWord(roll))
				roll += 1;
			else
				roll -= 1;
			if(SWord(pitch) > 0)
				pitch -= 1;
			else
				pitch += 1;
			hdg += roll / 1000;
			CalcXYZVel();
			NewDeadPosition();
			UByte texture;
			World.Y = _Collide.GroundAltitude(World, &texture) + 100;
			if(vel_ <= 0)
			{
				if(IsWater(texture))
				{
					ai.ManStep = 2;
					Trans_Obj.LaunchBubbles(*this, 1000, *currworld);
					SetManoeuvreTime(1000);
				}
				else
					ai.ManStep = 3;
			}
			break;
		}

		case 2:
		{
			World.Y -= 10;
			if(!TimeLeft())
				ai.ManStep = 3;
			break;
		}

		case 3:
		{
			SHAPE.DetatchAllVapourStreams(*this, 0);
			movecode = AUTO_NOPPILOT;
		}
	}

	UpdateDeathTime(DEADTIMEOUT_NORM);
}




//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCrashRoll
//Author		R. Hyde
//Date			Thu 12 Sep 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrashRoll()
{

	SLong	groundlevel;
	ANGLES	grndroll;
	ANGLES	grndpitch;
	Bool	onWater = FALSE;
	AreaType	theArea;
	UByte	slowshift;
	ANGLES	anglestep = (Angles)(Timer_Code.FRAMETIME * ANGLES_2Deg);//RJS 26Aug98
	AircraftAnimData	*adptr = (AircraftAnimData*) Anim;
	Bool	rolldone, pitchdone;


	_Collide.HaveWeLanded(*this,grndpitch,grndroll,TRUE);		//RJS 24Aug98
	groundlevel = fly.pModel->GroundHeight;

	slowshift = 4;
	theArea = (AreaType)Three_Dee.GetLandType();
//jon	if ((theArea == AT_sea) || (theArea == AT_river) || (theArea == AT_washShore))
	if ( IsWater(theArea) )
	{
		onWater = TRUE;
		slowshift += 6;
	}

	SWord	tmproll, tmppitch;
	SLong	crashtime = fly.pModel->CrashTime;

	World.Y = groundlevel;

	fly.rpm /= 2;														//RJS 10Jul00
	fly.rpm1 /= 2;

	if (this == Persons2::PlayerSeenAC)									//RJS 30Aug00
	{
		EngineInfo	left_engine;

		left_engine.rpm_starter = 0;
		left_engine.rattle = 0;
		left_engine.cockpitnobble = 0;
		left_engine.rpm_engine = fly.rpm;
		left_engine.velocity = vel_;

		_Miles.PlayEngine(this,left_engine,left_engine);	//RJS 13Jan00
	}

	if (vel_ > 0)
	{
		ANGLES	oldhdg = hdg;

		vel_ -= (Timer_Code.FRAMETIME << slowshift) * 1000;						//RJS 24Aug98

		anglestep += ((anglestep * vel_) / 500000);

		if (adptr->RIGHTWINGIN == BS_DEAD)
		{
			hdg += ANGLES_45Deg;
			roll += anglestep;

			tmproll = roll;
			if (tmproll < -ANGLES_90Deg)
				SHAPE.ForceDamage(*this,*this,&adptr->LEFTWINGOUT,BS_DEAD);
		}
		else
		{
			if (adptr->LEFTWINGIN == BS_DEAD)
			{
				hdg -= ANGLES_45Deg;
				roll -= anglestep;

				tmproll = roll;
				if (tmproll > ANGLES_90Deg)
					SHAPE.ForceDamage(*this,*this,&adptr->RIGHTWINGOUT,BS_DEAD);
			}
		}

		CalcXYZVel();
 		NewDeadPosition ();

		hdg = oldhdg;
	}
	else
	{
		SWord	maxangle = ANGLES_170Deg;
		bool	tipleft = false;

		if (Trans_Obj.View_Object && (this==Manual_Pilot.ControlledAC2))//RDH 02Sep98
			Trans_Obj.View_Object->SetToDeathView(DEATH_NORMAL);//RDH 02Sep98

		if (this == Manual_Pilot.ControlledAC2)					//RJS 08Sep98
			_Miles.StopEngine();								//RJS 08Sep98

		SHAPE.DetatchAllVapourStreams(*this,0);
		adptr->hasdust = FALSE;

		tmproll = roll;
		tmppitch = pitch;

		//Feeble gravity effect...
		//The bigger the angle, the slower it tips...

		if ((adptr->RIGHTWINGIN == BS_DEAD) && (tmproll < ANGLES_45Deg))
		{
			maxangle = ANGLES_0Deg;
			tipleft = true;
		}
		else
		{
			if ((adptr->LEFTWINGIN == BS_DEAD) && (tmproll > -ANGLES_45Deg))
				maxangle = ANGLES_0Deg;
			else
				tipleft = true;
		}

		if (tipleft == false)
		{
			if (tmproll >= maxangle)
				rolldone = TRUE;
			else
			{
				anglestep = (Angles) (ANGLES_1Deg * crashtime);

				roll+=anglestep;
				rolldone = FALSE;
			}
		}
		else
		{
			if (tmproll <= -maxangle)
				rolldone = TRUE;
			else
			{
				anglestep = (Angles) (ANGLES_1Deg * crashtime);

				roll-=anglestep;
				rolldone = FALSE;
			}
		}

		if (tmppitch < ANGLES_170Deg)
		{
			anglestep = (Angles) (ANGLES_1Deg * crashtime);

			tmppitch += anglestep;
			pitchdone = FALSE;
		}
		else
			pitchdone = TRUE;

		if (rolldone && pitchdone)
		{
			Trans_Obj.LaunchDirtMedium(*this,*currworld);
			movecode = AUTO_NOPPILOT;

			if (this == Manual_Pilot.ControlledAC2)
				GR_Quit3DNow=CRASHED_ALIVE;					//RDH 22Sep96
		}

		UpdateDeathTime(DEADTIMEOUT_NORM);								//RJS 10Oct00
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCrashBelly
//Author		Craig Beeston
//Date			Mon 20 Dec 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrashBelly()
{

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCrashDitch
//Author		Craig Beeston
//Date			Mon 20 Dec 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrashDitch()
{

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCrashOneWheel
//Author		Craig Beeston
//Date			Mon 20 Dec 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrashOneWheel()
{
	COORDS3D AcPos = World;
	if(World.Y > 5000)
		AcPos.Y = World.Y - 5000;
	else AcPos.Y = 0;
	SLong GroundHeight = _Collide.GroundAltitude(AcPos);
	AircraftAnimData* adptr = (AircraftAnimData*)Anim;

	fly.rpm /= 2;														//RJS 10Jul00
	fly.rpm1 /= 2;											//RJS 10Jul00

	if (this == Persons2::PlayerSeenAC)									//RJS 30Aug00
	{
		EngineInfo	left_engine;

		left_engine.rpm_starter = 0;
		left_engine.rattle = 0;
		left_engine.cockpitnobble = 0;
		left_engine.rpm_engine = fly.rpm;
		left_engine.velocity = vel_;

		_Miles.PlayEngine(this,left_engine,left_engine);
	}

	UpdateDeathTime(DEADTIMEOUT_DELAYED);				//RJS 10Oct00

	switch(ai.ManStep)
	{
		case PHASE0:
		{
			if(vel_ > 0)
			AddAccel2Vel(-1000);
			Float sin, cos;
			Math_Lib.high_sin_cos(hdg, sin, cos);
			vel_x = vel_ * sin;
			vel_y = 0;
			vel_z = vel_ * cos;
			NewDeadPosition();

			if(SWord(pitch) > ANGLES_0Deg + classtype->deckangle)
				pitch -= (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
			if(SWord(pitch) < ANGLES_0Deg + classtype->deckangle)
				pitch += (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;

			bool leftup  = false;
			bool rightup = false;

			AircraftAnimData* adptr = (AircraftAnimData*)Anim;
			if(adptr->acleglowerl != 0)
				leftup = true;
			if(adptr->acleglowerr != 0)
				rightup = true;

			COORDS3D coords, offset, PushedOffset;
			if(leftup)
			{
				SHAPE.GetContactPoint((itemptr)this, CT_WHLRIGHT, coords, offset, PushedOffset);
				World.Y += GroundHeight - coords.Y;

				roll -= (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
				SHAPE.GetContactPoint((itemptr)this, CT_LEFT, coords, offset, PushedOffset);
				if(coords.Y < GroundHeight)
					ai.ManStep = PHASE1;
			}
			else
			{
				SHAPE.GetContactPoint((itemptr)this, CT_WHLLEFT, coords, offset, PushedOffset);
				World.Y += GroundHeight - coords.Y;

				roll += (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
				SHAPE.GetContactPoint((itemptr)this, CT_RIGHT, coords, offset, PushedOffset);
				if(coords.Y < GroundHeight)
					ai.ManStep = PHASE1;
			}

			SetFlightParams();
			break;
		}

		case PHASE1:
		{
			SetManoeuvreTime(500);
			ai.ManStep = PHASE2;
			ai.desiredhdg = hdg;
			ai.desiredrange = vel_;		//USING desiredrange TO STORE INITIAL VELOCITY
			adptr->hasdust = TRUE;

			if (vel_ > 50000)
				Trans_Obj.LaunchExplodePuff(this,500,DRTSML,450,*mobileitem::currworld);
		}

		case PHASE2:
		{
			if(SWord(pitch) > ANGLES_0Deg + classtype->deckangle)
				pitch -= (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
			if(SWord(pitch) < ANGLES_0Deg + classtype->deckangle)
				pitch += (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;

			AddAccel2Vel(-5000);

			bool leftup  = false;
			bool rightup = false;

			if(adptr->acleglowerl != 0)
				leftup = true;
			if(adptr->acleglowerr != 0)
				rightup = true;

			SWord velhdg;
			COORDS3D coords, offset, PushedOffset;
			if(leftup)
			{
				SHAPE.GetContactPoint((itemptr)this, CT_WHLRIGHT, coords, offset, PushedOffset);
				World.Y += GroundHeight - coords.Y;
				SHAPE.GetContactPoint((itemptr)this, CT_LEFT, coords, offset, PushedOffset);
				if(coords.Y > GroundHeight)
					roll -= (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
				else
				{
					roll += (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
					if(adptr->LEFTWHEEL < BS_DEAD)
						SHAPE.ForceDamage(*this, *this, &adptr->LEFTWHEEL, BS_DEAD);
					if(adptr->LEFTWINGOUT < BS_DAMLV2)
						SHAPE.ForceDamage(*this, *this, &adptr->LEFTWINGOUT, BS_DAMLV2);
				}

				hdg    = ai.desiredhdg - Angles((ANGLES_135Deg * (500 - manoeuvretime)) / 500);
				velhdg = ai.desiredhdg - Angles((ANGLES_45Deg  * (500 - manoeuvretime)) / 500);
			}
			else
			{
				SHAPE.GetContactPoint((itemptr)this, CT_WHLLEFT, coords, offset, PushedOffset);
				World.Y += GroundHeight - coords.Y;
				SHAPE.GetContactPoint((itemptr)this, CT_RIGHT, coords, offset, PushedOffset);
				if(coords.Y > GroundHeight)
					roll += (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
				else
				{
					if (Math_Lib.rnd() < 1024)								//RJS 26Sep00
						Trans_Obj.LaunchHitGrass(this,GroundHeight,*mobileitem::currworld);

					roll -= (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
					if(adptr->RIGHTWHEEL < BS_DEAD)
						SHAPE.ForceDamage(*this, *this, &adptr->RIGHTWHEEL, BS_DEAD);
					if(adptr->RIGHTWINGOUT < BS_DAMLV2)
						SHAPE.ForceDamage(*this, *this, &adptr->RIGHTWINGOUT, BS_DAMLV2);
				}

				hdg    = ai.desiredhdg + Angles((ANGLES_135Deg * (500 - manoeuvretime)) / 500);
				velhdg = ai.desiredhdg + Angles((ANGLES_45Deg  * (500 - manoeuvretime)) / 500);
			}

			vel_ = (ai.desiredrange * manoeuvretime) / 500;
			Float sin, cos;
			Math_Lib.high_sin_cos(Angles(velhdg), sin, cos);
			vel_x = vel_ * sin;
			vel_y = 0;
			vel_z = vel_ * cos;
			NewDeadPosition();

			if(!TimeLeft())
			{
				ai.ManStep = PHASE3;
				adptr->hasdust = FALSE;
			}

			SetFlightParams();
			break;
		}

		case PHASE3:
		{
			//Do Nothing
			vel_x = vel_y = vel_z = vel_ = 0;							//RJS 11Aug00
		}
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCrashNose
//Author		Craig Beeston
//Date			Mon 20 Dec 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrashNose()
{
	COORDS3D AcPos = World;
	if(World.Y > 5000)
		AcPos.Y = World.Y - 5000;
	else AcPos.Y = 0;
	SLong GroundHeight = _Collide.GroundAltitude(AcPos);

	fly.rpm /= 2;														//RJS 10Jul00
	fly.rpm1 /= 2;											//RJS 10Jul00

	if (this == Persons2::PlayerSeenAC)									//RJS 30Aug00
	{
		EngineInfo	left_engine;

		left_engine.rpm_starter = 0;
		left_engine.rattle = 0;
		left_engine.cockpitnobble = 0;
		left_engine.rpm_engine = fly.rpm;
		left_engine.velocity = vel_;

		_Miles.PlayEngine(this,left_engine,left_engine);	//RJS 13Jan00
	}

	UpdateDeathTime(DEADTIMEOUT_DELAYED);				//RJS 10Oct00

	switch(ai.ManStep)
	{
		case PHASE0:
		{
			Float sin, cos;
			Math_Lib.high_sin_cos(hdg, sin, cos);
			vel_x = vel_ * sin;
			vel_y = 0;
			vel_z = vel_ * cos;
			NewDeadPosition();

			pitch -= (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
			COORDS3D coords, offset, PushedOffset;
			SHAPE.GetContactPoint((itemptr)this, CT_WHLLEFT, coords, offset, PushedOffset);
			SLong lwheel = coords.Y;
			SHAPE.GetContactPoint((itemptr)this, CT_WHLRIGHT, coords, offset, PushedOffset);
			SLong rwheel = coords.Y;
			if(lwheel > rwheel)
			{
				World.Y += GroundHeight - rwheel;
				roll -= (ANGLES_30Deg * Timer_Code.FRAMETIME) / 100;
			}
			else
			{
				World.Y += GroundHeight - lwheel;
				roll += (ANGLES_30Deg * Timer_Code.FRAMETIME) / 100;
			}
			SHAPE.GetContactPoint((itemptr)this, CT_WHLFRONT, coords, offset, PushedOffset);	//Prop
			SLong prop = coords.Y;
			if(prop < GroundHeight)
			{
				ai.ManStep = PHASE1;
				SetManoeuvreTime(256);

				if (Math_Lib.rnd() < 1024)								//RJS 26Sep00
					Trans_Obj.LaunchHitGrass(this,GroundHeight,*mobileitem::currworld);
			}

			SetFlightParams();
			break;
		}

		case PHASE1:
		{
			Float sin, cos;
			Math_Lib.high_sin_cos(hdg, sin, cos);
			vel_x = vel_ * sin;
			vel_y = 0;
			vel_z = vel_ * cos;
			NewDeadPosition();

			if (vel_ > 50000)											//RJS 31May00
				Trans_Obj.LaunchExplodePuff(this,500,DRTSML,450,*mobileitem::currworld);

			COORDS3D coords, offset, PushedOffset;
			SHAPE.GetContactPoint((itemptr)this, CT_FRONT, coords, offset, PushedOffset);
			SLong prop = coords.Y;
			if(prop > GroundHeight)
				pitch -= (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
			SHAPE.GetContactPoint((itemptr)this, CT_WHLLEFT, coords, offset, PushedOffset);
			SLong lwheel = coords.Y;
			SHAPE.GetContactPoint((itemptr)this, CT_WHLRIGHT, coords, offset, PushedOffset);
			SLong rwheel = coords.Y;
			if(lwheel > rwheel)
			{
				World.Y += GroundHeight - rwheel;
				roll -= (ANGLES_30Deg * Timer_Code.FRAMETIME) / 100;
			}
			else
			{
				World.Y += GroundHeight - lwheel;
				roll += (ANGLES_30Deg * Timer_Code.FRAMETIME) / 100;
			}

			if(TimeLeft())
			{
				AircraftAnimData* adptr = (AircraftAnimData*)Anim;
				if(adptr->FRONT != BS_DAMLV2)
					SHAPE.ForceDamage(*this, *this, &adptr->FRONT, BS_DAMLV2);
				adptr->acrpm = 128 * manoeuvretime / 200;
				vel_ -= (vel_ * Timer_Code.FRAMETIME) / manoeuvretime;
				if (vel_ <= 0)											//RJS 25Sep00
					vel_x = vel_y = vel_z = vel_ = 0;						//RJS 25Sep00
			}
			else
			{
				AircraftAnimData* adptr = (AircraftAnimData*)Anim;
				adptr->acrpm = 0;
				adptr->acrpm1 = 0;
				ai.ManStep = PHASE2;
			}

			SetFlightParams();
			break;
		}

		case PHASE2:
		{
			//do nothing
			vel_x = vel_y = vel_z = vel_ = 0;							//RJS 11Aug00
		}
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCrashFlip
//Author		Craig Beeston
//Date			Mon 20 Dec 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrashFlip()
{
	COORDS3D AcPos = World;
	if(World.Y > 5000)
		AcPos.Y = World.Y - 5000;
	else AcPos.Y = 0;
	SLong GroundHeight = _Collide.GroundAltitude(AcPos);
	AircraftAnimData* adptr = (AircraftAnimData*)Anim;

	fly.rpm /= 2;														//RJS 10Jul00
	fly.rpm1 /= 2;

	UpdateDeathTime(DEADTIMEOUT_DELAYED);							//RJS 10Oct00

	if (this == Persons2::PlayerSeenAC)									//RJS 30Aug00
	{
		EngineInfo	left_engine;

		left_engine.rpm_starter = 0;
		left_engine.rattle = 0;
		left_engine.cockpitnobble = 0;
		left_engine.rpm_engine = fly.rpm;
		left_engine.velocity = vel_;

		_Miles.PlayEngine(this,left_engine,left_engine);	//RJS 13Jan00
	}

	if (vel_ > 50000)
		Trans_Obj.LaunchExplodePuff(this,500,DRTSML,450,*mobileitem::currworld);

	switch(ai.ManStep)
	{
		case PHASE0:
		{
			FP mantime = 128.0 * (1.0 + FP(SWord(pitch)) / FP(ANGLES_90Deg));
			SetManoeuvreTime(UWord(mantime));
			ai.ManStep = PHASE1;
			adptr->acrpm = 0;
			adptr->acrpm1 = 0;
			adptr->hasdust = TRUE;
			SHAPE.ForceDamage(*this, *this, &adptr->FRONT, BS_DAMLV2);
		}

		case PHASE1:
		{
			Float sin, cos;
			Math_Lib.high_sin_cos(hdg, sin, cos);
			vel_x = vel_ * sin;
			vel_y = 0;
			vel_z = vel_ * cos;
			NewDeadPosition();

			SLong deltapitch = (ANGLES_90Deg * Timer_Code.FRAMETIME) / 128;
			pitch -= deltapitch;
			COORDS3D coords, offset, PushedOffset;
			SHAPE.GetContactPoint((itemptr)this, CT_WHLLEFT, coords, offset, PushedOffset);
			SLong lowest = coords.Y;
			SHAPE.GetContactPoint((itemptr)this, CT_WHLRIGHT, coords, offset, PushedOffset);
			if(coords.Y < lowest)
				lowest = coords.Y;
			SHAPE.GetContactPoint((itemptr)this, CT_FRONT, coords, offset, PushedOffset);
			if(coords.Y < lowest)
				lowest = coords.Y;

			World.Y += GroundHeight - lowest;

			if(vel_ > 0)
				AddAccel2Vel(-10000);
			else
				vel_x = vel_y = vel_z = vel_ = 0;						//RJS 25Sep00

			if(!TimeLeft())
			{
				ai.ManStep = PHASE2;
				SetManoeuvreTime(255);
				hdg += ANGLES_180Deg;
				roll += ANGLES_180Deg;
			}

			SetFlightParams();
			break;
		}

		case PHASE2:
		{
			Float sin, cos;
			Math_Lib.high_sin_cos(hdg, sin, cos);
			vel_x = vel_ * -sin;
			vel_y = 0;
			vel_z = vel_ * -cos;
			NewDeadPosition();

			COORDS3D coords, offset, PushedOffset;
			SHAPE.GetContactPoint((itemptr)this, CT_LEFT, coords, offset, PushedOffset);
			SLong lwing = coords.Y;
			SLong lowest = lwing;
			SHAPE.GetContactPoint((itemptr)this, CT_RIGHT, coords, offset, PushedOffset);
			SLong rwing = coords.Y;
			if(rwing < lowest)	lowest = rwing;
			SHAPE.GetContactPoint((itemptr)this, CT_FRONT, coords, offset, PushedOffset);
			SLong nose = coords.Y;
			if(nose < lowest)	lowest = nose;
			SHAPE.GetContactPoint((itemptr)this, CT_BACK, coords, offset, PushedOffset);
			SLong back = coords.Y;
			if(back < lowest)	lowest = back;

			World.Y += GroundHeight - lowest;

			SLong deltapitch = (ANGLES_45Deg * Timer_Code.FRAMETIME) / 100;
			if(nose < back)
				pitch += deltapitch;
			else
				pitch -= deltapitch;

			if(lwing < rwing)
				roll -= (ANGLES_45Deg * Timer_Code.FRAMETIME) / 100;
			else
				roll += (ANGLES_45Deg * Timer_Code.FRAMETIME) / 100;

			if(vel_ > 0)
			{
				if(lowest == lwing)
					hdg += (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;
				if(lowest == rwing)
					hdg -= (ANGLES_10Deg * Timer_Code.FRAMETIME) / 100;

				AddAccel2Vel(-10000);
			}
			else
			{
				vel_x = vel_y = vel_z = vel_ = 0;						//RJS 25Sep00
				if(!TimeLeft())
				{
					ai.ManStep = PHASE3;
					adptr->hasdust = FALSE;
				}
			}

			SetFlightParams();
			break;
		}

		case PHASE3:
		{
			//Do Nothing
			vel_x = vel_y = vel_z = vel_ = 0;							//RJS 11Aug00
		}

	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCrashTumble
//Author		Craig Beeston
//Date			Mon 20 Dec 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoCrashTumble()
{
	COORDS3D AcPos = World;
	if(World.Y > 5000)
		AcPos.Y = World.Y - 5000;
	else AcPos.Y = 0;
	UByte onwater;
	SLong GroundHeight = _Collide.GroundAltitude(AcPos, &onwater);
	if((onwater == AT_sea) || (onwater == AT_stream))
		onwater = 1;
	else
		onwater = 0;

	AircraftAnimData* adptr = (AircraftAnimData*)Anim;

	fly.rpm /= 2;														//RJS 10Jul00
	fly.rpm1 /= 2;

	if (this == Persons2::PlayerSeenAC)									//RJS 30Aug00
	{
		EngineInfo	left_engine;

		left_engine.rpm_starter = 0;
		left_engine.rattle = 0;
		left_engine.cockpitnobble = 0;
		left_engine.rpm_engine = fly.rpm;
		left_engine.velocity = vel_;

		_Miles.PlayEngine(this,left_engine,left_engine);	//RJS 13Jan00
	}

	UpdateDeathTime(DEADTIMEOUT_DELAYED);								//RJS 10Oct00

	switch(ai.ManStep)
	{
		case PHASE0:
		{
			SHAPE.ForceDamage(*this, *this, &adptr->FRONT, BS_DAMLV2);
			vel_y = 0;
			adptr->hasdust = TRUE;
			ai.ManStep = PHASE1;
			ai.desiredhdg = hdg;

			if((vel_ > 50000) && (!onwater))
				Trans_Obj.LaunchExplodePuff(this,500,DRTSML,450,*mobileitem::currworld);
		}

		case PHASE1:
		{
			COORDS3D coords, offset, PushedOffset, lowest;
			SHAPE.GetContactPoint((itemptr)this, CT_FRONT, coords, offset, PushedOffset);
			COORDS3D nose = coords;
			SHAPE.GetContactPoint((itemptr)this, CT_BACK, coords, offset, PushedOffset);
			COORDS3D tail = coords;
			SHAPE.GetContactPoint((itemptr)this, CT_LEFT, coords, offset, PushedOffset);
			COORDS3D lwing = coords;
			SHAPE.GetContactPoint((itemptr)this, CT_RIGHT, coords, offset, PushedOffset);
			COORDS3D rwing = coords;
			SHAPE.GetContactPoint((itemptr)this, CT_BELLY, coords, offset, PushedOffset);
			COORDS3D belly = coords;

			if(adptr->LEFTWINGOUT == BS_DEAD)
			{
				lwing.X = (lwing.X + belly.X) / 2;
				lwing.Y = (lwing.Y + belly.Y) / 2;
				lwing.Z = (lwing.Z + belly.Z) / 2;
			}
			if(adptr->RIGHTWINGOUT == BS_DEAD)
			{
				rwing.X = (rwing.X + belly.X) / 2;
				rwing.Y = (rwing.Y + belly.Y) / 2;
				rwing.Z = (rwing.Z + belly.Z) / 2;
			}

			lowest = nose;
			if(tail.Y < lowest.Y)	lowest = tail;
			if(lwing.Y < lowest.Y)	lowest = lwing;
			if(rwing.Y < lowest.Y)	lowest = rwing;
			if(belly.Y < lowest.Y)	lowest = belly;

			if(lowest.Y < GroundHeight)
			{
				if (Math_Lib.rnd() < 1024)								//RJS 26Sep00
					Trans_Obj.LaunchHitGrass(this,GroundHeight,*mobileitem::currworld);

				if (vel_ > 50000)
					Trans_Obj.LaunchExplodePuff(this,500,DRTSML,450,*mobileitem::currworld);	//RJS 31May00

				World.Y += GroundHeight - lowest.Y;
				vel_y = (10000 / 2) * (GroundHeight - lowest.Y) / Timer_Code.FRAMETIME;
				if(vel_y > 100000)
					vel_y = 100000;

				if(lowest.X == lwing.X)
				{
					if((_Analogue.FF_airframe) && (this == Persons2::PlayerSeenAC))
						_Analogue.FFstiff.SetZ(30000,  30000);

					if(adptr->LEFTWINGOUT != BS_DEAD)
					{
							SHAPE.ForceDamage(*this, *this, &adptr->LEFTWINGOUT, BS_DEAD);
					}
					else if(adptr->LEFTWINGIN != BS_DEAD)
						SHAPE.ForceDamage(*this, *this, &adptr->LEFTWINGIN, BS_DEAD);
					if(adptr->LEFTWHEEL != BS_DEAD)
						SHAPE.ForceDamage(*this, *this, &adptr->LEFTWHEEL, BS_DEAD);
				}
				else if(lowest.X == rwing.X)
				{
					if((_Analogue.FF_airframe) && (this == Persons2::PlayerSeenAC))
						_Analogue.FFstiff.SetZ(30000, -30000);

					if(adptr->RIGHTWINGOUT != BS_DEAD)
					{
							SHAPE.ForceDamage(*this, *this, &adptr->RIGHTWINGOUT, BS_DEAD);
					}
					else if(adptr->RIGHTWINGIN != BS_DEAD)
						SHAPE.ForceDamage(*this, *this, &adptr->RIGHTWINGIN, BS_DEAD);
					if(adptr->LEFTWHEEL != BS_DEAD)
						SHAPE.ForceDamage(*this, *this, &adptr->LEFTWHEEL, BS_DEAD);
				}
				else if(lowest.X == nose.X)
				{
					if((_Analogue.FF_airframe) && (this == Persons2::PlayerSeenAC))
						_Analogue.FFstiff.SetX(30000,  30000);

					if(adptr->FRONT != BS_DAMLV2)
						SHAPE.ForceDamage(*this, *this, &adptr->FRONT, BS_DAMLV2);
				}
				else if(lowest.X == tail.X)
				{
					if((_Analogue.FF_airframe) && (this == Persons2::PlayerSeenAC))
						_Analogue.FFstiff.SetX(30000, -30000);

					if(adptr->TAIL != BS_DEAD)
						SHAPE.ForceDamage(*this, *this, &adptr->TAIL, BS_DEAD);
				}
			}
			else
			{
				vel_y -= 980.6 * Timer_Code.FRAMETIME;
				World.Y += vel_y * Timer_Code.FRAMETIME / 10000;
			}

			FP oldvel = vel_;
			AddAccel2Vel(-FP(vel_) * FP(vel_) * 4e-7);

			vel_x = FP(vel_x) * FP(vel_) / oldvel;
			vel_z = FP(vel_z) * FP(vel_) / oldvel;
			World.X += vel_x * Timer_Code.FRAMETIME / 10000;
			World.Z += vel_z * Timer_Code.FRAMETIME / 10000;
			uniqueID.changed = TRUE;

			if(vel_ < classtype->landingspeed / 8)
			{
				ai.ManStep = PHASE2;
				SetManoeuvreTime(500);
			}

			FP deltaang = vel_ * 0.0025 * Timer_Code.FRAMETIME;
			SWord dhdg = SWord(hdg) - SWord(ai.desiredhdg);
			Float sin, cos;
			Math_Lib.high_sin_cos(Angles(dhdg), sin, cos);
			roll -= deltaang * sin;
			pitch -= deltaang * cos;

			if(SWord(pitch) > ANGLES_90Deg)
			{
				pitch =  Angles(32767 - SWord(pitch));
				hdg   += ANGLES_180Deg;
				roll  += ANGLES_180Deg;
			}

			if(-SWord(pitch) > ANGLES_90Deg)
			{
				pitch =  Angles(-32767 - SWord(pitch));
				hdg   += ANGLES_180Deg;
				roll  += ANGLES_180Deg;
			}

			SetFlightParams();
			break;
		}

		case PHASE2:
		{
			COORDS3D coords, offset, PushedOffset, lowest;
			SWord sin, cos;
			FP temp =  ANGLES_60Deg * Timer_Code.FRAMETIME;
			temp    /= 100.0 * 32768.0;

			if(AbsAngle(SWord(roll)) > ANGLES_90Deg)
			{
				SHAPE.GetContactPoint((itemptr)this, CT_FRONT, coords, offset, PushedOffset);
				COORDS3D nose = coords;
				SHAPE.GetContactPoint((itemptr)this, CT_BACK, coords, offset, PushedOffset);
				COORDS3D tail = coords;
				SHAPE.GetContactPoint((itemptr)this, CT_LEFT, coords, offset, PushedOffset);
				COORDS3D lwing = coords;
				SHAPE.GetContactPoint((itemptr)this, CT_RIGHT, coords, offset, PushedOffset);
				COORDS3D rwing = coords;

				COORDS3D belly={0};	//RERUN: silences uninit-struct analysis (only read where GetContactPoint ran)
				if((adptr->LEFTWINGOUT == BS_DEAD) || (adptr->RIGHTWINGOUT == BS_DEAD))
					SHAPE.GetContactPoint((itemptr)this, CT_BELLY, belly, offset, PushedOffset);
				if(adptr->LEFTWINGOUT == BS_DEAD)
				{
					lwing.X = (lwing.X + belly.X) / 2;
					lwing.Y = (lwing.Y + belly.Y) / 2;
					lwing.Z = (lwing.Z + belly.Z) / 2;
				}
				if(adptr->RIGHTWINGOUT == BS_DEAD)
				{
					rwing.X = (rwing.X + belly.X) / 2;
					rwing.Y = (rwing.Y + belly.Y) / 2;
					rwing.Z = (rwing.Z + belly.Z) / 2;
				}

				Math_Lib.high_sin_cos(pitch, sin, cos);
				if((cos > 0) && (cos < 1638))	cos =  1638;
				if((cos > -1638) && (cos < 0))	cos = -1638;
				lowest = nose;
				if(tail.Y < lowest.Y)
				{
					pitch -= cos * temp;
					lowest = tail;
				}
				else
					pitch += cos * temp;

				Math_Lib.high_sin_cos(roll, sin, cos);
				if((cos > 0) && (cos < 1638))	cos =  1638;
				if((cos > -1638) && (cos < 0))	cos = -1638;
				if(rwing.Y < lwing.Y)
				{
					roll -= cos * temp;
					if(rwing.Y < lowest.Y)
						lowest = rwing;
				}
				else
				{
					roll += cos * temp;
					if(lwing.Y < lowest.Y)
						lowest = lwing;
				}
			}
			else
			{
				SHAPE.GetContactPoint((itemptr)this, CT_FRONT, coords, offset, PushedOffset);
				COORDS3D nose = coords;
				SHAPE.GetContactPoint((itemptr)this, CT_WHLBACK, coords, offset, PushedOffset);
				COORDS3D tail = coords;
				SHAPE.GetContactPoint((itemptr)this, CT_LEFT, coords, offset, PushedOffset);
				COORDS3D lwing = coords;
				SHAPE.GetContactPoint((itemptr)this, CT_RIGHT, coords, offset, PushedOffset);
				COORDS3D rwing = coords;
				SHAPE.GetContactPoint((itemptr)this, CT_BELLY, coords, offset, PushedOffset);
				COORDS3D belly = coords;

				if(adptr->LEFTWINGOUT == BS_DEAD)
				{
					lwing.X = (lwing.X + belly.X) / 2;
					lwing.Y = (lwing.Y + belly.Y) / 2;
					lwing.Z = (lwing.Z + belly.Z) / 2;
				}
				if(adptr->RIGHTWINGOUT == BS_DEAD)
				{
					rwing.X = (rwing.X + belly.X) / 2;
					rwing.Y = (rwing.Y + belly.Y) / 2;
					rwing.Z = (rwing.Z + belly.Z) / 2;
				}

				Math_Lib.high_sin_cos(pitch, sin, cos);
				if((cos > 0) && (cos < 1638))	cos =  1638;
				if((cos > -655) && (cos < 0))	cos = -1638;
				lowest = belly;
				if(tail.Y < lowest.Y)
				{
					pitch -= cos * temp;
					lowest = tail;
				}
				else if(nose.Y < lowest.Y)
				{
					pitch += cos * temp;
					lowest = nose;
				}


				Math_Lib.high_sin_cos(roll, sin, cos);
				if((cos > 0) && (cos < 1638))	cos =  1638;
				if((cos > -1638) && (cos < 0))	cos = -1638;
				if(rwing.Y < lwing.Y)
				{
					roll -= cos * temp;
					if(rwing.Y < lowest.Y)
						lowest = rwing;
				}
				else
				{
					roll += cos * temp;
					if(lwing.Y < lowest.Y)
						lowest = lwing;
				}
			}

			if(lowest.Y < GroundHeight)
			{
				if (vel_ > 50000)
					Trans_Obj.LaunchExplodePuff(this,500,DRTSML,450,*mobileitem::currworld);

				World.Y += GroundHeight - lowest.Y;
				vel_y = (10000 / 2) * (GroundHeight - lowest.Y) / Timer_Code.FRAMETIME;

				if (Math_Lib.rnd() < 1024)								//RJS 26Sep00
					Trans_Obj.LaunchHitGrass(this,GroundHeight,*mobileitem::currworld);
			}
			else
			{
				vel_y -= 980.6 * Timer_Code.FRAMETIME;
				World.Y += vel_y * Timer_Code.FRAMETIME / 10000;
			}

			FP oldvel = vel_;
			AddAccel2Vel(-5000);

			if(vel_ > 0)
			{
				vel_x = FP(vel_x) * FP(vel_) / oldvel;
				vel_z = FP(vel_z) * FP(vel_) / oldvel;
				World.X += (vel_x * Timer_Code.FRAMETIME) / 10000;
				World.Z += (vel_z * Timer_Code.FRAMETIME) / 10000;
				uniqueID.changed = TRUE;
			}
			else
				vel_x = vel_y = vel_z = vel_ = 0;						//RJS 25Sep00

			if(!TimeLeft())
			{
				ai.ManStep = PHASE3;
				adptr->hasdust = FALSE;
			}
			SetFlightParams();
			break;
		}

		case PHASE3:
		{
			if (onwater)									//RJS 04Oct00
				adptr->IsInvisible = true;					//RJS 04Oct00

			vel_x = vel_y = vel_z = vel_ = 0;							//RJS 11Aug00
			break;
		}
	}
}





//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoFollowTheLeader
//Author		Craig Beeston
//Date			Thu 03 Feb 2000
//
//Description	1:	Follows a couple of waypoint and says a radio message or 2
//				2:	Does a series of non offensive manoeuvres
//				3:	Debrief
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoFollowTheLeader()
{
	ULong timer = (fly.pModel->FrameCount - uniqueID.count) * Timer_Code.FRAMETIME;
	ULong OnTailCount = ai.desiredalt;


	const ULong GOOD_RANGE = 25000;			//200 metres
	const SWord GOOD_ANGLE = ANGLES_45Deg;
	const UWord TOO_CLOSE  = 2000;			//20 metres
	const UWord TOO_FAR    = 50000;			//500 metres

	const ULong TRAINING_TIME  = 12000;	//2   mins
	const ULong	GOOD_ONTAILCOUNT =  9000;	//1.5 mins
	const ULong	BAD_ONTAILCOUNT  =  6000;	//1   min

	if(timer == 0)
	{
		//Initialising
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_FOLLOWLEADERINSTRUCT, MSG_FORMATIONFLYING_RPT, this, NULL, Persons2::PlayerGhostAC));
		ai.simpleacm = FALSE;
		Manual_Pilot.InitManualPilot(this);
		fly.pModel->bACM = TRUE;
		fly.pModel->UpdateModel(this);
		ai.manoeuvre = MANOEUVRE_STRAIGHTANDLEVEL;		//20 seconds
		ai.ManStep = 0;
		OnTailCount = 0;
	}

	SetUnfriendly(Persons2::PlayerGhostAC);								//CSB 1Aug00
	if((!Persons2::PlayerGhostAC) || (Persons2::PlayerGhostAC->Status.deadtime))
	{
		movecode = AUTO_FOLLOWWP;
		information = IF_OUT_POS;					//CSB 22Aug00
		manoeuvretime = 0;
		AutoFollowWp();
		OnTailCount = 0;
		return;
	}

	InterceptandRange(ai.unfriendly);

	if(Range < GOOD_RANGE)
	{
		SWord dheadg = SWord(hdg) + ANGLES_180Deg - SWord(HdgIntercept);
		SWord dpitch = SWord(pitch) + SWord(PitchIntercept);
		if(	(dheadg < GOOD_ANGLE) && (-dheadg < GOOD_ANGLE) && (dpitch < GOOD_ANGLE) && (-dpitch < GOOD_ANGLE) )
			OnTailCount += Timer_Code.FRAMETIME;

		if((Range < TOO_CLOSE) && ((fly.pModel->FrameCount & 0xff) == 0))
		{
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_FOLLOWLEADERNEARHIT, MSG_FORMATIONFLYING_RPT, this, NULL, Persons2::PlayerGhostAC));
		}
	}
	else if(Range > TOO_FAR)
	{
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_LEFTBEHIND, MSG_FORMATIONFLYING_RPT, this, NULL, Persons2::PlayerGhostAC));
		timer = TRAINING_TIME;
		ai.manoeuvre = MANOEUVRE_SELECT;
	}



	switch(ai.manoeuvre)
	{
		case MANOEUVRE_SCISSORS:			ACMAS().Scissors();			break;
		case MANOEUVRE_MILDSCISSORS:		ACMAS().MildScissors();		break;
		case MANOEUVRE_SPLITS:				ACMAS().SplitS();			break;
		case MANOEUVRE_ZOOMANDDROP:			ACMAS().ZoomandDrop();		break;
		case MANOEUVRE_STRAIGHTANDLEVEL:	ACMAS().StraightandLevel();	break;
		case MANOEUVRE_STRAIGHTDIVE:		ACMAS().StraightDive();		break;
		case MANOEUVRE_REVERSETURN:			ACMAS().ReverseTurn();		break;
		case MANOEUVRE_JINK:				ACMAS().Jink();				break;
		case MANOEUVRE_BREAKTURN:			ACMAS().BreakTurn();		break;
		case MANOEUVRE_LAZYTURN:			ACMAS().LazyTurn();			break;
		case MANOEUVRE_BREAKLOW:			ACMAS().BreakLow();			break;
		case MANOEUVRE_BREAKHIGH:			ACMAS().BreakHigh();		break;
		case MANOEUVRE_BREAK90:				ACMAS().Break90();			break;
		case MANOEUVRE_HIGBARRELROLL:		ACMAS().HiGBarrelRoll();	break;
		case MANOEUVRE_UNBALANCEDFLIGHT:	ACMAS().UnBalancedFlight();	break;
		case MANOEUVRE_ZOOM:				ACMAS().Zoom();				break;
		case MANOEUVRE_GAINSPEED:			ACMAS().GainSpeed();		break;
		case MANOEUVRE_STEEPDIVE:			ACMAS().SteepDive();		break;
		case MANOEUVRE_CLIMBTURN:			ACMAS().ClimbTurn();		break;
		case MANOEUVRE_LOWALT:				ACMAS().LowAlt();			break;
	}

	if(World.Y < FT_1000)
		ai.manoeuvre = MANOEUVRE_LOWALT;

	Manual_Pilot.ACMManualPilot(this);

	if(ai.manoeuvre == MANOEUVRE_SELECT)
		if(timer >= TRAINING_TIME)
		{
			//Debrief
			if(OnTailCount > GOOD_ONTAILCOUNT)
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_GOOD, MSG_FORMATIONFLYING_RPT, this, NULL, Persons2::PlayerGhostAC));
			else if(OnTailCount > BAD_ONTAILCOUNT)
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_NOTBAD, MSG_FORMATIONFLYING_RPT, this, NULL, Persons2::PlayerGhostAC));
			else
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_NOTGOOD, MSG_FORMATIONFLYING_RPT, this, NULL, Persons2::PlayerGhostAC));

			movecode = AUTO_FOLLOWWP;
			information = IF_OUT_POS;					//CSB 22Aug00
			manoeuvretime = 0;

			OnTailCount = 0;
		}
		else
		{
			ai.ManStep = 0;
			switch(Math_Lib.rnd(19))
			{
				case 0:		ai.manoeuvre = MANOEUVRE_SCISSORS;			break;
				case 1:		ai.manoeuvre = MANOEUVRE_MILDSCISSORS;		break;
				case 2:		ai.manoeuvre = MANOEUVRE_SPLITS;			break;
				case 3:		ai.manoeuvre = MANOEUVRE_ZOOMANDDROP;		break;
				case 4:		ai.manoeuvre = MANOEUVRE_STRAIGHTANDLEVEL;	break;
				case 5:		ai.manoeuvre = MANOEUVRE_STRAIGHTDIVE;		break;
				case 6:		ai.manoeuvre = MANOEUVRE_REVERSETURN;		break;
				case 7:		ai.manoeuvre = MANOEUVRE_JINK;				break;
				case 8:		ai.manoeuvre = MANOEUVRE_BREAKTURN;			break;
				case 9:		ai.manoeuvre = MANOEUVRE_LAZYTURN;			break;
				case 10:	ai.manoeuvre = MANOEUVRE_BREAKLOW;			break;
				case 11:	ai.manoeuvre = MANOEUVRE_BREAKHIGH;			break;
				case 12:	ai.manoeuvre = MANOEUVRE_BREAK90;			break;
				case 13:	ai.manoeuvre = MANOEUVRE_HIGBARRELROLL;		break;
				case 14:	ai.manoeuvre = MANOEUVRE_UNBALANCEDFLIGHT;	break;
				case 15:	ai.manoeuvre = MANOEUVRE_ZOOM;				break;
				case 16:	ai.manoeuvre = MANOEUVRE_GAINSPEED;			break;
				case 17:	ai.manoeuvre = MANOEUVRE_STEEPDIVE;			break;
				case 18:	ai.manoeuvre = MANOEUVRE_CLIMBTURN;			break;
			}
		}
	ai.desiredalt = OnTailCount;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		TrackBogey
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
void	MoveAirStruc::TrackBogey()
{
	//routine used for AUTO_TRACKING equate
	AutoPreCombat ();
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		TrackBandit
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
void	MoveAirStruc::TrackBandit()
{
	//## not used????
	//routine used for AUTO_TRACKING equate
	AutoPreCombat ();
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoPreCombat
//Author		R. Hyde
//Date			Thu 11 Jul 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoPreCombat ()
{
	if(!ai.unfriendly)
	{
		if(!Art_Int.AttackSpareInLeadersGroup(this))
			for(AirStrucPtr ld = FindFormpos0(); ld; ld = ld->fly.nextflight)
				for(AirStrucPtr ac = ld; ac; ac = ac->Follower())
				{
					ac->movecode = AUTO_FOLLOWWP;
				}
	}


	if((!formpos) && ((fly.pModel->FrameCount & 0x1fff) == 0))	//every 82 secs
	{
		FP gohomefract = 0.5;
		if((ai.unfriendly) && (AirStrucPtr(ai.unfriendly)->ai.manoeuvre == MANOEUVRE_SCREWYOUGUYSIMGOINGHOME))
			gohomefract = 0.25;
		if(GroupShouldGoHome(gohomefract))
		{
			if(follower)
   				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ALLGOHOME, MSG_STATUSREPORT, *this, NULL, Follower()));
			else if(fly.nextflight)
   				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ALLGOHOME, MSG_STATUSREPORT, *this, NULL, fly.nextflight));

			for(AirStrucPtr nf = this; nf; nf = nf->fly.nextflight)
				for(AirStrucPtr f = nf; f; f = f->Follower())
				{
					f->waypoint = f->waypoint->FindWP(WPNAME_EscDisperse);
					if((f->movecode != AUTO_COMBAT) || (!f->ai.unfriendly))
					{
						f->SetUnfriendly(NULL);
						f->movecode = AUTO_FOLLOWWP;
						f->information = IF_OUT_POS;
						f->manoeuvretime = 0;
						f->ai.manoeuvre = MANOEUVRE_SCREWYOUGUYSIMGOINGHOME;
					}
				}
		}
	}


	AirStrucPtr lead = NULL;

	if(formpos)
	{
		if(leader)
			lead = Leader();
		else
			lead = fly.leadflight;
		if((lead) && (lead->movecode != AUTO_PRECOMBAT))
			lead = NULL;
	}


	if(lead)
	{
		AutoFollowWpWing(lead);
		if((ai.unfriendly) && (Distance3DSquared(&ai.unfriendly->World) < FP(ENGAGERANGE) * FP(ENGAGERANGE)))
		{
			AirStrucPtr unf = AirStrucPtr(ai.unfriendly);
			if((!fly.numinsag) && (unf->fly.numinsag) && (unf->fly.expandedsag))	//CSB 1Aug00
				SetUnfriendly(unf);										//CSB 1Aug00

			Art_Int.SetEngage(this, *ai.unfriendly, MANOEUVRE_SELECT, ANGLES_0Deg, ANGLES_0Deg, false);
		}
	}
	else if(ai.unfriendly)
	{
		if(!TimeLeft())
		{
			InterceptandRange(FindCombatPos());
			if(PitchIntercept > Angles(CalcBestClimbPitch()))
	 			PitchIntercept = Angles(CalcBestClimbPitch());

			Bool level = AutoCalcPitch();
			level &= CalcHdgRoll();
			level &= CalcVelAlt(CalcMaxVelSL());
			CalcXYZVel ();

			if(level)
				if(Range > 300000)
					SetManoeuvreTime(FIVESECS100);
				else
					SetManoeuvreTime(SECS100);

			if(Range < COMBATRANGE)
				Art_Int.SetEngage(this, *ai.unfriendly, MANOEUVRE_SELECT, ANGLES_0Deg, ANGLES_0Deg, false);
			else if(Range > VISIBLERANGE)
				Art_Int.BreakOff(this);

		}
		NewPosition();
		SetFlightParams();
	}
	else
	{
		if(Art_Int.AttackSpareInLeadersGroup(this))
		{
			NewPosition();
			SetFlightParams();
		}
		else
		{
			Art_Int.BreakOff(this);
			AutoFollowWp();
		}
	}
}


/*	Float rangeto = RECOGNISERANGE;
	rangeto = rangeto * rangeto;
	Float maxrange = rangeto;
	if (candidate)
	{
		rangeto = Distance3DSquared(&candidate->World);
	}
	if	((rangeto < maxrange) &&	(			((candidate)	&&	(movecode==candidate->movecode))
												||	((candidate  == Persons2::PlayerGhostAC)  && (Persons2::PlayerSeenAC->movecode == AUTO_TRACKINGBOGIE))
										)
		)
	{//follower
		if (ai.unfriendly)
		{
			InterceptandRange (&ai.unfriendly->World);
			if (Range<COMBATRANGE)	//CSB 24/06/99
			{	//set for combat next frame
				//opportunity to re-think engagement and change unfriendly
				//leader should have already told follower to engage
				//as he hasn't try again
				if (	(leader)	&&	(ai.elementtactics == WELDEDWING))
				{//don't give a target: this should lead to weldedwing tactics //RDH 28/04/99
					Art_Int.SetEngage(this,NULL, MANOEUVRE_SELECT,ANGLES_0Deg,ANGLES_0Deg, FALSE);
				}else
					Art_Int.SetEngage(this,*ai.unfriendly, MANOEUVRE_SELECT,ANGLES_0Deg,ANGLES_0Deg, FALSE);
			}
		}
		AutoFollowWpWing (candidate);
	}
	else if (ai.unfriendly==NULL)
	{
		Art_Int.BreakOff(this);
		AutoFollowWp();
	}
	else
	{
		if	(!TimeLeft())
		{
			InterceptandRange (&ai.unfriendly->World);
			Bool	level=(Bool)(Range>MILES01);

			if (		(Range < RECOGNISERANGE)					  //RDH 26/03/99
					&&	(nationality == ((AirStruc*)ai.unfriendly)->nationality)   //RDH 26/03/99
				)													  //RDH 26/03/99
			{														  //RDH 26/03/99
				Art_Int.BreakOff(this);								  //RDH 26/03/99
			}else
			{
				if (Range<COMBATRANGE)
				{	//set for combat next frame. Tell all followers to break.
					Art_Int.AllEngage(this);
				}
				Bool	DoEngage=TRUE;

				InterceptandRange (FindCombatPos());
				if (PitchIntercept>classtype->bestclimbpitch)	//max climb rate//RDH 26Jul96
				{	//IF TOO STEEP make shallow and fix 3d range
					//not for tactician.. he wants to get alt.
			 		PitchIntercept=classtype->bestclimbpitch;		//RDH 26Jul96
				}

				level&=CalcVelAlt (classtype->cruisevel+100000);	//20mph above cruise
				level&=CalcHdgRoll ();

				level&=AutoCalcPitch ();
				CalcXYZVel ();
				if(level && Range > ENGAGERANGE)
					SetManoeuvreTime (FIVESECS100);
			}
		}
		NewPosition ();
		SetFlightParams ();
	}
*/


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindCombatPos
//Author		Jim Taylor
//Date			Thu 21 Nov 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
COORDS3D*	AirStruc::FindCombatPos ()
{
	return(&ai.unfriendly->World);


}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetTakeOffPos
//Author		R. Hyde
//Date			Tue 28 Nov 1995		//JIM 05/03/99
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Coords3D*	AirStruc::GetTakeOffPos(UniqueID takeoffID,Coords3D& despos)
{
	despos.X = despos.Y = despos.Z = 0;

	UniqueID homeid=ai.homebase->uniqueID.count;
	if (takeoffID==RunwaySBAND)
	{
		if (waypoint->target)
			if (	waypoint->target->uniqueID.count==homeid
				||	waypoint->target->SGT==homeid	)
			{
				despos=waypoint->target->World;
				despos.Y+=classtype->deckshunt;
				return &despos;
			}
	}
	ItemPtr to=NULL,lan=NULL;
	{
		{for (int i=RunwaySBAND;i!=RunwayEBAND;i++)
		{
			to=Persons2::ConvertPtrUID(UniqueID(i));
			breakif (to && to->SGT==homeid);
		}}
		{for (int i=RunwayEBAND;i!=BlokeBAND;i++)
		{
			lan=Persons2::ConvertPtrUID(UniqueID(i));
			breakif (lan && lan->SGT==homeid);
		}}
	}
	if (to && takeoffID==RunwaySBAND)
	{
		despos=to->World;
		despos.Y+=classtype->deckshunt;
		return &despos;
	}
	if (lan && takeoffID==RunwayEBAND)
	{
		despos=lan->World;
		despos.Y+=classtype->deckshunt;
		return &despos;
	}
	ItemPtr p=Persons2::ConvertPtrUID(takeoffID);
	if (p && p->SGT==homeid)
	{
		despos=p->World;
		despos.Y+=classtype->deckshunt;
		return &despos;
	}
	else
	{
		if((despos.X == 0) && (despos.Z == 0))
		{
			if(takeoffID == RunwaySBAND)
			{
				despos.X = ai.homebase->World.X;
				despos.Y = ai.homebase->World.Y + classtype->deckshunt;
				despos.Z = ai.homebase->World.Z - 25000;
			}
			else if(takeoffID == RunwayEBAND)
			{
				despos.X = ai.homebase->World.X;
				despos.Y = ai.homebase->World.Y + classtype->deckshunt;
				despos.Z = ai.homebase->World.Z + 25000;
			}
		}
		return(&despos);												//CSB 21Aug00

//TEMPCODE RDH 29/03/00 		despos=lan->World;
//TEMPCODE RDH 29/03/00 		int dx=(lan->World.X-to->World.X)/2;
//TEMPCODE RDH 29/03/00 		int	dz=(lan->World.Z-to->World.Z)/2;
//TEMPCODE RDH 29/03/00 		despos.X+=dz;
//TEMPCODE RDH 29/03/00 		despos.Z-=dx;
//TEMPCODE RDH 29/03/00 		switch (takeoffID)
//TEMPCODE RDH 29/03/00 		{
//TEMPCODE RDH 29/03/00 			case	UID_TAXI0: 	despos.X+=dz;despos.Z-=dx;
//TEMPCODE RDH 29/03/00 			case	UID_TAXI1: 	despos.X+=dz;despos.Z-=dx;
//TEMPCODE RDH 29/03/00 			case	UID_TAXI2:
//TEMPCODE RDH 29/03/00 		}
//TEMPCODE RDH 29/03/00 		despos.Y+=classtype->deckshunt;
//TEMPCODE RDH 29/03/00 		return &despos;
	}

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetTakeOffPoint
//Author		Craig Beeston
//Date			Thu 16 Dec 1999
//
//Description	Calc position of various points required during takeoff and landing
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Coords3D* AirStruc::GetTakeOffPoint(TAKEOFFPOINT point, Coords3D& despos)
{
	COORDS3D pos;
	switch(point)
	{
		case TP_SQUADRONSTART:
		{
			GetTakeOffPos(UniqueID(RunwaySBAND), pos);
			break;
		}
		case TP_SQUADRONTOUCHDOWN:
		{
			COORDS3D p1, p2;
			GetTakeOffPos(UniqueID(RunwayEBAND), p1);
			GetTakeOffPos(UniqueID(RunwaySBAND), p2);
			FP dist = 1.0 / FP(Distance3D(&p1, &p2));
			FP sin = (p1.X - p2.X) * dist;
			FP cos = (p1.Z - p2.Z) * dist;
			FP desdist = 10000;
			pos.X = p2.X + desdist * sin;
			pos.Y = p2.Y;
			pos.Z = p2.Z + desdist * cos;
			break;
		}
		case TP_RUNWAYEND:
		{
			GetTakeOffPos(UniqueID(RunwayEBAND), pos);
			break;
		}
		case TP_TAXI1:
		{
			COORDS3D* found = GetTakeOffPos(UniqueID(UID_TAXI0), pos);
			if(!found->X)
			{
				COORDS3D p1, p2;
				GetTakeOffPos(UniqueID(RunwayEBAND), p1);
				GetTakeOffPos(UniqueID(RunwaySBAND), p2);
				FP dist = 1.0 / FP(Distance3D(&p1, &p2));
				FP sin = (p1.X - p2.X) * dist;
				FP cos = (p1.Z - p2.Z) * dist;
				FP dx = 25000;
				FP dz = 50000;
				pos.X = p2.X + dx * cos + dz * sin;
				pos.Z = p2.Z - dx * sin + dz * cos;
			}
			break;
		}
		case TP_TAXI2:
		{
			COORDS3D* found = GetTakeOffPos(UniqueID(UID_TAXI1), pos);
			if(!found->X)
			{
				COORDS3D p1, p2;
				GetTakeOffPos(UniqueID(RunwayEBAND), p1);
				GetTakeOffPos(UniqueID(RunwaySBAND), p2);
				FP dist = 1.0 / FP(Distance3D(&p1, &p2));
				FP sin = (p1.X - p2.X) * dist;
				FP cos = (p1.Z - p2.Z) * dist;
				FP dx = 25000;
				FP dz = 10000;
				pos.X = p2.X + dx * cos + dz * sin;
				pos.Z = p2.Z - dx * sin + dz * cos;
			}
			break;
		}
		case TP_TAXI3:
		{
			COORDS3D* found = GetTakeOffPos(UniqueID(UID_TAXI2), pos);
			if(!found->X)
			{
				COORDS3D p1, p2;
				GetTakeOffPos(UniqueID(RunwayEBAND), p1);
				GetTakeOffPos(UniqueID(RunwaySBAND), p2);
				FP dist = 1.0 / FP(Distance3D(&p1, &p2));
				FP sin = (p1.X - p2.X) * dist;
				FP cos = (p1.Z - p2.Z) * dist;
				FP dx = 25000;
				FP dz = -17500;
				pos.X = p2.X + dx * cos + dz * sin;
				pos.Z = p2.Z - dx * sin + dz * cos;
			}
			break;
		}
		case TP_SQUADRONTURN:
		{
			COORDS3D p1, p2;
			GetTakeOffPos(UniqueID(RunwayEBAND), p1);
			GetTakeOffPos(UniqueID(RunwaySBAND), p2);
			FP dist = 1.0 / FP(Distance3D(&p1, &p2));
			FP sin = (p1.X - p2.X) * dist;
			FP cos = (p1.Z - p2.Z) * dist;
			FP desdist = -17500;
			pos.X = p2.X + desdist * sin;
			pos.Y = p2.Y;
			pos.Z = p2.Z + desdist * cos;
			break;
		}
		case TP_FLIGHTTURN:
		{
			COORDS3D p1, p2;
			GetTakeOffPos(UniqueID(RunwayEBAND), p1);
			GetTakeOffPos(UniqueID(RunwaySBAND), p2);
			FP dist = 1.0 / FP(Distance3D(&p1, &p2));
			FP sin = (p1.X - p2.X) * dist;
			FP cos = (p1.Z - p2.Z) * dist;
			FP desdist = -3000;
			int flightnum = (formpos&InFormMAX);

			if(nationality == NAT_RAF)
				desdist -= flightnum * 4000;
			else
				desdist -= flightnum * 6000;
			pos.X = p2.X + desdist * sin;
			pos.Y = p2.Y;
			pos.Z = p2.Z + desdist * cos;
			break;
		}
		case TP_ACPARK:
		{
			COORDS3D p1, p2;
			GetTakeOffPos(UniqueID(RunwayEBAND), p1);
			GetTakeOffPos(UniqueID(RunwaySBAND), p2);
			FP dist = 1.0 / FP(Distance3D(&p1, &p2));
			FP sin = (p1.X - p2.X) * dist;
			FP cos = (p1.Z - p2.Z) * dist;
			FP dx = 0;
			FP dz = 0;
			int flightnum = (formpos&InFormMAX);
			int flightpos = position();

			if(nationality == NAT_RAF)
			{
				dz = -4000 * flightnum;
				if(flightnum == 1)	dx -= 3000;
				if(flightnum == 2)	dx -= 1500;
				if(flightnum == 3)	dx += 1500;

				if(flightpos == 1)	dx += 6000;
				if(flightpos == 2)	dx -= 6000;
			}

			if(nationality == NAT_LUF)
			{
				dz = -6000 * flightnum;
				dx += 1500 * flightnum;

				if(flightpos == 1)	dx += 4500;
				if(flightpos == 2)	dx -= 4500;
				if(flightpos == 3)	dx -= 9000;
			}

			if(classtype->aerobaticfactor == AEROBATIC_LOW)
			{
				dz = -5000 * flightnum;

				if(flightpos == 1)	dx =  5000;
				if(flightpos == 2)	dx = -5000;
				if(flightpos == 3)	dx =  10000;
				if(flightpos == 4)	dx = -10000;
			}

			pos.X = p2.X + dx * cos + dz * sin;
			pos.Y = p2.Y;
			pos.Z = p2.Z - dx * sin + dz * cos;
			break;
		}
		case TP_BOWSER1:
		case TP_BOWSER2:
		case TP_BOWSER3:
		case TP_BOWSER4:
		{
			COORDS3D p1, p2;
			GetTakeOffPos(UniqueID(RunwayEBAND), p1);
			GetTakeOffPos(UniqueID(RunwaySBAND), p2);
			FP dist = 1.0 / FP(Distance3D(&p1, &p2));
			FP sin = (p1.X - p2.X) * dist;
			FP cos = (p1.Z - p2.Z) * dist;
			FP dx = 0;
			FP dz = -6000;
			if(nationality == NAT_RAF)
			{
				if(point == TP_BOWSER1)	{	dx = 4500;	dz = -11000;	}
				if(point == TP_BOWSER2)	{	dx = -4500;	dz = -9000;		}
				if(point == TP_BOWSER3)	{	dx = -6000;	dz = -3000;		}
				if(point == TP_BOWSER4)	{	dx = 3000;	dz = -1000;		}
			}
			if(nationality == NAT_LUF)
			{
				if(point == TP_BOWSER1)	{	dx = -4500;	dz = -9000;		}
				if(point == TP_BOWSER2)	{	dx = 4500;	dz = -9000;		}
				if(point == TP_BOWSER3)	{	dx = 3000;	dz = -3000;		}
				if(point == TP_BOWSER4)	{	dx = -6000;	dz = -3000;		}
			}
			pos.X = p2.X + dx * cos + dz * sin;
			pos.Y = p2.Y;
			pos.Z = p2.Z - dx * sin + dz * cos;
			break;
		}
	}

	if(ai.homebase->World.Y == 0)
	{
		Grid_Base* cornerAlt = (Grid_Base*) Grid_Base::gridfiles[Grid_Base::GF_CORNERALT].getdata();
		ai.homebase->World.Y = cornerAlt->getWorld(ai.homebase->World.X, ai.homebase->World.Z);
	}
	pos.Y = ai.homebase->World.Y + classtype->deckshunt;
	despos = pos;
	return &despos;
}

////////////////////////////////////////////////////////////////////////
//
// Function:    AutoTakeOff
// Date:        20/01/99
// Author:      JIM
//
// Description:
//
//Phases:
//0		Hold until time
//1		Hold until player gone
//2		Move towards TAXI1 at 40m every 15 seconds On reaching TAXI1, phase 5
//
//3		Hold until player gone
//4		Move towards TAXI0 at 40m every 15 seconds. On reaching TAXI0, phase 6
//
//5		Move all way to TAXI0 over ?15? seconds. On time up, phase 6
//
//6		Hold for 5 seconds
//		Within 5 seconds
//			1st Elt moves forward to LAUNCHL
//			2nd Elt moves forward to LAUNCHW
//7		Within 5 seconds
//			Both turn and move forward until leader at RUNWAY0 then PHASE 9
//
//8		Hold until player gone
//9		Take off acceleration up runway towards RUNWAY1
//
//10	Climb straight forward for fixed time
//
//11	Slow turn in formation
////////////////////////////////////////////////////////////////////////

AirStrucPtr flight_ctl::leadelt()
{
	if ((originalformpos&InWingMAX)!=2)
		assert(AirStrucPtr(int(this)-int(&AirStrucPtr(NULL)->fly))->position()==2);
	AirStrucPtr scan=leadflight;
	while (scan && &scan->fly.nextflight->fly!=this)
		scan=scan->fly.nextflight;
	return scan;
}




//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoWait4Time
//Author		Craig Beeston
//Date			Thu 06 Jul 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoWait4Time()
{
	if(timeofday >= waypoint->ETA)
	{
		movecode = AUTO_TAKEOFF;
		WayPointPtr w = waypoint;
		if(w->skipunder == UID_NULL)
			w = w->next;
		else
		{
			while((w->wpname != WPNAME_BombRendezvous) && (w->wpname != WPNAME_EscRendezvous))
				w = w->next;
			if(ai.submethod & ai_info::SM_RETURN_ESCORT)
				while(w->wpname != WPNAME_EscDisperse)
					w = w->next;
			else
				if((formation & FORMTYPE_COURSESEL) > FTC_RAF)
					while(w->wpname != WPNAME_EscRendezvous)
						w = w->next;
		}
		waypoint = w;
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoTakeOff
//Author		Craig Beeston
//Date			Tue 14 Dec 1999
//
//Description	BOB take-off sequence	1	-	10 secs to start engines
//										2	-	1/2 sec between aircraft starting to move
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoTakeOff ()
{

	AircraftAnimData*		acanim  = NULL;
	SimpleAircraftAnimData*	saganim = NULL;
	if(fly.numinsag)
		saganim = (SimpleAircraftAnimData*)Anim;
	else
		acanim = (AircraftAnimData*)Anim;

	switch(ai.ManStep)
	{
		case PHASE0:						//Quick Initialise
		{
			vel_  = 0;
			vel_x = 0;
			vel_y = 0;
			vel_z = 0;
//TEMPCODE RDH 29/03/00 			if(acanim)
//TEMPCODE RDH 29/03/00 			{
//TEMPCODE RDH 29/03/00 				if(fly.expandedsag)
//TEMPCODE RDH 29/03/00 					acanim->acrpm = classtype->minrpm;
//TEMPCODE RDH 29/03/00 				else
//TEMPCODE RDH 29/03/00 					acanim->acrpm = 0;
//TEMPCODE RDH 29/03/00 				acanim->acleglowerl = 0;
//TEMPCODE RDH 29/03/00 				acanim->acleglowerr = 0;
//TEMPCODE RDH 29/03/00 				if((fly.pModel->Type == AT_SPITFIRE) || (fly.pModel->Type == AT_HURRICANE) || (fly.pModel->Type == AT_JU87B))
//TEMPCODE RDH 29/03/00 					acanim->accanopy = 255;
//TEMPCODE RDH 29/03/00 			}
//TEMPCODE RDH 29/03/00 			else if(saganim)
//TEMPCODE RDH 29/03/00 			{
//TEMPCODE RDH 29/03/00 				saganim->acrpm = 0;
//TEMPCODE RDH 29/03/00 			}
//TEMPCODE RDH 29/03/00
//TEMPCODE RDH 29/03/00 			COORDS3D runway0, runway1;
//TEMPCODE RDH 29/03/00 			GetTakeOffPoint(TP_SQUADRONSTART, runway0);
//TEMPCODE RDH 29/03/00 			GetTakeOffPoint(TP_RUNWAYEND, runway1);
//TEMPCODE RDH 29/03/00 			InterceptandRange(&runway0, &runway1);
//TEMPCODE RDH 29/03/00 			hdg = HdgIntercept;
//TEMPCODE RDH 29/03/00
//TEMPCODE RDH 29/03/00 			GetTakeOffPoint(TP_ACPARK, despos);
//TEMPCODE RDH 29/03/00 			World = despos;
//TEMPCODE RDH 29/03/00 			pitch = classtype->deckangle;

			if(classtype->aerobaticfactor == AEROBATIC_LOW)
			{
				SLong rpm = classtype->maxrpm / 2;
				if(acanim)
					acanim->acrpm = rpm;
				else if(saganim)
					saganim->acrpm = rpm;
			}
			SetManoeuvreTime(1000);
			ai.ManStep = PHASE1;

			if(formpos == 0)
			{
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SURFACEWINDREQUEST, MSG_WIND_REQUEST, *this, NULL, *this));

				if(Save_Data.flightdifficulty [FD_WINDEFFECTS])
				{
					if(Save_Data.flightdifficulty [FD_WINDGUSTS])
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_SURFACEWIND_BEARINGNO_SPEED_GUSTS, MSG_WIND_LOW_REPORT, VOICE_TOWER, NULL, *this));
					else
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_SURFACEWIND_BEARINGNO_SPEED, MSG_WIND_LOW_REPORT, VOICE_TOWER, NULL, *this));

					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ALTWINDREQUEST, MSG_WIND_REQUEST, *this, NULL, *this));
					if(Save_Data.flightdifficulty [FD_WINDGUSTS])
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_ALTWIND_BEARINGNO_SPEED_GUSTS, MSG_WIND_HIGH_REPORT, VOICE_TOWER, NULL, *this));
					else
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_ALTWIND_BEARINGNO_SPEED, MSG_WIND_HIGH_REPORT, VOICE_TOWER, NULL, *this));
				}
				else
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_NOWIND, MSG_STATUSRESPONSE_DELAY_LP_RPT, VOICE_TOWER, NULL, *this));
			}
		}

		case PHASE1:						//Engine Starting
		{
			if(manoeuvretime < 200)
			{
				SLong rpm = (classtype->maxrpm * (200 - manoeuvretime)) / 200;
				if((acanim) && (acanim->acrpm < rpm))
					acanim->acrpm = rpm;
				else if((saganim) && (saganim->acrpm < rpm))
					saganim->acrpm = rpm;
			}

			if(!TimeLeft())
			{
				ai.ManStep = PHASE2;
				Trans_Obj.LaunchEngineStartup((mobileitem*)this, *currworld);
				SWord takeofftime = 0;
				if(!fly.numinsag)
				{
					bool foundme = false;
					AirStrucPtr nf = FindFormpos0();
					if(nf == Persons2::PlayerSeenAC)
						nf = Persons2::PlayerGhostAC;
					AirStrucPtr f = nf;
					AirStrucPtr me = this;
					if(me == Persons2::PlayerSeenAC)
						me = Persons2::PlayerGhostAC;
					while(!foundme)
					{
						if(f == me)
							foundme = true;
						else
						{
							if(f->follower)
								f = f->Follower();
							else
							{
								nf = nf->fly.nextflight;
								f = nf;
							}
							takeofftime++;
						}
					}
				}

				SetManoeuvreTime(takeofftime * 50);
			}
			break;
		}

		case PHASE2:						//Wait in line for 1/2 sec * ac number
		{
			if(!TimeLeft())
				ai.ManStep = PHASE3;
			break;
		}

		case PHASE3:						//Full Power - Accelerate and lift tail
		{
			SLong takeoffvel = 1.25 * classtype->landingspeed;
			SLong Accel = CalcAccel(takeoffvel);
			AddAccel2Vel(Accel);
			FP velfract = vel_;
			velfract /= takeoffvel;
			if((velfract > 0.5) && (velfract < 0.75))
			{
				FP newpitch = (4.0 * (0.75 - velfract) * FP(SWord(ANGLES_0Deg + classtype->deckangle)));
				pitch = Angles(SWord(newpitch));
			}
			Float sin, cos;
			Math_Lib.high_sin_cos(hdg, sin, cos);
			vel_x = vel_ * sin;
			vel_y = 0;
			vel_z = vel_ * cos;
			NewPosition(false);		//without wind effects
			if(velfract >= 1)
				ai.ManStep = PHASE4;
			break;
		}

		case PHASE4:
		{
			CalcVelAlt();
			Float deltapitch = FP(SWord(ANGLES_0Deg + classtype->deckangle)) * Timer_Code.FRAMETIME * (0.5 / 200);
			pitch += Angles(SWord(deltapitch));
			Float factor = FP(SWord(pitch));
			factor /= FP(SWord(ANGLES_0Deg + classtype->deckangle));
			factor = 2 * factor - 1;
			if(factor < 0) factor = 0;
			Float fpitch = factor * SWord(ANGLES_0Deg + classtype->deckangle);
			Float sinh, cosh, sinp, cosp;
			Math_Lib.high_sin_cos(Angles(SWord(fpitch)), sinp, cosp);
			Math_Lib.high_sin_cos(hdg, sinh, cosh);
			Float velhori = vel_ * cosp;

			SLong wx, wy, wz;						//artificially add in the effects of the wind shear close to the ground
			MissManCampSky().GetWind(World.Y, wx, wy, wz);

			vel_x = velhori * sinh + FP(wx) * factor;
			vel_y = vel_    * sinp + FP(wy) * factor;
			vel_z = velhori * cosh + FP(wz) * factor;

			NewPosition(false);

			if(SWord(pitch) > SWord(ANGLES_0Deg + classtype->deckangle))
			{
				ai.ManStep = PHASE5;
				SetManoeuvreTime(1000);
			}
			break;
		}

		case PHASE5:
		{
			FP despitch = FP(vel_ - classtype->minvel) / FP(CalcBestClimbSpeed() - classtype->minvel);
			despitch *= FP(CalcBestClimbPitch());
			if(SWord(pitch) > despitch)
				pitch -= 18 * Timer_Code.FRAMETIME;
			if(SWord(pitch) < despitch)
				pitch += 18 * Timer_Code.FRAMETIME;

			CalcVelAlt();
			CalcXYZVel();
			NewPosition();

			SWord delta = Timer_Code.FRAMETIME / 2;

			PMODEL pModel = fly.pModel;

			if((pModel->Type != AT_JU87B) && (acanim))		//Retract Gear
			{
				SWord leftleg  = acanim->acleglowerl;
				SWord rightleg = acanim->acleglowerr;

				leftleg  += delta;
				rightleg += delta;
				if(position() & 0x01)
					leftleg  += delta;
				else
					rightleg += delta;

				MODMAXMIN(leftleg, 0, 255);
				MODMAXMIN(rightleg, 0, 255);

				acanim->acleglowerl = leftleg;
				acanim->acleglowerr = rightleg;
			}

			if((acanim) && ((pModel->Type == AT_SPITFIRE) || (pModel->Type == AT_HURRICANE) || (pModel->Type == AT_JU87B)))
			{			//Close Hood
				SWord canopy = acanim->accanopy;
				canopy -= 2 * delta;
				MODMAXMIN(canopy, 0, 255);
				acanim->accanopy = canopy;
			}

			if(!TimeLeft())
			{
				ai.ManStep = PHASE6;
				SetManoeuvreTime(6000);
				if(!formpos)
				{
					AirStrucPtr buddy = FindBuddy();
					if((buddy) && (waypoint))
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_WARNNEWWAYPOINT, MSG_WARN_HEADING, *this, waypoint, buddy));
				}

				for(PAERODEVICE pDevice = pModel->DeviceList; pDevice; pDevice = pDevice->List.NextItem())
				{
					pDevice->ReqVal       = 0;
					pDevice->Attractor    = 0;
					pDevice->OldAttractor = 0;
					pDevice->Val          = 0;
				}

				if(acanim)
				{
					acanim->accanopy = 0;
					acanim->acleglowerl = 255;
					acanim->acleglowerr = 255;
				}
			}
			break;
		}

		case PHASE6:
		{
			int temptime = manoeuvretime;
			manoeuvretime = 0;
			AirStrucPtr lead = FindLeadCandidate();
			if((lead) && (lead->FindFormpos0() == FindFormpos0()))
				AutoFollowWpWing(lead);
			else
			{
				AutoFollowWp();
				if(vel_ > 0.9 * classtype->bestclimbvel)
					vel_ = 0.9 * classtype->bestclimbvel;
			}
			manoeuvretime = temptime;
			SWord wroll = SWord(roll);
			MODLIMIT(wroll, ANGLES_30Deg);
			roll = Angles(wroll);

			if(!TimeLeft())
			{
				movecode = AUTO_FOLLOWWP;
				information = IF_OUT_POS;
				ai.ManStep = 0;
				manoeuvretime = 0;
			}
		}
	}
	SetFlightParams();
}




void formationitem::WaitToTaxi ()
{
	movecode=AutoMoveCodeTypeSelect(GR_TruckMoveCode);
}
//////////////////////////////////////////////////////////////////////
//
// Function:    AutoTaxi
// Date:		10/06/99
// Author:		JIM
//
//Description:
//
//////////////////////////////////////////////////////////////////////
void formationitem::AutoTaxi ()
{
	if (leader && leader->waypoint==waypoint)
	{	//quick and dirty
	 	vel_x=leader->vel_x;
	 	vel_z=leader->vel_z;
	 	vel_=leader->vel_;
		hdg=leader->hdg;
		pitch=leader->pitch;
		roll=leader->roll;
		vel_y=contourlist->GetVVel(this);
		NewPosition ();
//		World.Y=leader->World.Y;
	}
	else
	{

		if (	roll != ANGLES_0Deg
			||	!TimeLeft()
			)
		{
			Coords3D p=*FindDesPos ();										//MS 28Nov96
				p.Y=World.Y;
			InterceptandRange (&p);				//MS 28Nov96

			roll=ANGLES_0Deg;

			SLong	reqvel;
			if (waypoint && waypoint->_vel)							  //JIM 17/01/00
		 		reqvel =  waypoint->_vel;
			else
				reqvel =  classtype->cruisevel;

			if (AbsAngle(hdg-HdgIntercept)>ANGLES_30Deg)
			{
				reqvel*=ANGLES_180Deg-AbsAngle(hdg-HdgIntercept);
				reqvel/=ANGLES_180Deg;
			}
			Bool	level=CalcVelSub (reqvel);
					level&=CalcHdgRollFlat ();
			pitch=ANGLES_0Deg;

			CalcXYZVel ();


			if (vel_==0)	vel_=1;									//DAW 07Dec96

			SLong	rangetime=(Range*10000.0)/vel_;				//RJS 10Jan00
			if (	Range<desposrange
				||	rangetime<=Timer_Code.FRAMETIME*4			//RDH 30Oct96
				)					//DAW 31Jul96
			{
				ExecuteWayPointAction ();
			}
			else
			{
//TempCode DAW 06Nov96 				((char*)0xB0000)[16]++;
				rangetime -=600;
				if (rangetime>30*100)
					rangetime=30*100;
				if (	level
					&&	rangetime>0)
					SetManoeuvreTime(rangetime);
			}
		}
		vel_y=contourlist->GetVVel(this);
		NewPosition ();
	}
}
//////////////////////////////////////////////////////////////////////
//
// Function:    AutoTaxiCorner
// Date:		17/03/00
// Author:		DAW
//
//Description: 	Cornering in a convoy.
//				First, move up to level with the waypoint, then turn
//				Until your heading is equal to the leader's heading
//				Note I am assuming flat ground at present!!!
//
//////////////////////////////////////////////////////////////////////
void formationitem::AutoTaxiCornerUpTo()
{
	if (leader)
	{
		NewPosition ();
		Coords3D p=*FindDesPos ();
		InterceptandRange (&p);
		if ((HdgIntercept-hdg)+ANGLES_90Deg<ANGLES_0Deg)
		{
			NextWayPoint();
			movecode=GROUND_CORNERTURN;
			//at this point I need to find speed at which this vehicle will take the turn so it stays in formation
			ANGLESInst	leadhdgbackup=leader->hdg;
			leader->hdg=ANGLES_0Deg;
			GetFollower_xyz(formation,leader);
			leader->hdg=leadhdgbackup;
			despos.X-=leader->World.X;
			//despos.X is range if leader was pointing north...	//10,000*100* = 95.89
			float	leadturncircle=vel_/(+classtype->turnpercs*(0.00009589*VEL_1MS/METRES01));
			if ((leader->hdg-hdg)>ANGLES_0Deg)
			{	//clockwise turn.
				//If despos.X is positive speed is decreased
				leadturncircle=vel_*(leadturncircle-despos.X)/leadturncircle;
			}
			else
			{	//anticlockwise turn
				leadturncircle=vel_*(leadturncircle+despos.X)/leadturncircle;
			}
			if (leadturncircle<0)
				leadturncircle=0;
			vel_=leadturncircle;
		}
	}
	else
	{
		AutoTaxi();
		movecode=GROUND_TAXI;
	}
}

void formationitem::AutoTaxiCornerTurn()
{
	CalcXYZVel ();
	NewPosition ();
	ANGLES a;
	if (!leader)
	{	//leader has an easy job
		//Stick to current vel and turn at given rate until pointing at waypoint
		Coords3D p=*FindDesPos ();
		p.Y=World.Y;
		InterceptandRange (&p);
		a=HdgIntercept-hdg;
	}
	else
		a=leader->hdg-hdg;
	int absdelta=Math_Lib.AbsSign(a);
	int maxturn=Timer_Code.FRAMETIME*ANGLES(classtype->turnpercs)/100;
	if (absdelta<maxturn)
	{
		hdg=hdg+a;
		movecode=GROUND_TAXI;
	}
	else
	{
		if (a>ANGLES_0Deg)
			hdg=Angles(hdg+maxturn);
		else
			hdg=Angles(hdg-maxturn);
	}
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GetVVel
//Author		R. Hyde
//Date			Sun 27 Oct 1996
//
//Description	Utilises a table to cache 1 tile of alt data
//				This should allow Paul to optimise his code a little.
//				I will require only 64 bytes per convoy, whereas Paul would
//				need 65K potentially and would be continually discarding.
//				The file system will also be able to discard up to 4K
//				associated with this tile.
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
ContourList *  ContourList::allocatedlist=NULL;

void	ContourList::FreeList()
{
	ContourList	*list,*next;
	list=allocatedlist;
	allocatedlist=NULL;
	while(list)
	{
		next=list->allocatednext;
		delete list;
		list=next;
	}
}

SLong	ContourList::GetVVel(FormationItemPtr	it)
{
	return 0;	//ASSUME ALL CONVOYS IN BoB ARE FLAT!!!
}




//////////////////////////////////////////////////////////////////////
//
// Function:    AssignFuelTruck
// Date:		12/06/00
// Author:		JIM
//
//Description:	Find a live, parked fuel truck
//				and set it up to refuel this group.
//				If there are no trucks at the airfield just return true.
//				If all trucks are busy or dead then return false.
//
//////////////////////////////////////////////////////////////////////
bool	MoveAirStruc::AssignFuelTruck()
{
	bool gotany=false;
	UniqueID home=ai.homebase->uniqueID.count;
	FormationItemPtr assignedtruck=NULL;
	for (int u=RefuellerBAND;u<RefuellerBANDEND;u++)
	{
		FormationItemPtr t=Persons2::ConvertPtrUID(UniqueID(u));
		if (t)
			if (t->SGT==home)
				if (t->Status.deadtime)
					gotany=true;
				else
					if (t->movecode!=AUTO_NOPPILOT)
						gotany=true;
					else
					{
						assignedtruck=t;
	/***************/	break;
					}
	}
	if (assignedtruck==NULL)
		return (!gotany);
	else
	{
		assignedtruck->movecode=GROUND_TAXI;
		WayPointPtr wp=assignedtruck->waypoint;
		FormationType* ft=&TakeOff_RefuelSpots.formationtype;
		for (int formpos=0;wp;wp=wp->next)
			if (wp->action==wpcheckfordamage)
			{
				COORDS3D*	dp=Formation_xyz(hdg,ft,formpos++);
				wp->target=this;
				wp->World=*dp;
			}
		return true;
	}

}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoLanding
//Author		Craig Beeston
//Date			Wed 21 Apr 1999
//
//Description	Brings aircraft into orbit around there home airfield, loose height
//				and loose speed, one at a time approach and land along takeoff line
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoLanding()
{
	const int flightpos = position();
	int ApproachRad = 50000;		//r = R * Sin(ApproachAng) / (1 + Sin(ApproachAng)) * 1.02ish
	SWord ApproachAng	= (SWord)ANGLES_30Deg;

	SWord ApproachZone= (SWord)ANGLES_2_5Deg;


	int OrbitRadius = 150000 + (formpos & 0xf0) * 100;
	int	OrbitAlt	= 30000  + (formpos & 0x0f) * 750;

	if(classtype->aerobaticfactor == AEROBATIC_LOW)
	{
		ApproachRad *= 2;
		OrbitRadius *= 2;
		OrbitAlt    *= 2;
	}

	const SWord dang = (ANGLES_90Deg * Timer_Code.FRAMETIME) / 100;

//#define PRINT_LANDING_DATA
#ifdef PRINT_LANDING_DATA
	PrintVar(30, 3 + flightpos, "Phase %.0f ", (FP)ai.ManStep);
	PrintVar(40, 3 + flightpos, "Alt %.1f ", (FP)((World.Y - ai.homebase->World.Y) * 0.01));
	PrintVar(50, 3 + flightpos, "Vel %.1f ", (FP)(vel * 0.1));
	PrintVar(60, 3 + flightpos, "Pitch %.1f ", (FP)(pitch / 182.04));
	PrintVar(70, 3 + flightpos, "Hdg %.1f ", (FP)(hdg / 182.04));
#endif

	switch(ai.ManStep)
	{
		case 0:	//orbit, loose height, loose speed
		{
			COORDS3D runway0;
			GetTakeOffPoint(TP_SQUADRONSTART, runway0);
			despos   = runway0;
			despos.Y = World.Y;
			InterceptandRange(&despos);

			{
				AutoOrbit(despos, OrbitRadius);

				SLong DeltaAlt = World.Y - runway0.Y;
				if(DeltaAlt > OrbitAlt + 1000)		{PitchIntercept = ANGLES_10Deg; PitchIntercept = -PitchIntercept;}
				else if(DeltaAlt < OrbitAlt - 1000)	 PitchIntercept = ANGLES_10Deg;
				else								 PitchIntercept = ANGLES_0Deg;

				AutoCalcPitch();
				SLong OrbitVel = (2 * classtype->landingspeed);
				if(		(classtype->aerobaticfactor != AEROBATIC_LOW)
					||	(Distance3DSquared(&Persons2::PlayerGhostAC->World) < FP(METRES2000) * FP(METRES2000))	)
					if(CalcVelAlt(OrbitVel))
						if((PitchIntercept == ANGLES_0Deg) && (pitch == ANGLES_0Deg))
							ai.ManStep = 1;			//Ready to start landing approach

				CalcXYZVel();
				NewPosition ();
				SetFlightParams ();
			}
			break;
		}

		case 1:	//orbit and wait for a gap to land
		{
			COORDS3D runway0, runway1;
			GetTakeOffPoint(TP_SQUADRONSTART, runway0);
			GetTakeOffPoint(TP_RUNWAYEND, runway1);

			despos   = runway0;
			despos.Y = World.Y;

			AutoOrbit(despos, OrbitRadius);

			CalcXYZVel();
			NewPosition ();
			SetFlightParams ();

			InterceptandRange(&runway0, &runway1);
			SWord RunwayHdg = HdgIntercept;
			InterceptandRange(&runway0);
			SWord HdgError = (SWord)HdgIntercept - RunwayHdg - ApproachAng;
			if((HdgError > -ApproachZone) && (HdgError < ApproachZone))
			{
				if(!AnyoneInPhaseN(2))
				{
					ai.ManStep = 2;	//

					if(fly.numinsag)
						SetManoeuvreTime(3000);
					else if(classtype->aerobaticfactor == AEROBATIC_LOW)
						SetManoeuvreTime(512);
					else
						SetManoeuvreTime(256);
				}
			}
			break;
		}

		case 2:	//Tight turn onto runway	//Timed
		case 3:	//Tight turn onto runway
		{
			if(ai.ManStep == 2)
			{
				if(!fly.numinsag)
				{
					AircraftAnimData* adptr = (AircraftAnimData*)Anim;
					if((fly.pModel->Type == AT_SPITFIRE) || (fly.pModel->Type == AT_HURRICANE) || (fly.pModel->Type == AT_JU87B))
						{
							SWord temp = 256 - manoeuvretime;
							MODMAXMIN(temp, 0, 255);
							adptr->accanopy = temp;
						}

					if(fly.pModel->Type != AT_JU87B)
					{
						SWord temp = adptr->acleglowerl - Timer_Code.FRAMETIME;
						MODMAXMIN(temp, adptr->LEFTWHEEL, 255);
						adptr->acleglowerl = temp;
						temp = adptr->acleglowerr - Timer_Code.FRAMETIME;
						MODMAXMIN(temp, adptr->RIGHTWHEEL, 255);
						adptr->acleglowerr = temp;
					}
				}
				if(!TimeLeft())
				{
					ai.ManStep = 3;						//Let the next guy into landing approach
					SetManoeuvreTime(256);		//Start putting flaps down
				}
			}
			else if((!fly.numinsag) && (TimeLeft()))
			{
				AircraftAnimData* adptr = (AircraftAnimData*)Anim;
				adptr->acflaps = -127 + manoeuvretime / 2;
			}

			COORDS3D runway0, runway1;
			GetTakeOffPoint(TP_SQUADRONTOUCHDOWN, runway0);
			GetTakeOffPoint(TP_RUNWAYEND, runway1);

			InterceptandRange(&runway0, &runway1);
			SWord RunwayHdg = HdgIntercept;

			runway0.Y = fly.pModel->GroundHeight;
			InterceptandRange(&runway0);
			AutoCalcPitch();

			COORDS3D delta;
			delta.X = runway0.X - World.X;
			delta.Y = runway0.Y - World.Y;
			delta.Z = runway0.Z - World.Z;
			RotateToHdg(delta, delta, Angles(RunwayHdg));
			SWord dhdg = SWord(hdg) - RunwayHdg;
			Float sin, cos;
			Math_Lib.high_sin_cos(Angles(dhdg), sin, cos);
			SLong rad = delta.X / (1 - cos);

			SWord desroll = -Math_Lib.arctan((vel_ / 10000) * (vel_ / 10000), (FP)(GRAVITY * rad));
			MODLIMIT(desroll, ANGLES_90Deg);
			if((desroll > 0) && (desroll < ANGLES_5Deg))
				desroll = ANGLES_5Deg;
			if((desroll < 0) && (desroll > 0 - ANGLES_5Deg))
				desroll = 0 - ANGLES_5Deg;
			SimpleMoveToRoll(desroll);
			hdg += CalcHdg();

			if(vel_ > classtype->landingspeed)
				AddAccel2Vel(-1000);
			CalcXYZVel();
			NewPosition ();
			SetFlightParams ();

			SWord HdgError = SWord(hdg) - RunwayHdg;
			if(HdgError < 0) HdgError = -HdgError;
			if(HdgError < ANGLES_HalfDeg)
			{
				hdg = (Angles)RunwayHdg;
				ai.ManStep = 4;
			}
			break;
		}

		case 4:	//Straight approach onto runway with sideslide to get on line
		{
			if(vel_ > classtype->landingspeed)
				AddAccel2Vel(-1000);

			COORDS3D runway0;
			GetTakeOffPoint(TP_SQUADRONTOUCHDOWN, runway0);
			InterceptandRange(&runway0);
			runway0.Y = fly.pModel->GroundHeight + 750;
			AutoCalcPitch();

			COORDS3D GlobDelta, LocDelta;

			GlobDelta.X = runway0.X - World.X;
			GlobDelta.Y = runway0.Y - World.Y;
			GlobDelta.Z = runway0.Z - World.Z;

			RotateToHdg(GlobDelta, LocDelta, hdg);

			{
				CalcHdgRoll();
				SLong desroll = 4 * LocDelta.X;

				MODLIMIT(desroll, ANGLES_45Deg);
				SimpleMoveToRoll((SWord)desroll);
				SWord VelHdg = (SWord)hdg + (2 * (SWord)roll) / 10;		//Direction of travel - different from aircraft hdg

				SLong despitch = (LocDelta.Y * ANGLES_FRACT) / Range;
				MODLIMIT(despitch, ANGLES_15Deg);
				PitchIntercept = (Angles)despitch;
				AutoCalcPitch();

				Float SinPitch, CosPitch, SinHdg, CosHdg;;
				Math_Lib.high_sin_cos (pitch,  SinPitch, CosPitch);
				Math_Lib.high_sin_cos ((Angles)VelHdg,  SinHdg, CosHdg);

				vel_y			= vel_ * SinPitch;
				SLong velhori	= vel_ * CosPitch;
				vel_x			= velhori * SinHdg;
				vel_z			= velhori * CosHdg;
				if((this != Persons2::PlayerSeenAC) && (this != Persons2::PlayerGhostAC))
					fly.rudder  = -3 * (SWord)roll;
			}

			NewPosition ();
			SetFlightParams ();

			int TimeToLevelOut = -(SWord)pitch / 10;	//cs
			int HeightLoss = -TimeToLevelOut * vel_y / 20000;	//cm
			HeightLoss = HeightLoss * 11 / 10;		//10% correction for Rounding/integration errors

			if(-LocDelta.Y < HeightLoss + 250)	//pull out to level at 2.5 metres
				ai.ManStep = 5;					//Level out, flare and land

			break;
		}

		case 5:	  //Level Out and flare
		{

			PitchIntercept = ANGLES_10Deg;
			if(pitch < PitchIntercept)
				pitch += 10 * Timer_Code.FRAMETIME;
			fly.cpitch = pitch;
			if(vel_ > 3 * classtype->landingspeed / 4)
				AddAccel2Vel(-1000);
			CalcXYZVel();

			if(SWord(pitch) > ANGLES_0Deg)	//Stomp vely to force AC onto ground
			{
				if(World.Y > fly.pModel->GroundHeight + classtype->deckshunt)
					vel_y = -10000;
				else
				{
					vel_y = 0;	//Touched Down
					World.Y = fly.pModel->GroundHeight + classtype->deckshunt;

						ai.ManStep = 6;		//Nose wheel down and slow down
					SetManoeuvreTime(1500);

					if(this == Persons2::PlayerSeenAC)
						Art_Int.exittimer = 1000;							//CSB 15Nov00

					if(!fly.numinsag)
					{
						AircraftAnimData* adptr = (AircraftAnimData*)Anim;
						if((classtype->visible != JU87) && ((adptr->acleglowerl != 0) || (adptr->acleglowerr != 0)))
						{
							movecode = AUTO_CRASHONEWHEEL;
							ai.ManStep = PHASE0;
						}
						else if((adptr->LEFTWINGIN > BS_DAMLV2) || (adptr->RIGHTWINGIN > BS_DAMLV2))
						{
							SWord rnd = Math_Lib.rnd(4);
							if(rnd == 0)
							{
								movecode = AUTO_CRASHNOSE;
								ai.ManStep = 0;
							}
							else if(rnd == 1)
							{
								movecode = AUTO_CRASHFLIP;
								ai.ManStep = 0;
							}
						}
					}
				}
			}
			NewPosition ();
			SetFlightParams ();
			break;
		}

		case 6:	 //Straight along runway lowering the nose and slowing down
		{
			if(pitch > classtype->deckangle)
			{
				pitch -= 10 * Timer_Code.FRAMETIME;
				if(pitch < classtype->deckangle) pitch = classtype->deckangle;
			}
			if(pitch < classtype->deckangle)
			{
				pitch += 10 * Timer_Code.FRAMETIME;
				if(pitch > classtype->deckangle) pitch = classtype->deckangle;
			}


			FP desvel = manoeuvretime / 1500.0;
			if(!fly.numinsag)
			{
				AircraftAnimData* adptr = (AircraftAnimData*)Anim;
				if(adptr->ENGINELEFT < BS_DAMLV2)
					adptr->acrpm = classtype->maxrpm * desvel + classtype->minrpm * (1.0 - desvel);
			}
			desvel = classtype->landingspeed * desvel + 100000 * (1.0 - desvel);
			vel_ = desvel;

			CalcXYZVel();
			vel_y = 0;

			int oldgroundalt = 0;
			if(!fly.numinsag)
				oldgroundalt = fly.pModel->GroundHeight;

			NewPosition (false);
				if(!fly.numinsag)
				{
					World.Y = fly.pModel->GroundHeight + classtype->deckshunt;
					FP deltapitch = FP(fly.pModel->GroundHeight - oldgroundalt) * 10430.9 * 10000.0 / FP(vel_ * Timer_Code.FRAMETIME);
					PitchIntercept = classtype->deckangle + Angles(SWord(deltapitch));
					AutoCalcPitch();
				}

			SetFlightParams ();

			if(!TimeLeft())
				ai.ManStep = 7;	//Turn off runway towards TP_TAXI1

			break;
		}

		case 7:  //Turn off runway towards TP_TAXI1
		case 8:  //Turn off runway towards TP_TAXI2
		case 9:  //Turn off runway towards TP_TAXI3
		case 10:  //Turn towards back of takeoff group
		{
			if(ai.ManStep == 7)
				GetTakeOffPoint(TP_TAXI1, despos);
			else if(ai.ManStep == 8)
				GetTakeOffPoint(TP_TAXI2, despos);
			else if(ai.ManStep == 9)
				GetTakeOffPoint(TP_TAXI3, despos);
			else if(ai.ManStep == 10)
				GetTakeOffPoint(TP_SQUADRONTURN, despos);

			despos.Y = fly.pModel->GroundHeight + classtype->deckshunt;
			InterceptandRange(&despos);

			SWord deltahdg = HdgIntercept - hdg;
			if(deltahdg > dang)			{	hdg += dang;		fly.rudder =  32767;	}
			else if(-deltahdg > dang)	{	hdg -= dang;		fly.rudder = -32767;	}
			else						{	hdg = HdgIntercept;	fly.rudder = 0;			}

			CalcXYZVel();
			vel_y = 0;

			int oldgroundalt = 0;
			if(!fly.numinsag)
				oldgroundalt = fly.pModel->GroundHeight;

			NewPosition (false);
			if(!fly.numinsag)
			{
				World.Y = despos.Y;
				FP deltapitch = FP(fly.pModel->GroundHeight - oldgroundalt) * 10430.9 * 10000.0 / FP(vel_ * Timer_Code.FRAMETIME);
				PitchIntercept = classtype->deckangle + Angles(SWord(deltapitch));
				AutoCalcPitch();
			}

			SetFlightParams ();

			if(Range < METRES10)
			{
				if(ai.ManStep == 10)
					ai.ManStep = 11;	//Turn towards centre of takeoff group
				if(ai.ManStep == 9)
					ai.ManStep = 10;	//Turn towards back of takeoff group
				else if(ai.ManStep == 8)
					ai.ManStep = 9;		//Turn towards TP_TAXI3
				else if(ai.ManStep == 7)
					ai.ManStep = 8;		//Turn towards TP_TAXI2
			}

			break;
		}

		case 11:  //Turn towards centre of takeoff group
		{
			GetTakeOffPoint(TP_FLIGHTTURN, despos);
			despos.Y = fly.pModel->GroundHeight + classtype->deckshunt;
			World.Y = despos.Y;
			InterceptandRange(&despos);

			SWord deltahdg = HdgIntercept - hdg;
			if(deltahdg > dang)			{	hdg += dang;		fly.rudder =  32767;	}
			else if(-deltahdg > dang)	{	hdg -= dang;		fly.rudder = -32767;	}
			else						{	hdg = HdgIntercept;	fly.rudder = 0;			}

			if(vel_ > 50000)
				AddAccel2Vel(-2500);
			if(Range < 500)
			{
				vel_ = 100 * Range;
				if(Range < 100)
					ai.ManStep = 12;	//Turn towards Parking Pos
			}

			CalcXYZVel();
			vel_y = 0;
			NewPosition (false);
			SetFlightParams ();
			break;
		}

		case 12:  //Turn back towards parking spot
		{
			GetTakeOffPoint(TP_ACPARK, despos);
			despos.Y = fly.pModel->GroundHeight + classtype->deckshunt;
			World.Y = despos.Y;
			InterceptandRange(&despos);

			SWord deltahdg = HdgIntercept - hdg;
			if(deltahdg > dang)			{	hdg += dang;		fly.rudder =  32767;	}
			else if(-deltahdg > dang)	{	hdg -= dang;		fly.rudder = -32767;	}
			else						{	hdg = HdgIntercept;	fly.rudder = 0;			}
			if((hdg == HdgIntercept) && (vel_ < 50000))
				AddAccel2Vel(2000);

			if(Range < 500)
			{
				vel_ = 100 * Range;
				if(Range < 50)
					ai.ManStep = 13;	//Turn towards end of runway and stop engine
			}

			CalcXYZVel();
			vel_y = 0;
			NewPosition (false);
			SetFlightParams ();
			break;
		}

		case 13:  //Turn back towards takeoff direction and stop
		{
			COORDS3D point1, point2;
			GetTakeOffPoint(TP_SQUADRONSTART, point1);
			GetTakeOffPoint(TP_RUNWAYEND, point2);
			InterceptandRange(&point1, &point2);

			SWord deltahdg = HdgIntercept - hdg;
			if(deltahdg > dang)			{	hdg += dang;		fly.rudder =  32767;	}
			else if(-deltahdg > dang)	{	hdg -= dang;		fly.rudder = -32767;	}
			else
			{
				hdg = HdgIntercept;
				fly.rudder = 0;
				ai.ManStep = 14;
				SetManoeuvreTime(255);
			}

			vel_  = 0;
			vel_x = 0;
			vel_y = 0;
			vel_z = 0;
			SetFlightParams ();
			break;
		}

		case 14:  //Stop Engine and raise flaps
		{
			if(!fly.numinsag)
			{
				AircraftAnimData* adptr = (AircraftAnimData*)Anim;
				adptr->acflaps = -manoeuvretime / 2;
				fly.thrustpercent = 0;
				fly.rpm  = (classtype->minrpm * manoeuvretime) / 255;
				fly.rpm1 = (classtype->minrpm * manoeuvretime) / 255;
				adptr->acrpm  = fly.rpm;
				adptr->acrpm1 = fly.rpm1;
			}
			if(!TimeLeft())
			{
				fly.rpm  = 0;
				fly.rpm1 = 0;

				if(!fly.numinsag)
				{
					AircraftAnimData* adptr = (AircraftAnimData*)Anim;
					adptr->acflaps = 0;
					adptr->acrpm  = 0;
					adptr->acrpm1 = 0;
				}

				if(nationality == NAT_RAF)
				{
					ai.ManStep = 15;
					SetManoeuvreTime(100);
				}
				else
					ai.ManStep = 18;
			}
			break;
		}

		case 15:	//Waiting to refuel - no truck assigned!
		{
			if(!TimeLeft())
			{
				if (formpos == 0)
					if (AssignFuelTruck())
					{
						ai.ManStep = 16;
						SetManoeuvreTime(6000);
					}
					else
						SetManoeuvreTime(1500);
				else
					if (fly.leadflight->movecode==movecode && fly.leadflight->ai.ManStep==16)
					{
						ai.ManStep = 16;
						SetManoeuvreTime(fly.leadflight->manoeuvretime+ +formpos);
					}
			}
			break;
		}

		case 16:	//refuelling / rearming
		{
			if(!TimeLeft())
			{
				if(fly.fuel_content[0] < 1000 * classtype->fueltankcap[0])
				{
					for(SWord i = 0; i < 4; i++)
						fly.fuel_content[i] = 1000 * classtype->fueltankcap[i];
					SetManoeuvreTime(60000);
				}
				else
				{
					Trans_Obj.ReloadWeapons(this);
					ai.ManStep = 16;
					SetManoeuvreTime(1000);
				}
			}
			break;
		}

		case 17:
		{
			if((formpos == 0) && (!TimeLeft()))
			{
				bool everyoneready = true;
				AirStrucPtr lead, ac;
				for(lead = this; lead; lead = lead->fly.nextflight)
					for(ac = lead; ac; ac = ac->Follower())
						if((ac->movecode != AUTO_LANDING) || (ac->ai.ManStep != 17))
							everyoneready = false;

				if(everyoneready)
					for(lead = this; lead; lead = lead->fly.nextflight)
						for(ac = lead; ac; ac = ac->Follower())
							if (!ac->AircraftDamaged())
							{
								ac->movecode = AUTO_TAKEOFF;
								ac->ai.ManStep = 0;
								ac->SetManoeuvreTime(0);

								const COORDS3D offsetE = { 1000000, 0, -1000000};
								const COORDS3D offsetW = {-1000000, 0, -1000000};
								ac->waypoint->target->World.X = ac->waypoint->World.X + offsetE.X;
								ac->waypoint->target->World.Y = ac->waypoint->World.Y + offsetE.Y;
								ac->waypoint->target->World.Z = ac->waypoint->World.Z + offsetE.Z;
							}
							else
							{}
				else
					SetManoeuvreTime(1000);
			}
			break;
		}

		case 18:
		{
			//Do Nothing !!!
			break;
		}
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoStraffe
//Author		Mark Shaw
//Date			Fri 18 Oct 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoStraffe()
{
/*	UWord	mvel,mdelay,mburst;									//RDH 31Jul98
	if (ai.ManStep==0)
	{
		ai.ManStep=1;
		int	index;
		for (index = 0; index < 6; index++)						//RDH 31Jul98
		{
			WeapAnimData*	weapon;										//RJS 29Aug96
			SLong			xpos, ypos, zpos;							//RJS 29Aug96
			weapon = SHAPE.GetWeaponLauncher(this,index,xpos,ypos,zpos,mvel,mdelay,mburst,LT_BOMB);//RDH 31Jul98
			if (weapon && weapon->LoadedStores > 0)
			{
				ai.ManStep=2;
			}

		}
	}

	if (!TimeLeft())
	{
		if (!SHAPE.GetLiveElementPos(formpos&InWingMAX,waypoint->target,despos.X,despos.Y,despos.Z))
			SHAPE.GetLiveElementPos(0,waypoint->target,despos.X,despos.Y,despos.Z);
		despos.X+=waypoint->target->World.X;
		despos.Y+=waypoint->target->World.Y;
		despos.Z+=waypoint->target->World.Z;
		InterceptandRange (&despos);
		if ((HdgIntercept-hdg-ANGLES_90Deg<<ANGLES_180Deg))
		{
			if (Range<METRES1000)
			{	//flying away
				HdgIntercept=hdg;
				PitchIntercept=ANGLES_15Deg;
				if (ai.ManStep==3)
					ai.ManStep=0;
			}
			else
			{
				if (waypoint->target->Status.deadtime)
					if (waypoint->target->Status.size>=FORMATIONSIZE)
					{
						FormationItemPtr f=*waypoint->target;
						if (f->follower)
							waypoint->target=f->follower;
						else
							Art_Int.BreakOff(this);
					}
					else
						Art_Int.BreakOff(this);
				else
					if (roll==ANGLES_0Deg)
						if (Math_Lib.rnd()<RND25PC)
							Art_Int.BreakOff(this);

			}

			Bool
				level=CalcVelSub (classtype->maxvel);
				level&=CalcHdgRoll ();
				level&=AutoCalcPitch ();
			if (level)
	 			SetManoeuvreTime(FIVESECS100);
		}
		else
		{	//flying towards
			//mod pitchintercept based on range
			ANGLES	pdiff=PitchIntercept-pitch;
			if (ai.ManStep==2)
				pdiff=PitchIntercept-fly.cpitch+ANGLES_2Deg;

			if (	Range<METRES350 ////////&& (Timer_Code.msgcounttime &8)==0
				&&	ai.ManStep<3
				&&	pdiff<ANGLES_0Deg
				)
			{
				int	index=4;
				if (ai.ManStep==2)
				for (index = 0; index < 4; index++)
				{
					WeapAnimData*	weapon;										//RJS 29Aug96
					SLong			xpos, ypos, zpos;							//RJS 29Aug96
					weapon = SHAPE.GetWeaponLauncher(this,index,xpos,ypos,zpos,mvel,mdelay,mburst,LT_BOMB);//RDH 31Jul98
					if (weapon && weapon->LoadedStores > 0)
					{
						weapon->LoadedStores--;
						Trans_Obj.LaunchBombDrop(this,BOMB,xpos,ypos,zpos,*currworld);//RJS 28Aug96
						ai.ManStep=3;
						break;
					}

				}
				elseif (ai.ManStep==1)
				{
					int	timeleft = weap.ShootDelay-Timer_Code.FRAMETIME;//DAW 31Jul96
					if (timeleft<=0)
					{
						for (index = 0; index < 4; index++)		//JIM 05Dec96
						{
							WeapAnimData*	weapon;										//RJS 29Aug96
							SLong			xpos, ypos, zpos;							//RJS 29Aug96
							weapon = SHAPE.GetWeaponLauncher(this,index,xpos,ypos,zpos,mvel,mdelay,mburst,LT_BULLET);//RDH 31Jul98
						 	weap.ShootDelay = 10;
							if (   (weapon)						//JIM 05Dec96
								&& (weapon->LoadedStores > 0)
								)
							{
			 						weapon->LoadedStores--;
									Trans_Obj.LaunchOneGunBullet(this,mvel,BULLET,xpos,ypos,zpos,*currworld);
							}
						}
					}
					else
						weap.ShootDelay = timeleft;

			}	}
			if (Range>METRES600)
			{	//1m height diff=1/2 deg
				int delth;
				if (ai.ManStep==2)
					delth=-(World.Y-despos.Y-METRES350);		//JIM 04Dec96
				else
					delth=-(World.Y-despos.Y-METRES450);
				if (delth>ANGLES_60Deg)	delth=ANGLES_60Deg;
				if (0-delth>ANGLES_60Deg)	delth=0-ANGLES_60Deg;
				HdgIntercept+=(Angles)((formpos&InWingMAX)*ANGLES_5Deg);
				PitchIntercept=(Angles)(delth+(formpos&InWingMAX)*ANGLES_HalfDeg);
			}
			elseif (Range<METRES100)
				PitchIntercept=ANGLES_20Deg;
			else
			{
				int	pim=(Range-METRES250)>>5;			//metres200=20K=110deg./16->7 deg
				PitchIntercept-=(Angles)pim;
			}
			Bool
				level=CalcVelSub (classtype->cruisevel);	//20mph above cruise
				level&=CalcHdgRoll ();
				level&=AutoCalcPitch ();
			if (level && Range>MILES01)
	 			SetManoeuvreTime(SECS100);
		}
	 	CalcXYZVel();
	}
	NewPosition ();
	SetFlightParams ();
*/
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CheckBombingStatus
//Author		Jim
//Date			26 Mar 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::CheckBombingStatus()
{
	AirStrucPtr lead=this;
	if (lead->Leader())
		lead=lead->Leader();
	if (lead->formpos)
		lead=lead->fly.leadflight;
	else
	{	//for all aircraft in formation, if any are still bombing then quick quit
		for (;lead;lead=lead->fly.nextflight)
		{
			for (AirStrucPtr foll=lead;foll;foll=foll->Follower())
				if (foll->movecode==AUTO_BOMB)
					return;
		}
		//ok so nobody is bombing.
		//next job: get status of target.
		if (EnoughDamage3D(ai.unfriendly))
			_Miles.SequenceAudible(FIL_MUSIC_OBJECTIVE_COMPLETED);
		else
			_Miles.SequenceAudible(FIL_MUSIC_OBJECTIVE_FAILED);
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoBomb
//Author		Mark Shaw				CSB
//Date			Fri 18 Oct 1996			25 Mar 1999

//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoBomb()
{
	if((waypoint) && (waypoint->wpname > 7))
		INT3;

	Bool IsLeader = TRUE;
	if((formpos != 0) || ((waypoint->World.Y > 200000) && (fly.leadflight)) )
		IsLeader = FALSE;

	if(ai.unfriendly == NULL)
	{
		if(IsLeader)
		{
			if((waypoint) && (waypoint->target))
				ai.unfriendly = *waypoint->target;
		}
		else
		{
			if((waypoint) && (waypoint->target))
				ai.unfriendly = *waypoint->target;

			if(ai.unfriendly == NULL)
				if((leader) && (Leader()->ai.unfriendly))
					ai.unfriendly = Leader()->ai.unfriendly;

			if(ai.unfriendly == NULL)
				if((fly.leadflight) && (fly.leadflight->ai.unfriendly))
					ai.unfriendly = fly.leadflight->ai.unfriendly;

			if(ai.unfriendly == NULL)
				INT3;
		}
	}


	if(classtype->aerobaticfactor == AEROBATIC_LOW)
		AutoBombHigh();
	else
		AutoBombDive();

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoBombHigh
//Author		Mark Shaw
//Date			Fri 18 Oct 1996
//
//Description	High Level Bombimg run
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoBombHigh()
{
	if((formpos != 0) || ((waypoint->World.Y > 200000) && (fly.leadflight)) )
	{
		if(ai.ManStep == 0)
			ai.ManStep = 1;
		AutoBombFollow();
	}
	else
	{
		if(ai.ManStep == 0)
		{
			if((!CheckForWeapon(LT_BOMB)) && (!CheckForWeapon(LT_NAPALM)))
				ai.ManStep = 15;	//Finish bombimg - climbing circle
			else
			{
				SHAPE.BombBayDoors(this,ANIM_BOMBDOORS_OPEN);		//RJS 28Jun99
				ai.ManStep = 1;

				AirStrucPtr callee = Follower();
				if(!callee)
					callee = fly.nextflight;
				if(callee)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_STARTINGTRGTRUN, MSG_SECONDRUN , *this, NULL, callee));
			}
		}

		if (!TimeLeft())
			if(ai.ManStep >= 13)		   //Stop bombing and move to next waypoint
			{
				if(follower)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CLEARINGAREA, MSG_CLEARINGAREA, *this, NULL, Follower()));
				waypoint = waypoint->NextWP(this);
				movecode = AUTO_FOLLOWWP;
				information = IF_OUT_POS;					//CSB 22Aug00
				manoeuvretime = 0;
				ai.ManStep = 0;

				Trans_Obj.RaidEnd(ai.unfriendly);			//RJS 09Mar00

				ai.unfriendly = NULL;
				SHAPE.BombBayDoors(this,ANIM_BOMBDOORS_CLOSE);	//RJS 28Jun99
			}
			else
			{
				SLong dy = World.Y - ai.unfriendly->World.Y;
				FP falltime = 4.2 * FSqrt(dy) + 0.0015 * dy;		//Drag and Gravity
				SLong wx, wy, wz;
				MissManCampSky().GetWind(World.Y, wx, wy, wz);


				SelectedElement(ai.unfriendly, 1, &despos);				//RJS 27Jun00	//select the 1st element of the group
				despos.X = despos.X - falltime * wx * 0.0001;	  //Wind
				despos.Y = World.Y  - falltime * wy * 0.0001;
				despos.Z = despos.Z - falltime * wz * 0.0001;

				InterceptandRange (&despos);
				{
					FP f1 = -5.056e-9 * vel_;
					FP f2 =  9.421e-5 * vel_;

					FP ReqRange = f1 * falltime * falltime + f2 * falltime;		//Drag

					if(Range < ReqRange - 1000)	//10 metres too close to drop bombs
					{
						HdgIntercept = hdg;
		 				CalcVelAlt (classtype->cruisevel);
						Bool level = CalcHdgRoll ();
						level&=AutoCalcPitch ();
						if(level)
							SetManoeuvreTime(falltime * 3);
					}
					else
					{
						AutoCalcPitch ();
						CalcVelAlt (classtype->cruisevel);
						CalcHdgRoll();
						UWord reqdeltahdg_pos;
						SWord reqdeltahdg_sign;
						SWord reqdeltahdg = SWord(HdgIntercept - hdg);
						Math_Lib.Pos(reqdeltahdg, reqdeltahdg_pos, reqdeltahdg_sign);
						if((reqdeltahdg_pos < ANGLES_1Deg) && (Range < ReqRange + 1000))	//10 metres too far to drop bombs
						{	//drop bomb now!
							Trans_Obj.RaidAlert(ai.unfriendly);		//RJS 09Mar00
							AirStrucPtr AFollower = FindABuddyWithPlayerGivenPriority();
							if((waypoint->World.Y > 200000) && (AFollower))
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_DROPBOMBS_L, MSG_STARTINGTRGTRUN, *this, NULL, AFollower));
							SetManoeuvreTime(falltime);

							{
								{
									Trans_Obj.DropOneBomb((mobileitem*)this, *currworld);
									ai.ManStep = 13;
								}
							}
						}
					}
				}
				CalcXYZVel();
			}

		if(ai.ManStep == 13)
			if(Trans_Obj.DropOneBomb((mobileitem*)this, *currworld) == 0)
				ai.ManStep = 14;

		NewPosition ();
		SetFlightParams ();
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoBombLow
//Author		Mark Shaw
//Date			Fri 18 Oct 1996
//
//Description	Low Level Bombing Run	eg. Napalm
//				If attack pattern is WIDE only the group leader gets past phase 0
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoBombLow()
{
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoBombDive
//Author		Mark Shaw
//Date			Fri 18 Oct 1996
//
//Description	Dive Bombing Attack
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	MoveAirStruc::AutoBombDive()
{
	if((!ai.unfriendly) && (waypoint) && (waypoint->target))
		ai.unfriendly = waypoint->target;

	ItemPtr thetarget = ai.unfriendly;

	if(World.Y > 250000)
		if(AirStrucPtr escortee = FindGroupLeader()->fly.leadflight)
			if(escortee->classtype->aerobaticfactor == AEROBATIC_LOW)
			{
				AutoBombFollow();
				return;
			}

	const int flightpos = position();

	switch(ai.ManStep)
	{
		case 0:	//Wait for the target to be available
		{
			if((formpos) || ((fly.leadflight) && (fly.leadflight->classtype->aerobaticfactor == AEROBATIC_LOW)))
				AutoBombFollow();
			else
			{
				int MaxOrbitAlt = ai.unfriendly->World.Y + 150000;
				if((waypoint) && (waypoint->World.Y > MaxOrbitAlt)) MaxOrbitAlt = waypoint->World.Y;
				MaxOrbitAlt += 10000;
				int MinOrbitAlt = MaxOrbitAlt - 20000;
				AutoOrbit(ai.unfriendly->World, 550000);
				if(World.Y > MaxOrbitAlt)	PitchIntercept = ANGLES_320Deg;
				else
				if(World.Y < MinOrbitAlt)	PitchIntercept = ANGLES_10Deg;
				else						PitchIntercept = ANGLES_0Deg;
				AutoCalcPitch();
				CalcXYZVel();
				NewPosition ();
				SetFlightParams ();
			}
			if((!formpos) && (ai.unfriendly) && (!TimeLeft()))
			{
				bool targfree = true;
				for(int i = 0; (targfree) && (i < ArtInt::ACARRAYSIZE); i++)
					if(ArtInt::ACArray[i])
					{
						AirStrucPtr as = ArtInt::ACArray[i]->GetLeadAc();
						if(		(as->nationality == nationality)
							&&	(as->movecode == AUTO_BOMB)
							&&	(as->ai.unfriendly == ai.unfriendly)
							//&&	(as->classtype->aerobaticfactor != AEROBATIC_LOW)
							&&	((as->AnyoneInPhaseN(1)) || (as->AnyoneInPhaseN(2)))		)
							targfree = false;
					}

				if(targfree)
				{
					for(AirStrucPtr ld = this; ld; ld = ld->fly.nextflight)
						for(AirStrucPtr ac = ld; ac; ac = ac->Follower())
							ac->ai.ManStep = 1;
				}
				else
					SetManoeuvreTime(500);
			}
			break;
		}
		case 1:	//Circle around Target
		case 2: //Circle around Target - Count X seconds		//Delay from last attacker
		{
			int MaxOrbitAlt = ai.unfriendly->World.Y + 150000;
			if((waypoint) && (waypoint->World.Y > MaxOrbitAlt)) MaxOrbitAlt = waypoint->World.Y;

			MaxOrbitAlt += 10000;
			int MinOrbitAlt = MaxOrbitAlt - 20000;

			AirStrucPtr MyLeader = NULL;
			Bool CanFollow = FALSE;
			if(ai.ManStep == 1)
			{
				if(fly.leadflight)	MyLeader = fly.leadflight;
				if(leader)			MyLeader = (AirStruc*)leader;
				if((MyLeader) && (MyLeader->movecode == AUTO_BOMB) && (MyLeader->ai.ManStep == 1))// && (MyLeader->World.Y > MinOrbitAlt))
					CanFollow = TRUE;
			}
			if(CanFollow)
				AutoFollowWpWing(MyLeader);
			else
			{
				AutoOrbit(ai.unfriendly->World, 500000);
				if(World.Y > MaxOrbitAlt)	PitchIntercept = ANGLES_320Deg;
				else
				if(World.Y < MinOrbitAlt)	PitchIntercept = ANGLES_10Deg;
				else						PitchIntercept = ANGLES_0Deg;
				AutoCalcPitch();
				CalcXYZVel();
				NewPosition ();
				SetFlightParams ();
			}

			if(ai.ManStep == 1)
			{
				bool canstartattack = false;
				if(formpos == 0)
				{
					canstartattack = true;
					if(!TimeLeft())
					{
						if(EnoughDamage3D(ai.unfriendly))		//If target is completely destroyed
							for (AirStrucPtr lead = *this;lead;lead=lead->fly.nextflight) //All aircraft stop bombing and circle target
		 						for (AirStrucPtr foll=lead;foll;foll=foll->Follower())
									foll->ai.ManStep = 15;
						else
							SetManoeuvreTime(1000);
					}

				}
				else
					if((AnyoneInPhaseN(3)) && (!AnyoneInPhaseN(2)))
						canstartattack = true;

				if(canstartattack)
				{
					int ElementNumber = 0;
					int ACNumber = (int)uniqueID.count - (int)FindLeadUID();
					ACNumber &= 0x3f;
					ai.unfriendly = SelectTarget(thetarget, &ElementNumber, ACNumber);
					ai.manoeuvre = (MANOEUVRE)ElementNumber;
					ai.ManStep = 2;
					SetManoeuvreTime(256);
					if(!SelectedElement(ai.unfriendly, ElementNumber))
					{
						AirStrucPtr buddy = FindBuddy();
						if(buddy)
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_TARGETNOTIDENTIFIED, MSG_CLEAR_RPT , *this, NULL, buddy));
						ai.ManStep = 15;
					}
				}
			}

			if(ai.ManStep == 2)
				if(!TimeLeft())
					if(CheckForAnyWeapon())
					{
						ChooseAWeapon();
						ai.ManStep = 3;
						if((!formpos) && (follower))
							if(weap.weapontype == LT_BOMB)
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_STARTINGTRGTRUN, MSG_SECONDRUN , *this, NULL, Follower()));
							else
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_STRAFTARGET, MSG_SECONDRUN , *this, NULL, Follower()));
					}
					else
						ai.ManStep = 15;
			break;
		}

		case 3:	//Hard Turn - don't count
		{
			if((fly.pModel->Type != AT_JU87B) || (weap.weapontype != LT_BOMB))
			{
				Trans_Obj.RaidAlert(ai.unfriendly);		//RJS 09Mar00
				ai.ManStep = 4;
			}

			{
				information = IF_OUT_POS;
				int ElementNumber = (int)ai.manoeuvre;
				bool targetlive = SelectedElement(ai.unfriendly, ElementNumber, &despos);
				if(!targetlive)
				{
					int ElementNumber = 0;
					int ACNumber = (int)uniqueID.count - (int)FindLeadUID();
					ACNumber &= 0x3f;
					ai.unfriendly = SelectTarget(thetarget, &ElementNumber, ACNumber);
					ai.manoeuvre = (MANOEUVRE)ElementNumber;
					targetlive = SelectedElement(ai.unfriendly, ElementNumber, &despos);
					ai.manoeuvre = (MANOEUVRE)ElementNumber;
				}

				if(weap.weapontype == LT_BOMB)
				{
					SLong error = 3840 - 10 * ai.combatskill;
					if(formpos & 0x01)	despos.X += error;
					else				despos.X -= error;
					if(formpos & 0x01)	despos.Z += error;
					else				despos.Z -= error;
				}

				InterceptandRange (&despos);
				if(-SWord(PitchIntercept) > ANGLES_75_5Deg)
				{
					Trans_Obj.RaidAlert(ai.unfriendly);		//RJS 09Mar00

					ai.ManStep = 4;
					if((!formpos) && (follower))
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_STARTINGTRGTRUN, MSG_SECONDRUN , *this, NULL, Follower()));
				}

				PitchIntercept = ANGLES_0Deg;
				AutoCalcPitch ();

				UWord reqdeltahdg_pos;
				SWord reqdeltahdg_sign;
				SWord reqdeltahdg = SWord(HdgIntercept - hdg);
				Math_Lib.Pos(reqdeltahdg, reqdeltahdg_pos, reqdeltahdg_sign);
				if(reqdeltahdg_pos > ANGLES_5Deg)
				{
					SWord DesRoll = MAXCOMBATROLL;
					if(reqdeltahdg_sign == -1)
						DesRoll = -DesRoll;
					roll += RequiredDRollSub(DesRoll);
					hdg  += (Angles)(CalcHdg());
				}
				else
					CalcHdgRoll ();

				CalcVelAlt(classtype->cruisevel);
				CalcXYZVel();
				NewPosition ();
				SetFlightParams ();
			}
			break;
		}

		case 4:	//Hard Turn - dive for target - don't count
		{
			if(!fly.numinsag)
			{
				AircraftAnimData* adptr = (AircraftAnimData*)Anim;
				SWord brakes = adptr->acbreak;
				if(brakes < 255)
				{
					brakes += 2.5 * Timer_Code.FRAMETIME;
					if(brakes > 255)
						brakes = 255;
					adptr->acbreak = brakes;
				}
			}

			information = IF_OUT_POS;
			int ElementNumber = (int)ai.manoeuvre;
			bool targetlive = SelectedElement(ai.unfriendly, ElementNumber, &despos);
			if(!targetlive)
			{
				int ElementNumber = 0;
				int ACNumber = (int)uniqueID.count - (int)FindLeadUID();
				ACNumber &= 0x3f;
				ai.unfriendly = SelectTarget(thetarget, &ElementNumber, ACNumber);
				ai.manoeuvre = (MANOEUVRE)ElementNumber;
				targetlive = SelectedElement(ai.unfriendly, ElementNumber, &despos);
				ai.manoeuvre = (MANOEUVRE)ElementNumber;
			}

			if(weap.weapontype == LT_BOMB)
			{
				SLong error = 20000 - 50 * ai.combatskill;
				if(formpos & 0x01)	despos.X += error;
				else				despos.X -= error;
				if(formpos & 0x01)	despos.Z += error;
				else				despos.Z -= error;
			}

			InterceptandRange (&despos);
			CalcVelAlt(1750000);

			AutoCalcPitch ();

			UWord reqdeltahdg_pos;
			SWord reqdeltahdg_sign;
			SWord reqdeltahdg = SWord(HdgIntercept - hdg);
			Math_Lib.Pos(reqdeltahdg, reqdeltahdg_pos, reqdeltahdg_sign);
			if(reqdeltahdg_pos > ANGLES_5Deg)
			{
				SWord DesRoll = MAXCOMBATROLL;
				if(reqdeltahdg_sign == -1)
					DesRoll = -DesRoll;
				roll += RequiredDRollSub(DesRoll);
				hdg  += (Angles)(CalcHdg());
			}
			else
				CalcHdgRoll ();

			CalcXYZVel();
			NewPosition ();
			SetFlightParams ();

			//Calc Pull out point
			int TimeToLevelOut = -(SWord)pitch * 100 / classtype->maxpitchrate;	//cs
			int HeightLoss = -TimeToLevelOut * vel_y / 20000;	//cm
			HeightLoss = HeightLoss * 11 / 10;		//10% correction for Rounding/integration errors
			SWord DiveTimeLeft = 1;
			if(vel_y < 0)
			{
				FP fDiveTimeLeft = -((FP)World.Y - (FP)despos.Y - (FP)HeightLoss - 10000.0) * 10000.0 / (FP)vel_y;	//cs
				if(fDiveTimeLeft < 32767)
					DiveTimeLeft = fDiveTimeLeft;
			}

			UWord deltahdg_pos	,deltapitch_pos;
			SWord deltahdg_sign	,deltapitch_sign;

			Math_Lib.Pos((SWord)(PitchIntercept - pitch), deltapitch_pos, deltapitch_sign);
			Math_Lib.Pos((SWord)(HdgIntercept   - hdg  ), deltahdg_pos  , deltahdg_sign  );

			if(classtype->visible != JU87)
				if((deltapitch_pos < ANGLES_2_5Deg) && (deltahdg_pos < ANGLES_2_5Deg) && (targetlive))
					if((Range < (vel_ + classtype->muzzlevel) / 100) && (DiveTimeLeft > 150))
					{
						if(fly.numinsag)
							Fake_Damage.HitGroup(ai.unfriendly, ItemPtr(this), Art_Int.CountSquadronSize(this) * 2);
						else
							FireABullet(100);
					}

			if(DiveTimeLeft < 0)
				ai.ManStep = 5;	//Pull Up

			if((classtype->visible == JU87) && (World.Y - ai.unfriendly->World.Y < 70000))
				ai.ManStep = 5;

			break;
		}

		case 5:	//Pull up to 20 deg
		{
			if(SWord(pitch) > ANGLES_10Deg)
			{
				if(!fly.numinsag)
				{
					AircraftAnimData* adptr = (AircraftAnimData*)Anim;
					SWord brakes = adptr->acbreak;
					if(brakes > 0)
					{
						brakes -= 2.5 * Timer_Code.FRAMETIME;
						if(brakes < 0)
							brakes = 0;
						adptr->acbreak = brakes;
					}
				}
			}

			int ElementNumber = (int)ai.manoeuvre;
			bool targetlive = SelectedElement(ai.unfriendly, ElementNumber, &despos);
			HdgIntercept = hdg;
			PitchIntercept = ANGLES_20Deg;
			AutoCalcPitch();
			CalcHdgRoll();
			AddAccel2Vel(CalcAccel(classtype->maxvel));

			CalcXYZVel();
			NewPosition ();
			SetFlightParams ();

			if(weap.weapontype == LT_BOMB)
			{
				if(targetlive)
				{
					FP timetotarget;
					if(FP(vel_x) * FP(vel_x) > FP(vel_z) * FP(vel_z))
						timetotarget = (despos.X - World.X) * 10000.0 / vel_x;
					else
						timetotarget = (despos.Z - World.Z) * 10000.0 / vel_z;

					FP vertpos = World.Y + vel_y * timetotarget * 0.0001 - 0.5 * 0.098065 * timetotarget * timetotarget;
					if(vertpos > despos.Y)
					{
						Trans_Obj.DropOneBomb((mobileitem* )this,*currworld);	//drop bomb now!
						Trans_Obj.DropOneBomb((mobileitem* )this,*currworld);	//drop bomb now!

						if((!formpos) && (follower))
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_DROPBOMBS_L, MSG_STARTINGTRGTRUN, *this, NULL, Follower()));

					}
				}
			}

			if((SWord(pitch) > 0) && (World.Y > despos.Y + 75000))	//1500 ft
				ai.ManStep = 15;

			break;
		}

		case 15:
		{
			AirStrucPtr MyLeader = NULL;
			if(fly.leadflight) MyLeader = fly.leadflight;
			if(leader) MyLeader = (AirStruc*)leader;
			if((MyLeader) && (MyLeader->ai.ManStep == 15))// && (MyLeader->World.Y - ai.unfriendly->World.Y > 240000))
				AutoFollowWpWing();
			else
			{
				AutoOrbit(ai.unfriendly->World, 600000);	//Circle climbing to 10000 ft - outside waiting bombers
				if(World.Y - ai.unfriendly->World.Y < 300000)	PitchIntercept = ANGLES_5Deg;
				else											PitchIntercept = ANGLES_0Deg;
				AutoCalcPitch();
				CalcXYZVel();
				NewPosition ();
				SetFlightParams ();
			}

			if(formpos == 0)
			{
				bool finished = true;
				for(AirStrucPtr lead = this; (lead) && (finished); lead = lead->fly.nextflight)
					for (AirStrucPtr foll = lead; (foll) && (finished); foll = foll->Follower())
						if(foll->ai.ManStep != 15)
							finished = false;

				if(finished)
				{
					Trans_Obj.RaidEnd(thetarget);			//RJS 09Mar00

					WayPointPtr oldwaypoint = waypoint;
					for (AirStrucPtr lead = *this;lead;lead=lead->fly.nextflight) //All aircraft stop bombing and circle target
						for (AirStrucPtr foll=lead;foll;foll=foll->Follower())
							if(		((foll->movecode == AUTO_BOMB) || (foll->movecode == AUTOSAG_BOMBAPPROACH))
								&&	(foll->waypoint == oldwaypoint)	)
							{
								foll->movecode = AUTO_FOLLOWWP;
								foll->information = IF_OUT_POS;					//CSB 22Aug00
								foll->manoeuvretime = 0;
								foll->waypoint = foll->waypoint->NextWP(foll);
								foll->ai.unfriendly = NULL;
							}
				}
			}

			break;
		}

		break;
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoBombFollow
//Author		Craig Beeston
//Date			Tue 11 May 1999
//
//Description	For high or wide bombing runs.  Aircraft just run AFWPW and drop bomb.
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoBombFollow()
{
	AirStrucPtr lf = NULL;
	Bool GoToAutoFollowWp = FALSE;

	if(leader)
		lf = Leader();
	else
		lf = fly.leadflight;

	if(lf == NULL)
		INT3;

	if((lf->fly.numinsag) && (lf->fly.expandedsag))
		lf = lf->fly.expandedsag;

	AutoFollowWpWing(lf);

	switch(ai.ManStep)
	{
		case 0:
		{
			if(lf->movecode == AUTO_FOLLOWWP)
				GoToAutoFollowWp = TRUE;
			break;
		}

		case 1:
		{
			SHAPE.BombBayDoors(this,ANIM_BOMBDOORS_OPEN);		//RJS 28Jun99
			if(lf->ai.ManStep == 13)
			{
				SetManoeuvreTime(Math_Lib.rnd(512));
				ai.ManStep = 12;
				if(!formpos)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_DROPBOMBS_F, MSG_STARTINGTRGTRUN, lf, NULL, this));
			}
			if(lf->movecode == AUTO_FOLLOWWP)
				GoToAutoFollowWp = TRUE;
			break;
		}

		case 12:
		{
			if(!TimeLeft())
				ai.ManStep = 13;
			break;
		}

		case 13:
		{
			if(Trans_Obj.DropOneBomb((mobileitem* )this,*currworld) == 0)
			{
				SHAPE.BombBayDoors(this,ANIM_BOMBDOORS_CLOSE);		//RJS 28Jun99
				ai.ManStep = 5;
			}
			break;
		}

		case 5:
		{
			if(lf->movecode == AUTO_FOLLOWWP)
				GoToAutoFollowWp = TRUE;
			break;
		}
		break;
	}

	if(GoToAutoFollowWp)
	{
		movecode = AUTO_FOLLOWWP;
		information = IF_OUT_POS;					//CSB 22Aug00
		if(classtype->aerobaticfactor == lf->classtype->aerobaticfactor)
			waypoint = lf->waypoint;
		else
			waypoint = waypoint->NextWP(this);
		SetManoeuvreTime(0);
		ai.unfriendly = NULL;
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoOrbit
//Author		Craig Beeston
//Date			Wed 31 Mar 1999
//
//Description	makes an aircraft orbit a given point with a circle of given radius
//
//Inputs		Centre, Radius
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::AutoOrbit(COORDS3D centre, int radius)
{
	centre.Y = World.Y;		//ignore vertical distance
	InterceptandRange(&centre);

	ANGLESInst oldhdgint = HdgIntercept;

	FP HdgOffset = ANGLES_90Deg;

	FP RadiusCorrector = (FP)Range / (FP)radius - 1.0;
	MODLIMIT(RadiusCorrector, 1);
	HdgOffset -= RadiusCorrector * HdgOffset;
	HdgIntercept += (Angles)((SWord)HdgOffset);

	SWord reqdeltahdg = SWord(HdgIntercept - hdg);
	SWord reqdeltahdg_pos;
	if(reqdeltahdg >= 0) reqdeltahdg_pos =  reqdeltahdg;
	else				 reqdeltahdg_pos = -reqdeltahdg;
	if((reqdeltahdg_pos < ANGLES_ThirdDeg) && (RadiusCorrector > -1) && (RadiusCorrector < 1))
	{
		if(reqdeltahdg > 0) HdgIntercept = hdg + ANGLES_ThirdDeg;
		else				HdgIntercept = hdg - ANGLES_ThirdDeg;
	}
	CalcHdgRoll();

	HdgIntercept = oldhdgint;
//	SWord IdealRoll = (vel * vel) / (GRAVITY * radius);

}


int	AirStruc::ModifyHdgRangeForFormation(int dhori)
{

	if((dhori != 0) && (Range != 0))
	{
		FP fract = FP(dhori) / FP(Range);
		MODLIMIT(fract, 1.0);
		HdgIntercept += 10172.9 * fract + 2997.1 * fract * fract * fract;
	}

	return Range;
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindDesPos
//Author		R. Hyde
//Date			Tue 28 Nov 1995									//JIM 01May96
//
//Description	Find position indicated by waypoint
//				by acumulating waypoint and backupwp and waypoint target.
//				SIMILAR CODE ALERT
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------

COORDS3D* AirStruc::InterceptandRangeFindDesPosAndVel ()
{
	WaypointOffsetSub wos;
	GenWaypointOffsetSub(formation,wos,(duty&PACK_AM_MASK)==PACK_AM_ATTACHED);
	despos=*FindDesPos ();	//desired position
	despos.Y+=wos.deltavert;
	int	dtime=wos.deltatime;	 //positive means arrive early!			//JIM 25Jul00
	InterceptandRange(&despos);			//angle
	ModifyHdgRangeForFormation(wos.deltahori);
	SWord dHdg, dPitch;																//CSB 4Aug00
	SLong dVel;																		//CSB 4Aug00
	WindTrackCorrection(vel_, HdgIntercept, PitchIntercept, dVel, dHdg, dPitch);	//CSB 4Aug00
	HdgIntercept   += dHdg;															//CSB 4Aug00
	PitchIntercept += dPitch;														//CSB 4Aug00

	if (waypoint)
	{
		if (!waypoint->ETA)
		{	//no ETA so work out an eta give the vel...
			int	reqvel=waypoint->_vel;
			if (reqvel==0)
				reqvel = 0.85 * CalcMaxVelSL();
			int reqtime=(Range*10000.0/reqvel);
			ai.DesEta()=
				waypoint->ETA=	//main raid to arrive x seconds after we do if dtime+ve
					reqtime+(timeofday+dtime);							//JIM 25Jul00
// recalculate reqvel to take into account truncation when float is assigned to int
// required for replay purposes specifically							//AMM 17Jul00

			reqvel=(Range*10000.0/reqtime);									//AMM 17Jul00

			ai.DesVel()=
				reqvel;
			ai.LastClipped()=NULL;
		}
		else if (waypoint->ETA!=ai.DesEta())
		{
			int dt=waypoint->ETA-(timeofday+dtime);						//JIM 25Jul00
			if (dt)
			{
				int	reqvel=(Range*10000.0/dt);
				reqvel += dVel;											//CSB 4Aug00

				if(!((classtype->visible == ME109) && (reqvel < classtype->minvel)))
				{
					int minvel = classtype->bestclimbvel;	//CSB 19Jul00
					if (reqvel < minvel || reqvel > classtype->maxvel)		//CSB 19Jul00
					{
						if (reqvel < minvel)								//CSB 19Jul00
							reqvel = minvel;								//CSB 19Jul00
						else
							reqvel=classtype->maxvel;
						int reqtime=(Range*10000.0/reqvel);
						if (ai.LastClipped()!=waypoint)
						{
							waypoint->ETA=reqtime+(timeofday+dtime);
							ai.LastClipped()=waypoint;
						}
					}
				}
				else
					reqvel = classtype->cruisevel;		//Escorts broken off but escortee on same waypoint

				ai.DesEta()=waypoint->ETA;
				ai.DesVel()=reqvel;


			}
			else
				ai.DesVel()=0;
		}
		else
			ai.LastClipped()=NULL;

	}
	else
		ai.DesVel()=0;
	if(wos.deltahori > 0)
		desposrange += wos.deltahori;
	else
		desposrange -= wos.deltahori;

	return	&despos;
}

COORDS3D* AirStruc::FindDesPos ()
{
	COORDS3D	*base=NULL;
	if (waypoint)
	{
		despos=waypoint->World;
		desposrange= waypoint->range;

		if (waypoint->target != NULL)
		{//	relative to WP target
			base = &waypoint->target->World;
		}else
		{
			if (fly.backupWP == NULL)
			{// WP is absolute
				despos=waypoint->World;
			return (&despos);
			}else
			{
				if (fly.backupWP->target != NULL)
				{// relative to backupWP target
					base = &fly.backupWP->target->World;
				}else
				{// Wp is relative to backupWP
					base = &fly.backupWP->World;
				}
			}

		}

		if (base)
		{
			despos.X += base->X;
				despos.Y += base->Y;

			despos.Z += base->Z;
		}
	}
	else														//RDH 06Aug96
	{
		INT3;
		assert(ai.homebase && "Null waypoint pointer and no home to go to!");
		despos=ai.homebase->World;
		despos.Y+=METRES800;									//JIM 02Dec96
		desposrange= METRES1600;								//JIM 02Dec96
	}
	if (despos.Y<=0)
	{																																	 		despos.Y=Land_Scape.GetGroundLevel(despos)-despos.Y;
	}
	return(&despos);

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindDesPos
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description	Finddespos for simpler items - no backup.
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
COORDS3D* mobileitem::FindDesPos ()
{
 	return	waypoint->FindDesPos();
}

COORDS3D* FindDesPos(WayPointPtr w)	{return (w->FindDesPos());}

COORDS3D* WayPoint::FindDesPos ()
{
	COORDS3D	*base,*offset;

	if (this==NULL)													//JIM 21Nov96
		_Error.EmitSysErr("still moving off end of list");		//JIM 21Nov96

	desposrange= range;
	if (target != NULL)
	{//	relative to WP target
		if (target->uniqueID.count<WayPointBAND || target->uniqueID.count>=WayPointBANDEND)
			base = &target->World;
		else
			base=::FindDesPos(*target);
		offset = &World;
		if (base->Y==0 && offset->Y<0)
		{
			base->Y=Land_Scape.GetGroundLevel(*base);
			offset->Y=-offset->Y;
		}

		despos.X = base->X + offset->X;

			despos.Y = base->Y + offset->Y;

		despos.Z = base->Z + offset->Z;
	}
	else
	{
		if (World.Y<=0)
		{
			int	gl=Land_Scape.GetGroundLevel(this);
			if (gl!=0)
			World.Y=gl-World.Y;
		}
		despos=World;
	}

	return (&despos);

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		InterceptandRange
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns	"Global" Range, HdgIntercept and PitchIntercept
//
//------------------------------------------------------------------------------
int item::Distance3D (COORDS3D* target)
{
	SLong deltax = target->X - World.X;
	SLong deltay = target->Y - World.Y;
	SLong deltaz = target->Z - World.Z;
	return Math_Lib.distance3d (deltax,deltay,deltaz);
}

int item::Distance3D (COORDS3D* subject, COORDS3D* target)	//CSB 26/04/99
{															//CSB 26/04/99
	SLong deltax = target->X - subject->X;					//CSB 26/04/99
	SLong deltay = target->Y - subject->Y;					//CSB 26/04/99
	SLong deltaz = target->Z - subject->Z;					//CSB 26/04/99
	return Math_Lib.distance3d (deltax,deltay,deltaz);		//CSB 26/04/99
}

FP item::Distance3DSquared (COORDS3D* target)
{
	FP deltax = FP(target->X) - World.X;
	deltax*=deltax;
	FP deltay = FP(target->Y) - World.Y;
	deltay*=deltay;
	FP deltaz = FP(target->Z) - World.Z;
	deltaz*=deltaz;
	return	deltax+deltay+deltaz;
}


FP item::DistanceApprox(COORDS3D* coords1, COORDS3D* coords2)
{
	FP d1 = FP(coords1->X) - coords2->X;	if(d1 < 0)	d1 *= -1;
	FP d2 = FP(coords1->Y) - coords2->Y;	if(d2 < 0)	d2 *= -1;
	FP d3 = FP(coords1->Z) - coords2->Z;	if(d3 < 0)	d3 *= -1;

	if(d2 > d1)	{FP temp = d1;	d1 = d2;	d2 = temp;	}
	if(d3 > d1)	{FP temp = d1;	d1 = d3;	d3 = temp;	}

	return(d1 + 0.385 * (d2 * d2 + d3 * d3) / d1);	//Max error [ -2% , +2% ]
}


void item::InterceptandRange (COORDS3D* target)
{
	SLong deltax = target->X - World.X;
	SLong deltay = target->Y - World.Y;
	SLong deltaz = target->Z - World.Z;
	Math_Lib.HighIntercept (deltax,deltay,deltaz,Range,HdgIntercept,PitchIntercept);	//JIM 18/03/00
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		InterceptandRange
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns	"Global" Range, HdgIntercept and PitchIntercept
//
//------------------------------------------------------------------------------
void item::InterceptandRange (COORDS3D* subject, COORDS3D* target)

{


	SLong deltax = target->X - subject->X;
	SLong deltay = target->Y - subject->Y;
	SLong deltaz = target->Z - subject->Z;
	Math_Lib.Intercept (deltax,deltay,deltaz,Range,HdgIntercept,PitchIntercept);

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcVelSub
//LastModified:	RDH 26Jul96
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool formationitem::CalcVelSub (SLong reqvel)

{
	SLong	accel, deltavel;
	SWord	deltavel_sign;
	ULong	deltavel_pos;
	SLong	climb_vel;

	UWord	roll_pos;											//RDH 28Nov96
	SWord	roll_sign;											//RDH 28Nov96

	WheeledType* classtype=formationitem::classtype;
//reduce vel if climbing
	if (((SWord)pitch) > ANGLES_0Deg)
	{
		climb_vel = (classtype->maxvel - classtype->bestclimbvel) *
									(SWord) (classtype->bestclimbpitch - pitch)/(ANGLES)classtype->bestclimbpitch;
	}else
	{
		climb_vel = (classtype->maxdivevel - 20000 - classtype->maxvel) *
									((SWord) pitch)/ANGLES_90Deg;

		climb_vel = classtype->maxvel - climb_vel;					//RDH 17Dec96

	}
//reduce vel if turning and climbing							//RDH 17Dec96

	if (((SWord)pitch) > ANGLES_0Deg)							//RDH 17Dec96
	{															//RDH 17Dec96
		Math_Lib.Pos ((SWord)roll,roll_pos,roll_sign);				//RDH 28Nov96
		climb_vel =   climb_vel * (SWord) (MAXCOMBATROLL - roll_pos)/MAXCOMBATROLL;

		climb_vel = classtype->bestclimbvel + climb_vel;
	}

	if (climb_vel > (classtype->maxdivevel-20000))						//RDH 28Nov96
		climb_vel = classtype->maxdivevel-20000;						//RDH 28Nov96
//something just over stall										//RDH 14Nov96
//really need an entry in planetype								//RDH 14Nov96
	if (climb_vel < MPH40)										//RDH 14Nov96
		climb_vel = MPH40;										//RDH 14Nov96
	if (slowdownleader)											//JIM 17Sep96
		if (formpos&InWingMAX)
			climb_vel-=50000;	//10mph	25;		//5mph								//JIM 17Sep96
		else
			climb_vel-=80000;	//15mph	25;		//5mph								//JIM 17Sep96
	slowdownleader=FALSE;										//JIM 17Sep96
	if (reqvel > climb_vel)
	 	reqvel = climb_vel;



	deltavel = (SLong) (reqvel - vel_);
//maxaccel is a fraction, where 256 = 10 cm/s/s						//RDH 19Feb96
	accel	=  classtype->maxaccel * Timer_Code.FRAMETIME / 10;


	if(deltavel >= 0)
	{	deltavel_pos = deltavel;
		deltavel_sign = MathLib::POSITIVE;	}
	else
	{	deltavel_pos = -deltavel;
		deltavel_sign = MathLib::NEGATIVE;	}

	if (accel>deltavel_pos)
	{
		if (deltavel_sign == MathLib::NEGATIVE)
		 vel_ -= deltavel_pos;
		else
		 vel_ += deltavel_pos;
		return(TRUE);
	}
	else
	{
		if (deltavel_sign == MathLib::NEGATIVE)
		 vel_ -= accel;
		else
		 vel_ += accel;
		return(FALSE);
	}
}






//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcVelWing
//Author		R. Hyde
//Date			Wed 28 Feb 1996
//
//Description	Leaders vel + some
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::CalcVelWing (AirStrucPtr lf, SLong Dist)
{
	Bool RetVal=TRUE; //x0r assume it is true
	if(Dist >= METRES1000)	//17 bits
		RetVal = CalcVelAlt(1e9);	//Requesting mach 600. Max Vel clip expected //JIM 22/04/99
	else
	{
		SLong ReqVel = 0;
		if(Dist > 0)
		{
			SLong MaxAcc = -CalcAccel(0);	//Ensure requesting decel not accel
			FP ThingToSqrt = FP(MaxAcc) * 2000.0 * Dist;
			if(ThingToSqrt > 0)
				ReqVel = lf->vel_ + FSqrt(ThingToSqrt);
			else
				ReqVel = lf->vel_;
		}
		else
		{
			SLong MaxAcc = CalcAccel(1000000000);	//Ensure requesting accel not decel
			FP ThingToSqrt = FP(MaxAcc) * 2000.0 * -Dist;
			if(ThingToSqrt > 0)
				ReqVel = lf->vel_ - FSqrt(ThingToSqrt);
			else
				ReqVel = lf->vel_;
		}

		if(ReqVel < classtype->minvel)
			ReqVel = classtype->minvel;

		Bool RetVal = CalcVelAlt(ReqVel);
		if((Dist > 0) && (vel_ > ReqVel))
		{
			vel_ = ReqVel;
			RetVal = FALSE;
		}
		if((Dist < 0) && (vel_ < ReqVel))
		{
			vel_ = ReqVel;
			RetVal = FALSE;
		}

	}
	return(RetVal);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SideSlip
//Author		R. Hyde					Craig Beeston
//Date			Wed 28 Feb 1996
//
//Description
//
//Inputs
//
//Returns		True if just got into formation
//
//------------------------------------------------------------------------------
Bool AirStruc::SideSlip (AirStrucPtr lf)
{


		SLong	dvel;

		PitchIntercept = lf->pitch;
		AutoCalcPitch();

		hdg 	= lf->hdg;

		SimpleMoveToRoll(lf->roll);

		dvel = vel_ - lf->vel_;							//RDH 30Nov96
		if(dvel < 0)
			dvel *= -1;

		if (dvel < 5000)									//RDH 30Nov96
			dvel = 5000;									//RDH 30Nov96
		vel_ = lf->vel_ + dvel;

		if(Range)
		{
			vel_x = lf->vel_x + SLong(dvel * (despos.X - World.X) / Range);
			vel_y = lf->vel_y + SLong(dvel * (despos.Y - World.Y) / Range);
			vel_z = lf->vel_z + SLong(dvel * (despos.Z - World.Z) / Range);
		}
		else
		{
			vel_x = lf->vel_x;
			vel_y = lf->vel_y;
			vel_z = lf->vel_z;
		}

		SWord DeltaX = (SWord)(despos.X - World.X);
		SWord DeltaY = (SWord)(despos.Y - World.Y);
		SWord DeltaZ = (SWord)(despos.Z - World.Z);
		if(DeltaX < 0) DeltaX *= -1;
		if(DeltaY < 0) DeltaY *= -1;
		if(DeltaZ < 0) DeltaZ *= -1;

		if((DeltaX < 50) && (DeltaY < 50) && (DeltaZ < 50))
			return TRUE;
		else
			return FALSE;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SideSlide
//Author		Craig Beeston
//Date			Tue 20 Apr 1999
//
//Description	Gets into formation by rolling and sliding with no change in heading
//
//Inputs
//
//Returns		True if just got into formation
//
//------------------------------------------------------------------------------
Bool AirStruc::SideSlide (AirStrucPtr lf)
{
	hdg = lf->hdg;

	COORDS3D GlobDelta, LocDelta;
	GlobDelta.X = despos.X - World.X;
	GlobDelta.Y = despos.Y - World.Y;
	GlobDelta.Z = despos.Z - World.Z;

	RotateToHdg(GlobDelta, LocDelta, hdg);

	SLong desroll = 2 * LocDelta.X;
	MODLIMIT(desroll, ANGLES_45Deg);
	SimpleMoveToRoll((SWord)desroll);

	SLong despitch = (LocDelta.Y * ANGLES_FRACT) / Range;
	MODLIMIT(despitch, ANGLES_15Deg);
	PitchIntercept = (Angles)despitch;
	SLong AbsDx = LocDelta.X;
	if(AbsDx < 0) AbsDx *= -1;
	AbsDx /= 2;
	if(AbsDx > ANGLES_20Deg) AbsDx = ANGLES_20Deg;
	PitchIntercept += AbsDx;
	AutoCalcPitch();

	SWord VelHdg = (SWord)hdg + (2 * (SWord)roll) / 10;		//Direction of travel - different from aircraft hdg

	Float SinHdg, CosHdg, SinPitch, CosPitch;
	Math_Lib.high_sin_cos (pitch,  SinPitch, CosPitch);
	Math_Lib.high_sin_cos ((Angles)VelHdg, SinHdg,   CosHdg);

	if(LocDelta.Z > 100000)
		CalcVelAlt();	//Max Vel
	else
	{
		SLong ReqVel = 0;
		SWord MaxAcc = 0;
		if(LocDelta.Z > 0)
			MaxAcc = CalcAccel(0);		//Ensure requesting decel not accel
		else
			MaxAcc = CalcAccel(2000000000);	//Ensure requesting accel not decel

		FP ThingToSqrt = (-MaxAcc * LocDelta.Z) * 2000.0;
		if(ThingToSqrt <= 0)
			ReqVel = fly.leadflight->vel_;
		else
			if(LocDelta.Z > 0)
				ReqVel = fly.leadflight->vel_ + FSqrt(ThingToSqrt);
			else
				ReqVel = fly.leadflight->vel_ - FSqrt(ThingToSqrt);

		CalcVelAlt(ReqVel);
	}

	vel_y			= vel_ * SinPitch;
	SLong velhori	= vel_ * CosPitch;
	vel_x			= velhori * SinHdg;
	vel_z			= velhori * CosHdg;

	NewPosition ();
	SetFlightParams ();


	if(GlobDelta.X < 0) GlobDelta.X *= -1;
	if(GlobDelta.Y < 0) GlobDelta.Y *= -1;
	if(GlobDelta.Z < 0) GlobDelta.Z *= -1;

	if((GlobDelta.X < 5000) && (GlobDelta.Y < 5000) && (GlobDelta.Z < 5000))	//All within 50m
		return TRUE;
	else
		return FALSE;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcHdgRollFlat
//Author		R. Hyde
//Date			Wed 28 Feb 1996
//
//Description	allows a flat turn using rudder up to 20 deg /sec
//
//Inputs
//
//Returns		TRUE if made the turn
//
//------------------------------------------------------------------------------
Bool formationitem::CalcHdgRollFlat ()

{
	SWord	reqdeltahdg,deltaroll;
	SLong	allowedturn;
	UWord	reqdeltahdg_pos;
	SWord	reqdeltahdg_sign;

	reqdeltahdg = 	(SWord) (hdg - HdgIntercept);
	if (Status.size==AirStrucSize)
		MoveRollToZero(reqdeltahdg);
	else
		roll = ANGLES_0Deg;

	Math_Lib.Pos ((SWord) reqdeltahdg, reqdeltahdg_pos,reqdeltahdg_sign);
	allowedturn = ANGLES_30Deg * Timer_Code.FRAMETIME/ 100;		//DAW 31Jul96
	if (reqdeltahdg_pos < allowedturn)
	{
		hdg = HdgIntercept;
		return(TRUE);

	}else
	{
		if (reqdeltahdg_sign==Math_Lib.POSITIVE)
		{
			hdg =  hdg - (Angles) allowedturn;
		}else
		{
			hdg =  hdg + (Angles) allowedturn;
		}
	}
//TempCode JIM 29Apr96 	deltaroll = RequiredDRoll (0);
//TempCode JIM 29Apr96 	roll	 += (Angles) deltaroll;
	return(FALSE);

}

void	formationitem::MoveRollToZero(SWord	reqdeltahdg)
{
	AirStrucPtr	This=*this;
	roll+=(Angles)This->RequiredDRoll(0);
	if((This != Persons2::PlayerSeenAC) && (This != Persons2::PlayerGhostAC))
	{
		This->fly.rudder = -MAXRUDDER * reqdeltahdg/ANGLES_20Deg;
		This->fly.aileron = 0;
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcHdgRoll
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//		 TR in deg per sec  = 360.g.tan RollAngle/(2.Pi.v)
//in Rowan units of 360 deg = 65536, v in 10cm/s units tan 45 = 256 not 1
//		 TR in cs			= 65536.98. tan roll/ 2.Pi.v.100.256
//							=  40 tan roll /v
//
//
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::CalcHdgRoll ()										  //DAW 16/06/99
{
	return AirStruc::CalcHdgRoll ((SWord) (hdg - HdgIntercept));
}
Bool AirStruc::CalcHdgRoll (SWord reqdeltahdg)

{
	SWord	deltaroll,currroll,tanroll;
//	SWord	reqdeltahdg;
	UWord	reqdeltahdg_pos;
	SWord	reqdeltahdg_sign;
	SLong	deltahdg;
	UWord	deltahdg_pos;
	SWord	deltahdg_sign;
	Bool	retval=FALSE;

	if (Save_Data.flightdifficulty [FD_WINDEFFECTS])
				WindCorrection();
	Math_Lib.Pos ((SWord) reqdeltahdg, reqdeltahdg_pos,reqdeltahdg_sign);
	if((reqdeltahdg_pos < ANGLES_ThirdDeg) && (roll != ANGLES_0Deg))
		retval=RollLevel (reqdeltahdg);
	else
	{
		deltaroll = RequiredDRoll (reqdeltahdg);
		roll	 += (Angles) deltaroll;
		deltahdg = CalcHdg();
		Math_Lib.Pos ((SWord) deltahdg, deltahdg_pos,deltahdg_sign);
		if (deltahdg_pos <reqdeltahdg_pos)
		{
			hdg =  hdg + (Angles) deltahdg;
		}else
		{
			retval=RollLevel (reqdeltahdg);
		}
	}
	return(retval);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcHdg
//Author		R. Hyde
//Date			Fri 11 Oct 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
SLong AirStruc::CalcHdg ()
{

	SWord	currroll,tanroll;
	SLong	deltahdg;
	UWord	deltahdg_pos;
	SWord	deltahdg_sign;
	Bool	retval=FALSE;
	UWord	reqdeltahdg_pos;
	SWord	reqdeltahdg_sign;
	MODEL& MOD = *fly.pModel;


		FP froll = FP(SWord(roll)) / FP(ANGLES_90Deg);
		if(froll >  1.0)	froll =  2.0 - froll;
		if(froll < -1.0)	froll = -2.0 - froll;
		FP froll5  = froll * froll * froll * froll * froll;
		FP froll15 = froll5 * froll5 * froll5;
		tanroll = 400.0 * froll + 1250.0 * froll5 + 2500.0 * froll15;

		MODLIMIT(tanroll, 256 * 9);
//HACK
		if (vel_==0)												//DAW 12Jul96
			vel_=1;
//ENDHACK
		FP fdeltahdg = (38000.0 * tanroll * Timer_Code.FRAMETIME) / vel_;	//DAW 31Jul96
		if(fdeltahdg >  32767) fdeltahdg =  32767;
		if(fdeltahdg < -32767) fdeltahdg = -32767;
		deltahdg = fdeltahdg;
// I reckon this 40 should be a 38 (.30664) 1000 * g / 256	//CSB 18/09/98
//stop turning on sixpence										//RDH 07Nov96


//	Stop the simple aircraft from turning faster than the flight model aircraft can	//CSB 01/06/99
		int maxdelta = (CalcMaxPitchRate() * Timer_Code.FRAMETIME) / 100;	//CSB 01/06/99
		if(deltahdg > maxdelta)												//CSB 01/06/99
			deltahdg = maxdelta;											//CSB 01/06/99
		if(-deltahdg > maxdelta)											//CSB 03/06/99
			deltahdg = -maxdelta;											//CSB 03/06/99

//	fly.turnrate = (fly.turnrate +
//								(SLong)(((deltahdg) * 100) / Timer_Code.FRAMETIME))/2;

//rads per cs
	MOD.Inst.I_Turn = (MOD.Inst.I_Turn + (Rowan2Rads(deltahdg) ) / Timer_Code.FRAMETIME)/2;

	return(deltahdg);


}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		WindCorrection
//Author		R. Hyde
//Date			Fri 30 Aug 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::WindCorrection()
{
	//rdh change to MA wind
//	SWord	sinhdg,coshdg,temppitch;
//	SLong	time;
//	SLong	deltax,deltay,deltaz;
//
//	time =  Range/vel;	//1/10s
//	Math_Lib.high_sin_cos(HdgIntercept,sinhdg,coshdg);

//	deltax =  MULSHSIN(Range,sinhdg,ANGLES_SHIFT) + MMC.wind.velx	* time;
//	deltay = 0;
//	deltaz =  MULSHSIN(Range,coshdg,ANGLES_SHIFT) + MMC.wind.velz	* time;
//	temppitch = PitchIntercept;
//	Math_Lib.Intercept (deltax,deltay,deltaz,Range,HdgIntercept,PitchIntercept);
//	PitchIntercept = (Angles)temppitch;

}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		RollLevel
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::RollLevel (int reqdeltahdg)

{
		SWord	deltaroll;

		hdg = Angles(hdg-reqdeltahdg); //not always got hdgintercept filled in!
	 	deltaroll = classtype->maxrollrate * Timer_Code.FRAMETIME/ 100;//DAW 31Jul96
	 	if ( !(roll & ANGLES_180Deg))
		{
			deltaroll = -deltaroll;
		}
		roll =  roll + (Angles) deltaroll;

	 	if ( (roll & ANGLES_180Deg) == (deltaroll & ANGLES_180Deg ))
		{
			roll =ANGLES_0Deg;
			return(TRUE);
		}
		return(FALSE);

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCalcPitch
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool formationitem::AutoCalcPitch ()

{
	SWord	reqdeltapitch, deltapitch;
	UWord	reqdeltapitch_pos;
	SWord	reqdeltapitch_sign;
	Bool	retval=FALSE;
 	reqdeltapitch = PitchIntercept - (SWord) pitch;
	Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);

	SWord deselev = -16384;

	if (reqdeltapitch_pos < ANGLES_FifthDeg)
	{
 		pitch = PitchIntercept;
		retval=TRUE;
	}
	else
	{
		deltapitch = classtype->maxpitchrate * Timer_Code.FRAMETIME/ 100;//DAW 31Jul96

		AirStrucPtr ac = NULL;
		if(Status.size == AIRSTRUCSIZE)							//CSB 3Aug00
		{
			ac = AirStrucPtr(this);
			if(		(ac->fly.leadflight == NULL)				//CSB 3Aug00
				&&	(ac->movecode != AUTO_COMBAT)
				&&	(ac->movecode != AUTO_BOMB)				//CSB 3Aug00
				&&	(ac->movecode != AUTO_LANDING)	 )		//CSB 3Aug00
			{
				deltapitch /= 4;											//CSB 3Aug00
				deselev    /= 4;
			}
		}

		if (deltapitch > reqdeltapitch_pos)
		{
 			pitch = PitchIntercept;
			deselev = 0;
			retval=TRUE;
		}
		else if(reqdeltapitch_sign == Math_Lib.NEGATIVE)			//JIM 05May96
		{
			deltapitch = -deltapitch;
			deselev	   = -deselev;
		}

		pitch += Angles(deltapitch);

		if((SWord(roll) < SWord(ANGLES_270Deg)) || (SWord(roll) > SWord(ANGLES_90Deg)))
			deselev = -deselev;

		if(ac)
			ac->fly.elevator = deselev;
	}

	return(retval);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoCalcPitch
//Author		R. Hyde
//Date			Thu 6 Jun 1996
//
//Description	THe aoa calc is only good for camels###
//				Something need to be done here as in FC in order to correct the	//fix this !!!!
//				drawing of aircraft at high angles of attack
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool  AirStruc::AutoCalcPitch()
{
	Bool	retval;
	UWord	d1,d2,vel_in_10mph;

	retval = formationitem::AutoCalcPitch();


	fly.cpitch = pitch;
//	pitch = pitch + fly.aoa;
	return(retval);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcXYZVel
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void mobileitem::CalcXYZVel ()

{
	Float	sin_theta,cos_theta;
	Float	sin_hdg, cos_hdg;
	Float	fvel = Float(vel_);									//RJS 10Jan00

	Math_Lib.high_sin_cos (pitch,sin_theta,cos_theta);

	vel_y = fvel * sin_theta;									//RJS 10Jan00

	Float	velhori = fvel * cos_theta;							//RJS 10Jan00

	Math_Lib.high_sin_cos (hdg, sin_hdg, cos_hdg);
	vel_x = velhori * sin_hdg;									//RJS 10Jan00
	vel_z = velhori * cos_hdg;									//RJS 10Jan00


}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcXYZVel
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::CalcXYZVel ()

{
	mobileitem::CalcXYZVel ();

}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ReachedDesPos
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
SWord AirStruc::ReachedDesPos ()

{
	SLong	desrange;

	if (desposrange ==0)
	{	//trial and error shows 2.5 * distance travelled in frame is about right
		//vel*time=10*dist, so want v*t/4
		//put in /3 'cos frametime is 3 so optimised out????
		if (Range>METRES500)		return (FALSE);
		desrange = (vel_ * Timer_Code.FRAMETIME) / 3000;
	}else
		desrange = desposrange;
	if (desrange > Range)
		return(TRUE);
	else
		return(FALSE);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		NewPosition
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description	delta = vel*0.3
//				to handle rounding, I have brought in fs = 0..1 over 1200ms
//				if delta should be 1.3,
//				then for 800ms it will be 1 and	for 400 ms it will be 2
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void mobileitem::NewPosition ()
{
	SLong deltax, deltay, deltaz;						//CSB 08/11/99
	deltax = (vel_x * Timer_Code.FRAMETIME) / 10000;	//CSB 08/11/99
	deltay = (vel_y * Timer_Code.FRAMETIME) / 10000;	//CSB 08/11/99
	deltaz = (vel_z * Timer_Code.FRAMETIME) / 10000;	//CSB 08/11/99


	CalcNewPos (deltax,deltay,deltaz);					//CSB 08/11/99

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		NewPosition
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::NewPosition (bool withwind)
{
	SLong deltax, deltay, deltaz;
	deltax = (vel_x * Timer_Code.FRAMETIME) / 10000;
	deltay = (vel_y * Timer_Code.FRAMETIME) / 10000;
	deltaz = (vel_z * Timer_Code.FRAMETIME) / 10000;

	if(withwind)	CalcNewPosWithWind(deltax, deltay, deltaz, 0);
	else			MovingItem::CalcNewPos(deltax, deltay, deltaz);

	if(((!currworld->vp) || (!currworld->vp->Accel())) && (fly.numinsag == 0))
	{
		if((movecode != AUTO_TAKEOFF) && (movecode != AUTO_LANDING))
		{
			ANGLES grndpitch=ANGLES_0Deg, grndroll=ANGLES_0Deg;			//AMM 11Jul00
			Collide::Pos landed = _Collide.HaveWeLanded(*this, grndpitch, grndroll);

			if((landed != _Collide.NOTTOUCHED)
				&&	(	(this == Manual_Pilot.ControlledAC2)
					 || !_DPlay.Implemented)	)
			{
				if(landed ==  _Collide.LANDED_OK)
	 				AutoToGrndPitchRoll(grndroll,grndpitch);
				else
				{
					World.Y = fly.pModel->GroundHeight;
					roll = grndroll;
					pitch = grndpitch;

					Trans_Obj.LandedEffect(*this,landed);
					if (this == Manual_Pilot.ControlledAC2)
						GR_Quit3DNow=CRASHED_DEAD;
				}
			}
		}
		else
			if(((fly.pModel->FrameCount & 0x07) == 0) || ((movecode == AUTO_LANDING) && (ai.ManStep > 5) && (ai.ManStep < 11)))
				fly.pModel->GroundHeight = _Collide.GroundAltitude(World);
	}

}

void AirStruc::RemoveFuelLeakage(weap_info weapinfo, SWord Xpos, UWord NFrames)
{
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FuelLeft
//Author		R. Hyde
//Date			Mon 3 Jun 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::FuelLeft (FP FuelRate)		//ONLY CALLED EVERY 128 FRAMES (256cs)!!!
{
//called every 256cs, at 25Hz this is every 64 frames. //AMM 25/11/99

	int usage = (FuelRate * 256 * classtype->fuelaboff);//cg,cs


	SWord i;
	SLong OldTotInt = 0;

	SWord NumTanks = 0;
	for(i = 0; i < 4; i++)
	{
		OldTotInt += fly.fuel_content[i];
		if((fly.use_this_tank[i]) && (fly.fuel_content[i]))
			NumTanks++;
	}

	for(i = 0; i < 4; i++)
		if((fly.use_this_tank[i]) && (fly.fuel_content[i]))
		{
			fly.fuel_content[i] -= usage / NumTanks;
			if(fly.fuel_content[i] < 0)
				fly.fuel_content[i] = 0;
		}

	SLong TotInt = 0;
	for(i = 0; i < 4; i++)
		TotInt += fly.fuel_content[i];

	if(TotInt <= 0)
	{
		if((this == Persons2::PlayerGhostAC) || (this == Persons2::PlayerSeenAC))
			OverLay.CancelAccel();

		if(		((this != Persons2::PlayerGhostAC) && (this != Persons2::PlayerSeenAC))
			||	(Persons2::PlayerGhostAC->classtype->aerobaticfactor == AEROBATIC_LOW)	)
		{
			fly.thrustpercent = 0;
			if(movecode != AUTO_LANDING)
			{
				if(OldTotInt != 0)
				{
					AirStrucPtr buddy = FindABuddyWithPlayerGivenPriority();
					SLong DistHome = Distance3D(&ai.homebase->World);
					if(DistHome < 10 * (World.Y - ai.homebase->World.Y))
					{
						if (!fly.numinsag)								//RJS 26Sep00
							BreakForm();								//RJS 26Sep00

						if(buddy)
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_NOFUELGLIDE, MSG_BINGOFUEL, *this, NULL, buddy));
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_MAYDAY, MSG_BINGOFUEL, *this, NULL, *this));
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_NEARESTAIRFIELD, MSG_MAYDAYREPLY_X, VOICE_TOWER, ai.homebase, *this));

						SetUnfriendly(NULL);
						movecode = AUTO_LANDING;
						ai.ManStep = 0;
					}
					else
					{
						if(fly.numinsag)
							SAGDeathSequenceOverride(AUTOSAG_DEATHGLIDE);
						else
						{
							BreakForm();								//RJS 26Sep00
							Manual_Pilot.DeathSequenceOverrideDiary(this, AUTO_DEATHGLIDE);	//RJS 31Oct00
						}

						if(buddy)
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_NOFUELDITCH, MSG_BINGOFUEL, *this, NULL, buddy));
					}
				}
			}
		}
	}
	else if((this != Persons2::PlayerGhostAC) || (this != Persons2::PlayerSeenAC))
	{

		if((FuelShort(10 * 60 * 100)) && (ai.manoeuvre != MANOEUVRE_SCREWYOUGUYSIMGOINGHOME))
		{
			AirStrucPtr callee = FindGroupLeader();
			if(nationality == NAT_RAF)
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_RAF_LOWFUEL, MSG_BINGOFUEL, *this, NULL, callee));
			else
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_LW_LOWFUEL, MSG_BINGOFUEL, *this, NULL, callee));
			if(!callee->AcIsPlayer())
				callee->AiLeaderBingoInstructions();
		}
	}
}




void	AirStruc::ChainBingoMessage(bool atbingohurting)
{
	AirStrucPtr recipient=NULL;
	switch (position())
	{
	case 0:
		if (formpos)
			recipient=fly.leadflight;
	break;
	case 3:															  //JIM 28/05/99
	case 1:
		recipient=Leader();
	break;
	case 2:															  //RJS 5/28/99
		recipient=fly.leadelt();
	break;
	}
	if (!atbingohurting)
	{
		if (recipient && weap.reportedbingo)
		{
			if(nationality == NAT_RAF)
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_RAF_LOWFUEL, MSG_BINGOFUEL, *this, *this, recipient));
			else
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_LW_LOWFUEL, MSG_BINGOFUEL, *this, *this, recipient));
			recipient->ChainBingoMessage(atbingohurting);
		}
		weap.reportedbingo=TRUE;
	}
	else
	{
		if (recipient && weap.reportedbingohurting)
		{
			if(nationality == NAT_RAF)
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_RAF_LOWFUEL, MSG_BINGOFUEL, *this, NULL, recipient));
			else
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_LW_LOWFUEL, MSG_BINGOFUEL, *this, NULL, recipient));
			recipient->ChainBingoMessage(atbingohurting);
		}
		weap.reportedbingohurting=TRUE;
	}
}


void	AirStruc::AiLeaderBingoInstructions()
{
	AirStrucPtr buddy = PlayerInGroup();
	if (!buddy)
		buddy = FindBuddy();

	if((fly.leadflight) && (fly.leadflight->classtype->aerobaticfactor == AEROBATIC_LOW))
	{
		fly.leadflight = NULL;
		if(buddy)
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_LOWFUELRTBFORGETBOMBERS, MSG_BINGOFUEL, *this, NULL, buddy));
	}

	if(movecode == AUTO_COMBAT)
	{
		if(fly.numinsag)
		{
			movecode = AUTOSAG_FOLLOWWP;
			ai.manoeuvre = MANOEUVRE_SCREWYOUGUYSIMGOINGHOME;
			if(AirStrucPtr(ai.unfriendly)->ai.attacker == this)
				AirStrucPtr(ai.unfriendly)->ai.attacker = NULL;
			ai.unfriendly = NULL;
		}
		else
			for(AirStrucPtr ld = this; ld; ld = ld->fly.nextflight)
				for(AirStrucPtr ac = ld; ac; ac = ac->Follower())
				{
					ac->waypoint = waypoint->FindWP(WPNAME_EscDisperse);
					if(ac->ai.unfriendly)
						ac->ACMAS().SelectNextDisengageManoeuvre();
					else
						ac->ai.manoeuvre = MANOEUVRE_SCREWYOUGUYSIMGOINGHOME;
				}
	}
	else

	if((ai.unfriendly) && (ai.unfriendly->Status.size == AIRSTRUCSIZE) && (AirStrucPtr(ai.unfriendly)->classtype->aerobaticfactor == AEROBATIC_LOW))
		if(buddy)
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_LOWFUELRTBFORGETBOMBERS, MSG_BINGOFUEL, *this, NULL, buddy));


}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetFlightParams
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description	CSB - The frame counter for each aircraft aircraft is incremented and the amount
//				of fuel used and the atmospheric conditions are calculated every N frames
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
extern int	 Count_SetFlightParams;
void AirStruc::SetFlightParams (bool dotimestuff)
{

	if (dotimestuff)
	{
		FP rpmfact = 0.005 * fly.thrustpercent + 0.5 * vel_ / classtype->maxvel;
		ULong targetrpm = rpmfact * classtype->maxrpm;

		if(targetrpm > fly.rpm)
		{
			fly.rpm += 10 * Timer_Code.FRAMETIME;
			if(fly.rpm > targetrpm) fly.rpm = targetrpm;
		}
		if(targetrpm < fly.rpm)
		{
			fly.rpm -= 10 * Timer_Code.FRAMETIME;
			if(fly.rpm < targetrpm) fly.rpm = targetrpm;
		}

		if(fly.numinsag == 0)
		{
			AircraftAnimData* adptr = (AircraftAnimData*)Anim;
			if(adptr)
			{
				adptr->acrpm = (3 * fly.rpm) / 5;
				adptr->acrpm1 = adptr->acrpm;

				SWord maxdelta = 2 * Timer_Code.FRAMETIME;

				SWord acroll = roll;
				if(acroll < 0)
					acroll = -acroll;
				if((acroll < ANGLES_90Deg) && (-acroll < fly.elevator))
					fly.elevator = -acroll;

				MODLIMIT(fly.elevator, 32767);
				SWord desval = -fly.elevator >> 8;
				SWord animval = SWord(SByte(adptr->acelevator));
				if(animval != desval)
				{
					if((desval > animval - maxdelta) && (desval < animval + maxdelta))
						animval = desval;
					else
						if(desval > animval)
							animval += maxdelta;
						else
							animval -= maxdelta;
					adptr->acelevator = animval;
				}

				MODLIMIT(fly.aileron, 32767);
				desval = -fly.aileron >> 8;
				animval = SWord(SByte(adptr->acaileronl));
				if(animval != desval)
				{
					if((desval > animval - maxdelta) && (desval < animval + maxdelta))
						animval = desval;
					else
						if(desval > animval)
							animval += maxdelta;
						else
							animval -= maxdelta;
					adptr->acaileronl = animval;
					adptr->acaileronr = -adptr->acaileronl;
				}

				MODLIMIT(fly.rudder, 32767);
				desval = fly.rudder >> 8;
				animval = SWord(SByte(adptr->acrudder));
				if(animval != desval)
				{
					if((desval > animval - maxdelta) && (desval < animval + maxdelta))
						animval = desval;
					else
						if(desval > animval)
							animval += maxdelta;
						else
							animval -= maxdelta;
					adptr->acrudder = animval;
				}

			}
		}
		else
		{
			SimpleAircraftAnimData* adptr = (SimpleAircraftAnimData*)Anim;
			if(adptr)
				adptr->acrpm = (3 * fly.rpm) / 5;
		}

		if(!(fly.pModel->FrameCount & EVERY256cs))	//This will do the following commands for each aircraft
		{											//once every n operations.
//			if (!_Replay.Record && !_Replay.Playback)
			{
				MissManCampSky().Ambient (World.Y,
			 				 	fly.pModel->AmbDensity,
							 	fly.pModel->AmbTemp,
								 fly.pModel->AmbPres);
			}
			//(FP)vel / (10 * FSqrt(1.403 * 287 * fly.pModel->AmbTemp) );
			//0.9 * fly.pModel->AmbDensity / 0.0001225 * (1 + 0.45 * Mach);

			if(classtype->aerobaticfactor == AEROBATIC_LOW)
			{
				if(movecode == AUTO_TAKEOFF)
				{
					if(vel_ > 0)
						fly.thrustpercent = 100;
					else
						fly.thrustpercent = 10;
				}
				else if(movecode == AUTO_LANDING)
				{
					if(vel_ > 0)
						fly.thrustpercent = 10;
					else
						fly.thrustpercent = 0;
				}
				else
				{
					if(SWord(pitch) > SWord(ANGLES_1Deg))
						fly.thrustpercent = 90;
					else if(SWord(pitch) < SWord(ANGLES_359Deg))
						fly.thrustpercent = 10;
					else
						fly.thrustpercent = 75;
				}
			}
			else
			{
				if(movecode != AUTO_COMBAT)
				{
					FP tp = 90;
					if(pitch < ANGLES_2_5Deg)
					{
						tp = 0.95 * FP(vel_) / FP(CalcMaxVelSL());
						tp = 100.0 * tp * tp * tp;
					}
					MODMAXMIN(tp, 0, 90);
					fly.thrustpercent = tp;
				}
				FP FuelUsageRate = fly.thrustpercent * 0.01;

				if(classtype->version == 2)	//Contant speed props - higher efficiency
					FuelUsageRate *= 0.65;

				fly.pModel->SetFuelTank(this, false);
				FuelLeft(FuelUsageRate);
			}//Old_Code DAW 22Nov98 			FP FuelUsageRate = fly.thrustpercent * 0.01;
		}
		fly.pModel->FrameCount++;
	}
}
//done in usermsg

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LeaderBreakOff
//Author		Jim Taylor
//Date			Wed 23 Oct 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	AirStruc::LeaderBreakOff ()
{
//	done in usermsg.cpp


}

//done in usermsg
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetManualFlightParams
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::SetManualFlightParams ()
{
	SetControlSurfaces();
	bool inaccel = false;
	if((MobileItem::currworld->vp != NULL) && (MobileItem::currworld->vp->Accel()))
		inaccel = true;

	if((!inaccel) && (fly.pModel))
	{
		fly.rpm = fly.pModel->Inst.I_EngineRpm0;
		fly.rpm1 = fly.pModel->Inst.I_EngineRpm1;						//RJS 21Jun00
	}
	else
		SetRpm ();
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetManoeuvreTime
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description	Sets the timer counter. This timer is used for various things
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void formationitem::SetManoeuvreTime (UWord	time)
{
 manoeuvretime=time;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		TimeLeft
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
SWord formationitem::TimeLeft ()

{
	SLong tmp=manoeuvretime-Timer_Code.FRAMETIME;				//DAW 31Jul96
	if (tmp <0)
	{
		manoeuvretime = 0;
		return (FALSE);
	}else
	{
		manoeuvretime =tmp; 					//PD 15Mar96
		return (manoeuvretime);
	}


}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		RequiredDRoll
//Author		R. Hyde
//Date			Tue 28 Nov 1995
//
//Description			a roll is chosen from a table for the required delta hdg
//						the delta roll is then calculated
//
//
//
//Inputs				reqdeltahdg
//
//Returns				deltaroll
//
//------------------------------------------------------------------------------
SWord AirStruc::RequiredDRoll (SWord reqdeltahdg)

{
	SWord	reqroll;

	reqroll = FindRequiredRoll (reqdeltahdg, RequiredBankData);

	//fix this !!!! to limit the angle of roll so that it can be assumed that
	//				the speed is not affected by the roll angle		//CSB 29/09/98
	return (RequiredDRollSub(reqroll));							//RDH 11Oct96
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		RequiredDRollSub
//Author		R. Hyde
//Date			Fri 11 Oct 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
SWord AirStruc::RequiredDRollSub (SWord reqroll)
{
	SWord	deltaroll,reqdeltaroll;
	UWord	reqdeltaroll_pos;
	SWord	reqdeltaroll_sign;

	reqdeltaroll = (SWord) ((Angles)reqroll - roll);
	Math_Lib.Pos (reqdeltaroll,reqdeltaroll_pos,reqdeltaroll_sign);


//gradual move to max may be required							//RDH 27Oct96
//	fly.rollrate = (Angles)classtype->maxrollrate;

//	deltaroll = fly.rollrate * Timer_Code.FRAMETIME/100;		//DAW 31Jul96

	deltaroll = (Angles)classtype->maxrollrate * Timer_Code.FRAMETIME/100;		//rdh

	fly.aileron = 32767;

	if(		(Status.size == AIRSTRUCSIZE)							//CSB 3Aug00
		&&	(AirStrucPtr(this)->fly.leadflight == NULL)				//CSB 3Aug00
		&&	(AirStrucPtr(this)->movecode != AUTO_COMBAT)			//CSB 3Aug00
		&&	(AirStrucPtr(this)->movecode != AUTO_BOMB)				//CSB 3Aug00
		&&	(AirStrucPtr(this)->movecode != AUTO_LANDING)	 )		//CSB 3Aug00
	{
		deltaroll /= 4;												//CSB 3Aug00
		fly.aileron /= 4;
	}
	else if((movecode == AUTO_COMBAT) && (!fly.numinsag))
	{
		AircraftAnimData* adptr = (AircraftAnimData*) Anim;
		FP damage = FP(adptr->LEFTWINGOUT + adptr->LEFTWINGIN + adptr->RIGHTWINGIN + adptr->RIGHTWINGOUT) * (1.0 / 1024.0);
		deltaroll = FP(deltaroll) * (1.0 - damage);
	}

	if (deltaroll > reqdeltaroll_pos)
	{
		deltaroll = reqdeltaroll_pos;
		fly.aileron = 0;
	}

	if (reqdeltaroll_sign == MathLib::NEGATIVE)
	{
		deltaroll	 = -deltaroll;
		fly.aileron  = -fly.aileron;
	}

	return (deltaroll);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindRequiredRoll
//Author		R. Hyde
//Date			Fri 15 Dec 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
SWord AirStruc::FindRequiredRoll (SWord reqdeltahdg, RequiredBankDataElement* rolldata)

{

	UWord	reqdeltahdg_pos;
	SWord	reqdeltahdg_sign;
	SWord	reqrolllower,reqrollupper;
	SWord	reqhdglower,reqhdgupper;
	SWord	reqroll;

//	RequiredBankDataElement*	rolldata;
//	rolldata = RequiredBankData;

	Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

	if (reqdeltahdg_pos == ANGLES_180Deg)
	{
		reqroll = MAXROLL;
	}else
	{
		reqrollupper = 0;
		reqhdgupper = 0;
		reqrolllower = reqrollupper;
		reqhdglower = reqhdgupper;
		while (rolldata->hdg < (Angles)reqdeltahdg_pos)
		{
			reqrolllower = reqrollupper;
			reqhdglower = reqhdgupper;
			rolldata++;
				reqrollupper = (ANGLES)rolldata->roll;
			reqhdgupper = (ANGLES)rolldata->hdg;
		}
		if (reqhdgupper - reqhdglower == 0)
		{
			reqroll = reqrolllower;
		}else
		{
			reqroll = reqrolllower + (reqrollupper - reqrolllower)
					* (reqdeltahdg_pos - reqhdglower) / (reqhdgupper - reqhdglower);
		}

	}
	if	(reqdeltahdg_sign == MathLib::POSITIVE)
	{
		reqroll = -reqroll;
	}
	return (reqroll);
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcNewPos
//Author		R. Hyde
//Date			Mon 30 Oct 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MovingItem::CalcNewPos(SLong deltax,SLong deltay,SLong deltaz)

{


	World.X += deltax;
  	World.Y += deltay;
	World.Z += deltaz;										//RDH 18Dec95
	uniqueID.changed = TRUE;									//RDH 15Jul96

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcNewPos
//Author		R. Hyde
//Date			Mon 30 Oct 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::CalcNewPosWithWind(SLong deltax,SLong deltay,SLong deltaz,SLong groundlevel)

{


//	if (Save_Data.flightdifficulty [FD_WINDEFFECTS] && !Timer_Code.accel)
//	{
//rdh change to MA wind
//		if (World.Y > groundlevel)
//		{
//			deltax +=  MMC.wind.velx	* TimerCode::FIXEDFRAMETIME /10;
//			deltaz +=  MMC.wind.velz	* TimerCode::FIXEDFRAMETIME /10;
//		}else
//		{
//			dhdg = hdg - MMC.wind.direction+ ANGLES_180Deg;
//			Math_Lib.high_sin_cos (dhdg, sind_hdg, cosd_hdg);
//			horivel =  MMC.wind.speed * cosd_hdg / ANGLES_FRACT;
//			horivel+= velhori;
//			if ((horivel < 0) ||  (velhori ==0))
//			{
//				deltax = 0;
//				deltaz = 0;
//			}else
//			{
//				Math_Lib.high_sin_cos (hdg, sinv_hdg, cosv_hdg);
//				deltax =  (horivel * sinv_hdg / ANGLES_FRACT);
//				deltax = deltax * TimerCode::FIXEDFRAMETIME /10;
//				deltaz =  (horivel * cosv_hdg / ANGLES_FRACT);
//				deltaz = deltaz * TimerCode::FIXEDFRAMETIME /10;
//			}
//
//		}
//
//	}


	//CSB 10/06/99
	SLong wx, wy, wz;
	MissManCampSky().GetWind(World.Y, wx, wy, wz);

	deltax += (wx * Timer_Code.FRAMETIME) / 10000;
	deltay += (wy * Timer_Code.FRAMETIME) / 10000;
	deltaz += (wz * Timer_Code.FRAMETIME) / 10000;


	MovingItem::CalcNewPos(deltax, deltay, deltaz);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetInstruments
//Author		R. Hyde
//Date			Tue 25 Jun 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
//void	AirStruc::SetInstruments ()
//{
//	UWord		d1,d2,altitude_1000m,densityratio;
//	SLong		altitude_cm;
//
//
//	altitude_cm = World.Y;
//	if (altitude_cm < 0)
//		altitude_cm = 0;
//	if (altitude_cm >MAXALTITUDE_CM)
//		altitude_cm = MAXALTITUDE_CM;
//
//	altitude_1000m =	(UWord) (altitude_cm/100000);
//
//
//	d1 =	airdensityratiosquareroot[altitude_1000m];
//	d2 =	airdensityratiosquareroot[altitude_1000m+1];
//	densityratio = (UWord) (d1+ (d2-d1) * (altitude_cm - altitude_1000m*100000)/100000);
//
//}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetControlSurfaces
//Author		R. Hyde
//Date			Mon 25 Mar 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	AirStruc::SetControlSurfaces ()
{
	UWord	elevator_pos;
	SWord	elevator_sign;
	SWord	deltaelevator;
	UWord	aileron_pos;
	SWord	aileron_sign;
	SWord	deltaaileron;
//	UWord	rudder_pos;
//	SWord	rudder_sign;
	SWord	deltarudder;

SLong const ELEVATORSTEPSIZE = MAXELEVATOR/8;
//Copy to animation data structure							//PD 19Apr96

	AircraftAnimData *adptr = (AircraftAnimData *)Anim;			//PD 19Apr96
	if (adptr)													//PD 19Apr96
	{




	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetRpm
//Author		R. Hyde
//Date			Mon 22 Apr 1996
//
//Description		  produce an rpm as a function of thrust ^ 2
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::SetRpm ()
//RDH 24May96 modified
{

//rpm calc

	SLong	reqrpm;

	if (fly.thrustpercent <2)
	{
		if (vel_ > 90000)
		{
			reqrpm = classtype->freerpm;
		}else
		{
			reqrpm = 0;
		}
	}else
	{
		reqrpm = fly.thrustpercent * (classtype->maxrpm - classtype->minrpm)/100;
		reqrpm = classtype->minrpm + reqrpm * reqrpm /
				(classtype->maxrpm - classtype->minrpm);
		reqrpm = reqrpm
				-  (SWord) pitch *  (classtype->maxdiverpm - classtype->maxrpm)
				/	(ANGLES_60Deg);
	}
	//effect of pitch

	if (fly.rpm > reqrpm)
	{
		fly.rpm -= Timer_Code.FRAMETIME;						//DAW 31Jul96
	}else
	{
																//RDH 05Dec96
			fly.rpm += Timer_Code.FRAMETIME;						//DAW 31Jul96
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//RJS 28Aug98
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		IsHealthyMovecode
//Author		Dave Whiteside
//Date			Fri 28 Aug 1998
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool	AirStruc::IsHealthyMovecode(ULong&	themovecode)
{
	Bool	retval = FALSE;
	themovecode = 0xFFFFFFFF;

	if (Status.size >= MOBILESIZE)
	{
		if (!_Replay.Playback || this!=Persons2::PlayerSeenAC)	//AMM 28Apr99
		{
			themovecode = movecode;
			if (SHAPESTUFF.GetShapePtr(shape)->AnimDataSize == AIRCRAFTANIM)
			{
				if (	(movecode != AUTO_NOPPILOT)
					&&	(movecode != AUTO_HITWATER)
					&&	(movecode != AUTO_CRASHROLL)
					&&	(movecode != AUTO_CRASHSKID)
					&&	(movecode != AUTO_SPIRAL2GROUND)				//DAW 03Sep98
					&&	(movecode != AUTO_DEATHSEQUENCE)
					&&	(movecode != AUTO_DEATHGLIDE)
					&&	(movecode != AUTO_CRASHTUMBLE)
					&&	(movecode != AUTO_CRASHONEWHEEL)
					&&	(movecode != AUTO_CRASHNOSE)
					&&	(movecode != AUTO_CRASHFLIP)	)
					retval = TRUE;
			}
		}
		else
		{
			themovecode = _Replay.ReplayDeathMode;
			if (	(themovecode == RDEATH_NULL)
				&&	(SHAPESTUFF.GetShapePtr(shape)->AnimDataSize == AIRCRAFTANIM)	)
				retval = TRUE;
		}														//RJS 29Oct98
	}

	return(retval);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetWeaponForce
//Author		Robert Slater
//Date			Wed 16 Sep 1998
//
//Description
//
//Inputs		  100000cg = 1kg
//					10m/sec
//					csec
//
//Returns
//
//------------------------------------------------------------------------------
void	AirStruc::SetWeaponForce(SLong	mass, SLong	velocity, SLong	delay)
{
	if (mass)
	{
		FP force = FP(mass) * FP(velocity) / FP(delay);

		force *= 0.0000001;	//CSB 08/01/99

		weap.weapforce = -force;
	}
	else
		weap.weapforce = 0;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcBestClimbSpeed
//Author		Craig Beeston
//Date			Mon 28 Sep 1998
//
//Description	Given an altitude the best climb velocity is calculated for that aircraft
//
//Inputs		Alt
//
//Returns		Vel
//
//------------------------------------------------------------------------------
SLong AirStruc::CalcBestClimbSpeed()
{
	Bool	retval = FALSE;

	SLong Height = World.Y;

	if(Height < 0)       Height = 0;
	if(Height > 2000000) Height = 2000000;

	SWord h = Height * 0.00001;	//cm -> km
	if (h > 19) h = 19;	//RERUN: [h+1] OOB at Height=2000000 (array [21]); interp still yields v[20]
	SLong v1, v2;
	v1 = classtype->bestclimbspeed[h];
	v2 = classtype->bestclimbspeed[h+1];
	FP ClimbVel = v1 + (Height - 100000.0 * h) / 100000.0 * (v2 - v1);


	ClimbVel = (ClimbVel * 98) / 100;

	return(ClimbVel);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcBestClimbPitch
//Author		Craig Beeston
//Date			Mon 28 Sep 1998
//
//Description	Given an altitude the best climb Pitch is calculated for that aircraft
//
//Inputs		Alt
//
//Returns		pitch
//
//------------------------------------------------------------------------------
SWord AirStruc::CalcBestClimbPitch()
{
	SLong Height = World.Y;

	if(Height < 0)       Height = 0;
	if(Height > 2000000) Height = 2000000;

	SWord h = Height * 0.00001;	//cm -> km
	if (h > 19) h = 19;	//RERUN: [h+1] OOB at Height=2000000 (array [21])
	SWord p1, p2;
	p1 = classtype->bestclimbang[h];
	p2 = classtype->bestclimbang[h+1];
	FP ClimbPitch = p1 + (Height - 100000.0 * h) * (p2 - p1) / 100000.0;

	{
		SLong StoresDrag = weap.left.drag + weap.centre.drag + weap.right.drag;

		ClimbPitch =  (ClimbPitch * (9000 - StoresDrag)) / 9000;
		ClimbPitch -= StoresDrag / 10;
	}

	ClimbPitch = (ClimbPitch * 90) / 100;

	if(ClimbPitch > ANGLES_0Deg)
		return(ClimbPitch);
	else
		return(0);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcMaxVelDive
//Author		Craig Beeston
//Date			Mon 28 Sep 1998
//
//Description	Given an altitude the Maximum Speed in a vertical dive is calculated for that aircraft
//
//Inputs		Alt
//
//Returns		Speed
//
//------------------------------------------------------------------------------
SLong AirStruc::CalcMaxVelDive()
{
	SLong Height = World.Y;

	if(Height < 0) Height = 0;
	if(Height > 2000000) Height = 2000000;

	SLong MaxVelDive;

	if(Height < 1100000)
		MaxVelDive = classtype->maxveldive_0km + Height *
					(classtype->maxveldive_11km - classtype->maxveldive_0km) / 1100000;
	else
		MaxVelDive = classtype->maxveldive_11km + (Height - 1100000) *
					(classtype->maxveldive_20km - classtype->maxveldive_11km) / 900000;

	return(MaxVelDive);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcMaxVelSL
//Author		Craig Beeston
//Date			Mon 28 Sep 1998
//
//Description	Given an altitude the Maximum Straight and Level Speed is calculated for that aircraft
//
//Inputs		Alt
//
//Returns		Speed
//
//------------------------------------------------------------------------------
SLong AirStruc::CalcMaxVelSL()
{
	Bool	retval = FALSE;

	SLong Height = World.Y;

	if(Height < 0)       Height = 0;
	if(Height > 2000000) Height = 2000000;

	SLong h = Height / 100000;	//cm -> km
	if (h > 19) h = 19;	//RERUN: [h+1] OOB at Height=2000000 (array [21])
	SLong v1, v2;
	v1 = classtype->maxvelsl[h];
	v2 = classtype->maxvelsl[h+1];
	FP maxvel = v1 + (Height - 100000.0 * h) / 100000.0 * (v2 - v1);



	return(maxvel);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcVelAlt
//Author		Craig Beeston
//Date			Tue 29 Sep 1998
//
//Description	Calculates the velocity for AutoFollowWP depending on the altitude of the AC
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::CalcVelAlt ()

{
	SLong	reqvel=0;
	if (waypoint)
		if (waypoint->ETA && waypoint->ETA==ai.DesEta())
			reqvel=ai.DesVel();
		else
		if (waypoint->_vel)
		 	reqvel =  waypoint->_vel;
	return CalcVelAlt (reqvel);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcVelAlt
//Author		Craig Beeston
//Date			Tue 29 Sep 1998
//
//Description	Calculates the velocity for AutoFollowWP depending on the altitude of the AC
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::CalcVelAlt (SLong reqvel)
{
	SLong LocalMaxVelSL = CalcMaxVelSL();
	SLong MaxVel = LocalMaxVelSL;

	if (!reqvel)
		reqvel = LocalMaxVelSL * 0.85;//fix this to represent a cruise velocity

	if((SWord)pitch == 0)
		MaxVel = LocalMaxVelSL;
	else
	if (((SWord)pitch) > ANGLES_0Deg)
	{
		SLong LocalBestClimbSpeed = CalcBestClimbSpeed();
		SLong LocalBestClimbPitch = CalcBestClimbPitch();

		FP PitchRatio;
		if(LocalBestClimbPitch > 0)
		{
			PitchRatio =  (SWord)pitch;
			PitchRatio /= LocalBestClimbPitch;
			PitchRatio *= PitchRatio;
			if(PitchRatio > 1.0) PitchRatio = 1.0;
			MaxVel = LocalMaxVelSL + PitchRatio * (LocalBestClimbSpeed - LocalMaxVelSL);
		}
		else MaxVel = LocalBestClimbSpeed;
	}
	else
	{
		SLong LocalMaxVelDive = CalcMaxVelDive();

		FP fPitch = 1.0 - FP(-pitch) / 16384.0;
		fPitch = 1 - fPitch * fPitch * fPitch;

		MaxVel = LocalMaxVelSL + SLong(fPitch * FP(LocalMaxVelDive - LocalMaxVelSL));
	}
	if (slowdownleader)											//JIM 17Sep96 //JIM 01/02/99
		if (formpos&InWingMAX)										  //JIM 01/02/99
			MaxVel = (MaxVel * 95) / 100;
		else														  //JIM 01/02/99
			MaxVel = (MaxVel * 90) / 100;

	slowdownleader = FALSE;										//JIM 17Sep96


	if (reqvel > MaxVel)
	 	reqvel = MaxVel;

	if (reqvel < classtype->minvel)
	 	reqvel = classtype->minvel;

	if(vel_ == reqvel)
		return(TRUE);

	SLong oldvel = vel_;
	SLong accel  = CalcAccel(reqvel);
	AddAccel2Vel(accel);
	if(	   ((oldvel < reqvel) && (vel_ > reqvel))
		|| ((oldvel > reqvel) && (vel_ < reqvel)) )
	{
		vel_ = reqvel;
		return(TRUE);
	}
	else return(FALSE);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcAccel
//Author		Craig Beeston
//Date			Tue 2 Feb 1999
//
//Description	Calculates the maximum possible accel or deccel of an AC at current alt and speed
//				Only really right for straight and level flight
//
//Inputs		Requested velocity
//
//Returns		Possible Acceleration/Decelleration (in dodgy unit (1 = 1/1000 m/s2) )
//
//------------------------------------------------------------------------------
SLong AirStruc::CalcAccel(SLong reqvel)
{
	SLong retval = 0;
	if(reqvel < 0) return 0;

	FP fMass = classtype->weightempty;
	for(SWord i = 0; i < 4; i++)
			fMass += fly.fuel_content[i] * 0.001;
		fMass += (weap.left.mass + weap.centre.mass + weap.right.mass) * 0.001;
	fMass = classtype->weightempty / fMass;

//Piston Prop
	FP fPower = 2.0 * (classtype->abs_ceiling - World.Y) / classtype->abs_ceiling;
	MODMAXMIN(fPower, 0, 1);

	FP maxvel = CalcMaxVelSL();
	FP fVel = vel_;
	if(fVel < 300000)
		fVel = 300000;
	if(vel_ < reqvel)
		fVel = 1.0 / fVel - FP(vel_) * FP(vel_) / (maxvel * maxvel * maxvel);
	else
		fVel =      -1.25 * FP(vel_) * FP(vel_) / (maxvel * maxvel * maxvel);

	FP accel = classtype->maxaccel * 300000.0 * fMass * fPower * fVel;

	if((pitch != 0) && (vel_))
		accel -= (9806.5 * vel_y) / vel_;	//Gravity in Dodgy units

	if(		(vel_ < reqvel) && (accel > 0)
		||	(vel_ > reqvel) && (accel < 0)	)
		retval = (SLong)accel;

	return retval;
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AddAccel2Vel
//Author		Craig Beeston
//Date			Fri 12 Feb 1999
//
//Description	Adds a given acceleration to the AC velocity using the clock
//
//Inputs		Acceleration (in dodgy units (256 = 10 m/s2) )
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::AddAccel2Vel(SLong Accel)
{
	vel_ += Accel * Timer_Code.FRAMETIME / 10;


}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcHdgRollLimited
//Author		(R. Hyde) Craig Beeston
//Date			(Tue 28 Nov 1995) Tue 29 Sep 1998
//
//Description
//		 TR in deg per sec  = 360.g.tan RollAngle/(2.Pi.v)
//in Rowan units of 360 deg = 65536, v in 10cm/s units tan 45 = 256 not 1
//		 TR in cs			= 65536.98. tan roll/ 2.Pi.v.100.256
//							=  40 tan roll /v
//
//				This does the same as CalcHdgRoll but will limit the angle of roll so that
//				the speed can be calculated based assuming no roll
//
//				At present this will only be called by AutoFollowWp
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::CalcHdgRollLimited ()
{
	return CalcHdgRoll();
	SWord	reqdeltahdg;

	if (Save_Data.flightdifficulty [FD_WINDEFFECTS])
				WindCorrection();
	reqdeltahdg = 	(SWord) (hdg - HdgIntercept);
	return	 CalcHdgRollLimited(reqdeltahdg);
}

Bool AirStruc::CalcHdgRollLimited (SWord reqdeltahdg)
{
	return CalcHdgRoll(reqdeltahdg);

	SWord	deltaroll,currroll,tanroll;
	UWord	reqdeltahdg_pos;
	SWord	reqdeltahdg_sign;
	SLong	deltahdg;
	UWord	deltahdg_pos;
	SWord	deltahdg_sign;
	Bool	retval=FALSE;
	Math_Lib.Pos ((SWord) reqdeltahdg, reqdeltahdg_pos,reqdeltahdg_sign);
	if ((Angles)reqdeltahdg_pos <(ANGLES)ANGLES_ThirdDeg)					//CSB 31/03/99
	{
		retval=RollLevel (reqdeltahdg);
	}else
	{
		SWord	reqroll;

		reqroll = FindRequiredRoll (reqdeltahdg, RequiredBankData);

	//fix this !!!! to limit the angle of roll so that it can be assumed that
	//				the speed is not affected by the roll angle		//CSB 29/09/98
		if((SWord)pitch > 0)
		{
			SWord RollLimit;

			FP n1 = 1 - 0.7857 * World.Y / classtype->abs_ceiling;
			FP BestPitch = CalcBestClimbPitch();
			FP n2 = (SWord)pitch / BestPitch;
			n2 *= n2;
			n2 = 1 - 0.5 * n2;
			RollLimit = 12743 * n1 * n2;
			if(RollLimit < 1365) RollLimit = 1365;	//7.5 deg	at ceiling

			SWord reqroll_sign;
			UWord reqroll_pos;
			Math_Lib.Pos (reqroll, reqroll_pos, reqroll_sign);

			if(reqroll_pos > RollLimit)
			{
				if(reqroll_sign == -1) reqroll = -RollLimit;
				if(reqroll_sign ==  1) reqroll =  RollLimit;
			}
		}

		deltaroll = RequiredDRollSub(reqroll);							//RDH 11Oct96

		roll	 += (Angles) deltaroll;
		deltahdg = CalcHdg();
		Math_Lib.Pos ((SWord) deltahdg, deltahdg_pos,deltahdg_sign);
		if (deltahdg_pos <reqdeltahdg_pos)
		{
			hdg =  hdg + (Angles) deltahdg;
		}else
		{
			retval=RollLevel (reqdeltahdg);
		}
	}
	if((this != Persons2::PlayerSeenAC) && (this != Persons2::PlayerGhostAC))
		fly.aileron = -MAXAILERON * reqdeltahdg/ANGLES_20Deg;
	return(retval);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetMaccaVels
//Author		Robert Slater
//Date			Tue 20 Oct 1998
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	AirStruc::SetMaccaVels()
{
	FPMATRIX	mat;
	SWord		tmphdg = -hdg;
	ANGLES		thePitch = pitch;
	ANGLES		theHdg = (Angles) tmphdg;
	Float		x,y,z;
	Float		mulfrac = 1.0 / 32768.0;
	Float		sina,cosa;

	mat.L11 = 1;	//x.x
	mat.L12	= 0;	//x.y
	mat.L13 = 0;	//x.z
	mat.L21 = 0;	//y.x
	mat.L22 = 1;	//y.y
	mat.L23 = 0;	//y.z
	mat.L31 = 0;	//z.x
	mat.L32 = 0;	//z.y
	mat.L33 = 1;	//z.z

/*	if (thePitch==ANGLES_90Deg)
		thePitch=ANGLES_90BareDeg;
	else
	if (thePitch==ANGLES_270Deg)
		thePitch=ANGLES_270BareDeg;*/

	if (theHdg)
	{
		Math_Lib.high_sin_cos(theHdg,sina,cosa);

		x = (mat.L11 * cosa) + (mat.L31 * sina);
		y = (mat.L12 * cosa) + (mat.L32 * sina);
		z = (mat.L13 * cosa) + (mat.L33 * sina);

		mat.L31 = (mat.L31 * cosa) - (mat.L11 * sina);
		mat.L32 = (mat.L32 * cosa) - (mat.L12 * sina);
		mat.L33 = (mat.L33 * cosa) - (mat.L13 * sina);

		mat.L11 = x;
		mat.L12 = y;
		mat.L13 = z;
	}

	Math_Lib.high_sin_cos(thePitch,sina,cosa);
	if (thePitch)
	{

		x = (mat.L31 * cosa) + (mat.L21 * sina);
		y = (mat.L32 * cosa) + (mat.L22 * sina);
		z = (mat.L33 * cosa) + (mat.L23 * sina);

		mat.L21 = (mat.L21 * cosa) - (mat.L31 * sina);
		mat.L22 = (mat.L22 * cosa) - (mat.L32 * sina);
		mat.L23 = (mat.L23 * cosa) - (mat.L33 * sina);

		mat.L31 = x;
		mat.L32 = y;
		mat.L33 = z;
	}

	Float	xx = mat.L31;
	Float	yy = mat.L32;
	Float	zz = mat.L33;
	Float	maxpoo;

	maxpoo = xx*xx;
	maxpoo += yy*yy;
	maxpoo += zz*zz;

	double mag = (double) maxpoo;
/*	_asm
	{
		fld mag;
		fsqrt;
		fstp mag;
	}*/
	mag = sqrt(mag);

	maxpoo = 1.0 / mag;

	xx *= maxpoo;
	yy *= maxpoo;
	zz *= maxpoo;

	xx *= Float(vel_);
	yy *= Float(vel_);
	zz *= Float(vel_);

	// Velocity
	vel_x = SLong(xx);
	vel_y = SLong(yy);
	vel_z = SLong(zz);

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CheckForWeapon
//Author		Robert Slater
//Date			Fri 26 Mar 1999
//
//Description	Check to see if the aircraft is carrying any of the given weapon
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::CheckForWeapon(UByte ltype)//Check for stores...
{
	if (Anim != NULL)													//RJS 15Nov00
	{
		MoveGunAnimData*	adptr = (MoveGunAnimData*) Anim;
		WeapAnimData*	weapon;
		int				index;

		if((fly.numinsag) && (ltype == LT_BULLET))
			return(TRUE);

		for (index=0; index < adptr->nolaunchers ; index++)
		{
			weapon = &adptr->weaponlaunchers[index];
			if (	weapon
				&& ((weapon->LauncherType & LT_MASK) == (ltype & LT_MASK))		  //RJS 01Apr99
				&&	(weapon->LoadedStores > 0))
				return TRUE;
		}
	}

	return FALSE;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CheckForAnyWeapon
//Author		Craig Beeston
//Date			Fri 26 Mar 1999
//
//Description	Check to see if the aircraft is carrying any weapons for ground attack
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::CheckForAnyWeapon()//Check for stores...
{
	Bool retval = FALSE;
	if (Anim != NULL)													//MS 15Nov00
	{
		if(fly.numinsag)
		{
			retval = TRUE;
		}
		else
		{
			MoveGunAnimData*	adptr = (MoveGunAnimData*) Anim;
			WeapAnimData*	weapon;
			int				index;

			for (index=0; index < adptr->nolaunchers; index++)
			{
				weapon = &adptr->weaponlaunchers[index];
				if ((weapon) && (weapon->LoadedStores > 0))
				{
					switch (weapon->LauncherType & LT_MASK)
					{
					case LT_BULLET:
					case LT_BOMB:
					case LT_ROCKET:
						retval = TRUE;
						break;
					}
				}
			}
		}
	}

	return retval;
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ChooseAWeapon
//Author		Craig Beeston
//Date			Fri 2 Apr 1999
//
//Description	Choose a weapon to use for dive bomb
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::ChooseAWeapon()
{
	Bool RetVal = FALSE;
	if(CheckForWeapon(LT_BOMB))
	{
		weap.weapontype = LT_BOMB;
		RetVal = TRUE;
	}
	else if(CheckForWeapon(LT_ROCKET))
	{
		weap.weapontype = LT_ROCKET;
		RetVal = TRUE;
	}
	else if(CheckForWeapon(LT_BULLET))
	{
		weap.weapontype = LT_BULLET;
		RetVal = TRUE;
	}
	return(RetVal);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CarryingNapalm
//Author		Craig Beeston
//Date			Thu 29 Apr 1999
//
//Description	Check to see if the aircraft is carrying napalm rather than bombs
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::CarryingNapalm()
{
	AircraftAnimData*	adptr = (AircraftAnimData*) Anim;
	WeapAnimData*	weapon;
	int				index;

	for (index=0; index < 5; index++)	//RERUN: was <8, OOB on weaponlaunchers[5] (read imagemap bytes as WeapAnimData)
	{
		weapon = &adptr->weaponlaunchers[index];
		if (	weapon
			&& ((weapon->LauncherType & LT_MASK) == (LT_BOMB & LT_MASK))
			&&	(weapon->LoadedStores > 0))
			if(weapon->LauncherType == LT_NAPALM) return TRUE;
	}

	return FALSE;
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SelectBombTarget
//Author		Craig Beeston
//Date			Sat 3 Apr 1999
//
//Description	Choose a target to bomb depending on duty
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::SelectBombTarget(Bool NewTarg)
{
	Bool RetVal = FALSE;
	return RetVal;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AnyoneInPhaseN
//Author		Craig Beeston
//Date			Sat 3 Apr 1999
//
//Description	Checks if any member of this group is in manoeuvre pahse N
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::AnyoneInPhaseN(char PhaseNum)
{
 	Bool retval = FALSE;
	AirStrucPtr grplead = FindGroupLeader();

	for(AirStrucPtr lead = grplead; (lead) && (!retval); lead = lead->fly.nextflight)
		for(AirStrucPtr foll = lead; (foll) && (!retval); foll = foll->Follower())
			if(foll->ai.ManStep == PhaseNum)
				retval = TRUE;

	return(retval);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		IsThisBombLeader
//Author		Craig Beeston
//Date			Fri 23 Apr 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool AirStruc::IsThisBombLeader()
{
 	if(formpos == 0)
		return(TRUE);
	else
	{
		if((waypoint) && (waypoint->skipunder))
			if(uniqueID.count == waypoint->skipunder)
				return(TRUE);
	}
	return(FALSE);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		IsLocalLeader
//Author		Craig Beeston
//Date			Tue 11 May 1999
//
//Description	Decide whether AC is a bombing leader depending on duty
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool AirStruc::IsLocalLeader()
{
	int flightpos = position();
	char localduty = duty;// & ACTIONSUBCODE;							//JIM 22/03/00
	switch (localduty)		//All other duties will straffe!
	{
	case	PACK_AM_RECON:
		break;	//shouldn't get here
	case	PACK_AM_DIVEBOMB:
		return (true);
	case	PACK_AM_LEVELBOMB:
		if(IsThisBombLeader())
			return(true);
		else
			return(false);
	case	PACK_AM_CHAINBOMB:
		if(flightpos==0)
			return(true);
		else
			return(false);
	}
	return(true);	//Shouldn't get this far
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CountGroup
//Author		Craig Beeston
//Date			Fri 23 Apr 1999
//
//Description	Counts the number of aircraft in this group
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
UWord AirStruc::CountGroup()
{
 	UWord NumAC = 0;
	AirStrucPtr grplead = FindGroupLeader();

	for (AirStrucPtr lead = grplead;lead;lead=lead->fly.nextflight)
		for (AirStrucPtr foll=lead;foll;foll=foll->Follower())
			NumAC++;

	return(NumAC);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		RotateToHdg
//Author		Craig Beeston
//Date			Fri 23 Apr 1999
//
//Description	Takes a vector and works out its components in a given heading
//
//Inputs		MULSHSIN
//
//Returns
//
//------------------------------------------------------------------------------
COORDS3D* AirStruc::RotateToHdg(COORDS3D GlobDelta, COORDS3D& LocalDelta, ANGLES Heading)
{
	Float SinHdg, CosHdg;
	Math_Lib.high_sin_cos (Heading, SinHdg, CosHdg);
	FP DX = CosHdg * FP(GlobDelta.X) - SinHdg * FP(GlobDelta.Z);
	FP DZ = CosHdg * FP(GlobDelta.Z) + SinHdg * FP(GlobDelta.X);

	LocalDelta.X = SLong(DX);
	LocalDelta.Y = GlobDelta.Y;
	LocalDelta.Z = SLong(DZ);
	return(&LocalDelta);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindLeadUID
//Author		Craig Beeston
//Date			Mon 7 Jun 1999
//
//Description	Finds the bombing leaders UniqueID
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
UniqueID AirStruc::FindLeadUID()
{
	if((waypoint) && (waypoint->skipunder))
		return(waypoint->skipunder);

	AirStrucPtr GrpLead = FindGroupLeader();
		return(GrpLead->uniqueID.count);

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		WindTrackCorrection
//Author		Craig Beeston
//Date			Thu 10 Jun 1999
//
//Description	Calculates the change in hdg needed to compensate for the wind
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::WindTrackCorrection(SLong Vel, SWord DesHdg, SWord DesPitch, SLong& dVel, SWord& dHdg, SWord& dPitch)
{
	SWord WindHdg;
	SLong WindVel;
	MissManCampSky().GetWindDirVel(World.Y, WindHdg, WindVel);
	if(!WindVel)
	{
		dHdg   = 0;
		dPitch = 0;
		dVel   = 0;
		return;
	}

	Float vel     = Vel;
	Float windvel = WindVel;
	SWord DeltaHdg = DesHdg - WindHdg + ANGLES_180Deg;

	Float sindhdg, cosdhdg;
	Math_Lib.high_sin_cos((Angles)DeltaHdg, sindhdg, cosdhdg);

	Float sinpitch, cospitch;
	Math_Lib.high_sin_cos((Angles)DesPitch, sinpitch, cospitch);

	Float NewVelHori = (vel * cospitch) + (windvel * cosdhdg);
	FP HdgCorr = WindVel * sindhdg / (NewVelHori * FPIE);

	FP PitchCorr = 10429.0 * windvel * sinpitch * cosdhdg / vel;

	FP VelCorr = windvel * cosdhdg * cospitch;

	dHdg   = HdgCorr;
	dPitch = PitchCorr;
	dVel   = VelCorr;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		WindBulletDrift
//Author		Craig Beeston
//Date			Fri 11 Jun 1999
//
//Description	Calculates the resulting change in velocity, pitch and heading due to the effects of the wind
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::WindBulletDrift(SLong Vel, SWord DesHdg, SWord DesPitch, SLong& dVel, SWord& dHdg, SWord& dPitch)
{
	SWord dhdg, dpitch;
	WindTrackCorrection(Vel, DesHdg, DesPitch, dVel, dhdg, dpitch);

	dHdg   = -dhdg;
	dPitch = -dpitch;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcBulletVel
//Author		Craig Beeston
//Date			Fri 11 Jun 1999
//
//Description	Calculates the velocity, pitch and heading of a bullet including the effects of the wind
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::CalcBulletVel(SLong MuzVel, SWord &Hdg, SWord &Pitch, SLong &BullVel)
{
	SWord dHdg, dPitch;
	SLong dVel;
	SLong bullvel   = vel_ + MuzVel;
	SWord bullhdg   = hdg;
	SWord bullpitch = pitch;
	WindBulletDrift(bullvel, hdg, pitch, dVel, dHdg, dPitch);

	bullvel   += dVel;
	bullhdg   += dHdg;
	bullpitch += dPitch;

	if(bullpitch > ANGLES_90Deg)
	{
		bullpitch =  ANGLES_180Deg - bullpitch;
		bullhdg   += ANGLES_180Deg;
	}
	if(-bullpitch > ANGLES_90Deg)
	{
		bullpitch = -ANGLES_180Deg - bullpitch;
		bullhdg   += ANGLES_180Deg;
	}

	BullVel = bullvel;
	Hdg     = bullhdg;
	Pitch   = bullpitch;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcBulletVel
//Author		Craig Beeston
//Date			Fri 11 Jun 1999
//
//Description	Calculates the velocity, pitch and heading of a bullet including the effects of the wind
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void AirStruc::CalcBulletVel(SLong MuzVel, SLong posx, SLong posy, SLong posz, SLong& vx, SLong& vy, SLong& vz)
{
	const FP CONVERGENCE = 22500;
	FP muzvel = MuzVel;
	Float sinh, cosh, sinp, cosp;
	Math_Lib.high_sin_cos(hdg, sinh, cosh);
	Math_Lib.high_sin_cos(pitch, sinp, cosp);

	SLong wx, wy, wz;
	MissManCampSky().GetWind(World.Y, wx, wy, wz);

	FP velx = muzvel * ((World.X + CONVERGENCE * cosp * sinh) - FP(posx)) / CONVERGENCE + vel_x + wx;
	FP vely = muzvel * ((World.Y + CONVERGENCE * sinp       ) - FP(posy)) / CONVERGENCE + vel_y + wy;
	FP velz = muzvel * ((World.Z + CONVERGENCE * cosp * cosh) - FP(posz)) / CONVERGENCE + vel_z + wz;

	vx = velx;	vy = vely;	vz = velz;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		UpdateDeathTime
//Author		Robert Slater
//Date			Mon 20 Dec 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void MoveAirStruc::UpdateDeathTime(UWord	deadTimeOut, bool	launchTransient)
{
	if (fly.pModel->CrashTime < DEADTIMEOUT_RECYCLE)					//RJS 10Oct00
	{
		fly.pModel->CrashTime += Timer_Code.FRAMETIME;//RJS 25Aug98

		if (	(this != Persons2::PlayerSeenAC)
			&&	(fly.pModel->CrashTime > deadTimeOut)	)
		{
			fly.pModel->CrashTime = DEADTIMEOUT_RECYCLE;				//RJS 10Oct00

			MinAnimData*	mad = (MinAnimData*)Anim;
			if ((mad->itemstate != DEAD) && !mad->IsInvisible)
			{
				movecode = AUTO_NOPPILOT;								//RJS 13Aug00
				if (launchTransient)
				{
					WorldStuff*	world = mobileitem::currworld;

					Trans_Obj.LaunchACTransient((mobileitem*)*this,classtype->SAGsubshape,*world);
				}
				else
				{
					vel_x = vel_y = vel_z = vel_ = 0;				//RJS 10Oct00

					mad->itemstate = DEAD;
					mad->IsInvisible = 1;
					if (	Trans_Obj.View_Object
						&&	(Trans_Obj.View_Object->trackeditem2 == this)	)
						Trans_Obj.View_Object->trackeditem2 = NULL;
				}
			}
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoBloke
//Author		Robert Slater
//Date			Mon 10 Jan 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void formationitem::AutoBloke ()
{
	if (movecode == GROUND_BLOKE)
	{
		if ((timeofday & 0x04)==0)							//Same as transients
			World.Y = Trans_Obj.GetGroundLevel(this);					//RJS 23May00

		// These blokes are specifically for airfields, which will always be flat.
		if (leader && leader->waypoint==waypoint)
		{	//quick and dirty
	 		vel_x=leader->vel_x;
			vel_y=0;
	 		vel_z=leader->vel_z;
	 		vel_=leader->vel_;
			pitch=leader->pitch;
			roll=leader->roll;
			NewPosition ();

			if (vel_)
			{
				hdg=leader->hdg;
				HdgIntercept = leader->HdgIntercept;

				Anim_Control.SetPlayerAnim(this,ANIMATION_WALK);
			}
			else
				BlokeTalk();											//RJS 17Apr00
		}
		else
		{

			if (	roll != ANGLES_0Deg
				||	!TimeLeft()
				)
			{
				Anim_Control.SetPlayerAnim(this,ANIMATION_WALK);

				Coords3D p=*FindDesPos ();
					p.Y=World.Y;
				InterceptandRange (&p);

				roll=ANGLES_0Deg;

				SLong	reqvel;
				if (waypoint && waypoint->_vel)
		 			reqvel =  waypoint->_vel;
				else
					reqvel =  classtype->cruisevel;

				UWord	absangle = AbsAngle(hdg-HdgIntercept);
				if (absangle>ANGLES_30Deg)
				{
					reqvel*=float(ANGLES_180Deg-absangle)/ANGLES_180Deg;
				}
				Bool	level=CalcVelSub (reqvel);
						level&=CalcHdgRollFlat ();
				pitch=ANGLES_0Deg;

				CalcXYZVel ();

				if (vel_==0)	vel_=1;

				SLong	rangetime=(Range*10000)/vel_;
				if (	Range<desposrange
					||	rangetime<=Timer_Code.FRAMETIME*4
					)
				{
					ExecuteWayPointAction ();
				}
				else
				{
	/*				rangetime -=600;
					if (rangetime>30*100)
						rangetime=30*100;
					if (	level
						&&	rangetime>0)
						SetManoeuvreTime(rangetime);*/
				}

				if (Math_Lib.rnd()<256)								//RJS 17Apr00
					manoeuvretime = 500 + Math_Lib.rnd(500);		//RJS 17Apr00

				vel_y=0;
				NewPosition ();
			}
			else
			{
				if (vel_)
				{
					if (follower)
						BlokeTalk();
					else
						BlokeDawdle();
				}
			}
		}
	}
	else
		Trans_Obj.MoveBlokeWithBrain(this,manoeuvretime);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AutoEmergencyVehicle
//Author		Robert Slater
//Date			Mon 10 Jan 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void formationitem::AutoEmergencyVehicle ()
{
	if (leader && leader->waypoint==waypoint)
	{	//quick and dirty
	 	vel_x=leader->vel_x;
	 	vel_y=leader->vel_y;
	 	vel_z=leader->vel_z;
	 	vel_=leader->vel_;
		hdg=leader->hdg;
		pitch=leader->pitch;
		roll=leader->roll;
		vel_y=contourlist->GetVVel(this);
		NewPosition ();
	}
	else
	{
		if (	roll != ANGLES_0Deg
			||	!TimeLeft()
			)
		{
			Coords3D p=*FindDesPos ();
			p.Y=World.Y;

			InterceptandRange (&p);

			roll=ANGLES_0Deg;

			SLong	reqvel;
			if (waypoint && waypoint->_vel)
		 		reqvel =  waypoint->_vel;
			else
				reqvel =  classtype->cruisevel;

			UWord	absangle = AbsAngle(hdg-HdgIntercept);
			if (absangle>ANGLES_30Deg)
			{
				reqvel*=ANGLES_180Deg-absangle;
				reqvel/=ANGLES_180Deg;
			}
			Bool	level=CalcVelSub (reqvel);

			level&=CalcHdgRollFlat ();

			pitch=ANGLES_0Deg;

			CalcXYZVel ();

			if (vel_==0)	vel_=1;

			SLong	rangetime=(Range*10000)/vel_;
			if (	Range<desposrange
				||	rangetime<=Timer_Code.FRAMETIME*4
				)
			{
				ExecuteWayPointAction ();
			}
			else
			{
				rangetime -=600;
				if (rangetime>30*100)
					rangetime=30*100;
				if (	level
					&&	rangetime>0)
					SetManoeuvreTime(rangetime);
			}
		}
		vel_y=contourlist->GetVVel(this);
		NewPosition ();
	}
}

void formationitem::AutoSlowTaxi ()
{
	if (leader && leader->waypoint==waypoint)
	{	//quick and dirty
	 	vel_x=leader->vel_x;
	 	vel_y=leader->vel_y;
	 	vel_z=leader->vel_z;
	 	vel_=leader->vel_;
		hdg=leader->hdg;
		pitch=leader->pitch;
		roll=leader->roll;
		vel_y=contourlist->GetVVel(this);
		NewPosition ();
	}
	else
	{
		if (	roll != ANGLES_0Deg
			||	!TimeLeft()
			)
		{
			pitch=ANGLES_0Deg;

			vel_ -= Timer_Code.FRAMETIME*500;	//(frametime*5ms)/100

			CalcXYZVel ();

			if (vel_<=0)
			{
				vel_= vel_x = vel_y = vel_z = 0;
				movecode = AUTO_NOPPILOT;
			}
		}
		vel_y=contourlist->GetVVel(this);
		NewPosition ();
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		BlokeDawdle
//Author		Robert Slater
//Date			Mon 10 Jan 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	mobileitem::BlokeDawdle()
{
	vel_ = vel_x = vel_y = vel_z = 0;


	if (Anim_Control.Finished(this,ANIMATION_BORED) || Anim_Control.NeverStarted(this,ANIMATION_BORED))
		Anim_Control.SetPlayerAnim(this,ANIMATION_BORED);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		BlokeTalk
//Author		Robert Slater
//Date			Mon 10 Jan 2000
//
//Description	Turn to faceeach other
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	mobileitem::BlokeTalk()
{
	formationitem*	formthis = (formationitem*)this;

	vel_ = vel_x = vel_y = vel_z = 0;


	if (Anim_Control.Finished(this,ANIMATION_TALK) || Anim_Control.NeverStarted(this,ANIMATION_TALK))
	{
		SLong	tmpx = 0;
		SLong	tmpz = 0;

		formationitem*	tmpitem = formthis->leader;
		if (!tmpitem)
			tmpitem = formthis->follower;

		tmpx = tmpitem->World.X - World.X;
		tmpz = tmpitem->World.Z - World.Z;

		formthis->HdgIntercept = Math_Lib.arctan(tmpx,tmpz);

		Anim_Control.SetPlayerAnim(this,ANIMATION_TALK);
	}

	SWord	hdgDelta = SWord(HdgIntercept);
	hdgDelta -= SWord(hdg);

	hdg += Angles(hdgDelta>>1);
}
