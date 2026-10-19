//;;------------------------------------------------------------------------------
//;;Filename       uniqueid.g
//;;System         
//;;Author         Jim Taylor
//;;Date           Mon 27 Nov 1995
//;;Description    Unique ID equates. 
//;;				Basic bands input and specific item UIDs output
//		NOTE:	/**/ indicates that external parsers will ignore the rest of the line!
//;;------------------------------------------------------------------------------
//;;This bit is manually editable:

#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#ifndef	UNIQUEIDBAND_Included
#define	UNIQUEIDBAND_Included
enum	UniqueIDBand
												
	{						 									//MS 14Oct98
		PilotedAcBAND			=	0x0000,	  /**/ NULL_BAND=0x0000,

		WayPointBAND			=	0x0100,			
		MissileBAND				=	0x0B00,	/**/	WayPointBANDEND = MissileBAND,
		LandscapeBAND			=	0x0E00,
		SagBAND					=	0x1200,	//Other tables in game rely on size<=256 (MAX_SAGINVALIDATES)
		RealAcBAND				=	0x1300,	/**/	SagBANDEND=RealAcBAND,
		//space here at 1180
		BritAAASiteBAND			=	0x1700,	/**/	RealAcBANDEND=BritAAASiteBAND,
		GermAAASiteBAND			=	0x1800,
		BritAAALargeBAND		=	0x1900,
  		CivilianBAND			=	0x1A00,	
		BritBridgeBAND			=	0x2800,
		BritHistoricBAND		=	0x2880,
		GermMobileBAND			=	0x2900,
		BritMobileBAND			=	0x2980,
	  	NeutMiscBAND			=	0x2A00,
		BritBoatBAND			=	0x2A80,	
		GermBoatBAND			=	0x2B00,
		PowerStationBAND		=	0x2B80,
		OilRefineryBAND			=	0x2C00,
		IndustryBAND			=	0x2C80,
		AircraftFactoryBAND		=	0x2D00,
		DocksBAND				= 	0x2D80,
		PortBAND				=	0x2E00,
		NavalBaseBAND			=	0x2E80,
		CityBAND				=	0x2F00,
		TownBAND				= 	0x3000,
		VillageBAND				=	0x3100,
		BarrageBalloonBAND		=	0x3200,						//MS 10Feb00
		RAF_FighterAFBAND		=	0x3300,//0x3200,	
		RAF_BomberAFBAND		=	0x3380,//0x3280,		
		RAF_CoastalAFBAND		=	0x3400,//0x3300,
		CLRadarBAND				=	0x3460,//0x3380,
		CHRadarBAND				=	0x3470,//0x3400,
		LUF_FighterLF2BAND		=	0x3480,	 //this band has all LW airfields
		LUF_FighterLF3BAND		=	0x3500,	//empty
		LUF_BomberLF2BAND		=	0x3580,	//empty
		LUF_BomberLF3BAND		=	0x3600,	/**/	 LUF_BomberBANDEND,	 //empty
		RefuellerBAND			=	0x3680,	//Loads of space here!!!
		TaxiBAND				=	0x3880,	/**/	RefuellerBANDEND=TaxiBAND,
		SectorOpsBAND			=	0x38D0,	//38ff=tangmere CC
		AfHangarBAND			=	0x3900,	//none//	RunwayEBANDEND=AfHangarBAND, //DAW 16/04/99
		AfStaffBAND				=	0x3910,	//44 instances
		AfStoresBAND			=	0x3940,	//44
		AfFieldBAND				=	0x3970,	//44
		AfNoScoreBAND			=	0x39A0,	//none//	AfRunwayBANDEND=AfNoScoreBAND,
		EventBAND				=	0x39B0,
		RunwaySBAND				=	0x3A80,	//220
		RunwayEBAND				=	0x3B80,	//173
		BlokeBAND				=	0x3C40,	/**/	RunwayEBANDEND=BlokeBAND,	//RJS 11Aug00
		EmergencyVehicleBAND	=	0x3D40,						//RJS 14Jan00
		NavigationBAND			= 	0x3E80, //3eec=Sheerness	//RJS 14Jan00
		NextBAND				=	0x3F00,						//RJS 14Jan00
		LastBAND				=	0x3f00,
		IllegalBAND				=	0x3fff,
	};
#endif
//;;This bit is auto-generated:	
//This bit is auto-generated:	NOT ANY MORE!!!
#ifndef	UNIQUEIDVALS_Included
#define	UNIQUEIDVALS_Included

enum	UniqueID
	{
		#include	"uidvals.g"
		UID_Null				=	0x000,
		UID_NULL				=	0x000,
		PilotedSepID			=	0x000,
		IllegalSepID			=	0x3fff,
		UID_BIT15				=	0x8000,
		UID_BIT14				=	0x4000,
		AbsolutingSepID			=	0xffff
	};
//;;------------------------------------------------------------------------------
//inline UniqueIDBand	operator+(UniqueID u) {return (UniqueIDBand) u;}
//inline UniqueID	operator+(UniqueIDBand u) {return (UniqueID) u;}
//------------------------------------------------------------------------------


#pragma pack(pop)
#endif
