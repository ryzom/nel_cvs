/** \file located_properties.h
 * a dialog that allow to tune a located properties. In a aprticle system, a located is an object that has :
 *  - a position
 *  - a mass
 *  - a speed vector
 *  - a lifetime
 *
 * $Id: located_properties.h,v 1.5 2001/06/27 16:48:56 vizerie Exp $
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


#if !defined(AFX_LOCATED_PROPERTIES_H__772D6C3B_6CFD_47B5_A132_A8D2352EACF9__INCLUDED_)
#define AFX_LOCATED_PROPERTIES_H__772D6C3B_6CFD_47B5_A132_A8D2352EACF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// located_properties.h : header file
//



#include "editable_range.h"
#include "particle_tree_ctrl.h"

namespace  NL3D
{
	class CPSLocated ;
} ; 


/////////////////////////////////////////////////////////////////////////////
// CLocatedProperties dialog

class CLocatedProperties : public CDialog
{
// Construction
public:
	CLocatedProperties(NL3D::CPSLocated *loc, CParticleDlg *pdlg);   // standard constructor

	~CLocatedProperties() ;

	void init(uint32 x, uint32 y) ;
// Dialog Data
	//{{AFX_DATA(CLocatedProperties)
	enum { IDD = IDD_LOCATED_PROPERTIES };
	CStatic	m_MaxNbParticles;
	CStatic	m_MassMaxPos;
	CStatic	m_MassMinPos;
	CStatic	m_LifeMaxPos;
	CStatic	m_LifeMinPos;
	BOOL	m_LimitedLifeTime;
	BOOL	m_SystemBasis;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocatedProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CEditableRangeFloat *_MinMass, *_MaxMass ;
	CEditableRangeFloat *_MinLife, *_MaxLife ;
	CEditableRangeUInt *_MaxNbParticles ;
	CEditableRangeUInt *_SkipFramesDlg ;
	
	 

	CParticleDlg *_ParticleDlg ;

	/// some wrappers used to read / write value from / to the particle system

		
		struct CMinMassWrapper : public IPSWrapperFloat
		{
		   NL3D::CPSLocated *Located ;
		   float get(void) const { return Located->getMinMass() ; }
		   void set(const float &v) { Located->setMinMass(v) ; }
		} _MinMassWrapper ;

		struct CMaxMassWrapper: public IPSWrapperFloat
		{
			NL3D::CPSLocated *Located ;
			float get(void) const { return Located->getMaxMass() ; }
			void set(const float &v) { Located->setMaxMass(v) ; }
		} _MaxMassWrapper ;


		struct CMaxNbParticlesWrapper : public IPSWrapperUInt
		{
			NL3D::CPSLocated *Located ;
			CParticleTreeCtrl *TreeCtrl ;
			uint32 get(void) const { return Located->getMaxSize() ; }
			void set(const uint32 &v) 
			{ 
				// if the max new size is lower than the current number of instance, we must suppress item
				// in the the CParticleTreeCtrl

				if (v < Located->getSize())
				{
					TreeCtrl->suppressLocatedInstanceNbItem(v) ;
				}

				Located->resize(v) ; 
			}
		} _MaxNbParticlesWrapper ;


		struct CMinLifeWrapper : public IPSWrapperFloat
		{
			NL3D::CPSLocated *Located ;
			float get(void) const { return Located->getMinLife() ; }
			void set(const float &v) { Located->setLifeTime(v, Located->getMaxLife()) ; }
		} _MinLifeWrapper ;

		
		struct CMaxLifeWrapper : public IPSWrapperFloat
		{
			NL3D::CPSLocated *Located ;
			float get(void) const { return Located->getMaxLife() ; }
			void set(const float &v) { Located->setLifeTime(Located->getMinLife(), v) ; }
		} _MaxLifeWrapper ;
					
				
		////////////////////////////////
		// wrapper for frame skipping //
		////////////////////////////////
		struct CSkipFrameWrapper : public IPSWrapperUInt
		{
			NL3D::CPSLocated *Located ;
			uint32 get(void) const { return Located->getFrameRate() ; }
			void set(const uint32 &value) { Located->setFrameRate(value) ; }
		} _SkipFrameRateWrapper ;



	// the located this dialog is editing

	NL3D::CPSLocated *_Located ;

	// Generated message map functions
	//{{AFX_MSG(CLocatedProperties)
	virtual BOOL OnInitDialog();
	afx_msg void OnLimitedLifeTime();
	afx_msg void OnSystemBasis();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATED_PROPERTIES_H__772D6C3B_6CFD_47B5_A132_A8D2352EACF9__INCLUDED_)
