/** \file start_stop_particle_system.h
 * <File description>
 *
 * $Id: start_stop_particle_system.h,v 1.2 2001/06/15 16:05:03 vizerie Exp $
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
	class CParticleSystem ;
	class CPSLocated ;
}




class CParticleDlg ;


/** this class helps to copy the position of initial instances in a particle
 *  system. This enable a system to run, and have its parameter modified.
 *  When the user press stop, he will find the system at t = 0, with the new parameters
 */
class CPSInitialPos
{
public:

	CPSInitialPos() : _PS(NULL) {}

	// construct this by copying the datas of the system
	void copySystemInitialPos(NL3D::CParticleSystem *ps) ;

	// reinitialize the system with its initial instances positions
	void restoreSystem() ;

protected:

	// initial position and speed of a located instance in a particle system
	struct CInitPSInstanceInfo
	{
		CInitPSInstanceInfo(const NLMISC::CVector &pos, const NLMISC::CVector &speed) : Pos(pos), Speed(speed) {}
		NLMISC::CVector Pos ;
		NLMISC::CVector Speed ;
	} ;

	typedef std::vector<CInitPSInstanceInfo> TInitInfoVect ;

	// initial infos of the system
	std::map<NL3D::CPSLocated *, TInitInfoVect> _StartInfos ;

	NL3D::CParticleSystem *_PS ;
} ;




/////////////////////////////////////////////////////////////////////////////
// CStartStopParticleSystem dialog

class CStartStopParticleSystem : public CDialog
{
// Construction
public:
	CStartStopParticleSystem(CParticleDlg *particleDlg);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStartStopParticleSystem)
	enum { IDD = IDD_PARTICLE_SYSTEM_START_STOP };
	CButton	m_StopPicture;
	CButton	m_StartPicture;
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

	
		// the dialog that own this dialog
	CParticleDlg *_ParticleDlg ;

	CPSInitialPos _SystemInitialPos ;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STAR_STOP_PARTICLE_SYSTEM_H__291E2631_42A1_4598_86AB_D6CE30C64DAF__INCLUDED_)
