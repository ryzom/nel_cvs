// located_target_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "located_target_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
	Create(IDD_LOCATED_TARGET_DLG, pParent) ;
	ShowWindow(SW_SHOW) ;
}


void CLocatedTargetDlg::OnAddTarget() 
{
	UpdateData() ;
	
	int totalCount = m_AvailableTargets.GetCount() ;
	nlassert(totalCount) ;
	std::vector<int> indexs ;
	indexs.resize(totalCount) ;
	int selCount = m_AvailableTargets.GetSelItems(totalCount, &indexs[0]) ;

	std::sort(indexs.begin(), indexs.begin() + selCount) ; // we never know ...

	for (int k = 0 ; k < selCount ; ++k)
	{
		NL3D::CPSLocated *loc = (NL3D::CPSLocated *) m_AvailableTargets.GetItemData(indexs[k] - k) ;
		nlassert(loc) ;
		_LBTarget->attachTarget(loc) ;
		m_AvailableTargets.DeleteString(indexs[k] - k) ;
		int l = m_Targets.AddString(loc->getName().c_str()) ;
		m_Targets.SetItemData(l, (DWORD) loc) ;
	}

	
	UpdateData(FALSE) ;


}

void CLocatedTargetDlg::OnRemoveTarget() 
{
	UpdateData() ;	


	int totalCount = m_Targets.GetCount() ;
	nlassert(totalCount) ;
	std::vector<int> indexs ;
	indexs.resize(totalCount) ;
	int selCount = m_Targets.GetSelItems(totalCount, &indexs[0]) ;

	std::sort(indexs.begin(), indexs.begin() + selCount) ; // we never know ...

	for (int k = 0 ; k < selCount ; ++k)
	{
		NL3D::CPSLocated *loc = (NL3D::CPSLocated *) m_Targets.GetItemData(indexs[k] - k) ;
		nlassert(loc) ;
		_LBTarget->detachTarget(loc) ;
		m_Targets.DeleteString(indexs[k] - k) ;
		int l = m_AvailableTargets.AddString(loc->getName().c_str()) ;
	
		m_AvailableTargets.SetItemData(l, (DWORD) loc) ;
	}

	UpdateData(FALSE) ;	
}

BOOL CLocatedTargetDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	uint k ;
	uint nbTarg = _LBTarget->getNbTargets() ;

	m_Targets.InitStorage(nbTarg, 128) ;

	std::set<NL3D::CPSLocated *> targetSet ;

	// fill the box thta tells us what the target are
	for(k = 0 ; k < nbTarg ; ++k)
	{
		m_Targets.AddString(_LBTarget->getTarget(k)->getName().c_str() ) ;
		m_Targets.SetItemData(k, (DWORD) _LBTarget->getTarget(k) ) ;
		targetSet.insert(_LBTarget->getTarget(k)) ;
	} ;

	// fill abox with the available targets
	NL3D::CParticleSystem  *ps = _LBTarget->getOwner()->getOwner() ;

	uint nbLocated = ps->getNbProcess() ;


	
	m_AvailableTargets.InitStorage(nbTarg, 128) ;
	for (k = 0 ; k < nbLocated ; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k)) ;
		if (loc)
		{
			if (targetSet.find(loc) == targetSet.end())
			{
				int l = m_AvailableTargets.AddString(loc->getName().c_str() ) ;				
				m_AvailableTargets.SetItemData(l, (DWORD) loc ) ;				
			}
		}
	}

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
