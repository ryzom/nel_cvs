// value_gradient_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "value_gradient_dlg.h"

#include "edit_attrib_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CValueGradientDlg dialog


CValueGradientDlg::CValueGradientDlg(TCreateDialog createDialogFunc, TModifyGradient modifyGradientFunc, TDisplayValue displayValueFunc
					, void *createLParam, void *modifyLParam, void *displayLParam				
					, uint initialSize
					, CWnd* pParent)
	: CDialog(CValueGradientDlg::IDD, pParent)
	 , _CreateDialogFunc(createDialogFunc),  _ModifyGradientFunc(modifyGradientFunc),  _DisplayValueFunc(displayValueFunc)
	 , _CreateLParam(createLParam), _ModifyLParam(modifyLParam), _DisplayLParam(displayLParam)	 
	 , _Size(initialSize)
	 , _EditValueDlg(NULL)
	 , _InfoToDelete(NULL)
{
	//{{AFX_DATA_INIT(CValueGradientDlg)
	//}}AFX_DATA_INIT
}


CValueGradientDlg::~CValueGradientDlg()
{
	delete _InfoToDelete ;
	delete _EditValueDlg ;
}


void CValueGradientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CValueGradientDlg)
	DDX_Control(pDX, IDC_GRADIENT_LIST, m_GradientList);
	DDX_Control(pDX, IDC_EDITED_VALUE, m_Value);
	DDX_Control(pDX, IDC_REMOVE_VALUE, m_RemoveCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CValueGradientDlg, CDialog)
	//{{AFX_MSG_MAP(CValueGradientDlg)
	ON_BN_CLICKED(IDC_ADD_VALUE, OnAddValue)
	ON_BN_CLICKED(IDC_INSERT_VALUE, OnInsertValue)
	ON_BN_CLICKED(IDC_REMOVE_VALUE, OnRemoveValue)
	ON_BN_CLICKED(IDC_VALUE_DOWN, OnValueDown)
	ON_BN_CLICKED(IDC_VALUE_UP, OnValueUp)
	ON_LBN_SELCHANGE(IDC_GRADIENT_LIST, OnSelchangeGradientList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CValueGradientDlg message handlers

void CValueGradientDlg::OnAddValue() 
{
	nlassert(_ModifyGradientFunc) ;
	UpdateData() ;
		++_Size ;
		_ModifyGradientFunc(Add, 0, _ModifyLParam) ;
		m_GradientList.AddString("value") ;		
		m_RemoveCtrl.EnableWindow(TRUE) ;

		m_GradientList.SetCurSel(_Size - 1) ;
		m_GradientList.Invalidate() ;
		OnSelchangeGradientList() ;
	UpdateData(FALSE) ;
}

void CValueGradientDlg::OnInsertValue() 
{
	nlassert(_ModifyGradientFunc) ;
	UpdateData() ;
		uint oldIndex = m_GradientList.GetCurSel() ;
		++_Size ;
		_ModifyGradientFunc(Insert, m_GradientList.GetCurSel(), _ModifyLParam) ;
		m_GradientList.InsertString(m_GradientList.GetCurSel(), "value") ;		
		m_GradientList.Invalidate() ;
		m_GradientList.SetCurSel(oldIndex) ;
		OnSelchangeGradientList() ;
	UpdateData(FALSE) ;	
}

void CValueGradientDlg::OnRemoveValue() 
{
	nlassert(_ModifyGradientFunc) ;
	UpdateData() ;
		uint oldIndex = m_GradientList.GetCurSel() ;
		--_Size ;
		_ModifyGradientFunc(Delete, m_GradientList.GetCurSel(), _ModifyLParam) ;
		m_GradientList.DeleteString(m_GradientList.GetCurSel()) ;				

		if (_Size <= 2)
		{
			m_RemoveCtrl.EnableWindow(FALSE) ;
		}
		if (oldIndex < _Size)
		{
			m_GradientList.SetCurSel(oldIndex) ;
		}
		else
		{
			m_GradientList.SetCurSel(oldIndex - 1) ;
		}
		m_GradientList.Invalidate() ;
	UpdateData(FALSE) ;		
}

void CValueGradientDlg::OnValueDown() 
{
	// TODO: Add your control notification handler code here
	
}

void CValueGradientDlg::OnValueUp() 
{
	// TODO: Add your control notification handler code here
	
}

BOOL CValueGradientDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateData() ;

	m_GradientList.InitStorage(_Size, 16) ;

	for (uint k = 0 ; k < _Size ; ++k)
	{
		m_GradientList.AddString("value") ;
	}
	


	m_RemoveCtrl.EnableWindow(_Size > 2 ? TRUE : FALSE) ;


	m_GradientList.SetCurSel(0) ;

	m_GradientList.setCtrlID(IDC_GRADIENT_LIST) ;
	m_GradientList.setDrawer(_DisplayValueFunc, _DisplayLParam) ;

	UpdateData(FALSE) ;		

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CValueGradientDlg::OnSelchangeGradientList() 
{
	UpdateData(TRUE) ;
	delete _EditValueDlg ;	
	delete _InfoToDelete ;
	_EditValueDlg =	_CreateDialogFunc(m_GradientList.GetCurSel(), _CreateLParam, &_InfoToDelete, this) ;

	RECT r, or ;
	GetWindowRect(&or) ;
	m_Value.GetWindowRect(&r) ;
	_EditValueDlg->init(r.left - or.left, r.top - or.top, this) ;
	UpdateData(FALSE) ;
	
}


void CValueGradientDlg::invalidateGrad(void)
{
	UpdateData(TRUE) ;
		m_GradientList.Invalidate() ;
	UpdateData(FALSE) ;
}
