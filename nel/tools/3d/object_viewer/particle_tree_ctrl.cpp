/** \file particle_tree_ctrl.cpp
 * <File description>
 *
 * $Id: particle_tree_ctrl.cpp,v 1.1 2001/06/12 08:39:50 vizerie Exp $
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


// particle_tree_ctrl.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "particle_tree_ctrl.h"
#include "located_bindable_dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



#include "nel/3d/particle_system.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_zone.h"
#include "nel/3d/ps_emitter.h"

#include "located_properties.h"
#include "located_bindable_dialog.h"


using NL3D::CParticleSystem ;
using NL3D::CPSLocated ;
using NL3D::CPSLocatedBindable ;




/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl


static const uint IconIDs[] = { IDB_FORCE, IDB_PARTICLE, IDB_EMITTER, IDB_LIGHT, IDB_PARTICLE, IDB_PARTICLE_SYSTEM, IDB_LOCATED } ;
static const uint NumIconIDs = sizeof(IconIDs) / sizeof(uint) ;


// this map is used to create increasing names
static std::map<std::string, uint> _PSElementIdentifiers ;

CParticleTreeCtrl::CParticleTreeCtrl(CParticleDlg *pdlg) : _ParticleDlg(pdlg)
{


	

	CBitmap bm[NumIconIDs] ;
	_ImageList.Create(16, 16, ILC_COLOR4, 0, NumIconIDs) ;
	

	for (uint k = 0 ; k  < NumIconIDs ; ++k)
	{
		bm[k].LoadBitmap(IconIDs[k]);
		_ImageList.Add(&bm[k], RGB(1, 1, 1));
	}

}

CParticleTreeCtrl::~CParticleTreeCtrl()
{
}



void CParticleTreeCtrl::buildTreeFromPS(CParticleSystem *ps)
{
	// for now, there's only one root ...
			
	HTREEITEM rootId =  InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE, NULL, 5, 5, 0, 0, 0, TVI_ROOT, TVI_LAST) ;




	// now, create each located


	HTREEITEM lastLocatedChild = 0, lastBoundObject ;


	uint k , l ;

	for (k = 0 ; k < ps->getNbProcess() ; k++)
	{
		// for now, we only have located
		nlassert(dynamic_cast<CPSLocated *>(ps->getProcess(k))) ;
		CPSLocated *loc = (CPSLocated *) ps->getProcess(k) ;


		
		// insert an item for the located
		lastLocatedChild =  InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM |TVIF_TEXT, loc->getName().c_str() , 6, 6, 0, 0, (LPARAM) loc, rootId, TVI_LAST) ;


		// now, insert each object that is bound to the located

		lastBoundObject = 0 ;

		for (l = 0 ; l < loc->getNbBoundObjects() ; ++l)
		{
			CPSLocatedBindable *lb = loc->getBoundObject(l) ;			
			// we ordered the image so that they match the type for a located bindable (force, particles, collision zones...)

			lastBoundObject = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT , lb->getName().c_str() , lb->getType(), lb->getType(), 0, 0, (LPARAM) lb, lastLocatedChild, TVI_LAST) ;
		}
	}
}


BEGIN_MESSAGE_MAP(CParticleTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CParticleTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CParticleTreeCtrl::init(void)
{
	this->SetImageList(&_ImageList, TVSIL_NORMAL) ;
}

/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl message handlers


void CParticleTreeCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	
	*pResult = 0;


	NLMISC::IStreamable *obj = ((NLMISC::IStreamable *) pNMTreeView->itemNew.lParam) ;
	
	// we use dynamic cast to see what kind of object we are dealing with
		
	if (dynamic_cast<CPSLocated *>(obj))
	{		
		CLocatedProperties *lp = new CLocatedProperties(dynamic_cast<CPSLocated *>(obj)) ;
		lp->init(0, 0, _ParticleDlg) ;	
		_ParticleDlg->setLeftPane(lp) ;
		return ;
	}
	else if (dynamic_cast<CPSLocatedBindable *>(obj))
	{		
		CLocatedBindableDialog *lbd = new CLocatedBindableDialog(dynamic_cast<CPSLocatedBindable *>(obj)) ;
		lbd->init(_ParticleDlg) ;
		_ParticleDlg->setLeftPane(lbd) ;
		return ;
	}

	_ParticleDlg->setLeftPane(NULL) ;
}



void CParticleTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

//	CTreeCtrl::OnRButtonDown(nFlags, point);

	// test wether there is an item under that point
	UINT flags ;
	HTREEITEM item  = this->HitTest(point, &flags) ;
	if (item)
	{
		this->SelectItem(item) ;
		RECT r ;
		GetWindowRect(&r) ;

		CMenu  menu ;
		CMenu* subMenu ;

		TVITEM item ;
		item.mask = TVIF_HANDLE | TVIF_PARAM ;
		item.hItem = GetSelectedItem() ;
		GetItem(&item) ;

		NLMISC::IClassable *obj =  (NLMISC::IClassable *) item.lParam ;




		if (dynamic_cast<CPSLocated *>(obj))
		{		   
		   menu.LoadMenu(IDR_LOCATED_MENU) ;		   		
		}
		else
		if (dynamic_cast<CPSLocatedBindable *>(obj))
		{
		   
		   menu.LoadMenu(IDR_LOCATED_BINDABLE_MENU) ;		   		
		}
		else
		{
			return ;
		}


		subMenu = menu.GetSubMenu(0);    
		nlassert(subMenu) ;
		subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, r.left + point.x, r.top + point.y, this) ;


	}
	

}

BOOL CParticleTreeCtrl::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (nCode != 0) return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	CPSLocatedBindable *toCreate = NULL ;
	switch(nID)
	{
		///////////////
		// particles //
		///////////////
		case IDM_DOT:
			toCreate = new NL3D::CPSDot ;
		break ;
		case IDM_LOOKAT:
			toCreate = new NL3D::CPSFaceLookAt ; 
		break ;
		case IDM_FANLIGHT:
			toCreate = new NL3D::CPSFanLight ;
		break ;
		case IDM_RIBBON:
			toCreate = new NL3D::CPSRibbon ;
		break ;
		case IDM_TAILDOT:
			toCreate = new NL3D::CPSTailDot ; 
		break ;
		case IDM_MESH:
			toCreate = new NL3D::CPSMesh ;
		break ;
		case IDM_CONSTRAINT_MESH:
			toCreate = new NL3D::CPSConstraintMesh ;
		break ;
		case IDM_FACE:
			toCreate = new NL3D::CPSFace ;
		break ;

		//////////////
		// emitters //
		//////////////

		case IDM_DIRECTIONNAL_EMITTER:
			toCreate = new NL3D::CPSEmitterDirectionnal ;
		break ;
		case IDM_OMNIDIRECTIONNAL_EMITTER:
			toCreate = new NL3D::CPSEmitterOmni ;
		break ;
		case IDM_CONIC_EMITTER:
			toCreate = new NL3D::CPSEmitterConic ;
		break ; 
		case IDM_RECTANGLE_EMITTER:
			toCreate = new NL3D::CPSEmitterRectangle ;
		break ;


		////////////////
		//   Zones    //
		////////////////

		case IDM_ZONE_PLANE:
			toCreate = new NL3D::CPSZonePlane ;
		break ;
		case IDM_ZONE_SPHERE:
			toCreate = new NL3D::CPSZoneSphere ;
		break ;
		case IDM_ZONE_DISC:
			toCreate = new NL3D::CPSZoneDisc ;
		break ;
		case IDM_ZONE_RECTANGLE:
			toCreate = new NL3D::CPSZoneRectangle ;
		break ;
		case IDM_ZONE_CYLINDER:
			toCreate = new NL3D::CPSZoneCylinder ;
		break ;

		///////////////
		//   forces  //
		///////////////
		case IDM_GRAVITY_FORCE:
			toCreate = new NL3D::CPSGravity ;
		break ;
		case IDM_SPRING_FORCE:
			toCreate = new NL3D::CPSSpring ;
		break ;

		//////////////
		// deletion //
		//////////////
		case IDM_DELETE_LOCATED:
		{
			_ParticleDlg->setLeftPane(NULL) ;
			TVITEM item ;
			item.mask = TVIF_HANDLE | TVIF_PARAM ;
			item.hItem = GetSelectedItem() ;
			GetItem(&item) ;
			nlassert(dynamic_cast<CPSLocated *>((NLMISC::IClassable *) item.lParam)) ;
			CPSLocated *loc = (CPSLocated *) item.lParam ;			
			CParticleSystem *ps = _ParticleDlg->getCurrPS() ;
			ps->remove(loc) ;	
			DeleteItem(item.hItem) ;			
			return TRUE ;
			
			//return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}
		break ;


		case IDM_DELETE_LOCATED_BINDABLE:
		{
			_ParticleDlg->setLeftPane(NULL) ;
			TVITEM item ;
			item.mask = TVIF_HANDLE | TVIF_PARAM ;
			item.hItem = GetSelectedItem() ;
			GetItem(&item) ;
			nlassert(dynamic_cast<CPSLocatedBindable *>((NLMISC::IClassable *) item.lParam)) ;
			CPSLocatedBindable *lb = (CPSLocatedBindable *) item.lParam ;			

			// now, get the father

			item.hItem = GetParentItem(GetSelectedItem()) ;
			GetItem(&item) ;
			

			nlassert(dynamic_cast<CPSLocated *>((NLMISC::IClassable *) item.lParam)) ;
			CPSLocated *loc = (CPSLocated *) item.lParam ;	
			loc->remove(lb) ;

			DeleteItem(GetSelectedItem()) ;			
			return TRUE ;			
		}
		break ;
	}


	if (toCreate)
	{
		// complete the name
		std::string name = toCreate->getName() ;

		char num[128] ;

		if (_PSElementIdentifiers.count(name))
		{			 
			sprintf(num, "%d", ++_PSElementIdentifiers[name]) ;
			toCreate->setName(name + num) ;
		}
		else
		{
			_PSElementIdentifiers[toCreate->getName()] = 0 ;
			toCreate->setName(name + "0") ;
		}

		TVITEM item ;
		item.mask = TVIF_HANDLE | TVIF_PARAM ;
		item.hItem = GetSelectedItem() ;

	


		GetItem(&item) ;
		CPSLocated *loc = (CPSLocated *) item.lParam ;
		loc->bind(toCreate) ;
		// insert the element in the tree
		InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, toCreate->getName().c_str(), toCreate->getType(), toCreate->getType(), 0, 0, (LPARAM) toCreate, GetSelectedItem(), TVI_LAST) ;
	}











	return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/// The user finished to edit a label in the tree
 
void CParticleTreeCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMTVDISPINFO* info = (NMTVDISPINFO*)pNMHDR;

	*pResult = 0;

	if (info->item.pszText)
	{
		this->SetItemText(info->item.hItem, info->item.pszText) ;
		NLMISC::IClassable *obj  = (NLMISC::IClassable *) info->item.lParam ;

		if (dynamic_cast<CPSLocated *>(obj))
		{
			((CPSLocated *) obj)->setName(std::string(info->item.pszText)) ;
		}
		else if (dynamic_cast<CPSLocatedBindable *>(obj))
		{
			((CPSLocatedBindable *) obj)->setName(std::string(info->item.pszText)) ;
		}
	}
}
