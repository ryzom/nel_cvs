// TypeManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEditor.h"
#include "TypeManagerDlg.h"

#include "TypeDlg.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTypeManagerDlg dialog

// ---------------------------------------------------------------------------
CTypeManagerDlg::CTypeManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTypeManagerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTypeManagerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ---------------------------------------------------------------------------
void CTypeManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTypeManagerDlg)
	DDX_Control(pDX, IDC_LISTTYPE, ListType);
	//}}AFX_DATA_MAP
}

// ---------------------------------------------------------------------------
void CTypeManagerDlg::set (const vector<SType> &types)
{
	LocalTypes = types;
}

// ---------------------------------------------------------------------------
const vector<SType> CTypeManagerDlg::get ()
{
	return LocalTypes;
}

// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CTypeManagerDlg, CDialog)
	//{{AFX_MSG_MAP(CTypeManagerDlg)
	ON_BN_CLICKED(IDC_ADDTYPE, OnAddtype)
	ON_BN_CLICKED(IDC_EDITTYPE, OnEdittype)
	ON_BN_CLICKED(IDC_REMOVETYPE, OnRemovetype)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTypeManagerDlg message handlers

void CTypeManagerDlg::OnAddtype() 
{
	// TODO: Add your control notification handler code here
	CTypeDlg typeDlg(this);
	if (typeDlg.DoModal() == IDOK)
	{
		SType typeTmp;
		typeTmp.Name = (LPCSTR)typeDlg.EditName;
		typeTmp.Color = typeDlg.ButtonColorValue;
		// Check if the name of the new type is the same as an existing one
		bool bFound = false;
		for (uint32 i = 0; i < LocalTypes.size(); ++i)
		if (LocalTypes[i].Name == typeTmp.Name)
		{
			bFound = true;
			break;
		}
		// If not Add it to the 2 lists (win and internal)
		if (!bFound)
		{
			LocalTypes.push_back (typeTmp);
			ListType.InsertString(-1, typeTmp.Name.c_str());
		}
	}
}

void CTypeManagerDlg::OnEdittype() 
{
	// TODO: Add your control notification handler code here
	CTypeDlg typeDlg(this);

	int cursel = ListType.GetCurSel();
	if (cursel == -1)
		return;

	typeDlg.EditName = LocalTypes[cursel].Name.c_str();
	typeDlg.ButtonColorValue = LocalTypes[cursel].Color;

	if (typeDlg.DoModal() == IDOK)
	{
		SType typeTmp;
		typeTmp.Name = (LPCSTR)typeDlg.EditName;
		typeTmp.Color = typeDlg.ButtonColorValue;
		LocalTypes[cursel] = typeTmp;
		ListType.DeleteString (ListType.GetCurSel());
		ListType.InsertString (cursel, typeTmp.Name.c_str());
	}
}

void CTypeManagerDlg::OnRemovetype() 
{
	// TODO: Add your control notification handler code here
	int cursel = ListType.GetCurSel();
	if (cursel == -1)
		return;
	for (uint32 i = cursel; i < (LocalTypes.size()-1); ++i)
		LocalTypes[i] = LocalTypes[i+1];
	LocalTypes.resize(LocalTypes.size()-1);
	ListType.DeleteString (ListType.GetCurSel());
}

BOOL CTypeManagerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (uint32 i = 0; i < LocalTypes.size(); ++i)
	{
		ListType.InsertString(-1, LocalTypes[i].Name.c_str());
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
