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

//Dummy???? // MapDlg.cpp : implementation file
//
// This file manages mouse input, and invalidation on the map	OnClickItem
// Also has movecode timer	OnTimer
//
#include "stdafx.h"
#include "MIG.h"
#include "MapDlg.h"
#include "messages.h"
#include "fileman.h"
#include "MIGView.h"
#include "node.h"
#include "mytime.h"
#include "comms.h"			  //H2HPlayerInfo
#include "miles.h"
#include "persons2.h"
#include "uiicons.h"
#include	"rchatter.h"										//RJS 28Mar00
#include	"mapsfx.h"											//RJS 08May00
#include "monotxt.h"
#include	"planetyp.h"
#ifdef _DEBUG
//#define new DEBUG_NEW
#ifndef	THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapDlg dialog

CMapDlg::CMapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_LButtonDown=FALSE;
	m_buttonid=0;
	m_hintid=0;
	TargetSelectSink=NULL;
	TargetSelectMode=STM_NORMAL;
}


void CMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapDlg, CDialog)
	//{{AFX_MSG_MAP(CMapDlg)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapDlg message handlers



void CMapDlg::OnClickItem(int m_buttonid)
{
#ifndef	BOB_DEMO_VER

//	RDialog::MakeTopDialog(CRect(100,100,0,0),
//		DialList(DialBox(FIL_NULL,
	typedef	HideEdgeControl::EdgeCtrl	Edge;
	typedef	HideEdgeControl	EDGE;
	_Miles.UIPlaySample(FIL_SFX_OFFICE_PAPER3);					//RDH 12Apr99

	if (TargetSelectSink)
	{
		switch (TargetSelectMode)
		{
		case	STM_LWAC:
			if (m_buttonid>=SagBAND&& m_buttonid<SagBANDEND)
			{
				info_airgrpPtr p=Persons2::ConvertPtrUID(UniqueID(m_buttonid));
				if (info_airgrpPtr(*p)->package->attackmethod>Profile::AM_LWPACKS)
					TargetSelectSink->RefreshData(m_buttonid);
			}
		break;
		case	STM_RAFAC:
			if (m_buttonid>=SagBAND&& m_buttonid<SagBANDEND)
			{
				info_airgrpPtr p=Persons2::ConvertPtrUID(UniqueID(m_buttonid));
				if (info_airgrpPtr(*p)->package->attackmethod<Profile::AM_LWPACKS)
					TargetSelectSink->RefreshData(m_buttonid);
			}
		break;
		case	STM_GROUND:
			if (m_buttonid>SagBANDEND
// dont want to be able to select LW airfields as targets ever
			&& (m_buttonid<LUF_FighterLF2BAND || m_buttonid>=LUF_FighterLF3BAND))	//AMM 15Nov00
				TargetSelectSink->RefreshData(m_buttonid);
		break;
		}
	}
	else
	if (m_buttonid>=WayPointBAND && m_buttonid<WayPointBANDEND)
	{

		int pack,squad,target;
		if (Profile::GetPackageFromWP((UniqueID)m_buttonid, pack,&squad,&target))
		{
			if (squad<0)
				squad=0;
			RDialog::m_pView->SetHiLightInfo(pack, squad,(UniqueID)m_buttonid);
			MainToolBar().OpenWP();
			RDialog*	routedial = MainToolBar().LoggedChild(CMainToolbar::ROUTE);
			if (routedial)
			{
				CRTabs* tab=(CRTabs*)routedial->fchild->fchild->GetDlgItem(IDJ_TABCTRL);
				tab->SelectTab(1);
			}
			Invalidate();

		}


	}
	else 
	if (m_buttonid>=SagBAND && m_buttonid<SagBANDEND)
	{
		//££need to get pack and set as hilight
		int packnum,sqline;
		if (Todays_Packages.GetACSquadEntry((UniqueID)m_buttonid,packnum,&sqline))
		{
			RDialog::m_pView->SetHiLightInfo(packnum, sqline,UID_Null);
			Invalidate();
			if (Todays_Packages.localplayer==NAT_RAF)
			{
				if (Todays_Packages[packnum].attackmethod>=Profile::AM_LWPACKS)
				{//lw
					MainToolBar().OnClickedHostileslist();
				}else
				{
					MainToolBar().OnClickedMissionfolder();
				}

			}else
			{
				if (Todays_Packages[packnum].attackmethod>=Profile::AM_LWPACKS)
				{//lw
					MainToolBar().OnClickedMissionfolder();
				}
			}

		}
	}else
	{
		Invalidate();
		UniqueIDBand band=Persons2::GetBandFromUID(UniqueID(m_buttonid));
		switch (band)
		{
		default:
			MainToolBar().OpenDossier((UniqueID)m_buttonid);
		break;
		case	CivilianBAND:
		case	CityBAND:
		case	TownBAND:
		case	VillageBAND:
		case	NavigationBAND:
			if (Todays_Packages.localplayer==NAT_RAF)
				MainToolBar().OpenDossier((UniqueID)m_buttonid);
		break;

		case		BarrageBalloonBAND:
		case		BritAAASiteBAND:
		case		BritAAALargeBAND:

		break;
		}
//		CTargetDossier::MakeSheet(CRect(180,180,0,0),(UniqueID)m_buttonid);
	}
#endif #ifndef	BOB_DEMO_VER

}
void CMapDlg::OnFailClickItem(int m_buttonid)
{
 	if (m_buttonid>=WayPointBAND && m_buttonid<WayPointBANDEND)
	{
		int pack,squad,target;
		if (Profile::GetPackageFromWP((UniqueID)m_buttonid, pack,&squad,&target))
		{
			if (squad<0)
				squad=0;
			RDialog::m_pView->SetHiLightInfo(pack, squad,(UniqueID)m_buttonid);
		}
		Invalidate();
	}
	else
	if (m_buttonid>=SagBAND && m_buttonid<SagBANDEND)
	{
		//££need to get pack and set as hilight
		int packnum,sqline;
		if (Todays_Packages.GetACSquadEntry((UniqueID)m_buttonid,packnum,&sqline))
		{
			RDialog::m_pView->SetHiLightInfo(packnum, sqline,UID_Null);
			Invalidate();
		}
	}
	else
		Invalidate();

}

void CMapDlg::OnDragItem(int m_buttonid)
{
#ifndef	BOB_DEMO_VER

//	RDialog::MakeTopDialog(CRect(100,100,0,0),
//		DialList(DialBox(FIL_NULL,
	ItemBasePtr p=Persons2::ConvertPtrUID(UniqueID(m_buttonid));
	if (!p)
		return;
	switch (p->Status.size)
	{
	case WAYPOINTSIZE:
		{
			int pack, wave, g;
			if (Profile::GetPackageFromWP((UniqueID)m_buttonid, pack))
			{
				RDialog::m_pView->SetHiLightInfo(pack, 0,(UniqueID)m_buttonid);

				info_waypoint* w=*p;
				if (w->World.X<METRES50KM)								w->World.X=METRES50KM;
				if (w->World.X>METRES500KM+METRES100KM+METRES50KM)		w->World.X=METRES500KM+METRES100KM+METRES50KM;
				if (w->World.Z<METRES50KM)								w->World.Z=METRES50KM;
				if (w->World.Z>METRES500KM+METRES100KM+METRES50KM)		w->World.Z=METRES500KM+METRES100KM+METRES50KM;

				Coords3D c=w->World;
				Todays_Packages[pack].SetTargetGlobrefs();			  //JIM 16/03/99
				Todays_Packages[pack].RecalcPrimaryPositions();		  //JIM 16/03/99
				if (w->target.Evaluate()!=ENABLE_COMPLEX_VAL)
					c-=Persons2::ConvertPtrUID(UniqueID(w->target.Evaluate()));
				Todays_Packages[pack].SetTargetGlobrefs();
				Todays_Packages[pack].RecalcPrimaryPositions();
				w->position.RecalcDeltasToMatch(c);
					Todays_Packages[pack].RecalcRoutePositionsAndTime();
				int fuel;
				Invalidate();										  //JIM 16/03/99

				bool cando=Todays_Packages[pack].CalcWPTimes();
				while (!cando)
				{
					Todays_Packages[pack].primarytargetETA+=SECSPERMIN;
					cando=Todays_Packages[pack].CalcWPTimes();
				}
				SquadNum sq;
				if ((sq=Todays_Packages[pack].CalcFuel(UID_NULL, fuel, fuel))!=SQ_ZERO)
				{
					Todays_Packages.MessageBoxFuelWarning(sq);
				}
				if (w->applygroup==ENABLE_COMPLEX_VAL)
				{

				}
				Campaign::GlobalRefreshData();
			}
		}
	break;
	}
#endif #ifndef	BOB_DEMO_VER

//	typedef	HideEdgeControl::EdgeCtrl	Edge;
//	typedef	HideEdgeControl	EDGE;
//	CTargetDossier::MakeSheet(CRect(180,180,0,0),(UniqueID)m_buttonid);
}

void	PackageList::MessageBoxFuelWarning(SquadNum sq)
{	//IDS_PHRASE_TITLE_ACFACTORYOUTBAD	//IDS_HIDC_RBUTTONDELETE
	Squadron* s=Node_Data[sq];
	CString message=s->FullName()+" ("+GetTargName(s->homeairfield)+")\n"+LoadResString(IDS_BINGOWARNING);
	if (!RDialog::messageboxopen)
		RDialog::RMessageBox(LoadResString(IDS_BINGOWARNINGTITLE),message ,NULL,IDS_OK);
}
void	PackageList::MessageBoxFuelWarning(int package)
{	//IDS_PHRASE_TITLE_ACFACTORYOUTBAD	//IDS_HIDC_RBUTTONDELETE
	CString message=GetTargName(Todays_Packages[package][0].instance,true)+" ("+GetTargName(Todays_Packages[package].packagetarget[0])+")\n"+LoadResString(IDS_BINGOWARNING);
	if (!RDialog::messageboxopen)
		RDialog::RMessageBox(LoadResString(IDS_PHRASE_TITLE_ACFACTORYOUTBAD),message+"\n"+LoadResString(IDS_HIDC_RBUTTONDELETE) ,NULL,IDS_OK);
}

bool CMapDlg::AllowDragItem(int m_buttonid)
{
//	RDialog::MakeTopDialog(CRect(100,100,0,0),
//		DialList(DialBox(FIL_NULL,
	if (TargetSelectSink)
		return false;

	if (m_buttonid<WayPointBAND)
		return	false;
	if (m_buttonid>=WayPointBANDEND)
		return	false;
	int	pack;
	if (!Profile::GetPackageFromWP((UniqueID)m_buttonid, pack))
		return false;
	if (Todays_Packages[pack].packagestatus>=Profile::PS_ACTIVE_MIN && Todays_Packages[pack].attackmethod>=Profile::AM_LWPACKS)
		return false;
	info_waypointPtr wp=Persons2::ConvertPtrUID(UniqueID(m_buttonid));
	switch (wp->uid.Evaluate())
	{
	case	-SGR_WPP_TakeOff:
	case	-SGR_WPP_EstPoint:
	case	-SGR_WPP_Target:
	case	-SGR_WPP_Landing:
	case	-SGR_WPC_TakeOff:
	case	-SGR_WPC_Target:
	case	-SGR_WPC_Landing:
	case	-SGR_WPC_FighterLimit:
		return false;
	default:;
	}
	switch (wp->wpact.Evaluate())
	{
	case	wpatakeoffno:
	case	wpalandingno:
	case	wpbombno:
	case	wpreccyno:
	case	WP_fighterlimit:
		return false;
	default:;
	}
	RDialog::m_pView->SetHiLightInfo(pack, 0,(UniqueID)m_buttonid);
	return true;
}



BOOL CMapDlg::OnEraseBkgnd(CDC* pDC) 
{
//	RECT	bounds;
//	int rv;
//	rv=GetUpdateRect(&bounds,0);
//	::AfxTrace("Refresh area (%i,%i),(%i,%i)\n",bounds.left,bounds.top,bounds.right,bounds.bottom);

//	CRect rect;
//	pDC->GetBoundsRect(&rect,NULL);
//	if (rect.Width()<30 && rect.Height()<30) return TRUE;
//	RDialog::m_pView->UpdateBitmaps(pDC);
	return TRUE;
}
int	inside_move=0;
void CMapDlg::OnPaint() 
{
	PAINTSTRUCT paintstruct;
	inside_move=10;
	CDC* pDC;
	pDC=this->BeginPaint(&paintstruct);
	RECT& bounds=paintstruct.rcPaint;
	static const bool mapdlgDbg = getenv("ROWAN_DEBUG_BMP") != nullptr;
	if (mapdlgDbg)
		fprintf(stderr,"[mapdlg] OnPaint bounds=%ld,%ld-%ld,%ld view=%p\n",
			(long)bounds.left,(long)bounds.top,(long)bounds.right,(long)bounds.bottom,(void*)RDialog::m_pView);
	inside_move=11;

	RDialog::m_pView->UpdateBitmaps(pDC,bounds);

	inside_move=12;
	this->EndPaint(&paintstruct);
	inside_move=13;
//TempCode JIM 9Nov00 	InvalidateAnotherItem();
	inside_move=0;

}


void	CMapDlg::InvalidateIcon(ItemBasePtr p)
{
	int	px,py;
	RDialog::m_pView->ScreenXY(p,px,py,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
	IconsUI i =IconsUI( RDialog::m_pView->DrawIconTest( p, p->uniqueID.count, false,true));
	if (i != ICON_NO_ICON)
	{
		InvalidateIcon(px,py,i);
		InvalidateIconExtra(px,py,p,i);
	}
}



//////////////////////////////////////////////////////////////////////
//
// Function:    FindMapItem
// Date:		15/12/99
// Author:		JIM
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
static int standardiconbands[][2]=
{
	{UID_NULL,SagBAND},
	{RealAcBAND,IllegalBAND},
	{0,0}
};

int	CMapDlg::FindMapItem(CPoint point)
{
	int	m_hintid=0;
	int	mwx,mwy;													  //JIM 16/02/99
//	int mwx=		65536*384 +((point.x+RDialog::m_pView->m_scrollpoint.x)*65536)/RDialog::m_pView->m_zoom; //JIM 16/02/99
//	int mwy=		65536*1792-((point.y+RDialog::m_pView->m_scrollpoint.y)*65536)/RDialog::m_pView->m_zoom; //JIM 16/02/99
	int worldicon=RDialog::m_pView->WorldXY(mwx,mwy,point.x,point.y);//m_iconradius*65536/RDialog::m_pView->m_zoom; //JIM 16/02/99
	int mwl=mwx-worldicon;
	int	mwr=mwx+worldicon;
	int	mwt=mwy+worldicon;
	int	mwb=mwy-worldicon;
	FileNum  FL_Icon[100];//AmberBattleNodeBANDEND-AmberBattleNodeBAND];

	ItemBasePtr p;
	FileNum  f;

	//Search for standard icon
	for	(int standardbandloop=0;standardiconbands[standardbandloop][1];standardbandloop++)
		for (int i=standardiconbands[standardbandloop][0],e=standardiconbands[standardbandloop][1];i<e;i++)
		{
			p=Persons2::ConvertPtrUID(UniqueID(i));
			if (p)
			{
				f = RDialog::m_pView->DrawIconTest(p, UniqueID(i), false,false);//,redUIDs,FL_Icon);
				if (f)
					if (	p->World.X>mwl && p->World.X<mwr &&	p->World.Z>mwb && p->World.Z<mwt )
						if (InsideIcon(IconsUI(f),p,point))
							m_hintid=i;
			}
		}
	mwl=mwx-worldicon*2;
	mwr=mwx+worldicon*2;
	mwt=mwy+worldicon*2;
	mwb=mwy-worldicon*2;
	//specific 'bigger' icons
	for (int i=SagBAND,e=RealAcBAND;i<e;i++)
	{
		info_airgrpPtr p=Persons2::ConvertPtrUID(UniqueID(i));
		if (p)
		{
			f = RDialog::m_pView->DrawIconTest(p, UniqueID(i), false,false);//,redUIDs,FL_Icon);
			if (f)
				if (	p->knownposition.X>mwl && p->knownposition.X<mwr &&	p->knownposition.Z>mwb && p->knownposition.Z<mwt )
					if (InsideIcon(IconsUI(f),p,point))
						m_hintid=i;
		}
	}

	return m_hintid;
}
bool	CMapDlg::OverMapItem(UniqueID i,CPoint point)
{
			ItemBasePtr p=Persons2::ConvertPtrUID(i);
			if (p)
			{
				FileNum f = RDialog::m_pView->DrawIconTest(p, UniqueID(i), false,false);//,redUIDs,FL_Icon);
				if (f)
					if (InsideIcon(IconsUI(f),p,point))
						return true;
			}
	  return false;
}

void CMapDlg::ResetIcon(UniqueID wpuid)
{
	int	oldbuttonid = m_buttonid;
	m_buttonid = wpuid;
	RedrawIcon(oldbuttonid);
}
void CMapDlg::RedrawIcon(int oldbuttonid)
{// doing 2 invalidaterects here removes the very slight flicker...

	ItemBasePtr p;
	int px,py;
	if (m_buttonid)
	{
		p=Persons2::ConvertPtrUID(UniqueID(m_buttonid));
		if (p)
		{
			RDialog::m_pView->ScreenXY(p,px,py,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
			IconsUI i =IconsUI( RDialog::m_pView->DrawIconTest( p, UniqueID(m_buttonid), false,true));
			if (i)
				InvalidateIcon(px,py,i);

		}
		else m_buttonid=UID_NULL;									  //DAW 07/05/99

	}
	if (oldbuttonid)
	{
		p=Persons2::ConvertPtrUID(UniqueID(oldbuttonid));
		if (p)
		{
			RDialog::m_pView->ScreenXY(p,px,py,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
			IconsUI i =IconsUI( RDialog::m_pView->DrawIconTest( p, UniqueID(oldbuttonid), false,true));
			if (i)
				InvalidateIcon(px,py,i);
		}
		else
			oldbuttonid=UID_NULL;									  //DAW 07/05/99
	}
}
void CMapDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_LButtonDown=TRUE;
	m_bDragging=FALSE;
	m_point=point;
	
	int oldbuttonid=m_buttonid;
	m_buttonid=NULL;
	m_buttonid=FindMapItem(point);
	RedrawIcon(oldbuttonid);
	CDialog::OnLButtonDown(nFlags, point);
}


void CMapDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	RDialog::m_pView->m_popupPoint=point;
	int px,py;
	ItemBasePtr p;
	CDC* pDC=GetDC();
	if (m_LButtonDown && !m_bDragging)
		if (m_buttonid)
		{
			if (!CRect(m_point.x-3,m_point.y-3,m_point.x+3,m_point.y+3).PtInRect(point))
				if (AllowDragItem(m_buttonid))
			{
				::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
				m_bDragging=TRUE;
				SetCapture();
				p=Persons2::ConvertPtrUID(UniqueID(m_buttonid));
				RDialog::m_pView->ScreenXY(p,px,py,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
				m_point.x=px;
				m_point.y=py;
				InvalidateIconText(pDC,px,py,UniqueID(m_hintid));
				m_hintid=NULL; // dont display hints while dragging...
			}
		}
		else
		{
 			if (!CRect(m_point.x-3,m_point.y-3,m_point.x+3,m_point.y+3).PtInRect(point))
				::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
		}
	if (m_LButtonDown && m_bDragging && m_buttonid)
	{
		ItemBasePtr p=Persons2::ConvertPtrUID(UniqueID(m_buttonid));
		if (p)
		{
			InvalidateRoutes(UniqueID(m_buttonid));
 			InvalidateDraggingRoutes(UniqueID(m_buttonid));

			RDialog::m_pView->WorldXY(p->World,point.x,point.y); //JIM 16/02/99

			InvalidateDragIcon(m_point.x,m_point.y,point.x,point.y, ICON_MAPFILTERS);
			m_oldpoint=m_point;

			m_point=point;
			InvalidateRoutes(UniqueID(m_buttonid));
			InvalidateDraggingRoutes(UniqueID(m_buttonid));
		}
		else
		{
			m_LButtonDown=FALSE;
			m_buttonid=UID_NULL;
		}
	}
	if (!m_LButtonDown)
	{
		// check if mouse pointer is over an icon and if so display
		// the hint text for it
		// first find if we are over one
		int oldhintid=m_hintid;
		m_hintid=NULL;
		m_hintid=FindMapItem(point);
		// doing 2 invalidaterects here removes the very slight flicker...
		if (oldhintid!=m_hintid)
		{
			if (m_hintid)
			{
				p=Persons2::ConvertPtrUID(UniqueID(m_hintid));
				RDialog::m_pView->ScreenXY(p,px,py,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
				InvalidateIconText(pDC,px,py,UniqueID(m_hintid));
			}
			if (oldhintid)
			{
				p=Persons2::ConvertPtrUID(UniqueID(oldhintid));
				if (p)
				{
					RDialog::m_pView->ScreenXY(p,px,py,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
					InvalidateIconText(pDC,px,py,UniqueID(oldhintid));
				}
			}
		}
	}
	CDialog::OnMouseMove(nFlags, point);
	ReleaseDC(pDC);
}

void CMapDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	if (m_LButtonDown)
	{
		m_LButtonDown=FALSE;
		if (m_buttonid)
		{
			if (!m_bDragging)
				if (OverMapItem(UniqueID(m_buttonid),point))
					OnClickItem(m_buttonid);
				else
					OnFailClickItem(m_buttonid);
			else
				OnDragItem(m_buttonid);
		}
		else
		{
			m_point-=point;
			CPoint	oldscrollpoint=RDialog::m_pView->m_scrollpoint;
			RDialog::m_pView->m_scrollpoint.x+=m_point.x;
			RDialog::m_pView->m_scrollpoint.y+=m_point.y;
			RDialog::m_pView->Zoom(0,0);						  //RDH 20/12/99
			m_point=point;
			RDialog::m_pView->m_mapdlg.ScrollWindow(oldscrollpoint.x-RDialog::m_pView->m_scrollpoint.x,oldscrollpoint.y-RDialog::m_pView->m_scrollpoint.y);

		}
	}
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	CDialog::OnLButtonUp(nFlags, point);
}
BOOL CMapDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL));
	
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

//////////////////////////////////////////////////////////////////////
//
// Function:    InvalidateRoutes
// Date:		18/02/00
// Author:		DAW
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
void	PackageList::InvalidateRoute(Profile* p,bool andicons,bool andsquads)
{
	RDialog::m_pView->InvalidateRoute(p-pack,andicons,andsquads);
}
void	PackageList::InvalidateIcon(UniqueID acuid)
{
	ItemBasePtr w=Persons2::ConvertPtrUID(acuid);
	RDialog::m_pView->m_mapdlg.InvalidateIcon(w);
	if (acuid==RDialog::m_pView->m_mapdlg.m_hintid)
	{
		int	px,py;
		CDC* pDC=RDialog::m_pView->GetDC();
		RDialog::m_pView->ScreenXY(w,px,py,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
		RDialog::m_pView->m_mapdlg.InvalidateIconText(pDC,px,py,acuid);
		RDialog::m_pView->ReleaseDC(pDC);
	}
}

void	CMapDlg::InvalidateDraggingRoutes(UniqueID through)
{
	info_basePtr p=Persons2::ConvertPtrUID(through);
	if (p && p->Status.size==WAYPOINTSIZE)
	{
		
		info_waypointPtr w=*p;
		WorldExpr* we=(WorldExpr*)(w->position.complex);
		UniqueID i1=UID_NULL,i2=UID_NULL;
		if (w->target.Evaluate()!=ENABLE_COMPLEX_VAL)
			i1=UniqueID(w->target.Evaluate());
		else
		{
			if (we->uniqueID.Evaluate()!=ENABLE_COMPLEX_VAL)
				i1=UniqueID(we->uniqueID.Evaluate());
		}
		if (we->rotate.complex)
		{
			RotExpr* re=(RotExpr*)we->rotate.complex;
			if (re->icpt.complex)
			{
				i2=UniqueID(re->icpt[1].Evaluate());
			}
		}
		if (i1)
			InvalidateLine(i1,through);
		if (i2)
			InvalidateLine(i2,through);
	}
}

void	CMIGView::DrawDraggingRoutes(CDC* pDC,UniqueID through)
{
	info_basePtr p=Persons2::ConvertPtrUID(through);
	if (p && p->Status.size==WAYPOINTSIZE)
	{
		
		info_waypointPtr w=*p;
		WorldExpr* we=(WorldExpr*)(w->position.complex);
		UniqueID i1=UID_NULL,i2=UID_NULL;
		if (w->target.Evaluate()!=ENABLE_COMPLEX_VAL)
			i1=UniqueID(w->target.Evaluate());
		else
		{
			if (we->uniqueID.Evaluate()!=ENABLE_COMPLEX_VAL)
				i1=UniqueID(we->uniqueID.Evaluate());
		}
		if (we->rotate.complex)
		{
			RotExpr* re=(RotExpr*)we->rotate.complex;
			if (re->icpt.complex)
			{
				i2=UniqueID(re->icpt[1].Evaluate());
			}
		}
		if (i1 || i2)
		{
			CPen pen (PS_SOLID,2,RGB(255,25,25));
			CPen* oldpen=NULL;
			if (pDC)
				oldpen=pDC->SelectObject(&pen);
			if (i1)
				PlotLine(pDC,i1,through);
			if (i2)
				PlotLine(pDC,i2,through);
			if (pDC && oldpen)
				pDC->SelectObject(oldpen);

		}
	}

}

void CMapDlg::InvalidateRoutes(UniqueID onlythrough)
{
	RDialog::m_pView->PlotRoutes(NULL,onlythrough);
/*	bool	seesubwp = true;
	ItemBasePtr pt1,pt2;

	// invalidates any routes coming in or out of m_buttonid
	// doesnt work
	for (int pack=0;pack<Profile::MAX_PACKS;pack++)
	{
		for (int wave=0;wave<Profile::MAX_WAVES;wave++)
		{
			for (int group=0;group<Profile::MAX_GROUPS;group++)
			{
				UniqueID start=Todays_Packages[pack][wave][group].takeoffAF;
				if (start)
				{
					UniqueID nextwp=Todays_Packages[pack][wave].firstwp;
					InvalidateMovedLine(start,nextwp);
					info_waypoint* wp=*Persons2::ConvertPtrUID(nextwp);
					UniqueID oldwp = nextwp;
					while ((nextwp=wp->nextwp)!=NULL)
					{
						wp=*Persons2::ConvertPtrUID(nextwp);
						if  (		
								(wp->uid !=ENABLE_COMPLEX_VAL) ||
								(wp->applygroup==ENABLE_COMPLEX_VAL) ||
								(		
									(seesubwp) &&
									(wp->applygroup == Todays_Packages.higroup)
								)
							)
						{
							//display subgroup wps as spurs
							InvalidateMovedLine(oldwp,nextwp);
							if  (wp->applyflight==ENABLE_COMPLEX_VAL)
							{
								oldwp=nextwp;
							}
						}
					}
					InvalidateMovedLine(nextwp,start);
				}
			}
		}
	}
	*/
}

void CMapDlg::InvalidateLine(UniqueID thiswp,UniqueID nextwp)
{
	ItemBasePtr pt1,pt2;
	pt1=Persons2::ConvertPtrUID(UniqueID(thiswp));
	pt2=Persons2::ConvertPtrUID(UniqueID(nextwp));
	if (!pt1 || !pt2) return;
	if (thiswp<WayPointBANDEND && nextwp<WayPointBANDEND)
	{

		info_base::VisibilityCondition tvc=info_waypointPtr(pt1)->GetVC(info_base::VCS_AFTER);
		info_base::VisibilityCondition nvc=info_waypointPtr(pt2)->GetVC(info_base::VCS_BEFORE);
		//copy draw to lastdraw
		info_waypointPtr(pt1)->UpdateDrawLast(info_base::VCS_AFTER);
		info_waypointPtr(pt2)->UpdateDrawLast(info_base::VCS_BEFORE);

		//if both ends are known and neither end HAD draw or hilight then return
		if (tvc&nvc& info_base::VC_KNOWN)
			if (((tvc|nvc)&info_base::VC_REFRESHMASK)==info_base::VC_NODRAW)
				return;
	}

	int px1,py1,px2,py2;
		RDialog::m_pView->ScreenXY(pt1,px1,py1,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
		RDialog::m_pView->ScreenXY(pt2,px2,py2,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
	InvalidateLine(CPoint(px1,py1),CPoint(px2,py2));
	int f = RDialog::m_pView->DrawIconTest(pt1, thiswp, false,true);//,redUIDs,FL_Icon);
	if (f)
		InvalidateIcon(px1,py1,IconsUI(f));										  //JIM 02/06/99
}


#ifndef	NDEBUG

DebugLineStats	debugboxmem
={{0x00,0x20,0x40,0x60,0x80,0xA0,0xC0,0xE0},0,0};
#endif

bool	ResetKnownPosition(info_airgrpPtr p,UniqueID i)
{

	int squadline;
	if (p->package->GetACSquadEntry(i,squadline))
	{	//otherwise it has been deleted!
		if (p->package->squadlist[squadline].squaddetected)
		{
			p->knownposition=p->World;
			if (p->attitude.complex)
				p->knownhdg=Angles(p->attitude[1].Evaluate());
			else
				p->knownhdg=ANGLES_180Deg;
			p->knowntime=p->lasttime=MMC.currtime;
			return true;
		}
		else
		{
			int deltatime=MMC.currtime-p->lasttime;
			p->lasttime=MMC.currtime;
			int	ptype=p->type.Evaluate();
			ptype=p->package->squadlist[squadline].Squadron().AcType();
			PlaneInit* pi=Plane_Type_Translate[ptype];
			WheeledType* wi=(WheeledType*)pi;
			int distance=float(deltatime)*wi->cruisevel/VEL_1CMS;
			Float sin,cos;
			ANGLES hdg=p->knownhdg;
			if (p->package->squadlist[squadline].status>=Profile::PS_OUTGOING)
				if (p->package->attackmethod>Profile::AM_LWPACKS)
				{
					if (hdg<<ANGLES_30Deg || hdg>>ANGLES_240Deg)
						hdg=ANGLES_135Deg;
				}
				else
					if (hdg>>ANGLES_60Deg && hdg<<ANGLES_210Deg)
						hdg=ANGLES_315Deg;

			Math_Lib.high_sin_cos(hdg,sin,cos);
			p->knownposition.X+=sin*distance;
			p->knownposition.Z+=cos*distance;
			return false;
		}
	}
	return false;
}

int doing_item=0;
void CMapDlg::InvalidateMobiles(bool isnewposition,bool clocking)
{	//Invalidates all icons for mobiles in SAG and ship bands
	Todays_Packages.SetVisibilityFlags();
	for (int u=GermMobileBAND;u<PowerStationBAND;u++)
	{	doing_item=u;
		info_itemSPtr p=Persons2::ConvertPtrUID(UniqueID(u));
		if (p)
		{
			InvalidateIcon(p);
			doing_item+=0x40000;
			if (isnewposition)
				RefreshTargettingWaypoints(UniqueID(u));
			if (u==m_hintid)
			{
				int	px,py;
				CDC* pDC=GetDC();
				RDialog::m_pView->ScreenXY(p,px,py,-RDialog::m_pView->m_scrollpoint.x,-RDialog::m_pView->m_scrollpoint.y);
				doing_item+=0x80000;
				InvalidateIconText(pDC,px,py,UniqueID(m_hintid));
				doing_item=0;
				ReleaseDC(pDC);
			}
		}
	}
	info_airgrpPtr p;
	if (isnewposition==false)
		for (int i=SagBAND,j=0;i<SagBANDEND;i++,j++)
		{	doing_item=i;
			p=Persons2::ConvertPtrUID(UniqueID(i));
		 	IconsUI icon =IconsUI( RDialog::m_pView->DrawIconTest( NULL, UniqueID(i), false,true));
			if (p)
			{
				p->knownicon=icon;
			}

		}
	else
	{
		for (int i=SagBAND,j=0;i<SagBANDEND;i++,j++)
		{
			p=Persons2::ConvertPtrUID(UniqueID(i));
			if (p)
			{
		 		IconsUI icon =IconsUI( RDialog::m_pView->DrawIconTest( NULL, UniqueID(i), false,true));
				if (p && icon)
					p->knownicon=icon;
				else
					if (!p->knownicon)
					{
						ResetKnownPosition(p,UniqueID(i));
					}
			}
		}
		InvalidateAnotherItem();
	}

}
//enum	{BIGGEST_ICON_RADIUS=50};
bool	CMapDlg::InvalidateAnotherItem()
{
	static int LastItemUID=SagBAND;
	for (int i=LastItemUID,jj=SagBAND;jj<SagBANDEND;i=((i<SagBANDEND-1)?(i+1):(SagBAND)),jj++)
	{
		info_airgrpPtr	p=Persons2::ConvertPtrUID(UniqueID(i));
		if (p)
		{
			int squad=p->type.Evaluate();
			do{
				if (p && p->knownicon)
				{
					int px,py;
					RDialog::m_pView->ScreenXY(p,px,py);
					InvalidateIcon(px,py,p->knownicon);	//Y==3 means refresh in-place
					InvalidateIconExtra(px,py,p,p->knownicon);
					if (i==m_hintid)
					{
						CDC* pDC=GetDC();
						InvalidateIconText(pDC,px,py,UniqueID(m_hintid));
						ReleaseDC(pDC);
					}
					ResetKnownPosition(p,UniqueID(i));
					RDialog::m_pView->ScreenXY(p,px,py);
					InvalidateIcon(px,py,p->knownicon);	//Y==3 means refresh in-place
					InvalidateIconExtra(px,py,p,p->knownicon);
					if (i==m_hintid)
					{
						CDC* pDC=GetDC();
						InvalidateIconText(pDC,px,py,UniqueID(m_hintid));
						ReleaseDC(pDC);
					}
					RefreshTargettingWaypoints(UniqueID(i));
					p->knownicon=ICON_NO_ICON;
				}
				i++;
				p=Persons2::ConvertPtrUID(UniqueID(i));
			}while (p &&  squad==p->type.Evaluate());
			LastItemUID=i;
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
//
// Function:    RefreshTargettingWaypoints
// Date:		14/03/00
// Author:		JIM
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
int refresh_wp=0,refresh_wp_uid=0,substep=0;
void	CMapDlg::RefreshTargettingWaypoints(UniqueID trg)
{
	if (refresh_wp)	INT3;
	refresh_wp=1;
	for (int p=0;p<Profile::MAX_PACKS;p++)
		if (Todays_Packages[p].packagestatus<Profile::PS_ACTIVE_MAX)
			for (int t=0;t<Profile::MAX_TARGETS;t++)
				if (Todays_Packages[p].packagetarget[t]==trg)
				{	//need to refresh target area waypoints
					//First refresh old positions:
					refresh_wp=p*0x100+t+0x10000;
					UniqueID wpt0,wpt1,wpdummy,w;
					Todays_Packages[p].GetKeyMainWPs(wpdummy,wpdummy,wpt0,wpt1,wpdummy,wpdummy);
					
					w=wpt0;
					info_waypointPtr wp=Persons2::ConvertPtrUID(w);
					wpdummy=w;
					w=wp->NextWP(-1,t);
					Persons2::SetLoc(SGR_Pack_Target+t,Todays_Packages[p].packagetarget[t]);
					UniqueID redraw0=UID_NULL,redraw1=UID_NULL,redraw2=UID_NULL;
					substep=0x10000;
 					while (w!=wpt1)
					{
						if (substep++>0x10080) INT3;
						wp=Persons2::ConvertPtrUID(w);
						if ( wp->target==trg || wp->target.UsesGlobRef()==(SGR_Pack_Target+t) )
						{
							redraw0=wpdummy;
							redraw1=w;
							redraw2=wp->NextWP(-1,t);
							break;
							
						}
						else
						{
							wpdummy=w;
							w=wp->NextWP(-1,t);
						}

					}
					if (redraw1)
					{	substep=0x2000;
						refresh_wp_uid=redraw1;
						InvalidateLine(redraw0,redraw1);
						substep++;
						InvalidateLine(redraw2,redraw1);
						substep++;
						InvalidateIcon(wp);
						substep++;
						wp->position.EvalW(wp->World);
						substep++;
						wp->World+=Persons2::ConvertPtrUID(trg)->World;
						substep++;
						InvalidateLine(redraw0,redraw1);
						substep++;
						InvalidateLine(redraw2,redraw1);
						substep++;
						InvalidateIcon(wp);
					}
					substep=0x3000;					
					if (t==0)
					{
						w=wpt0;
						redraw1=UID_NULL;
						while (w!=wpt1)
						{	if (substep++>0x3080) INT3;
							wp=Persons2::ConvertPtrUID(w);
							if ( wp->uid.Evaluate()==-SGR_WPP_Target)
							{
								redraw0=wpdummy;
								redraw1=w;
								redraw2=wp->NextWP(-1,-2);
								break;
							}
							else
							{
								wpdummy=w;
								w=wp->NextWP(-1,-2);
							}
						}
						if (redraw1)
						{	substep=0x4000;
							refresh_wp_uid=redraw1+0x10000;
	 						InvalidateLine(redraw0,redraw1);
						substep++;
							InvalidateLine(redraw2,redraw1);
						substep++;
							InvalidateIcon(wp);
						substep++;
							wp->position.EvalW(wp->World);
						substep++;
							InvalidateLine(redraw0,redraw1);
						substep++;
							InvalidateLine(redraw2,redraw1);
						substep++;
							InvalidateIcon(wp);
						substep++;
						}
					}

				}
	refresh_wp=0;
}

//////////////////////////////////////////////////////////////////////
//
// Function:    InvalidateLine
// Date:		14/03/00
// Author:		DAW
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
void CMapDlg::InvalidateLine(CPoint point1,CPoint point2)
{	// invalidates a line from point1 to point2
	// ideally youd have thought you could use invalidate region
	// however this is very very slow...

	// so instead we invalidate (millions) (perhaps 20) of little rectangles...

//	CRect rect(point1.x-1,point1.y-1,point2.x+1,point2.y+1);
//	rect.NormalizeRect();
//	InvalidateRect(rect);
	const int RECTSIZE=50;
	CRect clientrect;
	CRect rect;
	CRect intersectrect;
	GetClientRect(clientrect);
	double dx=point2.x-point1.x;
	double dy=point2.y-point1.y;
	int steps;
	if (abs(dx)>abs(dy))
	{
		steps=abs(dx/RECTSIZE)+1;
		if (steps>1)
		{
			dy=dy/steps;
			dx=dx/steps;
		}
	}
	else
	{
		steps=abs(dy/RECTSIZE)+1;
		if (steps>1)
		{
			dy=dy/steps;
			dx=dx/steps;
		}
	}
	int radius=2;
	double x1,y1,x2,y2;
	if (dx>0)
	{
		x1=point1.x-dx/2-radius;
		x2=point1.x+dx/2+radius;
	}
	else
	{
		x2=point1.x-dx/2+radius;
		x1=point1.x+dx/2-radius;
	}
	if (dy>0)
	{
		y1=point1.y-dy/2-radius;
		y2=point1.y+dy/2+radius;
	}
	else
	{
		y2=point1.y-dy/2+radius;
		y1=point1.y+dy/2-radius;
	}
	for (int i=0;i<=steps;i++)
	{
		CRect rect(x1,y1,x2,y2);
		if (intersectrect.IntersectRect(clientrect,rect))
		{
			InvalidateRect(rect);
#ifndef NDEBUG
			debugboxmem.Set(rect);
#endif
		}
		x1+=dx;
		x2+=dx;
		y1+=dy;
		y2+=dy;
	}
#ifndef NDEBUG
	debugboxmem.lastline++;
#endif
}

/*
	// the polygon technique:
	// this isnt perfect
	CPoint points[6];
	int orientation;
	// figure out the orientation of point1 to point2
	// 0=topright 1=bottomright 2=bottomleft 3=topleft
	if (point1.x<=point2.x && point1.y>=point2.y) orientation=0;
	if (point1.x<=point2.x && point1.y<=point2.y) orientation=1;
	if (point1.x>=point2.x && point1.y<=point2.y) orientation=2;
	if (point1.x>=point2.x && point1.y>=point2.y) orientation=3;
	int i=0;
	if (orientation!=0) 
	{
		points[i]=CPoint(point1.x+2,point1.y-2);
		i++;
	}
	if (orientation!=1) 
	{
		points[i]=CPoint(point1.x+2,point1.y+2);
		i++;
	}
	if (orientation!=2) 
	{
		points[i]=CPoint(point1.x-2,point1.y+2);
		i++;
	}
	if (orientation!=3) 
	{
		points[i]=CPoint(point1.x-2,point1.y-2);
		i++;
	}

	if (orientation!=2) 
	{
		points[i]=CPoint(point2.x+2,point2.y-2);
		i++;
	}
	if (orientation!=3) 
	{
		points[i]=CPoint(point2.x+2,point2.y+2);
		i++;
	}
	if (orientation!=0) 
	{
		points[i]=CPoint(point2.x-2,point2.y+2);
		i++;
	}
	if (orientation!=1) 
	{
		points[i]=CPoint(point2.x-2,point2.y-2);
		i++;
	}
	CRgn region;
	region.CreatePolygonRgn(points,8,WINDING);
	InvalidateRgn(&region);
}
*/




//////////////////////////////////////////////////////////////////////
//
// Function:    OnTimer
// Date:		29/02/00
// Author:		JIM
//
//Description: 	movecode timer
//
//////////////////////////////////////////////////////////////////////
extern	int	Icon_Select_Add;
void CMapDlg::OnTimer(UINT nIDEvent) 
{
#ifndef	BOB_DEMO_VER

	if (m_view->m_currentpage==0)
	{
		if (RDialog::messageboxopen==false && !Master_3d.currinst)
			if (MMC.currtime==MORNINGPERIODSTART)
			{
				if (MMC.accumulator++)
				{
					MMC.accumulator=0;
					MMC.currtime+=SECSPERSAGFRAME;
					Node_Data.StartOfDay();

				}
			}
			else
			{
				if (MMC.curracceltype==MMC.ACCEL_DIALOGSPEED && MMC.curraccelrate<=3)
					if (!Campaign::AnyDialogsOpen())
						MMC.accumulator=0;
					else
					{
						CMainFrame* main=(CMainFrame*)AfxGetMainWnd();
	 					main->m_titlebar.Redraw();
					}


				MMC.accumulator+=MMC.curraccelrate;
				if (MMC.accumulator>=0)
				{  
					InvalidateMobiles(false,false);
					int	oldcolour=MMC.TimeColour();
					int nextperiod=Node_Data.PerformMoveCycle();
					Node_Data.PerformNextPeriod(nextperiod);
					InvalidateMobiles(true,oldcolour!=MMC.TimeColour());

					Campaign::GlobalRefreshData();

					CMainFrame* main=(CMainFrame*)AfxGetMainWnd();
					main->m_titlebar.Redraw();
					main->m_reportbar.Refresh();
					if (main->m_titlebar.LoggedChild(main->m_titlebar.CLOCK))
						main->m_titlebar.LoggedChild(main->m_titlebar.CLOCK)->fchild->RefreshData(-2);
			

				}

				if (_Radio.MessageFinished())				//RJS 28Mar00
					Node_Data.intel.SayNextMessage();		//RJS 28Mar00
			}
// Random ambience...
		if (Math_Lib.rnd() > 45000)										//RJS 25Oct00
			_Miles.UIPlayAmbient(SFX_OfficeAmbience);

		_Miles.UIProcessAmbience();										//RJS 9Jun00
	}
	if (nIDEvent==maptimer && _DPlay.PlayerCreated)
	{
		if (DPlay::H2H_Player[_DPlay.mySlot].status==DPlay::CPS_MAP)
		{

			_DPlay.UIUpdateMainSheet();
		}
	}
#endif
	CDialog::OnTimer(nIDEvent);
}

void	NodeData::PerformNextPeriod(int periodnum)
{
	switch (periodnum)
	{
	case 0:	/*..............*/		break;
	case 1:
	case 2:	FourHourPeriod(false);	break;
	case 3:	EndOfDay();				break;
	}
}

int	NodeData::PerformMoveCycle()	//returns next period number
{	
	int	periodrv=0;
	static volatile bool	insidemove=false;
	if (!insidemove)													//DAW 15/05/00
	{
		insidemove=true;
		while(MMC.accumulator>=0)
		{
			if (Save_Data.minsbetweensavegame)
				if (Save_Data.savegamesecscounter)
					Save_Data.savegamesecscounter--;
				else
				{
					CFiling::SaveTimedGame();
					Save_Data.savegamesecscounter=Save_Data.minsbetweensavegame*3;
				}
			MMC.currtime+=SECSPERSAGFRAME;
			MMC.accumulator-=SECSPERSAGFRAME;
			if (MMC.accumulator>MMC.curraccelrate)						//JIM 10Jul00
				MMC.accumulator=MMC.curraccelrate;						//JIM 10Jul00
			Profile::MoveSAGs();
			if (Todays_Packages.localplayer==NAT_RAF)
			{
				int &enemycounter=Todays_Packages.nextraidnum;
				if (Todays_Packages.nextraidnum==0 || MMC.currtime/SECSPERHR>(MMC.currtime-SECSPERSAGFRAME)/SECSPERHR )
					if (MMC.currtime<HR16)
						enemycounter=100*((MMC.currtime/SECSPERHR)%10);	
					else
						enemycounter=100*((MMC.currtime/SECSPERHR)%10)+50;	
			}
			if (MMC.currtime>MMC.dusktime)		//use rather than AFTERNOONPERIODEND so we can see dusk
				periodrv=3;
			else
			{
				Node_Data.GenerateProductionRates();
				Node_Data.DeliverProductionRates(SECSPERSAGFRAME);
				Node_Data.UpdateReadiness(SECSPERSAGFRAME);
				if (	(MMC.currtime>MIDDAYPERIODSTART && MMC.currtime-SECSPERSAGFRAME<=MIDDAYPERIODSTART)	)
					periodrv=1;
				else
				if (	(MMC.currtime>AFTERNOONPERIODSTART && MMC.currtime-SECSPERSAGFRAME<=AFTERNOONPERIODSTART)	)
					periodrv=2;
			}
		}
		insidemove=false;
	}
	return	periodrv;
}

//////////////////////////////////////////////////////////////////////
//
// Function:    OnInitDialog
// Date:		29/02/00
// Author:		JIM
//
//Description: 
//
//////////////////////////////////////////////////////////////////////
BOOL CMapDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
 	maptimer=SetTimer(DPlay::TIMER_MAP,1000,NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMapDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	KillTimer(maptimer);
	
}



LRESULT CMapDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	int rv= CDialog::DefWindowProc(message, wParam, lParam);
	switch (message)
	{
	case	WM_NCCREATE:			case	WM_PARENTNOTIFY:
	case	WM_NCCALCSIZE:			case	0x04c8:	//MCIWNDM_NOTIFY:
	case	WM_CREATE:				case	WM_NCPAINT:
	case	WM_SIZE:				case	WM_SETCURSOR:
	case	WM_MOVE:				case	WM_SETFONT:
	case	WM_INITDIALOG:			case	WM_DISPLAYCHANGE:
	case	WM_CTLCOLORSTATIC:		case	WM_IDLEUPDATECMDUI: //363
	case	WM_GETFONT:				case		WM_GETDLGCODE:
	case	WM_WINDOWPOSCHANGING:	case		WM_SETFOCUS:
	case	WM_SHOWWINDOW:			case		WM_USER:
	case	WM_PAINT:				case		0x0364://	 WM_INITIALUPDATE
	case	WM_WINDOWPOSCHANGED:	case		WM_KILLFOCUS:
	case	 WM_NCHITTEST:			case		WM_TIMER:



		return rv;
	}
	return rv;
}
