// choose_frame_delay.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_frame_delay.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseFrameDelay dialog


CChooseFrameDelay::CChooseFrameDelay(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseFrameDelay::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseFrameDelay)
	m_FrameDelay = 0;
	//}}AFX_DATA_INIT
}


void CChooseFrameDelay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseFrameDelay)
	DDX_Text(pDX, IDC_FRAME_DELAY, m_FrameDelay);
	DDV_MinMaxUInt(pDX, m_FrameDelay, 0, 10000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseFrameDelay, CDialog)
	//{{AFX_MSG_MAP(CChooseFrameDelay)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseFrameDelay message handlers
