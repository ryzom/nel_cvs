// NewGeorgesFormDlg.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "NewGeorgesFormDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewGeorgesFormDlg dialog


CNewGeorgesFormDlg::CNewGeorgesFormDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewGeorgesFormDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewGeorgesFormDlg)
	str = _T("");
	//}}AFX_DATA_INIT
}


void CNewGeorgesFormDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewGeorgesFormDlg)
	DDX_Text(pDX, IDC_EDIT1, str);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewGeorgesFormDlg, CDialog)
	//{{AFX_MSG_MAP(CNewGeorgesFormDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewGeorgesFormDlg message handlers
