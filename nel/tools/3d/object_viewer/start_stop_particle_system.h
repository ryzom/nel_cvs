/** \file start_stop_particle_system.h
 * a pop-up dialog that allow to start and stop a particle system
 *
 * $Id: start_stop_particle_system.h,v 1.12 2003/10/07 12:33:51 vizerie Exp $
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
 * MA 02
*/

#if !defined(AFX_STAR_STOP_PARTICLE_SYSTEM_H__291E2631_42A1_4598_86AB_D6CE30C64DAF__INCLUDED_)
#define AFX_STAR_STOP_PARTICLE_SYSTEM_H__291E2631_42A1_4598_86AB_D6CE30C64DAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "object_viewer.h"
#include "animation_dlg.h"


#include "nel/misc/vector.h"


namespace NL3D
{
	class CParticleSystem;
	class CPSLocated;
	class CPSLocatedBindable;
	struct IPSMover;
}




class CParticleDlg;
class CAnimationDlg;


/** this class helps to copy the position of initial instances in a particle
 *  system. This enable a system to run, and have its parameter modified.
 *  When the user press stop, he will find the system at t = 0, with the new parameters
 */
class CPSInitialPos
{
public:

	CPSInitialPos() : _PS(NULL) {}

	// construct this by copying the datas of the system
	void copySystemInitialPos(NL3D::CParticleSystem *ps);

	/** reinitialize the system with its initial instances positions
	  * Works only once per copySystemInitialPos() call
	  */	  
	void restoreSystem();

	/// send back true when bbox display is enabled
	bool isBBoxDisplayEnabled();
	
	/// update data when a located in a particle system has been removed	
	void removeLocated(NL3D::CPSLocated *loc);

	/// update data when a located bindable in a particle system has been removed	
	void removeLocatedBindable(NL3D::CPSLocatedBindable *lb);	

	// reset all initial infos
	void reset();

	

	// initial position and speed of a located instance in a particle system
	struct CInitPSInstanceInfo
	{	
		uint32 Index;
		NL3D::CPSLocated *Loc;
		NLMISC::CVector Speed;
		NLMISC::CVector Pos;		
	};

	// rotation and scale of an element
	struct CRotScaleInfo
	{	
		uint32 Index;
		NL3D::CPSLocated *Loc;
		NL3D::CPSLocatedBindable *LB;
		NL3D::IPSMover *Psm;
		NLMISC::CMatrix Rot;
		NLMISC::CVector Scale;
	};

	NL3D::CParticleSystem *getPS() { return _PS; }
	const NL3D::CParticleSystem *getPS() const { return _PS; }


protected:




	typedef std::vector<CInitPSInstanceInfo> TInitInfoVect;
	typedef std::vector<CRotScaleInfo> TRotScaleInfoVect;
	typedef std::vector< std::pair<NL3D::CPSLocated *, uint32> > TInitialLocatedSizeVect;

	TInitInfoVect _InitInfoVect;
	TRotScaleInfoVect _RotScaleInfoVect;

	// initial number of instances for each located
	TInitialLocatedSizeVect  _InitialSizeVect;

	NL3D::CParticleSystem *_PS;
};




/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem dialog

class CStartStopParticleSystem : public CDialog, public CObjectViewer::IMainLoopCallBack
{
// Construction
public:
	CStartStopParticleSystem(CParticleDlg *particleDlg, CAnimationDlg *animationDLG);   // standard constructor
	// dtor
	~CStartStopParticleSystem();	
	/// return true if a system is being played
	bool isRunning() const { return _Running; }
	/** Return true if a system is paused.
	  * Must call only if running
	  */
	bool isPaused() const 
	{ 
		nlassert(isRunning());
		return _Paused;
	}
	/// force the system to stop
	void stop();
	/// force the system to start
	void start();	
	/// toggle between start and stop
	void toggle();
	/** call this to say that a located has been removed
	 *  When the system starts, the initial state is memorised
	 *  This must be called to ensure that the system won't try to restore instance of a removed located
	 */
	void removeLocated(NL3D::CPSLocated *loc)
	{
		if (_Running)
		{
			_SystemInitialPos.removeLocated(loc);
		}
	}
	/** call this to say that a located bindable has been removed
	 *  When the system starts, the initial state is memorised
	 *  This must be called to ensure that the system won't try to restore instance of a removed located bindable
	 */
	void removeLocatedBindable(NL3D::CPSLocatedBindable *lb)
	{
		if (_Running)
		{
			_SystemInitialPos.removeLocatedBindable(lb);
		}
	}
	/// This remove any memorized instance from the system
	void reset();
	/// return true when the display bbox button is pressed...
	bool isBBoxDisplayEnabled();
	// enable / disbale auto-count
	void enableAutoCount(bool enable);	
	// reset the autocount the next time the system will be started
	void resetAutoCount(bool reset = true);
	bool getResetFlag() const { return _ResetAutoCount; }		
// Dialog Data
	//{{AFX_DATA(CStartStopParticleSystem)
	enum { IDD = IDD_PARTICLE_SYSTEM_START_STOP };
	CButton	m_PausePicture;
	CButton	m_StopPicture;
	CButton	m_StartPicture;
	BOOL	m_DisplayBBox;	
	int		m_SpeedSliderPos;
	BOOL	m_DisplayHelpers;
	BOOL	m_LinkPlayToScenePlay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStartStopParticleSystem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:

	// Generated message map functions
	//{{AFX_MSG(CStartStopParticleSystem)
	virtual BOOL OnInitDialog();
	afx_msg void OnStartSystem();
	afx_msg void OnStopSystem();
	afx_msg void OnPause();
	afx_msg void OnReleasedcaptureAnimSpeed(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDisplayHelpers();
	afx_msg void OnEnableAutoCount();
	afx_msg void OnResetCount();
	afx_msg void OnAutoRepeat();
	afx_msg void OnLinkPlayToScenePlay();
	afx_msg void OnLinkToSkeleton();
	afx_msg void OnUnlinkFromSkeleton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// return true if a system is being played
	bool _Running;
	// true if the system is in pause mode
	bool _Paused;
	// need the auto-count to be rested
	bool _ResetAutoCount;
	// Last number of particle that was displayed (keep this to avoid flickering)
	sint _LastCurrNumParticles;
	sint _LastMaxNumParticles;
	// last displayed date for the system
	float _LastSystemDate;
	//
	bool _AutoRepeat;
	//
	float _LastSceneAnimFrame;

	// the dialog that own this dialog
	CParticleDlg  *_ParticleDlg;
	CAnimationDlg *_AnimationDLG;	

	CPSInitialPos _SystemInitialPos;

private:
	void setSpeedSliderValue(float value);
	// From CObjectViewer::IMainLoopCallBack : update display of number of particles
	virtual void go();	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STAR_STOP_PARTICLE_SYSTEM_H__291E2631_42A1_4598_86AB_D6CE30C64DAF__INCLUDED_)
