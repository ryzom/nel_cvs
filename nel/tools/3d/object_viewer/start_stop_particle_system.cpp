/** \file start_stop_particle_system.cpp
 * a pop-up dialog that allow to start and stop a particle system
 *
 * $Id: start_stop_particle_system.cpp,v 1.12 2002/04/25 08:31:35 vizerie Exp $
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

#include "3d/particle_system.h"
#include "3d/ps_located.h"
#include "3d/ps_sound.h"
#include "3d/particle_system_model.h"
#include "particle_dlg.h"


#include  <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*static char trace_buf[200];
#define NL_TRACE sprintf(trace_buf, "%d", __LINE__); \
				::MessageBox(NULL, trace_buf, NULL, MB_OK);
*/

/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem dialog


CStartStopParticleSystem::CStartStopParticleSystem(CParticleDlg *particleDlg)
	: CDialog(CStartStopParticleSystem::IDD, particleDlg), _ParticleDlg(particleDlg), _Running(FALSE) 
{
	//{{AFX_DATA_INIT(CStartStopParticleSystem)
	m_DisplayBBox = TRUE;
	//}}AFX_DATA_INIT
}

bool CStartStopParticleSystem::isBBoxDisplayEnabled()
{
	UpdateData();
	return m_DisplayBBox ? true : false;
}
	
void CStartStopParticleSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStartStopParticleSystem)
	DDX_Control(pDX, IDC_STOP_PICTURE, m_StopPicture);
	DDX_Control(pDX, IDC_START_PICTURE, m_StartPicture);
	DDX_Check(pDX, IDC_DISPLAY_BBOX, m_DisplayBBox);
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
	
	HBITMAP bm[2];
		
	bm[0] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_START_SYSTEM));
	bm[1] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_STOP_SYSTEM));

	
	m_StartPicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[0]);
	m_StopPicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[1]);


	m_StopPicture.EnableWindow(FALSE);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStartStopParticleSystem::OnStartSystem() 
{


	_Running = true;
	_SystemInitialPos.copySystemInitialPos(_ParticleDlg->getCurrPS() );	
	// enable the system to take the right date from the scene	
	_ParticleDlg->getCurrPSModel()->enableAutoGetEllapsedTime(true);		
	_ParticleDlg->getCurrPSModel()->enableDisplayTools(false); 	


	_ParticleDlg->ParticleTreeCtrl->suppressLocatedInstanceNbItem(0);


	m_StartPicture.EnableWindow(FALSE);
	m_StopPicture.EnableWindow(TRUE);	
	UpdateData(FALSE);

	NL3D::CParticleSystem *ps = _SystemInitialPos.getPS();
	if (ps)
	{
		ps->reactivateSound();		
	}
}

void CStartStopParticleSystem::OnStopSystem() 
{
	_Running = false;
	_SystemInitialPos.restoreSystem();

	_ParticleDlg->ParticleTreeCtrl->rebuildLocatedInstance();
	_ParticleDlg->getCurrPSModel()->enableAutoGetEllapsedTime(false);	
	_ParticleDlg->getCurrPSModel()->setEllapsedTime(0.f); // pause
	_ParticleDlg->getCurrPSModel()->enableDisplayTools(true); 

	m_StartPicture.EnableWindow(TRUE);
	m_StopPicture.EnableWindow(FALSE);
	UpdateData(FALSE);
	// go through all the ps to disable sounds
	
	NL3D::CParticleSystem *ps = _SystemInitialPos.getPS();
	if (ps)
	{
		ps->stopSound();
	}
}

void CStartStopParticleSystem::toggle()
{
	if (_Running) stop();
	else start();
}



void CStartStopParticleSystem::start()
{
	if (!_Running)
	{
		OnStartSystem();
	}
}



void CStartStopParticleSystem::stop()
{
	if (_Running)
	{
		OnStopSystem();
	}
}


///////////////////////////////////
// CPSInitialPos  implementation //
///////////////////////////////////
void CPSInitialPos::reset()
{
	_InitInfoVect.clear();
	_RotScaleInfoVect.clear();
	_InitialSizeVect.clear();
}
void CPSInitialPos::copySystemInitialPos(NL3D::CParticleSystem *ps)
{
	reset();
	uint32 nbLocated = ps->getNbProcess();

	_PS = ps; 
	for(uint32 k = 0; k < nbLocated; ++k)
	{

		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k));
		if (loc)
		{

			_InitialSizeVect.push_back(std::make_pair(loc, loc->getSize()) );
			for (uint32 l = 0; l < loc->getSize(); ++l)
			{

				CInitPSInstanceInfo ii;
				ii.Index = l;
				ii.Loc = loc;
				ii.Pos = loc->getPos()[l];
				ii.Speed = loc->getSpeed()[l];
				_InitInfoVect.push_back(ii);
				
				for (uint32 m = 0; m < loc->getNbBoundObjects(); ++m)
				{

					if (dynamic_cast<NL3D::IPSMover *>(loc->getBoundObject(m)))
					{
						CRotScaleInfo rsi;
						rsi.Loc = loc;
						rsi.LB = loc->getBoundObject(m);
						rsi.Index = l;
						rsi.Psm = dynamic_cast<NL3D::IPSMover *>(loc->getBoundObject(m));
						rsi.Scale = rsi.Psm->getScale(l);											
						rsi.Rot = rsi.Psm->getMatrix(l);
						_RotScaleInfoVect.push_back(rsi);
					}
				}
			}
		}
	}
}


// PRIVATE : a predicate used in CPSInitialPos::removeLocated
struct CRemoveLocatedPred
{
	NL3D::CPSLocated *Loc	;
};

// private : predicate to remove located from a CPSInitialPos::TInitialLocatedSizeVect vector
struct CRemoveLocatedFromLocatedSizePred : public CRemoveLocatedPred
{
	bool operator()(const std::pair<NL3D::CPSLocated *, uint32> &value) { return Loc == value.first; }
};

// private : predicate to remove located from a PSInitialPos::CInitPSInstanceInfo vector
struct CRemoveLocatedFromInitPSInstanceInfoVectPred : public CRemoveLocatedPred
{
	bool operator()(const CPSInitialPos::CInitPSInstanceInfo &value) { return value.Loc == Loc; }
};

// private : predicate to remove located from a PSInitialPos::CRotScaleInfo vector
struct CRemoveLocatedFromRotScaleInfoVectPred : public CRemoveLocatedPred
{
	bool operator()(const CPSInitialPos::CRotScaleInfo &value) { return value.Loc == Loc; }
};

// private : predicate to remove located bindable pointers in a TRotScaleInfoVect vect
struct CRemoveLocatedBindableFromRotScaleInfoVectPred
{
	// the located bindable taht has been removed
	NL3D::CPSLocatedBindable *LB;
	bool operator()(const CPSInitialPos::CRotScaleInfo &value) { return value.LB == LB; }
};




void CPSInitialPos::removeLocated(NL3D::CPSLocated *loc)
{
	// in each container, we delete every element that has a pointer over lthe located loc
	// , by using the dedicated predicate. 

	CRemoveLocatedFromLocatedSizePred p;
	p.Loc = loc;
	_InitialSizeVect.erase(std::remove_if(_InitialSizeVect.begin(), _InitialSizeVect.end(), p)
							, _InitialSizeVect.end() );

	CRemoveLocatedFromInitPSInstanceInfoVectPred p2;
	p2.Loc = loc;
	_InitInfoVect.erase(std::remove_if(_InitInfoVect.begin(), _InitInfoVect.end(), p2)
						, _InitInfoVect.end());

	CRemoveLocatedFromRotScaleInfoVectPred p3;
	p3.Loc = loc;
	_RotScaleInfoVect.erase(std::remove_if(_RotScaleInfoVect.begin(), _RotScaleInfoVect.end(), p3)
							 , _RotScaleInfoVect.end());

}


void CPSInitialPos::removeLocatedBindable(NL3D::CPSLocatedBindable *lb)
{
	CRemoveLocatedBindableFromRotScaleInfoVectPred p;
	p.LB = lb;
	_RotScaleInfoVect.erase(std::remove_if(_RotScaleInfoVect.begin(), _RotScaleInfoVect.end(), p)
							, _RotScaleInfoVect.end() );
}





	// reinitialize the system with its initial instances positions
void CPSInitialPos::restoreSystem()
{
	nlassert(_PS); // no system has been memorized yet


	// delete all the instance of the system

	for (uint k = 0; k < _PS->getNbProcess(); ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(_PS->getProcess(k));
		if (loc)
		{
			while (loc->getSize())
			{
				loc->deleteElement(0);
			}

			nlassert(loc->getSize() == 0);
		}
	}

	// recreate the initial number of instances

	for (TInitialLocatedSizeVect ::iterator itSize = _InitialSizeVect.begin(); itSize != _InitialSizeVect.end(); ++itSize)
	{
	//	nlassert(itSize->first->getSize() == 0)
		for (uint l = 0; l < itSize->second; ++l)
		{
			itSize->first->newElement();
		}

		uint realSize = itSize->first->getSize();
		uint size = itSize->second;
		
	}



	for (TInitInfoVect::iterator it = _InitInfoVect.begin(); it != _InitInfoVect.end(); ++it)
	{
		if (it->Index < it->Loc->getSize())
		{
			it->Loc->getPos()[it->Index] = it->Pos;
			it->Loc->getSpeed()[it->Index] = it->Speed;
		}
	}
	for (TRotScaleInfoVect::iterator it2 = _RotScaleInfoVect.begin(); it2 != _RotScaleInfoVect.end(); ++it2)
	{
		if (it2->Index < it2->Loc->getSize())
		{
			it2->Psm->setMatrix(it2->Index, it2->Rot);
			if (it2->Psm->supportNonUniformScaling())
			{
				it2->Psm->setScale(it2->Index, it2->Scale);
			}
			else if (it2->Psm->supportUniformScaling())
			{
				it2->Psm->setScale(it2->Index, it2->Scale.x);
			}
		}
	}
}
