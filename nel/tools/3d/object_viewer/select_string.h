#if !defined(AFX_SELECT_STRING_H__A9ECE120_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_SELECT_STRING_H__A9ECE120_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// select_string.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectString dialog

class CSelectString : public CDialog
{
// Construction
public:
	CSelectString(const std::vector<std::string>& vectString, const char* title, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectString)
	enum { IDD = IDD_SELECT_STRING };
	CListBox	ListCtrl;
	//}}AFX_DATA

	std::string							Title;
	std::vector<std::string>			Strings;
	int									Selection;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectString)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectString)
	virtual void OnOK();
	afx_msg void OnDblclkList();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnEmpty();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECT_STRING_H__A9ECE120_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
