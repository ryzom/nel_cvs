/** \file located_properties.cpp
 * <File description>
 *
 * $Id: located_properties.cpp,v 1.1 2001/06/12 08:39:50 vizerie Exp $
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

#include "nel/3d/ps_located.h"


using NL3D::CPSLocated ;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocatedProperties dialog


CLocatedProperties::CLocatedProperties(NL3D::CPSLocated *loc, CWnd* pParent /*=NULL*/)
	: CDialog(CLocatedProperties::IDD, pParent), _Located(loc)
{
	//{{AFX_DATA_INIT(CLocatedProperties)
	m_LimitedLifeTime = FALSE;
	m_SystemBasis = FALSE;
	//}}AFX_DATA_INIT


	_MinLife = new CEditableRangeFloat("MIN_LIFE", 0.1f, 10.1f) ;
	_MaxLife = new CEditableRangeFloat("MAX_LIFE", 0.1f, 10.1f) ;


	_MinMass = new CEditableRangeFloat("MIN_MASS", 0.1f, 1.1f) ;
	_MaxMass = new CEditableRangeFloat("MAX_MASS", 0.1f, 1.1f) ;


}

CLocatedProperties::~CLocatedProperties()
{
	delete _MinLife ;
	delete _MaxLife ;
	delete _MinMass ;
	delete _MaxMass ;
}

void CLocatedProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedProperties)
	DDX_Control(pDX, IDC_PARTICLE_NUMBER_POS, m_ParticleNumberPos);
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

//////////////////////////////////////////////////////////////////
// some function to wrap from the dialog to the particle system //
//////////////////////////////////////////////////////////////////

static float readerMinMass(void *lParam)
{
	return ((CPSLocated *) lParam)->getMinMass() ;
}

static void writerMinMass(float value, void *lParam)
{
	((CPSLocated *) lParam)->setMinMass(value) ;
}

static float readerMaxMass(void *lParam)
{
	return ((CPSLocated *) lParam)->getMaxMass() ;
}

static void writerMaxMass(float value, void *lParam)
{
	((CPSLocated *) lParam)->setMaxMass(value) ;
}


static float readerMinLife(void *lParam)
{
	return ((CPSLocated *) lParam)->getMinLife() ;
}

static void writerMinLife(float value, void *lParam)
{
	((CPSLocated *) lParam)->setLifeTime(value, ((CPSLocated *) lParam)->getMaxLife()) ;
}

static float readerMaxLife(void *lParam)
{
	return ((CPSLocated *) lParam)->getMaxLife() ;
}

static void writerMaxLife(float value, void *lParam)
{
	((CPSLocated *) lParam)->setLifeTime(((CPSLocated *) lParam)->getMaxLife(), value) ;
}












///////////////////////////////////////////


void CLocatedProperties::init(uint32 x, uint32 y, CWnd *pParent)
{

	Create(IDD_LOCATED_PROPERTIES, pParent) ;
	RECT r, pr  ;
	GetClientRect(&r) ;
	MoveWindow(x, y, r.right, r.bottom) ;	

	GetWindowRect(&pr) ;


	m_LifeMinPos.GetWindowRect(&r) ;
	_MinLife->setReader(readerMinLife, _Located) ;
	_MinLife->setWriter(writerMinLife, _Located) ;
	_MinLife->init(r.left - pr.left, r.top - pr.top, this) ;

	m_LifeMaxPos.GetWindowRect(&r) ;
	_MaxLife->setReader(readerMaxLife, _Located) ;
	_MaxLife->setWriter(writerMaxLife, _Located) ;
	_MaxLife->init(r.left - pr.left, r.top - pr.top, this) ;


	m_MassMinPos.GetWindowRect(&r) ;
	_MinMass->setReader(readerMinMass, _Located) ;
	_MinMass->setWriter(writerMinMass, _Located) ;
	_MinMass->init(r.left - pr.left, r.top - pr.top, this) ;

	m_MassMaxPos.GetWindowRect(&r) ;	
	_MaxMass->setReader(readerMaxMass, _Located) ;
	_MaxMass->setWriter(writerMaxMass, _Located) ;
	_MaxMass->init(r.left - pr.left, r.top - pr.top, this) ;

	
	m_SystemBasis = _Located->isInSystemBasis() ;
	m_LimitedLifeTime = !_Located->getLastForever() ;

	UpdateData(false) ;

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
