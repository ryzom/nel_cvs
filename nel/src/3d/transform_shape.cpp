/** \file transform_shape.cpp
 * <File description>
 *
 * $Id: transform_shape.cpp,v 1.22 2002/03/14 18:35:35 vizerie Exp $
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

#include "3d/transform_shape.h"
#include "3d/driver.h"
#include "3d/skeleton_model.h"


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
bool		CTransformShape::isBigLightable() const
{
	// By default a shape is considered big for lightable if it uses localAttenuation
	if(Shape)
		return Shape->useLightingLocalAttenuation ();
	else
		return false;
}


// ***************************************************************************
bool	CTransformShapeClipObs::clip(IBaseClipObs *caller) 
{
	CClipTrav			*trav= (CClipTrav*)Trav;
	CTransformShape		*m= (CTransformShape*)Model;

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
				// Ok, not shown
				return false;
		}

		// Else finer clip with pyramid.
		return m->Shape->clip(trav->WorldPyramid, HrcObs->WorldMatrix);
	}
	else
		return false;
}


// ***************************************************************************
void	CTransformShapeRenderObs::traverse(IObs *caller)
{
	CRenderTrav			*trav= (CRenderTrav*)Trav;
	CTransformShape		*m= (CTransformShape*)Model;
	IDriver				*drv= trav->getDriver();


	// if the transform is lightable (ie not a fully lightmaped model), setup lighting
	if(m->isLightable())
	{
		// useLocalAttenuation for this shape ??
		bool				useLocalAttenuation;
		if(m->Shape)
			useLocalAttenuation= m->Shape->useLightingLocalAttenuation ();
		else
			useLocalAttenuation= false;

		// Get HrcObs.
		CTransformHrcObs	*hrcObs= (CTransformHrcObs*)HrcObs;

		// the std case is to take my model lightContribution
		if(hrcObs->_AncestorSkeletonModel==NULL)
			trav->changeLightSetup(&m->getLightContribution(), useLocalAttenuation);
		// but if skinned/sticked (directly or not) to a skeleton, take its.
		else
			trav->changeLightSetup(&hrcObs->_AncestorSkeletonModel->getLightContribution(), useLocalAttenuation);
	}
	// else must disable the lightSetup
	else
	{
		// setting NULL will disable all lights
		trav->changeLightSetup(NULL, false);
	}


	// render the shape.
	if(m->Shape)
		m->Shape->render( drv, m, trav->isCurrentPassOpaque() );
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

	// If this isntance is binded or skinned to a skeleton, take the world matrix of this one as
	// center for LoadBalancing Resolution.
	if(skeleton)
	{
		// Take the root bone of the skeleton as reference (bone 0)
		// And so get our position.
		_ModelPos= skeleton->Bones[0].getWorldMatrix().getPos();
	}
	else
	{
		// get our position from 
		_ModelPos= HrcObs->WorldMatrix.getPos();
	}


	// Then compute distance from camera.
	_ModelDist= ( loadTrav->CamPos - _ModelPos).norm();


	// Get the number of triangles this model use now.
	_FaceCount= trans->getNumTriangles(_ModelDist);	
	loadTrav->NbFacePass0+= _FaceCount;
}


// ***************************************************************************
void	CTransformShapeLoadBalancingObs::traversePass1()
{
	CLoadBalancingTrav		*loadTrav= (CLoadBalancingTrav*)Trav;
	CTransformShape			*trans= static_cast<CTransformShape*>(Model);


	// Set the result into the isntance.
	trans->_NumTrianglesAfterLoadBalancing= loadTrav->computeModelNbFace(_FaceCount, _ModelDist);

}



} // NL3D
