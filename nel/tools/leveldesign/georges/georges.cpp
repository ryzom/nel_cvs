// ---------------------------------------------------------------------------
// Georges.cpp : Defines the class behaviors for the application.
// ---------------------------------------------------------------------------

#include "stdafx.h"
#include "Georges.h"

#include "MainFrm.h"

#include "georgesView.h"
#include "georgesFrame.h"
#include "georgesDoc.h"

#include "dfnView.h"
#include "dfnFrame.h"
#include "dfnDoc.h"

#include "../georges_lib/formbodyelt.h"
#include "../georges_lib/formbodyeltatom.h"
#include "../georges_lib/formbodyeltlist.h"
#include "../georges_lib/formbodyeltstruct.h"

#include "nel/misc/file.h"

// ---------------------------------------------------------------------------

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ---------------------------------------------------------------------------
// SEnvironnement
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
SEnvironnement::SEnvironnement()
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	DirDfnTyp = sCurDir;
	DirDfnTyp += "\\";
	DirPrototype = DirDfnTyp;
	DirLevel = DirDfnTyp;
}

// ---------------------------------------------------------------------------
void SEnvironnement::serial (NLMISC::IStream&f)
{
	int version = f.serialVersion (0);

	f.serial (DirDfnTyp);
	f.serial (DirPrototype);
	f.serial (DirLevel);
}

// ---------------------------------------------------------------------------
// CGeorgesApp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP (CGeorgesApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND (ID_FILE_OPEN_DFN, OpenDfn)
	ON_COMMAND (ID_FILE_NEW_DFN, NewDfn)
	ON_COMMAND (ID_FILE_NEW_INSTANCE, NewInstance)
	ON_COMMAND (ID_FILE_OPEN_INSTANCE, OpenInstance)
END_MESSAGE_MAP ()

// ---------------------------------------------------------------------------
CGeorgesApp::CGeorgesApp()
{
	_MultiDocTemplate = NULL;
	_MultiDocTemplateDfn = NULL;

	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	DirExe = sCurDir;
	DirExe += "\\";

	// Load the .CFG
	LoadCfg();
}

// ---------------------------------------------------------------------------
BOOL CGeorgesApp::initInstance(int x, int y, int cx, int cy)
{
	if (_MultiDocTemplate == NULL)
	{
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
			RUNTIME_CLASS(CGeorgesFrame),
			RUNTIME_CLASS(CGeorgesView));
		AddDocTemplate(_MultiDocTemplate);

		_MultiDocTemplateDfn = new CMultiDocTemplate(
			IDR_TYPE_DFN,
			RUNTIME_CLASS(CDfnDoc),
			RUNTIME_CLASS(CDfnFrame),
			RUNTIME_CLASS(CDfnView));
		AddDocTemplate(_MultiDocTemplateDfn);
	}

	m_pMainWnd = new CMainFrame();
	((CMainFrame*)m_pMainWnd)->CreateX = x;
	((CMainFrame*)m_pMainWnd)->CreateY = y;
	((CMainFrame*)m_pMainWnd)->CreateCX = cx;
	((CMainFrame*)m_pMainWnd)->CreateCY = cy;
	((CMainFrame*)m_pMainWnd)->LoadFrame (IDR_MAINFRAME);

	// Enable DDE Execute open
	EnableShellOpen ();
	RegisterShellFileTypes (TRUE);

	// The one and only window has been initialized, so show and update it.
	m_pMainWnd->ShowWindow (SW_SHOW);
	m_pMainWnd->UpdateWindow ();

	NLMISC::CPath::removeAllAlternativeSearchPath ();
	NLMISC::CPath::addSearchPath (Env.DirLevel, true, true);
    NLMISC::CPath::addSearchPath (Env.DirPrototype, true, true);
    NLMISC::CPath::addSearchPath (Env.DirLevel, true, true);

	return TRUE;
}

// ---------------------------------------------------------------------------
void CGeorgesApp::LoadCfg ()
{
	try
	{
		NLMISC::CIFile fileIn;
		std::string sGeorgesCfg = DirExe;
		sGeorgesCfg += "Georges.cfg";
		fileIn.open (sGeorgesCfg.c_str());
		fileIn.serial (Env);
	}
	catch (NLMISC::Exception& )
	{
		// Cannot display anything here so init with a default Environnement
		SEnvironnement newEnv;
		Env = newEnv;
	}
}

// ---------------------------------------------------------------------------
void CGeorgesApp::SaveCfg ()
{
	try
	{
		NLMISC::COFile fileOut;
		std::string sGeorgesCfg = DirExe;
		sGeorgesCfg += "Georges.cfg";
		fileOut.open (sGeorgesCfg.c_str());
		fileOut.serial (Env);
	}
	catch (NLMISC::Exception& )
	{
		// Not succeeded ... But that's not a bug deal
	}
}


// ---------------------------------------------------------------------------
// CAboutDlg
// ---------------------------------------------------------------------------
class CAboutDlg : public CDialog
{
public:
	enum { IDD = IDD_ABOUTBOX };

	CAboutDlg () : CDialog (CAboutDlg::IDD)
	{
	}


protected:
	virtual void DoDataExchange (CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
	}

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP (CAboutDlg, CDialog)
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
void CGeorgesApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// ---------------------------------------------------------------------------
CStringEx CGeorgesApp::GetDirDfnTyp () const
{
	return Env.DirDfnTyp;
}

// ---------------------------------------------------------------------------
CStringEx CGeorgesApp::GetDirPrototype () const
{
	return Env.DirPrototype;
}

// ---------------------------------------------------------------------------
CStringEx CGeorgesApp::GetDirLevel () const
{
	return Env.DirLevel;
}

// ---------------------------------------------------------------------------
void CGeorgesApp::SetDirLevel (const CStringEx &_sxDirectory)
{
	Env.DirLevel = _sxDirectory;
	NLMISC::CPath::removeAllAlternativeSearchPath ();
	NLMISC::CPath::addSearchPath (Env.DirLevel, true, true);
	NLMISC::CPath::addSearchPath (Env.DirPrototype, true, true);
	NLMISC::CPath::addSearchPath (Env.DirLevel, true, true);
}

// ---------------------------------------------------------------------------
void CGeorgesApp::SetDirPrototype (const CStringEx &_sxDirectory)
{
	Env.DirPrototype = _sxDirectory;
	NLMISC::CPath::removeAllAlternativeSearchPath ();
	NLMISC::CPath::addSearchPath (Env.DirLevel, true, true);
	NLMISC::CPath::addSearchPath (Env.DirPrototype, true, true);
	NLMISC::CPath::addSearchPath (Env.DirLevel, true, true);
}

// ---------------------------------------------------------------------------
void CGeorgesApp::SetDirDfnTyp (const CStringEx &_sxDirectory)
{
	Env.DirDfnTyp = _sxDirectory;
	NLMISC::CPath::removeAllAlternativeSearchPath ();
	NLMISC::CPath::addSearchPath (Env.DirLevel, true, true);
	NLMISC::CPath::addSearchPath (Env.DirPrototype, true, true);
	NLMISC::CPath::addSearchPath (Env.DirDfnTyp, true, true);
}

// ---------------------------------------------------------------------------
void CGeorgesApp::SaveAllDocument()
{
	if( !_MultiDocTemplate )
		return;
	POSITION pos = _MultiDocTemplate->GetFirstDocPosition();
	if( !pos )
		return;
	CDocument* pdoc = _MultiDocTemplate->GetNextDoc(pos);
	while( pdoc )
	{
		CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( pdoc );
		pgdoc->DoFileSave();
		if( !pos )
			return;
		pdoc = _MultiDocTemplate->GetNextDoc(pos);
	}
}

// ---------------------------------------------------------------------------
void CGeorgesApp::CloseAllDocument()
{
}

// ---------------------------------------------------------------------------
void CGeorgesApp::UpdateAllDocument()
{
	if (_MultiDocTemplate == NULL)
		return;

	POSITION pos = _MultiDocTemplate->GetFirstDocPosition();
	if( !pos )
		return;
	CDocument* pdoc = _MultiDocTemplate->GetNextDoc(pos);
	while( pdoc )
	{
		CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( pdoc );
		pgdoc->UpdateDocument();
		if( !pos )
			return;
		pdoc = _MultiDocTemplate->GetNextDoc(pos);
	}
}

// ---------------------------------------------------------------------------
void CGeorgesApp::OpenDfn ()
{
	CFileDialog Dlg (true);
	int s = Dlg.m_ofn.Flags;
	Dlg.m_ofn.Flags |= OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;
	Dlg.m_ofn.lpstrTitle  = "Opening a DFN file";
	Dlg.m_ofn.lpstrFilter = "Define files (*.typ;*.dfn)|*.typ;*.dfn";
	Dlg.m_ofn.lpstrInitialDir = Env.DirDfnTyp.c_str ();

	if (Dlg.DoModal() != IDOK )
		return;

	CString fn = Dlg.GetPathName ();

	try
	{
		if (Dlg.GetFileExt() == "typ")
			_MultiDocTemplate->OpenDocumentFile (fn);
		else
			_MultiDocTemplateDfn->OpenDocumentFile (fn);
	}
	catch (NLMISC::Exception &e)
	{
		m_pMainWnd->MessageBox (e.what(), "Georges_Lib", MB_ICONERROR|MB_OK);
	}
}

// ---------------------------------------------------------------------------
void CGeorgesApp::OpenInstance ()
{
	CFileDialog Dlg (true);
	int s = Dlg.m_ofn.Flags;
	Dlg.m_ofn.Flags |= OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;
	Dlg.m_ofn.lpstrTitle  = "Opening an Instance file";
	Dlg.m_ofn.lpstrFilter = "Instances files (*.*)\0*.*";
	Dlg.m_ofn.lpstrInitialDir = Env.DirPrototype.c_str ();

	if (Dlg.DoModal() != IDOK )
		return;

	CString fn = Dlg.GetPathName ();

	try
	{
		_MultiDocTemplate->OpenDocumentFile (fn);
	}
	catch (NLMISC::Exception &e)
	{
		m_pMainWnd->MessageBox (e.what(), "Georges_Lib", MB_ICONERROR|MB_OK);
	}
}

// ---------------------------------------------------------------------------
void CGeorgesApp::NewDfn ()
{
	try
	{
		_MultiDocTemplateDfn->OpenDocumentFile (NULL);
	}
	catch (NLMISC::Exception &e)
	{
		m_pMainWnd->MessageBox (e.what(), "Georges_Lib", MB_ICONERROR|MB_OK);
	}
}

// ---------------------------------------------------------------------------
void CGeorgesApp::NewInstance ()
{
	try
	{
		_MultiDocTemplate->OpenDocumentFile (NULL);
	}
	catch (NLMISC::Exception &e)
	{
		m_pMainWnd->MessageBox (e.what(), "Georges_Lib", MB_ICONERROR|MB_OK);
	}
}

