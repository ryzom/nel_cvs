/** \file transform_shape.cpp
 * <File description>
 *
 * $Id: transform_shape.cpp,v 1.30 2002/06/28 14:21:29 berenguier Exp $
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

#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/debug.h"
#include "3d/driver.h"
#include "3d/transform_shape.h"
#include "3d/skeleton_model.h"
#include "3d/mesh_base_instance.h"
#include "3d/clip_trav.h"
#include "3d/render_trav.h"
#include "3d/load_balancing_trav.h"


using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
void		CTransformShape::registerBasic()
{
	CMOT::registerModel(TransformShapeId, TransformId, CTransformShape::creator);
	CMOT::registerObs(ClipTravId, TransformShapeId, CTransformShapeClipObs::creator);
	CMOT::registerObs(RenderTravId, TransformShapeId, CTransformShapeRenderObs::creator);
	CMOT::registerObs(LoadBalancingTravId, TransformShapeId, CTransformShapeLoadBalancingObs::creator);
}


// ***************************************************************************
CTransformShape::CTransformShape()
{
	_NumTrianglesAfterLoadBalancing= 100;
	_CurrentLightContribution= NULL;
	_CurrentUseLocalAttenuation= false;

	// By default all transformShape are LoadBalancable
	CTransform::setIsLoadbalancable(true);

	// The model is renderable
	CTransform::setIsRenderable(true);
}


// ***************************************************************************
float		CTransformShape::getNumTriangles (float distance)
{
	// Call shape method
	return Shape->getNumTriangles (distance);
}


// ***************************************************************************
void		CTransformShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	if(Shape)
	{
		Shape->getAABBox(bbox);
	}
	else
	{
		bbox.setCenter(CVector::Null);
		bbox.setHalfSize(CVector::Null);
	}
}


// ***************************************************************************
void		CTransformShape::changeLightSetup(CRenderTrav *rdrTrav)
{
	// setup the instance lighting.
	rdrTrav->changeLightSetup(_CurrentLightContribution, _CurrentUseLocalAttenuation);
}


// ***************************************************************************
bool	CTransformShapeClipObs::clip(IBaseClipObs *caller) 
{
	CClipTrav			*trav= (CClipTrav*)Trav;
	CTransformShape		*m= (CTransformShape*)Model;

	// reset
	_ClipDueToDistMax= false;

	if(m->Shape)
	{
		// first test DistMax (faster).
		float maxDist = m->Shape->getDistMax();
		// if DistMax test enabled
		if(maxDist!=-1)
		{
			// Calc the distance
			float sqrDist = (trav->CamPos - m->getMatrix().getPos()).sqrnorm ();
			maxDist*=maxDist;
			
			// if dist > maxDist, skip
			if (sqrDist > maxDist)
			{
				// flag this state
				_ClipDueToDistMax= true;
				// Ok, not shown
				return false;
			}
		}

		// Else finer clip with pyramid, only if needed
		if(trav->ForceNoFrustumClip)
			return true;
		else
			return m->Shape->clip(trav->WorldPyramid, HrcObs->WorldMatrix);
	}
	else
		return false;
}


// ***************************************************************************
void	CTransformShapeRenderObs::traverse(IObs *caller)
{
	H_AUTO( NL3D_TrShape_Render );

	CTransformShape		*m= (CTransformShape*)Model;


	// Compute the current lighting setup for this instance
	//===================

	// if the transform is lightable (ie not a fully lightmaped model), setup lighting
	if(m->isLightable())
	{
		// useLocalAttenuation for this shape ??
		if(m->Shape)
			m->_CurrentUseLocalAttenuation= m->Shape->useLightingLocalAttenuation ();
		else
			m->_CurrentUseLocalAttenuation= false;

		// Get HrcObs.
		CTransformHrcObs	*hrcObs= (CTransformHrcObs*)HrcObs;

		// the std case is to take my model lightContribution
		if(hrcObs->_AncestorSkeletonModel==NULL)
			m->_CurrentLightContribution= &m->getLightContribution();
		// but if skinned/sticked (directly or not) to a skeleton, take its.
		else
			m->_CurrentLightContribution= &hrcObs->_AncestorSkeletonModel->getLightContribution();
	}
	// else must disable the lightSetup
	else
	{
		// setting NULL will disable all lights
		m->_CurrentLightContribution= NULL;
		m->_CurrentUseLocalAttenuation= false;
	}


	// render the shape.
	//=================
	if(m->Shape)
	{
		CRenderTrav			*rdrTrav= (CRenderTrav*)Trav;
		bool				currentPassOpaque= rdrTrav->isCurrentPassOpaque();

		// shape must be rendered in a CMeshBlockManager ??
		float polygonCount;
		IMeshGeom	*meshGeom= NULL;
		// true only if in pass opaque
		if( currentPassOpaque )
			meshGeom= m->Shape->supportMeshBlockRendering(m, polygonCount);

		// if ok, add the meshgeom to the block manager.
		if(meshGeom)
		{
			CMeshBaseInstance	*inst= safe_cast<CMeshBaseInstance*>(m);
			rdrTrav->MeshBlockManager.addInstance(meshGeom, inst, polygonCount);
		}
		// else render it.
		else
		{
			// setup the lighting
			m->changeLightSetup( rdrTrav );

			// render the shape.
			IDriver				*drv= rdrTrav->getDriver();
			m->Shape->render( drv, m, currentPassOpaque );
		}
	}
}


// ***************************************************************************
void	CTransformShapeLoadBalancingObs::traverse(IObs *caller)
{
	CLoadBalancingTrav		*loadTrav= (CLoadBalancingTrav*)Trav;
	if(loadTrav->getLoadPass()==0)
		traversePass0();
	else
		traversePass1();

	// There is no reason to do a hierarchy for LoadBalancing. => no traverseSons()
}



// ***************************************************************************
void	CTransformShapeLoadBalancingObs::traversePass0()
{
	CLoadBalancingTrav		*loadTrav= (CLoadBalancingTrav*)Trav;
	CTransformShape			*trans= static_cast<CTransformShape*>(Model);
	CSkeletonModel			*skeleton= trans->getSkeletonModel();

	// World Model position
	const CVector		*modelPos;

	// If this isntance is binded or skinned to a skeleton, take the world matrix of this one as
	// center for LoadBalancing Resolution.
	if(skeleton)
	{
		// Take the root bone of the skeleton as reference (bone 0)
		// And so get our position.
		modelPos= &skeleton->Bones[0].getWorldMatrix().getPos();
	}
	else
	{
		// get our position from 
		modelPos= &HrcObs->WorldMatrix.getPos();
	}


	// Then compute distance from camera.
	float	modelDist= ( loadTrav->CamPos - *modelPos).norm();


	// Get the number of triangles this model use now.
	_FaceCount= trans->getNumTriangles(modelDist);	
	LoadBalancingGroup->addNbFacesPass0(_FaceCount);
}


// ***************************************************************************
void	CTransformShapeLoadBalancingObs::traversePass1()
{
	CTransformShape			*trans= static_cast<CTransformShape*>(Model);


	// Set the result into the isntance.
	trans->_NumTrianglesAfterLoadBalancing= LoadBalancingGroup->computeModelNbFace(_FaceCount);

}



} // NL3D
