/** \file particle_dlg.cpp
 * <File description>
 *
 * $Id: particle_dlg.cpp,v 1.3 2001/06/15 16:24:45 corvazier Exp $
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


// particle_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "particle_dlg.h"
#include "editable_range.h"
#include "located_properties.h"


// TODO : remove these include when the test system will be removed
#include "3d/particle_system.h"
#include "3d/ps_force.h"
#include "3d/ps_emitter.h"
#include "3d/ps_particle.h"
#include "3d/ps_util.h"
#include "3d/ps_zone.h"
#include "3d/ps_color.h"
#include "3d/ps_float.h"
#include "3d/ps_int.h"
#include "3d/ps_plane_basis_maker.h"
#include "3d/particle_system_model.h"
#include "3d/particle_system_shape.h"
#include "3d/texture_file.h"
#include "3d/texture_grouped.h"
#include "3d/nelu.h"
#include "3d/font_manager.h"
#include "3d/font_generator.h"

#include "nel/misc/file.h"

#include "start_stop_particle_system.h"


using namespace NL3D ;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParticleDlg dialog


CParticleDlg::CParticleDlg(CWnd *pParent, CSceneDlg* sceneDlg)
	: CDialog(CParticleDlg::IDD, pParent), SceneDlg(sceneDlg), _CurrentRightPane(NULL)
{
	//{{AFX_DATA_INIT(CParticleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT





	// Font manager
	FontManager = new NL3D::CFontManager ;	
	FontManager->setMaxMemory(2000000);
		
	// Font generator
	FontGenerator = new NL3D::CFontGenerator ("arial.ttf") ;



	//////


	_ParticleTreeCtrl = new CParticleTreeCtrl(this) ;


	// for now, let's create a dummy system for test

	CParticleSystem *ps = new CParticleSystem ;

	// now, saves the system, and use a stream to instanciate it in the scene

	CParticleSystemShape psc   ;
	psc.buildFromPS(*ps) ;

	{
		CShapeStream st(&psc) ;
		NLMISC::COFile oFile("dummy.shape") ;

		oFile.serial(st) ;
	}
	delete ps ;


	
	_CurrSystemModel = dynamic_cast<CParticleSystemModel *>(NL3D::CNELU::Scene.createInstance("dummy.shape")) ;

	nlverify(_CurrSystemModel) ;

	_CurrSystemModel->enableDisplayTools() ;
	_CurrSystemModel->setEllapsedTime(0.f) ;
	_CurrPS = _CurrSystemModel->getPS() ;

	_CurrPS->setFontManager(FontManager) ;
	_CurrPS->setFontGenerator(FontGenerator) ;





	_StartStopDlg = new CStartStopParticleSystem(this) ;
		

}


void CParticleDlg::moveElement(const NLMISC::CMatrix &mat)
{
	_ParticleTreeCtrl->moveElement(mat) ;
}

NLMISC::CMatrix CParticleDlg::getElementMatrix(void) const
{
	return _ParticleTreeCtrl->getElementMatrix() ;
}



CParticleDlg::~CParticleDlg()
{
	//NL3D::CNELU::Scene.deleteInstance(_CurrSystemModel) ;

	delete _ParticleTreeCtrl ;
	delete _CurrentRightPane ;

	delete _StartStopDlg ;

	delete FontManager ;
	delete FontGenerator ;
}



void CParticleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParticleDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CParticleDlg, CDialog)
	//{{AFX_MSG_MAP(CParticleDlg)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParticleDlg message handlers

void CParticleDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_OBJ_PARTICLE_DLG);
	CDialog::OnDestroy();		
}

BOOL CParticleDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect r ;
	GetWindowRect(&r) ;
	
	_ParticleTreeCtrl->Create(WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER
   | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_EDITLABELS 
   | TVS_DISABLEDRAGDROP , r, this, 0x1005) ;


	_ParticleTreeCtrl->buildTreeFromPS(_CurrPS, _CurrSystemModel) ;
	_ParticleTreeCtrl->init() ;
	_ParticleTreeCtrl->ShowWindow(SW_SHOW) ;


	_StartStopDlg->Create(IDD_PARTICLE_SYSTEM_START_STOP, this) ;	



	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}




void CParticleDlg::OnSize(UINT nType, int cx, int cy) 
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	bool blocked = false ;

	if (_ParticleTreeCtrl->m_hWnd && this->m_hWnd)
	{	

	/*	if (_CurrentRightPane)
		{
			if (cx < (120 + _CurrRightPaneWidth))
			{
				cx = 120 + _CurrRightPaneWidth ;
				blocked = true ;
			}
			if (cy < (20 + _CurrRightPaneHeight))
			{
				cy  = 20 + _CurrRightPaneHeight ;
				blocked = true ;
			}
		}
		else
		{
			if (cx < 120) 
			{
				blocked = true ;
				cx = 120 ;
			}
			if (cy < 80) 
			{
				blocked = true ;
				cy = 80 ;		
			}
		}*/


		CRect r = getTreeRect(cx, cy) ;			
		_ParticleTreeCtrl->MoveWindow(&r) ;

		if (_CurrentRightPane)
		{								
			_CurrentRightPane->MoveWindow(r.right + 10, r.top, r.right + _CurrRightPaneWidth + 10, r.top + _CurrRightPaneHeight) ;
		}
		

	
		CDialog::OnSize(nType, cx, cy);

	
	}
}


CRect CParticleDlg::getTreeRect(int cx, int cy) const
{
	const uint ox = 10, oy = 10 ;

	if (_CurrentRightPane)
	{		
		CRect res(ox, oy, cx - _CurrRightPaneWidth - 10, cy - 10) ; 
		return res ;
	}
	else
	{
		CRect res(ox, oy, cx - 10, cy - 10) ;
		return res ;
	}
}

void CParticleDlg::setRightPane(CWnd *pane)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	delete _CurrentRightPane ;
	_CurrentRightPane = pane ;
	RECT r ;	
	if (pane)
	{
		
		pane->ShowWindow(SW_SHOW) ;
	
	
		_CurrentRightPane->GetClientRect(&r) ;

		_CurrRightPaneWidth = r.right ;
		_CurrRightPaneHeight = r.bottom ;
	
	}

	GetClientRect(&r) ;
	this->SendMessage(WM_SIZE, SIZE_RESTORED, r.right + (r.bottom << 16)) ;	
	GetWindowRect(&r) ;
	this->MoveWindow(&r) ;
	if (_CurrentRightPane)
	{
		_CurrentRightPane->Invalidate() ;
	}
	this->Invalidate() ;
	_ParticleTreeCtrl->Invalidate() ;
}



LRESULT CParticleDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{

	if (message == WM_GETMINMAXINFO)
	{
		sint cx = 150, cy = 150 ;
		if (_CurrentRightPane)
		{
			RECT r ;
			_CurrentRightPane->GetClientRect(&r) ;
			cx += _CurrRightPaneWidth ;
			if (cy < (_CurrRightPaneHeight + 20) ) cy = _CurrRightPaneHeight + 20 ;
		}


		MINMAXINFO *inf = 	(MINMAXINFO *) lParam ;
		inf->ptMinTrackSize.x = cx ;
		inf->ptMinTrackSize.y = cy ;

	}
	
	return CDialog::WindowProc(message, wParam, lParam);
	
}

void CParticleDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	_StartStopDlg->ShowWindow(bShow) ;	
		
}
