// value_from_emitter_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "value_from_emitter_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CValueFromEmitterDlg dialog


CValueFromEmitterDlg::CValueFromEmitterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CValueFromEmitterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CValueFromEmitterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CValueFromEmitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CValueFromEmitterDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CValueFromEmitterDlg, CDialog)
	//{{AFX_MSG_MAP(CValueFromEmitterDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CValueFromEmitterDlg message handlers

BOOL CValueFromEmitterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	init() ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
