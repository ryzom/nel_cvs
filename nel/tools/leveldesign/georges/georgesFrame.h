/////////////////////////////////////////////////////////////////////////////
//
// GeorgesFrame.h : interface of the CGeorgesFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDFRM_H__22A72D32_4D6E_4F87_BE20_8EFC6F058F17__INCLUDED_)
#define AFX_CHILDFRM_H__22A72D32_4D6E_4F87_BE20_8EFC6F058F17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CGeorgesFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CGeorgesFrame)
public:
	CGeorgesFrame();

// Attributes
//protected:
public:
	/// the splitter window in the view
	CSplitterWnd m_wndSplitter;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesFrame)
	public:
	virtual BOOL PreCreateWindow (CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage (MSG* pMsg);

	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeorgesFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesFrame)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDFRM_H__22A72D32_4D6E_4F87_BE20_8EFC6F058F17__INCLUDED_)
