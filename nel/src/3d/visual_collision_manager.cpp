/** \file visual_collision_manager.cpp
 * <File description>
 *
 * $Id: visual_collision_manager.cpp,v 1.8 2004/03/12 16:27:52 berenguier Exp $
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
float					CVisualCollisionManager::getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone)
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
	// try to intersect with any meshs
	CQuadGrid<CMeshCol*>::CIterator		it;
	for(it= _MeshQuadGrid.begin();it!=_MeshQuadGrid.end();it++)
	{
		float	meshCol= (*it)->getCameraCollision(camCol);
		// Keep only yhe smallest value
		minCol= min(minCol, meshCol);
	}

	return minCol;
}


// ***************************************************************************
void	CVisualCollisionManager::CStaticGrid::create(uint nbQuads, uint nbElts, const NLMISC::CAABBox &gridBBox)
{
	nlassert(nbQuads>0 && isPowerOf2(nbQuads));

	// init the grid
	_GridSize= nbQuads;
	_GridSizePower= getPowerOf2(nbQuads);
	_Grid.resize(_GridSize*_GridSize);
	// start with 0 elt in each case
	memset(_Grid.getPtr(), 0, _Grid.size() * sizeof(CCase));

	// init the Elt Build
	_EltBuild.resize(nbElts);

	// total size is 0
	_GridDataSize= 0;

	// bbox init
	_GridPos= gridBBox.getMin();
	_GridScale= gridBBox.getSize();
	_GridScale.x= _GridSize / _GridScale.x;
	_GridScale.y= _GridSize / _GridScale.y;

	// reset intersection data
	_ItSession= 0;
}

// ***************************************************************************
void	CVisualCollisionManager::CStaticGrid::add(uint16 id, const NLMISC::CAABBox &bbox)
{
	CVector	minp= bbox.getMin() - _GridPos;
	CVector	maxp= bbox.getMax() - _GridPos;

	// compute the 2D bbox
	sint	xmin= (sint)floorf(minp.x*_GridScale.x);
	sint	ymin= (sint)floorf(minp.y*_GridScale.y);
	sint	xmax= (sint)ceilf(maxp.x*_GridScale.x);
	sint	ymax= (sint)ceilf(maxp.y*_GridScale.y);
	clamp(xmin, 0, (sint)_GridSize-1);
	clamp(ymin, 0, (sint)_GridSize-1);
	clamp(xmax, 0, (sint)_GridSize);
	clamp(ymax, 0, (sint)_GridSize);

	// set in the elt build
	_EltBuild[id].X0= xmin;
	_EltBuild[id].Y0= ymin;
	_EltBuild[id].X1= xmax;
	_EltBuild[id].Y1= ymax;

	// for each case touched, increment NumElts
	for(uint y=ymin;y<(uint)ymax;y++)
	{
		for(uint x=xmin;x<(uint)xmax;x++)
		{
			_Grid[(y<<_GridSizePower)+x].NumElts++;
			_GridDataSize++;
		}
	}
}

// ***************************************************************************
void	CVisualCollisionManager::CStaticGrid::compile()
{
	uint	i;

	// create the data
	_GridData.resize(_GridDataSize);

	// Init Start ptr for each case
	uint	idx= 0;
	for(i=0;i<_Grid.size();i++)
	{
		_Grid[i].Start= idx;
		idx+= _Grid[i].NumElts;
		// reset NumElts, because use it like an index below
		_Grid[i].NumElts= 0;
	}
	nlassert(_GridDataSize==idx);

	// For each element, fill the grid and grid data
	for(i=0;i<_EltBuild.size();i++)
	{
		CEltBuild	&eb= _EltBuild[i];

		for(uint y=eb.Y0;y<eb.Y1;y++)
		{
			for(uint x=eb.X0;x<eb.X1;x++)
			{
				CCase	&gcase= _Grid[(y<<_GridSizePower)+x];
				uint	idx= gcase.Start + gcase.NumElts;
				// store the idx
				_GridData[idx]= i;
				// increment the number of elements for this case
				gcase.NumElts++;
			}
		}
	}

	// create the temp array used for intersection test
	_Sessions.resize(_EltBuild.size());
	_Sessions.fill(0);
	
	// clear no more needed data
	_EltBuild.clear();
}

// ***************************************************************************
uint	CVisualCollisionManager::CStaticGrid::select(const NLMISC::CAABBox &bbox, std::vector<uint16> &res)
{
	// increment the intersection session
	_ItSession++;
	// enlarge the result array as needed
	if(res.size()<_Sessions.size())
		res.resize(_Sessions.size());
	// the number of selected element
	uint	numSel= 0;

	// compute the 2D bbox
	CVector	minp= bbox.getMin() - _GridPos;
	CVector	maxp= bbox.getMax() - _GridPos;
	sint	xmin= (sint)floorf(minp.x*_GridScale.x);
	sint	ymin= (sint)floorf(minp.y*_GridScale.y);
	sint	xmax= (sint)ceilf(maxp.x*_GridScale.x);
	sint	ymax= (sint)ceilf(maxp.y*_GridScale.y);
	clamp(xmin, 0, (sint)_GridSize-1);
	clamp(ymin, 0, (sint)_GridSize-1);
	clamp(xmax, 0, (sint)_GridSize);
	clamp(ymax, 0, (sint)_GridSize);

	// for each case touched, increment NumElts
	for(uint y=ymin;y<(uint)ymax;y++)
	{
		for(uint x=xmin;x<(uint)xmax;x++)
		{
			CCase	&gcase= _Grid[(y<<_GridSizePower)+x];
			// for each element in this case
			for(uint i= gcase.Start;i<gcase.Start + gcase.NumElts;i++)
			{
				uint	elt= _GridData[i];

				// if not alread inserted in the dest
				if(_Sessions[elt]!=_ItSession)
				{
					// mark as intersected
					_Sessions[elt]= _ItSession;
					// append
					res[numSel++]= elt;
				}
			}
		}
	}
	
	// return the number of selected elements
	return numSel;
}


// ***************************************************************************
float					CVisualCollisionManager::CMeshCol::getCameraCollision(CCameraCol &camCol)
{
	// first test if intersect with the bboxes
	if(!camCol.BBox.intersect(BBox))
		return 1;

	// Select triangles
	static std::vector<uint16>		selection;
	uint	numSel= QuadGrid.select(camCol.BBox, selection);

	// **** For all triangles, test if intersect the camera collision
	float		sqrMinDist= FLT_MAX;
	for(uint i=0;i<numSel;i++)
	{
		uint			triId= selection[i];
		// build the triangle
		camCol.minimizeDistanceAgainstTri(
			Vertices[Triangles[triId*3+0]],
			Vertices[Triangles[triId*3+1]],
			Vertices[Triangles[triId*3+2]],
			sqrMinDist);
	}
	
	// **** return the collision found, between [0,1]
	if(sqrMinDist == FLT_MAX)
		return 1;
	else
	{
		float	f= 1;
		float	d= (camCol.End-camCol.Start).norm();
		if(d>0)
		{
			f= sqrtf(sqrMinDist) / d;
			f= min(f, 1.f);
		}
		return f;
	}
}

// ***************************************************************************
void					CVisualCollisionManager::CMeshCol::build(const std::vector<CVector> &vertices, const std::vector<uint16> &triangles)
{
	uint	i;
	nlassert(!vertices.empty());

	// copy
	Vertices= vertices;
	Triangles= triangles;
	
	// Build the bbox for this col mesh
	BBox.setCenter(vertices[0]);
	for(i=1;i<vertices.size();i++)
		BBox.extend(vertices[i]);

	// Build the Static Grid
	uint	numTris= triangles.size()/3;
	QuadGrid.create(16, numTris, BBox);
	// Add all triangles
	for(i=0;i<numTris;i++)
	{
		CAABBox		bb;
		bb.setCenter(Vertices[Triangles[i*3+0]]);
		bb.extend(Vertices[Triangles[i*3+1]]);
		bb.extend(Vertices[Triangles[i*3+2]]);
		QuadGrid.add(i, bb);
	}
	// compile
	QuadGrid.compile();
}

// ***************************************************************************
uint					CVisualCollisionManager::addMeshCollision(const std::vector<CVector> &vertices, const std::vector<uint16> &triangles)
{
	if(vertices.empty() || triangles.empty())
		return 0;
	// triangles id are stored in uint16 form. so their should not be more than 65535*3 indices
	if(vertices.size()>65535 || triangles.size()>65535*3)
		return 0;

	// allocate a new id
	uint32	id= _MeshIdPool++;

	// insert in map
	CMeshCol	&mesh= _Meshs[id];

	// Build the col mesh
	mesh.build(vertices, triangles);

	// insert in quadGrid
	mesh.QuadGridIt= _MeshQuadGrid.insert(mesh.BBox.getMin(), mesh.BBox.getMax(), &mesh);

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

