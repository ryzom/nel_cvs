// Generate.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEditor.h"
#include "resource.h"
#include "Generate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGenerate dialog


CGenerate::CGenerate(CWnd* pParent /*=NULL*/)
	: CDialog(CGenerate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGenerate)
	MinX = 0;
	MinY = 0;
	MaxY = 0;
	MaxX = 0;
	ZoneBaseX = 0;
	ZoneBaseY = 0;
	ComboMaterialString = _T("");
	//}}AFX_DATA_INIT
}


void CGenerate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenerate)
	DDX_Control(pDX, IDC_COMBOMATERIAL, ComboMaterial);
	DDX_Text(pDX, IDC_EDITMINX, MinX);
	DDX_Text(pDX, IDC_EDITMINY, MinY);
	DDX_Text(pDX, IDC_EDITMAXY, MaxY);
	DDX_Text(pDX, IDC_EDITMAXX, MaxX);
	DDX_Text(pDX, IDC_EDITZONEBASEX, ZoneBaseX);
	DDX_Text(pDX, IDC_EDITZONEBASEY, ZoneBaseY);
	DDX_CBString(pDX, IDC_COMBOMATERIAL, ComboMaterialString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenerate, CDialog)
	//{{AFX_MSG_MAP(CGenerate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenerate message handlers

BOOL CGenerate::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (uint32 i = 0; i < AllMaterials.size(); ++i)
		ComboMaterial.InsertString(-1, AllMaterials[i].c_str());
	ComboMaterial.SetCurSel (0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGenerate::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData (TRUE);

	if (MinX > MaxX)
	{
		MessageBox ("MinX > MaxX", "Error", MB_OK|MB_ICONSTOP);
		return;
	}
	if (MinY > MaxY)
	{
		MessageBox ("MinY > MaxY", "Error", MB_OK|MB_ICONSTOP);
		return;
	}
	CDialog::OnOK();
}
