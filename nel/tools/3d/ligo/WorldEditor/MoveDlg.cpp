// MoveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEditor.h"
#include "resource.h"
#include "MoveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMoveDlg dialog


CMoveDlg::CMoveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMoveDlg)
	XOffset = 0;
	YOffset = 0;
	//}}AFX_DATA_INIT
}


void CMoveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMoveDlg)
	DDX_Text(pDX, IDC_EDITXOFFSET, XOffset);
	DDX_Text(pDX, IDC_EDITYOFFSET, YOffset);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMoveDlg, CDialog)
	//{{AFX_MSG_MAP(CMoveDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMoveDlg message handlers
