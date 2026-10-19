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
//Filename       spotted.cpp
//System         
//Author         R. Hyde 
//Date           Tue 3 Mar 1998
//Description    
//
//				These routines called from visiblecheck() in msgai.com
//
//				Note the data structure does not have the concept of elements
//				We have flights.
//				The code must compensate
//
//------------------------------------------------------------------------------
//	Both SpottedUnfriendly and AlertFighters can result in a force splitting (don't confuse with 
//	split manoeuvre)
//
//	The difference is that SpottedUnfirendly will result in leader set going and others remaining
//	and AlertFighter will result in a SendFlight, ie leader staying
//
//
//

//On moving from precombat to visible range need equivalent to SpottedUnfr
//	Check decoy
//	Explore FirstTactics
//	Consider starting BVR
//
//
//Notes on airstruc pointers
//
//for caller:
//	if caller->leader == null caller is element leader
//	if caller->Follower()
//		nex wingman in element
//
//
//
//There are 3 candidates for Group Leader:
//		currac
//		currac->leader
//		currac->fly.leadflight
//
//To find all fighter leaders
//		found = AirStruc::AircraftList
//
//		while found
//		{
//			if (found->formpos == 0)
//				this is a leader
//			found = found->nextmobile;
//		}
//	if leader of group escorting something else 
//		then leadflight points to escortee lead
//	else leadflight  == NULL
//
//can use leadelt() to find leader of next element up
//
//	VisibleCheck code is used for either group leader or any ac not information
//
//
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
//#include	"winmove.h"											//ARM 06Dec96
//TEMPCODE DAW 13/04/00 #include	"textref.h"
#include	"replay.h"
#include	"sqddiary.h"

extern ArtInt	Art_Int;

DECISIONAI(Rendezvous,AirStrucPtr);

DECISIONAI(DecideTopCover, AirStrucPtr);
OPTIONAI(SetUpTopCover, AirStrucPtr);
OPTIONAI(SetUpEngageFormation, AirStrucPtr);

DECISIONAI(SplitForce, AirStrucPtr);
OPTIONAI(SetUpEngageTrg, AirStrucPtr);
OPTIONAI(SetUpMeBombersYouFighters, AirStrucPtr);
OPTIONAI(SetUpMeFightersYouBombers, AirStrucPtr);

DECISIONAI(AttackingManoeuvre,AirStrucPtr);
OPTIONAI(SetupDiveAndZoom,AirStrucPtr)
OPTIONAI(SetupHeadOnAttack,AirStrucPtr)
OPTIONAI(SetupLineAbreast,AirStrucPtr)
OPTIONAI(SetupLineAstern,AirStrucPtr)
OPTIONAI(SetupVic,AirStrucPtr)
OPTIONAI(SetupSelectTarget,AirStrucPtr);

DECISIONAI(DefensiveManoeuvre1, AirStrucPtr);
OPTIONAI(SetupSandwich,AirStrucPtr);
OPTIONAI(SetupSplit,AirStrucPtr);
OPTIONAI(SetupTurnTowards,AirStrucPtr);

DECISIONAI(DefensiveManoeuvre2, AirStrucPtr);
OPTIONAI(SetUpDefensiveSplit, AirStrucPtr);
OPTIONAI(SetUpDefensiveSplitLeftHi, AirStrucPtr);
OPTIONAI(SetUpDefensiveSplitRightHi, AirStrucPtr);
OPTIONAI(SetUpDefensiveSplitBothHi, AirStrucPtr);
OPTIONAI(DoNothing,AirStrucPtr);

DECISIONAI(DefensiveManoeuvre110, AirStrucPtr);
OPTIONAI(SetUpLufberry,AirStrucPtr);

/////////////////////////////////////////////

OPTIONAI(AllGo, AirStrucPtr);
OPTIONAI(RelaySighting, AirStrucPtr);


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SpottedUnFriendly
//Author		R. Hyde 
//Date			RDH 20Jan99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool ArtInt::SpottedUnfriendly(AirStruc* caller,AirStruc* trg)
{
	if(caller->ShouldDumpAndRun(true, false, trg))
		caller->DumpAndRun();

//Stop repeated spot messages from bombers
	{																																			//CSB 4Aug00
		AirStrucPtr grouplead = caller;																											//CSB 4Aug00
		while((grouplead->fly.leadflight))// && (grouplead->classtype->aerobaticfactor == grouplead->fly.leadflight->classtype->aerobaticfactor))	//CSB 4Aug00
			grouplead = grouplead->fly.leadflight;																								//CSB 4Aug00
		if((!grouplead->fly.numinsag) && (grouplead->fly.expandedsag))																			//CSB 4Aug00
			grouplead = grouplead->fly.expandedsag;																								//CSB 4Aug00
																																				//CSB 4Aug00
		AirStrucPtr trglead = trg;																												//CSB 4Aug00
		while((trglead->fly.leadflight) && (trglead->classtype->aerobaticfactor == trglead->fly.leadflight->classtype->aerobaticfactor))		//CSB 4Aug00
			trglead = trglead->fly.leadflight;																									//CSB 4Aug00
		if((!trglead->fly.numinsag) && (trglead->fly.expandedsag))																				//CSB 4Aug00
			trglead = trglead->fly.expandedsag;																									//CSB 4Aug00
																																				//CSB 4Aug00
		{
			grouplead->ai.spottedunfriendly[3] = grouplead->ai.spottedunfriendly[2];
			grouplead->ai.spottedunfriendly[2] = grouplead->ai.spottedunfriendly[1];
			grouplead->ai.spottedunfriendly[1] = grouplead->ai.spottedunfriendly[0];
			grouplead->ai.spottedunfriendly[0] = trglead;																									//CSB 4Aug00
		}
	}																																			//CSB 4Aug00

	ScriptTables script = ChooseSpottedScript(caller, trg);
	AirStrucPtr lead = caller->FindFormpos0();
	if((lead->fly.leadflight) && (lead->fly.leadflight->classtype->aerobaticfactor == caller->classtype->aerobaticfactor))
	{
		AirStrucPtr leadac = lead->fly.leadflight->GetLeadAc();
		if((leadac->movecode != AUTO_COMBAT) && (leadac->movecode != AUTO_PRECOMBAT) && (leadac->ai.manoeuvre != MANOEUVRE_SCREWYOUGUYSIMGOINGHOME))
			lead = leadac;
	}

	trg = trg->GetLeadAc();

	while((trg->fly.leadflight) && (!trg->fly.leadflight->GetLeadAc()->IsOutNumberedBy(1.0)))
		trg = trg->fly.leadflight->GetLeadAc();




	_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_SPOT, caller, trg, lead));

	if(caller != lead)
  	{
		if(		(!Save_Data.gamedifficulty[GD_AUTOVECTORING])
			&&	((lead == Persons2::PlayerGhostAC) || (lead == Persons2::PlayerSeenAC))		)
			return(false);

		if((ArtInt::CountSquadronSize(trg) == 1) || (!Math_Lib.rnd(3)))
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_IHAVEIT, MSG_SPOT_REPLY, lead, trg, caller)); 
		else
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_IVE_GOT_THEM, MSG_SPOT_REPLY, lead, trg, caller)); 
		caller = lead;
	}

	bool retval;
//TempCode JIM 28Mar100 	if( ((caller->duty & PACK_AM_MASK) == PACK_AM_LWSTART && caller->duty!=PACK_AM_DECOYPATROL) || (caller->classtype->aerobaticfactor == AEROBATIC_LOW) )//JIM 28Mar100
	if(		(caller->classtype->aerobaticfactor == AEROBATIC_LOW)
		||	((caller->classtype->aerobaticfactor == AEROBATIC_MEDIUM) && (trg->classtype->aerobaticfactor == AEROBATIC_HIGH))	
		||	(caller->GroupShouldGoHome(0.5)) || (caller->ai.manoeuvre == MANOEUVRE_SCREWYOUGUYSIMGOINGHOME)	)
		retval = AlertFighters(caller, trg, caller);
	else
	{
		InterceptRangeFromTo(caller, trg);
		if(		(trg->ai.unfriendly) && (trg->ai.unfriendly->Status.size == AIRSTRUCSIZE)
			&&	(AirStrucPtr(trg->ai.unfriendly)->classtype->aerobaticfactor == AEROBATIC_LOW)	)
		{
			AirStrucPtr callee = caller->FindBuddy();
			if(callee)
				if(Math_Lib.rnd(2))
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLER_CALLEE_LETSHELP, MSG_SPOT_ACTION, caller, trg, callee));
				else
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_CALLER_WILLHELP, MSG_SPOT_ACTION, caller, trg, callee));
		}

		int callernum = CountFormationSize(caller);
		if(caller->fly.leadflight)
			callernum -= CountSquadronSize(caller->fly.leadflight);

		UnitSize size;
		AirStrucPtr ic2 = SplitUnit(caller, size);
		if(		((ic2) && (ic2 != caller) && (callernum >= 6) && (callernum >= CountFormationSize(trg)))
			||	((size == UNIT_RAF_WING) || (size == UNIT_LUF_GESCHWADER) || (size == UNIT_LUF_GRUPPEN))	)
		   	retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, caller, trg, caller), DecideTopCover);
		else
		   	retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, caller, trg, caller), SetUpEngageFormation);

	}
	return(retval);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ChooseSpottedScript
//Author		R. Hyde 
//Date			RDH 20Jan99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ScriptTables ArtInt::ChooseSpottedScript(AirStrucPtr caller, AirStrucPtr trg)
{
	ScriptTables script = SCRIPT_NULL;
	const SWord RelPosAng = ANGLES_10Deg;
	caller->InterceptandRange(trg);
	SWord dhdg = AbsAngle(caller->HdgIntercept - caller->hdg);

	if(trg->classtype->aerobaticfactor == AEROBATIC_LOW)
		script = SCRIPT_BOMBERS;
	else if(	(trg->movecode == AUTO_COMBAT)
			&&	(trg->ai.unfriendly) && (trg->ai.unfriendly->Status.size == AIRSTRUCSIZE)
			&&	(AirStrucPtr(trg->ai.unfriendly)->FindFormpos0() != caller->FindFormpos0())		)
	{
		AirStrucPtr trgunf = AirStrucPtr(trg->ai.unfriendly);
		if((CountSquadronSize(trgunf) == 1) && (trgunf->World.Y < caller->World.Y))
			script = SCRIPT_INTROUBLE;
		else if(trgunf->classtype->aerobaticfactor == AEROBATIC_LOW)
			script = SCRIPT_BOMBERUNDERATTACK;
		else if(caller->nationality == NAT_RAF)
		{
			if(CountFormationSize(trgunf) < CountFormationSize(trg))
				script = SCRIPT_FIGHTERSNEEDHELP;
			else if(Math_Lib.rnd(2))
				script = SCRIPT_DOGFIGHT;
			else
				script = SCRIPT_FRIENDLIES;
		}
		else
		{
				script = SCRIPT_FRIENDLIES;
		}
	}
	else
	{
		if((caller->movecode == AUTO_PRECOMBAT) || (caller->movecode == AUTO_COMBAT))
		{
			if(caller->Distance3DSquared(&trg->World) < FP(METRES1000) * FP(METRES1000))
				script = SCRIPT_MOREBANDITSEXCITED;
			else if((trg->World.Y > caller->World.Y) && (trg->vel_y < -500000))
				script = SCRIPT_COMINGDOWN;
			else if(Math_Lib.rnd(1))
			{
				if(dhdg < ANGLES_45Deg)	script = SCRIPT_BANDITAHEAD;
				else					script = SCRIPT_BANDIT_POS;
			}
			else 
				script = SCRIPT_MOREBANDITS;
		}
		else
		{
			if(!Math_Lib.rnd(10))
			{
				WorldStuff*	worldptr = mobileitem::currworld;
				FP dotprod =  (trg->World.X - caller->World.X) * (worldptr->sunpos.X - caller->World.X)
							+ (trg->World.Y - caller->World.Y) * (worldptr->sunpos.Y - caller->World.Y)
							+ (trg->World.Z - caller->World.Z) * (worldptr->sunpos.Z - caller->World.Z);
				if(dotprod > 0)
				{
					dotprod = (dotprod * dotprod)
								/ (caller->Distance3DSquared(&worldptr->sunpos) * caller->Distance3DSquared(&trg->World));
					if(dotprod > 0.9)
						if(CountSquadronSize(trg) > 1)
							script = SCRIPT_PLURAL_AIRCRAFT_EXCLAIM_INSUN;
						else
							script = SCRIPT_AIRCRAFTEXCLAIMINSUN;
				}
				if(script == SCRIPT_NULL)
					if(CountSquadronSize(trg) > 1)
						script = SCRIPT_PLURAL_AIRCRAFT_EXCLAIM;
					else
						script = SCRIPT_AIRCRAFTEXCLAIM;
			}
			else if(	(dhdg < RelPosAng) || (dhdg > ANGLES_180Deg - RelPosAng)
					||	((dhdg > ANGLES_90Deg - RelPosAng) && (dhdg < ANGLES_90Deg + RelPosAng))	)
				script = SCRIPT_BANDIT_RELPOS;
			else
				if(dhdg < ANGLES_45Deg)
				{
					if((caller->nationality == NAT_RAF))
						script = SCRIPT_BANDITSAHEADRAF;
					else
						script = SCRIPT_BANDITSAHEADLW;
				}
				else if(dhdg > ANGLES_135Deg)
				{
					if(caller->Distance3DSquared(&trg->World) < FP(METRES1000) * FP(METRES1000))
					{
						if((caller->nationality == NAT_RAF))
							script = SCRIPT_BANDITSBEHINDEXCITEDRAF;
						else
							script = SCRIPT_BANDITSBEHINDEXCITEDLW;
					}
					else if((caller->nationality == NAT_RAF))
						script = SCRIPT_BANDITSBEHINDRAF;
					else
						script = SCRIPT_BANDITSBEHINDLW;
				}
				else
				{
					if(caller->Distance3DSquared(&trg->World) < FP(METRES1000) * FP(METRES1000))
					{
						if((caller->nationality == NAT_RAF))
							script = SCRIPT_BANDITSABEAMEXCITEDRAF;
						else
							script = SCRIPT_BANDITSABEAMEXCITEDLW;
					}
					else if((caller->nationality == NAT_RAF))
						script = SCRIPT_BANDITSABEAMRAF;
					else
						script = SCRIPT_BANDITSABEAMLW;
				}
		}
	}

	return(script);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LeaveTopCover
//Author		Craig Beeston
//Date			21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
TABLEAI(DecideTopCover, AirStrucPtr)
{
	{PHRASE_S_TOPCOVER,			&SetUpTopCover,			SCRIPT_NULL},	
	{PHRASE_S_ALLATTACKBOMBERS,	&SetUpEngageFormation,	SCRIPT_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL}
};

INSTANCEAI(DecideTopCover, AirStrucPtr)
{
	bool retval;
	int numenemyfighters = CountFormationSize(trg);
	if(trg->classtype->aerobaticfactor != AEROBATIC_HIGH)
		numenemyfighters -= CountSquadronSize(trg);

	int numourfighters = CountFormationSize(caller);
	AirStrucPtr formlead = caller;
	if(formlead->fly.leadflight)
		formlead = formlead->fly.leadflight;
	if(formlead->classtype->aerobaticfactor != AEROBATIC_HIGH)
		numourfighters -= CountSquadronSize(formlead);

	if(((numenemyfighters) && (!caller->fly.leadflight) && (numourfighters > numenemyfighters))/* || (FindEscort(caller))*/)
		retval = LeaveTopCover(caller, trg);
	else
		retval = EngageFormation(caller, trg);
	return(retval);
}

INSTANCEAI(SetUpTopCover, AirStrucPtr)
{
	return(LeaveTopCover(caller, trg));
}

INSTANCEAI(SetUpEngageFormation, AirStrucPtr)
{
	return(EngageFormation(caller, trg));
}

bool ArtInt::LeaveTopCover(AirStruc* caller, AirStruc* trg)
{
	UnitSize unit;
	ScriptTables script = SCRIPT_NULL;
	AirStrucPtr ldr_topcover = SplitUnit(caller, unit);
	AirStrucPtr ldr_attack	 = caller;

	if(ldr_topcover)
	{
		bool bombers = false;
		if(trg->classtype->aerobaticfactor < AEROBATIC_HIGH)
			bombers = true;
		bool fighters = false;
		if((!bombers) || (FindEscort(trg)))
			fighters = true;

		int attack_duty   = ldr_attack->ai.submethod & ai_info::SM_MASK_A2A;
		int topcover_duty = ldr_topcover->ai.submethod & ai_info::SM_MASK_A2A;

		if(		((bombers) && (fighters)  && (attack_duty != 0/*SM_ANYA2A*/) 	  && (topcover_duty == 0/*SM_ANYA2A*/))
			||	((bombers) && (!fighters) && (attack_duty == 1/*SM_NOTBOMBERS*/)  && (topcover_duty != 1/*SM_NOTBOMBERS*/))
			||	((!bombers) && (fighters) && (attack_duty == 2/*SM_NOTFIGHTERS*/) && (topcover_duty != 2/*SM_NOTFIGHTERS*/))	)
		{
			AirStrucPtr temp = ldr_topcover;
			ldr_topcover = ldr_attack;
			ldr_attack = temp;
		}

		switch(unit)
		{
			case UNIT_RAF_WING:			script = SCRIPT_WINGTOPCOVER;			break;
			case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONTOPCOVER;		break;
			case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTTOPCOVER;			break;
		}
		_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_TOP_COVER, caller, NULL, ldr_topcover)); 
		
		for(bool stoploop = false; (ldr_topcover) && (!stoploop); ldr_topcover = ldr_topcover->fly.nextflight)
			for(AirStrucPtr ac = ldr_topcover; (ac) && (!stoploop); ac = ac->Follower())
			{
				if(ac == ldr_attack)
					stoploop = true;
				else
					SetEngage(ac, trg, MANOEUVRE_FORCETOPCOVER, ANGLES_0Deg, ANGLES_0Deg, false);
			}
	}
	return(EngageFormation(ldr_attack, trg));
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EngageFormation
//Author		Craig Beeston 
//Date			Fri 21 Jan 2000
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool ArtInt::EngageFormation(AirStruc* caller, AirStruc* trg)
{
	bool retval;
	if(trg->classtype->aerobaticfactor == AEROBATIC_HIGH)
		retval = EngageTrg(caller, trg);
	else
	{
		UnitSize size;
		AirStrucPtr ic2 = SplitUnit(caller, size);
		if((ic2) && (ic2 != caller) && (FindEscort(trg)))
		   	retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, caller, trg, caller), SplitForce);
		else
			retval = EngageTrg(caller, trg);
	}

	if (caller->nationality == NAT_RAF)								//RJS 09Mar00
		Squad_Diary.ArrivedAtTarget(caller,trg);					//RJS 09Mar00

	return(retval);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SplitForce
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
TABLEAI(SplitForce, AirStrucPtr)
{
	{PHRASE_S_ALLATTACKBOMBERS,			&SetUpEngageTrg,			SCRIPT_NULL},	
	{PHRASE_S_ATTACKBOMBERSANDFIGHTERS,	&SetUpMeBombersYouFighters,	SCRIPT_NULL},
	{PHRASE_S_ATTACKFIGHTERSANDBOMBERS,	&SetUpMeFightersYouBombers,	SCRIPT_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL}
};

INSTANCEAI(SplitForce, AirStrucPtr)
{
	bool retval;
	{
		UnitSize unit;
		AirStrucPtr ldr_fighters = SplitUnit(caller, unit);
		AirStrucPtr ldr_bombers	 = caller;

		int fighters_duty = ldr_fighters->ai.submethod & ai_info::SM_MASK_A2A;
		int bombers_duty  = ldr_bombers->ai.submethod & ai_info::SM_MASK_A2A;

		if(		((fighters_duty == 2/*SM_NOTFIGHTERS*/) && (bombers_duty != 2/*SM_NOTFIGHTERS*/))
			||	((bombers_duty  == 1/*SM_NOTBOMBERS*/) && (fighters_duty != 1/*SM_NOTBOMBERS*/))	)
			retval = MeFightersYouBombers(caller, trg);	
		else	
			retval = MeBombersYouFighters(caller, trg);	
	}

	return(retval);
}

INSTANCEAI(SetUpMeBombersYouFighters, AirStrucPtr)
{
	return(MeBombersYouFighters(caller, trg));
}

INSTANCEAI(SetUpMeFightersYouBombers, AirStrucPtr)
{
	return(MeFightersYouBombers(caller, trg));
}


INSTANCEAI(SetUpEngageTrg, AirStrucPtr)
{
	UnitSize unit;
	ScriptTables script = SCRIPT_NULL;
	AirStrucPtr msgcallee = SplitUnit(caller, unit);
	if(msgcallee)
	{
		switch(unit)
		{
			case UNIT_RAF_WING:			script = SCRIPT_WINGALLBOMBERS;			break;
			case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONALLBOMBERS;		break;
			case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTALLBOMBERS;		break;
		}

		_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_INITIALCOMBAT, caller, trg, msgcallee));
	}
	return(EngageTrg(caller, trg));
}


bool ArtInt::MeFightersYouBombers(AirStruc* caller, AirStruc* trg)
{
	UnitSize unit;
	ScriptTables script = SCRIPT_NULL;
	AirStrucPtr ldr_bomb = SplitUnit(caller, unit);

	switch(unit)
	{
		case UNIT_RAF_WING:			script = SCRIPT_WINGFIGHTERS;		break;
		case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONFIGHTERS;	break;
		case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTFIGHTERS;		break;
	}

	_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_SPLIT_FORCE, caller, trg, ldr_bomb));

	EngageTrg(ldr_bomb, trg);

	return(EngageTrg(caller, FindEscort(trg)));
}


bool ArtInt::MeBombersYouFighters(AirStruc* caller, AirStruc* trg)
{
	UnitSize unit;
	ScriptTables script = SCRIPT_NULL;
	AirStrucPtr ldr_fight = SplitUnit(caller, unit);

	switch(unit)
	{
		case UNIT_RAF_WING:			script = SCRIPT_WINGBOMBERS;		break;
		case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONBOMBERS;	break;
		case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTBOMBERS;		break;
	}

	AirStrucPtr escort = FindEscort(trg);
	_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_SPLIT_FORCE, caller, escort, ldr_fight));

	EngageTrg(ldr_fight, escort);

	return(EngageTrg(caller, trg));
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EngageTrg
//Author		Craig Beeston 
//Date			Fri 21 Jan 2000
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool ArtInt::EngageTrg(AirStruc* caller, AirStruc* trg)
{
	bool retval;
	bool chosen = false;
	caller->ai.manoeuvre = MANOEUVRE_SELECT;

	if(trg->classtype->aerobaticfactor == AEROBATIC_HIGH)
	{
		SWord cdh = AbsAngle(SWord(caller->HdgIntercept - caller->hdg));
		SWord tdh = AbsAngle(SWord(caller->HdgIntercept + ANGLES_180Deg - trg->hdg));

		if((cdh > ANGLES_90Deg) && (tdh < ANGLES_90Deg))
		{
			if(/*	   ((caller->formation == SQUAD_FINGERFOUR) || (caller->formation == SQUAD_SHWARME))
				&&*/ (	((trg->leader)			&& (trg->Distance3D(&trg->Leader()->World)		 < 10000))
					||	((trg->fly.leadflight)	&& (trg->Distance3D(&trg->fly.leadflight->World) < 10000))
					||	((trg->follower)		&& (trg->Distance3D(&trg->Follower()->World)	 < 10000))
					||	((trg->fly.nextflight)	&& (trg->Distance3D(&trg->fly.nextflight->World) < 10000))	)	)

			   	retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, caller, trg, caller), DefensiveManoeuvre1);
			else
			   	retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, caller, trg, caller), DefensiveManoeuvre2);
			chosen = true;
		}
	}

	if(!chosen)
	{
		retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, caller, trg, caller), AttackingManoeuvre);
		AirStrucPtr callee = caller->FindBuddy();
		if(callee)
		{
			ScriptTables script;
			switch(Math_Lib.rnd(2))
			{
				case 0:		
					if(caller->nationality == NAT_RAF)
						script = SCRIPT_SAFETYCATCHES;
					else
						script = SCRIPT_SAFETYCATCHES_LW;
					break;
				case 1:
					if(CountFormationSize(trg) > 12)	script = SCRIPT_DIRECTION_DOZENS;
					else								script = SCRIPT_GETINTOTHEM;
					break;
			}
			_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_INITIAL_COMBAT, caller, trg, callee));
		}
	}

	//Set rest of group

	return(retval);
}


//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description	
//			   	
//				
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
TABLEAI(AttackingManoeuvre,AirStrucPtr)
{
	{PHRASE_ITSDIVEANDZOOM,			&SetupDiveAndZoom,	SCRIPT_ITSDIVEANDZOOM},	
	{PHRASE_ITSHEADONATT,			&SetupHeadOnAttack,	SCRIPT_ITSHEADONATT},
	{PHRASE_ITSLINEABREAST,			&SetupLineAbreast,	SCRIPT_ITSLINEABREAST},
	{PHRASE_ITSLINEASTERN,			&SetupLineAstern,	SCRIPT_ITSLINEASTERN},
	{PHRASE_ITSAVIC,				&SetupVic,			SCRIPT_ITSAVIC},
	{PHRASE_SELECTYOUROWNTARGETS,	&SetupSelectTarget,	SCRIPT_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL}
};

INSTANCEAI(AttackingManoeuvre,AirStrucPtr)
{
	tGroupLevel group = GroupLevel(caller);
	bool retval = false;
	ScriptTables script, script2;

	if(EngageAppropriate(caller, trg))
	{
		script = (ScriptTables)PHRASE_SELECTYOUROWNTARGETS;
		if (caller->fly.nextflight)
			callee = caller->fly.nextflight;
		else if( (caller->ai.elementtactics != WELDEDWING) && (caller->Follower()) )
			callee = caller->Follower();
		else
			callee = NULL;

		if((trg->classtype->aerobaticfactor != AEROBATIC_LOW) || (caller == Persons2::PlayerGhostAC))
			retval = SendMsgAndCall(MESSAGE_STRUC(script, MSG_ATTACK_METHOD, caller, trg, caller), SetupSelectTarget);	
		else
		{
			if(caller->ai.firsttactic == TACTIC_NULL)
				switch(caller->ai.submethod & ai_info::SM_MASK_FIRST)
				{
					case 8/*SM_DIVEANDZOOM*/:	caller->ai.firsttactic = TACTIC_DIVEANDZOOM;	break;
					case 16/*SM_HEADON*/:		caller->ai.firsttactic = TACTIC_HEADONATTACK;	break;
					case 24/*SM_LINEABREAST*/:	caller->ai.firsttactic = TACTIC_LINEABREAST;	break;
					case 32/*SM_LINEASTERN*/:	caller->ai.firsttactic = TACTIC_LINEASTERN;		break;
					case 40/*SM_VIC*/:			caller->ai.firsttactic = TACTIC_VICATTACK;		break;
					case 48/*SM_INDIVIDUAL*/:	caller->ai.firsttactic = TACTIC_INDIVIDUAL;		break;
				}

			if(caller->ai.firsttactic == TACTIC_DIVEANDZOOM)
				retval = SendMsgAndCall(MESSAGE_STRUC(script, MSG_ATTACK_METHOD, caller, trg, caller), SetupDiveAndZoom);	
			else if(caller->ai.firsttactic == TACTIC_LINEABREAST)
				retval = SendMsgAndCall(MESSAGE_STRUC(PHRASE_NULL,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupLineAbreast);	
			else if(caller->ai.firsttactic == TACTIC_LINEASTERN)
				retval = SendMsgAndCall(MESSAGE_STRUC(PHRASE_NULL,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupLineAstern);	
			else if(caller->ai.firsttactic == TACTIC_HEADONATTACK)
				retval = SendMsgAndCall(MESSAGE_STRUC(PHRASE_NULL,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupHeadOnAttack);	
			else if(caller->ai.firsttactic == TACTIC_VICATTACK)
				retval = SendMsgAndCall(MESSAGE_STRUC(PHRASE_NULL,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupVic);	
			else if(caller->ai.firsttactic == TACTIC_INDIVIDUAL)
				retval = SendMsgAndCall(MESSAGE_STRUC(PHRASE_NULL,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupSelectTarget);	
			else
			{
				switch(Math_Lib.rnd(5))
				{
					case 0:
					{
						if((trg->classtype->aerobaticfactor == AEROBATIC_LOW) && (CountSquadronSize(caller) > 1) && (CountSquadronSize(trg) > 1))
							retval = SendMsgAndCall(MESSAGE_STRUC(script,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupHeadOnAttack);	
						else
							retval = SendMsgAndCall(MESSAGE_STRUC(script,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupSelectTarget);	
						break;
					}
					case 1:	{	retval = SendMsgAndCall(MESSAGE_STRUC(script,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupDiveAndZoom);	break;	}
					case 2:	{	retval = SendMsgAndCall(MESSAGE_STRUC(script,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupLineAbreast);	break;	}
					case 3:	{	retval = SendMsgAndCall(MESSAGE_STRUC(script,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupLineAstern);	break;	}
					case 4:	{	retval = SendMsgAndCall(MESSAGE_STRUC(script,MSG_CHOSENMANOEUVRE,caller,trg,caller), SetupVic);			break;	}
				}
			}
		}
#pragma warnmsg ("RadioChat:code commented out")
	}
	else
	{
	}

	return(retval);
}


//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date																  //RDH 24/02/99
//
//Description			   red manoeuvre
//			   	
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupDiveAndZoom,AirStrucPtr)
{
	if(trg->classtype->aerobaticfactor == AEROBATIC_LOW)
		trg = FindTailie(trg);
	caller->SetFollowersTactic(TACTIC_DIVEANDZOOM);
	SetEngage(caller,trg, MANOEUVRE_DIVEANDZOOM,ANGLES_0Deg,ANGLES_0Deg, FALSE);
	SetUpRemainderOfGroupToEngage(caller,trg,MANOEUVRE_DIVEANDZOOM,ANGLES_0Deg,ANGLES_0Deg,FALSE);
 	return(true);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetUpHeadOnAttack
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupHeadOnAttack, AirStrucPtr)
{
	caller->SetFollowersTactic(TACTIC_HEADONATTACK);
	SetEngage(caller, trg, MANOEUVRE_HEADONATTACK, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	SetUpRemainderOfGroupToEngage(caller, trg, MANOEUVRE_HEADONATTACK, ANGLES_0Deg, ANGLES_0Deg, FALSE);
 	return(true);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetupLineAbreast
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupLineAbreast,AirStrucPtr)
{
	while(trg->fly.nextflight)
		trg = trg->fly.nextflight;

	caller->SetFollowersTactic(TACTIC_LINEABREAST);
	SetEngage(caller, trg, MANOEUVRE_LINEABREAST, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	SetUpRemainderOfGroupToEngage(caller, trg, MANOEUVRE_LINEABREAST, ANGLES_0Deg, ANGLES_0Deg, FALSE);
 	return(true);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetupLineAstern
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupLineAstern,AirStrucPtr)
{
	while(trg->fly.nextflight)
		trg = trg->fly.nextflight;

	caller->SetFollowersTactic(TACTIC_LINEASTERN);
	SetEngage(caller, trg, MANOEUVRE_LINEASTERN, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	SetUpRemainderOfGroupToEngage(caller, trg, MANOEUVRE_LINEASTERN, ANGLES_0Deg, ANGLES_0Deg, FALSE);
 	return(true);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetUpVic
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupVic, AirStrucPtr)
{
	while(trg->fly.nextflight)
		trg = trg->fly.nextflight;

	caller->SetFollowersTactic(TACTIC_VICATTACK);
	for(AirStrucPtr	nf = caller; nf; nf = nf->fly.nextflight)
		for(AirStrucPtr f = nf; f; f = f->Follower())
			if(f->ai.manoeuvre != MANOEUVRE_FORCETOPCOVER)
			{
				AirStrucPtr unf = trg;
				if(f->position())
					for(SWord  i = 0; i < f->position(); i++)
						if(unf->follower)
							unf = unf->Follower();

				SetEngage(f, unf, MANOEUVRE_VICATTACK, ANGLES_0Deg, ANGLES_0Deg, FALSE);
			}

	return(true);
}


//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description	
//			   	
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupSelectTarget,AirStrucPtr)
{
	if(Math_Lib.rnd(2))
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SELECTYOUROWNTARGETS, MSG_INITIALCOMBAT, caller, trg, caller));
	else
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLER_CALLERGROUP_SELECT_TARGET, MSG_INITIALCOMBAT, caller, trg, caller));

	AirStruc* follower = caller->Follower();
	if (	(follower)	&& (caller->ai.elementtactics == WELDEDWING))
 		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_COVERME, MSG_FOLLOWME, caller, NULL, follower));

	if (caller->IsUsingPilotedAcLog())
		if (!trg->IsUsingPilotedAcLog())
			 Persons2::UpdateLog(trg,0,EventLog::ATTACKED);

	caller->SetFollowersTactic(TACTIC_NULL);
	for(AirStrucPtr nf = caller; (nf) && (nf->ai.manoeuvre != MANOEUVRE_FORCETOPCOVER); nf = nf->fly.nextflight)
		for(AirStrucPtr ac = nf; (ac) && (ac->ai.manoeuvre != MANOEUVRE_FORCETOPCOVER); ac = ac->Follower())
		{
			AirStrucPtr tnf, tac;
			AirStrucPtr mytarget = NULL;
			for(tnf = trg; (tnf) && (!mytarget); tnf = tnf->fly.nextflight)
				for(tac = tnf; (tac) && (!mytarget); tac = tac->Follower())
					if((!tac->ai.attacker) && (!tac->Status.deadtime))
						mytarget = tac;

			if(!mytarget)
				mytarget = trg;

			SetEngage(ac, mytarget, MANOEUVRE_PEELOFF, ANGLES_0Deg, ANGLES_0Deg, FALSE);
		}
 	return(true);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
TABLEAI(DefensiveManoeuvre1, AirStrucPtr)
{
	{PHRASE_S_SANDWICH,			&SetupSandwich,		SCRIPT_CALLEE_SANDWICH},
	{PHRASE_DEFENSIVESPLIT,		&SetupSplit,		SCRIPT_NULL},
	{PHRASE_S_TURNIN,			&SetupTurnTowards,	SCRIPT_CALLEE_TURNIN},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL}
};

INSTANCEAI(DefensiveManoeuvre1, AirStrucPtr)
{
	bool retval;

	if(EngageAppropriate(caller, trg))
	{
		SWord rnd = Math_Lib.rnd(4);

		if( (rnd == 0) || ((rnd == 3) && (caller->ai.combatskill < SKILL_REGULAR)) )
			retval = SendMsgAndCall(MESSAGE_STRUC(PHRASE_DEFENSIVESPLIT, MSG_ATTACK_METHOD, caller, trg, caller), SetupSplit);
		else if( (rnd == 1) || ((rnd == 3) && (caller->ai.combatskill < SKILL_ACE)) )
			retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, caller, trg, caller), SetupTurnTowards);
		else
			retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CLEAR, caller, trg, caller), SetupSandwich);
	}
 	else
 	   	retval = SendMsgAndCall(MESSAGE_STRUC(OLDPHRASE_IGNOREFORNOW,MSG_SPOTTEDASNEWUNFRIENDLY,caller,trg,caller),DoNothing);

	return(retval);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetUpSandwich
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupSandwich, AirStrucPtr)
{
	SetEngage(caller, trg, MANOEUVRE_BREAK90, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	SetUpRemainderOfGroupToEngage(caller, trg, MANOEUVRE_TURNINGFIGHT, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	return(false);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetupTurnTowards
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupTurnTowards, AirStrucPtr)
{
	SetEngage(caller, trg, MANOEUVRE_TURNINGFIGHT, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	SetUpRemainderOfGroupToEngage(caller, trg, MANOEUVRE_TURNINGFIGHT, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	return(true);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetUpSplit
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetupSplit, AirStrucPtr)
{
	ScriptTables script = SCRIPT_NULL;
	UnitSize unit;
	AirStrucPtr msgcallee = SplitUnit(caller, unit);

	if((caller->nationality == NAT_LUF) || (!Math_Lib.rnd(5)))
		script = SCRIPT_CALLER_GROUP_SPLIT;
	else
		switch(unit)
		{
			case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLIT;			break;
			case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLIT;		break;
			case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLIT;		break;
			case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLIT;		break;
		}

	_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_INITIALCOMBAT, caller, trg, msgcallee));

	DefensiveSplit(caller, trg, ANGLES_0Deg, ANGLES_0Deg);
	return(true);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
TABLEAI(DefensiveManoeuvre2, AirStrucPtr)
{
	{PHRASE_DEFENSIVESPLIT,				&SetUpDefensiveSplit,			PHRASE_NULL},
	{PHRASE_S_DEFENSIVESPLITLEFTHI,		&SetUpDefensiveSplitLeftHi,		PHRASE_NULL},
	{PHRASE_S_DEFENSIVESPLITRIGHTHI,	&SetUpDefensiveSplitRightHi,	PHRASE_NULL},
	{PHRASE_S_DEFENSIVESPLITBOTHHI,		&SetUpDefensiveSplitBothHi,		PHRASE_NULL},
	{PHRASE_S_SELECTOWNTRG,				&SetupSelectTarget,				PHRASE_NULL},
	{PHRASE_NULL,						&DoNothing,						PHRASE_NULL},
	{PHRASE_NULL},
	{PHRASE_NULL}
};

INSTANCEAI(DefensiveManoeuvre2, AirStrucPtr)
{
	bool retval;

	if(EngageAppropriate(caller, trg))
	{
		UnitSize unit;
		AirStrucPtr ac2 = SplitUnit(caller, unit);
		switch(Math_Lib.rnd(5))
		{
			case 0:
			{	ScriptTables script = SCRIPT_CALLER_GROUP_SPLIT;
				switch(unit)
				{
					case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLIT;			break;
					case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLIT;		break;
					case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLIT;		break;
					case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLIT;		break;
				}
				retval = SendMsgAndCall(MESSAGE_STRUC(script, MSG_CHOSENMANOEUVRE, caller, trg, caller), SetUpDefensiveSplit);
				break;
			}
			case 1:
			{
				ScriptTables script = SCRIPT_CALLER_GROUP_SPLIT_LEFTHI;
				switch(unit)
				{
					case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLITLEFTHI;			break;
					case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLITLEFTHI;		break;
					case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLITLEFTHI;			break;
					case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLITLEFTHI;		break;
				}
				retval = SendMsgAndCall(MESSAGE_STRUC(script, MSG_CHOSENMANOEUVRE, caller, trg, caller), SetUpDefensiveSplitLeftHi);
				break;
			}
			case 2:
			{
				ScriptTables script = SCRIPT_CALLER_GROUP_SPLIT_RIGHTHI;
				switch(unit)
				{
					case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLITRIGHTHI;			break;
					case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLITRIGHTHI;		break;
					case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLITRIGHTHI;		break;
					case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLITRIGHTHI;		break;
				}
				retval = SendMsgAndCall(MESSAGE_STRUC(script, MSG_CHOSENMANOEUVRE, caller, trg, caller), SetUpDefensiveSplitRightHi);
				break;
			}
			case 3:
			{
				ScriptTables script = SCRIPT_CALLER_GROUP_SPLIT_BOTHHI;
				switch(unit)
				{
					case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLITBOTHHI;			break;
					case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLITBOTHHI;		break;
					case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLITBOTHHI;			break;
					case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLITBOTHHI;		break;
				}
				retval = SendMsgAndCall(MESSAGE_STRUC(script, MSG_CHOSENMANOEUVRE, caller, trg, caller), SetUpDefensiveSplitBothHi);
				break;
			}
			case 4:
				retval = SendMsgAndCall(MESSAGE_STRUC(PHRASE_SELECTYOUROWNTARGETS, MSG_CHOSENMANOEUVRE, caller, trg, caller), SetupSelectTarget);
				break;
			break;
		}
	}
 	else
 	   	retval = SendMsgAndCall(MESSAGE_STRUC(OLDPHRASE_IGNOREFORNOW,MSG_SPOTTEDASNEWUNFRIENDLY,caller,trg,caller),DoNothing);

	return(retval);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetUpDefensiveSplit
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetUpDefensiveSplit, AirStrucPtr)
{
	UnitSize unit;
	ScriptTables script = SCRIPT_NULL;
	AirStrucPtr msgcallee = SplitUnit(caller, unit);

	if((caller->nationality == NAT_LUF) || (!Math_Lib.rnd(5)))
		script = SCRIPT_CALLER_GROUP_SPLIT;
	else
		switch(unit)
		{
			case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLIT;			break;
			case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLIT;		break;
			case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLIT;		break;
			case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLIT;		break;
		}

	if(script)
		_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_INITIALCOMBAT, caller, trg, msgcallee));

	DefensiveSplit(caller, trg, ANGLES_0Deg, ANGLES_0Deg);
	return(true);
}


//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description	
//			   	
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetUpDefensiveSplitLeftHi, AirStrucPtr)
{
	UnitSize unit;
	ScriptTables script = SCRIPT_NULL;
	AirStrucPtr msgcallee = SplitUnit(caller, unit);

	if((caller->nationality == NAT_LUF) || (!Math_Lib.rnd(5)))
		script = SCRIPT_CALLER_GROUP_SPLIT_LEFTHI;
	else
		switch(unit)
		{
			case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLITLEFTHI;			break;
			case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLITLEFTHI;		break;
			case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLITLEFTHI;			break;
			case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLITLEFTHI;		break;
		}

	if(script)
		_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_INITIALCOMBAT, caller, trg, msgcallee));

	DefensiveSplit(caller, trg, BESTCLIMBPITCH,ANGLES_0Deg);
	return(true);
}


//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description	
//			   	
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetUpDefensiveSplitRightHi, AirStrucPtr)
{
	UnitSize unit;
	ScriptTables script = SCRIPT_NULL;
	AirStrucPtr msgcallee = SplitUnit(caller, unit);

	if((caller->nationality == NAT_LUF) || (!Math_Lib.rnd(5)))
		script = SCRIPT_CALLER_GROUP_SPLIT_RIGHTHI;
	else
		switch(unit)
		{
			case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLITRIGHTHI;			break;
			case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLITRIGHTHI;		break;
			case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLITRIGHTHI;		break;
			case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLITRIGHTHI;		break;
		}

	if(script)
		_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_INITIALCOMBAT, caller, trg, msgcallee));

	DefensiveSplit(caller, trg, ANGLES_0Deg,BESTCLIMBPITCH);
	return(true);
}


//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description	
//			   	
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(SetUpDefensiveSplitBothHi, AirStrucPtr)
{
	UnitSize unit;
	ScriptTables script = SCRIPT_NULL;
	AirStrucPtr msgcallee = SplitUnit(caller, unit);

	if((caller->nationality == NAT_LUF) || (!Math_Lib.rnd(5)))
		script = SCRIPT_CALLER_GROUP_SPLIT_BOTHHI;
	else
		switch(unit)
		{
			case UNIT_RAF_WING:			script = SCRIPT_WINGDEFENSIVESPLITBOTHHI;			break;
			case UNIT_RAF_SQUADRON:		script = SCRIPT_SQUADRONDEFENSIVESPLITBOTHHI;		break;
			case UNIT_RAF_FLIGHT:		script = SCRIPT_FLIGHTDEFENSIVESPLITBOTHHI;			break;
			case UNIT_RAF_SECTION:		script = SCRIPT_SECTIONDEFENSIVESPLITBOTHHI;		break;
		}

	if(script)
		_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_INITIALCOMBAT, caller, trg, msgcallee));

	DefensiveSplit(caller, trg, BESTCLIMBPITCH, BESTCLIMBPITCH);
	return(true);
}


//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description	
//			   	
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
INSTANCEAI(DoNothing,AirStrucPtr)
{
	if((caller->fly.leadflight) && (caller->fly.leadflight->classtype->aerobaticfactor == AEROBATIC_LOW))
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_STAYWITHBOMBERS, MSG_INITIALCOMBAT, caller, trg, callee));
	else
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_LETGO, MSG_INITIALCOMBAT, caller, trg, callee));

	return(false);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DefensiveSplit
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
void	ArtInt::DefensiveSplit(AirStruc* caller,AirStruc* trg, ANGLES leftpitch,ANGLES rightpitch)
{
	tGroupLevel group;
	AirStruc* caller2;
	FindGroupAndLeader2(caller, caller2, group);
	if (!caller2)
	{
		SetEngage(caller,trg, MANOEUVRE_SELECT,ANGLES_0Deg, ANGLES_0Deg,  FALSE);
	}else
	{
		caller2->information=IF_OUT_POS;
		SwitchCallerToLeft(caller, caller2);


		tGroupLevel trggroup;
		AirStruc* trg2;
		trg = trg->FindGroupLeaderClose();								  //RDH 23/06/99
		FindGroupAndLeader2(trg, trg2, trggroup);			//trg on the left
		if (!trg2)
			trg2 = trg;
		SwitchCallerToLeft(trg,trg2);


		Angles desiredhdg = (Angles)((SWord)ANGLES_195Deg + caller->hdg);
	   //set first half
		SetEngage(caller,trg,MANOEUVRE_SPLITMANOEUVRE, desiredhdg,leftpitch,true);
		SetUpRemainderOfGroupToEngage(caller,trg,MANOEUVRE_SPLITMANOEUVRE, desiredhdg,leftpitch,true);
 		if (	(caller->Follower())	&& (caller->ai.elementtactics == WELDEDWING))
 			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_STAYCLOSE, MSG_FOLLOWME, caller, NULL, caller->Follower()));
		
		//find caller's second half
		desiredhdg = (Angles)((SWord)ANGLES_170Deg + caller2->hdg);

		SetEngage(caller2,trg2,MANOEUVRE_SPLITMANOEUVRE, desiredhdg,rightpitch,true); //RDH 27/06/99
		SetUpRemainderOfGroupToEngage(caller2,trg,MANOEUVRE_SPLITMANOEUVRE, desiredhdg, rightpitch,true);
 		if (	(caller2->Follower())	&& (caller2->ai.elementtactics == WELDEDWING))
 			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_COVERME, MSG_FOLLOWME, caller2, NULL, caller2->Follower()));
	}	
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		
//Author		Craig Beeston
//Date			Tue 15 Feb 2000
//
//Description	
//				
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
TABLEAI(DefensiveManoeuvre110, AirStrucPtr)
{
	{PHRASE_DEFENSIVESPLIT,				&SetUpDefensiveSplit,			PHRASE_NULL},
	{PHRASE_S_DEFENSIVESPLITLEFTHI,		&SetUpDefensiveSplitLeftHi,		PHRASE_NULL},
	{PHRASE_S_DEFENSIVESPLITRIGHTHI,	&SetUpDefensiveSplitRightHi,	PHRASE_NULL},
	{PHRASE_S_DEFENSIVESPLITBOTHHI,		&SetUpDefensiveSplitBothHi,		PHRASE_NULL},
	{PHRASE_S_SELECTOWNTRG,				&SetupSelectTarget,				PHRASE_NULL},
	{PHRASE_DEFENSIVECIRCLE,			&SetUpLufberry,					SCRIPT_CALLER_GROUP_CIRCLE},
	{PHRASE_NULL,						&DoNothing,						PHRASE_NULL},
	{PHRASE_NULL}
};

INSTANCEAI(DefensiveManoeuvre110, AirStrucPtr)
{
	bool retval;
	if((caller->classtype->visible == ME110) && (CountSquadronSize(caller) > 4))
		retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CHOSENMANOEUVRE, caller, trg, caller), SetUpLufberry);
	else
		retval = SendMsgAndCall(MESSAGE_STRUC(SCRIPT_NULL, MSG_CHOSENMANOEUVRE, caller, trg, caller), DefensiveManoeuvre2);
	return(retval);
}

INSTANCEAI(SetUpLufberry, AirStrucPtr)
{
	SetEngage(caller, trg, MANOEUVRE_LUFBERRY, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	SetUpRemainderOfGroupToEngage(caller, trg, MANOEUVRE_LUFBERRY, ANGLES_0Deg, ANGLES_0Deg, FALSE);
	return(true);
}






//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		
//Author		R. Hyde 
//Date			21 Jan 1999
//
//Description		   callee is lowest flight free in caller's group
//						if there isn't a free flight then redirect lowest which is attacking a/c in same flight that eader is attacking		   	
//				
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AlertFighters
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
bool	ArtInt::AlertFighters(AirStruc* caller,AirStruc* trg, AirStruc* friendlythatneedshelp)
{

	AirStrucPtr helpee = friendlythatneedshelp->FindFormpos0();
	AirStrucPtr helper = FindEscort(helpee);

	FP helperrange2 = FP(MILES100) * FP(MILES100);
	if(helper)
		if((helper->ai.unfriendly == NULL) && (!helper->GroupShouldGoHome(0.5)))
			helperrange2 = helper->Distance3DSquared(&helpee->World);
		else
			helper = NULL;

	if(helper == NULL)
	{
		for(AirStrucPtr ac = AirStrucPtr(AirStruc::ACList); ac; ac = AirStrucPtr(ac->nextmobile))
			if(		(!ac->formpos)
				&&	(!ac->Status.deadtime)
				&&	(!ac->Status.deadtime)	
				&&	(ac->nationality == helpee->nationality)
				&&	(ac->classtype->aerobaticfactor == AEROBATIC_HIGH)
				&&	(CanAttackType(ac, trg))
				&&	(ac != helpee)
				&&	(ac->ai.unfriendly == NULL)
				&&	(ac->Distance3DSquared(&helpee->World) < helperrange2)
				&&	(!ac->GroupShouldGoHome(0.5))
				&&	(ac->ai.manoeuvre != MANOEUVRE_SCREWYOUGUYSIMGOINGHOME)	)
			{
				helper = ac;
				helperrange2 = ac->Distance3DSquared(&helpee->World);
			}
	}

	if((helper) && (helperrange2 < FP(VISIBLERANGE) * FP(VISIBLERANGE)))
	{
		if((helper == Persons2::PlayerGhostAC) || (helper == Persons2::PlayerSeenAC))
		{
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_BOMBERUNDERATTACK, MSG_SPOTTEDBYOTHERS, caller, trg, helper));
			return(true);
		}
		else
			return(SendMsgAndCall(MESSAGE_STRUC(SCRIPT_BOMBERUNDERATTACK, MSG_SPOTTEDBYOTHERS, helper, trg, helper), DecideTopCover));
	}

	return(false);
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SpottedNewUnfriendly
//Author		Jim Taylor
//Date			Tue 22 Oct 1996
//
//Description	
//
//
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ChooseManoeuvre
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	ArtInt::ChooseManoeuvre(AirStruc* caller,AirStruc* trg, bool newspot)
{
	bool retval = SendMsgAndCall(MESSAGE_STRUC(PHRASE_NULL,MSG_SPOTTEDASNEWUNFRIENDLY,caller,trg,caller), DecideTopCover);	
	return(retval);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ChooseBanditRelMsg
//Author		R. Hyde 
//Date			Wed 20 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
MESSAGE_STRUC	ArtInt::ChooseBanditRelMsg(MsgType mtype, AirStruc* caller,AirStruc* trg, AirStruc* callee, bool newspot)
{
	ScriptTables script;

	if (newspot)
	{
			script = SCRIPT_BANDIT_RELPOS;
	}else
	{
		if (caller->Range < DANGERRANGE)
			script = SCRIPT_MOREBANDITSEXCITED;
		else
			script = SCRIPT_MOREBANDITS;
	}
	return(MESSAGE_STRUC(script, mtype,caller,trg, callee));
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ChooseBanditOnTailMsg
//Author		R. Hyde 
//Date			Wed 20 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
MESSAGE_STRUC	ArtInt::ChooseBanditOnTailMsg(MsgType mtype, AirStruc* caller,AirStruc* trg, AirStruc* callee, bool newspot)
{
	ScriptTables script;

	if (newspot)
	{
		RndVal	rndpc = (RndVal)Math_Lib.rnd(RndValMAX);
		if (rndpc >  RND50PC)
		{
				if(caller->nationality == NAT_RAF)
				{
					if (caller->Range < DANGERRANGE)
						script = SCRIPT_BANDITSBEHINDEXCITEDRAF;
					else
						script = SCRIPT_BANDITSBEHINDRAF;
				}else
				{
					if (caller->Range < DANGERRANGE)
						script = SCRIPT_BANDITSBEHINDEXCITEDLW;
					else
						script = SCRIPT_BANDITSBEHINDLW;
				}
		}else
			script = SCRIPT_BANDIT_RELPOS;
	}else
	{
		if (caller->Range < DANGERRANGE)
			script = SCRIPT_MOREBANDITSEXCITED;
		else
			script = SCRIPT_MOREBANDITS;
	}
	return(MESSAGE_STRUC(script, mtype,caller,trg, callee));
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ChooseBanditHeadOnMsg
//Author		R. Hyde 
//Date			Wed 20 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
MESSAGE_STRUC	ArtInt::ChooseBanditHeadOnMsg(MsgType mtype, AirStruc* caller,AirStruc* trg,AirStruc*  callee, bool newspot)
{
	ScriptTables script;

	if (newspot)
	{
		RndVal	rndpc = (RndVal)Math_Lib.rnd(RndValMAX);
		if (rndpc >  RND50PC)
		{
			if(caller->nationality == NAT_RAF)
				script = SCRIPT_BANDITSAHEADRAF;
			else
				script = SCRIPT_BANDITSAHEADLW;
		}
		else
			script = SCRIPT_BANDIT_RELPOS;
	}else
	{
		if (caller->Range < DANGERRANGE)
			script = SCRIPT_MOREBANDITSEXCITED;
		else
			script = SCRIPT_MOREBANDITS;
	}
	return(MESSAGE_STRUC(script, mtype,caller,trg, callee));
}





			
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		IfEyeIsAndTrgIsNotUsingPilotedAcLog
//Author		R. Hyde 
//Date			Wed 20 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EngageAppropriate()
//Author		R. Hyde 
//Date			29/1/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool ArtInt::EngageAppropriate(AirStruc* caller,AirStruc* trg)
{
	bool retval = true;
	for(AirStrucPtr ld = caller; (ld) && (retval); ld = ld->fly.nextflight)
		for(AirStrucPtr ac = ld; (ac) && (retval); ac = ac->Follower())
			if(ac->ai.manoeuvre == MANOEUVRE_SCREWYOUGUYSIMGOINGHOME)
				retval = false;

	if((retval) && (caller->GroupShouldGoHome(0.5)))
		retval = false;

	return(retval);
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindGroupLeaderInFormation()
//Author		R. Hyde 
//Date			29/1/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
AirStruc* AirStruc::FindGroupLeaderInFormation()
{
	AirStruc* grplead;
	grplead = this;
	//find the leader to go for
	//Always go for leader of target's flight
	if (grplead->information && grplead->leader)
		grplead=*grplead->leader;
	//go for their group leader
	if (grplead->formpos && grplead->fly.leadflight && grplead->information)
		grplead=grplead->fly.leadflight->GetLeadAc();

	return(grplead);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindGroupLeaderClose()
//Author		R. Hyde 
//Date			29/1/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
AirStruc* AirStruc::FindGroupLeaderClose()
{
	AirStruc* grplead;
	grplead = this;
	//find the leader to go for
	//Always go for leader of target's flight
	if (grplead->leader && Art_Int.AircraftAreClose(grplead, (AirStruc*)grplead->leader))
		grplead=*grplead->leader;
	//go for their group leader
	if (grplead->formpos && grplead->fly.leadflight && Art_Int.AircraftAreClose(grplead, grplead->fly.leadflight))
		grplead=grplead->fly.leadflight->GetLeadAc();

	return(grplead);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindGroupLeader()
//Author		R. Hyde 
//Date			 //RDH 18/02/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
AirStruc* AirStruc::FindGroupLeader()
{
	AirStruc* grplead;
	grplead = this;
	//find the leader to go for
	if(grplead->leader)
		grplead = grplead->Leader();
	//go for their group leader
	if((grplead->formpos) && (grplead->fly.leadflight))
		grplead = grplead->fly.leadflight->GetLeadAc();

	return(grplead);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		
//Author		R. Hyde 
//Date			 //RDH 18/02/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GroupLevel
//Author		R. Hyde 
//Date			Mon 16 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ArtInt::tGroupLevel ArtInt::GroupLevel(AirStruc* caller)
{
	if (		(caller->leader)
	   		&&  (caller->leader->information)
		)
		caller = (AirStruc*)caller->leader;

	if (		(caller->fly.leadflight)
	   		&&  (caller->fly.leadflight->information)
		)
		caller = (AirStruc*)caller->fly.leadflight;

	if 	(		(caller->fly.nextflight)
	   		&&  (caller->fly.nextflight->information)
		)
	{
		if 	(		(caller->fly.nextflight->fly.nextflight)
		   		&&  (caller->fly.nextflight->fly.nextflight->information)
			)
			return(GROUP_SQUADRON);
		else
			return(GROUP_FLIGHT);


	}else
	{
		if  (	(caller->Follower())
	   		&&  (caller->Follower()->information)
			)
			return(GROUP_ELEMENT);
		else
			return(GROUP_LONER);
	}

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		GroupLevelNotInFormation
//Author		R. Hyde 
//Date			Mon 16 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ArtInt::tGroupLevel ArtInt::GroupLevelNotInFormation(AirStruc* caller)
{
	if (caller->leader)
		caller = (AirStruc*)caller->leader;

	if (caller->fly.leadflight)
		caller = (AirStruc*)caller->fly.leadflight;

	if 	(		(caller->fly.nextflight)
		)
	{
		if 	(		(caller->fly.nextflight->fly.nextflight)
			)
			return(GROUP_SQUADRON);
		else
			return(GROUP_FLIGHT);

	}else
	{
		if  (	(caller->Follower())
			)
			return(GROUP_ELEMENT);
		else
			return(GROUP_LONER);
	}

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
void ArtInt::InterceptRangeFromTo(AirStruc* from, AirStruc* to)

{


	SLong deltax = to->World.X - from->World.X;
	SLong deltay = to->World.Y - from->World.Y;
	SLong deltaz = to->World.Z - from->World.Z;
	Math_Lib.Intercept (deltax,deltay,deltaz,from->Range,from->HdgIntercept,from->PitchIntercept);
}
SLong ArtInt::RangeFromTo(AirStruc* from, AirStruc* to)
{
	ANGLES	oldhdgint, oldpitchint;
	SLong	oldrange;
	SLong range;
	
	oldhdgint = from->HdgIntercept;
	oldpitchint = from->PitchIntercept;
	oldrange  = from->Range;

	InterceptRangeFromTo(from, to);
	range = from->Range;

	from->PitchIntercept = oldpitchint;
	from->HdgIntercept = oldhdgint;
	from->Range = oldrange;


	return (range);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AcHasBombs
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
//Procedure		ForceHasBombers(trg)
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
//Procedure		BingoFuel
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
//Procedure		Winchester
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
//Procedure		SetUpRemainderOfGroupToEngage
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
void	ArtInt::SetUpRemainderOfGroupToEngage(AirStruc* caller,AirStruc* trg,MANOEUVRE manoeuvre,ANGLES desiredhdg,ANGLES  desiredpitch, bool surprised)
{
	for(AirStrucPtr ldr = caller; ldr; ldr = ldr->fly.nextflight)
		for(AirStrucPtr ac = ldr; ac; ac = ac->Follower())
			if((ac != caller) && (!ac->ai.unfriendly) && (ac->ai.manoeuvre != MANOEUVRE_FORCETOPCOVER))
			{
				AirStrucPtr newtrg = NULL;
				AirStrucPtr starttrg = trg->FindFormpos0();
				if(starttrg->classtype->aerobaticfactor == AEROBATIC_LOW)
					while(starttrg->fly.nextflight)
						starttrg = starttrg->fly.nextflight;

				for(AirStrucPtr trgldr = starttrg; (trgldr) && (!newtrg); trgldr = trgldr->fly.nextflight)
					for(AirStrucPtr trgac = trgldr; (trgac) && (!newtrg); trgac = trgac->Follower())
						if((!trgac->ai.attacker) && (!trgac->Status.deadtime))
							newtrg = trgac;

				if(!newtrg) 
					newtrg = starttrg;
			 	SetEngage(ac, newtrg, manoeuvre, desiredhdg, desiredpitch, surprised);
			}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PrepareForCombat
//Author		 //RDH 04/03/99
//Date			
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	ArtInt::PrepareForCombat(AirStrucPtr caller)
{
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetEngage
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
void	ArtInt::SetEngage(AirStrucPtr caller,AirStrucPtr trg,MANOEUVRE manoeuvre,ANGLES   desiredhdg,ANGLES   desiredpitch, bool surprised)
{
#ifndef	NDEBUG
 	if(caller->classtype->aerobaticfactor == AEROBATIC_LOW)
 		INT3;				 	//TRYING TO PUT A HEAVY BOMBER INTO COMBAT
	if(trg == caller)
		INT3;					//TRYING TO GO INTO COMBAT WITH SELF
	if((caller) && (trg) && (trg->nationality == caller->nationality))
		INT3;					//TRYING TO ATTACK A FRIENDLY
	if((caller->fly.numinsag) && (caller->fly.expandedsag))				//CSB 31Jul00
		INT3;															//CSB 31Jul00
	if((!caller->fly.numinsag) && (trg->fly.numinsag) && (trg->fly.expandedsag))					//CSB 31Jul00
		INT3;															//CSB 31Jul00
	if((caller->Status.deadtime && !caller->uniqueID.commsmove) || (trg->Status.deadtime && !trg->uniqueID.commsmove))
		INT3;
	if(trg->IsOutNumberedBy(2.0))
		int DoNothingForABreakPoint = 0;
	caller->DoAllSAGChecks();													//CSB 1Aug00
	if(caller->ai.manoeuvre == MANOEUVRE_DISENGAGED)
		int DoNothingForABreakpoint = 0;
#endif

	if((caller->fly.numinsag) && (caller->fly.expandedsag))				//CSB 31Jul00
		caller = caller->fly.expandedsag;								//CSB 31Jul00
	
	if((trg->fly.numinsag) && (trg->fly.expandedsag))					//CSB 31Jul00
		trg = trg->fly.expandedsag;										//CSB 31Jul00

	if((caller->fly.numinsag) && (!caller->fly.expandedsag) && (!trg->fly.numinsag))	//CSB 1Aug00
		trg = trg->fly.expandedsag;														//CSB 1Aug00

	if (caller->IsUsingPilotedAcLog() || (trg && trg->IsUsingPilotedAcLog()))//JIM 03Jun99
		OverLay.CancelAccel();									//JIM 03Jun99

	caller->ai.desiredrange = 0;										//CSB 26Jun00

	if (caller==Persons2::PlayerSeenAC)							//AMM 28Aug98
		caller=Persons2::PlayerGhostAC;							//AMM 28Aug98

//I don't know why we did this, it messes up links 
	if(caller->movecode == AUTO_BOMB)									//CSB 16Aug00
	{
		caller->movecode = AUTO_FOLLOWWP;								//CSB 16Aug00
		caller->information = IF_OUT_POS;					//CSB 22Aug00
		caller->ai.unfriendly = NULL;
	}


	if (trg)
	{
		if (Manual_Pilot.ControlledAC2 == trg)
		{
			trg = Persons2::PlayerGhostAC;
		}

		if(caller->ai.unfriendly == trg)								//CSB 07/03/00
		{																//CSB 07/03/00
			caller->information = IF_OUT_POS;							//CSB 07/03/00
			caller->movecode = AUTO_COMBAT;							//CSB 07/03/00
			caller->ai.manoeuvre = manoeuvre;							//CSB 07/03/00
			caller->ai.ManStep = PHASE0;								//CSB 07/03/00
			caller->manoeuvretime = 0;									//CSB 07/03/00
		}
		else
		{//new engage

			PrepareForCombat(caller);
			caller->ai.ManStep = PHASERESET;
			if (manoeuvre != MANOEUVRE_SELECT)
			{
				caller->ai.manoeuvre = manoeuvre;
				caller->ai.desiredhdg = desiredhdg;
				caller->ai.desiredpitch = desiredpitch;
			}else
			{
				caller->ai.desiredhdg = ANGLES_0Deg;
				caller->ai.desiredpitch = ANGLES_0Deg;
			
					caller->ai.manoeuvre=MANOEUVRE_SELECT;
			}
			caller->SetUnfriendly(trg);									//CSB 1Aug00

			caller->manoeuvretime=0;	//CSB 24/06/99	

			caller->information = IF_OUT_POS;

			if((caller->Distance3DSquared(&trg->World) > FP(COMBATRANGE) * FP(COMBATRANGE))	&& (manoeuvre == MANOEUVRE_SELECT))
			{
				caller->movecode = AUTO_PRECOMBAT;
				caller->SetManoeuvreTime(0);
				caller->information = IF_OUT_POS;
			}
			else
			{
				caller->movecode = AUTO_COMBAT;
			}
//##rdh test purposes
			if (caller==Persons2::PlayerGhostAC)
			{
				caller= Manual_Pilot.ControlledAC2;
				caller->SetUnfriendly(trg);
				caller->information=IF_OUT_POS;
				caller->movecode=AUTO_COMBAT;
				caller->ai.manoeuvre=MANOEUVRE_SELECT;

			}

			_DPlay.OwnerChangeDecision(caller,trg);				//AMM 27Nov98

			//##	if (surprised)
			//##modify morale based on range
		}
	}else
	{//assume that if we get here with no target then weldedwing was required
		//Just follow leader...
		if	(		(caller->leader)
				&&	(((AirStrucPtr)caller->leader)->ai.unfriendly)
				&& (((AirStrucPtr)caller->leader)->ai.unfriendly->Status.size == AIRSTRUCSIZE)
			)
			//20Apr99
			caller->SetUnfriendly(AirStrucPtr(caller->Leader()->ai.unfriendly));		//CSB 1Aug00
		else
			caller->SetUnfriendly(NULL);		//CSB 1Aug00

		caller->ai.ManStep = PHASERESET;
		caller->information = IF_OUT_POS;							  //CSB 05/01/00
		caller->ai.desiredhdg = ANGLES_0Deg;
		caller->ai.desiredpitch = ANGLES_0Deg;
		caller->ai.manoeuvre = MANOEUVRE_WELDEDWINGMAN; 
		if(caller->Range > COMBATRANGE)
		{
			caller->movecode = AUTO_PRECOMBAT;
			caller->SetManoeuvreTime(0);
			caller->information = IF_OUT_POS;
		}
		else
		{
			caller->manoeuvretime = 0;
			caller->movecode = AUTO_COMBAT;
		}
	}

	caller->DoAllSAGChecks();													//CSB 1Aug00
}		

	
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SpottedNeutral
//Author		R. Hyde 
//Date			Fri 6 Mar 1998
//
//Description	Not needed for MiG Alley
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		MakeForceToRespond
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------


bool	ArtInt::TargetFacingCaller(AirStruc*  caller, AirStruc*  trg,ANGLES angle)
{
	ANGLES delta = trg->HdgIntercept + ANGLES_180Deg + angle - trg->hdg;		  //RDH 12/06/99
	ANGLES anglerange = angle + angle;								  //RDH 12/06/99
	if (delta << anglerange)										  //RDH 12/06/99
	 	return(true);
	else
		return(false);

}
bool	ArtInt::TargetOnCallerTail(AirStruc*  caller, AirStruc*  trg, ANGLES angle)
{
	ANGLES delta = caller->HdgIntercept + ANGLES_180Deg + angle - caller->hdg; //RDH 12/06/99
	ANGLES anglerange = angle + angle;								  //RDH 12/06/99
	if (delta << anglerange)										  //RDH 12/06/99
	 	return(true);
	else
		return(false);

}
//------------------------------------------------------------------------------
AirStrucPtr	ArtInt::SetFlightWhoCanEngage(AirStrucPtr caller,AirStrucPtr trg)
{
	AirStrucPtr	tmpt=trg;
	if (tmpt==NULL)	return(tmpt);
	switch (caller->ai.flighttactics)
	{
		case TOPCOVER:
		case SWORDANDSHIELD:
		case WELDEDWING:
		{	
			tmpt=NULL;
			break;
		}
		case LOOSEDEUCE:
		{		 
			tmpt=nextfl(trg,true);								//JIM 19Sep97
			if (!tmpt)
				//can't find a next flight so go down the followers
				tmpt=nextfol(trg,true);
			if (!tmpt)
				{
 					if (trg->fly.leadflight)
					{	
						if (trg->information)
 							tmpt=trg->fly.leadflight;
						else
						{
							SLong	oldrange = trg->Range;
						   trg->InterceptandRange(&trg->fly.leadflight->World);
						   if (trg->Range < RECOGNISERANGE)
								tmpt = trg->fly.leadflight;			  //RDH 23/06/99
					   		trg->Range = oldrange; 
						}
					}
				}

			break;
		}

	}
	return (tmpt);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetFollowerWhoCanEngage
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
AirStrucPtr	ArtInt::SetFollowerWhoCanEngage(AirStrucPtr caller,AirStrucPtr trg)
{
	AirStrucPtr	tmpt=trg;
	if (tmpt==NULL)	return(tmpt);
	switch (caller->ai.elementtactics)
	{
		case TOPCOVER:
		case SWORDANDSHIELD:
		case WELDEDWING:
		{	
			tmpt=NULL;
			break;
		}
		case LOOSEDEUCE:
		{
			tmpt=nextfol(trg,true);								//JIM 19Sep97
			if (!tmpt)
			{
				//can't find a follower so try next flight
				if (trg->leader)
					trg = trg->Leader();
				tmpt=nextfl(trg,true);								  //RDH 12/05/99
				if (!tmpt)
				{
 					if (trg->fly.leadflight)
					{	
						if (trg->information)
 							tmpt=trg->fly.leadflight;
						else
						{
							SLong	oldrange = trg->Range;
						   trg->InterceptandRange(&trg->fly.leadflight->World);
						   if (trg->Range < RECOGNISERANGE)
								tmpt = trg->fly.leadflight;
					   		trg->Range = oldrange; 
						}
					}
				}

			}

			break;
		}
		default:
			tmpt=NULL;
			break;


	}
	return (tmpt);
}


//------------------------------------------------------------------------------


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetFollowerEngage
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
AirStrucPtr	ArtInt::SetFollowerEngage(AirStrucPtr caller,AirStrucPtr trg, MANOEUVRE  manoeuvre,ANGLES desiredhdg, ANGLES desiredpitch, bool surprised)
{
	caller->information=IF_OUT_POS;
	AirStrucPtr	tmpt=trg;
	if (!caller->AircraftDamaged() && caller->classtype->aerobaticfactor!=AEROBATIC_LOW)								//JIM 29Aug96
	if (caller->movecode < AUTO_COMBAT)
	{
		if (		(manoeuvre == MANOEUVRE_LINEASTERN)
				||	(manoeuvre == MANOEUVRE_LINEABREAST)
		   )
		{//all followers go after the same as the lead in these manoeuvres
			tmpt = trg;
		}else
		{
			tmpt = SetFollowerWhoCanEngage(caller,trg);			
			//null means not found or not allowed
			if (!tmpt)
			{
				if (caller->ai.elementtactics == LOOSEDEUCE)
					 tmpt = trg;
			}
		}
		SetEngage(caller,tmpt, manoeuvre,desiredhdg, desiredpitch, surprised);
		if (tmpt)
			trg=tmpt;
	}
	return(trg);
}
bool ArtInt::CallerOnRight(AirStruc* ac,AirStruc* trg)
{
	ANGLES	relhdg;
	relhdg =  HdgIntFromTo(ac,trg) - ac->hdg;
	if 	(relhdg << ANGLES_180Deg)
		return (false);
	else
		return(true);
	
}
void	ArtInt::FindGroupAndLeader2(AirStruc*& caller, AirStruc*& caller2,tGroupLevel& group)
{
	group = GroupLevelNotInFormation(caller);						  //RDH 23/06/99
	caller2 = NULL;													  //RDH 02/06/99
	int count = 0;
	if  (group == GROUP_ELEMENT)
	{
		caller2 = caller;
		do
		{
			count++;
			caller2 = ((AirStruc *)caller2->Follower());
		}while (caller2);
		count = count/2;
		int  i = 0;
		caller2 = caller;
		while (i < count)
		{
			i++;
			caller2 = ((AirStruc *)caller2->Follower());
		}
		
	}
	else if (group == GROUP_FLIGHT)
		caller2 =  caller->fly.nextflight;
	else if (group == GROUP_SQUADRON)
	{
		//GROUP_SQUADRON means bigger than a flight
		//should never be more tha 3 flights, so split at end of first flight
		if (caller->fly.nextflight)
			caller2 =  caller->fly.nextflight->fly.nextflight;


	}else
		caller2 = NULL;


}
void	ArtInt::SwitchCallerToLeft(AirStruc*& caller, AirStruc*& callee)
{
	if (callee)
	{
		//we want caller on the left for standard move
		//can invert later if we want them to cross
		if (CallerOnRight(caller,callee))
		{
			AirStruc* tmp = callee;
			callee = caller;
			caller = tmp;
		}
	}

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		nextfl
//Author		Jim Taylor
//Date			Wed 26 Jun 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
AirStrucPtr	ArtInt::nextfl(AirStrucPtr trg,bool unassigned)
{

	SLong	oldrange = trg->Range;
 	AirStrucPtr	t2=NULL;
	
	if (trg->fly.nextflight)
	{
		if (trg->fly.nextflight->information)
			t2 = trg->fly.nextflight;
		else
		{
		   trg->InterceptandRange(&trg->fly.nextflight->World);
			   if (trg->Range < RECOGNISERANGE)
					t2 = trg->fly.nextflight;				
		}
	}


	trg->Range = oldrange; 
	return(t2);
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		nextfol
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
bool	ArtInt::AircraftAreClose(AirStrucPtr trg, AirStrucPtr foll)
{

	

	SLong deltax = trg->World.X - foll->World.X;
	if (deltax < 0)
		deltax = -deltax;
	SLong deltaz = trg->World.Z - foll->World.Z;
	if (deltaz < 0)
		deltaz = -deltaz;

  if (		(deltax < RECOGNISERANGE)
		&&	(deltaz < RECOGNISERANGE)
	 )
	return(true);
  else
	return(false);


}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		nextfol
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
AirStrucPtr	ArtInt::nextfol(AirStrucPtr trg,bool unassigned)
{

	SLong	oldrange = trg->Range;
 	AirStrucPtr	t2=NULL;

	if (trg->Follower())
	{
		if	(trg->Follower()->information)	
			t2 = trg->Follower();
		else
		{
		   trg->InterceptandRange(&trg->Follower()->World);
		   if (trg->Range < RECOGNISERANGE)
				t2 = trg->Follower();				
		}
	}

 	trg->Range = oldrange; 
	return(t2);

}
ANGLES ArtInt::HdgIntFromTo(AirStruc* ac,AirStruc* trg)
{
	ANGLES	oldhdgint, oldpitchint;
	SLong	oldrange;
	ANGLES	hdgint;
	oldhdgint = ac->HdgIntercept;
	oldpitchint = ac->PitchIntercept;
	oldrange  = ac->Range;

	InterceptRangeFromTo(ac, trg);
	hdgint = ac->HdgIntercept;

	ac->PitchIntercept = oldpitchint;
	ac->HdgIntercept = oldhdgint;
	ac->Range = oldrange;
	return(hdgint);

}




//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindEscort
//Author		Craig Beeston
//Date			Fri 21 Jan 2000
//
//Description	Finds a SAG which is escorting
// 	
//Inputs		
//
//Returns	
//------------------------------------------------------------------------------
AirStruc* ArtInt::FindEscort(AirStruc* caller)
{
	AirStrucPtr leader = caller->GetLeadAc();
	AirStrucPtr escort = NULL;

	for(int i = 0; (i < ACARRAYSIZE) && (!escort); i++)
		if(ACArray[i])
		{
			AirStrucPtr as = ACArray[i]->GetLeadAc();

			if(		(as->classtype->aerobaticfactor == AEROBATIC_HIGH) && (as->fly.leadflight)
				&&	(as->fly.leadflight->GetLeadAc() == leader)
				&&	(as->fly.leadflight->movecode != AUTO_COMBAT) && (as->fly.leadflight->movecode != AUTO_PRECOMBAT)
				&&	(as->Distance3DSquared(&leader->World) < FP(HALFVISIBLERANGE) * FP(HALFVISIBLERANGE))	)
				escort = as;
		}

	return(escort);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		BreakCall
//Author		R. Hyde 
//Date			Thu 4 Mar 1999
//
//Description	Worst call first, try to assess skill and morale and then choose
// 				returns the desired script
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ScriptTables ArtInt::BreakCall(AirStrucPtr caller, AirStrucPtr callee, AirStrucPtr agg)		  //CSB 08/02/00
{
	ScriptTables script = SCRIPT_LOOKOUT;
	
	if(Math_Lib.rnd(100) < 10)
		switch(Math_Lib.rnd(3))
		{
			case 0:		script = SCRIPT_AIRCRAFTEXCLAIM;	break;
			case 1:		script = SCRIPT_BANDITINCIRCUIT;	break;
			case 2:		script = SCRIPT_LOOKANOTHER;		break;
		}
	else if(caller == FindTailie(caller))// is weaver)
		script = SCRIPT_WEAVERCALLBANDIT;
	else if(caller->ai.morale > 205 + Math_Lib.rnd(50))
		script = SCRIPT_BREAKHELL;
	else if(caller->ai.morale < Math_Lib.rnd(50))
		script = SCRIPT_LOOKOUTLEADER;
	else
	{
		SWord rand = Math_Lib.rnd(30);
		if(rand >= 26)
			rand = (21 * caller->ai.combatskill) / 255 + rand - 25;

		bool breakleft = true;
		if(FP(agg->World.X - callee->World.X) * FP(callee->vel_z) - FP(agg->World.Z - callee->World.Z) * FP(callee->vel_x) > 0)
			breakleft = false;

		switch(rand)
		{
			case 0:		script = SCRIPT_LOOKOUT;	break;
			case 1:		script = SCRIPT_BEHIND;		break;
			case 2:		script = SCRIPT_BEHIND_BEHIND;	break;
			case 3:		script = SCRIPT_LOOKBEHIND;	break;
			case 4:		script = SCRIPT_LOOKBEHINDYOU;	break;
			case 5:		script = SCRIPT_BANDITSONTAIL;	break;
			case 6:		script = SCRIPT_BANDITSONSIX;	break;
			case 7:		script = SCRIPT_LOOKBLIGHTER;	break;
			case 8:		if(breakleft)	script = SCRIPT_BREAKLEFT;
						else			script = SCRIPT_BREAKRIGHT;
						break;
			case 9:		if(breakleft)	script = SCRIPT_BANDITBREAKLEFT;
						else			script = SCRIPT_BANDITBREAKRIGHT;
						break;
			case 10:	script = SCRIPT_CALLEE_BEHIND_BEHIND;	break;
			case 11:	script = SCRIPT_BEHIND_CALLEE_BEHIND;	break;
			case 12:	script = SCRIPT_BEHIND_CALLEE_ONTAIL;	break;
			case 13:	script = SCRIPT_CALLEE_LOOKBEHIND;	break;
			case 14:	script = SCRIPT_OHNO_CALLEE_LOOKBEHIND;	break;
			case 15:	script = SCRIPT_CALLEE_LOOKBEHINDYOU;	break;
			case 16:	script = SCRIPT_CALLEE_BANDITSONTAIL;	break;
			case 17:	script = SCRIPT_CALLEE_BANDITSONSIX;	break;
			case 18:	script = SCRIPT_CALLEE_LOOKBLIGHTER;	break;
			case 19:	if(breakleft)	script = SCRIPT_CALLEE_BEHIND_ONLEFT;
						else			script = SCRIPT_CALLEE_BEHIND_ONRIGHT;
						break;
			case 20:	if(breakleft)	script = SCRIPT_BEHIND_CALLEE_ONLEFT;
						else			script = SCRIPT_BEHIND_CALLEE_ONRIGHT;
						break;
			case 21:	if(breakleft)	script = SCRIPT_CALLEE_LOOKBEHIND_ONLEFT;
						else			script = SCRIPT_CALLEE_LOOKBEHIND_ONRIGHT;
						break;
			case 22:	if(breakleft)	script = SCRIPT_CALLEE_LOOKBEHINDYOU_ONLEFT;
						else			script = SCRIPT_CALLEE_LOOKBEHINDYOU_ONRIGHT;
						break;
			case 23:	if(breakleft)	script = SCRIPT_CALLEE_BANDITSONTAILBREAKLEFT;
						else			script = SCRIPT_CALLEE_BANDITSONTAILBREAKRIGHT;
						break;
			case 24:	if(breakleft)	script = SCRIPT_CALLEE_BANDITSONSIXBREAKLEFT;
						else			script = SCRIPT_CALLEE_BANDITSONSIXBREAKRIGHT;
						break;
			case 25:	if(breakleft)	script = SCRIPT_CALLEE_BANDITBREAKLEFT;
						else			script = SCRIPT_CALLEE_BANDITBREAKRIGHT;
						break;
		}
	}

	return script;
}


//////////////////////////////////////////////////////////////////////
//
// Function:    Rendezvous
// Date:		04/04/99
// Author:		JIM
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
TABLEAI(Rendezvous,AirStrucPtr)
{	{PHRASE_NULL}
};

INSTANCEAI(Rendezvous,AirStrucPtr)
{
	return false;
}

/*
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SpottedUnknown
//Author		R. Hyde 
//Date			Fri 27 Feb 1998
//
//Description	Do we want a closer look?
//			   	Only if threatlevel >=BANDITANYWHERE
//				and aggressionlevel >=AL_DEFENSIVE
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	ArtInt::SpottedUnknown(AirStruc* caller,AirStruc* trg)
{
//rdh 19/1/99	if (caller->ai.unfriendly==NULL)
//rdh 19/1/99	{
//rdh 19/1/99
//rdh 19/1/99//##temp rdh			if  (	(caller->ai.threatlevel >=BANDITANYWHERE)
//rdh 19/1/99//				&&	caller->movecode < AUTO_TRACKINGBOGIE
//rdh 19/1/99//				&&	caller->ai.aggressionlevel >= AL_DEFENSIVE
//rdh 19/1/99//				)
//rdh 19/1/99
//rdh 19/1/99}

	if (caller->IsUsingPilotedAcLog())
	{
		if (!trg->IsUsingPilotedAcLog())
		{
//TempCode RDH 27Mar98 			Persons_2.AddMESSAGE_STRUC(UserMsg::SIGHTED,TEXT_UNKNOWNSIGHTED,caller);
			caller->PlayerSequenceAudible(FIL_MUSIC_SPOTTED_UNKNOWN);
			Persons2::UpdateLog(trg->ai.eventlog,0,EventLog::SEENBYPLAYERS);
		}
	}
	elseif (!trg->IsUsingPilotedAcLog())
	{
		Persons2::UpdateLog(trg->ai.eventlog,0,EventLog::SEENBYOTHERS);
	}
	if (caller->ai.unfriendly==NULL)
	{

//##temp rdh			if  (	(caller->ai.threatlevel >=BANDITANYWHERE)
//				&&	caller->movecode < AUTO_TRACKINGBOGIE
//				&&	caller->ai.aggressionlevel >= AL_DEFENSIVE
//				)

	}
	return(true);


}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SpottedContrails
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
void	ArtInt::SpottedContrails(AirStruc* caller,AirStruc* trg)
{
//like spotted unknown
//given higher priority?
//do we know who has set contrails, if we could track it easily if required
}	
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SpottedOldContrails
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
void	ArtInt::SpottedOldContrails(AirStruc* caller,AirStruc* trg)
{
//like spotted unknown
//given lower priority?
//do we know who has set contrails, if we could track it easily if required
}	
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DoesFriendNeedHelp
//Author		R. Hyde 
//Date			Fri 27 Feb 1998
//
//Description	Assess whether friendly needs helping out
//				caller and trg are same nationality
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	ArtInt::DoesFriendNeedHelp(AirStruc* caller,AirStruc* trg)
{
	SLong	RangeEyeTrg = caller->Range;
	
	if (	trg->ai.attacker
		&&	trg->ai.attacker->Status.size==AIRSTRUCSIZE
		&&	!((AirStruc*)*trg->ai.attacker)->ai.attacker
		)
	{//trg has attacker that is an aircraft that is not under attack
		caller->InterceptandRange(&trg->ai.attacker->World);

		if (	(	(!caller->ai.unfriendly)
				||	(	((AirStruc*)caller->ai.unfriendly)->nationality == caller->nationality)
				||	((RangeEyeTrg - caller->Range) > MILES01)
				)
				&& (caller->ai.morale > MORALE_MEDIUM)
			)
		{//caller doesn't have unf or unf is really on caller's side or this attacker closer
		 //and morale high 
			return(SpottedUnfriendly(caller,*trg->ai.attacker));
		}else
			return(false);
//TempCode RDH 27Mar98 		if (trg->ai.eventlog->stage!=EventLog::TAKEOFF) //player flight
//TempCode RDH 27Mar98 			//##message to differentiate between bomber,fb and fighter
//TempCode RDH 27Mar98 			Persons_2.AddMessageIfPlayerSq(caller,UserMsg::SIGHTED,TEXT_DOGFIGHTSIGHTED,trg);
		
	}else
	{
		return(false);
		if (trg->Range>RECOGNISERANGE-METRES500)
			if (caller->ai.eventlog->stage==EventLog::TAKEOFF)	//player flight
			if (trg->ai.eventlog->stage!=EventLog::TAKEOFF)	//non-player flight
			{
 				Persons_2.UpdateLog(trg->ai.eventlog,0,EventLog::SEENBYPLAYERS);
				caller->PlayerSequenceAudible(FIL_MUSIC_FRIENDLY_SEEN);
			}
	}

	caller->Range = RangeEyeTrg;
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SpottedNewUnfriendly
//Author		Jim Taylor
//Date			Tue 22 Oct 1996
//
//Description	
//				if caller is leader
//					if trg has no attacker
//						if range < DANGER THEN attack
//						if range < ENGAGE
//							if caller has no unfriendly or is a friendly THEN attack
//							if range to caller->unf > ENGAGERANGE THEN attack
//
//
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	ArtInt::SpottedNewUnfriendly(AirStruc* caller,AirStruc* trg)
{
	if (caller->leader==NULL)
		if (trg->ai.attacker==NULL)
		{
			int	newrange=trg->Range;
			if (newrange<ENGAGERANGE)
				return(SpottedUnfriendly(caller,trg));
			if (newrange<COMBATRANGE)
			{
				if (	(caller->ai.unfriendly==NULL)
//##					||	(caller->ai.unfriendly->nationality == caller->nationality)
					)
					return(SpottedUnfriendly(caller,trg));
				caller->InterceptandRange(caller->ai.unfriendly);
				if (caller->Range>COMBATRANGE)
					return(SpottedUnfriendly(caller,trg));
			}
		}
	return(false);
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
void ArtInt::InterceptRangeFromTo(AirStruc* from, AirStruc* to)

{


	SLong deltax = to->World.X - from->World.X;
	SLong deltay = to->World.Y - from->World.Y;
	SLong deltaz = to->World.Z - from->World.Z;
	Math_Lib.Intercept (deltax,deltay,deltaz,from->Range,from->HdgIntercept,from->PitchIntercept);
	
	
	

}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AcHasBombs
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
bool ArtInt::AcHasBombs(AirStruc* trg)
{
	return(true);
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ForceHasBombers(trg)
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
bool ArtInt::ForceHasBombers(AirStruc* trg)
{
	return(true);
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		BingoFuel
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
bool ArtInt::BingoFuel(AirStruc* trg)
{
	return(true);
}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Winchester
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
bool ArtInt::Winchester(AirStruc* trg)
{
	return(true);
}
*/
