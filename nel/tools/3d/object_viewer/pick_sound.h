#if !defined(AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_)
#define AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// pick_sound.h : header file
//

#include <vector>
#include <string>

/////////////////////////////////////////////////////////////////////////////
// CPickSound dialog

class CPickSound : public CDialog
{
// Construction
public:
	typedef std::vector<const char *> TNameVect;
	CPickSound(TNameVect &names, CWnd* pParent = NULL);   // standard constructor


	const std::string &getName(void) const { return _CurrName; }

// Dialog Data
	//{{AFX_DATA(CPickSound)
	enum { IDD = IDD_PICK_SOUND };
	CListBox	m_NameList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPickSound)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	TNameVect _Names;
	std::string _CurrName;

	// Generated message map functions
	//{{AFX_MSG(CPickSound)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PICK_SOUND_H__14638414_B951_439C_A087_5468A8CDFCE1__INCLUDED_)
