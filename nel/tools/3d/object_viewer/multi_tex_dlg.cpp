/** \file multi_tex_dlg.cpp
 * A dialog to tune multexturing for particles that support it
 *
 * $Id: multi_tex_dlg.cpp,v 1.4.4.1 2003/06/02 11:26:00 boucher Exp $
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
//


#include "std_afx.h"
#include "object_viewer.h"
#include "multi_tex_dlg.h"
#include "3d/ps_particle_basic.h"
#include "3d/texture_bump.h"
#include "popup_notify.h"

#include "texture_chooser.h"

#undef new

/////////////////////////////////////////////////////////////////////////////
// CMultiTexDlg dialog


CMultiTexDlg::CMultiTexDlg(NL3D::CPSMultiTexturedParticle *mtp, IPopupNotify* pn, CWnd *pParent)
	: CDialog(IDD_MULTITEX, pParent),
	  _MTP(mtp),
	  _MainTexDlg(NULL), _AltTexDlg(NULL),
	  _PN(pn)
{
	nlassert(_MTP)
	//{{AFX_DATA_INIT(CMultiTexDlg)
	m_ForceBasicCaps = NL3D::CPSMultiTexturedParticle::areBasicCapsForced();
	m_UseParticleDateAlt = _MTP->getUseLocalDateAlt();
	m_UseParticleDate = _MTP->getUseLocalDate();
	//}}AFX_DATA_INIT
}

void CMultiTexDlg::init(CWnd *pParent)
{
	Create(IDD_MULTITEX, pParent);
	ShowWindow(SW_SHOW);
}

CMultiTexDlg::~CMultiTexDlg()
{
	#define REMOVE_DLG(dlg) if (dlg) { (dlg)->DestroyWindow(); delete (dlg); }
	REMOVE_DLG(_MainTexDlg);
	REMOVE_DLG(_AltTexDlg);
}


void CMultiTexDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiTexDlg)
	DDX_Control(pDX, IDC_USE_PARTICLE_DATE2, m_UseParticleDateAltCtrl);
	DDX_Control(pDX, IDC_USE_PARTICLE_DATE, m_UseParticleDateCtrl);
	DDX_Control(pDX, IDC_FORCE_BASIC_CAPS, m_ForceBasicCapsCtrl);
	DDX_Control(pDX, IDC_ENABLE_ALTERNATE_TEX, m_AltTexCtrl);
	DDX_Control(pDX, IDC_ALTERNATE_OP, m_AlternateOpCtrl);
	DDX_Control(pDX, IDC_MAIN_OP, m_MainOpCtrl);
	DDX_Check(pDX, IDC_FORCE_BASIC_CAPS, m_ForceBasicCaps);
	DDX_Check(pDX, IDC_USE_PARTICLE_DATE2, m_UseParticleDateAlt);
	DDX_Check(pDX, IDC_USE_PARTICLE_DATE, m_UseParticleDate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiTexDlg, CDialog)
	//{{AFX_MSG_MAP(CMultiTexDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ENABLE_ALTERNATE_TEX, OnEnableAlternate)
	ON_BN_CLICKED(IDC_UPDATE_SPEED, OnUpdateSpeed)
	ON_BN_CLICKED(IDC_UPDATE_SPEED_ALTERNATE, OnUpdateSpeedAlternate)
	ON_CBN_SELCHANGE(IDC_ALTERNATE_OP, OnSelchangeAlternateOp)
	ON_CBN_SELCHANGE(IDC_MAIN_OP, OnSelchangeMainOp)
	ON_BN_CLICKED(IDC_FORCE_BASIC_CAPS, OnForceBasicCaps)
	ON_BN_CLICKED(IDC_USE_PARTICLE_DATE, OnUseParticleDate)
	ON_BN_CLICKED(IDC_USE_PARTICLE_DATE2, OnUseParticleDateAlt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiTexDlg message handlers

BOOL CMultiTexDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	_TexWrapper.MTP			 = _MTP;
	_AlternateTexWrapper.MTP = _MTP;

	RECT r;
	
	_MainTexDlg = new CTextureChooser;
	_MainTexDlg->setWrapper(&_TexWrapper);
	GetDlgItem(IDC_TEX_CHOOSER)->GetWindowRect(&r);
	ScreenToClient(&r);
	_MainTexDlg->init(r.left, r.top, this);

	_AltTexDlg = new CTextureChooser;
	_AltTexDlg->setWrapper(&_AlternateTexWrapper);
	GetDlgItem(IDC_TEX_CHOOSER_ALTERNATE)->GetWindowRect(&r);
	ScreenToClient(&r);
	_AltTexDlg->init(r.left, r.top, this);
	
	readValues(false);
	updateAlternate();
	updateTexOp();

	if (_MTP->isAlternateTexEnabled())
	{
		m_AltTexCtrl.SetCheck(1);
	}

	

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//======================================================
// texture wrappers
	
	
NL3D::ITexture *CMultiTexDlg::CMainTexWrapper::get(void)
{
	return MTP->getTexture2();
}

void CMultiTexDlg::CMainTexWrapper::set(NL3D::ITexture *tex)
{
	MTP->setTexture2(tex);
}

NL3D::ITexture *CMultiTexDlg::CAlternateTexWrapper::get(void)
{
	return MTP->getTexture2Alternate();
}

void CMultiTexDlg::CAlternateTexWrapper::set(NL3D::ITexture *tex)
{
	MTP->setTexture2Alternate(tex);
}


//======================================================

void CMultiTexDlg::readValues(bool alternate)
{
	/// get the values, and put them in the dialog
	char buf[128];
	if (!alternate)
	{
		sprintf(buf, "%.3f", _MTP->getScrollSpeed(0).x); GetDlgItem(IDC_U_SPEED_1)->SetWindowText(buf);
		sprintf(buf, "%.3f", _MTP->getScrollSpeed(0).y); GetDlgItem(IDC_V_SPEED_1)->SetWindowText(buf);
		sprintf(buf, "%.3f", _MTP->getScrollSpeed(1).x); GetDlgItem(IDC_U_SPEED_2)->SetWindowText(buf);
		sprintf(buf, "%.3f", _MTP->getScrollSpeed(1).y); GetDlgItem(IDC_V_SPEED_2)->SetWindowText(buf);
	}
	else
	{
		if (_MTP->isAlternateTexEnabled())
		{
			sprintf(buf, "%.3f", _MTP->getAlternateScrollSpeed(0).x); GetDlgItem(IDC_U_SPEED_1_ALTERNATE)->SetWindowText(buf);
			sprintf(buf, "%.3f", _MTP->getAlternateScrollSpeed(0).y); GetDlgItem(IDC_V_SPEED_1_ALTERNATE)->SetWindowText(buf);
			sprintf(buf, "%.3f", _MTP->getAlternateScrollSpeed(1).x); GetDlgItem(IDC_U_SPEED_2_ALTERNATE)->SetWindowText(buf);
			sprintf(buf, "%.3f", _MTP->getAlternateScrollSpeed(1).y); GetDlgItem(IDC_V_SPEED_2_ALTERNATE)->SetWindowText(buf);
		}
		else
		{
			GetDlgItem(IDC_U_SPEED_1_ALTERNATE)->SetWindowText("");
			GetDlgItem(IDC_V_SPEED_1_ALTERNATE)->SetWindowText("");
			GetDlgItem(IDC_U_SPEED_2_ALTERNATE)->SetWindowText("");
			GetDlgItem(IDC_V_SPEED_2_ALTERNATE)->SetWindowText("");
		}
	}
	sprintf(buf, "%.3f", _MTP->getBumpFactor()); GetDlgItem(IDC_BUMP_FACTOR)->SetWindowText(buf);

}


//======================================================
void	CMultiTexDlg::updateBumpFactorEnabled()
{
	BOOL bEnvBumpMapUsed = _MTP->getMainTexOp() == NL3D::CPSMultiTexturedParticle::EnvBumpMap ? TRUE : FALSE;
	GetDlgItem(IDC_BUMP_FACTOR_TXT)->EnableWindow(bEnvBumpMapUsed);
	GetDlgItem(IDC_BUMP_FACTOR)->EnableWindow(bEnvBumpMapUsed);
}


//======================================================
void CMultiTexDlg::writeValues(bool alternate)
{
	char u1[10], u2[10], v1[10], v2[10];
	NLMISC::CVector2f vs1, vs2;


	if (!alternate)
	{
		GetDlgItem(IDC_U_SPEED_1)->GetWindowText(u1, 10);
		GetDlgItem(IDC_V_SPEED_1)->GetWindowText(v1, 10);
		GetDlgItem(IDC_U_SPEED_2)->GetWindowText(u2, 10);
		GetDlgItem(IDC_V_SPEED_2)->GetWindowText(v2, 10);

		if (sscanf(u1, "%f", &vs1.x) == 1 &&
			sscanf(v1, "%f", &vs1.y) == 1 &&
			sscanf(u2, "%f", &vs2.x) == 1 &&
			sscanf(v2, "%f", &vs2.y) == 1)
		{
			_MTP->setScrollSpeed(0, vs1);	
			_MTP->setScrollSpeed(1, vs2);	
		}
		else
		{
			MessageBox("Invalid value(s)", "Error", MB_OK | MB_ICONEXCLAMATION);
		}
	}
	else
	{
		if (_MTP->isAlternateTexEnabled())
		{
			GetDlgItem(IDC_U_SPEED_1_ALTERNATE)->GetWindowText(u1, 10);
			GetDlgItem(IDC_V_SPEED_1_ALTERNATE)->GetWindowText(v1, 10);
			GetDlgItem(IDC_U_SPEED_2_ALTERNATE)->GetWindowText(u2, 10);
			GetDlgItem(IDC_V_SPEED_2_ALTERNATE)->GetWindowText(v2, 10);	
			if (sscanf(u1, "%f", &vs1.x) == 1 &&
			sscanf(v1, "%f", &vs1.y) == 1 &&
			sscanf(u2, "%f", &vs2.x) == 1 &&
			sscanf(v2, "%f", &vs2.y) == 1)
			{
				_MTP->setAlternateScrollSpeed(0, vs1);	
				_MTP->setAlternateScrollSpeed(1, vs2);	
			}
		}
	}

	char bumpFactorTxt[10];
	float bumpFactor; 
	GetDlgItem(IDC_BUMP_FACTOR)->GetWindowText(bumpFactorTxt, 10);
	if (sscanf(bumpFactorTxt, "%f", &bumpFactor) == 1)
	{
		_MTP->setBumpFactor(bumpFactor);
	}
}

//======================================================
void CMultiTexDlg::OnClose() 
{		
	CDialog::OnClose();
	if (_PN)
		_PN->childPopupClosed(this);
}

void CMultiTexDlg::OnEnableAlternate() 
{
	_MTP->enableAlternateTex(!_MTP->isAlternateTexEnabled());
	updateAlternate();
}

//======================================================
void CMultiTexDlg::updateAlternate()
{
	BOOL bEnable = _MTP->isAlternateTexEnabled();

	GetDlgItem(IDC_U_SPEED_1_ALTERNATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_V_SPEED_1_ALTERNATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_U_SPEED_2_ALTERNATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_V_SPEED_2_ALTERNATE)->EnableWindow(bEnable);	
	GetDlgItem(IDC_UPDATE_SPEED_ALTERNATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_ALTERNATE_OP)->EnableWindow(bEnable);
	_AltTexDlg->EnableWindow(bEnable);
	readValues(true);
}

//======================================================
void CMultiTexDlg::OnUpdateSpeed() 
{
	writeValues(false);	
}

//======================================================
void CMultiTexDlg::OnUpdateSpeedAlternate() 
{
	writeValues(true);	
}

//======================================================
void CMultiTexDlg::updateTexOp()
{
	m_MainOpCtrl.SetCurSel((int) _MTP->getMainTexOp());
	m_AlternateOpCtrl.SetCurSel((int) _MTP->getAlternateTexOp());
	UpdateData(FALSE);
	updateBumpFactorEnabled();
}

//======================================================
void CMultiTexDlg::OnSelchangeAlternateOp() 
{
	UpdateData(TRUE);
	_MTP->setAlternateTexOp((NL3D::CPSMultiTexturedParticle::TOperator) m_AlternateOpCtrl.GetCurSel());	
}

//======================================================
void CMultiTexDlg::OnSelchangeMainOp() 
{
	UpdateData(TRUE);
	_MTP->setMainTexOp((NL3D::CPSMultiTexturedParticle::TOperator) m_MainOpCtrl.GetCurSel());	
	updateBumpFactorEnabled();
	
}

//======================================================
void CMultiTexDlg::OnForceBasicCaps() 
{
	UpdateData();
	NL3D::CPSMultiTexturedParticle::forceBasicCaps(m_ForceBasicCaps ? true : false /* VC WARNING */);
	
}

//======================================================
void CMultiTexDlg::OnUseParticleDate() 
{
	UpdateData();
	_MTP->setUseLocalDate(m_UseParticleDate ? true : false /* VC WARNING */);	
}

//======================================================
void CMultiTexDlg::OnUseParticleDateAlt() 
{
	UpdateData();
	_MTP->setUseLocalDateAlt(m_UseParticleDateAlt ? true : false /* VC WARNING */);	
}
