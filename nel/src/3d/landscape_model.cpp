/** \file landscape_model.cpp
 * <File description>
 *
 * $Id: landscape_model.cpp,v 1.33 2003/04/14 09:33:08 berenguier Exp $
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

#include <vector>
#include "nel/misc/hierarchical_timer.h"
#include "3d/landscape_model.h"
#include "3d/landscape.h"
#include "3d/cluster.h"
#include "3d/scene.h"
#include "3d/light_trav.h"
using namespace std;
using namespace NLMISC;

#define NL3D_MEM_LANDSCAPE					NL_ALLOC_CONTEXT( 3dLand )

namespace NL3D 
{



// ***************************************************************************
void	CLandscapeModel::registerBasic()
{
	CScene::registerModel(LandscapeModelId, TransformId, CLandscapeModel::creator);
}


// ***************************************************************************
CLandscapeModel::CLandscapeModel()
{
	Landscape.OwnerModel = this;
	Landscape.init();
	_ActiveAdditive=false;
	_Additive=1.f;

	// The model is renderable
	CTransform::setIsRenderable(true);

	// AnimDetail behavior: Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail(true);

	// RenderFilter: We are a Landscape
	_RenderFilterType= UScene::FilterLandscape;
}


// ***************************************************************************
void	CLandscapeModel::initModel()
{
	CTransform::initModel();

	// Enable the landscape to be clipped by the Cluster System.
	getOwnerScene()->getRoot()->clipDelChild(this);
	getOwnerScene()->getRootCluster()->clipAddChild(this);

	// After creating the landscape (and so the VegetableManager in the ctor).
	// we must init correclty the VegetableManager.
	Landscape.createVegetableBlendLayersModels(getOwnerScene());
}


// ***************************************************************************
void	CLandscapeModel::traverseClip()
{
	CClipTrav		&clipTrav= getOwnerScene()->getClipTrav();

	/// if already clipped in this frame, This means that we are visible from 2 CCluster.
	if (_ClipDate == clipTrav.CurrentDate)
	{
		// We may set the WorldFrustumPyramid. But do this only if not already set.
		if(!ClusteredPyramidIsFrustum)
		{
			/* If the current pyramid is different from the last stored pyramid, this mean per exemple
			 that the player is in a room and look the landscape from 2 windows through portals.
			 The other case is that the camera is in 2 clusters, so the clusterSystem start from this 2, and so we arrive 
			 here 2 times (but through the same portal => same pyramid obviously...)
			 */
			if( ClusteredPyramid!=clipTrav.WorldPyramid )
			{
				// in this rare case take the Whole frustum pyramid (a more correct code is to make an Union of the 2 pyramids...)
				ClusteredPyramid= clipTrav.WorldFrustumPyramid;
				ClusteredPyramidIsFrustum= true;
			}
		}
	}
	else
	{
		// If the camera is in Rootcluster or visible through a protal from one Room Cluster only, use the clustered pyramid.
		ClusteredPyramid= clipTrav.WorldPyramid;
		// We are not sure we are the FrustumPyramid
		ClusteredPyramidIsFrustum= false;
	}

	// Call normal CTransform traverseClip to insert this model in render list etc...
	// NB: CTransform::traverseClip() update _ClipDate...
	CTransform::traverseClip();
}


// ***************************************************************************
void	CLandscapeModel::traverseAnimDetail()
{
	// Father call (usefull?)
	CTransform::traverseAnimDetail();

	// The real Landscape clip is done in traverseAnimDetail
	H_AUTO( NL3D_Landscape_Clip );

	CClipTrav		&clipTrav= getOwnerScene()->getClipTrav();

	// Yes, this is ugly, but the clip pass is finished in render(), for clipping TessBlocks.
	// This saves an other Landscape patch traversal, so this is faster...
	// Order them in order which clip faster (first horizontal, then vertical).
	// NB: TessBlock are ALWAYS clipped with the frustum pyramid, not the clustered one (faster clip for most common cases).
	CurrentPyramid[0]= clipTrav.WorldFrustumPyramid[NL3D_CLIP_PLANE_LEFT];
	CurrentPyramid[1]= clipTrav.WorldFrustumPyramid[NL3D_CLIP_PLANE_RIGHT];
	CurrentPyramid[2]= clipTrav.WorldFrustumPyramid[NL3D_CLIP_PLANE_TOP];
	CurrentPyramid[3]= clipTrav.WorldFrustumPyramid[NL3D_CLIP_PLANE_BOTTOM];
	nlassert(NL3D_TESSBLOCK_NUM_CLIP_PLANE==4);

	// Before Landscape clip, must setup Driver, for good VB allocation.
	Landscape.setDriver(getOwnerScene()->getRenderTrav().getDriver());

	// Use the Clustered pyramid for Patch, but Frustum pyramid for TessBlocks.
	// We are sure that pyramid has normalized plane normals.
	Landscape.clip(clipTrav.CamPos, ClusteredPyramid);
}


// ***************************************************************************
void	CLandscapeModel::traverseRender()
{
	NL3D_MEM_LANDSCAPE

	CRenderTrav		&renderTrav= getOwnerScene()->getRenderTrav();

	// Change the landscape cetner. All Geomorphed pos (in VertexBuffer only or during VertexProgram)
	// substract this position.
	Landscape.setPZBModelPosition(renderTrav.CamPos);

	// setup the model matrix
	CMatrix		m;
	m.identity();
	// ZBuffer Precion: set the modelMatrix to the current landscape PZBModelPosition.
	// NB: don't use renderTrav.CamPos directly because setPZBModelPosition() may modify the position
	m.setPos(Landscape.getPZBModelPosition());
	renderTrav.getDriver()->setupModelMatrix(m);


	// Scene Time/Lighting Mgt.
	CScene		*scene= getOwnerScene();
	nlassert(scene);
	{
		// For vegetable, set the animation Time.
		Landscape.setVegetableTime(scene->getCurrentTime());

		// For vegetable updateLighting, set the system Time.
		Landscape.setVegetableUpdateLightingTime(scene->getCurrentSystemTime());

		// updateLighting
		H_BEFORE( NL3D_Landscape_UpdateLighting );
		Landscape.updateLighting(scene->getCurrentSystemTime());
		H_AFTER( NL3D_Landscape_UpdateLighting );

		// if SceneLighting enabled
		if(scene->isLightingSystemEnabled())
		{
			H_AUTO( NL3D_Landscape_DynamicLighting );

			// For vegetable, set the lighting 
			Landscape.setupVegetableLighting(scene->getSunAmbient(), scene->getSunDiffuse(), 
				scene->getSunDirection());

			// current visible Dynamic light list are registered in LightTrav::LightingManager
			CLightTrav		&lightTrav= scene->getLightTrav();
			// Get all dynamic light list, and light landscape with it.
			Landscape.computeDynamicLighting(lightTrav.LightingManager.getAllDynamicLightList());
		}
	}

	// First, refine.
	H_BEFORE( NL3D_Landscape_Refine );
	Landscape.refine(renderTrav.CamPos);
	H_AFTER( NL3D_Landscape_Refine );

	// then render.
	H_BEFORE( NL3D_Landscape_Render );
	Landscape.render(renderTrav.CamPos, renderTrav.CamLook, CurrentPyramid, isAdditive ());
	H_AFTER( NL3D_Landscape_Render );
}



} // NL3D
