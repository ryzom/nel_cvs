// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "master.h"

#include "MainFrm.h"
#include "NewRegion.h"
#include "NewGeorgesFormDlg.h"
#include "PrimNameDlg.h"
#include "ChooseTag.h"
#include "ChooseDir.h"
#include "exportdlg.h"
#include "exportcbdlg.h"

#include "/code/nel/tools/leveldesign/georges/georges_interface.h"
#include "/code/nel/tools/leveldesign/logic_editor/logic_editor_interface.h"

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
SEnvironnement::SEnvironnement()
{
	MasterX				= 0;
	MasterY				= 0;
	MasterTreeX			= 0;
	MasterTreeY			= 0;
	MasterTreeCX		= 100;
	MasterTreeCY		= 100;
	MasterTreeLocked	= true;

	char tmp[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, tmp);
	RootDir				= tmp;
	RootDir += "\\";

	WorldEdOpened = false;
	WorldEdX = 50;
	WorldEdY = 50;
	WorldEdCX = 600;
	WorldEdCY = 400;

	GeorgesOpened = false;
	GeorgesX = 50;
	GeorgesY = 50;
	GeorgesCX = 300;
	GeorgesCY = 300;

	LogicEditorOpened = false;
	LogicEditorX = 50;
	LogicEditorY = 50;
	LogicEditorCX = 300;
	LogicEditorCY = 300;
}

// ---------------------------------------------------------------------------
void SEnvironnement::serial (NLMISC::IStream& s)
{
	sint version = s.serialVersion (1);

	if (version > 0)
		s.serial (ExportOptions);

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
 
	s.serial (GeorgesOpened);
	s.serial (GeorgesX);
	s.serial (GeorgesY);
	s.serial (GeorgesCX);
	s.serial (GeorgesCY);

	s.serial (LogicEditorOpened);
	s.serial (LogicEditorX);
	s.serial (LogicEditorY);
	s.serial (LogicEditorCX);
	s.serial (LogicEditorCY);
}

/////////////////////////////////////////////////////////////////////////////
// CMasterCB
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
CMasterCB::CMasterCB ()
{
	_MainFrame = NULL;
}

// ---------------------------------------------------------------------------
void CMasterCB::setMainFrame (CMainFrame*pMF)
{
	_MainFrame = pMF;
}

// ---------------------------------------------------------------------------
vector<string> &CMasterCB::getAllPrimZoneNames ()
{
	return _PrimZoneList;
}

// ---------------------------------------------------------------------------
void CMasterCB::setAllPrimZoneNames (vector<string> &primZoneList)
{
	_PrimZoneList = primZoneList;
	_MainFrame->georgesUpdatePatatoid();
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
	ON_COMMAND(ID_REGION_EXPORT, onRegionExport)

	ON_COMMAND(ID_REGION_EMPTY_TRASH, onRegionEmptyTrash)
	ON_COMMAND(ID_REGION_EMPTY_BACKUP, onRegionEmptyBackup)
	ON_COMMAND(ID_REGION_REGION_BACKUP_ALL, onRegionBackupAll)
	ON_COMMAND(ID_REGION_TRASH_REGIONS, onRegionTrashRegions)
	ON_COMMAND(ID_REGION_RESTORE_TAG, onRegionRestoreTag)

	ON_COMMAND(ID_REGION_DELETE_IN_TRASH, OnRegionDeleteInTrash)
	ON_COMMAND(ID_REGION_DELETE_IN_BACKUP, OnRegionDeleteInBackup)
	ON_COMMAND(ID_REGION_DELETE, OnRegionDelete)
	ON_COMMAND(ID_REGION_RESTORE_FROM_TRASH, OnRegionRestoreFromTrash)
	ON_COMMAND(ID_REGION_RESTORE_FROM_BACKUP, OnRegionRestoreFromBackup)
	ON_COMMAND(ID_REGION_BACKUP, OnRegionBackup)

	ON_COMMAND(ID_OPTIONS_TREELOCK, onOptionsTreeLock)
	ON_COMMAND(ID_OPTIONS_SETROOT, onOptionsSetRoot)

	ON_COMMAND(ID_WINDOWS_WORLDEDITOR, onWindowsWorldEditor)
	ON_COMMAND(ID_WINDOWS_GEORGES, onWindowsGeorges)
	ON_COMMAND(ID_WINDOWS_LOGICEDITOR, onWindowsLogicEditor)

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
	_WorldEditor = NULL;
	_Georges = NULL;
	_LogicEditor = NULL;
	_Tree = NULL;
	_Export = NULL;
}

// ---------------------------------------------------------------------------
CMainFrame::~CMainFrame()
{
	delete _Export;
}

// ---------------------------------------------------------------------------
// Used by getAllInterfaces method to retrieve all DLLs and tools Interfaces

typedef IWorldEditor* (*IWORLDEDITOR_GETINTERFACE)(int version);
const char *IWORLDEDITOR_GETINTERFACE_NAME = "IWorldEditorGetInterface";
typedef void (*IWORLDEDITOR_RELINTERFACE)(IWorldEditor*pWE);
const char *IWORLDEDITOR_RELINTERFACE_NAME = "IWorldEditorReleaseInterface";

typedef IGeorges* (*IGEORGES_GETINTERFACE)(int version);
const char *IGEORGES_GETINTERFACE_NAME = "IGeorgesGetInterface";
typedef void (*IGEORGES_RELINTERFACE)(IGeorges *pGeorges);
const char *IGEORGES_RELINTERFACE_NAME = "IGeorgesReleaseInterface";

typedef ILogicEditor* (*ILOGICEDITOR_GETINTERFACE)(int version);
const char *ILOGICEDITOR_GETINTERFACE_NAME = "ILogicEditorGetInterface";
typedef void (*ILOGICEDITOR_RELINTERFACE)(ILogicEditor *pLogicEditor);
const char *ILOGICEDITOR_RELINTERFACE_NAME = "ILogicEditorReleaseInterface";

// ---------------------------------------------------------------------------
void CMainFrame::getAllInterfaces()
{
	// Get WorldEditor Interface
	if (_WorldEditor == NULL)
	{
		IWORLDEDITOR_GETINTERFACE IWEGetInterface = NULL;

		#ifdef NL_RELEASE_DEBUG
			_WorldEditorModule = AfxLoadLibrary ("WorldEditor_release_debug.dll");
		#endif
		#ifdef NL_DEBUG_FAST
			_WorldEditorModule = AfxLoadLibrary ("WorldEditor_debug_fast.dll");
		#endif
		if (_WorldEditorModule != NULL)
		{
			IWEGetInterface = (IWORLDEDITOR_GETINTERFACE)::GetProcAddress (_WorldEditorModule, IWORLDEDITOR_GETINTERFACE_NAME);
			if (IWEGetInterface != NULL)
				_WorldEditor = IWEGetInterface (WORLDEDITOR_VERSION);
		}
		else
		{
			MessageBox("WorldEditor dll not Loaded", "Warning");
		}
	}

	// Get Georges Interface
	if (_Georges == NULL)
	{
		IGEORGES_GETINTERFACE IGGetInterface = NULL;

		#ifdef NL_RELEASE_DEBUG
			_GeorgesModule = AfxLoadLibrary ("Georges_release_debug.dll");
		#endif
		#ifdef NL_DEBUG_FAST
			_GeorgesModule = AfxLoadLibrary ("Georges_debug_fast.dll");
		#endif
		if (_GeorgesModule != NULL)
		{
			IGGetInterface = (IGEORGES_GETINTERFACE)::GetProcAddress (_GeorgesModule, IGEORGES_GETINTERFACE_NAME);
			if (IGGetInterface != NULL)
				_Georges = IGGetInterface (GEORGES_VERSION);
		}
		else
		{
			MessageBox("Georges dll not Loaded", "Warning");
		}
	}

	// Get LogicEditor Interface
	if (_LogicEditor == NULL)
	{
		ILOGICEDITOR_GETINTERFACE ILEGetInterface = NULL;

		#ifdef NL_RELEASE_DEBUG
			_LogicEditorModule = AfxLoadLibrary ("Logic_Editor_rd.dll");
		#endif
		#ifdef NL_DEBUG_FAST
			_LogicEditorModule = AfxLoadLibrary ("Logic_Editor_df.dll");
		#endif
		if (_LogicEditorModule != NULL)
		{
			ILEGetInterface = (ILOGICEDITOR_GETINTERFACE)::GetProcAddress (_LogicEditorModule, ILOGICEDITOR_GETINTERFACE_NAME);
			if (ILEGetInterface != NULL)
				_LogicEditor = ILEGetInterface (LOGIC_EDITOR_VERSION);
		}
		else
		{
			MessageBox("LogicEditor dll not Loaded", "Warning");
		}
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::releaseAllInterfaces()
{
	// Release the WorldEditor interface
	if (_WorldEditor != NULL)
	{
		IWORLDEDITOR_RELINTERFACE IWERelInterface = NULL;
		IWERelInterface = (IWORLDEDITOR_RELINTERFACE)::GetProcAddress (_WorldEditorModule, IWORLDEDITOR_RELINTERFACE_NAME);
		IWERelInterface (_WorldEditor);
	}

	// Release the Georges interface
	if (_Georges != NULL)
	{
		IGEORGES_RELINTERFACE IGRelInterface = NULL;
		IGRelInterface = (IGEORGES_RELINTERFACE)::GetProcAddress (_GeorgesModule, IGEORGES_RELINTERFACE_NAME);
		IGRelInterface (_Georges);
	}

	// Release the LogicEditor interface
	if (_LogicEditor != NULL)
	{
		ILOGICEDITOR_RELINTERFACE ILERelInterface = NULL;
		ILERelInterface = (ILOGICEDITOR_RELINTERFACE)::GetProcAddress (_LogicEditorModule, ILOGICEDITOR_RELINTERFACE_NAME);
		ILERelInterface (_LogicEditor);
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
	//onRegionSave ();
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
void CMainFrame::openGeorges ()
{
	if (_Environnement.GeorgesOpened)
		return;
	_Environnement.GeorgesOpened = true;
	if (_Georges != NULL)
	{
		_Georges->initUILight (_Environnement.GeorgesX, _Environnement.GeorgesY, 
								_Environnement.GeorgesCX, _Environnement.GeorgesCY);
	}
	GetMenu()->CheckMenuItem (ID_WINDOWS_GEORGES, MF_CHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::openGeorgesFile (const char *fileName)
{
	_Georges->SetWorkDirectory (_Environnement.RootDir + _ActiveRegionPath + "\\" + _ActiveRegion);
	_Georges->LoadDocument (fileName);
}

// ---------------------------------------------------------------------------
void CMainFrame::closeGeorges ()
{
	if (!_Environnement.GeorgesOpened)
		return;
	_Environnement.GeorgesOpened = false;
	RECT r;
	CFrameWnd *pFW = (CFrameWnd*)_Georges->getMainFrame();
	pFW->GetWindowRect(&r);
	_Environnement.GeorgesY = r.top;
	_Environnement.GeorgesX = r.left;
	_Environnement.GeorgesCY = r.bottom - r.top;
	_Environnement.GeorgesCX = r.right - r.left;
	_Georges->releaseUI ();
	GetMenu()->CheckMenuItem (ID_WINDOWS_GEORGES, MF_UNCHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::georgesUpdatePatatoid ()
{
	if (_Georges == NULL)
		return;
	_Georges->SetWorkDirectory (_Environnement.RootDir + _ActiveRegionPath + "\\" + _ActiveRegion);
	_Georges->SetTypPredef ("patat_name.typ", _MasterCB.getAllPrimZoneNames());
}

// ---------------------------------------------------------------------------
void CMainFrame::georgesCreateFilesWhenNewRegion ()
{
	// When we arrive in this function the current directory is the new region being created
	if (_Georges == NULL)
		return;

	CreateDirectory ("dfn", NULL);
	char curdir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curdir);
	string fullname = string(curdir) + "\\dfn\\patat_name.typ";
	vector< pair< string, string > > lpsx;
	vector< pair< string, string > > lpsx2;
	lpsx.push_back (make_pair(string("PatatFrite"),	string("PatatFrite")));
	lpsx.push_back (make_pair(string("PatatVapeur"), string("PatatVapeur")));
	_Georges->MakeTyp (fullname, "string", "PATAT", "true", "", "", "PatatFrite", &lpsx, &lpsx2);
}

// ---------------------------------------------------------------------------
void CMainFrame::georgesCreatePlantName ()
{
	WIN32_FIND_DATA fdTmp;
	char curdir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curdir);

	string plantname = _Environnement.RootDir + "common\\dfn\\";
	if (!SetCurrentDirectory(plantname.c_str()))
		return;

	plantname += "plant_name.typ";

	// If plantname file do not already exists
	HANDLE hFind = FindFirstFile(plantname.c_str(), &fdTmp);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose (hFind);
		if (!DeleteFile (plantname.c_str()))
		{
			MessageBox (plantname.c_str(), "Cannot overwrite");
			return;
		}
	}
	
	vector< pair< string, string > > lpsx;
	vector< pair< string, string > > lpsx2;
	lpsx.push_back (std::make_pair(string("xxx.plant"),	string("xxx.plant")));
	lpsx.push_back (std::make_pair(string("yyy.plant"), string("yyy.plant")));
 	_Georges->MakeTyp (plantname, "string", "PLANT", "true", "", "", "xxx.plant", &lpsx, &lpsx2);

	// Parse the plant directory and add all these predef
	string plantdir = _Environnement.RootDir + "common";
	SetCurrentDirectory (plantdir.c_str());

	vector<string> allPlants;
	CExport::getAllFiles (".plant", allPlants);

	for (uint32 i = 0; i < allPlants.size(); ++i)
	{
		char fName[_MAX_FNAME];
		char ext[_MAX_FNAME];
		::_splitpath((const char*)allPlants[i].c_str(), NULL, NULL, fName, ext) ;						
		allPlants[i] = string(fName) + string(ext);
	}
	_Georges->SetTypPredef ("plant_name.typ", allPlants);

	SetCurrentDirectory (curdir);
}

// ---------------------------------------------------------------------------
void CMainFrame::openLogicEditor ()
{
	if (_Environnement.LogicEditorOpened)
		return;
	_Environnement.LogicEditorOpened = true;
	if (_LogicEditor != NULL)
		_LogicEditor->initUILight (_Environnement.LogicEditorX, _Environnement.LogicEditorY, 
								_Environnement.LogicEditorCX, _Environnement.LogicEditorCY);
	GetMenu()->CheckMenuItem (ID_WINDOWS_LOGICEDITOR, MF_CHECKED);
}

// ---------------------------------------------------------------------------
void CMainFrame::openLogicEditorFile (const char *fileName)
{
	_LogicEditor->loadFile (fileName);
}

// ---------------------------------------------------------------------------
void CMainFrame::closeLogicEditor ()
{
	if (!_Environnement.LogicEditorOpened)
		return;
	_Environnement.LogicEditorOpened = false;
	RECT r;
	CFrameWnd *pFW = (CFrameWnd*)_LogicEditor->getMainFrame();
	pFW->GetWindowRect(&r);
	_Environnement.LogicEditorY = r.top;
	_Environnement.LogicEditorX = r.left;
	_Environnement.LogicEditorCY = r.bottom - r.top;
	_Environnement.LogicEditorCX = r.right - r.left;
	_LogicEditor->releaseUI ();
	GetMenu()->CheckMenuItem (ID_WINDOWS_LOGICEDITOR, MF_UNCHECKED);
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
	if (!SetCurrentDirectory (dirName.c_str()))
		return;
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
void CMainFrame::emptyBackup ()
{
	if (MessageBox("Do you really want to empty backup folder?", "Take care", MB_OKCANCEL|MB_ICONSTOP) == IDOK)
	{
		char sCurDir[MAX_PATH];
		GetCurrentDirectory (MAX_PATH, sCurDir);
		SetCurrentDirectory (_Environnement.RootDir.c_str());

		if (!SetCurrentDirectory("Backup"))
		{
			SetCurrentDirectory (sCurDir);
			return;
		}
		SetCurrentDirectory (_Environnement.RootDir.c_str());
		deltree ("Backup");
		SetCurrentDirectory (sCurDir);

		updateTree ();
	}
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
int getMaxTag(const std::string &dirName, const std::string &tagName)
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	SetCurrentDirectory (dirName.c_str());
	string tmp = tagName + "*";
	hFind = FindFirstFile (tmp.c_str(), &findData);	
	int ret = 0;
	while (hFind != INVALID_HANDLE_VALUE)
	{
		int nb =	(findData.cFileName[tagName.size()+0]-'0')*100 + 
					(findData.cFileName[tagName.size()+1]-'0')*10 + 
					(findData.cFileName[tagName.size()+2]-'0')*1;
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
	int nb = getMaxTag(sBackDir, "_T");
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
void CMainFrame::regionTrashAll ()
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	string sRegDir = _Environnement.RootDir + "Regions";
	string sTrashDir = _Environnement.RootDir + "Trash";
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
				string dstDir = sTrashDir + "\\" + findData.cFileName;
				deltree (dstDir);
				CreateDirectory (dstDir.c_str(), NULL);
				copyTree (srcDir, dstDir);
			}
		}

		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);
	deltree (sRegDir);
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
void CMainFrame::trashDelete		(const char *str)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	string sDir = _Environnement.RootDir + "Trash";
	SetCurrentDirectory (sDir.c_str());
	deltree (str);
	RemoveDirectory (str);
	SetCurrentDirectory (sCurDir);
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::trashRestoreOne	(const char *str)
{
	string srcDir = _Environnement.RootDir + "Trash\\" + str;
	string dstDir = _Environnement.RootDir + "Regions\\" + str;
	deltree (dstDir);
	CreateDirectory (dstDir.c_str(), NULL);
	copyTree (srcDir, dstDir);
	deltree (srcDir);
	RemoveDirectory (srcDir.c_str());
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::backupDelete		(const char *str)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	string sDir = _Environnement.RootDir + "Backup";
	SetCurrentDirectory (sDir.c_str());
	deltree (str);
	RemoveDirectory (str);
	SetCurrentDirectory (sCurDir);
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::backupRestoreOne	(const char *str)
{
	string srcDir = _Environnement.RootDir + "Backup\\" + str;
	string dstDir = _Environnement.RootDir + "Regions\\";
	if (str[0] == '_')
		dstDir += &str[6];
	else
		dstDir += &str[5];
	deltree (dstDir);
	CreateDirectory (dstDir.c_str(), NULL);
	copyTree (srcDir, dstDir);
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::regionDelete		(const char *str)
{
	string srcDir = _Environnement.RootDir + "Regions\\" + str;
	string dstDir = _Environnement.RootDir + "Trash\\" + str;
	deltree (dstDir);
	CreateDirectory (dstDir.c_str(), NULL);
	copyTree (srcDir, dstDir);
	deltree (srcDir);
	RemoveDirectory (srcDir.c_str());
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::regionBackupOne	(const char *str)
{
	string dstDir = _Environnement.RootDir + "Backup\\";
	int nMaxTag = getMaxTag(dstDir, "V")+1;
	dstDir += "V";
	dstDir += toString((nMaxTag/100)%10);
	dstDir += toString((nMaxTag/10) %10);
	dstDir += toString((nMaxTag/1)  %10);
	dstDir += "_";
	dstDir += str;

	string srcDir = _Environnement.RootDir + "Regions\\" + str;
	deltree (dstDir);
	CreateDirectory (dstDir.c_str(), NULL);
	copyTree (srcDir, dstDir);
	updateTree ();
}

// ---------------------------------------------------------------------------
void CMainFrame::regionNewPrim (const char *str)
{
	if (_WorldEditor != NULL)
	{
		// Ask the name to the user
		CPrimNameDlg dial;
		if (dial.DoModal() == IDOK)
		{
			string primname = dial.str;
			string sTmp = _Environnement.RootDir + "Regions\\" + str + "\\" + primname;
			_WorldEditor->createEmptyPrimFile (sTmp.c_str());
			updateTree ();
		}
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::regionNewGeorges (const char *str)
{
	if (_Georges == NULL)
		return;
	CNewGeorgesFormDlg dial;
	if (dial.DoModal() == IDOK)
	{
		// Ask the user the .dfn to use
		char curdir[MAX_PATH];
		GetCurrentDirectory (MAX_PATH, curdir);
		string newDir = _Environnement.RootDir + "common\\dfn";
		SetCurrentDirectory (newDir.c_str());

		CFileDialog fd (true, "*.dfn", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
						"Definition (*.dfn)|*.dfn", this);
		fd.m_ofn.lpstrInitialDir = newDir.c_str();
		if (fd.DoModal() == IDOK)
		{
			string sTmp = _Environnement.RootDir + "Regions\\" + str + "\\" + (LPCSTR)dial.str;
			_Georges->SetWorkDirectory (_Environnement.RootDir + "Regions\\" + str);
			_Georges->createInstanceFile (sTmp, (LPCSTR)fd.GetFileName());
		}

		SetCurrentDirectory (curdir);
		updateTree ();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::setActiveRegion (const std::string &Region, const std::string &Dir)
{
	_ActiveRegion = Region;
	_ActiveRegionPath = Dir;
	if (_Georges != NULL)
		_Georges->SetWorkDirectory (_Environnement.RootDir + _ActiveRegionPath + "\\" + _ActiveRegion);

	if (_Environnement.WorldEdOpened)
	{
		string sTmp = _Environnement.RootDir + _ActiveRegionPath + "\\" + _ActiveRegion + "\\" + _ActiveRegion + ".land";
		openWorldEditorFile (sTmp.c_str());
	}
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
	// TREE
	_Tree->SetWindowPos (&wndTop, _Environnement.MasterTreeX, _Environnement.MasterTreeY, 
						_Environnement.MasterTreeCX, _Environnement.MasterTreeCY, SWP_SHOWWINDOW);
	_Tree->ShowWindow (SW_SHOW);
	_Tree->update (_Environnement.RootDir);
	if (_Environnement.MasterTreeLocked)
		GetMenu()->CheckMenuItem (ID_OPTIONS_TREELOCK, MF_CHECKED);

	// WORLDEDITOR
	if (_WorldEditor != NULL)
	{
		_MasterCB.setMainFrame (this);
		_WorldEditor->setMasterCB (&_MasterCB);
		_WorldEditor->setRootDir (_Environnement.RootDir.c_str());
	}
	if (_Environnement.WorldEdOpened)
	{ 
		_Environnement.WorldEdOpened = false;
		openWorldEditor ();
	}

	// GEORGES
	if (_Georges != NULL)
	{
		_Georges->SetRootDirectory (_Environnement.RootDir + "common");
		_Georges->SetWorkDirectory (_Environnement.RootDir + "common\\dfn");
		georgesCreatePlantName ();		
	}
	if (_Environnement.GeorgesOpened)
	{ 
		_Environnement.GeorgesOpened = false;
		openGeorges ();
	}

	// LOGICEDITOR
//	if (_LogicEditor != NULL)
//		_LogicEditor->setRootDir (_Environnement.RootDir.c_str());
	if (_Environnement.LogicEditorOpened)
	{ 
		_Environnement.LogicEditorOpened = false;
		openLogicEditor ();
	}

	return 0;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::PreCreateWindow (CREATESTRUCT& cs)
{
	// Load the config file
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	_MasterExeDir = sCurDir;
	try
	{
		CIFile fIn;
		fIn.open ("master.cfg");
		_Environnement.serial (fIn);
	}
	catch (Exception&e)
	{
		MessageBox (e.what(), "Warning");
	}

	// Restore the master window position
	cs.x = _Environnement.MasterX;
	cs.y = _Environnement.MasterY;

	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	
	cs.style = WS_OVERLAPPED;	
	cs.cx = 360;
	cs.cy = GetSystemMetrics (SM_CYCAPTION) + 
			GetSystemMetrics (SM_CYMENU) + 
			GetSystemMetrics (SM_CYFRAME);

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
	CNewRegion newRegDial(this);
	if (newRegDial.DoModal() != IDOK)
		return;

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

	// Import the .land file
	CFileDialog fd (true, "*.land", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
					"Landscape (*.land)|*.land", this);

	int retFD = fd.DoModal();

	string newName = _Environnement.RootDir + "Regions\\" + newDirName + "\\" + newDirName + ".land";
	if (retFD == IDOK)
	{
		CopyFile (fd.GetFileName(), newName.c_str(), false);
	}

	SetCurrentDirectory (_Environnement.RootDir.c_str());
	SetCurrentDirectory ("Regions");
	SetCurrentDirectory (newDirName.c_str());

	// Ok so now create default files
	//_WorldEditor->createDefaultFiles (newDirName.c_str());
	georgesCreateFilesWhenNewRegion ();

	_Tree->update (_Environnement.RootDir);
	SetCurrentDirectory (sCurDir);
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
	if (_Environnement.GeorgesOpened)
	{
//		if (_Georges)
//			_Georges->saveOpenedFiles();
	}
	if (_Environnement.LogicEditorOpened)
	{
//		if (_LogicEditor)
//			_LogicEditor->saveOpenedFiles();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::onRegionExport ()
{
	onRegionSave ();

	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);
	string sTmp = _Environnement.RootDir + "Regions\\";
	SetCurrentDirectory (sTmp.c_str());

	vector<string> vRegNames;
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile ("*.*", &fd);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (!((strcmp (fd.cFileName, ".") == 0) || (strcmp (fd.cFileName, "..") == 0)))
		{
			if ((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				vRegNames.push_back (string(fd.cFileName));
			}
		}
		if (FindNextFile (hFind, &fd) == 0)
			break;
	}

	if (vRegNames.size() == 0)
		return;

	CExportDlg dlg (this);
	
	_Environnement.ExportOptions.SourceDir = _ActiveRegion;
	dlg.setOptions (_Environnement.ExportOptions, vRegNames);
	if (dlg.DoModal() == IDOK)
	{
		CExportCBDlg *pDlg = new CExportCBDlg();
		//Check if new succeeded and we got a valid pointer to a dialog object
		if (_Export == NULL)
			_Export = new CExport;
		if (pDlg != NULL)
		{
			BOOL ret = pDlg->Create (IDD_EXPORTCB, this);
			if (!ret)   //Create failed.
			{
				delete pDlg;
				pDlg = NULL;
			}
			pDlg->ShowWindow (SW_SHOW);
		}
   
		_Environnement.ExportOptions.SourceDir = _Environnement.RootDir + "Regions\\" + _Environnement.ExportOptions.SourceDir;
		_Environnement.ExportOptions.RootDir = _Environnement.RootDir + "Common\\";
		_Export->export (_Environnement.ExportOptions, pDlg->getExportCB());

		pDlg->setFinishedButton ();
		while (pDlg->getFinished () == false)
		{
			pDlg->pump ();
		}
		pDlg->DestroyWindow ();
	}
	SetCurrentDirectory (sCurDir);
}

// ---------------------------------------------------------------------------
void CMainFrame::onRegionEmptyTrash ()
{
	emptyTrash ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onRegionEmptyBackup ()
{
	emptyBackup ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onRegionBackupAll ()
{
	regionBackupAll ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onRegionTrashRegions ()
{
	regionTrashAll ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onRegionRestoreTag ()
{
	backupRestoreAll ();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnRegionDeleteInTrash ()
{
	CChooseDir chooseDir(this);
	chooseDir.setPath (_Environnement.RootDir + "Trash");
	if (chooseDir.DoModal() == IDOK)
	{
		trashDelete (chooseDir.getSelected());
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnRegionDeleteInBackup ()
{
	CChooseDir chooseDir(this);
	chooseDir.setPath (_Environnement.RootDir + "Backup");
	if (chooseDir.DoModal() == IDOK)
	{
		backupDelete (chooseDir.getSelected());
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnRegionDelete ()
{
	CChooseDir chooseDir(this);
	chooseDir.setPath (_Environnement.RootDir + "Regions");
	if (chooseDir.DoModal() == IDOK)
	{
		regionDelete (chooseDir.getSelected());
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnRegionRestoreFromTrash ()
{
	CChooseDir chooseDir(this);
	chooseDir.setPath (_Environnement.RootDir + "Trash");
	if (chooseDir.DoModal() == IDOK)
	{
		trashRestoreOne (chooseDir.getSelected());
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnRegionRestoreFromBackup ()
{
	CChooseDir chooseDir(this);
	chooseDir.setPath (_Environnement.RootDir + "Backup");
	if (chooseDir.DoModal() == IDOK)
	{
		backupRestoreOne (chooseDir.getSelected());
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnRegionBackup ()
{
	CChooseDir chooseDir(this);
	chooseDir.setPath (_Environnement.RootDir + "Regions");
	if (chooseDir.DoModal() == IDOK)
	{
		regionBackupOne (chooseDir.getSelected());
	}
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
void CMainFrame::onWindowsGeorges ()
{
	if (!_Environnement.GeorgesOpened)
		openGeorges ();
	else
		closeGeorges ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onWindowsLogicEditor ()
{
	if (!_Environnement.LogicEditorOpened)
		openLogicEditor ();
	else
		closeLogicEditor ();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnClose ()
{
	RECT r;

	// Master Tree saves
	_Tree->GetWindowRect (&r);
	_Environnement.MasterTreeX = r.left;
	_Environnement.MasterTreeY = r.top;
	_Environnement.MasterTreeCX = r.right-r.left;
	_Environnement.MasterTreeCY = r.bottom-r.top;

	// WorldEditor saves
	if (_Environnement.WorldEdOpened)
	{
		closeWorldEditor ();
		_Environnement.WorldEdOpened = true;
	}

	// Georges saves
	if (_Environnement.GeorgesOpened)
	{
		closeGeorges ();
		_Environnement.GeorgesOpened = true;
	}

	// LogicEditor saves
	if (_Environnement.LogicEditorOpened)
	{
		closeLogicEditor ();
		_Environnement.LogicEditorOpened = true;
	}

	// Save the environnement
	SetCurrentDirectory (_MasterExeDir.c_str());
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

	releaseAllInterfaces ();

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

