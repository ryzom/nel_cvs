// object_viewer.h : main header file for the OBJECT_VIEWER DLL
//

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

#include "resource.h"
#include "main_dlg.h"
#include "animation_set_dlg.h"
#include "animation_dlg.h"
#include "scene_dlg.h"
#include <nel/3d/animation_set.h>
#include <nel/3d/channel_mixer.h>
#include <nel/3d/shape.h>
#include <nel/3d/event_mouse_listener.h>
#include <nel/misc/event_listener.h>

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

#ifndef OBJECT_VIEWER_EXPORT
#define OBJECT_VIEWER_EXPORT __declspec( dllimport ) 
#endif // OBJECT_VIEWER_EXPORT

class CObjectViewer : public NLMISC::IEventListener
{
	friend class CSceneDlg;
public:
	OBJECT_VIEWER_EXPORT CObjectViewer ();
	OBJECT_VIEWER_EXPORT ~CObjectViewer ();

	// Init the UI
	OBJECT_VIEWER_EXPORT void initUI ();

	// Go
	OBJECT_VIEWER_EXPORT void go ();

	// Release the UI
	OBJECT_VIEWER_EXPORT void releaseUI ();

	// Listener
	virtual void operator ()(const NLMISC::CEvent& event);

	// Not exported
	void setAnimTime (float animStart, float animEnd);

private:
	CAnimationDlg								*_AnimationDlg;
	CMainDlg									*_SlotDlg;
	CAnimationSetDlg							*_AnimationSetDlg;
	CSceneDlg									*_SceneDlg;
	std::vector<class NL3D::IShape*>			_ListShape;
	std::vector<class NL3D::CTransformShape*>	_ListTransformShape;
	NL3D::CAnimationSet							_AnimationSet;
	NL3D::CChannelMixer							_ChannelMixer;
	NL3D::CEvent3dMouseListener					_MouseListener;
};

void setRegisterWindowState (const CWnd *pWnd, const char* keyName);
void getRegisterWindowState (CWnd *pWnd, const char* keyName, bool resize);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
