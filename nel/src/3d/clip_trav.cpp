/** \file clip_trav.cpp
 * <File description>
 *
 * $Id: clip_trav.cpp,v 1.26 2002/06/28 14:21:29 berenguier Exp $
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

using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
CClipTrav::CClipTrav() : ViewPyramid(6), WorldPyramid(6)
{
	_VisibleList.reserve(1024);
	CurrentDate = 0;
	Accel.create (64, 16.0f);

	ForceNoFrustumClip= false;
	_QuadGridClipManager= NULL;


	HrcTrav= NULL;
	AnimDetailTrav= NULL;
	LoadBalancingTrav= NULL;
	LightTrav= NULL;
	RenderTrav = NULL;
}

// ***************************************************************************
CClipTrav::~CClipTrav()
{
}

// ***************************************************************************
IObs* CClipTrav::createDefaultObs() const
{
	return new CDefaultClipObs;
}

// ***************************************************************************
bool CClipTrav::fullSearch (vector<CCluster*>& result, CInstanceGroup *pIG, CVector& pos)
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

// ***************************************************************************
void CClipTrav::traverse()
{
	H_AUTO( NL3D_TravClip );

	// Increment the current date of the traversal
	++CurrentDate;
	// Update Clip infos.
	ITravCameraScene::update();

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
		_QuadGridClipManager->updateClustersFromCamera(this, CamPos);
	}

	// Clear the traversals list.
	nlassert(AnimDetailTrav && LoadBalancingTrav && LightTrav && RenderTrav);
	AnimDetailTrav->clearVisibleList();
	LoadBalancingTrav->clearVisibleList();
	LightTrav->clearLightedList();
	RenderTrav->clearRenderList();


	/* For all objects marked visible in preceding render, reset Visible state here.
		NB: must reset Visible State to false because sometimes ClipObs::traverse() are even not executed
		(Cluster clip, QuadGridClipManager clip...).
		And somes models read this Visible state. eg: Skins/StickedObjects test the Visible state of 
		their _AncestorSkeletonModel.
	*/
	for (i=0;i<_VisibleList.size();i++)
	{
		// if the observer still exists (see ~IBaseClipObs())
		if( _VisibleList[i] )
		{
			// disable his visibility.
			_VisibleList[i]->Visible= false;
			// let him know that it is no more in the list.
			_VisibleList[i]->_IndexInVisibleList= -1;
		}
	}
	// Clear The visible List.
	_VisibleList.clear();


	// Found where is the camera
	//========================

	// Found the cluster where the camera is
	static vector<CCluster*> vCluster;

	vCluster.clear();

	// In which cluster is the camera ?
	bool bInWorld = true;
	CQuadGrid<CCluster*>::CIterator itAcc;
	if (Camera->getClusterSystem() == (CInstanceGroup*)-1)
	{
		if (fullSearch(vCluster, RootCluster->Group, CamPos))
			bInWorld = false;
		for (i = 0; i < vCluster.size(); ++i)
			link(NULL, vCluster[i]);
	}
	else
	{
		Accel.select (CamPos, CamPos);
		itAcc = Accel.begin();
		while (itAcc != Accel.end())
		{
			CCluster *pCluster = *itAcc;
			if (pCluster->Group == Camera->getClusterSystem())
			if (pCluster->isIn (CamPos))
			{
				link (NULL, pCluster);
				vCluster.push_back (pCluster);
				bInWorld = false;
			}
			++itAcc;
		}
	}

	/// \todo check if necessary ... I think not 
	if (bInWorld)
	{
		link (NULL, RootCluster);
		vCluster.push_back (RootCluster);
	}


	// Manage Moving Objects
	//=====================

	// Unlink the moving objects from their clusters
	for (i = 0; i < HrcTrav->_MovingObjects.size(); ++i)
	{
		CTransformShape *pTfmShp = dynamic_cast<CTransformShape*>(HrcTrav->_MovingObjects[i]);
		if (pTfmShp == NULL)
			continue;

		static vector<IModel*> vModels;
		vModels.clear();
		IModel *pFather = getFirstParent (HrcTrav->_MovingObjects[i]);
		while (pFather != NULL)
		{
			// Does the father is a cluster, or a CQuadGridClipCluster ??
			if ( dynamic_cast<CCluster*>(pFather)!= NULL  ||  dynamic_cast<CQuadGridClipCluster*>(pFather)!=NULL )
			{
				vModels.push_back (pFather);
			}			
			pFather = getNextParent (HrcTrav->_MovingObjects[i]);
		}
		for (j = 0; j < vModels.size(); ++j)
		{
			unlink (vModels[j], HrcTrav->_MovingObjects[i]);
		}
		unlink (NULL, HrcTrav->_MovingObjects[i]);
	}
	
	// Affect the moving objects to their clusters
	for (i = 0; i < HrcTrav->_MovingObjects.size(); ++i)
	{
		CTransformShape *pTfmShp = dynamic_cast<CTransformShape*>(HrcTrav->_MovingObjects[i]);
		if (pTfmShp == NULL)
			continue;

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
				link (pCluster, pTfmShp);
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
				if(!_QuadGridClipManager->linkModel(pTfmShp, this))
					// if fails, link to "root".
					link (RootCluster, pTfmShp);
			}
			else
			{
				link (RootCluster, pTfmShp);
			}
		}
	}

	// Clip the graph.
	//=====================

	// Traverse the graph.
	if (Root)
		Root->traverse (NULL);


	// Unlink the cluster where we are
	for (i = 0; i < vCluster.size(); ++i)
	{
		unlink (NULL, vCluster[i]);
	}


	// At the end of the clip traverse, must update clip for Objects which have a skeleton ancestor
	// =========================
	// those are linked to the SonsOfAncestorSkeletonModelGroup, so traverse it now.
	if (SonsOfAncestorSkeletonModelGroup)
		SonsOfAncestorSkeletonModelGroup->getObs(ClipTravId)->traverse(NULL);
}


// ***************************************************************************
void CClipTrav::setHrcTrav (CHrcTrav* trav)
{
	HrcTrav = trav;
}

// ***************************************************************************
void CClipTrav::setAnimDetailTrav(CAnimDetailTrav *trav)
{
	AnimDetailTrav= trav;
}

// ***************************************************************************
void CClipTrav::setLoadBalancingTrav(CLoadBalancingTrav *trav)
{
	LoadBalancingTrav= trav;
}

// ***************************************************************************
void CClipTrav::setLightTrav (CLightTrav* trav)
{
	LightTrav = trav;
}

// ***************************************************************************
void CClipTrav::setRenderTrav (CRenderTrav* trav)
{
	RenderTrav = trav;
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
void CClipTrav::setSonsOfAncestorSkeletonModelGroup(CRootModel *m)
{
	SonsOfAncestorSkeletonModelGroup= m;
}


// ***************************************************************************
void CClipTrav::addVisibleObs(CTransformClipObs *obs)
{
	obs->_IndexInVisibleList= _VisibleList.size();
	_VisibleList.push_back(obs);
}



}
