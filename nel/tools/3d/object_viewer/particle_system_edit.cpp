 /** \file particle_system_edit.cpp
 * Dialog used to edit global parameters of a particle system.
 *
 * $Id: particle_system_edit.cpp,v 1.15 2003/04/14 15:32:43 vizerie Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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
#include "particle_system_edit.h"
#include "3d/particle_system.h"
#include "3d/ps_color.h"
#include "editable_range.h"
#include "auto_lod_dlg.h"
#include "ps_global_color_dlg.h"
#include "choose_name.h"


/////////////////////////////////////////////////////////////////////////////
// CParticleSystemEdit dialog

CTimeThresholdWrapper CParticleSystemEdit::_TimeThresholdWrapper;

CMaxViewDistWrapper CParticleSystemEdit::_MaxViewDistWrapper;

CMaxNbIntegrationWrapper CParticleSystemEdit::_MaxNbIntegrationWrapper;

CLODRatioWrapper CParticleSystemEdit::_LODRatioWrapper;

CUserParamWrapper CParticleSystemEdit::_UserParamWrapper[4];

//=====================================================
CParticleSystemEdit::CParticleSystemEdit(NL3D::CParticleSystem *ps)
	: _PS(ps), _TimeThresholdDlg(NULL), _MaxIntegrationStepDlg(NULL)
		, _MaxViewDistDlg(NULL), _LODRatioDlg(NULL), _AutoLODDlg(NULL),
		_GlobalColorDlg(NULL)
{
	//{{AFX_DATA_INIT(CParticleSystemEdit)
	m_AccurateIntegration = FALSE;
	m_EnableSlowDown = FALSE;
	m_DieWhenOutOfRange = FALSE;
	m_DieWhenOutOfFrustum = FALSE;
	m_EnableLoadBalancing = FALSE;
	m_BypassMaxNumSteps = FALSE;
	//}}AFX_DATA_INIT
}


//=====================================================
CParticleSystemEdit::~CParticleSystemEdit()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow(); delete wnd; wnd = NULL; }
	REMOVE_WND(_TimeThresholdDlg);	
	REMOVE_WND(_MaxIntegrationStepDlg);
	REMOVE_WND(_MaxViewDistDlg);
	REMOVE_WND(_LODRatioDlg); 
	REMOVE_WND(_AutoLODDlg);
	REMOVE_WND(_GlobalColorDlg);
}

//=====================================================
void CParticleSystemEdit::init(CWnd *pParent)   // standard constructor
{
	Create(CParticleSystemEdit::IDD, pParent);


	const sint xPos = 80;
	sint yPos = 162;

	_MaxViewDistDlg = new CEditableRangeFloat (std::string("MAX VIEW DIST"), 0, 100.f);
	_MaxViewDistWrapper.PS = _PS;
	_MaxViewDistDlg->enableLowerBound(0, true);
	_MaxViewDistDlg->setWrapper(&_MaxViewDistWrapper);
	_MaxViewDistDlg->init(87, 325, this);

	_LODRatioDlg = new CEditableRangeFloat (std::string("LOD RATIO"), 0, 1.f);
	_LODRatioWrapper.PS = _PS;
	_LODRatioDlg->enableLowerBound(0, true);
	_LODRatioDlg->enableUpperBound(1, true);
	_LODRatioDlg->setWrapper(&_LODRatioWrapper);
	_LODRatioDlg->init(87, 357, this);


	_TimeThresholdDlg = new CEditableRangeFloat (std::string("TIME THRESHOLD"), 0, 0.5f);
	_TimeThresholdWrapper.PS = _PS;
	_TimeThresholdDlg->enableLowerBound(0, true);
	_TimeThresholdDlg->setWrapper(&_TimeThresholdWrapper);
	_TimeThresholdDlg->init(87, 74, this);


	_MaxIntegrationStepDlg = new CEditableRangeUInt (std::string("MAX INTEGRATION STEPS"), 0, 4);	
	_MaxNbIntegrationWrapper.PS = _PS;
	_MaxIntegrationStepDlg->enableLowerBound(0, true);
	_MaxIntegrationStepDlg->setWrapper(&_MaxNbIntegrationWrapper);
	_MaxIntegrationStepDlg->init(87, 115, this);


				
	for (uint k = 0; k < 4; ++k)
	{
		_UserParamWrapper[k].PS = _PS;
		_UserParamWrapper[k].Index = k;
		CEditableRangeFloat *erf = new CEditableRangeFloat (std::string("USER PARAM") + (char) (k + 65), 0, 1.0f);
		erf->enableLowerBound(0, false);
		erf->enableUpperBound(1, false);
		pushWnd(erf);
		erf->setWrapper(&_UserParamWrapper[k]);	
		erf->init(xPos, yPos, this);
		yPos += 35;
	}


	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	_PS->getAccurateIntegrationParams(t, max, csd, klt);
	
	m_PrecomputeBBoxCtrl.SetCheck(!_PS->getAutoComputeBBox());
	m_EnableSlowDown = csd;	
	((CButton *)	GetDlgItem(IDC_SHARABLE))->SetCheck(_PS->isSharingEnabled());	

	m_AccurateIntegration = _PS->isAccurateIntegrationEnabled();

	BOOL bAutoLOD = _PS->isAutoLODEnabled();
	((CButton *)	GetDlgItem(IDC_ENABLE_AUTO_LOD))->SetCheck(bAutoLOD);
	GetDlgItem(IDC_EDIT_AUTO_LOD)->EnableWindow(bAutoLOD);


	/// global color
	int bGlobalColor = _PS->getColorAttenuationScheme() != NULL ?  1 : 0;
	((CButton *) GetDlgItem(IDC_GLOBAL_COLOR))->SetCheck(bGlobalColor);
	GetDlgItem(IDC_EDIT_GLOBAL_COLOR)->EnableWindow(bGlobalColor);



	updateIntegrationParams();
	updatePrecomputedBBoxParams();	
	updateLifeMgtPresets();

	m_EnableLoadBalancing = _PS->isLoadBalancingEnabled();
	_MaxIntegrationStepDlg->EnableWindow(_PS->getBypassMaxNumIntegrationSteps() ? FALSE : TRUE);

	UpdateData(FALSE);
	ShowWindow(SW_SHOW);	
}

//=====================================================
void CParticleSystemEdit::updateIntegrationParams(void)
{
	BOOL ew = _PS->isAccurateIntegrationEnabled();
	_TimeThresholdDlg->EnableWindow(ew);
	_MaxIntegrationStepDlg->EnableWindow(ew);
	m_EnableSlowDownCtrl.EnableWindow(ew);
}


//=====================================================
void CParticleSystemEdit::updateDieOnEventParams(void)
{
	BOOL ew = _PS->getDestroyCondition() == NL3D::CParticleSystem::none ? FALSE : TRUE;
	GetDlgItem(IDC_APPLY_AFTRE_DELAY)->EnableWindow(ew);
	char out[128];
	sprintf(out, "%.2g", _PS->getDelayBeforeDeathConditionTest());
	GetDlgItem(IDC_APPLY_AFTRE_DELAY)->SetWindowText(out);
}

//=====================================================
void CParticleSystemEdit::OnPrecomputeBbox() 
{
	UpdateData();
	_PS->setAutoComputeBBox(!_PS->getAutoComputeBBox() ? true : false);
	updatePrecomputedBBoxParams();	
}

//=====================================================
void CParticleSystemEdit::updatePrecomputedBBoxParams(void)
{
	BOOL ew = !_PS->getAutoComputeBBox();
	m_BBoxXCtrl.EnableWindow(ew);
	m_BBoxYCtrl.EnableWindow(ew);
	m_BBoxZCtrl.EnableWindow(ew);
	
	if (!ew)
	{
		m_BBoxXCtrl.SetWindowText("");
		m_BBoxYCtrl.SetWindowText("");
		m_BBoxZCtrl.SetWindowText("");
	}
	else
	{
		NLMISC::CAABBox b;
		_PS->computeBBox(b);
		char out[128];
		sprintf(out, "%.3g", b.getHalfSize().x); m_BBoxXCtrl.SetWindowText(out);
		sprintf(out, "%.3g", b.getHalfSize().y); m_BBoxYCtrl.SetWindowText(out);
		sprintf(out, "%.3g", b.getHalfSize().z); m_BBoxZCtrl.SetWindowText(out);
	}

	UpdateData(FALSE);
}

//=====================================================
void CParticleSystemEdit::OnSelchangePsDieOnEvent() 
{
	UpdateData();
	_PS->setDestroyCondition((NL3D::CParticleSystem::TDieCondition) m_DieOnEvent.GetCurSel());
	updateDieOnEventParams();
}

//=====================================================
void CParticleSystemEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParticleSystemEdit)
	DDX_Control(pDX, IDC_BYPASS_MAX_NUM_STEPS, m_BypassMaxNumStepsCtrl);
	DDX_Control(pDX, IDC_FORCE_LIFE_TIME_UPDATE, m_ForceLifeTimeUpdate);
	DDX_Control(pDX, IDC_DIE_WHEN_OUT_OF_FRUSTRUM, m_DieWhenOutOfFrustumCtrl);
	DDX_Control(pDX, IDC_DIE_WHEN_OUT_OF_RANGE, m_DieWhenOutOfRangeCtrl);
	DDX_Control(pDX, IDC_ANIM_TYPE_CTRL, m_AnimTypeCtrl);
	DDX_Control(pDX, IDC_LIFE_MGT_PRESETS, m_PresetCtrl);
	DDX_Control(pDX, IDC_PS_DIE_ON_EVENT, m_DieOnEvent);
	DDX_Control(pDX, IDC_PRECOMPUTE_BBOX, m_PrecomputeBBoxCtrl);
	DDX_Control(pDX, IDC_BB_Z, m_BBoxZCtrl);
	DDX_Control(pDX, IDC_BB_Y, m_BBoxYCtrl);
	DDX_Control(pDX, IDC_BB_X, m_BBoxXCtrl);
	DDX_Control(pDX, IDC_ENABLE_SLOW_DOWN, m_EnableSlowDownCtrl);
	DDX_Check(pDX, IDC_ACCURATE_INTEGRATION, m_AccurateIntegration);
	DDX_Check(pDX, IDC_ENABLE_SLOW_DOWN, m_EnableSlowDown);
	DDX_Check(pDX, IDC_DIE_WHEN_OUT_OF_RANGE, m_DieWhenOutOfRange);
	DDX_Check(pDX, IDC_DIE_WHEN_OUT_OF_FRUSTRUM, m_DieWhenOutOfFrustum);
	DDX_Check(pDX, IDC_ENABLE_LOAD_BALANCING, m_EnableLoadBalancing);
	DDX_Check(pDX, IDC_BYPASS_MAX_NUM_STEPS, m_BypassMaxNumSteps);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CParticleSystemEdit, CDialog)
	//{{AFX_MSG_MAP(CParticleSystemEdit)
	ON_BN_CLICKED(IDC_ACCURATE_INTEGRATION, OnAccurateIntegration)
	ON_BN_CLICKED(IDC_ENABLE_SLOW_DOWN, OnEnableSlowDown)
	ON_BN_CLICKED(IDC_PRECOMPUTE_BBOX, OnPrecomputeBbox)
	ON_BN_CLICKED(IDC_UPDATE_BBOX, OnUpdateBbox)
	ON_BN_CLICKED(IDC_INC_BBOX, OnIncBbox)
	ON_BN_CLICKED(IDC_DEC_BBOX, OnDecBbox)
	ON_BN_CLICKED(IDC_DIE_WHEN_OUT_OF_RANGE, OnDieWhenOutOfRange)
	ON_CBN_SELCHANGE(IDC_PS_DIE_ON_EVENT, OnSelchangePsDieOnEvent)
	ON_EN_CHANGE(IDC_APPLY_AFTRE_DELAY, OnChangeApplyAfterDelay)
	ON_BN_CLICKED(IDC_DIE_WHEN_OUT_OF_FRUSTRUM, OnDieWhenOutOfFrustum)	
	ON_CBN_SELCHANGE(IDC_LIFE_MGT_PRESETS, OnSelchangeLifeMgtPresets)
	ON_CBN_SELCHANGE(IDC_ANIM_TYPE_CTRL, OnSelchangeAnimTypeCtrl)
	ON_BN_CLICKED(IDC_SHARABLE, OnSharable)
	ON_BN_CLICKED(IDC_EDIT_AUTO_LOD, OnEditAutoLod)
	ON_BN_CLICKED(IDC_ENABLE_AUTO_LOD, OnEnableAutoLod)
	ON_BN_CLICKED(IDC_FORCE_LIFE_TIME_UPDATE, OnForceLifeTimeUpdate)
	ON_BN_CLICKED(IDC_EDIT_GLOBAL_COLOR, OnEditGlobalColor)
	ON_BN_CLICKED(IDC_GLOBAL_COLOR, OnGlobalColor)
	ON_BN_CLICKED(IDC_ENABLE_LOAD_BALANCING, OnEnableLoadBalancing)
	ON_BN_CLICKED(IDC_GLOBAL_USER_PARAM_1, OnGlobalUserParam1)
	ON_BN_CLICKED(IDC_GLOBAL_USER_PARAM_2, OnGlobalUserParam2)
	ON_BN_CLICKED(IDC_GLOBAL_USER_PARAM_3, OnGlobalUserParam3)
	ON_BN_CLICKED(IDC_GLOBAL_USER_PARAM_4, OnGlobalUserParam4)
	ON_BN_CLICKED(IDC_BYPASS_MAX_NUM_STEPS, OnBypassMaxNumSteps)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParticleSystemEdit message handlers

//=====================================================
void CParticleSystemEdit::OnAccurateIntegration() 
{
	UpdateData();
	_PS->enableAccurateIntegration(m_AccurateIntegration ? true : false);
	updateIntegrationParams();
}

//=====================================================
void CParticleSystemEdit::OnEnableSlowDown() 
{
	UpdateData();
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	_PS->getAccurateIntegrationParams(t, max, csd, klt);
	_PS->setAccurateIntegrationParams(t, max, m_EnableSlowDown ? true : false, klt);
}

//=====================================================
void CParticleSystemEdit::OnUpdateBbox() 
{
	UpdateData();
	NLMISC::CVector h;
	char inX[128], inY[128], inZ[128];
	m_BBoxXCtrl.GetWindowText(inX, 128);
	m_BBoxYCtrl.GetWindowText(inY, 128);
	m_BBoxZCtrl.GetWindowText(inZ, 128);
	if (sscanf(inX, "%f", &h.x) == 1
		&& sscanf(inY, "%f", &h.y) == 1
		&& sscanf(inZ, "%f", &h.z) == 1
	   )
	{
		NLMISC::CAABBox b;
		b.setHalfSize(h);
		_PS->setPrecomputedBBox(b);
	}
	else
	{
		MessageBox("Invalid entry","error", MB_OK);
	}
}

//=====================================================
void CParticleSystemEdit::OnIncBbox() 
{
	NLMISC::CAABBox b;
	_PS->computeBBox(b);
	b.setHalfSize(1.1f * b.getHalfSize());
	_PS->setPrecomputedBBox(b);
	updatePrecomputedBBoxParams();
}

//=====================================================
void CParticleSystemEdit::OnDecBbox() 
{
	NLMISC::CAABBox b;
	_PS->computeBBox(b);	 
	b.setHalfSize(0.9f * b.getHalfSize());
	_PS->setPrecomputedBBox(b);
	updatePrecomputedBBoxParams();	
}

//=====================================================
void CParticleSystemEdit::OnDieWhenOutOfRange() 
{
	UpdateData();
	_PS->setDestroyModelWhenOutOfRange(m_DieWhenOutOfRange ? true : false);
}

//=====================================================
void CParticleSystemEdit::OnDieWhenOutOfFrustum() 
{
	UpdateData();
	_PS->destroyWhenOutOfFrustum(m_DieWhenOutOfFrustum ? true : false);
	m_AnimTypeCtrl.EnableWindow(!m_DieWhenOutOfFrustum);
}

//=====================================================
void CParticleSystemEdit::OnChangeApplyAfterDelay() 
{
	char in[128];	
	GetDlgItem(IDC_APPLY_AFTRE_DELAY)->GetWindowText(in, 128);		
	float value;
	if (sscanf(in, "%f", &value) == 1)
	{
		_PS->setDelayBeforeDeathConditionTest(value);
	}
}

//=====================================================
void CParticleSystemEdit::OnSelchangeLifeMgtPresets() 
{
	UpdateData(TRUE);
	if (m_PresetCtrl.GetCurSel() == NL3D::CParticleSystem::SpellFX)
	{
		if (!_PS->canFinish())
		{
			m_PresetCtrl.SetCurSel((int) _PS->getBehaviourType());
			MessageBox("The system must have a finite duration for this setting! Please check that.", "Error", MB_ICONEXCLAMATION);
			return;
		}
	}
	_PS->activatePresetBehaviour((NL3D::CParticleSystem::TPresetBehaviour) m_PresetCtrl.GetCurSel());	
	updateLifeMgtPresets();
}

//=====================================================
void CParticleSystemEdit::OnSelchangeAnimTypeCtrl() 
{
	UpdateData(TRUE);
	_PS->setAnimType((NL3D::CParticleSystem::TAnimType) m_AnimTypeCtrl.GetCurSel());
}

//=====================================================
void CParticleSystemEdit::updateLifeMgtPresets()
{
	m_PresetCtrl.SetCurSel((int) _PS->getBehaviourType());
	m_DieWhenOutOfRange = _PS->getDestroyModelWhenOutOfRange();
	m_DieWhenOutOfFrustum = _PS->doesDestroyWhenOutOfFrustum();
	m_BypassMaxNumSteps = _PS->getBypassMaxNumIntegrationSteps();
	m_DieOnEvent.SetCurSel((int) _PS->getDestroyCondition());
	m_AnimTypeCtrl.SetCurSel((int) _PS->getAnimType());
	updateDieOnEventParams();

	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	_PS->getAccurateIntegrationParams(t, max, csd, klt);
	((CButton *)	GetDlgItem(IDC_FORCE_LIFE_TIME_UPDATE))->SetCheck(klt);

	BOOL bEnable =  _PS->getBehaviourType() == NL3D::CParticleSystem::UserBehaviour ? TRUE :  FALSE;
	
	m_DieWhenOutOfRangeCtrl.EnableWindow(bEnable);
	m_DieWhenOutOfFrustumCtrl.EnableWindow(bEnable);
	m_DieOnEvent.EnableWindow(bEnable);
	m_AnimTypeCtrl.EnableWindow(bEnable);
	m_ForceLifeTimeUpdate.EnableWindow(bEnable);
	m_BypassMaxNumStepsCtrl.EnableWindow(bEnable);
	_MaxIntegrationStepDlg->EnableWindow(_PS->getBypassMaxNumIntegrationSteps() ? FALSE : TRUE);

	UpdateData(FALSE);
}



//=====================================================
void CParticleSystemEdit::OnSharable() 
{
	bool shared = ((CButton *)	GetDlgItem(IDC_SHARABLE))->GetCheck() != 0;
	_PS->enableSharing(shared);	
}

//=====================================================
void CParticleSystemEdit::OnEditAutoLod() 
{
	GetDlgItem(IDC_EDIT_AUTO_LOD)->EnableWindow(FALSE);
	GetDlgItem(IDC_ENABLE_AUTO_LOD)->EnableWindow(FALSE);
	GetDlgItem(IDC_SHARABLE)->EnableWindow(FALSE);	
	nlassert(_AutoLODDlg == NULL);
	CAutoLODDlg *autoLODDlg = new CAutoLODDlg(_PS, this, this);	
	autoLODDlg->init(this);
	_AutoLODDlg = autoLODDlg;
}

//=====================================================
void CParticleSystemEdit::childPopupClosed(CWnd *child)
{
	if (child == _AutoLODDlg)
	{
		GetDlgItem(IDC_EDIT_AUTO_LOD)->EnableWindow(TRUE);
		GetDlgItem(IDC_ENABLE_AUTO_LOD)->EnableWindow(TRUE);
		GetDlgItem(IDC_SHARABLE)->EnableWindow(TRUE);
		REMOVE_WND(_AutoLODDlg);
	}
	else
	if (child == _GlobalColorDlg)
	{
		GetDlgItem(IDC_GLOBAL_COLOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_GLOBAL_COLOR)->EnableWindow(TRUE);
		REMOVE_WND(_GlobalColorDlg);
	}
			
}

//=====================================================
void CParticleSystemEdit::OnEnableAutoLod() 
{
	BOOL bEnable = ((CButton *) GetDlgItem(IDC_ENABLE_AUTO_LOD))->GetCheck() != 0;
	GetDlgItem(IDC_EDIT_AUTO_LOD)->EnableWindow(bEnable);
	_PS->enableAutoLOD(bEnable ? true : false /* performance warning */);
}

///=====================================================================
void CParticleSystemEdit::OnEditGlobalColor() 
{
	nlassert(!_GlobalColorDlg);
	GetDlgItem(IDC_GLOBAL_COLOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_GLOBAL_COLOR)->EnableWindow(FALSE);		
	CPSGlobalColorDlg *gcd = new CPSGlobalColorDlg(_PS, this, this);
	gcd->init(this);
	_GlobalColorDlg = gcd;
}

///=====================================================================
void CParticleSystemEdit::OnGlobalColor() 
{	
	/// if the system hasn't a global color scheme, add one.
	if (_PS->getColorAttenuationScheme() == NULL)
	{
		_PS->setColorAttenuationScheme(new NL3D::CPSColorGradient);
		GetDlgItem(IDC_EDIT_GLOBAL_COLOR)->EnableWindow(TRUE);
	}
	else
	{
		_PS->setColorAttenuationScheme(NULL);
		GetDlgItem(IDC_EDIT_GLOBAL_COLOR)->EnableWindow(FALSE);
	}
}


/////////////////////////////
// WRAPPERS IMPLEMENTATION //
/////////////////////////////

//=====================================================
float CTimeThresholdWrapper::get(void) const
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	PS->getAccurateIntegrationParams(t, max, csd, klt);
	return t;
}

//=====================================================
void CTimeThresholdWrapper::set(const float &tt)
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	PS->getAccurateIntegrationParams(t, max, csd, klt);
	PS->setAccurateIntegrationParams(tt, max, csd, klt);	
}


//=====================================================
uint32 CMaxNbIntegrationWrapper::get(void) const
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	PS->getAccurateIntegrationParams(t, max, csd, klt);
	return max;
}

//=====================================================
void CMaxNbIntegrationWrapper::set(const uint32 &nmax)
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	PS->getAccurateIntegrationParams(t, max, csd, klt);
	PS->setAccurateIntegrationParams(t, nmax, csd, klt);	
}
	
//=====================================================
float CUserParamWrapper::get(void) const 
{ 
	return PS->getUserParam(Index); 
}

//=====================================================
void CUserParamWrapper::set(const float &v)
{
	PS->setUserParam(Index, v); 
}

//=====================================================
float CMaxViewDistWrapper::get(void) const
{
	return PS->getMaxViewDist();
}

//=====================================================
void CMaxViewDistWrapper::set(const float &d)
{
	PS->setMaxViewDist(d);
}

//=====================================================
float CLODRatioWrapper::get(void) const
{
	return PS->getLODRatio();
}

//=====================================================
void CLODRatioWrapper::set(const float &v)
{
	PS->setLODRatio(v);
}

//=====================================================
void CParticleSystemEdit::OnForceLifeTimeUpdate() 
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;	
	_PS->getAccurateIntegrationParams(t, max, csd, klt);
	klt = ((CButton *)	GetDlgItem(IDC_FORCE_LIFE_TIME_UPDATE))->GetCheck() != 0;
	_PS->setAccurateIntegrationParams(t, max, csd, klt);
}

//=====================================================
void CParticleSystemEdit::OnEnableLoadBalancing() 
{
	UpdateData(TRUE);
	if (m_EnableLoadBalancing == FALSE)
	{
		int result = MessageBox("Are you sure ?", "Load balancing on/off", MB_OKCANCEL);
		if (result == IDOK)
		{
			_PS->enableLoadBalancing(false);
		}
		else
		{
			m_EnableLoadBalancing = TRUE;
		}
	}
	else
	{
		_PS->enableLoadBalancing(false);
	}
	UpdateData(FALSE);
}

//=====================================================
static void chooseGlobalUserParam(uint userParam, NL3D::CParticleSystem *ps, CWnd *parent)
{
	nlassert(ps);
	CChooseName cn(!ps->getGlobalValueName(userParam).empty() ? ps->getGlobalValueName(userParam).c_str() : "", parent);
	if (cn.DoModal() == IDOK)
	{
		ps->bindGlobalValueToUserParam(cn.getName().c_str(), userParam);
	}
}	

//=====================================================
void CParticleSystemEdit::OnGlobalUserParam1() 
{
	nlassert(_PS)
	chooseGlobalUserParam(0, _PS, this);
}

//=====================================================
void CParticleSystemEdit::OnGlobalUserParam2() 
{	
	chooseGlobalUserParam(1, _PS, this);
}

//=====================================================
void CParticleSystemEdit::OnGlobalUserParam3() 
{
	chooseGlobalUserParam(2, _PS, this);
}

//=====================================================
void CParticleSystemEdit::OnGlobalUserParam4() 
{
	chooseGlobalUserParam(3, _PS, this);
}

//=====================================================
void CParticleSystemEdit::OnBypassMaxNumSteps() 
{
	UpdateData(TRUE);
	if (m_BypassMaxNumSteps && !_PS->canFinish())
	{		
		MessageBox("The system must have a finite duration for this setting! Please check that.", "error", MB_ICONEXCLAMATION);
		return;
	}
	_PS->setBypassMaxNumIntegrationSteps(m_BypassMaxNumSteps != FALSE);	
	_MaxIntegrationStepDlg->EnableWindow(_PS->getBypassMaxNumIntegrationSteps() ? FALSE : TRUE);
}
