#if !defined(AFX_TYPESELDLG_H__C138FEAF_AB39_41BB_B71F_D903CE4DF909__INCLUDED_)
#define AFX_TYPESELDLG_H__C138FEAF_AB39_41BB_B71F_D903CE4DF909__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TypeSelDlg.h : header file
//

#include "mainfrm.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CTypeSelDlg dialog

class CTypeSelDlg : public CDialog
{
// Construction
public:

	CTypeSelDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data

	std::vector<SType> *_TypesInit;
	std::string _TypeSelected;

	//{{AFX_DATA(CTypeSelDlg)
	enum { IDD = IDD_TYPESEL };
	CListBox	TypeList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTypeSelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTypeSelDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TYPESELDLG_H__C138FEAF_AB39_41BB_B71F_D903CE4DF909__INCLUDED_)
