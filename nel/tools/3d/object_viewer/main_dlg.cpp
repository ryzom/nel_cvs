// main_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "main_dlg.h"
#include <nel/3d/channel_mixer.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NLMISC;
using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CMainDlg dialog


CMainDlg::CMainDlg(CObjectViewer* main, CWnd* pParent /*=NULL*/)
	: CDialog(CMainDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMainDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Main=main;
}


void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
	//{{AFX_MSG_MAP(CMainDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainDlg message handlers

BOOL CMainDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Get size
	RECT rect;
		
	// Init Slots
	for (uint s=0; s<CChannelMixer::NumAnimationSlot; s++)
	{
		// Init the slot s
		BOOL initialized=Slots[s].Create (IDD_SLOT, this);

		// Position
		Slots[s].init (s, AnimationSet, Main);
		Slots[s].GetClientRect (&rect);
		Slots[s].SetWindowPos( NULL, 0, s*(rect.bottom-rect.top), 0, 0, SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_SHOWWINDOW);
		Slots[s].setAnimation (0, NULL, NULL);
	}

	// Resize the main window
	RECT client;
	RECT window;
	GetClientRect (&client);
	GetWindowRect (&window);
	SetWindowPos( NULL, 0, 0, rect.right-rect.left+window.right-window.left-client.right+client.left, 
		NL3D::CChannelMixer::NumAnimationSlot*(rect.bottom-rect.top)+window.bottom-window.top-client.bottom+client.top, 
		SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_SHOWWINDOW);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMainDlg::init (NL3D::CAnimationSet* animationSet)
{
	AnimationSet=animationSet;
}

void CMainDlg::setAnimTime (float animStart, float animEnd)
{
	// Init Slots
	for (uint s=0; s<CChannelMixer::NumAnimationSlot; s++)
		Slots[s].setAnimTime (animStart, animEnd);
}

void CMainDlg::getSlot ()
{
	for (uint id=0; id<NL3D::CChannelMixer::NumAnimationSlot; id++)
	{
		// Update slots UI values
		Slots[id].UpdateData();

		// Set the animation
		if (Slots[id].isEmpty ())
			Playlist.setAnimation (id, CAnimationPlaylist::empty);
		else			
			Playlist.setAnimation (id, Slots[id].AnimationId);

		// Set the skeleton weight
		if (Slots[id].SkeletonWeightTemplate)
			Playlist.setSkeletonWeight (id, Slots[id].SkeletonWeightId, Slots[id].SkeletonWeightInverted!=0);
		else
			Playlist.setSkeletonWeight (id, CAnimationPlaylist::empty, false);

		// Set others values
		Playlist.setTimeOrigin (id, Slots[id].Offset);
		Playlist.setSpeedFactor (id, Slots[id].SpeedFactor);
		Playlist.setStartWeight (id, Slots[id].StartBlend, Slots[id].StartTime);
		Playlist.setEndWeight (id, Slots[id].EndBlend, Slots[id].EndTime);
		Playlist.setWeightSmoothness (id, Slots[id].Smoothness);

		// Switch between wrap modes
		switch (Slots[id].ClampMode)
		{
		case 0:
			Playlist.setWrapMode (CAnimationPlaylist::Clamp);
			break;
		case 1:
			Playlist.setWrapMode (CAnimationPlaylist::Repeat);
			break;
		case 2:
			Playlist.setWrapMode (CAnimationPlaylist::Disable);
			break;
		}
	}
}


void CMainDlg::setSlot ()
{
	for (uint id=0; id<NL3D::CChannelMixer::NumAnimationSlot; id++)
	{
		// Update slots UI values
		Slots[id].UpdateData();

		// Set the animation
		uint animation=Playlist.getAnimation (id);
		if (animation<AnimationSet->getNumAnimation())
			Slots[id].setAnimation (animation, AnimationSet->getAnimation (animation), AnimationSet->getAnimationName (animation).c_str());

		// Set the skeleton weight
		bool inverted;
		uint skeleton=Playlist.getSkeletonWeight (id, inverted);
		Slots[id].SkeletonWeightInverted=inverted;
		if (skeleton<AnimationSet->getNumSkeletonWeight())
			Slots[id].setSkeletonTemplateWeight (skeleton, AnimationSet->getSkeletonWeight (skeleton));

		// Set others values
		Slots[id].Offset=Playlist.getTimeOrigin (id);
		Slots[id].SpeedFactor=Playlist.getSpeedFactor (id);
		Slots[id].StartBlend=Playlist.getStartWeight (id, Slots[id].StartTime);
		Slots[id].EndBlend=Playlist.getEndWeight (id, Slots[id].EndTime);
		Slots[id].Smoothness=Playlist.getWeightSmoothness (id);

		// Switch between wrap modes
		switch (Playlist.getWrapMode ())
		{
		case CAnimationPlaylist::Clamp:
			Slots[id].ClampMode=0;
			break;
		case CAnimationPlaylist::Repeat:
			Slots[id].ClampMode=1;
			break;
		case CAnimationPlaylist::Disable:
			Slots[id].ClampMode=2;
			break;
		}
		Slots[id].UpdateData(FALSE);
	}
}
