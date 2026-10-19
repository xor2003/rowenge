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
//Filename       acmmodel.cpp
//System         
//Author         Andrew McRae
//Date           Wed 16 Apr 1997
//Description    Mig Alley Flight Model
//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>

#include "dosdefs.h"
#include "worldinc.h"
#include "modvec.h"
#include "model.h"
#include "monotxt.h"
#include "savegame.h"
#include "ranges.h"
#include	<math.h>
#include "planetyp.h"
#include	"replay.h"
#include	"persons2.h"

//#define PRINT_AI_DATA


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Null
//Author		Andrew McRae
//Date			Fri 13 Jun 1997
//
//Description	reset everything
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AcmModel::Null ()
{
	ULong n = sizeof(AcmModel);
	for (UByte* p=(UByte*)this;n;n--,p++) {	*p = 0;	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Calc
//Author		Andrew McRae
//Date			
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AcmModel::Calc ()
{
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Pilot
//Author		Andrew McRae
//Date			Mon 12 May 1997
//
//Description	Determine target AcmPoint
//
//Inputs		none
//
//Returns		none
//
//------------------------------------------------------------------------------
void AcmModel::Pilot (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	SWord OldElevator = pModel->Elevator;	//CSB 08/03/99
	SWord OldAileron  = pModel->Aileron;	//CSB 08/03/99
	SWord OldRudder   = pModel->Rudder;	//CSB 08/03/99

	if (AcmMode[ACM_CLIMBRATE])
		ModClimbRate (ControlledAC);

	if (AcmMode[ACM_HEIGHT])
		ModHeight (ControlledAC);

	if (AcmMode[ACM_MINHEIGHT])
		ModMinHeight (ControlledAC);

	if (AcmMode[ACM_KEEPWINGSLEVEL])
		ModKeepWingsLevel (ControlledAC);

	if (AcmMode[ACM_ROLLRATE])
		ModRollRate (ControlledAC);

	if (AcmMode[ACM_ROLL])
		ModRoll (ControlledAC);

	if (AcmMode[ACM_FASTROLL])
		ModFastRoll (ControlledAC);

	if (AcmMode[ACM_SETTHRUST])
		SetThrust (ControlledAC);

	if (AcmMode[ACM_PITCHRATE])
		ModPitchRate (ControlledAC);

	if (AcmMode[ACM_PITCH])
		ModPitch (ControlledAC);

	if (AcmMode[ACM_VELPITCH])
		ModVelPitch (ControlledAC);

	if (AcmMode[ACM_SPEED])
		ModSpeed (ControlledAC);

	if (AcmMode[ACM_TURNRATE])
		ModTurnRate (ControlledAC);

	if (AcmMode[ACM_VECTOR])
		ModVector (ControlledAC);

	if (AcmMode[ACM_STAYVERTICAL])
		ModStayVertical (FALSE);

	if (AcmMode[ACM_ZEROSPECIFICPOWER])
		ModZeroSpecificPower (ControlledAC);

	if (AcmMode[ACM_YAWRATE])
		ModYawRate (ControlledAC);

	if (AcmMode[ACM_LIFTLIMIT])
		ModLiftLimit (ControlledAC);

	if (AcmMode[ACM_RUDDER])
		ModRudder (ControlledAC);

	if (AcmMode[ACM_ELEVATOR])
		ModElevator (ControlledAC);

	if (AcmMode[ACM_AILERON])
		ModAileron (ControlledAC);

	if (AcmMode[ACM_AOA])
		ModAoA (ControlledAC);

	if (AcmMode[ACM_RELAOA])
		ModRelAoA (ControlledAC);


	//Yaw Damping
	if((pModel->Rudder < 3277) && (pModel->Rudder > -3277))
	{
		pModel->Rudder = -pModel->RotVel.y / pModel->AirVel.z * yawmodifier;
		MODLIMIT(pModel->Rudder, 3276);
	}

	SWord MaxDelta = 6553 * MODEL_DT;

	if(pModel->Elevator > OldElevator + MaxDelta)
		pModel->Elevator = OldElevator + MaxDelta;
	if(pModel->Elevator < OldElevator - MaxDelta)
		pModel->Elevator = OldElevator - MaxDelta;

	if(pModel->Aileron > OldAileron + MaxDelta)
		pModel->Aileron = OldAileron + MaxDelta;
	if(pModel->Aileron < OldAileron - MaxDelta)
		pModel->Aileron = OldAileron - MaxDelta;

	if(pModel->Rudder > OldRudder + MaxDelta)
		pModel->Rudder = OldRudder + MaxDelta;
	if(pModel->Rudder < OldRudder - MaxDelta)
		pModel->Rudder = OldRudder - MaxDelta;


	OldAoa = pModel->MainPlaneList->aoa[0][0];
	if(pModel->MainPlaneList->aoa[1][0] * pModel->MainPlaneList->aoa[1][0] > OldAoa * OldAoa)
		OldAoa = pModel->MainPlaneList->aoa[1][0];

	ActionAeroDeviceCommands (ControlledAC);

	Clear();
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ActionAeroDeviceCommands
//Author		Andrew McRae
//Date			Thu 18 Sep 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AcmModel::ActionAeroDeviceCommands (AirStrucPtr const ControlledAC)
{
	// Flaps, Speed Brakes, Slats, Gear up/down
	PMODEL pModel = ControlledAC->fly.pModel;

	PAERODEVICE pAeroDevice = pModel->DeviceList;
	while (pAeroDevice != NULL)
	{

		ULong command = ((ULong)pAeroDevice->Type << 1) + ACM_AERODEVICE_START;

		if (AcmMode[(ACM_COMMAND)(command - 1)])
		{
			pAeroDevice->Attractor = 0x0000;
			AcmMode %= (ACM_COMMAND)(command - 1);
		}

		if (AcmMode[(ACM_COMMAND)command])
		{
			pAeroDevice->Attractor = 0x4000;
			AcmMode %= (ACM_COMMAND)command;
		}

		pAeroDevice->Action (ControlledAC, AeroDevice::CT_AUTO);

		pAeroDevice = pAeroDevice->List.NextItem ();
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModClimbRate
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModClimbRate (AirStrucPtr const ControlledAC)
{
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModHeight
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModHeight (AirStrucPtr const ControlledAC)
{
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModMinHeight
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModMinHeight (AirStrucPtr const ControlledAC)
{
	if (ControlledAC->fly.pModel->Pos.y - CtrlHeight < 0)	
		Pitch(ControlledAC->CalcBestClimbPitch());
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModRollRate
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModRollRate (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	const	FP	MAXROLLRATEFACTOR = 100/FPIE;

	FP deltaroll = CtrlRollRate - pModel->RotVel.z;
	deltaroll = AngleSign (deltaroll);

	CtrlAileron = rollmodifier * MaxAileron * deltaroll * MAXROLLRATEFACTOR;
	ModAileron(ControlledAC);

#ifdef PRINT_AI_DATA
PrintVar(50, 23, "MRollR %.1f ", FP(CtrlRollRate * 57.3 * 100.0));
PrintVar(65, 23, "%.1f ", FP(pModel->RotVel.z * 57.3 * 100));
#endif
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModRoll
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModRoll (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;
	FP deltaroll = AngleSign (CtrlRoll - pModel->fRoll);
	FP aileron = deltaroll * rollmodifier;
	MODLIMIT(aileron, MaxAileron);
	pModel->Aileron = aileron;

#ifdef PRINT_AI_DATA
PrintVar(50, 23, "MRoll %.1f ", FP(CtrlRoll * 57.3));
PrintVar(65, 23, "%.1f ", FP(pModel->fRoll * 57.3));
#endif


//integral control
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModFastRoll
//Author		Craig Beeston
//Date			Tue 18 Jan 2000
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModFastRoll (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	FP deltaroll = AngleSign (CtrlRoll - pModel->fRoll);
	FP RollRate = pModel->RotVel.z;
	FP DesRollRate = deltaroll * 0.05;
	FP DesAileron  = (DesRollRate - RollRate) * 3.2767e6;
	MODLIMIT(DesAileron, MaxAileron);
	pModel->Aileron = DesAileron;

#ifdef PRINT_AI_DATA
PrintVar(50, 23, "MFastR %.1f ", FP(CtrlRoll * 57.3));
PrintVar(65, 23, "%.1f ", FP(pModel->fRoll * 57.3));
#endif

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModKeepWingsLevel
//Author		Andrew McRae
//Date			Fri 5 Sep 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModKeepWingsLevel (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	if ((pModel->fRoll > F1PIE2) && (pModel->fRoll < F3PIE2))
		CtrlRoll = FPIE;
	else
		CtrlRoll = 0;
	ModFastRoll(ControlledAC);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetThrust
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::SetThrust (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	ControlledAC->fly.thrustpercent = CtrlThrust;
	pModel->ModelThrottle = CtrlThrust;
	for(PENGINE pEngine = pModel->EngineList; pEngine; pEngine = pEngine->List.NextItem() )
		pEngine->ThrottleSetting = pModel->ModelThrottle;
	AcmMode %= ACM_SETTHRUST;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModPitchRate
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModPitchRate (AirStrucPtr const ControlledAC)
{
	SLong	daileron;
	PMODEL pModel = ControlledAC->fly.pModel;

	daileron = -CtrlPitchRate/4;		//up is positive

	if (ControlledAC->roll >> ANGLES_180Deg)
		daileron = -daileron;
	if  (		(		(ControlledAC->roll << ANGLES_40Deg)
					&&	(daileron < 0)
				)
			||	(		(ControlledAC->roll >> ANGLES_320Deg)
					&&	(daileron > 0)
				)
		)
	{
		ModRoll(ControlledAC);
	}
	else
	{
		CtrlAileron = pModel->Aileron + daileron;
	}
	ModAileron(ControlledAC);
}


inline void AcmModel::ModAoA (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	FP aoa0 = AngleSign (pModel->MainPlaneList->aoa[0][0]) + AngleSign (pModel->MainPlaneList->aoa[0][1]);
	FP aoa1 = AngleSign (pModel->MainPlaneList->aoa[1][0]) + AngleSign (pModel->MainPlaneList->aoa[1][1]);
	aoa0 *= 0.5;
	aoa1 *= 0.5;
	aoa0 = (aoa1+aoa0)/2;

	CtrlAoA = AngleSign (CtrlAoA);
	FP deltapitch = aoa0 - CtrlAoA;

	pModel->Elevator = aoamodifier * deltapitch;
	MODLIMIT(pModel->Elevator, MaxElevatorForTightTurn);
	pModel->Elevator *= fLimitAoa(ControlledAC);

#ifdef PRINT_AI_DATA
	PrintVar(50, 24, "MAoa %.1f ", FP(CtrlAoA * 57.3));
	PrintVar(65, 24, "%.1f ", FP(aoa0 * 57.3));
#endif

///////////////////////////////////////////////////
}


//try to get to within CtrlAoA of critical aoa
inline void AcmModel::ModRelAoA (AirStrucPtr const ControlledAC)
{
#ifdef PRINT_AI_DATA
PrintString(50, 23, "ModRelAoa                   ");
PrintString(50, 24, "PitchModifier Int           ");
#endif
	PMODEL pModel = ControlledAC->fly.pModel;

	UByte	FlyingFactor = ControlledAC->FlyingFactor();
	FP aoa0 = AngleSign (pModel->MainPlaneList->aoa[0][0]) + AngleSign (pModel->MainPlaneList->aoa[0][1]);
	FP aoa1 = AngleSign (pModel->MainPlaneList->aoa[1][0]) + AngleSign (pModel->MainPlaneList->aoa[1][1]);
	aoa0 *= 0.5;
	aoa1 *= 0.5;

	FP maxaoa = pModel->MainPlaneList->ACMmaxaoa;// -
//					((256 - FlyingFactor) * 0.174)/256;	//* 10 degs

	if (aoa0 < aoa1)
		aoa0 = aoa1;

	FP deltapitch = aoa0 - (maxaoa - CtrlAoA);

	if  (	(!NearMaxAoA(ControlledAC))
		||	(deltapitch > 0)
		)
	{
		if (deltapitch > DeltaPitchForMaxElevator)
			pModel->Elevator = MaxElevatorForTightTurn;
		else if (deltapitch < -DeltaPitchForMaxElevator)
			pModel->Elevator = -MaxElevatorForTightTurn;
		else
//			pModel->Elevator =  3 *pitchmodifier * MaxElevatorForTightTurn * deltapitch / DeltaPitchForMaxElevator;
			pModel->Elevator =  pitchmodifier * MaxElevatorForTightTurn * deltapitch / DeltaPitchForMaxElevator;


//Integral control
		if (deltapitchsum > IntPitchForMaxElevator)
		{
			pModel->Elevator += MaxElevatorForTightTurn;
			deltapitchsum = IntPitchForMaxElevator;
		}
		else if (deltapitchsum < -IntPitchForMaxElevator)
		{
			pModel->Elevator -= MaxElevatorForTightTurn;
			deltapitchsum = -IntPitchForMaxElevator;
		}
		else
			pModel->Elevator +=  pitchintmodifier *MaxElevatorForTightTurn * deltapitchsum / IntPitchForMaxElevator;


		if (pModel->Elevator > MaxElevatorForTightTurn)
			pModel->Elevator = MaxElevatorForTightTurn;
		if (pModel->Elevator < -MaxElevatorForTightTurn)
			pModel->Elevator = -MaxElevatorForTightTurn;


		if  (		(ControlledAC->roll > ANGLES_90Deg)
				&&	(ControlledAC->roll < ANGLES_270Deg)
			)
				pModel->Elevator = -pModel->Elevator; 

		lastdeltapitch = deltapitch;
		deltapitchsum += deltapitch;

	}

}


Bool AcmModel::NearMaxAoA(AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	UByte	FlyingFactor = ControlledAC->FlyingFactor();
	FP aoa0 = AngleSign (pModel->MainPlaneList->aoa[0][1]);
	FP aoa1 = AngleSign (pModel->MainPlaneList->aoa[1][1]);

	FP maxaoa = pModel->MainPlaneList->ACMmaxaoa;

#ifdef PRINT_AI_DATA
	PrintString(60, 6, "               ");
#endif
	if( (aoa0 > maxaoa) || (aoa1 > maxaoa) )
	{
		CtrlElevator = 0;
		if (CtrlElevator > 0)
			CtrlElevator = 0;
		ModElevator(ControlledAC);	
#ifdef PRINT_AI_DATA
		PrintString(60, 6, "**** STALL ****");
#endif
		return (TRUE);
	}
	else
		return(FALSE);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		fLimitAoa
//Author		Craig Beeston
//Date			Tue 18 Jan 2000
//
//Description	Returns a factor between 0 and 1 for modifying elevator inputs to limit the aoa based on skill
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
FP AcmModel::fLimitAoa(AirStrucPtr const ControlledAC)
{
	FP retval = 1;
	PMODEL pModel = ControlledAC->fly.pModel;
	FP aoa	  = pModel->MainPlaneList->aoa[0][0];
	if(pModel->MainPlaneList->aoa[1][0] * pModel->MainPlaneList->aoa[1][0] > aoa * aoa)
		aoa = pModel->MainPlaneList->aoa[1][0];
	FP maxaoa = pModel->MainPlaneList->ACMmaxaoa * (256.0 + FP(ControlledAC->FlyingFactor())) / 512.0;

	FP AoaRate = (aoa - OldAoa) / MODEL_DT;
	if(aoa * AoaRate > 0)
	{
		if(aoa < 0) 
		{
			maxaoa *= -1;
			AoaRate *= -1;
		}
		FP f_aoa = 0.1 + (1.0 - 0.90 * aoa / maxaoa) * 0.0025 / AoaRate;
		MODMAXMIN(f_aoa, 0.1, 1);
		retval = f_aoa;
	}

#ifdef PRINT_AI_DATA
	PrintVar(45, 8 ,"MAoa %.1f ", FP(maxaoa * 57.3));
	PrintVar(55, 8 ,"FAoa %.2f ", FP(retval));
	
	static float aveaoa = 0;
	aveaoa = (aoa + 9 * aveaoa) / 10;
	PrintVar(45, 10 ,"AAoa %.2f ", FP(aveaoa * 57.3));
#endif

	return(retval);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModPitch
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModPitch (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;
	FP deltapitch = AngleSign(CtrlPitch) - AngleSign(Rowan2Rads(ControlledAC->pitch));
	FP tempelev   = -pitchmodifier * deltapitch;

	Float sin, cos;								
	Math_Lib.high_sin_cos(ControlledAC->roll, sin, cos);
	pModel->Elevator = tempelev;
	if(cos < 0) 
		pModel->Elevator *= cos;
	MODLIMIT(pModel->Elevator, MaxElevatorForTightTurn);
	pModel->Elevator *= fLimitAoa(ControlledAC);

	pModel->Rudder   = tempelev * sin;

#ifdef PRINT_AI_DATA
PrintVar(50, 24, "MPitch %.1f ", FP(CtrlPitch * 57.3));
PrintVar(65, 24, "%.1f ", FP(ControlledAC->pitch / 182.04));
#endif

///////////////////////////
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModVelPitch
//Author		Craig Beeston
//Date			Fri 19 Nov 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModVelPitch (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;
	FP deltapitch = AngleSign(CtrlPitch) - AngleSign(Rowan2Rads(ControlledAC->fly.cpitch));
	FP tempelev   = -pitchmodifier * deltapitch;

	Float sin, cos;								
	Math_Lib.high_sin_cos(ControlledAC->roll, sin, cos);
	pModel->Elevator = tempelev;
	if(cos < 0) 
		pModel->Elevator *= cos;
	MODLIMIT(pModel->Elevator, MaxElevatorForTightTurn);
	pModel->Elevator *= fLimitAoa(ControlledAC);

	pModel->Rudder   = tempelev * sin;

#ifdef PRINT_AI_DATA
PrintVar(50, 24, "MVelP %.1f ", FP(CtrlPitch * 57.3));
PrintVar(65, 24, "%.1f ", FP(ControlledAC->fly.cpitch / 182.04));
#endif

///////////////////////
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModSpeed
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModSpeed (AirStrucPtr const ControlledAC)
{
//	pModel->ThrottleFrac += (SLong)((CtrlSpeed - pModel->AirSpeed) * Speed_Throttle_Offset_Power);
//	pModel->ThrottleFrac += (SLong)((pModel->OldAirSpeed - pModel->AirSpeed) * Speed_Throttle_Damping_Power);
//	MODMAXMIN (pModel->ThrottleFrac, 0, 25600);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModSpeedWithElevators
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModSpeedWithElevators (AirStrucPtr const ControlledAC)
{
#ifdef PRINT_AI_DATA
PrintString(50, 23, "ModSpeedWithElevators       ");
PrintString(50, 24, "PitchIntModifier            ");
#endif
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModTurnRate
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModTurnRate (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;
	pModel->Elevator = -turnmodifier * CtrlTurnRate;
	MODLIMIT(pModel->Elevator, MaxElevatorForTightTurn);
	pModel->Elevator *= fLimitAoa(ControlledAC);

#ifdef PRINT_AI_DATA
PrintVar(50, 24, "MTurn %.1f ", FP(CtrlTurnRate * 57.3 * 100.0));
PrintVar(65, 24, "%.1f ", FP(-pModel->RotVel.x * 57.3 * 100.0));
#endif

///////////////////////////////////////////////
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModVector
//Author		Andrew McRae
//Date			Wed 21 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline void AcmModel::ModVector (AirStrucPtr const ControlledAC)
{
}


void AcmModel::ModYawRate(AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;

	FP velmodifier = 1;	//CSB 08/03/99
	FP modifier = yawmodifier * velmodifier * velmodifier;
	FP intmodifier = yawintmodifier * velmodifier * velmodifier;

	FP	distancemod = 0.00001 * (ControlledAC->Range - METRES500);		//max out at 1500m
	if (distancemod < 0)
		distancemod = 0;
	if (distancemod > 1)
		distancemod = 1;
	else
		distancemod = 1 - distancemod;

	int MaxIntRudder  = MaxElevatorForTightTurn/4;
 
	FP delta = OldCtrlYawRate - CtrlYawRate;
	if (delta < 0)
		delta = -delta;
	if(delta > 0.05)
	{//change of nearly 3 degs
		deltayawsum = 0;
		OldCtrlYawRate = CtrlYawRate;
	}
 


		if (CtrlYawRate < -.5)
			pModel->Rudder = -modifier *  MaxElevatorForTightTurn * 0.5;
		else if (CtrlYawRate > 0.5)
			pModel->Rudder = modifier * MaxElevatorForTightTurn * 0.5;
		else
			pModel->Rudder =   modifier * MaxElevatorForTightTurn * CtrlYawRate;

		if (deltayawsum > 40)
		{
			pModel->Rudder += intmodifier *MaxIntRudder;
			if (CtrlYawRate < 0)
				deltayawsum += CtrlYawRate;
		}
		else if (deltayawsum < -40)
		{
			pModel->Rudder -= intmodifier *MaxIntRudder;
			if (CtrlYawRate >= 0)
				deltayawsum += CtrlYawRate;
		}
		else
		{
			pModel->Rudder +=  intmodifier * MaxIntRudder * deltayawsum / 40;
			deltayawsum += CtrlYawRate;
		}
		pModel->Rudder = pModel->Rudder * distancemod;
}


void AcmModel::ZeroIntControl()
{
	deltayawsum = 0;
	deltarollsum = 0;
	deltapitchsum = 0;
//##	ControlledAC->ai.deltapitchsum = 0;

}


void AcmModel::ModLiftLimit (AirStrucPtr const ControlledAC)
{
	PMODEL pModel = ControlledAC->fly.pModel;
	CtrlAoA = pModel->MainPlaneList->ACMmaxaoa * (256.0 + FP(ControlledAC->FlyingFactor())) / 512.0;
	ModAoA(ControlledAC);

#ifdef PRINT_AI_DATA
PrintVar(50, 24, "MLift %.1f ", FP(CtrlAoA * 57.3));
#endif

////////////////////////////////
}


void AcmModel::ModZeroSpecificPower (AirStrucPtr const ControlledAC)
{
	ModLiftLimit(ControlledAC);

#ifdef PRINT_AI_DATA
PrintVar(50, 24, "M0_SP %.0f ", FP(0));
#endif

///////////////////////////////////////////////////////////
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ModStayVertical
//Author		Andrew McRae
//Date			Mon 8 Sep 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AcmModel::StayVertical (AirStrucPtr const ControlledAC)
{
	AcmMode |= ACM_STAYVERTICAL;
	ModStayVertical (TRUE);
}


inline void AcmModel::ModRudder(AirStrucPtr const ControlledAC)
{
	ControlledAC->fly.pModel->Rudder = CtrlRudder;

}


inline void AcmModel::ModElevator(AirStrucPtr const ControlledAC)
{
	ControlledAC->fly.pModel->Elevator = CtrlElevator;

}


inline void AcmModel::ModAileron(AirStrucPtr const ControlledAC)
{
	ControlledAC->fly.pModel->Aileron = CtrlAileron;

}


inline void AcmModel::ModStayVertical (Bool Init)
{
	// use elevator & rudder & aileron
//	PMODEL pModel = ControlledAC->fly.pModel;
//
//	static ULong Stage = 0;
//
//	if (Init)
//	{
//		Stage = 0;
//		return;
//	}
//
//	FP sRoll = AngleSign (pModel->fRoll);
//
//	switch (Stage)
//	{
//		case 0:
//			Roll (0);
//			if ((sRoll < DEGS2RADS(25)) && (sRoll > -DEGS2RADS(25)))
//				Stage = 1;
//			break;
//
//		case 1:
//			Pitch (90);
//			break;
//	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CalcControls
//Author		Andrew McRae
//Date			Mon 12 May 1997
//
//Description	Calc desired Orientation and speed to achieve ACMPoint
//				And determine if Acm point has been passed
//				(i.e. end or go to next point)
//
//Inputs		Acm Point
//
//Returns		TRUE if completed this point, else FALSE
//
//------------------------------------------------------------------------------
//Bool AcmModel::Logic (ACMPOINT& Pnt)
Bool AcmModel::Logic (AirStrucPtr const ControlledAC)
{
	return FALSE;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		Control
//Author		Andrew McRae
//Date			Mon 12 May 1997
//
//Description	Calc actual control inputs to achieve desired attitude & speed
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AcmModel::Control (FORI& Ori, FP Speed)
{
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DeltaAngle
//Author		Andrew McRae
//Date			Thu 15 May 1997
//
//Description	returns signed difference between to angles
//
//Inputs		a1, a2
//
//Returns		a2 - a1
//
//------------------------------------------------------------------------------
inline SWord DeltaAngle (UWord a1, UWord a2)
{
	SLong dif = (SLong)a2 - (SLong)a1;
	if (dif > 32767) dif = dif - 65536;
	if (dif < -32768) dif = 65536 + dif;
	return (SWord)dif;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		DeltaAngle
//Author		Andrew McRae
//Date			
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
FP DeltaAngle (FP a1, FP a2)
{
	FP dif = a2 - a1;
	if (dif > FPIE) dif = dif - F2PIE;
	if (dif < -FPIE) dif = F2PIE + dif;
	return dif;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ControlAttractor
//Author		Andrew McRae
//Date			Thu 8 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
SWord ControlAttractor (SWord val, SWord att, UWord rate)
{
	SLong dval = (SLong)(att - val);
	if (dval > 0)
	{
		dval >>= rate;
		if (!dval) dval = 1;
		val += dval;
		return val;
	}
	if (dval < 0)
	{
		dval = -dval;
		dval >>= rate;
		if (!dval) dval = 1;
		val -= dval;
		return val;
	}
	return val;
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		CtrlShiftRight
//Author		Andrew McRae
//Date			Mon 12 May 1997
//
//Description	Shifts a number right always leaving 1 bit set
//
//Inputs		the number and shift value
//
//Returns		the shifted number
//
//------------------------------------------------------------------------------
SWord CtrlShiftRight (SWord val, SWord shift)
{
	if (!val) return val;
	val = val >> shift;
	if (!val) val = 1;
	return val;
}
