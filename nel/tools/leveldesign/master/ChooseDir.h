#if !defined(AFX_CHOOSEDIR_H__51164D9B_33B9_488B_AA09_75F2EF07A48A__INCLUDED_)
#define AFX_CHOOSEDIR_H__51164D9B_33B9_488B_AA09_75F2EF07A48A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseDir.h : header file
//

#include <string>
#include <vector>


/////////////////////////////////////////////////////////////////////////////
// CChooseDir dialog

class CChooseDir : public CDialog
{
	std::string _Path;
	std::vector<std::string> _Names;
	int _Sel;
// Construction
public:
	CChooseDir(CWnd* pParent = NULL);   // standard constructor

	void setPath (const std::string &path);
	const char *getSelected ();

// Dialog Data
	//{{AFX_DATA(CChooseDir)
	enum { IDD = IDD_CHOOSEDIR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	BOOL OnInitDialog ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseDir)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseDir)
	afx_msg void OnSelChangeList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEDIR_H__51164D9B_33B9_488B_AA09_75F2EF07A48A__INCLUDED_)
