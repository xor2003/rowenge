/*
//{{AFX_INCLUDES()

//}}AFX_INCLUDES
*/
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#if !defined(AFX_MAINTBAR_H__C13BD762_E594_11D1_A1F0_0080C8582DE4__INCLUDED_)
#define AFX_MAINTBAR_H__C13BD762_E594_11D1_A1F0_0080C8582DE4__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MainTbar.h : header file
//
#include "RToolbar.h"

/////////////////////////////////////////////////////////////////////////////
// CMainToolbar dialog

class CMainToolbar : public CRToolBar
{	friend	class	CMIGView;
// Construction
public:
	CMainToolbar(CWnd* pParent = NULL);   // standard constructor
	enum	{	ACALLOC, BASES, SQUADRONLIST,WEATHER,
				REVIEW, PILOTDATA, ASSETLIST, MISSIONFOLDER, HOSTILESLIST,
				DOSSIER, ACDOSSIER, DIARY,INTERCEPTOFFERED,
				TAKEOVEROFFERED, ROUTE , WAYPOINT,
				MAX_ID
			};

	void ChildDialClosed(int dialnum,RDialog*,int rv);
	static int dialids[MAX_ID];
	CRButton	m_dialids[MAX_ID];
	void CloseAllDialogs();
	void CloseAllDialogsBut(int);
	enum{RAFENDDAYREVIEW = 7, LWENDDAYREVIEW = 4};
	static int LWReview[LWENDDAYREVIEW];
	static int RAFReview[RAFENDDAYREVIEW];
// Dialog Data
	//{{AFX_DATA(CMainToolbar)
	enum { IDD = IDDT_MAINTOOLBAR };
		// NOTE: the ClassWizard will add data members here
	CRButton	m_IDC_FRAG2;

	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainToolbar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
   void OpenDirectives();
   void OpenDossier(UniqueID item);
   void OpenACDossier(int item, int gruppe);
   void OpenDiary(int squad);
	void OpenInterceptOffered(int pack,int raid);
	void OpenTakeOfferOffered();
	void OpenRoute();
	void OpenWP();
	void	OpenMissionfolder(bool special=false);

   void OpenDossierNotFrontLine(UniqueID item);
   void DialoguesVisible( bool vis);
	void OnClickedMissionlog();
   void OpenHostileslist();
	void AnythingImportantToReview();
	void FindSomethingToReview();
	bool EndofWar();
	void TailorHints();


	// Generated message map functions
	//{{AFX_MSG(CMainToolbar)
	afx_msg void OnClickedChangeToTitle();
	afx_msg void OnClickedBases();
	afx_msg void OnClickedDirectives();
	afx_msg void OnClickedMissionresults();
	afx_msg void OnClickedPlayerlog();
	afx_msg void OnClickedWeather();
	afx_msg void OnClickedDis();
	afx_msg void OnClickedFrag2();
	afx_msg void EndDayReview();
 	afx_msg void EndDayRouting();

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClickedAircraftalloc();
	afx_msg void OnClickedSquaronlist();
	afx_msg void OnClickedReview();
	afx_msg void OnClickedPilotdata();
	afx_msg void OnClickedAssetlist();
	afx_msg void OnClickedMissionfolder();
	afx_msg void OnClickedHostileslist();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_MAINTBAR_H__C13BD762_E594_11D1_A1F0_0080C8582DE4__INCLUDED_)
