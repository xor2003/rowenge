/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
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

questions about this file may be asked at http://www.simhq.com/
*/

// Ready.cpp : implementation file
//

#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "MIG.H"
#include "RSTATIC.H" // RERUN for CRStatic
#include "RRADIO.H" // RERUN for CRRadio
#include "RCOMBO.H" // RERUN for CRCombo
#include "RCOMBOX.H" //RERUN
#include "RBUTTON.H" //RERUN for CRButton
#include "RTOOLBAR.H" //RERUN for CRToolBar
#include "REDIT.H" //RERUN for CREdit
#include "RLISTBOX.H" //RERUN for CRListBox
#include "MIGVIEW.H"
#include "TITLEBAR.H"
#include "SAVEGAME.H"
#include "MAINTBAR.H" //RERUN see below
#include "MAINFRM.H" //RERUN combination with MAINTBAR for MainToolBar
#include "VISITORS.H"
#include "RRADIO.H"
#include "WINMOVE.H"
#include "MISSMAN2.H"
#include "SQUICK1.H" //RERUN for CSQuick1
#include "MESSENGN.H"
#include "PERSONS2.H"
#include "COMMS.H"
#include "FULLPANE.H"

#include "READY.H"



/* #ifdef _DEBUG
#define new DEBUG_NEW
#ifndef THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif */

/////////////////////////////////////////////////////////////////////////////
// CReadyRoom dialog


CReadyRoom::CReadyRoom(CWnd* pParent /*=NULL*/)
	: RowanDialog(CReadyRoom::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReadyRoom)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CReadyRoom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReadyRoom)
	DDX_Control(pDX, IDCX_PLAYERCHAT, m_IDCX_PLAYERCHAT);
	DDX_Control(pDX,IDC_RLIST_UN_PLAYER,m_IDC_RLIST_UN_PLAYER);
	DDX_Control(pDX,IDC_RSTATICCTRL3,m_IDC_RSTATICCTRL3);
	DDX_Control(pDX,IDC_RLIST_INCOMING_CHAT,m_IDC_RLIST_INCOMING_CHAT);
	DDX_Control(pDX,IDC_PLAYERCHAT,m_IDC_PLAYERCHAT);
	DDX_Control(pDX,IDC_RRADIO_DETAILS,m_IDC_RRADIO_DETAILS);
	DDX_Control(pDX,IDC_GAMENAME,m_IDC_GAMENAME);
	DDX_Control(pDX,IDC_RSTATICCTRL24,m_IDC_RSTATICCTRL24);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReadyRoom, CDialog)
	//{{AFX_MSG_MAP(CReadyRoom)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_EN_UPDATE(IDCX_PLAYERCHAT, OnUpdatePlayerchat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReadyRoom message handlers

void CReadyRoom::RedrawRadioButtons()
{
	CRRadio* pradio;
	pradio=GETDLGITEM(IDC_RRADIO_DETAILS);
	CRStatic* s = GETDLGITEM(IDC_RSTATICCTRL3);
	



	if (_DPlay.sendtowho==255 || _DPlay.GameType==DEATHMATCH)
	{
		_DPlay.sendtowho=255;
		pradio->SetCurrentSelection(0);
	}
	else
	{
		_DPlay.sendtowho=128;
		pradio->SetCurrentSelection(1);
	}

	CRListBox* rlistbox;
	rlistbox=GETDLGITEM(IDC_RLIST_UN_PLAYER);
	rlistbox->SetHilightRow(-1);



}

BOOL CReadyRoom::OnInitDialog() 
{
	CDialog::OnInitDialog();
//	m_timer=SetTimer(2364,0,NULL);
	
	CRListBox* rlistbox;
	CString string;

	if (_DPlay.UpdateMission)
	{
		_DPlay.SendCSQuickStrucToPlayers(FALSE);
		_DPlay.UpdateMission=FALSE;
	}

// create dplay session if host

	Bool res;
	if (m_IDCX_PLAYERCHAT.m_hWnd)
		m_IDCX_PLAYERCHAT.SetLimitText(60);
	if (_DPlay.UIPlayerType==PLAYER_HOST && !_DPlay.PlayerCreated)
	{
		if (_DPlay.Lobbied)
		{
			res=_DPlay.LobbyConnect();
		}
		else
		{
			res=_DPlay.UINewPlayer(&_DPlay.PlayerName[0],&_DPlay.SessionName[0]);
		}

		if (res)
		{
			res=_DPlay.SetUpPlayerInfo(&_DPlay.PlayerName[0]);

			if (!res)
			{
				RMessageBox(IDS_MIGALLEY,IDS_COULDNTSETUPPLAYER,0,IDS_OK);

				_DPlay.LeaveCommsFlag=TRUE;
			}
		}
		else
		{
			RMessageBox(IDS_MIGALLEY,IDS_NOTSESSION,0,IDS_OK);

// dont create ready room - leave

			_DPlay.LeaveCommsFlag=TRUE;
		}
	}

	if (_DPlay.GameType==COMMSQUICKMISSION) // && _DPlay.Host)
	{
		_DPlay.GameIndex=CSQuick1::currquickmiss;
	}


	rlistbox=GETDLGITEM(IDC_RLIST_INCOMING_CHAT);
	rlistbox->Clear();
	rlistbox->AddColumn(120);
	rlistbox->AddColumn(30);

//	rlistbox=(CRListBox*)this->GetDlgItem(IDC_RSTATICCTRL4);
//	rlistbox->Clear();
//	rlistbox->AddString(_DPlay.GameName,1);


	CRRadio* pradio;
	pradio=GETDLGITEM(IDC_RRADIO_DETAILS);
	CRStatic* s = GETDLGITEM(IDC_RSTATICCTRL3);

	pradio->ShowWindow(SW_SHOW);
	s->ShowWindow(SW_SHOW);
	pradio->AddButton(RESSTRING(EVERYBODY));

	if (_DPlay.GameType!=DEATHMATCH)
		pradio->AddButton(RESSTRING(MYSIDEONLY));

	RedrawRadioButtons();

	_DPlay.MainSheetCount=FALSE;
	_DPlay.FlyNowFlag=FALSE;

	if (!_DPlay.LoadRadioMessages())
	{//initialise messages
		_DPlay.InitMessages();
	}
	
	rroom_timer=SetTimer(TIMER_READYROOM,0,NULL); // set sheet up first before starting timer

	_DPlay.SendInReadyRoomMessage();

	if (_DPlay.Host)
	{
		if (_DPlay.GameType==COMMSQUICKMISSION)
		{
// when host enters ready room reset pilots to NOTFLYING because this will 
// be a new game

			for (int p=0;p<PILOTS_PER_CAMPAIGN;p++)
			{
				MMC.Active_Pilots[p]=MMC.Active_Pilots_Init[p];
			}
		}
		_DPlay.SendPilotsStatus(false);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CReadyRoom, CDialog)
    //{{AFX_EVENTSINK_MAP(CReadyRoom)
	ON_EVENT(CReadyRoom, IDC_PLAYERCHAT, 1 /* ReturnPressed */, OnReturnPressedPlayerchat, VTS_BSTR)
	ON_EVENT(CReadyRoom, IDC_RRADIO_DETAILS, 1 /* Selected */, OnSelectedRradioDetails, VTS_I4)
	ON_EVENT(CReadyRoom, IDC_RLIST_UN_PLAYER, 1 /* Select */, OnSelectRlistUnPlayer, VTS_I4 VTS_I4)
	ON_EVENT(CReadyRoom, IDC_PLAYERCHAT, 2 /* TextChanged */, OnTextChangedPlayerchat, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CReadyRoom::OnReturnPressedPlayerchat(LPCTSTR text) 
{
	if (text[0])
	{
		CRListBox* rlistbox;
		CREdit* poo;

		rlistbox=GETDLGITEM(IDC_RLIST_INCOMING_CHAT);
		if (!strcmp(text,"megashootcheaton"))
		{
			_DPlay.megashootcheat=true;
		}
		else if (!strcmp(text,"megashootcheatoff"))
		{
			_DPlay.megashootcheat=false;
		}
		else if (!strcmp(text,"invulnerablecheaton"))
		{
			_DPlay.megashootcheat=false;
		}
		else if (!strcmp(text,"invulnerablecheatoff"))
		{
			_DPlay.megashootcheat=false;
		}
		else if (!strcmp(text,"liftcheaton"))
		{
			_DPlay.liftcheat=true;
		}
		else if (!strcmp(text,"liftcheatoff"))
		{
			_DPlay.liftcheat=false;
		}
		else
		{

			rlistbox->AddString((DPlay::H2H_Player+_DPlay.mySlot)->name,0);
			rlistbox->AddString(text,1);
	//	_DPlay.UISendDialogue((DPlay::H2H_Player+_DPlay.mySlot),(char*)text);
			_DPlay.UISendDialogue((char*)text);
			_DPlay.AddChatMessageToBuffer((DPlay::H2H_Player+_DPlay.mySlot)->name,(char*)text);
		}
		rlistbox->SetHilightRow(-1) ;							//AMM 10Jul99
		if (m_IDCX_PLAYERCHAT.m_hWnd)
		{
			m_IDCX_PLAYERCHAT.SetWindowText("");

		}
		else
		{
			poo=GETDLGITEM(IDC_PLAYERCHAT);
			poo->SetCaption("");										  //AMM 06/07/99
		}

	}
}
void	CReadyRoom::OnOK()
{
	CString buffer;
	m_IDCX_PLAYERCHAT.GetWindowText(buffer);
	OnReturnPressedPlayerchat(buffer);
}

void CReadyRoom::OnTimer(UINT nIDEvent) 
{
// have this first for quick exit in case bad session creation 

	if (_DPlay.LeaveCommsFlag)
	{
		KillTimer(rroom_timer);

// get me outta here asap

		FullPanel()->OnSelectRlistbox(0,0);					
		return;

	}

	if (nIDEvent==rroom_timer && _DPlay.PlayerCreated)
	{

		if (_DPlay.DisplayInfo)
		{
			_DPlay.DisplayPlayerInfo();
			_DPlay.DisplayInfo=FALSE;
		}

		_DPlay.UIUpdateMainSheet();

		if (_DPlay.FlyNowFlag)
		{
			if (_DPlay.UINetworkSelectFly())
			{
				KillTimer(rroom_timer);

// need to launch quickmissionflight sheet

// 				FullScreen* fs;
				FullPanel()->OnSelectRlistbox(1,1);
				return;
			}
			else
			{
				_DPlay.FlyNowFlag=FALSE;
				DPlay::H2H_Player[_DPlay.mySlot].status=CPS_READYROOM;
				_DPlay.SendInReadyRoomMessage();
			}
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CReadyRoom::OnClose() 
{
//	_DPlay.MainSheetCount=FALSE;

	KillTimer(rroom_timer);
	CDialog::OnClose();
}

void	DPlay::DisplayReadyDialogue(char* name, char* text)
{
	CRListBox* rlistbox;
 	rlistbox=(CRListBox*)RDialog::m_pView->m_pfullpane->pdial[0]->GetDlgItem(IDC_RLIST_INCOMING_CHAT);
	if (rlistbox)
	{
	 	rlistbox->AddString(name,0);
 		rlistbox->AddString(text,1);
		rlistbox->SetHilightRow(-1) ;							//AMM 10Jul99 //RERUN: was outside the NULL guard (scan-build)
	}
	//delete rlistbox;
}

void	DPlay::DisplayPlayerInfo()
{
// now only called from ready room so dont need check


	CRListBox* rlistbox;
	CString string;
	ULong n,t;

// may need to update gamename as well

	if (_DPlay.GameType==COMMSQUICKMISSION) 
	{
		_DPlay.GameIndex=CSQuick1::currquickmiss;
	}

	rlistbox=(CRListBox*)RDialog::m_pView->m_pfullpane->pdial[0]->GetDlgItem(IDC_RLIST_UN_PLAYER);
	rlistbox->Clear();
	rlistbox->AddColumn(40);
	rlistbox->AddColumn(140);
	rlistbox->AddColumn(60);
	rlistbox->AddColumn(90);
	rlistbox->AddColumn(50);
	rlistbox->AddColumn(2);

//	string.LoadString(IDS_UN);
	rlistbox->AddString("",0);

	string.LoadString(IDS_NAME);
	rlistbox->AddString(string,1);

	string.LoadString(IDS_AIRCRAFT);
	rlistbox->AddString(string,2);

	string.LoadString(IDS_STATUS);
	rlistbox->AddString(string,3);
	string.LoadString(IDS_KILLS);
	rlistbox->AddString(string,4);

	string.LoadString(IDS_LOSSES);
	rlistbox->AddString(string,5);

	t=1;
	char num[8],num2[8];
	char score[10];

	for (n=0;n<MAXPLAYERS;n++)
	{
		if ((H2H_Player+n)->MyTeam==TS_UN || (H2H_Player+n)->MyTeam==TS_SIDE_NONE)
		{
			if (_DPlay.GameType==DEATHMATCH)
			{
				std::snprintf(score, sizeof(score), "%d", t++);
			}
			else
			{
				std::snprintf(score, sizeof(score), "%d", t++);
				snprintf(score,sizeof(score),"UN %s",num);
			}
			rlistbox->AddString(score,0);
			rlistbox->AddString(H2H_Player[n].name,1);
			rlistbox->AddString(RESLIST(SQ_BF_F86A, H2H_Player[n].squadron), 2);
			rlistbox->AddString(RESLIST(CPS_STATUS,H2H_Player[n].status),3);

//create score
			
			if (_DPlay.GameType==DEATHMATCH)
			{
				std::snprintf(num, sizeof(num), "%d", (H2H_Player + n)->kills);
				rlistbox->AddString(num,4);
				std::snprintf(num, sizeof(num), "%d", (H2H_Player + n)->deaths);
				rlistbox->AddString(num,5);
			}
			else
			{
// need form 1/3 etc to indicate kills/team kills etc
				
				std::snprintf(num, sizeof(num), "%d", (H2H_Player + n)->kills);
				if ((H2H_Player+n)->MyTeam==(H2H_Player+mySlot)->MyTeam)
				{
					std::snprintf(num2, sizeof(num2), "%d", MyTeamKills);
				}
				else
				{
					std::snprintf(num2, sizeof(num2), "%d", EnemyTeamKills);
				}
				snprintf(score,sizeof(score),"%s",num);
				strcat(score,"/");
				strcat(score,num2);
				rlistbox->AddString(score,4);

				std::snprintf(num, sizeof(num), "%d", (H2H_Player + n)->deaths);

				if ((H2H_Player+n)->MyTeam==(H2H_Player+mySlot)->MyTeam)
				{
					std::snprintf(num2, sizeof(num2), "%d", MyTeamDeaths);
				}
				else
				{
					std::snprintf(num2, sizeof(num2), "%d", EnemyTeamDeaths);
				}
				snprintf(score,sizeof(score),"%s",num);
				strcat(score,"/");
				strcat(score,num2);
				rlistbox->AddString(score,5);
			}

		}
	}



	if (_DPlay.GameType!=DEATHMATCH)
		t=1;

	for (n=0;n<MAXPLAYERS;n++)
	{
		if ((H2H_Player+n)->MyTeam==TS_COMMIE)
		{
			if (_DPlay.GameType==DEATHMATCH)
			{
				std::snprintf(score, sizeof(score), "%d", t++);
			}
			else
			{
				std::snprintf(score, sizeof(score), "%d", t++);
				snprintf(score,sizeof(score),"Red %s",num);
			}
			rlistbox->AddString(score,0);
			rlistbox->AddString(H2H_Player[n].name,1);
			rlistbox->AddString(RESLIST(SQ_BF_F86A, H2H_Player[n].squadron), 2);
			rlistbox->AddString(RESLIST(CPS_STATUS,H2H_Player[n].status),3);

//create score
			
			if (_DPlay.GameType==DEATHMATCH)
			{
				std::snprintf(num, sizeof(num), "%d", (H2H_Player + n)->kills);
				rlistbox->AddString(num,4);
				std::snprintf(num, sizeof(num), "%d", (H2H_Player + n)->deaths);
				rlistbox->AddString(num,5);
			}
			else
			{
// need form 1/3 etc to indicate kills/team kills etc

				std::snprintf(num, sizeof(num), "%d", (H2H_Player + n)->kills);
				if ((H2H_Player+n)->MyTeam==(H2H_Player+mySlot)->MyTeam)
				{
					std::snprintf(num2, sizeof(num2), "%d", MyTeamKills);
				}
				else
				{
					std::snprintf(num2, sizeof(num2), "%d", EnemyTeamKills);
				}
				snprintf(score,sizeof(score),"%s",num);
				strcat(score,"/");
				strcat(score,num2);
				rlistbox->AddString(score,4);

				std::snprintf(num, sizeof(num), "%d", (H2H_Player + n)->deaths);
				if ((H2H_Player+n)->MyTeam==(H2H_Player+mySlot)->MyTeam)
				{
					std::snprintf(num2, sizeof(num2), "%d", MyTeamDeaths);
				}
				else
				{
					std::snprintf(num2, sizeof(num2), "%d", EnemyTeamDeaths);
				}
				snprintf(score,sizeof(score),"%s",num);
				strcat(score,"/");
				strcat(score,num2);
				rlistbox->AddString(score,5);			
			}
		}
	}	
	//delete rlistbox;

	CRStatic* rstatic;

//	rstatic=(CRStatic*)RDialog::m_pView->m_pfullpane->pdial[0]->GetDlgItem(IDC_RSTATICCTRL4);
	rstatic=(CRStatic*)RDialog::m_pView->m_pfullpane->pdial[0]->GetDlgItem(IDC_GAMENAME);
//	CRStatic* rstatic =  GetDlgItem(IDC_GAMENAME);
//	if (_DPlay.GameType==COMMSQUICKMISSION && _DPlay.Host)
//	{
//		rstatic->SetString(RESLIST(QUICK_1, CSQuick1::currquickmiss));
//	}else
//	{
//	}
	//delete	rstatic;
	if (_DPlay.GameType==COMMSQUICKMISSION)
		rstatic->SetString(LoadResString(CSQuick1::quickmissions[CSQuick1::currquickmiss].missionname));
	else
		rstatic->SetString(_DPlay.GameName);

}

void CReadyRoom::OnDestroy() 
{
//	_DPlay.MainSheetCount=FALSE;

	KillTimer(rroom_timer);

	CDialog::OnDestroy();
}

void CReadyRoom::OnSelectedRradioDetails(long ButtonNum) 
{

	if (ButtonNum)
		_DPlay.sendtowho=128;
	else
		_DPlay.sendtowho=255;

	RedrawRadioButtons();



}


void CReadyRoom::OnSelectRlistUnPlayer(long row, long column) 
{
	CRRadio* pradio;
	pradio=GETDLGITEM(IDC_RRADIO_DETAILS);
	pradio->SetCurrentSelection(-1);
	_DPlay.SelectPlayerToChatTo(row-1);
	
}

void CReadyRoom::OnTextChangedPlayerchat(LPSTR text) 
{
	// TODO: Add your control notification handler code here
		trunc(text,40);
}

void CReadyRoom::OnUpdatePlayerchat() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	
}
