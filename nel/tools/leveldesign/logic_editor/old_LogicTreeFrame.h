#if !defined(AFX_LOGICTREEFRAME_H__27DC78D4_7303_47EE_9304_CAE363886AC8__INCLUDED_)
#define AFX_LOGICTREEFRAME_H__27DC78D4_7303_47EE_9304_CAE363886AC8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogicTreeFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogicTreeFrame frame

class CLogicTreeFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CLogicTreeFrame)
protected:
	CLogicTreeFrame();           // protected constructor used by dynamic creation

// Attributes
public:
	/// the splitter window in the view
	CSplitterWnd m_wndSplitter;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogicTreeFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CLogicTreeFrame();

	// Generated message map functions
	//{{AFX_MSG(CLogicTreeFrame)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGICTREEFRAME_H__27DC78D4_7303_47EE_9304_CAE363886AC8__INCLUDED_)
