// choose_animation.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "choose_animation.h"


/////////////////////////////////////////////////////////////////////////////
// CChooseAnimation dialog


CChooseAnimation::CChooseAnimation(CWnd* pParent /*=NULL*/)
	: CDialog(CChooseAnimation::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseAnimation)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChooseAnimation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseAnimation)
	DDX_Control(pDX, IDC_ANIM_LIST, m_AnimList);
	//}}AFX_DATA_MAP
}

void CChooseAnimation::init(const std::vector<std::string> &animList)
{
	_AnimList = animList;
}

//*********************************************************************************************************************
void CChooseAnimation::OnOK()
{
	_Selection = -1;
	for(uint k = 0; k < (uint) m_AnimList.GetCount(); ++k)
	{
		if (m_AnimList.GetSel(k))
		{
			_Selection = k;
			break;
		}
	}	
	CDialog::OnOK();
}

//*********************************************************************************************************************
void CChooseAnimation::OnCancel()
{
	_Selection = -1;
	CDialog::OnCancel();
}




BEGIN_MESSAGE_MAP(CChooseAnimation, CDialog)
	//{{AFX_MSG_MAP(CChooseAnimation)
	ON_LBN_DBLCLK(IDC_ANIM_LIST, OnDblclkAnimList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseAnimation message handlers

BOOL CChooseAnimation::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	for(uint k = 0; k < _AnimList.size(); ++k)
	{
		m_AnimList.InsertString(k, _AnimList[k].c_str());
	}	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseAnimation::OnDblclkAnimList() 
{
	OnOK();	
}
