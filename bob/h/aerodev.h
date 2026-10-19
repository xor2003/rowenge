//------------------------------------------------------------------------------
//Filename       aerodev.h
//System         
//Author         Andrew McRae
//Date           Tue 12 Aug 1997
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	AERODEV_Included
#define	AERODEV_Included

#define	DEFAULT_AERODEV 0


class AeroDevice
{

public:

	enum DT_TYPE
	{
		DT_VOID,
		DT_GEAR,
		DT_FLAPS,
		DT_SLATS,
		DT_SPEEDBRAKES,
		DT_DIVEBRAKES,
		DT_CANOPY,
	};

	enum CONTROL_TYPE
	{
		CT_KEYS,
		CT_AUTO
	};

	enum POWER_TYPE
	{
		PT_VOID,
		PT_MANUAL,
		PT_ELECTRIC,
		PT_HYDRAULIC,
		PT_PNEUMATIC
	};

//CONSTANTS

//NEW CONSTANTS		//CSB 05/10/99	
	AirStrucPtr ControlledAC;
	PMODEL pModel;
	DT_TYPE Type;
	CONTROL_TYPE ControlType;
	POWER_TYPE PowerType;
	FP dCl, dCd, dCm, dAlpha;
	SWord Stages, Rate;
	KeyVal3D UpKey, DownKey;
	SWord BuffetVel;
	UByte	DeviceID;//RJS 18Sep98



//VARIABLES

//NEW VARIABLES
	SWord Val;
	SWord Attractor;
	SWord OldAttractor;
	SWord ReqVal;
	Bool  Broken;


	AeroDevice (AirStrucPtr const controlledAC,
				PMODEL pmodel,		// parent flight model class
				DT_TYPE type,		// DT_xxxxx
				CONTROL_TYPE controltype,
				POWER_TYPE powertype,
				FP dcl,	  			// delta CL
				FP dcd,				// delta CD
				FP dcm,				// delta CM
				FP dalpha,			// delta Alpha
				SWord stages,
				SWord rate,			// movement rate
				KeyVal3D upkey,		// control key (0 if none)
				KeyVal3D downkey,
				SWord buffetvel,
				UByte* pAnimByte,	// anim byte address
				SLong	minval=0,
				SLong	maxval=16384,
				SLong	Scale=0);		// anim scale

	~AeroDevice ();

	LINKEDLIST<AeroDevice> List;
	void Process ();
	void GearUp ();
	void GearDown ();

	void Action (AirStrucPtr const ControlledAC, CONTROL_TYPE,Bool sendpacket = TRUE);//RJS 21Sep98

} ;

typedef AeroDevice AERODEVICE, *PAERODEVICE;


#endif
#pragma pack(pop)
