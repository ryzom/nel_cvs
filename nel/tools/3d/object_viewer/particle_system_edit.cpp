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


CParticleSystemEdit::CParticleSystemEdit(NL3D::CParticleSystem *ps)
	: _PS(ps), _TimeThresholdDlg(NULL), _MaxIntegrationStepDlg(NULL)
		, _MaxViewDistDlg(NULL), _LODRatioDlg(NULL)
{
	//{{AFX_DATA_INIT(CParticleSystemEdit)
	m_AccurateIntegration = FALSE;
	m_EnableSlowDown = FALSE;
	m_PrecomputeBBox = FALSE;
	//}}AFX_DATA_INIT
}


CParticleSystemEdit::~CParticleSystemEdit()
{
	#define  REMOVE_WND(wnd) if (wnd) { wnd->DestroyWindow() ; delete wnd ; }
	REMOVE_WND(_TimeThresholdDlg)  ;	
	REMOVE_WND(_MaxIntegrationStepDlg)  ;
	REMOVE_WND(_MaxViewDistDlg) ;
	REMOVE_WND(_LODRatioDlg) ; 
}

CParticleSystemEdit::init(CWnd *pParent)   // standard constructor
{
	Create(CParticleSystemEdit::IDD, pParent) ;


	const sint xPos = 80 ;
	sint yPos = 157 ;

	_MaxViewDistDlg = new CEditableRangeFloat (std::string("MAX VIEW DIST"), 0, 100.f) ;
	_MaxViewDistWrapper.PS = _PS ;
	_MaxViewDistDlg->enableLowerBound(0, true) ;
	_MaxViewDistDlg->setWrapper(&_MaxViewDistWrapper) ;
	_MaxViewDistDlg->init(87, 325, this) ;

	_LODRatioDlg = new CEditableRangeFloat (std::string("LOD RATIO"), 0, 1.f) ;
	_LODRatioWrapper.PS = _PS ;
	_LODRatioDlg->enableLowerBound(0, true) ;
	_LODRatioDlg->enableUpperBound(1, true) ;
	_LODRatioDlg->setWrapper(&_LODRatioWrapper) ;
	_LODRatioDlg->init(87, 350, this) ;


	_TimeThresholdDlg = new CEditableRangeFloat (std::string("TIME THRESHOLD"), 0, 0.5f) ;
	_TimeThresholdWrapper.PS = _PS ;
	_TimeThresholdDlg->enableLowerBound(0, true) ;
	_TimeThresholdDlg->setWrapper(&_TimeThresholdWrapper) ;
	_TimeThresholdDlg->init(87, 74, this) ;


	_MaxIntegrationStepDlg = new CEditableRangeUInt (std::string("MAX INTEGRATION STEPS"), 0, 4) ;	
	_MaxNbIntegrationWrapper.PS = _PS ;
	_MaxIntegrationStepDlg->enableLowerBound(0, true) ;
	_MaxIntegrationStepDlg->setWrapper(&_MaxNbIntegrationWrapper) ;
	_MaxIntegrationStepDlg->init(87, 115, this) ;


				
	for (uint k = 0 ; k < 4 ; ++k)
	{
		_UserParamWrapper[k].PS = _PS ;
		_UserParamWrapper[k].Index = k ;
		CEditableRangeFloat *erf = new CEditableRangeFloat (std::string("USER PARAM") + (char) (k + 65), 0, 0.999f) ;
		erf->enableLowerBound(0, false) ;
		erf->enableUpperBound(1, true) ;
		pushWnd(erf) ;
		erf->setWrapper(&_UserParamWrapper[k]) ;	
		erf->init(xPos, yPos, this) ;
		yPos += 35 ;
	}


	NL3D::CAnimationTime t ;
	uint32 max ;
	bool csd ;
	_PS->getAccurateIntegrationParams(t, max, csd) ;

	m_AccurateIntegration = _PS->isAccurateIntegrationEnabled() ;
	m_EnableSlowDown = csd ;

	updateIntegrationParams() ;


	UpdateData(FALSE) ;
	ShowWindow(SW_SHOW) ;	
}

void CParticleSystemEdit::updateIntegrationParams(void)
{
	BOOL ew = _PS->isAccurateIntegrationEnabled() ;
	_TimeThresholdDlg->EnableWindow(ew) ;
	_MaxIntegrationStepDlg->EnableWindow(ew) ;
	m_EnableSlowDownCtrl.EnableWindow(ew) ;
}

void CParticleSystemEdit::updatePrecomputedBBoxParams(void)
{
	
}

void CParticleSystemEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParticleSystemEdit)
	DDX_Control(pDX, IDC_ENABLE_SLOW_DOWN, m_EnableSlowDownCtrl);
	DDX_Check(pDX, IDC_ACCURATE_INTEGRATION, m_AccurateIntegration);
	DDX_Check(pDX, IDC_ENABLE_SLOW_DOWN, m_EnableSlowDown);
	DDX_Check(pDX, IDC_PRECOMPUTE_BBOX, m_PrecomputeBBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CParticleSystemEdit, CDialog)
	//{{AFX_MSG_MAP(CParticleSystemEdit)
	ON_BN_CLICKED(IDC_ACCURATE_INTEGRATION, OnAccurateIntegration)
	ON_BN_CLICKED(IDC_ENABLE_SLOW_DOWN, OnEnableSlowDown)
	ON_BN_CLICKED(IDC_PRECOMPUTE_BBOX, OnPrecomputeBbox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParticleSystemEdit message handlers

void CParticleSystemEdit::OnAccurateIntegration() 
{
	UpdateData() ;
	_PS->enableAccurateIntegration(m_AccurateIntegration ? true : false) ;
	updateIntegrationParams() ;
}

void CParticleSystemEdit::OnEnableSlowDown() 
{
	UpdateData() ;
	NL3D::CAnimationTime t ;
	uint32 max ;
	bool csd ;
	_PS->getAccurateIntegrationParams(t, max, csd) ;
	_PS->setAccurateIntegrationParams(t, max, m_EnableSlowDown ? true : false) ;
}

/////////////////////////////
// WRAPPERS IMPLEMENTATION //
/////////////////////////////

float CParticleSystemEdit::CTimeThresholdWrapper::get(void) const
{
	NL3D::CAnimationTime t ;
	uint32 max ;
	bool csd ;
	PS->getAccurateIntegrationParams(t, max, csd) ;
	return t ;
}
void CParticleSystemEdit::CTimeThresholdWrapper::set(const float &tt)
{
	NL3D::CAnimationTime t ;
	uint32 max ;
	bool csd ;
	PS->getAccurateIntegrationParams(t, max, csd) ;
	PS->setAccurateIntegrationParams(tt, max, csd) ;	
}

uint32 CParticleSystemEdit::CMaxNbIntegrationWrapper::get(void) const
{
	NL3D::CAnimationTime t ;
	uint32 max ;
	bool csd ;
	PS->getAccurateIntegrationParams(t, max, csd) ;
	return max ;
}
void CParticleSystemEdit::CMaxNbIntegrationWrapper::set(const uint32 &nmax)
{
	NL3D::CAnimationTime t ;
	uint32 max ;
	bool csd ;
	PS->getAccurateIntegrationParams(t, max, csd) ;
	PS->setAccurateIntegrationParams(t, nmax, csd) ;	
}
	
float CParticleSystemEdit::CUserParamWrapper::get(void) const 
{ 
	return PS->getUserParam(Index) ; 
}
void CParticleSystemEdit::CUserParamWrapper::set(const float &v)
{
	PS->setUserParam(Index, v) ; 
}
	


float CParticleSystemEdit::CMaxViewDistWrapper::get(void) const
{
	return PS->getMaxViewDist() ;
}

void CParticleSystemEdit::CMaxViewDistWrapper::set(const float &d)
{
	PS->setMaxViewDist(d) ;
}

float CParticleSystemEdit::CLODRatioWrapper::get(void) const
{
	return PS->getLODRatio() ;
}
void CParticleSystemEdit::CLODRatioWrapper::set(const float &v)
{
	PS->setLODRatio(v) ;
}







void CParticleSystemEdit::OnPrecomputeBbox() 
{
	// TODO: Add your control notification handler code here
	
}
