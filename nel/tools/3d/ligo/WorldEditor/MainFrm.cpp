// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"

#include <afxdlgs.h> // CFileDialog

#include "WorldEditor.h"
#include "display.h"
#include "toolsLogic.h"
#include "toolsZone.h"

#include "MainFrm.h"
#include "resource.h"

using namespace NLLIGO;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MENUSELECT()
	ON_COMMAND(ID_FILE_OPEN, OnMenuFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnMenuFileSave)
	ON_COMMAND(ID_FILE_EXIT, OnMenuFileExit)
	ON_COMMAND(ID_MODE_ZONE, OnMenuModeZone)
	ON_COMMAND(ID_MODE_LOGIC, OnMenuModeLogic)
	ON_COMMAND(ID_VIEW_GRID, OnMenuViewGrid)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	//ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

// ---------------------------------------------------------------------------
CMainFrame::CMainFrame()
{
	_Mode = 0;
	_SplitterCreated = false;
}

// ---------------------------------------------------------------------------
CMainFrame::~CMainFrame()
{
}

// ---------------------------------------------------------------------------
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	/*if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}*/
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
									| CBRS_SIZE_DYNAMIC) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
/*
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
*/
	return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT,
	 CCreateContext* pContext)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// create a splitter with 1 row, 2 columns
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	// add the first splitter pane - the default view in column 0
	if (!m_wndSplitter.CreateView(0, 0, 
		RUNTIME_CLASS(CDisplay), CSize(640, 512), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	// add the second splitter pane - an input view in column 1
	if (!m_wndSplitter.CreateView(0, 1,
		RUNTIME_CLASS(CToolsZone), CSize(100, 100), pContext))
	{
		TRACE0("Failed to create second pane\n");
		return FALSE;
	}

	_SplitterCreated = true;

	return TRUE;
}

// ---------------------------------------------------------------------------
void CMainFrame::initDisplay()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SetActiveView((CView*)m_wndSplitter.GetPane(0,0));
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	dispWnd->setCellSize (_Config.CellSize);
	dispWnd->init (this);
}

// ---------------------------------------------------------------------------
void CMainFrame::uninitTools()
{
	if (_Mode == 0) // Mode Zone
	{
		CToolsZone *toolWnd = dynamic_cast<CToolsZone*>(m_wndSplitter.GetPane(0,1));
		toolWnd->uninit ();
	}
	if (_Mode == 1) // Mode Logic
	{
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
		toolWnd->uninit ();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::initTools()
{
	if (_Mode == 0) // Mode Zone
	{
		m_wndSplitter.DeleteView(0, 1);
		m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CToolsZone), CSize(100, 100), NULL);
		SetActiveView((CView*)m_wndSplitter.GetPane(0,1));
		CToolsZone *toolWnd = dynamic_cast<CToolsZone*>(m_wndSplitter.GetPane(0,1));
		toolWnd->init (this);
		adjustSplitter ();
	}
	if (_Mode == 1) // Mode Logic
	{
		m_wndSplitter.DeleteView(0, 1);
		m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CToolsLogic), CSize(100, 100), NULL);
		SetActiveView((CView*)m_wndSplitter.GetPane(0,1));
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
		toolWnd->init (this);
		adjustSplitter ();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::displayCoordinates (NLMISC::CVector &v)
{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	std::string sTmp;
	sTmp = "( " + NLMISC::toString(v.x) + " , " + NLMISC::toString(v.y) + " )";
	m_wndStatusBar.SetWindowText (sTmp.c_str());
}

/*
void CMainFrame::OnPaint()
{
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	dispWnd->OnPaint();

	CTools *toolWnd = dynamic_cast<CTools*>(m_wndSplitter.GetPane(0,1));
	toolWnd->OnPaint();

	m_wndToolBar.Invalidate();
	m_wndStatusBar.Invalidate();
}*/



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
// ---------------------------------------------------------------------------
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

// ---------------------------------------------------------------------------
void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
bool CMainFrame::loadConfig ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// Get the module path
	HMODULE hModule = GetModuleHandle ("WORLDEDITOR.EXE");
	if (hModule)
	{
		// Get the path
		char sModulePath[256];
		int res=GetModuleFileName (hModule, sModulePath, 256);

		// Success ?
		if (res)
		{
			// Path
			char sDrive[256];
			char sDir[256];
			_splitpath (sModulePath, sDrive, sDir, NULL, NULL);
			_makepath (sModulePath, sDrive, sDir, "ligoscape", ".cfg");

			try
			{
				// Load the config file
				_Config.read (sModulePath);

				// ok
				return true;
			}
			catch (NLMISC::Exception& e)
			{
				MessageBox (e.what(), "Warning");
			}
		}
	}
	// Can't found the module put some default values
	_Config.CellSize = 160.0f;
	_Config.Snap = 1.0f;
	return false;
}

// MESSAGES FROM MENU

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileOpen()
{
	CFileDialog dialog (true, "logic");
	if (dialog.DoModal() == IDOK)
	{
		_PRegionBuilder.load ((LPCTSTR)dialog.GetFileName());
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileSave()
{
	CFileDialog dialog (false, "logic");
	if (dialog.DoModal() == IDOK)
	{
		_PRegionBuilder.save ((LPCTSTR)dialog.GetFileName());
	}	
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileExit()
{
	OnClose();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuModeZone()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuItem (ID_MODE_ZONE, MF_CHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_MODE_LOGIC, MF_UNCHECKED|MF_BYCOMMAND);
	if (_Mode != 0) // Mode Zone
	{
		uninitTools();
		_Mode = 0;
		initTools();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuModeLogic()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuItem (ID_MODE_ZONE, MF_UNCHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_MODE_LOGIC, MF_CHECKED|MF_BYCOMMAND);
	if (_Mode != 1) // Mode Logic
	{
		uninitTools();
		_Mode = 1;
		initTools();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuViewGrid()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	dispWnd->setDisplayGrid (!dispWnd->getDisplayGrid());
	CMenu *menu = GetMenu();
	menu->CheckMenuItem (ID_VIEW_GRID, dispWnd->getDisplayGrid()?MF_CHECKED|MF_BYCOMMAND:MF_UNCHECKED|MF_BYCOMMAND);

}

// ---------------------------------------------------------------------------
void CMainFrame::OnClose ()
{
	_Exit = true;
}

// ---------------------------------------------------------------------------
void CMainFrame::OnSize (UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize (nType, cx, cy);
	if (nType != SIZE_MINIMIZED)
		adjustSplitter ();
}

// ---------------------------------------------------------------------------
void CMainFrame::adjustSplitter ()
{
	if (_SplitterCreated)
	{
		RECT r;
		GetClientRect (&r);
		if (r.right-r.left > 380)
		{
			if (_Mode == 0) // Mode Zone
				m_wndSplitter.SetColumnInfo (0, r.right-r.left-380, 100); // 380 really experimental value
			
			if (_Mode == 1) // Mode Logic
				m_wndSplitter.SetColumnInfo (0, r.right-r.left-300, 10);
		}
		m_wndSplitter.RecalcLayout ();
	}
}
