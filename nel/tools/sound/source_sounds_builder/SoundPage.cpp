// SoundPage.cpp : implementation file
//

#include "stdafx.h"
#include "source_sounds_builder.h"
#include "source_sounds_builderDlg.h"
#include "SoundPage.h"

#include <nel/misc/common.h>
using namespace NLMISC;

#include <string>
using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSoundPage dialog


CSoundPage::CSoundPage(CWnd* pParent /*=NULL*/)
	: CDialog(CSoundPage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundPage)
	m_Filename = _T("");
	m_Gain = 1.0f;
	m_Pos3D = FALSE;
	m_MinDist = 1.0f;
	m_MaxDist = 1000000.0f;
	m_InnerAngleDeg = 360;
	m_OuterAngleDeg = 360;
	m_OuterGain = 1.0f;
	m_Looped = FALSE;
	//}}AFX_DATA_INIT

	_CurrentSound = NULL;
	_Tree = NULL;
	_AudioMixer = NULL;
	_Source = NULL;
}


void CSoundPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundPage)
	DDX_Text(pDX, IDC_EditFilename, m_Filename);
	DDX_Text(pDX, IDC_EditGain, m_Gain);
	DDV_MinMaxFloat(pDX, m_Gain, 0.f, 1.f);
	DDX_Check(pDX, IDC_Pos3D, m_Pos3D);
	DDX_Text(pDX, IDC_EditMinDist, m_MinDist);
	DDV_MinMaxFloat(pDX, m_MinDist, 0.f, 1.e+006f);
	DDX_Text(pDX, IDC_EditMaxDist, m_MaxDist);
	DDV_MinMaxFloat(pDX, m_MaxDist, 0.f, 1.e+006f);
	DDX_Text(pDX, IDC_EditInnerAngle, m_InnerAngleDeg);
	DDV_MinMaxUInt(pDX, m_InnerAngleDeg, 0, 360);
	DDX_Text(pDX, IDC_EditOuterAngle, m_OuterAngleDeg);
	DDV_MinMaxUInt(pDX, m_OuterAngleDeg, 0, 360);
	DDX_Text(pDX, IDC_EditOuterGain, m_OuterGain);
	DDV_MinMaxFloat(pDX, m_OuterGain, 0.f, 1.f);
	DDX_Check(pDX, IDC_Looped, m_Looped);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSoundPage, CDialog)
	//{{AFX_MSG_MAP(CSoundPage)
	ON_BN_CLICKED(IDC_Pos3D, OnPos3D)
	ON_BN_CLICKED(IDC_Apply, OnApply)
	ON_BN_CLICKED(IDC_ChooseFile, OnChooseFile)
	ON_BN_CLICKED(IDC_Remove, OnRemove)
	ON_BN_CLICKED(IDC_PlaySound, OnPlaySound)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_Cancel, OnCancel)
	ON_BN_CLICKED(IDC_Looped, OnLooped)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*
 *
 */
void		CSoundPage::getPropertiesFromSound()
{
	m_Filename = _CurrentSound->getFilename().c_str();
	m_Gain = _CurrentSound->getGain();
	m_Pos3D = _CurrentSound->isDetailed();

	if ( m_Pos3D )
	{
		m_MinDist = _CurrentSound->getMinDistance();
		m_MaxDist = _CurrentSound->getMaxDistance();
		m_InnerAngleDeg = radToDeg( _CurrentSound->getConeInnerAngle() );
		m_OuterAngleDeg = radToDeg( _CurrentSound->getConeOuterAngle() );
		m_OuterGain = _CurrentSound->getConeOuterGain();
	}
	UpdateData( false );

	OnPos3D(); // enable/disable 3d properties
}


/////////////////////////////////////////////////////////////////////////////
// CSoundPage message handlers


/*
 *
 */
void CSoundPage::setCurrentSound( CSound *sound, HTREEITEM hitem )
{
	_CurrentSound = sound;
	_HItem = hitem;
	if ( _Source != NULL )
	{
		_Source->stop();
	}
	m_Looped = false;
	UpdateData( false );
}


/*
 *
 */
void CSoundPage::OnPos3D() 
{
	UpdateData( true );
	GetDlgItem( IDC_EditMinDist )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditMaxDist )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditInnerAngle )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditOuterAngle )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditOuterGain )->EnableWindow( m_Pos3D );
}


/*
 *
 */
void CSoundPage::OnApply() 
{
	if ( _Source != NULL )
	{
		_Source->stop();
	}

	UpdateData( true );
	UpdateCurrentSound();

	(static_cast<CSource_sounds_builderDlg*>(GetOwner()))->setModified();

	nlassert( _Tree && _CurrentSound );
	_Tree->SetItemText( _HItem, m_Filename );
	_Tree->SelectItem( NULL );

	GetOwner()->SetFocus();
}


/*
 *
 */
void CSoundPage::OnCancel() 
{
	if ( _Source != NULL )
	{
		_Source->stop();
	}

	nlassert( _Tree );
	_Tree->SelectItem( NULL );
	GetOwner()->SetFocus();
}


/*
 *
 */
void CSoundPage::OnChooseFile() 
{
	// Prompt filename
	CFileDialog opendlg( true, "wav", "", 0, "PCM Wave files (*.wav)|*.wav", this );
	if ( opendlg.DoModal()==IDOK )
	{
		m_Filename = opendlg.GetFileName();
		UpdateData( false );
	}
}


/*
 *
 */
void CSoundPage::OnRemove() 
{
	nlassert( _Tree && _CurrentSound );
	_Tree->DeleteItem( _HItem );
}


/*
 *
 */
void CSoundPage::OnPlaySound() 
{
	CWaitCursor waitcursor;

	if ( _AudioMixer == NULL )
	{
		// Load driver
		_AudioMixer = UAudioMixer::createAudioMixer();
		try
		{
			_AudioMixer->init();
		}
		catch( Exception& e )
		{
			waitcursor.Restore();

			CString s;
			s.Format( "No sound driver: %s", e.what() );
			AfxMessageBox( s );
			return;
		}
	}

	// Load sound
	UpdateData( true );
	UpdateCurrentSound();
	try 
	{
		nlassert( _CurrentSound );
		_CurrentSound->loadBuffer( string(m_Filename) );

		// Play source
		if ( _Source == NULL )
		{
			_Source = _AudioMixer->createSource( _CurrentSound );
		}
		else
		{
			_Source->stop();
			_Source->setSound( _CurrentSound );
		}
		_Source->setLooping( m_Looped );
		_Source->play();

		waitcursor.Restore();
	}
	catch ( Exception& e )
	{
		waitcursor.Restore();

		CString s;
		s.Format( "%s", e.what() );
		AfxMessageBox( s );
	}
}


/*
 *
 */
void CSoundPage::OnLooped() 
{
	UpdateData( true );
	if ( m_Looped )
	{
		OnPlaySound();
	}
	else
	{
		if ( _Source != NULL )
		{
			_Source->stop();
		}
	}
}


/*
 *
 */
void CSoundPage::UpdateCurrentSound()
{
	if ( ! m_Pos3D )
	{
		_CurrentSound->setProperties( string(m_Filename), m_Gain, m_Pos3D );
	}
	else
	{
		_CurrentSound->setProperties( string(m_Filename), m_Gain, m_Pos3D,
			m_MinDist, m_MaxDist, degToRad(m_InnerAngleDeg), degToRad(m_OuterAngleDeg), m_OuterGain );
	}
}


/*
 *
 */
void CSoundPage::OnClose() 
{
	if ( _AudioMixer != NULL )
	{
		if ( _Source != NULL )
		{
			delete _Source;
		}
		delete _AudioMixer;
	}
}

