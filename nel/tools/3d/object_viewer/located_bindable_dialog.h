/** \file located_bindable_dialog.h
 * a dialog for located bindable properties (particles ...)
 *
 * $Id: located_bindable_dialog.h,v 1.6 2001/06/25 13:26:50 vizerie Exp $
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

#if !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
#define AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

namespace NL3D
{
	class CPSLocatedBindable ;
}



#include "ps_wrapper.h"
#include "nel/misc/rgba.h"
#include "3d/texture.h"
#include "3d/ps_plane_basis.h"


#include "dialog_stack.h"

using NLMISC::CRGBA ;

// located_bindable_dialog.h : header file
//




/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog

class CLocatedBindableDialog : public CDialog, CDialogStack
{
// Construction
public:
	// create this dialog to edit the given bindable
	CLocatedBindableDialog(NL3D::CPSLocatedBindable *bindable);   // standard constructor


	// init the dialog as a child of the given wnd
	void init(CWnd* pParent = NULL) ;
// Dialog Data
	//{{AFX_DATA(CLocatedBindableDialog)
	enum { IDD = IDD_LOCATED_BINDABLE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocatedBindableDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL




// Implementation
protected:

	NL3D::CPSLocatedBindable *_Bindable ;



	// Generated message map functions
	//{{AFX_MSG(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	//////////////////////////////////////////////
	// wrappers to various element of bindables //
	//////////////////////////////////////////////

		//////////
		// size //
		//////////
			struct tagSizeWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
			{
			   NL3D::CPSSizedParticle *S ;
			   float get(void) const { return S->getSize() ; }
			   void set(const float &v) { S->setSize(v) ; }
			   scheme_type *getScheme(void) const { return S->getSizeScheme() ; }
			   void setScheme(scheme_type *s) { S->setSizeScheme(s) ; }
			} _SizeWrapper ;
			
		///////////
		// color //
		///////////
			struct tagColorWrapper : public IPSWrapperRGBA, IPSSchemeWrapperRGBA
			{
			   NL3D::CPSColoredParticle *S ;
			   CRGBA get(void) const { return S->getColor() ; }
			   void set(const CRGBA &v) { S->setColor(v) ; }
			   scheme_type *getScheme(void) const { return S->getColorScheme() ; }
			   void setScheme(scheme_type *s) { S->setColorScheme(s) ; }
			} _ColorWrapper ;
			
		//////////////
		// angle 2D //
		//////////////
			struct tagAngle2DWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
			{
			   NL3D::CPSRotated2DParticle *S ;
			   float get(void) const { return S->getAngle2D() ; }
			   void set(const float &v) { S->setAngle2D(v) ; }
			   scheme_type *getScheme(void) const { return S->getAngle2DScheme() ; }
			   void setScheme(scheme_type *s) { S->setAngle2DScheme(s) ; }
			} _Angle2DWrapper ;

		/////////////////
		// plane basis //
		/////////////////
			struct tagPlaneBasisWrapper : public IPSWrapper<NL3D::CPlaneBasis>, IPSSchemeWrapper<NL3D::CPlaneBasis>
			{
			   NL3D::CPSRotated3DPlaneParticle *S ;
			   NL3D::CPlaneBasis get(void) const { return S->getPlaneBasis() ; }
			   void set(const NL3D::CPlaneBasis &p) { S->setPlaneBasis(p) ; }
			   scheme_type *getScheme(void) const { return S->getPlaneBasisScheme() ; }
			   void setScheme(scheme_type *s) { S->setPlaneBasisScheme(s) ; }
			} _PlaneBasisWrapper ;


		///////////////
		//   texture //
		///////////////
			
			struct tagTextureWrapper : public IPSWrapperTexture
			{
				NL3D::CPSTexturedParticle *P ;
				NL3D::ITexture *get(void)  { return P->getTexture() ; }
				void set(NL3D::ITexture *t) { P->setTexture(t) ; }
			} _TextureWrapper ;

		///////////////////////
		// motion blur coeff //
		///////////////////////

			struct tagMotionBlurCoeffWrapper : public IPSWrapperFloat
			{
				NL3D::CPSFaceLookAt *P ;
			   float get(void) const { return P->getMotionBlurCoeff() ; }
			   void set(const float &v) { P->setMotionBlurCoeff(v) ; }
			}  _MotionBlurCoeffWrapper ;

			struct tagMotionBlurThresholdWrapper : public IPSWrapperFloat
			{
				NL3D::CPSFaceLookAt *P ;
			   float get(void) const { return P->getMotionBlurThreshold() ; }
			   void set(const float &v) { P->setMotionBlurThreshold(v) ; }
			}  _MotionBlurThresholdWrapper ;

		///////////////
		// fanlight  //
		///////////////
			struct tagFanLightWrapper : public IPSWrapperUInt
			{
				NL3D::CPSFanLight *P ;
			  uint32 get(void) const { return P->getNbFans() ; }
			   void set(const uint32 &v) { P->setNbFans(v) ; }
			}  _FanLightWrapper ;
			struct tagFanLightPhase : public IPSWrapperFloat
			{
				NL3D::CPSFanLight *P ;
				float get(void) const { return P->getPhaseSpeed() ; }
			    void set(const float &v) { P->setPhaseSpeed(v) ; }
			}  _FanLightPhaseWrapper ;

		///////////////////////
		// ribbon / tail dot //
		///////////////////////
			
			struct tagTailParticleWrapper : public IPSWrapperUInt
			{
				NL3D::CPSTailParticle *P ;
			    uint32 get(void) const { return P->getTailNbSeg() ; }
			    void set(const uint32 &v) { P->setTailNbSeg(v) ; }
			}  _TailParticleWrapper ;
		
		/////////////////////////////
		//		shockwave          //
		/////////////////////////////
	
			struct tagRadiusCutWrapper : public IPSWrapperFloat
			{
			   NL3D::CPSShockWave *S ;
			   float get(void) const { return S->getRadiusCut() ; }
			   void set(const float &v) { S->setRadiusCut(v) ; }
			} _RadiusCutWrapper ;

			struct tagShockWaveNbSegWrapper : public IPSWrapperUInt
			{
			   NL3D::CPSShockWave *S ;
			   uint32 get(void) const { return S->getNbSegs() ; }
			   void set(const uint32 &v) { S->setNbSegs(v) ; }
			} _ShockWaveNbSegWrapper ;





};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATED_BINDABLE_DIALOG_H__C715DCAB_3F07_4777_96DA_61AE2E420B09__INCLUDED_)
