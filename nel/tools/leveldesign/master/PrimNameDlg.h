#if !defined(AFX_PRIMNAMEDLG_H__F0E3AA5C_BDB0_4164_A70B_D3C86235E508__INCLUDED_)
#define AFX_PRIMNAMEDLG_H__F0E3AA5C_BDB0_4164_A70B_D3C86235E508__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrimNameDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrimNameDlg dialog

class CPrimNameDlg : public CDialog
{
// Construction
public:
	CPrimNameDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrimNameDlg)
	enum { IDD = IDD_PRIMNAME };
	CString	str;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrimNameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrimNameDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRIMNAMEDLG_H__F0E3AA5C_BDB0_4164_A70B_D3C86235E508__INCLUDED_)
