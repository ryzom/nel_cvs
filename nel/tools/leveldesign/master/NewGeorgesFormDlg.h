#if !defined(AFX_NEWGEORGESFORMDLG_H__8EA28C00_C511_4ED4_B072_0BC47A2DAEE0__INCLUDED_)
#define AFX_NEWGEORGESFORMDLG_H__8EA28C00_C511_4ED4_B072_0BC47A2DAEE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewGeorgesFormDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewGeorgesFormDlg dialog

class CNewGeorgesFormDlg : public CDialog
{
// Construction
public:
	CNewGeorgesFormDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewGeorgesFormDlg)
	enum { IDD = IDD_GEORGESFORM };
	CString	str;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewGeorgesFormDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewGeorgesFormDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWGEORGESFORMDLG_H__8EA28C00_C511_4ED4_B072_0BC47A2DAEE0__INCLUDED_)
