/*// WorldEditor_Exe.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "WorldEditor_Exe.h"

#include "../worldeditor/worldeditor_interface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp

BEGIN_MESSAGE_MAP(CWorldEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CWorldEditorApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp construction

CWorldEditorApp::CWorldEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWorldEditorApp object

CWorldEditorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp initialization

BOOL CWorldEditorApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));


	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	IWorldEditor *pWorldEditor = IWorldEditor::getInterface();

	pWorldEditor->initUI();

	CWnd *pMainFrame = (CWnd*)(pWorldEditor->getMainFrame());

	m_pMainWnd = pMainFrame;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp message handlers


*/






// object_viewer_exe.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../worldeditor/worldeditor_interface.h"
#include <string>

using namespace std;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// Remove command line ""
	char commandLine[512];
	char *commandLinePtr=commandLine;
	strcpy (commandLine, lpCmdLine);
	if (commandLine[0]=='"')
		commandLinePtr++;
	if (commandLinePtr[strlen (commandLinePtr)-1]=='"')
		commandLinePtr[strlen (commandLinePtr)-1]=0;

	char sCurDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, sCurDir);
	string sRootDir = string(sCurDir) + "\\";

 	// Create a object viewer
	IWorldEditor *pWorldEditor=IWorldEditor::getInterface();

	if (pWorldEditor)
	{
		pWorldEditor->setRootDir (sRootDir.c_str());
		// Init ui
		pWorldEditor->initUI ();

		// Go
		pWorldEditor->go ();

		// Release ui
		pWorldEditor->releaseUI ();

		// Delete the pointer
		IWorldEditor::releaseInterface (pWorldEditor);
	}


	return 0;
}
