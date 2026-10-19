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
//Filename       wpaction.cpp
//System         
//Author         R. Hyde 
//Date           Mon 19 Feb 1996
//Description    What happens when you reach a waypoint
//
//Notes:	All aircraft in flight go through waypoint action simultaniously
//
//
//
//
//
//
//
//
//------------------------------------------------------------------------------

#include	"dosdefs.h"
	#define F_COMMON
	#define F_GRAFIX
	#define F_BATTLE
#include	"files.g"
#include	"worldinc.h"
#include	"flymodel.h"
#include	"mytime.h"
#include	"ai.h"
#include	"mymath.h"
#include	"myerror.h"
#include	"3dcom.h"
#include	"persons2.h"

#include	"miles.h"
/////////
/////////	BELOW:	Waypoint actions
/////////
/////////
/////////
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		nextwaypoint
//Author		R. Hyde 
//Date			Mon 19 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
WayPoint*	WayPoint::NextWP(UniqueID uniqueID)
{
	WayPoint* waypoint=this;
	while (waypoint->next)
	{
		waypoint = waypoint->next;
		breakif (!waypoint->skipover);
		breakif (uniqueID<=waypoint->skipover && uniqueID>=waypoint->skipunder);
	}
	assert(waypoint);
		
	return waypoint;
}
WayPoint*	WayPoint::NextWP(MobileItemPtr owner)
{
	return NextWP(owner->uniqueID.count);
}
WayPoint*	WayPoint::NextWP(AirStrucPtr owner)
{
	if (owner->fly.expandedsag && owner->fly.numinsag==0)
		return NextWP(owner->fly.expandedsag->uniqueID.count);
	else
		return NextWP(owner->uniqueID.count);
}

void	mobileitem::NextWayPoint ()

{
	if (Status.size==AIRSTRUCSIZE)
	{
		if (!waypoint->wpnum)
			if (AirStrucPtr(*this)->AcIsPlayer())
				switch (waypoint->wpname)
				{
				case WPNAME_Target:
					{
						_Miles.SequenceAudible(FIL_MUSIC_OBJECTIVE_SIGHTED);
						if (GR_TruckMoveCode==GROUND_TOGLOBREF)
							OverLay.DecisionMessage(SecretMessages,0,Persons2::PlayerGhostAC,NULL,Persons2::PlayerGhostAC);
					}
				break;
				}

		waypoint=waypoint->NextWP(AirStrucPtr(*this));

		if (uniqueID.count==waypoint->skipunder)
		{
			AirStrucPtr ap=*this;
			if (ap-> formpos)	//wing splitting off
			{
				_Radio.TriggerMsg(MESSAGE_STRUC(PHRASE_BREAKINGOFF, MSG_MOVETOACTIVE, ap->fly.leadflight, NULL, ap));
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_BREAK_ORBITREPLY, MSG_DISENGAGE_DELAY_LP_RPT, ap,NULL,ap->fly.leadflight));
			}  
		}
	}
	else
		waypoint=waypoint->NextWP(this);

}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		TeleportWayPoint
//Author		Jim Taylor
//Date			Tue 30 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	mobileitem::TeleportWayPoint ()
{
	AirStrucPtr This=(AirStrucPtr)*this;
	NextWayPoint();
	World=*This->FindDesPos();

	if (waypoint->_vel == 0)										//MS 07May96
	{ ;															//MS 07May96
//TempCode MS 07May96 		vel =  classtype->cruisevel;							//MS 07May96
	}else														//MS 07May96
	{															//MS 07May96
	 	vel_ =  waypoint->_vel;									//MS 07May96
	}															//MS 07May96

	ExecuteWayPointAction();
	InterceptandRange(This->FindDesPos());
	roll=ANGLES_0Deg;											//MS 07May96
	hdg=HdgIntercept;
	pitch=PitchIntercept;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EndLoopWayPoint
//Author		Jim Taylor
//Date			Tue 30 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
#undef THIS
BFieldWaypointActions	mobileitem::EndLoopWayPoint ()
{	
	if (waypoint->NextWP(this) && waypoint->NextWP(this)->ETA && waypoint->NextWP(this)->ETA<timeofday)//MS 02Sep96
	{
		NextWayPoint();
		return(wpacnoactionno);
	}
	else
	{
		if (Status.size==AirStrucSize)
		{
			AirStrucPtr This=*this;
			if(This->FuelShort(15 * 60 * 100)	||	This->AircraftDamaged()	)
			{
				This->SayEndPatrolFuel();
				NextWayPoint();
				This->SetGroupWP(waypoint);
				return(wpforcebreakloop);
			}
			else
			if (EscorteePastWP(waypoint))
			{
				NextWayPoint();
				This->SetGroupWP(waypoint);
				return(wpforcebreakloop);
			}
			else
			{
				WeapAnimData*	weapon=NULL;										//RJS 29Aug96
				int index=0;
				SLong	xpos,ypos,zpos;
				ULong	mvel;											//RJS 12Nov99
				UWord	mdelay,mburst;									//RJS 12Nov99
		
				if (Anim == NULL)								//RJS 21Apr99
				{
					assert (this==Persons2::PlayerGhostAC);
				}
				else
				{
					while (index<6 && !weapon)
						weapon = SHAPE.GetWeaponLauncher(this,index++,xpos,ypos,zpos,mvel,mdelay,mburst,LT_BULLET);//RDH 31Jul98
					if (weapon && weapon->Stores+weapon->LoadedStores<20)
					{
						NextWayPoint();
						This->SetGroupWP(waypoint);
						return(wpforcebreakloop);
					}
				}
			}
		}
	}
	//go back
	{
		WayPointPtr	wp=waypoint->prev;							//MS 02Sep96
		while(	wp!=NULL
			&&	wp->action!=wpstartloopno
			&&	wp->action!=wpstoploopchkno
			)
		{
			wp->ETA=0;		//looping arround again, the old ETA must be rubbish!
			wp=wp->prev;
		}
		if (wp==NULL)
			NextWayPoint();
		else
			waypoint=wp;
	}
	return(wpstoploopchkno);
}


bool	mobileitem::EscorteePastWP(WayPointPtr	waypoint)
{
	if (Status.size<AIRSTRUCSIZE)		return false;
	AirStrucPtr as=*this;
	if (as->formpos)			 		return false;
	if (!as->fly.leadflight)	 		return false;
	if (!as->fly.leadflight->waypoint)	return true;
	do	waypoint=waypoint->next; 
	while (waypoint && waypoint->skipunder);
	
	as=as->fly.leadflight;
	
	while (waypoint && waypoint!=as->waypoint)
		waypoint=waypoint->next;
	
	if (waypoint)						return true;
	else				 				return false;

}
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		MidLoopWayPoint
//Author		Jim Taylor
//Date			Tue 30 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
BFieldWaypointActions	mobileitem::MidLoopWayPoint ()
{	

BFieldWaypointActions	rv=wpbreakloop;
	if (Status.size!=AirStrucSize)
	{
		rv=wpbreakloop;
		NextWayPoint();
		return(rv);
	}
	else
	{
		if (	(((AirStrucPtr)*this)->FuelShort(15 * 60 * 100))//MS 02Sep96
			||	(((AirStrucPtr)*this)->AircraftDamaged())
			)
		{
			((AirStrucPtr)*this)->SayEndPatrolFuel();
			rv=wpforcebreakloop;
		}
		else
		{
			WayPointPtr wp=waypoint;
			while (wp && wp->action!=wpstoploopchkno)
				wp=wp->next;
			if (EscorteePastWP(wp))
				rv=wpforcebreakloop;
			else
			if (Anim==NULL)										//RJS 21Apr99
			{
				assert (this==Persons2::PlayerGhostAC);
			}
			else
			{
				WeapAnimData*	weapon=NULL;										//RJS 29Aug96
				int index=0;
				SLong	xpos,ypos,zpos;
				ULong	mvel;											//RJS 12Nov99
				UWord	mdelay,mburst;									//RJS 12Nov99

				if (Anim==NULL)										  //DAW 26/04/99
				{
					assert (this==Persons2::PlayerGhostAC);
				}
				else
				{
					while (index<6 && !weapon)									//RDH 31Jul98
						weapon = SHAPE.GetWeaponLauncher(this,index++,xpos,ypos,zpos,mvel,mdelay,mburst,LT_BULLET);//RDH 31Jul98
					if (weapon && weapon->Stores+weapon->LoadedStores<20)
						rv=wpforcebreakloop;
				}
			}
		}
	}
	if (rv==wpforcebreakloop)
	{
		WayPointPtr wp=waypoint;
		UniqueID id = uniqueID.count;
		if(Status.size == AIRSTRUCSIZE)									//CSB 26Jul00
		{																//CSB 26Jul00
			AirStrucPtr ac = *this;										//CSB 26Jul00
			if((ac->fly.expandedsag) && (ac->fly.numinsag == 0))			//CSB 26Jul00
				id = ac->fly.expandedsag->uniqueID.count;					//CSB 26Jul00
		}																//CSB 26Jul00
		while (wp 
			&& (wp->action!=wpstoploopchkno || wp->skipunder>id || wp->skipover<id)	//CSB 26Jul00
			)
			wp=wp->next;
		assert (wp);
		wp = wp->NextWP(this);											//CSB 26Jul00
		AirStrucPtr(*this)->SetGroupWP(wp);
//		ForceBreakLoopWayPoint();

	}
	else
		NextWayPoint();
	return(rv);
}


void	mobileitem::ForceBreakLoopWayPoint()
{
	//I have already forced the group to go home, so this is pointless!

//	WayPointPtr wp=waypoint;
//	while (wp && wp->action!=wpstoploopchkno)
//		wp=wp->next;
//	if (wp)
//		waypoint=wp;
//	NextWayPoint();
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		WaitDeadWayPoint
//Author		Jim Taylor
//Date			Wed 31 Jul 1996
//
//Description	Waits for its target to die
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	mobileitem::WaitDeadWayPoint ()
{
	if (waypoint->target==NULL || waypoint->target->Status.deadtime)
		NextWayPoint();
	else
		if (	movecode!=AUTO_TAXI
			&&	movecode!=GROUND_TAXI
			&&	waypoint->prev
			)
			waypoint=waypoint->prev;
		else
			((FormationItem*)*this)->manoeuvretime=2000;	//force 20 secs straight line

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ALandingWayPoint
//Author		Jim Taylor
//Date			Wed 31 Jul 1996
//
//Description	Set up to land on homebase
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	mobileitem::LandingWayPoint ()
{
	//Want to keep on landing waypoint!
	if (Status.size==AirStrucSize)
	{
		AirStrucPtr a=*this;

		if(Manual_Pilot.controlmode == ManualPilot::AUTOACCEL_HOME)
			if((a == Persons2::PlayerSeenAC) || (a == Persons2::PlayerGhostAC))
				OverLay.CancelAccel();

		if (a->formpos==0)
			a->fly.leadflight=NULL;		//break from bombers

		a->movecode = AUTO_LANDING;										//CSB 14Jun00
		a->ai.ManStep = 0;												//CSB 14Jun00
		a->SetManoeuvreTime(0);											//CSB 14Jun00

		if (!_Replay.Playback)
			Squad_Diary.SetLandingTime(a->ai.diaryptr);					//RJS 29Feb00
	}
}

void	FormationItem::DisperseWayPoint()
{
	if (waypoint->next->action==wpdisperseno)
	{	//If next wp is also disperse then we are on 'fighter disperse'
		//Need to know if we are a fighter.
		Squad_Diary.SetDisperseTime(AirStrucPtr(*this)->ai.diaryptr);	//use view time

		if (duty>=PACK_AM_ESCORT && duty<=PACK_AM_RETURN)
		{
			NextWayPoint();
		}
	}
	NextWayPoint();
}
void	FormationItem::FighterLimitWayPoint()
{
	if (classtype->planetext==PT_ME109)
	{
		while (waypoint->action!=wpdisperseno)
			NextWayPoint();

		AirStrucPtr ac = AirStrucPtr(this);																//CSB 23Aug00
		if((ac->fly.leadflight) && (ac->fly.leadflight->classtype->aerobaticfactor != AEROBATIC_HIGH))	//CSB 23Aug00
		{																								//CSB 23Aug00
			ac->fly.leadflight = NULL;																	//CSB 23Aug00
			if(ac->fly.expandedsag)																		//CSB 23Aug00
				ac->fly.expandedsag->fly.leadflight = NULL;												//CSB 23Aug00
		}																								//CSB 23Aug00
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		KillItNow
//Author		Jim Taylor
//Date			Wed 31 Jul 1996
//
//Description	USED AS RECONN WAYPOINT ACTION! Something to do with reconn spotters in FC
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	mobileitem::KillItNow ()
{
		NextWayPoint();					//Move on to next waypoint
		if (Status.size>=AIRSTRUCSIZE)	//But stay on current heading for 60 seconds	//RJS 4Sep00
		{
			AirStrucPtr a=*this;
			if (vel_y>=0)
				a->SetManoeuvreTime(60*SECS100);	//travel 3 mins in straight line
			else
				a->SetManoeuvreTime(10*SECS100);	//travel 3 mins in straight line
			Squad_Diary.RaidResult(a, MR_RECONCOMPLETED);
		}


}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		StartBombing
//Author		Jim Taylor
//Date			Tue 10 Dec 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	mobileitem::StartBombing()
{
	AirStrucPtr a=*this;
	{

		assert(waypoint->target&&"Need target to kill in wp");
////TEMPCODE JIM 21/04/99 		if (waypoint->target->Status.deadtime)
////TEMPCODE JIM 21/04/99 		{														//JIM 05Dec96
////TEMPCODE JIM 21/04/99 			NextWayPoint();
////TEMPCODE JIM 21/04/99 			if (movecode==AUTO_BOMB)
////TEMPCODE JIM 21/04/99 				Art_Int.BreakOff(*this);
////TEMPCODE JIM 21/04/99 			return;		//SKIP PAST ACCEL CLEAR
////TEMPCODE JIM 21/04/99 		}
////TEMPCODE JIM 21/04/99 		else
		{
			a->ai.unfriendly=waypoint->target;						  //JIM 21/04/99
			movecode=AUTO_BOMB;
			a->ai.ManStep = 0;
			a->information = IF_OUT_POS;
			a->manoeuvretime = 0;
			if(!a->formpos)
			{
				AirStrucPtr buddy = a->FindBuddy();
				if((a->ai.unfriendly) && (buddy))
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SEETARGET, MSG_SECONDRUN, a, a->ai.unfriendly, buddy));
			}
		}
	}


	Squad_Diary.ArrivedAtTarget(a,waypoint->target);				//RJS 09Mar00
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		StartStraffe
//Author		Jim Taylor
//Date			Tue 10 Dec 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	mobileitem::StartStraffe()
{
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EmergencyService
//Author		Robert Slater
//Date			Mon 11 Jan 2000
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	mobileitem::EmergencyService()
{
	if (waypoint->target && waypoint->target->Status.size==AIRSTRUCSIZE)
	{
		NextWayPoint ();
		FormationItem*	fitem = *this;
		vel_ = vel_x = vel_y = vel_z = 0;
		fitem->manoeuvretime = 3*MINS100;
	}
	else
	{
		NextWayPoint ();
		Coords3D	worldPos;
		if (SHAPE.CheckForDamage(*this,worldPos))
		{
			// Stop...
			vel_ = vel_x = vel_y = vel_z = 0;

			if (Status.size == FORMATIONSIZE)
			{
				FormationItem*	fitem = *this;
				fitem->manoeuvretime = 10*MINS100;	 //Max time!

				Trans_Obj.LaunchEmergencyDudes(fitem,worldPos,*currworld);
			}
		}
	}
}



/////////
/////////
/////////	ABOVE:	Waypoint actions
/////////
/////////



/////////
/////////
/////////	BELOW:	Waypoint associated stuff, but not waypoint actions.
/////////
/////////

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FuelShort
//Author		Jim Taylor
//Date			Tue 30 Jul 1996									//JIM 21Mar99
//
//Description	Returns whether fuel will run out at current rates of use
//
//Inputs		safety margin time - so you can specify more in cruise
//				and less in combat.
//				This stops patrols with just enough fuel staying out
//				when they wouldn't be any use in a fight.
//
//Returns	true if can't get home...
//
//------------------------------------------------------------------------------
Bool	AirStruc::FuelShort (int safetytime,int* fuelavailP,int* safetyfuelP)
{
	if (ai.homebase && classtype->aerobaticfactor!=AEROBATIC_LOW)
	{
		int fuelavail = 0;
		for(SWord i = 0; i < 4; i++)
			fuelavail += fly.fuel_content[i];
		
		int sq=ai.squadnum();

		if (sq<PT_BRIT_NONFLY)
			sq=GR_Scram_Squad[sq];

		if (fuelavailP)
			*fuelavailP=fuelavail;
		if (safetyfuelP)
			*safetyfuelP=sq;
		if (sq>fuelavail)		//sq is low for squad num if not pilotable.
			return	TRUE;

		FP time = (FP(Distance3D(&ai.homebase->World)) * 11000.0) / FP(classtype->cruisevel);
		time += safetytime;
		int fuelused = time * classtype->fuelaboff;
		
		if (fuelused>fuelavail)
			return	(TRUE);
		else
			return	(FALSE);
	}
	else
		return(FALSE);
}

void	AirStruc::SayEndPatrolFuel()
{
	if (AcIsFlightLeader())
	{
		AirStrucPtr buddy=PlayerInGroup();
		if (buddy==NULL)
			buddy=FindBuddy();
		if (buddy)
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ONESWEEP, MSG_STATUSREPORT, *this, NULL, buddy));
	}
}


/////////
/////////
/////////	ABOVE:	Waypoint associated stuff, but not waypoint actions.
/////////
/////////
