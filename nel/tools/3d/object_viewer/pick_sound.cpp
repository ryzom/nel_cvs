// pick_sound.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "pick_sound.h"
#include "sound_system.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPickSound dialog


CPickSound::CPickSound(TNameVect &names, CWnd* pParent /*=NULL*/)
	: CDialog(CPickSound::IDD, pParent), _Names(names)
{
	
	//{{AFX_DATA_INIT(CPickSound)
	//}}AFX_DATA_INIT
}


void CPickSound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPickSound)
	DDX_Control(pDX, IDC_LIST1, m_NameList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPickSound, CDialog)
	//{{AFX_MSG_MAP(CPickSound)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchange)
	ON_BN_CLICKED(IDC_BUTTON1, OnPlaySound)
	ON_BN_CLICKED(IDC_PLAY_SOUND, OnPlaySound)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPickSound message handlers

BOOL CPickSound::OnInitDialog() 
{
	CDialog::OnInitDialog();
	UpdateData();
	
	for (TNameVect::const_iterator it = _Names.begin(); it	!= _Names.end(); ++it)
	{
		m_NameList.AddString(*it);
	}

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPickSound::OnSelchange() 
{
	UpdateData();
	char str[1024];
	nlassert(m_NameList.GetTextLen(m_NameList.GetCurSel()) < 1024);
	
	m_NameList.GetText(m_NameList.GetCurSel(), str);
	_CurrName = str; 
	
}

void CPickSound::OnPlaySound() 
{
	int curSel = m_NameList.GetCurSel();
	if (curSel == LB_ERR) return;
	CString sName;
	m_NameList.GetText(curSel, sName);
	CSoundSystem::play(std::string( (LPCTSTR) sName));	
}
