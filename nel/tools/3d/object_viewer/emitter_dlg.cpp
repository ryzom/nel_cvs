/** \file emitter_dlg.cpp
 * a dialog to tune emitter properties in a particle system
 *
 * $Id: emitter_dlg.cpp,v 1.14 2003/04/14 15:30:58 vizerie Exp $
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
#include "emitter_dlg.h"
#include "direction_attr.h"
#include "particle_tree_ctrl.h"

#include "3d/particle_system.h"



/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg dialog


CEmitterDlg::CEmitterDlg(NL3D::CPSEmitter *emitter)
	: _Emitter(emitter), _PeriodDlg(NULL), _GenNbDlg(NULL)
	  , _StrenghtModulateDlg(NULL)
	  , _SpeedInheritanceFactorDlg(NULL)
{
	nlassert(_Emitter);
	//{{AFX_DATA_INIT(CEmitterDlg)
	m_UseSpeedBasis = FALSE;
	m_ConvertSpeedVectorFromEmitterBasis = FALSE;
	m_ConsistentEmission = _Emitter->isConsistentEmissionEnabled();
	m_BypassAutoLOD = FALSE;
	//}}AFX_DATA_INIT
}

CEmitterDlg::~CEmitterDlg()
{
	_PeriodDlg->DestroyWindow();
	_GenNbDlg->DestroyWindow();
	_StrenghtModulateDlg->DestroyWindow();
	_SpeedInheritanceFactorDlg->DestroyWindow();
	_DelayedEmissionDlg->DestroyWindow();
	_MaxEmissionCountDlg->DestroyWindow();

	delete _PeriodDlg;
	delete _GenNbDlg;
	delete _StrenghtModulateDlg;
	delete _SpeedInheritanceFactorDlg;
	delete _DelayedEmissionDlg;
	delete _MaxEmissionCountDlg;
}


void CEmitterDlg::init(CWnd* pParent)
{
	Create(IDD_EMITTER_DIALOG, pParent);	
	// fill the emitted type combo box with all the types of located	
	initEmittedType();	
	m_EmissionTypeCtrl.SetCurSel((int) _Emitter->getEmissionType() );
	ShowWindow(SW_SHOW); 
	UpdateData(FALSE);
}

void CEmitterDlg::initEmittedType()
{	
	m_EmittedTypeCtrl.ResetContent();
	NL3D::CParticleSystem *ps = _Emitter->getOwner()->getOwner();
	uint nbLocated = ps->getNbProcess(); 
	m_EmittedTypeCtrl.InitStorage(nbLocated, 16);	
	for (uint k = 0; k < nbLocated; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k));
		if (loc) // is this a located
		{
			m_EmittedTypeCtrl.AddString(loc->getName().c_str());
			_LocatedList.push_back(loc);			
			if (loc == _Emitter->getEmittedType())
			{
				m_EmittedTypeCtrl.SetCurSel(k);
			}
		}
	}
}

void CEmitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmitterDlg)
	DDX_Control(pDX, IDC_TYPE_OF_EMISSION, m_EmissionTypeCtrl);
	DDX_Control(pDX, IDC_EMITTED_TYPE, m_EmittedTypeCtrl);
	DDX_Check(pDX, IDC_USE_SPEED_BASIS, m_UseSpeedBasis);
	DDX_Check(pDX, IDC_CONVERT_SPEED_VECTOR_FROM_EMITTER_BASIS, m_ConvertSpeedVectorFromEmitterBasis);
	DDX_Check(pDX, IDC_CONSISTENT_EMISSION, m_ConsistentEmission);
	DDX_Check(pDX, IDC_BYPASS_AUTOLOD, m_BypassAutoLOD);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmitterDlg, CDialog)
	//{{AFX_MSG_MAP(CEmitterDlg)
	ON_CBN_SELCHANGE(IDC_EMITTED_TYPE, OnSelchangeEmittedType)
	ON_CBN_SELCHANGE(IDC_TYPE_OF_EMISSION, OnSelchangeTypeOfEmission)
	ON_BN_CLICKED(IDC_USE_SPEED_BASIS, OnUseSpeedBasis)	
	ON_BN_CLICKED(IDC_CONVERT_SPEED_VECTOR_FROM_EMITTER_BASIS, OnConvertSpeedVectorFromEmitterBasis)
	ON_BN_CLICKED(IDC_CONSISTENT_EMISSION, OnConsistentEmission)
	ON_BN_CLICKED(IDC_BYPASS_AUTOLOD, OnBypassAutoLOD)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg message handlers

void CEmitterDlg::OnSelchangeEmittedType() 
{
	UpdateData();
	uint k = m_EmittedTypeCtrl.GetCurSel();
	if (!_Emitter->setEmittedType(_LocatedList[k]))
	{
		MessageBox("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX', and thus, should have a finite duration. This operation create a loop in the system, and so is forbidden.", "Error", MB_ICONEXCLAMATION);
		initEmittedType();
	}	
}

void CEmitterDlg::OnSelchangeTypeOfEmission() 
{
	UpdateData();
	if (!_Emitter->setEmissionType((NL3D::CPSEmitter::TEmissionType) m_EmissionTypeCtrl.GetCurSel()))
	{
		MessageBox(PS_NO_FINITE_DURATION_ARROR_MSG, "Error", MB_ICONEXCLAMATION);
		m_EmissionTypeCtrl.SetCurSel((int) _Emitter->getEmissionType());		
	}

	updatePeriodDlg();
}


void CEmitterDlg::updatePeriodDlg(void)
{
	BOOL bEnable = _Emitter->getEmissionType() == NL3D::CPSEmitter::regular;
	_PeriodDlg->EnableWindow(bEnable);
	_DelayedEmissionDlg->EnableWindow(bEnable);
	_MaxEmissionCountDlg->EnableWindow(bEnable);
}

BOOL CEmitterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	RECT r;

	 GetDlgItem(IDC_SPEED_INHERITANCE_FACTOR_FRAME)->GetWindowRect(&r);
	 ScreenToClient(&r);
	_SpeedInheritanceFactorDlg = new CEditableRangeFloat("SPEED_INHERITANCE_FACTOR", -1.f, 1.f);
	_SpeedInheritanceFactorWrapper.E = _Emitter;
	_SpeedInheritanceFactorDlg->setWrapper(&_SpeedInheritanceFactorWrapper);
	_SpeedInheritanceFactorDlg->init(r.left, r.top, this);

	 GetDlgItem(IDC_DELAYED_EMISSION_FRAME)->GetWindowRect(&r);
	 ScreenToClient(&r);
	_DelayedEmissionDlg = new CEditableRangeFloat("DELAYED_EMISSION", 0.f, 10.f);
	_DelayedEmissionDlg->enableLowerBound(0.f, false);
	_DelayedEmissionWrapper.E = _Emitter;
	_DelayedEmissionDlg->setWrapper(&_DelayedEmissionWrapper);
	_DelayedEmissionDlg->init(r.left, r.top, this);

	GetDlgItem(IDC_MAX_EMISSION_COUNT_FRAME)->GetWindowRect(&r);
	 ScreenToClient(&r);
	_MaxEmissionCountDlg = new CEditableRangeUInt("MAX_EMISSION_COUNT", 0, 100);	
	_MaxEmissionCountDlg->enableUpperBound(256, false);
	_MaxEmissionCountWrapper.E = _Emitter;
	_MaxEmissionCountWrapper.HWnd = (HWND) (*this);
	_MaxEmissionCountDlg->setWrapper(&_MaxEmissionCountWrapper);
	_MaxEmissionCountDlg->init(r.left, r.top, this);
	_MaxEmissionCountWrapper.MaxEmissionCountDlg = _MaxEmissionCountDlg;



	uint posX = 13;
	uint posY = r.bottom + 5;	

	// setup the dialog for the period of emission edition

	_PeriodDlg = new CAttribDlgFloat("EMISSION_PERIOD", 0.f, 2.f);	
	_PeriodWrapper.E = _Emitter;
	_PeriodDlg->setWrapper(&_PeriodWrapper);
	_PeriodDlg->setSchemeWrapper(&_PeriodWrapper);
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_EMISSION_PERIOD));
	_PeriodDlg->init(bmh, posX, posY, this);
	posY += 120;

	// setup the dialog that helps tuning the number of particle being emitted at a time

	_GenNbDlg = new CAttribDlgUInt("EMISSION_GEN_NB",1,11);
	_GenNbWrapper.E = _Emitter;
	_GenNbDlg->setWrapper(&_GenNbWrapper);
	_GenNbDlg->setSchemeWrapper(&_GenNbWrapper);
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_EMISSION_QUANTITY));
	_GenNbDlg->init(bmh, posX, posY, this);
	posY += 120;

	if (dynamic_cast<NL3D::CPSModulatedEmitter *>(_Emitter))
	{
		_StrenghtModulateDlg = new CAttribDlgFloat("EMISSION_GEN_NB",1,11);
		_ModulatedStrenghtWrapper.E = dynamic_cast<NL3D::CPSModulatedEmitter *>(_Emitter);
		_StrenghtModulateDlg->setWrapper(&_ModulatedStrenghtWrapper);
		_StrenghtModulateDlg->setSchemeWrapper(&_ModulatedStrenghtWrapper);
		bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_MODULATE_STRENGHT));
		_StrenghtModulateDlg->init(bmh, posX, posY, this);
		posY += 120;
	}

	// deals with emitters that have a direction
	if (dynamic_cast<NL3D::CPSDirection *>(_Emitter))
	{
		CDirectionAttr *da = new CDirectionAttr(std::string("DIRECTION"));
		pushWnd(da);		
		_DirectionWrapper.E = dynamic_cast<NL3D::CPSDirection *>(_Emitter);
		da->setWrapper(&_DirectionWrapper);
		da->setDirectionWrapper(dynamic_cast<NL3D::CPSDirection *>(_Emitter));
		da->init(posX, posY, this);
		da->GetClientRect(&r);
		posY += r.bottom;
	}

	// radius  for conic emitter
	if (dynamic_cast<NL3D::CPSEmitterConic *>(_Emitter))
	{
		CEditableRangeFloat *ecr = new CEditableRangeFloat(std::string("CONIC EMITTER RADIUS"), 0.1f, 2.1f);
		pushWnd(ecr);
		_ConicEmitterRadiusWrapper.E = dynamic_cast<NL3D::CPSEmitterConic *>(_Emitter);
		ecr->setWrapper(&_ConicEmitterRadiusWrapper);
		ecr->init(posX + 80, posY, this);

		CStatic *s = new CStatic;			
		pushWnd(s);
		s->Create("Radius :", SS_LEFT, CRect(posX, posY + 10 , posX + 70, posY + 32), this);
		s->ShowWindow(SW_SHOW);

		ecr->GetClientRect(&r);
		posY += r.bottom;
	}


	m_UseSpeedBasis = _Emitter->isSpeedBasisEmissionEnabled();
	this->m_ConvertSpeedVectorFromEmitterBasis = _Emitter->isSpeedVectorInEmitterBasis();

	updatePeriodDlg();

	// bypass auto LOD
	nlassert(_Emitter->getOwner() && _Emitter->getOwner()->getOwner());
	NL3D::CParticleSystem &ps = *_Emitter->getOwner()->getOwner();
	CButton *button = (CButton *) GetDlgItem(IDC_BYPASS_AUTOLOD);
	if (ps.isAutoLODEnabled() && !ps.isSharingEnabled())
	{		
		button->EnableWindow(TRUE);
		button->SetCheck(_Emitter->getBypassAutoLOD() ? 1 : 0);
	}
	else
	{
		button->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmitterDlg::OnUseSpeedBasis() 
{
	UpdateData();
	_Emitter->enableSpeedBasisEmission(m_UseSpeedBasis ? true : false);
	UpdateData(TRUE);
}

void CEmitterDlg::OnConvertSpeedVectorFromEmitterBasis() 
{
	UpdateData();
	_Emitter->setSpeedVectorInEmitterBasis(m_ConvertSpeedVectorFromEmitterBasis ? true : false);
	UpdateData(TRUE);
}


void CEmitterDlg::OnConsistentEmission() 
{
	UpdateData();
	_Emitter->enableConsistenEmission(m_ConsistentEmission != 0 ? true : false /* VC6 warning */);
	UpdateData(TRUE);
}

void CEmitterDlg::OnBypassAutoLOD() 
{
	UpdateData();
	_Emitter->setBypassAutoLOD(m_BypassAutoLOD ? true : false);
	UpdateData(TRUE);	
}

void CEmitterDlg::CMaxEmissionCountWrapper::set(const uint32 &count)
{
   if (!E->setMaxEmissionCount((uint8) count))
   {
	   ::MessageBox(HWnd, PS_NO_FINITE_DURATION_ARROR_MSG, "Error", MB_ICONEXCLAMATION);
	   MaxEmissionCountDlg->updateValueFromReader();
   }
}
