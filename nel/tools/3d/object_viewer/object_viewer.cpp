/** \file object_viewer.cpp
 * : Defines the initialization routines for the DLL.
 *
 * $Id: object_viewer.cpp,v 1.6 2001/04/26 17:57:41 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

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

	// Hotspot color
	_HotSpotColor.R=255;
	_HotSpotColor.G=255;
	_HotSpotColor.B=0;
	_HotSpotColor.A=255;

	// Hotspot size
	_HotSpotSize=10.f;
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

	// Camera
	CFrustum frustrum;
	frustrum.initPerspective( 75.f*(float)Pi/180.f, 1.33f, 0.1f, 1000.f);
	CNELU::Camera->setFrustum (frustrum);
		

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

		// Eval channel mixer for transform
		_ChannelMixer.eval (false);

		// Mouse listener
		_SceneDlg->UpdateData ();
		if (_SceneDlg->ObjectMode)
			_MouseListener.setMouseMode (CEvent3dMouseListener::edit3d);
		else
		{
			_MouseListener.setMouseMode (CEvent3dMouseListener::firstPerson);
			_MouseListener.setSpeed (_SceneDlg->MoveSpeed);
		}

		// New matrix from camera
		CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
		CNELU::Camera->setMatrix (_MouseListener.getViewMatrix());

		// Clear the buffers
		CNELU::clearBuffers(CRGBA(120,120,120));

		// Draw the scene
		CNELU::Scene.render();

		// Draw the hotSpot
		if (_SceneDlg->ObjectMode)
		{
			float radius=_HotSpotSize/2.f;
			CNELU::Driver->setupModelMatrix (CMatrix::Identity);
			CDRU::drawLine (_MouseListener.getHotSpot()+CVector (radius, 0, 0), _MouseListener.getHotSpot()+CVector (-radius, 0, 0), _HotSpotColor, *CNELU::Driver);
			CDRU::drawLine (_MouseListener.getHotSpot()+CVector (0, radius, 0), _MouseListener.getHotSpot()+CVector (0, -radius, 0), _HotSpotColor, *CNELU::Driver);
			CDRU::drawLine (_MouseListener.getHotSpot()+CVector (0, 0, radius), _MouseListener.getHotSpot()+CVector (0, 0, -radius), _HotSpotColor, *CNELU::Driver);
		}

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
		_ListTransformShape[i]->registerToChannelMixer (&_ChannelMixer, _ListShapeBaseName[i]);
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
	std::vector<CMeshDesc> meshArray=_ListMeshes;

	// serial list of shape
	f.serialCont (meshArray);

	// If reading, read shapes
	if (f.isReading ())
	{
		// Load each shape
		for (uint s=0; s<meshArray.size(); s++)
			loadMesh (meshArray[s].MeshName.c_str(), meshArray[s].SkeletonName.c_str());
	}

	// List of animation
	std::vector<std::string> stringArray=_AnimationSetDlg->_ListAnimation;

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

bool CObjectViewer::loadMesh (const char* meshFilename, const char* skeleton)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add to the path
	char drive[256];
	char dir[256];
	char path[256];

	// Add search path for the mesh
	_splitpath (meshFilename, drive, dir, NULL, NULL);
	_makepath (path, drive, dir, NULL, NULL);
	CPath::addSearchPath (path);

	// Add search path for the skeleton
	_splitpath (skeleton, drive, dir, NULL, NULL);
	_makepath (path, drive, dir, NULL, NULL);
	CPath::addSearchPath (path);

	// Shape pointer
	IShape *shapeMesh=NULL;
	IShape *shapeSkel=NULL;

	// Open a file
	CIFile file;
	if (file.open (meshFilename))
	{
		// Sream a shape
		CShapeStream streamShape;
		try
		{
			// Stream it
			streamShape.serial (file);

			// Add the shape
			shapeMesh=streamShape.getShapePointer();
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
		_snprintf (msg, 512, "Can't open the file %s for reading.", meshFilename);
		_SceneDlg->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
	}

	// Continue ?
	if (shapeMesh)
	{
		// Skel error ?
		bool skelError=false;

		if (skeleton)
		{
			// Open a file
			if (file.open (skeleton))
			{
				// Sream a shape
				CShapeStream streamShape;
				try
				{
					// Stream it
					streamShape.serial (file);

					// Add the shape
					shapeSkel=streamShape.getShapePointer();
				}
				catch (Exception& e)
				{
					_SceneDlg->MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);

					// error
					skelError=true;
				}
			}
			else
			{
				// Create a message
				char msg[512];
				_snprintf (msg, 512, "Can't open the file %s for reading.", meshFilename);
				_SceneDlg->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);

				// error
				skelError=true;
			}
		}

		// Remove the mesh shape ?
		if (skelError)
		{
			if (shapeMesh)
				delete shapeMesh;
			shapeMesh=NULL;
		}
	}

	// Add the shape
	if (shapeMesh)
		addMesh (shapeMesh, shapeSkel, meshFilename, "");

	return false;
}

// ***************************************************************************

void CObjectViewer::resetCamera ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_SceneDlg->OnResetCamera();
}

// ***************************************************************************

CTransformShape	*CObjectViewer::addMesh (NL3D::IShape* pMeshShape, NL3D::IShape* pSkelShape, const char* name, const char *animBaseName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// *** Add the shape

	// Store the shape pointer
	CNELU::ShapeBank->add (name, CSmartPtr<IShape> (pMeshShape));

	// Store the name of the shape
	_ListShape.push_back (name);
	_ListShapeBaseName.push_back (animBaseName);

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

	// Return the instance
	return pTrShape;
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

void CObjectViewer::setAmbientColor (const NLMISC::CRGBA& color)
{
	CNELU::Driver->setAmbientColor (color);
}

// ***************************************************************************

void CObjectViewer::setLight (unsigned char id, const NL3D::CLight& light)
{
	CNELU::Driver->enableLight (id);
	CNELU::Driver->setLight (id, light);
}

// ***************************************************************************


