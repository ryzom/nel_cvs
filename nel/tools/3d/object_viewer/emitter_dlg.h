/** \file emitter_dlg.h
 * a dialog to tune emitter properties in a particle system
 *
 * $Id: emitter_dlg.h,v 1.9 2003/04/07 12:43:20 vizerie Exp $
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

#if !defined(AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_)
#define AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "3d/ps_particle.h"
#include "3d/ps_emitter.h"

namespace NL3D
{
	class NL3D::CPSEmitter;
}

#include "attrib_dlg.h"
#include "dialog_stack.h"

/////////////////////////////////////////////////////////////////////////////
// CEmitterDlg dialog

class CEmitterDlg : public CDialog, public CDialogStack
{
// Construction
public:
	CEmitterDlg(NL3D::CPSEmitter *emitter);   // standard constructor

	~CEmitterDlg();


	void init(CWnd* pParent = NULL);


// Dialog Data
	//{{AFX_DATA(CEmitterDlg)
	enum { IDD = IDD_EMITTER_DIALOG };
	CComboBox	m_EmissionTypeCtrl;
	CComboBox	m_EmittedTypeCtrl;
	BOOL	m_UseSpeedBasis;
	BOOL	m_ConvertSpeedVectorFromEmitterBasis;
	BOOL	m_ConsistentEmission;
	BOOL	m_BypassAutoLOD;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmitterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// the emitter being edited
	NL3D::CPSEmitter	 *_Emitter;	
	CAttribDlgFloat		 *_PeriodDlg;
	CAttribDlgUInt		 *_GenNbDlg;
	CAttribDlgFloat		 *_StrenghtModulateDlg;
	CEditableRangeFloat  *_SpeedInheritanceFactorDlg;
	CEditableRangeFloat  *_DelayedEmissionDlg;
	CEditableRangeUInt   *_MaxEmissionCountDlg;

	// Generated message map functions
	//{{AFX_MSG(CEmitterDlg)
	afx_msg void OnSelchangeEmittedType();
	afx_msg void OnSelchangeTypeOfEmission();
	virtual BOOL OnInitDialog();
	afx_msg void OnUseSpeedBasis();	
	afx_msg void OnConvertSpeedVectorFromEmitterBasis();
	afx_msg void OnConsistentEmission();
	afx_msg void OnBypassAutoLOD();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	/// disable / enable the period edition dialog if it is needed
	void updatePeriodDlg(void);

	//////////////////////////////////////////////
	//	WRAPPERS TO EDIT THE EMITTER PROPERTIES //
	//////////////////////////////////////////////



		////////////////////////
		// period of emission //
		////////////////////////

			struct CPeriodWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSEmitter *E;
			   float get(void) const { return E->getPeriod(); }
			   void set(const float &v) { E->setPeriod(v); }
			   scheme_type *getScheme(void) const { return E->getPeriodScheme(); }
			   void setScheme(scheme_type *s) { E->setPeriodScheme(s); }
			} _PeriodWrapper;

		//////////////////////////////////////////////
		// number of particle to generate each time //
		//////////////////////////////////////////////

			struct CGenNbWrapper : public IPSWrapperUInt, IPSSchemeWrapperUInt 
			{
			   NL3D::CPSEmitter *E;
			   uint32 get(void) const { return E->getGenNb(); }
			   void set(const uint32 &v) { E->setGenNb(v); }
			   scheme_type *getScheme(void) const { return E->getGenNbScheme(); }
			   void setScheme(scheme_type *s) { E->setGenNbScheme(s); }
			} _GenNbWrapper;

		////////////////////////////////////////////////////////
		// wrappers to emitters that have strenght modulation //
		////////////////////////////////////////////////////////

			struct CModulateStrenghtWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSModulatedEmitter *E;
			   float get(void) const { return E->getEmitteeSpeed(); }
			   void set(const float &v) { E->setEmitteeSpeed(v); }
			   scheme_type *getScheme(void) const { return E->getEmitteeSpeedScheme(); }
			   void setScheme(scheme_type *s) { E->setEmitteeSpeedScheme(s); }
			} _ModulatedStrenghtWrapper;

		//////////////////////////////////////////////////
		// wrappers to set the speed inheritance factor //
		//////////////////////////////////////////////////

			struct CSpeedInheritanceFactorWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSEmitter *E;
			   float get(void) const { return E->getSpeedInheritanceFactor(); }
			   void set(const float &f) { E->setSpeedInheritanceFactor(f); }	
			} _SpeedInheritanceFactorWrapper;

		////////////////////////////////////////////////
		// wrappers to tune the direction of emitters //
		////////////////////////////////////////////////

			struct CDirectionWrapper : public IPSWrapper<NLMISC::CVector>
			{
			   NL3D::CPSDirection *E;
			   NLMISC::CVector get(void) const { return E->getDir(); }
			   void set(const NLMISC::CVector &d){ E->setDir(d); }	
			} _DirectionWrapper;

		//////////////////////////////////////////////
		// wrapper to tune the radius of an emitter //
		//////////////////////////////////////////////
		
			struct CConicEmitterRadiusWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSEmitterConic *E;
			   float get(void) const { return E->getRadius(); }
			   void set(const float &f) { E->setRadius(f); }	
			} _ConicEmitterRadiusWrapper;


		//////////////////////////////////////////////
		// wrapper to tune delayed emission		    //
		//////////////////////////////////////////////
		
			struct CDelayedEmissionWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSEmitter *E;
			   float get(void) const { return E->getEmitDelay(); }
			   void set(const float &f) { E->setEmitDelay(f); }	
			} _DelayedEmissionWrapper;


		////////////////////////////////////////////////
		// wrapper to tune max number of emissions	  //
		////////////////////////////////////////////////
		
			struct CMaxEmissionCountWrapper : public IPSWrapperUInt
			{
			   NL3D::CPSEmitter *E;
			   uint32 get(void) const { return E->getMaxEmissionCount(); }
			   void set(const uint32 &count) { E->setMaxEmissionCount((uint8) count); }	
			} _MaxEmissionCountWrapper;


	// contains pointers to the located
	std::vector<NL3D::CPSLocated *> _LocatedList;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTER_DLG_H__7D6DB229_8E72_4A60_BD03_8A3EF3F506CF__INCLUDED_)
