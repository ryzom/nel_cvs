/** \file located_properties.cpp
 * <File description>
 *
 * $Id: located_properties.cpp,v 1.5 2001/06/18 11:18:57 vizerie Exp $
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


// located_properties.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "located_properties.h"

#include "3d/ps_located.h"

#include "particle_dlg.h"
#include "particle_tree_ctrl.h"

using NL3D::CPSLocated ;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocatedProperties dialog


CLocatedProperties::CLocatedProperties(NL3D::CPSLocated *loc,  CParticleDlg *pdlg)
	: CDialog(CLocatedProperties::IDD, pdlg), _Located(loc), _ParticleDlg(pdlg)
{
	//{{AFX_DATA_INIT(CLocatedProperties)
	m_LimitedLifeTime = FALSE;
	m_SystemBasis = FALSE;
	//}}AFX_DATA_INIT


	_MinLife = new CEditableRangeFloat("MIN_LIFE", 0.1f, 10.1f) ;
	_MaxLife = new CEditableRangeFloat("MAX_LIFE", 0.1f, 10.1f) ;


	_MinMass = new CEditableRangeFloat("MIN_MASS", 0.1f, 1.1f) ;
	_MaxMass = new CEditableRangeFloat("MAX_MASS", 0.1f, 1.1f) ;

	_MaxNbParticles = new CEditableRangeUInt("MAX_NB_PARTICLES", 1, 501) ;
}

CLocatedProperties::~CLocatedProperties()
{
	delete _MinLife ;
	delete _MaxLife ;
	delete _MinMass ;
	delete _MaxMass ;
	delete _MaxNbParticles ;
}

void CLocatedProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedProperties)
	DDX_Control(pDX, IDC_PARTICLE_NUMBER_POS, m_MaxNbParticles);
	DDX_Control(pDX, IDC_MASS_MIN_VALUE, m_MassMaxPos);
	DDX_Control(pDX, IDC_MASS_MAX_VALUE, m_MassMinPos);
	DDX_Control(pDX, IDC_LIFE_MAX_VALUE, m_LifeMaxPos);
	DDX_Control(pDX, IDC_LIFE_MIN_VALUE, m_LifeMinPos);
	DDX_Check(pDX, IDC_LIMITED_LIFE_TIME, m_LimitedLifeTime);
	DDX_Check(pDX, IDC_SYSTEM_BASIS, m_SystemBasis);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedProperties, CDialog)
	//{{AFX_MSG_MAP(CLocatedProperties)
	ON_BN_CLICKED(IDC_LIMITED_LIFE_TIME, OnLimitedLifeTime)
	ON_BN_CLICKED(IDC_SYSTEM_BASIS, OnSystemBasis)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocatedProperties message handlers

BOOL CLocatedProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
















///////////////////////////////////////////


void CLocatedProperties::init(uint32 x, uint32 y)
{

	Create(IDD_LOCATED_PROPERTIES, (CWnd *) _ParticleDlg) ;
	RECT r, pr  ;
	GetClientRect(&r) ;
	MoveWindow(x, y, r.right, r.bottom) ;	

	GetWindowRect(&pr) ;


	m_LifeMinPos.GetWindowRect(&r) ;
	_MinLifeWrapper.Located = _Located ;
	_MinLife->setWrapper(&_MinLifeWrapper) ;	
	_MinLife->init(r.left - pr.left, r.top - pr.top, this) ;

	m_LifeMaxPos.GetWindowRect(&r) ;
	_MaxLifeWrapper.Located = _Located ;
	_MaxLife->setWrapper(&_MaxLifeWrapper) ;	
	_MaxLife->init(r.left - pr.left, r.top - pr.top, this) ;


	m_MassMinPos.GetWindowRect(&r) ;
	_MinMassWrapper.Located = _Located ;
	_MinMass->setWrapper(&_MinMassWrapper) ;	
	_MinMass->init(r.left - pr.left, r.top - pr.top, this) ;

	m_MassMaxPos.GetWindowRect(&r) ;
	_MaxMassWrapper.Located = _Located ;
	_MaxMass->setWrapper(&_MaxMassWrapper) ;	
	_MaxMass->init(r.left - pr.left, r.top - pr.top, this) ;

	m_MaxNbParticles.GetWindowRect(&r) ;
	_MaxNbParticlesWrapper.TreeCtrl = _ParticleDlg->ParticleTreeCtrl ;
	_MaxNbParticlesWrapper.Located = _Located ;
	_MaxNbParticles->setWrapper(&_MaxNbParticlesWrapper) ;	
	_MaxNbParticles->init(r.left - pr.left, r.top - pr.top, this) ;

	
	m_SystemBasis = _Located->isInSystemBasis() ;
	m_LimitedLifeTime = !_Located->getLastForever()  ;

	UpdateData(FALSE) ;

	OnLimitedLifeTime() ;

	ShowWindow(SW_SHOW) ;
}

void CLocatedProperties::OnLimitedLifeTime() 
{
	
	UpdateData() ;
	_MinLife->EnableWindow(m_LimitedLifeTime) ;
	_MaxLife->EnableWindow(m_LimitedLifeTime) ;
	

	if (!m_LimitedLifeTime)
	{
		_Located->setLastForever() ;
	}
	else
	{
		_Located->setLifeTime(_Located->getMinLife(), _Located->getMaxLife()) ;
	}
}

void CLocatedProperties::OnSystemBasis() 
{
	UpdateData() ;
	_Located->setSystemBasis(m_SystemBasis ? true : false) ;
}
