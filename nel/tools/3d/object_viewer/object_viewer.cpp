/** \file object_viewer.cpp
 * : Defines the initialization routines for the DLL.
 *
 * $Id: object_viewer.cpp,v 1.34 2001/09/04 16:19:34 vizerie Exp $
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

#undef OBJECT_VIEWER_EXPORT
#define OBJECT_VIEWER_EXPORT __declspec( dllexport ) 

#include "object_viewer.h"
#include <3d/nelu.h>
#include <3d/mesh.h>
#include <3d/transform_shape.h>
#include <3d/mesh_instance.h>
#include <3d/text_context.h>
#include <3d/skeleton_model.h>
#include <3d/init_3d.h>

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/config_file.h>

#include <nel/sound/u_audio_mixer.h>


#include "editable_range.h"
#include "range_manager.h"
#include "located_properties.h"
#include "color_button.h"
#include "particle_dlg.h"
#include "resource.h"
#include "main_frame.h"
#include "sound_system.h"



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

class CObjView : public CView
{
public:
	CObjView() 
	{
		MainFrame=NULL;
	};
	virtual ~CObjView() {};
	virtual void OnDraw (CDC *) {};
	afx_msg BOOL OnEraseBkgnd(CDC* pDC) 
	{ 
		return FALSE; 
	}
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if ((CNELU::Driver) && MainFrame)
			MainFrame->DriverWindowProc (CNELU::Driver, m_hWnd, message, wParam, lParam);
			
		return CView::WindowProc(message, wParam, lParam);
	}
	DECLARE_DYNCREATE(CObjView)
	CMainFrame	*MainFrame;
};

// ***************************************************************************

IMPLEMENT_DYNCREATE(CObjView, CView)

// ***************************************************************************

CObjectViewer::CObjectViewer ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	init3d ();

	_SlotDlg=NULL;
	_AnimationSetDlg=NULL;
	_AnimationDlg=NULL;
	_ParticleDlg = NULL ;
	_FontGenerator = NULL ;

	// Setup animation set
	_ChannelMixer.setAnimationSet (&_AnimationSet);

	// Hotspot color
	_HotSpotColor.R=255;
	_HotSpotColor.G=255;
	_HotSpotColor.B=0;
	_HotSpotColor.A=255;

	_BackGroundColor = CRGBA::Black ;

	// Hotspot size
	_HotSpotSize=10.f;

	// Charge l'object_viewer.ini
	try
	{
		// Get the module path
#ifdef NL_DEBUG
		HMODULE hModule = GetModuleHandle("object_viewer_debug.dll");
#elif defined (NL_RELEASE_DEBUG)
		HMODULE hModule = GetModuleHandle("object_viewer_rd.dll");
#else
		HMODULE hModule = GetModuleHandle("object_viewer.dll");
#endif
		nlassert (hModule);
		char sModulePath[256];
		char sDrive[256];
		char sDir[256];
		int res=GetModuleFileName(hModule, sModulePath, 256);
		nlassert(res);
		_splitpath (sModulePath, sDrive, sDir, NULL, NULL);
		_makepath (sModulePath, sDrive, sDir, "object_viewer", ".cfg");

		// Load the config file
		CConfigFile cf;
		cf.load (sModulePath);
		
		// Add search pathes
		CConfigFile::CVar &search_pathes = cf.getVar ("search_pathes");
		for (uint i=0; i<(uint)search_pathes.size(); i++)
			CPath::addSearchPath (search_pathes.asString(i));

		// set the sound file name
		try
		{
			CConfigFile::CVar &sound_file = cf.getVar("sound_file");
			CSoundSystem::setSoundBank(sound_file.asString().c_str());
		}
		catch (EUnknownVar &)
		{
			//::MessageBox(NULL, "warning : 'sound_file' variable not defined", "Objectviewer.cfg", MB_OK|MB_ICONEXCLAMATION);
		}

	}
	catch (Exception& e)
	{
		::MessageBox (NULL, e.what(), "Objectviewer.cfg", MB_OK|MB_ICONEXCLAMATION);
	}
}

// ***************************************************************************

CObjectViewer::~CObjectViewer ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_MainFrame)
		delete (_MainFrame);
	if (_SlotDlg)	
		delete _SlotDlg;
	if (_AnimationSetDlg)
		delete _AnimationSetDlg;
	if (_AnimationDlg)
		delete _AnimationDlg;
	if (_ParticleDlg)
		delete _ParticleDlg ;
	if (_FontGenerator)
		delete _FontGenerator ;
}

// ***************************************************************************

void initCamera ()
{
	// Camera
	CFrustum frustrum;
	uint32 width, height;
	CNELU::Driver->getWindowSize (width, height);
	frustrum.initPerspective( 75.f*(float)Pi/180.f, (float)width/(float)height, 0.1f, 1000.f);
	CNELU::Camera->setFrustum (frustrum);
}

// ***************************************************************************

void CObjectViewer::initUI (HWND parent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// init sound
	CSoundSystem::initSoundSystem();

	// The fonts manager
	_FontManager.setMaxMemory(2000000);

	// The windows path
	uint dSize = ::GetWindowsDirectory(NULL, 0) ;
	nlverify(dSize) ;
	char *wd = new char[dSize] ;	
	nlverify(::GetWindowsDirectory(wd, dSize)) ;
	_FontPath=wd;
	_FontPath+="\\fonts\\arial.ttf" ;

	// The font generator
	_FontGenerator = new NL3D::CFontGenerator ( _FontPath ) ;
	delete[] wd ;

	// The viewport
	CViewport viewport;

	// Create the icon
	HICON hIcon = (HICON)LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON,
		16, 16, 0);

	// Create a doomy driver
	IDriver *driver= CDRU::createGlDriver();

	// Get parent window
	CWnd parentWnd;
	CWnd *parentWndPtr=NULL;
	if (parent)
	{
		parentWnd.Attach (parent);
		parentWndPtr=&parentWnd;
	}

	// Create the main frame
	_MainFrame = new CMainFrame (this, (winProc) driver->getWindowProc());

	// Read register
	_MainFrame->registerValue (true);

	// Create the window
	_MainFrame->CFrameWnd::Create (AfxRegisterWndClass(0, 0, NULL, hIcon), 
		"NeL object viewer", 0x00cfc000, /*WS_OVERLAPPEDWINDOW,*/ CFrameWnd::rectDefault, parentWndPtr,
		MAKEINTRESOURCE(IDR_OBJECT_VIEWER_MENU), 0x00000300 /*WS_EX_ACCEPTFILES*/ /*|WS_EX_CLIENTEDGE*/);

	// Detach the hwnd
	parentWnd.Detach ();

	// Delete doomy driver
	delete driver;

	// Create a cwnd
	getRegisterWindowState (_MainFrame, REGKEY_OBJ_VIEW_OPENGL_WND, true);
	_MainFrame->ActivateFrame ();
	_MainFrame->ShowWindow (SW_SHOW);
	_MainFrame->UpdateWindow();

	// Context to open a view
	CCreateContext context;
	context.m_pCurrentDoc=NULL;
	context.m_pCurrentFrame=_MainFrame;
	context.m_pLastView=NULL;
	context.m_pNewDocTemplate=NULL;
	context.m_pNewViewClass=RUNTIME_CLASS(CObjView);

	// Create a view
	CObjView *view = (CObjView*)_MainFrame->CreateView (&context);
	view->ShowWindow (SW_SHOW);
	_MainFrame->SetActiveView(view);
	view = (CObjView*)_MainFrame->GetActiveView();
	view->MainFrame = _MainFrame;

	_MainFrame->ShowWindow (SW_SHOW);
	
	// Init NELU
	CNELU::init (640, 480, viewport, 32, true, view->m_hWnd);
	//CNELU::init (640, 480, viewport, 32, true, _MainFrame->m_hWnd);

	// Camera
	initCamera ();

	// Create animation set dialog
	_AnimationDlg=new CAnimationDlg (this, _MainFrame);
	_AnimationDlg->Create (IDD_ANIMATION);
	getRegisterWindowState (_AnimationDlg, REGKEY_OBJ_VIEW_ANIMATION_DLG, false);

	// Create animation set dialog
	_AnimationSetDlg=new CAnimationSetDlg (this, _MainFrame);
	_AnimationSetDlg->Create (IDD_ANIMATION_SET);
	getRegisterWindowState (_AnimationSetDlg, REGKEY_OBJ_VIEW_ANIMATION_SET_DLG, false);

	// Create the main dialog
	_SlotDlg=new CMainDlg (this, _MainFrame);
	_SlotDlg->init (&_AnimationSet);
	_SlotDlg->Create (IDD_MAIN_DLG);
	getRegisterWindowState (_SlotDlg, REGKEY_OBJ_VIEW_SLOT_DLG, false);

	// Create particle dialog
	_ParticleDlg=new CParticleDlg (this, _MainFrame, _MainFrame);
	_ParticleDlg->Create (IDD_PARTICLE);
	getRegisterWindowState (_ParticleDlg, REGKEY_OBJ_PARTICLE_DLG, false);

	// Set backgroupnd color
	setBackGroundColor(_MainFrame->BgColor);
	_MainFrame->update ();

	// Set current frame
	setAnimTime (0.f, 100.f);

	// Add mouse listener to event server
	_MouseListener.addToServer(CNELU::EventServer);
		
	CNELU::Driver->activate ();

	// Enable sum of vram
	CNELU::Driver->enableUsedTextureMemorySum ();
}

// ***************************************************************************

void CObjectViewer::go ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Text context to show infos
	CTextContext topInfo;
	topInfo.init (CNELU::Driver, &_FontManager);
	topInfo.setKeep800x600Ratio(false);
	topInfo.setFontGenerator (_FontPath);
	topInfo.setHotSpot (CComputedString::TopLeft);
	topInfo.setColor (CRGBA (255,255,255));
	topInfo.setFontSize (12);
	

	do
	{
		CNELU::Driver->activate ();

		

		// Handle animation
		_AnimationDlg->handle ();

		// Update the playlist
		_SlotDlg->getSlot ();

		// Setup the channel mixer
		_SlotDlg->Playlist.setupMixer (_ChannelMixer, _AnimationDlg->getTime());

		// Animate the automatic animation in the scene
		//CNELU::Scene.animate( (float) + NLMISC::CTime::ticksToSecond( NLMISC::CTime::getPerformanceTime() ) );

		CNELU::Scene.animate( (float) 0.001f * NLMISC::CTime::getLocalTime());

		// Eval channel mixer for transform
		_ChannelMixer.eval (false);



		// Clear the buffers


		CNELU::clearBuffers(_BackGroundColor);

		// Draw the scene		
		CNELU::Scene.render();		
		
		// call of callback list
		{
			std::vector<IMainLoopCallBack *> copyVect(_CallBackList.begin(), _CallBackList.end()) ;

			for (std::vector<IMainLoopCallBack *>::iterator it = _CallBackList.begin(); it != _CallBackList.end() ; ++it)
			{
				(*it)->go() ;
			}
		}

		// Profile polygon count
		CPrimitiveProfile in, out;
		CNELU::Driver->profileRenderedPrimitives (in, out);

		// Draw the hotSpot
		if (_MainFrame->MoveMode)
		{
			float radius=_HotSpotSize/2.f;
			CNELU::Driver->setupModelMatrix (CMatrix::Identity);
			CDRU::drawLine (_MouseListener.getHotSpot()+CVector (radius, 0, 0), _MouseListener.getHotSpot()+CVector (-radius, 0, 0), _HotSpotColor, *CNELU::Driver);
			CDRU::drawLine (_MouseListener.getHotSpot()+CVector (0, radius, 0), _MouseListener.getHotSpot()+CVector (0, -radius, 0), _HotSpotColor, *CNELU::Driver);
			CDRU::drawLine (_MouseListener.getHotSpot()+CVector (0, 0, radius), _MouseListener.getHotSpot()+CVector (0, 0, -radius), _HotSpotColor, *CNELU::Driver);
		}

		// Test some keys
		if (CNELU::AsyncListener.isKeyPushed(KeyF3))
		{
			// Change render mode
			switch (CNELU::Driver->getPolygonMode())
			{
			case IDriver::Filled:
				CNELU::Driver->setPolygonMode (IDriver::Line);
				break;
			case IDriver::Line:
				CNELU::Driver->setPolygonMode (IDriver::Point);
				break;
			case IDriver::Point:
				CNELU::Driver->setPolygonMode (IDriver::Filled);
				break;
			}
		}
		
		// Calc FPS
		static sint64 lastTime=NLMISC::CTime::getPerformanceTime ();
		sint64 newTime=NLMISC::CTime::getPerformanceTime ();
		float fps = (float)(1.0 / NLMISC::CTime::ticksToSecond (newTime-lastTime));
		lastTime=newTime;
		char msgBar[1024];
		uint nbPlayingSources, nbSources;
		if (CSoundSystem::getAudioMixer())
		{
			nbPlayingSources = CSoundSystem::getAudioMixer()->getPlayingSourcesNumber();
			nbSources = CSoundSystem::getAudioMixer()->getSourcesNumber();
		}
		else
		{
			nbPlayingSources = nbSources = NULL;
		}
														   
		sprintf (msgBar, "Nb tri: %d -Texture VRAM used (Mo): %5.2f -Texture VRAM allocated (Mo): %5.2f -Distance: %5.0f -Sounds: %d/%d -Fps: %03.1f",						 
						 in.NLines+in.NPoints+in.NQuads*2+in.NTriangles+in.NTriangleStrips, (float)CNELU::Driver->getUsedTextureMemory () / (float)(1024*1024), 
						 (float)CNELU::Driver->profileAllocatedTextureMemory () / (float)(1024*1024), 
						 (_SceneCenter-CNELU::Camera->getMatrix().getPos()).norm(),						 
						 nbPlayingSources,
						 nbSources,
						 fps
						 );
		_MainFrame->StatusBar.SetWindowText (msgBar);

	

		// Swap the buffers
		CNELU::swapBuffers();


		if (_MainFrame->MoveMode)
			_MouseListener.setMouseMode (CEvent3dMouseListener::edit3d);
		else
		{
			_MouseListener.setMouseMode (CEvent3dMouseListener::firstPerson);
			_MouseListener.setSpeed (_MainFrame->MoveSpeed);
		}

		// Reset camera aspect ratio
		initCamera ();

		if (!_MainFrame->MoveElement)
		{
			// New matrix from camera
			CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
			CNELU::Camera->setMatrix (_MouseListener.getViewMatrix());
		}
		else
		{
			// for now we apply a transform on the selected object in the particle system			
			_ParticleDlg->moveElement(_MouseListener.getModelMatrix()) ;		
		}

		// Pump message from the server
		CNELU::EventServer.pump();

		// Pump others message for the windows
		MSG	msg;
		while ( PeekMessage(&msg, NULL,0,0,PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		CSoundSystem::setListenerMatrix(_MouseListener.getViewMatrix());
		CSoundSystem::poll();

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
			setRegisterWindowState (_MainFrame, REGKEY_OBJ_VIEW_OPENGL_WND);
		}
	}

	// Write register
	_MainFrame->registerValue (false);

	// Release the emitter from the server
	_MouseListener.removeFromServer (CNELU::EventServer);

	// exit
	CNELU::release();

	// release sound
	CSoundSystem::releaseSoundSystem();
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
	int ver=f.serialVersion (2);

	// update data
	_AnimationDlg->UpdateData ();
	_MainFrame->UpdateData ();

	// serial animation data
	f.serial (_AnimationDlg->Start);
	f.serial (_AnimationDlg->End);
	f.serial (_AnimationDlg->Speed);
	bool loop=_AnimationDlg->Loop!=0;
	f.serial (loop);
	_AnimationDlg->Loop=loop;
	bool euler=_MainFrame->Euler!=0;
	f.serial (euler);
	_MainFrame->Euler=euler;
	sint32 ui=_AnimationDlg->UICurrentFrame;
	f.serial (ui);
	_AnimationDlg->UICurrentFrame=ui;
	f.serial (_AnimationDlg->CurrentFrame);

	// update data
	_AnimationDlg->UpdateData (FALSE);
	_MainFrame->UpdateData (FALSE);

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

	if (ver > 1)
	{	
		// serial the ranges for particles edition
		CRangeManager<float>::serial(f) ;
		CRangeManager<uint32>::serial(f) ;
		CRangeManager<sint32>::serial(f) ;
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
	if (skeleton)
	{
		_splitpath (skeleton, drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		CPath::addSearchPath (path);
	}

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
			_MainFrame->MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			return false;
		}
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", meshFilename);
		_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	// Continue ?
	if (skeleton&&(strcmp (skeleton, "")!=0))
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
					_MainFrame->MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);

					// error
					skelError=true;
				}
			}
			else
			{
				// Create a message
				char msg[512];
				_snprintf (msg, 512, "Can't open the file %s for reading.", meshFilename);
				_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);

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
			return false;
		}
	}

	// Add the skel shape
	NL3D::CSkeletonModel *transformSkel=NULL;
	if (shapeSkel)
		transformSkel=addSkel (shapeSkel, skeleton, "");

	// Add the skel shape
	if (shapeMesh)
		addMesh (shapeMesh, meshFilename, "", transformSkel);

	// Add an entry for config
	_ListMeshes.push_back (CMeshDesc (meshFilename, skeleton));

	return true;
}

// ***************************************************************************

void CObjectViewer::resetCamera ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_MainFrame->OnResetCamera();
}

// ***************************************************************************

CTransformShape	*CObjectViewer::addMesh (NL3D::IShape* pMeshShape, const char* meshName, const char *meshBaseName, CSkeletonModel* pSkel)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// *** Add the shape

	// Store the shape pointer
	CNELU::ShapeBank->add (meshName, CSmartPtr<IShape> (pMeshShape));

	// Store the name of the shape
	_ListShapeBaseName.push_back (meshBaseName);

	// Create a model and add it to the scene
	CTransformShape	*pTrShape=CNELU::Scene.createInstance (meshName);
	nlassert (pTrShape);

	// Set the rot model
	if (_MainFrame->Euler)
		pTrShape->setTransformMode (ITransformable::RotEuler);
	else
		pTrShape->setTransformMode (ITransformable::RotQuat);

	// Store the transform shape pointer
	_ListTransformShape.push_back (pTrShape);

	// *** Bind to the skeleton

	// Get a mesh instance
	CMeshBaseInstance  *meshInstance=dynamic_cast<CMeshBaseInstance*>(pTrShape);

	// Bind the mesh
	if (pSkel)
		pSkel->bindSkin (meshInstance);

	// Return the instance
	return pTrShape;
}

// ***************************************************************************

CSkeletonModel *CObjectViewer::addSkel (NL3D::IShape* pSkelShape, const char* skelName, const char *skelBaseName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// *** Add the shape

	// Store the shape pointer
	CNELU::ShapeBank->add (skelName, CSmartPtr<IShape> (pSkelShape));

	// Create a model and add it to the scene
	CTransformShape	*pTrShape=CNELU::Scene.createInstance (skelName);
	nlassert (pTrShape);

	// Get a skeleton model
	CSkeletonModel *skelModel=dynamic_cast<CSkeletonModel*>(pTrShape);

	// Is a skel ?
	if (skelModel)
	{
		// Set the rot model
		if (_MainFrame->Euler)
			pTrShape->setTransformMode (ITransformable::RotEuler);
		else
			pTrShape->setTransformMode (ITransformable::RotQuat);

		// Store the name of the shape
		_ListShapeBaseName.push_back (skelBaseName);

		// Store the transform shape pointer
		_ListTransformShape.push_back (skelModel);
	}

	// Return the instance
	return skelModel;
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

void CObjectViewer::setAutoAnimation (NL3D::CAnimation* pAnim)
{
	CNELU::Scene.setAutoAnim (pAnim);
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


/** add an object that will be notified each time a frame is processed
  * \see removeMainLoopCallBack()
  */
void CObjectViewer::registerMainLoopCallBack(IMainLoopCallBack *i)
{
//	nlassert(std::find(_CallBackList.begin(), _CallBackList.end(), i) == _CallBackList.begin()) ; // the object was register twice !!
	_CallBackList.push_back(i) ;
}

/// remove an object that was registered with registerMainLoopCallBack()
void CObjectViewer::removeMainLoopCallBack(IMainLoopCallBack *i)
{
	std::vector<IMainLoopCallBack *>::iterator it = std::find(_CallBackList.begin(), _CallBackList.end(), i) ;
	nlassert(it  != _CallBackList.end()) ; // this object wasn't registered
	_CallBackList.erase(it) ;	
}