// particle_system_edit.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "particle_system_edit.h"
#include "3d/particle_system.h"
#include "editable_range.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParticleSystemEdit dialog

CTimeThresholdWrapper CParticleSystemEdit::_TimeThresholdWrapper ;

CMaxViewDistWrapper CParticleSystemEdit::_MaxViewDistWrapper ;

CMaxNbIntegrationWrapper CParticleSystemEdit::_MaxNbIntegrationWrapper  ;

CLODRatioWrapper CParticleSystemEdit::_LODRatioWrapper ;

CUserParamWrapper CParticleSystemEdit::_UserParamWrapper[4] ;

CParticleSystemEdit::CParticleSystemEdit(NL3D::CParticleSystem *ps)
	: _PS(ps), _TimeThresholdDlg(NULL), _MaxIntegrationStepDlg(NULL)
		, _MaxViewDistDlg(NULL), _LODRatioDlg(NULL)
{
	//{{AFX_DATA_INIT(CParticleSystemEdit)
	m_AccurateIntegration = FALSE;
	m_EnableSlowDown = FALSE;
	m_DieWhenOutOfRange = FALSE;
	m_DieWhenOutOfFrustum = FALSE;
	m_PerformMotionWhenOutOfFrustum = FALSE;
	//}}AFX_DATA_INIT
}


CParticleSystemEdit::~CParticleSystemEdit()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow(); delete wnd; }
	REMOVE_WND(_TimeThresholdDlg);	
	REMOVE_WND(_MaxIntegrationStepDlg);
	REMOVE_WND(_MaxViewDistDlg);
	REMOVE_WND(_LODRatioDlg); 
}

void CParticleSystemEdit::init(CWnd *pParent)   // standard constructor
{
	Create(CParticleSystemEdit::IDD, pParent);


	const sint xPos = 80;
	sint yPos = 157;

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
	_LODRatioDlg->init(87, 350, this);


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
		CEditableRangeFloat *erf = new CEditableRangeFloat (std::string("USER PARAM") + (char) (k + 65), 0, 0.999f);
		erf->enableLowerBound(0, false);
		erf->enableUpperBound(1, true);
		pushWnd(erf);
		erf->setWrapper(&_UserParamWrapper[k]);	
		erf->init(xPos, yPos, this);
		yPos += 35;
	}


	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	_PS->getAccurateIntegrationParams(t, max, csd);

	m_AccurateIntegration = _PS->isAccurateIntegrationEnabled();
	m_PrecomputeBBoxCtrl.SetCheck(!_PS->getAutoComputeBBox());
	m_EnableSlowDown = csd;
	m_DieWhenOutOfRange = _PS->getDestroyModelWhenOutOfRange();
	m_DieWhenOutOfFrustum = _PS->doesDestroyWhenOutOfFrustum();
	m_PerformMotionWhenOutOfFrustum = _PS->doesPerformMotionWhenOutOfFrustum();
	m_PerformMotionWhenOutOfFrustumDlg.EnableWindow(!_PS->doesDestroyWhenOutOfFrustum());
	m_DieOnEvent.SetCurSel((int) _PS->getDestroyCondition());

	updateIntegrationParams();
	updatePrecomputedBBoxParams();
	updateDieOnEventParams();

	UpdateData(FALSE);
	ShowWindow(SW_SHOW);	
}

void CParticleSystemEdit::updateIntegrationParams(void)
{
	BOOL ew = _PS->isAccurateIntegrationEnabled();
	_TimeThresholdDlg->EnableWindow(ew);
	_MaxIntegrationStepDlg->EnableWindow(ew);
	m_EnableSlowDownCtrl.EnableWindow(ew);
}


void CParticleSystemEdit::updateDieOnEventParams(void)
{
	BOOL ew = _PS->getDestroyCondition() == NL3D::CParticleSystem::none ? FALSE : TRUE;
	GetDlgItem(IDC_APPLY_AFTRE_DELAY)->EnableWindow(ew);
	char out[128];
	sprintf(out, "%.2g", _PS->getDelayBeforeDeathConditionTest());
	GetDlgItem(IDC_APPLY_AFTRE_DELAY)->SetWindowText(out);
}

void CParticleSystemEdit::OnPrecomputeBbox() 
{
	UpdateData();
	_PS->setAutoComputeBBox(!_PS->getAutoComputeBBox() ? true : false);
	updatePrecomputedBBoxParams();	
}

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



void CParticleSystemEdit::OnSelchangePsDieOnEvent() 
{
	UpdateData();
	_PS->setDestroyCondition((NL3D::CParticleSystem::TDieCondition) m_DieOnEvent.GetCurSel());
	updateDieOnEventParams();
}



void CParticleSystemEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParticleSystemEdit)
	DDX_Control(pDX, IDC_PERFORM_MOTION_WHEN_OUT_OF_FRUSTUM, m_PerformMotionWhenOutOfFrustumDlg);
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
	DDX_Check(pDX, IDC_PERFORM_MOTION_WHEN_OUT_OF_FRUSTUM, m_PerformMotionWhenOutOfFrustum);
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
	ON_BN_CLICKED(IDC_PERFORM_MOTION_WHEN_OUT_OF_FRUSTUM, OnPerformMotionWhenOutOfFrustum)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParticleSystemEdit message handlers

void CParticleSystemEdit::OnAccurateIntegration() 
{
	UpdateData();
	_PS->enableAccurateIntegration(m_AccurateIntegration ? true : false);
	updateIntegrationParams();
}

void CParticleSystemEdit::OnEnableSlowDown() 
{
	UpdateData();
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	_PS->getAccurateIntegrationParams(t, max, csd);
	_PS->setAccurateIntegrationParams(t, max, m_EnableSlowDown ? true : false);
}

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


void CParticleSystemEdit::OnIncBbox() 
{
	NLMISC::CAABBox b;
	_PS->computeBBox(b);
	b.setHalfSize(1.1f * b.getHalfSize());
	_PS->setPrecomputedBBox(b);
	updatePrecomputedBBoxParams();
}

void CParticleSystemEdit::OnDecBbox() 
{
	NLMISC::CAABBox b;
	_PS->computeBBox(b);	 
	b.setHalfSize(0.9f * b.getHalfSize());
	_PS->setPrecomputedBBox(b);
	updatePrecomputedBBoxParams();	
}


void CParticleSystemEdit::OnDieWhenOutOfRange() 
{
	UpdateData();
	_PS->setDestroyModelWhenOutOfRange(m_DieWhenOutOfRange ? true : false);
}



void CParticleSystemEdit::OnDieWhenOutOfFrustum() 
{
	UpdateData();
	_PS->destroyWhenOutOfFrustum(m_DieWhenOutOfFrustum ? true : false);
	m_PerformMotionWhenOutOfFrustumDlg.EnableWindow(!m_DieWhenOutOfFrustum);
}



void CParticleSystemEdit::OnPerformMotionWhenOutOfFrustum() 
{
	UpdateData();
	_PS->performMotionWhenOutOfFrustum(m_PerformMotionWhenOutOfFrustum ? true : false);	
}



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



/////////////////////////////
// WRAPPERS IMPLEMENTATION //
/////////////////////////////

float CTimeThresholdWrapper::get(void) const
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	PS->getAccurateIntegrationParams(t, max, csd);
	return t;
}
void CTimeThresholdWrapper::set(const float &tt)
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	PS->getAccurateIntegrationParams(t, max, csd);
	PS->setAccurateIntegrationParams(tt, max, csd);	
}

uint32 CMaxNbIntegrationWrapper::get(void) const
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	PS->getAccurateIntegrationParams(t, max, csd);
	return max;
}
void CMaxNbIntegrationWrapper::set(const uint32 &nmax)
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	PS->getAccurateIntegrationParams(t, max, csd);
	PS->setAccurateIntegrationParams(t, nmax, csd);	
}
	
float CUserParamWrapper::get(void) const 
{ 
	return PS->getUserParam(Index); 
}
void CUserParamWrapper::set(const float &v)
{
	PS->setUserParam(Index, v); 
}
	


float CMaxViewDistWrapper::get(void) const
{
	return PS->getMaxViewDist();
}

void CMaxViewDistWrapper::set(const float &d)
{
	PS->setMaxViewDist(d);
}

float CLODRatioWrapper::get(void) const
{
	return PS->getLODRatio();
}
void CLODRatioWrapper::set(const float &v)
{
	PS->setLODRatio(v);
}
