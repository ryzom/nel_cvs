/** \file start_stop_particle_system.cpp
 * a pop-up dialog that allow to start and stop a particle system
 *
 * $Id: start_stop_particle_system.cpp,v 1.20 2004/03/11 17:27:08 vizerie Exp $
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
#include "located_properties.h"

#include "3d/particle_system.h"
#include "3d/ps_located.h"
#include "3d/ps_sound.h"
#include "3d/ps_emitter.h"
#include "3d/particle_system_model.h"
#include "particle_dlg.h"


#include  <algorithm>

/*static char trace_buf[200];
#define NL_TRACE sprintf(trace_buf, "%d", __LINE__); \
				::MessageBox(NULL, trace_buf, NULL, MB_OK);
*/

/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem dialog

//******************************************************************************************************
CStartStopParticleSystem::CStartStopParticleSystem(CParticleDlg *particleDlg,
												   CAnimationDlg *animationDLG)
	: CDialog(CStartStopParticleSystem::IDD, particleDlg),
	  _ParticleDlg(particleDlg),
	  _Running(false),
	  _Paused(false),
	  _ResetAutoCount(true),
	  _LastCurrNumParticles(-1),
	  _LastMaxNumParticles(-1),
	  _LastSystemDate(-1.f),
	  _AutoRepeat(false),
	  _AnimationDLG(animationDLG),
	  _LastSceneAnimFrame(0.f)
{
	nlassert(particleDlg && particleDlg->getObjectViewer());
	particleDlg->getObjectViewer()->registerMainLoopCallBack(this);
	//{{AFX_DATA_INIT(CStartStopParticleSystem)
	m_DisplayBBox = TRUE;
	m_SpeedSliderPos = 100;	
	m_DisplayHelpers = FALSE;
	m_LinkPlayToScenePlay = FALSE;
	//}}AFX_DATA_INIT
}

//******************************************************************************************************
CStartStopParticleSystem::~CStartStopParticleSystem()
{
	nlassert(_ParticleDlg && _ParticleDlg->getObjectViewer());
	_ParticleDlg->getObjectViewer()->removeMainLoopCallBack(this);
}

//******************************************************************************************************
bool CStartStopParticleSystem::isBBoxDisplayEnabled()
{
	UpdateData();
	return m_DisplayBBox ? true : false;
}
	
//******************************************************************************************************
void CStartStopParticleSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStartStopParticleSystem)
	DDX_Control(pDX, IDC_PAUSE_PICTURE, m_PausePicture);
	DDX_Control(pDX, IDC_STOP_PICTURE, m_StopPicture);
	DDX_Control(pDX, IDC_START_PICTURE, m_StartPicture);
	DDX_Check(pDX, IDC_DISPLAY_BBOX, m_DisplayBBox);	
	DDX_Slider(pDX, IDC_ANIM_SPEED, m_SpeedSliderPos);
	DDX_Check(pDX, IDC_DISPLAY_HELPERS, m_DisplayHelpers);
	DDX_Check(pDX, IDC_LINK_PLAY_TO_SCENE_PLAY, m_LinkPlayToScenePlay);
	//}}AFX_DATA_MAP


}


BEGIN_MESSAGE_MAP(CStartStopParticleSystem, CDialog)
	//{{AFX_MSG_MAP(CStartStopParticleSystem)
	ON_BN_CLICKED(IDC_START_PICTURE, OnStartSystem)
	ON_BN_CLICKED(IDC_STOP_PICTURE, OnStopSystem)
	ON_BN_CLICKED(IDC_PAUSE_PICTURE, OnPause)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_ANIM_SPEED, OnReleasedcaptureAnimSpeed)
	ON_BN_CLICKED(IDC_DISPLAY_HELPERS, OnDisplayHelpers)
	ON_BN_CLICKED(IDC_ENABLE_AUTO_COUNT, OnEnableAutoCount)
	ON_BN_CLICKED(IDC_RESET_COUNT, OnResetCount)
	ON_BN_CLICKED(IDC_AUTOREPEAT, OnAutoRepeat)
	ON_BN_CLICKED(IDC_LINK_PLAY_TO_SCENE_PLAY, OnLinkPlayToScenePlay)
	ON_BN_CLICKED(IDC_LINK_TO_SKELETON, OnLinkToSkeleton)
	ON_BN_CLICKED(IDC_UNLINK_FROM_SKELETON, OnUnlinkFromSkeleton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem message handlers

//******************************************************************************************************
BOOL CStartStopParticleSystem::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	HBITMAP bm[3];		
	bm[0] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_START_SYSTEM));
	bm[1] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_STOP_SYSTEM));
	bm[2] = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PAUSE_SYSTEM));	
	m_StartPicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[0]);
	m_StopPicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[1]);
	m_PausePicture.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bm[2]);
	m_StopPicture.EnableWindow(FALSE);
	m_PausePicture.EnableWindow(FALSE);
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_ANIM_SPEED);
	sl->SetRange(0, 100);
	setSpeedSliderValue(1.f);
	GetDlgItem(IDC_DISPLAY_HELPERS)->EnableWindow(FALSE);	
	((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->SetCheck(0);
	GetDlgItem(IDC_RESET_COUNT)->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//******************************************************************************************************
void CStartStopParticleSystem::OnStartSystem() 
{
	UpdateData();
	if (!_Running)
	{	
		_Running = true;
		_SystemInitialPos.copySystemInitialPos(_ParticleDlg->getCurrPS() );	
		// enable the system to take the right date from the scene	
		_ParticleDlg->getCurrPSModel()->enableAutoGetEllapsedTime(true);		
		_ParticleDlg->getCurrPSModel()->enableDisplayTools(m_DisplayHelpers != FALSE);
		_ParticleDlg->getCurrPS()->setSystemDate(0.f);
		_ParticleDlg->getCurrPS()->reactivateSound();
		if (_ParticleDlg->getCurrPS()->getAutoCountFlag())
		{
			if (_ResetAutoCount)
			{
				// reset particle size arrays
				_ParticleDlg->getCurrPS()->matchArraySize();
				_ResetAutoCount = false;
				GetDlgItem(IDC_RESET_COUNT)->EnableWindow(TRUE);
			}
		}		
	}
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{
		// start animation for the scene too
		if (_AnimationDLG)
		{
			_AnimationDLG->Playing = true;
		}
	}
	_ParticleDlg->getCurrPSModel()->enableAutoGetEllapsedTime(true);
	_Paused = false;
	_ParticleDlg->ParticleTreeCtrl->suppressLocatedInstanceNbItem(0);
	m_StartPicture.EnableWindow(FALSE);
	m_StopPicture.EnableWindow(TRUE);
	m_PausePicture.EnableWindow(TRUE);
	UpdateData(FALSE);
	NL3D::CParticleSystem *ps = _SystemInitialPos.getPS();
	if (ps)
	{
		ps->reactivateSound();		
	}	
	GetDlgItem(IDC_DISPLAY_HELPERS)->EnableWindow(TRUE);
	
}

//******************************************************************************************************
void CStartStopParticleSystem::OnStopSystem() 
{		
	UpdateData();
	_Running = false;
	_Paused = false;
	_SystemInitialPos.restoreSystem();

	_ParticleDlg->ParticleTreeCtrl->rebuildLocatedInstance();
	_ParticleDlg->getCurrPSModel()->enableAutoGetEllapsedTime(false);	
	_ParticleDlg->getCurrPSModel()->setEllapsedTime(0.f);
	_ParticleDlg->getCurrPSModel()->enableDisplayTools(true);

	m_StartPicture.EnableWindow(TRUE);
	m_StopPicture.EnableWindow(FALSE);
	m_PausePicture.EnableWindow(FALSE);

	UpdateData(FALSE);
	// go through all the ps to disable sounds
	
	NL3D::CParticleSystem *ps = _SystemInitialPos.getPS();
	if (ps)
	{
		ps->stopSound();
	}
	GetDlgItem(IDC_DISPLAY_HELPERS)->EnableWindow(FALSE);	
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{
		// start animation for the scene too
		if (_AnimationDLG)
		{
			_AnimationDLG->Playing = false;
			_AnimationDLG->setCurrentFrame(_AnimationDLG->Start);			
		}
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::OnPause() 
{
	nlassert(_Running);
	_ParticleDlg->getCurrPSModel()->enableAutoGetEllapsedTime(false);		
	_ParticleDlg->getCurrPSModel()->setEllapsedTime(0.f); // pause
	m_StartPicture.EnableWindow(TRUE);
	m_StopPicture.EnableWindow(TRUE);
	m_PausePicture.EnableWindow(FALSE);
	_Paused = true;
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{
		// start animation for the scene too
		if (_AnimationDLG)
		{
			_AnimationDLG->Playing = false;
		}
	}
	UpdateData(FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::toggle()
{
	if (_Running) stop();
	else start();
}

//******************************************************************************************************
void CStartStopParticleSystem::start()
{
	if (!_Running)
	{
		OnStartSystem();
	}
}

//******************************************************************************************************
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
//******************************************************************************************************
void CPSInitialPos::reset()
{
	_InitInfoVect.clear();
	_RotScaleInfoVect.clear();
	_InitialSizeVect.clear();
}

//******************************************************************************************************
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

//******************************************************************************************************
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

//******************************************************************************************************
void CPSInitialPos::removeLocatedBindable(NL3D::CPSLocatedBindable *lb)
{
	CRemoveLocatedBindableFromRotScaleInfoVectPred p;
	p.LB = lb;
	_RotScaleInfoVect.erase(std::remove_if(_RotScaleInfoVect.begin(), _RotScaleInfoVect.end(), p), _RotScaleInfoVect.end() );
}

//******************************************************************************************************
// reinitialize the system with its initial instances positions
void CPSInitialPos::restoreSystem()
{
	nlassert(_PS); // no system has been memorized yet
	_PS->stopSound();
	// delete all the instance of the system
	NL3D::CPSEmitter::setBypassEmitOnDeath(true);
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
	NL3D::CPSEmitter::setBypassEmitOnDeath(false);

	// recreate the initial number of instances

	for (TInitialLocatedSizeVect ::iterator itSize = _InitialSizeVect.begin(); itSize != _InitialSizeVect.end(); ++itSize)
	{
	//	nlassert(itSize->first->getSize() == 0)
		for (uint l = 0; l < itSize->second; ++l)
		{
			itSize->first->newElement(NLMISC::CVector::Null, NLMISC::CVector::Null, NULL, 0, itSize->first->getMatrixMode(), 0.f);
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

//******************************************************************************************************
void CStartStopParticleSystem::setSpeedSliderValue(float value)
{
	m_SpeedSliderPos = (int) (value * 100);
	UpdateData(FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnReleasedcaptureAnimSpeed(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateData();
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_ANIM_SPEED);
	_ParticleDlg->getCurrPSModel()->setEllapsedTimeRatio(m_SpeedSliderPos * 0.01f);
	*pResult = 0;		
}

//******************************************************************************************************
void CStartStopParticleSystem::reset()
{ 
	_SystemInitialPos.reset(); 
	setSpeedSliderValue(1.f);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnDisplayHelpers() 
{
	UpdateData();
	_ParticleDlg->getCurrPSModel()->enableDisplayTools(!_Running || m_DisplayHelpers != FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnEnableAutoCount() 
{		
	bool autoCount = ((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->GetCheck() != 0;
	if (autoCount)
	{	
		CString caption;
		CString mess;
		caption.LoadString(IDS_PARTICLE_SYSTEM_EDITOR);
		mess.LoadString(IDS_ENABLE_AUTOCOUNT);
		if (MessageBox((LPCTSTR) mess, (LPCTSTR) caption, MB_OKCANCEL) != IDOK)
		{
			((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->SetCheck(0);
			return;
		}
	}
	enableAutoCount(autoCount);	
	resetAutoCount();
	if (_Running)
	{
		OnStopSystem();
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::resetAutoCount(bool reset /* = true */)
{
	_ResetAutoCount = reset;
	GetDlgItem(IDC_RESET_COUNT)->EnableWindow(_ParticleDlg->getCurrPS()->getAutoCountFlag() && !reset ? TRUE : FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnResetCount() 
{
	resetAutoCount();
	if (_Running)
	{
		OnStopSystem();
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::enableAutoCount(bool enable)
{	
	((CButton *) GetDlgItem(IDC_ENABLE_AUTO_COUNT))->SetCheck(enable ? 1 : 0);
	_ParticleDlg->getCurrPS()->setAutoCountFlag(enable);
	GetDlgItem(IDC_RESET_COUNT)->EnableWindow(enable && !_ResetAutoCount ? TRUE : FALSE);
	CLocatedProperties *lp =dynamic_cast<CLocatedProperties *>(_ParticleDlg->getRightPane());
	if (lp)
	{
		lp->getParticleCountDlg()->EnableWindow(enable ? FALSE : TRUE);
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::go()
{	
	NL3D::CParticleSystem *ps = _ParticleDlg->getCurrPS();
	sint currNumParticles = (sint) ps->getCurrNumParticles();
	sint maxNumParticles = (sint) ps->getMaxNumParticles();
	// if linked with scene animation, restart if animation ends
	if (m_LinkPlayToScenePlay) // is scene animation subordinated to the fx animation
	{		
		// start animation for the scene too
		if (_AnimationDLG && _Running)
		{
			if (_LastSceneAnimFrame > _AnimationDLG->CurrentFrame) // did animation restart ?
			{			
				// restart system
				_SystemInitialPos.restoreSystem();
				_SystemInitialPos.copySystemInitialPos(ps);
				ps->setSystemDate(0.f);			
			}
			_LastSceneAnimFrame = _AnimationDLG->CurrentFrame;
		}
	}
	else
	if (_AutoRepeat) // auto repeat feature
	{
		if (_Running)
		{		
			if (ps->getSystemDate() > ps->evalDuration())
			{
				if (currNumParticles == 0)
				{ 
					// restart system
					_SystemInitialPos.restoreSystem();
					_SystemInitialPos.copySystemInitialPos(ps);			
					ps->setSystemDate(0.f);	
					_ParticleDlg->getCurrPS()->reactivateSound();
				}
			}
		}
	}
	
	// display number of particles		
	if (currNumParticles != _LastCurrNumParticles || maxNumParticles != _LastMaxNumParticles)
	{	
		CString numParts;	
		numParts.LoadString(IDS_NUM_PARTICLES);
		numParts += CString(NLMISC::toString("%d / %d",(int) currNumParticles, (int) maxNumParticles).c_str());
		GetDlgItem(IDC_NUM_PARTICLES)->SetWindowText((LPCTSTR) numParts);
		_LastCurrNumParticles = currNumParticles;
		_LastMaxNumParticles = maxNumParticles;
	}
	// display system date
	if (_ParticleDlg->getCurrPS()->getSystemDate() != _LastSystemDate)
	{
		_LastSystemDate = ps->getSystemDate();
		CString sysDate;	
		sysDate.LoadString(IDS_SYSTEM_DATE);
		sysDate += CString(NLMISC::toString("%.2f s",_LastSystemDate).c_str());
		GetDlgItem(IDC_SYSTEM_DATE)->SetWindowText((LPCTSTR) sysDate);
	}	
}

//******************************************************************************************************
void CStartStopParticleSystem::OnAutoRepeat() 
{
	_AutoRepeat = ((CButton *) GetDlgItem(IDC_AUTOREPEAT))->GetCheck() != 0;
}

//******************************************************************************************************
void CStartStopParticleSystem::OnLinkPlayToScenePlay() 
{	
	// There are 2 play buttons : - one to activate play for the scene animation.
	//                            - one to activate play for the fxs.
	//  When this method is called, the 'play' button of the scene is controlled by the 'play' button of the particle editor
	//  and thus is not accessible.

	UpdateData(TRUE);
	stop();
	_AnimationDLG->setCurrentFrame(_AnimationDLG->Start);
	_LastSceneAnimFrame = _AnimationDLG->Start;
	_AnimationDLG->Playing = false;
	_AnimationDLG->Loop = TRUE;
	_AnimationDLG->UpdateData(FALSE);
	_AnimationDLG->EnableWindow(!m_LinkPlayToScenePlay);	
	GetDlgItem(IDC_ANIM_SPEED)->EnableWindow(!m_LinkPlayToScenePlay);
	if (m_LinkPlayToScenePlay) 
	{
		m_SpeedSliderPos = 100;
		_ParticleDlg->getCurrPSModel()->setEllapsedTimeRatio(1.f);
	}
	UpdateData(FALSE);
}

//******************************************************************************************************
void CStartStopParticleSystem::OnLinkToSkeleton() 
{
	CObjectViewer *ov = _ParticleDlg->getObjectViewer();
	if (!ov->isSkeletonPresent())
	{
		CString caption;
		CString mess;
		caption.LoadString(IDS_ERROR);
		mess.LoadString(IDS_NO_SKELETON_IN_SCENE);
		MessageBox((LPCTSTR) mess, (LPCTSTR) caption, MB_ICONEXCLAMATION);
		return;
	}
	CString chooseBoneForPS;
	chooseBoneForPS.LoadString(IDS_CHOOSE_BONE_FOR_PS);
	NL3D::CSkeletonModel *skel;
	uint boneIndex;
	if (ov->chooseBone((LPCTSTR) chooseBoneForPS, skel, boneIndex))
	{
		_ParticleDlg->stickPSToSkeleton(skel, boneIndex);
	}
}

//******************************************************************************************************
void CStartStopParticleSystem::OnUnlinkFromSkeleton() 
{
	if (!_ParticleDlg->isPSStickedToSkeleton())
	{
		CString caption;
		CString mess;
		caption.LoadString(IDS_WARNING);
		mess.LoadString(IDS_NOT_STICKED_TO_SKELETON);
		return;
	}
	_ParticleDlg->unstickPSFromSkeleton();
}
