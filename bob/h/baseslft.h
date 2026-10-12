#if !defined(AFX_BASESLFT_H__354A1BA0_8ECF_11D3_8ADB_00A0D21BD395__INCLUDED_)
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#define AFX_BASESLFT_H__354A1BA0_8ECF_11D3_8ADB_00A0D21BD395__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BasesLft.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BasesLuftflotte dialog

class BasesLuftflotte : public RowanDialog
{
// Construction
public:
	static RDialog* Make();

	BasesLuftflotte(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(BasesLuftflotte)
	enum { IDD = IDD_BASES_LUFTFLOTTE };
		// NOTE: the ClassWizard will add data members here
	CRButton	m_IDJ_TITLE;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BasesLuftflotte)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(BasesLuftflotte)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_BASESLFT_H__354A1BA0_8ECF_11D3_8ADB_00A0D21BD395__INCLUDED_)
