/* STOP AFX_INCLUDES
//{{AFX_INCLUDES()
#include "rstatic.h"
#include "redit.h"
#include "rbutton.h"
//}}AFX_INCLUDES

*/
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#if !defined(AFX_RAFCOMBT_H__82448F80_91D1_11D3_8ADB_00A0D21BD395__INCLUDED_)
#define AFX_RAFCOMBT_H__82448F80_91D1_11D3_8ADB_00A0D21BD395__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RAFCombt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// RAFCombat dialog

class RAFCombat : public RowanDialog
{
// Construction
public:
	UINT debrief_timer;
	static RDialog* Make(int index, bool date);

	RAFCombat(int index, bool date, CWnd* pParent = NULL);   // standard constructor
	void PreDestroyPanel();
	void SaveText();
	typedef int	StatsPerType[4];
	bool datereq;
	int entry;
// Dialog Data
	//{{AFX_DATA(RAFCombat)
	enum { IDD = IDD_RAFCOMBAT };
	CRStatic	m_IDC_RSTATICCRDATE;
	CRStatic	m_IDC_RSTATICENEMYAC;
	CRStatic	m_IDC_RSTATICENEMYCONC;
	CRStatic	m_IDC_RSTATICFLIGHTSQUAD;
	CRStatic	m_IDC_RSTATICHTENEMY;
	CRStatic	m_IDC_RSTATICOURAC;
	CRStatic	m_IDC_RSTATICOURPERS;
	CRStatic	m_IDC_RSTATICPLACEATTK;
	CRStatic	m_IDC_RSTATICTIMEATTK;
	CRStatic	m_IDC_RSTATICTYPEENEMYAC;
	CREdit	m_IDC_NAME;
	CRStatic	m_IDC_RSTATICUNIT;
	CRButton	m_IDJ_TITLE;
	CRStatic	m_IDC_RSTATICCTRL24;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RAFCombat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void	LWRefresh();
	void	RAFRefresh();
	CString EnemyReport(StatsPerType* acarray,int c0,int c1);

	// Generated message map functions
	//{{AFX_MSG(RAFCombat)
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_RAFCOMBT_H__82448F80_91D1_11D3_8ADB_00A0D21BD395__INCLUDED_)
