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
//Filename       anmradio.cpp
//System
//Author         Robert Slater
//Date           Fri 5 Feb 1999
//Description
//				Radio chatter triggered by damage....
//
//------------------------------------------------------------------------------

#include	"dosdefs.h"
#include	"worldinc.h"
#include	"boxcol.h"
#include	"rchatter.h"
#include	"anmradio.h"										//RJS 04Feb99
#include	"3dinstr.h"											//RJS 13May99
#include	"radio.g"											//RJS 13May99
#include	"animdata.h"
#include	"flymodel.h"
#include	"mymath.h"
#include	"ai.h"
#include	"transite.h"
#include	"persons2.h"

#ifndef	NDEBUG

#define AFX_CDECL __cdecl
typedef char CHAR;
#define CONST               const
typedef CONST CHAR *LPCSTR, *PCSTR;
typedef LPCSTR LPCTSTR;
void AFX_CDECL AfxTrace(LPCTSTR lpszFormat, ...);

//#define	_CHATTRACE_

#endif


DamageChat	_DamageChat;


DamageChat::DamageChat()
{
	acHit = acHitter = buddy = leader = NULL;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ExecuteRadio
//Author		Robert Slater
//Date			Thu 4 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::ExecuteRadio(	ItemPtr				itm,
									ItemPtr				hitter,
									HitEffectRecord&	hitstruc	)
{
	acHitAlive = false;											//RJS 31Jan00

#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("Entering Radio...\n");
	SETFPCW(cw);

	_Radio.damageMessage = true;
#endif
	if (hitter)												//RJS 15Jun99
	{
 		acHit = (AirStrucPtr) itm;
 		acHitter = (AirStrucPtr) hitter;

		if (acHit)												//RJS 31Jan00
		{
			MinAnimData*	mad = acHit->Anim;
			if (mad->itemstate < DYING)									//RJS 26Jun00
				acHitAlive = true;										//RJS 26Jun00
		}

		if (acHit && acHit->nationality != acHitter->nationality)	//RERUN: acHit null-guarded above then derefed//RJS 25Jun99
		{
 			buddy = acHit->FindBuddy();

 			if (acHit->nationality != Manual_Pilot.ControlledAC2->nationality)
 			{
 				acHitIsFriend = FALSE;
 				leader = acHitter->GroupLeader();
 			}
 			else
 			{
 				acHitIsFriend = TRUE;
 				leader = acHit->GroupLeader();										//CSB 3Aug00
				if(!leader)															//CSB 3Aug00
				{																	//CSB 3Aug00
					AirStrucPtr newleader = NULL;									//CSB 3Aug00
					FP range2 = 200000.0 * 200000.0;								//CSB 3Aug00
					for(AirStrucPtr ac = AirStrucPtr(mobileitem::GetACList()); ac; ac = AirStrucPtr(ac->nextmobile))//CSB 3Aug00
						if((!ac->Status.deadtime) && (ac->nationality == acHit->nationality))//CSB 3Aug00
						{															//CSB 3Aug00
							FP r2 = ac->Distance3DSquared(&acHit->World);			//CSB 3Aug00
							if(r2 < range2)											//CSB 3Aug00
							{														//CSB 3Aug00
								range2 = r2;										//CSB 3Aug00
								newleader = ac;										//CSB 3Aug00
							}														//CSB 3Aug00
						}															//CSB 3Aug00
					if(!leader)														//CSB 3Aug00
						return;														//CSB 3Aug00
				}																	//CSB 3Aug00
 			}

 			switch (hitstruc.val)
 			{
 			case RADIO_GENERAL_LV1:
 			{
 				if (!acHitIsFriend)
 					OtherSideDamageLevel1();
 				else
 					PlayerSideDamageLevel1();
 			}
 			break;
 			case RADIO_GENERAL_LV2:
 			{
 				if (!acHitIsFriend)
 					OtherSideDamageLevel2();
 				else
 					PlayerSideDamageLevel2();
 			}
 			break;
			case RADIO_BADNEWS:
 			{
 				if (!acHitIsFriend)
 					OtherSideKilled();
 				else
 					PlayerSideKilled();
 			}
 			break;
 			case RADIO_BLACKSMOKE:
 				BlackSmoke();
 				break;
 			case RADIO_FUELLEAK:
 				FuelLeak();
 				break;
			case RADIO_GLYCOL:
 				EngineDamageGlycol();
 				break;
			case RADIO_ENGINEDAMAGE_LV1:
				if (acHitIsFriend)
	 				EngineDamage();
 				break;
 			}
		}
	}
#ifdef	_CHATTRACE_
	_Radio.damageMessage = false;

	::AfxTrace("... exiting Radio\n");
	SETFPCW(cw);
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		OtherSideDamageLevel1
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::OtherSideDamageLevel1()
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tOtherSideDamageLevel1\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif
	UWord	voxrandom = Math_Lib.rnd();							//RJS 31Jan00
	if ((voxrandom < VOX_RAND_1IN8) && acHitAlive)						//RJS 26Jun00
	{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\t...IN\n");
	SETFPCW(cw);
#endif

		buddy = acHitter->FindABuddyWithPlayerGivenPriority();
		AirStrucPtr	hitterplayer = acHitter->AcIsPlayer();

		if (buddy && !hitterplayer)
		{
			if (acHitter->ai.combatskill < SKILL_REGULAR)
		 		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_OTHERDAMAGE1LOW, MSG_DAMAGELEVEL1, acHitter, acHit, buddy));
			else
			{
				if (acHitter->ai.combatskill < SKILL_ACE)
		 			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_OTHERDAMAGE1MED, MSG_DAMAGELEVEL1, acHitter, acHit, buddy));
				else
		 			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_OTHERDAMAGE1HI, MSG_DAMAGELEVEL1, acHitter, acHit, buddy));
			}
		}
		else
		{
			if (hitterplayer)
	 			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_OTHERDAMAGE1HI, MSG_PLAYERDAMAGELEVEL1, acHitter, acHit, acHitter));
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		OtherSideDamageLevel2
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::OtherSideDamageLevel2()
{
/*
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tOtherSideDamageLevel2\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif */
	UWord	voxrandom = Math_Lib.rnd();							//RJS 31Jan00
	if ((voxrandom < VOX_RAND_1IN8) && acHitAlive)						//RJS 26Jun00
	{/*
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\t...IN\n");
	SETFPCW(cw);
#endif */
		if (!acHitter->AcIsPlayer())
		{
			buddy = acHitter->FindABuddyWithPlayerGivenPriority();
			if (buddy)
			{
				_Radio.ClearMessages(MSG_DAMAGELEVEL1,acHit);
				if (acHitter->ai.combatskill < SKILL_REGULAR)
				{
#ifdef USE_PHRASE_ONE_AIRCRAFT
					if (	(acHit->classtype->phrasename == PHRASE_JU87)
						||	(acHit->classtype->phrasename == PHRASE_BF110))
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_RATSINBARREL, MSG_DAMAGELEVEL2, acHitter, acHit, buddy));
					else
#endif
			 			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_OTHERDAMAGE2LOW, MSG_DAMAGELEVEL2, acHitter, acHit, buddy));
				}
				else
				{
					if (acHitter->ai.combatskill < SKILL_ACE)
			 			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_OTHERDAMAGE2MED, MSG_DAMAGELEVEL2, acHitter, acHit, buddy));
					else
			 			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_OTHERDAMAGE2HI, MSG_DAMAGELEVEL2, acHitter, acHit, buddy));
				}
			}
		}
		else
		{
			if (voxrandom < VOX_RAND_1IN32)
			{
				_Radio.ClearMessages(MSG_DAMAGELEVEL1,acHit);
				_Radio.ClearMessages(MSG_PLAYERDAMAGELEVEL1,acHit);

				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_OTHERDAMAGE2HI, MSG_PLAYERDAMAGELEVEL2, acHitter, acHit, acHitter));
			}
		}

	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PlayerSideDamageLevel1
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::PlayerSideDamageLevel1()
{

#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tPlayerSideDamageLevel1\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif
	UWord	voxrandom = Math_Lib.rnd();
	if ((voxrandom < VOX_RAND_1IN8) && acHitAlive)						//RJS 26Jun00
	{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\t...IN\n");
	SETFPCW(cw);
#endif
		if (	(voxrandom < VOX_RAND_1IN32)
			&&	!acHit->AcIsPlayer()		)
		{
// Random
			buddy = acHit->FindABuddyNotPlayer();
			if (buddy)
			{
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE1CONV, MSG_DAMAGELEVEL1, buddy, acHit, acHit));
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE1CONVREPLY, MSG_DAMAGELEVEL1REPLY, acHit,acHit,buddy));
			}
		}
		else
		{
// Based on skill
			buddy = acHit->FindABuddyWithPlayerGivenPriority();
			if (buddy)
			{
				if (acHit->ai.combatskill < SKILL_REGULAR)
				{
					if (!acHit->AcIsPlayer())
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE1CONV, MSG_DAMAGELEVEL1, acHit, acHit, buddy));
				}
				else
				{
					MsgType	damagemsg;

					if (acHit->AcIsPlayer())
						damagemsg = MSG_PLAYERDAMAGELEVEL1;
					else
						damagemsg = MSG_DAMAGELEVEL1;

					if (acHit->ai.combatskill < SKILL_ACE)
					{
						if (acHit->nationality == NAT_RED)
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE1MEDRAF, damagemsg, acHit, acHit, buddy));
						else
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE1MED, damagemsg, acHit, acHit, buddy));
					}
					else
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE1HI, damagemsg, acHit, acHit, buddy));
				}
			}
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PlayerSideDamageLevel2
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::PlayerSideDamageLevel2()
{

#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tPlayerSideDamageLevel2\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif

	UWord	voxrandom = Math_Lib.rnd();
	if ((voxrandom < VOX_RAND_1IN8) && acHitAlive)						//RJS 26Jun00
	{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\t...IN\n");
	SETFPCW(cw);
#endif
	 	buddy = acHit->FindABuddyWithPlayerGivenPriority();

		if (acHit->ai.morale < MORALE_POOR)
		{
			if (!acHit->AcIsPlayer())
			{
				Art_Int.BreakOff(acHit);
				acHit->ai.morale = MORALE_ROCKBOTTOM;


				if (buddy)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE2COWARD, MSG_DAMAGELEVEL2, acHit, acHit, buddy));
			}
		}
		else
		{
			MsgType	damagemsg;

			if (acHit->AcIsPlayer())
				damagemsg = MSG_PLAYERDAMAGELEVEL2;
			else
				damagemsg = MSG_DAMAGELEVEL2;

			if (acHit->ai.combatskill < SKILL_REGULAR)
			{
				if (!acHit->AcIsPlayer())								//RJS 14Sep00
				{

					if (buddy)
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE2LOW, damagemsg, acHit, acHit, buddy));
				}
			}
			else
			{
				if (acHit->ai.combatskill < SKILL_ACE)
				{
					if (buddy && !acHit->AcIsPlayer())					//RJS 14Sep00
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE2MED, damagemsg, acHit, acHit, buddy));
				}
				else
				{
					if (voxrandom < VOX_RAND_1IN64)
					{
						if (acHit->nationality == NAT_RAF)
						{
							buddy = acHit->FindABuddyNotPlayer();
							if (buddy)
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERDAMAGE2, damagemsg, buddy, acHit, acHit));
						}
						else
						{
							if (buddy)
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE2HI, damagemsg, acHit, acHit, buddy));
						}
					}
					else
					{
						if (buddy)
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_PLAYERDAMAGE2HI, damagemsg, acHit, acHit, buddy));
					}
				}
			}
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FuelLeak
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::FuelLeak()
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEMPTY: FuelLeak\n");
	SETFPCW(cw);
#endif
	if (acHitIsFriend)
	{




	}
	else
	{





	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		LeaderLeaking
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::LeaderLeaking()
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEMPTY: LeaderLeaking\n");
	SETFPCW(cw);
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FollowerLeaking
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::FollowerLeaking()
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEMPTY: FollowerLeaking\n");
	SETFPCW(cw);
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		FuelLeakMsg
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::FuelLeakMsg(AirStrucPtr	recipient)
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEMPTY: FuelLeakMsg\n");
	SETFPCW(cw);
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		BlackSmoke
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::BlackSmoke()
{

#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tBlackSmoke\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif

	UWord	voxrandom = Math_Lib.rnd();
	if ((voxrandom < VOX_RAND_1IN8) && acHitAlive)						//RJS 26Jun00
	{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\t...IN\n");
	SETFPCW(cw);
#endif
		if (acHitIsFriend)
		{
			MsgType	mtype;
			if (acHit->AcIsPlayer())
				mtype = MSG_PLAYERDAMAGELEVEL2;
			else
				mtype = MSG_DAMAGELEVEL2;

			if (voxrandom < VOX_RAND_1IN32)
			{
				buddy = acHit->FindABuddyNotPlayer();
				if (buddy)
				{
					if (voxrandom < VOX_RAND_1IN64)
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SMOKINGBADLY,mtype,buddy,acHit,acHit));
					else
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_FIREINENGINE,mtype,buddy,acHit,acHit));
				}
			}
			else
			{
				buddy = acHit->FindABuddyWithPlayerGivenPriority();
				if (buddy)
				{
					if ((voxrandom < VOX_RAND_1IN24) && !acHit->AcIsPlayer())
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_BURNINGUP,mtype,acHit,acHit,buddy));
					else
					 	_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SMOKE,mtype,acHit,acHit,buddy));
				}
			}

			EjectMsg(leader);
		}
		else
		{
			MsgType	mtype;
			if (acHitter->AcIsPlayer())
				mtype = MSG_PLAYERDAMAGELEVEL2;
			else
				mtype = MSG_DAMAGELEVEL2;

			buddy = acHitter->FindABuddyWithPlayerGivenPriority();
			if (buddy)													//RJS 4Sep00
			{
				if (voxrandom < VOX_RAND_1IN32)
			 		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ENEMYSMOKING,mtype,acHitter,acHit,buddy));
				else
			 		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_ENEMYSMOKING,mtype,acHitter,acHit,buddy));
			}
		}
	}

// Eject if you are piss-weak...
	if (	!acHit->AcIsPlayer()										//RJS 18Oct00
		&&	(voxrandom < VOX_RAND_1IN32)	)
	{
		if (acHit->ai.morale < MORALE_POOR)
			Trans_Obj.LaunchCanopyEject(acHit,*mobileitem::currworld);
		else
		{
			if (	(acHit->ai.combatskill < SKILL_REGULAR)
				&&	(voxrandom < VOX_RAND_1IN64)	)
				Trans_Obj.LaunchCanopyEject(acHit,*mobileitem::currworld);
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SmokingBadly
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::SmokingBadly(AirStrucPtr	buddy)
{

#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEMPTY: SmokingBadly\n");
	SETFPCW(cw);
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		BlackSmokeMsg
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::BlackSmokeMsg(AirStrucPtr	buddy)
{

#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEMPTY: BlackSmokeMsg\n");
	SETFPCW(cw);
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EjectMsg
//Author		Robert Slater
//Date			Fri 5 Feb 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::EjectMsg(AirStrucPtr	caller)
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEjectMsg\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif
	if (caller && (Math_Lib.rnd() < VOX_RAND_1IN4)	)					//RJS 2Aug00
	{
		if (!caller->AcIsPlayer())									//RDH 14May99
		{
 			ScriptTables	theScript;

 			if ((caller->ai.combatskill > SKILL_REGULAR) && (caller->ai.morale > MORALE_MEDIUM))
 				theScript = SCRIPT_EJECT_CALLER;
 			else
 			{
 				if (caller->ai.morale == MORALE_MEDIUM)
 					theScript = SCRIPT_EJECT_CALLEE;
 				else
 					theScript = SCRIPT_EJECT;
 			}

 			_Radio.TriggerMsg(MESSAGE_STRUC(theScript, MSG_DAMAGELEVEL2REPLY, caller, acHit, acHit));
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PlayerSideKilled
//Author		R. Hyde
//Date			Thu 29 Apr 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::PlayerSideKilled()
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tPlayerSideKilled\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif

	UWord	voxrandom = Math_Lib.rnd();							//RJS 31Jan00
	if (voxrandom < VOX_RAND_1IN8)								//RJS 31Jan00
	{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\t...IN\n");
	SETFPCW(cw);
#endif
		if (!acHit->AcIsPlayer())
		{
			buddy = acHit->FindABuddyWithPlayerGivenPriority();
			if (buddy)
			{
		 		ScriptTables	theScript = SCRIPT_NULL;

				if (voxrandom < VOX_RAND_1IN16)
					theScript = SCRIPT_PROPDEAD;
				else
				{
					if (voxrandom < VOX_RAND_1IN32)
						theScript = SCRIPT_ENGINEDIED;
					else
					{
						if (voxrandom < VOX_RAND_1IN64)
							theScript = SCRIPT_PLAYERKILLED;
						else
						{
							buddy = acHit->FindABuddyNotPlayer();
							if (buddy)
							{
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLER_PLAYERDAMAGE2CONV0,MSG_KILLREPORT, acHit, acHit, buddy));
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLER_PLAYERDAMAGE2CONV1,MSG_KILLREPORTREPLY, buddy, acHit, acHit));
							}
						}
					}
				}

				_Radio.TriggerMsg(MESSAGE_STRUC(theScript,MSG_KILLREPORT, acHit, acHit, buddy));//RJS 31Jan00
			}
		}
	}

	EjectMsg(leader);
	if (!acHit->AcIsPlayer())
		Trans_Obj.LaunchCanopyEject(acHit,*mobileitem::currworld);				//RJS 28Apr99

	if (!acHit->uniqueID.commsmove)									  //AMM 07/07/99
		acHit->ai.radiosilent = TRUE;			//RJS 09Jun99
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		OtherSideKilled
//Author		R. Hyde
//Date			Thu 29 Apr 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::OtherSideKilled()
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tOtherSideKilled\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif
	UWord	voxrandom = Math_Lib.rnd();							//RJS 31Jan00
	if (voxrandom < VOX_RAND_1IN4)								//RJS 31Jan00	//RJS 26Jun00
	{
		Trans_Obj.LaunchCanopyEject(acHit,*mobileitem::currworld);		//RJS 28Jun00

		if (voxrandom < VOX_RAND_1IN8)									//RJS 28Jun00
		{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\t...IN\n");
	SETFPCW(cw);
#endif
			MsgType	mtype;

			buddy = acHitter->FindABuddyWithPlayerGivenPriority();
			if (buddy)
			{
				_Radio.ClearMessages(MSG_DAMAGELEVEL2,acHit);					//RJS 15Jun99
				if (acHitter->AcIsPlayer())
				{
					_Radio.ClearMessages(MSG_PLAYERDAMAGELEVEL2,acHit);					//RJS 15Jun99
					mtype = MSG_PLAYERKILLREPORT;
				}
				else
					mtype = MSG_KILLREPORT;

				if (acHitter->nationality == NAT_RED)				//RJS 31Jan00
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_OTHERKILLED,mtype, acHitter, acHit, buddy));//RJS 31Jan00
				else
				{
					if (!acHitter->AcIsPlayer())
					{
						if (voxrandom < VOX_RAND_1IN32)
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_GOTHIM,mtype, acHitter, acHit, buddy));//RJS 31Jan00
						else
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_OTHERKILLED2,mtype, acHitter, acHit, buddy));//RJS 31Jan00
					}
					else
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_OTHERKILLED2,mtype, acHitter, acHit, buddy));//RJS 31Jan00
				}
			}
		}
	}
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PlayerSideBlewUp
//Author		R. Hyde
//Date			Thu 29 Apr 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::PlayerSideBlewUp(AirStrucPtr	ac)
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tPlayerSideBlewUp\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
	_Radio.damageMessage = true;
#endif
	if (ac->AcIsPlayer())										//RJS 01Jul99
	{
		ac->ai.radiosilent = FALSE;			//RJS 09Jun99
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERBLEWUP, (MsgType)(MSG_BLOWUPREPORT+MSG_HIPRIORITY), ac, ac, NULL));//RJS 31Jan00
	}
	else
	{
		if (Math_Lib.rnd() < VOX_RAND_1IN64)					//RJS 31Jan00
		{
			buddy = ac->FindBuddy();
			if (buddy)
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PETESAKE, MSG_BLOWUPREPORT, ac, ac, buddy));
		}
		else
		{
			buddy = ac->FindAcInGroup();
			if (buddy)
			{
				if (ac->PlayerInGroup())
				{
					if (!buddy->AcIsPlayer())
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLER_PLYRSIDEKILLED0, MSG_BLOWUPREPORT, buddy, ac, Persons2::PlayerSeenAC));
				}else
				{
					AirStruc*	lead = ((AirStruc*) buddy->leader);
					if (	(lead)
						&&	(lead->fly.leadflight)
						&&	(lead->fly.leadflight != ac)	)
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLER_PLYRSIDEKILLED0, MSG_BLOWUPREPORT, buddy, ac,lead ));
				}
			}
		}
	}
	if (!ac->uniqueID.commsmove)									  //AMM 07/07/99
		ac->ai.radiosilent = TRUE;			//RJS 09Jun99

#ifndef	NDEBUG
	_Radio.damageMessage = false;
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		OtherSideBlewUp
//Author		R. Hyde
//Date			Thu 29 Apr 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::OtherSideBlewUp(AirStrucPtr	ac,AirStrucPtr	acDead,bool	migdead)
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tOtherSideBlewUp\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);

	_Radio.damageMessage = true;
#endif
	if (ac)														//DAW 01Jun99
	{
		if (ac != acDead)
		{
			AlertGoHome(ac);

			if (Math_Lib.rnd() < VOX_RAND)						//RJS 01Jul99
			{
				if (!ac->AcIsPlayer())
				{
					buddy = ac->FindABuddyWithPlayerGivenPriority();
					if (buddy)
					{
						if (ac->ai.combatskill < SKILL_REGULAR)
						{
							_Radio.ClearMessages(MSG_DAMAGELEVEL2,acDead);			//RJS 15Jun99
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_BOASTKILL,MSG_BLOWUPREPORT, ac, acDead, buddy));//RDH 18May99

						}
						else
						{
								_Radio.ClearMessages(MSG_DAMAGELEVEL2,acDead);		//RJS 15Jun99
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_OTHERSIDEBLEW,MSG_BLOWUPREPORT, ac, acDead, buddy));//RDH 18May99
						}
					}
				}
			}
		}
		else
		{
			if (acDead->lasthitter.count)
			{
				ItemPtr	lasthitter = (ItemPtr)Persons2::ConvertPtrUID((UniqueID)acDead->lasthitter.count);
				if (lasthitter && (lasthitter->Status.size == AIRSTRUCSIZE))
					OtherSideBlewUp((AirStrucPtr)lasthitter,acDead,migdead);
			}
		}
	}

	if (!acDead->uniqueID.commsmove)								  //AMM 07/07/99
		acDead->ai.radiosilent = TRUE;			//RJS 09Jun99

#ifndef	NDEBUG
	_Radio.damageMessage = false;
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PlayerSidePiledIn
//Author		R. Hyde
//Date			Thu 29 Apr 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::PlayerSidePiledIn(AirStrucPtr	ac)
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tPlayerSidePiledIn\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);

	_Radio.damageMessage = true;
#endif
	if (ac)
	{
		if (Math_Lib.rnd() < VOX_RAND)							//RJS 01Jul99
		{
			if (ac->AcIsPlayer())
			{
				buddy = ac->FindBuddy();
				if (buddy)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PILEDINREP,MSG_BLOWUPREPORT, buddy, ac, ac));//RJS 31Jan00
			}
			else
			{
				if (ac->PlayerInGroup())
				{
					buddy = Manual_Pilot.ControlledAC2->FindBuddy();
					if (buddy && (buddy != ac))
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PILEDINREP,MSG_BLOWUPREPORTREPLY, buddy, ac, Manual_Pilot.ControlledAC2));//RJS 31Jan00
				}
			}
		}
	}

	if (ac && !ac->uniqueID.commsmove)									  //AMM 07/07/99
		ac->ai.radiosilent = TRUE;			//RJS 09Jun99

#ifndef	NDEBUG
	_Radio.damageMessage = false;
#endif
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SayWeapon
//Author		Robert Slater
//Date			Thu 13 May 1999
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::SayWeapon(SLong	index)
{
 	SLong	theScript = PHRASE_NONE;

 	switch (index & LT_MASK)
 	{
 	case LT_BULLET:
 		theScript = PHRASE_GUNS;
 	break;
	case LT_CANNON:
		theScript = PHRASE_CANNONS;										//RJS 26Jul00
	break;
 	case LT_ANYWEAPON:
 		theScript = PHRASE_GUNSANDCANNON;
 	break;
 	}

 	_Radio.TriggerMsg(MESSAGE_STRUC(theScript,MsgType(MSG_HIPRIORITY+MSG_DONTBUFFER),Manual_Pilot.ControlledAC2,NULL,NULL));	//RJS 23Aug00
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		AlertGoHome
//Author		Robert Slater
//Date			Mon 24 May 1999
//
//Description	Tell everyone to bugger off if there are no bandits left to kill.
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::AlertGoHome(AirStrucPtr	killer)
{
	int	bandcnt = 0;

	killer->FindBandit(bandcnt);
	if (!bandcnt)
	{
		AirStrucPtr	theLeader = killer->GroupLeader();
		if (theLeader)
		{
			AirStrucPtr	buddy = theLeader->FindABuddyWithPlayerGivenPriority();
			if (buddy)
			{

				int	theScript = SCRIPT_REGROUP;		//RERUN: init (rnd(4) always 0-3; silences uninit analysis)
				int	choice = Math_Lib.rnd(4);
				switch (choice)
				{
				case 0:
					theScript = SCRIPT_REGROUP;
					break;
				case 1:
					theScript = SCRIPT_ALLGOHOME;
					break;
				case 2:
					theScript = SCRIPT_SENDEVERYBODY;
					break;
				case 3:
					theScript = SCRIPT_HOMEWELLDONE;
					break;
				}

				_Radio.TriggerMsg(MESSAGE_STRUC(theScript,MSG_NOBANDITCALL_X,theLeader,buddy,buddy));
			}
		}
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EngineDamage
//Author		Robert Slater
//Date			Mon 31 Jan 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::EngineDamage()
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEngineDamage\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif

	if (Math_Lib.rnd() < VOX_RAND_1IN8)									//RJS 26Jun00
	{
		AirStrucPtr	buddy = acHit->FindBuddy();
		if (buddy)
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLEE_ENGINEDAMAGE,MSG_DAMAGELEVEL1,acHit,acHit,buddy));
	}
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		EngineDamageGlycol
//Author		Robert Slater
//Date			Mon 31 Jan 2000
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	DamageChat::EngineDamageGlycol()
{
#ifdef	_CHATTRACE_
	UWord cw=GETFPCW();
	::AfxTrace("\tEngineDamageGlycol\t%d\n",Math_Lib.GetRndCount());
	SETFPCW(cw);
#endif

	if (Math_Lib.rnd() < VOX_RAND_1IN8)									//RJS 26Jun00
	{
		AirStrucPtr	buddy = acHit->FindBuddy();
		if (buddy)
		{
			if (acHitIsFriend)
			{
				if (Math_Lib.rnd() < VOX_RAND_1IN4)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SEEGLYCOLYOURS,MSG_DAMAGELEVEL2,buddy,acHit,acHit));
				else
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SEEGLYCOLMINE,MSG_DAMAGELEVEL2,acHit,acHit,buddy));
			}
			else
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_GOTHIMLEAKING,MSG_DAMAGELEVEL2,acHitter,acHit,buddy));
		}
	}
}
