#if !defined(AFX_MOVEDLG_H__C18719D1_9BB9_42EC_9CFF_ED4C16C1DF74__INCLUDED_)
#define AFX_MOVEDLG_H__C18719D1_9BB9_42EC_9CFF_ED4C16C1DF74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MoveDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMoveDlg dialog

class CMoveDlg : public CDialog
{
// Construction
public:
	CMoveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMoveDlg)
	enum { IDD = IDD_MOVE };
	int		XOffset;
	int		YOffset;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMoveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMoveDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVEDLG_H__C18719D1_9BB9_42EC_9CFF_ED4C16C1DF74__INCLUDED_)
