/** \file mesh_multi_lod_instance.cpp
 * An instance of CMeshMulitLod
 *
 * $Id: mesh_multi_lod_instance.cpp,v 1.8 2002/04/25 15:25:55 berenguier Exp $
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

#include "std3d.h"

#include "3d/mesh_multi_lod_instance.h"
#include "3d/mesh_multi_lod.h"
#include "3d/coarse_mesh_manager.h"
#include "3d/scene.h"

#include "nel/misc/debug.h"

using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

CMeshMultiLodInstance::CMeshMultiLodInstance ()
{
	// No flags
	Flags=0;
}

// ***************************************************************************

CMeshMultiLodInstance::~CMeshMultiLodInstance ()
{
	// Cast shape
	CMeshMultiLod *shape=safe_cast<CMeshMultiLod*> ((IShape*)Shape);

	// Manager pointer
	CCoarseMeshManager *manager;

	// Get the manager
	if (shape->isStatic ())
		manager=getScene()->getStaticCoarseMeshManager ();
	else
		manager=getScene()->getDynamicCoarseMeshManager ();

	// Manager ok ?
	if (manager)
	{
		// Coarse mesh loaded ?
		if (Flags&Coarse0Loaded)
		{
			manager->removeMesh (CoarseMeshId[0]);
		}
		if (Flags&Coarse1Loaded)
		{
			manager->removeMesh (CoarseMeshId[1]);
		}
	}
}

// ***************************************************************************

void		CMeshMultiLodInstance::registerBasic()
{
	CMOT::registerModel (MeshMultiLodInstanceId, MeshBaseInstanceId, CMeshMultiLodInstance::creator);
	CMOT::registerObs (LoadBalancingTravId, MeshMultiLodInstanceId, CMeshMultiLodBalancingObs::creator);
}

// ***************************************************************************

void		CMeshMultiLodBalancingObs::traverse(IObs *caller)
{
	// Call previous
	CTransformShapeLoadBalancingObs::traverse (caller);

	// If this is the second pass of LoadBalancing, choose the Lods, according to getNumTrianglesAfterLoadBalancing()
	CLoadBalancingTrav		*loadTrav= (CLoadBalancingTrav*)Trav;
	if(loadTrav->getLoadPass()==1)
	{
		// Get a pointer on the model
		CMeshMultiLodInstance *model=safe_cast<CMeshMultiLodInstance*> (Model);

		// Get a pointer on the shape
		CMeshMultiLod *shape=safe_cast<CMeshMultiLod*> ((IShape*)model->Shape);

		// Reset render pass
		model->setTransparency(false);
		model->setOpacity(false);

		// Get the wanted number of polygons
		float polygonCount=model->getNumTrianglesAfterLoadBalancing ();

		// Look for the good slot
		uint meshCount=shape->_MeshVector.size();
		model->Lod0=0;
		if (meshCount>1)
		{
			// Look for good i
			while ( polygonCount < shape->_MeshVector[model->Lod0].EndPolygonCount )
			{
				model->Lod0++;
				if (model->Lod0==meshCount-1)
					break;
			}
		}
		
		// The slot
		CMeshMultiLod::CMeshSlot	&slot=shape->_MeshVector[model->Lod0];

		// Get the distance with polygon count
		float distance=(polygonCount-slot.B)/slot.A;

		// Get the final polygon count
		if (slot.MeshGeom)
			model->PolygonCountLod0=slot.MeshGeom->getNumTriangles (distance);

		// Second slot in use ?
		model->Lod1=0xffffffff;

		// The next slot
		CMeshMultiLod::CMeshSlot	*nextSlot=NULL;

		// Next slot exist ?
		if (model->Lod0!=meshCount-1)
		{
			nextSlot=&(shape->_MeshVector[model->Lod0+1]);
		}

		// Max dist before blend
		float startBlend;
		if (nextSlot)
			startBlend=slot.DistMax-nextSlot->BlendLength;
		else
			startBlend=slot.DistMax-slot.BlendLength;

		// In blend zone ?
		if ( startBlend < distance )
		{
			// Alpha factor for main Lod
			model->BlendFactor = (slot.DistMax-distance)/(slot.DistMax-startBlend);
			if (model->BlendFactor<0)
				model->BlendFactor=0;
			nlassert (model->BlendFactor<=1);

			// Render this mesh
			if (slot.MeshGeom)
			{
				if (slot.Flags&CMeshMultiLod::CMeshSlot::BlendOut)
				{
					// Render the geom mesh with alpha blending with goodPolyCount
					model->setTransparency(true);
					model->Flags|=CMeshMultiLodInstance::Lod0Blend;
				}
				else
				{
					// Render the geom mesh without alpha blending with goodPolyCount
					model->setTransparency (slot.isTransparent());
					model->setOpacity (slot.isOpaque());
					model->Flags&=~CMeshMultiLodInstance::Lod0Blend;
				}
			}
			else
				model->Lod0=0xffffffff;

			// Next mesh, BlendIn actived ?
			if (nextSlot && shape->_MeshVector[model->Lod0+1].MeshGeom && (nextSlot->Flags&CMeshMultiLod::CMeshSlot::BlendIn))
			{
				// Render the geom mesh with alpha blending with nextSlot->BeginPolygonCount
				model->PolygonCountLod1=nextSlot->MeshGeom->getNumTriangles (distance);
				model->Lod1=model->Lod0+1;
				model->setTransparency(true);
			}
		}
		else
		{
			if (slot.MeshGeom)
			{
				// Render without blend with goodPolyCount
				model->setTransparency (slot.isTransparent());
				model->setOpacity (slot.isOpaque());
				model->Flags&=~CMeshMultiLodInstance::Lod0Blend;
			}
			else
				model->Lod0=0xffffffff;
		}
	}
}

// ***************************************************************************
void		CMeshMultiLodInstance::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	if(Shape)
	{
		// Get a pointer on the shape.
		CMeshMultiLod *pMesh =safe_cast<CMeshMultiLod*> ((IShape*)Shape);
		// Affect the mesh directly.
		pMesh->changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
	}
}

} // NL3D
