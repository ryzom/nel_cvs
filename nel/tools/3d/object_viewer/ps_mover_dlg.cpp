// ps_mover_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "ps_mover_dlg.h"
#include "particle_tree_ctrl.h"
#include "3d/ps_located.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPSMoverDlg dialog


CPSMoverDlg::CPSMoverDlg(CParticleTreeCtrl *parent, HTREEITEM editedItem)   // standard constructor
	: CDialog(CPSMoverDlg::IDD, parent)
{

	_TreeCtrl = parent ;
	_EditedNode = (CParticleTreeCtrl::CNodeType *) parent->GetItemData(editedItem) ;

	nlassert(_EditedNode->Type == CParticleTreeCtrl::CNodeType::locatedInstance) ;

	const NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;	
	
	m_X.Format("%.3g", pos.x) ;
	m_Y.Format("%.3g", pos.y) ;
	m_Z.Format("%.3g", pos.z) ;

	//{{AFX_DATA_INIT(CPSMoverDlg)
	//}}AFX_DATA_INIT


}

void CPSMoverDlg::updatePosition(void)
{
	UpdateData() ;
	const NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;	
	
	m_X.Format("%.3g", pos.x) ;
	m_Y.Format("%.3g", pos.y) ;
	m_Z.Format("%.3g", pos.z) ;

	UpdateData(FALSE) ;
}

void CPSMoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPSMoverDlg)
	DDX_Control(pDX, IDC_SUB_COMPONENT, m_SubComponentCtrl);
	DDX_Text(pDX, IDC_XPOS, m_X);
	DDX_Text(pDX, IDC_YPOS, m_Y);
	DDX_Text(pDX, IDC_ZPOS, m_Z);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPSMoverDlg, CDialog)
	//{{AFX_MSG_MAP(CPSMoverDlg)
	ON_BN_CLICKED(IDC_UPDATE_XPOS, OnUpdateXpos)
	ON_BN_CLICKED(IDC_UPDATE_YPOS, OnUpdateYpos)
	ON_BN_CLICKED(IDC_UPDATE_ZPOS, OnUpdateZpos)
	ON_LBN_SELCHANGE(IDC_SUB_COMPONENT, OnSelchangeSubComponent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPSMoverDlg message handlers

void CPSMoverDlg::OnUpdateXpos() 
{
	UpdateData() ;
	NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;
	float x ;
	if (::sscanf(m_X, "%f", &x) == 1)
	{
		pos.x = x ;
	}
	else
	{
		MessageBox("invalid entry", "error") ;
	}
	UpdateData(FALSE) ;
}

void CPSMoverDlg::OnUpdateYpos() 
{
	UpdateData() ;
	NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;
	float y ;
	if (::sscanf(m_Y, "%f", &y) == 1)
	{
		pos.y = y ;
	}
	else
	{
		MessageBox("invalid entry", "error") ;
	}
	UpdateData(FALSE) ;	
}

void CPSMoverDlg::OnUpdateZpos() 
{
	UpdateData() ;
	NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;
	float z ;
	if (::sscanf(m_Z, "%f", &z) == 1)
	{
		pos.z = z ;
	}
	else
	{
		MessageBox("invalid entry", "error") ;
	}
	UpdateData(FALSE) ;	
}

BOOL CPSMoverDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	nlassert(_EditedNode) ;
	uint numBound = _EditedNode->Loc->getNbBoundObjects() ;
	
	uint nbCandidates = 0 ;

	for (uint k = 0 ; k < numBound ; ++k)
	{
		if (dynamic_cast<NL3D::IPSMover *>(_EditedNode->Loc->getBoundObject(k)))
		{
			m_SubComponentCtrl.AddString(_EditedNode->Loc->getBoundObject(k)->getName().c_str()) ;
			m_SubComponentCtrl.SetItemData(nbCandidates, (DWORD) _EditedNode->Loc->getBoundObject(k)) ;
			++nbCandidates ;			
		}
	}
	m_SubComponentCtrl.SetSel(0) ;	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPSMoverDlg::OnSelchangeSubComponent() 
{
	UpdateData() ;
	nlassert(_EditedNode) ;

	NL3D::CPSLocatedBindable *lb = (NL3D::CPSLocatedBindable *) m_SubComponentCtrl.GetItemData(m_SubComponentCtrl.GetCurSel()) ;
	_EditedNode->LocMover = dynamic_cast<NL3D::IPSMover *>(lb) ;
	_EditedNode->Loc->getOwner()->setCurrentEditedElement(_EditedNode->Loc, _EditedNode->LocatedInstanceIndex, lb) ;
	
	UpdateData(FALSE) ;
}

void CPSMoverDlg::init(CWnd *parent)
{

	Create(CPSMoverDlg::IDD, parent) ;
	ShowWindow(SW_SHOW) ;
}
