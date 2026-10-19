//------------------------------------------------------------------------------
//Filename       modinst.h
//System         
//Author         Andrew McRae
//Date           Wed 27 Aug 1997
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	MODINST_Included
#define	MODINST_Included

#define	DEFAULT_MODINST 0

//typedef class Model *PMODEL;
#include "modvec.h"

enum	AnimSteps
{
	ARC_IMMEDIATE = 1,
	ARC_1SEC = 100,
	ARC_30SEC = 3000,
	ARC_240SEC = 24000
};

enum	SrcBaseType
{
	SBASE_DEFAULT = 0,
	SBASE_AC,
	SBASE_ANIM
};

#define	RUNNING_VOLTS  28
#define	MaxDialSrc 		8
#define	MaxAutoDial		48					//RJS 03Dec99

struct	DialSpec
{
	int	DestValOffset;
	int	SrcValOffset;
	int	SrcValStructIndex;

	FP	ValScale;

	FP	minval;
	FP	maxval;

	int		minIntval;
	int		maxIntval;
	int		Intvalscale;

	UWord	shiftsrc:6,
			shiftdest:6,
			useint:1;

	UWord	torestval;
	SrcBaseType	btype;

	void	SetDialValue(ItemPtr,UByteP,UByteP*);
};

struct	TimedDialSpec											//RJS 29Jun98
{
	SWord	theTimer;
	SLong	SrcVal;
	SLong	OldGap;
	SLong	OldVal;

	int		destOffset;

	int		timeUp;
	int		timeDown;

	Bool	signedDest;

	UByte	nobitsDest;

	void	SetTimedAnim(void*	adptr);
};


typedef class _instruments
{

public:

	//BOB INSTRUMENTS
	FP I_TrueGroundSpeed;
	FP I_TrueAirSpeed;
	FP I_IndicatedAirSpeed;
	FP I_MachNo;
	FP I_PitchAngle;
	FP I_RollAngle;
	FP I_Heading;
	FP I_RateOfClimb;
	FP I_Altitude;
	FP I_RadioAltitude;
	FP I_Turn;
	FP I_Slip;
	SWord I_UndercarraigePosition0;
	SWord I_UndercarraigePosition1;
	SWord I_MechanicalUndercarraigeIndicator;
	SWord I_ElevatorTrimTab;
	SWord I_FuelContents;
	SByte I_FuelWarningLamp0;
	SByte I_FuelWarningLamp1;
	SByte I_FuelWarningLamp2;
	SByte I_FuelWarningLamp3;
	FP I_FuelPressure;
	FP I_OilContent;
	FP I_OilPressure;
	FP I_OilTemperature;
	FP I_RadiatorTemperature0;
	FP I_RadiatorTemperature1;
	FP I_Boost0;
	FP I_Boost1;
	FP I_EngineRpm0;
	FP I_EngineRpm1;
	FP I_VoltMeter;
	FP I_PropPitch0;
	FP I_PropPitch1;
	FP I_FlapsIndicator;
	FP I_NormalAcc;

	//BOB CONTROLS
	SWord C_FlapLever;
	SWord C_UndercarraigeLever;
	SWord C_DiveBrakeLever;
	FP C_ElevatorTrim;
	FP C_RudderTrim;
	FP C_H_Gate;
	SWord C_EmergencyUndercarraige;
	FP C_Throttle0;
	FP C_Throttle1;
	FP C_PropPitch0;
	FP C_PropPitch1;
	SWord C_Magnetos0;
	SWord C_Magnetos1;
	SWord C_PrimerPump0;
	SWord C_PrimerPump1;
	SWord C_FuelCock0;
	SWord C_FuelCock1;
	SWord C_StarterButton0;
	SWord C_StarterButton1;
	SWord C_BoostControlCutout;
	SWord C_HoodCatch;
	SWord C_FuelGuageSelector;

	

	UByteP	DialSrcList[MaxDialSrc];							//RJS 16Feb98
	UWord	DialSrcCnt;											//RJS 16Feb98

	DialSpec	DialList[MaxAutoDial];							//RJS 29Jun98
	UWord		DialCnt;

	TimedDialSpec	TimedDialList[MaxAutoDial];					//RJS 29Jun98
	UWord			TimedDialCnt;

	SWord FuelGuageTank [4];
	SLong FuelGuageScale[4];

	void SetInstruments (AirStrucPtr const ControlledAC,PMODEL);

	SLong	SetTimedAnim (SLong&,UWord&,SLong,UWord,UWord);		//RJS 23Jun98
	SLong	EngineDamage(UByteP, int);							//RJS 16Feb98
	void	SetCockpitAnims(ItemPtr,UByteP);					//RJS 20Jul98
	void	UpdateCockpitTimers(SLong);							//RJS 16Feb98
	void	NewDialValue(void*,UByteP,void*,UByteP,int,int,int,UByte,UByte,UWord	restrot = 0);
	void	AddAutoDial(void*,UByteP,void*,SWordP,int,int,int scaler = 0);//RJS 24Apr98
	void	AddAutoDial(void*,SWordP,void*,SWordP,int,int,int scaler = 0);//RJS 24Apr98
	void	AddAutoDial(void*,UByteP,void*,ULongP,int,int,int scaler = 0);//RJS 24Apr98
	void	AddAutoDial(void*,SWordP,void*,ULongP,int,int,int scaler = 0);//RJS 24Apr98

	void	NewDialValue(void*,UByteP,void*,UByteP,FP,FP,FP,UByte,UWord	restrot = 0);
	void	AddAutoDial(void*,UByteP,void*,FP*,FP,FP,FP scaler = 0);//RJS 24Apr98
	void	AddAutoDial(void*,SWordP,void*,FP*,FP,FP,FP scaler = 0);//RJS 24Apr98

	void	AddAutoDial(void*,UByteP,void*,SLongP,int,int,int scaler = 0);//RJS 24Apr98
	void	AddAutoDial(void*,SWordP,void*,SLongP,int,int,int scaler = 0);//RJS 24Apr98

	void	AddTimedDial(void*,UByteP,int,int,Bool signedSrc=FALSE);//RJS 29Jun98
	void	AddTimedDial(void*,SByteP,int,int,Bool signedSrc=FALSE);//RJS 29Jun98
	void	AddTimedDial(void*,UWordP,int,int,Bool signedSrc=FALSE);//RJS 29Jun98
	void	AddTimedDial(void*,SWordP,int,int,Bool signedSrc=FALSE);//RJS 29Jun98
	void	AddTimedDial(void*,ULongP,int,int,Bool signedSrc=FALSE);//RJS 29Jun98
	void	AddTimedDial(void*,SLongP,int,int,Bool signedSrc=FALSE);//RJS 29Jun98

	void	NewTimedDial(int,int,int,Bool,Bool,UByte);			//RJS 29Jun98

	void	SetFuelGaugeSelector(SWord tank1, ULong scale1, SWord tank2, ULong scale2, SWord tank3, ULong scale3, SWord tank4, ULong scale4);

	void Null ()		
 	{
		I_TrueGroundSpeed = 0;
		I_TrueAirSpeed = 0;
		I_IndicatedAirSpeed = 0;
		I_MachNo = 0;
		I_PitchAngle = 0;
		I_RollAngle = 0;
		I_Heading = 0;
		I_RateOfClimb = 0;
		I_Altitude = 0;
		I_RadioAltitude = 0;
		I_Turn = 0;
		I_Slip = 0;
		I_UndercarraigePosition0 = 0;
		I_UndercarraigePosition1 = 0;
		I_MechanicalUndercarraigeIndicator = 0;
		I_ElevatorTrimTab = 0;
		I_FuelContents = 0;
		I_FuelWarningLamp0 = 0;
		I_FuelWarningLamp1 = 0;
		I_FuelWarningLamp2 = 0;
		I_FuelWarningLamp3 = 0;
		I_FuelPressure = 0;
		I_OilContent = 0;
		I_OilPressure = 0;
		I_OilTemperature = 0;
		I_RadiatorTemperature0 = 0;
		I_RadiatorTemperature1 = 0;
		I_Boost0 = 0;
		I_Boost1 = 0;
		I_EngineRpm0 = 0;
		I_EngineRpm1 = 0;
		I_VoltMeter = 0;
		I_PropPitch0 = 0;
		I_PropPitch1 = 0;
		I_FlapsIndicator = 0;
		I_NormalAcc = 0;

		//BOB CONTROLS
		C_FlapLever = 0;
		C_UndercarraigeLever = 0;
		C_ElevatorTrim = 0;
		C_RudderTrim = 0;
		C_H_Gate = 0;
		C_EmergencyUndercarraige = 0;
		C_Throttle0 = 0;
		C_Throttle1 = 0;
		C_PropPitch0 = 0;
		C_PropPitch1 = 0;
		C_Magnetos0 = 0;
		C_Magnetos1 = 0;
		C_PrimerPump0 = 0;
		C_PrimerPump1 = 0;
		C_FuelCock0 = 0;
		C_FuelCock1 = 0;
		C_StarterButton0 = 0;
		C_StarterButton1 = 0;
		C_BoostControlCutout = 0;
		C_HoodCatch = 0;
		C_FuelGuageSelector = 0;

	
 	}	

}
INSTRUMENTS,
*PINSTRUMENTS;

#pragma pack(pop)
#endif
