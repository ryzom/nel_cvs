/** \file visual_collision_mesh.cpp
 * <File description>
 *
 * $Id: visual_collision_mesh.cpp,v 1.1 2004/03/23 15:38:43 berenguier Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#include "3d/visual_collision_mesh.h"
#include "3d/quad_grid.h"
#include "3d/camera_col.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// CStaticGrid
// ***************************************************************************
// ***************************************************************************
	

// ***************************************************************************
void	CVisualCollisionMesh::CStaticGrid::create(uint nbQuads, uint nbElts, const NLMISC::CAABBox &gridBBox)
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
void	CVisualCollisionMesh::CStaticGrid::add(uint16 id, const NLMISC::CAABBox &bbox)
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
void	CVisualCollisionMesh::CStaticGrid::compile()
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
uint	CVisualCollisionMesh::CStaticGrid::select(const NLMISC::CAABBox &bbox, std::vector<uint16> &res)
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
// ***************************************************************************
// CVisualCollisionMesh
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CVisualCollisionMesh::CVisualCollisionMesh()
{
}

// ***************************************************************************
bool					CVisualCollisionMesh::build(const std::vector<CVector> &vertices, const std::vector<uint32> &triangles)
{
	uint	i;
	// if no vertices, or no triangles, abort
	if(vertices.empty())
		return false;
	if(triangles.empty())
		return false;
	// vertices and triangles id are stored in uint16 form. so their should not be more than 65535*3 indices
	if(vertices.size()>65535 || triangles.size()>65535*3)
		return false;
	
	// copy
	Vertices= vertices;
	
	// compress indexes to 16 bits
	Triangles.resize(triangles.size());
	for(i=0;i<Triangles.size();i++)
		Triangles[i]= (uint16)triangles[i];
	
	// Build the Local bbox for this col mesh
	CAABBox		localBBox;
	localBBox.setCenter(vertices[0]);
	for(i=1;i<vertices.size();i++)
		localBBox.extend(vertices[i]);
	
	// Build the Static Grid
	uint	numTris= triangles.size()/3;
	QuadGrid.create(16, numTris, localBBox);
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
	
	return true;
}

// ***************************************************************************
float					CVisualCollisionMesh::getCameraCollision(const CMatrix &instanceMatrix, CCameraCol &camCol)
{
	// Make the Camera Collision local to the mesh!
	CCameraCol	camColLocal;
	camColLocal.setApplyMatrix(camCol, instanceMatrix.inverted());


	// Select triangles
	static std::vector<uint16>		selection;
	uint	numSel= QuadGrid.select(camColLocal.BBox, selection);
	
	// **** For all triangles, test if intersect the camera collision
	float		sqrMinDist= FLT_MAX;
	for(uint i=0;i<numSel;i++)
	{
		uint			triId= selection[i];
		// build the triangle
		camColLocal.minimizeDistanceAgainstTri(
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
		float	d= (camColLocal.End-camColLocal.Start).norm();
		if(d>0)
		{
			f= sqrtf(sqrMinDist) / d;
			f= min(f, 1.f);
		}
		return f;
	}
}

// ***************************************************************************
NLMISC::CAABBox			CVisualCollisionMesh::computeWorldBBox(const CMatrix &instanceMatrix)
{
	CAABBox		ret;
	if(!Vertices.empty())
	{
		ret.setCenter(instanceMatrix*Vertices[0]);
		for(uint i=1;i<Vertices.size();i++)
		{
			ret.extend(instanceMatrix*Vertices[i]);
		}
	}

	return ret;
}

} // NL3D
