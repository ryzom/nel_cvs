// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "master.h"

#include "MainFrm.h"
#include "NewRegion.h"
#include "ChooseTag.h"

#include "/code/nel/tools/3d/ligo/worldeditor/worldeditor_interface.h"

#include "nel/misc/file.h"
#include "nel/misc/stream.h"

using namespace NLMISC;
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
CEnvironnement::CEnvironnement()
{
	MasterX				= 0;
	MasterY				= 0;
	MasterTreeX			= 0;
	MasterTreeY			= 0;
	MasterTreeCX		= 100;
	MasterTreeCY		= 100;
	MasterTreeLocked	= true;
	RootDir				= ".\\";

	WorldEdOpened = false;
	WorldEdX = 50;
	WorldEdY = 50;
	WorldEdCX = 300;
	WorldEdCY = 300;
}

// ---------------------------------------------------------------------------
void CEnvironnement::serial (NLMISC::IStream& s)
{
	sint version = s.serialVersion (0);
	s.serial (MasterX);
	s.serial (MasterY);
	s.serial (MasterTreeX);
	s.serial (MasterTreeY);
	s.serial (MasterTreeCX);
	s.serial (MasterTreeCY);
	s.serial (MasterTreeLocked);
	s.serial (RootDir);

	s.serial (WorldEdOpened);
	s.serial (WorldEdX);
	s.serial (WorldEdY);
	s.serial (WorldEdCX);
	s.serial (WorldEdCY);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame
/////////////////////////////////////////////////////////////////////////////

//IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_COMMAND(ID_REGION_NEW, onRegionNew)
	ON_COMMAND(ID_REGION_SAVE, onRegionSave)
	ON_COMMAND(ID_REGION_EMPTY_TRASH, onRegionEmptyTrash)
	ON_COMMAND(ID_OPTIONS_TREELOCK, onOptionsTreeLock)
	ON_COMMAND(ID_OPTIONS_SETROOT, onOptionsSetRoot)
	ON_COMMAND(ID_WINDOWS_WORLDEDITOR, onWindowsWorldEditor)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

// ---------------------------------------------------------------------------
CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	_WorldEditor = NULL;
	_Tree = NULL;
}

// ---------------------------------------------------------------------------
CMainFrame::~CMainFrame()
{
}

// ---------------------------------------------------------------------------
// Used by getAllInterfaces method to retrieve all DLLs and tools Interfaces

typedef IWorldEditor* (*IWORLDEDITOR_GETINTERFACE)(int version); 
const char *IWORLDEDITOR_GETINTERFACE_NAME = "IWorldEditorGetInterface";
typedef void (*IWORLDEDITOR_RELINTERFACE)(IWorldEditor*pWE); 
const char *IWORLDEDITOR_RELINTERFACE_NAME = "IWorldEditorReleaseInterface";

// ---------------------------------------------------------------------------
void CMainFrame::getAllInterfaces()
{
	if (_WorldEditor == NULL)
	{
		IWORLDEDITOR_GETINTERFACE IWEGetInterface = NULL;

		#ifdef NL_RELEASE_DEBUG
			_WorldEditorModule = AfxLoadLibrary ("WorldEditor_release_debug.dll");
		#endif
		#ifdef NL_DEBUG_FAST
			_WorldEditorModule = AfxLoadLibrary ("WorldEditor_debug_fast.dll");
		#endif
		IWEGetInterface = (IWORLDEDITOR_GETINTERFACE)::GetProcAddress (_WorldEditorModule, IWORLDEDITOR_GETINTERFACE_NAME);
		if (IWEGetInterface != NULL)
			_WorldEditor = IWEGetInterface (WORLDEDITOR_VERSION);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::releaseAllInterfaces()
{
	if (_WorldEditor != NULL)
	{
		IWORLDEDITOR_RELINTERFACE IWERelInterface = NULL;
		IWERelInterface = (IWORLDEDITOR_RELINTERFACE)::GetProcAddress (_WorldEditorModule, IWORLDEDITOR_RELINTERFACE_NAME);
		IWERelInterface (_WorldEditor);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::openWorldEditor()
{
	if (_Environnement.WorldEdOpened)
		return;
	_Environnement.WorldEdOpened = true;
	if (_WorldEditor != NULL)
		_WorldEditor->initUILight (_Environnement.WorldEdX, _Environnement.WorldEdY, 
								_Environnement.WorldEdCX, _Environnement.WorldEdCY);
	GetMenu()->CheckMenuItem (ID_WINDOWS_WORLDEDITOR, MF_CHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::openWorldEditorFile(const char *fileName)
{
	_WorldEditor->loadFile (fileName);
}

// ---------------------------------------------------------------------------
void CMainFrame::closeWorldEditor()
{
	onRegionSave();
	if (!_Environnement.WorldEdOpened)
		return;
	_Environnement.WorldEdOpened = false;
	RECT r;
	CFrameWnd *pFW = (CFrameWnd*)_WorldEditor->getMainFrame();
	pFW->GetWindowRect(&r);
	_Environnement.WorldEdY = r.top;
	_Environnement.WorldEdX = r.left;
	_Environnement.WorldEdCY = r.bottom - r.top;
	_Environnement.WorldEdCX = r.right - r.left;
	_WorldEditor->releaseUI ();
	GetMenu()->CheckMenuItem (ID_WINDOWS_WORLDEDITOR, MF_UNCHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::updateTree()
{
	_Tree->update (_Environnement.RootDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::deltree (const std::string &dirName)
{
	// Get all directory object
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	SetCurrentDirectory (dirName.c_str());
	hFind = FindFirstFile ("*.*", &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		// Look if the name is a system directory
		if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				// If this is a directory deltree annd delete the directory
				deltree (findData.cFileName);
				RemoveDirectory (findData.cFileName);
			}
			else
			{
				// If this is a file delete
				DeleteFile (findData.cFileName);
			}
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::emptyTrash ()
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	SetCurrentDirectory (_Environnement.RootDir.c_str());

	if (!SetCurrentDirectory("Trash"))
	{
		SetCurrentDirectory (sCurDir);
		return;
	}
	SetCurrentDirectory (_Environnement.RootDir.c_str());
	deltree ("Trash");
	SetCurrentDirectory (sCurDir);

	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::cleanBackup ()
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	CChooseTag chooseTag(this);
	chooseTag.setPath (_Environnement.RootDir + "Backup");
	if (chooseTag.DoModal() == IDOK)
	{
		const char *sTag = chooseTag.getSelected();
		// Parse the directory and all files delete all directories older than 
		// the date of the selected tag
		string sTmp = _Environnement.RootDir;
		sTmp += "Backup";
		SetCurrentDirectory (sTmp.c_str());
		if (sTag != NULL)
		{
			WIN32_FIND_DATA findData;
			FILETIME timeSelWrite = { 0xFFFFFFFF, 0xFFFFFFFF };
			sTmp = sTag;
			sTmp += "*";
  			HANDLE hFind = FindFirstFile (sTmp.c_str(), &findData);

			// Get the oldest time in the tag selection
			while (hFind != INVALID_HANDLE_VALUE)
			{
				if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
				{
					if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					{
						// Get the tag if any
						if ((findData.ftLastWriteTime.dwHighDateTime < timeSelWrite.dwHighDateTime) ||
							(
								(findData.ftLastWriteTime.dwHighDateTime == timeSelWrite.dwHighDateTime) &&
								(findData.ftLastWriteTime.dwLowDateTime < timeSelWrite.dwLowDateTime)
							))
						{
							timeSelWrite = findData.ftLastWriteTime;
						}
					}
				}
				if (FindNextFile (hFind, &findData) == 0)
					break;
			}
			FindClose (hFind);

			hFind = FindFirstFile ("*.*", &findData);
			while (hFind != INVALID_HANDLE_VALUE)
			{
				if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
				{
					if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					{
						FILETIME timeCreate, timeLastAccess, timeWrite;
						GetFileTime (hFind, &timeCreate, &timeLastAccess, &timeWrite);

						if ((findData.ftLastWriteTime.dwHighDateTime < timeSelWrite.dwHighDateTime) ||
							(
								(findData.ftLastWriteTime.dwHighDateTime == timeSelWrite.dwHighDateTime) &&
								(findData.ftLastWriteTime.dwLowDateTime < timeSelWrite.dwLowDateTime)
							))
						{
							deltree (findData.cFileName);
							RemoveDirectory (findData.cFileName);
						}
					}
				}
				if (FindNextFile (hFind, &findData) == 0)
					break;
			}
			FindClose (hFind);
		}
	}
	SetCurrentDirectory (sCurDir);
	updateTree ();
}

// ---------------------------------------------------------------------------
int getMaxTag(const std::string &dirName)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	SetCurrentDirectory (dirName.c_str());
	hFind = FindFirstFile ("_T*", &findData);	
	int ret;
	while (hFind != INVALID_HANDLE_VALUE)
	{
		int nb =	(findData.cFileName[2]-'0')*100 + 
					(findData.cFileName[3]-'0')*10 + 
					(findData.cFileName[4]-'0')*1;
		if (nb > ret)
			ret = nb;
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	SetCurrentDirectory (sCurDir);
	return ret;
}

// ---------------------------------------------------------------------------
void copyTree (const string &srcDir, const string &dstDir)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	string tmp = srcDir + "\\*";
	hFind = FindFirstFile (tmp.c_str(), &findData);	
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				string newDirSrc = srcDir + "\\" + findData.cFileName;
				string newDirDst = dstDir + "\\" + findData.cFileName;
				CreateDirectory(newDirDst.c_str(), NULL);
				copyTree (newDirSrc, newDirDst);
			}
			else
			{
				string fnameSrc = srcDir + "\\" + findData.cFileName;
				string fnameDst = dstDir + "\\" + findData.cFileName;
				CopyFile(fnameSrc.c_str(), fnameDst.c_str(), false);
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
}

// ---------------------------------------------------------------------------
void CMainFrame::regionBackupAll ()
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	string sRegDir = _Environnement.RootDir + "Regions";
	string sBackDir = _Environnement.RootDir + "Backup";
	int nb = getMaxTag(sBackDir);
	nb++;
	string sTag = "_T";
	sTag += toString((nb/100)%10);
	sTag += toString((nb/10) %10);
	sTag += toString((nb/1)  %10);
	sTag += "_";

	SetCurrentDirectory (sRegDir.c_str());
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	hFind = FindFirstFile ("*", &findData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				string srcDir = sRegDir + "\\" + findData.cFileName;
				string dstDir = sBackDir + "\\" + sTag + findData.cFileName;
				CreateDirectory (dstDir.c_str(), NULL);
				copyTree (srcDir, dstDir);
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	SetCurrentDirectory (sCurDir);
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::backupRestoreAll ()
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	string sRegDir = _Environnement.RootDir + "Regions";
	string sBackDir = _Environnement.RootDir + "Backup";
	CChooseTag chooseTag(this);
	chooseTag.setPath (sBackDir);
	if (chooseTag.DoModal() == IDOK)
	{
		const char *sTag = chooseTag.getSelected();
		// Parse the directory and all files delete all directories older than 
		// the date of the selected tag
		SetCurrentDirectory (sBackDir.c_str());
		if (sTag != NULL)
		{
			WIN32_FIND_DATA findData;
			string sTmp = sBackDir + "\\" + sTag + "*";
  			HANDLE hFind = FindFirstFile (sTmp.c_str(), &findData);

			// Delete all regions to replace them by backup stuff
			if (hFind != INVALID_HANDLE_VALUE)
			{
				sTmp = _Environnement.RootDir;
				sTmp += "Regions";
				deltree (sTmp);
			}

			while (hFind != INVALID_HANDLE_VALUE)
			{
				if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
				{
					if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
					{
						string srcDir = sBackDir + "\\" + findData.cFileName;
						string dstDir = sRegDir + "\\" + (&findData.cFileName[6]);
						CreateDirectory (dstDir.c_str(), NULL);
						copyTree (srcDir, dstDir);
					}
				}

				if (FindNextFile (hFind, &findData) == 0)
					break;
			}
			FindClose (hFind);
		}
	}
	SetCurrentDirectory (sCurDir);
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnSize (UINT nType, int cx, int cy)
{
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMove (int x, int y)
{
	RECT r;
	GetWindowRect (&r);
	_Environnement.MasterY = r.top;
	_Environnement.MasterX = r.left;
	if ((_Tree != NULL) && (_Environnement.MasterTreeLocked))
	{
		_Tree->SetWindowPos (&wndTop, x, y, 0, 0, SWP_NOSIZE);
		SetFocus();
	}
}

// ---------------------------------------------------------------------------
int CMainFrame::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	// Create the menu
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the tree
	_Tree = new CMasterTreeDlg ();
	_Tree->Create (IDD_MASTERTREE, this);

	// Load tools and get interface
	getAllInterfaces ();

	// Restore all (windows position and size, menu checked, etc...)
	_Tree->SetWindowPos (&wndTop, _Environnement.MasterTreeX, _Environnement.MasterTreeY, 
						_Environnement.MasterTreeCX, _Environnement.MasterTreeCY, SWP_SHOWWINDOW);
	_Tree->ShowWindow (SW_SHOW);
	_Tree->update (_Environnement.RootDir);
	if (_Environnement.MasterTreeLocked)
		GetMenu()->CheckMenuItem (ID_OPTIONS_TREELOCK, MF_CHECKED);
	if (_WorldEditor != NULL)
		_WorldEditor->setRootDir (_Environnement.RootDir.c_str());
	if (_Environnement.WorldEdOpened)
	{ // Force the WorldEditor to open
		_Environnement.WorldEdOpened = false;
		openWorldEditor ();
	}

	return 0;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::PreCreateWindow (CREATESTRUCT& cs)
{
	// Load the config file
	try
	{
		CIFile fIn;
		fIn.open ("master.cfg");
		_Environnement.serial (fIn);
	}
	catch(Exception&e)
	{
		MessageBox (e.what(),"Warning");
	}

	// Restore the master window position
	cs.x = _Environnement.MasterX;
	cs.y = _Environnement.MasterY;

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	
	cs.style = WS_OVERLAPPED;	
	cs.cx = 360;
	cs.cy = 45;

	return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::OnEraseBkgnd (CDC* pDC)
{
	return true;
}

// ---------------------------------------------------------------------------
void CMainFrame::createDirIfNotExist (const string& dirName, const string& errorMsg)
{
	char sCurDir[MAX_PATH];
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = true;
	GetCurrentDirectory (MAX_PATH, sCurDir);
	if (!SetCurrentDirectory (dirName.c_str()))
	{
		if (!CreateDirectory (dirName.c_str(), &sa))
			MessageBox (errorMsg.c_str(), "Error", MB_ICONERROR|MB_OK);
	}
	SetCurrentDirectory (sCurDir);
}
// ---------------------------------------------------------------------------
void CMainFrame::onRegionNew ()
{
	// Ask for a region name
	NewRegion newRegDial(this);
	if (newRegDial.DoModal() == IDOK)
	{
		// Create directory and template class
		string newDirName = (LPCSTR)newRegDial.str;
		char sCurDir[MAX_PATH];
		GetCurrentDirectory (MAX_PATH, sCurDir);
		SetCurrentDirectory (_Environnement.RootDir.c_str());

		createDirIfNotExist ("Regions", "Cannot create Regions system directory");
		createDirIfNotExist ("Backup", "Cannot create Backup system directory");
		createDirIfNotExist ("Trash", "Cannot create Trash system directory");

		SetCurrentDirectory ("Regions");

		if (SetCurrentDirectory (newDirName.c_str()))
		{
			MessageBox ("Region already exists.", "Error", MB_ICONERROR|MB_OK);
			SetCurrentDirectory (sCurDir);
			return;
		}

		createDirIfNotExist (newDirName, "Cannot create a region.");

		// Ok so now create default files
		SetCurrentDirectory (newDirName.c_str());
		_WorldEditor->createDefaultFiles (newDirName.c_str());

		_Tree->update (_Environnement.RootDir);
		SetCurrentDirectory (sCurDir);
	}
}

// ---------------------------------------------------------------------------
// Save all in all modules
void CMainFrame::onRegionSave ()
{
	if (_Environnement.WorldEdOpened)
	{
		if (_WorldEditor)
			_WorldEditor->saveOpenedFiles();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::onRegionEmptyTrash ()
{
	emptyTrash ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onOptionsTreeLock ()
{
	_Environnement.MasterTreeLocked = !_Environnement.MasterTreeLocked;
	if (_Environnement.MasterTreeLocked)
	{
		GetMenu()->CheckMenuItem (ID_OPTIONS_TREELOCK, MF_CHECKED);
		_Tree->SetWindowPos (&wndTop, _Environnement.MasterX, _Environnement.MasterY+45, 0, 0, SWP_NOSIZE);
		SetFocus();
	}
	else
		GetMenu()->CheckMenuItem (ID_OPTIONS_TREELOCK, MF_UNCHECKED);
}

// ---------------------------------------------------------------------------
int CALLBACK BrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	switch(uMsg) 
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
void CMainFrame::onOptionsSetRoot ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the root path";
	bi.ulFlags = 0;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)_Environnement.RootDir.c_str();
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	_Environnement.RootDir = str;
	_Environnement.RootDir += "\\";
	_Tree->update (_Environnement.RootDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::onWindowsWorldEditor ()
{
	if (!_Environnement.WorldEdOpened)
		openWorldEditor ();
	else
		closeWorldEditor ();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnClose ()
{
	RECT r;

	// Master Tree saves
	_Tree->GetWindowRect (&r);
	_Environnement.MasterTreeX = r.top;
	_Environnement.MasterTreeY = r.left;
	_Environnement.MasterTreeCX = r.right-r.left;
	_Environnement.MasterTreeCY = r.bottom-r.top;

	// WorldEditor saves
	if (_Environnement.WorldEdOpened)
	{
		CFrameWnd *pFW = (CFrameWnd*)_WorldEditor->getMainFrame();
		pFW->GetWindowRect(&r);
		_Environnement.WorldEdY = r.top;
		_Environnement.WorldEdX = r.left;
		_Environnement.WorldEdCY = r.bottom - r.top;
		_Environnement.WorldEdCX = r.right - r.left;
	}

	// Save the environnement
	try
	{
		COFile fOut;
		fOut.open ("master.cfg");
		_Environnement.serial (fOut);
	}
	catch(Exception&e)
	{
		MessageBox (e.what(), "Error", MB_ICONERROR|MB_OK);
	}
	
	DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

