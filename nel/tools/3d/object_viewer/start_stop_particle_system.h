/** \file start_stop_particle_system.h
 * a pop-up dialog that allow to start and stop a particle system
 *
 * $Id: start_stop_particle_system.h,v 1.9 2002/04/25 08:31:35 vizerie Exp $
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




#include "nel/misc/vector.h"


namespace NL3D
{
	class CParticleSystem;
	class CPSLocated;
	class CPSLocatedBindable;
	struct IPSMover;
}




class CParticleDlg;


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

class CStartStopParticleSystem : public CDialog
{
// Construction
public:
	CStartStopParticleSystem(CParticleDlg *particleDlg);   // standard constructor


	/// return true if a system is being played
	bool isRunning(void) const { return _Running; }

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
	void reset() { _SystemInitialPos.reset(); }



	/// return true when the display bbox button is pressed...
	bool isBBoxDisplayEnabled();

// Dialog Data
	//{{AFX_DATA(CStartStopParticleSystem)
	enum { IDD = IDD_PARTICLE_SYSTEM_START_STOP };
	CButton	m_StopPicture;
	CButton	m_StartPicture;
	BOOL	m_DisplayBBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStartStopParticleSystem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStartStopParticleSystem)
	virtual BOOL OnInitDialog();
	afx_msg void OnStartSystem();
	afx_msg void OnStopSystem();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// return true if a system is being played
	bool _Running;

	// the dialog that own this dialog
	CParticleDlg *_ParticleDlg;

	CPSInitialPos _SystemInitialPos;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STAR_STOP_PARTICLE_SYSTEM_H__291E2631_42A1_4598_86AB_D6CE30C64DAF__INCLUDED_)
