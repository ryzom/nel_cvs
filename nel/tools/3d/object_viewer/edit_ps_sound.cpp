// edit_ps_sound.cpp : implementation file
//

#include "std_afx.h"
#include "3d/ps_located.h"
#include "object_viewer.h"
#include "edit_ps_sound.h"
#include "attrib_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditPSSound dialog



CEditPSSound::CEditPSSound(NL3D::CPSSound *sound) : _Sound(sound), _VolumeDlg(NULL)
												   , _FrequencyDlg(NULL)
{
	nlassert(sound);
}

CEditPSSound::~CEditPSSound()
{
	if (_VolumeDlg)
	{
		_VolumeDlg->DestroyWindow();
		delete _VolumeDlg;
	}
	if (_FrequencyDlg)
	{
		_FrequencyDlg->DestroyWindow();
		delete _FrequencyDlg;
	}
}

void CEditPSSound::init(CWnd* pParent /*= NULL*/)
{
	Create(IDD_SOUND, pParent);
	
	const uint posX = 3;
	uint	   posY = 60;
	RECT	   r;
	


	nlassert(_Sound);

	_VolumeWrapper.S = _Sound;
	_VolumeDlg = new CAttribDlgFloat(std::string("SOUND VOLUME"), 0, 1);
	_VolumeDlg->setWrapper(&_VolumeWrapper);
	_VolumeDlg->setSchemeWrapper(&_VolumeWrapper);	
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SOUND_VOLUME));
	_VolumeDlg->init(bmh, posX, posY, this);
	_VolumeDlg->GetClientRect(&r);
	posY += r.bottom + 3;	


	_FrequencyWrapper.S = _Sound;
	_FrequencyDlg = new CAttribDlgFloat(std::string("SOUND FREQ"), 0, 44000);
	_FrequencyDlg->setWrapper(&_FrequencyWrapper);
	_FrequencyDlg->setSchemeWrapper(&_FrequencyWrapper);	
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SOUND_FREQ));
	_FrequencyDlg->init(bmh, posX, posY, this);
	_FrequencyDlg->GetClientRect(&r);
	posY += r.bottom + 3;	


	ShowWindow(SW_SHOW); 
	UpdateData(FALSE);
}





void CEditPSSound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditPSSound)
	DDX_Text(pDX, IDC_SOUND_NAME, m_SoundName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditPSSound, CDialog)
	//{{AFX_MSG_MAP(CEditPSSound)
	ON_BN_CLICKED(IDC_BROWSE_SOUND, OnBrowseSound)
	ON_EN_CHANGE(IDC_SOUND_NAME, OnChangeSoundName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditPSSound message handlers

void CEditPSSound::OnBrowseSound() 
{
	CFileDialog fd(TRUE, ".wav", "*.wav", 0, NULL, this) ;
	if (fd.DoModal() == IDOK)
	{
		// Add to the path
		char drive[256];
		char dir[256];
		char path[256];

		// Add search path for the sound
		_splitpath (fd.GetPathName(), drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		NLMISC::CPath::addSearchPath (path);
		_Sound->setSoundName(std::string(path));
		m_SoundName = path;
		UpdateData(FALSE);												
	}					
}

BOOL CEditPSSound::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	nlassert(_Sound);
	m_SoundName = _Sound->getSoundName().c_str();
	
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditPSSound::OnChangeSoundName() 
{
	nlassert(_Sound);
	UpdateData();
	_Sound->setSoundName( (LPCTSTR) m_SoundName );	
}
