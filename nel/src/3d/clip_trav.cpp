/** \file clip_trav.cpp
 * <File description>
 *
 * $Id: clip_trav.cpp,v 1.36 2003/06/27 12:48:47 besson Exp $
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

#include "std3d.h"

#include "nel/misc/types_nl.h"
#include "3d/clip_trav.h"
#include "3d/hrc_trav.h"
#include "3d/render_trav.h"
#include "3d/anim_detail_trav.h"
#include "3d/load_balancing_trav.h"
#include "3d/cluster.h"
#include "3d/scene_group.h"
#include "3d/transform_shape.h"
#include "3d/camera.h"
#include "3d/quad_grid_clip_cluster.h"
#include "3d/quad_grid_clip_manager.h"
#include "3d/root_model.h"
#include "nel/misc/hierarchical_timer.h"
#include "3d/scene.h"
#include "3d/skeleton_model.h"
#include "3d/fast_floor.h"

using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
CClipTrav::CClipTrav() : ViewPyramid(6), WorldPyramid(6)
{
	_VisibleList.resize(1024);
	_CurrentNumVisibleModels= 0;
	CurrentDate = 0;
	Accel.create (64, 16.0f);

	ForceNoFrustumClip= false;
	_QuadGridClipManager= NULL;
}

// ***************************************************************************
CClipTrav::~CClipTrav()
{
}

// ***************************************************************************
bool CClipTrav::fullSearch (vector<CCluster*>& result, CInstanceGroup *pIG, const CVector& pos)
{
	uint32 i, j;

	for (i = 0; i < pIG->_ClusterInstances.size(); ++i)
	{
		for (j = 0; j < pIG->_ClusterInstances[i]->Children.size(); ++j)
		{
			if (fullSearch (result, pIG->_ClusterInstances[i]->Children[j]->Group, pos))
				return true;
		}
	}

	for (i = 0; i < pIG->_ClusterInstances.size(); ++i)
	{
		if (pIG->_ClusterInstances[i]->isIn(pos))
			result.push_back (pIG->_ClusterInstances[i]);
	}
	if (result.size() > 0)
		return true;
	return false;
}

/// Set cluster tracking on/off (ie storage of thje visible cluster during clip traversal)
void CClipTrav::setClusterVisibilityTracking(bool track)
{
	_TrackClusterVisibility = track;
}
/// Check the activation of cluster visibility tracking.
bool CClipTrav::getClusterVisibilityTracking()
{
	return _TrackClusterVisibility;
}
/// Add a visible cluster to the list
void CClipTrav::addVisibleCluster(CCluster *cluster)
{
	_VisibleClusters.push_back(cluster);
}
/** Return the list of cluster visible after the clip traversal
 *	You must activate the cluster tracking to obtain a result.
*/
const std::vector<CCluster*> &CClipTrav::getVisibleClusters()
{
	return _VisibleClusters;
}


// ***************************************************************************
void CClipTrav::traverse()
{
	H_AUTO( NL3D_TravClip );

	// The root must exist
	CTransform	*sceneRoot= Scene->getRoot();

	// Increment the current date of the traversal
	++CurrentDate;
	// Update Clip infos.
	CTravCameraScene::update();

	// Compute pyramid in view basis.
	CVector		pfoc(0,0,0);
	CVector		lb(Left,  Near, Bottom );
	CVector		lt(Left,  Near, Top    );
	CVector		rb(Right, Near, Bottom );
	CVector		rt(Right, Near, Top    );

	CVector		lbFar(Left,  Far, Bottom);
	CVector		ltFar(Left,  Far, Top   );
	CVector		rtFar(Right, Far, Top   );

	uint32 i, j;

	ViewPyramid[NL3D_CLIP_PLANE_NEAR].make(lt, lb, rt);
	ViewPyramid[NL3D_CLIP_PLANE_FAR].make(lbFar, ltFar, rtFar);

	ViewPyramid[NL3D_CLIP_PLANE_LEFT].make(pfoc, lt, lb);
	ViewPyramid[NL3D_CLIP_PLANE_TOP].make(pfoc, rt, lt);
	ViewPyramid[NL3D_CLIP_PLANE_RIGHT].make(pfoc, rb, rt);
	ViewPyramid[NL3D_CLIP_PLANE_BOTTOM].make(pfoc, lb, rb);
	
	// Compute pyramid in World basis.
	// The vector transformation M of a plane p is computed as p*M-1.
	// Here, ViewMatrix== CamMatrix-1. Hence the following formula.
	for (i = 0; i < 6; i++)
	{
		WorldPyramid[i]= ViewPyramid[i]*ViewMatrix;
	}

	// bkup this pyramid (because this one may be modified by the cluster system).
	WorldFrustumPyramid= WorldPyramid;


	// update the QuadGridClipManager.
	if(_QuadGridClipManager)
	{
		_QuadGridClipManager->updateClustersFromCamera(CamPos);
	}

	H_BEFORE( NL3D_TravClip_ClearLists );

	// Clear the traversals list.
	Scene->getAnimDetailTrav().clearVisibleList();
	Scene->getLoadBalancingTrav().clearVisibleList();
	Scene->getLightTrav().clearLightedList();
	Scene->getRenderTrav().clearRenderList();

	H_AFTER( NL3D_TravClip_ClearLists );


	H_BEFORE( NL3D_TravClip_ResetVisible );

	/* For all objects marked visible in preceding render, reset _Visible state here.
		NB: must reset _Visible State to false because sometimes traverseClip() are even not executed
		(Cluster clip, QuadGridClipManager clip...).
		And somes models read this _Visible state. eg: Skins/StickedObjects test the Visible state of 
		their _AncestorSkeletonModel.
	*/
	for (i=0;i<_CurrentNumVisibleModels;i++)
	{
		// if the model still exists (see ~CTransform())
		if( _VisibleList[i] )
		{
			// disable his visibility.
			_VisibleList[i]->_Visible= false;
			// let him know that it is no more in the list.
			_VisibleList[i]->_IndexInVisibleList= -1;
		}
	}
	// Clear The visible List.
	_CurrentNumVisibleModels= 0;
	// Clear the visible cluster list.
	_VisibleClusters.clear();

	H_AFTER( NL3D_TravClip_ResetVisible );

	// Found where is the camera
	//========================

	H_BEFORE( NL3D_TravClip_FindCameraCluster);

	// Found the cluster where the camera is
	static vector<CCluster*> vCluster;

	vCluster.clear();
	sceneRoot->clipDelChild(RootCluster);

	// In which cluster is the camera ?
	CQuadGrid<CCluster*>::CIterator itAcc;
	if (Camera->getClusterSystem() == (CInstanceGroup*)-1)
	{
		fullSearch(vCluster, RootCluster->Group, CamPos);
		for (i = 0; i < vCluster.size(); ++i)
			sceneRoot->clipAddChild(vCluster[i]);
	}
	else
	{
		bool bInWorld = true;
		Accel.select (CamPos, CamPos);
		itAcc = Accel.begin();
		while (itAcc != Accel.end())
		{
			CCluster *pCluster = *itAcc;
			if (pCluster->Group == Camera->getClusterSystem())
			if (pCluster->isIn (CamPos))
			{
				sceneRoot->clipAddChild(pCluster);
				vCluster.push_back (pCluster);
				bInWorld = false;
			}
			++itAcc;
		}

		if (bInWorld)
		{
			sceneRoot->clipAddChild(RootCluster);
			vCluster.push_back (RootCluster);
		}
	}

	/// Flag all cluster to know if they are in camera or not.
	for(i=0;i<vCluster.size();i++)
	{
		vCluster[i]->setCameraIn(true);
	}


	H_AFTER( NL3D_TravClip_FindCameraCluster);

	// Manage Moving Objects
	//=====================

	H_BEFORE( NL3D_TravClip_MovingObjects);

	// Unlink the moving objects from their clusters
	CHrcTrav	&hrcTrav= Scene->getHrcTrav();
	for (i = 0; i < hrcTrav._MovingObjects.size(); ++i)
	{
		CTransformShape *pTfmShp = hrcTrav._MovingObjects[i];

		static vector<CTransform*> vModels;
		vModels.clear();
		uint	numClipParents= pTfmShp->clipGetNumParents();
		for(j=0;j<numClipParents;j++)
		{
			CTransform *pFather = pTfmShp->clipGetParent(j);

			// Does the father is a cluster ??
			if ( pFather->isCluster() )
			{
				vModels.push_back (pFather);
			}
		}
		// Remove me from all clusters
		for (j = 0; j < vModels.size(); ++j)
		{
			vModels[j]->clipDelChild(pTfmShp);
		}
		// Remove me from Root Too
		sceneRoot->clipDelChild(pTfmShp);

		// NB: only the SonsOfAncestorSkeletonModelGroup may still be here.
	}
	
	// Affect the moving objects to their clusters
	for (i = 0; i < hrcTrav._MovingObjects.size(); ++i)
	{
		CTransformShape *pTfmShp = hrcTrav._MovingObjects[i];

		bool bInWorld = true;
		CAABBox box;
		pTfmShp->getAABBox (box);
		// Transform the box in the world
		CVector c = box.getCenter();
		CVector p = box.getCenter()+box.getHalfSize();
		const CMatrix &wm = pTfmShp->getWorldMatrix();
		c = wm * c;
		p = wm * p;
		float s = (p - c).norm();

		Accel.select (c+CVector(s,s,s), c+CVector(-s,-s,-s));
		itAcc = Accel.begin();
		while (itAcc != Accel.end())
		{
			CCluster *pCluster = *itAcc;
			if (pCluster->Group == pTfmShp->getClusterSystem())
			if (pCluster->isIn (c,s))
			{
				pCluster->clipAddChild(pTfmShp);
				bInWorld = false;
			}
			++itAcc;
		}

		// Moving object in the world -> link to root or to the CQuadGridClipManager.
		if (bInWorld)
		{
			if( _QuadGridClipManager && pTfmShp->isQuadGridClipEnabled() )
			{
				// try to insert in the best cluster of the _QuadGridClipManager.
				if(!_QuadGridClipManager->linkModel(pTfmShp))
					// if fails, link to "root".
					RootCluster->clipAddChild(pTfmShp);
			}
			else
			{
				RootCluster->clipAddChild(pTfmShp);
			}
		}
	}

	H_AFTER( NL3D_TravClip_MovingObjects);

	// Clip the graph.
	//=====================

	H_BEFORE( NL3D_TravClip_Traverse);


	// Traverse the graph.
	sceneRoot->traverseClip();


	// Unlink the cluster where we are
	for (i = 0; i < vCluster.size(); ++i)
	{
		// reset Camera In Flag.
		vCluster[i]->setCameraIn(false);

		// remove from Clip Root
		sceneRoot->clipDelChild(vCluster[i]);
	}

	H_AFTER( NL3D_TravClip_Traverse);

	// Load Balance the Skeleton CLod state here. 
	// =========================
	/* Can't do it in LoadBalancingTrav because sons with _AncestorSkeletonModel!=NULL may be hiden if a skeleton
		is displayed in CLod mode.
		So must do it here, then clip all sons of AncestoreSkeletonModelGroup.
	*/
	H_BEFORE( NL3D_TravClip_LoadBalanceCLod);
	loadBalanceSkeletonCLod();
	H_AFTER( NL3D_TravClip_LoadBalanceCLod);

	H_BEFORE( NL3D_TravClip_SkeletonClip);

	// At the end of the clip traverse, must update clip for Objects which have a skeleton ancestor
	// =========================
	// those are linked to the SonsOfAncestorSkeletonModelGroup, so traverse it now.
	if (Scene->SonsOfAncestorSkeletonModelGroup)
		Scene->SonsOfAncestorSkeletonModelGroup->traverseClip();

	// Update Here the Skin render Lists of All visible Skeletons
	// =========================
	/*
		Done here, because AnimDetail and Render need correct lists. NB: important to do it 
		before Render Traversal, because updateSkinRenderLists() may change the transparency flag!!
		NB: can't do it in the traverseClip() of the skeleton since _DisplayLodCharacterFlag must be updated for this frame.
	*/
	CScene::ItSkeletonModelList		itSkel;
	for(itSkel= Scene->getSkeletonModelListBegin(); itSkel!=Scene->getSkeletonModelListEnd(); itSkel++)
	{
		CSkeletonModel	*sm= *itSkel;
		// if visible
		if(sm->isClipVisible())
			sm->updateSkinRenderLists();
	}

	H_AFTER( NL3D_TravClip_SkeletonClip);
}


// ***************************************************************************
void CClipTrav::setQuadGridClipManager(CQuadGridClipManager *mgr)
{
	_QuadGridClipManager= mgr;
}

// ***************************************************************************
void CClipTrav::registerCluster (CCluster* pCluster)
{
	pCluster->AccelIt = Accel.insert (pCluster->getBBox().getMin(), pCluster->getBBox().getMax(), pCluster);
}

// ***************************************************************************
void CClipTrav::unregisterCluster (CCluster* pCluster)
{
	Accel.selectAll();
	CQuadGrid<CCluster*>::CIterator itAcc = Accel.begin();
	while (itAcc != Accel.end())
	{
		CCluster *pC = *itAcc;
		if (pCluster == pC)
		{
			Accel.erase (itAcc);
			break;
		}
		++itAcc;
	}
}


// ***************************************************************************
void CClipTrav::loadBalanceSkeletonCLod()
{
	CScene::ItSkeletonModelList		itSkel;
	_TmpSortSkeletons.clear();

	// **** compute CLod priority of each skeleton,
	for(itSkel= Scene->getSkeletonModelListBegin(); itSkel!=Scene->getSkeletonModelListEnd(); itSkel++)
	{
		CSkeletonModel	*sm= *itSkel;
		float	pr= sm->computeDisplayLodCharacterPriority();
		// If valid priority (CLOd enabled, and skeleton visible)
		if(pr>0)
		{
			// if the priority is >1, then display as CLod
			if(pr>1)
				sm->setDisplayLodCharacterFlag(true);
			// else load balance.
			else
			{
				CSkeletonKey	key;
				// don't bother OptFastFloor precision. NB: 0<pr<=1 here.
				key.Priority= OptFastFloor(pr*0xFFFFFF00);
				key.SkeletonModel= sm;
				_TmpSortSkeletons.push_back(key);
			}
		}
	}

	// **** sort by priority in ascending order
	uint	nMaxSkelsInNotCLodForm= Scene->getMaxSkeletonsInNotCLodForm();
	// Optim: need it only if too many skels
	if(_TmpSortSkeletons.size()>nMaxSkelsInNotCLodForm)
	{
		sort(_TmpSortSkeletons.begin(), _TmpSortSkeletons.end());
	}

	// **** set CLod flag 
	uint	n= min(nMaxSkelsInNotCLodForm, _TmpSortSkeletons.size());
	uint	i;
	// The lowest priority are displayed in std form
	for(i=0;i<n;i++)
	{
		_TmpSortSkeletons[i].SkeletonModel->setDisplayLodCharacterFlag(false);
	}
	// the other are displayed in CLod form
	for(i=n;i<_TmpSortSkeletons.size();i++)
	{
		_TmpSortSkeletons[i].SkeletonModel->setDisplayLodCharacterFlag(true);
	}

}


// ***************************************************************************
void CClipTrav::reserveVisibleList(uint numModels)
{
	// enlarge only.
	if(numModels>_VisibleList.size())
		_VisibleList.resize(numModels);
}


}
