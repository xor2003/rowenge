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

//Filename       flymodel.cpp
//System
//Author         R. Hyde
//Date           Mon 11 Sep 1995
//Description    THIS IS THE OLD INTEGER BASED FLIGHT MODEL, OR REMNANTS OF IT!
//------------------------------------------------------------------------------

//#define		_NOFLIGHTMODEL_

#include	"dosdefs.h"
#define F_GRAFIX
#define F_COMMON
#include	"files.g"
#include	"worldinc.h"
#include	"planetyp.h"
#include	"myangles.h"
#include	"enumbits.m"
#include	"mytime.h"											//RDH 25Apr96
//MATHABLE	(ANGLES)
#include	"flymodel.h"
#include	"mymath.h"
#include	"vertex.h"
#include	"matrix.h"											//RDH 23May96
//TEMPCODE DAW 13/04/00 #include	"textref.h"
#include	"viewsel.h"											//PD 19Feb96
#include	"hardpasm.h"										//RDH 14Mar96
#include	"3dcom.h"											//PD 16Apr96
#include	"landscap.h"									//PD 15Mar96
#include	"missman2.h"										//JIM 22Aug96
#include	"savegame.h"
#include	"miles.h"											//DAW 04Jul96
#include	"world.h"											//DAW 04Jul96
#include	"transite.h"										//RDH 26Jul96
#include	"collided.h"										//PD 06Sep96
#include	"speed.h"
#include "persons2.h"											//RDH 24Jul96
#include	"ranges.h"
#include	"winmove.h"											//ARM 10Oct96
#include	"modvec.h"											//ARM 08Jan97
#include	"model.h"											//ARM 07Jan97
#include	"shapes.h"
#include	"replay.h"
#include	"sqddiary.h"
#include	"boxcol.h"													//AMM 4Jul00

//extern	ULong	GR_Quit3DNow;
extern DPlay _DPlay;

										//ARM 06Nov96
//-----------------------------------------------------------------------------------------
//1	Permanent notes on the flight model
//
//
//	1	Units
//			 	vel,i_a_s			10 cm/s
//				vel_cm  			cm/s
//				vel_mm  			cm/s
//				WorldX,Y,Z			cm
//				forces				N
//				airdensity			g/m^3
//				sos					m/s
//				angles				all use full word
//				trig functions		signed word, 7fff = 1.0, 8000 = -1
//
//	2	In the model, y is reversed:
//
//			cposy = -ControlledAC->World.Y; 		world y postion of ac
//
//	3	Angles
//			    roll right wing down = +ve
//				hdg change to right  = +ve
//				pitch up			 = +ve
//
//		When the ac is pointing above the velocity vector, then aoa = +ve
//		When the ac has a bigger heading than velocity vector, then slip = -ve
//
//
//


//-----------------------------------------------------------------------------------------
//2	Notes on relationship with rest of code
//
//
//	1	We calculate the position of a point on the aircraft. There are a number of
//		significant points:
//					pilot eye
//					centre around which aircraft turns
//					centre of drawn shape
//		At present we don't distinguish between two
//
//	2	Changing from auto to manual has changed, I will probably need to know
//		so that trim can be allowed
//
//	3
//
//	8
//	 	if (ControlledAC2 == ControlledAC)
//				true for manual pilot
//

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//INSTANCE		Manual_Pilot
//Author		Jim Taylor
//Date			Tue 31 Oct 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
ManualPilot	Manual_Pilot;

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ManualPilot	CONSTRUCTOR
//Author		R. Hyde
//Date			Thu 26 Oct 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
	ManualPilot::ManualPilot()
{
	View_Point=NULL;
// 	machdata=Mach_Data;
//	airdensitydata=AirDensity_Data;
//	airdensityratio=AirDensity_Ratio;
//	shokdata=Shok_Data;
	ControlledAC2=NULL;

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Associate
//Author		Jim Taylor
//Date			Tue 31 Oct 1995
//
//Description	Associates specific 3d item to manual movecode.
//				May also be used to flag auto/manual switch,
//				so put any trim init code here!
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	ManualPilot::Associate	(
			AirStruc*	manualas,				//item to move
			CONTROLMODE		inmanual)		//TRUE	//flag for man/auto/auto accel
{
	if  (inmanual!=controlmode || ControlledAC2!=manualas)
	{
		controlmode=inmanual?MANUAL:AUTO;
		ControlledAC2=manualas;
		InitManualPilot(ControlledAC2);
	}
}
//JIM 15Oct96

void	ManualPilot::AutoToggle(CONTROLMODE dead)
{
	if (dead==PILOTDEAD)
		controlmode=PILOTDEAD;
	else
		if (controlmode!=PILOTDEAD)								//JIM 11Oct96
		{														//JIM 11Oct96
			ControlledAC2->information=IF_ALLBAD;
			ControlledAC2->slowdownleader=FALSE;
			if (dead==TOGGLE)									//JIM 11Oct96
			{
				dead=controlmode==AUTO?MANUAL:AUTO;
				Associate(ControlledAC2,dead);
			}
			else AutoToggleAircraft(ControlledAC2,dead);					//rdh 6/4/98
		}														//JIM 11Oct96
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Author		R. Hyde
//Date		Wed 30 Aug 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	ManualPilot::MainManualPilot()
{
//TempCode PD 3Nov97
//	return;
//TempCode PD 3Nov97

//Add timing thing here	//CSB 02/03/99

	if (ControlledAC2)
	{
		WorldStuff &world = mobileitem::GetWorld();
		UWord	oldsector=	world.GetSector(ControlledAC2);


///////////////////////////////////////////
//tmp fix should not be running acm on player ac in final version//RDH 20Aug96
		if(controlmode == MANUAL)
		{
			if((((Persons2::PlayerGhostAC->timeofday) & 0x07ff) == 0) && (Persons2::PlayerGhostAC->slowdownleader))
			{
				Persons2::PlayerGhostAC->slowdownleader = FALSE;		  //JIM 22/03/99
				AirStrucPtr found = NULL;
				for(AirStrucPtr l = Persons2::PlayerGhostAC; (l) && (!found); l = l->fly.nextflight)
					for(AirStrucPtr w = l; (w) && (!found); w = w->Follower())
						if((w->movecode == AUTO_FOLLOWWP) && (w->information == IF_OUT_POS) && (Persons2::PlayerGhostAC->vel_ > w->vel_))
						{
							found = w;
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_TOOFAST, MSG_FORMATIONFLYING_RPT, found, NULL, Persons2::PlayerGhostAC));
						}
			}

			GetKeyCommon (ControlledAC2);				//RDH 15Dec95

			if (!_Replay.Playback)
			{
				_DPlay.CheckSeen();
				_DPlay.AddDeltas(Persons2::PlayerSeenAC,FALSE,1);
#ifndef	_NOFLIGHTMODEL_
 				FlightModelFixedTime(Persons2::PlayerSeenAC);
#endif
 				Persons2::PlayerSeenAC->SetManualFlightParams ();

				_DPlay.AddDeltas(Persons2::PlayerSeenAC,TRUE,1);
				_DPlay.BackupSeen();
			}


			SHAPE.RipStores(ControlledAC2);						//RJS 03Jun98

//			ControlledAC2->CheckManualCourse();					//JIM 03Dec96
//being done by macca.
		}
		else if (controlmode==AUTOACCEL_WAYPT || controlmode==AUTOACCEL_FWD ||
				controlmode==AUTOACCEL_MSG || controlmode==AUTOACCEL_MISS ||
				controlmode==AUTOACCEL_HOME)
		{
			_DPlay.CheckSeen();
			_DPlay.AddDeltas(Persons2::PlayerSeenAC,FALSE,2); 				//AMM 10Mar99
			AutoMoveCodeTypeSelect bupMoveCode=ControlledAC2->movecode;
			ControlledAC2->movecode=AUTO_ACCELERATE;
			mobileitem::MoveItem((MobileItemPtr)ControlledAC2,world);
			ControlledAC2->movecode=bupMoveCode;
			_DPlay.AddDeltas(Persons2::PlayerSeenAC,TRUE,2);				//AMM 10Mar99
			_DPlay.BackupSeen();
		}
		else
		{
//			if (controlmode == MANUAL)								//RDH 20Aug96
//				GetStickKeys(ControlledAC2);

			if (!_Replay.Playback)
			{
// this moves players ac if player is not in control i.e autospiral, autores etc

//				WorldStuff &world = mobileitem::GetWorld();

				_DPlay.CheckSeen();
				_DPlay.AddDeltas(ControlledAC2,FALSE,3);
				mobileitem::MoveItem((MobileItemPtr)(ControlledAC2),world);
				_DPlay.AddDeltas(ControlledAC2,TRUE,3);
				_DPlay.BackupSeen();

// update info line
				if (_DPlay.Implemented)
		 			OverLay.SetInfoLineVals(Persons2::PlayerSeenAC->vel_ * 0.001, 0, Persons2::PlayerSeenAC->World.Y * (3.2808 / 100), Persons2::PlayerSeenAC->hdg / 182, Persons2::PlayerSeenAC->fly.thrustpercent);
			}
			else												//AMM 31Aug98
			{													//AMM 31Aug98
// shouldnt ever get in here?
				INT3;

 			}													//AMM 31Aug98


//			View_Object->right_aoa = ControlledAC2->fly.aoa;
//			View_Object->left_aoa = ControlledAC2->fly.aoa;
		}

///////////////////////////////////////////

		if (ControlledAC2->uniqueID.changed)
		{
			UWord	newsector=	world.GetSector(ControlledAC2);
			if (newsector!=oldsector)
			{
				world.RemoveFromSector(ControlledAC2,oldsector);
				world.AddToWorld(ControlledAC2);
			}
			ControlledAC2->uniqueID.changed = FALSE;						//RDH 26Jun96
		}

 	}
//Add timing thing here	//CSB 02/03/99

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DeathSequenceOverride
//Author		Robert Slater
//Date			Wed 15 Apr 1998
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
Bool	ManualPilot::DeathSequenceOverride(ItemPtr	itm, int	newmovecode,bool commscalled)//AMM 26Jan99
{

	Bool	wasset = FALSE;										//DAW 03Sep98
	if (itm->Status.size == AIRSTRUCSIZE)						//RJS 25Nov99
	{
		AirStrucPtr		acptr = (AirStrucPtr) itm;

		if((acptr->movecode == AUTO_SPAREAC) || (acptr->movecode == AUTOSAG_DESTROYED))
		{
			INT3;
			return(TRUE);
		}

		if (!acptr->fly.numinsag)								//MS 20Jun00
		{
			acptr->ai.ManStep = 0;								//RJS 03Nov00

			MinAnimData*	mad = (MinAnimData*) acptr->Anim;

			if (	(acptr->movecode != newmovecode)
				&&	(acptr->movecode != AUTO_NOPPILOT)					//RJS 27Aug98
				&&	(acptr->movecode != AUTO_DEATHSEQUENCE)
				&&	(acptr->movecode != AUTO_HITWATER)					//RJS 27Aug98
				&&	(acptr->movecode != AUTO_CRASHSKID)					//RJS 27Aug98
				&&	(acptr->movecode != AUTO_CRASHROLL)					//RJS 27Aug98
				&&	(acptr->movecode != AUTO_CRASHONEWHEEL)			  //CSB 18/02/00
				&&	(acptr->movecode != AUTO_CRASHNOSE)				  //CSB 18/02/00
				&&	(acptr->movecode != AUTO_CRASHFLIP)				  //CSB 18/02/00
				&&	(acptr->movecode != AUTO_CRASHTUMBLE)			  //CSB 18/02/00
				&&	(acptr->movecode != AUTO_SPIRAL2GROUND)	)		//DAW 03Sep98
			{
				if (itm == Persons2::PlayerSeenAC)						//RJS 30Oct98
				{
					Squad_Diary.UpdatePlayerLog(EFS_KIA);	//RJS 18Feb00

					controlmode=PILOTDEAD;
//Dead					acptr->Status.deadtime = 1000;//Just to make sure...

					if (_Replay.Record)									//RJS 30Oct98
					{
						UByte	effect = RDEATH_NULL;

						switch (newmovecode)
						{
							case AUTO_HITWATER:
							case AUTO_CRASHSKID:
								effect = RDEATH_SKID;
								break;
							case AUTO_CRASHROLL:
								effect = RDEATH_ROLL;
								break;
							case AUTO_SPIRAL2GROUND:
							case AUTO_DEATHSEQUENCE:
								effect = RDEATH_DIVE;
								break;
						}

		 				_DPlay.NewSpecial(DPlay::PIDC_DEATHMOVEEFFECT,effect);
					}
				}

				acptr->movecode = (AutoMoveCodeTypeSelect) newmovecode;
				wasset = TRUE;										//DAW 03Sep98

				if (!acptr->Status.deadtime)						//RJS 17May99
				{
// Must only come in here once...
					ItemPtr	lasthitter = acptr;
					if (acptr->lasthitter.count)
					{
						lasthitter = (ItemPtr)Persons2::ConvertPtrUID((UniqueID)acptr->lasthitter.count);
						if (!lasthitter)  		//checks to see if it's an airstruc within ScoreSimpleItemKilled
							lasthitter = acptr;
					}

					Squad_Diary.AddKill(lasthitter,acptr);		//RJS 21Feb00


					acptr->SetUnfriendly(NULL);
					acptr->BreakForm();								//RJS 17May99
				}
			}
			else
			{
				//It's ok to say you can't move anymore,
				//but not ok to say you can...
				if (newmovecode == AUTO_NOPPILOT)
					acptr->movecode = (AutoMoveCodeTypeSelect) newmovecode;
			}

			//Make sure it is dead!
			ShapeDescPtr	sdptr = SHAPESTUFF.GetShapePtr(acptr->shape);	//RJS 17May99
			acptr->Status.deadtime = sdptr->DeadTime;						//RJS 17May99
			acptr->Status.deadscale = sdptr->DeadScale;						//RJS 17May99
	 		acptr->Status.deaded = TRUE;									//RJS 17May99
			if (acptr == Persons2::PlayerSeenAC)				//RJS 09Jul99
			{
				Persons2::PlayerGhostAC->Status.deadtime = sdptr->DeadTime;
				Persons2::PlayerGhostAC->Status.deadscale = sdptr->DeadScale;
				Persons2::PlayerGhostAC->Status.deaded = TRUE;
				Persons2::PlayerGhostAC->overview_movecode=acptr->overview_movecode;
			}

			mad->itemstate = DEAD;								//RJS 23May99

			if (sdptr->AnimDataSize == AIRCRAFTANIM)
			{
				AircraftAnimData* adptr = (AircraftAnimData*)acptr->Anim;//DAW 09Jun99
				adptr->gunshooting = 0;									//RJS 25Aug00
				adptr->cannonshooting = 0;									//RJS 25Aug00
			}

			if(		(acptr->leader) || (acptr->follower) || (acptr->fly.leadflight) || (acptr->fly.nextflight)	//CSB 8Aug00
				||	(acptr->fly.expandedsag) || (acptr->ai.unfriendly) || (acptr->ai.attacker)	)				//CSB 8Aug00
				acptr->BreakForm();																				//CSB 8Aug00

		}
		if (	(acptr->movecode == AUTO_NOPPILOT)							//RJS 22Jun00
			&&	(	!(_Replay.Record || _Replay.Playback || _DPlay.Implemented)
				||	!BoxCol::GetTestState()	))
			acptr->vel_x = acptr->vel_y = acptr->vel_z = acptr->vel_ = 0;	//RJS 22Jun00
	}

#ifndef NDEBUG															//CSB 14Aug00
	if(itm->Status.size == AIRSTRUCSIZE)								//CSB 14Aug00
		AirStrucPtr(itm)->DoAllSAGChecks();								//CSB 14Aug00
#endif																	//CSB 14Aug00

	AirStrucPtr player = Persons2::PlayerGhostAC;
	if((itm->Status.size == AIRSTRUCSIZE) && (AirStrucPtr(itm)->nationality != player->nationality))
		if(!player->fly.expandedsag)
		{
			bool enemyleft = false;
			for(AirStrucPtr ac = AirStrucPtr(mobileitem::ACList); (ac) && (!enemyleft); ac = AirStrucPtr(ac->nextmobile))
				if((ac->nationality != player->nationality) && (!ac->Status.deadtime))
					if(player->Distance3DSquared(&ac->World) < FP(2 * VISIBLERANGE) * FP(2 * VISIBLERANGE))
						enemyleft = true;

			if(!enemyleft)
				Art_Int.SendMsgAndCall(MESSAGE_STRUC(PHRASE_THINKING, MSG_CLEAR, player, player, player), SetupQuitContinue);
		}

	return(wasset);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FlightModelFixedTime
//Author		R. Hyde
//Date			Sun 7 Apr 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	ManualPilot::FlightModelFixedTime(AirStrucPtr const ControlledAC2)
{
	UWord	loopcount;
	ANGLES	test;

	groundlevel = _Collide.LowestSafeAlt(ControlledAC2,
											ControlledAC2->pitch,
											ControlledAC2->roll);//RDH 21Oct96

//to allow for small changes in ground level add 2 cm			//RDH 29Nov96
	groundlevel +=2;											//RDH 29Nov96

		ControlledAC2->fly.pModel->Base (ControlledAC2);							//ARM 07Jan97





//need a test here for dead aircraft +++


}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Author		R. Hyde
//Date		Wed 30 Aug 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	ManualPilot::ACMManualPilot(AirStrucPtr const ControlledAC2)
{
	if (ControlledAC2)
	{
		keyboardcontrol = FALSE;

// All of the next bit is done in the flight model

		//Calculating the fuel used will now be done within the flight model	CSB
		FlightModelFixedTime(ControlledAC2);
		ControlledAC2->SetManualFlightParams ();				//DAW 04Jul96
 	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Author		R. Hyde
//Date		Wed 30 Aug 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	ManualPilot::SubManualPilot(AirStrucPtr const ControlledAC)
{



//TempCode ARM 27Aug97 		ANGLES a,b;
//TempCode ARM 27Aug97 		ControlledAC->fly.pModel->HaveWeLanded (ControlledAC, a,b);

}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		InitManualPilot
//Author		Paul.
//Date			Thu 2 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void ManualPilot::InitManualPilot(AirStrucPtr const ControlledAC)

{

	InitManualPilotSub(ControlledAC);


	//Do this whenever you switch auto/man/auto

	ControlledAC->ai.ManStep = PHASE0;							//RDH 10Sep96
	ControlledAC->ai.manoeuvre = MANOEUVRE_SELECT;				//JIM 05Aug96
	ControlledAC->manoeuvretime = 0;							//JIM 05Aug96
//	ControlledAC->ai.JinkTime = 0;							//JIM 05Aug96

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		InitManualPilot
//Author		Paul.
//Date			Thu 2 Nov 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void ManualPilot::InitManualPilotSub(AirStrucPtr const ControlledAC)

{

		InitSimpleAc(ControlledAC);
///		Save_Data.keysensitivity = 2;											//RDH 12Mar96
		trim =0;													//RDH 15Nov96
		lastmsgtime = 0;

//this does a big initialise									//rdh
		if (ControlledAC2 == ControlledAC)							//ARM 01May97
		{// Player's Aircraft
			ControlledAC->fly.pModel->Initialise (ControlledAC,TRUE);			//ARM 01May97
		}else														//ARM 01May97
		{// ACM Aircraft
			ControlledAC->fly.pModel->Initialise (ControlledAC,FALSE);			//ARM 01May97
		}
}

void ManualPilot::InitSimpleAc(AirStrucPtr const ControlledAC)
{
//	ControlledAC->fly.rpm_lastcycle = ControlledAC->fly.thrustpercent;
//	ControlledAC->fly.rpmtrimtime = 0;

//the following need to be set on moving to manual
	if (ControlledAC->vel_ > 0)
		ControlledAC->fly.cpitch = ControlledAC->pitch;			//RDH 10Apr96
//	ControlledAC->fly.turnrate = ANGLES_0Deg;					//RDH 19Oct96
	ControlledAC->fly.redeffect = 0;							//RJS 13Nov96
	ControlledAC->ai.moraleinc = 0;
	ControlledAC->ai.deltapitchsum = 0;		//rdh
}
void ManualPilot::AutoToggleAircraft(AirStrucPtr const ControlledAC, CONTROLMODE mode)
{
	if (mode!=controlmode)
	{
		if (mode==MANUAL)	ControlledAC->fly.pModel->Controlled = TRUE;
		else				ControlledAC->fly.pModel->Controlled = FALSE;
		controlmode=mode;
		InitFlightModelPos(ControlledAC);
		ControlledAC->ai.ManStep = PHASE0;							//RDH 10Sep96
		ControlledAC->ai.manoeuvre = MANOEUVRE_SELECT;				//JIM 05Aug96
		ControlledAC->manoeuvretime = 0;							//JIM 05Aug96
		ControlledAC->information=IF_ALLBAD;							  //JIM 16/04/99
//		ControlledAC->ai.JinkTime = 0;							//JIM 05Aug96
	}
}

void ManualPilot::InitFlightModelPos(AirStrucPtr const ControlledAC)
{
	InitSimpleAc(ControlledAC);
//init aoa for first frame where it may be used before it is set

	ControlledAC->fly.elevator = 0;
	ControlledAC->fly.aileron  = 0;
	ControlledAC->fly.rudder   = 0;
	ControlledAC->fly.pModel->Elevator = 0;
	ControlledAC->fly.pModel->Aileron  = 0;
	ControlledAC->fly.pModel->Rudder   = 0;

	PMAINPLANE pMainPlane = ControlledAC->fly.pModel->MainPlaneList;
	while(pMainPlane)
	{
		pMainPlane->aoa[0][0] = 0;
		pMainPlane->aoa[0][1] = 0;
		pMainPlane->aoa[1][0] = 0;
		pMainPlane->aoa[1][1] = 0;
		pMainPlane = pMainPlane->List.NextItem();
	}

	ControlledAC->fly.pModel->InitialiseEngines(ControlledAC);
	ControlledAC->fly.pModel->UpdateModel (ControlledAC);
}

UByte	DUMMY;
UByte&	item::AnimByte(Animfieldref)
{
	return(DUMMY=0);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DeathSequenceOverrideDiary
//Author		Robert Slater
//Date			Tue 31 Oct 2000
//
//Description	This is a top-level death sequence override function,
//				with diary wrapper...
//				(it must not be embedded deep within damage or other death code)
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool	ManualPilot::DeathSequenceOverrideDiary(ItemPtr	itm, int	newmovecode,bool commscalled)
{
	bool	retval;

	Squad_Diary.SetAC(itm,itm->Anim);

	retval = DeathSequenceOverride(itm,newmovecode,commscalled);
	if (retval && (newmovecode != AUTO_NOPPILOT)	)					//RJS 16Nov00
	{
// we don't want to block out transient dead aircraft if we are still flying!
		AirStrucPtr	ac = AirStrucPtr(itm);
		if (	ac->fly.pModel
			&&	(ac->World.Y > (ac->fly.pModel->GroundHeight + 10000)	)
			&&	(ac->Anim != NULL)	)
		{
			MinAnimData*	mad = (MinAnimData*)ac->Anim;
			mad->itemstate = DYING;
		}
	}

	Squad_Diary.UpdateACDamage();

	return retval;
}







