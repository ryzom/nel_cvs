#if !defined(AFX_TypeManagerDlg_H__BCA5979B_8741_4D1A_BCA7_ECFAFB8A14E1__INCLUDED_)
#define AFX_TypeManagerDlg_H__BCA5979B_8741_4D1A_BCA7_ECFAFB8A14E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TypeManagerDlg.h : header file
//
#include "mainfrm.h"
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CTypeManagerDlg dialog

class CTypeManagerDlg : public CDialog
{

	std::vector<CType>	LocalTypes;

// Construction
public:
	CTypeManagerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTypeManagerDlg)
	enum { IDD = IDD_TYPEMANAGER };
	CListBox	ListType;
	//}}AFX_DATA

	void set (const std::vector<CType> &types);
	const std::vector<CType> get ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTypeManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTypeManagerDlg)
	afx_msg void OnAddtype();
	afx_msg void OnEdittype();
	afx_msg void OnRemovetype();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TypeManagerDlg_H__BCA5979B_8741_4D1A_BCA7_ECFAFB8A14E1__INCLUDED_)
