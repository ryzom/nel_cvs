/** \file start_stop_particle_system.cpp
 * <File description>
 *
 * $Id: start_stop_particle_system.cpp,v 1.1 2001/06/12 08:39:50 vizerie Exp $
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


// start_stop_particle_system.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "start_stop_particle_system.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem dialog


CStartStopParticleSystem::CStartStopParticleSystem(CWnd* pParent /*=NULL*/)
	: CDialog(CStartStopParticleSystem::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStartStopParticleSystem)
	//}}AFX_DATA_INIT
}


void CStartStopParticleSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStartStopParticleSystem)
	DDX_Control(pDX, IDC_STOP_PICTURE, m_StopPicture);
	DDX_Control(pDX, IDC_START_PICTURE, m_StartPicture);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStartStopParticleSystem, CDialog)
	//{{AFX_MSG_MAP(CStartStopParticleSystem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem message handlers

BOOL CStartStopParticleSystem::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	HBITMAP bm[2] ;
		
	bm[0] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_START_SYSTEM)) ;
	bm[1] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_STOP_SYSTEM)) ;

	
	m_StartPicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[0]) ;
	m_StopPicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[1]) ;

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
