// edit_user_param.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "edit_user_param.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditUserParam dialog


CEditUserParam::CEditUserParam(uint32 userParamIndex, CWnd* pParent /*=NULL*/)
	: _UserParamIndex(userParamIndex), CDialog(CEditUserParam::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditUserParam)
	//}}AFX_DATA_INIT
}


void CEditUserParam::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditUserParam)
	DDX_Control(pDX, IDC_USER_PARAM_INDEX, m_UserParamIndex);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditUserParam, CDialog)
	//{{AFX_MSG_MAP(CEditUserParam)
	ON_CBN_SELCHANGE(IDC_USER_PARAM_INDEX, OnSelchangeUserParamIndex)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditUserParam message handlers

BOOL CEditUserParam::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_UserParamIndex.SetCurSel(_UserParamIndex) ;	
	UpdateData(FALSE) ;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditUserParam::OnSelchangeUserParamIndex() 
{
	UpdateData() ;
	_UserParamIndex = m_UserParamIndex.GetCurSel() ;
}
