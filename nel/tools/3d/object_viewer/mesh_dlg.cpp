/** \file mesh_dlg.cpp
 * A dialog that allows to choose a mesh (for mesh particles), and display the current mesh name 
 * $Id: mesh_dlg.cpp,v 1.4 2001/12/06 16:57:04 vizerie Exp $
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



#include "nel/misc/path.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMeshDlg dialog


CMeshDlg::CMeshDlg(NL3D::CPSShapeParticle *sp)
	: _ShapeParticle(sp)
{
	//{{AFX_DATA_INIT(CMeshDlg)
	m_ShapeName = sp->getShape().c_str();
	//}}AFX_DATA_INIT
}

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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMeshDlg message handlers

void CMeshDlg::OnBrowseShape() 
{
	
	CFileDialog fd(TRUE, ".shape", "*.shape", 0, NULL, this);
	if (fd.DoModal() == IDOK)
	{
		// Add to the path
		char drive[256];
		char dir[256];
		char path[256];

		// Add search path for the texture
		_splitpath (fd.GetPathName(), drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		NLMISC::CPath::addSearchPath (path);

		try
		{		
			_ShapeParticle->setShape(std::string(fd.GetFileName()));		
			m_ShapeName = fd.GetFileName();;
			/*if (_ShapeParticle->setShape(std::string(fd.GetFileName())))
			{
				m_ShapeName = fd.GetFileName();
			}
			else
			{
				MessageBox("Unable to set shape (invalid or not found)", "Error", MB_OK | MB_ICONEXCLAMATION);
			}*/
		}
		catch (NLMISC::Exception &e)
		{
			MessageBox(e.what(), "shape loading error");
		}		
	}
	UpdateData(false);
}
