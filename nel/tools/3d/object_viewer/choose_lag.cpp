// choose_lag.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_lag.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseLag dialog


CChooseLag::CChooseLag(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseLag::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseLag)
	m_LagTime = 0;
	//}}AFX_DATA_INIT
}


void CChooseLag::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseLag)
	DDX_Text(pDX, IDC_LAG, m_LagTime);
	DDV_MinMaxUInt(pDX, m_LagTime, 0, 10000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseLag, CDialog)
	//{{AFX_MSG_MAP(CChooseLag)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseLag message handlers
