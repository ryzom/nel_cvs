/** \file particle_tree_ctrl.cpp
 * <File description>
 *
 * $Id: particle_tree_ctrl.cpp,v 1.4 2001/06/15 16:24:45 corvazier Exp $
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
#include "emitter_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 


#include "3d/particle_system.h"
#include "3d/particle_system_model.h"
#include "3d/particle_system_shape.h"
#include "3d/ps_located.h"
#include "3d/ps_particle.h"
#include "3d/ps_force.h"
#include "3d/ps_zone.h"
#include "3d/ps_emitter.h"
#include "3d/nelu.h"

#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include "located_properties.h"
#include "located_bindable_dialog.h"
#include "located_target_dlg.h"
#include "scene_dlg.h"
#include "object_viewer.h"


using NL3D::CParticleSystem ;
using NL3D::CParticleSystemModel ;
using NL3D::CPSLocated ;
using NL3D::CPSLocatedBindable ;




/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl


static const uint IconIDs[] = { IDB_FORCE, IDB_PARTICLE, IDB_EMITTER, IDB_LIGHT, IDB_PARTICLE, IDB_PARTICLE_SYSTEM, IDB_LOCATED, IDB_LOCATED_INSTANCE } ;
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
	for (std::vector<CNodeType *>::iterator it = _NodeTypes.begin() ; it != _NodeTypes.end() ; ++it)
	{
		delete *it ;
	}
}


void CParticleTreeCtrl::rebuildLocatedInstance(void)
{
	HTREEITEM currPS = GetRootItem(), currLocated ;

	currLocated = this->GetChildItem(currPS) ;

	while(currLocated)
	{
		CNodeType *nt = (CNodeType *) GetItemData(currLocated) ;
		nlassert(nt->Type == CNodeType::located) ;
		CPSLocated *loc = nt->Loc ;

		for (uint32 k = 0 ; k < loc->getSize() ; ++k)
		{
			CNodeType *newNt = new CNodeType(loc, k) ;
			_NodeTypes.push_back(newNt) ;
			InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, "instance", 7, 7, 0, 0, (LPARAM) newNt, currLocated, TVI_LAST) ;
		}
		currLocated = GetNextItem(currLocated, TVGN_NEXT) ;
	}
}


void CParticleTreeCtrl::buildTreeFromPS(CParticleSystem *ps, CParticleSystemModel *psm)
{
	// for now, there's only one root ...
			
	CNodeType *nt = new CNodeType(ps, psm) ;
	_NodeTypes.push_back(nt) ;

	HTREEITEM rootId =  InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE, NULL, 5, 5, 0, 0, NULL, TVI_ROOT, TVI_LAST) ;

	// set the param (doesn't seems to work during first creation)


	SetItemData(rootId, (LPARAM) nt) ;

	// now, create each located


	HTREEITEM lastLocatedChild = 0, lastBoundObject ;


	uint k , l ;

	for (k = 0 ; k < ps->getNbProcess() ; k++)
	{
		// for now, we only have located
		nlassert(dynamic_cast<CPSLocated *>(ps->getProcess(k))) ;
		CPSLocated *loc = (CPSLocated *) ps->getProcess(k) ;


		
		// insert an item for the located
		nt = new CNodeType(loc) ;
		_NodeTypes.push_back(nt) ;
		lastLocatedChild =  InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM |TVIF_TEXT, loc->getName().c_str() , 6, 6, 0, 0, (LPARAM) nt, rootId, TVI_LAST) ;


		// now, insert each object that is bound to the located

		lastBoundObject = 0 ;

		for (l = 0 ; l < loc->getNbBoundObjects() ; ++l)
		{
			CPSLocatedBindable *lb = loc->getBoundObject(l) ;			
			// we ordered the image so that they match the type for a located bindable (force, particles, collision zones...)

			nt = new CNodeType(lb) ;
			_NodeTypes.push_back(nt) ;
			lastBoundObject = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT , lb->getName().c_str() , lb->getType(), lb->getType(), 0, 0, (LPARAM) nt, lastLocatedChild, TVI_LAST) ;
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

	static NL3D::CParticleSystem *lastClickedPS = NULL ;
	

	CNodeType *nt = (CNodeType  *) pNMTreeView->itemNew.lParam ;
	

	nlassert(nt) ;
		
	switch (nt->Type)
	{
		case CNodeType::located:
		{	
			CLocatedProperties *lp = new CLocatedProperties(nt->Loc) ;
			lp->init(0, 0, _ParticleDlg) ;	
			_ParticleDlg->setRightPane(lp) ;
			if (lastClickedPS)
			{
				lastClickedPS->setCurrentEditedElement(NULL) ;
			}
			return ;
		}
		break ;
		case CNodeType::locatedBindable:
		{
			if (dynamic_cast<NL3D::CPSEmitter *>(nt->Bind))
			{
				CEmitterDlg *ed =  new CEmitterDlg(dynamic_cast<NL3D::CPSEmitter *>(nt->Bind)) ;
				ed->init(_ParticleDlg) ;
				_ParticleDlg->setRightPane(ed) ;
			}
			else
			if (dynamic_cast<NL3D::CPSTargetLocatedBindable *>(nt->Bind))
			{
				CLocatedTargetDlg *ltd =  new CLocatedTargetDlg(dynamic_cast<NL3D::CPSTargetLocatedBindable *>(nt->Bind)) ;
				ltd->init(_ParticleDlg) ;
				_ParticleDlg->setRightPane(ltd) ;
			}
			else
			{
				CLocatedBindableDialog *lbd = new CLocatedBindableDialog(nt->Bind) ;
				lbd->init(_ParticleDlg) ;
				_ParticleDlg->setRightPane(lbd) ;			
			}

			if (lastClickedPS)
			{
				lastClickedPS->setCurrentEditedElement(NULL) ;
			}

			return ;
		}
		break ;
		case CNodeType::particleSystem:
		{
			_ParticleDlg->setRightPane(NULL) ;
			if (lastClickedPS)
			{
				lastClickedPS->setCurrentEditedElement(NULL) ;
			}
		}
		break ;
		case CNodeType::locatedInstance:
		{
			_ParticleDlg->setRightPane(NULL) ;
			NL3D::CParticleSystem *ps = nt->Loc->getOwner() ;
			ps->setCurrentEditedElement(nt->Loc, nt->LocatedInstanceIndex) ;
			CObjectViewer *ov = _ParticleDlg->SceneDlg->ObjView ;
			ov->getMouseListener().setModelMatrix(_ParticleDlg->getElementMatrix()) ;
			lastClickedPS = ps ;
		}
		break ;
	}
	
	
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

		CNodeType *nt =  (CNodeType *) item.lParam ;



		switch (nt->Type)
		{
			case CNodeType::located:
				 menu.LoadMenu(IDR_LOCATED_MENU) ;
			break ;
			case CNodeType::locatedBindable:
				menu.LoadMenu(IDR_LOCATED_BINDABLE_MENU) ;		 
			break ;
			case CNodeType::particleSystem:
				 menu.LoadMenu(IDR_PARTICLE_SYSTEM_MENU) ;	
			break ;
			default:
				return ;
			break ;
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
			_ParticleDlg->setRightPane(NULL) ;
			TVITEM item ;
			item.mask = TVIF_HANDLE | TVIF_PARAM ;
			item.hItem = GetSelectedItem() ;
			GetItem(&item) ;
			
			CNodeType *nt = (CNodeType *) item.lParam ;
			CPSLocated *loc = nt->Loc ;			
			CParticleSystem *ps = _ParticleDlg->getCurrPS() ;
			ps->remove(loc) ;	
			DeleteItem(item.hItem) ;			
			delete nt ;

			_NodeTypes.erase(std::find(_NodeTypes.begin(), _NodeTypes.end(), nt)) ;
			return TRUE ;
			
			//return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}
		break ;


		case IDM_DELETE_LOCATED_BINDABLE:
		{
			_ParticleDlg->setRightPane(NULL) ;
			TVITEM item ;
			item.mask = TVIF_HANDLE | TVIF_PARAM ;
			item.hItem = GetSelectedItem() ;
			GetItem(&item) ;	
			
			CNodeType *nt = (CNodeType *) item.lParam ;
			CPSLocatedBindable *lb = nt->Bind ;			

			// now, get the father

			item.hItem = GetParentItem(GetSelectedItem()) ;
			GetItem(&item) ;
			

			
			CPSLocated *loc = ((CNodeType *) item.lParam)->Loc ;	
			loc->remove(lb) ;

			DeleteItem(GetSelectedItem()) ;			
			
			delete nt ; 
			_NodeTypes.erase(std::find(_NodeTypes.begin(), _NodeTypes.end(), nt)) ;

			return TRUE ;			
		}
		break ;


		// instanciate an element
		case ID_INSTANCIATE_LOCATED:
		{
			TVITEM item ;
			item.mask = TVIF_HANDLE | TVIF_PARAM ;
			item.hItem = GetSelectedItem() ;
			GetItem(&item) ;

			CNodeType *nt = (CNodeType *) item.lParam ;
			CPSLocated *loc = nt->Loc ;	
			if (loc->getSize() == loc->getMaxSize())
			{
				loc->resize(loc->getMaxSize() + 1) ;
				// force a re-update of the left pane
				NM_TREEVIEW nmt ;
				LRESULT pResult ;
				nmt.itemNew.lParam = GetItemData(GetSelectedItem()) ;
				OnSelchanged((NMHDR *) &nmt, &pResult) ;
			}

			sint32 objIndex = loc->newElement() ;

			nt = new CNodeType(loc, objIndex) ;
			_NodeTypes.push_back(nt) ;
			// insert the element in the tree
			InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, "instance", 7, 7, 0, 0, (LPARAM) nt, GetSelectedItem(), TVI_LAST) ;
			Invalidate() ;
		}
		break ;

		////////////////////////
		// PARTICLE SYSTEM OP //
		////////////////////////
		
		case ID_MENU_NEWLOCATED:
		{
			std::string name ; 
			char num[128] ;
			if (_PSElementIdentifiers.count(std::string("located")))
			{
				sprintf(num, "%d", ++_PSElementIdentifiers[std::string("located")]) ;
			}
			else
			{
				name = std::string("located 0") ;
			}		

			TVITEM item ;
			item.mask = TVIF_HANDLE | TVIF_PARAM ;
			item.hItem = GetSelectedItem() ;

			GetItem(&item) ;

			CPSLocated *loc = new CPSLocated ;
			loc->setName(name) ;
			CParticleSystem *ps = ((CNodeType *) item.lParam)->PS ;
			ps->attach(loc) ;

			CNodeType *nt = new CNodeType(loc) ;
			_NodeTypes.push_back(nt) ;
			// insert item in tree
			InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, name.c_str(), 6, 6, 0, 0, (LPARAM) nt, GetSelectedItem(), TVI_LAST) ;
			Invalidate() ;
		}
		break ;
		case ID_MENU_LOAD_PS:
		{
				CFileDialog fd(TRUE, "ps", NULL, 0, NULL, this) ;
				if (fd.DoModal() == IDOK)
				{
					// Add to the path
					char drive[256];
					char dir[256];
					char path[256];

					// Add search path for the texture
					_splitpath (fd.GetPathName(), drive, dir, NULL, NULL);
					_makepath (path, drive, dir, NULL, NULL);
					NLMISC::CPath::addSearchPath (path);


					CNodeType *nt = (CNodeType *) GetItemData(GetSelectedItem()) ;
					NL3D::CNELU::Scene.deleteInstance(nt->PSModel) ;					
				
					nt->PSModel = dynamic_cast<CParticleSystemModel *>(NL3D::CNELU::Scene.createInstance(std::string((LPCTSTR) fd.GetFileName()))) ;
					nt->PS = nt->PSModel->getPS() ;									
					nt->PSModel->setEllapsedTime(0.f) ; // system is paused
					nt->PSModel->enableDisplayTools(true) ;
				
					_ParticleDlg->setRightPane(NULL) ;
					_ParticleDlg->setNewCurrPS(nt->PS, nt->PSModel) ;

					nt->PS->setFontManager(_ParticleDlg->FontManager) ;
					nt->PS->setFontGenerator(_ParticleDlg->FontGenerator) ;

					DeleteItem(TVI_ROOT) ;
					buildTreeFromPS(nt->PS, nt->PSModel) ;
					rebuildLocatedInstance() ;
				}

		}
		break ;
		case ID_MENU_SAVE_PS:
		{
			
				CFileDialog fd(FALSE, "ps", NULL, 0, NULL, this) ;
				if (fd.DoModal() == IDOK)
				{
					// Add to the path
					char drive[256];
					char dir[256];
					char path[256];

					// Add search path for the texture
					_splitpath (fd.GetPathName(), drive, dir, NULL, NULL);
					_makepath (path, drive, dir, NULL, NULL);
					NLMISC::CPath::addSearchPath (path);


					CNodeType *nt = (CNodeType *) GetItemData(GetSelectedItem()) ;

					// build a shape from our system, and save it
					NL3D::CParticleSystemShape psc  ;

					try
					{
						psc.buildFromPS(*nt->PS) ;
						{
							NL3D::CShapeStream st(&psc) ;
							NLMISC::COFile oFile((LPCTSTR) fd.GetPathName()) ;
							oFile.serial(st) ;
						}
					}
					catch (NLMISC::Exception &E)
					{
						MessageBox(E.what(), "save error") ;
					}	
				}
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
		CPSLocated *loc = ((CNodeType *) item.lParam)->Loc ;
		loc->bind(toCreate) ;
		CNodeType *nt = new CNodeType(toCreate) ;
		_NodeTypes.push_back(nt) ;
		// insert the element in the tree
		// we want that the instance always appears in the last position

		HTREEITEM son = this->GetChildItem(GetSelectedItem()) ;
		HTREEITEM lastSon ;

		if (!son)
		{
			lastSon = GetSelectedItem() ;
		}
		else
		{

			lastSon  = TVI_FIRST ;

			while (son != NULL)
			{
				item.hItem = son ;
				GetItem(&item) ;
				if (((CNodeType *) item.lParam)->Type == CNodeType::locatedInstance)
				{
					break ;
				}
				lastSon = son ;
				son = this->GetChildItem(son) ;
			}
		}


		InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, toCreate->getName().c_str(), toCreate->getType(), toCreate->getType(), 0, 0, (LPARAM) nt, GetSelectedItem(), lastSon) ;
		Invalidate() ;
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

		CNodeType *nt = (CNodeType *) info->item.lParam ;

		switch (nt->Type)
		{
			case CNodeType::located :
			{
				nt->Loc->setName(std::string(info->item.pszText)) ;
			}
			break ;
			case CNodeType::locatedBindable :
			{
				nt->Bind->setName(std::string(info->item.pszText)) ;
			}
			break ;
		}

	}
}


void CParticleTreeCtrl::moveElement(const NLMISC::CMatrix &mat)
{
	
	// the current element must be an instance
	if (::IsWindow(m_hWnd))
	{
		HTREEITEM currItem = GetSelectedItem() ;
		if (currItem)
		{
			CNodeType *nt = (CNodeType *) GetItemData(currItem) ;
			if (nt->Type == CNodeType::locatedInstance)
			{
				nt->Loc->getPos()[nt->LocatedInstanceIndex] = mat.getPos() ;
			}
		}
	}
}



NLMISC::CMatrix CParticleTreeCtrl::getElementMatrix(void) const
{
	HTREEITEM currItem = GetSelectedItem() ;
	if (currItem)
	{
		CNodeType *nt = (CNodeType *) GetItemData(currItem) ;
		if (nt->Type == CNodeType::locatedInstance)
		{
			NLMISC::CVector pos = nt->Loc->getPos()[nt->LocatedInstanceIndex] ;
			NLMISC::CMatrix m ;
			m.identity() ;
			m.setPos(pos) ;
			return m ; 
		}
	}

	return NLMISC::CMatrix::Identity ;
}
