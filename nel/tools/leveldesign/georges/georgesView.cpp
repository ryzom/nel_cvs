// SuperVw.cpp : implementation file
//

#include "stdafx.h"
#include "Georges.h"
#include "GeorgesView.h"
#include "GeorgesDoc.h"
#include "NewItem.h"

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CGeorgesView, CView)

CGeorgesView::CGeorgesView()
{
	m_bDrag=TRUE;
}

CGeorgesView::~CGeorgesView()
{
}


BEGIN_MESSAGE_MAP(CGeorgesView, CView)
	//{{AFX_MSG_MAP(CGeorgesView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_COMMAND(ID_TOOLS_COLLAPSE, OnToolsCollapse)
	ON_COMMAND(ID_TOOLS_EXPAND, OnToolsExpand)
	ON_COMMAND(ID_TOOLS_EXPANDALL, OnToolsExpandall)
	ON_COMMAND(ID_TOOLS_DELETEALL, OnToolsDeleteall)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CGeorgesView::AssertValid() const
{
	CView::AssertValid();
}

void CGeorgesView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG

CGeorgesDoc* CGeorgesView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGeorgesDoc)));
	return (CGeorgesDoc*)m_pDocument;
}

/////////////////////////////////////////////////////////////////////////////

void CGeorgesView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	if(m_List.GetSafeHwnd())
			m_List.SetWindowPos(0,-1,-1,cx,cy,SWP_SHOWWINDOW);
}



int CGeorgesView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_List.Create(WS_CHILD | WS_VISIBLE,CRect(0,0,0,0),this,0x100);	
	m_List.InitializeGrid( GetDocument() );
	return 0;
}

//Yes I know this is so cheap programming, but let's all get on with our lifes :)
void CGeorgesView::OnToolsCollapse() 
{
	m_List.SendMessage(WM_KEYDOWN,(WPARAM)VK_SUBTRACT,0);
}

void CGeorgesView::OnToolsExpand() 
{
	m_List.SendMessage(WM_KEYDOWN,(WPARAM)VK_ADD,0);
}

void CGeorgesView::OnToolsExpandall() 
{
	m_List.SendMessage(WM_KEYDOWN,(WPARAM)VK_MULTIPLY,0);
}

void CGeorgesView::OnToolsDeleteall() 
{
	m_List._DeleteAll();	
}

void CGeorgesView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_List.LoadItem();
	CView::OnUpdate( pSender, lHint, pHint);
}



void CGeorgesView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

// ---------------------------------------------------------------------------
BOOL CGeorgesView::PreTranslateMessage (MSG *pMsg)
{
	return m_List.PreTranslateMessage (pMsg);
}

void CGeorgesView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo) 
{
	CView::OnPrepareDC(pDC, pInfo);
}

BOOL CGeorgesView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CGeorgesView::OnBeginPrinting(CDC* , CPrintInfo* )
{
	
}

void CGeorgesView::OnEndPrinting(CDC* , CPrintInfo* )
{
	// TODO: add cleanup after printing
}

void CGeorgesView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if(m_List.GetExtendedStyle() & LVS_EX_CHECKBOXES)
		m_List.HowToLoopThroughAllItems_that_has_a_checkmark_and_print_them_or_what_ever(pDC);
	else
		m_List.HowToLoopThroughAllItems_if_we_wanted_to_print_them_or_what_ever(pDC);	
	
	CView::OnPrint(pDC, pInfo);
}
