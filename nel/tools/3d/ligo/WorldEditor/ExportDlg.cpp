// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEditor.h"
#include "ExportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog


// ---------------------------------------------------------------------------
CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDlg)
	OutZoneDir = _T("");
	RefZoneDir = _T("");
	TileBankFile = _T("");
	HeightMapFile = _T("");
	ZFactor = _T("");
	Lighting = 0;
	//}}AFX_DATA_INIT
}


// ---------------------------------------------------------------------------
void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
	DDX_Text(pDX, IDC_EDIT_OUTZONEDIR, OutZoneDir);
	DDX_Text(pDX, IDC_EDIT_REFZONEDIR, RefZoneDir);
	DDX_Text(pDX, IDC_EDIT_TILEBANKFILE, TileBankFile);
	DDX_Text(pDX, IDC_EDIT_HEIGHTMAPFILE, HeightMapFile);
	DDX_Text(pDX, IDC_EDIT_ZFACTOR, ZFactor);

	if (pDX->m_bSaveAndValidate)
	{
		// Download
		if (GetCheckedRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO) == IDC_RADIOLIGHTNOISE)
			Lighting = 2;
		else
		if (GetCheckedRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO) == IDC_RADIOLIGHTPATCH)
			Lighting = 1;
		else
		if (GetCheckedRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO) == IDC_RADIOLIGHTNO)
			Lighting = 0;
	}
	else
	{
		// Upload
		if (Lighting == 0)
			CheckRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO, IDC_RADIOLIGHTNO);
		else
		if (Lighting == 1)
			CheckRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO, IDC_RADIOLIGHTPATCH);
		else
		if (Lighting == 2)
			CheckRadioButton (IDC_RADIOLIGHTNOISE, IDC_RADIOLIGHTNO, IDC_RADIOLIGHTNOISE);
	}
	//}}AFX_DATA_MAP
}


// ---------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	//{{AFX_MSG_MAP(CExportDlg)
	ON_BN_CLICKED(IDC_BUTTON_REFZONEDIR, OnButtonRefzonedir)
	ON_BN_CLICKED(IDC_BUTTON_OUTZONEDIR, OnButtonOutzonedir)
	ON_BN_CLICKED(IDC_BUTTON_TILEBANKFILE, OnButtonTilebankfile)
	ON_BN_CLICKED(IDC_BUTTON_HEIGHTMAPFILE, OnButtonHeightmapfile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

void CExportDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

// ---------------------------------------------------------------------------
// This function serve to initiate the browsing dialog box to the good position in the tree
int CALLBACK expBrowseCallbackProc (HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	switch(uMsg) 
	{
		case BFFM_INITIALIZED: 
			SendMessage (hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
		default:
		break;
	}
	return 0;
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonRefzonedir() 
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path";
	bi.ulFlags = 0;
	bi.lpfn = expBrowseCallbackProc;
	bi.lParam = (LPARAM)(LPCSTR)RefZoneDir;
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	RefZoneDir = str;
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonOutzonedir() 
{
	BROWSEINFO	bi;
	char		str[MAX_PATH];
	ITEMIDLIST*	pidl;
	char sTemp[1024];

	bi.hwndOwner = this->m_hWnd;
	bi.pidlRoot = NULL;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = sTemp;;
	bi.lpszTitle = "Choose the path";
	bi.ulFlags = 0;
	bi.lpfn = expBrowseCallbackProc;
	bi.lParam = (LPARAM)(LPCSTR)OutZoneDir;
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	OutZoneDir = str;
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonTilebankfile() 
{
	CFileDialog dialog (true, "smallbank", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "SmallBank (*.smallbank)|*.smallbank", this);
	if (dialog.DoModal() == IDOK)
	{
		TileBankFile = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnButtonHeightmapfile() 
{
	CFileDialog dialog (true, "tga", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Grayscale Tga (*.tga)|*.tga", this);
	if (dialog.DoModal() == IDOK)
	{
		HeightMapFile = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}