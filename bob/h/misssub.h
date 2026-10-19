//------------------------------------------------------------------------------
//Filename       misssub.h
//System
//Author         Jim Taylor
//Date           Thu 22 Aug 1996
//Description
//------------------------------------------------------------------------------
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	MISSSUB_Included
#define	MISSSUB_Included

#define	DEFAULT_MISSSUB 0

#include	"ranges.h"
//#include	"node.h"
#include "package.h"
#include "myangles.h"
enum	{
				MAXTNKWPINROUTE = 2

			};

enum					{LOWCLOUD = 0,CLOUDAT10000FT,BADWEATHER,BADWEATHER10000FT, MAXWEATHERNUM = 4};

enum	SpecialEventBit {SpecialEventMIN = 0,HOMEBASEDESTROYED = 0,
						PLAYERKILLED, RESERVEEXHAUSTED,
//above this you are dead
						PLAYERWOUNDED,RESERVES_2LEFT,RESERVES_6LEFT,
						ACEKILLED,NEARTARGET,
						FATALITYINSQUAD,WOUNDEDINSQUAD,
						MORALEEXCELLENT,MORALEGOOD,MORALEPOOR,MORALEROCKBOTTOM,
						BESTDAY,
//
						LIEUTENANTPROMOTION,CAPTAINPROMOTION,

//Spring Offensive training
						CRASHLAND,BENTGEAR,GOODLANDING,
						QUITTOOSOON,QUITTOOSOONSO_1,

						CAMPAIGNLOSTTAEJONOVERRUN,
						CAMPAIGNWONENTEREDPYONGYANG,
						CAMPAIGNLOST,
//awarded medals
						AWARDED_MEDALOFHONOUR,
//landing
						PLAYERLANDEDINCOUNTRY,PLAYERCAPTURED,	//RDH 29Nov96
//target hit
						ATTACKEDAIRFRIENDLY,KILLEDAIRFRIENDLY,KILLEDGROUNDFRIENDLY,
						ENEMYDESTROYEDTARGET,FRIENDLYDESTROYEDTARGET,
					  SpecialEventMAX};
//			In 3d Temp_Pilots morale

MAKEFIELD (SpecialEventBit,SpecialEventMIN,SpecialEventMAX);


enum	NewsEvent {NewsEventMIN = 0, NEWSWOUNEDRECOVERED, NEWSACEIN,
									 NEWSACEOUT,NEWSMISSION,NEWSMERITPROMOTION,
									 NEWSDEADMANPROMOTION,NEWSTHEPROMOTION,
									NEWSWOUNDEDMANPROMOTION,	//DAW 23Aug96
									//Spring Offensive: go operational
									 NEWSOPERATIONALGOOD,NEWSOPERATIONALBAD,
//last mission
						NEWSLASTMISSIONOFCAMPAIGN,
//updates
						NEWSCRCUPDATE,
						NEWS_HAT_25_GOOD,
						NEWS_HAT_25_BAD,
						NEWS_HAT_50_GOOD,
						NEWS_HAT_50_BAD,
						NEWS_HAT_75_GOOD,
						NEWS_HAT_75_BAD,
						NEWS_SPRING_25_GOOD,
						NEWS_SPRING_25_BAD,
						NEWS_SPRING_50_GOOD,
						NEWS_SPRING_50_BAD,
						NEWS_SPRING_75_GOOD,
						NEWS_SPRING_75_BAD,



									//Spring Offensive: lastday
									NEWSSTRONGPOSITION,NEWSWEAKPOSITION,NEWSAVERAGEPOSITION,
									NEWSBESTDAY,				//DAW 23Aug96
										NewsEventMAX};


MAKEFIELD (NewsEvent,NewsEventMIN,NewsEventMAX);



enum	pilotstatus	{NOTFLYING = 0,FLYING, WOUNDED, MIA, IS_DEAD = MIA, KIA};
//moved to flyinit.h											//RDH 23Sep96


enum 	RankType {SECLIEUTENANT= 0,SECLIEUTENANT_EXPERT,LIEUTENANT,CAPTAIN,MAJOR, RANK_INC = 1};
enum					{MAXCONVOYS = 12,MAXTANKCONVOYS = 8,MAXTRUCKCONVOYS = 6,MAXROUTES = 4,MAXGUNS = 16};

enum	HatMissionFamily {HP_VOLUNTEER = 0,
			HP_NOCHOICE = 0, HP_BALLOON,HP_PATROL,HP_ESCORT,HP_ROVING};

enum	SquadPanelSelection {PERSONNEL = 0, CONFIGURE, PREFERENCES, PAINTSHOP,SAVEGAME};
enum	ConfigureSelection	{SAVECONFIGURE,RETURNFROMCONFIGURE,CANCELCONFIGURE,GOTOJOYSTICKCONFIG,
								CANCELSCRNCONFIG,ACCEPTSCREENCONFIG,TRYSELECTEDSCREENCONFIG

								};
enum	CtrlVar	{
								GOTOACCEL,GOTOMAP
				};
enum	FlightInfoSelection { FLYNOW= 0, JUSTRETURN};

enum	{
//		MAXCIRCUSMISSIONS 		= 21,
//		MAXHATMISSIONS 			= 21,
//		MAXSPRINGMISSIONS 		= 15,							//RDH 23Nov96
//		MAXTANKMISSIONS 		= 16,

//		CIRCUSPLAYERSLOT 		= 0,
//		HATPLAYERSLOT 			= 11,

//		CIRCUSTARGETKILL 		= 52,
		RADIOMESSAGELEN			= 64, //AMM 21/02/00
		PASSWORDNAMELEN			= 11,
		SESSIONNAMELEN			= 32,
		MAXPLAYERS				=8,
		PILOTS_PER_SQUADRON 	= 24,
		PILOTABLE_SQUADRONS		=	5,
		PILOTS_PER_CAMPAIGN 	= (int)PILOTS_PER_SQUADRON* (int)PILOTABLE_SQUADRONS,
		PILOTACTIVE 			= 0,
		PILOTDEAD 				= -1,
		PILOTS_IN_RESERVE 		= 80,
		ACESLIST 				= 0,
		CASUALTYLIST 			= 20,		//(int)ACESLIST + (int)PILOTS_IN_RESERVE,
		FLIGHTSPERSQUADRON	 	= 4,
		FLYINGFLIGHTSPERSQUADRON= 32,//(int)FLIGHTSPERSQUADRON - 1,//Jim 30 jun 98

		WRAPMIGSQUADRONS		=4,
		MAXMIGSQUADRONS			=12,
//		MAXIMAGEMAPSPERAIRCRAFT = 8,
		};

struct	MissionType;
typedef	MissionType* DayMission [3];
typedef	MissionType* SingleMission;

typedef	MissionType* MissionFamily [6];
typedef MissionFamily MissionArea [5];

extern	SingleMission Spring51_Missions [];
extern	SingleMission PeaceTalk_Missions [];
extern	SingleMission Intro_Missions [];
extern	SingleMission Scramble_Missions [];						//RDH 02Aug96
extern	SingleMission H2H_Missions [];						//RDH 02Aug96
extern 	MissionArea Hat_Missions [2];

struct FormationPos
{
	SLong	range;
	ANGLESInst	bearing;
	SLong	delta_alt;
};

	enum					{MAXFLIGHT_FORMS_NUM = 4};
	enum					{MAXAC_FORMATIONS_NUM = 4,MAXAC_EXTENDEDFORMATIONS_NUM=10};
struct FormationType
{
	int		numentries;
//	TextRef			name;
#pragma warning( disable : 4200)
	FormationPos	wingpos[40];
#pragma warning( default : 4200)
	int	GetPerFl(int totalac)
	{	int fl,fl2,fln=numentries;
		if (fln<0)	return -fln;
		if (fln<10) return fln;
		//The logic here is that these are a number of digits.
		//Above the highest digit is a zero digit which indicates the end of the list
		//Above that may be a 2 digit to indicate that pairs are handled by this movecode. This is currently not tested for. It is a debug
		//As we strip off each digit,
		//	If the next digit is a zero we must accept the current digit
		//	If the current digit is a divisor of the totalac then we want the current digit
		for (fl=fln%10,fl2=fln/10,fln=fl2%10,fl2=fl2/10; fln; fl=fln,fln=fl2%10,fl2=fl2/10)		//JIM 9Aug00
			breakif (totalac%fl==0);
		return	fl;
	}
};

struct	ExtFormationType
{
	FormationType formationtype;
//	FormationPos	extrawingpos[MAXAC_EXTENDEDFORMATIONS_NUM];

};

struct	WaypointOffsetSub
{
	int	deltatime,	//up to 15 mins (in cs = 90000)
		deltahori,	//up to 4km		(in cm = 400000)
		deltavert;	//up to 10,000ft(in cm = 300000)
};
struct	WayPointOffset
{

	WaypointOffsetSub	members[16];	//LEADER GETS AN ENTRY;
};

typedef FormationType* SingleFormation;
extern SingleFormation Aircraft_Formations [];	//Index by FORMTYPE_WING
extern SingleFormation Squadron_Formations [];	//Index by FORMTYPE_SQUAD+FORMTYPE_WING
extern SingleFormation Escort_Formations [];	//Index by FORMTYPE_INTERCHILD+FORMTYPE_SQUAD
extern	ExtFormationType	TakeOff_FormationWing, TakeOff_FormationSquad,	TakeOff_RefuelSpots;
extern	WayPointOffset		WayPointOffsets_Escort,*WayPointOffsets_CourseSel[];
extern SingleFormation Ground_Formations [];	//Index by FORMTYPE_WING
//#define MAXPLAYERS 12


struct 	PilotName	{ UWord nameref;char initial1,initial2; };

struct PilotHist
{
	typedef PilotName Name;
	Name	name;
	SWord	kills;
	pilotstatus	status;
	RankType	rank;
	SLong		lastdate;
};

//structure redefined 11/8/98 rdh
enum	HideSkills
{
	HSflyingskill=0,
	HScombatskill=8,
	HSflyingpotential=16,
	HScombatpotential=24,
	HStoskill=4,
	HSmask=255,
	HSskillmask=15,
	HSallskills=0x01010101
};










//MARK, this is the list of events in the order of use			//MS 04Mar99


enum	AccelSwitches	{	AS_NOTSWITCH,
							AS_RAID_FORMING,
							AS_TAKEOFF,
							AS_COAST,
							AS_SMALL_ENGAGEMENT,
							AS_LARGE_ENGAGEMENT,
							AS_TARG,
							AS_LOSSES,
							AS_LAND,
							AS_MAX};
MAKEFIELD(AccelSwitches,0,AS_MAX);

enum	HostileListFilters	{
							HL_FORMING,
							HL_INCOMING,
							HL_TARGETAREA,
							HL_DETAILRAID,
							HL_ENEMYSIGHTED,
							HL_ENEMYNOTSIGHTED,
							HL_OUTGOING,
							HL_NORESPONSE,
							HL_RESPONSES,
							HL_MAX
								};

MAKEFIELD(HostileListFilters,0,HL_MAX);

enum	SelectTargetMode	{STM_NORMAL,STM_LWAC,STM_RAFAC,STM_GROUND};

//////////////////////////////////////////////////////////////////////
//
// Function:    Directives
// Date:		28/01/00
// Author:		JIM
//
//Description: 	BOB directives
//				There are 4 sperate sections:
//					DirLW		LW	directives
//						various fields
//						One main set
//						Contains 1 instance of MissLW, perhaps
//						TargLW	Rule per target class
//						Additional sets for historical strategies
//					DirRAF		RAF directives
//						One main set
//						Additional sets for historical strategies
//					MissLW		LW	missions
//						patrol/recon/attack sets
//						historical sets
//					MissRAF		RAF missions
//						patrol/<1sq/1grupp/<=4grupp/>4grupp
//						historical sets
//
//////////////////////////////////////////////////////////////////////
//struct	Directives ;
#ifdef	PACKAGE_INCLUDED
#include "flyinit.h"
#define	DIRECTIVES_DEFINED

struct	Directives
{
	typedef	Profile::AttackMethod	AttackMethod;
	typedef	Profile::BetterRule	BetterRule;
	struct	LW	//LUFTWAFFE SPECIFIC DIRECTIVES
	{
		enum	MissTypes {PATROL,RECON,ATTACK,MAXMISSTYPES};
		enum	HistTypes {LWHISTORICALPHASE1A, LWHISTORICALPHASE1B,LWHISTORICALPHASE1C,LWHISTORICALPHASE1D,
							LWHISTORICALPHASE2A, LWHISTORICALPHASE2B, LWHISTORICALPHASE2C, LWHISTORICALPHASE2D,
							LWHISTORICALPHASE3A, LWHISTORICALPHASE3B, LWHISTORICALPHASE3C, LWHISTORICALPHASE3D,
							LWHISTORICALPHASE4A, LWHISTORICALPHASE4B, LWHISTORICALPHASE4C, LWHISTORICALPHASE4D,
							MAXHISTTYPES};
		enum	Ju87Usage	{JU_NONE,JU_AM,JU_MD,JU_PM,JU_ANY};
		MAKEFIELD(AttackMethod,Profile::AM_FIELD_MIN,Profile::AM_FIELD_MAX);

		struct	Targ	//Describes one target category
		{
			enum	Categories	//Note I have broughts non-bombing ones together
			{RECON,AIRFIELDS,DOCKS,RADARS,CONVOYS,LONDON,FACTORIES,SWEEPSNDECOYS,RESTING,MAXTARGTYPES};

			float	attached,detached;	//remainder from 1.0 are return - same order as Directives::LW::Miss
			enum	Flags {NONE = 0, FIGHTERSSTRAFFE=1};
			UByte	flags;
			UByte	secondarytargets;
			UWord	bombersperraid;	//Less than 10 A/C gets that number of A/C
									//>=10 gives bpr/10 staffel
									//>=30 gives bpr/30 gruppe
			float acproportions[PT_GERMCOUNT];
			ULong	lastdateandtimeused;	//used to decide which is highest priority recon
			float&	operator[](int ind)	{return acproportions[ind-PT_GER_FLYABLE];}
		};

		struct	Miss	//Describes one LW Mission profile
		{
			AttackMethodField	escortmethods;
			ONLYFIELD(UWord,BetterRule,			acselectionrules);		//And straffing
			ONLYFIELD(UWord,FormationTypeIndex,	formations)[4];			//1 per "page",bomber, attached,detached, return
			ONLYFIELD(UByte,AttackMethod,		bomberattackmethod);
			SWord		bombers,attfighters,detfighters,retfighters;	//generate proportions at display time
			UByte		NumTargets;
			ULong		mainalt;	//in cm

		};
		struct Reconn
		{
			UByte	missions;	//spin
			UByte	ac;			//spin 1-3
			bool	escort;
			bool	morningonly;
			int		escortactype;
		};
		struct	Dir		//Describes the directives settings
		{
			Targ	targetlines[Targ::MAXTARGTYPES];
			Reconn	reconn;
			float	morningquota,middayquota;	//remainder from 1.0 are eveningquota
			AttackMethodField	escortmethods;
			int		arrivalminsseperation;	//2=coordinated;15 is consecutive.
			Ju87Usage	ju87usage;
			float		escortproportion;
		};

		struct	OneDirResult	//Describes one resultant mission
		{	enum	{MAXTARGETS=7};					//I think we would want to put patrols on a seperate page
			Miss	mission;
			UniqueID	targets[MAXTARGETS];	//1+MAXSECONDARIES+1				//JIM 14Aug00
			int			ETA;
			int*		usesquadsavail;
			int			squadsavail[PT_VEHICLES];
		};

		Miss	userprofiles[MAXMISSTYPES];
		Dir		current;
		Dir		histdir[MAXHISTTYPES];
//TEMPCODE JIM 28/01/00 		Miss	histmiss[Miss::MAXHIST];
		static void	AI();
		static void	SetUpReconn();


	}
	lw;


	struct	RAF	//RAF SPECIFIC DIRECTIVES
	{
		enum	MissTypes {PATROL,ANTISWEEP,ANTISINGLE,ANTIUNESC,ANTIESC,ANTIBIG,MAXMISSTYPES};
		enum	MissTypesBits {MTB_PATROL=1,MTB_ANTISWEEP=2,MTB_ANTISINGLE=4,MTB_ANTIUNESC=8,MTB_ANTIESC=16,MTB_ANTIBIG=32,MTB_MAXMISSTYPES=64,MTB_ANTIALL=64-2};
		enum	HistTypes {HISTORICALSCENARIO_PHASE1,
							HISTORICALSCENARIO_PHASE2,
							HISTORICALSCENARIO_PHASE3,
							HISTORICALSCENARIO_PHASE4,
							HISTORICALSCENARIO_A,
							HISTORICALSCENARIO_B,
							HISTORICALSCENARIO_C,
							HISTORICALSCENARIO_D,
							HISTORICALSCENARIO_E,
							HISTORICALSCENARIO_F,
							HISTORICALSCENARIO_G,
							HISTORICALSCENARIO_H,
							MAXHISTTYPES};
		MAKEFIELD(MissTypes,PATROL,MAXMISSTYPES);

		struct	Miss	//Describes one response profile
		{
			ONLYFIELD(UWord,BetterRule,			acselectionrules);		//And straffing
			ONLYFIELD(UByte,Profile::AttackMethod,	packageattackmethod);	//tactics
			int	mainalt;
			struct	PerType
			{
				SByte				numac;		//div 12 to get squads	or use -ve numbers
				ONLYFIELD(UByte,Profile::SubMethod,		what);	//avoid bombers/fighters
				ONLYFIELD(UWord,FormationTypeIndex,		formation);
			};
			PerType	hurr,spit;
		};
		struct	Dir		//Describes the RAF directives
		{
			MissTypesField	sanctioned;
			enum  GeneralSize	{ONESQ,PAIRED,BIGW};	//I guess these are patrol sizes
			ONLYFIELD(UByte,AttackMethod,	approach);	//tactics is not on directives????

			int		restifcatbelow,restifavailbelow;

			//The squads not resting are partitioned so the following add up to 1.0 each

			float	gr11radarpatrol,gr11convoypatrol,gr11coastalpatrol;	//remainder from 1.0 can intercept
			float	gr1012afpat,gr1012acfactpat,gr1012supfactpat;		//remainder from 1.0 can intercept

			//The squads on intercept are applied using the following rules

			float	convoylevel;			//1.0 is matched
			bool	interceptbeforetarget;	//
			GeneralSize	unitsize;

		};
		struct	DirResult	//Describes one patrol mission profile
		{					//We could list the 10/12 and 11 patrols on seperate pages
			Miss		mission;
			UniqueID	target;
			Dir::GeneralSize	unitsize;
			SquadNum	squadrons[6];
			int			ETA;
		};					//We could list rested and intercept ready squads on further pages
		Miss	userprofiles[MAXMISSTYPES];
		Dir		current;
		Dir		histdir[MAXHISTTYPES];
		static void	SessionAI();
		static void	AI();
		static void	ModifyPatrols();


//TEMPCODE JIM 28/01/00 		Miss	histmiss[Miss::MAXHIST];
	}	raf;


};
int	Todays_Packages_NewPackage(UniqueID targ,Directives::RAF::Miss& missiondesc,int targetalt=0);//,int* actypecounts=NULL);
int	Todays_Packages_NewPackage(UniqueID targ,Directives::LW::Miss& missiondesc,UniqueID* secondarytrg= NULL, bool Gruppe = false,int* actypecounts=NULL);

struct	LWDirectivesResults
{
	struct	TargetList
	{
  	UniqueID	target;
	int			priority;
	};
	enum	{MAX_TARGETS = 300, MAX_RESULTS =100};

	TargetList target[MAX_TARGETS];

		struct FighterSplit
	{
		float ret;
		float att;
		float det;
	};
			struct	Targets
	{
		TargetList target[MAX_TARGETS];
	};

	Targets trgarray[8];
	int reconntrg[4];

	FighterSplit	otherraids;
	FighterSplit	firstraids;


	Directives::LW::OneDirResult	dirresults[MAX_RESULTS];
	Directives::LW::Dir		tempdir;
	float LWGrAvail[PT_GERMCOUNT];

	static int	InsertTarget(TargetList* target, UniqueID uid, int currtrg, int uidbandstart);
	static int TargetX(int i);

	static int	FillTargetList(TargetList* target,int uidbandstart, int uidbandend, int trgtype);
	static	void FillDirectivesResults(LWDirectivesResults* dr);
	static int	FillReconnDirectivesLine(LWDirectivesResults* dr, int k);
	static	int	HowManyReconEscortRequiredThisPeriod(LWDirectivesResults* dr);
	static	int	 FillOneDirectivesLine(LWDirectivesResults* dr, int i, int k, int& missions);
	static 	void FindAcAvail(LWDirectivesResults* dr);
	static int FindMaxSquads(LWDirectivesResults* dr);
	static	void SetLWDirectivesResults();
	static	void FillTargetLists(LWDirectivesResults* d);
	static	void MakeLWPackages(LWDirectivesResults* dr, bool manual);
	static	void AutoLWPackages();
	static	void LaunchDirectiveMissions();
	static  void SetUpLWDirectives(Directives::LW::HistTypes dirtype);
 	static Directives::LW::MissTypes  LWMissType();


};
struct	RAFDirectivesResults
{
	struct	AvailSquads
	{

		int gr11radarpatrol;
		int gr11convoypatrol;
		int gr11coastalpatrol;
		int gr11intercept;
		int gr11total;
		int	gr11maxtotal;

		int gr1012afpat;
		int	gr1012acfactpat;
		int gr1012supfactpat;
		int gr1012intercept;
		int gr1012total;
		int	gr1012maxtotal;

	};
	AvailSquads	availsquads;
	struct	TargetList
	{
  	UniqueID	target;
	int			priority;
	};
	enum	{MAX_TARGETS = 300, MAX_RESULTS =100};
	enum	{MAX_AF = 100};
	struct	AirField
	{
		UniqueID uid;
		int		priority;

	};


	TargetList target[MAX_TARGETS];
	enum	{COASTAL, SOUTHLONDON, NORTHLONDON};
	int oldfrontlinefields;
	int newfrontlinefields;
	static 	int	FindAfUsage();


	Directives::RAF::DirResult	dirresults[MAX_RESULTS];
	Directives::RAF::Dir		tempdir;
	static void	FillDirectivesResults(RAFDirectivesResults* dr);
	static void	FindSquadsAvail(RAFDirectivesResults* dr);
	static void SetDirectivesResults(RAFDirectivesResults* dr);
	static int	SetAfPatrols(RAFDirectivesResults* dr, int patrols, int k);
	static int	InsertTarget(TargetList* target, UniqueID uid, int currtrg);
	static void	SetAfPatrolTargets(RAFDirectivesResults* dr);
	static	void	SetFactPatrolTargets(RAFDirectivesResults* dr, bool acfact);
	static int SetPatrols(RAFDirectivesResults* dr, int patrols, int k, Profile::BetterRule br);
	static void	SetStdPatrolTargets(RAFDirectivesResults* dr,int uidbandstart, int uidbandend,bool patrolifalive);
	static	void MakeRAFPatrols(RAFDirectivesResults* dr);
	static	void AutoRAFPatrols();
	static	void SetRAFIntercept(int pack, int raid,bool afteregress);
	static	void RaidAcNumbers(int pack,int raid, int& strike, int& detachedescort, int& attachedescort, int& returnescort);
	void 	MoveSquadronswrtFrontLine();
	static	int FindAirfield(int priority, int maxaf, AirField* Airfields);


	static void MoveSquadron(int sqnum,  int loc0, int loc1, int loc2);
	static	void	SetUpRAFDirectives(Directives::RAF::HistTypes dirtype);
	static  int	InterceptSanctioned(int pack, int raid);
   	static Directives::RAF::MissTypes  RAFMissType(int packnum);
	static Directives::RAF::MissTypes  RAFInterceptType(int lwpack, int sqnum);


};

struct	PeriodReviewBase
{
	enum	ReviewType {KILL0,KILL1,KILL2,AVAILSTART,DAMAGE,REPAIR,DESTROY,CREATE,TOTALLOSSES, SORTIES, NEWREPAIRS,POTENTIALREPAIRS,REPAIRSINSYSTEM};
	struct	TypeStats
	{

		UWord	totalkills[3],	//Set for SAGs on map //spit/hurr or bomber/109/110

				availablestart,
				damaged,		//Set in Squad::ApplyFlyingLosses
				repaired,		//Set in AircraftRepairs4Hour
				destroyed,		//Set in Squad::ApplyFlyingLosses
				created,		//Set in DeliverProductionRates
				totallosses,
				sorties,
				newrepairs,
				potentialrepairs,
				repairsinsystem
				;
	};
	TypeStats	raf[2],lw[6];
};

struct	LastWeekReview:PeriodReviewBase
{
	int	lwtodaylosses,lwrecordlosses;
	int	raftodaylosses,rafrecordlosses;
	int	acproductiondelta[2],dayssincedeltapos[2];
	float	rafloss[2],lwloss[5];

	int		Note(PlaneTypeSelect scorer,ReviewType field,int delta);
	void	SetNote(PlaneTypeSelect scorer,ReviewType field,int value);
	int		Note(PlaneTypeSelect scorer,PlaneTypeSelect target,int delta);
	void	ClearToday();
	float& DecayingLossRate(PlaneTypeSelect	actype);

};
struct	PeriodReview:PeriodReviewBase
{
	void operator=(LastWeekReview* lwr)
	{ *(PeriodReviewBase*)this=*(PeriodReviewBase*)lwr;	}
};


#endif

#pragma pack(pop)
#endif
