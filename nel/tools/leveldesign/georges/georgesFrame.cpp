// ChildFrm.cpp : implementation of the CGeorgesFrame class
//

#include "stdafx.h"
#include "georges.h"

#include "GeorgesFrame.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeorgesFrame

IMPLEMENT_DYNCREATE (CGeorgesFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP (CGeorgesFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CGeorgesFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeorgesFrame construction/destruction

CGeorgesFrame::CGeorgesFrame()
{
	// TODO: add member initialization code here
	
}

CGeorgesFrame::~CGeorgesFrame()
{
}

BOOL CGeorgesFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CGeorgesFrame::PreTranslateMessage (MSG* pMsg)
{
	GetActiveView()->PreTranslateMessage (pMsg);
	return CMDIChildWnd::PreTranslateMessage (pMsg);
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesFrame diagnostics

#ifdef _DEBUG
void CGeorgesFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CGeorgesFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGeorgesFrame message handlers

BOOL CGeorgesFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{
		// create static splitter view with two panes
/*	VERIFY(m_wndSplitter.CreateStatic( this,1,2, WS_CHILD | WS_VISIBLE ));
	VERIFY(m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CLogicTreeView),CSize(180,0),pContext));
	VERIFY(m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CEditorFormView),CSize(0,0),pContext));	
*/	
	return CMDIChildWnd::OnCreateClient(lpcs, pContext);
	//return TRUE;
}

