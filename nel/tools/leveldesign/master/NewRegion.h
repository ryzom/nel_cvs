#if !defined(AFX_NEWREGION_H__27E9FA5F_8636_46BF_BD58_BA0E95CB9297__INCLUDED_)
#define AFX_NEWREGION_H__27E9FA5F_8636_46BF_BD58_BA0E95CB9297__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewRegion.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NewRegion dialog

class NewRegion : public CDialog
{
// Construction
public:
	NewRegion(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(NewRegion)
	enum { IDD = IDD_NEWREGION };
	CString	str;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NewRegion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NewRegion)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWREGION_H__27E9FA5F_8636_46BF_BD58_BA0E95CB9297__INCLUDED_)
