// bin_op_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "bin_op_dlg.h"
#include "popup_notify.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBinOpDlg dialog


CBinOpDlg::CBinOpDlg(IPopupNotify *pn, CWnd* pParent /*=NULL*/)
	: _PN(pn), CDialog(CBinOpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBinOpDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CBinOpDlg::create(CWnd *pParent)
{
	CDialog::Create(IDD_BIN_OP, pParent);	
	ShowWindow(SW_SHOW);
}


void CBinOpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBinOpDlg)
	DDX_Control(pDX, IDC_BIN_OP, m_BinOp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBinOpDlg, CDialog)
	//{{AFX_MSG_MAP(CBinOpDlg)
	ON_CBN_SELCHANGE(IDC_BIN_OP, OnSelchangeBinOp)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBinOpDlg message handlers

BOOL CBinOpDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();					
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBinOpDlg::OnSelchangeBinOp() 
{
	UpdateData() ;
	newOp(m_BinOp.GetItemData(m_BinOp.GetCurSel())) ;	
}

void CBinOpDlg::OnClose() 
{
	CDialog::OnClose();
	if (_PN) _PN->childPopupDestroyed(this);	
}
