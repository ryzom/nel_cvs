#if !defined(AFX_CHOOSE_ANIMATION_H__2C1770D4_3508_4C3B_8052_72B716F6356B__INCLUDED_)
#define AFX_CHOOSE_ANIMATION_H__2C1770D4_3508_4C3B_8052_72B716F6356B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_animation.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseAnimation dialog

class CChooseAnimation : public CDialog
{
// Construction
public:
	CChooseAnimation(CWnd* pParent = NULL);   // standard constructor
	void init(const std::vector<std::string> &animList);
	std::string getSelectedAnim() const { return _Selection == -1 ? "" : _AnimList[_Selection]; }	
// Dialog Data
	//{{AFX_DATA(CChooseAnimation)
	enum { IDD = IDD_CHOOSE_ANIM };
	CListBox	m_AnimList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseAnimation)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	std::vector<std::string> _AnimList;
	sint					 _Selection;
	// Generated message map functions
	//{{AFX_MSG(CChooseAnimation)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkAnimList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_ANIMATION_H__2C1770D4_3508_4C3B_8052_72B716F6356B__INCLUDED_)
