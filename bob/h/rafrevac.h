/* STOP AFX_INCLUDES
//{{AFX_INCLUDES()
#include "rlistbox.h"
//}}AFX_INCLUDES

*/
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#if !defined(AFX_RAFREVAC_H__CDABD341_A197_11D3_8ADC_00A0D21BD395__INCLUDED_)
#define AFX_RAFREVAC_H__CDABD341_A197_11D3_8ADC_00A0D21BD395__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RAFRevAc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// RAFReviewAircraft dialog

class RAFReviewAircraft : public RowanDialog
{
// Construction
public:
	static RDialog* Make();

	RAFReviewAircraft(CWnd* pParent = NULL);   // standard constructor
	enum {SPITS, HURR, TOTAL,MAXTYPES};

// Dialog Data
	//{{AFX_DATA(RAFReviewAircraft)
	enum { IDD = IDD_RAFREVIEWAIRCRAFT };
	CRListBox	m_IDC_RLISTBOXCTRL1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RAFReviewAircraft)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(RAFReviewAircraft)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_RAFREVAC_H__CDABD341_A197_11D3_8ADC_00A0D21BD395__INCLUDED_)
