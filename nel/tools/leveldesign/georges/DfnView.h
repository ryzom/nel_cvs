// ---------------------------------------------------------------------------
// DfnView.h
// ---------------------------------------------------------------------------
#if !defined(AFX_DfnView_H__48B715E6_80AB_11D2_9A97_002018026B76__INCLUDED_)
#define AFX_DfnView_H__48B715E6_80AB_11D2_9A97_002018026B76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CDfnView.h : header file
//

// ---------------------------------------------------------------------------

#include "dfndoc.h"
#include "dfnGrid.h"

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
class CDfnView : public CView
{
protected:
	CDfnView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDfnView)

// Attributes
public:
	CDfnGrid DfnEltList;
	BOOL m_bDrag;
// Attributes
public:
	CDfnDoc* GetDocument();
// Operations
public:

	void convertToDfn (CDfnDoc::TDfn &out);
	void makeFromDfn (CDfnDoc::TDfn &in);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDfnView)
	public:

	virtual void OnDraw (CDC* pDC);
	virtual BOOL PreTranslateMessage (MSG* pMsg);

// Implementation
protected:

	virtual ~CDfnView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CDfnView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DfnView_H__48B715E6_80AB_11D2_9A97_002018026B76__INCLUDED_)
