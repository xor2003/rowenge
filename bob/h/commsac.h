/*
//{{AFX_INCLUDES()

//}}AFX_INCLUDES
*/
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#if !defined(AFX_COMMSAC_H__A2162461_5B7F_11D2_9FD5_D0EC00C10000__INCLUDED_)
#define AFX_COMMSAC_H__A2162461_5B7F_11D2_9FD5_D0EC00C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CommsAc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCommsDeathMatchAc dialog

class CCommsDeathMatchAc : public RowanDialog
{
// Construction
public:
	CCommsDeathMatchAc(CWnd* pParent = NULL);   // standard constructor
	UINT	acselect_timer;

// Dialog Data
	//{{AFX_DATA(CCommsDeathMatchAc)
	enum { IDD = IDDX_COMMSAC };
	CRCombo	m_IDC_CURRENTAC;
	CRStatic	m_IDC_CURRENTAIRCRAFT;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommsDeathMatchAc)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCommsDeathMatchAc)
	virtual BOOL OnInitDialog();
	afx_msg void OnTextChangedCurrentac(LPCTSTR text, short index);
	afx_msg void OnTimer(UINT nIDEvent);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_COMMSAC_H__A2162461_5B7F_11D2_9FD5_D0EC00C10000__INCLUDED_)
