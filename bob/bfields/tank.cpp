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
//Filename       tank.cpp
//System         
//Author         R. Hyde 
//Date           Fri 9 Aug 1996
//Description    Code which sequences the loading of battlefields
//				Decides which file to load next and what globrefs need setting up.
//				This is basically a state engine loop
//				Originally invented to drive the dynamic tank campaign in FC
//------------------------------------------------------------------------------
//Questions
//
//
//	To reduce the number of UIDs needed for storing aircraft, and to ensure they are loaded monotonic increasing, so don't need specifying 
//	I reorder the sparse set of actual flying aircraft to use sequential numbers.
//	Some of the code in this file sees un-reordered targets and some sees reordered targets
//
#define F_GRAFIX
#define F_SOUNDS
#define F_COMMON

			#include	"dosdefs.h"
//TEMPCODE DAW 13/04/00 			#include	"text.h"
			#include	"persons2.h"
			#include	"savegame.h"
			#include	"missman2.h"
			#include	"enumbits.m"
			#include	"mymath.h"
			#include	"Mytime.h"			//CFont
			#include	"myerror.h"	
			#include	"node.h"
#define	QUICKQUICKQUICK
			#include	"squick1.h"
			#include	"globrefs.h"  
			#include	"stub3d.h"
			#include	"rchatter.h"
#include "monotxt.h"							//RJS 15Aug00

MissMan	Miss_Man;
//extern	ULong GR_WP1;
//extern	ULong GR_WP2;
//extern	ULong GR_ConvoyCount;
//extern	ULong GR_ConvoyUid;
//extern	ULong GR_AAASite;
//extern	ULong
//
//					GR_MainPosX,
//					GR_MainPosY,
//					GR_MainPosZ,
//					GR_SubPosX,
//					GR_SubPosY,
//					GR_SubPosZ,
//					GR_MainAction,		//RDH 05Dec96
//					GR_SubAction,		//RDH 05Dec96
//					GR_MainObjective,										//MS 21Jun96
//					GR_SubObjective;										//MS 21Jun96
BITABLE(Persons2::BattlefieldType);
MATHABLE(Persons2::BattlefieldType);

#ifndef	NDEBUG
#define	_ALERTFUDGE_
#endif
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SetSecondWP
//Author		R. Hyde 
//Date			Thu 17 Oct 1996
//
//Description		Miss_Man.camp.playerbfield==FIL_NULL when entering map
//						When entering map we don't want to increment wps
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//DEADvoid	Persons2::SetSecondWP (WayPointPtr	tmpitem)
//DEAD{
//DEAD	UniqueIDBand	uidband;
//DEAD
//DEAD	if (tmpitem)
//DEAD	{
//DEAD		uidband = getbandfromUID((UniqueID)tmpitem->target->uniqueID.count);
//DEAD		if (uidband == RedMilitaryBAND)
//DEAD		{
//DEAD				if (Miss_Man.camp.playerbfield==FIL_NULL)
//DEAD				{
//DEAD					GR_WP2 = GR_WP1;
//DEAD				}else
//DEAD				{
//DEAD					if (tmpitem->next==NULL)					//RDH 05Dec96
//DEAD						GR_WP2 = GR_WP1;						//RDH 05Dec96
//DEAD					else										//RDH 05Dec96
//DEAD						GR_WP2 = tmpitem->next->uniqueID.count;	//RDH 05Dec96
//DEAD				}
//DEAD
//DEAD		}else
//DEAD		{
//DEAD			if (tmpitem->target->Status.deadtime != 0)
//DEAD			{
//DEAD				GR_WP2 = GR_WP1;
//DEAD			}else
//DEAD			{
//DEAD				if (Miss_Man.camp.playerbfield==FIL_NULL)
//DEAD				{
//DEAD					GR_WP2 = GR_WP1;
//DEAD				}else
//DEAD				{
//DEAD					if (tmpitem->next==NULL)					//RDH 05Dec96
//DEAD						GR_WP2 = GR_WP1;						//RDH 05Dec96
//DEAD					else										//RDH 05Dec96
//DEAD						GR_WP2 = tmpitem->next->uniqueID.count;	//RDH 05Dec96
//DEAD				}
//DEAD			}
//DEAD		}
//DEAD	}else
//DEAD	{
//DEAD		GR_WP2 = GR_WP1;
//DEAD	}
//DEAD
//DEAD}
//DEAD
inline UniqueID operator--(UniqueID& u,int) {return u=UniqueID(u-1);}
inline Persons2::BattlefieldType operator +=(Persons2::BattlefieldType& u,int i){return u=Persons2::BattlefieldType(u+i);}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FindNextBf
//Author		R. Hyde 
//Date			Fri 9 Aug 1996
//
//Description	
//
//Inputs	nextbftype	- state control variable
//
//Returns	filenumber, or null to skip this one
//			nextbftype updated - set to FINISHBF to quit
//			some globrefs updated
//------------------------------------------------------------------------------
UByte RndSq256()
{
	UWord rv=Math_Lib.rnd();
	rv=rv|(rv>>8);
	return (UByte)rv;
}

int	RndSq256(int v)
{
	return (v*RndSq256())>>8;
}
enum	{MIGDAYSOUTH,MIGDAYDIRECT,MIGDAYLATEBARCAP,MIGDAYALLBARCAP,MIGDAYBIGTRAIN};


FileNum	Persons2::FindNextBf (BattlefieldType &nextbftype)
{


	GR_movecode=AUTO_FOLLOWWP;
	UniqueIDBand	uidband;								//RDH 05Dec96
	BattlefieldType	n;
	WayPointPtr	tmpitem;
	{
		GR_TruckMoveCode=GROUND_TAXI;
		GR_TakeOffTime = MMC.currtime;
	}
	GR_GlobalSkillMin = SKILL_NOVICE;						//RDH 01Oct96
	GR_GlobalSkillMax = SKILL_MAX;							//CSB 29Oct00
	switch (nextbftype & BFPHASESMASK)
	{
	case	MAINWORLDBF:
	{
		GR_BLUESKILLCONFIG=Save_Data.autopilotskillLW;
		GR_REDSKILLCONFIG=Save_Data.autopilotskillRAF;
//				if (Todays_Packages[0][0][0].uid)
//				if (Todays_Packages[0].packagetarget)
//					nextbftype = ROUTEBF;
//				else
//					nextbftype = SCRAMBF;
//
		nextbftype = CONVOYBF;	//REMOVE MACCAS  IF STATEMENT EDITS HERE!
//
		return(FIL_1_MAINWLD);
	}

	case	PLAYROUTEBF:
	{	//for the player's target area, launch all neighboring battlefields
		FileNum rv=FIL_NULL;
		switch (nextbftype)
		{
		case	PLAYROUTEBF:
		{
			if (Todays_Packages[0].packagestatus!=Profile::PS_SPARE)
			{	//find target[0] from selected package
				Profile* prof=&Todays_Packages[Todays_Packages.hipack];
				int tn=prof->squadlist[Todays_Packages.hisquad].targetindex;
				if (tn<0) tn=0;											//CSB 24/05/00
				GR_Pack_Target[0]=prof->packagetarget[tn].SGT;
				if (GR_Pack_Target[0]>=0x4000)
					GR_Pack_Target[0]=0;
				else
					if (GR_Pack_Target[0]>=SagBAND && GR_Pack_Target[0]<SagBANDEND)
					{
						int pt=GR_Pack_Target[0];
						GR_Pack_Target[0]=UID_NULL;
						for (int p=0;p<Profile::MAX_PACKS;p++)
							for (int s=0,max=Todays_Packages[p].squadlist.Max();s<max;s++)
								if (Todays_Packages[p][s].instance==pt)
								{
									GR_Pack_Target[0]=UniqueID(Todays_Packages[p].packagetarget[0]);
									s=max;
									p=Profile::MAX_PACKS;
								}
					}
				SquadNum sq=SquadNum(prof->squadlist[Todays_Packages.hisquad].squadnum);
				GR_Pack_Target[6]=Node_Data[sq].homeairfield;
				GR_Pack_Target[7]=Node_Data[sq].squadron;
			}
			else
			{
				GR_Pack_Target[0]=CSQuick1::quickdef.target;
				SquadNum sq=SquadNum(CSQuick1::quickdef.line[CSQuick1::quickdef.plside][CSQuick1::quickdef.plwave][CSQuick1::quickdef.plgrp].actype);
				UniqueIDBand	targband=GetBandFromUID(CSQuick1::quickdef.target);
				GR_Pack_Target[6]=Node_Data[sq].homeairfield;
				if (	CSQuick1::quickdef.currmissnum!=CSQuick1::quickdef.altmissnum 
					&& 	(targband==RAF_FighterAFBAND ||	 targband==LUF_FighterLF2BAND) 
					)
					GR_Pack_Target[6]=CSQuick1::quickdef.target;
				GR_Pack_Target[7]=Node_Data[sq].squadron;



			}
			{//context
				_Radio.phrase_groundcontroller=PHRASE_GROUNDCONTROLLERRAF0X-PHRASE_GROUNDCONTROLLERRAF;
				SquadNum sq=SquadNum(GR_Pack_Target[7]);
				if (sq>SQ_LW_START)
					_Radio.phrase_groundcontroller=/*PHRASE_GROUNDCONTROLLERLW+*/Node_Data.geschwader[GruppenPtr(Node_Data[sq])->wadernum].luftflotte;
				else
				{
					TargetPtr trg=&Node_Data[UniqueID(GR_Pack_Target[6])];
					if (trg->IsA(Target::TSS_TargetAirfield))
					{
						TargetSectorField* sf=TargetFighterFieldPtr(*trg)->getsectoraf();
						if (sf->groupname==IDS_GROUP_11)
							if (sf->sectorname<=('f'&0xf))
								_Radio.phrase_groundcontroller=/*PHRASE_GROUNDCONTROLLERRAF+*/sf->sectorname;
					}
				}
			}
			TargetGroup* tg=NULL;
			if (GR_Pack_Target[0])
				tg=Node_Data[UniqueID(GR_Pack_Target[0])];
			if (tg)
			{
				GR_Pack_Target[1]=tg->secondaries[0];
				GR_Pack_Target[2]=tg->secondaries[1];
				GR_Pack_Target[3]=tg->secondaries[2];
				GR_Pack_Target[4]=tg->secondaries[3];
				GR_Pack_Target[5]=tg->secondaries[4];
			}
			else
			{
				GR_Pack_Target[1]=
					GR_Pack_Target[2]=
						GR_Pack_Target[3]=
							GR_Pack_Target[4]=
									GR_Pack_Target[5]=UID_NULL;
			}
		}
		break;
///////////////////////////////////////////////////////////////
		case	TARGETDETBF:
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),false);
		break;
///////////////////////////////////////////////////////////////
		case	TARGETMOBBF:
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),true);
		break;
///////////////////////////////////////////////////////////////
		case	TAKEOFFDETBF:
			GR_Pack_Target[0]=GR_Pack_Target[6];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),false);
		break;
///////////////////////////////////////////////////////////////
		case	TAKEOFFMOBBF:
			GR_Pack_Target[0]=GR_Pack_Target[6];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),true);
		break;
///////////////////////////////////////////////////////////////
		case	TAKEOFFPTSBF:
		{
			GR_Pack_Target[0]=GR_Pack_Target[6];
			if (Node_Data[UniqueID(GR_Pack_Target[0])].IsA(Target::TSS_TargetAirfield))
			{
				TargetFighterField* tg=Node_Data[UniqueID(GR_Pack_Target[0])];
				rv=tg->taxibattlefield;
			}
		}
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG0DETBF:
			GR_Pack_Target[0]=GR_Pack_Target[1];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),false);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG0MOBBF:
			GR_Pack_Target[0]=GR_Pack_Target[1];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),true);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG1DETBF:
			GR_Pack_Target[0]=GR_Pack_Target[2];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),false);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG1MOBBF:
			GR_Pack_Target[0]=GR_Pack_Target[2];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),true);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG2DETBF:
			GR_Pack_Target[0]=GR_Pack_Target[3];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),false);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG2MOBBF:
			GR_Pack_Target[0]=GR_Pack_Target[3];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),true);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG3DETBF:
			GR_Pack_Target[0]=GR_Pack_Target[4];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),false);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG3MOBBF:
			GR_Pack_Target[0]=GR_Pack_Target[4];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),true);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG4DETBF:
			GR_Pack_Target[0]=GR_Pack_Target[5];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),false);
		break;
///////////////////////////////////////////////////////////////
		case	SECTRG4MOBBF:
			GR_Pack_Target[0]=GR_Pack_Target[5];
			rv=Node_Data.TargetToBf(UniqueID(GR_Pack_Target[0]),true);
		break;
///////////////////////////////////////////////////////////////
		case	ENDPLAYTARGBF:
		{}
		}
		if (nextbftype==ENDPLAYTARGBF)
		{
			if (Todays_Packages[0].packagestatus==Profile::PS_SPARE)
				nextbftype=SCRAMBF;
			else
				nextbftype=PACKPLAYERBF;
		}
		else
			nextbftype++;
		return rv;
	}
	break;
//////////////////////////////////////////////////////////////////////
//
// Function:    SCRAMBF
// Date:		15/02/00
// Author:		RJS
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
	case	SCRAMBF:
	{	//set up to load the scramble battlefield
		FileNum rv=FIL_NULL;
		switch (nextbftype)
		{
		case SCRAMBF:
		{	//set up to do airfields for all custom squads
		}
		break;
///////////////////////////////////////////////////
		case	SQ0TODETBF:
		case	SQ1TODETBF:
		case	SQ2TODETBF:
		case	SQ3TODETBF:
		case	SQ4TODETBF:
		case	SQ5TODETBF:
		case	SQ6TODETBF:
		case	SQ7TODETBF:
		case	SQ8TODETBF:
		case	SQ9TODETBF:
		case	SQ10TODETBF:
		case	SQ11TODETBF:
		case	SQ12TODETBF:
		{
			Squadron* sq=Node_Data[SquadNum(nextbftype-SQ0TODETBF)];
			if (sq)
			{
				GR_Pack_Target[0]=sq->homeairfield;
				rv=Node_Data.TargetToBf(sq->homeairfield);
			}
		}
		break;
///////////////////////////////////////////////////
		case LOADSCRAMBF:
		{	
			GR_OkToQuit3DNow=0x1000000*100;
			int glind=0;
			for(int s=0;s<2;s++)
				for(int w=0;w<8;w++)
					for(int g=0;g<3;g++)
						if(CSQuick1::quickdef.line[s][w][g].flights || CSQuick1::quickdef.line[s][w][g].AllowZero())
						{
							GR_Scram_Alt[glind]=CSQuick1::quickdef.line[s][w][g].alt;
							//ac count depends on aircraft type...

							int	acperflight=CSQuick1::quickdef.line[s][w][g].ACPerFlight();




							GR_Scram_AC[glind]=CSQuick1::quickdef.line[s][w][g].flights*acperflight;
							GR_Scram_Skill[glind]=CSQuick1::quickdef.line[s][w][g].skill;
							GR_Scram_Squad[glind]=CSQuick1::quickdef.line[s][w][g].actype;
							glind++;
						}
						else
							GR_Scram_AC[glind]=0;
			for (int sq=0;sq<=PT_GER_NONFLY;sq++)				//AMM 02Jun99
				GR_Pack_Sq_Used[sq]=0;

			GR_Pack_Target[0]=CSQuick1::quickdef.target;
			for (int c=0;c<16;c++)
				if (CSQuick1::quickdef.targets[c/4][c%4]==CSQuick1::quickdef.target)
					GR_PackTrgNum=c;
			if (CSQuick1::quickdef.plside)
				rv= (FileNum)CSQuick1::quickdef.altmissnum;
			else
				rv= (FileNum)CSQuick1::quickdef.currmissnum;
//Fudge...
#ifdef	_ALERTFUDGE_
	Trans_Obj.alertfudge = ConvertPtrUID(CSQuick1::quickdef.target);
#endif
		}
		break;
		case  ENDSCRAMBF:
		{
			Diary::RaidGroup*	rptr = Squad_Diary.GetRaidGroup(Manual_Pilot.ControlledAC2->ai.diaryptr);
			Diary::Intercept*	iptr = Squad_Diary.GetFirstIntercept(rptr);
			if (iptr)
				Squad_Diary.UpdateIntercept(iptr,Manual_Pilot.ControlledAC2,iptr->takeofftime);
		}
		break;
		}
		if (nextbftype==ENDSCRAMBF)
			nextbftype=PACKPLAYERBF;
		else
			nextbftype++;
		return rv;
	}
	break;
//////////////////////////////////////////////////////////////////////
//
// Function:    CONVOYBF
// Date:		04/02/00
// Author:		DAW
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
	case	CONVOYBF:
	if (Todays_Packages[0].packagestatus==Profile::PS_SPARE)
	{
		nextbftype=PLAYROUTEBF;
		return FIL_NULL;
	}
	else
	{
		int convnum=(nextbftype-CONVOYBF);
		TargetConvoy* convoy=Node_Data.convoylist+convnum;
		FileNum rv=FIL_NULL;
		if (convoy->uid==UID_NULL)
			nextbftype=PLAYROUTEBF;
		else
		{
			nextbftype = BattlefieldType(nextbftype+1);
			if (convoy->wpnum!=-1)
			{
				GR_redstrength=convoy->truestatus;
				GR_ItemInfoOverride=convoy->uid;
				GR_ItemWpOverride=convoy->wpnum;
				GR_ItemXOverride=convoy->X;
				GR_ItemZOverride=convoy->Z;
				rv=convoy->detailbattlefield;
			}

		}
		return rv;
	}



//////////////////////////////////////////////////////////////////////
//
// Function:    PLAYERBF
// Date:		04/02/00
// Author:		DAW
//
//Description: 
//
//////////////////////////////////////////////////////////////////////

	case	PACKPLAYERBF:
	{	
		switch (nextbftype)
		{
/////////////////////////////////////////////////
		case PACKPLAYERBF:
		{
			GR_PackNum=-1;
			GR_PackFilNum=0; 

		}
		case PACKNEXTPACKBF:	//INTENTIONAL FALL-THROUGH!
		{
			int packnum=GR_PackNum+1;
			while (packnum<Profile::MAX_PACKS && Todays_Packages[packnum].wpsused==0)
				packnum++;
			GR_PackNum=packnum;
			if (packnum<Profile::MAX_PACKS)								//CSB 27Jul00
				if (Todays_Packages[packnum].attackmethod<Profile::AM_LWPACKS)
					if (GR_REDSKILLCONFIG>=2)
						GR_GlobalSkillMin = 3*(GR_REDSKILLCONFIG-2);						//RDH 01Oct96
					else
						GR_GlobalSkillMax = SKILL_HERO-(2-GR_REDSKILLCONFIG)*2;							//RDH 01Oct96
				else
					if (GR_BLUESKILLCONFIG>=2)
						GR_GlobalSkillMin = 3*(GR_BLUESKILLCONFIG-2);						//RDH 01Oct96
					else
						GR_GlobalSkillMax = SKILL_HERO-(2-GR_BLUESKILLCONFIG)*2;							//RDH 01Oct96
			if (packnum==Profile::MAX_PACKS)
				nextbftype=FINISHBF;
			else
				nextbftype=PACKTRGRESETBF;
		}
		return FIL_NULL;
/////////////////////////////////////////////////
		case PACKTRGRESETBF:
		{
			GR_OkToQuit3DNow=MMC.earliestquit3dtimecs;
			GR_PackTrgNum=0;
			nextbftype++;
		}
		//intensional fall-through
/////////////////////////////////////////////////
		case PACKTRGDETBF:
		{
			UniqueID t=Todays_Packages[GR_PackNum].packagetarget[GR_PackTrgNum];
			if (t==UID_NULL)
			{
				nextbftype++;		 //LOOP HERE UNTIL ALL TARGETS PROCESSED
				return FIL_NULL;
			}
			else
			{
				GR_PackTrgNum++;
				return Node_Data.TargetToBf(t,false);
			}
		}
/////////////////////////////////////////////////
		case PACKTORESETBF:		 
		{
			GR_PackTrgNum=0;
			nextbftype++;
		}
		//intensional fall-through
/////////////////////////////////////////////////
		case PACKTODETBF:
		{
			SquadNum sq=SquadNum(Todays_Packages[GR_PackNum][GR_PackTrgNum].squadnum);
			UniqueID t=Node_Data[sq].homeairfield;
			GR_PackTrgNum++;
			if (GR_PackTrgNum==Todays_Packages[GR_PackNum].squadlist)
				nextbftype++;	 //LOOP HERE UNTIL ALL TAKEOFFS PROCESSED
			return Node_Data.TargetToBf(t,false);
		}
/////////////////////////////////////////////////
		case  PACKACBF:
		{
			//The aircraft for each package...

			Todays_Packages[GR_PackNum].SetTargetUIDs(true);
			Todays_Packages[GR_PackNum].SetTargetGlobrefs(true);
			//Zoiks! These targets are pre- the reordering!
			for (int i=0;i<10;i++)
				SetLoc(SGR_Pack_Target+i,Todays_Packages.Reorder(UniqueID(GetLoc(SGR_Pack_Target+i))));
			WipeGlobrefPackVars();										//CSB 01/06/00
			nextbftype++;
			return FileNum(FIL_RUNTIMEPACK0+GR_PackFilNum++);
		}
		//intensional fall-through if run out of packs
/////////////////////////////////////////////////
		}	//end switch
		return FIL_NULL;
	}
	}	//end switch
	return(FIL_NULL);
}



//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LaunchConvoy
//Author		R. Hyde 
//Date			Fri 9 Aug 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Persons2::LaunchConvoy(FileNum bfield,SWord size)


{
	Bool	repeatuid,retval = FALSE;
	return (retval);
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LaunchTankConvoy
//Author		R. Hyde 
//Date			Fri 9 Aug 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Persons2::LaunchTankConvoy ()
{
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LaunchTruckConvoyTest
//Author		Jim Taylor
//Date			Thu 17 Oct 1996
//
//Description	
//if gun killed this time launch a truck
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Persons2::LaunchTruckConvoyTest ()
{

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LaunchTruckConvoy
//Author		R. Hyde 
//Date			Fri 9 Aug 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Persons2::LaunchTruckConvoy ()
{
	Bool	retval = FALSE;
	return (retval);

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		TankEndMission
//Author		R. Hyde 
//Date			Fri 9 Aug 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Persons2::TankEndMission()
{
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		lowlevel_squadron
//Author		Jim Taylor
//Date			Wed 25 Sep 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Persons2::lowlevel_squadron(int snequ)
{
//		if (snequ>SNEQU_LASTACE)
//			if (snequ>SNEQU_SECONDSET)
//				eventloglist->home=_Text.ConvSnip0((TextList)LIST_SQUADS2,snequ-SNEQU_SECONDSET-1);
//			else
//				eventloglist->home=_Text.ConvSnip0((TextList)LIST_SQUADS,snequ-SNEQU_LASTACE-1);
//		else
//		{
//			eventloglist->home=_Text.ConvSnip0((TextList)LIST_ACESQUADS,snequ);
//			eventloglist->name=_Text.ConvSnip0((TextList)LIST_ACES,snequ);
//		}	
		return(FALSE);
}


void	Persons4::PleaseWarnUser(char* msg,char* title)
{
	//bugger off.
}


//////////////////////////////////////////////////////////////////////
//
// Function:    Post3DInfo
// Date:		24/03/00
// Author:		JIM
//
//Description: A temporary arry that holds the latest 3d position of each SAG
//
//////////////////////////////////////////////////////////////////////
struct	Post3DInfo
{
	Coords3D	position;
	UWord		waypoint;
	UWord		fuelkilos;
	UniqueID	unfriendly;
	UWord		movecode;

	Post3DInfo()	{waypoint=0xffff;}
	void		Set(AirStrucPtr a);
#ifdef	PACKAGE_INCLUDED
	int			Get(info_airgrp* a,Profile& pk,int sqnum,UniqueID route);	//returns fuel
#endif
	static		Post3DInfo*	New();
	void		Delete();

};
	enum	{MAX_SAGINVALIDATES=256};
	Post3DInfo* Persons3::post3dinfo=NULL;
//////////////////////////////////////////////////////////////////////
//
// Function:    Post3DInfo::Set
// Date:		24/03/00
// Author:		JIM
//
//Description:	Fill this structure with data from the airstruc
//
//	Coords3D	position;
//	UWord		waypoint;
//	UWord		fuelkilos;

//////////////////////////////////////////////////////////////////////
void		Post3DInfo::Set(AirStrucPtr a)
{
	position=a->World;
	WayPointPtr w=a->waypoint;	//->prev;						//MS 17Nov100
	int c=1000;
	if (w)
		for (c=-1;w;c++)
			w=w->prev;
	waypoint=c;
	fuelkilos=(a->fly.fuel_content[0]+a->fly.fuel_content[1]+a->fly.fuel_content[2]+a->fly.fuel_content[3])/100000;
	if (a->fly.expandedsag)
		movecode=a->fly.expandedsag->movecode;
	else
		movecode=a->movecode;
	ItemPtr	u=NULL;
	if (a->ai.unfriendly)
		u=a->ai.unfriendly;
	else
		if (a->fly.expandedsag && a->fly.expandedsag->ai.unfriendly)
			u=a->fly.expandedsag->ai.unfriendly;
	if (u && u->Status.size<AIRSTRUCSIZE)
		u=NULL;
	if (u && AirStrucPtr(u)->fly.expandedsag && AirStrucPtr(u)->fly.numinsag==0)
		u=AirStrucPtr(u)->fly.expandedsag;
	if (u)
		unfriendly=u->uniqueID.count;
	else
		unfriendly=UID_NULL;




}
//////////////////////////////////////////////////////////////////////
//
// Function:    Post3DInfo::Get
// Date:		24/03/00
// Author:		JIM
//
//Description: Copy data from this structure to the airgrp
//
//////////////////////////////////////////////////////////////////////
int		Post3DInfo::Get(info_airgrp* a,Profile& pack,int sqnum,UniqueID route)
{
	if (waypoint==0xffff)
		return 0;
	Profile::Squad* sq=&pack[sqnum];
	int rv=sq->numacleft;
	a->knownposition=
		a->World=
			position;
	a->position.MakeAbsWorldPos(a->World.Y);
	a->position[0][0]=a->World.X;
	a->position[0][2]=a->World.Z;

	bool	passedip=false;
	for (int c=waypoint;c;c--)
	{
		info_waypoint* w=Persons2::ConvertPtrUID(route);
		if (w->nextwp == UID_Null)										//RJS 15Nov00
			break;
		else
		{
			route=w->nextwp;
			if (w->uid.Evaluate()==-SGR_WPP_Egress)
				passedip=true;
		}
	}
	a->movecode=movecode;
	//generate status from movecode:
	switch (movecode)
	{
	case	AUTOSAG_WAITTAKEOFF:	sq->status=Profile::PS_PLANNED;		return 0;	
	case	AUTOSAG_TAKEOFF:		sq->status=Profile::PS_TAKINGOFF;	return 0;
	case	AUTOSAG_FOLLOWWP:
	case	AUTOSAG_FOLLOWWPHOME:
		if (passedip)				sq->status=Profile::PS_OUTGOING;
		else						sq->status=Profile::PS_INCOMING;
		break;
	case	AUTOSAG_LANDING:		sq->status=Profile::PS_LANDING;		break;
	case	AUTOSAG_REFUELLING:		sq->status=Profile::PS_REFUELLING;	break;
	case	AUTOSAG_PRECOMBAT:		sq->status=Profile::PS_ENEMYSIGHTED;break;
	case	AUTOSAG_COMBAT:			sq->status=Profile::PS_ENEMYSIGHTED;break;
	case	AUTOSAG_COMBATCIRCLES:	sq->status=Profile::PS_ENEMYSIGHTED;break;
	case	AUTOSAG_POSTCOMBAT:		sq->status=Profile::PS_ENEMYSIGHTED;break;
	case	AUTOSAG_BOMBAPPROACH:	sq->status=Profile::PS_TARGETAREA;	break;
	case	AUTOSAG_BOMB:			sq->status=Profile::PS_TARGETAREA;	break;

	case	AUTOSAG_DEATHGLIDE:
	case	AUTOSAG_DESTROYED:		
	{
		pack.SquadComplete(sqnum);	//Apply devastating losses if squad wiped out...	
		if (sq->numacleft>0)
		{
// Update Note log...
			int	numacleft = sq->numacleft;								//RJS 12Nov00

			MMC.thisweekreview.Note(sq->Squadron().AcType(),PeriodReview::DESTROY,numacleft / 2);

			numacleft -= (numacleft / 2);								//RJS 12Nov00

			sq->numacditched+=sq->numacleft;
			sq->numacrecoveredmed+=numacleft/2;							//RJS 12Nov00

			MMC.thisweekreview.Note(sq->Squadron().AcType(),PeriodReview::DAMAGE,numacleft / 2);

			numacleft -= (numacleft / 2);

			sq->numacrecoveredbad+=numacleft;							//RJS 12Nov00
			
			MMC.thisweekreview.Note(sq->Squadron().AcType(),PeriodReview::DESTROY,numacleft);

			sq->numpilotslost+=numacleft;								//RJS 12Nov00
			rv=- (int)sq->numacleft; //x0r realy =- here? not the -= ?
			sq->numacleft=0;
		}
	}
	break;

	case	AUTOSAG_LAST:			sq->status=Profile::PS_COMPLETE;	break;




	default: INT3;
	}
	a->wpref=route;
	a->target=unfriendly;
	sq->fuelleft=fuelkilos*100000;	
	return rv;	
}



//////////////////////////////////////////////////////////////////////
//
// Function:    Post3DInfo::New
// Date:		24/03/00
// Author:		JIM
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
Post3DInfo*	Post3DInfo::New()
{
	return	new Post3DInfo[MAX_SAGINVALIDATES];
}
//////////////////////////////////////////////////////////////////////
//
// Function:    Post3DInfo::Delete
// Date:		24/03/00
// Author:		JIM
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
void		Post3DInfo::Delete()
{
	delete	[]	this;
}

//////////////////////////////////////////////////////////////////////
//
// Function:    SaveAircraftPositions
// Date:		24/03/00
// Author:		JIM
//
//Description: Saves aircraft position info in the 3d before 3d exits
//	For each SAG 
//			post3dinfo entry is filled
//	For each convoy leader
//			nodedata is filled
//
//////////////////////////////////////////////////////////////////////
void	Persons3::SaveAircraftPositions(Inst3d* inst3d)
{
	if (post3dinfo)
		post3dinfo->Delete();
	FormationItemPtr m;
	GR_TakeOffTime=inst3d->timeofday/100;
	post3dinfo=post3dinfo->New();
	{for (int i=SagBAND,j=0;i<SagBANDEND;i++,j++)
		if ((m=ConvertPtrUID(UniqueID(i)))!=NULL)
		{
			AirStrucPtr a=*m;
			post3dinfo[j].Set(a);
	}	}
	{for (int i=BritBoatBAND;i<PowerStationBAND;i++)
		if ((m=ConvertPtrUID(UniqueID(i)))!=NULL)
		{
			UniqueID u=UniqueID(i);
			if (m->SGT)
				u=m->SGT;
			TargetConvoy* convoy=Node_Data[u];
			if (convoy)
			{
				if (m->leader==NULL)
				{
					convoy->X=m->World.X;
					convoy->Z=m->World.Z;
					WayPointPtr w=m->waypoint;
					int c=100;
					if (w)
						for (c=-1;w;c++)
							w=w->prev;
					convoy->wpnum=c;
				}
				if (m->Status.deadtime)
					if (convoy->truestatus<100)
					{
						convoy->truestatus+=13-convoy->truestatus/8;	//RJS 31Oct00
					}
				convoy->knownstatus=convoy->truestatus;
			}
	}	}
	Dead_Stream.SetWorldDead(inst3d->world);
}
//////////////////////////////////////////////////////////////////////
//
// Function:    RestoreAircraftPositions
// Date:		24/03/00
// Author:		JIM
//
//Description: Restores the aircraft position info onto the map screen
//
//////////////////////////////////////////////////////////////////////
void	Persons3::RestoreAircraftPositions()
{
	if (post3dinfo)
	{	UniqueID instuid;
		Profile::Squad* sq;
		MMC.currtime=GR_TakeOffTime;									//DAW 7Aug00
		for (int p=0;p<Profile::MAX_PACKS;p++)
		{
			int s=0,smax=Todays_Packages[p].squadlist;
			for (;s<smax;s++)
			{
				if ((instuid=Todays_Packages[p][s].instance)!=UID_NULL)
					if (post3dinfo[instuid-SagBAND].Get(ConvertPtrUID(instuid),Todays_Packages[p],s,Todays_Packages[p].route)<0)	//indicates that aircraft were removed to zero.
					{
						Todays_Packages.ChangeMessagesUID(Todays_Packages[p][s].instance,UID_BIT14+((p)<<5)+Todays_Packages[p].GetRaidIndFromSquadEntry(s));
					}

			}
			if (smax)
			{
				Todays_Packages[p].ApplyDiaryLosses();
				Todays_Packages[p].FixPackageStatus();
			}
			else
				assert(Todays_Packages[p].packagestatus==Profile::PS_SPARE);
		}
		post3dinfo->Delete();
		post3dinfo=NULL;
	}
	Node_Data.CheckDamage(Dead_Stream);
}

void	Persons2::SetUpDiary()
{
// Fix-up Quick-Mission Diary Records...

	Squad_Diary.EmptyDiary();
	Squad_Diary.CreateQuickEntry(GR_Scram_Alt,GR_Scram_AC,GR_Scram_Squad,0,CSQuick1::quickdef.target);
}
