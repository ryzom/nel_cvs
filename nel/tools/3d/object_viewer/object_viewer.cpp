// object_viewer.cpp : Defines the initialization routines for the DLL.
//

#include "std_afx.h"

#define OBJECT_VIEWER_EXPORT __declspec( dllexport ) 

#include "object_viewer.h"
#include <nel/3d/nelu.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NL3D;
using namespace NLMISC;

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
// CObject_viewerApp

BEGIN_MESSAGE_MAP(CObject_viewerApp, CWinApp)
	//{{AFX_MSG_MAP(CObject_viewerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObject_viewerApp construction

CObject_viewerApp::CObject_viewerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CObject_viewerApp object

CObject_viewerApp theApp;

// ***************************************************************************

CObjectViewer::CObjectViewer ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_SlotDlg=NULL;
	_AnimationSetDlg=NULL;
	_AnimationDlg=NULL;
	_SceneDlg=NULL;

	// Setup animation set
	_ChannelMixer.setAnimationSet (&_AnimationSet);
}

// ***************************************************************************

CObjectViewer::~CObjectViewer ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_SlotDlg)
		delete _SlotDlg;
	if (_AnimationSetDlg)
		delete _AnimationSetDlg;
	if (_AnimationDlg)
		delete _AnimationDlg;
	if (_SceneDlg)
		delete _SceneDlg;
}

// ***************************************************************************

void CObjectViewer::initUI ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// The viewport
	CViewport viewport;

	// Init NELU
	CNELU::init (640, 480, viewport);

	// Create a cwnd
	CWnd driverWnd;
	driverWnd.Attach((HWND)CNELU::Driver->getDisplay());
	getRegisterWindowState (&driverWnd, REGKEY_OBJ_VIEW_OPENGL_WND, true);

	// Hide the main window
	//driverWnd.ShowWindow (SW_HIDE);

	// Create animation set dialog
	_AnimationDlg=new CAnimationDlg (this, &driverWnd);
	_AnimationDlg->Create (IDD_ANIMATION);
	getRegisterWindowState (_AnimationDlg, REGKEY_OBJ_VIEW_ANIMATION_DLG, false);

	// Create animation set dialog
	_AnimationSetDlg=new CAnimationSetDlg (&_AnimationSet, &driverWnd);
	_AnimationSetDlg->Create (IDD_ANIMATION_SET);
	getRegisterWindowState (_AnimationSetDlg, REGKEY_OBJ_VIEW_ANIMATION_SET_DLG, false);

	// Create the main dialog
	_SlotDlg=new CMainDlg (this, &driverWnd);
	_SlotDlg->init (&_AnimationSet);
	_SlotDlg->Create (IDD_MAIN_DLG);
	getRegisterWindowState (_SlotDlg, REGKEY_OBJ_VIEW_SLOT_DLG, false);

	// Create animation set dialog
	_SceneDlg=new CSceneDlg (this, &driverWnd);
	_SceneDlg->Create (IDD_SCENE);
	_SceneDlg->ShowWindow (TRUE);
	getRegisterWindowState (_SceneDlg, REGKEY_OBJ_VIEW_SCENE_DLG, false);

	// Show the windows
	//driverWnd.ShowWindow (SW_SHOW);

	// Set current frame
	setAnimTime (0.f, 10.f);

	// Register this as listener
	CNELU::EventServer.addListener (EventDestroyWindowId, this);

	// Add mouse listener to event server
	_MouseListener.addToServer(CNELU::EventServer);
		
	// Detach the hwnd
	driverWnd.Detach ();
}

// ***************************************************************************

void CObjectViewer::operator ()(const CEvent& event)
{
	// Destro window ?
	if (event==EventDestroyWindowId)
	{
	}
}

// ***************************************************************************

void CObjectViewer::go ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	do
	{
		// Handle animation
		_AnimationDlg->handle ();

		// Update the playlist
		_SlotDlg->getSlot ();

		// Setup the channel mixer
		_SlotDlg->Playlist.setupMixer (_ChannelMixer, _AnimationDlg->getTime());
		
		// New matrix from camera
		CNELU::Camera->setMatrix (_MouseListener.getViewMatrix());

		// Clear the buffers
		CNELU::clearBuffers(CRGBA(120,120,120));

		// Draw the scene
		CNELU::Scene.render();

		// Swap the buffers
		CNELU::swapBuffers();

		// Pump message from the server
		CNELU::EventServer.pump();

		// Pump others message for the windows
		MSG	msg;
		while ( PeekMessage(&msg, NULL,0,0,PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	while (!CNELU::AsyncListener.isKeyPushed(KeyESCAPE)&&CNELU::Driver->isActive());
}

// ***************************************************************************

void CObjectViewer::releaseUI ()
{
	if (CNELU::Driver->isActive())
	{
		// register window position
		if (_SceneDlg)
			setRegisterWindowState (_SceneDlg, REGKEY_OBJ_VIEW_SCENE_DLG);
		if (_AnimationDlg)
			setRegisterWindowState (_AnimationDlg, REGKEY_OBJ_VIEW_ANIMATION_DLG);
		if (_SlotDlg)
			setRegisterWindowState (_SlotDlg, REGKEY_OBJ_VIEW_SLOT_DLG);
		if (_AnimationSetDlg)
			setRegisterWindowState (_AnimationSetDlg, REGKEY_OBJ_VIEW_ANIMATION_SET_DLG);
		if (CNELU::Driver->getDisplay())
		{
			CWnd driverWnd;
			driverWnd.Attach((HWND)CNELU::Driver->getDisplay());
			setRegisterWindowState (&driverWnd, REGKEY_OBJ_VIEW_OPENGL_WND);
			driverWnd.Detach ();
		}
	}

	// Release the emitter from the server
	_MouseListener.removeFromServer (CNELU::EventServer);

	// Remove this as listener
	CNELU::EventServer.removeListener (EventDestroyWindowId, this);

	// exit
	CNELU::release();
}

// ***************************************************************************

void setRegisterWindowState (const CWnd *pWnd, const char* keyName)
{
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, keyName, &hKey)==ERROR_SUCCESS)
	{
		RECT rect;
		pWnd->GetWindowRect (&rect);
		RegSetValueEx(hKey, "Left", 0, REG_DWORD, (LPBYTE)&rect.left, 4);
		RegSetValueEx(hKey, "Right", 0, REG_DWORD, (LPBYTE)&rect.right, 4);
		RegSetValueEx(hKey, "Top", 0, REG_DWORD, (LPBYTE)&rect.top, 4);
		RegSetValueEx(hKey, "Bottom", 0, REG_DWORD, (LPBYTE)&rect.bottom, 4);
	}
}

// ***************************************************************************

void getRegisterWindowState (CWnd *pWnd, const char* keyName, bool resize)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, keyName, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD len=4;
		DWORD type;
		RECT rect;
		RegQueryValueEx (hKey, "Left", 0, &type, (LPBYTE)&rect.left, &len);
		RegQueryValueEx (hKey, "Right", 0, &type, (LPBYTE)&rect.right, &len);
		RegQueryValueEx (hKey, "Top", 0, &type, (LPBYTE)&rect.top, &len);
		RegQueryValueEx (hKey, "Bottom", 0, &type, (LPBYTE)&rect.bottom, &len);

		// Set window pos
		pWnd->SetWindowPos (NULL, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOOWNERZORDER|SWP_NOZORDER|
			(resize?0:SWP_NOSIZE));
	}
}

// ***************************************************************************

void CObjectViewer::setAnimTime (float animStart, float animEnd)
{
	// Dispatch the command
	_SlotDlg->setAnimTime (animStart, animEnd);
	_AnimationDlg->setAnimTime (animStart, animEnd);
}

// ***************************************************************************


