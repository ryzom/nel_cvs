/** \file visual_collision_manager.cpp
 * <File description>
 *
 * $Id: visual_collision_manager.cpp,v 1.10 2004/05/07 11:41:11 berenguier Exp $
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

#include "3d/visual_collision_manager.h"
#include "3d/visual_collision_entity.h"
#include "3d/landscape.h"
#include "3d/camera_col.h"
#include "nel/misc/common.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
// Those blocks size are computed to be approximatively one block for 10 entities.
const	uint	TileDescNodeAllocatorBlockSize= 40000;
const	uint	PatchQuadBlockAllocatorBlockSize= 160;
// For Mesh QuadGrid
const	uint	MeshColQuadGridSize= 64;
const	float	MeshColQuadGridEltSize= 20;


// ***************************************************************************
CVisualCollisionManager::CVisualCollisionManager() : 
	_TileDescNodeAllocator(TileDescNodeAllocatorBlockSize), 
	_PatchQuadBlockAllocator(PatchQuadBlockAllocatorBlockSize)
{
	_Landscape= NULL;

	// Default.
	setSunContributionPower(0.5f, 0.5f);

	// init the mesh quadGrid
	_MeshQuadGrid.create(MeshColQuadGridSize, MeshColQuadGridEltSize);
	// valid id start at 1
	_MeshIdPool= 1;
}
// ***************************************************************************
CVisualCollisionManager::~CVisualCollisionManager()
{
	_Landscape= NULL;
}


// ***************************************************************************
void					CVisualCollisionManager::setLandscape(CLandscape *landscape)
{
	_Landscape= landscape;
}


// ***************************************************************************
CVisualCollisionEntity		*CVisualCollisionManager::createEntity()
{
	return new CVisualCollisionEntity(this);
}


// ***************************************************************************
void						CVisualCollisionManager::deleteEntity(CVisualCollisionEntity	*entity)
{
	delete entity;
}


// ***************************************************************************
CVisualTileDescNode		*CVisualCollisionManager::newVisualTileDescNode()
{
	return _TileDescNodeAllocator.allocate();
}

// ***************************************************************************
void					CVisualCollisionManager::deleteVisualTileDescNode(CVisualTileDescNode *ptr)
{
	_TileDescNodeAllocator.free(ptr);
}

// ***************************************************************************
CPatchQuadBlock			*CVisualCollisionManager::newPatchQuadBlock()
{
	return _PatchQuadBlockAllocator.allocate();
}

// ***************************************************************************
void					CVisualCollisionManager::deletePatchQuadBlock(CPatchQuadBlock *ptr)
{
	_PatchQuadBlockAllocator.free(ptr);
}


// ***************************************************************************
void					CVisualCollisionManager::setSunContributionPower(float power, float maxThreshold)
{
	NLMISC::clamp(power, 0.f, 1.f);

	for(uint i=0; i<256; i++)
	{
		float	f= i/255.f;
		f = powf(f/maxThreshold, power);
		sint	uf= (sint)floor(255*f);
		NLMISC::clamp(uf, 0, 255);
		_SunContributionLUT[i]= uf;
	}

}

// ***************************************************************************
// ***************************************************************************
// Camera collision
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
float					CVisualCollisionManager::getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone, bool playerIsInside)
{
	float	minCol= 1;

	// try col with landscape
	if(_Landscape)
	{
		minCol= _Landscape->getCameraCollision(start, end, radius, cone);
	}

	// try col with meshes
	CCameraCol		camCol;
	camCol.build(start, end, radius, cone);
	_MeshQuadGrid.select(camCol.BBox.getMin(), camCol.BBox.getMax());
	// try to intersect with any instance meshs
	CQuadGrid<CMeshInstanceCol*>::CIterator		it;
	for(it= _MeshQuadGrid.begin();it!=_MeshQuadGrid.end();it++)
	{
		// Skip this mesh according to special flag (known as the "matis serre bug")
		if((*it)->AvoidCollisionWhenPlayerInside && playerIsInside)
			continue;
		if((*it)->AvoidCollisionWhenPlayerOutside && !playerIsInside)
			continue;
		
		// collide
		float	meshCol= (*it)->getCameraCollision(camCol);
		// Keep only yhe smallest value
		minCol= min(minCol, meshCol);
	}

	return minCol;
}


// ***************************************************************************
float		CVisualCollisionManager::CMeshInstanceCol::getCameraCollision(CCameraCol &camCol)
{
	// if mesh still present (else it s may be an error....)
	if(Mesh)
	{
		// first test if intersect with the bboxes
		if(!camCol.BBox.intersect(WorldBBox))
			return 1;

		// get the collision with the mesh
		return Mesh->getCameraCollision(WorldMatrix, camCol);
	}
	else
		// no collision
		return 1;
}


// ***************************************************************************
uint					CVisualCollisionManager::addMeshInstanceCollision(CVisualCollisionMesh *mesh, const CMatrix &instanceMatrix, bool avoidCollisionWhenInside, bool avoidCollisionWhenOutside)
{
	if(!mesh)
		return 0;

	// allocate a new id
	uint32	id= _MeshIdPool++;

	// insert in map
	CMeshInstanceCol	&meshInst= _Meshs[id];

	// Build the col mesh instance
	meshInst.Mesh= mesh;
	meshInst.WorldMatrix= instanceMatrix;
	meshInst.WorldBBox= mesh->computeWorldBBox(instanceMatrix);
	meshInst.AvoidCollisionWhenPlayerInside= avoidCollisionWhenInside;
	meshInst.AvoidCollisionWhenPlayerOutside= avoidCollisionWhenOutside;
	
	// insert in quadGrid
	meshInst.QuadGridIt= _MeshQuadGrid.insert(meshInst.WorldBBox.getMin(), meshInst.WorldBBox.getMax(), &meshInst);

	return id;
}

// ***************************************************************************
void					CVisualCollisionManager::removeMeshCollision(uint id)
{
	// find in map
	TMeshColMap::iterator	it= _Meshs.find(id);
	if(it!=_Meshs.end())
	{
		// remove from the quadgrid
		_MeshQuadGrid.erase(it->second.QuadGridIt);
		// remove from the map
		_Meshs.erase(it);
	}
}


} // NL3D

