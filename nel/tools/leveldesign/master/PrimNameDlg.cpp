// PrimNameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "PrimNameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrimNameDlg dialog


CPrimNameDlg::CPrimNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrimNameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrimNameDlg)
	str = _T("");
	//}}AFX_DATA_INIT
}


void CPrimNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrimNameDlg)
	DDX_Text(pDX, IDC_EDIT1, str);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrimNameDlg, CDialog)
	//{{AFX_MSG_MAP(CPrimNameDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrimNameDlg message handlers

BOOL CPrimNameDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	GetDlgItem(IDC_EDIT1)->SetFocus();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
