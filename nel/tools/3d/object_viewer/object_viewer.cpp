/** \file object_viewer.cpp
 * : Defines the initialization routines for the DLL.
 *
 * $Id: object_viewer.cpp,v 1.57 2002/02/28 13:41:24 berenguier Exp $
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

#include <vector>


#include "object_viewer.h"

#include <3d/nelu.h>
#include <3d/mesh.h>
#include <3d/transform_shape.h>
#include <3d/mesh_instance.h>
#include <3d/text_context.h>
#include <3d/skeleton_model.h>
#include <3d/init_3d.h>
#include <3d/scene_group.h>
#include <3d/animation_playlist.h>
#include <3d/track_keyframer.h>
#include <3d/font_generator.h>



#include <nel/misc/common.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/config_file.h>

#include <nel/sound/u_audio_mixer.h>
#include <3d/water_pool_manager.h>
#include <3d/landscape_model.h>
#include <3d/visual_collision_manager.h>
#include <3d/visual_collision_entity.h>




#include "editable_range.h"
#include "range_manager.h"
#include "located_properties.h"
#include "color_button.h"
#include "particle_dlg.h"
#include "resource.h"
#include "main_frame.h"
#include "sound_system.h"
#include "scheme_manager.h"
#include "day_night_dlg.h"
#include "water_pool_editor.h"
#include "vegetable_dlg.h"
#include "dialog_progress.h"
#include "global_wind_dlg.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static char SDrive[256];
static char SDir[256];



using namespace std;
using namespace NL3D;
using namespace NLMISC;
using namespace NLSOUND;

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
	_ParticleDlg = NULL;
	_FontGenerator = NULL;
	_VegetableLandscape= NULL;
	_VegetableCollisionManager= NULL;
	_VegetableCollisionEntity= NULL;
	_CameraFocal = 75.f; // default value for the focal

	// no lag is the default
	_Lag = 0;

	// Hotspot color
	_HotSpotColor.R=255;
	_HotSpotColor.G=255;
	_HotSpotColor.B=0;
	_HotSpotColor.A=255;

	_BackGroundColor = CRGBA::Black;

	// Hotspot size
	_HotSpotSize=10.f;

	_Wpm = &NL3D::GetWaterPoolManager();

	// Charge l'object_viewer.ini
	try
	{
		// Get the module path
// must test it first, because NL_DEBUG_FAST and NL_DEBUG are declared at same time.
#ifdef NL_DEBUG_FAST
		HMODULE hModule = GetModuleHandle("object_viewer_debug_fast.dll");
#elif defined (NL_DEBUG)
		HMODULE hModule = GetModuleHandle("object_viewer_debug.dll");
#elif defined (NL_RELEASE_DEBUG)
		HMODULE hModule = GetModuleHandle("object_viewer_rd.dll");
#else
		HMODULE hModule = GetModuleHandle("object_viewer.dll");
#endif
		nlassert (hModule);		
		char sModulePath[256];
		int res=GetModuleFileName(hModule, sModulePath, 256);
		nlassert(res);
		_splitpath (sModulePath, SDrive, SDir, NULL, NULL);
		_makepath (sModulePath, SDrive, SDir, "object_viewer", ".cfg");

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
			for (uint i=0; i<(uint)sound_file.size(); i++)
				CSoundSystem::addSoundBank(sound_file.asString(i).c_str());
		}
		catch (EUnknownVar &)
		{
			//::MessageBox(NULL, "warning : 'sound_file' variable not defined", "Objectviewer.cfg", MB_OK|MB_ICONEXCLAMATION);
		}

		// load the camera focal
		try
		{
			CConfigFile::CVar &camera_focal = cf.getVar("camera_focal");
			_CameraFocal = camera_focal.asFloat();
		}
		catch (EUnknownVar &)
		{
		}


		// load Scene light setup.
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_enabled");
			_SceneLightEnabled = var.asInt() !=0 ;
		}
		catch (EUnknownVar &)
		{
			_SceneLightEnabled= false;
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_sun_ambiant");
			_SceneLightSunAmbiant.R = var.asInt(0);
			_SceneLightSunAmbiant.G = var.asInt(1);
			_SceneLightSunAmbiant.B = var.asInt(2);
		}
		catch (EUnknownVar &)
		{
			_SceneLightSunAmbiant= NLMISC::CRGBA::Black;
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_sun_diffuse");
			_SceneLightSunDiffuse.R = var.asInt(0);
			_SceneLightSunDiffuse.G = var.asInt(1);
			_SceneLightSunDiffuse.B = var.asInt(2);
		}
		catch (EUnknownVar &)
		{
			_SceneLightSunDiffuse= NLMISC::CRGBA::White;
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_sun_specular");
			_SceneLightSunSpecular.R = var.asInt(0);
			_SceneLightSunSpecular.G = var.asInt(1);
			_SceneLightSunSpecular.B = var.asInt(2);
		}
		catch (EUnknownVar &)
		{
			_SceneLightSunSpecular= NLMISC::CRGBA::White;
		}
		try
		{
			CConfigFile::CVar &var = cf.getVar("scene_light_sun_dir");
			_SceneLightSunDir.x = var.asFloat(0);
			_SceneLightSunDir.y = var.asFloat(1);
			_SceneLightSunDir.z = var.asFloat(2);
			_SceneLightSunDir.normalize();
		}
		catch (EUnknownVar &)
		{
			_SceneLightSunDir.set(0, 1, -1);
			_SceneLightSunDir.normalize();
		}



		// Load vegetable Landscape cfg.
		loadVegetableLandscapeCfg(cf);


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
		delete _ParticleDlg;
	if (_DayNightDlg)
		delete _DayNightDlg;
	if (_WaterPoolDlg)
		delete _WaterPoolDlg;
	if (_VegetableDlg)
		delete _VegetableDlg;
	if (_GlobalWindDlg)
		delete _GlobalWindDlg;
	if (_FontGenerator)
		delete _FontGenerator;
}

// ***************************************************************************

void initCamera (float focal)
{
	// Camera
	CFrustum frustrum;
	uint32 width, height;
	CNELU::Driver->getWindowSize (width, height);
	frustrum.initPerspective( focal *(float)Pi/180.f, (float)width/(float)height, 0.1f, 1000.f);
	CNELU::Camera->setFrustum (frustrum);
}

// ***************************************************************************

void CObjectViewer::initUI (HWND parent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// init sound
	CSoundSystem::initSoundSystem ();

	// The fonts manager
	_FontManager.setMaxMemory(2000000);

	// The windows path
	uint dSize = ::GetWindowsDirectory(NULL, 0);
	nlverify(dSize);
	char *wd = new char[dSize];	
	nlverify(::GetWindowsDirectory(wd, dSize));
	_FontPath=wd;
	_FontPath+="\\fonts\\arial.ttf";

	// The font generator
	_FontGenerator = new NL3D::CFontGenerator ( _FontPath );
	delete[] wd;

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

	// Init default lighting seutp.
	setupSceneLightingSystem(_SceneLightEnabled, _SceneLightSunDir, _SceneLightSunAmbiant, _SceneLightSunDiffuse, _SceneLightSunSpecular);

	// Camera
	initCamera (_CameraFocal);

	_MainFrame->OnResetCamera();

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

	// Create water pool editor dialog
	_WaterPoolDlg = new CWaterPoolEditor(_Wpm, _MainFrame);
	_WaterPoolDlg->Create (IDD_WATER_POOL);
	getRegisterWindowState (_WaterPoolDlg, REGKEY_OBJ_WATERPOOL_DLG, false);


	// Create day night dialog
	_DayNightDlg = new CDayNightDlg (this, _MainFrame);
	_DayNightDlg->Create (IDD_DAYNIGHT);
	getRegisterWindowState (_DayNightDlg, REGKEY_OBJ_DAYNIGHT_DLG, false);


	// Create vegetable dialog
	_VegetableDlg=new CVegetableDlg (this, _MainFrame);
	_VegetableDlg->Create (IDD_VEGETABLE_DLG);
	getRegisterWindowState (_VegetableDlg, REGKEY_OBJ_VIEW_VEGETABLE_DLG, false);


	// Create global wind dialog
	_GlobalWindDlg= new CGlobalWindDlg (this, _MainFrame);
	_GlobalWindDlg->Create(IDD_GLOBAL_WIND);
	getRegisterWindowState (_GlobalWindDlg, REGKEY_OBJ_GLOBAL_WIND_DLG, false);


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

	char sModulePath[256];
	// load the scheme bank if one is present		
	CIFile iF;
	::_makepath (sModulePath, SDrive, SDir, "default", ".scb");		
	if (iF.open(sModulePath))
	{
		try
		{
			iF.serial(SchemeManager);
		}
		catch (NLMISC::EStream &e)
		{
			::MessageBox(NULL, ("Unable to load the default scheme bank file : "  + std::string(e.what())).c_str(), "Object Viewer", MB_ICONEXCLAMATION);
		}
	}
	iF.close();
	
	// try to load a default config file for the viewer (for anitmation and particle edition setup)
	::_makepath (sModulePath, SDrive, SDir, "default", ".ovcgf");
	if (iF.open (sModulePath))
	{
		try
		{
			serial (iF);
		}
		catch (Exception& e)
		{
			::MessageBox (NULL, (std::string("error while loading default.ovcgf : ") + e.what()).c_str(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}

}

// ***************************************************************************

void CObjectViewer::addTransformation (CMatrix &current, CAnimation *anim, float begin, float end, ITrack *posTrack, ITrack *rotquatTrack, 
									   ITrack *nextPosTrack, ITrack *nextRotquatTrack, bool removeLast)
{
	// In place ?
	if (_AnimationDlg->Inplace)
	{
		// Just identity
		current.identity();
	}
	else
	{
		// Remove the start of the animation
		CQuat rotEnd (0,0,0,1);
		CVector posEnd (0,0,0);
		if (rotquatTrack)
		{
			// Interpolate the rotation
			rotquatTrack->interpolate (end, rotEnd);
		}
		if (posTrack)
		{
			// Interpolate the position
			posTrack->interpolate (end, posEnd);
		}

		// Add the final rotation and position
		CMatrix tmp;
		tmp.identity ();
		tmp.setRot (rotEnd);
		tmp.setPos (posEnd);

		// Incremental ?
		if (_AnimationDlg->IncPos)
			current *= tmp;
		else
			current = tmp;

		if (removeLast)
		{
			CQuat rotStart (0,0,0,1);
			CVector posStart (0,0,0);
			if (nextRotquatTrack)
			{
				// Interpolate the rotation
				nextRotquatTrack->interpolate (begin, rotStart);
			}
			if (nextPosTrack)
			{
				// Interpolate the position
				nextPosTrack->interpolate (begin, posStart);
			}
			// Remove the init rotation and position of the next animation
			tmp.identity ();
			tmp.setRot (rotStart);
			tmp.setPos (posStart);
			tmp.invert ();
			current *= tmp;

			// Normalize the mt
			CVector I = current.getI ();
			CVector J = current.getJ ();
			I.z = 0;
			J.z = 0;
			J.normalize ();
			CVector K = I^J;
			K.normalize ();
			I = J^K;
			I.normalize ();
			tmp.setRot (I, J, K);
			tmp.setPos (current.getPos ());
			current = tmp;
		}
	}
}

// ***************************************************************************

void CObjectViewer::setupPlaylist (float time)
{
	// Update animation dlg

	// Some animation in the list ?
	if (_AnimationSetDlg->PlayList.GetCount()>0)
	{
		// Gor each object
		uint i;
		for (i=0; i<_ChannelMixer.size(); i++)
		{
			// A playlist
			CAnimationPlaylist playlist;

			// Empty with playlist
			uint j;
			for (j=0; j<CChannelMixer::NumAnimationSlot; j++)
			{
				// Empty slot
				playlist.setAnimation (j, CAnimationPlaylist::empty);
			}

			// Index choosed
			uint choosedIndex = 0xffffffff;

			// Track here
			bool there = false;

			// Current matrix
			CMatrix	current;
			current.identity ();

			// Current animation
			CAnimation *anim=_AnimationSet.getAnimation (_AnimationSetDlg->PlayList.GetItemData (0));
			ITrack *posTrack = (ITrack *)anim->getTrackByName ((_ListInstance[i].ShapeBaseName+"pos").c_str());
			ITrack *rotquatTrack = (ITrack *)anim->getTrackByName ((_ListInstance[i].ShapeBaseName+"rotquat").c_str());
			there = posTrack || rotquatTrack;

			// Accumul time
			float startTime=0;
			float endTime=anim->getEndTime()-anim->getBeginTime();

			// Animation index
			int index = 0;

			// Get animation used in the list
			while (time>=endTime)
			{
				// Next animation
				index++;
				if (index<_AnimationSetDlg->PlayList.GetCount())
				{
					// Pointer on the animation
					CAnimation *newAnim=_AnimationSet.getAnimation (_AnimationSetDlg->PlayList.GetItemData (index));
					ITrack *newPosTrack = (ITrack *)newAnim->getTrackByName ((_ListInstance[i].ShapeBaseName+"pos").c_str());
					ITrack *newRotquatTrack = (ITrack *)newAnim->getTrackByName ((_ListInstance[i].ShapeBaseName+"rotquat").c_str());

					// Add the transformation
					addTransformation (current, anim, newAnim->getBeginTime(), anim->getEndTime(), posTrack, rotquatTrack, newPosTrack, newRotquatTrack, true);

					// Pointer on the animation
					anim = newAnim;
					posTrack = newPosTrack;
					rotquatTrack = newRotquatTrack;

					// Add start time
					startTime = endTime;
					endTime = startTime + anim->getEndTime()-anim->getBeginTime();

				}
				else
				{
					// Add the transformation
					addTransformation (current, anim, 0, anim->getEndTime(), posTrack, rotquatTrack, NULL, NULL, false);

					break;
				}
			}

			// Time cropped ?
			if (index>=_AnimationSetDlg->PlayList.GetCount())
			{
				// Yes
				index--;

				// Good index
				choosedIndex = _AnimationSetDlg->PlayList.GetItemData (index);
				anim=_AnimationSet.getAnimation (choosedIndex);

				// End time for last anim
				startTime = anim->getEndTime () - time;
			}
			else
			{
				// No

				// Add the transformation
				addTransformation (current, anim, 0, anim->getBeginTime() + time - startTime, posTrack, rotquatTrack, NULL, NULL, false);

				// Good index
				choosedIndex = _AnimationSetDlg->PlayList.GetItemData (index);

				// Get the animation
				anim=_AnimationSet.getAnimation (choosedIndex);

				// Final time
				startTime -= anim->getBeginTime ();
			}

			// Set the slot		
			playlist.setTimeOrigin (0, startTime);
			playlist.setWrapMode (0, CAnimationPlaylist::Clamp);
			playlist.setStartWeight (0, 1, 0);
			playlist.setEndWeight (0, 1, 1);
			playlist.setAnimation (0, choosedIndex);

			// Setup the channel
			playlist.setupMixer (_ChannelMixer[i], _AnimationDlg->getTime());

			// Setup the pos and rot for this shape
			if (there)
			{
				_ListInstance[i].TransformShape->setPos (current.getPos());
				_ListInstance[i].TransformShape->setRotQuat (current.getRot());
			}
		}
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
		_AnimationSetDlg->UpdateData ();
		if (_AnimationSetDlg->UseMixer)
		{
			// For each channel mixer
			for (uint i=0; i<_ChannelMixer.size(); i++)
			{
				_SlotDlg->Playlist.setupMixer (_ChannelMixer[i], _AnimationDlg->getTime());
			}
		}
		else
			setupPlaylist (_AnimationDlg->getTime());

		// Eval sound tracks
		evalSoundTrack (_AnimationDlg->getLastTime(), _AnimationDlg->getTime());

		// Animate the automatic animation in the scene
		//CNELU::Scene.animate( (float) + NLMISC::CTime::ticksToSecond( NLMISC::CTime::getPerformanceTime() ) );

		CNELU::Scene.animate( (float) 0.001f * NLMISC::CTime::getLocalTime());

		// Eval channel mixer for transform
		for (uint i=0; i<_ChannelMixer.size(); i++)
			_ChannelMixer[i].eval (false);

		// Clear the buffers


		CNELU::clearBuffers(_BackGroundColor);

		// Draw the scene		
		CNELU::Scene.render();		
		
		// call of callback list
		{
			std::vector<IMainLoopCallBack *> copyVect(_CallBackList.begin(), _CallBackList.end());

			for (std::vector<IMainLoopCallBack *>::iterator it = _CallBackList.begin(); it != _CallBackList.end(); ++it)
			{
				(*it)->go();
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

		// Display std info.
		sprintf (msgBar, "Nb tri: %d -Texture VRAM used (Mo): %5.2f -Texture VRAM allocated (Mo): %5.2f -Distance: %5.0f -Sounds: %d/%d -Fps: %03.1f",						 
						 in.NLines+in.NPoints+in.NQuads*2+in.NTriangles+in.NTriangleStrips, (float)CNELU::Driver->getUsedTextureMemory () / (float)(1024*1024), 
						 (float)CNELU::Driver->profileAllocatedTextureMemory () / (float)(1024*1024), 
						 (_SceneCenter-CNELU::Camera->getMatrix().getPos()).norm(),						 
						 nbPlayingSources,
						 nbSources,
						 fps
						 );
		// Display
		_MainFrame->StatusBar.SetWindowText (msgBar);

		// Display Vegetable info.
		if(_VegetableDlg!=NULL)
		{
			if(_VegetableLandscape != NULL)
			{
				char vegetMsgBar[1024];
				sprintf (vegetMsgBar, "%d", _VegetableLandscape->Landscape.getNumVegetableFaceRendered());
				_VegetableDlg->StaticPolyCount.SetWindowText(vegetMsgBar);
			}
			else
			{
				_VegetableDlg->StaticPolyCount.SetWindowText("0");
			}
		}

	

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
		initCamera (_CameraFocal);

		if (!_MainFrame->MoveElement)
		{
			// New matrix from camera
			CNELU::Camera->setTransformMode (ITransformable::DirectMatrix);
			CNELU::Camera->setMatrix (_MouseListener.getViewMatrix());

			// Vegetable: manage collision snapping if wanted and possible
			if(_VegetableSnapToGround && _VegetableLandscape)
			{
				// get matrix from camera.
				CMatrix	matrix= CNELU::Camera->getMatrix();
				// snap To ground.
				CVector	pos= matrix.getPos();
				// if succes to snap to ground
				if(_VegetableCollisionEntity->snapToGround(pos))
				{
					pos.z+= _VegetableSnapHeight;
					matrix.setPos(pos);
					// reset the moveListener and the camera.
					_MouseListener.setMatrix(matrix);
					CNELU::Camera->setMatrix(matrix);
				}
			}
		}
		else
		{
			// for now we apply a transform on the selected object in the particle system			
			_ParticleDlg->moveElement(_MouseListener.getModelMatrix());		
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


		// simulate lag
		if (_Lag)
		{
			NLMISC::nlSleep(_Lag);
		}


		// Save last time
		_LastTime=_AnimationDlg->getTime();
	}
	while (!CNELU::AsyncListener.isKeyPushed(KeyESCAPE)&&CNELU::Driver->isActive());
}

// ***************************************************************************

void CObjectViewer::releaseUI ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// release sound
	CSoundSystem::releaseSoundSystem();

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

	// remove first possibly created collisions objects.
	if(_VegetableCollisionEntity)
	{
		_VegetableCollisionManager->deleteEntity(_VegetableCollisionEntity);
		_VegetableCollisionEntity= NULL;
	}
	if(_VegetableCollisionManager)
	{
		delete _VegetableCollisionManager;
		_VegetableCollisionManager= NULL;
	}

	// delete Landscape
	if(_VegetableLandscape)
	{
		CNELU::Scene.deleteModel(_VegetableLandscape);
		_VegetableLandscape= NULL;
	}

	// Release all instances and all Igs.
	removeAllInstancesFromScene();

	// release other 3D.
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
	// Reset the channel mixxer array
	_ChannelMixer.resize (_ListInstance.size());

	// Add all the instance in the channel mixer
	for (uint i=0; i<_ListInstance.size(); i++)
	{
		// Reset the channels
		_ChannelMixer[i].resetChannels ();

		// Setup animation set
		_ChannelMixer[i].setAnimationSet (&_AnimationSet);

		// Register the transform
		_ListInstance[i].TransformShape->registerToChannelMixer (&(_ChannelMixer[i]), _ListInstance[i].ShapeBaseName);
	}

	// Enable / disable channels
	enableChannels ();
}

// ***************************************************************************

void CObjectViewer::enableChannels ()
{
	// Disable some channels
	_AnimationSetDlg->UpdateData ();
	bool enable = (_AnimationSetDlg->UseMixer == 1);

	// Add all the instance in the channel mixer
	for (uint i=0; i<_ListInstance.size(); i++)
	{
		// Get the base name
		std::string &baseName = _ListInstance[i].ShapeBaseName;

		// Get the pos and rot channel id
		uint posId = _AnimationSet.getChannelIdByName (baseName+"pos");
		uint rotQuatId = _AnimationSet.getChannelIdByName (baseName+"rotquat");
		uint rotEulerId = _AnimationSet.getChannelIdByName (baseName+"roteuler");

		if (posId != CAnimationSet::NotFound)
			_ChannelMixer[i].enableChannel (posId, enable);
		if (rotQuatId != CAnimationSet::NotFound)
			_ChannelMixer[i].enableChannel (rotQuatId, enable);
		if (rotEulerId != CAnimationSet::NotFound)
			_ChannelMixer[i].enableChannel (rotEulerId, enable);
	}
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
		{
			loadMesh (meshArray[s].MeshNames, meshArray[s].SkeletonName.c_str());
		}
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
		CRangeManager<float>::serial(f);
		CRangeManager<uint32>::serial(f);
		CRangeManager<sint32>::serial(f);
	}
}

// ***************************************************************************

bool CObjectViewer::loadInstanceGroup(const char *igFilename)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add to the path
	char drive[256];
	char dir[256];
	char path[256];

	// Add search path for the mesh
	_splitpath (igFilename, drive, dir, NULL, NULL);
	_makepath (path, drive, dir, NULL, NULL);
	CPath::addSearchPath (path);

	
	// Open a file
	CIFile file;
	if (file.open (igFilename))
	{		
		// Shape pointer
		NL3D::CInstanceGroup	*ig= new NL3D::CInstanceGroup;	

		try
		{
			// Stream it
			file.serial(*ig);

			// Append the ig.
			addInstanceGroup(ig);
		}
		catch (Exception& e)
		{
			// clean
			delete ig;
			_MainFrame->MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			return false;
		}
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", igFilename);
		_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	

	return true;		
}



// ***************************************************************************

bool CObjectViewer::loadMesh (std::vector<std::string> &meshFilename, const char* skeleton)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Add to the path
	char drive[256];
	char dir[256];
	char path[256];

	// Add search path for the skeleton
	if (skeleton)
	{
		_splitpath (skeleton, drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		CPath::addSearchPath (path);
	}

	// Open a file
	CIFile file;

	// Shape pointer
	IShape *shapeSkel=NULL;
	NL3D::CSkeletonModel *transformSkel=NULL;

	// Skel error ?
	bool skelError=false;

	// Continue ?
	if (skeleton&&(strcmp (skeleton, "")!=0))
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

	// Skeleton error ?
	if (skelError)
		return false;

	// Skeleton used ?
	bool skelUsed = false;

	// For each meshes
	for (uint i=0; i<meshFilename.size(); i++)
	{
		// Filename
		const char *fileName = meshFilename[i].c_str();

		// Add search path for the mesh
		_splitpath (fileName, drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		CPath::addSearchPath (path);

		// Shape pointer
		IShape *shapeMesh=NULL;

		if (file.open (fileName))
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
				continue;
			}
		}
		else
		{
			// Create a message
			char msg[512];
			_snprintf (msg, 512, "Can't open the file %s for reading.", fileName);
			_MainFrame->MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			continue;
		}

		// Add the skel shape
		if (shapeSkel&&(!skelUsed))
		{
			// Add the skel
			transformSkel=addSkel (shapeSkel, skeleton, "");
			skelUsed = true;
		}

		// Add the skel shape
		if (shapeMesh)
			addMesh (shapeMesh, fileName, "", transformSkel);
	}

	// Skel not used ?
	if ((!skelUsed)&&shapeSkel)
	{
		// Remove it
		delete shapeSkel;
	}

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

CTransformShape	*CObjectViewer::addMesh (NL3D::IShape* pMeshShape, const char* meshName, const char *meshBaseName, CSkeletonModel* pSkel, bool createInstance)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// *** Add the shape

	// Store the shape pointer
	CNELU::ShapeBank->add (meshName, CSmartPtr<IShape> (pMeshShape));

	// Must create the instance?
	if(createInstance)
	{
		// Create a model and add it to the scene
		CTransformShape	*pTrShape=CNELU::Scene.createInstance (meshName);
		nlassert (pTrShape);

		// Set the rot model
		if (_MainFrame->Euler)
			pTrShape->setTransformMode (ITransformable::RotEuler);
		else
			pTrShape->setTransformMode (ITransformable::RotQuat);

		// Store the transform shape pointer
		CInstanceInfo	iInfo;
		iInfo.TransformShape= pTrShape;
		// Store the name of the shape
		iInfo.ShapeBaseName= meshBaseName;
		iInfo.MustDelete= true;
		_ListInstance.push_back (iInfo);	


		// *** Bind to the skeleton

		// Get a mesh instance
		CMeshBaseInstance  *meshInstance=dynamic_cast<CMeshBaseInstance*>(pTrShape);

		// Bind the mesh
		if (pSkel)
			pSkel->bindSkin (meshInstance);

		// Return the instance
		return pTrShape;
	}
	else
		return NULL;
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

		// Store the transform shape pointer
		CInstanceInfo	iInfo;
		iInfo.TransformShape= skelModel;
		// Store the name of the shape
		iInfo.ShapeBaseName= skelBaseName;
		iInfo.MustDelete= true;
		_ListInstance.push_back (iInfo);	
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
	_AnimationSetDlg->UseMixer = 1;
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

	// Setup also Scene lighting system here, even if not used.
	CNELU::Scene.setAmbientGlobal(color);
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
//	nlassert(std::find(_CallBackList.begin(), _CallBackList.end(), i) == _CallBackList.begin()); // the object was register twice !!
	_CallBackList.push_back(i);
}

/// remove an object that was registered with registerMainLoopCallBack()
void CObjectViewer::removeMainLoopCallBack(IMainLoopCallBack *i)
{
	std::vector<IMainLoopCallBack *>::iterator it = std::find(_CallBackList.begin(), _CallBackList.end(), i);
	nlassert(it  != _CallBackList.end()); // this object wasn't registered
	_CallBackList.erase(it);	
}

// ***************************************************************************
void CObjectViewer::activateTextureSet(uint index)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	std::vector<CInstanceInfo>::iterator it;
	for (it = _ListInstance.begin(); it != _ListInstance.end(); ++it)
	{
		NL3D::CTransformShape	*trShape= it->TransformShape;
		if (dynamic_cast<NL3D::CMeshBaseInstance *>(trShape))
		{
			static_cast<NL3D::CMeshBaseInstance *>(trShape)->selectTextureSet(index);
		}		
	}	
}

// ***************************************************************************

void CObjectViewer::removeAllInstancesFromScene()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Remove all stand alone TransformShapes.
	std::vector<CInstanceInfo>::iterator it;
	for (it = _ListInstance.begin(); it != _ListInstance.end(); ++it)
	{
		if(it->MustDelete)
			CNELU::Scene.deleteInstance(it->TransformShape);
	}
	_ListInstance.clear();

	// Remove added/loaded igs and their instances.
	for(uint igId=0; igId<_ListIG.size(); igId++)
	{
		// remove instances.
		_ListIG[igId]->removeFromScene(CNELU::Scene);
		// free up the ig.
		delete _ListIG[igId];
	}
	_ListIG.clear();

}


// ***************************************************************************
void CObjectViewer::evalSoundTrack (float lastTime, float currentTime)
{
	if (lastTime!=currentTime)
	{
		// For each channel of the mixer
		for (uint slot=0; slot<CChannelMixer::NumAnimationSlot; slot++)
		{
			// Anim id
			uint animId=_SlotDlg->Playlist.getAnimation (slot);

			// Channel actif ?
			if (_SlotDlg->Playlist.getAnimation (slot)!=CAnimationPlaylist::empty)
			{
				// Get the animation
				CAnimation *anim=_AnimationSet.getAnimation (animId);
				nlassert (anim);

				// Get the sound track
				uint trackId=anim->getIdTrackByName ("NoteTrack");
				if (trackId!=CAnimation::NotFound)
				{
					// Get the track
					ITrack *track=anim->getTrack (trackId);
					nlassert (track);

					// Dynamic cast
					UTrackKeyframer *soundTrackKF = dynamic_cast<UTrackKeyframer *>(track);
					if (soundTrackKF)
					{
						// Sound keys
						std::vector<TAnimationTime> result;

						// Get local begin and endTime
						TAnimationTime localLastTime = _SlotDlg->Playlist.getLocalTime (slot, lastTime, _AnimationSet);
						TAnimationTime localCurrentTime = _SlotDlg->Playlist.getLocalTime (slot, currentTime, _AnimationSet);

						// Good interval
						if (localLastTime<=localCurrentTime)
						{
							// Get keys in this interval
							soundTrackKF->getKeysInRange(localLastTime, localCurrentTime, result);
						}
						else
						{
							// Get begin and last time
							TAnimationTime beginTime=track->getBeginTime ();
							TAnimationTime endTime=track->getEndTime ();

							// Time must have been clamped
							nlassert (localCurrentTime<=endTime);
							nlassert (localLastTime>=beginTime);

							// Get keys to the end
							soundTrackKF->getKeysInRange(localCurrentTime, endTime, result);

							// Get keys at the beginning
							soundTrackKF->getKeysInRange(beginTime, localLastTime, result);
						}

						// Process sounds
						NLSOUND::UAudioMixer *audioMixer = CSoundSystem::getAudioMixer ();
						if( audioMixer )
						{	
							vector<TAnimationTime>::iterator itResult;
							for( itResult = result.begin(); itResult != result.end(); ++itResult ) 
							{
								string soundName;
								double keyTime = *itResult;
								nlinfo("keyTime = %f  result size : %d",*itResult,result.size());
								
								if( !track->interpolate( *itResult, soundName) )
								{
									nlwarning("The key at offset %f is not a string",*itResult);
								}
								else
								{
									// if there are step sounds
									if( soundName == "step" )
									{
 										// need to spawn a sound linked to the anim
										string dummySound = "PAShommecourseappartdur1a";
										USource *source = audioMixer->createSource (dummySound.c_str() , true );
										if (source)
										{
											source->setPos (CVector::Null);
											source->play ();
 											nlinfo ("launching dummy sound %s for the step event", dummySound.c_str());
										}
										else
										{
	 										nlwarning ("sound not found for the step event: '%s'", dummySound.c_str());
										}
									}
 									else if (soundName.find ("snd_") != string::npos)
 									{
 										// need to spawn a sound linked to the anim
										USource *source = audioMixer->createSource ( soundName.c_str(), true );
										if (source)
										{
											source->setPos (CVector::Null);
											source->play ();
 											nlinfo ("launching sound for anim event from notetrack '%s'", soundName.c_str());
										}
										else
										{
	 										nlwarning ("sound not found: '%s'", soundName.c_str());
										}
 									}
 									else
 									{
 										nlwarning ("unknown notetrack event: '%s'", soundName.c_str());
 									}
								}
							}
						}
					}
				}
			}
		}
	}
}



// ***************************************************************************
void CObjectViewer::addInstanceGroup(NL3D::CInstanceGroup *ig)
{
	// Add all models to the scene		
	ig->addToScene(CNELU::Scene, CNELU::Driver);
	// Unfreeze all objects from HRC.
	ig->unfreezeHRC();

	// Keep a reference on them, but they'll be destroyed by IG.
	for (uint k = 0; k < ig->getNumInstance(); ++k)
	{
		CInstanceInfo	iInfo;
		iInfo.TransformShape= ig->_Instances[k];
		iInfo.ShapeBaseName= ig->getInstanceName(k) + ".";
		iInfo.MustDelete= false;
		_ListInstance.push_back (iInfo);	
	}

	// Add the ig to the list.
	_ListIG.push_back(ig);
}

// ***************************************************************************
void CObjectViewer::setupSceneLightingSystem(bool enable, const NLMISC::CVector &sunDir, NLMISC::CRGBA sunAmbiant, NLMISC::CRGBA sunDiffuse, NLMISC::CRGBA sunSpecular)
{
	CNELU::Scene.enableLightingSystem(enable);

	// Setup sun.
	CNELU::Scene.setSunAmbient(sunAmbiant);
	CNELU::Scene.setSunDiffuse(sunDiffuse);
	CNELU::Scene.setSunSpecular(sunSpecular);
	CNELU::Scene.setSunDirection(sunDir);
}



// ***************************************************************************
// ***************************************************************************
// Vegetable Landscape Part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CObjectViewer::loadVegetableLandscapeCfg(NLMISC::CConfigFile &cf)
{
	// vegetable display is true by default.
	_VegetableEnabled= true;
	_VegetableSnapToGround= true;


	// Load landscape setup
	// --------------
	try
	{
		// tileBank setup.
		CConfigFile::CVar &tileBank = cf.getVar("veget_tile_bank");
		_VegetableLandscapeTileBank= tileBank.asString();
		CConfigFile::CVar &tileFarBank = cf.getVar("veget_tile_far_bank");
		_VegetableLandscapeTileFarBank= tileFarBank.asString();
		// zone list.
		_VegetableLandscapeZoneNames.clear();
		CConfigFile::CVar &zones = cf.getVar("veget_landscape_zones");
		for (uint i=0; i<(uint)zones.size(); i++)
			_VegetableLandscapeZoneNames.push_back(zones.asString(i).c_str());
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeTileBank.clear();
		_VegetableLandscapeTileFarBank.clear();
		_VegetableLandscapeZoneNames.clear();
	}


	// Load Landscape params.
	// --------------
	// threshold
	try
	{
		CConfigFile::CVar &thre= cf.getVar("veget_landscape_threshold");
		_VegetableLandscapeThreshold= thre.asFloat();
		// clamp to avoid divide/0.
		_VegetableLandscapeThreshold= max(_VegetableLandscapeThreshold, 0.001f);
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeThreshold= 0.003f;
	}
	// tilenear
	try
	{
		CConfigFile::CVar &tileNear= cf.getVar("veget_landscape_tile_near");
		_VegetableLandscapeTileNear= tileNear.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeTileNear= 50;
	}
	// ambient
	try
	{
		CConfigFile::CVar &color= cf.getVar("veget_landscape_ambient");
		_VegetableLandscapeAmbient.R= color.asInt(0);
		_VegetableLandscapeAmbient.G= color.asInt(1);
		_VegetableLandscapeAmbient.B= color.asInt(2);
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeAmbient.set(80, 80, 80);
	}
	// diffuse
	try
	{
		CConfigFile::CVar &color= cf.getVar("veget_landscape_diffuse");
		_VegetableLandscapeDiffuse.R= color.asInt(0);
		_VegetableLandscapeDiffuse.G= color.asInt(1);
		_VegetableLandscapeDiffuse.B= color.asInt(2);
	}
	catch (EUnknownVar &)
	{
		_VegetableLandscapeDiffuse.set(255, 255, 255);
	}
	// Snapping
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_landscape_snap_height");
		_VegetableSnapHeight= var.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableSnapHeight= 1.70f;
	}


	// Load Vegetable params.
	// --------------

	// vegetable texture
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_texture");
		_VegetableTexture= var.asString();
	}
	catch (EUnknownVar &)
	{
		_VegetableTexture= "";
	}

	// vegetable ambient
	try
	{
		CConfigFile::CVar &color= cf.getVar("veget_ambient");
		_VegetableAmbient.R= color.asInt(0);
		_VegetableAmbient.G= color.asInt(1);
		_VegetableAmbient.B= color.asInt(2);
	}
	catch (EUnknownVar &)
	{
		_VegetableAmbient.set(80, 80, 80);
	}
	// vegetable diffuse
	try
	{
		CConfigFile::CVar &color= cf.getVar("veget_diffuse");
		// setup to behave correclty ie as maxLightFactor:
		sint	R= color.asInt(0) - _VegetableAmbient.R;	clamp(R, 0, 255);	_VegetableDiffuse.R= R;
		sint	G= color.asInt(1) - _VegetableAmbient.G;	clamp(G, 0, 255);	_VegetableDiffuse.G= G;
		sint	B= color.asInt(2) - _VegetableAmbient.B;	clamp(B, 0, 255);	_VegetableDiffuse.B= B;
	}
	catch (EUnknownVar &)
	{
		sint	R= 255 - _VegetableAmbient.R;	clamp(R, 0, 255);	_VegetableDiffuse.R= R;
		sint	G= 255 - _VegetableAmbient.G;	clamp(G, 0, 255);	_VegetableDiffuse.G= G;
		sint	B= 255 - _VegetableAmbient.B;	clamp(B, 0, 255);	_VegetableDiffuse.B= B;
	}
	// vegetable lightDir
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_light_dir");
		_VegetableLightDir.x= var.asFloat(0);
		_VegetableLightDir.y= var.asFloat(1);
		_VegetableLightDir.z= var.asFloat(2);
		_VegetableLightDir.normalize();
	}
	catch (EUnknownVar &)
	{
		_VegetableLightDir.set(0, 1, -1);
		_VegetableLightDir.normalize();
	}

	// windDir
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_wind_dir");
		_VegetableWindDir.x= var.asFloat(0);
		_VegetableWindDir.y= var.asFloat(1);
		_VegetableWindDir.z= var.asFloat(2);
	}
	catch (EUnknownVar &)
	{
		_VegetableWindDir.x= 0.5f;
		_VegetableWindDir.y= 0.5f;
		_VegetableWindDir.z= 0;
	}
	// windFreq
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_wind_freq");
		_VegetableWindFreq= var.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableWindFreq= 0.5;
	}
	// windPower
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_wind_power");
		_VegetableWindPower= var.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableWindPower= 1;
	}
	// windBendMin
	try
	{
		CConfigFile::CVar &var= cf.getVar("veget_wind_bend_min");
		_VegetableWindBendMin= var.asFloat();
	}
	catch (EUnknownVar &)
	{
		_VegetableWindBendMin= 0;
	}


}


// ***************************************************************************
bool		CObjectViewer::createVegetableLandscape()
{
	// If not already done.
	if(!_VegetableLandscape)
	{
		// create the landscape.
		_VegetableLandscape= static_cast<CLandscapeModel*>(CNELU::Scene.createModel(LandscapeModelId));

		// Create a Progress Dialog.
		CDialogProgress		dlgProgress;
		dlgProgress.Create(CDialogProgress::IDD, _MainFrame);
		dlgProgress.ShowWindow(true);

		try
		{
			if(_VegetableLandscapeTileBank=="")
			{
				throw Exception("Landscape CFG not fully defined");
			}

			// Load The Bank files (copied from CLandscapeUser :) ).
			// ================
			// progress
			dlgProgress.ProgressText.SetWindowText("Loading TileBanks...");
			dlgProgress.ProgressBar.SetPos(0);
			// load
			CIFile bankFile(CPath::lookup(_VegetableLandscapeTileBank));
			_VegetableLandscape->Landscape.TileBank.serial(bankFile);
			_VegetableLandscape->Landscape.TileBank.makeAllPathRelative();
			_VegetableLandscape->Landscape.TileBank.makeAllExtensionDDS();
			_VegetableLandscape->Landscape.TileBank.setAbsPath ("");

			// progress
			dlgProgress.ProgressBar.SetPos(50);
			// load
			CIFile farbankFile(CPath::lookup(_VegetableLandscapeTileFarBank));
			_VegetableLandscape->Landscape.TileFarBank.serial(farbankFile);
			if ( ! _VegetableLandscape->Landscape.initTileBanks() )
			{
				nlwarning( "You need to recompute bank.farbank for the far textures" );
			}
			bankFile.close();
			farbankFile.close();


			// flushTiles.
			// ================
			if(CNELU::Driver)
			{
				// progress
				dlgProgress.ProgressText.SetWindowText("Loading Tiles...");
				dlgProgress.ProgressBar.SetPos(0);

				// count nbText to load.
				sint	ts;
				sint	nbTextTotal= 0;
				for (ts=0; ts<_VegetableLandscape->Landscape.TileBank.getTileSetCount (); ts++)
				{
					CTileSet *tileSet=_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
					nbTextTotal+= tileSet->getNumTile128();
					nbTextTotal+= tileSet->getNumTile256();
					nbTextTotal+= CTileSet::count;
				}

				// load.
				sint	nbTextDone= 0;
				for (ts=0; ts<_VegetableLandscape->Landscape.TileBank.getTileSetCount (); ts++)
				{
					CTileSet *tileSet=_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
					sint tl;
					for (tl=0; tl<tileSet->getNumTile128(); tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (CNELU::Driver, (uint16)tileSet->getTile128(tl), 1);
						dlgProgress.ProgressBar.SetPos(nbTextDone*100/nbTextTotal);
					}
					for (tl=0; tl<tileSet->getNumTile256(); tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (CNELU::Driver, (uint16)tileSet->getTile256(tl), 1);
						dlgProgress.ProgressBar.SetPos(nbTextDone*100/nbTextTotal);
					}
					for (tl=0; tl<CTileSet::count; tl++, nbTextDone++)
					{
						_VegetableLandscape->Landscape.flushTiles (CNELU::Driver, (uint16)tileSet->getTransition(tl)->getTile (), 1);
						dlgProgress.ProgressBar.SetPos(nbTextDone*100/nbTextTotal);
					}
				}
			}


			// misc setup.
			// ================
			_VegetableLandscape->Landscape.setThreshold(_VegetableLandscapeThreshold);
			_VegetableLandscape->Landscape.setTileNear(_VegetableLandscapeTileNear);
			_VegetableLandscape->Landscape.setupStaticLight(_VegetableLandscapeDiffuse, _VegetableLandscapeAmbient, 1);
			_VegetableLandscape->Landscape.loadVegetableTexture(_VegetableTexture);
			_VegetableLandscape->Landscape.setupVegetableLighting(_VegetableAmbient, _VegetableDiffuse, _VegetableLightDir);
			_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);


			// Load the zones.
			// ================
			// landscape recentering.
			bool	zoneLoaded= false;
			CAABBox	landscapeBBox;
			// progress
			dlgProgress.ProgressText.SetWindowText("Loading Zones...");
			dlgProgress.ProgressBar.SetPos(0);
			uint	nbZones= _VegetableLandscapeZoneNames.size();
			for(uint i=0; i<nbZones;i++)
			{
				// open the file
				CIFile	zoneFile(CPath::lookup(_VegetableLandscapeZoneNames[i]));
				CZone	zone;
				// load
				zoneFile.serial(zone);
				// append to landscape
				_VegetableLandscape->Landscape.addZone(zone);
				// progress
				dlgProgress.ProgressBar.SetPos(i*100/nbZones);

				// Add to the bbox.
				if(!zoneLoaded)
				{
					zoneLoaded= true;
					landscapeBBox.setCenter(zone.getZoneBB().getCenter());
				}
				else
					landscapeBBox.extend(zone.getZoneBB().getCenter());
			}

			// After All zone loaded, recenter the mouse listener on the landscape.
			if(zoneLoaded)
			{
				CMatrix		matrix;
				_MouseListener.setHotSpot(landscapeBBox.getCenter());
				matrix.setPos(landscapeBBox.getCenter());
				matrix.rotateX(-(float)Pi/4);
				matrix.translate(CVector(0,-100,0));
				_MouseListener.setMatrix(matrix);
			}

			// Create collisions objects.
			_VegetableCollisionManager= new CVisualCollisionManager;
			_VegetableCollisionManager->setLandscape(&_VegetableLandscape->Landscape);
			_VegetableCollisionEntity= _VegetableCollisionManager->createEntity();
		}
		catch (Exception &e)
		{
			// close the progress dialog
			dlgProgress.DestroyWindow();

			MessageBox(_MainFrame->m_hWnd, e.what(), "Failed to Load landscape", MB_OK | MB_APPLMODAL);

			// remove first possibly created collisions objects.
			if(_VegetableCollisionEntity)
			{
				_VegetableCollisionManager->deleteEntity(_VegetableCollisionEntity);
				_VegetableCollisionEntity= NULL;
			}
			if(_VegetableCollisionManager)
			{
				delete _VegetableCollisionManager;
				_VegetableCollisionManager= NULL;
			}

			// remove the landscape
			CNELU::Scene.deleteModel(_VegetableLandscape);
			_VegetableLandscape= NULL;

			return false;
		}

		// close the progress dialog
		dlgProgress.DestroyWindow();
	}

	return true;
}


// ***************************************************************************
void		CObjectViewer::showVegetableLandscape()
{
	if(_VegetableLandscape)
	{
		_VegetableLandscape->show();
	}
}

// ***************************************************************************
void		CObjectViewer::hideVegetableLandscape()
{
	if(_VegetableLandscape)
	{
		_VegetableLandscape->hide();
	}
}


// ***************************************************************************
void		CObjectViewer::enableLandscapeVegetable(bool enable)
{
	// update
	_VegetableEnabled= enable;

	// update view.
	if(_VegetableLandscape)
	{
		_VegetableLandscape->Landscape.enableVegetable(_VegetableEnabled);
	}
}


// ***************************************************************************
void		CObjectViewer::refreshVegetableLandscape(const NL3D::CTileVegetableDesc &tvdesc)
{
	// if landscape is displayed.
	if(_VegetableLandscape)
	{
		// first disable the vegetable, to delete any vegetation
		_VegetableLandscape->Landscape.enableVegetable(false);

		// Then change all the tileSet of all the TileBanks.
		for (sint ts=0; ts<_VegetableLandscape->Landscape.TileBank.getTileSetCount (); ts++)
		{
			CTileSet *tileSet=_VegetableLandscape->Landscape.TileBank.getTileSet (ts);
			// change the vegetableTileDesc of this tileSet.
			tileSet->setTileVegetableDesc(tvdesc);
		}

		// re-Enable the vegetable (if wanted).
		_VegetableLandscape->Landscape.enableVegetable(_VegetableEnabled);
	}
}


// ***************************************************************************
void		CObjectViewer::setVegetableWindPower(float w)
{
	_VegetableWindPower= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);
}
// ***************************************************************************
void		CObjectViewer::setVegetableWindBendStart(float w)
{
	_VegetableWindBendMin= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);
}
// ***************************************************************************
void		CObjectViewer::setVegetableWindFrequency(float w)
{
	_VegetableWindFreq= w;
	if(_VegetableLandscape)
		_VegetableLandscape->Landscape.setVegetableWind(_VegetableWindDir, _VegetableWindFreq, _VegetableWindPower, _VegetableWindBendMin);
}


// ***************************************************************************
void		CObjectViewer::snapToGroundVegetableLandscape(bool enable)
{
	// update
	_VegetableSnapToGround= enable;
}


// ***************************************************************************
// ***************************************************************************
// Global wind part.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CObjectViewer::setGlobalWindPower(float w)
{
	if(_MainFrame)
	{
		clamp(w, 0.f, 1.f);
		_MainFrame->GlobalWindPower= w;
		CNELU::Scene.setGlobalWindPower(w);
	}
}


// ***************************************************************************
float		CObjectViewer::getGlobalWindPower() const
{
	if(_MainFrame)
		return _MainFrame->GlobalWindPower;
	else
		return 1.f;
}


