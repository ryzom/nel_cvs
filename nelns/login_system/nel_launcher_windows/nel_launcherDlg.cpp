// nel_launcherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nel_launcher.h"
#include "nel_launcherDlg.h"
#include <string>
#include <vector>
#include <process.h>
#include <direct.h>
#include "mshtml.h"

#include "nel/misc/config_file.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;

CConfigFile ConfigFile;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char *PleaseWaitFilename = "pleasewait.html";
string PleaseWaitFullPath;


/////////////////////////////////////////////////////////////////////////////
// CNel_launcherDlg dialog

CNel_launcherDlg::CNel_launcherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNel_launcherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNel_launcherDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNel_launcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNel_launcherDlg)
	DDX_Control(pDX, IDC_EXPLORER1, m_explore);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNel_launcherDlg, CDialog)
	//{{AFX_MSG_MAP(CNel_launcherDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNel_launcherDlg message handlers

BOOL CNel_launcherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//CWebBrowser2 m_browser - member variable  

	// load the pleasewait html page if available
	PleaseWaitFullPath = CPath::getFullPath (PleaseWaitFilename, false);
	if (NLMISC::CFile::isExists (PleaseWaitFullPath))
		m_explore.Navigate(PleaseWaitFullPath.c_str(), NULL, NULL, NULL, NULL);

	ConfigFile.load ("nel_launcher.cfg");

	m_explore.Navigate(ConfigFile.getVar ("StartupPage").asString().c_str(), NULL, NULL, NULL, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNel_launcherDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNel_launcherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BEGIN_EVENTSINK_MAP(CNel_launcherDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(CNel_launcherDlg)
	ON_EVENT(CNel_launcherDlg, IDC_EXPLORER1, 250 /* BeforeNavigate2 */, OnBeforeNavigate2Explorer1, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CNel_launcherDlg, IDC_EXPLORER1, 259 /* DocumentComplete */, OnDocumentCompleteExplorer1, VTS_DISPATCH VTS_PVARIANT)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNel_launcherDlg::OnBeforeNavigate2Explorer1(LPDISPATCH pDisp, VARIANT FAR* URL, VARIANT FAR* Flags, VARIANT FAR* TargetFrameName, VARIANT FAR* PostData, VARIANT FAR* Headers, BOOL FAR* Cancel) 
{
/*	CString cstr = URL->bstrVal;
	string str = string((const char*)cstr);

	if (str.find("?nel_quit=1") != string::npos)	
	{
		exit(0);
	}

	string token ("nel_exe=");

	int spos = str.find (token);
	if (spos == string::npos) return;

	int spos2 = str.find ("&", spos+token.size ()+1);
	if (spos == string::npos) return;

	string path;
	string exe = str.substr (spos+token.size (), spos2-spos-token.size ());

	CConfigFile::CVar *var = ConfigFile.getVarPtr (exe);
	if (var == NULL)
	{
		char str[1024];
		smprintf (str, 1024, "Don't know the executable filename for the application '%s'", exe.c_str ());
		MessageBox (str, "nel_launcher error");
		exit(0);
	}
	else
	{
		exe = var->asString (0);
		path = var->asString (1);
	}

	token = "nel_args=";
	spos = str.find (token);
	if (spos == string::npos) return;

	string rawargs = str.substr (spos+token.size ());

	// decode arguments (%20 -> space)

	vector<string> vargs;
	const char *args[50];
	int argspos = 0;

	int pos1 = 0, pos2 = 0;
	while (true)
	{
		pos2 = rawargs.find("%20", pos1);
		if (pos2==string::npos)
		{
			if(pos1!=rawargs.size())
			{
				string res = rawargs.substr (pos1);
				vargs.push_back (res);
			}
			break;
		}
		if(pos1 != pos2)
		{
			string res = rawargs.substr (pos1, pos2-pos1);
			vargs.push_back (res);
		}
		pos1 = pos2+3;
	}

	int i;
	int size;
	if(vargs.size()>47) size = 47;
	else size = vargs.size();

	args[0] = exe.c_str ();
	for(i = 0; i < size; i++)
	{
		args[i+1] = vargs[i].c_str ();
	}
	args[i+1] = NULL;


	// execute, should better use CreateProcess()

	_chdir (path.c_str());
	_execvp (exe.c_str(), args);
	exit(0);
*/	
/*	string cmd;

	cmd=exe+" "+rawargs;
	while (true)
	{
		int pos = cmd.find ("%20");
		if (pos == string::npos)
			break;

		cmd.replace (pos, 3, " ");
	}

	WinExec (cmd.c_str(), SW_SHOWNORMAL);
*/
}

void CNel_launcherDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if (m_explore.m_hWnd == NULL) return;
	CRect rect;
	GetClientRect (&rect);	
	m_explore.SetWindowPos ((CWnd* )HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
}

void CNel_launcherDlg::OnDocumentCompleteExplorer1(LPDISPATCH pDisp, VARIANT FAR* URL) 
{
	IHTMLDocument2* pHTMLDocument2;
	LPDISPATCH lpDispatch;
	lpDispatch = m_explore.GetDocument();

    if (lpDispatch)
	{
		HRESULT hr;
		hr = lpDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID*) &pHTMLDocument2);
		lpDispatch->Release();

		if (FAILED(hr))
			return;

		if (pHTMLDocument2 == NULL)
			return;

		IHTMLElement* pBody;
		hr = pHTMLDocument2->get_body(&pBody);

		if (FAILED(hr))
			return;

		if (pBody == NULL)
			return;

		BSTR bstr;                
		pBody->get_innerHTML(&bstr);
		CString csourceCode( bstr );
		string str( (LPCSTR)csourceCode );

		SysFreeString(bstr);
		pBody->Release();


		// now I have the web page, look if there's something interesting in it.

		// if something start with <!--nel it s cool
		if (str.find ("<!--nel") == string::npos) return;

		string token ("nel_exe=");
		int spos = str.find (token);
		if (spos == string::npos) return;

		int spos2 = str.find (" ", spos+token.size ());
		if (spos == string::npos) return;

		string path;
		string exe = str.substr (spos+token.size (), spos2-spos-token.size ());

		CConfigFile::CVar *var = ConfigFile.getVarPtr (exe);
		if (var == NULL)
		{
			char str[1024];
			smprintf (str, 1024, "Don't know the executable filename for the application '%s'", exe.c_str ());
			MessageBox (str, "nel_launcher error");
			exit(0);
		}
		else
		{
			exe = var->asString (0);
			path = var->asString (1);
		}

		token = "nel_args=";
		spos = str.find (token);
		if (spos == string::npos) return;

		spos2 = str.find ("-->", spos+token.size ()+1);
		if (spos == string::npos) return;

		string rawargs = str.substr (spos+token.size (), spos2-spos-token.size ());

		// convert the command line in an array of string for the _execvp() function

		vector<string> vargs;
		const char *args[50];
		int argspos = 0;

		uint pos1 = 0, pos2 = 0;
		while (true)
		{
			pos2 = rawargs.find(" ", pos1);
			if (pos2==string::npos)
			{
				if(pos1!=rawargs.size())
				{
					string res = rawargs.substr (pos1);
					vargs.push_back (res);
				}
				break;
			}
			if(pos1 != pos2)
			{
				string res = rawargs.substr (pos1, pos2-pos1);
				vargs.push_back (res);
			}
			pos1 = pos2+1;
		}

		int i;
		int size;
		if(vargs.size()>47) size = 47;
		else size = vargs.size();

		args[0] = exe.c_str ();
		for(i = 0; i < size; i++)
		{
			args[i+1] = vargs[i].c_str ();
		}
		args[i+1] = NULL;


		// execute, should better use CreateProcess()

		_chdir (path.c_str());
		_execvp (exe.c_str(), args);
		exit(0);
	}
}
