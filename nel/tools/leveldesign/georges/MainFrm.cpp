// ---------------------------------------------------------------------------
// MainFrm.cpp : implementation of the CMainFrame class
// ---------------------------------------------------------------------------

#include "stdafx.h"
#include "Georges.h"
#include "Georgesdoc.h"
#include "DfnDoc.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ---------------------------------------------------------------------------
IMPLEMENT_DYNCREATE (CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP (CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE ()
	ON_WM_CLOSE ()
	ON_COMMAND (ID_FILE_DIR_DFNTYP, OnFileDirDfnTyp)
	ON_COMMAND (ID_FILE_DIR_PROTOTYPE, OnFileDirPrototype)
	ON_COMMAND (ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND (ID_EDIT_REDO, OnEditRedo)
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// ---------------------------------------------------------------------------
CMainFrame::CMainFrame ()
{
	CreateX = CreateY = CreateCX = CreateCY = 0;
	Exit = false;
}

// ---------------------------------------------------------------------------
CMainFrame::~CMainFrame ()
{
}

// ---------------------------------------------------------------------------
int CMainFrame::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableDocking (CBRS_ALIGN_ANY);
	EnableDocking (CBRS_ALIGN_ANY);
	DockControlBar (&m_wndToolBar);

	return 0;
}

// ---------------------------------------------------------------------------
void CMainFrame::DoClose()
{
	CMDIFrameWnd::OnClose (); // Parse all documents asking to save if changed
}

// ---------------------------------------------------------------------------
void CMainFrame::OnClose()
{
	Exit = true;
}


// ---------------------------------------------------------------------------
BOOL CMainFrame::PreCreateWindow (CREATESTRUCT& cs)
{
	// We are launched by the master tool with a predefined size and position
	if ((CreateCX != 0)&&(CreateCY != 0))
	{
		cs.x = CreateX;
		cs.y = CreateY;
		cs.cx = CreateCX;
		cs.cy = CreateCY;
		// cs.style ~= WS_SYSMENU;
	}

	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

// ---------------------------------------------------------------------------
// Undo an action over a Georges or Dfn document
// The accelerators do all the keyboard link stuff with a pretranslate msg
void CMainFrame::OnEditUndo() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMDIChildWnd *pChild = MDIGetActive();
	if (pChild == NULL)
		return;
	CGeorgesDoc *pGeorgesDoc = dynamic_cast<CGeorgesDoc*>(pChild->GetActiveDocument());
	if (pGeorgesDoc != NULL)
	{
		pGeorgesDoc->Undo();
		return;
	}

	CDfnDoc *pDfnDoc = dynamic_cast<CDfnDoc*>(pChild->GetActiveDocument());
	if (pDfnDoc != NULL)
	{
		pDfnDoc->Undo();
		return;
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnEditRedo() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMDIChildWnd *pChild = MDIGetActive();
	if (pChild == NULL)
		return;
	CGeorgesDoc *pGeorgesDoc = dynamic_cast<CGeorgesDoc*>(pChild->GetActiveDocument());
	if (pGeorgesDoc != NULL)
	{
		pGeorgesDoc->Redo();
		return;
	}

	CDfnDoc *pDfnDoc = dynamic_cast<CDfnDoc*>(pChild->GetActiveDocument());
	if (pDfnDoc != NULL)
	{
		pDfnDoc->Redo();
		return;
	}
}

// ---------------------------------------------------------------------------
// This is just a function to process the initialisation event of the directoryBrowser
int CALLBACK dataDirBrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	switch (uMsg) 
	{
		case BFFM_INITIALIZED: 
			SendMessage (hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
		default:
		break;
	}
	return 0;
}

// ---------------------------------------------------------------------------
void CMainFrame::OnFileDirDfnTyp ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path for DFN";
	bi.ulFlags = 0;
	bi.lpfn = dataDirBrowseCallbackProc;

	// Keep an instance of the dfntyp directory locally
	char sDir[512];
	strcpy (sDir, ((CGeorgesApp*)AfxGetApp())->GetDirDfnTyp().c_str());
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	try 
	{
		pidl = SHBrowseForFolder (&bi);
		if (!SHGetPathFromIDList(pidl, str)) 
		{
			return;
		}
	}
	catch(exception &e)
	{
		MessageBox (e.what(), "Error",MB_ICONERROR|MB_OK);
		return;
	}
	((CGeorgesApp*)AfxGetApp())->SetDirDfnTyp (str);
	((CGeorgesApp*)AfxGetApp())->SaveCfg ();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnFileDirPrototype ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path for Prototypes";
	bi.ulFlags = 0;
	bi.lpfn = dataDirBrowseCallbackProc;
	
	char sDir[512];
	strcpy (sDir, ((CGeorgesApp*)AfxGetApp())->GetDirPrototype().c_str());
	bi.lParam = (LPARAM)sDir;
	
	bi.iImage = 0;
	try
	{
		pidl = SHBrowseForFolder (&bi);
		if (!SHGetPathFromIDList(pidl, str)) 
		{
			return;
		}
	}
	catch(exception &e)
	{
		MessageBox (e.what(), "Error",MB_ICONERROR|MB_OK);
		return;
	}
	((CGeorgesApp*)AfxGetApp())->SetDirPrototype (str);
	((CGeorgesApp*)AfxGetApp())->SaveCfg ();
}
