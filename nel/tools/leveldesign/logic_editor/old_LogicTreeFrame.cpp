// LogicTreeFrame.cpp : implementation file
//

#include "stdafx.h"
#include "logic_editor.h"
#include "LogicTreeFrame.h"

#include "StatesView.h"
#include "ConditionsView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogicTreeFrame

IMPLEMENT_DYNCREATE(CLogicTreeFrame, CMDIChildWnd)

CLogicTreeFrame::CLogicTreeFrame()
{
}

CLogicTreeFrame::~CLogicTreeFrame()
{
}


BEGIN_MESSAGE_MAP(CLogicTreeFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CLogicTreeFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogicTreeFrame message handlers

BOOL CLogicTreeFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{

		// create static splitter view with two panes
	VERIFY(m_wndSplitter.CreateStatic( this,2,1, WS_CHILD | WS_VISIBLE ));
	VERIFY(m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CStatesView),CSize(10,200),pContext));
	VERIFY(m_wndSplitter.CreateView(1,0,RUNTIME_CLASS(CConditionsView),CSize(10,10),pContext));	
	
	//return CMDIChildWnd::OnCreateClient(lpcs, pContext);
	return TRUE;	
}
