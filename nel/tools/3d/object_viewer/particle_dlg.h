/** \file particle_dlg.h
 * The main dialog for particle system edition. If holds a tree constrol describing the system structure,
 * and show the properties of the selected object
 *
 * $Id: particle_dlg.h,v 1.13 2003/10/07 12:32:42 vizerie Exp $
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


#if !defined(AFX_PARTICLE_DLG_H__AD58E337_952E_4C0D_A6D8_F87AFFEA3A24__INCLUDED_)
#define AFX_PARTICLE_DLG_H__AD58E337_952E_4C0D_A6D8_F87AFFEA3A24__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// particle_dlg.h : header file
//


#include "object_viewer.h"
#include "particle_tree_ctrl.h"
#include "nel/misc/matrix.h"


namespace NL3D
{
	class CParticleSystem;
	class CParticleSystemModel;	
	class CFontManager;
	class CFontGenerator;
};


class CStartStopParticleSystem;
class CSceneDlg;
class CParticleTreeCtrl;
class CMainFrame;
class CAnimationDlg;


/////////////////////////////////////////////////////////////////////////////
// CParticleDlg dialog

class CParticleDlg : public CDialog, public CObjectViewer::IMainLoopCallBack
{
// Construction
public:
	CParticleDlg::CParticleDlg(class CObjectViewer* main, CWnd *pParent, CMainFrame* mainFrame, CAnimationDlg *animDLG);
	~CParticleDlg();

	void setRightPane(CWnd *pane);	
	void refreshRightPane();	

// Dialog Data
	//{{AFX_DATA(CParticleDlg)
	enum { IDD = IDD_PARTICLE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParticleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation

public:
	// the tree for viewing the system
	CParticleTreeCtrl *ParticleTreeCtrl;
	// inherited from CObjectViewer::IMainLoopCallBack
	void go(void);

public:

	friend class CParticleTreeCtrl;
	/// reset the particle system (but doesn't update the window showing it)
	void resetSystem(void);

	NL3D::CParticleSystem *getCurrPS() { return _CurrPS; }
	const NL3D::CParticleSystem *getCurrPS() const  { return _CurrPS; }


	NL3D::CParticleSystemModel *getCurrPSModel() { return _CurrSystemModel; }
	const NL3D::CParticleSystemModel *getCurrPSModel() const  { return _CurrSystemModel; }

	// set a new particle system that is the current system
	void setNewCurrPS(NL3D::CParticleSystem *ps, NL3D::CParticleSystemModel *psm)
	{
		_CurrPS = ps;
		_CurrSystemModel = psm;
	}

	// get matrix of current fx
	const NLMISC::CMatrix &getPSMatrix() const;
	// set matrix of current fx
	void setPSMatrix(const NLMISC::CMatrix &mat);
	// get world matrix of current fx
	const NLMISC::CMatrix &getPSWorldMatrix() const;	
	// set world matrix of current fx
	void setPSWorldMatrix(const NLMISC::CMatrix &mat);
	
	// move the current selected element using the given matrix
	void moveElement(const NLMISC::CMatrix &mat);	

	// get the matrix of the current selected element selected, or identity if there's none
	NLMISC::CMatrix getElementMatrix(void) const;
	
	// the scene dialog
	CMainFrame *MainFrame;


	// the fonts used for particle edition
	NL3D::CFontManager *FontManager;
	NL3D::CFontGenerator *FontGenerator;


	// get the current right pane of the editor (NULL if none)
	CWnd *getRightPane(void) { return CurrentRightPane; }
	const CWnd *getRightPane(void) const { return CurrentRightPane; }

	// auto bbox for fx
	void  setAutoBBox(bool enable) { _AutoUpdateBBox = enable; }
	bool  getAutoBBox() const { return 	_AutoUpdateBBox; }

	// reset the auto bbox
	void  resetAutoBBox() {	_EmptyBBox = true; }

	// get the object viewer instance
	CObjectViewer *getObjectViewer() const { return _ObjView; }

	/** Stick the current edited fx to a skeleton.	  
	  * This also reset the fx matrix, and prevent from changing it.
	  */
	void stickPSToSkeleton(NL3D::CSkeletonModel *skel, uint bone);

	// unstick the current edited fx from its parent skeleton (if there's one)
	void unstickPSFromSkeleton();

	// return true is the current edited fx is sticked to a skeleton.
	bool isPSStickedToSkeleton() const { return _ParentSkel != NULL; }

	CStartStopParticleSystem *StartStopDlg;

protected:
	CObjectViewer *_ObjView;	

	// the current system that is being edited
	NL3D::CParticleSystem *_CurrPS;

	// the current model that holds our system
	NL3D::CParticleSystemModel *_CurrSystemModel;

	// parent skeleton for the particle system
	NLMISC::CRefPtr<NL3D::CSkeletonModel> _ParentSkel;

	// the system bbox must be updated automatically
	bool						_AutoUpdateBBox;
	
	// the last computed bbox for the system
	bool						_EmptyBBox;
	NLMISC::CAABBox			    _CurrBBox;
	

	// the current right pane of the editor
	CWnd   *CurrentRightPane;
	sint32 CurrRightPaneWidth, CurrRightPaneHeight;

	CRect getTreeRect(int cx, int cy) const;


	// Generated message map functions
	//{{AFX_MSG(CParticleDlg)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_DLG_H__AD58E337_952E_4C0D_A6D8_F87AFFEA3A24__INCLUDED_)
