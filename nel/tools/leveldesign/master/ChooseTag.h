#if !defined(AFX_CHOOSETAG_H__4185965E_4D29_491F_8601_7B29BBF28328__INCLUDED_)
#define AFX_CHOOSETAG_H__4185965E_4D29_491F_8601_7B29BBF28328__INCLUDED_

#include <string>
#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseTag.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseTag dialog

class CChooseTag : public CDialog
{
	std::string _Path;
	std::vector<std::string> _Names;
	int _Sel;
// Construction
public:
	CChooseTag (CWnd* pParent = NULL);   // standard constructor
	void setPath (const std::string &path);
	const char *getSelected ();

// Dialog Data
	//{{AFX_DATA(CChooseTag)
	enum { IDD = IDD_CHOOSETAG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	BOOL OnInitDialog ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseTag)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseTag)
	afx_msg void OnSelchangeList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSETAG_H__4185965E_4D29_491F_8601_7B29BBF28328__INCLUDED_)
