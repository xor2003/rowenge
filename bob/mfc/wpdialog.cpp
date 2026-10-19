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

// RowanDialog: // WPDialog.cpp : implementation file
//
#include "stdafx.h"
#include "MIGView.h"
#include "_mfc.h"
#include "bob.h"
#include "WPDialog.h"
#include "package.h"
#include "persons2.h"
#include "mytime.h"
#include "monotxt.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#ifndef THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// WayPointDialog dialog


RDialog*	WayPointDialog::Make()
{
	return
		MakeTopDialog(::Place(POSN_MAX,POSN_MAX),
			DialBox(FIL_D_WAYPOINT,new WayPointDialog())
			);
}


WayPointDialog::WayPointDialog( CWnd* pParent /*=NULL*/)
	: RowanDialog(WayPointDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(WayPointDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	stepsize = 5;
	steps = 32;
	int t;
	t = Todays_Packages[Todays_Packages.hipack].primarytargetETA;
	earliestgoodeta=t;
	earliesttrgeta =MMC.currtime;//t;//Todays_Packages[Todays_Packages.hipack].FindEarliestPrimaryTargetETA(t);
	acnum=Todays_Packages.hiwp;
	
}


void WayPointDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WayPointDialog)
	DDX_Control(pDX, IDC_RLISTBOXCTRL1, m_IDC_RLISTBOXCTRL1);
	DDX_Control(pDX, IDC_RSTATICRNGFROM, m_IDC_RSTATICRNGFROM);
	DDX_Control(pDX, IDC_RSTATICRELTOT, m_IDC_RSTATICRELTOT);
	DDX_Control(pDX, IDC_RSTATICFUEL, m_IDC_RSTATICFUEL);
	DDX_Control(pDX, IDC_RSTATICETA, m_IDC_RSTATICETA);
	DDX_Control(pDX, IDC_RSTATICESCORTFUEL, m_IDC_RSTATICESCORTFUEL);
	DDX_Control(pDX, IDC_RSTATICBRGFROM, m_IDC_RSTATICBRGFROM);
	DDX_Control(pDX, IDC_RCOMBOALT, m_IDC_RCOMBOALT);
	DDX_Control(pDX, IDJ_TITLE, m_IDJ_TITLE);
	DDX_Control(pDX, IDC_RSPINETA, m_IDC_RSPINETA);
	DDX_Control(pDX, IDC_RBUTTONDELETE, m_IDC_RBUTTONDELETE);
	DDX_Control(pDX, IDC_RBUTTONINSERT, m_IDC_RBUTTONINSERT);
	DDX_Control(pDX, IDC_RSTATICALT, m_IDC_RSTATICALT);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(WayPointDialog, CDialog)
	//{{AFX_MSG_MAP(WayPointDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WayPointDialog message handlers
void WayPointDialog::RefreshData(int childnum)
{
	if (childnum == -2)
		if (acnum!=Todays_Packages.hiwp)
			OnCancel();
		else
			Refresh();
}

void WayPointDialog::Refresh(bool remakespin)
{
	info_waypoint* wp=*Persons2::ConvertPtrUID(Todays_Packages.hiwp);


	CRButton* title;
	title = GETDLGITEM(IDJ_TITLE);

	if (wp)
		if (wp->applywave==-2)
		{
			title->SetString(CSprintf("(%s)",LoadResString(Profile::WPType(Todays_Packages.hiwp))));
			CRCombo* combo=GETDLGITEM(IDC_RCOMBOALT);
			combo->Clear();
			combo->SetCaption("");
			GETDLGITEM(IDC_RSTATICALT)->SetString("");
			GETDLGITEM(IDC_RSTATICRELTOT)->SetString("");
			GETDLGITEM(IDC_RSTATICFUEL)->SetString("");
			GETDLGITEM(IDC_RSTATICESCORTFUEL)->SetString("");
			{
 				CRButton*	but = GETDLGITEM(IDC_RBUTTONDELETE);
  				but->SetForeColor(RGB(160,160,0));
				but->SetBackColor(RGB(160,160,0));
				but->SetDisabled(true);
			}
			{
 				CRButton*	but = GETDLGITEM(IDC_RBUTTONINSERT);
  				but->SetForeColor(RGB(160,160,0));
				but->SetBackColor(RGB(160,160,0));
				but->SetDisabled(true);
			}
				CRSpinBut* spinbut;
				spinbut=GETDLGITEM(IDC_RSPINETA);
				spinbut->ShowWindow(SW_HIDE);
			CRStatic*   s;
				s=GETDLGITEM(IDC_RSTATICETA);
				s->ShowWindow(SW_SHOW);
				s->SetString("");
		}
		else
		{
			title->SetString(LoadResString(Profile::WPType(Todays_Packages.hiwp)));
			CRStatic*   s;
			s=GETDLGITEM(IDC_RSTATICALT);
			s->SetString(CSprintf("%s, %s", RESSTRING(ALT),LoadResString(Save_Data.alt.mediumabbr)));
			int t;
			s=GETDLGITEM(IDC_RSTATICRELTOT);
			t=wp->etas/60;
			t=t - Todays_Packages[Todays_Packages.hipack].primarytargetETA/60;
			if (t>=0)
				s->SetString(CSprintf("+%02i:%02i",t/60,t%60));
			else
				s->SetString(CSprintf("-%02i:%02i",-t/60,-t%60));


			int strikefuel, escortfuel;
			Todays_Packages[Todays_Packages.hipack].CalcFuel(Todays_Packages.hiwp, strikefuel, escortfuel);
			s=GETDLGITEM(IDC_RSTATICFUEL);
			if (strikefuel >= 0)
				s->SetString(CSprintf("%3i%s",strikefuel,RESSTRING(MINS)));
			else
				s->SetString(CSprintf("%s",RESSTRING(NA)));

			s=GETDLGITEM(IDC_RSTATICESCORTFUEL);
			if (escortfuel >= 0)
				s->SetString(CSprintf("%3i%s",escortfuel,RESSTRING(MINS)));
			else
				s->SetString(CSprintf("%s",RESSTRING(NA)));



			if (wp->prevwp)
			{
				ULong range,brg;
				Todays_Packages[Todays_Packages.hipack].CalcRangeBrgFromPrev(Todays_Packages.hiwp,range,brg);
	 			s=GETDLGITEM(IDC_RSTATICRNGFROM);
				s->SetString(CSprintf("%i %s", (int)range, LoadResString(Save_Data.dist.longabbr)));
				s=GETDLGITEM(IDC_RSTATICBRGFROM);
				s->SetString(CSprintf("%i°", (int)brg));
			}else
			{
	 			s=GETDLGITEM(IDC_RSTATICRNGFROM);
				s->SetString(RESSTRING(NA));
				s=GETDLGITEM(IDC_RSTATICBRGFROM);
				s->SetString(RESSTRING(NA));
			}

				CRButton*	but = GETDLGITEM(IDC_RBUTTONDELETE);
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

			CRCombo* combo=GETDLGITEM(IDC_RCOMBOALT);
			combo->Clear();

			int altind = (wp->World.Y*10+ (Save_Data.alt.mediummm*Save_Data.alt.mediumstepsize)/2)/(Save_Data.alt.mediummm*Save_Data.alt.mediumstepsize)-Save_Data.alt.minstepcount;
			if (altind <0)
				altind = 0;
			if (altind >= (Save_Data.alt.maxstepcount - Save_Data.alt.minstepcount))
			altind = Save_Data.alt.maxstepcount - Save_Data.alt.minstepcount - 1;
			combo->MakeNumList(Save_Data.alt.maxstepcount-Save_Data.alt.minstepcount,
						Save_Data.alt.minstepcount*Save_Data.alt.mediumstepsize,
						Save_Data.alt.mediumstepsize
							)->SetIndex(altind);

			CRSpinBut* spinbut;
			spinbut=GETDLGITEM(IDC_RSPINETA);


			if (Profile::WPType(Todays_Packages.hiwp) == IDS_WP_IP)
			{
				s=GETDLGITEM(IDC_RSTATICETA);
				s->ShowWindow(SW_HIDE);
				spinbut->ShowWindow(SW_SHOW);

				if (remakespin)
					MakeTimeEntries(Todays_Packages[Todays_Packages.hipack].primarytargetETA);
			}else
			{
				spinbut->ShowWindow(SW_HIDE);
				s=GETDLGITEM(IDC_RSTATICETA);
				s->ShowWindow(SW_SHOW);
				t=wp->etas/60;
				s->SetString(CSprintf("%02i:%02i",t/60,t%60));
 
			}
		}
}
BOOL WayPointDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	Refresh();

	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(WayPointDialog, CDialog)
    //{{AFX_EVENTSINK_MAP(WayPointDialog)
	ON_EVENT(WayPointDialog, IDC_RBUTTONNEXT, 1 /* Clicked */, OnClickedRbuttonnext, VTS_NONE)
	ON_EVENT(WayPointDialog, IDC_RBUTTONINSERT, 1 /* Clicked */, OnClickedRbuttoninsert, VTS_NONE)
	ON_EVENT(WayPointDialog, IDC_RBUTTONDELETE, 1 /* Clicked */, OnClickedRbuttondelete, VTS_NONE)
	ON_EVENT(WayPointDialog, IDC_RBUTTONCENTRE, 1 /* Clicked */, OnClickedRbuttoncentre, VTS_NONE)
	ON_EVENT(WayPointDialog, IDC_RBUTTONPREV, 1 /* Clicked */, OnClickedRbuttonprev, VTS_NONE)
	ON_EVENT(WayPointDialog, IDC_RBUTTONROUTE, 1 /* Clicked */, OnClickedRbuttonroute, VTS_NONE)
	ON_EVENT(WayPointDialog, IDC_RCOMBOALT, 1 /* TextChanged */, OnTextChangedRcomboalt, VTS_BSTR VTS_I2)
	ON_EVENT(WayPointDialog, IDC_RSPINETA, 1 /* TextChanged */, OnTextChangedRspineta, VTS_BSTR VTS_I2)
	ON_EVENT(WayPointDialog, IDC_RSPINETA, 2 /* SpinCompleted */, OnSpinCompletedRspineta, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void WayPointDialog::OnClickedRbuttonnext() 
{
#ifndef	BOB_DEMO_VER

	if (Todays_Packages.hiwp )
	{
		info_waypoint* wp=*Persons2::ConvertPtrUID(Todays_Packages.hiwp );
		if (wp->nextwp)
		{
			RDialog::m_pView->SetHiLightInfo(Todays_Packages.hipack, 0,wp->nextwp);

			Refresh();

			RDialog*	routedial = MainToolBar().LoggedChild(CMainToolbar::ROUTE);
			if (routedial)
			{
				CRTabs* tab=(CRTabs*)routedial->fchild->fchild->GetDlgItem(IDJ_TABCTRL);
				tab->SelectTab(1);
			}
			m_pView->m_mapdlg.Invalidate();

		}


	}
#endif	
}

void WayPointDialog::OnClickedRbuttoninsert() 
{
#ifndef	BOB_DEMO_VER

	ClickedOnInsert();
	Refresh();
	RDialog*	routedial = MainToolBar().LoggedChild(CMainToolbar::ROUTE);
	if (routedial)
	{
		routedial->GlobalRefreshData();
	}

#endif
}
void WayPointDialog::ClickedOnInsert()
{
	//NEED TO HANDLE THAT THE CURR WP MAY BE A LOCAL WP
	//not doing this: IF TO/LAND then need squadline as next param
	//IF TARGET AREA need target number or -1 for patrol
	//
	//I think that the only way to handle this is by changing the buttons as we did in MiG:
	//TakeOff:		Delete disabled		Insert after DISABLEDenabled			--> same squadline
	//Any SetUID:	Delete disabled
	//		RV,EP,Trg,bmbdisp			Insert after disabled
	//Landing:		Delete disabled		Insert -> insert before			--> same squadline
	//Bomb/Patrol	Delete removed		Insert before + Insert after	--> same targindex
	//User:			Delete enabled		Insert after					--> same squadline/targindex
	info_waypoint* wp=*Persons2::ConvertPtrUID(Todays_Packages.hiwp);
	UniqueID newwp = UID_NULL;

	if (wp->uid.Evaluate() == -SGR_WPP_IP)
	{
		UniqueID patrolwp = wp->NextWP(-1,-1);	//do nothing, wavenum = -1 is patrol
		if (patrolwp)
		{
			info_waypoint* wp=*Persons2::ConvertPtrUID(patrolwp);
			if (wp->applywave.Evaluate() == -1)		//if patrol wp
				newwp=InsertWaypoint(Todays_Packages.hipack,Todays_Packages.hiwp, GOT_UNDEFINED_VAL, -1);
		}
		for (int i = 0; i < 10;i++)
		{
			if (wp->NextWP(-1,i)!=wp->NextWP(-10,-10))
			{
				newwp=InsertWaypoint(Todays_Packages.hipack,Todays_Packages.hiwp, ENABLE_COMPLEX_VAL, i);
			}
		}
	}else
	{
		int trgline = wp->applywave.Evaluate();
		int squadline = wp->applygroup.Evaluate();
		newwp=InsertWaypoint(Todays_Packages.hipack,Todays_Packages.hiwp, squadline, trgline);
	}
	if (newwp)
	{
		RDialog::m_pView->SetHiLightInfo(Todays_Packages.hipack, Todays_Packages.hisquad, newwp);

		Todays_Packages[Todays_Packages.hipack].RecalcRoutePositionsAndTime();

	}
}

void WayPointDialog::OnClickedRbuttondelete() 
{
#ifndef	BOB_DEMO_VER

	ClickedOnDelete();
	Refresh();
	RDialog*	routedial = MainToolBar().LoggedChild(CMainToolbar::ROUTE);
	if (routedial)
	{
		routedial->GlobalRefreshData();
	}

#endif
}
void WayPointDialog::ClickedOnDelete()
{
	info_waypoint* wp=*Persons2::ConvertPtrUID(Todays_Packages.hiwp);
	UniqueID nextwp = wp->NextWP(-1,-1);

	if (DeleteWayPoint(Todays_Packages.hipack,Todays_Packages.hiwp))
	{

		RDialog::m_pView->SetHiLightInfo(Todays_Packages.hipack, Todays_Packages.hisquad, nextwp);

		Todays_Packages[Todays_Packages.hipack].RecalcRoutePositionsAndTime();
	}
		
}

void WayPointDialog::OnClickedRbuttoncentre() 
{
	m_pView->ZoomToUID(UniqueID(Todays_Packages.hiwp ),m_pView->m_zoom);

	
}

void WayPointDialog::OnClickedRbuttonprev() 
{
#ifndef	BOB_DEMO_VER

	if (Todays_Packages.hiwp )
	{
		info_waypoint* wp=*Persons2::ConvertPtrUID(Todays_Packages.hiwp );
		if (wp->prevwp)
		{
			RDialog::m_pView->SetHiLightInfo(Todays_Packages.hipack, 0,wp->prevwp);

			Refresh();
			RDialog*	routedial = MainToolBar().LoggedChild(CMainToolbar::ROUTE);
			if (routedial)
			{
				CRTabs* tab=(CRTabs*)routedial->fchild->fchild->GetDlgItem(IDJ_TABCTRL);
				tab->SelectTab(1);
			}
			m_pView->m_mapdlg.Invalidate();

		}
	}

#endif	
}

void WayPointDialog::OnClickedRbuttonroute() 
{
#ifndef	BOB_DEMO_VER

	MainToolBar().OpenRoute();
#endif	
}

void WayPointDialog::OnTextChangedRcomboalt(LPCTSTR Caption, short index) 
{
#ifndef	BOB_DEMO_VER

	info_waypoint* wp=*Persons2::ConvertPtrUID(Todays_Packages.hiwp );
			
	Todays_Packages[Todays_Packages.hipack].SetTargetGlobrefs();						//CSB 27Jun00
	Todays_Packages[Todays_Packages.hipack].RecalcPrimaryPositions();					//CSB 27Jun00
	wp->World.Y = (index + Save_Data.alt.minstepcount) * (Save_Data.alt.mediummm*Save_Data.alt.mediumstepsize)/10;
	Coords3D pos=wp->World;
	if (wp->target.Evaluate()!=ENABLE_COMPLEX_VAL)
	{
		Coords3D tpos=Persons2::ConvertPtrUID(UniqueID(wp->target.Evaluate()))->World;
		pos.X-=tpos.X;
		pos.Y-=tpos.Y;
		pos.Z-=tpos.Z;
	}
	wp->position.RecalcDeltasToMatch(pos);												//CSB 28Jun00
	



    while (!Todays_Packages[Todays_Packages.hipack].CalcWPTimes())
		Todays_Packages[Todays_Packages.hipack].primarytargetETA+=5;

	RDialog*	routedial = MainToolBar().LoggedChild(CMainToolbar::ROUTE);
	if (routedial)
	{
		routedial->GlobalRefreshData();
	}

#endif	
}

void	WayPointDialog::MakeTimeEntries(int ETA)
{
	const int mintime=MMC.currtime+SECSPERMIN*5,maxtime=HR21;
	int counttime=ETA;
	int fillpoint=CURRENTRY;
	timeentries[fillpoint--]=counttime;
	counttime=counttime+9;
	counttime=counttime-(counttime%10);	//go to 10 sec boundary (always don't insert current)
	int nextcheck=counttime-(counttime%60);	//set up 1 min boundary for 10 sec steps
	int	nextfill=fillpoint-4;				
	if (nextcheck<mintime)		nextcheck=mintime;
	while (counttime>nextcheck)	timeentries[fillpoint--]=(counttime-=10);
	if (nextcheck!=mintime)
	{
		if (fillpoint>nextfill)		nextcheck=counttime-60;
		if (nextcheck<mintime)		nextcheck=mintime;
		while (counttime>nextcheck)	timeentries[fillpoint--]=(counttime-=10);
		if (nextcheck!=mintime)
		{
			nextcheck=counttime-(counttime%(5*SECSPERMIN));	//set up 5 min boundary for 1 min steps
			if (nextcheck<mintime)		nextcheck=mintime;
			nextfill=fillpoint-4;
			while (counttime>nextcheck)	timeentries[fillpoint--]=(counttime-=60);
			if (nextcheck!=mintime)
			{
				if (fillpoint>nextfill)		nextcheck=counttime-5*SECSPERMIN;
				while (counttime>nextcheck)	timeentries[fillpoint--]=(counttime-=60);
				if (nextcheck!=mintime)
				{
					nextcheck=counttime-(counttime%(30*SECSPERMIN));	//set up 30 min boundary for 5 min steps
					if (nextcheck<mintime)		nextcheck=mintime;
					nextfill=fillpoint-4;
					while (counttime>nextcheck)	timeentries[fillpoint--]=(counttime-=5*SECSPERMIN);
					if (nextcheck!=mintime)
					{
						if (fillpoint>nextfill)		nextcheck=counttime-30*60;
						if (nextcheck<mintime)		nextcheck=mintime;
						while (counttime>nextcheck)	timeentries[fillpoint--]=(counttime-=5*SECSPERMIN);
						if (nextcheck!=mintime)
						{
							nextcheck=mintime;
							while (counttime>nextcheck)	timeentries[fillpoint--]=(counttime-=15*SECSPERMIN);
						}
					}
				}
			}

		}
	}
	Mono_Text.Print(fillpoint,(UByteP)"Fillpoint -");
	starttimeentries=timeentries+fillpoint+1;
 	fillpoint=CURRENTRY+1;
	counttime=ETA-(counttime%10);	//go to 10 sec boundary (always don't insert current)
	nextcheck=counttime+60-(counttime%60);	//set up 1 min boundary for 10 sec steps
	nextfill=fillpoint+4;				
	if (nextcheck>maxtime)		nextcheck=maxtime;
	while (counttime<nextcheck)	timeentries[fillpoint++]=(counttime+=10);
	if (nextcheck!=maxtime)
	{
		if (fillpoint<nextfill)		nextcheck=counttime+60;
		if (nextcheck>maxtime)		nextcheck=maxtime;
		while (counttime<nextcheck)	timeentries[fillpoint++]=(counttime+=10);
		if (nextcheck!=mintime)
		{
			nextcheck=counttime+5*SECSPERMIN-(counttime%(5*SECSPERMIN));	//set up 5 min boundary for 1 min steps
			if (nextcheck>maxtime)		nextcheck=maxtime;
			nextfill=fillpoint+4;
			while (counttime<nextcheck)	timeentries[fillpoint++]=(counttime+=60);
			if (nextcheck!=mintime)
			{
				if (fillpoint<nextfill)		nextcheck=counttime+5*SECSPERMIN;
				while (counttime<nextcheck)	timeentries[fillpoint++]=(counttime+=60);
				if (nextcheck!=mintime)
				{
					nextcheck=counttime+30*SECSPERMIN-(counttime%(30*SECSPERMIN));	//set up 30 min boundary for 5 min steps
					if (nextcheck<mintime)		nextcheck=mintime;
					nextfill=fillpoint-4;
					while (counttime<nextcheck)	timeentries[fillpoint++]=(counttime+=5*SECSPERMIN);
					if (nextcheck!=mintime)
					{
						if (fillpoint<nextfill)		nextcheck=counttime+30*60;
						if (nextcheck>mintime)		nextcheck=mintime;
						while (counttime<nextcheck)	timeentries[fillpoint++]=(counttime+=5*SECSPERMIN);
						if (nextcheck!=maxtime)
						{
							nextcheck=maxtime;
							while (counttime<nextcheck)	timeentries[fillpoint++]=(counttime+=15*SECSPERMIN);
						}
					}
				}
			}
		}
	}
	CRSpinBut* spinbut;
    spinbut=GETDLGITEM(IDC_RSPINETA);
	spinbut->Clear();
	timeentries[fillpoint]=0;
	for (int i=0;starttimeentries[i];i++)
		if (starttimeentries[i]%60)
			spinbut->AddString(CSprintf("%02i:%02i:%02i",starttimeentries[i]/3600,(starttimeentries[i]/60)%60,(starttimeentries[i])%60));
		else
			spinbut->AddString(CSprintf("%02i:%02i",starttimeentries[i]/3600,(starttimeentries[i]/60)%60));

	spinbut->SetIndex(timeentries+CURRENTRY-starttimeentries);
}

void WayPointDialog::OnTextChangedRspineta(LPCTSTR caption, short Index) 
{
#ifndef	BOB_DEMO_VER

		int time =starttimeentries[Index];

		Profile&pk=	Todays_Packages[Todays_Packages.hipack];
		bool targetmobile=false;

		int oldeta=	pk.primarytargetETA;

		if (pk.packagetarget[1])
		{
			ItemBasePtr p=Persons2::ConvertPtrUID(pk.packagetarget[1]);
			if (p && p->Status.size>=MOBILESIZE)
				targetmobile=true;
		}
		if (pk.packagetarget[0])
		{
			ItemBasePtr p=Persons2::ConvertPtrUID(pk.packagetarget[0]);
			if (p && p->Status.size>=MOBILESIZE)
				targetmobile=true;
		}
		if (targetmobile)
			Todays_Packages.InvalidateRoute(&pk,true);

		if (		(time >= earliesttrgeta)
				&&	(time < HR21)
		   )
		{
			Todays_Packages.squelchfuelwarning=true;
			pk.CalcRoutePositionsAndTime(time,false);
			if (pk.primarytargetETA==time)
			{
				if (earliestgoodeta>pk.primarytargetETA)
					earliestgoodeta=pk.primarytargetETA;
			}
			else
			{
				pk.CalcRoutePositionsAndTime(earliestgoodeta,false);
				MakeTimeEntries(pk.primarytargetETA);
			}
			Todays_Packages.squelchfuelwarning=false;
		}
		if (targetmobile)
			Todays_Packages.InvalidateRoute(&pk,true);


		Refresh(false);
	RDialog*	routedial = MainToolBar().LoggedChild(CMainToolbar::ROUTE);
	if (routedial)
	{
		routedial->GlobalRefreshData();
	}

#endif	
}
void WayPointDialog::OnSpinCompletedRspineta() 
{
	// TODO: Add your control notification handler code here
	int fuel;														//rdh 17Jul00
	SquadNum sq;
		Profile&pk=	Todays_Packages[Todays_Packages.hipack];
	if ((sq=pk.CalcFuel(UID_NULL, fuel, fuel))!=SQ_ZERO)
		Todays_Packages.MessageBoxFuelWarning(sq);
	MakeTimeEntries(pk.primarytargetETA);
	
}

bool WayPointDialog::DeleteWayPoint(int packnum,UniqueID u) 
{
	// TODO: Add your control notification handler code here
	if (u)
	{
		RDialog::m_pView->InvalidateRoute(packnum);
		info_waypoint* wp=*Persons2::ConvertPtrUID(u);
		RDialog::m_pView->m_mapdlg.InvalidateIcon(wp);
		if (wp->uid==ENABLE_COMPLEX_VAL)
		{	//I can delete it!
			info_waypoint* wp2=*Persons2::ConvertPtrUID(wp->nextwp);
			if (wp2)
				wp2->prevwp=wp->prevwp;
			wp2=*Persons2::ConvertPtrUID(wp->prevwp);
			if (wp2)
				wp2->nextwp=wp->nextwp;
			else
				Todays_Packages[packnum].route=wp->nextwp;
			delete wp;
			Todays_Packages[packnum].wpsused--;
			Persons2::delUID(u);
			RDialog::m_pView->InvalidateRoute(packnum);
		}
		return true;
	}
	else
		return false;
}


UniqueID WayPointDialog::InsertWaypoint(int packnum,UniqueID insertafter,int squadline,int trgline) 
{
	if (Todays_Packages[packnum].wpsused>60+Todays_Packages[packnum].squadlist.Max()*2)
		return UID_NULL;															//JIM 29Sep00

	// TODO: Add your control notification handler code here
	int	relfromcode=SGR_Pack_Takeoff,reltocode=SGR_Pack_Takeoff;
	COORDS3D	*posfrom=NULL,*posto=NULL;
	COORDS3D	*relfrom=NULL,*relto=NULL;
	info_waypoint	*prev=NULL,*next=NULL;

		RDialog::m_pView->InvalidateRoute(packnum);
	UniqueID u=insertafter,insertbefore=UID_NULL;
	if (u)
	{
		info_waypoint *t=
			prev=
				*Persons2::ConvertPtrUID(u);
		u=t->nextwp;
		do{
			if (t->applygroup.Evaluate()==ENABLE_COMPLEX_VAL || t->applygroup.Evaluate()==squadline)
				if (t->applywave.Evaluate()==ENABLE_COMPLEX_VAL || t->applywave.Evaluate()==trgline)
				{
					break;
				}
			t=*Persons2::ConvertPtrUID(t->prevwp);
		}while(t);
		posfrom=&t->World;
		do{
			if ((t->uid!=ENABLE_COMPLEX_VAL && t->uid>-SGR_WPP_Landing) || t->wpact==wpatakeoffno)
				if (t->applygroup.Evaluate()==ENABLE_COMPLEX_VAL)// || t->applygroup.Evaluate()==squadline)
					if (t->applywave.Evaluate()==ENABLE_COMPLEX_VAL)// || t->applywave.Evaluate()==trgline)
					{
						relfromcode=-t->uid;
						relfrom=&t->World;
						break;
					}
			t=*Persons2::ConvertPtrUID(t->prevwp);
		}while(t);

	}

	if (u)
	{
		info_waypoint *t=
				*Persons2::ConvertPtrUID(u);
		insertbefore=u;
		next=t;
		do{
			if (t->applygroup.Evaluate()==ENABLE_COMPLEX_VAL || t->applygroup.Evaluate()==squadline)
				if (t->applywave.Evaluate()==ENABLE_COMPLEX_VAL || t->applywave.Evaluate()==trgline)
				{
					break;
				}
			t=*Persons2::ConvertPtrUID(t->nextwp);
		}while(t);
		posto=&t->World;

		do{
			if ((t->uid!=ENABLE_COMPLEX_VAL && t->uid>-SGR_WPP_Landing) || t->wpact==wpalandingno)
				if (t->applygroup.Evaluate()==ENABLE_COMPLEX_VAL)// || t->applygroup.Evaluate()==squadline)
					if (t->applywave.Evaluate()==ENABLE_COMPLEX_VAL)// || t->applywave.Evaluate()==trgline)
					{
						reltocode=-t->uid;
						relto=&t->World;
						break;
					}
			t=*Persons2::ConvertPtrUID(t->nextwp);
		}while(t);
	}

	if (!(posfrom&&posto&&relfrom&&relto))
	{
		_Error.EmitSysErr("Unexpected wp insert problem");
	}
//Ok.. make a waypoint. 
//Set its position to midway between posfrom and posto
//Set the description of its position to the same place but
//using relto and relfrom to drive it!!!!
	u=Profile::MakeInterceptWP((SavedGlobrefs)relfromcode,(SavedGlobrefs)reltocode,50,UID_Null,SGR_Pack_BlankZero);
	if (u)
	{
		Todays_Packages[packnum].wpsused++;
		info_waypoint* w=*Persons2::ConvertPtrUID(u);
		if (trgline>=-1)		//-1 is escort patrol. -2 is estimated dummy
			w->applywave=trgline;
		else
		if (squadline>=0)
			w->applygroup=squadline;
		w->World.X=(posfrom->X+posto->X)/2;
		w->World.Y=(posfrom->Y+posto->Y)/2;
		w->World.Z=(posfrom->Z+posto->Z)/2;
		w->position.RecalcDeltasToMatch(w->World);
		if (next)
		{
			w->nextwp=insertbefore;
			next->prevwp=u;
		}
		else
			w->nextwp=UID_Null;

		if (prev)
		{
			w->prevwp=insertafter;
			prev->nextwp=u;
		}
		else
		{
			INT3;
			Todays_Packages[packnum].route=u;
			w->prevwp=UID_Null;	
		}
		RDialog::m_pView->m_mapdlg.InvalidateIcon(w);
		RDialog::m_pView->InvalidateRoute(packnum);
	}
	return u;
}

