// lb_extern_id_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "lb_extern_id_dlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLBExternIDDlg dialog


CLBExternIDDlg::CLBExternIDDlg(uint32 id, CWnd* pParent /* = NULL*/)
	: CDialog(CLBExternIDDlg::IDD, pParent), _ID(id)
{
	//{{AFX_DATA_INIT(CLBExternIDDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLBExternIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLBExternIDDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLBExternIDDlg, CDialog)
	//{{AFX_MSG_MAP(CLBExternIDDlg)
	ON_BN_CLICKED(IDC_ENABLE_EXTERN_ID, OnEnableExternId)
	ON_EN_CHANGE(IDC_ID_VALUE, OnChangeIdValue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLBExternIDDlg message handlers

BOOL CLBExternIDDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
		
	GetDlgItem(IDC_ID_VALUE)->EnableWindow(_ID != 0);
	((CButton *) GetDlgItem(IDC_ENABLE_EXTERN_ID))->SetCheck(_ID != 0 ? 1 : 0);

	if (_ID)
	{
		char val[5];
		for (uint k = 0; k < 4; ++k)
		{
			#ifdef NL_LITTLE_ENDIAN
				val[k] = (unsigned char) (_ID >> ((3 - k) << 3));
			#else
				val[k] = (unsigned char) (_ID >> (k << 3));				
			#endif
		}
		val[4] = '\0';		
		GetDlgItem(IDC_ID_VALUE)->SetWindowText(val);

	}
	else
	{
		_ID = 0;
	}	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


static uint32 StringToID(const char *buf)
{
	uint32 id;
	#ifdef NL_LITTLE_ENDIAN
		id = ((uint32) buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]);		
	#else
		id = *(uint32 *) buf;		
	#endif
	return id;
}

void CLBExternIDDlg::OnEnableExternId() 
{
	if (_ID == 0)
	{
		GetDlgItem(IDC_ID_VALUE)->EnableWindow(TRUE);
		_ID = StringToID("NONE");		
		GetDlgItem(IDC_ID_VALUE)->SetWindowText("NONE");		
	}
	else
	{
		GetDlgItem(IDC_ID_VALUE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ID_VALUE)->SetWindowText("");		
		_ID = 0;
	}		
}

void CLBExternIDDlg::OnChangeIdValue() 
{	
	if (!((CButton *) GetDlgItem(IDC_ENABLE_EXTERN_ID))->GetCheck()) return;
	char buf[6];
	::memset(buf, 0, 6);
	GetDlgItem(IDC_ID_VALUE)->GetWindowText(buf, 6);
	_ID = StringToID(buf);	
	if (_ID)
	{
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_ENABLE_EXTERN_ID)->EnableWindow(TRUE);		
		if (::strlen(buf) > 4)
		{
			buf[4] = '\0';
			GetDlgItem(IDC_ID_VALUE)->SetWindowText(buf);
		}
	}
	else
	{
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDC_ENABLE_EXTERN_ID)->EnableWindow(FALSE);	
	}

}
