// georges.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "georges.h"

#include "MainFrm.h"
#include "georgesDoc.h"
#include "georgesView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGeorgesApp

BEGIN_MESSAGE_MAP(CGeorgesApp, CWinApp)
	//{{AFX_MSG_MAP(CGeorgesApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeorgesApp construction

CGeorgesApp::CGeorgesApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	_Initialized = false;
	_DocTemplate = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesApp initialization

BOOL CGeorgesApp::initInstance(int x, int y, int cx, int cy)
{
	AfxEnableControlContainer();

	if (!_Initialized)
	{
		_Initialized = true;
		LoadStdProfileSettings();  // Load standard INI file options (including MRU)

		// Register the application's document templates.  Document templates
		//  serve as the connection between documents, frame windows and views.

		
		_DocTemplate = new CSingleDocTemplate(
			IDR_MAINFRAME,
			RUNTIME_CLASS(CGeorgesDoc),
			RUNTIME_CLASS(CMainFrame),       // main SDI frame window
			RUNTIME_CLASS(CGeorgesView));
		AddDocTemplate(_DocTemplate);
	}

	CMainFrame* pMainFrame = new CMainFrame;
	pMainFrame->CreateX = x;
	pMainFrame->CreateY = y;
	pMainFrame->CreateCX = cx;
	pMainFrame->CreateCY = cy;
	CCreateContext context;
	context.m_pCurrentDoc = _DocTemplate->CreateNewDocument();
	context.m_pCurrentFrame = NULL;
	context.m_pLastView = NULL;
	context.m_pNewDocTemplate = _DocTemplate;
	context.m_pNewViewClass = RUNTIME_CLASS(CGeorgesView);

	if (!pMainFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
								NULL, &context))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CGeorgesApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesApp message handlers

