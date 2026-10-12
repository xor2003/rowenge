/* STOP AFX_INCLUDES
//{{AFX_INCLUDES()
#include "rlistbox.h"
#include "redit.h"
//}}AFX_INCLUDES

*/
#pragma pack(push,1)	// self-assert: dosdefs-era headers require pack(1)
									// regardless of ambient packing - class/file-layout ODR.
#if !defined(AFX_COMMCHAT_H__C9C5C760_ABC7_11D3_8ADC_00A0D21BD395__INCLUDED_)
#define AFX_COMMCHAT_H__C9C5C760_ABC7_11D3_8ADC_00A0D21BD395__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommChat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CommsChat dialog

class CommsChat : public RowanDialog
{
// Construction
public:
	static RDialog* Make();

	CommsChat(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CommsChat)
	enum { IDD = IDD_COMMSCHAT };
	CRListBox	m_IDC_RLIST_INCOMING_CHAT;
	CREdit	m_IDC_PLAYERCHAT;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CommsChat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CommsChat)
	virtual BOOL OnInitDialog();
	afx_msg void OnReturnPressedPlayerchat(LPCTSTR text);
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_COMMCHAT_H__C9C5C760_ABC7_11D3_8ADC_00A0D21BD395__INCLUDED_)
