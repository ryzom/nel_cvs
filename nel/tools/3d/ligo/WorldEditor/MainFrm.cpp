// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"

#include <afxdlgs.h> // CFileDialog

#include "nel/misc/file.h"

#include "WorldEditor.h"
#include "display.h"
#include "toolsLogic.h"
#include "toolsZone.h"

#include "MainFrm.h"
#include "resource.h"

#include "generateDlg.h"
#include "TypeManagerDlg.h"
#include "moveDlg.h"

#include "exportDlg.h"
#include "exportCBDlg.h"
//#include "export.h"

#include <string>

using namespace NLLIGO;
using namespace std;
using namespace NLMISC;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CType

// ---------------------------------------------------------------------------
void SType::serial (NLMISC::IStream&f)
{
	int version = f.serialVersion(0);
	f.serial (Name);
	f.serial (Color);
}

/////////////////////////////////////////////////////////////////////////////
// CType

// ---------------------------------------------------------------------------
SEnvironnement::SEnvironnement ()
{
}

// ---------------------------------------------------------------------------
void SEnvironnement::serial (NLMISC::IStream&f)
{
	int version = f.serialVersion (1);

	f.serialCont (Types);
	f.serial (BackgroundColor);
	f.serial (ExportOptions);

	if (version > 0)
		f.serial (DataDir);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_MENUSELECT()
	ON_COMMAND(ID_FILE_NEWLOGIC, OnMenuFileNewLogic)
	ON_COMMAND(ID_FILE_UNLOADLOGIC, OnMenuFileUnloadLogic)
	ON_COMMAND(ID_FILE_OPENLOGIC, OnMenuFileOpenLogic)
	ON_COMMAND(ID_FILE_SAVELOGIC, OnMenuFileSaveLogic)
	ON_COMMAND(ID_FILE_NEW, OnMenuFileNewLandscape)
	ON_COMMAND(ID_FILE_UNLOAD, OnMenuFileUnloadLandscape)
	ON_COMMAND(ID_FILE_OPENLANDSCAPE, OnMenuFileOpenLandscape)
	ON_COMMAND(ID_FILE_SAVELANDSCAPE, OnMenuFileSaveLandscape)
	ON_COMMAND(ID_FILE_GENERATE, OnMenuFileGenerate)
	ON_COMMAND(ID_FILE_EXPORT, OnMenuFileExportToLevelD)
	ON_COMMAND(ID_FILE_VIEW, OnMenuFileView)
	ON_COMMAND(ID_FILE_SETDATADIR, OnMenuFileSetDataDirectory)
	ON_COMMAND(ID_FILE_EXIT, OnMenuFileExit)
	ON_COMMAND(ID_MODE_ZONE, OnMenuModeZone)
	ON_COMMAND(ID_MODE_TRANSITION, OnMenuModeTransition)
	ON_COMMAND(ID_MODE_LOGIC, OnMenuModeLogic)
	ON_COMMAND(ID_MODE_TYPE, OnMenuModeType)
	ON_COMMAND(ID_MODE_SELECT, onMenuModeSelectZone)
	ON_COMMAND(ID_MODE_UNDO, onMenuModeUndo)
	ON_COMMAND(ID_MODE_REDO, onMenuModeRedo)
	ON_COMMAND(ID_MODE_MOVE, onMenuModeMove)
	ON_COMMAND(ID_MODE_COUNT_ZONES, onMenuModeCountZones)
	ON_COMMAND(ID_VIEW_GRID, OnMenuViewGrid)
	ON_COMMAND(ID_VIEW_BACKGROUND, OnMenuViewBackground)

	ON_WM_CLOSE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction contexthelp

// ---------------------------------------------------------------------------
CMainFrame::CMainFrame()
{
	_Mode = 0;
	_SplitterCreated = false;
	CreateX = CreateY = CreateCX = CreateCY = 0;
	_MasterCB = NULL;
	char curDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, curDir);
	_ExeDir = string(curDir) + "\\";
	_DataDir = string(curDir) + "\\";
}

// ---------------------------------------------------------------------------
CMainFrame::~CMainFrame()
{
}

// ---------------------------------------------------------------------------
void CMainFrame::setExeDir (const char* str)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState());
	_ExeDir = str;
	if ((str[strlen(str)-1] != '\\') || (str[strlen(str)-1] != '/'))
		_ExeDir += "\\";
}

// ---------------------------------------------------------------------------
void CMainFrame::setDataDir (const char* str)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState());
	bool bPutZoneModeAtTheEnd = (_Mode == 0); // Mode Zone

	OnMenuModeLogic ();
	_DataDir = str;
	if ((str[strlen(str)-1] != '\\') && (str[strlen(str)-1] != '/'))
		_DataDir += "\\";
	_Environnement.DataDir = _DataDir;
	_ZoneBuilder.init (_DataDir, false);
	if (bPutZoneModeAtTheEnd)
		OnMenuModeZone ();
}

// ---------------------------------------------------------------------------
void CMainFrame::loadLand (const char* str, const char* path)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState());
	_ZoneBuilder.load (str, path);
	_ZoneBuilder.stackReset ();
	//OnMenuModeZone ();
}

// ---------------------------------------------------------------------------
void CMainFrame::loadPrim (const char* str, const char* path)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState());
	_PRegionBuilder.load (str, path);
	//OnMenuModeLogic ();
}

// ---------------------------------------------------------------------------
void CMainFrame::saveAll ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState());
	_ZoneBuilder.autoSaveAll ();
	_PRegionBuilder.autoSaveAll ();
}

// ---------------------------------------------------------------------------
int CMainFrame::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ((CreateCX == 0)&&(CreateCY == 0))
	{
		if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}
	}
	else // Mode light
	{
		if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_wndToolBar.LoadToolBar(IDR_MAINFRAME_LIGHT))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}
	}

	if (!m_wndStatusBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
									| CBRS_SIZE_DYNAMIC) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking (CBRS_ALIGN_ANY);
	EnableDocking (CBRS_ALIGN_ANY);
	DockControlBar (&m_wndToolBar);

	CDocument *pNewDoc = new CDocument;
	InitialUpdateFrame(pNewDoc, TRUE);

	return 0;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::PreCreateWindow (CREATESTRUCT& cs)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState());

	if ((CreateCX != 0)&&(CreateCY != 0))
	{
		cs.x = CreateX;
		cs.y = CreateY;
		cs.cx = CreateCX;
		cs.cy = CreateCY;
	}

	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CMainFrame::OnCreateClient (LPCREATESTRUCT, CCreateContext* pContext)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState());
	// create a splitter with 1 row, 2 columns
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	// add the first splitter pane - the default view in column 0
	if (!m_wndSplitter.CreateView(0, 0, 
		RUNTIME_CLASS(CDisplay), CSize(640, 512), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	// add the second splitter pane - an input view in column 1
	if (!m_wndSplitter.CreateView(0, 1,
		RUNTIME_CLASS(CToolsLogic), CSize(100, 100), pContext))
	{
		TRACE0("Failed to create second pane\n");
		return FALSE;
	}

	_SplitterCreated = true;

	return TRUE;
}

// ---------------------------------------------------------------------------
void CMainFrame::initDisplay()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SetActiveView((CView*)m_wndSplitter.GetPane (0,0));
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	dispWnd->setCellSize (_Config.CellSize);
	dispWnd->init (this);
}

// ---------------------------------------------------------------------------
void CMainFrame::uninitTools()
{
	if (_Mode == 0) // Mode Zone
	{
		CToolsZone *toolWnd = dynamic_cast<CToolsZone*>(m_wndSplitter.GetPane(0,1));
		toolWnd->uninit ();
	}
	if (_Mode == 1) // Mode Logic
	{
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
		toolWnd->uninit ();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::initTools()
{
	if (_Mode == 0) // Mode Zone
	{
		m_wndSplitter.DeleteView (0, 1);
		m_wndSplitter.CreateView (0, 1, RUNTIME_CLASS(CToolsZone), CSize(100, 100), NULL);
		SetActiveView ((CView*)m_wndSplitter.GetPane(0,1));
		CToolsZone *toolWnd = dynamic_cast<CToolsZone*>(m_wndSplitter.GetPane(0,1));
		toolWnd->init (this);
		adjustSplitter ();
	}
	if (_Mode == 1) // Mode Logic
	{
		m_wndSplitter.DeleteView (0, 1);
		m_wndSplitter.CreateView (0, 1, RUNTIME_CLASS(CToolsLogic), CSize(100, 100), NULL);
		SetActiveView ((CView*)m_wndSplitter.GetPane(0,1));
		CToolsLogic *toolWnd = dynamic_cast<CToolsLogic*>(m_wndSplitter.GetPane(0,1));
		toolWnd->init (this);
		adjustSplitter ();
	}
	SetActiveView ((CView*)m_wndSplitter.GetPane(0,0));
}

// ---------------------------------------------------------------------------
void CMainFrame::primZoneModified()
{
	if (_MasterCB != NULL)
	{
		vector<string> allNames;
		_PRegionBuilder.getAllPrimZoneNames (allNames);
		_MasterCB->setAllPrimZoneNames (allNames);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::displayCoordinates (CVector &v)
{
	std::string sTmp;
	sTmp = "( " + toString(v.x) + " , " + toString(v.y) + " )";

	for (uint32 i = sTmp.size(); i < 30; ++i)
		sTmp += "_";
	// Write the zone name
	sint32 x, y;

	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	x = (sint32)(v.x / dispWnd->_CellSize);
	y = (sint32)(v.y / dispWnd->_CellSize);
	if ((v.x >= 0) && (x <= 255) && (v.y <= 0) && (y >= -255))
	{
		sTmp += toString(-(y)) + " ";
		sTmp += ('A' + ((x)/26));
		sTmp += ('A' + ((x)%26));
	}
	else
	{
		sTmp += "NOT A VALID ZONE";
	}

	m_wndStatusBar.SetWindowText (sTmp.c_str());
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
// ---------------------------------------------------------------------------
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

// ---------------------------------------------------------------------------
void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
bool CMainFrame::init (bool bMakeAZone)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Load the Ligoscape.cfg
	try
	{
		string sConfigFileName = _ExeDir;
		sConfigFileName += "ligoscape.cfg";
		// Load the config file
		_Config.read (sConfigFileName.c_str());
	}
	catch (Exception& e)
	{
		MessageBox (e.what(), "Warning");

		// Can't found the module put some default values
		_Config.CellSize = 160.0f;
		_Config.Snap = 1.0f;
	}

	// Load the WorldEditor.cfg
	try
	{
		CIFile fileIn;
		string sWorldEdCfg = _ExeDir;
		sWorldEdCfg += "WorldEditor.cfg";
		fileIn.open (sWorldEdCfg.c_str());
		fileIn.serial (_Environnement);
		// Update all from environnement
		CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
		dispWnd->setBackgroundColor (_Environnement.BackgroundColor);
		if (bMakeAZone) // Do we are launched in a stand alone mode ?
		{
			_DataDir = _Environnement.DataDir; // Yes get the data path from cfg file
			if ((_DataDir[_DataDir.size()-1] != '\\') && (_DataDir[_DataDir.size()-1] != '/'))
			{
				_DataDir += "\\";
				_Environnement.DataDir += "\\";
			}
		}
	}

	catch (Exception& e)
	{
		MessageBox (e.what(), "Warning");
		SEnvironnement newEnv;
		_Environnement = newEnv;
	}

	// Initialize the zoneBuilder (load bank and this kind of stuff)
	_ZoneBuilder.init (_DataDir, bMakeAZone);

	return false;
}

// ---------------------------------------------------------------------------
void CMainFrame::uninit ()
{
	// Save the WorldEditor.cfg
	try
	{
		COFile fileOut;
		string sWorldEdCfg = _ExeDir;
		sWorldEdCfg += "WorldEditor.cfg";
		fileOut.open (sWorldEdCfg.c_str());
		fileOut.serial(_Environnement);
	}
	catch (Exception& e)
	{
		MessageBox (e.what(), "Warning");
	}

	// Check to save all files
	_ZoneBuilder.uninit ();
	_PRegionBuilder.uninit ();
}

// ******************
// MESSAGES FROM MENU
// ******************

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileNewLogic ()
{
	_PRegionBuilder.newZone ();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileUnloadLogic ()
{
	CSelectDialog seldial(this);

	seldial.setLogic (&_PRegionBuilder);
	if (seldial.DoModal () == IDOK)
	{
		_PRegionBuilder.unload (seldial.getSel());
		CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
		dispWnd->OnDraw	(NULL);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileOpenLogic ()
{
	CFileDialog dialog (true, "prim", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Primitives (*.prim)|*.prim", this);
	if (dialog.DoModal() == IDOK)
	{
		_PRegionBuilder.load ((LPCTSTR)dialog.GetFileName(), "");
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileSaveLogic ()
{
	CSelectDialog seldial(this);

	seldial.setLogic (&_PRegionBuilder);
	if (seldial.DoModal () == IDOK)
	{
		CFileDialog dialog (false, "prim");
		if (dialog.DoModal() == IDOK)
		{
			_PRegionBuilder.save (seldial.getSel(), (LPCTSTR)dialog.GetFileName());
		}
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileNewLandscape ()
{
	_ZoneBuilder.newZone ();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileUnloadLandscape ()
{
	CSelectDialog seldial(this);

	seldial.set (&_ZoneBuilder);
	if (seldial.DoModal () == IDOK)
	{
		_ZoneBuilder.unload (seldial.getSel());
		CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
		dispWnd->OnDraw	(NULL);
		_ZoneBuilder.stackReset ();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileOpenLandscape ()
{
	CFileDialog dialog (true, "land", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Landscape (*.land)|*.land", this);
	if (dialog.DoModal() == IDOK)
	{
		_ZoneBuilder.load ((LPCTSTR)dialog.GetFileName(), "");
		_ZoneBuilder.stackReset ();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileSaveLandscape ()
{
	CFileDialog dialog (false, "land");
	if (dialog.DoModal() == IDOK)
	{
		_ZoneBuilder.save ((LPCTSTR)dialog.GetFileName());
		_ZoneBuilder.stackReset ();
	}	
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileGenerate ()
{
	CGenerateDlg dialog;
	_ZoneBuilder.getZoneBank().getCategoryValues ("material", dialog.AllMaterials);
	if (dialog.DoModal() == IDOK)
	{
		_ZoneBuilder.generate (dialog.MinX, dialog.MinY, dialog.MaxX, dialog.MaxY, 
								dialog.ZoneBaseX, dialog.ZoneBaseY, dialog.ComboMaterialString);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileExportToLevelD ()
{
	CExportDlg dialog;
	dialog.OutZoneDir = _Environnement.ExportOptions.OutZoneDir.c_str();
	dialog.RefZoneDir = _Environnement.ExportOptions.RefZoneDir.c_str();
	dialog.TileBankFile = _Environnement.ExportOptions.TileBankFile.c_str();
	dialog.HeightMapFile = _Environnement.ExportOptions.HeightMapFile.c_str();
	dialog.ZFactor = toString(_Environnement.ExportOptions.ZFactor).c_str();
	dialog.HeightMapFile2 = _Environnement.ExportOptions.HeightMapFile2.c_str();
	dialog.ZFactor2 = toString(_Environnement.ExportOptions.ZFactor2).c_str();
	dialog.Lighting = _Environnement.ExportOptions.Light;
	dialog.ZoneMin = _Environnement.ExportOptions.ZoneMin.c_str();
	dialog.ZoneMax = _Environnement.ExportOptions.ZoneMax.c_str();

	if (dialog.DoModal() == IDOK)
	{
		_Environnement.ExportOptions.OutZoneDir = (LPCSTR)dialog.OutZoneDir;
		_Environnement.ExportOptions.RefZoneDir = (LPCSTR)dialog.RefZoneDir;
		_Environnement.ExportOptions.TileBankFile = (LPCSTR)dialog.TileBankFile;
		_Environnement.ExportOptions.HeightMapFile = (LPCSTR)dialog.HeightMapFile;
		_Environnement.ExportOptions.ZFactor = (float)atof ((LPCSTR)dialog.ZFactor);
		_Environnement.ExportOptions.HeightMapFile2 = (LPCSTR)dialog.HeightMapFile2;
		_Environnement.ExportOptions.ZFactor2 = (float)atof ((LPCSTR)dialog.ZFactor2);
		_Environnement.ExportOptions.Light = dialog.Lighting;
		_Environnement.ExportOptions.ZoneMin = (LPCSTR)dialog.ZoneMin;
		_Environnement.ExportOptions.ZoneMax = (LPCSTR)dialog.ZoneMax;

		CExportCBDlg *pDlg = new CExportCBDlg();
		//Check if new succeeded and we got a valid pointer to a dialog object
		CExport Export;
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

		_Environnement.ExportOptions.ZoneRegion = (NLLIGO::CZoneRegion*)_ZoneBuilder.getPtrCurZoneRegion();
		_Environnement.ExportOptions.LigoBankDir = _DataDir + "ZoneLigos";
		Export.export (_Environnement.ExportOptions, pDlg->getExportCB());

		pDlg->setFinishedButton ();
		while (pDlg->getFinished () == false)
		{
			pDlg->pump ();
		}
		pDlg->DestroyWindow ();
	}
}

// ---------------------------------------------------------------------------
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

void CMainFrame::OnMenuFileSetDataDirectory ()
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;
	bi.lpszTitle = "Choose the path";
	bi.ulFlags = 0;
	bi.lpfn = dataDirBrowseCallbackProc;

	char sDir[512];
	strcpy(sDir, _DataDir.c_str());
	bi.lParam = (LPARAM)sDir;

	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	setDataDir (str);
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuFileExit ()
{
	OnClose();
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuModeZone ()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuItem (ID_MODE_ZONE, MF_CHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_MODE_LOGIC, MF_UNCHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_MODE_TRANSITION, MF_UNCHECKED|MF_BYCOMMAND);
	if (_Mode == 1) // if we were in Logic mode switch
	{
		uninitTools ();
		_Mode = 0;
		initTools ();
	}
	_Mode = 0;
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	dispWnd->OnDraw	(NULL);
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuModeTransition ()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuItem (ID_MODE_ZONE, MF_UNCHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_MODE_LOGIC, MF_UNCHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_MODE_TRANSITION, MF_CHECKED|MF_BYCOMMAND);
	if (_Mode == 1) // if we were in Logic mode switch
	{
		uninitTools ();
		_Mode = 0;
		initTools ();
	}
	_Mode = 2;
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	dispWnd->OnDraw	(NULL);
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuModeLogic ()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuItem (ID_MODE_ZONE, MF_UNCHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_MODE_LOGIC, MF_CHECKED|MF_BYCOMMAND);
	menu->CheckMenuItem (ID_MODE_TRANSITION, MF_UNCHECKED|MF_BYCOMMAND);
	if (_Mode != 1) // Mode Logic
	{
		uninitTools();
		_Mode = 1;
		initTools();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuModeType ()
{
	CTypeManagerDlg tmDial(this);

	tmDial.set (_Environnement.Types);
	if (tmDial.DoModal () == IDOK)
	{
		_Environnement.Types = tmDial.get ();
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::onMenuModeSelectZone ()
{
	CSelectDialog seldial(this);

	seldial.set (&_ZoneBuilder);
	if (seldial.DoModal () == IDOK)
	{
		_ZoneBuilder.setCurZoneRegion (seldial.getSel());
		CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
		dispWnd->OnDraw	(NULL);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::onMenuModeUndo ()
{
	if ((_Mode == 0) || (_Mode == 2)) // Mode Zone or Mode Transition
		_ZoneBuilder.undo ();
	else
		_PRegionBuilder.undo ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onMenuModeRedo ()
{
	if ((_Mode == 0) || (_Mode == 2)) // Mode Zone or Mode Transition
		_ZoneBuilder.redo ();
	else
		_PRegionBuilder.redo ();
}

// ---------------------------------------------------------------------------
void CMainFrame::onMenuModeMove ()
{
	CMoveDlg dialog;
	if (dialog.DoModal() == IDOK)
	if (_ZoneBuilder._ZoneRegionNames.size() > 0)
	{
		_ZoneBuilder.move (dialog.XOffset, dialog.YOffset);
		string tmp = _ZoneBuilder._ZoneRegionNames[_ZoneBuilder._ZoneRegionSelected], tmp2;
		for(uint32 i = 0; i < tmp.size(); ++i)
			if (tmp[i] == '.')
				break;
			else
				tmp2 += tmp[i];
		tmp2 += ".prim";
		_PRegionBuilder.move (tmp2, dialog.XOffset*_Config.CellSize, dialog.YOffset*_Config.CellSize);
		CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
		dispWnd->OnDraw	(NULL);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::onMenuModeCountZones ()
{
	if (_ZoneBuilder._ZoneRegionNames.size() > 0)
	{
		uint32 nNbZones = _ZoneBuilder.countZones ();
		string tmp = "There are " + toString(nNbZones) + " zones";
		MessageBox(tmp.c_str(), "Information", MB_ICONINFORMATION|MB_OK);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuViewGrid ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
	dispWnd->setDisplayGrid (!dispWnd->getDisplayGrid());
	CMenu *menu = GetMenu();
	menu->CheckMenuItem (ID_VIEW_GRID, dispWnd->getDisplayGrid()?MF_CHECKED|MF_BYCOMMAND:MF_UNCHECKED|MF_BYCOMMAND);
}

// ---------------------------------------------------------------------------
void CMainFrame::OnMenuViewBackground ()
{
	CColorDialog colDial;

	if (colDial.DoModal() == IDOK)
	{
		int r = GetRValue(colDial.GetColor());
		int g = GetGValue(colDial.GetColor());
		int b = GetBValue(colDial.GetColor());
		_Environnement.BackgroundColor = CRGBA(r,g,b,255);
		CDisplay *dispWnd = dynamic_cast<CDisplay*>(m_wndSplitter.GetPane(0,0));
		dispWnd->setBackgroundColor (_Environnement.BackgroundColor);
		dispWnd->OnDraw	(NULL);
	}
}

// ---------------------------------------------------------------------------
void CMainFrame::OnClose ()
{
	_Exit = true;
}

// ---------------------------------------------------------------------------
void CMainFrame::OnSize (UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED)
	{
		if (cx < 100)
			cx = 100;
		if (cy < 100)
			cy = 100;
	}
	CFrameWnd::OnSize (nType, cx, cy);
	if (nType != SIZE_MINIMIZED)
		adjustSplitter ();
}

// ---------------------------------------------------------------------------
void CMainFrame::adjustSplitter ()
{
	if (_SplitterCreated)
	{
		RECT r;
		GetClientRect (&r);
		if (r.right-r.left > 380)
		{
			if ((_Mode == 0) || (_Mode == 2)) // Mode Zone
				m_wndSplitter.SetColumnInfo (0, r.right-r.left-380, 100); // 380 really experimental value
			
			if (_Mode == 1) // Mode Logic
				m_wndSplitter.SetColumnInfo (0, r.right-r.left-300, 10);
		}
		m_wndSplitter.RecalcLayout ();
	}
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// CSelectDialog
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
CSelectDialog::CSelectDialog (CWnd*pParent): CDialog(IDD_SELECTZONE, pParent) 
{
	_Sel = -1;
	_ToInit = NULL;
	_ToInitLogic = NULL;
}

// ---------------------------------------------------------------------------
BOOL CSelectDialog::OnInitDialog ()
{
	CListBox *pLB = (CListBox*)GetDlgItem (IDC_LISTZONE);
	if (_ToInit != NULL)
	{
		uint32 nNbRegion = _ToInit->getNbZoneRegion ();
		for (uint32 i = 0; i < nNbRegion; ++i)
			pLB->InsertString (-1, _ToInit->getZoneRegionName(i).c_str());
		
		_Sel = _ToInit->getCurZoneRegion();
		pLB->SetCurSel (_Sel);
	}
	
	if (_ToInitLogic != NULL)
	{
		uint32 nNbRegion = _ToInitLogic->getNbZoneRegion ();
		for (uint32 i = 0; i < nNbRegion; ++i)
			pLB->InsertString (-1, _ToInitLogic->getZoneRegionName(i).c_str());
		
		_Sel = 0;
		pLB->SetCurSel (_Sel);
	}
	return true;
}

// ---------------------------------------------------------------------------
void CSelectDialog::set (CBuilderZone *pBZ)
{
	_ToInit = pBZ;
}

// ---------------------------------------------------------------------------
void CSelectDialog::setLogic (CBuilderLogic *pBL)
{
	_ToInitLogic = pBL;
}

// ---------------------------------------------------------------------------
void CSelectDialog::OnOK ()
{
	_Sel = ((CListBox*)GetDlgItem(IDC_LISTZONE))->GetCurSel();
	CDialog::OnOK();
}

// ---------------------------------------------------------------------------
int CSelectDialog::getSel()
{
	return _Sel;
}