/** \file located_bindable_dialog.cpp
 * a dialog for located bindable properties (particles ...)
 *
 * $Id: located_bindable_dialog.cpp,v 1.26 2003/08/08 16:58:17 vizerie Exp $
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
#include "3d/ps_mesh.h"
#include "3d/ps_force.h"
#include "3d/ps_emitter.h"
#include "3d/ps_zone.h"
#include "3d/particle_system_model.h"


#include "texture_chooser.h"
#include "attrib_dlg.h"
#include "precomputed_rotations_dlg.h"
#include "tail_particle_dlg.h"
#include "mesh_dlg.h"
#include "texture_anim_dlg.h"
#include "particle_dlg.h"
#include "constraint_mesh_dlg.h"
#include "constraint_mesh_tex_dlg.h"
#include "ribbon_dlg.h"


using NL3D::CPSLocatedBindable; 


/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog


//***********************************************************************************
CLocatedBindableDialog::CLocatedBindableDialog(NL3D::CPSLocatedBindable *bindable)
	: _Bindable(bindable), _SizeCtrl(NULL)
{
	//{{AFX_DATA_INIT(CLocatedBindableDialog)
	m_IndependantSizes = FALSE;
	//}}AFX_DATA_INIT
}

/// dtor
//***********************************************************************************
CLocatedBindableDialog::~CLocatedBindableDialog()
{
	if (_SizeCtrl)
	{
		_SizeCtrl->DestroyWindow();
		delete _SizeCtrl;
	}
}

//***********************************************************************************
void CLocatedBindableDialog::init(CParticleDlg* pParent)
{
	Create(IDD_LOCATED_BINDABLE, pParent);
	ShowWindow(SW_SHOW);
	_ParticleDlg = pParent;

	NL3D::CParticleSystem *ps = _Bindable->getOwner()->getOwner();
	if (ps->isSharingEnabled())
	{	
		GetDlgItem(IDC_NO_AUTO_LOD)->ShowWindow(TRUE);
		if (ps->isAutoLODEnabled() == false)
		{
			GetDlgItem(IDC_NO_AUTO_LOD)->EnableWindow(FALSE);
		}
		else
		{
			((CButton *) GetDlgItem(IDC_NO_AUTO_LOD))->SetCheck(NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable)->isAutoLODDisabled());
		}
	}
	else
	{
		GetDlgItem(IDC_NO_AUTO_LOD)->ShowWindow(FALSE);
	}

	uint yPos = 60;
	const uint xPos = 5;
	RECT rect;

	// has the particle a material ?
	if (dynamic_cast<NL3D::CPSMaterial *>(_Bindable))
	{
		m_BlendingMode.SetCurSel((uint) (dynamic_cast<NL3D::CPSMaterial *>(_Bindable))->getBlendingMode() );
	}
	else
	{
		m_BlendingMode.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BLENDING_MODE_STATIC)->ShowWindow(SW_HIDE);
	}

	if (dynamic_cast<NL3D::CPSParticle *>(_Bindable))
	{
		NL3D::CPSParticle *p = (NL3D::CPSParticle *) _Bindable;

		// check support for lighting
		if (p->supportGlobalColorLighting())
		{
			GetDlgItem(ID_GLOBAL_COLOR_LIGHTING)->ShowWindow(SW_SHOW);
			// if global color lighting is forced for all objects, don't allow to modify
			GetDlgItem(ID_GLOBAL_COLOR_LIGHTING)->EnableWindow(ps->getForceGlobalColorLightingFlag() ? FALSE : TRUE);
			((CButton *) GetDlgItem(ID_GLOBAL_COLOR_LIGHTING))->SetCheck(p->usesGlobalColorLighting() ? 1 : 0);
		}
		else
		{
			GetDlgItem(ID_GLOBAL_COLOR_LIGHTING)->ShowWindow(SW_HIDE);
		}
		// check support for color
		if (dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable))
		{
			
			CAttribDlgRGBA *ad = new CAttribDlgRGBA("PARTICLE_COLOR");
			pushWnd(ad);

			_ColorWrapper.S = dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable);		
			ad->setWrapper(&_ColorWrapper);			
			ad->setSchemeWrapper(&_ColorWrapper);

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_COLOR));
			ad->init(bmh, xPos, yPos, this);
			ad->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// init size ctrl
		_SizeCtrlX = xPos;
		_SizeCtrlY = yPos;
		yPos += updateSizeControl();

		// check support for angle 2D
		if (dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable))
		{
			
			CAttribDlgFloat *ad = new CAttribDlgFloat("PARTICLE_ANGLE2D", 0.f, 256.f);
			pushWnd(ad);

			_Angle2DWrapper.S = dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable);
			ad->setWrapper(&_Angle2DWrapper);						
			ad->setSchemeWrapper(&_Angle2DWrapper);	

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_ANGLE));
			ad->init(bmh, xPos, yPos, this);
			ad->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		

		CAttribDlgPlaneBasis *pb = NULL;

		// check support for plane basis
		if (dynamic_cast<NL3D::CPSRotated3DPlaneParticle *>(_Bindable))
		{
			pb = new CAttribDlgPlaneBasis("PARTICLE_PLANE_BASIS");
			pushWnd(pb);
			_PlaneBasisWrapper.S = dynamic_cast<NL3D::CPSRotated3DPlaneParticle *>(_Bindable);
			pb->setWrapper(&_PlaneBasisWrapper);
			pb->setSchemeWrapper(&_PlaneBasisWrapper);
			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BASIS));
			pb->init(bmh, xPos, yPos, this);
			pb->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		
		}

		// check support for precomputed rotations
		if (dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable))
		{
			CPrecomputedRotationsDlg *pr = new CPrecomputedRotationsDlg(dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable)
																		, pb);
			pushWnd(pr);
			pr->init(this, xPos, yPos);
			pr->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// if we're dealing with a face look at, motion blur can be tuned
		if (dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable))
		{

			CEditableRangeFloat *mbc = new CEditableRangeFloat(std::string("MOTION_BLUR_COEFF"), 0, 5);
			pushWnd(mbc);
			_MotionBlurCoeffWrapper.P = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
			mbc->setWrapper(&_MotionBlurCoeffWrapper);
			mbc->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;			
			pushWnd(s);
			s->Create("Fake motion blur coeff.", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);


			mbc->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			mbc = new CEditableRangeFloat(std::string("MOTION_BLUR_THRESHOLD"), 0, 5);
			pushWnd(mbc);
			_MotionBlurThresholdWrapper.P = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
			mbc->setWrapper(&_MotionBlurThresholdWrapper);
			mbc->init(xPos + 140, yPos, this);

			s = new CStatic;			
			pushWnd(s);
			s->Create("Fake motion blur threshold.", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);

			mbc->GetClientRect(&rect);
			yPos += rect.bottom + 3;			
		}

		// if we're dealing with a shockwave, we add dlg for the radius cut, and the number of segments
		if (dynamic_cast<NL3D::CPSShockWave *>(_Bindable))
		{
			NL3D::CPSShockWave *sw = static_cast<NL3D::CPSShockWave *>(_Bindable);
			CEditableRangeFloat *rc = new CEditableRangeFloat(std::string("RADIUS CUT"), 0, 1);
			pushWnd(rc);			
			_RadiusCutWrapper.S = sw;
			rc->setWrapper(&_RadiusCutWrapper);
			rc->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;			
			pushWnd(s);
			s->Create("Radius cut.", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);


			rc->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			CEditableRangeUInt *snbs = new CEditableRangeUInt(std::string("SHOCK WAVE NB SEG"), 3, 24);
			pushWnd(snbs);
			_ShockWaveNbSegWrapper.S = sw;
			snbs->enableLowerBound(3, false);
			snbs->setWrapper(&_ShockWaveNbSegWrapper);
			snbs->init(xPos + 140, yPos, this);

			s = new CStatic;			
			pushWnd(s);
			s->Create("Nb segs", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);

			snbs->GetClientRect(&rect);
			yPos += rect.bottom + 3;


			CEditableRangeFloat *uvd = new CEditableRangeFloat(std::string("TEX UFACTOR"), 0, 5);
			pushWnd(uvd);
			_ShockWaveUFactorWrapper.S = sw;
			uvd->setWrapper(&_ShockWaveUFactorWrapper);
			uvd->init(xPos + 140, yPos, this);
			s = new CStatic;			
			pushWnd(s);
			s->Create("Texture U factor :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);
			uvd->GetClientRect(&rect);
			yPos += rect.bottom + 3;	
			
		}

		// fanlight
		if (dynamic_cast<NL3D::CPSFanLight *>(_Bindable))
		{
			CEditableRangeUInt *nbf = new CEditableRangeUInt(std::string("NB_FANS"), 3, 127);
			pushWnd(nbf);
			nbf->enableLowerBound(3, false);
			nbf->enableUpperBound(128, true);
			_FanLightWrapper.P = dynamic_cast<NL3D::CPSFanLight *>(_Bindable);
			nbf->setWrapper(&_FanLightWrapper);
			nbf->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;			
			pushWnd(s);
			s->Create("Nb fan lights :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);

			nbf->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			nbf = new CEditableRangeUInt(std::string("PHASE_SMOOTHNESS"), 0, 31);
			pushWnd(nbf);
			nbf->enableUpperBound(32, true);
			_FanLightSmoothnessWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			nbf->setWrapper(&_FanLightSmoothnessWrapper);
			nbf->init(xPos + 140, yPos, this);
			s = new CStatic;			
			pushWnd(s);
			s->Create("Phase smoothnes:", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);

			nbf->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			CEditableRangeFloat *nbfp = new CEditableRangeFloat(std::string("FAN_LIGHT_PHASE"), 0, 4.f);
			pushWnd(nbfp);			
			_FanLightPhaseWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			nbfp->setWrapper(&_FanLightPhaseWrapper);
			nbfp->init(xPos + 140, yPos, this);
			s = new CStatic;			
			pushWnd(s);
			s->Create("Fan light speed :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);

			nbf->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			nbfp = new CEditableRangeFloat(std::string("FAN_LIGHT_INTENSITY"), 0, 4.f);
			pushWnd(nbfp);			
			_FanLightIntensityWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			nbfp->setWrapper(&_FanLightIntensityWrapper);
			nbfp->init(xPos + 140, yPos, this);
			s = new CStatic;			
			pushWnd(s);
			s->Create("Fan light intensity:", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);

			nbf->GetClientRect(&rect);
			yPos += rect.bottom + 3;

		}


		// tail particle
		if (dynamic_cast<NL3D::CPSTailParticle *>(_Bindable))
		{
			CEditableRangeUInt *nbs;
			if (!dynamic_cast<NL3D::CPSRibbonLookAt *>(_Bindable))
			{
				nbs = new CEditableRangeUInt(std::string("TAIL_NB_SEGS_"), 2, 16);
				nbs->enableLowerBound(1, true);
			}
			else
			{
				nbs = new CEditableRangeUInt(std::string("LOOKAT_RIBBON_TAIL_NB_SEGS_"), 2, 16);
				nbs->enableLowerBound(1, true);
			}

			pushWnd(nbs);

			if (dynamic_cast<NL3D::CPSTailDot *>(_Bindable))
			{
				nbs->enableUpperBound(256, true);
			}
			
			
			_TailParticleWrapper.P = dynamic_cast<NL3D::CPSTailParticle *>(_Bindable);
			nbs->setWrapper(&_TailParticleWrapper);
			nbs->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;			
			pushWnd(s);
			s->Create("Nb segs :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);	
			
			nbs->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			CTailParticleDlg *tpd = new CTailParticleDlg(dynamic_cast<NL3D::CPSTailParticle *>(_Bindable));
			pushWnd(tpd);
			tpd->init(this, xPos, yPos);
				
			tpd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}


		// shape particle
		if (dynamic_cast<NL3D::CPSShapeParticle *>(_Bindable))
		{
			CMeshDlg *md = new CMeshDlg(dynamic_cast<NL3D::CPSShapeParticle *>(_Bindable), _ParticleDlg);
			md->init(this, xPos, yPos);
			md->GetClientRect(&rect);
			yPos += rect.bottom + 3;
			pushWnd(md);
		}

		// constraint mesh particle
		if (dynamic_cast<NL3D::CPSConstraintMesh *>(_Bindable))
		{
			CConstraintMeshDlg *cmd = new CConstraintMeshDlg(static_cast<NL3D::CPSConstraintMesh *>(_Bindable));
			cmd->init(xPos, yPos, this);
			cmd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
			CConstraintMeshTexDlg *cmtd = new CConstraintMeshTexDlg(static_cast<NL3D::CPSConstraintMesh *>(_Bindable),
																	this);
			cmtd->init(xPos, yPos, this);
			cmtd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
			pushWnd(cmd);
			pushWnd(cmtd);			
		}



		// check support for animated texture
		if (dynamic_cast<NL3D::CPSTexturedParticle *>(_Bindable))
		{
			CTextureAnimDlg *td = new CTextureAnimDlg(dynamic_cast<NL3D::CPSTexturedParticle *>(_Bindable),
													  dynamic_cast<NL3D::CPSMultiTexturedParticle *>(_Bindable)
													 );			
			pushWnd(td);
						
			td->init(xPos, yPos, this);
			td->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// unanimated texture
		if (dynamic_cast<NL3D::CPSTexturedParticleNoAnim *>(_Bindable))
		{
			NL3D::CPSTexturedParticleNoAnim *tp = dynamic_cast<NL3D::CPSTexturedParticleNoAnim *>(_Bindable);
			_TextureNoAnimWrapper.TP = tp;
			CTextureChooser *tc = new CTextureChooser(dynamic_cast<NL3D::CPSMultiTexturedParticle *>(_Bindable));			
			tc->enableRemoveButton();
			tc->setWrapper(&_TextureNoAnimWrapper);
			pushWnd(tc);

			tc->init(xPos, yPos, this);
			tc->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}

		// ribbon texture (doesn't support texture animation for now)
		if (dynamic_cast<NL3D::CPSRibbon *>(_Bindable))
		{									
		
			// add dialog for uv tuning with ribbon
			CEditableRangeFloat *uvd = new CEditableRangeFloat(std::string("RIBBON UFACTOR"), 0, 5);
			pushWnd(uvd);
			_RibbonUFactorWrapper.R = static_cast<NL3D::CPSRibbon *>(_Bindable);
			uvd->setWrapper(&_RibbonUFactorWrapper);
			uvd->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;			
			pushWnd(s);
			s->Create("Texture U factor :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);
			uvd->GetClientRect(&rect);
			yPos += rect.bottom + 3;

			uvd = new CEditableRangeFloat(std::string("RIBBON VFACTOR"), 0, 5);
			pushWnd(uvd);
			_RibbonVFactorWrapper.R = static_cast<NL3D::CPSRibbon *>(_Bindable);
			uvd->setWrapper(&_RibbonVFactorWrapper);
			uvd->init(xPos + 140, yPos, this);
			s = new CStatic;			
			pushWnd(s);
			s->Create("Texture V factor :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);
			uvd->GetClientRect(&rect);
			yPos += rect.bottom + 3;

		}		

		if (dynamic_cast<NL3D::CPSRibbonBase *>(_Bindable))
		{									
		
			// add dialog for uv tuning with ribbon
			CEditableRangeFloat *sd = new CEditableRangeFloat(std::string("SEGMENT DURATION"), 0.05f, 0.5f);
			sd->enableLowerBound(0, true);
			pushWnd(sd);
			_SegDurationWrapper.R = static_cast<NL3D::CPSRibbonLookAt *>(_Bindable);
			sd->setWrapper(&_SegDurationWrapper);
			sd->init(xPos + 140, yPos, this);
			CStatic *s = new CStatic;			
			pushWnd(s);
			s->Create("Seg Duration :", SS_LEFT, CRect(xPos, yPos, xPos + 139, yPos + 32), this);
			s->ShowWindow(SW_SHOW);
			sd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}
	
		// 'look at' independant sizes
		bool isLookAt = dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable) != NULL;		
		GetDlgItem(IDC_INDE_SIZES)->ShowWindow(isLookAt ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_SIZE_WIDTH)->ShowWindow(isLookAt ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_SIZE_HEIGHT)->ShowWindow(isLookAt ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_WIDTH_HEIGHT_BOX)->ShowWindow(isLookAt ? SW_SHOW : SW_HIDE);		
		((CButton *) GetDlgItem(IDC_SIZE_WIDTH))->SetCheck(1);
		if (isLookAt)
		{
			NL3D::CPSFaceLookAt *la = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
			m_IndependantSizes = la->hasIndependantSizes();
			((CButton *) GetDlgItem(IDC_INDE_SIZES))->SetCheck(m_IndependantSizes);
			updateIndependantSizes();
		}

		/// new ribbon base class
		if (dynamic_cast<NL3D::CPSRibbonBase *>(_Bindable))
		{
			CRibbonDlg *rd = new CRibbonDlg(static_cast<NL3D::CPSRibbonBase *>(_Bindable),
											this);
			rd->init(this, xPos, yPos);
			pushWnd(rd);
			rd->GetClientRect(&rect);
			yPos += rect.bottom + 3;
		}		
	}	
	UpdateData();
}


//***********************************************************************************
void CLocatedBindableDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedBindableDialog)
	DDX_Control(pDX, IDC_BLENDING_MODE, m_BlendingMode);
	DDX_Check(pDX, IDC_INDE_SIZES, m_IndependantSizes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedBindableDialog, CDialog)
	//{{AFX_MSG_MAP(CLocatedBindableDialog)
	ON_CBN_SELCHANGE(IDC_BLENDING_MODE, OnSelchangeBlendingMode)
	ON_BN_CLICKED(IDC_INDE_SIZES, OnIndeSizes)
	ON_BN_CLICKED(IDC_SIZE_WIDTH, OnSizeWidth)
	ON_BN_CLICKED(IDC_SIZE_HEIGHT, OnSizeHeight)
	ON_BN_CLICKED(IDC_NO_AUTO_LOD, OnNoAutoLod)
	ON_BN_CLICKED(ID_GLOBAL_COLOR_LIGHTING, OnGlobalColorLighting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog message handlers

//***********************************************************************************
void CLocatedBindableDialog::OnSelchangeBlendingMode() 
{
	UpdateData();
	NL3D::CPSMaterial *m = dynamic_cast<NL3D::CPSMaterial *>(_Bindable);
	nlassert(m);
	m->setBlendingMode( (NL3D::CPSMaterial::TBlendingMode) m_BlendingMode.GetCurSel());	
	_ParticleDlg->getCurrPSModel()->touchTransparencyState();
	_ParticleDlg->getCurrPSModel()->touchLightableState();
}


//***********************************************************************************
void CLocatedBindableDialog::updateIndependantSizes() 
{
	UpdateData();
	// make sure we are dealing with 'LookAt' for now
	nlassert(dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable));

	GetDlgItem(IDC_SIZE_WIDTH)->EnableWindow(m_IndependantSizes);
	GetDlgItem(IDC_SIZE_HEIGHT)->EnableWindow(m_IndependantSizes);
}


//***********************************************************************************
// user asked for independant sizes
void CLocatedBindableDialog::OnIndeSizes() 
{
	UpdateData();
	// make sure we are dealing with 'LookAt' for now
	nlassert(dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable));
	NL3D::CPSFaceLookAt *la = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	la->setIndependantSizes(m_IndependantSizes ? true : false /* VCC warning*/);
	updateIndependantSizes();
	updateSizeControl();	
}

//***********************************************************************************
uint CLocatedBindableDialog::updateSizeControl()
{	
	HBITMAP bmh;
	if (!dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable)) return 0;
	// if a previous control was there, remove it
	if (_SizeCtrl)
	{
		_SizeCtrl->DestroyWindow();
		delete _SizeCtrl;
		_SizeCtrl = NULL;
	}

	NL3D::CPSFaceLookAt *fla = dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	// LookAt case
	if (fla && fla->hasIndependantSizes())
	{
		int editWidth = (((CButton *) GetDlgItem(IDC_SIZE_WIDTH))->GetCheck());
		_SizeCtrl = new CAttribDlgFloat(editWidth ? "PARTICLE_WIDTH" : "PARTICLE_HEIGHT", 0.f, 1.f);

		
		if (editWidth) // wrap to the wanted size
		{
			_SizeWrapper.S = fla;
		}
		else
		{
			_SizeWrapper.S = &fla->getSecondSize();
		}
		_SizeCtrl->setWrapper(&_SizeWrapper);
		_SizeCtrl->setSchemeWrapper(&_SizeWrapper);
				
		bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(editWidth ? IDB_PARTICLE_WIDTH : IDB_PARTICLE_HEIGHT));	
	}
	else // general case. Wrap to the size interface and the appropriate dialog
	{
		_SizeCtrl = new CAttribDlgFloat("PARTICLE_SIZE", 0.f, 1.f);

		_SizeWrapper.S = dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable);
		_SizeCtrl->setWrapper(&_SizeWrapper);
		_SizeCtrl->setSchemeWrapper(&_SizeWrapper);
				
		bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_SIZE));		
	}
	RECT rect;
	_SizeCtrl->init(bmh, _SizeCtrlX, _SizeCtrlY, this);
	_SizeCtrl->GetClientRect(&rect);
	return rect.bottom + 3;		
}

//***********************************************************************************
void CLocatedBindableDialog::OnSizeWidth() 
{
	updateSizeControl();		
}

//***********************************************************************************
void CLocatedBindableDialog::OnSizeHeight() 
{
	updateSizeControl();	
}

//***********************************************************************************
void CLocatedBindableDialog::OnNoAutoLod() 
{
	NL3D::CPSParticle *p = NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable);
	p->disableAutoLOD(((CButton *) GetDlgItem(IDC_NO_AUTO_LOD))->GetCheck() != 0);
}

//***********************************************************************************
void CLocatedBindableDialog::OnGlobalColorLighting() 
{	
	NL3D::CPSParticle *p = NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable);
	p->enableGlobalColorLighting(((CButton *) GetDlgItem(ID_GLOBAL_COLOR_LIGHTING))->GetCheck() == 1);
	_ParticleDlg->getCurrPSModel()->touchLightableState();
}
