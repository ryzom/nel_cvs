// choose_frame_delay.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_frame_delay.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseFrameDelay dialog


//*****************************************************************************************************
CChooseFrameDelay::CChooseFrameDelay(CObjectViewer *objectViewer, CWnd* pParent)
	: CDialog(CChooseFrameDelay::IDD, pParent)
{
	nlassert(objectViewer);
	_CFDWrapper.OV = objectViewer;
	//{{AFX_DATA_INIT(CChooseFrameDelay)	
	//}}AFX_DATA_INIT
	_ER = new CEditableRangeUInt("CHOOSE_FRAME_DELAY", 0, 500);
	_ER->enableLowerBound(0, false);
	_ER->enableUpperBound(10000, false);	
	_ER->setWrapper(&_CFDWrapper);
}

CChooseFrameDelay::~CChooseFrameDelay()
{
	_ER->DestroyWindow();
	delete _ER;	
}

//*****************************************************************************************************
void CChooseFrameDelay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseFrameDelay)	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseFrameDelay, CDialog)
	//{{AFX_MSG_MAP(CChooseFrameDelay)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseFrameDelay message handlers


BOOL CChooseFrameDelay::OnInitDialog() 
{
	CDialog::OnInitDialog();
	RECT r;
	GetDlgItem(IDC_FRAME_DELAY)->GetWindowRect(&r);
	ScreenToClient(&r);
	_ER->init(r.left, r.top, this);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

uint32 CChooseFrameDelay::CCFDWrapper::get(void) const { return OV->getFrameDelay(); }
void   CChooseFrameDelay::CCFDWrapper::set(const uint32 &value) { OV->setFrameDelay(value); }

void CChooseFrameDelay::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_CHOOSE_FRAME_DELAY_DLG);
	CDialog::OnDestroy();		
}
