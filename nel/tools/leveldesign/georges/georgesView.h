// georgesView.h : interface of the CGeorgesView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEORGESVIEW_H__D3FDCE92_15B3_4874_8A5F_CABD8546FCF2__INCLUDED_)
#define AFX_GEORGESVIEW_H__D3FDCE92_15B3_4874_8A5F_CABD8546FCF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPropertyList;
class CPropertyItem;

class CGeorgesView : public CFormView
{
	CPropertyList	*_List;

	CPropertyItem	*pPipo1;
	CPropertyItem	*pPipo2;
	CPropertyItem	*pPipo3;
	CPropertyItem	*pPipo4;
	CPropertyItem	*pPipo5;
	//CListBox*_List;

protected: // create from serialization only
	CGeorgesView();
	DECLARE_DYNCREATE(CGeorgesView)

// Attributes
public:
	CGeorgesDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeorgesView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGeorgesView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGeorgesView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize (UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in georgesView.cpp
inline CGeorgesDoc* CGeorgesView::GetDocument()
   { return (CGeorgesDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GEORGESVIEW_H__D3FDCE92_15B3_4874_8A5F_CABD8546FCF2__INCLUDED_)
