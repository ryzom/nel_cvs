// NewRegion.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "NewRegion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NewRegion dialog


NewRegion::NewRegion(CWnd* pParent /*=NULL*/)
	: CDialog(NewRegion::IDD, pParent)
{
	//{{AFX_DATA_INIT(NewRegion)
	str = _T("");
	//}}AFX_DATA_INIT
}


void NewRegion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NewRegion)
	DDX_Text(pDX, IDC_EDIT1, str);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NewRegion, CDialog)
	//{{AFX_MSG_MAP(NewRegion)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NewRegion message handlers
