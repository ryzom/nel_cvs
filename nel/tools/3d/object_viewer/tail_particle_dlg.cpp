/** \file tail_particle_dlg.cpp
 * A dailog that helps to tune propertie of particle that owns a tail 
 * $Id: tail_particle_dlg.cpp,v 1.3 2002/02/27 15:45:52 vizerie Exp $
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
 * MA 02
*/

#include "std_afx.h"
#include "object_viewer.h"
#include "3d/ps_particle.h"
#include "tail_particle_dlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTailParticleDlg dialog


CTailParticleDlg::CTailParticleDlg(NL3D::CPSTailParticle *tp) 
{
	nlassert(tp) ;
	_TailParticle = tp ;

	//{{AFX_DATA_INIT(CTailParticleDlg)
	m_TailFade = tp->getColorFading() ;
	m_TailInSystemBasis = tp->isInSystemBasis() ;
	m_TailPersistAfterDeath = FALSE;
	//}}AFX_DATA_INIT
}


void CTailParticleDlg::init(CWnd *pParent, sint x, sint y)
{
	Create(IDD_TAIL_PARTICLE, pParent) ;
	RECT r ;
	GetClientRect(&r) ;
	r.top += y; r.bottom += y ;
	r.right += x ; r.left += x ;
	MoveWindow(&r) ;

	if (!dynamic_cast<NL3D::CPSRibbon *>(_TailParticle))
	{
		m_TailShape.EnableWindow(FALSE) ;
		m_TailShape.ShowWindow(FALSE) ;		
		m_TailPersistAfterDeathCtrl.EnableWindow(FALSE) ;
		m_TailPersistAfterDeathCtrl.ShowWindow(FALSE) ;
	}
	else
	{
///		m_TailPersistAfterDeath = (dynamic_cast<NL3D::CPSRibbon *>(_TailParticle))->getPersistAfterDeath() ;		
	}

	UpdateData() ;

	ShowWindow(SW_SHOW) ;
}

void CTailParticleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTailParticleDlg)
	DDX_Control(pDX, IDC_TAIL_SHAPE, m_TailShape);
	DDX_Control(pDX, IDC_TAIL_PERSIST_AFTER_DEATH, m_TailPersistAfterDeathCtrl);
	DDX_Check(pDX, IDC_TAIL_FADE, m_TailFade);
	DDX_Check(pDX, IDC_TAIL_IN_SYSTEM_BASIS, m_TailInSystemBasis);
	DDX_Check(pDX, IDC_TAIL_PERSIST_AFTER_DEATH, m_TailPersistAfterDeath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTailParticleDlg, CDialog)
	//{{AFX_MSG_MAP(CTailParticleDlg)
	ON_BN_CLICKED(IDC_TAIL_FADE, OnTailFade)
	ON_BN_CLICKED(IDC_TAIL_IN_SYSTEM_BASIS, OnTailInSystemBasis)
	ON_BN_CLICKED(IDC_TAIL_PERSIST_AFTER_DEATH, OnTailPersistAfterDeath)
	ON_CBN_SELCHANGE(IDC_TAIL_SHAPE, OnSelchangeTailShape)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTailParticleDlg message handlers

void CTailParticleDlg::OnTailFade() 
{
	UpdateData() ;
	_TailParticle->setColorFading(m_TailFade ? true : false) ;
	UpdateData(FALSE) ;
	
}

void CTailParticleDlg::OnTailInSystemBasis() 
{
	UpdateData() ;
	_TailParticle->setSystemBasis(m_TailFade ? true : false) ;
	UpdateData(FALSE) ;	
}

void CTailParticleDlg::OnTailPersistAfterDeath() 
{
	UpdateData() ;
	nlassert(dynamic_cast<NL3D::CPSRibbon *>(_TailParticle)) ;
///	(dynamic_cast<NL3D::CPSRibbon *>(_TailParticle))->setPersistAfterDeath(m_TailPersistAfterDeath ? true : false) ;
	UpdateData(FALSE) ;
}

void CTailParticleDlg::OnSelchangeTailShape() 
{
	UpdateData() ;
	NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle) ;
	nlassert(r) ;
	switch (m_TailShape.GetCurSel() )
	{
		case 0: // triangle
			r->setShape(NL3D::CPSRibbon::Triangle, NL3D::CPSRibbon::NbVerticesInTriangle) ;
		break ;
		case 1:	// quad
			r->setShape(NL3D::CPSRibbon::Losange, NL3D::CPSRibbon::NbVerticesInLosange) ;
		break ;
		case 2: // octogon
			r->setShape(NL3D::CPSRibbon::HeightSides, NL3D::CPSRibbon::NbVerticesInHeightSide) ;
		break ;
		case 3: // pentacle
			r->setShape(NL3D::CPSRibbon::Pentagram, NL3D::CPSRibbon::NbVerticesInPentagram) ;
		break ;
	}

	Invalidate() ;
}

void CTailParticleDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle) ;
	// if we're dealing with a ribbon, we draw the shape used for extrusion
	if (r)
	{
		const uint x = 270, y = 15, size = 32 ;
	

		dc.FillSolidRect(x, y, size, size, 0xffffff) ;
		
		std::vector<NLMISC::CVector> verts ;
		verts.resize(r->getNbVerticesInShape() ) ;
		r->getShape(&verts[0]) ;

		CPen p ;
		p.CreatePen(PS_SOLID, 1, (COLORREF) 0) ;
		CPen *old = dc.SelectObject(&p) ;

		dc.MoveTo((int) (x + (size / 2) * (1 + verts[0].x)), (int) (y + (size / 2) * (1 - verts[0].y))) ;
		for (std::vector<NLMISC::CVector>::const_iterator it = verts.begin() ; it != verts.end() ; ++it)
		{
			dc.LineTo((int) (x + (size / 2) * (1 + it->x)), (int) (y + (size / 2) * (1 - it->y))) ;
		}
		dc.LineTo((int) (x + (size / 2) * (1 + verts[0].x)), (int) (y + (size / 2) * (1 - verts[0].y))) ;
		dc.SelectObject(old) ;
	}
}
