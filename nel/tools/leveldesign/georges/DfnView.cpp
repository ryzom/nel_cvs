// SuperVw.cpp : implementation file
//

#include "stdafx.h"
#include "DfnView.h"
#include "DfnDoc.h"
#include "resource.h"

using namespace std;

// ---------------------------------------------------------------------------
// CDfnView
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP (CDfnView, CView)
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE (CDfnView, CView)

// ---------------------------------------------------------------------------
CDfnView::CDfnView ()
{
	m_bDrag=TRUE;
}

// ---------------------------------------------------------------------------
CDfnView::~CDfnView ()
{
}

#ifdef _DEBUG

// ---------------------------------------------------------------------------
void CDfnView::AssertValid () const
{
	CView::AssertValid();
}

// ---------------------------------------------------------------------------
void CDfnView::Dump (CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG

// ---------------------------------------------------------------------------
CDfnDoc* CDfnView::GetDocument () // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDfnDoc)));
	return (CDfnDoc*)m_pDocument;
}

// ---------------------------------------------------------------------------
void CDfnView::convertToDfn (CDfnDoc::TDfn &out)
{
	DfnEltList.convertToDfn (out);
}

// ---------------------------------------------------------------------------
void CDfnView::makeFromDfn (CDfnDoc::TDfn &in)
{
	DfnEltList.makeFromDfn (in);
}

// ---------------------------------------------------------------------------
void CDfnView::OnSize (UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	if (DfnEltList.GetSafeHwnd())
		DfnEltList.SetWindowPos (0, -1, -1, cx, cy, SWP_SHOWWINDOW);
}

// ---------------------------------------------------------------------------
int CDfnView::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	DfnEltList.Create (WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, 0x101);	
	DfnEltList.SetExtendedStyle(LVS_EX_GRIDLINES);

	// Initialize the CSuperGridCtrl (Initialize the column names)
	LPTSTR lpszCols[] = { _T("Name"), _T("List"), _T("Type"), 0 };
	LV_COLUMN lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	for (int x = 0; lpszCols[x]!=NULL; x++)
    {
		if (x == 0)
			lvColumn.cx = 200;
		else if (x == 1)
			lvColumn.cx = 50;
		else if (x == 2)
			lvColumn.cx = 300;
		lvColumn.pszText = lpszCols[x];
		DfnEltList.InsertColumn(x,&lvColumn);
    }
	return 0;
}

// ---------------------------------------------------------------------------
void CDfnView::OnDraw (CDC* pDC)
{
}

// ---------------------------------------------------------------------------
BOOL CDfnView::PreTranslateMessage (MSG* pMsg)
{	// Pretranslate is usefull because CSuperGridCtrl do special action on keystrokes
	return DfnEltList.PreTranslateMessage (pMsg);
}

