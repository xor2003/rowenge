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
//Filename       scoring.cpp
//System         
//Author         R. Hyde 
//Date           Mon 9 Sep 1996
//Description    DEAD I THINK in BoB
//------------------------------------------------------------------------------
#include	"dosdefs.h"
#include	"world.h"
#include	"worldinc.h"
#include	"flymodel.h"
#include 	"ui.h"
#include	"transite.h"
#include	"mymath.h"
#include	"mytime.h"
#include	"maps.h"
#include	"ai.h"
#include "persons2.h"											//RDH 24Jul96

MATHABLE	(SkillType);
MATHABLE	(MoraleType);


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		IncSkill
//Author		R. Hyde 
//Date			Mon 9 Sep 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
SkillType AirStruc::IncSkill (AirStrucPtr ac,SWord aircraft_no)
{
	return ((SkillType)
		(Math_Lib.rnd((SKILL_HERO - MMC.Temp_Pilots[aircraft_no].skill) * ac->ai.morale)/MORALE_TOPMORALE));//RDH 04Dec96
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModIfyPilotMorale
//Author		R. Hyde 
//Date			Mon 9 Sep 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::ModifyPilotMorale(SWord i, MoraleType moralestep)
{
	if (	(MMC.Temp_Pilots[i].morale + moralestep) > MORALE_TOPMORALE)
	{
		MMC.Temp_Pilots[i].morale = MORALE_TOPMORALE;
	}elseif	(	(MMC.Temp_Pilots[i].morale + moralestep) < MORALE_ROCKBOTTOM)
	{
		MMC.Temp_Pilots[i].morale = MORALE_ROCKBOTTOM;
	}else
	{
		MMC.Temp_Pilots[i].morale += moralestep;
	}

}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModIfyPlayerMerit
//Author		R. Hyde 
//Date			Mon 9 Sep 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::ModifyPlayerMerit(DeltaMeritType meritstep)
{

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EndGameScoring
//Author		R. Hyde 
//Date			Sun 8 Sep 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//Other Factors:	death of leader or buddy
//
//
//
//
//------------------------------------------------------------------------------
void	AirStruc::EndGameScoring()
{
}
