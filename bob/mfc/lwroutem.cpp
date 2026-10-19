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

// RowanDialog: // LWRouteM.cpp : implementation file
//

#include "stdafx.h"
#include "MIGView.h"
#include "bob.h"
#include "LWRouteM.h"
#include    "persons2.h"
#include	"messengn.h"										//RJS 27Mar00
#include "globrefs.h"
#ifdef _DEBUG
//#define new DEBUG_NEW
#ifndef THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// LWRouteMain dialog
RDialog*	LWRouteMain::Make(int tab)
{
	return
		MakeTopDialog(::Place(POSN_CENTRE,POSN_CENTRE),
			DialBox(FIL_D_ROUTE,new LWRouteMain(tab))
			);
}



LWRouteMain::LWRouteMain(int t,CWnd* pParent /*=NULL*/)
	: RowanDialog(LWRouteMain::IDD, pParent)
{
	//{{AFX_DATA_INIT(LWRouteMain)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	tab = t;
}
void LWRouteMain::RefreshData(int childnum)
{
	if (childnum == -2)
	{
		RefreshList();
		RefreshButtons();
	}
}


void LWRouteMain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LWRouteMain)
	DDX_Control(pDX, IDC_RLISTBOXCTRL1, m_IDC_RLISTBOXCTRL1);
	DDX_Control(pDX, IDC_RBUTTONINSERT, m_IDC_RBUTTONINSERT);
	DDX_Control(pDX, IDC_RBUTTONDELETE, m_IDC_RBUTTONDELETE);
	DDX_Control(pDX, IDC_RBUTTONCENTRE, m_IDC_RBUTTONCENTRE);
	DDX_Control(pDX, IDC_RBUTTONNEWDOGLEG, m_IDC_RBUTTONNEWDOGLEG);
	DDX_Control(pDX, IDC_RBUTTONAPPLYALT, m_IDC_RBUTTONAPPLYALT);
	//}}AFX_DATA_MAP
	GETDLGITEM(IDC_RLISTBOXCTRL1)->AddString("",0);
}


BEGIN_MESSAGE_MAP(LWRouteMain, CDialog)
	//{{AFX_MSG_MAP(LWRouteMain)
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_SELECTTAB, OnSelectTab)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// LWRouteMain message handlers

BOOL LWRouteMain::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	

	RefreshList();
	RefreshButtons();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void	LWRouteMain::RefreshButtons()
{
  	if (Todays_Packages.hiwp != UID_Null)
	{
		info_waypoint* wp=*Persons2::ConvertPtrUID(Todays_Packages.hiwp);

		CRButton*	but;
		but = GETDLGITEM(IDC_RBUTTONINSERT);
		if (		(wp->uid.Evaluate() == -SGR_WPP_TakeOff)
				||	(wp->uid.Evaluate() == -SGR_WPP_EscDisperse)
				||	(wp->uid.Evaluate() == -SGR_WPP_BombDisperse)
				||	(wp->uid.Evaluate() == -SGR_WPP_Landing)
				||	(wp->uid.Evaluate() == -SGR_WPC_TakeOff)
				||	(wp->uid.Evaluate() == -SGR_WPC_Landing)

			)
		{
   			but->SetForeColor(RGB(160,160,0));
			but->SetBackColor(RGB(160,160,0));
			but->SetDisabled(true);
		}else
		{
			but->SetForeColor(RGB(255,255,0));
			but->SetBackColor(RGB(255,255,0));
			but->SetDisabled(false);
		}


		but = GETDLGITEM(IDC_RBUTTONDELETE);
		if (wp->uid.Evaluate() == GOT_UNDEFINED_VAL)				//user wp
		{
			but->SetForeColor(RGB(255,255,0));
			but->SetDisabled(false);
			but->SetBackColor(RGB(255,255,0));
		}else
		{
   			but->SetForeColor(RGB(160,160,0));
			but->SetBackColor(RGB(160,160,0));
			but->SetDisabled(true);
		}

		but = GETDLGITEM(IDC_RBUTTONCENTRE);
		but->SetForeColor(RGB(255,255,0));
		but->SetBackColor(RGB(255,255,0));
		but->SetDisabled(false);

		but = GETDLGITEM(IDC_RBUTTONNEWDOGLEG);
		if (RFullPanelDial::gameside == RFullPanelDial::SIDE_LW)
		{
			but->ShowWindow(true);
			if (Todays_Packages.pack[Todays_Packages.hipack].packagestatus < Profile::PS_TAKINGOFF)
			{
				but->SetForeColor(RGB(255,255,0));
				but->SetBackColor(RGB(255,255,0));
				but->SetDisabled(false);
			}else
			{
				but->SetForeColor(RGB(160,160,0));
				but->SetBackColor(RGB(160,160,0));
				but->SetDisabled(true);
			}
		}else
		{
			but->ShowWindow(false);
		}
		but = GETDLGITEM(IDC_RBUTTONAPPLYALT);
		but->SetForeColor(RGB(255,255,0));
		but->SetBackColor(RGB(255,255,0));
		but->SetDisabled(false);

	}else
	{
		CRButton*	but = GETDLGITEM(IDC_RBUTTONINSERT);
		but->SetForeColor(RGB(160,160,0));
		but->SetBackColor(RGB(160,160,0));
		but->SetDisabled(true);
		but = GETDLGITEM(IDC_RBUTTONDELETE);
		but->SetForeColor(RGB(160,160,0));
		but->SetBackColor(RGB(160,160,0));
		but->SetDisabled(true);
		but = GETDLGITEM(IDC_RBUTTONCENTRE);
		but->SetForeColor(RGB(160,160,0));
		but->SetBackColor(RGB(160,160,0));
		but->SetDisabled(true);
		but = GETDLGITEM(IDC_RBUTTONNEWDOGLEG);
		but->SetForeColor(RGB(160,160,0));
		but->SetBackColor(RGB(160,160,0));
		but->SetDisabled(true);
		but = GETDLGITEM(IDC_RBUTTONAPPLYALT);
		but->SetForeColor(RGB(160,160,0));
		but->SetBackColor(RGB(160,160,0));
		but->SetDisabled(true);
	}


}



void	LWRouteMain::OnSelectTab()
{
	RefreshList();
	RefreshButtons();

}

int	LWRouteMain::UpdateWpType(info_waypoint* wp, WpState	wpstate)
{
	switch  (wpstate)
	{
		case	FIRST:
		{
				wpstate = FIRSTTAKEOFF;
			break;
		}
		case	FIRSTTAKEOFF:
		{
				wpstate = TAKEOFF;
			break;
		}
		case	TAKEOFF:
		{
			if (wp->applygroup == GOT_UNDEFINED_VAL)
				wpstate = INGRESS;
			break;
		}
		case	INGRESS:
		{
			if (wp->applywave != GOT_UNDEFINED_VAL)
			{
				if (wp->applywave == -1)
					wpstate = TARGETESCORT;
				else
					wpstate = TARGETSTRIKE;
			}
			break;
		}
		case	TARGETSTRIKE:
		{
			if (wp->applywave == GOT_UNDEFINED_VAL)
				wpstate = EGRESS;
			if (wp->applywave == -1)
				wpstate = TARGETESCORT;
			break;
		}
		case	TARGETESCORT:
		{
			if (wp->applywave == GOT_UNDEFINED_VAL)
				wpstate = EGRESS;
			if (wp->applywave != -1)
				wpstate = TARGETSTRIKE;
			break;
		}
		case	EGRESS:
		{
			if (wp->applygroup != GOT_UNDEFINED_VAL)
				wpstate = LANDING;
			break;
		}
		case	LANDING:
		{
			if (wp->nextwp == UID_NULL)
				wpstate = LAST;
			break;
		}

	}
	return (wpstate);

}
void	LWRouteMain::FillWpList()
{

	WpState wpstate = FIRST;
		int i = 0;

	if (Todays_Packages.pack[Todays_Packages.hipack].packagestatus!=Profile::PS_SPARE)
	{
		Profile& pack = Todays_Packages.pack[Todays_Packages.hipack];

		UniqueID wpuid = pack.route;
		while (	(i < MAXWPS) && (wpuid))
		{
			info_waypoint* wp=*Persons2::ConvertPtrUID(wpuid);
			wpstate = (WpState)UpdateWpType(wp, wpstate);

 			if (	(	(	(wpstate == FIRSTTAKEOFF)
						||	(wpstate == TAKEOFF)
						)
						&&	(tab == 0)
					)
					||
					(	(	(wpstate == FIRSTTAKEOFF)
						||	(wpstate == INGRESS)
						||	(wpstate == EGRESS)
						)
						&&	(tab == 1)
					)
					||
					(	(	(wpstate == TARGETSTRIKE)
						)
						&&	(tab == 2)
					)
					||
					(	(	(wpstate == TARGETESCORT)
						)
						&&	(tab == 3)
					)
					||
					(	(	(wpstate == LANDING)
						||	(wpstate == LAST)
						)
						&&	(tab == 4)
					)
				)
			{
				if (wp->applywave != -2)
				{//not special dummy target wps: esitmated and target
					WpList[i] = wpuid;
					i++;
				}
			}
			wpuid = wp->nextwp;
		}

	}
	WpList[i] = UID_NULL;
	maxentry = i;
}


void	LWRouteMain::RefreshList()
{
	CString buffer;
	CRListBox* rlistbox;
	rlistbox=GETDLGITEM(IDC_RLISTBOXCTRL1);
	rlistbox->Clear();
		//rlistbox->AddColumn(160);
		//rlistbox->AddColumn(TARGETFIELDWIDTH);										  //RDH 02/06/99
		//rlistbox->AddColumn(70);										  //RDH 02/06/99
		//rlistbox->AddColumn(50);										  //RDH 02/06/99
		//rlistbox->AddColumn(50);
	rlistbox->AddString("",0);
	rlistbox->AddString(RESSTRING(RELTOT),4);
		rlistbox->AddString(RESSTRING(LOCATION),1);
	rlistbox->AddString(RESSTRING(ALT),2);
	rlistbox->AddString(RESSTRING(ETA),3);


	FillWpList();


		int i = 0;
		Profile& pack = Todays_Packages.pack[Todays_Packages.hipack];
	int currhilight = -1;

		while (WpList[i])
		{
			info_waypoint* wp=*Persons2::ConvertPtrUID(WpList[i]);
			if (wp->applygroup.Evaluate()!=ENABLE_COMPLEX_VAL)
			{
				Squadron& sq=pack[wp->applygroup.Evaluate()].Squadron();
				if (pack.attackmethod<Profile::AM_LWPACKS)
	 				rlistbox->AddString(LoadResString(Profile::WPType(WpList[i]))+" ("+sq.ShortName() + ")",0);
				else
	 				rlistbox->AddString(LoadResString(Profile::WPType(WpList[i]))+" ("+sq.ShortName() + "/"+ sq.SubName()+")",0);
			}
			else
			if (wp->applywave.Evaluate()!=ENABLE_COMPLEX_VAL && wp->applywave.Evaluate()>=0)
	 			rlistbox->AddString(CSprintf("%i: %s",wp->applywave.Evaluate(),LoadResString(Profile::WPType(WpList[i]))),0);
			else
				rlistbox->AddString(LoadResString(Profile::WPType(WpList[i])),0);

			{
				int& gri=wp->target.UsesGlobRef();
				if (&gri!=&EventVal::minus1)
				{
					int theref=gri;
					if (theref<=SGR_Pack_Target9 && theref>=SGR_Pack_Target)
						rlistbox->AddString(GetTargName(Todays_Packages.pack[Todays_Packages.hipack].packagetarget[theref-SGR_Pack_Target]),1);
					else
					if (theref==SGR_Pack_DogLeg)
   						rlistbox->AddString(GetTargName(Todays_Packages.pack[Todays_Packages.hipack].doglegloc),1);
					else
						rlistbox->AddString(_MsgBuffer.GetPlaceName(wp->World),1);
				}
				else
				if (	(wp->target != GOT_UNDEFINED_VAL)
					&&	 (wp->target.Evaluate())
					)
						rlistbox->AddString(GetTargName((UniqueID)wp->target.Evaluate()),1);
				else
				{
					CString name = _MsgBuffer.GetPlaceName(wp->World);	//RJS 27Mar00
					rlistbox->AddString(name,1);
				}
			}
			if (Profile::WPType(WpList[i]) == IDS_WP_TAKEOFF)
					rlistbox->AddString("",2);
			else
			{
				int alt = (wp->World.Y*10)/Save_Data.alt.mediummm;
				alt = 100*((alt + 50)/100);
				rlistbox->AddString(CSprintf("%i%s",alt,LoadResString(Save_Data.alt.mediumabbr)),2);
			}
			int t=wp->etas/60;
			rlistbox->AddString(CSprintf("%02i:%02i",t/60,t%60),3);
			t=t - pack.primarytargetETA/60;
		    if (t>=0)
				rlistbox->AddString(CSprintf("%02i:%02i",+t/60,t%60),4);
			else
				rlistbox->AddString(CSprintf("-%02i:%02i",-t/60,-t%60),4);
		
			if (Todays_Packages.hiwp == WpList[i])
				currhilight = i + 1;

			
			i++;
		}


	if (currhilight >= 0)
		rlistbox->SetHilightRow(currhilight);

}

BEGIN_EVENTSINK_MAP(LWRouteMain, CDialog)
    //{{AFX_EVENTSINK_MAP(LWRouteMain)
	ON_EVENT(LWRouteMain, IDC_RLISTBOXCTRL1, 1 /* Select */, OnSelectRlistboxctrl1, VTS_I4 VTS_I4)
	ON_EVENT(LWRouteMain, IDC_RBUTTONNEWDOGLEG, 1 /* Clicked */, OnClickedRbuttonnewdogleg, VTS_NONE)
	ON_EVENT(LWRouteMain, IDC_RBUTTONINSERT, 1 /* Clicked */, OnClickedRbuttoninsert, VTS_NONE)
	ON_EVENT(LWRouteMain, IDC_RBUTTONDELETE, 1 /* Clicked */, OnClickedRbuttondelete, VTS_NONE)
	ON_EVENT(LWRouteMain, IDC_RBUTTONCENTRE, 1 /* Clicked */, OnClickedRbuttoncentre, VTS_NONE)
	ON_EVENT(LWRouteMain, IDC_RBUTTONAPPLYALT, 1 /* Clicked */, OnClickedRbuttonapplyalt, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void LWRouteMain::OnSelectRlistboxctrl1(long row, long column) 
{

	
	int index = row -1;
	if (		(row > 0)
			&&	(row <= maxentry)
		)
	{
		RDialog::m_pView->SetHiLightInfo(Todays_Packages.hipack, 0,WpList[index]);

		RefreshList();

		RefreshButtons();
		RefreshWpDialog();
		m_pView->m_mapdlg.Invalidate();




	}



	
}
void LWRouteMain::RefreshWpDialog()
{
#ifndef	BOB_DEMO_VER

//RefreshChild didn't work
//TEMPCODE RDH 07/01/00 	RDialog*	wpdial = MainToolBar().LoggedChild(CMainToolbar::WAYPOINT);
//TEMPCODE RDH 07/01/00 	if (wpdial)
//TEMPCODE RDH 07/01/00 	{
//TEMPCODE RDH 07/01/00 		MainToolBar().RefreshChild(CMainToolbar::WAYPOINT);
//TEMPCODE RDH 07/01/00 	}

		if (Todays_Packages.hiwp != UID_NULL)
				MainToolBar().OpenWP();

#endif
}
void LWRouteMain::OnClickedRbuttonnewdogleg() 
{
	Profile& pk = Todays_Packages.pack[Todays_Packages.hipack];

	TargetGroup*	targp=Node_Data[pk.packagetarget[0].currentid];

	int  i = 0;
	while  (	(pk.doglegloc != targp->doglegs[i])
			&&	(i < 3)
			)
			i++;
	i++;
	if (i >= 3 || targp->doglegs[i] == UID_Null)
		i = 0;
	if (targp->doglegs[i] != UID_Null)
	{
		pk.doglegloc = targp->doglegs[i];

		{


			RefreshWpDialog();
			Todays_Packages[Todays_Packages.hipack].RecalcRoutePositionsAndTime();
 			m_pView->m_mapdlg.Invalidate();
			RefreshList();
			RefreshButtons();

		}
	}

	
}

void LWRouteMain::OnClickedRbuttoninsert() 
{
	WayPointDialog::ClickedOnInsert();
	RefreshList();
	RefreshButtons();
	RefreshWpDialog();

	
}

void LWRouteMain::OnClickedRbuttondelete() 
{
	WayPointDialog::ClickedOnDelete();
	RefreshList();
	RefreshButtons();
	RefreshWpDialog();

	
}

void LWRouteMain::OnClickedRbuttoncentre() 
{
	m_pView->ZoomToUID(UniqueID(Todays_Packages.hiwp),m_pView->m_zoom);
	
}

void LWRouteMain::OnClickedRbuttonapplyalt() 
{
	if (Todays_Packages.pack[Todays_Packages.hipack].packagestatus!=Profile::PS_SPARE)
	{
		Profile& pack = Todays_Packages.pack[Todays_Packages.hipack];

		UniqueID wpuid = pack.route;
		info_waypoint* cwp=*Persons2::ConvertPtrUID(Todays_Packages.hiwp);
		int worldY=cwp->World.Y;
		if (cwp->uid==-SGR_WPP_BombRendezvous || cwp->uid==-SGR_WPP_BombDisperse)
			worldY+=worldY;

		Todays_Packages[Todays_Packages.hipack].SetTargetGlobrefs();						//CSB 27Jun00
		Todays_Packages[Todays_Packages.hipack].RecalcPrimaryPositions();					//CSB 27Jun00

		while (wpuid)
		{
			info_waypoint* wp=*Persons2::ConvertPtrUID(wpuid);

			if (wp->applygroup.Evaluate()==ENABLE_COMPLEX_VAL)	//Not TO or land		//JIM 28Jun00
			{
				if (wp->uid==-SGR_WPP_BombRendezvous || wp->uid==-SGR_WPP_BombDisperse)
					wp->World.Y = worldY/2;
				else
					wp->World.Y = worldY;
				Coords3D	pos=wp->World;
				if (wp->target.Evaluate()!=ENABLE_COMPLEX_VAL)
					pos-=Persons2::ConvertPtrUID(UniqueID(wp->target.Evaluate()));
				wp->position.RecalcDeltasToMatch(pos);					//CSB 27Jun00
			}


			wpuid = wp->nextwp;
		}
		if (RFullPanelDial::gameside == RFullPanelDial::SIDE_LW)
		{
			Directives::LW::MissTypes misstype = LWDirectivesResults::LWMissType();
			MMC.directives.lw.userprofiles[misstype].mainalt = cwp->World.Y;
		}else
		{
  			Directives::RAF::MissTypes misstype = RAFDirectivesResults::RAFMissType(Todays_Packages.hipack);
			MMC.directives.raf.userprofiles[misstype].mainalt = cwp->World.Y;
		}

	}
    while (!Todays_Packages[Todays_Packages.hipack].CalcWPTimes())
		Todays_Packages[Todays_Packages.hipack].primarytargetETA+=10;
	RefreshList();
	RefreshButtons();
}
