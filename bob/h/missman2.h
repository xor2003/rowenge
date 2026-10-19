//------------------------------------------------------------------------------
//Filename       missman2.h
//System         
//Author         Jim Taylor
//Date           Thu 22 Aug 1996
//Description    
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	MISSMAN2_Included
#define	MISSMAN2_Included

#define	DEFAULT_MISSMAN2 0

//this means that I dont want any common files							//RDH 18/05/00
#define	f_common

#define		F_SOUNDS
#define		F_COMMON
#include	"Files.g"
#include	"myangles.h"											//RDH 20Dec95
#include	"savegame.h"
#include	"worldinc.h"										//RDH 11Aug96
//#ifdef	PACKAGE_INCLUDED
#include	"sky.h"

//#endif //package_included

#ifdef	DEFAULT_MISS_MAN
//
// If the module is declared a second time, then this alternate construction 
// is made to allow instances to occur.
//
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

#else
//
// This definition defines substructures by simple inheritance
// That will not allow initialisation.
//
	#define	basestruct(subname)							\
	struct	subname					{

	#define	substruct(subname,parentname)				\
	struct	subname : parentname	{

	#define	endstruct(subname)		}

#endif

#define	DEFAULT_MISS_MAN 0
struct	MissionType;
struct	PilotType;
struct	PeriodReview;																//DAW 23Aug96
#ifndef	MISS_MAN_Included2										//DAW 23Aug96

#include "bitfield.h"											//DAW 23Aug96
#include	"misssub.h"

#endif
//////////////////////////
//
// This is a far better subdivision of fields
// in order to manage a copy of the active campaign
//
//////////////////////////
#if defined(PACKAGE_INCLUDED) //&& defined (DIRECTIVES_DEFINED)
basestruct	(CampaignBase)
	int	garbage;
	FileNum	introsmack;
	FileNum	introquarterart;
endstruct(CampaignBase);
//////////////////////////
substruct	(CampaignPtrs,CampaignBase)
//	MissionType*	currmission;
//	MissionType*	dfltmission;

endstruct(CampaignPtrs);
//////////////////////////

substruct	(CampaignStatic,CampaignPtrs)
	UniqueID		playerhomebase;
	UniqueID		playertakeoff;
	bool			firsttime;
	SLong			startdate;
	SLong			ripdate;
	SLong			currdate;
	SLong			lastdate;
	SLong			currtime;
	SLong			currperiodtime;
	SLong			dawntime;
	SLong			dusktime;
	SLong			sunsouth;
	UByte			sl_category;		//squadlist 

//	SWord			weathernum;											//RDH 02Oct96
//	struct			Wind {
//						SWord	speed;
//						SWord	velx;
//						SWord	velz;
//						ANGLESInst	direction;			//direction from which the wind comes
//						}	wind;

	RankType		playerrank;
	SkillType		newpilotskill;
	SWord			frontdeltastrength,
					reservesavail;
	SWord			currentplayerslot;
	char	PlayerName[PLAYERNAMELEN];
	char	aircraftname[20];
	UniqueID	InitialFront[3];
//	FileNum	mainbfield;
	Directives::LW::HistTypes	lwdir;
	Directives::RAF::HistTypes	rafdir;
	FileNum			camp_win_anim;
	FileNum			camp_lost_anim;
endstruct(CampaignStatic);
//////////////////////////
substruct	(CampaignBlueSquad,CampaignStatic)
	Directives		directives;

endstruct(CampaignBlueSquad);
//////////////////////////
substruct	(CampaignSquad,CampaignBlueSquad)


endstruct(CampaignSquad);
//////////////////////////
substruct	(CampaignPackages,CampaignSquad)
//	Package package[MAX_PACKAGE_NUM];
enum					{MAX_MISSIONPERCAMPAIGN = 8};


endstruct(CampaignPackages);
//////////////////////////
substruct	(CampaignPilots,CampaignPackages)
endstruct(CampaignPilots);
//////////////////////////
//DEADsubstruct	(CampaignFiles,CampaignPilotsOnly)
//DEAD
//DEAD	TextSnip0	objecttext;
//DEAD	TextSnip0	introtext;
//DEAD	TextSnip0	editnametext;
//DEAD	TextSnip0	editnametitle;
//DEAD
//DEADendstruct(CampaignFiles);
//////////////////////////
//DEADsubstruct	(CampaignPilots,CampaignFiles)
//DEAD	PilotType	Reserve_Pilots[PILOTS_IN_RESERVE];
//DEADendstruct(CampaignPilots);
//DEAD//////////////////////////
//DEADsubstruct	(CampaignTank,CampaignPilots)
//DEAD	SLong		Tank_timer;
//DEAD	SLong		Truck_timer;
//DEADendstruct(CampaignTank);
//DEAD//////////////////////////
//DEADsubstruct	(CampaignTankZero,CampaignTank)
//DEAD	int			blueacflying;
//DEAD	UniqueID	blueguns[MAXGUNS];
//DEAD	UniqueID	blueaaa[MAXGUNS];
//DEAD	TnkMapWpSet	tankcampwp[MAXTNKWPINROUTE];							//RDH 08Oct96
//DEAD	struct Convoy {
//DEAD		UniqueID	uid;
//DEAD 		FileNum		bfield;
//DEAD		SWord		size;
//DEAD
//DEAD		};	Convoy convoys[MAXCONVOYS];
//DEAD
//DEADendstruct(CampaignTankZero);
//////////////////////////
#undef	basestruct
#undef	substruct
#undef	endstruct

#ifndef	MISS_MAN_Included2
#define	MISS_MAN_Included2

struct	CampaignZPilots:CampaignPilots
{
//DEAD	PilotType		Wounded_Pilots[CASUALTYLIST],
//DEAD					Dead_Pilots[CASUALTYLIST];
//DEAD	SquadronInfo	Temp_Squadron;
//DEAD	PilotType		Temp_Pilots[PILOTS_PER_CAMPAIGN];
  PilotHist		dead_pilots[CASUALTYLIST];
};

struct	CampaignZero:CampaignZPilots
{
//Dead	WpSetType		players_wpset;
//Dead	SLong			TrainingCount;
//Dead	SLong			TrainingRange;
//Dead	SWord			currmission_no;
//Dead	SWord			missionwpset;
	PeriodReview	firstweek,prevweeks[6];
	LastWeekReview	thisweekreview;
	int				earliestquit3dtimecs;
	SWord			missionarea;
	MeritType		playermerit;
	SWord			killsonbestday;
	SWord			currmissionfamily;
	SWord			dayssincetransfer;
	SWord			nextdeadpilot;
	SWord			playersquadron,playeracnum;
	int				curr_score;				//used in mission scoring //RDH 19/03/99
	int				curr_msn;				//used in mission scoring //RDH 19/03/99
	bool			wincamp;	
	FileNum			smacker;				//a smacker generated by last mission
	int				msgwithsmacker;				//a smacker generated by last mission //RDH 26/06/99
	UByte			sl_group;				//squad list setup
	UByte			sl_ac;
	UByte			sl_flotte;				//squad list setup
	UByte			sl_geschwadertype;
	UByte			sl_status;
	UByte			sl_min_ac;
	UByte			sl_min_pilot;			//squad list setup
	SLong			curraccelrate;
	SLong			accumulator;
	bool			directivespopup;
	bool			autoscramble;
	enum	AccelType	{ACCEL_PAUSED,ACCEL_NORMAL,
				ACCEL_NONRAIDSPD,ACCEL_RECONNSPD,ACCEL_RAIDSPD,ACCEL_DIALOGSPEED, 
				ACCEL_MAX};
	AccelType		curracceltype;
	SLong			accelrates[ACCEL_MAX];
	HostileListFiltersField	hostilefilters;
	struct Fav
	{
		AccelSwitchesField	decelon,flyon, flyonfav;
		int				group;
		int				ac;
		int				sector;
		int				squadron;
		int				forcesquadron;		//go to frag without dialog request	//RDH 27/03/00
		int				flotte;
		int				geschwadertype;
		int				geschwader;
		int				gruppe;
		int				forcegruppe;		//go to frag without dialog request	//RDH 27/03/00		

	}	fav;
//mission folder flags
	bool			rafplanned;
	bool			rafoutgoing;
	bool			rafengaged;
	bool			rafincoming;
	bool			raflanded;
	bool			rafpatrol;
	bool			rafintercept;
	bool			lwplanned;
	bool			lwactive;
	bool			lwlanded;
	bool			lwstrike;
	bool			lwattachedescort;
	bool			lwdetachedescort;
	bool			lwreconn;
//intell flags
	bool		intel[6];//engagement;
//	bool		intelflight;
//	bool		intelmanagement;
//	bool		intelspotting;
//	bool		inteltargetstatus;
	bool		pacsuccessful;				//jt
	UByte		phasechanged;				//rdh	periods since change
//	UByte		dayssinceacincreased[2];	//jt
	UniqueID	historictargetuiddamaged;	//jt
	UniqueID	historictargetuiddestroyed;	//jt
//	UWord		lwtodayslosses;				//jt
//	UWord		raftodayslosses;			//jt
//	UWord		lwrecordlosses;				//rdh
//	UWord		rafrecordlosses;			//rdh
	SWord		undefendedconvoylosses;
	SWord		undefendedcriticaltargetlosses;
			


//Dead	SWord			lastrecoveredpilot;
//Dead	TextSnip0		lasttransferee;
//Dead	TextSnip0		lastdeadsuperior;
//Dead	TextSnip0		specialeventtext;							//RDH 06Sep96
//Dead	FileNum		playerbfield;
//Dead	TextPerm	playernameref;
	ATMOSPHERE		Sky;
 	char	CommsPassword[11];
	SWord	packageprevscores[10];

	void	SetCurrMission (MissionType*);
	bool	ChkEndCampaign();


	void	NextMission();
	void	NextDay();
	void	ReplenishAcStock(); //RDH 15/02/99
	void	RotateMiGSquadron(int squad);							  //RDH 09/03/99
	void	ChkRotateMiGSquadrons();								  //RDH 09/03/99
	void	ModifyMiGAggressionLevel(int squad);					  //RDH 09/03/99 //RDH 09/03/99
	void	SwapPilot(int groupnum, int src,int dest);	
	void	SwapElement(int groupnum, int src, int dest);
	void	SwapFlight(int groupnum, int src, int dest);
	void	RotateFlightsCheck();
	void	ModifySkill(int groupnum, int daysonduty);

	void	CampaignInit();
	void	InitIcons();
	void	UpdatePlayerLog();										  //RDH 19/06/99
	void	ZeroDisDay();
	FileNum	FindSmacker(int index);
	void	MakeNewPilot(int pilot);
	void	KillPilot(int pilot, pilotstatus status);
	void	MoveDeadPilots();
	void	SetOldPackScore(int packnum);
	bool	GetMissionSuccess(int packnum);
	enum	TimeColours	{TC_RED,TC_YELLOW,TC_BLUE,TC_OLD};
	static bool	AnyDialogsOpen();
	static void	GlobalRefreshData();
	TimeColours	TimeColour(int timetotest=0);	//


	//Set campaign up
	CampaignZero& operator = (CampaignBase	*);
	CampaignZero& operator = (CampaignZero&	);
	//Update existing campaign
	CampaignZero& operator += (CampaignBase	*);
	CampaignZero& operator += (CampaignZero&	);



};

struct	CampaignZero;
struct	CampaignBase;
typedef	CampaignZero	Campaign;



//------------------------------------------------------------------------------
class	 MissMan
{
public:
	static enum	CampaignName {
		INTROCAMPAIGN = 0,
		CONVOY_CAMPAIGN = 0,
		RADAR_CAMPAIGN,
		ARFIELD_CAMPAIGN,
		BLITZ_CAMPAIGN,
 		SCRAMBLECAMPAIGN,
		MINCAMPAIGNNUM = 0,
 		MAXCAMPAIGNNUM = 5,										//RDH 05Sep96
		} 	currcampaignnum;

	static CampaignBase	*campaigntable[MAXCAMPAIGNNUM];
	Campaign	camp;

};
extern	MissMan Miss_Man;
extern	Campaign&	MMC;		//This is to help the completion wizard
#define	MMC	Miss_Man.camp
//#pragma warnmsg()
#endif
//#pragma warnmsg()
#endif
//#pragma warnmsg()
class Atmosphere;
typedef Atmosphere ATMOSPHERE, *PATMOSPHERE;
	ATMOSPHERE&	MissManCampSky();
int	MissManDawnTime();
int	MissManDuskTime();
int	MissManSunSouth();


#pragma pack(pop)
#endif
