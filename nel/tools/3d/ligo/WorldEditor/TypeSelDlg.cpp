// TypeSelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEditor.h"
#include "TypeSelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTypeSelDlg dialog


CTypeSelDlg::CTypeSelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTypeSelDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTypeSelDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTypeSelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTypeSelDlg)
	DDX_Control(pDX, IDC_LIST, TypeList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTypeSelDlg, CDialog)
	//{{AFX_MSG_MAP(CTypeSelDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTypeSelDlg message handlers

void CTypeSelDlg::OnOK() 
{
	// TODO: Add extra validation here

	if (TypeList.GetCurSel() == -1)
		return;

	CString sTmp;
	TypeList.GetText(TypeList.GetCurSel(), sTmp);
	_TypeSelected = (LPCSTR)sTmp;

	CDialog::OnOK();
}

BOOL CTypeSelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (uint32 i = 0; i < _TypesInit->size(); ++i)
	{
		TypeList.InsertString(-1, _TypesInit->operator[](i).Name.c_str());
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
