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

// RowanDialog: // FullPane.cpp : implementation file
//

#define F_SOUNDS
#define F_BATTLE
#define F_COMMON
#include "stdafx.h"
#include "mig.h"
#include <stdlib.h>
#include "FullPane.h"
#include "messages.h"
#include "MIGView.h"
#include "replay.h"
#include "winmove.h"
#include "aggrgtor.h"													//RDH 01/03/00
#include "campback.h"
#include "sairclms.h"
#include "globrefs.h"
#include "persons2.h"
#include "load.h"
#include	"miles.h"
#include	"bstream.h"
#include "overlay.h"
#include	"credits.h"
#include "package.h"
#include "uiicons.h"	
#include "stub3d.h"
#include	"sqddiary.h"
#include "radio.g"
#include "monotxt.h"


#ifdef _DEBUG
//#define new DEBUG_NEW
#ifndef	THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


bool	RFullPanelDial::incomms=false,RFullPanelDial::in3d=false,RFullPanelDial::specialevent=false;
RFullPanelDial::GameStates	RFullPanelDial::gamestate=RFullPanelDial::TITLE;
RFullPanelDial::GameSide		RFullPanelDial::gameside;
/////////////////////////////////////////////////////////////////////////////
// RFullPanelDial dialog


//
// If there are any conflicts in this lot the data is now in FPLayout.cpp
//
//
//
//
// The system stuff that used to be here is now in FullPSys.cpp
//
//
//


//************************************** YOUR PROCEDURES HERE


Bool RFullPanelDial::ConfirmExit(FullScreen*&fs)
{	
	CMainFrame* mainframe = ((CMainFrame*)AfxGetMainWnd());
	Save_Data.SavePreferences();
	Save_Data.ChangeMode(SaveData::UIR_FIRST);
	mainframe->m_doIExist=0;
	mainframe->DestroyWindow();

//	((CMainFrame*)AfxGetMainWnd())->OnBye();
	return FALSE;
}

Bool RFullPanelDial::IntroSmackInitForCredits()
{
	if (	FILEMAN.existnumberedfile(FIL_VID_INTRO))
		LaunchSmacker(0,FIL_VID_INTRO);
	LaunchDial(new	CCredits(IDS_CT_PROGRAMMINGTEAMTITLE),1);
	return TRUE;
}
Bool RFullPanelDial::IntroSmackInit()
{
	Save_Data.InitPreferences((intptr_t)Master_3d.winst);
	if (	FILEMAN.existnumberedfile(FIL_VID_INTRO))
		Save_Data.ChangeMode(SaveData::UIR_SMACKER);
	UpdateSize();
	_Replay.DeleteTempFiles();
	FILEMAN.namenumberedfile(FIL_VID_INTRO);
//	Master_3d.SmallWin();
	SetCursorPos(10,10);
	LaunchSmacker(0,FIL_VID_INTRO);
	return TRUE;
}
void RFullPanelDial::ResetLoadGameOption()
{
	CRListBox* prlistbox;
	prlistbox=GETDLGITEM(IDC_RLISTBOX);
	prlistbox->ReplaceString("",0,2);
}
void RFullPanelDial::ResetQuitOption()
{
	CRListBox* prlistbox;
	prlistbox=GETDLGITEM(IDC_RLISTBOX);
	prlistbox->ReplaceString("",0,0);
}

Bool RFullPanelDial::QuickMissionDesc()
{ //CCampBack
	SetTitleText();
	LaunchDial(new CCampBack(CSQuick1::quickdef.missiondesc, IDD_QUICKTEXT),1);
	return TRUE;
}
Bool RFullPanelDial::QuickMissionParameters()
{ //CCampBack
	LaunchDial(new QuickParameters(),1);
	return TRUE;
}

Bool RFullPanelDial::QuickMissionBlue()
{
	int	ilun[8][2]={{0}};
	int initind=0;
	for (int wave=0;wave<8;wave++)
		for (int grp=0;grp<3;grp++)
			if (CSQuick1::quickdef.line[0][wave][grp].flights||CSQuick1::quickdef.line[0][wave][grp].DutyFlags())
			{
				ilun[initind][0]=wave;
				ilun[initind][1]=grp;
				initind++;
			}
	const DialBox&	ND=DialBox::Null();

	LaunchDial(1,
		DialList(DialBox(FIL_NULL,new QuickMissionPanel),			
			DialList ( DialBox(FIL_NULL,new EmptyChildWindow, EDGES_NOSCROLLBARS_NODRAGGING),
				(initind>0)?DialBox(FIL_NULL,new CSQuickLine(0,ilun[0][0],ilun[0][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initind>1)?DialBox(FIL_NULL,new CSQuickLine(0,ilun[1][0],ilun[1][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initind>2)?DialBox(FIL_NULL,new CSQuickLine(0,ilun[2][0],ilun[2][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initind>3)?DialBox(FIL_NULL,new CSQuickLine(0,ilun[3][0],ilun[3][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initind>4)?DialBox(FIL_NULL,new CSQuickLine(0,ilun[4][0],ilun[4][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initind>5)?DialBox(FIL_NULL,new CSQuickLine(0,ilun[5][0],ilun[5][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initind>6)?DialBox(FIL_NULL,new CSQuickLine(0,ilun[6][0],ilun[6][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initind>7)?DialBox(FIL_NULL,new CSQuickLine(0,ilun[7][0],ilun[7][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND

			)));
			
	return TRUE;


}
Bool RFullPanelDial::QuickMissionRed()
{

	int	ilch[8][2]={{0}};
	int initindch=0;
	for (int wave=0;wave<8;wave++)
		for (int grp=0;grp<3;grp++)
			if (CSQuick1::quickdef.line[1][wave][grp].flights||CSQuick1::quickdef.line[1][wave][grp].DutyFlags())
			{
				ilch[initindch][0]=wave;
				ilch[initindch][1]=grp;
				initindch++;
			}
	const DialBox&	ND = DialBox::Null();

	LaunchDial(1,
		DialList(DialBox(FIL_NULL,new QuickMissionPanel),			
			DialList ( DialBox(FIL_NULL,new EmptyChildWindow, EDGES_NOSCROLLBARS_NODRAGGING),
				(initindch>0)?DialBox(FIL_NULL,new CSQuickLine(1,ilch[0][0],ilch[0][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initindch>1)?DialBox(FIL_NULL,new CSQuickLine(1,ilch[1][0],ilch[1][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initindch>2)?DialBox(FIL_NULL,new CSQuickLine(1,ilch[2][0],ilch[2][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initindch>3)?DialBox(FIL_NULL,new CSQuickLine(1,ilch[3][0],ilch[3][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initindch>4)?DialBox(FIL_NULL,new CSQuickLine(1,ilch[4][0],ilch[4][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initindch>5)?DialBox(FIL_NULL,new CSQuickLine(1,ilch[5][0],ilch[5][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initindch>6)?DialBox(FIL_NULL,new CSQuickLine(1,ilch[6][0],ilch[6][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND,
				(initindch>7)?DialBox(FIL_NULL,new CSQuickLine(1,ilch[7][0],ilch[7][1]),EDGES_NOSCROLLBARS_NODRAGGING):ND
				)));
			
	return TRUE;
}

Bool RFullPanelDial::QuickMissionInit()
{
	LaunchDial(0,new CSQuick1);

#ifndef	BOB_DEMO_VER
	ChangeFamilyAnim();
#endif

	return QuickMissionDesc();
		
	return TRUE;
}
Bool RFullPanelDial::BoBFragInit()
{
	LaunchDial(new BoBFrag,0);
	if (incomms)
		LaunchDial(new CommsChat,1);


		
	return TRUE;
}

#ifndef	BOB_DEMO_VER

Bool RFullPanelDial::QuickViewInit()
{
	DialBox	Top(FIL_NULL,new CQuickView);
	int	ilun[8][2]={{0}};
	int initind=0;


	{
	for (int wave=0;wave<8;wave++)
		for (int grp=0;grp<3;grp++)
			if (CSQuick1::quickdef.line[1][wave][grp].flights)	//AMM 10Aug00
			{
				ilun[initind][0]=wave;
				ilun[initind][1]=grp;
				initind++;
			}
	}

	int	ilch[8][2]={{0}};
	int initindch=0;
	for (int wave=0;wave<8;wave++)
		for (int grp=0;grp<3;grp++)
			if (CSQuick1::quickdef.line[0][wave][grp].flights)	//AMM 10Aug00
			{
				ilch[initindch][0]=wave;
				ilch[initindch][1]=grp;
				initindch++;
			}
	DialBox*	ND=NULL;

	LaunchDial(0,
		DialList(Top,			
			DialList ( DialBox(FIL_NULL,new EmptyChildWindow, EDGES_NOSCROLLBARS_NODRAGGING),
				(initind>0)?DialBox(FIL_NULL,new CViewQList(1,ilun[0][0],ilun[0][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initind>1)?DialBox(FIL_NULL,new CViewQList(1,ilun[1][0],ilun[1][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initind>2)?DialBox(FIL_NULL,new CViewQList(1,ilun[2][0],ilun[2][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initind>3)?DialBox(FIL_NULL,new CViewQList(1,ilun[3][0],ilun[3][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initind>4)?DialBox(FIL_NULL,new CViewQList(1,ilun[4][0],ilun[4][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initind>5)?DialBox(FIL_NULL,new CViewQList(1,ilun[5][0],ilun[5][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initind>6)?DialBox(FIL_NULL,new CViewQList(1,ilun[6][0],ilun[6][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initind>7)?DialBox(FIL_NULL,new CViewQList(1,ilun[7][0],ilun[7][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND
				),
			DialList ( DialBox(FIL_NULL,new EmptyChildWindow,EDGES_NOSCROLLBARS_NODRAGGING),
				(initindch>0)?DialBox(FIL_NULL,new CViewQList(0,ilch[0][0],ilch[0][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initindch>1)?DialBox(FIL_NULL,new CViewQList(0,ilch[1][0],ilch[1][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initindch>2)?DialBox(FIL_NULL,new CViewQList(0,ilch[2][0],ilch[2][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initindch>3)?DialBox(FIL_NULL,new CViewQList(0,ilch[3][0],ilch[3][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initindch>4)?DialBox(FIL_NULL,new CViewQList(0,ilch[4][0],ilch[4][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initindch>5)?DialBox(FIL_NULL,new CViewQList(0,ilch[5][0],ilch[5][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initindch>6)?DialBox(FIL_NULL,new CViewQList(0,ilch[6][0],ilch[6][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND,
				(initindch>7)?DialBox(FIL_NULL,new CViewQList(0,ilch[7][0],ilch[7][1]),EDGES_NOSCROLLBARS_NODRAGGING):*ND
			)));
			
	return TRUE;
}

#endif	//BOB_DEMO_VER

//extern	ULong	GR_Scram_Alt[],GR_Scram_Squad[],
//				GR_Scram_Skill[],GR_Scram_AC[],GR_Pack_PT_Used[];
//				GR_Pack_Target,GR_Pack_PlayerSquad,GR_Pack_PlayerPos;
static RDialog* flybox;


void	RFullPanelDial::ChildDialClosed(int,RDialog* d,int l)
{
	if (localnote)
		(this->*localnote)(d,l);
	

}
Bool RFullPanelDial::StartFlying()
{	 //First copy over all the info from the CSQuick1::quickdef to globrefs
	//Now need the code in scramble...
	bool fIsRunning=in3d;
	in3d=true;
//	Todays_Packages[0][0][0].uid=UID_NULL;
#ifndef NDEBUG
//#define PLACEMENT	CRect(0,0,400,300)
//#define PLACEMENT	CRect(0,0,512,384)
//#define PLACEMENT	CRect(0,0,640,480)
//#define PLACEMENT	CRect(0,0,800,600)
#define PLACEMENT	CRect(0,0,1024,768)
//#define PLACEMENT	CRect(0,0,1026,770)  
#else
#define	PLACEMENT	Place(POSN_CENTRE,POSN_CENTRE, 0, 0)
#endif
	OverLay.loader_art=FIL_LOADING;
	LogChild(0,
		flybox=MakeTopDialog(PLACEMENT,DialBox(OverLay.loader_art,new Rtestsh1(NULL,fIsRunning),EDGES_NOSCROLLBARS)));

#undef PLACEMENT 
	localnote=(&RFullPanelDial::OnFlyingClosed);
	return TRUE;
}

bool RFullPanelDial::OnFlyingClosed(RDialog*,int rv)
{

	FullScreen* s=NULL;

	if (rv==IDCANCEL)
	{
//		if (!Inst3d::fTaskSwitched)
			LaunchScreen(&options3d);
//		_Replay.MenuViewPlayback=false;
	}
	else
	{
		if (!_Replay.RestorePosition)							//AMM 18Feb99
		{														//AMM 18Feb99
			strcpy(_Replay.pfilename,Save_Data.lastreplayname);	//AMM 18Feb99
		}														//AMM 18Feb99

		in3d=false;

#ifndef	BOB_DEMO_VER
		CMainFrame* main=(CMainFrame*)AfxGetMainWnd();
		main->m_titlebar.Redraw();

		if (_Replay.RestorePosition)
		{
			LaunchScreen(&replaysave);
		}
		else if (_Replay.MenuViewPlayback)
		{
			LaunchScreen(&replayload);
			_Replay.QMPlayback=false;
		}
		else
		if (gamestate==HOT || gamestate==QUICK)
		{	
			if (!_Replay.QMPlayback)
			{
				if (_DPlay.PlayerCreated && !_DPlay.Host)
				{
					FullScreen* fs;
					SelectReadyRoom(fs);
					LaunchScreen(fs);
				}
				else
#endif
				{
					LaunchScreen(&quickmissiondebrief);
				}
#ifndef	BOB_DEMO_VER
			}
			else
			{
				_Replay.QMPlayback=false;
				LaunchScreen(&replaysave);
			}
		}
		else
		if (gamestate==MATCH)
		{
			_Replay.QMPlayback=false;
			FullScreen* fs;
			SelectReadyRoom(fs);
			LaunchScreen(fs);
		}else
		{
			_Replay.QMPlayback=false;
			if (!_Replay.MapPlayback)
			{
				CMainFrame* main=(CMainFrame*)AfxGetMainWnd();

				if (_DPlay.PlayerCreated) //AMM 17/05/99
				{ //AMM 17/05/99
					if (_DPlay.Host) //AMM 17/05/99
					{
						_DPlay.SendStatusChange(DPlay::CPS_MAP);
					}
					else //AMM 17/05/99
					{
						_DPlay.SendStatusChange(DPlay::CPS_DEBRIEF);
					}
				} //AMM 17/05/99

				main->m_toolbar1.SetFiltersFromSaveGame();
				m_pView->m_mapdlg.Invalidate();

				main->m_titlebar.Redraw();

				MMC.NextMission();
				Persons4::StartUpMapWorld(true);

				{
					LaunchMap(s, true);
				}
			}
			else
			{
				Persons4::StartUpMapWorld(true,"rbackup.dat");
				_Replay.MapPlayback=false;
				LaunchMap(s, true);
			}
		}								
#endif
	}
	return true;
}

#ifndef	BOB_DEMO_VER

Bool	RFullPanelDial::Pseudo3D(FullScreen*&fs)
{
	FragFly(fs);

//	INT3;	//SET UP YOUR RETURN VALUES NOW!!!!

//##test data begins

//	MMC.DisDayEntry(Debrief::TARGUID, UID_BrRlYalu0, Debrief::T_AttackWest03_H);
//	MMC.DisDayEntry(Debrief::DISCARDTARGUID, UID_BrRdYalu, Debrief::T_DeceptionFound00_H);

//	MMC.DisDayEntry(Debrief::NAME, UID_BrRdYalu, Debrief::T_ByPassBuilt_H);





//##test data ends

	//Drag the following to a watch window:
	//	gamestate specialevent incomms
	//	SAVE_DATA
	//	MMC Miss_Man.camp
	//	Supply_Tree					fileblock
	OnFlyingClosed(this,0);
	return FALSE;
}
Bool	RFullPanelDial::FragFly(FullScreen*&fs)
{
	if (_DPlay.PlayerCreated)
	{
		if (_DPlay.Host)
		{
// send fly now message to guests
			if (!_DPlay.UINetworkSelectFly())
				return FALSE;
		}
		else
		{
// if game in progress then join, otherwise dont do anything
			if (!_DPlay.FlyNowFlag)
			{
				if (DPlay::H2H_Player[0].status==DPlay::CPS_3D)
				{
					if (!_DPlay.JoinGame())
					{
						_DPlay.SendStatusChange(DPlay::CPS_FRAG);
						return FALSE;			
					}
				}
				else
				{
					return FALSE;
				}
			}
		}
	}
		MMC.msgwithsmacker = 0;											//RDH 26/06/99
		MMC.smacker = FIL_NULL;

///CANT DO THIS HERE!!		PrepareForFrag(fs);										//AMM 17Nov98 //JIM 21/12/98
///This stomps quickmission into Todays_Packages,
///but from campaign I already have Todays_Packages set up.
///Also, we are about to enter the 3d. This is far too late!

//		if (incomms && _DPlay.GameType!=COMMSCAMPAIGN)
//			PrepareForFrag(fs);


// this saves out binary bfield from map. needs transmitting to other players before
// they arrive at the frag screen so that ac are set up correct
// put somewhere else
//
//		if (!incomms											//AMM 17Nov98
//		|| (_DPlay.Host && _DPlay.GameType==COMMSCAMPAIGN)		//AMM 17Nov98
//		)														//AMM 17Nov98
//		{														//AMM 17Nov98
//			Todays_Packages.SaveBin();							
//		}														//AMM 17Nov98
//CANT DO IT HERE... You have just stuffed illegals into UID ptrs!!!

// need to save out a dummy save game here in case record is started
// in middle of mission

		if (Todays_Packages[0].packagestatus!=Profile::PS_SPARE)
		{
			MMC.playersquadron=Todays_Packages[0][0].squadnum;
			MMC.playeracnum=0;
		}
		if (!_DPlay.PlayerCreated && _Replay.IsCampState(gamestate))
		{
			_Replay.SaveTempSaveGame(_Replay.replaysavebackup);
		}

		return(TRUE);

}

#endif

Bool	RFullPanelDial::FragFly2(FullScreen*&fs)
{
	if (MMC.playersquadron==-1)
		return FALSE;													//JIM 12Oct00
	if (!_DPlay.PlayerCreated && _Replay.IsCampState(gamestate))
	{
		_Replay.SaveTempSaveGame(_Replay.replaysavebackup);
	}

	return TRUE;
}
Bool	RFullPanelDial::FragBackupOptions(FullScreen*&fs)
{
	pdial[0]->RefreshData(-1);
	return TRUE;
}
Bool	RFullPanelDial::ReturnToMap(FullScreen*&fs)
{
	if (gamestate==QUICK)
		return TRUE;
	_DPlay.SendStatusChange(DPlay::CPS_MAP);
#ifndef	BOB_DEMO_VER
	LaunchMap(fs, false);
#endif
	return FALSE;
}

#ifndef	BOB_DEMO_VER

Bool	RFullPanelDial::ExitLastDay(FullScreen*&fs)
{
		CloseVideo();
		Save_Data.ChangeMode(SaveData::UIR_SCRAMBLE);
		return TRUE;

}
Bool	RFullPanelDial::ReturnToMapAfterReview(FullScreen*&fs)
{
	if (Node_Data.review.messages[Node_Data.review.latest].title ==  IDS_PHRASE_TITLE_FIGHTERCOMMANDEXHAUSTED)
	{
		fs=&title;
	}else{
		_DPlay.SendStatusChange(DPlay::CPS_MAP);
		Persons4::StartUpMapWorld();

		LaunchMap(fs, false);
	}
	return FALSE;
}

bool RFullPanelDial::CommsPlayer(int pos, int& i, int squadnum)
{
	if (!RFullPanelDial::incomms)										//JIM 19/05/99
		return false;													//JIM 19/05/99
	i = 0;
	while	(		(i < MAXPLAYERS)							//AMM 04Nov98
			)
	{
		if	(		(DPlay::H2H_Player[i].squadron == squadnum)
			&&		(DPlay::H2H_Player[i].position == pos)
			&&		(DPlay::H2H_Player[i].status!=DPlay::CPS_NONE)		//AMM 04Nov98

			)
			return (true);
		i++;
	}
	return (false);
}
bool RFullPanelDial::DuplicateCommsPlayer(int pos, int player, int squadnum)
{
	int i = 0;
	while	(		(i < player)
			)
	{
		if	(		(DPlay::H2H_Player[i].squadron == squadnum)
			&&		(DPlay::H2H_Player[i].position == pos)

			)
			return (true);
		i++;
	}
	return (false);
}


#endif	//BOB_DEMO_VER

Bool RFullPanelDial::QuitFlying(FullScreen*&fs)
{
// mainly for testing in debug, when guests click on Fly in a game and
// it activates the debrief option....
	if (_DPlay.PlayerCreated)											//AMM 8Aug00
		return FALSE;													//AMM 8Aug00

	flybox->PostMessage(WM_COMMAND,IDOK,NULL);
	return FALSE;
}


Bool RFullPanelDial::QuickMissionDebriefInit()
{
	CRListBox* prlistbox;
	prlistbox=GETDLGITEM(IDC_RLISTBOX);
#ifndef	BOB_DEMO_VER
	if (incomms)
		prlistbox->ReplaceString("",0,3);
	else
		prlistbox->ReplaceString(RESSTRING(REPLAY),0,3);
#endif

//	CRListBox* prlistbox;
//	prlistbox=GETDLGITEM(IDC_RLISTBOX);
	prlistbox->SetHilightCol(1);
	prlistbox->SetHilightRow(0);

	LaunchDial(new 	RAFCombat(0, false),0);

	return TRUE;
}
Bool RFullPanelDial::QuickMissionDebriefGrndInit()
{
	CRListBox* prlistbox;
	prlistbox=GETDLGITEM(IDC_RLISTBOX);
#ifndef	BOB_DEMO_VER
	if (incomms)
		prlistbox->ReplaceString("",0,3);
	else
		prlistbox->ReplaceString(RESSTRING(REPLAY),0,3);
#endif

//	CRListBox* prlistbox;
//	prlistbox=GETDLGITEM(IDC_RLISTBOX);
	prlistbox->SetHilightCol(2);
	prlistbox->SetHilightRow(0);


	currDiaryEntry = 0;													//RJS 4Sep00



	Diary::Player*	pptr = Squad_Diary.GetFirstPlayer();
 	SquadronBase*	squad;
 	squad =	Squad_Diary.GetSquadron(pptr);


 	if (RFullPanelDial::gameside != RFullPanelDial::SIDE_LW)
	{
		sq = (SquadNum)squad->squadnum;
		sqdetails[0]  = Squad_Diary.FindLast(sq);
		sqdetails[1]  = NULL;
		LaunchDial(new RAFDiaryDetails(&sq, &currDiaryEntry, sqdetails,true),0);	//RJS 4Sep00
	}else
	{
		sq = (SquadNum)squad->squadnum;
		sqdetails[0]  = Squad_Diary.FindLast(sq);
		sqdetails[1]  = NULL;
		LaunchDial(new LWDiaryDetails(&sq, &currDiaryEntry, sqdetails,true),0);	//RJS 4Sep00
	}
	return TRUE;
}

Bool RFullPanelDial::ReplayLoadInit()
{
	if (_Replay.IsCampState(gamestate))
	{
		_Replay.BackupSaveGame();
	}

	CString	wildcard="*.cam";
	selectedfile=Save_Data.lastreplayname;
	strcpy(_Replay.pfilename,selectedfile);

	LaunchDial(new	CLoad(IDDX_SREPLAY,LSD_LOAD,	FIL_VIDEOS_START_DIR,wildcard,selectedfile),0);

//	LaunchDial(new CSReplay,0);
	return TRUE;
}
Bool RFullPanelDial::ReplaySaveInit()
{
	if (_Replay.IsCampState(gamestate))
	{
		_Replay.BackupSaveGame();
	}


	CRListBox* prlistbox;
	prlistbox=GETDLGITEM(IDC_RLISTBOX);
	if (_Replay.RestorePosition)
		prlistbox->ReplaceString("",0,2);
	else
		prlistbox->ReplaceString(RESSTRING(VIEW),0,2);

	CString	wildcard="*.cam";
	selectedfile=Save_Data.lastreplayname;

	if (!_Replay.RestorePosition)
	{
//		strcpy(_Replay.pfilename,selectedfile);
		strcpy(_Replay.pfilename,"replay.dat");
	}


	LaunchDial(new	CLoad(IDDX_SAVEREPLAY,LSD_SAVE,	FIL_VIDEOS_START_DIR,wildcard,selectedfile),0);

//	LaunchDial(new CSReplay,0);
	return TRUE;
}

#ifndef	BOB_DEMO_VER
Bool RFullPanelDial::CampaignSelectInit()
{
	CRListBox* prlistbox;
	prlistbox=GETDLGITEM(IDC_RLISTBOX);
	prlistbox->SetHilightRow(0);										//RDH 19/05/99
	prlistbox->SetHilightCol(1);										//RDH 19/05/99

	LaunchDial(new CSCampaign,0);
	ChangeCamp(0);
	return TRUE;
} 
Bool RFullPanelDial::CampaignEnterNameInit()
{
	CRListBox* prlistbox;
	prlistbox=GETDLGITEM(IDC_RLISTBOX);
	prlistbox->SetHilightRow(0);										//RDH 19/05/99
	prlistbox->SetHilightCol(1);										//RDH 19/05/99
 	Node_Data.BuildTargetTable();
 	Node_Data.LoadCleanNodeTree();
	MMC.fav.flyon=0;
	MMC.fav.flyonfav=-1;
	if (RFullPanelDial::gamestate==RFullPanelDial::PILOT)
	{
		LaunchDial(new ControlFly,0);
		LaunchDial(new CampaignEnterName(whichcamp),1);
	}
	else
	{
		LaunchDial(new CampaignEnterName(whichcamp),1);
		LaunchSmacker(2,FileNum(FIL_VID_PHASE1BEGINS+whichcamp));
	}

	return TRUE;
}

Bool RFullPanelDial::EndDayReviewInit()
{
	LaunchDial(new EndOfDayReviewList,0);
	ChangeReview(Node_Data.review.latest);
	return TRUE;
}
Bool RFullPanelDial::EndWarInit()
{
	Save_Data.ChangeMode(SaveData::UIR_SMACKER);
	int	titletext = Node_Data.review.MessageTitle(Node_Data.review.latest);
	int offset = titletext - IDS_PHRASE_TITLE_MONUMENTSURVIVES;
 	File_Man.DiscardDirList(FileNum(FIL_VID_MONUMENTSURVIVES+offset));
	File_Man.ExistNumberedFile(FileNum(FIL_VID_MONUMENTSURVIVES+offset));
	LaunchSmacker(0,FileNum(FIL_VID_MONUMENTSURVIVES+offset));		//JIM 2Oct00
  	return TRUE;

}

Bool RFullPanelDial::CampaignOverInit()
{
//	Miss_Man.currcampaignnum = MissMan::SCRAMBLECAMPAIGN;		//RDH 16Apr96
//	Miss_Man.camp = Miss_Man.campaigntable[MissMan::SCRAMBLECAMPAIGN];

//	LaunchDial(new CSCampaign,0);
	return TRUE;
}

Bool RFullPanelDial::CampaignStartInit()
{
	LaunchSmacker(1,FIL_VID_INTRO);
//	LaunchDial(new CCampBackEntireWar(IDS_CAMPDESC0),2);
	return TRUE;
}

Bool RFullPanelDial::LockerRoomInit()
{
	LaunchDial(new CLockerRoom,0);
	return TRUE;
}
Bool RFullPanelDial::LoadCampaign(FullScreen*&fs)
{
			return FALSE;

}
bool RFullPanelDial::LoadCampaignData()
{
	return TRUE;
}

Bool RFullPanelDial::SetUpRafLoadGame(FullScreen*&fs)
{
	CString	wildcard="*.bsr";
	selectedfile=Save_Data.lastsavegame;
	if (pdial[0])
	{
		pdial[0]->PreDestroyPanel();
		pdial[0]->DestroyPanel();
		pdial[0]=NULL;
	}

	LaunchDial(new	CLoad(IDDX_LOADFULL,LSD_LOAD,			FIL_SAVEGAMEDIR,wildcard,selectedfile),0);

	return TRUE;

}
Bool RFullPanelDial::SetUpLoadGame()
{	FullScreen* fs=NULL;
	SetUpRafLoadGame(fs);
	CRListBox* prlistbox;
	prlistbox=GETDLGITEM(IDC_RLISTBOX);
	prlistbox->SetHilightCol(0);
	prlistbox->SetHilightRow(0);

	return TRUE;

}

Bool RFullPanelDial::SetUpLWLoadGame(FullScreen*&fs)
{
	CString	wildcard="*.bsl";
	selectedfile=Save_Data.lastsavegame;
	if (pdial[0])
	{
		pdial[0]->PreDestroyPanel();
		pdial[0]->DestroyPanel();
		pdial[0]=NULL;
	}
	LaunchDial(new	CLoad(IDDX_LOADFULL,LSD_LOAD,			FIL_SAVEGAMEDIR,wildcard,selectedfile),0);

	return TRUE;

}
Bool RFullPanelDial::SelectServiceInit()
{
	LaunchDial(new CSelectService,0);
	return TRUE;
}

Bool RFullPanelDial::SelectSessionInit()
{
	LaunchDial(new CSelectSession,0);
	return TRUE;
}

Bool RFullPanelDial::VisitorsBookInit()
{
	LaunchDial(new CVisitorsBook,0);
	return TRUE;
}

Bool RFullPanelDial::ReadyRoomInit()
{
	LaunchDial(new CReadyRoom,0);
	LaunchDial(new CommsChat,1);

	return TRUE;
}

Bool RFullPanelDial::FragInit()
{
	return TRUE;
}

Bool RFullPanelDial::RadioInit()
{
	LaunchDial(new CRadio,0);
	return TRUE;
}

#endif //BOB_DEMO_VER

Bool RFullPanelDial::VariantsInit()
{
	return TRUE;
}					
void RFullPanelDial::ResetVariantOption()
{
}

#ifndef BOB_DEMO_VER

Bool RFullPanelDial::PaintShopDesc()
{
	if (pdial[0])
	{
		pdial[0]->PreDestroyPanel();
		pdial[0]->DestroyPanel();
		pdial[0]=NULL;
	}
	if (pdial[1])
	{
		pdial[1]->PreDestroyPanel();
		pdial[1]->DestroyPanel();
		pdial[1]=NULL;
	}
//TEMPCODE JIM 17/05/99	if (pdial[2])
//TEMPCODE JIM 17/05/99	{
//TEMPCODE JIM 17/05/99		pdial[2]->PreDestroyPanel();
//TEMPCODE JIM 17/05/99		pdial[2]->DestroyPanel();
//TEMPCODE JIM 17/05/99		pdial[2]=NULL;
//TEMPCODE JIM 17/05/99	}

	if (DPlay::H2H_Player[_DPlay.mySlot].squadron	< PT_GER_FLYABLE)
	{
		LaunchDial(new CCommsPaint,0);
		LaunchDial(new CCurrEmblem,1);
	}
	return TRUE;
}
	
Bool RFullPanelDial::PaintShopInit()
{
//	LaunchDial(new CPaint(0,0),0);
	if	(_DPlay.GameType <= DPlay::TEAMPLAY)
		LaunchDial(new CCommsDeathMatchAc,2);


	return PaintShopDesc();
//	DialBox	Top(FIL_NULL,new CCommsPaint);
//
//	LaunchDial(0,
//		DialList(Top,			
//			DialList ( DialBox(FIL_NULL,new EmptyChildWindow, EDGES_NOSCROLLBARS_NODRAGGING),
//				DialBox(FIL_NULL,new CCurrEmblem,EDGES_NOSCROLLBARS_NODRAGGING)
//					)));
	
	return TRUE;
}

#endif	//#ifndef BOB_DEMO_VER

Bool RFullPanelDial::ReplayLoad(FullScreen*&fs)
{

//	pdial[0]->DestroyPanel();
//	pdial[0]=NULL;

//	if (!_Replay.LoadReplayData())
//		_Error.EmitSysErr("Error: Could not load replay file");

	//this is the file to load
	//use CFiling::LoadGame to see how to code the load
	CString file =selectedfile;
	Save_Data.lastreplayname=selectedfile;

	char* temp;
	temp=file.GetBuffer(0);

	_Replay.LoadReplayData(temp);

// load also views now....

	if (!_Replay.ValidReplayFile())
	{
		RMessageBox(IDS_BOB,IDS_INVALIDREPLAYFILE,NULL,IDS_OK);
		return FALSE;
	}

	_Replay.Playback=TRUE;
	_Replay.Record=FALSE;

// need to initialise some sort of miss_man structure for now

	Miss_Man.currcampaignnum = MissMan::SCRAMBLECAMPAIGN;		//RDH 16Apr96
	Miss_Man.camp = Miss_Man.campaigntable[MissMan::SCRAMBLECAMPAIGN];
	Node_Data.BuildTargetTable();
	SetTitleText(true);

	return TRUE;
}
Bool RFullPanelDial::ReplaySave(FullScreen*&fs)
{
//	if (!_Replay.SaveReplayData())
//		_Error.EmitSysErr("Error: Could not save replay file");

	//this is the file to load
	CString file =selectedfile;

	Save_Data.lastreplayname=selectedfile;

	char* temp;
	temp=file.GetBuffer(0);
	
	_Replay.SaveReplayData(temp);

	return TRUE;
}

Bool RFullPanelDial::ReplayView(FullScreen*&fs)
{
	if (_Replay.RestorePosition)
		return FALSE;

	if (!_Replay.MenuViewPlayback)
	{
		strcpy(_Replay.pfilename,"replay.dat");
		_Replay.QMPlayback=true;
	}

	if (!_Replay.ValidReplayFile())
	{
		RMessageBox(IDS_BOB,IDS_INVALIDREPLAYFILE,NULL,IDS_OK);
		return FALSE;
	}

	_Replay.Playback=TRUE;
	_Replay.Record=FALSE;

// need to initialise some sort of miss_man structure for now

	Miss_Man.currcampaignnum = MissMan::SCRAMBLECAMPAIGN;		//RDH 16Apr96
	Miss_Man.camp = Miss_Man.campaigntable[MissMan::SCRAMBLECAMPAIGN];
	Node_Data.BuildTargetTable();
	SetTitleText(true);

//	gamestate=QUICK;
//	flybox=MakeTopDialog(CRect(50,50,690,530),DialBox(FIL_TITLE_640,new Rtestsh1()));
//	LogChild(0,flybox);
//	localnote=(OnFlyingClosed);
	
	return TRUE;
}

Bool RFullPanelDial::ExitVariant(FullScreen*&fs)
{
#ifndef	BOB_DEMO_VER
	if (incomms)
	{
		if (gamestate==QUICK)
			fs=&commsquick;
		else
			fs = &paintshop;

	}else
#endif
	{
		fs=&quickmission;
	}
	return TRUE;

}

#ifndef	BOB_DEMO_VER

Bool RFullPanelDial::StartComms(FullScreen*&fs)
{
	_Replay.Record=FALSE;
	_Replay.Playback=FALSE;

	_Replay.BackupPrefs(true);										//AMM 25May99
	_DPlay.BackupHostPrefs();

// reset quick mission params in case we have been to single player first

	CSQuick1::currquickmiss=-1;											//AMM 18Sep00
	CSQuick1::currquickfamily=-1;										//AMM 18Sep00
	_DPlay.ClearPackages();												//AMM 18Sep00



	if (_DPlay.StartCommsSession())
		return TRUE;
	RMessageBox(IDS_BOB,IDS_NOTCONNECTED,NULL,IDS_OK);

	return FALSE;
}

Bool RFullPanelDial::GetSessions(FullScreen*&fs)
{
//	if (strcmp("Empire Lobby",&_DPlay.ServiceName[0]))
	{
		_DPlay.UISelectServiceProvider(&_DPlay.ServiceName[0]);
//		_DPlay.UIGetSessionListUpdate();
		if (!_DPlay.UIGetSessionListUpdate())
		{
// problem initialising connection, exit to main menu

			_DPlay.ExitDirectPlay();

			if (_DPlay.StartCommsSession())
			{
				fs=&selectservice;
			}
			else
			{
				fs=&title;
			}
		}
		if (_DPlay.singlesession)
		{
			if (_DPlay.FindGoodSession())
			{
				if (_DPlay.JoinComms())
				{
					fs=&multiplayer;
					return TRUE;
				}
			}

			_DPlay.CommsMessage(IDS_BADSESSIONS2);
			return FALSE;

		}
	}
	return TRUE;
}

Bool RFullPanelDial::CreateCommsGame(FullScreen*&fs)
{
	_DPlay.UISelectServiceProvider(&_DPlay.ServiceName[0]);
	_DPlay.UIPlayerType = DPlay::PLAYER_HOST;
	return TRUE;
}

Bool RFullPanelDial::JoinCommsGame(FullScreen*&fs)
{
//	Bool res;

	return (_DPlay.JoinComms());

}

//Bool RFullPanelDial::VisitorsBook(FullScreen*&fs)
//{
//	pdial[0]->DestroyPanel();
//	pdial[0]=NULL;
//
//	Bool res;
//
//	if (_DPlay.UIPlayerType==PLAYER_GUEST)
//	{	
//		res=_DPlay.UINewPlayer(&_DPlay.PlayerName[0],&_DPlay.SessionName[0]);
//		
//		if (res)
//		{
//			_DPlay.SendUpdateToVisitorsBook(&_DPlay.PlayerName[0],FALSE);
//		}
//		else
//		{
//			MessageBox("Could not create session or player","Whoops",MB_OK);
//		}
//
//		_DPlay.DestroyAndClose(_DPlay.myDPlayID);
//	}
//
//	LaunchDial(new CLockerRoom,0);
//	return FALSE;
//}
Bool RFullPanelDial::ResetMessages(FullScreen*&fs)
{
	_DPlay.InitMessages();
	fs = &radio;
	return TRUE;

}
Bool RFullPanelDial::SelectReadyRoomFromRadio(FullScreen*&fs)
{
	pdial[0]->OnOK();
	return(SelectReadyRoom(fs));
}
Bool RFullPanelDial::SelectReadyRoom(FullScreen*&fs)
{
	return SelectReadyRoomStatic(fs);
}
Bool RFullPanelDial::SelectReadyRoomStatic(FullScreen*&fs)
{


	switch (_DPlay.GameType)
	{
		case DPlay::TEAMPLAY:
		case DPlay::DEATHMATCH:
		{
			if	(_DPlay.Side)
			{
				if	(_DPlay.UIPlayerType==DPlay::PLAYER_HOST)
					fs=&readyroomhostmatch;
				else
					fs=&readyroomguestmatch;
			}else
			{
				if	(_DPlay.UIPlayerType==DPlay::PLAYER_HOST)
					fs=&readyroomhostmatchred;
				else
					fs=&readyroomguestmatchred;
			}
			break;
		}
		case DPlay::COMMSQUICKMISSION:
		{
			if	(_DPlay.Side)
			{
				if	(_DPlay.UIPlayerType==DPlay::PLAYER_HOST)
					fs=&readyroomhostqmission;
				else
					fs=&readyroomguestqmission;
			}else
			{
				if	(_DPlay.UIPlayerType==DPlay::PLAYER_HOST)
					fs=&readyroomhostqmissionred;
				else
					fs=&readyroomguestqmissionred;

			}
			break;
		}
	}
	return TRUE;
}
Bool RFullPanelDial::CreatePlayer(FullScreen*&fs)
{
	incomms=true;
//??rdh	gamestate=QUICK;	//need to set QUICK, MATCH, or CAMP
						//need to get mission description from host to slaves
	switch (_DPlay.GameType)
	{
		case DPlay::TEAMPLAY:
		{
			gamestate = MATCH;
			break;
		}
		case DPlay::DEATHMATCH:
		{
			gamestate = MATCH;
			break;
		}
		case DPlay::COMMSQUICKMISSION:
		{
			gamestate = QUICK;
			break;
		}
	}

	pdial[0]->DestroyPanel();
	pdial[0]=NULL;

	ULong res;
	if (_DPlay.UIPlayerType==DPlay::PLAYER_HOST)
	{
// host now creates session at start of ready room

		res=DP_OK;

	}
	else
	{
// already in session

//		res=_DPlay.UINewPlayer(&_DPlay.PlayerName[0],&_DPlay.SessionName[0]);

//		if (res)
		{
// password check

			res=_DPlay.AttemptToJoin();

			if (res==DP_OK)
			{
				res=_DPlay.SetUpPlayerInfo(&_DPlay.PlayerName[0]);

				if (res)
					res=DP_OK;
				else
					res=DPERR_INVALIDPARAM;


				if (res!=DP_OK)
				{
					RMessageBox(IDS_BOB,IDS_NOTCONTACTHOST,NULL,IDS_OK);
				}
			}
			else
			{
				RMessageBox(IDS_BOB,IDS_INCORRECTPASSWORD,NULL,IDS_OK);
			}
		}
//		else
//		{
//			MessageBox("Could not create session or player","Whoops",MB_OK);
//		}
	}


	if (res==DP_OK)
	{
		{
			Miss_Man.currcampaignnum = MissMan::SCRAMBLECAMPAIGN;		//RDH 16Apr96
			Miss_Man.camp = Miss_Man.campaigntable[MissMan::SCRAMBLECAMPAIGN];
			MMC.currtime=28000;
			Node_Data.BuildTargetTable();
			Node_Data.LoadCleanNodeTree();

			strncpy(MMC.PlayerName,DPlay::H2H_Player[_DPlay.mySlot].name,PLAYERNAMELEN-1); MMC.PlayerName[PLAYERNAMELEN-1]=0; //AMM 21/01/00
			SetTitleText();
		}


// should be unassigned initially

		MMC.playeracnum=-1;
		MMC.playersquadron=-1;

// sie is set as RAF by default, check to see if it should be LW
// in quick missions only though

		if (!_DPlay.Side && _DPlay.GameType==DPlay::COMMSQUICKMISSION)
			CSQuick1::quickdef.plside=1;
		else
			CSQuick1::quickdef.plside=0;

		if	(	(_DPlay.GameType == DPlay::COMMSQUICKMISSION)
			&&	(_DPlay.UIPlayerType==DPlay::PLAYER_HOST)
			)
		{
//			Miss_Man.currcampaignnum = MissMan::SCRAMBLECAMPAIGN;		//RDH 16Apr96
//			Miss_Man.camp = Miss_Man.campaigntable[MissMan::SCRAMBLECAMPAIGN];

			DPlay::H2H_Player[_DPlay.mySlot].squadron=-1;
			DPlay::H2H_Player[_DPlay.mySlot].position=-1;

			fs=&commsquick;
		}else
		{
			SelectReadyRoom(fs);
		}

// dont need this csquick set up when game is launched


		return TRUE;
	}
	else
	{
//		_DPlay.DestroyAndClose(_DPlay.myDPlayID);
		LaunchDial(new CLockerRoom,0);
		return FALSE;
	}
}


Bool RFullPanelDial::CommsSelectFly(FullScreen*&fs)
{

	if (!_DPlay.UINetworkSelectFly())
		return FALSE;

	return TRUE;
}

#endif	BOB_DEMO_VER

Bool RFullPanelDial::InitReplay(FullScreen*&fs)
{
	_Replay.MenuViewPlayback=true;

	strcpy(_Replay.pfilename,"replay.dat");
//	_Replay.GetReplayFilename(_Replay.filename);

//	_Replay.Record=TRUE;
	return TRUE;
}

#ifndef	BOB_DEMO_VER

Bool RFullPanelDial::AllowExitFromComms(FullScreen*&fs)
{
	if (!_DPlay.PlayerCreated)
	{
		CleanUpComms(fs);
		return TRUE;
	}
	return FALSE;
}

Bool RFullPanelDial::CleanUpComms(FullScreen*&fs)
{
	CSQuick1::currquickmiss=-1;

	CSQuick1::currquickfamily=-1;



	_DPlay.ClearPackages();


	_DPlay.UILeavingMainSheet();
//	_DPlay.DestroyAndClose(_DPlay.myDPlayID);
	_DPlay.ExitDirectPlay();

	if (_DPlay.Lobbied)
	{
		_DPlay.Lobbied=FALSE;
// quit game

		ConfirmExit(fs);
		return FALSE;
	}
	return TRUE;
}

#endif	BOB_DEMO_VER


#ifndef	BOB_DEMO_VER

void	RFullPanelDial::ChangeCamp(int campnum)
{

	if (pdial[1])
	{
		pdial[1]->PreDestroyPanel();
		pdial[1]->DestroyPanel();
		pdial[1]=NULL;
	}
	LaunchDial(new PhaseDescription(campnum),1);

	if (pdial[2])
	{
		pdial[2]->PreDestroyPanel();
		pdial[2]->DestroyPanel();
		pdial[2]=NULL;
	}
	else
		CloseVideo();
	//else
	//close video because viseo doesn't play in a dialog

//not used for BoB, in mig we could exchange description and background
//TEMPCODE RDH 03/04/00 	if (whichcamp&0x10)
//TEMPCODE RDH 03/04/00 	{
//TEMPCODE RDH 03/04/00 		whichcamp=campnum+0x10;
//TEMPCODE RDH 03/04/00 		LaunchDial(new CCampBack(IDS_CAMPDESC0+campnum),2);
//TEMPCODE RDH 03/04/00 	}
//TEMPCODE RDH 03/04/00 	else
//TEMPCODE RDH 03/04/00 	if (whichcamp&0x20)
//TEMPCODE RDH 03/04/00 	{
//TEMPCODE RDH 03/04/00 		whichcamp=campnum+0x20;
//TEMPCODE RDH 03/04/00 		LaunchDial(new CCampBack(IDS_CAMPOBJ0+campnum),2);
//TEMPCODE RDH 03/04/00 	}
//TEMPCODE RDH 03/04/00 	else
//TEMPCODE RDH 03/04/00 	{
		whichcamp=campnum;
		File_Man.DiscardDirList(FileNum(FIL_VID_PHASE1BEGINS+whichcamp));
		File_Man.ExistNumberedFile(FileNum(FIL_VID_PHASE1BEGINS+whichcamp));
//	MCIWndClose(g_hwndMCIWnd);
//		LaunchSmacker(1,FileNum(FIL_VID_THOUSANDBOMBER+whichcamp));
	LaunchSmacker(2,FileNum(FIL_VID_PHASE1BEGINS+whichcamp));

//		OpenSmack(FileNum(FIL_SMACK_CAMP1INTRO+whichcamp),(int)m_hWnd,0,0,640,480);

//test code - how to change a video
}										

void	RFullPanelDial::ChangeFamilyAnim()
{
	int index = CSQuick1::currquickfamily;
	if (index > 6)
		index = 6;
	if (pdial[2])
	{
		pdial[2]->PreDestroyPanel();
		pdial[2]->DestroyPanel();
		pdial[2]=NULL;
	}
	else
		CloseVideo();

	File_Man.DiscardDirList(FileNum(FIL_VID_NOVICE+index));
	File_Man.ExistNumberedFile(FileNum(FIL_VID_NOVICE+index));
	LaunchSmacker(2,FileNum(FIL_VID_NOVICE+index));


}

void	RFullPanelDial::ChangeReview(int msgnum)
{

	if (pdial[1])
	{
		pdial[1]->PreDestroyPanel();
		pdial[1]->DestroyPanel();
		pdial[1]=NULL;
	}
	LaunchDial(new EndOfDayReviewText(msgnum),1);

 	if (pdial[2])
	{
		pdial[2]->PreDestroyPanel();
		pdial[2]->DestroyPanel();
		pdial[2]=NULL;
	}
	else
		CloseVideo();

	int	titletext = Node_Data.review.MessageTitle(msgnum);


	if (		(titletext >= IDS_PHRASE_TITLE_MONUMENTSURVIVES)
			&&	(titletext <= IDS_PHRASE_TITLE_SEALIONRAFWIN)
		)
	{
		int offset = titletext - IDS_PHRASE_TITLE_MONUMENTSURVIVES;
	 	File_Man.DiscardDirList(FileNum(FIL_VID_MONUMENTSURVIVES+offset));
		File_Man.ExistNumberedFile(FileNum(FIL_VID_MONUMENTSURVIVES+offset));
		LaunchSmacker(2,FileNum(FIL_VID_MONUMENTSURVIVES+offset));
	}else
	{
		FileNum file = FileNum((titletext - IDS_PHRASE_TITLE_RAFHEADON) + FIL_SNAP_HEADON);

		LaunchDial(new EndofDayReviewBmp(msgnum),2, file);
	}



}										



Bool	RFullPanelDial::StartCampSmacker(FullScreen*&fs)
{
	if (pdial[2])
	{
		pdial[2]->PreDestroyPanel();
		pdial[2]->DestroyPanel();
		pdial[2]=NULL;
	}
	whichcamp=(whichcamp&0x0f);
	File_Man.DiscardDirList(FileNum(FIL_VID_PHASE1BEGINS+whichcamp));
	File_Man.ExistNumberedFile(FileNum(FIL_VID_PHASE1BEGINS+whichcamp));
	return FALSE;
}
Bool	RFullPanelDial::StartCampBackground(FullScreen*&fs)
{
	return FALSE;
}
Bool	RFullPanelDial::StartCampObjectives(FullScreen*&fs)
{
	return FALSE;
}

#endif	//BOB_DEMO_VER

Bool	RFullPanelDial::TitleInit()
{
	_Replay.DeleteTempFiles();
	_Replay.MenuViewPlayback=false;
	_Replay.MapPlayback=false;
	incomms=in3d=false;
	gamestate=TITLE;
	SetTitleText(true);


	return	TRUE;
}

Bool	RFullPanelDial::SetUpHotShot(FullScreen*&fs)
{
	int i;
	for (i=0;CSQuick1::quickmissions[i].titlename!=IDS_CONFIGIGNORED;i++)
	{}
	CSQuick1::currquickmiss=i;
	CSQuick1::quickdef=CSQuick1::quickmissions[i]; //FB strike
	Miss_Man.currcampaignnum = MissMan::SCRAMBLECAMPAIGN;		//RDH 16Apr96
	Miss_Man.camp = Miss_Man.campaigntable[MissMan::SCRAMBLECAMPAIGN];
	Node_Data.BuildTargetTable();
	Node_Data.LoadCleanNodeTree();
	MMC.playersquadron=CSQuick1::quickdef.line[CSQuick1::quickdef.plside][CSQuick1::quickdef.plwave][CSQuick1::quickdef.plgrp].actype;
	//plside,plwave,plgrp,plac
	MMC.playeracnum=CSQuick1::quickdef.plac;

	SetTitleText();
	CSQuick1::currquickmiss=-1;

	CSQuick1::currquickfamily=-1;


	gamestate=HOT;
	return TRUE;
}

Bool	RFullPanelDial::SetQuickState(FullScreen*&fs)
{

	Miss_Man.currcampaignnum = MissMan::SCRAMBLECAMPAIGN;		//RDH 16Apr96
	Miss_Man.camp = Miss_Man.campaigntable[MissMan::SCRAMBLECAMPAIGN];
	Miss_Man.camp.Sky.Conditions = 0; //always clear for quick missions
	Miss_Man.camp.currtime =	HR14;		//always start in afternoon
	gamestate=QUICK;
	SetTitleText();

	return TRUE;
}
extern	CDC g_OffScreenDC;
void	Campaign::InitIcons()
{
		if (RFullPanelDial::gamestate==RFullPanelDial::PILOT)
		{
			if (RFullPanelDial::gameside==RFullPanelDial::SIDE_RAF)
				Todays_Packages.localplayer=NAT_RED;
			else
				Todays_Packages.localplayer=NAT_BLUE;
			Todays_Packages.britishisauto=true;
			Todays_Packages.germanisauto=true;
		}
		else
			if (RFullPanelDial::gameside==RFullPanelDial::SIDE_RAF)
			{
				Todays_Packages.localplayer=NAT_RED;
				Todays_Packages.britishisauto=false;
				Todays_Packages.germanisauto=true;
			}
			else
			{
				Todays_Packages.localplayer=NAT_BLUE;
				Todays_Packages.britishisauto=true;
				Todays_Packages.germanisauto=false;
			}
	if (Todays_Packages.localplayer==NAT_RAF)							//RDH 20/03/00
		IconDescUI::LoadInstances(GetDC(NULL),ICON_SET_1);
	else
		IconDescUI::LoadInstances(GetDC(NULL),ICON_SET_1_LW);
	Node_Data.BuildTargetTable();
	Node_Data.LoadCleanNodeTree();										//JIM 20Sep00
}

Bool	RFullPanelDial::SetUpPilot(FullScreen*&fs)
{
	gamestate = PILOT;
	return TRUE;
}
Bool	RFullPanelDial::SetUpCommander(FullScreen*&fs)
{
	gamestate = COMMANDER;
	return TRUE;
}

Bool	RFullPanelDial::SetUpLW(FullScreen*&fs)
{
	gameside = SIDE_LW;
	return SetUpCommander(fs);//TRUE;
}
Bool	RFullPanelDial::SetUpRAF(FullScreen*&fs)
{
	gameside = SIDE_RAF;

	return SetUpCommander(fs);//TRUE;
}

Bool	RFullPanelDial::IfCommsToReadyRoom(FullScreen*&fs)
{
	Miss_Man.camp = Miss_Man.campaigntable[MissMan::SCRAMBLECAMPAIGN];	//CSB 02/07/99	

#ifndef	BOB_DEMO_VER
	if (incomms)
	{

//		fs=&readyroom;
		SelectReadyRoom(fs);
	}
#endif	//BOB_DEMO_VER
	if (gamestate==HOT)
		fs=&title;
	return CheckForDemo(fs);
}

#ifndef	BOB_DEMO_VER

Bool	RFullPanelDial::MapSpecialsInit()
{
	return TRUE;
}
Bool	RFullPanelDial::DeadPilotInit()
{
		LaunchSmacker(1, MMC.introsmack);
		LaunchDial(new CCampBack(IDS_PLAYERDIED),2);
	return TRUE;
}
Bool	RFullPanelDial::ContinueAlthoughDead(FullScreen*&fs)
{

		return LaunchMap(fs, true);

}
Bool	RFullPanelDial::ContinueAfterSpecialDebrief(FullScreen*&fs)
{
	MMC.msgwithsmacker = 0;


	return LaunchMap(fs, true);

	return TRUE;
}
Bool	RFullPanelDial::ReloadBecauseDead(FullScreen*&fs)
{
	char buffer[fileman::pathnamesize];
		BIStream bis(File_Man.namenumberedfile(
			File_Man.fakefile(FIL_SAVEGAMEDIR,"Auto Save.sav"),buffer));
		if (bis.is_open())
			bis>>Miss_Man;

		return LaunchMap(fs, true);

	return TRUE;
}


Bool	RFullPanelDial::LaunchMapIfAlive(FullScreen*&fs)
{
	if (incomms || true)	//test for alive...
	{
		return LaunchMap(fs, true);
	}
	else
		fs=&title;

	return TRUE;
}

Bool	RFullPanelDial::LaunchMapFirstTime(FullScreen*&fs)
{
	CloseVideo(); 

	FileNum f;
//	m_pView->GetDocument()->SetTitle(LoadResString(IDS_L_SCAMPAIGNSELECT1+(whichcamp&0x0f)));
	m_pView->m_zoom = 2.0;//Save_Data.defaultzoom;						//JIM 11Oct00
	m_pView->m_size.cx=256*4*m_pView->m_zoom-5;
	m_pView->m_size.cy=256*4*m_pView->m_zoom-5;
	m_pView->m_oldzoom=m_pView->m_zoom;
	m_pView->m_scrollpoint=CPoint(500,800);
	
	CampaignZero::Fav	fav=MMC.fav;
	if (whichcamp>=0)
	{
		Miss_Man.currcampaignnum = MissMan::CampaignName(whichcamp&0x0f);							//RDH 19/03/99
		Miss_Man.camp = Miss_Man.campaigntable[Miss_Man.currcampaignnum];	//RDH 15/02/99	

	}
	if (gamestate == PILOT)
	{
		MMC.fav=fav;
		MMC.autoscramble=true;
		MMC.accelrates[MMC.ACCEL_NONRAIDSPD]=
			MMC.accelrates[MMC.ACCEL_RECONNSPD]=
				MMC.accelrates[MMC.ACCEL_RAIDSPD]=
					600;
		MMC.accelrates[MMC.ACCEL_DIALOGSPEED]=2;
	}

	SetTitleText();

	Node_Data.BuildTargetTable();
 	Persons4::StartUpMapWorld();
	Node_Data.SkipToDate(MMC.currdate-MMC.startdate);							//JIM 16/05/00
	
	CMainFrame* main=(CMainFrame*)AfxGetMainWnd();
	main->m_titlebar.Redraw(true);



//	pmainwnd->m_toolbar2.OnClickedPlayerlog();	
	return LaunchMap(fs, true);
}
Bool	RFullPanelDial::DoLoadCommsGame(FullScreen*&fs)
{
	CFiling::LoadGame(selectedfile);
	if (selectedfile=="" || selectedfile[0]=='.')
		return FALSE;
	Save_Data.lastsavegame = selectedfile;

//	strcpy(_DPlay.Password,MMC.CommsPassword);
//	strcpy(_DPlay.Password,Save_Data.CommsPassword);
	if (CreatePlayer(fs))
	{
		LaunchMap(fs, true);										//AMM 22Jan99
	}

	return FALSE;												//AMM 22Jan99

}
Bool	RFullPanelDial::DoLoadGame(FullScreen*&fs)
{
	if (selectedfile=="" || selectedfile==".sav")
		return FALSE;
	if (!CFiling::LoadGame(selectedfile))
		return FALSE;
	Save_Data.lastsavegame = selectedfile;
	return LaunchMap(fs, false);
}

#endif	//#ifndef	BOB_DEMO_VER


Bool	RFullPanelDial::CheckForDemo(FullScreen*&fs)	
{
	return	TRUE;
}

Bool	RFullPanelDial::CheckForMissingMission(FullScreen*&fs)	
{
	if (CSQuick1::quickdef.plside)
 		gameside = SIDE_LW;
	else
		gameside = SIDE_RAF;


	
	if (CSQuick1::quickdef.currmissnum == FIL_NULL)
		return	FALSE;

	return TRUE;
}

void	DPlay::FillCSQuickStruc()
{
	ULong n;

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (DPlay::H2H_Player[n].squadron==-1)					//AMM 08Nov98
		{														//AMM 08Nov98
			DPlay::H2H_Player[n].squadron=0;					//AMM 08Nov98
		}														//AMM 08Nov98

		if (DPlay::H2H_Player[n].status!=CPS_NONE)
		{
// for now use just blue field to store AC.
// actype=playersquadron
// flight=1
// only 1st grouyp gets filled in for deathmatch

			CSQuick1::quickdef.line[0][n][0].actype=DPlay::H2H_Player[n].squadron;
			CSQuick1::quickdef.line[0][n][0].flights=1;

		}
		else
		{
			CSQuick1::quickdef.line[0][n][0].actype=0; // empty slots use default actype0(f86a)
			CSQuick1::quickdef.line[0][n][0].flights=1;
		}
	}
}

Bool RFullPanelDial::CheckLobby(FullScreen*&fs)
{
//		UpdateSize();													//DAW 15/06/99
		CloseVideo();																		//DAW 15/06/99
		Save_Data.ChangeMode(SaveData::UIR_SCRAMBLE);
	{
		int	failed=0;
		UpdateSize();
		failed=GetCurrentRes();

		PAINTSTRUCT paintstruct;
		CDC* pDC;
		pDC=BeginPaint(&paintstruct);
		if (GetDeviceCaps(pDC->m_hDC,BITSPIXEL)<15)
			failed=0;
		if (failed==0)
		{
			//attempt to improve 256 colour palette match?
		}
		EndPaint(&paintstruct);
		Invalidate();

		if (failed==0)
		{
			GETDLGITEM(IDC_RLISTBOX)->Clear();
			MessageBox(RESSTRING(RESWARNING), RESSTRING(BOB));
			ConfirmExit(fs);
			return FALSE;
		}
	}
		_Miles.InitSoundFonts();		//RJS 24Nov98					//DAW 15/06/99

	
#ifndef	BOB_DEMO_VER
	if (_DPlay.Lobbied)
	{
		fs=&multiplayer;
	}else
#endif
		_DPlay.lpDPL=NULL;;
	return CheckForDemo(fs);
}

#ifndef	BOB_DEMO_VER

Bool RFullPanelDial::CanGuestJoin(FullScreen*&fs)
{
// wowa - only go in here if we are joining..............
	if (!_DPlay.FlyNowFlag)
	{
		if (DPlay::H2H_Player[0].status==DPlay::CPS_3D)
		{
			if (!_DPlay.JoinGame())
			{
				_DPlay.SendStatusChange(DPlay::CPS_READYROOM);
				return FALSE;
			}

			return TRUE;
		}
	}
	else
		return TRUE;

	return FALSE;
}

#endif	//#ifndef	BOB_DEMO_VER

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SendCSQuickStrucToPlayers
//Author		Andy McMaster
//Date			Sun 8 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendCSQuickStrucToPlayers(Bool init)
{
#ifndef	BOB_DEMO_VER
	if (GameType==COMMSQUICKMISSION)
	{
		GameIndex=CSQuick1::currquickmiss;
	}
	SendCS((UByte*)&CSQuick1::quickdef,sizeof(QuickDef),init);
#endif	//#ifndef	BOB_DEMO_VER
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		UpdateCSQuick
//Author		Andy McMaster
//Date			Sun 8 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::UpdateCSQuick(UByte* pack,ULong num)
{
#ifndef	BOB_DEMO_VER
	memcpy(&CSQuick1::quickdef,pack,sizeof(QuickDef));
	CSQuick1::currquickmiss=num;

// need to call PrepareForFrag

#endif	//#ifndef	BOB_DEMO_VER
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		PackagePrepareForFrag
//Author		Andy McMaster
//Date			9 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

Bool RFullPanelDial::ReInitCommsInterface(FullScreen*&fs)
{
#ifndef	BOB_DEMO_VER
	_DPlay.lpDP4->Release();

	if (!_DPlay.CreateDPlayInterface())
	{
// if cant get a new dplay interface then go back to main menu

		fs=&title;
	}

#endif	//#ifndef	BOB_DEMO_VER
	return TRUE;
}

Bool	RFullPanelDial::LobbyCheck(FullScreen*&fs)
{
#ifndef	BOB_DEMO_VER
	if (_DPlay.Lobbied)
	{
		_DPlay.ExitDirectPlay();
		_DPlay.Lobbied=FALSE;

		ConfirmExit(fs);
		return FALSE;
	}
	else if (_DPlay.UIPlayerType==DPlay::PLAYER_HOST || _DPlay.singlesession)
	{
		_DPlay.ExitDirectPlay();
		_DPlay.StartCommsSession();
		fs=&selectservice;
	}
	else
	{
// need to close current session to be able to enumerate sessions

		_DPlay.lpDP4->Close();

		if (!_DPlay.UIGetSessionListUpdate())
		{
// problem initialising connection, exit to main menu

			_DPlay.CommsMessage(IDS_BADSESSIONS2);
			_DPlay.ExitDirectPlay();
			fs=&title;
		}
	}
		
#endif	//#ifndef	BOB_DEMO_VER
	return TRUE;
}

void	DPlay::CommsMessage(UINT mess)
{
	RDialog::RMessageBox(IDS_BOB,mess,NULL,IDS_OK);
}

#ifndef	BOB_DEMO_VER
Bool	DPlay::LoadRadioMessages()
{
	char buffer[fileman::pathnamesize];
	ULong n;

//	BIStream bis			//rdh 10/2/99
	File_Man.namenumberedfile(File_Man.fakefile(FIL_SAVEGAMEDIR,"radio.txt"),buffer);//rdh 10/2/99


	HANDLE messfile;
//	int ch,index;
	bool ok;
	DWORD numread;
//	char rmess[128];

	messfile=CreateFile(
		buffer,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (messfile==INVALID_HANDLE_VALUE || !GetFileSize(messfile,NULL))
	{
		InitMessages();		//rdh 10/2/99
		return FALSE;
	}
	else
	{
		for (n=0;n<NUMRADIOMESSAGES;n++)
		{

//				ok=
					ReadFile(
					messfile,
					Messages[mySlot][n],
					128,
					&numread,
					NULL);											//DAW 09/03/99
		}
		CloseHandle(messfile);
	}
	return TRUE;
}

void	DPlay::SaveRadioMessages()
{
	char buffer[fileman::pathnamesize];
//	char nl='\n';
	ULong n,numwrote;
//	char rmess[128];

//	BIStream bis(File_Man.namenumberedfile(File_Man.fakefile(FIL_SAVEGAMEDIR,"radio.txt"),buffer));
	File_Man.namenumberedfile(File_Man.fakefile(FIL_SAVEGAMEDIR,"radio.txt"),buffer);

	HANDLE	messfile;

	messfile=CreateFile(
		buffer,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		0,
		NULL);

	for (n=0;n<NUMRADIOMESSAGES;n++)
	{

//		len=strlen(Messages[mySlot][n]);
//		strncpy(rmess,Messages[mySlot][n],127);
//		rmess[127]='\n';

		WriteFile(
			messfile,
			Messages[mySlot][n],
			128,
			&numwrote,
			NULL);
	}

	CloseHandle(messfile);
}
#endif	//#ifndef	BOB_DEMO_VER

void	Replay::GetReplayFilename(char* buff)
{
	char buffer[fileman::pathnamesize];

	strcpy(buffer,buff);

//	BIStream bis(File_Man.namenumberedfile(File_Man.fakefile(FIL_VIDEOS_START_DIR,buffer),buff));
	File_Man.namenumberedfile(File_Man.fakefile(FIL_VIDEOS_START_DIR,buffer),buff);

}

void	DPlay::GetCommsSavename(char* buff)
{
	char buffer[fileman::pathnamesize];

	strcpy(buffer,buff);

//	BIStream bis(File_Man.namenumberedfile(File_Man.fakefile(FIL_SAVEGAMEDIR,buffer),buff));
	File_Man.namenumberedfile(File_Man.fakefile(FIL_SAVEGAMEDIR,buffer),buff);
}

void	DPlay::GetLastSaveName(char* buff)
{
	strcpy(buff,Save_Data.lastsavegame);
}

#ifndef	BOB_DEMO_VER
Bool	RFullPanelDial::ReplaySaveBack(FullScreen*&fs)
{
	if (_Replay.RestorePosition)
	{
		_Replay.Playback=TRUE;
		fs=&quickmissionflight;
	}
	else if (_Replay.IsCampState(gamestate))
	{
// launch map debrief

		LaunchMap(fs, false);
		return FALSE;
	}

// else back to quickmissiondebrief

	return TRUE;
}
#endif	//#ifndef	BOB_DEMO_VER

Bool	RFullPanelDial::ReplayLoadBack(FullScreen*&fs)
{
#ifndef	BOB_DEMO_VER
	if (_Replay.IsCampState(gamestate))
	{
		LaunchMap(fs, false);
		return FALSE;
	}
#endif	//#ifndef	BOB_DEMO_VER
	return CheckForDemo(fs);
}

bool	DPlay::LoadDummySavegame(char* name)
{
	CString	temp=name;

	bool retval;

	retval=CFiling::LoadGame(temp);


	return retval;

}

ULong	Replay::GetState()
{
	return RFullPanelDial::gamestate;
}

void	Replay::SetState(ULong state)							//AMM 31Mar99
{																//AMM 31Mar99
	RFullPanelDial::gamestate=(RFullPanelDial::GameStates)state;							//AMM 31Mar99
}																//AMM 31Mar99

bool	Replay::IsCampState(ULong state)
{
	if (state==RFullPanelDial::PILOT
	||	state==RFullPanelDial::COMMANDER)
		return true;

	return false;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SaveTempSaveGame
//Author		Andy McMaster
//Date			Fri 22 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::SaveTempSaveGame(char* name)//, bool pack)
{
// save game as dreplay.sav for writing to header

	CString	temp=name;

	Todays_Packages.SaveBin();

	return (CFiling::SaveGame(temp));
}

Bool	RFullPanelDial::DebriefReplayCheck(FullScreen*&fs)
{
	if (incomms)
		return FALSE;

	if (_Replay.badrecord)
	{

		RMessageBox(IDS_BOB,IDS_BADREPLAY,NULL,IDS_OK);
		return FALSE;
	}

	return TRUE;
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		TruncateTempFiles
//Author		Andy McMaster
//Date			Thu 13 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::DeleteTempFiles()
{
	char buffer[fileman::pathnamesize];

	File_Man.namenumberedfile(File_Man.fakefile(FIL_VIDEOS_START_DIR,"replay.dat"),buffer);
	DeleteFile(buffer);

	File_Man.namenumberedfile(File_Man.fakefile(FIL_SAVEGAMEDIR,replaysavebackup),buffer);
	DeleteFile(buffer);

	File_Man.namenumberedfile(File_Man.fakefile(FIL_SAVEGAMEDIR,"dcomms.dat"),buffer);
	DeleteFile(buffer);

	File_Man.namenumberedfile(File_Man.fakefile(FIL_VIDEOS_START_DIR,"tblock.dat"),buffer);
	DeleteFile(buffer);

	File_Man.namenumberedfile(File_Man.fakefile(FIL_SAVEGAMEDIR,"rbackup.dat"),buffer);
	DeleteFile(buffer);
}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		ClearPackages
//Author		Andy McMaster
//Date			Fri 14 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ClearPackages()	 //THESE ALL NEED TO USE THE NEW	FragScreenBody STRUCTURE
{

	if (GameType<COMMSQUICKMISSION)
	{
		for (int s = 0; s < 2; s++)
		{
			for (int w=0;w<8;w++)
			{
				for (int g=0;g<3;g++)
				{
					CSQuick1::quickdef.line[s][w][g].duty=0;
					CSQuick1::quickdef.line[s][w][g].flights=0;
					CSQuick1::quickdef.line[s][w][g].actype=-1;
				}
			}
		}
	}

}

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
//Procedure		SelectRRVariants
//Author		Dave Whiteside
//Date			Sun 27 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	RFullPanelDial::SelectRRVariants(FullScreen*&fs)
{
	if (_DPlay.Host)
		return TRUE;

	return FALSE;
}

static	PolyOutLine		//note: if poly goes through 0,0 that must be first point!
	SinglePlayerOutlines[]=
	{	{NULL,0,	{{0,0},{1024,0},{0,768}}		},
		{NULL,-1,	{{1024,0},{1024,768},{0,768}}	}
	},
	GameTypeOutlines[]=
	{	{NULL,0,	{{0,0},{1024,0},{0,768}}		},
		{NULL,-1,	{{1024,0},{1024,768},{0,768}}	}
	};

Bool	RFullPanelDial::SinglePlayerInit()
{

//	ScaleHitAreas();
//	polylist.Add(SinglePlayerOutlines,2);
//	polylist=GETDLGITEM(IDC_RLISTBOX);
	LaunchDial(new	SideSelect(0),0);
	return TRUE;
}
Bool	RFullPanelDial::GameTypeInit()
{
//	ScaleHitAreas();
//	polylist.Add(GameTypeOutlines,2);
//	polylist=GETDLGITEM(IDC_RLISTBOX);
	LaunchDial(new	GameSelect,0);
	return TRUE;
}

Bool	RFullPanelDial::JumpToWebSite(FullScreen*&fs)
{
	_DPlay.JumpToWebSite();

	return TRUE;
}
