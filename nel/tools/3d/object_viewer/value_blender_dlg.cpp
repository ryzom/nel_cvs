// ValueBlenderDlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "value_blender_dlg.h"
#include "edit_attrib_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CValueBlenderDlg dialog


CValueBlenderDlg::CValueBlenderDlg(TCreateDialog createFunc, void *lParam, CWnd* pParent /*=NULL*/)
	: _CreateFunc(createFunc), _CreateFuncParam(lParam), CDialog(CValueBlenderDlg::IDD, pParent)
	  , _InfoToDelete1(NULL), _InfoToDelete2(NULL)
{
	nlassert(createFunc) ;
	//{{AFX_DATA_INIT(CValueBlenderDlg)
	//}}AFX_DATA_INIT
}

CValueBlenderDlg::~CValueBlenderDlg()
{
	delete _Dlg1 ;
	delete _Dlg2 ;
	delete _InfoToDelete1 ;
	delete _InfoToDelete2 ;
}


void CValueBlenderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CValueBlenderDlg)
	DDX_Control(pDX, IDC_VALUE2, m_Value2);
	DDX_Control(pDX, IDC_VALUE1, m_Value1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CValueBlenderDlg, CDialog)
	//{{AFX_MSG_MAP(CValueBlenderDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CValueBlenderDlg message handlers

BOOL CValueBlenderDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	UpdateData() ;
	nlassert(_CreateFunc) ;
	
	_Dlg1 = _CreateFunc(0, _CreateFuncParam, &_InfoToDelete1) ;
	_Dlg2 = _CreateFunc(1, _CreateFuncParam, &_InfoToDelete2) ;



	RECT r, or ;
	
	GetWindowRect(&or) ;

	m_Value1.GetWindowRect(&r) ;
	_Dlg1->init(r.left - or.left, r.top - or.top, this) ;
	m_Value2.GetWindowRect(&r) ;
	_Dlg2->init(r.left - or.left, r.top - or.top, this) ;	
	
	UpdateData(FALSE) ;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
