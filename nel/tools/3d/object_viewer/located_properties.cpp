/** \file located_properties.cpp
 * a dialog that allow to tune a located properties. In a aprticle system, a located is an object that has :
 *  - a position
 *  - a mass
 *  - a speed vector
 *  - a lifetime
 *
 * $Id: located_properties.cpp,v 1.11 2001/09/12 13:29:49 vizerie Exp $
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
#include "attrib_dlg.h"

using NL3D::CPSLocated;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocatedProperties dialog


CLocatedProperties::CLocatedProperties(NL3D::CPSLocated *loc,  CParticleDlg *pdlg)
	: CDialog(CLocatedProperties::IDD, pdlg), _Located(loc), _ParticleDlg(pdlg), _MassDialog(NULL), _LifeDialog(NULL)
{
	//{{AFX_DATA_INIT(CLocatedProperties)
	m_LimitedLifeTime = FALSE;
	m_SystemBasis = FALSE;
	m_DisgradeWithLOD = FALSE;
	//}}AFX_DATA_INIT


	

	_MaxNbParticles = new CEditableRangeUInt("MAX_NB_PARTICLES", 1, 501);

	_SkipFramesDlg = new CEditableRangeUInt("LOCATED SKIP FRAME RATE", 0, 4);
	
}

CLocatedProperties::~CLocatedProperties()
{
	_MassDialog->DestroyWindow();
	_LifeDialog->DestroyWindow();
	_MaxNbParticles->DestroyWindow();
	_SkipFramesDlg->DestroyWindow();

	delete _LifeDialog;
	delete _MassDialog;
	delete _MaxNbParticles;
	delete _SkipFramesDlg;
	
}

void CLocatedProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedProperties)
	DDX_Control(pDX, IDC_PARTICLE_NUMBER_POS, m_MaxNbParticles);
	DDX_Check(pDX, IDC_LIMITED_LIFE_TIME, m_LimitedLifeTime);
	DDX_Check(pDX, IDC_SYSTEM_BASIS, m_SystemBasis);
	DDX_Check(pDX, IDC_DISGRADE_WITH_LOD, m_DisgradeWithLOD);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedProperties, CDialog)
	//{{AFX_MSG_MAP(CLocatedProperties)
	ON_BN_CLICKED(IDC_LIMITED_LIFE_TIME, OnLimitedLifeTime)
	ON_BN_CLICKED(IDC_SYSTEM_BASIS, OnSystemBasis)
	ON_BN_CLICKED(IDC_DISGRADE_WITH_LOD, OnDisgradeWithLod)
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

	Create(IDD_LOCATED_PROPERTIES, (CWnd *) _ParticleDlg);
	RECT r, pr;
	GetClientRect(&r);
	MoveWindow(x, y, r.right, r.bottom);	

	GetWindowRect(&pr);


	const sint xPos = 0;
	sint yPos = 100;

	_LifeDialog = new CAttribDlgFloat("LIFETIME");
	_LifeDialog->enableMemoryScheme(false);

	_LifeWrapper.Located = _Located;

	_LifeDialog->setWrapper(&_LifeWrapper);			

	_LifeDialog->setSchemeWrapper(&_LifeWrapper);

	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_LIFE_TIME));

	_LifeDialog->init(bmh, xPos, yPos, this);


	_LifeDialog->GetClientRect(&r);	
	yPos += r.bottom + 3;

	if (_Located->getLastForever())
	{
		_LifeDialog->EnableWindow(FALSE);
	}	


	_MassDialog = new CAttribDlgFloat("PARTICLE_MASS", 0.001f, 10);	
	_MassDialog->enableLowerBound(0, true); // 0 is disallowed
	_MassDialog->enableMemoryScheme(false);
	_MassWrapper.Located = _Located;
	_MassDialog->setWrapper(&_MassWrapper);			
	_MassDialog->setSchemeWrapper(&_MassWrapper);
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_WEIGHT));
	_MassDialog->init(bmh, xPos, yPos, this);
	_MassDialog->GetClientRect(&r);	
	yPos += r.bottom + 3;
	


	m_MaxNbParticles.GetWindowRect(&r);
	_MaxNbParticlesWrapper.TreeCtrl = _ParticleDlg->ParticleTreeCtrl;
	_MaxNbParticlesWrapper.Located = _Located;
	_MaxNbParticles->setWrapper(&_MaxNbParticlesWrapper);	
	_MaxNbParticles->init(r.left - pr.left, r.top - pr.top, this);

	
	m_SystemBasis = _Located->isInSystemBasis();
	m_LimitedLifeTime = _Located->getLastForever() ? FALSE : TRUE;



	_SkipFrameRateWrapper.Located = _Located;
	_SkipFramesDlg->setWrapper(&_SkipFrameRateWrapper);
	_SkipFramesDlg->init(99, 339, this);

	UpdateData(FALSE);

	m_DisgradeWithLOD = _Located->hasLODDegradation();



	ShowWindow(SW_SHOW);


}

void CLocatedProperties::OnLimitedLifeTime() 
{
	
	UpdateData();
	

	if (!m_LimitedLifeTime)
	{
		_Located->setLastForever();
		_LifeDialog->EnableWindow(FALSE);
	}
	else
	{
		_Located->setInitialLife(_Located->getInitialLife());
		_LifeDialog->EnableWindow(TRUE);
	}
}

void CLocatedProperties::OnSystemBasis() 
{
	UpdateData();
	_Located->setSystemBasis(m_SystemBasis ? true : false);
}

void CLocatedProperties::OnDisgradeWithLod() 
{
	UpdateData();
	_Located->forceLODDegradation(m_DisgradeWithLOD ? true : false /* to avoid warning from MSVC */);
	
}
