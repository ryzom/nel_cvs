// light_group_factor.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "light_group_factor.h"

using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CLightGroupFactor dialog


CLightGroupFactor::CLightGroupFactor(CWnd* pParent /*=NULL*/)
	: CDialog(CLightGroupFactor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightGroupFactor)
	LightGroup1 = 255;
	LightGroup2 = 255;
	LightGroup3 = 255;
	//}}AFX_DATA_INIT
}


void CLightGroupFactor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightGroupFactor)
	DDX_Control(pDX, IDC_LIGHT_GROUP3, LightGroup3Ctrl);
	DDX_Control(pDX, IDC_LIGHT_GROUP2, LightGroup2Ctrl);
	DDX_Control(pDX, IDC_LIGHT_GROUP1, LightGroup1Ctrl);
	DDX_Slider(pDX, IDC_LIGHT_GROUP1, LightGroup1);
	DDX_Slider(pDX, IDC_LIGHT_GROUP2, LightGroup2);
	DDX_Slider(pDX, IDC_LIGHT_GROUP3, LightGroup3);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightGroupFactor, CDialog)
	//{{AFX_MSG_MAP(CLightGroupFactor)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightGroupFactor message handlers

BOOL CLightGroupFactor::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Light group
	LightGroup1Ctrl.SetRange (0, 255);
	LightGroup2Ctrl.SetRange (1, 255);
	LightGroup3Ctrl.SetRange (2, 255);
	LightGroup1 = 255;
	LightGroup2 = 255;
	LightGroup3 = 255;

	UpdateData (FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightGroupFactor::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	handle ();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CLightGroupFactor::handle ()
{
	UpdateData ();

	// Set the light factors
	CNELU::Scene->setLightGroupColor (0, CRGBA (LightGroup1, LightGroup1, LightGroup1));
	CNELU::Scene->setLightGroupColor (1, CRGBA (LightGroup2, LightGroup2, LightGroup2));
	CNELU::Scene->setLightGroupColor (2, CRGBA (LightGroup3, LightGroup3, LightGroup3));
}