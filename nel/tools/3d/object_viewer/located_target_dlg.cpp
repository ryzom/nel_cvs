/** \file located_target_dlg.cpp
 * a dialog that allow to choose targets for a particle system object (collision zone, forces)
 *
 * $Id: located_target_dlg.cpp,v 1.8.4.1 2003/06/02 11:26:00 boucher Exp $
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


#include "3d/particle_system.h"
#include "object_viewer.h"
#include "located_target_dlg.h"
#include "collision_zone_dlg.h"
#include "editable_range.h"
#include "attrib_dlg.h"
#include "direction_attr.h"

#undef new

/////////////////////////////////////////////////////////////////////////////
// CLocatedTargetDlg dialog


CLocatedTargetDlg::CLocatedTargetDlg(NL3D::CPSTargetLocatedBindable *lbTarget) : _LBTarget(lbTarget)	
{
	//{{AFX_DATA_INIT(CLocatedTargetDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLocatedTargetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedTargetDlg)
	DDX_Control(pDX, IDC_AVAILABLE_TARGET, m_AvailableTargets);
	DDX_Control(pDX, IDC_TARGET, m_Targets);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedTargetDlg, CDialog)
	//{{AFX_MSG_MAP(CLocatedTargetDlg)
	ON_BN_CLICKED(IDC_ADD_TARGET, OnAddTarget)
	ON_BN_CLICKED(IDC_REMOVE_TARGET, OnRemoveTarget)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocatedTargetDlg message handlers


void CLocatedTargetDlg::init(CWnd* pParent)
{
	Create(IDD_LOCATED_TARGET_DLG, pParent);
	ShowWindow(SW_SHOW);
}


void CLocatedTargetDlg::OnAddTarget() 
{
	UpdateData();
	
	int totalCount = m_AvailableTargets.GetCount();
	nlassert(totalCount);
	std::vector<int> indexs;
	indexs.resize(totalCount);
	int selCount = m_AvailableTargets.GetSelItems(totalCount, &indexs[0]);

	std::sort(indexs.begin(), indexs.begin() + selCount); // we never know ...

	for (int k = 0; k < selCount; ++k)
	{
		NL3D::CPSLocated *loc = (NL3D::CPSLocated *) m_AvailableTargets.GetItemData(indexs[k] - k);
		nlassert(loc);
		_LBTarget->attachTarget(loc);
		m_AvailableTargets.DeleteString(indexs[k] - k);
		int l = m_Targets.AddString(loc->getName().c_str());
		m_Targets.SetItemData(l, (DWORD) loc);
	}

	
	UpdateData(FALSE);


}

void CLocatedTargetDlg::OnRemoveTarget() 
{
	UpdateData();	


	int totalCount = m_Targets.GetCount();
	nlassert(totalCount);
	std::vector<int> indexs;
	indexs.resize(totalCount);
	int selCount = m_Targets.GetSelItems(totalCount, &indexs[0]);

	std::sort(indexs.begin(), indexs.begin() + selCount); // we never know ...

	for (int k = 0; k < selCount; ++k)
	{
		NL3D::CPSLocated *loc = (NL3D::CPSLocated *) m_Targets.GetItemData(indexs[k] - k);
		nlassert(loc);
		_LBTarget->detachTarget(loc);
		m_Targets.DeleteString(indexs[k] - k);
		int l = m_AvailableTargets.AddString(loc->getName().c_str());
	
		m_AvailableTargets.SetItemData(l, (DWORD) loc);
	}

	UpdateData(FALSE);	
}

BOOL CLocatedTargetDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	RECT r;

	uint k;
	uint nbTarg = _LBTarget->getNbTargets();

	m_Targets.InitStorage(nbTarg, 128);

	std::set<NL3D::CPSLocated *> targetSet;

	// fill the box thta tells us what the target are
	for(k = 0; k < nbTarg; ++k)
	{
		m_Targets.AddString(_LBTarget->getTarget(k)->getName().c_str() );
		m_Targets.SetItemData(k, (DWORD) _LBTarget->getTarget(k) );
		targetSet.insert(_LBTarget->getTarget(k));
	};

	// fill abox with the available targets
	NL3D::CParticleSystem  *ps = _LBTarget->getOwner()->getOwner();

	uint nbLocated = ps->getNbProcess();


	
	m_AvailableTargets.InitStorage(nbTarg, 128);
	for (k = 0; k < nbLocated; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k));
		if (loc)
		{
			if (targetSet.find(loc) == targetSet.end())
			{
				int l = m_AvailableTargets.AddString(loc->getName().c_str() );				
				m_AvailableTargets.SetItemData(l, (DWORD) loc );				
			}
		}
	}


	const sint posX = 5;
	sint posY = 180;

	// collision zone case

	if (dynamic_cast<NL3D::CPSZone *>(_LBTarget))
	{
		CCollisionZoneDlg *czd = new CCollisionZoneDlg(dynamic_cast<NL3D::CPSZone *>(_LBTarget));
		pushWnd(czd);
		czd->init(posX, posY, this);
	}


	// force with intensity case

	if (dynamic_cast<NL3D::CPSForceIntensity *>(_LBTarget))
	{
		_ForceIntensityWrapper.F = dynamic_cast<NL3D::CPSForceIntensity *>(_LBTarget);
		CAttribDlgFloat *fi = new CAttribDlgFloat(std::string("FORCE INTENSITY"), 0, 100);
		pushWnd(fi);			
		fi->setWrapper(&_ForceIntensityWrapper);
		fi->setSchemeWrapper(&_ForceIntensityWrapper);

		HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_FORCE_INTENSITY));
		fi->init(bmh, posX, posY, this);
		

		fi->GetClientRect(&r);
		posY += r.bottom + 3;			
	}

	// vortex (to tune viscosity)
	if (dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget))
	{
		CEditableRangeFloat *rv = new CEditableRangeFloat(std::string("RADIAL_VISCOSITY"), 0, 1);
		pushWnd(rv);
		_RadialViscosityWrapper.V = dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget);
		rv->setWrapper(&_RadialViscosityWrapper);
		rv->init(posX + 140, posY, this);
		CStatic *s = new CStatic;			
		pushWnd(s);
		s->Create("Radial viscosity : ", SS_LEFT, CRect(posX, posY, posX + 139, posY + 32), this);
		s->ShowWindow(SW_SHOW);


		rv->GetClientRect(&r);
		posY += r.bottom + 3;

		CEditableRangeFloat *tv = new CEditableRangeFloat(std::string("TANGENTIAL_VISCOSITY"), 0, 1);
		pushWnd(tv);
		_TangentialViscosityWrapper.V = dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget);
		tv->setWrapper(&_TangentialViscosityWrapper);
		tv->init(posX + 140, posY, this);

		s = new CStatic;			
		pushWnd(s);
		s->Create("Tangential Viscosity : ", SS_LEFT, CRect(posX, posY, posX + 139, posY + 32), this);
		s->ShowWindow(SW_SHOW);

		tv->GetClientRect(&r);
		posY += r.bottom + 3;
	}

	// deals with emitters that have a direction
	if (dynamic_cast<NL3D::CPSDirection *>(_LBTarget))
	{
		CDirectionAttr *da = new CDirectionAttr(std::string("DIRECTION"));
		pushWnd(da);		
		_DirectionWrapper.E = dynamic_cast<NL3D::CPSDirection *>(_LBTarget);
		da->setWrapper(&_DirectionWrapper);
		da->setDirectionWrapper(dynamic_cast<NL3D::CPSDirection *>(_LBTarget));
		da->init(posX, posY, this);
		da->GetClientRect(&r);
		posY += r.bottom;
	}

	// Brownian (to tune parametric factor)
	if (dynamic_cast<NL3D::CPSBrownianForce *>(_LBTarget))
	{
		CEditableRangeFloat *rv = new CEditableRangeFloat(std::string("PARAMETRIC_FACTOR"), 0, 64);
		pushWnd(rv);
		_ParamFactorWrapper.F = static_cast<NL3D::CPSBrownianForce *>(_LBTarget);
		rv->setWrapper(&_ParamFactorWrapper);
		rv->init(posX + 140, posY, this);
		CStatic *s = new CStatic;			
		pushWnd(s);
		s->Create("Parametric factor : ", SS_LEFT, CRect(posX, posY, posX + 139, posY + 40), this);
		s->ShowWindow(SW_SHOW);

		rv->GetClientRect(&r);
		posY += r.bottom + 3;
	
	}



	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
