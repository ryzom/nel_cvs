// logic.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"

#undef LOGIC_EDITOR_EXPORT
#define LOGIC_EDITOR_EXPORT __declspec( dllexport ) 


#include "logic_editor.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "logic_editorDoc.h"
#include "logic_editorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorApp

BEGIN_MESSAGE_MAP(CLogic_editorApp, CWinApp)
	//{{AFX_MSG_MAP(CLogic_editorApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorApp construction

CLogic_editorApp::CLogic_editorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLogic_editorApp object

CLogic_editorApp theApp;



/////////////////////////////////////////////////////////////////////////////
// CLogic_editorApp initialization
//BOOL CLogic_editorApp::InitInstance()
BOOL CLogic_editorApp::initInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.


#ifdef _AFXDLL
	//Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	//Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif


	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_LOGIC_TYPE,
		RUNTIME_CLASS(CLogic_editorDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CLogic_editorView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	
	// The main window has been initialized, so show and update it.
	//pMainFrame->ShowWindow(m_nCmdShow);
	//pMainFrame->UpdateWindow();
	

	return TRUE;
}




//-----------------------------------------------
//	initInstanceLight
//
//-----------------------------------------------
BOOL CLogic_editorApp::initInstanceLight( int x, int y, int cx, int cy )
{
	AfxEnableControlContainer();
	
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	// Load standard INI file options (including MRU)
	LoadStdProfileSettings();  

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_LOGIC_TYPE,
		RUNTIME_CLASS(CLogic_editorDoc),
		RUNTIME_CLASS(CChildFrame),
		RUNTIME_CLASS(CLogic_editorView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	pMainFrame->createX = x;
	pMainFrame->createY = y;
	pMainFrame->createCX = cx;
	pMainFrame->createCY = cy;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;
	
	/*
	static_cast<CMainFrame*>(theApp.m_pMainWnd)->createX = x;
	static_cast<CMainFrame*>(theApp.m_pMainWnd)->createY = y;
	static_cast<CMainFrame*>(theApp.m_pMainWnd)->createCX = cx;
	static_cast<CMainFrame*>(theApp.m_pMainWnd)->createCY = cy;
	*/

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

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
/*void CLogic_editorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}*/

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorApp message handlers







//---------------------------------------------
//	CLogicEditor
//
//---------------------------------------------
CLogicEditor::CLogicEditor()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	
		
} // CLogicEditor //




//---------------------------------------------
//	initUI
//
//---------------------------------------------
void CLogicEditor::initUI( HWND parent )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	theApp.initInstance();
		
	// The main window has been initialized, so show and update it.
	theApp.m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
	theApp.m_pMainWnd->UpdateWindow();

} // initUI //



//---------------------------------------------
//	initUILight
//
//---------------------------------------------
void CLogicEditor::initUILight (int x, int y, int cx, int cy)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	theApp.initInstanceLight(x,y,cx,cy);
		
	// The main window has been initialized, so show and update it.
	theApp.m_pMainWnd->ShowWindow(SW_SHOW);
	theApp.m_pMainWnd->UpdateWindow();
	

} // initUILight //



//---------------------------------------------
//	Go
//
//---------------------------------------------
void CLogicEditor::go()
{
	do
	{
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	while (true);

} // go



//---------------------------------------------
//	releaseUI
//
//---------------------------------------------
void CLogicEditor::releaseUI()
{
		
} // releaseUI



//---------------------------------------------
//	getInterface
//
//---------------------------------------------
ILogicEditor * ILogicEditor::getInterface( int version )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Check version number
	if( version != LOGIC_EDITOR_VERSION )
	{
		MessageBox( NULL, "Bad version of logic_editor.dll.", "Logic Editor", MB_ICONEXCLAMATION|MB_OK);
		return NULL;
	}
	else
		return new CLogicEditor;

} // getInterface //



//---------------------------------------------
//	getMainFrame
//
//---------------------------------------------
void * CLogicEditor::getMainFrame ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (theApp.m_pMainWnd == NULL)
		initUI();
	
	return theApp.m_pMainWnd;

} // getMainFrame //



//---------------------------------------------
//	releaseInterface
//
//---------------------------------------------
void ILogicEditor::releaseInterface( ILogicEditor * logicEditor )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	delete logicEditor;

} // releaseInterface //


//---------------------------------------------
//	releaseInterface
//
//---------------------------------------------
ILogicEditor * ILogicEditorGetInterface( int version )
{
	return ILogicEditor::getInterface( version );

} // releaseInterface //


//---------------------------------------------
//	ILogicEditorReleaseInterface
//
//---------------------------------------------
void ILogicEditorReleaseInterface( ILogicEditor * pWE )
{
	ILogicEditor::releaseInterface( pWE );

} // ILogicEditorReleaseInterface //


