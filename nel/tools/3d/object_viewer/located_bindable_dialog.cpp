/** \file located_bindable_dialog.cpp
 * a dialog for located bindable properties (particles ...)
 *
 * $Id: located_bindable_dialog.cpp,v 1.9 2001/07/04 17:17:52 vizerie Exp $
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

#include "std_afx.h"
#include "object_viewer.h"
#include "located_bindable_dialog.h"



#include "3d/ps_located.h"
#include "3d/ps_particle.h"
#include "3d/ps_force.h"
#include "3d/ps_emitter.h"
#include "3d/ps_zone.h"


#include "texture_chooser.h"
#include "attrib_dlg.h"
#include "precomputed_rotations_dlg.h"
#include "tail_particle_dlg.h"
#include "mesh_dlg.h"
#include "texture_anim_dlg.h"


using NL3D::CPSLocatedBindable ; 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif





/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog


CLocatedBindableDialog::CLocatedBindableDialog(NL3D::CPSLocatedBindable *bindable)
	: _Bindable(bindable)
{
	//{{AFX_DATA_INIT(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLocatedBindableDialog::init(CWnd* pParent)
{
	Create(IDD_LOCATED_BINDABLE, pParent) ;
	ShowWindow(SW_SHOW) ;

	uint yPos = 35 ;
	const uint xPos = 5 ;
	RECT rect ;

	// has the particle a material ?
	if (dynamic_cast<NL3D::CPSMaterial *>(_Bindable))
	{
		m_BlendingMode.SetCurSel((uint) (dynamic_cast<NL3D::CPSMaterial *>(_Bindable))->getBlendingMode() ) ;
	}
	else
	{
		m_BlendingMode.ShowWindow(SW_HIDE) ;
		GetDlgItem(IDC_BLENDING_MODE_STATIC)->ShowWindow(SW_HIDE) ;
	}

	if (dynamic_cast<NL3D::CPSParticle *>(_Bindable))
	{
		NL3D::CPSParticle *p = (NL3D::CPSParticle *) _Bindable ;

		// check support for color
		if (dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable))
		{
			
			CAttribDlgRGBA *ad = new CAttribDlgRGBA("PARTICLE_COLOR") ;
			pushWnd(ad) ;

			_ColorWrapper.S = dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable) ;		
			ad->setWrapper(&_ColorWrapper) ;			
			ad->setSchemeWrapper(&_ColorWrapper) ;

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_COLOR)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}

		// check support for size
		if (dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable))
		{
			
			CAttribDlgFloat *ad = new CAttribDlgFloat("PARTICLE_SIZE", 0.f, 10.f) ;
			pushWnd(ad) ;

			_SizeWrapper.S = dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable) ;
			ad->setWrapper(&_SizeWrapper) ;
			ad->setSchemeWrapper(&_SizeWrapper) ;
					
			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_SIZE)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}


		// check support for angle 2D
		if (dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable))
		{
			
			CAttribDlgFloat *ad = new CAttribDlgFloat("PARTICLE_ANGLE2D", 0.f, 256.f) ;
			pushWnd(ad) ;

			_Angle2DWrapper.S = dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable) ;
			ad->setWrapper(&_Angle2DWrapper) ;						
			ad->setSchemeWrapper(&_Angle2DWrapper) ;	

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_ANGLE)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}

		

		CAttribDlgPlaneBasis *pb = NULL ;

		// check support for plane basis
		if (dynamic_cast<NL3D::CPSRotated3DPlaneParticle *>(_Bindable))
		{
			pb = new CAttribDlgPlaneBasis("PARTICLE_PLANE_BASIS") ;
			pushWnd(pb) ;
			_PlaneBasisWrapper.S = dynamic_cast<NL3D::CPSRotated3DPlaneParticle *>(_Bindable) ;
			pb->setWrapper(&_PlaneBasisWrapper) ;
			pb->setSchemeWrapper(&_PlaneBasisWrapper) ;
			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BASIS)) ;
			pb->init(bmh, xPos, yPos, this) ;
			pb->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		
		}

		// check support for precomputed rotations
		if (dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable))
		{
			CPrecomputedRotationsDlg *pr = new CPrecomputedRotationsDlg(dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable)
																		, pb) ;
			pushWnd(pr) ;
			pr->init(this, xPos, yPos) ;
			pr->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}

		// if we're dealing with a face look at, motion blur can be tuned
		if (dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable))
		{

			CEditableRangeFloat *mbc = new CEditableRangeFloat(std::string("MOTION_BLUR_COEFF"), 0, 5) ;
			pushWnd(mbc) ;
			_MotionBlurCoeffWrapper.P = dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable) ;
			mbc->setWrapper(&_MotionBlurCoeffWrapper) ;
			mbc->init(xPos + 140, yPos, this) ;
			CStatic *s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Fake motion blur coeff.", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;


			mbc->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;

			mbc = new CEditableRangeFloat(std::string("MOTION_BLUR_THRESHOLD"), 0, 5) ;
			pushWnd(mbc) ;
			_MotionBlurThresholdWrapper.P = dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable) ;
			mbc->setWrapper(&_MotionBlurThresholdWrapper) ;
			mbc->init(xPos + 140, yPos, this) ;

			s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Fake motion blur threshold.", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;

			mbc->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;


			
		}

		// if we're dealing with a shockwave, we add dlg for the radius cut, and the number of segments
		if (dynamic_cast<NL3D::CPSShockWave *>(_Bindable))
		{

			CEditableRangeFloat *rc = new CEditableRangeFloat(std::string("RADIUS CUT"), 0, 1) ;
			pushWnd(rc) ;
			_RadiusCutWrapper.S = dynamic_cast<NL3D::CPSShockWave *>(_Bindable) ;
			rc->setWrapper(&_RadiusCutWrapper) ;
			rc->init(xPos + 140, yPos, this) ;
			CStatic *s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Radius cut.", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;


			rc->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;

			CEditableRangeUInt *snbs = new CEditableRangeUInt(std::string("SHOCK WAVE NB SEG"), 3, 24) ;
			pushWnd(snbs) ;
			_ShockWaveNbSegWrapper.S = dynamic_cast<NL3D::CPSShockWave *>(_Bindable) ;
			snbs->enableLowerBound(3, false) ;
			snbs->setWrapper(&_ShockWaveNbSegWrapper) ;
			snbs->init(xPos + 140, yPos, this) ;

			s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Nb segs", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;

			snbs->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
			
		}

		// fanlight

		if (dynamic_cast<NL3D::CPSFanLight *>(_Bindable))
		{
			CEditableRangeUInt *nbf = new CEditableRangeUInt(std::string("NB_FANS"), 3, 127) ;
			pushWnd(nbf) ;
			nbf->enableUpperBound(128, true) ;
			_FanLightWrapper.P = dynamic_cast<NL3D::CPSFanLight *>(_Bindable) ;
			nbf->setWrapper(&_FanLightWrapper) ;
			nbf->init(xPos + 140, yPos, this) ;
			CStatic *s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Nb fan lights :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;

			nbf->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;

			CEditableRangeFloat *nbfp = new CEditableRangeFloat(std::string("FAN_LIGHT_PHASE"), 0, 4.f) ;
			pushWnd(nbfp) ;			
			_FanLightPhaseWrapper.P = dynamic_cast<NL3D::CPSFanLight *>(_Bindable) ;
			nbfp->setWrapper(&_FanLightPhaseWrapper) ;
			nbfp->init(xPos + 140, yPos, this) ;
			s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Fan light speed :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;

			nbf->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;

		}


		// tail particle
		if (dynamic_cast<NL3D::CPSTailParticle *>(_Bindable))
		{
			CEditableRangeUInt *nbs = new CEditableRangeUInt(std::string("TAIL_NB_SEGS"), 1, 255) ;
			pushWnd(nbs) ;
			if (dynamic_cast<NL3D::CPSTailDot *>(_Bindable))
			{
				nbs->enableUpperBound(256, true) ;
			}
			nbs->enableLowerBound(0, true) ;
			_TailParticleWrapper.P = dynamic_cast<NL3D::CPSTailParticle *>(_Bindable) ;
			nbs->setWrapper(&_TailParticleWrapper) ;
			nbs->init(xPos + 140, yPos, this) ;
			CStatic *s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Nb segs :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;	
			
			nbs->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;

			CTailParticleDlg *tpd = new CTailParticleDlg(dynamic_cast<NL3D::CPSTailParticle *>(_Bindable)) ;
			pushWnd(tpd) ;
			tpd->init(this, xPos, yPos) ;
				
			tpd->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}


		// shape particle

		if (dynamic_cast<NL3D::CPSShapeParticle *>(_Bindable))
		{
			CMeshDlg *md = new CMeshDlg(dynamic_cast<NL3D::CPSShapeParticle *>(_Bindable)) ;
			md->init(this, xPos, yPos) ;
			md->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}


		// check support for animated texture
		if (dynamic_cast<NL3D::CPSTexturedParticle *>(_Bindable))
		{
			CTextureAnimDlg *td = new CTextureAnimDlg(dynamic_cast<NL3D::CPSTexturedParticle *>(_Bindable))  ;			
			pushWnd(td) ;
						
			td->init(xPos, yPos, this) ;
			td->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}

		// ribbon texture (doesn't support texture animation for now)
		if (dynamic_cast<NL3D::CPSRibbon *>(_Bindable))
		{
			_RibbonTextureWrapper.R = dynamic_cast<NL3D::CPSRibbon *>(_Bindable) ;
			CTextureChooser *tc = new CTextureChooser  ;			
			tc->enableRemoveButton() ;
			tc->setWrapper(&_RibbonTextureWrapper) ;
			pushWnd(tc) ;
						
			tc->init(xPos, yPos, this) ;
			tc->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;

			// add dialog for uv tuning with ribbon
			CEditableRangeFloat *uvd = new CEditableRangeFloat(std::string("RIBBON UFACTOR"), 0, 5) ;
			pushWnd(uvd) ;
			_RibbonUFactorWrapper.R = dynamic_cast<NL3D::CPSRibbon *>(_Bindable) ;
			uvd->setWrapper(&_RibbonUFactorWrapper) ;
			uvd->init(xPos + 140, yPos, this) ;
			CStatic *s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Texture U factor :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;
			uvd->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;

			uvd = new CEditableRangeFloat(std::string("RIBBON VFACTOR"), 0, 5) ;
			pushWnd(uvd) ;
			_RibbonVFactorWrapper.R = dynamic_cast<NL3D::CPSRibbon *>(_Bindable) ;
			uvd->setWrapper(&_RibbonVFactorWrapper) ;
			uvd->init(xPos + 140, yPos, this) ;
			s = new CStatic ;			
			pushWnd(s) ;
			s->Create("Texture V factor :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this) ;
			s->ShowWindow(SW_SHOW) ;
			uvd->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;

		}

	}	
}


void CLocatedBindableDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedBindableDialog)
	DDX_Control(pDX, IDC_BLENDING_MODE, m_BlendingMode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedBindableDialog, CDialog)
	//{{AFX_MSG_MAP(CLocatedBindableDialog)
	ON_CBN_SELCHANGE(IDC_BLENDING_MODE, OnSelchangeBlendingMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog message handlers

void CLocatedBindableDialog::OnSelchangeBlendingMode() 
{
	UpdateData() ;
	NL3D::CPSMaterial *m = dynamic_cast<NL3D::CPSMaterial *>(_Bindable) ;
	nlassert(m) ;
	m->setBlendingMode( (NL3D::CPSMaterial::TBlendingMode) m_BlendingMode.GetCurSel()) ;	
}
