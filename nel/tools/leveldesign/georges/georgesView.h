// GeorgesView.h : interface of the CGeorgesView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEORGESVIEW_H__CFA2DDEB_C5A3_40B6_BC45_B36EEECCEA65__INCLUDED_)
#define AFX_GEORGESVIEW_H__CFA2DDEB_C5A3_40B6_BC45_B36EEECCEA65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ListViewEx.h"

class CGeorgesView : public CListViewEx
{
protected: // create from serialization only
	CGeorgesView();
	DECLARE_DYNCREATE(CGeorgesView)

	void GetDispInfo( LVITEM* const );
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

// Attributes
public:
	CGeorgesDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesView)
	public:
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeorgesView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	unsigned int xSubItem;
	unsigned int ySubItem;

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in GeorgesView.cpp
inline CGeorgesDoc* CGeorgesView::GetDocument()
   { return (CGeorgesDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGESVIEW_H__CFA2DDEB_C5A3_40B6_BC45_B36EEECCEA65__INCLUDED_)
