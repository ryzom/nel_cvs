// TypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEditor.h"
#include "TypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTypeDlg dialog


CTypeDlg::CTypeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTypeDlg)
	EditName = _T("");
	ButtonColor.setColor(CRGBA(255,255,255,255));
	//}}AFX_DATA_INIT
}


void CTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTypeDlg)
	DDX_Control(pDX, IDC_BUTTONCOLOR, ButtonColor);
	DDX_Text(pDX, IDC_EDITNAME, EditName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTypeDlg, CDialog)
	//{{AFX_MSG_MAP(CTypeDlg)
	ON_BN_CLICKED(IDC_BUTTONCOLOR, OnButtoncolor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTypeDlg message handlers

void CTypeDlg::OnButtoncolor() 
{
	// TODO: Add your control notification handler code here
	CColorDialog coldlg;
	if (coldlg.DoModal() == IDOK)
	{
		int r = GetRValue(coldlg.GetColor());
		int g = GetGValue(coldlg.GetColor());
		int b = GetBValue(coldlg.GetColor());
		ButtonColor.setColor(CRGBA(r,g,b,255));
		Invalidate();
	}
}

BOOL CTypeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	ButtonColor.setColor (ButtonColorValue);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTypeDlg::OnOK() 
{
	// TODO: Add extra validation here
	ButtonColorValue = ButtonColor.getColor();
	CDialog::OnOK();
}
