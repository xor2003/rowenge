/* STOP AFX_INCLUDES
//{{AFX_INCLUDES()

//}}AFX_INCLUDES

*/
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#if !defined(AFX_DIRNORES_H__789AD080_790B_11D4_ADE7_0040052179B6__INCLUDED_)
#define AFX_DIRNORES_H__789AD080_790B_11D4_ADE7_0040052179B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DirNoRes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DirectivesNoResults dialog

class DirectivesNoResults : public RowanDialog
{
// Construction
public:
	static RDialog* Make();

	DirectivesNoResults(CWnd* pParent = NULL);   // standard constructor
	void	OnOK();
	void	OnCancel();
// Dialog Data
	//{{AFX_DATA(DirectivesNoResults)
	enum { IDD = IDD_DIRECTIVENORESULTS };
		// NOTE: the ClassWizard will add data members here
	CRStatic	m_IDC_RSTATICCTRL1;
	CRStatic	m_IDC_RSTATICCTRL60;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DirectivesNoResults)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DirectivesNoResults)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_DIRNORES_H__789AD080_790B_11D4_ADE7_0040052179B6__INCLUDED_)
