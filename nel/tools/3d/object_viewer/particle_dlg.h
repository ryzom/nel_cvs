/** \file particle_dlg.h
 * <File description>
 *
 * $Id: particle_dlg.h,v 1.1 2001/06/12 08:39:50 vizerie Exp $
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


#include "particle_tree_ctrl.h"



namespace NL3D
{
	class CParticleSystem ;
} ;


class CStartStopParticleSystem ;

/////////////////////////////////////////////////////////////////////////////
// CParticleDlg dialog

class CParticleDlg : public CDialog
{
// Construction
public:
	CParticleDlg(CWnd* pParent = NULL);   // standard constructor
	~CParticleDlg() ;

	void setLeftPane(CWnd *pane) ;

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

	NL3D::CParticleSystem *getCurrPS() { return _CurrPS ; }
	const NL3D::CParticleSystem *getCurrPS() const  { return _CurrPS ; }
	

	
protected:

	// the tree for viewing the system
	CParticleTreeCtrl *_ParticleTreeCtrl ;


	CStartStopParticleSystem *_StartStopDlg ;

	// the current system that is being edited
	NL3D::CParticleSystem *_CurrPS ;

	// the current right pane of the editor
	CWnd *_CurrentRightPane ;
	sint32 _CurrRightPaneWidth, _CurrRightPaneHeight ;

	CRect getTreeRect(int cx, int cy) const ;

	// Generated message map functions
	//{{AFX_MSG(CParticleDlg)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_DLG_H__AD58E337_952E_4C0D_A6D8_F87AFFEA3A24__INCLUDED_)
