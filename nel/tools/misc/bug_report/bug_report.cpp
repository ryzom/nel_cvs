// bug_report.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "bug_report.h"
#include "bug_reportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CBug_reportApp

BEGIN_MESSAGE_MAP(CBug_reportApp, CWinApp)
	//{{AFX_MSG_MAP(CBug_reportApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBug_reportApp construction

CBug_reportApp::CBug_reportApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBug_reportApp object

CBug_reportApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBug_reportApp initialization

BOOL CBug_reportApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CBug_reportDlg dlg;
	m_pMainWnd = &dlg;

	char *cmd = GetCommandLine ();

	// skip the program name
	if (cmd[0] == '"')
		cmd = strchr (cmd+1, '"') + 1;

	while(true)
	{
		char *token1;
		static bool first = true;
		if (first)
		{
			token1 = strtok( cmd, " " );
			first = false;
		}
		else
			token1 = strtok( NULL, " " );

		if (token1 == NULL)
			break;

		char *token2 = strtok( NULL, " " );
		if (token2 == NULL)
			break;

		if (string(token1) == "ClientVersion")
			dlg.set (IDC_CLIENTVERSION, token2);
		else if (string(token1) == "ShardVersion")
			dlg.set (IDC_SHARDVERSION, token2);
		else if (string(token1) == "ShardName")
			dlg.set (IDC_SHARDNAME, token2);
		else if (string(token1) == "AttachedFile")
			dlg.set (IDC_ATTACHEDFILE, token2);
		else if (string(token1) == "AttachedFile")
			dlg.set (IDC_ATTACHEDFILE, token2);
		else if (string(token1) == "Language")
			dlg.set (IDC_LANGUAGE, token2);
		else if (string(token1) == "DumpFilename")
			dlg.DumpFilename = token2;
		else
			MessageBox (NULL, "Bad command line option", token1, MB_OK);
	}
	
	
	
	
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
