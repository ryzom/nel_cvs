/** \file ps_mover_dlg.h
 * this dialog display coordinate of an instance of a located in a particle system 
 * $Id: ps_mover_dlg.h,v 1.3 2001/06/25 13:00:37 vizerie Exp $
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

#if !defined(AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_)
#define AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 


namespace NL3D
{
	class CEvent3dMouseListener ;
}

#include "ps_wrapper.h"

template <class T> class CEditableRangeT ;
typedef CEditableRangeT<float> CEditableRangeFloat ;


/////////////////////////////////////////////////////////////////////////////
// CPSMoverDlg dialog

class CPSMoverDlg : public CDialog
{
// Construction
public:
	// construct the object with a pointer to the tree ctrl, and to the item that is being edited
	CPSMoverDlg(class CParticleTreeCtrl *parent, NL3D::CEvent3dMouseListener *ml,  HTREEITEM editedNode);   // standard constructor

	// dtor
	~CPSMoverDlg() ;

// Dialog Data
	//{{AFX_DATA(CPSMoverDlg)
	enum { IDD = IDD_PS_MOVER };
	CListBox	m_SubComponentCtrl;
	CString	m_X;
	CString	m_Y;
	CString	m_Z;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPSMoverDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:
	// position has to be updated (for mouse edition)
	void updatePosition(void) ;


	void init(CWnd *parent) ;

	

// Implementation
protected:

	CParticleTreeCtrl *_TreeCtrl ;
	CParticleTreeCtrl::CNodeType *_EditedNode ;
	NL3D::CEvent3dMouseListener *_MouseListener ;


	CEditableRangeFloat *_Scale, *_XScale, *_YScale, *_ZScale ;

	CStatic  *_ScaleText, *_XScaleText, *_YScaleText, *_ZScaleText ;

	// this generate control for scaling
	void createScaleControls(void) ;

	// delete scale controls if presents
	void cleanScaleCtrl(void) ;


	// Generated message map functions
	//{{AFX_MSG(CPSMoverDlg)
	afx_msg void OnUpdateXpos();
	afx_msg void OnUpdateYpos();
	afx_msg void OnUpdateZpos();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSubComponent();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	// wrappers to scale objects
	struct tagUniformScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const { return M->getScale(Index).x ; }
		void set(const float &v) { M->setScale(Index, v) ; }
	} _UniformScaleWrapper ;

	/// wrapper to scale the X coordinate
	struct tagXScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const { return M->getScale(Index).x ; }
		void set(const float &s) 
		{ 
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(s, v.y, v.z)) ; 
		}
	} _XScaleWrapper ;

	/// wrapper to scale the Y coordinate
	struct tagYScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const { return M->getScale(Index).y ; }
		void set(const float &s) 
		{ 
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(v.x, s, v.z) ) ; 
		}
	} _YScaleWrapper ;

	/// wrapper to scale the Z coordinate
	struct tagZScaleWrapper : public IPSWrapperFloat
	{
		uint32 Index ;
		NL3D::IPSMover *M ;
		float get(void) const { return M->getScale(Index).z ; }
		void set(const float &s) 
		{ 
			NLMISC::CVector v = M->getScale(Index) ;
			M->setScale(Index, NLMISC::CVector(v.x, v.y, s) ) ; 
		}
	} _ZScaleWrapper ;
	 

	// update the mouse listener position when the user entered a value with the keyboard
	void CPSMoverDlg::updateListener(void) ;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PS_MOVER_DLG_H__C1C4348E_3384_4557_B99E_CBE4E5492C0C__INCLUDED_)
