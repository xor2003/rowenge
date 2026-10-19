//------------------------------------------------------------------------------
//Filename       planetyp.h
//System         
//Author         R. Hyde 
//Date           Thu 26 Oct 1995
//Description    Use I_planetype	to declare an instance
//				 Use name.I			to access the instance
// This module contains macros to initialise inherited classes "c" style.
// If the module is declared a second time, then this alternate construction 
// is made to allow instances to occur. 
// The re-caller has to undef PLANETYP_Included to avoid accidental double call
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	PLANETYP_Included
#define	PLANETYP_Included
#include	"DosDefs.h"
#include	"files.g"		//NON-GRAPHICS FILES				//RDH 16Jul96
#include	"myANGLES.h"											//ARM 02Aug96
#include	"flyinit.h"											//ARM 02Aug96
#include	"worldinc.h"										//ARM 03Feb97

#ifdef	DEFAULT_PLANETYP
//
// If the module is declared a second time, then this alternate construction 
// is made to allow instances to occur.
//
	#define	emptybasestruct(subname,parentname)						\
	struct	II_##subname			{

	#define	basestruct(subname)							\
	struct	II_##subname			{

	#define	substruct(subname,parentname)				\
	struct	II_##subname			{					\
		II_##parentname	parentinst;

	#define	endstruct(subname)		};					\
	union	I_##subname									\
		{	II_##subname II;subname I;}

	#define	startinst(strucname,instname)	 			\
		II_##strucname	I_##instname	=	{

	#define	endinst(strucname,instname)	 	};			\
		strucname&	instname=*(strucname*)&I_##instname;
#undef 	DEFAULT_PLANETYP
#else
//
// This definition defines substructures by simple inheritance
// That will not allow initialisation.
//
#include	"shapenum.g"
	#define	emptybasestruct(subname,parentname) struct	subname : parentname	{

	#define	basestruct(subname) struct	subname					{

	#define	substruct(subname,parentname) struct	subname : parentname	{

	#define	endstruct(subname)		}

enum	ModelPatch	{PATCHWW1=1,PATCHWW2,PATCHWW3,PATCHMAX=0xffffffff};
enum	InstType	{lightinstrumtypes=0};


MINMAX(WeightEmpty,0,100000);
MINMAX(FuelLoad,0,2000000000);
MINMAX(FuelTankCap,0,200000000);
MINMAX(FuelTankXpos,-32767,32767);
MINMAX(FuelABoff,0,65535);
MINMAX(ThrustperStage,0,65535);
MINMAX(ABStages,0,65535);
MINMAX(MilMax,0,65535);
MINMAX(MaxTurnRate,0,65535);
MINMAX(MaxRollRate,0,65535);
MINMAX(MaxPitchRate,0,65535);
MINMAX(MaxAil,0,65535);
MINMAX(MaxAccel,0,262143);
MINMAX(RollRateAccel,0,65535);
MINMAX(LandingSpeed,0,2147483648);
MINMAX(MaxDiveRpm,0,65535);
MINMAX(MaxRpm,0,65535);
MINMAX(FreeRpm,0,65535);
MINMAX(MaxTorque,0,65535);
MINMAX(MinRpm,0,65535);
MINMAX(BestClimbVel,0,2147483648);
MINMAX(MaxDiveVel,0,2147483648);

MINMAX(MaxVel,0,2147483648);
MINMAX(MinVel,0,2147483648);
MINMAX(CruiseVel,0,2147483648);
MINMAX(BestClimbAng,0,65536);	//CSB 28/09/98
MINMAX(BestClimbSpeed,0,2147483648);	//CSB 28/09/98
MINMAX(MaxVelSL,0,2147483648);	//CSB 28/09/98

MINMAX(MaxVelDive_0km,0,2147483648);	//CSB 28/09/98
MINMAX(MaxVelDive_11km,0,2147483648);	//CSB 28/09/98
MINMAX(MaxVelDive_20km,0,2147483648);	//CSB 28/09/98

MINMAX(MovableFireZoneType,-32768,32767);
MINMAX(Damage,0,65535);

enum	ACFlags	{hooka=1,bda=2,HUDINACTIVE=4,etca=8};
MINMAX(FightCeiling,	0,10000000);
MINMAX(Abs_Ceiling,		0,10000000);	//CSB 29/09/98
MINMAX(MuzzleVel,		0,2147483648);
#endif
#define	DEFAULT_PLANETYP 0

//TempCode JIM 23Jan96 basestruct(PlaneInit)	//stuff from planetype that is only used for initialisation

//TempCode JIM 23Jan96 isbasestruct(TextBlockKnown);
struct	DumbPlaneType;
struct	PlaneType;

basestruct(PlaneInit)
	WeightEmpty	weightempty;
	FuelLoad	maxintfuel;
	FuelTankCap fueltankcap[4];
	FuelTankXpos fueltankxpos[4];
	FuelTankType fueltanktype[4];
	operator	DumbPlaneType* ()	{return(DumbPlaneType*)this;}
	operator	PlaneType* ()	{return(PlaneType*)this;}

endstruct(PlaneInit);

substruct(WheeledType,PlaneInit)
	ShapeNum	shadowshpno;
	PlaneTypeSelect			planetext;	//	TextSnip0	planetext;
	ShapeNum	visible,SAGsubshape,commsVisible;				//RJS 12Oct00
	FuelABoff	fuelaboff;
	
	int	/*MaxVel*/		 	maxvel;
	int	/*MinVel*/			minvel;
	int	/*CruiseVel*/		cruisevel;

	int	/*BestClimbVel*/	bestclimbvel;
		ANGLESInst		 	bestclimbpitch;
	int	/*MaxDiveVel*/	 	maxdivevel;
	MaxAccel	maxaccel;
	ANGLESInst				turnpercs;
	int						radiusturn;

endstruct(WheeledType);

substruct(DumbPlaneType,WheeledType)
	ShapeNum	cockpit1;										//PD 20Jun96
	GunnerInfo	gunnerpit[4];									//RJS 11Oct00
	Float		cockscale;										//RJS 19Jan99
	ANGLESInst	hdgarc;											//RJS 03Dec99
	ANGLESInst	pitchmax;										//RJS 03Dec99
	ANGLESInst	pitchmin;										//RJS 03Dec99
	SWord		finpitch;											  //CSB 17/12/99
	UWord		finlhdgmin;											  //CSB 17/12/99
	UWord		finlhdgmax;											  //CSB 17/12/99
	UWord		finrhdgmin;											  //CSB 17/12/99
	UWord		finrhdgmax;											  //CSB 17/12/99
	ULong		phrasename;										//RJS 10Feb99
	AerobaticFactor	aerobaticfactor;
	ANGLESInst		deckangle;
	UWord			deckshunt;
	MaxRollRate	maxrollrate;
	MaxPitchRate	maxpitchrate;
	LandingSpeed	landingspeed;
	MaxDiveRpm	 	maxdiverpm;
	FreeRpm	 	freerpm;
	MaxRpm	 	maxrpm;
	MinRpm	 	minrpm;
	MovableFireZoneType	movablefirezonetype;
	Damage		damage;
	   
	FightCeiling	fightceiling;
	Abs_Ceiling		abs_ceiling;
	Float	 	maxturnrateaoaSL;

	MuzzleVel		muzzlevel;
	FileNum			gunsnd[5];									//RJS 31Jul00
	FileNum			enginesnd;									//RJS 09Jun00
	FileNum			enginesubsnd;									  //CSB 14/01/00
	FileNum			enginestartsnd;									  //CSB 14/01/00
	UByte			noGuns;										//RJS 29Mar99
	SWord			gundelay;									//RJS 12Apr99
	UWord			gunfrequency;								//RJS 31May99

       
endstruct(DumbPlaneType);

substruct(PlaneType,DumbPlaneType)

	InstType	insttype;
	ACFlags		acflags;

	BestClimbAng	bestclimbang[21];	//CSB 28/09/98
	BestClimbSpeed  bestclimbspeed[21];	//CSB 28/09/98
	MaxVelSL		maxvelsl[21];	//CSB 14/10/99	
	
	MaxVelDive_0km	maxveldive_0km;	//CSB 28/09/98
	MaxVelDive_11km	maxveldive_11km;	//CSB 28/09/98
	MaxVelDive_20km	maxveldive_20km;	//CSB 28/09/98

	ULong		version;

	void (*pSetupFunction) (AirStrucPtr const, ULong);		//ARM 28Jan97

endstruct(PlaneType);

typedef	PlaneType	*PTypePtr;

#undef	basestruct
#undef	emptybasestruct
#undef	substruct
#undef	endstruct

extern	PlaneInit*	Plane_Type_Translate[];

#pragma pack(pop)
#endif
