/** \file scene_dlg.h
 * <File description>
 *
 * $Id: scene_dlg.h,v 1.8 2001/06/25 12:52:18 vizerie Exp $
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

#if !defined(AFX_SCENE_DLG_H__A9ECE124_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_SCENE_DLG_H__A9ECE124_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 


#include "nel/misc/event_listener.h"



/// this class is used to catch events from the mouse, and too display a popup menu when mouse is clicked on the view

class CSceneDlgMouseListener : public NLMISC::IEventListener
{
public:
	class CObjectViewer	*ObjViewerDlg ;	
	class CSceneDlg *SceneDlg ;
	/** 
	  * Register the listener to the server.
	  */
	void addToServer (NLMISC::CEventServer& server);

protected:
	virtual void operator ()(const class NLMISC::CEvent& event) ;

} ;

/////////////////////////////////////////////////////////////////////////////
// CSceneDlg dialog

class CSceneDlg : public CDialog
{
// Construction
	friend class CObjectViewer;
public:
	CSceneDlg(class CObjectViewer *objView, CWnd* pParent = NULL);   // standard constructor
	~CSceneDlg();

// Dialog Data
	//{{AFX_DATA(CSceneDlg)
	enum { IDD = IDD_SCENE };
	CButton	EnableZCtrl;
	CButton	EnableYCtrl;
	CButton	EnableXCtrl;
	CEdit	MoveSpeedCtrl;
	BOOL	ViewAnimation;
	BOOL	ViewAnimationSet;
	BOOL	ViewSlots;
	BOOL	Euler;
	BOOL	ObjectMode;
	float	MoveSpeed;
	BOOL	ViewParticle;
	BOOL	MoveElement;
	BOOL	EnableX;
	BOOL	EnableY;
	BOOL	EnableZ;
	//}}AFX_DATA

	class CObjectViewer	*ObjView;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSceneDlg)
	afx_msg void OnClearScene();
	afx_msg void OnLoadMesh();
	afx_msg void OnLoadPlaylist();
	afx_msg void OnLoadScene();
	afx_msg void OnSavePlaylist();
	afx_msg void OnViewAnimation();
	afx_msg void OnViewAnimationset();
	afx_msg void OnViewSlots();
	virtual BOOL OnInitDialog();
	afx_msg void OnResetCamera();
	afx_msg void OnDestroy();
	afx_msg void OnViewParticles();
	afx_msg void OnMoveElement();
	afx_msg void OnEnableX();
	afx_msg void OnEnableY();
	afx_msg void OnEnableZ();
	afx_msg void OnEnableElementXrotate();
	afx_msg void OnEnableElementYrotate();
	afx_msg void OnEnableElementZrotate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	CSceneDlgMouseListener _RightButtonMouseListener ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCENE_DLG_H__A9ECE124_1C51_11D5_9CD4_0050DAC3A412__INCLUDED_)
