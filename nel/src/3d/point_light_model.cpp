/** \file point_light_model.cpp
 * <File description>
 *
 * $Id: point_light_model.cpp,v 1.1 2002/02/06 16:54:56 berenguier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "3d/point_light_model.h"
#include "3d/light_trav.h"
#include "3d/root_model.h"
#include "3d/skeleton_model.h"


namespace NL3D {


// ***************************************************************************
void	CPointLightModel::registerBasic()
{
	CMOT::registerModel( PointLightModelId, TransformId, CPointLightModel::creator);
	CMOT::registerObs( LightTravId,			PointLightModelId, CPointLightModelLightObs::creator );
}

// ***************************************************************************
CPointLightModel::CPointLightModel()
{
	_DeltaPosToSkeletonWhenOutOfFrustum.set(0, 0, 1.5f);
}


// ***************************************************************************
CPointLightModel::~CPointLightModel()
{
}


// ***************************************************************************
void	CPointLightModel::initModel()
{
	CTransform::initModel();

	// link to the LightModelRoot in the lightTrav.
	IObs		*obs= getObs(LightTravId);
	CLightTrav	*lightTrav= (CLightTrav*)obs->Trav;
	nlassert( lightTrav->LightModelRoot );
	// link me to the root of light.
	lightTrav->link(lightTrav->LightModelRoot, this);
	
}


// ***************************************************************************
void			CPointLightModel::setDeltaPosToSkeletonWhenOutOfFrustum(const CVector &deltaPos)
{
	_DeltaPosToSkeletonWhenOutOfFrustum= deltaPos;
}


// ***************************************************************************
const CVector	&CPointLightModel::getDeltaPosToSkeletonWhenOutOfFrustum() const
{
	return _DeltaPosToSkeletonWhenOutOfFrustum;
}


// ***************************************************************************
void	CPointLightModelLightObs::traverse(IObs *caller)
{
	CPointLightModel	*plModel= (CPointLightModel*)Model;
	CLightTrav			*lightTrav= (CLightTrav*)Trav;

	// Note: any dynamic light is supposed to always move each frame, so they are re-inserted in the 
	// quadGrid each frame.


	// reset all models lighted by this light.
	// Then models are marked dirty and their light setup is reseted
	plModel->PointLight.resetLightedModels();


	// if the light is visible (ie not hiden)
	if( plModel->isHrcVisible() )
	{
		// If the light is not hidden by any skeleton.
		if( plModel->isClipVisible() )
		{
			// recompute the worldPosition of the light.
			plModel->PointLight.setPosition( plModel->getWorldMatrix().getPos() );
		}
		else
		{
			// We are hidden because a skeleton has hide us (or else don't know why).
			nlassert(plModel->getHrcObs()->_AncestorSkeletonModel);
			const CMatrix &skMatrix= plModel->getHrcObs()->_AncestorSkeletonModel->getWorldMatrix();

			plModel->PointLight.setPosition( skMatrix * plModel->_DeltaPosToSkeletonWhenOutOfFrustum );
		}

		// now, insert this light in the quadGrid. NB: in CLightTrav::traverse(), the quadGrid is cleared before here.
		// This light will touch (resetLighting()) any model it may influence.
		lightTrav->LightingManager.addDynamicLight(&plModel->PointLight);
	}

}



} // NL3D
