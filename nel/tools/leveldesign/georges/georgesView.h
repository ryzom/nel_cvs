#if !defined(AFX_GEORGESVIEW_H__48B715E6_80AB_11D2_9A97_002018026B76__INCLUDED_)
#define AFX_GEORGESVIEW_H__48B715E6_80AB_11D2_9A97_002018026B76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CGeorgesView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
#include "MySuperGrid.h"

class CGeorgesView : public CView
{
protected:
	CGeorgesView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CGeorgesView)

// Attributes
public:
	CMySuperGrid m_List;
	BOOL m_bDrag;
// Attributes
public:
	CGeorgesDoc* GetDocument();
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesView)
	public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
protected:
	virtual ~CGeorgesView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnToolsCollapse();
	afx_msg void OnToolsExpand();
	afx_msg void OnToolsExpandall();
	afx_msg void OnToolsDeleteall();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGESVIEW_H__48B715E6_80AB_11D2_9A97_002018026B76__INCLUDED_)
