/** \file start_stop_particle_system.cpp
 * <File description>
 *
 * $Id: start_stop_particle_system.cpp,v 1.2 2001/06/15 16:05:03 vizerie Exp $
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

#include "nel/3d/particle_system.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/particle_system_model.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem dialog


CStartStopParticleSystem::CStartStopParticleSystem(CParticleDlg *particleDlg)
	: CDialog(CStartStopParticleSystem::IDD, particleDlg), _ParticleDlg(particleDlg)
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
	ON_BN_CLICKED(IDC_START_PICTURE, OnStartSystem)
	ON_BN_CLICKED(IDC_STOP_PICTURE, OnStopSystem)
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


	m_StopPicture.EnableWindow(FALSE) ;

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStartStopParticleSystem::OnStartSystem() 
{
	_SystemInitialPos.copySystemInitialPos(_ParticleDlg->getCurrPS() ) ;
	_ParticleDlg->getCurrPSModel()->setEllapsedTime(0.01f) ;
	_ParticleDlg->getCurrPSModel()->enableDisplayTools(false) ; 

	m_StartPicture.EnableWindow(FALSE) ;
	m_StopPicture.EnableWindow(TRUE) ;
	UpdateData(FALSE) ;	
}

void CStartStopParticleSystem::OnStopSystem() 
{
	_SystemInitialPos.restoreSystem() ;
	_ParticleDlg->getCurrPSModel()->setEllapsedTime(0.f) ; // pause
	_ParticleDlg->getCurrPSModel()->enableDisplayTools(true) ; 

	m_StartPicture.EnableWindow(TRUE) ;
	m_StopPicture.EnableWindow(FALSE) ;
	UpdateData(FALSE) ;

}


///////////////////////////////////
// CPSInitialPos  implementation //
///////////////////////////////////
void CPSInitialPos::copySystemInitialPos(NL3D::CParticleSystem *ps)
{
	_StartInfos.clear() ;
	uint32 nbLocated = ps->getNbProcess() ;
	_PS = ps ; 
	for(uint32 k = 0 ; k < nbLocated ; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k)) ;
		if (loc)
		{
			for (uint32 l = 0 ; l < loc->getSize() ; ++l)
			{
				_StartInfos[loc].push_back(CInitPSInstanceInfo(loc->getPos()[l], loc->getSpeed()[l])) ;
			}
		}
	}
}




	// reinitialize the system with its initial instances positions
void CPSInitialPos::restoreSystem()
{
	nlassert(_PS) ; // no system has been memorized yet
	uint32 nbLocated = _PS->getNbProcess() ;
	for(uint32 k = 0 ; k < nbLocated ; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(_PS->getProcess(k)) ;
		if (loc)
		{
			uint32 size = loc->getSize() ;
			for	(uint32 l = 0 ; l < size ; ++l)
			{
				loc->deleteElement(0) ;
			}
			if (_StartInfos.count(loc))
			{
				TInitInfoVect::const_iterator curr = _StartInfos[loc].begin()
											 , end = _StartInfos[loc].end() ;

				while (curr != end)
				{
					loc->newElement(curr->Pos, curr->Speed) ;
					++curr ;
				}

			}
		}
	}
}
