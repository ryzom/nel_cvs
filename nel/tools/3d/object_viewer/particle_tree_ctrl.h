/** \file particle_tree_ctrl.h
 * <File description>
 *
 * $Id: particle_tree_ctrl.h,v 1.1 2001/06/12 08:39:50 vizerie Exp $
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


#if !defined(AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_)
#define AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// particle_tree_ctrl.h : header file
//


class CParticleDlg ;



namespace NL3D
{
	class CParticleSystem ;
}

/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl window

class CParticleTreeCtrl : public CTreeCtrl
{

// Construction
public:
	CParticleTreeCtrl(CParticleDlg *);
	virtual ~CParticleTreeCtrl();
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParticleTreeCtrl)	
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL


	/// build a portion of the tree using the given particle system
	void buildTreeFromPS(NL3D::CParticleSystem *ps) ;

	void init(void) ;
	// Generated message map functions
protected:
	//{{AFX_MSG(CParticleTreeCtrl)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	CImageList _ImageList ;  // the image list containing the icons


	// the dialog that contain us
	CParticleDlg *_ParticleDlg ;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_)
