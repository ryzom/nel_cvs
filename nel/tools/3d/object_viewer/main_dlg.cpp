/** \file main_dlg.cpp
 * <File description>
 *
 * $Id: main_dlg.cpp,v 1.5 2001/06/15 16:24:45 corvazier Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "std_afx.h"
#include "object_viewer.h"
#include "main_dlg.h"
#include <3d/channel_mixer.h>

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
	ON_WM_DESTROY()
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
		if (Slots[id].isEmpty ()||!Slots[id].enable)
			Playlist.setAnimation (id, CAnimationPlaylist::empty);
		else			
			Playlist.setAnimation (id, Slots[id].AnimationId);

		// Set the skeleton weight
		if (Slots[id].SkeletonWeightTemplate)
			Playlist.setSkeletonWeight (id, Slots[id].SkeletonWeightId, Slots[id].SkeletonWeightInverted!=0);
		else
			Playlist.setSkeletonWeight (id, CAnimationPlaylist::empty, false);

		// Set others values
		Playlist.setTimeOrigin (id, Slots[id].getTimeOffset());
		Playlist.setSpeedFactor (id, Slots[id].SpeedFactor);
		Playlist.setStartWeight (id, Slots[id].StartBlend, Slots[id].getStartTime());
		Playlist.setEndWeight (id, Slots[id].EndBlend, Slots[id].getEndTime());
		Playlist.setWeightSmoothness (id, Slots[id].Smoothness);

		// Switch between wrap modes
		switch (Slots[id].ClampMode)
		{
		case 0:
			Playlist.setWrapMode (id, CAnimationPlaylist::Clamp);
			break;
		case 1:
			Playlist.setWrapMode (id, CAnimationPlaylist::Repeat);
			break;
		case 2:
			Playlist.setWrapMode (id, CAnimationPlaylist::Disable);
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
			Slots[id].setSkeletonTemplateWeight (skeleton, AnimationSet->getSkeletonWeight (skeleton), AnimationSet->getSkeletonWeightName (skeleton).c_str());

		// Set others values
		Slots[id].Offset=(int)(Playlist.getTimeOrigin (id)*Main->getFrameRate());
		Slots[id].SpeedFactor=Playlist.getSpeedFactor (id);
		float time;
		Slots[id].StartBlend=Playlist.getStartWeight (id, time);
		Slots[id].StartTime=(int)(time*Main->getFrameRate());
		Slots[id].EndBlend=Playlist.getEndWeight (id, time);
		Slots[id].EndTime=(int)(time*Main->getFrameRate());
		Slots[id].Smoothness=Playlist.getWeightSmoothness (id);

		// Switch between wrap modes
		switch (Playlist.getWrapMode (id))
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
		Slots[id].updateScrollBar ();
	}
}

void CMainDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_OBJ_VIEW_SLOT_DLG);

	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
