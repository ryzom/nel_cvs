// ExportCBDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEditor.h"
#include "ExportCBDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportCB

/////////////////////////////////////////////////////////////////////////////
CExportCB::CExportCB()
{
	_Canceled = false;
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::setExportCBDlg (CExportCBDlg *dlg)
{
	_Dialog = dlg;
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::cancel ()
{
	_Canceled = true;
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::pump()
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CExportCB::isCanceled ()
{
	pump();
	return _Canceled;
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispPass (const std::string &Text)
{
	_Dialog->PassText = Text.c_str();
	_Dialog->ProgressBar.SetPos (0);
	_Dialog->UpdateData (FALSE); // Upload
	_Dialog->Invalidate();
	pump();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispPassProgress (float percentage)
{
	_Dialog->ProgressBar.SetPos ((sint32)(100*percentage));
	_Dialog->UpdateData (FALSE); // Upload
	pump();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispInfo (const std::string &Text)
{
	_Dialog->InfoText = CString(Text.c_str()) + "\r\n" + _Dialog->InfoText;
	_Dialog->UpdateData (FALSE); // Upload
	pump();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispWarning (const std::string &Text)
{
	_Dialog->InfoText = CString("WARNING : ") + CString(Text.c_str()) + "\r\n" + _Dialog->InfoText;
	_Dialog->UpdateData (FALSE); // Upload
	pump();
}

/////////////////////////////////////////////////////////////////////////////
void CExportCB::dispError (const std::string &Text)
{
	_Dialog->InfoText = CString("ERROR : ") + CString(Text.c_str()) + "\r\n" + _Dialog->InfoText;
	_Dialog->UpdateData (FALSE); // Upload
	pump();
}

/////////////////////////////////////////////////////////////////////////////
// CExportCBDlg dialog


/////////////////////////////////////////////////////////////////////////////
CExportCBDlg::CExportCBDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportCBDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportCBDlg)
	InfoText = _T("");
	PassText = _T("");
	_Finished = false;
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
void CExportCBDlg::setFinishedButton ()
{
	CButton *but = (CButton*)GetDlgItem (IDCANCEL);
	but->SetWindowText ("FINISHED");
	Invalidate ();
	pump ();
}


/////////////////////////////////////////////////////////////////////////////
void CExportCBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportCBDlg)
	DDX_Control(pDX, IDC_PROGRESS, ProgressBar);
	DDX_Text(pDX, IDC_EDIT_INFO, InfoText);
	DDX_Text(pDX, IDC_PASSTEXT, PassText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportCBDlg, CDialog)
	//{{AFX_MSG_MAP(CExportCBDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportCBDlg message handlers

/////////////////////////////////////////////////////////////////////////////
void CExportCBDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	CButton *but = (CButton*)GetDlgItem (IDCANCEL);
	CString zeText;
	but->GetWindowText (zeText);
	if (zeText == "Cancel")
		_ExportCB.cancel ();
	else
		_Finished = true;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CExportCBDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	_ExportCB.setExportCBDlg (this);
	ProgressBar.SetRange (0, 100);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
