// ExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "master.h"
#include "ExportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog


// ---------------------------------------------------------------------------
CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDlg)
	GenerateLandscape = FALSE;
	GenerateVegetable = FALSE;
	OutLandscapeDir = _T("");
	OutVegetableDir = _T("");
	RegionName = _T("");
	_Options = NULL;
	LandBankFile = _T("");
	LandFarBankFile = _T("");
	_Finished = false;
	//}}AFX_DATA_INIT
}

// ---------------------------------------------------------------------------
void CExportDlg::setOptions (SExportOptions &options, vector<string> &regNames)
{
	_Options = &options;
	_Regions = &regNames;
}

// ---------------------------------------------------------------------------
void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
	DDX_Control(pDX, IDC_REGIONLIST, RegionList);
	DDX_Check(pDX, IDC_GENERATE_LANDSCAPE, GenerateLandscape);
	DDX_Check(pDX, IDC_GENERATE_VEGETABLE, GenerateVegetable);
	DDX_Text(pDX, IDC_OUT_LANDSCAPE_DIR, OutLandscapeDir);
	DDX_Text(pDX, IDC_OUT_VEGETABLE_DIR, OutVegetableDir);
	DDX_CBString(pDX, IDC_REGIONLIST, RegionName);
	DDX_Text(pDX, IDC_LAND_BANK_FILE, LandBankFile);
	DDX_Text(pDX, IDC_LAND_FAR_BANK_FILE, LandFarBankFile);
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	//{{AFX_MSG_MAP(CExportDlg)
	ON_BN_CLICKED(IDC_EXPLORE_LANDSCAPE, OnExploreOutLandscapeDir)
	ON_BN_CLICKED(IDC_EXPLORE_VEGETABLE, OnExploreOutVegetableDir)
	ON_BN_CLICKED(IDC_EXPLORE_LAND_BANK_FILE, OnExploreLandBankFile)
	ON_BN_CLICKED(IDC_EXPLORE_LAND_FAR_BANK_FILE, OnExploreLandFarBankFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

// ---------------------------------------------------------------------------
BOOL CExportDlg::OnInitDialog () 
{
	CDialog::OnInitDialog ();
	
	// TODO: Add extra initialization here
	GenerateLandscape = _Options->GenerateLandscape;
	GenerateVegetable = _Options->GenerateVegetable;
	OutLandscapeDir = _Options->OutLandscapeDir.c_str ();
	OutVegetableDir = _Options->OutVegetableDir.c_str ();
	LandBankFile = _Options->LandBankFile.c_str ();
	LandFarBankFile = _Options->LandFarBankFile.c_str ();
	for (uint32 i=0; i < _Regions->size(); ++i)
	{
		RegionList.InsertString (-1, (_Regions->operator[](i)).c_str());
	}
	RegionName = _Options->SourceDir.c_str ();
	UpdateData (FALSE); // Upload

	if (RegionName == "")
		RegionList.SetCurSel (0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ---------------------------------------------------------------------------
void CExportDlg::OnOK () 
{
	UpdateData (TRUE); // Download

	_Options->GenerateLandscape = GenerateLandscape?true:false;
	_Options->GenerateVegetable = GenerateVegetable?true:false;
	_Options->OutLandscapeDir = (LPCSTR)OutLandscapeDir;
	_Options->OutVegetableDir = (LPCSTR)OutVegetableDir;
	_Options->LandBankFile = (LPCSTR)LandBankFile;
	_Options->LandFarBankFile = (LPCSTR)LandFarBankFile;
	_Options->SourceDir = (LPCSTR)RegionName;

	CDialog::OnOK();
}

// ---------------------------------------------------------------------------
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
void CExportDlg::OnExploreOutLandscapeDir () 
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
	bi.lParam = (LPARAM)(LPCSTR)OutLandscapeDir;
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	OutLandscapeDir = str;
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnExploreOutVegetableDir() 
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
	bi.lParam = (LPARAM)(LPCSTR)OutVegetableDir;
	bi.iImage = 0;
	pidl = SHBrowseForFolder (&bi);
	if (!SHGetPathFromIDList(pidl, str)) 
	{
		return;
	}
	OutVegetableDir = str;
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnExploreLandBankFile() 
{
	CFileDialog dialog (true, "bank", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "Bank (*.bank)|*.bank", this);
	if (dialog.DoModal() == IDOK)
	{
		LandBankFile = dialog.GetPathName ();
	}
	UpdateData (FALSE); // Upload
}

// ---------------------------------------------------------------------------
void CExportDlg::OnExploreLandFarBankFile() 
{
	CFileDialog dialog (true, "farbank", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, "FarBank (*.farbank)|*.farbank", this);
	if (dialog.DoModal() == IDOK)
	{
		LandFarBankFile = dialog.GetPathName();
	}
	UpdateData (FALSE); // Upload
}
