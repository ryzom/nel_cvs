#if !defined(AFX_TYPEDLG_H__755F39BC_15C5_44AD_BFB0_B52D335F3173__INCLUDED_)
#define AFX_TYPEDLG_H__755F39BC_15C5_44AD_BFB0_B52D335F3173__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TypeDlg.h : header file
//
#include "resource.h"
#include "color_button.h"

/////////////////////////////////////////////////////////////////////////////
// CTypeDlg dialog

class CTypeDlg : public CDialog
{
// Construction
public:
	CTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTypeDlg)
	enum { IDD = IDD_TYPE };
	CColorButton ButtonColor;

	CRGBA ButtonColorValue;
	CString	EditName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTypeDlg)
	afx_msg void OnButtoncolor();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TYPEDLG_H__755F39BC_15C5_44AD_BFB0_B52D335F3173__INCLUDED_)
