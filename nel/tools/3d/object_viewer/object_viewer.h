/** \file object_viewer.cpp
 * main header file for the OBJECT_VIEWER DLL
 *
 * $Id: object_viewer.h,v 1.10 2001/06/22 12:45:42 besson Exp $
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

#if !defined(AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#define REGKEY_OBJ_VIEW "Software\\Nevrax\\nel\\object_viewer"
#define REGKEY_OBJ_VIEW_OPENGL_WND "Software\\Nevrax\\nel\\object_viewer\\opengl_wnd"
#define REGKEY_OBJ_VIEW_SCENE_DLG "Software\\Nevrax\\nel\\object_viewer\\scene_dlg"
#define REGKEY_OBJ_VIEW_ANIMATION_DLG "Software\\Nevrax\\nel\\object_viewer\\animation_dlg"
#define REGKEY_OBJ_VIEW_ANIMATION_SET_DLG "Software\\Nevrax\\nel\\object_viewer\\animation_set_dlg"
#define REGKEY_OBJ_VIEW_SLOT_DLG "Software\\Nevrax\\nel\\object_viewer\\slot_dlg"
#define REGKEY_OBJ_PARTICLE_DLG "Software\\Nevrax\\nel\\object_viewer\\particle_dlg"


#include "resource.h"
#include "object_viewer_interface.h"
#include "main_dlg.h"
#include "animation_set_dlg.h"
#include "animation_dlg.h"
#include "scene_dlg.h"
#include "particle_dlg.h"
#include <3d/animation_set.h>
#include <3d/channel_mixer.h>
#include <3d/shape.h>
#include <3d/event_mouse_listener.h>
#include <3d/light.h>
#include <nel/misc/event_listener.h>
#include <nel/misc/stream.h>

/////////////////////////////////////////////////////////////////////////////
// CObject_viewerApp
// See object_viewer.cpp for the implementation of this class
//

class CObject_viewerApp : public CWinApp
{
public:
	CObject_viewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObject_viewerApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CObject_viewerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

class CMeshDesc
{
public:
	CMeshDesc () {};
	CMeshDesc (const char* meshName, const char* skeletonName)
	{
		MeshName=meshName;
		SkeletonName=skeletonName;
	}
	std::string		MeshName;
	std::string		SkeletonName;
	void			serial (NLMISC::IStream& s)
	{
		int ver=s.serialVersion (0);
		s.serial (MeshName);
		s.serial (SkeletonName);
	}
};

class CObjectViewer : public IObjectViewer, public NLMISC::IEventListener
{
	friend class CSceneDlg;
	friend class CAnimationSetDlg;
public:
	CObjectViewer ();
	~CObjectViewer ();

	// Init the UI
	void initUI ();

	// Go
	void go ();

	// Release the UI
	void releaseUI ();

	// Set single animtion.
	void setSingleAnimation (NL3D::CAnimation*	pAnim, const char* name);

	// Set automatic animation
	void setAutoAnimation (NL3D::CAnimation* pAnim);

	// Add a mesh
	NL3D::CTransformShape	*addMesh (NL3D::IShape* pMeshShape, NL3D::IShape* pSkelShape, const char* meshName, const char* skelName, const char *animBaseName);

	// Load a mesh
	bool loadMesh (const char* meshFilename, const char* skeleton="");

	// Set ambient color
	void setAmbientColor (const NLMISC::CRGBA& color);

	// Set ambient color
	void setLight (unsigned char id, const NL3D::CLight& light);

	// Load a shape
	void resetCamera ();

	// Listener
	virtual void operator ()(const NLMISC::CEvent& event);

	// get the mouse listener
	NL3D::CEvent3dMouseListener &getMouseListener(void) { return _MouseListener ; }
	const NL3D::CEvent3dMouseListener &getMouseListener(void) const { return _MouseListener ; }

	// get the particle dialog
	CParticleDlg *getParticleDialog(void) { return _ParticleDlg ; }
	const CParticleDlg *getParticleDialog(void) const { return _ParticleDlg ; }




	// Not exported
	void setAnimTime (float animStart, float animEnd);

	// Reset the slots
	void resetSlots ();

	// Reinit and refill the channel mixer channels
	void reinitChannels ();

	// Return the frame rate
	float getFrameRate ();

	// Serial the config
	void serial (NLMISC::IStream& f);

private:
	CAnimationDlg								*_AnimationDlg;
	CMainDlg									*_SlotDlg;
	CAnimationSetDlg							*_AnimationSetDlg;
	CSceneDlg									*_SceneDlg;
	CParticleDlg								*_ParticleDlg ;
	std::vector<std::string>					_ListShapeBaseName;
	std::vector<CMeshDesc>						_ListMeshes;
	std::vector<class NL3D::CTransformShape*>	_ListTransformShape;
	NL3D::CAnimationSet							_AnimationSet;
	NL3D::CChannelMixer							_ChannelMixer;
	NL3D::CEvent3dMouseListener					_MouseListener;
	NLMISC::CRGBA								_HotSpotColor;
	float										_HotSpotSize;
};

void setRegisterWindowState (const CWnd *pWnd, const char* keyName);
void getRegisterWindowState (CWnd *pWnd, const char* keyName, bool resize);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
