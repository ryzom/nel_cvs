/** \file mesh_dlg.cpp
 * A dialog that allows to choose a mesh (for mesh particles), and display the current mesh name 
 * $Id: mesh_dlg.cpp,v 1.6 2002/11/04 15:40:44 boucher Exp $
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
#include "object_viewer.h"
#include "mesh_dlg.h"
#include "3d/ps_particle.h"
#include "3d/ps_mesh.h"
#include "edit_morph_mesh_dlg.h"
#include "3d/particle_system_model.h"



#include "nel/misc/path.h"

///==================================================================
CMeshDlg::CMeshDlg(NL3D::CPSShapeParticle *sp, CParticleDlg  *particleDlg)
	: _ShapeParticle(sp), _EMMD(NULL), _ParticleDlg(particleDlg)
{
	//{{AFX_DATA_INIT(CMeshDlg)
	//}}AFX_DATA_INIT
}

CMeshDlg::~CMeshDlg()
{
	if (_EMMD)
	{
		_EMMD->DestroyWindow();
		delete _EMMD;
		_EMMD = NULL;
	}
}

///==================================================================
void CMeshDlg::init(CWnd *pParent, sint x, sint y)
{
	Create(IDD_CHOOSE_MESH, pParent);
	RECT r;
	GetClientRect(&r);
	r.top += y; r.bottom += y;
	r.right += x; r.left += x;
	MoveWindow(&r);	

	ShowWindow(SW_SHOW);
}


///==================================================================
void CMeshDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMeshDlg)
	DDX_Text(pDX, IDC_SHAPE_NAME, m_ShapeName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMeshDlg, CDialog)
	//{{AFX_MSG_MAP(CMeshDlg)
	ON_BN_CLICKED(IDC_BROWSE_SHAPE, OnBrowseShape)
	ON_BN_CLICKED(IDC_ENABLE_MORPHING, OnEnableMorphing)
	ON_BN_CLICKED(IDC_EDIT_MORPH, OnEditMorph)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///==================================================================
void CMeshDlg::OnBrowseShape() 
{
	
	CFileDialog fd(TRUE, ".shape", "*.shape", 0, NULL, this);
	if (fd.DoModal() == IDOK)
	{
		// Add to the path
		char drive[256];
		char dir[256];
		char path[256];
		char fname[256];
		char ext[256];


		// Add search path for the texture
		_splitpath (fd.GetPathName(), drive, dir, fname, ext);
		_makepath (path, drive, dir, NULL, NULL);
		NLMISC::CPath::addSearchPath (path);

		try
		{		
			_ShapeParticle->setShape(std::string(fname) + ext);		
			m_ShapeName = (std::string(fname) + ext).c_str();
			_ParticleDlg->getCurrPSModel()->touchTransparencyState();			
		}
		catch (NLMISC::Exception &e)
		{
			MessageBox(e.what(), "shape loading error");
		}		
	}
	UpdateData(false);
}


///==================================================================
BOOL CMeshDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	if (!dynamic_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle))
	{
		// hide the unused fields
		GetDlgItem(IDC_ENABLE_MORPHING)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_MORPH)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MORPH_FRAME)->ShowWindow(SW_HIDE);
		m_ShapeName = _ShapeParticle->getShape().c_str();		
	}
	else
	{		
		NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
		if (cm->getNumShapes() > 1)
		{
			((CButton *) GetDlgItem(IDC_ENABLE_MORPHING))->SetCheck(TRUE);
		}
		updateForMorph();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

///==================================================================
void CMeshDlg::updateForMorph()
{
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	if (cm)
	{
		BOOL enable = cm->getNumShapes() > 1;						
		GetDlgItem(IDC_EDIT_MORPH)->EnableWindow(enable);
		GetDlgItem(IDC_BROWSE_SHAPE)->EnableWindow(!enable);
		GetDlgItem(IDC_SHAPE_NAME)->EnableWindow(!enable);
		if (!enable)
		{
			m_ShapeName = _ShapeParticle->getShape().c_str();
		}
		else
		{
			m_ShapeName = "";
		}
	}
	UpdateData(FALSE);
}

///==================================================================
void CMeshDlg::OnEnableMorphing() 
{
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	if (((CButton *) GetDlgItem(IDC_ENABLE_MORPHING))->GetCheck())
	{
		// morphing enabled..		
		std::string currName[2] = { cm->getShape(), cm->getShape()};
		cm->setShapes(currName, 2);		
	}
	else
	{
		// morphing disabled
		std::string currName = cm->getShape(0);
		cm->setShape(currName);
	}
	updateForMorph();
}

///==================================================================
void CMeshDlg::OnEditMorph() 
{
	nlassert(_EMMD == NULL);
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	EnableWindow(FALSE);
	_EMMD = new CEditMorphMeshDlg(cm, this, _ParticleDlg, this);
	_EMMD->init(this);
}

///==================================================================
void CMeshDlg::childPopupClosed(CWnd *child)
{
	nlassert(_EMMD == child);
	_EMMD->DestroyWindow();
	delete _EMMD;
	_EMMD = NULL;
	EnableWindow(TRUE);
}


///==================================================================
BOOL CMeshDlg::EnableWindow( BOOL bEnable)
{
	if (!bEnable)
	{
		GetDlgItem(IDC_EDIT_MORPH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BROWSE_SHAPE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENABLE_MORPHING)->EnableWindow(FALSE);
		GetDlgItem(IDC_SHAPE_NAME)->EnableWindow(FALSE);	
	}
	else
	{
		GetDlgItem(IDC_ENABLE_MORPHING)->EnableWindow(TRUE);
		updateForMorph();
	}
	return CDialog::EnableWindow(bEnable);
}

