/** \file mesh_dlg.h
 * A dialog that allows to choose a mesh (for mesh particles), and display the current mesh name 
 * $Id: mesh_dlg.h,v 1.1 2001/06/25 13:27:25 vizerie Exp $
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

#if !defined(AFX_MESH_DLG_H__27F79AAD_DEA7_4A3E_85E9_9DB32B9419C5__INCLUDED_)
#define AFX_MESH_DLG_H__27F79AAD_DEA7_4A3E_85E9_9DB32B9419C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	struct CPSShapeParticle ;
}

/////////////////////////////////////////////////////////////////////////////
// CMeshDlg dialog

class CMeshDlg : public CDialog
{
// Construction
public:
	CMeshDlg(NL3D::CPSShapeParticle *sp);   // standard constructor

	void init(CWnd *pParent, sint x, sint y) ;

// Dialog Data
	//{{AFX_DATA(CMeshDlg)
	enum { IDD = IDD_CHOOSE_MESH };
	CString	m_ShapeName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NL3D::CPSShapeParticle *_ShapeParticle ;


	// Generated message map functions
	//{{AFX_MSG(CMeshDlg)
	afx_msg void OnBrowseShape();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESH_DLG_H__27F79AAD_DEA7_4A3E_85E9_9DB32B9419C5__INCLUDED_)
