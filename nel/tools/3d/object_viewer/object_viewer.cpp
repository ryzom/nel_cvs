// object_viewer.cpp : Defines the initialization routines for the DLL.
//

#include "std_afx.h"

#define OBJECT_VIEWER_EXPORT __declspec( dllexport ) 

#include "object_viewer.h"
#include <nel/3d/nelu.h>
#include <nel/3d/mesh.h>
#include <nel/3d/transform_shape.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

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
	_AnimationSetDlg=new CAnimationSetDlg (this, &driverWnd);
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
	setAnimTime (0.f, 100.f);

	// Register this as listener
	CNELU::EventServer.addListener (EventDestroyWindowId, this);

	// Add mouse listener to event server
	_MouseListener.addToServer(CNELU::EventServer);
		
	// Detach the hwnd
	driverWnd.Detach ();

	CNELU::Driver->activate ();
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
		CNELU::Driver->activate ();

		// Handle animation
		_AnimationDlg->handle ();

		// Update the playlist
		_SlotDlg->getSlot ();

		// Setup the channel mixer
		_SlotDlg->Playlist.setupMixer (_ChannelMixer, _AnimationDlg->getTime());

		// Default Transform for all the models
		for (uint i=0; i<_ListTransformShape.size(); i++)
		{
			// Set default position values
			CAnimatedValueVector *pos=(CAnimatedValueVector*)&(((CMesh*)(IShape*)_ListTransformShape[i]->Shape)->getDefaultPos ()->getValue());
			_ListTransformShape[i]->setPos (pos->Value);

			CAnimatedValueVector *scale=(CAnimatedValueVector*)&(((CMesh*)(IShape*)_ListTransformShape[i]->Shape)->getDefaultScale ()->getValue());
			_ListTransformShape[i]->setScale (scale->Value);

			if (_SceneDlg->Euler)
			{
				CAnimatedValueVector *euler=(CAnimatedValueVector*)&(((CMesh*)(IShape*)_ListTransformShape[i]->Shape)->getDefaultRotEuler ()->getValue());
				_ListTransformShape[i]->setRotEuler (euler->Value);
			}
			else
			{
				CAnimatedValueQuat *quat=(CAnimatedValueQuat*)&(((CMesh*)(IShape*)_ListTransformShape[i]->Shape)->getDefaultRotQuat ()->getValue());
				_ListTransformShape[i]->setRotQuat (quat->Value);
			}
		}

		// Eval channel mixer for transform
		_ChannelMixer.eval (false);
		
		// New matrix from camera
		CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
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
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

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


void CObjectViewer::resetSlots ()
{
	// Reset the animation set
	_AnimationSet.reset ();

	// Set no animation in slot UI
	for (uint i=0; i<NL3D::CChannelMixer::NumAnimationSlot; i++)
		_SlotDlg->Slots[i].setAnimation (0xffffffff, NULL, NULL);
}

// ***************************************************************************

void CObjectViewer::reinitChannels ()
{
	// Reset the channels
	_ChannelMixer.resetChannels ();

	// Add all the instance in the channel mixer
	for (uint i=0; i<_ListTransformShape.size(); i++)
		_ListTransformShape[i]->registerToChannelMixer (&_ChannelMixer, "");
}

// ***************************************************************************

float CObjectViewer::getFrameRate ()
{
	return _AnimationDlg->Speed;
}

// ***************************************************************************

void CObjectViewer::serial (NLMISC::IStream& f)
{
	// serial "OBJV_CFG"
	f.serialCheck ((uint32)'VJBO');
	f.serialCheck ((uint32)'GFC_');

	// serial the version
	int ver=f.serialVersion (0);

	// update data
	_AnimationDlg->UpdateData ();
	_SceneDlg->UpdateData ();

	// serial animation data
	f.serial (_AnimationDlg->Start);
	f.serial (_AnimationDlg->End);
	f.serial (_AnimationDlg->Speed);
	bool loop=_AnimationDlg->Loop!=0;
	f.serial (loop);
	_AnimationDlg->Loop=loop;
	bool euler=_SceneDlg->Euler!=0;
	f.serial (euler);
	_SceneDlg->Euler=euler;
	sint32 ui=_AnimationDlg->UICurrentFrame;
	f.serial (ui);
	_AnimationDlg->UICurrentFrame=ui;
	f.serial (_AnimationDlg->CurrentFrame);

	// update data
	_AnimationDlg->UpdateData (FALSE);
	_SceneDlg->UpdateData (FALSE);

	// For each slot
	for (uint i=0; i<NL3D::CChannelMixer::NumAnimationSlot; i++)
	{
		// Update values
		_SlotDlg->Slots[i].UpdateData();

		// Serial the slot state
		bool slotState=_SlotDlg->Slots[i].enable!=0;
		f.serial (slotState);
		_SlotDlg->Slots[i].enable=slotState;

		// Update values
		_SlotDlg->Slots[i].UpdateData(FALSE);
	}
		

	// view matrix
	CMatrix mt=_MouseListener.getViewMatrix();
	f.serial (mt);
	_MouseListener.setMatrix (mt);

	// serial list of shape
	std::vector<std::string> stringArray=_ListShape;

	// serial list of shape
	f.serialCont (stringArray);

	// If reading, read shapes
	if (f.isReading ())
	{
		// Load each shape
		for (uint s=0; s<stringArray.size(); s++)
			loadShape (stringArray[s].c_str());
	}

	// List of animation
	stringArray=_AnimationSetDlg->_ListAnimation;

	// Serial the list
	f.serialCont (stringArray);

	// Serial the list of animation
	if (f.isReading ())
	{
		// Load animation file
		for (uint s=0; s<stringArray.size(); s++)
			_AnimationSetDlg->loadAnimation (stringArray[s].c_str());
	}

	// List of skeleton
	stringArray=_AnimationSetDlg->_ListSkeleton;

	// Serial the list
	f.serialCont (stringArray);

	// Serial the list of animation
	if (f.isReading ())
	{
		// Load animation file
		for (uint s=0; s<stringArray.size(); s++)
			_AnimationSetDlg->loadSkeleton (stringArray[s].c_str());
	}

	// Serial the play list
	if (f.isReading ())
	{
		// Serial the play list
		f.serial (_SlotDlg->Playlist);

		// Set the play list
		_SlotDlg->setSlot ();
	}
	else
	{
		// Set the play list
		_SlotDlg->getSlot ();

		// Serial the play list
		f.serial (_SlotDlg->Playlist);
	}

	// Some init
	if (f.isReading ())
	{
		// Init start and end time
		setAnimTime (_AnimationDlg->Start, _AnimationDlg->End);

		// Touch the channel mixer
		reinitChannels ();
	}
}

// ***************************************************************************

bool CObjectViewer::loadShape (const char* filename)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add to the path
	char drive[256];
	char dir[256];
	char path[256];
	_splitpath (filename, drive, dir, NULL, NULL);
	_makepath (path, drive, dir, NULL, NULL);
	CPath::addSearchPath (path);

	// Open a file
	CIFile file;
	if (file.open (filename))
	{
		// Sream a shape
		CShapeStream streamShape;
		try
		{
			// Stream it
			streamShape.serial (file);

			// Add the shape
			addShape (streamShape.getShapePointer(), filename);

			return true;
		}
		catch (Exception& e)
		{
			_SceneDlg->MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", filename);
		_SceneDlg->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
	}
	return false;
}

// ***************************************************************************

void CObjectViewer::resetCamera ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_SceneDlg->OnResetCamera();
}

// ***************************************************************************

void CObjectViewer::addShape (IShape* pShape, const char* name)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Store the shape pointer
	CNELU::Scene.addShape (name, CSmartPtr<IShape> (pShape));

	// Store the name of the shape
	_ListShape.push_back (name);

	// Create a model and add it to the scene
	CTransformShape	*pTrShape=CNELU::Scene.createInstance (name);
	nlassert (pTrShape);

	// Set the rot model
	if (_SceneDlg->Euler)
		pTrShape->setTransformMode (ITransformable::RotEuler);
	else
		pTrShape->setTransformMode (ITransformable::RotQuat);

	// Store the transform shape pointer
	_ListTransformShape.push_back (pTrShape);
}

// ***************************************************************************

IObjectViewer* IObjectViewer::getInterface (int version)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Check version number
	if (version!=OBJECT_VIEWER_VERSION)
	{
		MessageBox (NULL, "Bad version of object_viewer.dll.", "NeL object viewer", MB_ICONEXCLAMATION|MB_OK);
		return NULL;
	}
	else
		return new CObjectViewer;
}

// ***************************************************************************

void IObjectViewer::releaseInterface (IObjectViewer* view)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	delete view;
}

// ***************************************************************************

void CObjectViewer::setSingleAnimation (NL3D::CAnimation* pAnim, const char* name)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add the animation to the animationSet
	_AnimationSetDlg->UpdateData (TRUE);
	_AnimationSetDlg->addAnimation (pAnim, name);
	_AnimationSetDlg->UpdateData (FALSE);

	// Set time
	setAnimTime (pAnim->getBeginTime()*_AnimationDlg->Speed, pAnim->getEndTime()*_AnimationDlg->Speed);

	// Set the animation in the first slot
	_SlotDlg->UpdateData ();
	_SlotDlg->Slots[0].setAnimation (_AnimationSet.getAnimationIdByName(name), pAnim, name);
	_SlotDlg->Slots[0].StartBlend=1.f;
	_SlotDlg->Slots[0].EndBlend=1.f;
	_SlotDlg->Slots[0].Offset=0;
	_SlotDlg->Slots[0].enable=TRUE;
	_SlotDlg->UpdateData (FALSE);

	// Reinit
	reinitChannels ();
}

// ***************************************************************************

