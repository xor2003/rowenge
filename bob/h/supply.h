/*
//{{AFX_INCLUDES()
#include "rbutton.h"
//}}AFX_INCLUDES
*/
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#if !defined(AFX_SUPPLY_H__7CC731A1_CA3D_11D1_A5A3_0040052179B6__INCLUDED_)
#define AFX_SUPPLY_H__7CC731A1_CA3D_11D1_A5A3_0040052179B6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Supply.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSupply dialog
class CSupply : public RowanDialog
{
// Construction
public:
	CSupply(int index,CWnd* pParent = NULL);   // standard constructor
	~CSupply();
 	int sortindex;
	int currrow;

	int panel;
// Dialog Data
	//{{AFX_DATA(CSupply)
	enum { IDD = IDD_TRG_ASSET_LIST };
	CRStatic	m_IDC_RSTATICCTRL2;
	CRCombo	m_IDC_SORTCOMBO;
	CRListBox	m_IDC_RLISTBOXCTRL1;
//	CRButton	m_IDC_AUTHORISE;
//	CRButton	m_IDC_DOSSIER;
	CRButton	m_IDC_RBUTTONAUTHORISE;
	CRButton	m_IDC_RBUTTONDETAILS;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSupply)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
//TEMPCODE JIM 07/02/00 	DirectivesResults::TargetList target[DirectivesResults::MAX_TARGETS];
	LWDirectivesResults*		dr;
//	int	 FillPanel(LWDirectivesResults::TargetList* target, int panel);
	int	 FillPanel(int panel);

	void ReDraw(int	numoftargets);
//	int	InsertTarget(UniqueID uid, int currtrg);

	void ReDrawButtons();



	// Generated message map functions
	//{{AFX_MSG(CSupply)
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedDossier();
	afx_msg void OnTextChangedSortcombo(LPCTSTR text, short index);
	afx_msg void OnSelectRlistboxctrl1(long row, long column);
	afx_msg void OnClickedAuthorise();
	afx_msg void OnClose();
	afx_msg void OnClickedRbuttondetails();
	afx_msg void OnClickedRbuttonauthorise();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_SUPPLY_H__7CC731A1_CA3D_11D1_A5A3_0040052179B6__INCLUDED_)
