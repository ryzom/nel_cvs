/** \file shadow_poly_receiver.cpp
 * <File description>
 *
 * $Id: shadow_poly_receiver.cpp,v 1.3 2004/03/03 18:59:54 berenguier Exp $
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

#include "nel/misc/hierarchical_timer.h"
#include "3d/shadow_poly_receiver.h"
#include "3d/shadow_map.h"
#include "3d/driver.h"


using	namespace std;
using	namespace NLMISC;


namespace NL3D {


// ***************************************************************************
CShadowPolyReceiver::CShadowPolyReceiver(uint quadGridSize, float quadGridCellSize)
{
	_Vertices.reserve(64);
	_FreeVertices.reserve(64);
	_FreeTriangles.reserve(64);
	_Triangles.reserve(64);

	_TriangleGrid.create(quadGridSize, quadGridCellSize);

	_VB.setVertexFormat(CVertexBuffer::PositionFlag);
}


// ***************************************************************************
uint			CShadowPolyReceiver::addTriangle(const NLMISC::CTriangle &tri)
{
	uint	id;

	// Look for a free triangle entry.
	if(_FreeTriangles.empty())
	{
		_Triangles.push_back(TTriangleGrid::CIterator());
		id= _Triangles.size()-1;
		// enlarge render size.
		_RenderTriangles.resize(_Triangles.size() * 3);
	}
	else
	{
		id= _FreeTriangles.back();
		_FreeTriangles.pop_back();
	}

	// Allocate vertices, reusing same ones.
	CTriangleId		triId;
	CVector			v[3];
	v[0]= tri.V0;
	v[1]= tri.V1;
	v[2]= tri.V2;
	for(uint i=0;i<3;i++)
	{
		// Find the vertex in the map.
		TVertexMap::iterator	it;
		it= _VertexMap.find(v[i]);
		// if not found, allocate it
		if(it==_VertexMap.end())
		{
			triId.Vertex[i]= allocateVertex(v[i]);
		}
		// else get its id
		else
		{
			triId.Vertex[i]= it->second;
		}

		// increment the reference of this vertex
		incVertexRefCount(triId.Vertex[i]);
	}
	
	// Insert the triangle in the quadGrid.
	CAABBox		bb;
	bb.setCenter(tri.V0);
	bb.extend(tri.V1);
	bb.extend(tri.V2);
	// insert in QuadGrid and store iterator for future remove
	_Triangles[id]= _TriangleGrid.insert(bb.getMin(), bb.getMax(), triId);

	return id;
}

// ***************************************************************************
void			CShadowPolyReceiver::removeTriangle(uint id)
{
	nlassert(id<_Triangles.size());
	// Must not be NULL iterator.
	nlassert(_Triangles[id]!=_TriangleGrid.end());

	// Release Vertices
	const CTriangleId		&triId= *_Triangles[id];
	releaseVertex(triId.Vertex[0]);
	releaseVertex(triId.Vertex[1]);
	releaseVertex(triId.Vertex[2]);

	// Delete Triangle.
	_TriangleGrid.erase(_Triangles[id]);
	_Triangles[id]= _TriangleGrid.end();
	// Append to free list.
	_FreeTriangles.push_back(id);
}


// ***************************************************************************
uint			CShadowPolyReceiver::allocateVertex(const CVector &v)
{
	uint	id;

	// Look for a free vertex entry.
	if(_FreeVertices.empty())
	{
		// Add the vertex, and init refCount to 0.
		_Vertices.push_back(v);
		id= _Vertices.size()-1;

		// Resize the VBuffer at max possible
		_VB.setNumVertices(_Vertices.size());
	}
	else
	{
		id= _FreeVertices.back();
		_FreeVertices.pop_back();
		// init entry
		_Vertices[id]= v;
		_Vertices[id].RefCount= 0;
	}

	// insert in the map (should not be here)
	_VertexMap.insert( make_pair(v, id) );

	return id;
}

// ***************************************************************************
void			CShadowPolyReceiver::releaseVertex(uint id)
{
	nlassert(id<_Vertices.size());
	// dec ref
	nlassert(_Vertices[id].RefCount>0);
	_Vertices[id].RefCount--;
	// no more used?
	if(_Vertices[id].RefCount==0)
	{
		// Free it.
		_FreeVertices.push_back(id);
		// Remove it from map.
		TVertexMap::iterator	it= _VertexMap.find(_Vertices[id]);
		nlassert(it!=_VertexMap.end());
		_VertexMap.erase(it);
	}
}

// ***************************************************************************
void			CShadowPolyReceiver::incVertexRefCount(uint id)
{
	nlassert(id<_Vertices.size());
	nlassert(_Vertices[id].RefCount < NL3D_SPR_MAX_REF_COUNT);
	_Vertices[id].RefCount++;
}


// ***************************************************************************
void			CShadowPolyReceiver::render(IDriver *drv, CMaterial &shadowMat, const CShadowMap *shadowMap, const CVector &casterPos, const CVector &vertDelta)
{
	uint	i, j;

	// **** Fill Triangles that are hit by the Caster
	// First select with quadGrid
	CAABBox		worldBB;
	worldBB= shadowMap->LocalBoundingBox;
	worldBB.setCenter(worldBB.getCenter() + casterPos);
	_TriangleGrid.select(worldBB.getMin(), worldBB.getMax());

	// For all triangles, reset vertices flags.
	TTriangleGrid::CIterator	it;
	for(it=_TriangleGrid.begin();it!=_TriangleGrid.end();it++)
	{
		CTriangleId		&triId= *it;
		for(i=0;i<3;i++)
		{
			_Vertices[triId.Vertex[i]].Flags= 0;
			_Vertices[triId.Vertex[i]].VBIdx= -1;
		}
	}

	// Copute the world Clip Volume
	static	std::vector<CPlane>		worldClipPlanes;
	CMatrix		worldMat;
	// set -casterPos, because to transform a plane, we must do plane * M-1
	worldMat.setPos(-casterPos);
	// Allow max bits of planes clip.
	worldClipPlanes.resize(min((uint)shadowMap->LocalClipPlanes.size(), (uint)NL3D_SPR_NUM_CLIP_PLANE));
	// Transform into world
	for(i=0;i<worldClipPlanes.size();i++)
	{
		worldClipPlanes[i]= shadowMap->LocalClipPlanes[i] * worldMat;
	}

	// For All triangles, clip them.
	uint	currentTriIdx= 0;
	uint	currentVbIdx= 0;
	for(it=_TriangleGrid.begin();it!=_TriangleGrid.end();it++)
	{
		CTriangleId		&triId= *it;
		uint			triFlag= NL3D_SPR_NUM_CLIP_PLANE_MASK;

		// for all vertices, clip them
		for(i=0;i<3;i++)
		{
			uint	vid= triId.Vertex[i];
			uint	vertexFlags= _Vertices[vid].Flags;

			// if this vertex is still not computed
			if(!vertexFlags)
			{
				// For all planes of the Clip Volume, clip this vertex.
				for(j=0;j<worldClipPlanes.size();j++)
				{
					// out if in front
					bool	out= worldClipPlanes[j]*_Vertices[vid] > 0;

					vertexFlags|= ((uint)out)<<j;
				}

				// add the bit flag to say "computed".
				vertexFlags|= NL3D_SPR_NUM_CLIP_PLANE_SHIFT;

				// store
				_Vertices[vid].Flags= vertexFlags;
			}

			// And all vertex bits.
			triFlag&= vertexFlags;
		}

		// if triangle not clipped, add the triangle
		if( (triFlag & NL3D_SPR_NUM_CLIP_PLANE_MASK)==0 )
		{
			// Add the 3 vertices to the VB, and to the index buffer.
			for(i=0;i<3;i++)
			{
				uint	vid= triId.Vertex[i];
				sint	vbId= _Vertices[vid].VBIdx;

				// if not yet inserted in the VB, do it.
				if(vbId==-1)
				{
					// allocate a new place in the VBuffer
					vbId= currentVbIdx++;
					_Vertices[vid].VBIdx= vbId;
					// set the coord
					_VB.setVertexCoord(vbId, _Vertices[vid]+vertDelta);
				}

				// add the index to the tri list.
				_RenderTriangles[currentTriIdx++]= vbId;
			}
		}
	}


	// **** Render
	drv->activeVertexBuffer(_VB);
	drv->renderTriangles(shadowMat, &_RenderTriangles[0], currentTriIdx/3);
	// TestYoyo. Show in Red triangles selected
	/*static	CMaterial	tam;
	tam.initUnlit();
	tam.setColor(CRGBA(255,0,0,128));
	tam.setZFunc(CMaterial::always);
	tam.setZWrite(false);
	tam.setBlend(true);
	tam.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
	tam.setDoubleSided(true);
	drv->renderTriangles(tam, &_RenderTriangles[0], currentTriIdx/3);*/
}


// ***************************************************************************
float			CShadowPolyReceiver::getCylinderCollision(const CVector &start, const CVector &end, float radius, bool cone)
{
	H_AUTO( NL3D_getRayCollision )

	// **** build the cylinder pyramid
	// approximate with a box
	CMatrix		mat;
	mat.setRot(CVector::I, (start-end).normed(), CVector::K);
	mat.normalize(CMatrix::YZX);
	// build the start 4 points
	CVector		ps[4];
	uint		NPlanes;
	// cone or cylinder?
	if(cone)
	{
		NPlanes= 5;
		ps[0]= start;
		ps[1]= start;
		ps[2]= start;
		ps[3]= start;
	}
	else
	{
		NPlanes= 6;
		mat.setPos(start);
		ps[0]= mat * CVector(radius, 0, -radius);
		ps[1]= mat * CVector(radius, 0, radius);
		ps[2]= mat * CVector(-radius, 0, radius);
		ps[3]= mat * CVector(-radius, 0, -radius);
	}
	// build the end 4 points
	CVector		pe[4];
	mat.setPos(end);
	pe[0]= mat * CVector(radius, 0, -radius);
	pe[1]= mat * CVector(radius, 0, radius);
	pe[2]= mat * CVector(-radius, 0, radius);
	pe[3]= mat * CVector(-radius, 0, -radius);
	CPlane	pyramid[6];
	// try to roder for optimisation
	// left/right
	pyramid[0].make(ps[3], pe[3], pe[2]);
	pyramid[1].make(ps[1], pe[1], pe[0]);
	// end
	pyramid[2].make(pe[0], pe[1], pe[2]);
	// top-bottom
	pyramid[3].make(ps[2], pe[2], pe[1]);
	pyramid[4].make(ps[0], pe[0], pe[3]);
	// start if not cone
	if(!cone)
		pyramid[5].make(ps[0], ps[2], ps[1]);
	
	// **** select with quadGrid
	CAABBox		worldBB;
	worldBB.setCenter(start);
	worldBB.extend(end);
	_TriangleGrid.select(worldBB.getMin(), worldBB.getMax());
	
	// **** For all triangles, test if intersect the pyramid
	TTriangleGrid::CIterator	it;
	CVector		arrayIn[3+6];
	CVector		arrayOut[3+6];
	CVector		*pIn= arrayIn;
	CVector		*pOut= arrayOut;
	float		sqrShortDist= FLT_MAX;
	for(it=_TriangleGrid.begin();it!=_TriangleGrid.end();it++)
	{
		CTriangleId		&triId= *it;
		// build the triangle
		pIn[0]= _Vertices[triId.Vertex[0]];
		pIn[1]= _Vertices[triId.Vertex[1]];
		pIn[2]= _Vertices[triId.Vertex[2]];
		sint	nVert= 3;
		// clip against the pyramid
		for(uint i=0;i<NPlanes;i++)
		{
			nVert= pyramid[i].clipPolygonBack(pIn, pOut, nVert);
			swap(pIn, pOut);
			if(!nVert)
				break;
		}
		// if clipped => collision
		if(nVert)
		{
			// get the nearest distance
			for(sint i=0;i<nVert;i++)
			{
				float	sqrDist= (pIn[i]-start).sqrnorm();
				if(sqrDist<sqrShortDist)
					sqrShortDist= sqrDist;
			}
		}
	}
			
	// **** return the collision found, between [0,1]
	if(sqrShortDist == FLT_MAX)
		return 1;
	else
	{
		float	f= 1;
		float	d= (end-start).norm();
		if(d>0)
		{
			f= sqrtf(sqrShortDist) / d;
			f= min(f, 1.f);
		}
		return f;
	}

}



} // NL3D
