// ChooseDir.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "ChooseDir.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseDir dialog


CChooseDir::CChooseDir(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseDir::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseDir)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// ---------------------------------------------------------------------------
void CChooseDir::setPath (const string &path)
{
	_Path = path;
	_Sel = 0;
}

// ---------------------------------------------------------------------------
BOOL CChooseDir::OnInitDialog ()
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	CListBox *pLB = (CListBox*)GetDlgItem(IDC_LIST);

	SetCurrentDirectory (_Path.c_str());
	hFind = FindFirstFile ("*.*", &findData);
	
	while (hFind != INVALID_HANDLE_VALUE)
	{

		if ((stricmp (findData.cFileName, ".") != 0) && (stricmp (findData.cFileName, "..") != 0))
		{
			if (findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				pLB->InsertString (-1, findData.cFileName);
				string tmp = findData.cFileName;
				_Names.push_back (tmp);
			}
		}
		if (FindNextFile (hFind, &findData) == 0)
			break;
	}
	FindClose (hFind);

	return true;	
}

// ---------------------------------------------------------------------------
const char *CChooseDir::getSelected ()
{
	if (_Names.size() == 0)
		return NULL;

	if (_Sel == LB_ERR) 
		return _Names[0].c_str();
	else
		return _Names[_Sel].c_str();
}

// ---------------------------------------------------------------------------
void CChooseDir::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseDir)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseDir, CDialog)
	//{{AFX_MSG_MAP(CChooseDir)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseDir message handlers

void CChooseDir::OnSelChangeList() 
{
	// TODO: Add your control notification handler code here
	CListBox *pLB = (CListBox*)GetDlgItem(IDC_LIST);
	_Sel = pLB->GetCurSel();
}
