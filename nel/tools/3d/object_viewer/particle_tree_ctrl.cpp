/** \file particle_tree_ctrl.cpp
 * shows the structure of a particle system
 *
 * $Id: particle_tree_ctrl.cpp,v 1.33 2002/04/25 08:30:54 vizerie Exp $
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
#include "main_frame.h"
#include "particle_system_edit.h"
#include "particle_dlg.h"
#include "start_stop_particle_system.h"
#include "edit_ps_sound.h"
#include "dup_ps.h"
 


#include "3d/particle_system.h"
#include "3d/particle_system_model.h"
#include "3d/particle_system_shape.h"
#include "3d/ps_located.h"
#include "3d/ps_particle.h"
#include "3d/ps_mesh.h"
#include "3d/ps_force.h"
#include "3d/ps_zone.h"
#include "3d/ps_sound.h"
#include "3d/ps_emitter.h"
#include "3d/ps_edit.h"
#include "3d/nelu.h"

#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include "located_properties.h"
#include "located_bindable_dialog.h"
#include "located_target_dlg.h"
#include "lb_extern_id_dlg.h"

#include "object_viewer.h"
#include "ps_mover_dlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



using NL3D::CParticleSystem;
using NL3D::CParticleSystemModel;
using NL3D::CPSLocated;
using NL3D::CPSLocatedBindable;




/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl


static const uint IconIDs[] = { IDB_FORCE, IDB_PARTICLE, IDB_EMITTER, IDB_LIGHT, IDB_COLLISION_ZONE, IDB_SOUND
								, IDB_PARTICLE_SYSTEM, IDB_LOCATED, IDB_LOCATED_INSTANCE };
static const uint NumIconIDs = sizeof(IconIDs) / sizeof(uint);


// this map is used to create increasing names
static std::map<std::string, uint> _PSElementIdentifiers;


//=====================================================================================================
CParticleTreeCtrl::CParticleTreeCtrl(CParticleDlg *pdlg) : _ParticleDlg(pdlg), _LastClickedPS(NULL)
{
	CBitmap bm[NumIconIDs];
	_ImageList.Create(16, 16, ILC_COLOR4, 0, NumIconIDs);
	for (uint k = 0; k  < NumIconIDs; ++k)
	{
		bm[k].LoadBitmap(IconIDs[k]);
		_ImageList.Add(&bm[k], RGB(1, 1, 1));
	}
}

//=====================================================================================================
CParticleTreeCtrl::~CParticleTreeCtrl()
{
	reset();	
}

//=====================================================================================================
void CParticleTreeCtrl::reset()
{
	for (std::vector<CNodeType *>::iterator it = _NodeTypes.begin(); it != _NodeTypes.end(); ++it)
	{
		delete *it;
	}
	_NodeTypes.clear();
}


//=====================================================================================================
void CParticleTreeCtrl::rebuildLocatedInstance(void)
{
	HTREEITEM currPS = GetRootItem(), currLocated;

	currLocated = this->GetChildItem(currPS);

	while(currLocated)
	{
		CNodeType *nt = (CNodeType *) GetItemData(currLocated);
		nlassert(nt->Type == CNodeType::located);
		CPSLocated *loc = nt->Loc;

		for (uint32 k = 0; k < loc->getSize(); ++k)
		{
			CNodeType *newNt = new CNodeType(loc, k);
			_NodeTypes.push_back(newNt);
			// bind located instance icon
			InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, "instance", 8, 8, 0, 0, (LPARAM) newNt, currLocated, TVI_LAST);
		}
		currLocated = GetNextItem(currLocated, TVGN_NEXT);
	}
	Invalidate();
}

//=====================================================================================================
void CParticleTreeCtrl::suppressLocatedInstanceNbItem(uint32 newSize)
{
	HTREEITEM currPS = GetRootItem(), currLocated, currLocElement, nextCurrLocElement;
	currLocated = this->GetChildItem(currPS);
	while(currLocated)
	{				
		currLocElement = GetChildItem(currLocated);			
		while (currLocElement)
		{
			CNodeType *nt = (CNodeType *) GetItemData(currLocElement);

			nextCurrLocElement = GetNextItem(currLocElement, TVGN_NEXT);
			// remove instance item
			if (nt->Type == CNodeType::locatedInstance)
			{
				if (nt->LocatedInstanceIndex >= newSize)
				{
					_NodeTypes.erase(std::find(_NodeTypes.begin(), _NodeTypes.end(), nt));
					DeleteItem(currLocElement);					
					delete nt;
				}
			}
			currLocElement = nextCurrLocElement;		
		}
		
		currLocated = GetNextItem(currLocated, TVGN_NEXT);
	}	
	Invalidate();
}


//=====================================================================================================
void CParticleTreeCtrl::buildTreeFromPS(CParticleSystem *ps, CParticleSystemModel *psm)
{	
	reset();
	DeleteAllItems();
	// for now, there's only one root ...			
	CNodeType *nt = new CNodeType(ps, psm);
	_NodeTypes.push_back(nt);
	// bind particle system icon
	HTREEITEM rootHandle =  InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT, ps->getName().c_str(), 6, 6, 0, 0, NULL, TVI_ROOT, TVI_LAST);
	// set the param (doesn't seems to work during first creation)
	SetItemData(rootHandle, (LPARAM) nt);
	// now, create each located		
	for (uint k = 0; k < ps->getNbProcess(); k++)
	{				
		CPSLocated *loc = dynamic_cast<CPSLocated *>(ps->getProcess(k));		
		if (loc) createNodeFromLocated(loc, rootHandle);
	}
}

//=====================================================================================================
void CParticleTreeCtrl::createNodeFromLocated(NL3D::CPSLocated *loc, HTREEITEM rootHandle)
{	
	// insert an item for the located
	CNodeType *nt = new CNodeType(loc);
	_NodeTypes.push_back(nt);
	// bind located icon
	HTREEITEM nodeHandle =  InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM |TVIF_TEXT, loc->getName().c_str() , 7, 7, 0, 0, (LPARAM) nt, rootHandle, TVI_LAST);
	// now, insert each object that is bound to the located	
	for (uint l = 0; l < loc->getNbBoundObjects(); ++l)
	{
		createNodeFromLocatedBindable(loc->getBoundObject(l), nodeHandle);				
	}
}

//=====================================================================================================
void CParticleTreeCtrl::createNodeFromLocatedBindable(NL3D::CPSLocatedBindable *lb, HTREEITEM rootHandle)
{
	// we ordered the image so that they match the type for a located bindable (force, particles, collision zones...)
	CNodeType *nt = new CNodeType(lb);
	_NodeTypes.push_back(nt);
	InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT , lb->getName().c_str() , lb->getType(), lb->getType(), 0, 0, (LPARAM) nt, rootHandle, TVI_LAST);
}



BEGIN_MESSAGE_MAP(CParticleTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CParticleTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=====================================================================================================
void CParticleTreeCtrl::init(void)
{
	this->SetImageList(&_ImageList, TVSIL_NORMAL);
}

/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl message handlers


void CParticleTreeCtrl::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;	
	*pResult = 0;
	CNodeType *nt = (CNodeType  *) pNMTreeView->itemNew.lParam;	
	nlassert(nt);
		
	switch (nt->Type)
	{
		case CNodeType::located:
		{	

			CLocatedProperties *lp = new CLocatedProperties(nt->Loc, _ParticleDlg);			
			lp->init(0, 0);	

			_ParticleDlg->setRightPane(lp);
			if (_LastClickedPS)
			{
				_LastClickedPS->setCurrentEditedElement(NULL);
			}



			return;
		}
		break;
		case CNodeType::locatedBindable:
		{
			if (dynamic_cast<NL3D::CPSEmitter *>(nt->Bind))
			{
				CEmitterDlg *ed =  new CEmitterDlg(static_cast<NL3D::CPSEmitter *>(nt->Bind));
				ed->init(_ParticleDlg);
				_ParticleDlg->setRightPane(ed);
			}
			else
			if (dynamic_cast<NL3D::CPSTargetLocatedBindable *>(nt->Bind))
			{
				CLocatedTargetDlg *ltd =  new CLocatedTargetDlg(static_cast<NL3D::CPSTargetLocatedBindable *>(nt->Bind));
				ltd->init(_ParticleDlg);
				_ParticleDlg->setRightPane(ltd);
			}
			else
			if (dynamic_cast<NL3D::CPSSound *>(nt->Bind))
			{
				CEditPSSound *epss =  new CEditPSSound(static_cast<NL3D::CPSSound *>(nt->Bind));
				epss->init(_ParticleDlg);
				_ParticleDlg->setRightPane(epss);
			}
			else
			{
				CLocatedBindableDialog *lbd = new CLocatedBindableDialog(nt->Bind);
				lbd->init(_ParticleDlg);
				_ParticleDlg->setRightPane(lbd);			
			}

			if (_LastClickedPS)
			{
				_LastClickedPS->setCurrentEditedElement(NULL);
			}



			return;
		}
		break;
		case CNodeType::particleSystem:
		{
			CParticleSystemEdit *pse = new CParticleSystemEdit(nt->PS);
			pse->init(_ParticleDlg);
			_ParticleDlg->setRightPane(pse);
			if (_LastClickedPS)
			{
				_LastClickedPS->setCurrentEditedElement(NULL);
			}
		}
		break;
		case CNodeType::locatedInstance:
		{			
			NL3D::CParticleSystem *ps = nt->Loc->getOwner();
			
			
			
			_LastClickedPS = ps;
			
			CPSMoverDlg *moverDlg = new CPSMoverDlg(this, &_ParticleDlg->MainFrame->ObjView->getMouseListener(), nt->Loc, nt->LocatedInstanceIndex);			
			moverDlg->init(_ParticleDlg);
			_ParticleDlg->setRightPane(moverDlg);


			CObjectViewer *ov = _ParticleDlg->MainFrame->ObjView;

			if(_ParticleDlg->MainFrame->MoveElement)
			{
				ov->getMouseListener().setModelMatrix(_ParticleDlg->getElementMatrix());
			}
			ps->setCurrentEditedElement(nt->Loc, nt->LocatedInstanceIndex, moverDlg->getLocatedBindable());
			
		}
		break;
	}
	

	
}



void CParticleTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{

	if (_LastClickedPS)
	{
		_LastClickedPS->setCurrentEditedElement(NULL);
	}

//	CTreeCtrl::OnRButtonDown(nFlags, point);

	// test wether there is an item under that point
	UINT flags;
	HTREEITEM item  = this->HitTest(point, &flags);
	if (item)
	{
		this->SelectItem(item);
		RECT r;
		GetWindowRect(&r);

		CMenu  menu;
		CMenu* subMenu;

		TVITEM item;
		item.mask = TVIF_HANDLE | TVIF_PARAM;
		item.hItem = GetSelectedItem();
		GetItem(&item);

		CNodeType *nt =  (CNodeType *) item.lParam;


		UINT bIsRunning = _ParticleDlg->StartStopDlg->isRunning() ? MF_BYCOMMAND | MF_DISABLED | MF_GRAYED : MF_ENABLED;

		switch (nt->Type)
		{
			case CNodeType::located:
			{
				 menu.LoadMenu(IDR_LOCATED_MENU);
				 menu.EnableMenuItem(ID_INSTANCIATE_LOCATED,  bIsRunning);
				 menu.EnableMenuItem(IDM_COPY_LOCATED,  bIsRunning);
				 menu.EnableMenuItem(IDM_PASTE_BINDABLE,  bIsRunning);
			}
			break;
			case CNodeType::locatedBindable:
				menu.LoadMenu(IDR_LOCATED_BINDABLE_MENU);		 				
				menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD1N2, MF_UNCHECKED | MF_BYCOMMAND);
				menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD1, MF_UNCHECKED | MF_BYCOMMAND);
				menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD2, MF_UNCHECKED | MF_BYCOMMAND);
				// check the menu to tell which lod is used for this located bindable
				if (nt->Bind->getLOD() == NL3D::PSLod1n2) menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD1N2, MF_CHECKED | MF_BYCOMMAND);
				if (nt->Bind->getLOD() == NL3D::PSLod1) menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD1, MF_CHECKED | MF_BYCOMMAND);
				if (nt->Bind->getLOD() == NL3D::PSLod2) menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD2, MF_CHECKED | MF_BYCOMMAND);

				 menu.EnableMenuItem(IDM_COPY_BINDABLE,  bIsRunning);
			break;
			case CNodeType::particleSystem:
				 menu.LoadMenu(IDR_PARTICLE_SYSTEM_MENU);	
 				 menu.EnableMenuItem(IDM_PASTE_LOCATED,  bIsRunning);
			break;
			case CNodeType::locatedInstance:
				menu.LoadMenu(IDR_LOCATED_INSTANCE_MENU);
			break;
			default:
				return;
			break;
		}
		

		subMenu = menu.GetSubMenu(0);    
		nlassert(subMenu);
		subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, r.left + point.x, r.top + point.y, this);


	}
	


}

BOOL CParticleTreeCtrl::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{

	if (nCode != 0) return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	CPSLocatedBindable *toCreate = NULL;
				
	CNodeType *nt = (CNodeType *) GetItemData(GetSelectedItem());
	bool	  createLocAndBindable = false; // when set to true, must create a located and a simultaneously a located bindable

	switch(nID)
	{
		///////////////
		// particles //
		///////////////
		case IDM_DOT_LOC: createLocAndBindable = true;
		case IDM_DOT:
			toCreate = new NL3D::CPSDot;
		break;
		case IDM_LOOKAT_LOC:  createLocAndBindable = true;
		case IDM_LOOKAT:
			toCreate = new NL3D::CPSFaceLookAt; 
		break;
		case IDM_FANLIGHT_LOC:  createLocAndBindable = true;
		case IDM_FANLIGHT:
			toCreate = new NL3D::CPSFanLight;
		break;
		case IDM_RIBBON_LOC:  createLocAndBindable = true;
		case IDM_RIBBON:
			toCreate = new NL3D::CPSRibbon;
		break;
		case IDM_TAILDOT_LOC: createLocAndBindable = true;
		case IDM_TAILDOT:
			toCreate = new NL3D::CPSTailDot; 
		break;
		case IDM_MESH_LOC:  createLocAndBindable = true;
		case IDM_MESH:
			toCreate = new NL3D::CPSMesh;			
		break;
		case IDM_CONSTRAINT_MESH_LOC:  createLocAndBindable = true;
		case IDM_CONSTRAINT_MESH:
			toCreate = new NL3D::CPSConstraintMesh;			
		break;
		case IDM_FACE_LOC:  createLocAndBindable = true;
		case IDM_FACE:
			toCreate = new NL3D::CPSFace;
		break;
		case IDM_SHOCKWAVE_LOC:  createLocAndBindable = true;
		case IDM_SHOCKWAVE:
			toCreate = new NL3D::CPSShockWave;
		break;
		case IDM_RIBBON_LOOK_AT_LOC:  createLocAndBindable = true;
		case IDM_RIBBON_LOOK_AT:
			toCreate = new NL3D::CPSRibbonLookAt;
		break;

		//////////////
		// emitters //
		//////////////

		case IDM_DIRECTIONNAL_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_DIRECTIONNAL_EMITTER:
			toCreate = new NL3D::CPSEmitterDirectionnal;
		break;
		case IDM_OMNIDIRECTIONNAL_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_OMNIDIRECTIONNAL_EMITTER:
			toCreate = new NL3D::CPSEmitterOmni;
		break;
		case IDM_CONIC_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_CONIC_EMITTER:
			toCreate = new NL3D::CPSEmitterConic;
		break; 
		case IDM_RECTANGLE_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_RECTANGLE_EMITTER:
			toCreate = new NL3D::CPSEmitterRectangle;
		break;
		case IDM_SPHERICAL_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_SPHERICAL_EMITTER:
			toCreate = new NL3D::CPSSphericalEmitter;
		break;
		case IDM_RADIAL_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_RADIAL_EMITTER:
			toCreate = new NL3D::CPSRadialEmitter;
		break;

		////////////////
		//   Zones    //
		////////////////
		
		case IDM_ZONE_PLANE_LOC:  createLocAndBindable = true;
		case IDM_ZONE_PLANE:
			toCreate = new NL3D::CPSZonePlane;
		break;
		case IDM_ZONE_SPHERE_LOC:  createLocAndBindable = true;
		case IDM_ZONE_SPHERE:
			toCreate = new NL3D::CPSZoneSphere;
		break;
		case IDM_ZONE_DISC_LOC:  createLocAndBindable = true;
		case IDM_ZONE_DISC:
			toCreate = new NL3D::CPSZoneDisc;
		break;
		case IDM_ZONE_RECTANGLE_LOC:  createLocAndBindable = true;
		case IDM_ZONE_RECTANGLE:
			toCreate = new NL3D::CPSZoneRectangle;
		break;
		case IDM_ZONE_CYLINDER_LOC:  createLocAndBindable = true;
		case IDM_ZONE_CYLINDER:
			toCreate = new NL3D::CPSZoneCylinder;
		break;

		///////////////
		//   forces  //
		///////////////
		case IDM_GRAVITY_FORCE_LOC:  createLocAndBindable = true;
		case IDM_GRAVITY_FORCE:
			toCreate = new NL3D::CPSGravity;
		break;
		case IDM_DIRECTIONNAL_FORCE_LOC:  createLocAndBindable = true;
		case IDM_DIRECTIONNAL_FORCE:
			toCreate = new NL3D::CPSDirectionnalForce;
		break;
		case IDM_SPRING_FORCE_LOC:  createLocAndBindable = true;
		case IDM_SPRING_FORCE:
			toCreate = new NL3D::CPSSpring;
		break;
		case IDM_FLUID_FRICTION_LOC:  createLocAndBindable = true;
		case IDM_FLUID_FRICTION:
			toCreate = new NL3D::CPSFluidFriction;
		break;
		case IDM_CENTRAL_GRAVITY_LOC:  createLocAndBindable = true;
		case IDM_CENTRAL_GRAVITY:
			toCreate = new NL3D::CPSCentralGravity;
		break;
		case IDM_CYLINDRIC_VORTEX_LOC:  createLocAndBindable = true;
		case IDM_CYLINDRIC_VORTEX:
			toCreate = new NL3D::CPSCylindricVortex;
		break;
		case IDM_BROWNIAN_MOVE_LOC:  createLocAndBindable = true;
		case IDM_BROWNIAN_MOVE:
			toCreate = new NL3D::CPSBrownianForce;
		break;
		case IDM_MAGNETIC_FORCE_LOC:  createLocAndBindable = true;
		case IDM_MAGNETIC_FORCE:
			toCreate = new NL3D::CPSMagneticForce;
		break;

		///////////////
		//    sound  //
		///////////////
		case IDM_SOUND_LOC:  createLocAndBindable = true;
		case IDM_SOUND:
			toCreate = new NL3D::CPSSound;
			if (!_ParticleDlg->StartStopDlg->isRunning())
			{
				(static_cast<NL3D::CPSSound *>(toCreate))->stopSound();
			}
		break;

		//////////////
		// deletion //
		//////////////
		case IDM_DELETE_LOCATED:
		{
			_ParticleDlg->setRightPane(NULL);
			
			CPSLocated *loc = nt->Loc;			
			CParticleSystem *ps = _ParticleDlg->getCurrPS();
			ps->remove(loc);	
			DeleteItem(GetSelectedItem());
			delete nt;

			_NodeTypes.erase(std::find(_NodeTypes.begin(), _NodeTypes.end(), nt));


			// if the system is running, we must destroy initial infos about the located
			// , as they won't need to be restored when the stop button will be pressed
			_ParticleDlg->StartStopDlg->removeLocated(loc);

			_ParticleDlg->getCurrPSModel()->touchTransparencyState();
			

			return TRUE;
			
			//return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}
		break;


		case IDM_DELETE_LOCATED_BINDABLE:
		{					

			_NodeTypes.erase(std::find(_NodeTypes.begin(), _NodeTypes.end(), nt));

			CPSLocatedBindable *lb = nt->Bind;			

			// now, get the father								
			CPSLocated *loc = ((CNodeType *) GetItemData(GetParentItem(GetSelectedItem())))->Loc;	
			loc->remove(lb);
			delete nt; 
			

			// if the system is running, we must destroy initial infos 
			// that what saved about the located bindable, when the start button was pressed, as they won't need
			// to be restored

			_ParticleDlg->StartStopDlg->removeLocatedBindable(lb);


			_ParticleDlg->setRightPane(NULL);
			DeleteItem(GetSelectedItem());			
			
			_ParticleDlg->getCurrPSModel()->touchTransparencyState();
			
			
			return TRUE;			
		}
		break;

		case IDM_DELETE_LOCATED_INSTANCE:
		{
			nlassert(nt->Type == CNodeType::locatedInstance);		
			DeleteItem(GetSelectedItem());			
			_NodeTypes.erase(std::find(_NodeTypes.begin(), _NodeTypes.end(), nt));			
			suppressLocatedInstanceNbItem(0);			
			nt->Loc->deleteElement(nt->LocatedInstanceIndex);					
			delete nt;
			rebuildLocatedInstance();			
		}
		break; 

		// instanciate an element
		case ID_INSTANCIATE_LOCATED:
		{						
			if (nt->Loc->getSize() == nt->Loc->getMaxSize())
			{
				nt->Loc->resize(nt->Loc->getMaxSize() + 1);
				// force a re-update of the left pane
				NM_TREEVIEW nmt;
				LRESULT pResult;
				nmt.itemNew.lParam = GetItemData(GetSelectedItem());
				OnSelchanged((NMHDR *) &nmt, &pResult);
			}

			sint32 objIndex = nt->Loc->newElement();

			nt = new CNodeType(nt->Loc, objIndex);
			_NodeTypes.push_back(nt);
			// insert the element in the tree
			InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, "instance", 8, 8, 0, 0, (LPARAM) nt, GetSelectedItem(), TVI_LAST);
			Invalidate();
		}
		break;

		////////////
		// LOD OP //
		////////////
		case IDM_LB_LOD1N2:
			nlassert(nt->Type = CNodeType::locatedBindable);
			nt->Bind->setLOD(NL3D::PSLod1n2);
		break;
		case IDM_LB_LOD1:
			nlassert(nt->Type = CNodeType::locatedBindable);
			nt->Bind->setLOD(NL3D::PSLod1);
		break;
		case IDM_LB_LOD2:
			nlassert(nt->Type = CNodeType::locatedBindable);
			nt->Bind->setLOD(NL3D::PSLod2);
		break;

		////////////////
		// extern ID  //
		////////////////
		case IDM_LB_EXTERN_ID:
		{
			nlassert(nt->Type = CNodeType::locatedBindable);
			nlassert(nt->Bind);
			CLBExternIDDlg 	dlg(nt->Bind->getExternID());
			int res = dlg.DoModal();
			if ( res == IDOK )
			{
				nt->Bind->setExternID( dlg.getNewID() );
			}
		}
		break;
		////////////////////////
		//		COPY / PASTE  //
		////////////////////////
		case IDM_COPY_LOCATED:
			nlassert(nt->Type == CNodeType::located);
			nlassert(nt->Loc);			
			_LocatedCopy.reset(NLMISC::safe_cast<NL3D::CPSLocated *>(::DupPSLocated(nt->Loc)));
		break;
		case IDM_COPY_BINDABLE:
			nlassert(nt->Type == CNodeType::locatedBindable);
			nlassert(nt->Bind);			
			_LocatedBindableCopy.reset(::DupPSLocatedBindable(nt->Bind));
		break;
		case IDM_PASTE_LOCATED:
		{			
			nlassert(nt->Type== CNodeType::particleSystem);
			nlassert(nt->PS);
			CPSLocated *copy = dynamic_cast<CPSLocated *>(::DupPSLocated(_LocatedCopy.get()));
			if (!copy) break;
			nt->PS->attach(copy);
			createNodeFromLocated(copy, GetRootItem());
			Invalidate();
		}
		break;
		case IDM_PASTE_BINDABLE:
		{
			nlassert(nt->Type == CNodeType::located);
			nlassert(nt->Loc);
			CPSLocatedBindable *copy = ::DupPSLocatedBindable(_LocatedBindableCopy.get());
			if (!copy) break;
			nt->Loc->bind(copy);
			createNodeFromLocatedBindable(copy, GetSelectedItem());
			Invalidate();
		}
		break;


		////////////////////////
		// PARTICLE SYSTEM OP //
		////////////////////////
		case ID_MENU_NEWLOCATED:
		{			
			createLocated(nt->PS, GetSelectedItem());												
			Invalidate();
		}
		break;
		case ID_MENU_LOAD_PS:
		{


				try
				{				
					_ParticleDlg->StartStopDlg->stop();
					static char BASED_CODE szFilter[] = "ps & shapes files(*.ps;*.shape)|*.ps; *.shape||";
					CFileDialog fd( TRUE, ".ps", "*.ps;*.shape", 0, szFilter);
					int res = fd.DoModal();
					if (res != IDOK)
					{ 
						return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
					}
					
					// Add to the path
					char drive[256];
					char dir[256];
					char path[256];

					// Add search path for the texture
					_splitpath (fd.GetPathName(), drive, dir, NULL, NULL);
					_makepath (path, drive, dir, NULL, NULL);
					NLMISC::CPath::addSearchPath (path);
														
				
				
					NL3D::IShape *sh = nt->PSModel->Shape;																			
					NL3D::CNELU::Scene.deleteInstance(nt->PSModel);
					NL3D::CNELU::Scene.getShapeBank()->reset();						
				
					DeleteItem(TVI_ROOT);				
				
					NL3D::CParticleSystemModel *newModel = dynamic_cast<CParticleSystemModel *>(NL3D::CNELU::Scene.createInstance(std::string((LPCTSTR) fd.GetFileName())));

					if (newModel)
					{			
						CParticleSystemModel *psm = newModel;					
						psm->setEditionMode(true); // this also force the system instanciation
						CParticleSystem      *ps  = psm->getPS();

						psm->enableAutoGetEllapsedTime(false);
						psm->setEllapsedTime(0.f); // system is paused
						psm->enableDisplayTools(true);							
				
						_ParticleDlg->setRightPane(NULL);
						_ParticleDlg->setNewCurrPS(ps, psm);

						ps->setFontManager(_ParticleDlg->FontManager);
						ps->setFontGenerator(_ParticleDlg->FontGenerator);
						ps->stopSound();
						
						buildTreeFromPS(ps, psm);
						
					}
					else
					{
						throw NLMISC::Exception("Unable to load or intanciate the system");
						
					}								
				}

				catch (NLMISC::Exception &e)
				{
					MessageBox(e.what(), "error loading particle system");				
					// create a blank system
					_ParticleDlg->resetSystem();
					_ParticleDlg->setRightPane(NULL);
					buildTreeFromPS(_ParticleDlg->getCurrPS(), _ParticleDlg->getCurrPSModel());
				}		
				
									
				rebuildLocatedInstance();
				_LastClickedPS = NULL;

				// reset the camera
				_ParticleDlg->MainFrame->OnResetCamera();

		}
		break;
		case IDM_MERGE_PS:
		{
			try
			{		
				_ParticleDlg->StartStopDlg->stop();
				static char BASED_CODE szFilter[] = "ps & shapes files(*.ps;*.shape)|*.ps; *.shape||";
				CFileDialog fd( TRUE, ".ps", "*.ps;*.shape", 0, szFilter);
				
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
																																				
					NL3D::CParticleSystemModel *newModel = dynamic_cast<CParticleSystemModel *>(NL3D::CNELU::Scene.createInstance(std::string((LPCTSTR) fd.GetFileName())));

					if (newModel)
					{									
						nt->PSModel->getPS()->merge( NLMISC::safe_cast<NL3D::CParticleSystemShape *>((NL3D::IShape *) newModel->Shape) );				
					}
					NL3D::CNELU::Scene.deleteInstance(newModel);
					NL3D::CNELU::Scene.getShapeBank()->reset();

					DeleteItem(TVI_ROOT);
					buildTreeFromPS(nt->PS, nt->PSModel);
					_ParticleDlg->MainFrame->OnResetCamera();			
				}					
			}

			catch (NLMISC::Exception &e)
			{
				MessageBox(e.what(), "error merging particle system");						
			}	
		}
		break;
		case ID_MENU_SAVE_PS:
		{
				_ParticleDlg->StartStopDlg->stop();
				std::string fileName;
				if (nt->PS->getName() != std::string())
				{
					fileName = nt->PS->getName() + std::string(".ps");
				}
				else
				{
					fileName = "*.ps";
				}
				CFileDialog fd(FALSE, ".ps", fileName.c_str(), OFN_OVERWRITEPROMPT, NULL, this);
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


					nlassert(nt->Type == CNodeType::particleSystem);

					// build a shape from our system, and save it
					NL3D::CParticleSystemShape psc;

					try
					{
						psc.buildFromPS(*nt->PS);
						{
							NL3D::CShapeStream st(&psc);
							NLMISC::COFile oFile((LPCTSTR) fd.GetPathName());
							oFile.serial(st);
						}
					}
					catch (NLMISC::Exception &E)
					{
						MessageBox(E.what(), "save error");
					}	
				}
		}
		break;
		case ID_MENU_DELETE_PS:
		{
			if (MessageBox("Delete the system ?", "Particle editor", MB_YESNO) == IDYES)
			{
				NL3D::CParticleSystem *ps = nt->PS;			
				while (ps->getNbProcess())
				{					
					DeleteItem(TVI_ROOT);
					ps->remove(ps->getProcess(0));					
				}
				////
				_ParticleDlg->StartStopDlg->reset();
				buildTreeFromPS(nt->PS, nt->PSModel);
				_ParticleDlg->MainFrame->OnResetCamera();
			}
		}
		break;
	}


	if (toCreate)
	{
		HTREEITEM son, lastSon, father;
		if (createLocAndBindable)
		{
			
			std::pair<CParticleTreeCtrl::CNodeType *, HTREEITEM> p = createLocated(nt->PS, GetSelectedItem());
			nt = p.first;
			son = 0;
			father = p.second;
			lastSon = p.second;

		}
		else
		{
			son = GetChildItem(GetSelectedItem());
			father = GetSelectedItem();
		}
			

		// complete the name
		std::string name = toCreate->getName();
		char num[128];
		if (_PSElementIdentifiers.count(name))
		{			 
			sprintf(num, "%d", ++_PSElementIdentifiers[name]);
			toCreate->setName(name + num);
		}
		else
		{
			_PSElementIdentifiers[toCreate->getName()] = 0;
			toCreate->setName(name + "0");
		}

		
		
		nt->Loc->bind(toCreate);
		CNodeType *newNt = new CNodeType(toCreate);
		_NodeTypes.push_back(newNt);

		// insert the element in the tree
		// we want that the instance always appears in the last position				
		if (!createLocAndBindable)
		{
			if (!son)
			{
				lastSon = GetSelectedItem();
			}
			else
			{

				lastSon  = TVI_FIRST;

				
				while (son != NULL)
				{				
					if (((CNodeType *) GetItemData(son))->Type == CNodeType::locatedInstance)
					{
						break;
					}
					lastSon = son;
					son = GetChildItem(son);
				}
			}
		}


		InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, toCreate->getName().c_str(), toCreate->getType(), toCreate->getType(), 0, 0, (LPARAM) newNt, father, lastSon);

		_ParticleDlg->getCurrPSModel()->touchTransparencyState();		
		Invalidate();
	}











	return CTreeCtrl::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

std::pair<CParticleTreeCtrl::CNodeType *, HTREEITEM> CParticleTreeCtrl::createLocated(NL3D::CParticleSystem *ps, HTREEITEM headItem)
{ 
	std::string name; 
	char num[128];
	if (_PSElementIdentifiers.count(std::string("located")))
	{
		sprintf(num, "%d", ++_PSElementIdentifiers[std::string("located")]);
	}
	else
	{
		name = std::string("located 0");
	}		

	
	
	CPSLocated *loc = new CPSLocated;
	loc->setName(name);
	loc->setSystemBasis(true);	
	ps->attach(loc);

	CNodeType *newNt = new CNodeType(loc);
	_NodeTypes.push_back(newNt);
	// insert item in tree
	HTREEITEM insertedItem = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, name.c_str(), 7, 7, 0, 0, (LPARAM) newNt, headItem, TVI_LAST);
	_ParticleDlg->getCurrPSModel()->touchTransparencyState();
	return std::make_pair(newNt, insertedItem);
}



/// The user finished to edit a label in the tree
void CParticleTreeCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMTVDISPINFO* info = (NMTVDISPINFO*)pNMHDR;

	*pResult = 0;

	if (info->item.pszText)
	{
		this->SetItemText(info->item.hItem, info->item.pszText);

		CNodeType *nt = (CNodeType *) info->item.lParam;

		switch (nt->Type)
		{
			case CNodeType::particleSystem :
			{
				nt->PS->setName(std::string(info->item.pszText));
			}
			break;
			case CNodeType::located :
			{
				nt->Loc->setName(std::string(info->item.pszText));
			}
			break;
			case CNodeType::locatedBindable :
			{
				nt->Bind->setName(std::string(info->item.pszText));
			}
			break;
		}

	}
}


void CParticleTreeCtrl::moveElement(const NLMISC::CMatrix &m)
{
	static NLMISC::CMatrix mat;

	// no == operator yet... did the matrix change ?
	if (m.getPos() == mat.getPos()
		&& m.getI() == mat.getI() 
		&& m.getJ() == mat.getJ() 
		&& m.getK() == mat.getK() 
	   ) return;


	mat = m;
	
	// the current element must be an instance
	if (::IsWindow(m_hWnd))
	{
		HTREEITEM currItem = GetSelectedItem();
		if (currItem)
		{
			CWnd *rightPane = _ParticleDlg->getRightPane();
			NL3D::IPSMover *psm = NULL;
			if (dynamic_cast<CPSMoverDlg *>(rightPane ))
			{
				CPSMoverDlg *rp = (CPSMoverDlg *) rightPane;
				psm = ((CPSMoverDlg *) rightPane)->getMoverInterface();
												
				if (psm && !psm->onlyStoreNormal())
				{									
					psm->setMatrix(rp->getLocatedIndex(), mat);										
				}
				else
				{
					rp->getLocated()->getPos()[rp->getLocatedIndex()] = mat.getPos();
				}

				// update the dialog				
				rp->updatePosition();
			}
		}
	}
}



NLMISC::CMatrix CParticleTreeCtrl::getElementMatrix(void) const
{
	HTREEITEM currItem = GetSelectedItem();
	if (currItem)
	{
		CNodeType *nt = (CNodeType *) GetItemData(currItem);
		if (nt->Type == CNodeType::locatedInstance)
		{
			NLMISC::CVector pos = nt->Loc->getPos()[nt->LocatedInstanceIndex];
			NLMISC::CMatrix m;
			m.identity();
			m.setPos(pos);
			return m; 
		}
	}

	return NLMISC::CMatrix::Identity;
}

