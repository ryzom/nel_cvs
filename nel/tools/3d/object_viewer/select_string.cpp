// select_string.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "select_string.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectString dialog


CSelectString::CSelectString(const std::vector<std::string>& vectString, const char* title, CWnd* pParent /*=NULL*/)
	: CDialog(CSelectString::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectString)
	//}}AFX_DATA_INIT
	Title=title;
	Strings=vectString;
	Selection=-1;
}


void CSelectString::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectString)
	DDX_Control(pDX, IDC_LIST, ListCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectString, CDialog)
	//{{AFX_MSG_MAP(CSelectString)
	ON_LBN_DBLCLK(IDC_LIST, OnDblclkList)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_BN_CLICKED(IDEMPTY, OnEmpty)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectString message handlers

void CSelectString::OnOK() 
{
	// TODO: Add extra validation here
	if (Selection!=-1)
		CDialog::OnOK();
	else
		CDialog::OnCancel();
}

void CSelectString::OnDblclkList() 
{
	// TODO: Add your control notification handler code here
	OnSelchangeList();
	OnOK();
}

BOOL CSelectString::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Change title
	SetWindowText (Title.c_str());

	// Add string
	for (uint s=0; s<Strings.size(); s++)
		ListCtrl.InsertString (-1, Strings[s].c_str());
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectString::OnSelchangeList() 
{
	// Get selection
	UpdateData ();
	Selection=ListCtrl.GetCurSel ();
	if (Selection==LB_ERR)
		Selection=-1;
}

void CSelectString::OnEmpty() 
{
	// TODO: Add your control notification handler code here
	Selection=-1;
	CDialog::OnOK();
}
