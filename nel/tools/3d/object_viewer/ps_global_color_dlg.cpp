// ps_global_color_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "ps_global_color_dlg.h"
#include "popup_notify.h"
#include "attrib_dlg.h"
#include "3d/particle_system.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPSGlobalColorDlg dialog


CPSGlobalColorDlg::CPSGlobalColorDlg(NL3D::CParticleSystem *ps, IPopupNotify *pn, CWnd* pParent /* = NULL */)
	: _PS(ps),
	  _PN(pn),
	  CDialog(CPSGlobalColorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPSGlobalColorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPSGlobalColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPSGlobalColorDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPSGlobalColorDlg, CDialog)
	//{{AFX_MSG_MAP(CPSGlobalColorDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///===========================================================
void CPSGlobalColorDlg::init(CWnd *pParent)
{
	Create(IDD_GLOBAL_COLOR, pParent);
	ShowWindow(SW_SHOW);
}

///===========================================================
CPSGlobalColorDlg::CGlobalColorWrapper::scheme_type *CPSGlobalColorDlg::CGlobalColorWrapper::getScheme(void) const
{
	nlassert(PS);
	return PS->getColorAttenuationScheme();
}

///===========================================================
void CPSGlobalColorDlg::CGlobalColorWrapper::setScheme(CPSGlobalColorDlg::CGlobalColorWrapper::scheme_type *s)
{
	PS->setColorAttenuationScheme(s);
}


/////////////////////////////////////////////////////////////////////////////
// CPSGlobalColorDlg message handlers

void CPSGlobalColorDlg::OnClose() 
{		
	CDialog::OnClose();
	if (_PN)
		_PN->childPopupClosed(this);
}

///===========================================================
BOOL CPSGlobalColorDlg::OnInitDialog() 
{		
	CDialog::OnInitDialog();
	_GlobalColorWrapper.PS = _PS;
	
	RECT r;	
	CAttribDlgRGBA *adr = new CAttribDlgRGBA("GLOBAL_PS_COLOR");	
	adr->setSchemeWrapper(&_GlobalColorWrapper);
	adr->enableMemoryScheme(false);
	adr->enableNbCycles(false);
	adr->enableSrcInput(false);
	adr->disableConstantValue();
	GetDlgItem(IDC_COLOR_ATTRIB)->GetWindowRect(&r);
	ScreenToClient(&r);
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_COLOR));
	adr->init(bmh, r.left, r.top, this);
	pushWnd(adr);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
