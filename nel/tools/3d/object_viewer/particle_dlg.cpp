/** \file particle_dlg.cpp
 * The main dialog for particle system edition. If holds a tree constrol describing the system structure,
 * and show the properties of the selected object
 *
 * $Id: particle_dlg.cpp,v 1.7 2001/06/26 11:59:37 vizerie Exp $
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
	: CDialog(CParticleDlg::IDD, pParent), SceneDlg(sceneDlg), CurrentRightPane(NULL)
{
	//{{AFX_DATA_INIT(CParticleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT





	// Font manager
	FontManager = new NL3D::CFontManager ;	
	FontManager->setMaxMemory(2000000);
		
	uint dSize = ::GetWindowsDirectory(NULL, 0) ;
	nlverify(dSize) ;
	char *wd = new char[dSize] ;	
	nlverify(::GetWindowsDirectory(wd, dSize)) ;

	std::string wds(wd) ;
	wds+="\\fonts\\arial.ttf" ;

	// Font generator
	FontGenerator = new NL3D::CFontGenerator ( wds.c_str() ) ;

	delete[] wd ;



	//////

	NL3D::IModel *model = CNELU::Scene.createModel(NL3D::ParticleSystemModelId) ;

	nlassert(dynamic_cast<CParticleSystemModel *>(model)) ;
	_CurrSystemModel = (CParticleSystemModel *) model ;
	_CurrSystemModel->setParticleSystem(new NL3D::CParticleSystem) ;
	ParticleTreeCtrl = new CParticleTreeCtrl(this) ;

	_CurrSystemModel->enableDisplayTools() ;
	_CurrSystemModel->setEllapsedTime(0.f) ;
	_CurrPS = _CurrSystemModel->getPS() ;

	_CurrPS->setFontManager(FontManager) ;
	_CurrPS->setFontGenerator(FontGenerator) ;

	StartStopDlg = new CStartStopParticleSystem(this) ;
	
	#ifdef NL_DEBUG

//	_CrtSetDbgFlag (_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF) ;
	#endif

}


void CParticleDlg::moveElement(const NLMISC::CMatrix &mat)
{
	ParticleTreeCtrl->moveElement(mat) ;
}

NLMISC::CMatrix CParticleDlg::getElementMatrix(void) const
{
	return ParticleTreeCtrl->getElementMatrix() ;
}



CParticleDlg::~CParticleDlg()
{
	//NL3D::CNELU::Scene.deleteInstance(_CurrSystemModel) ;

	delete ParticleTreeCtrl ;
	delete CurrentRightPane ;

	delete StartStopDlg ;

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
	
	ParticleTreeCtrl->Create(WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER
   | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS | TVS_EDITLABELS 
   | TVS_DISABLEDRAGDROP , r, this, 0x1005) ;


	ParticleTreeCtrl->buildTreeFromPS(_CurrPS, _CurrSystemModel) ;
	ParticleTreeCtrl->init() ;
	ParticleTreeCtrl->ShowWindow(SW_SHOW) ;


	StartStopDlg->Create(IDD_PARTICLE_SYSTEM_START_STOP, this) ;	



	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}




void CParticleDlg::OnSize(UINT nType, int cx, int cy) 
{	
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	bool blocked = false ;

	if (ParticleTreeCtrl->m_hWnd && this->m_hWnd)
	{	

	/*	if (_CurrentRightPane)
		{
			if (cx < (120 + CurrRightPaneWidth))
			{
				cx = 120 + CurrRightPaneWidth ;
				blocked = true ;
			}
			if (cy < (20 + CurrRightPaneHeight))
			{
				cy  = 20 + CurrRightPaneHeight ;
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
		ParticleTreeCtrl->MoveWindow(&r) ;

		if (CurrentRightPane)
		{								
			CurrentRightPane->MoveWindow(r.right + 10, r.top, r.right + CurrRightPaneWidth + 10, r.top + CurrRightPaneHeight) ;
		}
		

	
		CDialog::OnSize(nType, cx, cy);

	
	}
}


CRect CParticleDlg::getTreeRect(int cx, int cy) const
{
	const uint ox = 10, oy = 10 ;

	if (CurrentRightPane)
	{		
		CRect res(ox, oy, cx - CurrRightPaneWidth - 10, cy - 10) ; 
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
	if (CurrentRightPane)
	{
		CurrentRightPane->DestroyWindow() ;
	}
	delete CurrentRightPane ;
	CurrentRightPane = pane ;
	RECT r ;	
	if (pane)
	{
		
		pane->ShowWindow(SW_SHOW) ;
	
	
		CurrentRightPane->GetClientRect(&r) ;

		CurrRightPaneWidth = r.right ;
		CurrRightPaneHeight = r.bottom ;
	
	}

	GetClientRect(&r) ;
	this->SendMessage(WM_SIZE, SIZE_RESTORED, r.right + (r.bottom << 16)) ;	
	GetWindowRect(&r) ;
	this->MoveWindow(&r) ;
	if (CurrentRightPane)
	{
		CurrentRightPane->Invalidate() ;
	}
	this->Invalidate() ;
	ParticleTreeCtrl->Invalidate() ;
}



LRESULT CParticleDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{

	if (message == WM_GETMINMAXINFO)
	{
		sint cx = 150, cy = 150 ;
		if (CurrentRightPane)
		{
			RECT r ;
			CurrentRightPane->GetClientRect(&r) ;
			cx += CurrRightPaneWidth ;
			if (cy < (CurrRightPaneHeight + 20) ) cy = CurrRightPaneHeight + 20 ;
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
	StartStopDlg->ShowWindow(bShow) ;	
		
}
