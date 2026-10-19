//------------------------------------------------------------------------------
//Filename       3dinstr.h
//System         
//Author         Paul.   
//Date           Wed 16 Aug 1995
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	Dinstr_Included
#define	Dinstr_Included



//Offset in animdata...

enum	AnimSizes	{MINANIM=0,									//RJS 30Nov99
					AIRCRAFTANIM,
					SIMPLEAIRCRAFTANIM,	   						//RJS 17Nov99
					BLOKESANIM,									//RJS 01Jun99
					POLYPITANIM,   
					SUNANIM,	   
					WINDSOCANIM,   
					MOVEGUNANIM,   
					BARAGEANIM,	   
					TANKANIM,	   
					TRUCKANIM,	   
					TRAINANIM,	   
					TROOPANIM,	   
					BRIDGEANIM,	   
					LAUNCHERANIM,  
					CLONEANIM,	   
					FLAREANIM,	   
					THUGANIM,	   
					MISSILEANIM,   
					DEBRISANIM,	   
					SMOKETRAILANIM,
					PARACHUTEANIM,
					BIRDLAUNCHERANIM,							//RJS 13Dec99
					BLASTANIM,									//RJS 15Dec99
					BALLOONANIM,									//RJS 15Dec99
					WIREANIM,									//RJS 21Dec99
					AMBIENCEANIM,										//RJS 27Jun00
					TWOGUNANIM  										//RJS 27Jun00
					};								

//������������������������������������������������������������������������������
//������������������������������������������������������������������������������

enum	LnchrType	{LT_BULLET = 0, LT_BULLET_RAF = 0, LT_BULLET_LUF, LT_BULLET_REAR,
					 LT_CANNON = 0x10,
					 LT_BOMB = 0x20, LT_BOMB500 = 0x20, LT_BOMBPAIR500, LT_BOMB1000,LT_SMALLBOMB,LT_NAPALM,LT_BOMBB29,LT_BMB1B29,
					 LT_ROCKET = 0x30,LT_ROCKET4=0x30,LT_ROCKET6,LT_ROCKET8,
					 LT_WEAPON = 0x40,
					 LT_MOVEGUN = 0x70,
						LT_BRIT_CANNON = 0x70, LT_BRIT_BOFORS, LT_BRIT_3IN, LT_BRIT_3_7IN, LT_BRIT_4_5IN, LT_BRIT_PAC,
						LT_LUF_NOSE, LT_LUF_REARTOP, LT_LUF_REARBOT,
						LT_RAF_TURRET,
					 LT_SMKTRAILS = 0x80, LT_FUEL = 0x80, LT_VAPOUR, LT_CONTRAIL, LT_INTFUEL,
					 LT_DUST,LT_MUDDY,LT_JETDUST,LT_WINGVAP,LT_ENGINE, LT_FUNNEL,
					 LT_CONTACT = 0xd0, CT_WHLLEFT = 0xd0, CT_WHLRIGHT, CT_WHLFRONT,
					 CT_WHLBACK, CT_FRONT, CT_BACK, CT_LEFT, CT_RIGHT, CT_BELLY,
					 CT_EYE,CT_EYEGUN,CT_GRAVITY,CT_PILOT,CT_DUMMY,//RJS 06Dec99
					 LT_ANYWEAPON = 0xe0, LT_AWBULLET = 0xe0,LT_AWCANNON,//RJS 06Dec99
					 LT_MASK = 0xf0
					};											//RJS 01Oct98

enum	DialType	{DT_RPM = 0, DT_ALT = 1, DT_MPH = 2, DT_FUEL = 3};//RJS 16Sep96


//PD 17Apr96 - new a/c bits off code to cope with drawing dammaged
//				a/c sections

enum BitFlags {	BF_LBWing	=	0x00,
				BF_LTWing	=	0x02,
				BF_RBWing	=	0x04,
				BF_RTWing	=	0x06,
				BF_Tail		=	0x08,
				BF_Gear		=	0x0A,
				BF_Nose		=	0x0C,
				BF_TheRest	=	0x0E,							
				BF_Pilot	=	0x10,							
				BF_Engine	=	0x12,							
				BF_Fuel		=	0x14
			  };												//RJS 27Feb97

enum BitStates {BS_ALIVE=0,
				BS_DAMLV1=85,									//RJS 02Apr98
				BS_DAMLV2=170,									//RJS 02Apr98
				BS_DEAD = 255};

enum DamageTypes
{
	DMT_BULLET = 0,
	DMT_FIRE,
	DMT_SHOCKWAVE,												//RJS 06Dec98
	DMT_EXPLOSION												//RJS 06Dec98
};

enum	BitFlagsMan												//RJS 05Nov99
{																//RJS 05Nov99
	BF_LEFTARM = 0,												//RJS 05Nov99
	BF_RIGHTARM,												//RJS 05Nov99
	BF_LEFTLEG,													//RJS 05Nov99
	BF_RIGHTLEG,												//RJS 05Nov99
	BF_HEAD,													//RJS 05Nov99
	BF_ONFIRE,													//RJS 05Nov99
	BF_HELMET,													//RJS 05Nov99
	BF_PARACHUTE,												//RJS 22Dec99
	BF_GUN,														//RJS 22Dec99
	BF_TORSO													//RJS 22Dec99
};																//RJS 22Dec99



#pragma pack(pop)
#endif
