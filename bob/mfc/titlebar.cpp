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

// RowanDialog: // TitleBar.cpp : implementation file
//

#include "stdafx.h"
#include "mig.h"
#include "TitleBar.h"
#include "migview.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#ifndef THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif
/////////////////////////////////////////////////////////////////////////////
// TitleBar dialog

TitleBar::TitleBar(CWnd* pParent /*=NULL*/)
	: CRToolBar(pParent)
{
	//{{AFX_DATA_INIT(TitleBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
void TitleBar::ChildDialClosed(int dialnum,RDialog*,int rv)
{
	if (dialnum==TIMECONTROL)
		GETDLGITEM(IDC_DATETIME)->SetPressed(FALSE);

}

void TitleBar::Redraw(bool setaccel)
{
	CRButton*   b;
	b=GETDLGITEM(IDC_DATETIME);
	b->SetString(CSprintf("%s %s x%i",
		GetDateName(MMC.currdate, DATE_LONG),GetTimeName(MMC.currtime),MMC.curraccelrate	));
	COLORREF arrowcolour=RGB(50,50,50);

	switch (MMC.TimeColour())
	{
	case Campaign::TC_RED:	arrowcolour=RGB(255,0,0);	break;
	case Campaign::TC_YELLOW:	arrowcolour=RGB(150,150,0);	break;
	case Campaign::TC_BLUE:	arrowcolour=RGB(0,0,255);	break;
	}
	b->SetForeColor(arrowcolour);
	if (setaccel)
		OnClickedFastforward(); 
	else
		UnPressBtns();

	b->Invalidate();													  //RDH 11/06/99

}

void TitleBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(TitleBar)
	DDX_Control(pDX,IDC_DATE,m_IDC_DATE);
	DDX_Control(pDX, IDC_DATETIME, m_IDC_DATETIME);
	DDX_Control(pDX, IDC_FASTFORWARD, m_IDC_FASTFORWARD);
	DDX_Control(pDX, IDC_CONTROL, m_IDC_CONTROL);
	DDX_Control(pDX, IDC_PAUSE, m_IDC_PAUSE);
	DDX_Control(pDX, IDC_PLAY, m_IDC_PLAY);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TitleBar, CRToolBar)
	//{{AFX_MSG_MAP(TitleBar)
	ON_WM_INITMENUPOPUP()
	ON_WM_INITMENU()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TitleBar message handlers



BEGIN_EVENTSINK_MAP(TitleBar, CRToolBar)
    //{{AFX_EVENTSINK_MAP(TitleBar)
	ON_EVENT(TitleBar, IDC_CONTROL, 1 /* Clicked */, OnClickedControl, VTS_NONE)
	ON_EVENT(TitleBar, IDC_DATETIME, 1 /* Clicked */, OnClickedDatetime, VTS_NONE)
	ON_EVENT(TitleBar, IDC_PAUSE, 1 /* Clicked */, OnClickedPause, VTS_NONE)
	ON_EVENT(TitleBar, IDC_FASTFORWARD, 1 /* Clicked */, OnClickedFastforward, VTS_NONE)
	ON_EVENT(TitleBar, IDC_PLAY, 1 /* Clicked */, OnClickedPlay, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void TitleBar::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CRToolBar::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	
	// TODO: Add your message handler code here
	
}

void TitleBar::OnInitMenu(CMenu* pMenu) 
{
	CRToolBar::OnInitMenu(pMenu);
	

}

void	TitleBar::OpenTimeControl(int tabnum)
{
	if (!LoggedChild(TIMECONTROL))
	{
		RDialog*	rv=ControlTopLevel::Make();
		LogChild(TIMECONTROL,rv);
		CRTabs* tab=(CRTabs*)rv->fchild->GetDlgItem(IDJ_TABCTRL);
		tab->SelectTab(tabnum);
	}
	else
	{

	}
}

void TitleBar::OnClickedControl() 
{
	if (RDialog::m_pView->m_currentpage != 0)							//RJS 29Oct00
 		return;															//RJS 29Oct00

	if (!LoggedChild(TIMECONTROL))
		LogChild(TIMECONTROL,ControlTopLevel::Make()
						);
	else
		CloseLoggedChild(TIMECONTROL);

}

void TitleBar::OnClickedDatetime() 
{
	if (RDialog::m_pView->m_currentpage != 0)							//RJS 29Oct00
 		return;															//RJS 29Oct00

	UnPressBtns();
	if (!LoggedChild(CLOCK))
		LogChild(CLOCK,Clock::Make()
						);
	else
		CloseLoggedChild(CLOCK);

	
}
 void TitleBar::OpenCombatReport(int index, bool date) 
{
 
	if (!LoggedChild(COMBATREPORT))
		LogChild(COMBATREPORT,RAFCombat::Make(index, true)
							);
	else
		CloseLoggedChild(COMBATREPORT);
}

void TitleBar::OpenLWTask() 
{

	if (!LoggedChild(TASK))
		LogChild(TASK,LWTaskTop::Make()
						);
	else
		CloseLoggedChild(TASK);

}
void TitleBar::OpenRAFTask() 
{

	if (!LoggedChild(TASK))
		LogChild(TASK,RAFTaskTop::Make()
						);
	else
		CloseLoggedChild(TASK);

}
void TitleBar::OpenTote(UniqueID uid) 
{
	if (LoggedChild(TOTE))
		CloseLoggedChild(TOTE);


	LogChild(TOTE,ToteSector::Make(uid)	);

}
void TitleBar::OpenAcUnit(int oldunit, int typestolist, int tab, int am, int unitdisplaylevel) 
{

	if (!LoggedChild(ACUNIT))
		LogChild(ACUNIT,AircraftUnit::Make(oldunit,(AircraftUnit::TypesToList)typestolist,tab,am,unitdisplaylevel)
						);
	else
		CloseLoggedChild(ACUNIT);

}
void TitleBar::OpenAcUnitRAF(int unit, bool spitfire,  int tab, int attackmethod) 
{

	if (!LoggedChild(ACUNIT))
		LogChild(ACUNIT,AcUnitRAF::Make(unit, spitfire, tab, attackmethod)
						);
	else
		CloseLoggedChild(ACUNIT);

}
void TitleBar::DialoguesVisible( bool vis)
{
		MakeChildrenVisible(vis);

}

void TitleBar::OpenSelTrg(int squad, int tab, int unitdisplaylevel, SelectTargetMode selecttargetmode) 
{


	if (!LoggedChild(SELTRG))
		LogChild(SELTRG,SelectTarget::Make(squad, tab, unitdisplaylevel, selecttargetmode)
						);
	else
		CloseLoggedChild(SELTRG);

}

void TitleBar::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CRToolBar::OnShowWindow(bShow, nStatus);
	

	if (bShow) 
		Redraw();
}

void TitleBar::OnClickedPause() 
{
	if (RDialog::m_pView->m_currentpage != 0)							//RJS 29Oct00
 		return;															//RJS 29Oct00

	MMC.curracceltype=MMC.ACCEL_PAUSED;
	MMC.curraccelrate = MMC.accelrates[MMC.ACCEL_PAUSED];
	UnPressBtns();

	GETDLGITEM(IDC_DATETIME)->SetString(CSprintf("%s: %s x%i",
		GetDateName(MMC.currdate, DATE_LONG),GetTimeName(MMC.currtime),MMC.curraccelrate	));

	GETDLGITEM(IDC_DATETIME)->Invalidate();
	
}

void TitleBar::OnClickedFastforward() 
{
	if (RDialog::m_pView->m_currentpage != 0)							//RJS 29Oct00
 		return;															//RJS 29Oct00

	if (MMC.curracceltype<=MMC.ACCEL_NORMAL)
		MMC.curracceltype=MMC.ACCEL_DIALOGSPEED;
	MMC.curraccelrate = MMC.accelrates[MMC.curracceltype];
	UnPressBtns();
	GETDLGITEM(IDC_DATETIME)->SetString(CSprintf("%s: %s x%i",
		GetDateName(MMC.currdate, DATE_LONG),GetTimeName(MMC.currtime),MMC.curraccelrate	));
	GETDLGITEM(IDC_DATETIME)->Invalidate();
	
}
void	TitleBar::ClickPause()
{
	  OnClickedPause();
}
void	TitleBar::ClickPlay()
{
	OnClickedPlay();
}

void TitleBar::OnClickedPlay() 
{
	if (RDialog::m_pView->m_currentpage != 0)							//RJS 29Oct00
 		return;															//RJS 29Oct00

	MMC.curracceltype=MMC.ACCEL_NORMAL;
	MMC.curraccelrate = MMC.accelrates[MMC.ACCEL_NORMAL];
	UnPressBtns();
	GETDLGITEM(IDC_DATETIME)->SetString(CSprintf("%s: %s x%i",
		GetDateName(MMC.currdate, DATE_LONG),GetTimeName(MMC.currtime),MMC.curraccelrate	));
	GETDLGITEM(IDC_DATETIME)->Invalidate();
	
}

void TitleBar::UnPressBtns()
{
	GETDLGITEM(IDC_FASTFORWARD)->SetPressed(MMC.curracceltype>MMC.ACCEL_NORMAL);
	GETDLGITEM(IDC_PAUSE)->SetPressed(MMC.curracceltype==MMC.ACCEL_PAUSED);
	GETDLGITEM(IDC_PLAY)->SetPressed(MMC.curracceltype==MMC.ACCEL_NORMAL);
	GETDLGITEM(IDC_DATETIME)->SetPressed(FALSE);
}
void TitleBar::UnPressControlBtn()
{
	GETDLGITEM(IDC_CONTROL)->SetPressed(FALSE);

}
void IntelBuffer::ClickPlay() 
{
	TitleBarPtr().ClickPlay();
}
