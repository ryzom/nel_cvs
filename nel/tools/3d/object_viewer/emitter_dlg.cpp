// emitter_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "emitter_dlg.h"

#include "nel/3d/particle_system.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg dialog


CEmitterDlg::CEmitterDlg(NL3D::CPSEmitter *emitter)
	: _Emitter(emitter), _PeriodDlg(NULL), _GenNbDlg(NULL)
	  , _StrenghtModulateDlg(NULL)
	  , _SpeedInheritanceFactorDlg(NULL)
{
	nlassert(_Emitter) ;
	//{{AFX_DATA_INIT(CEmitterDlg)
	m_UseSpeedBasis = FALSE;
	//}}AFX_DATA_INIT
}

CEmitterDlg::~CEmitterDlg()
{
	delete _PeriodDlg ;
	delete _GenNbDlg ;
	delete _StrenghtModulateDlg ;
	delete _SpeedInheritanceFactorDlg ;
}


void CEmitterDlg::init(CWnd* pParent)
{
	Create(IDD_EMITTER_DIALOG, pParent) ;
	;

	// fill the emitted type combo box with all the types of located

	NL3D::CParticleSystem *ps = _Emitter->getOwner()->getOwner() ;
	
	uint nbLocated = ps->getNbProcess() ; 

	m_EmittedTypeCtrl.InitStorage(nbLocated, 16) ;
	

	for (uint k = 0 ; k < nbLocated ; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k)) ;
		if (loc) // is this a located
		{
			m_EmittedTypeCtrl.AddString(loc->getName().c_str()) ;
			_LocatedList.push_back(loc) ;			
			if (loc == _Emitter->getEmittedType())
			{
				m_EmittedTypeCtrl.SetCurSel(k) ;
			}
		}
	}

	m_EmissionTypeCtrl.SetCurSel((int) _Emitter->getEmissionType() ) ;






	ShowWindow(SW_SHOW) ; 
	UpdateData(FALSE) ;
}

void CEmitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmitterDlg)
	DDX_Control(pDX, IDC_TYPE_OF_EMISSION, m_EmissionTypeCtrl);
	DDX_Control(pDX, IDC_EMITTED_TYPE, m_EmittedTypeCtrl);
	DDX_Check(pDX, IDC_USE_SPEED_BASIS, m_UseSpeedBasis);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmitterDlg, CDialog)
	//{{AFX_MSG_MAP(CEmitterDlg)
	ON_CBN_SELCHANGE(IDC_EMITTED_TYPE, OnSelchangeEmittedType)
	ON_CBN_SELCHANGE(IDC_TYPE_OF_EMISSION, OnSelchangeTypeOfEmission)
	ON_BN_CLICKED(IDC_USE_SPEED_BASIS, OnUseSpeedBasis)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg message handlers

void CEmitterDlg::OnSelchangeEmittedType() 
{
	UpdateData() ;
	uint k = m_EmittedTypeCtrl.GetCurSel() ;
	_Emitter->setEmittedType(_LocatedList[k]) ;
	
}

void CEmitterDlg::OnSelchangeTypeOfEmission() 
{
	UpdateData() ;
	_Emitter->setEmissionType((NL3D::CPSEmitter::TEmissionType) m_EmissionTypeCtrl.GetCurSel()) ;	

	updatePeriodDlg() ;
}


void CEmitterDlg::updatePeriodDlg(void)
{
	if (_Emitter->getEmissionType() != NL3D::CPSEmitter::regular)
	{
		_PeriodDlg->EnableWindow(FALSE) ;
	}
	else
	{
		_PeriodDlg->EnableWindow(TRUE) ;
	}
}

BOOL CEmitterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	
	
	uint posX = 13 ;
	uint posY = 60 ;



	_SpeedInheritanceFactorDlg = new CEditableRangeFloat("SPEED_INHERITANCE_FACTOR", -1.f, 1.f) ;
	_SpeedInheritanceFactorWrapper.E = _Emitter ;
	_SpeedInheritanceFactorDlg->setWrapper(&_SpeedInheritanceFactorWrapper) ;
	_SpeedInheritanceFactorDlg->init(posX + 100, posY, this) ;

	posY += 30 ; 

	// setup the dialog for the period of emission edition

	_PeriodDlg = new CAttribDlgFloat("EMISSION_PERIOD", 0.f, 2.f) ;
	_PeriodWrapper.E = _Emitter ;
	_PeriodDlg->setWrapper(&_PeriodWrapper) ;
	_PeriodDlg->setSchemeWrapper(&_PeriodWrapper) ;
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_EMISSION_PERIOD)) ;
	_PeriodDlg->init(bmh, posX, posY, this) ;
	posY += 80 ;

	// setup the dialog that helps tuning the number of particle being emitted at a time

	_GenNbDlg = new CAttribDlgUInt("EMISSION_GEN_NB",1,11) ;
	_GenNbWrapper.E = _Emitter ;
	_GenNbDlg->setWrapper(&_GenNbWrapper) ;
	_GenNbDlg->setSchemeWrapper(&_GenNbWrapper) ;
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_EMISSION_QUANTITY)) ;
	_GenNbDlg->init(bmh, posX, posY, this) ;
	posY += 80 ;

	if (dynamic_cast<NL3D::CPSModulatedEmitter *>(_Emitter))
	{
		_StrenghtModulateDlg = new CAttribDlgFloat("EMISSION_GEN_NB",1,11) ;
		_ModulatedStrenghtWrapper.E = dynamic_cast<NL3D::CPSModulatedEmitter *>(_Emitter) ;
		_StrenghtModulateDlg->setWrapper(&_ModulatedStrenghtWrapper) ;
		_StrenghtModulateDlg->setSchemeWrapper(&_ModulatedStrenghtWrapper) ;
		bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_MODULATE_STRENGHT)) ;
		_StrenghtModulateDlg->init(bmh, posX, posY, this) ;
		posY += 80 ;
	}

	m_UseSpeedBasis = _Emitter->isSpeedBasisEmissionEnabled() ;

	updatePeriodDlg() ;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmitterDlg::OnUseSpeedBasis() 
{
	UpdateData() ;
	_Emitter->enableSpeedBasisEmission(m_UseSpeedBasis ? true : false) ;
	UpdateData(TRUE) ;
}
