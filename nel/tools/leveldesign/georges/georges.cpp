// Georges.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Georges.h"

#include "MainFrm.h"
#include "GeorgesDoc.h"
#include "GeorgesView.h"
#include "childfrm.h"

#include "../georges_lib/formbodyelt.h"
#include "../georges_lib/formbodyeltatom.h"
#include "../georges_lib/formbodyeltlist.h"
#include "../georges_lib/formbodyeltstruct.h"


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
	_MultiDocTemplate = NULL;
	sxrootdirectory = "U:\\";
	sxworkdirectory = "U:\\";
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGeorgesApp object

/////////////////////////////////////////////////////////////////////////////
// CGeorgesApp initialization

BOOL CGeorgesApp::initInstance(int x, int y, int cx, int cy)
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
/*
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
*/

	if (_MultiDocTemplate == NULL)
	{
		/*NLMISC_REGISTER_CLASS( CFormBodyElt );
		NLMISC_REGISTER_CLASS( CFormBodyEltAtom );
		NLMISC_REGISTER_CLASS( CFormBodyEltList );
		NLMISC_REGISTER_CLASS( CFormBodyEltStruct );*/
		// Change the registry key under which our settings are stored.
		// TODO: You should modify this string to be something appropriate
		// such as the name of your company or organization.
		SetRegistryKey(_T("Local AppWizard-Generated Applications"));

		LoadStdProfileSettings(16);  // Load standard INI file options (including MRU)

		// Register the application's document templates.  Document templates
		//  serve as the connection between documents, frame windows and views.

		_MultiDocTemplate = new CMultiDocTemplate(
			IDR_MAINFRAME,
			RUNTIME_CLASS(CGeorgesDoc),
			RUNTIME_CLASS(CChildFrame),
			RUNTIME_CLASS(CGeorgesView));
		AddDocTemplate(_MultiDocTemplate);

		// Enable DDE Execute open
		EnableShellOpen();
		RegisterShellFileTypes(TRUE);
	}

	m_pMainWnd = new CMainFrame();
	((CMainFrame*)m_pMainWnd)->CreateX = x;
	((CMainFrame*)m_pMainWnd)->CreateY = y;
	((CMainFrame*)m_pMainWnd)->CreateCX = cx;
	((CMainFrame*)m_pMainWnd)->CreateCY = cy;
	((CMainFrame*)m_pMainWnd)->LoadFrame (IDR_MAINFRAME);

/*
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
*/
	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	NLMISC::CPath::removeAllAlternativeSearchPath();
					
    NLMISC::CPath::addSearchPath( sxworkdirectory, true, true );
    NLMISC::CPath::addSearchPath( sxrootdirectory, true, true );

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

void CGeorgesApp::SetWorkDirectory( const CStringEx _sxworkdirectory )
{
	if( sxworkdirectory != _sxworkdirectory )
	{
		sxworkdirectory = _sxworkdirectory;
		NLMISC::CPath::removeAllAlternativeSearchPath();
		NLMISC::CPath::addSearchPath( sxworkdirectory, true, true );
		NLMISC::CPath::addSearchPath( sxrootdirectory, true, true );
	}
}

void CGeorgesApp::SetRootDirectory( const CStringEx _sxrootdirectory )
{
	if( sxrootdirectory != _sxrootdirectory )
	{
		sxrootdirectory = _sxrootdirectory;
		NLMISC::CPath::removeAllAlternativeSearchPath();
		NLMISC::CPath::addSearchPath( sxworkdirectory, true, true );
		NLMISC::CPath::addSearchPath( sxrootdirectory, true, true );
	}
}

CStringEx CGeorgesApp::GetWorkDirectory() const
{
	return( sxworkdirectory );
}

CStringEx CGeorgesApp::GetRootDirectory() const
{
	return( sxrootdirectory );
}

void CGeorgesApp::SaveAllDocument()
{
}

void CGeorgesApp::CloseAllDocument()
{
}

/////////////////////////////////////////////////////////////////////////////
// CGeorgesApp message handlers

