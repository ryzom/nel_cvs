// edit_ps_sound.cpp : implementation file
//

#include "std_afx.h"
#include "3d/ps_located.h"
#include "nel/3d/u_particle_system_sound.h"
#include "3d/particle_system.h"
#include "object_viewer.h"
#include "edit_ps_sound.h"
#include "attrib_dlg.h"
#include "pick_sound.h"
#include "sound_system.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/// particle system sound system initialisation
void initPSSoundSystem(NLSOUND::UAudioMixer *am)
{
	NL3D::UParticleSystemSound::initPSSound(am);	
}

void releasePSSoundSystem(void)
{
	// nothing to do for now
}







/////////////////////////////////////////////////////////////////////////////
// CEditPSSound dialog



CEditPSSound::CEditPSSound(NL3D::CPSSound *sound) : _Sound(sound), _GainDlg(NULL)
												   , _PitchDlg(NULL)
{
	nlassert(sound);
}

CEditPSSound::~CEditPSSound()
{
	if (_GainDlg)
	{
		_GainDlg->DestroyWindow();
		delete _GainDlg;
	}
	if (_PitchDlg)
	{
		_PitchDlg->DestroyWindow();
		delete _PitchDlg;
	}
}

void CEditPSSound::init(CWnd* pParent /*= NULL*/)
{
	Create(IDD_SOUND, pParent);
	
	const uint posX = 3;
	uint	   posY = 60;
	RECT	   r;
	


	nlassert(_Sound);

	_GainWrapper.S = _Sound;
	_GainDlg = new CAttribDlgFloat(std::string("SOUND VOLUME"), 0, 1);
	_GainDlg->setWrapper(&_GainWrapper);
	_GainDlg->setSchemeWrapper(&_GainWrapper);	
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SOUND_VOLUME));
	_GainDlg->init(bmh, posX, posY, this);
	_GainDlg->GetClientRect(&r);
	posY += r.bottom + 3;	


	_PitchWrapper.S = _Sound;
	_PitchDlg = new CAttribDlgFloat(std::string("SOUND FREQ"), 0, 1);
	_PitchDlg->setWrapper(&_PitchWrapper);
	_PitchDlg->setSchemeWrapper(&_PitchWrapper);	
	bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_SOUND_FREQ));
	_PitchDlg->init(bmh, posX, posY, this);
	_PitchDlg->GetClientRect(&r);
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
	CPickSound::TNameVect names;
	

	NLSOUND::UAudioMixer *audioMixer = CSoundSystem::getAudioMixer();
	if (audioMixer)
	{
		audioMixer->getSoundNames(names);
	}

	CPickSound ps(names, this);

	if (ps.DoModal() == IDOK)
	{
		m_SoundName = ps.getName().c_str();
		_Sound->setSoundName(ps.getName());
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
