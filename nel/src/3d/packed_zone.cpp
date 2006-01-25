/** \file packed_zone.cpp
 * Collision on a set of packed zones (packed representation)
 *
 * $Id:
 */

/* Copyright, 2000-2006 Nevrax Ltd.
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
//
#include "packed_zone.h"
#include "driver.h"
#include "material.h"
//
#include "nel/misc/matrix.h"
#include "nel/misc/polygon.h"
#include "nel/misc/path.h"
#include "nel/misc/grid_traversal.h"
//

using namespace NLMISC;

namespace NL3D
{

// some function to ease writing of some primitives into a vertex buffer
static inline void pushVBLine2D(NLMISC::CVector *&dest, const NLMISC::CVector &v0, const NLMISC::CVector &v1)
{
	dest->x = v0.x;
	dest->y = v0.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = v1.x;
	dest->y = v1.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = v0.x;
	dest->y = v0.y;
	dest->z = -0.5f;
	++ dest;
}

static inline void pushVBTri2D(NLMISC::CVector *&dest, const NLMISC::CTriangle &tri)
{
	dest->x = tri.V0.x;
	dest->y = tri.V0.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = tri.V1.x;
	dest->y = tri.V1.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = tri.V2.x;
	dest->y = tri.V2.y;
	dest->z = -0.5f;
	++ dest;
}


static inline void pushVBQuad2D(NLMISC::CVector *&dest, const NLMISC::CQuad &quad)
{
	dest->x = quad.V0.x;
	dest->y = quad.V0.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = quad.V1.x;
	dest->y = quad.V1.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = quad.V2.x;
	dest->y = quad.V2.y;
	dest->z = -0.5f;
	++ dest;
	dest->x = quad.V3.x;
	dest->y = quad.V3.y;
	dest->z = -0.5f;
	++ dest;
}

static inline void pushVBQuad(NLMISC::CVector *&dest, const NLMISC::CQuad &quad)
{
	*dest++ = quad.V0;
	*dest++ = quad.V1;
	*dest++ = quad.V2;
	*dest++ = quad.V3;	
}


// compute rasters union. 
static void computeRastersUnion(const CPolygon2D::TRasterVect &inRaster0, CPolygon2D::TRasterVect &inRaster1, sint minY0, sint minY1,
						 CPolygon2D::TRasterVect &outRaster, sint &finalMinY)
{
	if (inRaster0.empty())
	{
		if (inRaster1.empty())
		{
			outRaster.empty();
			finalMinY = -1;
			return;
		}
		outRaster = inRaster1;
		finalMinY = minY1;
		return;
	}
	else if (inRaster1.empty())
	{
		outRaster = inRaster0;
		finalMinY = minY0;
		return;
	}
	nlassert(&outRaster != &inRaster0);
	nlassert(&outRaster != &inRaster1);
	finalMinY = std::min(minY0, minY1);	
	sint maxY = std::max(minY0 + (sint) inRaster0.size(), minY1 + (sint) inRaster1.size());	
	outRaster.resize(maxY - finalMinY);
	for(sint y = 0; y < (sint) outRaster.size(); ++y)
	{
		outRaster[y].first  = INT_MAX;
		outRaster[y].second = INT_MIN;		
		sint raster0Y = y + finalMinY - minY0;
		if (raster0Y >= 0  && raster0Y < (sint) inRaster0.size())
		{
			//if (inRaster0[raster0Y].second >= inRaster0[raster0Y].first)
			{
				outRaster[y].first = std::min(outRaster[y].first, inRaster0[raster0Y].first);
				outRaster[y].second = std::max(outRaster[y].second, inRaster0[raster0Y].second);
			}
		}
		sint raster1Y = y + finalMinY - minY1;
		if (raster1Y >= 0  && raster1Y < (sint) inRaster1.size())
		{
			//if (inRaster1[raster1Y].second >= inRaster1[raster1Y].first)
			{
				outRaster[y].first = std::min(outRaster[y].first, inRaster1[raster1Y].first);
				outRaster[y].second = std::max(outRaster[y].second, inRaster1[raster1Y].second);
			}
		}
	}
}

static void addQuadToSilhouette(const CVector &v0, const CVector &v1, const CVector &v2, CVector &v3, CPolygon2D::TRasterVect &sil, sint &minY, float cellSize)
{
	static CPolygon2D quad;
	quad.Vertices.resize(4);
	quad.Vertices[0] = v0 / cellSize;
	quad.Vertices[1] = v1 / cellSize;
	quad.Vertices[2] = v2 / cellSize;
	quad.Vertices[3] = v3 / cellSize ;
	//
	static CPolygon2D::TRasterVect newQuad;
	static CPolygon2D::TRasterVect result;
	sint newMinY = -1;
	sint resultMinY;
	quad.computeOuterBorders(newQuad, newMinY);
	computeRastersUnion(sil, newQuad, minY, newMinY, result, resultMinY);
	minY = resultMinY;
	sil.swap(result);
}


//***************************************************************************************
void CPackedZone32::unpackTri(const CPackedTri &src, CVector dest[3]) const
{	
	// TODO: add 'multiply-add' operator
	dest[0].set(Verts[src.V0].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V0].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V0].Z * _PackedLocalToWorld.z + _Origin.z);
	dest[1].set(Verts[src.V1].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V1].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V1].Z * _PackedLocalToWorld.z + _Origin.z);
	dest[2].set(Verts[src.V2].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V2].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V2].Z * _PackedLocalToWorld.z + _Origin.z);

}


//***************************************************************************************
CPackedZone32::CPackedZone32()
{
	CellSize = 0;	
}

//***************************************************************************************
void CPackedZone32::serial(NLMISC::IStream &f) throw (NLMISC::EStream)
{
	f.serialVersion(0);
	f.serial(Box);
	f.serialCont(Verts);
	f.serialCont(Tris);
	f.serialCont(TriLists);
	f.serial(Grid);
	f.serial(CellSize);
	f.serial(_Origin);
	f.serial(_WorldToLocal);
	f.serial(ZoneX);
	f.serial(ZoneY);
}

// used by CPackedZone::build
struct CZoneInstance
{
	const CShapeInfo *SI;
	CMatrix    WorldMat;
};

//***************************************************************************************
void CPackedZone32::build(std::vector<const CTessFace*> &leaves,
						float cellSize,
						std::vector<CInstanceGroup *> igs,
						const TShapeCache &shapeCache,
						const NLMISC::CAABBox &baseZoneBBox,
						sint32	zoneX,
						sint32	zoneY
					   )
{
	nlassert(cellSize > 0);
	Verts.clear();
	Tris.clear();
	TriLists.clear();
	Grid.clear();
	if (leaves.empty()) return;
	for(uint k = 0; k < leaves.size(); ++k)
	{
		if (k == 0)
		{
			Box.setMinMax(leaves[k]->VBase->EndPos, leaves[k]->VBase->EndPos);
			Box.extend(leaves[k]->VLeft->EndPos);
			Box.extend(leaves[k]->VRight->EndPos);
		}
		else
		{
			Box.extend(leaves[k]->VBase->EndPos);
			Box.extend(leaves[k]->VLeft->EndPos);
			Box.extend(leaves[k]->VRight->EndPos);
		}
	}
	CAABBox landBBox = Box;
	landBBox.extend(baseZoneBBox.getMin());
	landBBox.extend(baseZoneBBox.getMax());	
	// list of instances that contribute to that zone
	std::vector<CZoneInstance> instances;
	// extends with instances that intersect the land bbox with respect to x/y
	for(uint k = 0; k < igs.size(); ++k)
	{
		if (!igs[k]) continue;
		for(uint l = 0; l < igs[k]->getNumInstance(); ++l)
		{
			CMatrix instanceMatrix;
			igs[k]->getInstanceMatrix(l, instanceMatrix);
			if (NLMISC::strlwr(NLMISC::CFile::getExtension(igs[k]->getShapeName(l))) == "pacs_prim") continue;
			std::string stdShapeName = standardizeShapeName(igs[k]->getShapeName(l));
			TShapeCache::const_iterator &it = shapeCache.find(stdShapeName);
			if (it != shapeCache.end())
			{
				CAABBox xformBBox = CAABBox::transformAABBox(instanceMatrix, it->second.LocalBBox);
				if (xformBBox.getMin().x < landBBox.getMax().x &&
					xformBBox.getMin().y < landBBox.getMax().y &&
					xformBBox.getMax().y >= landBBox.getMin().y &&
					xformBBox.getMax().x >= landBBox.getMin().x)
				{
					Box.extend(xformBBox.getMin());
					Box.extend(xformBBox.getMax());
					CZoneInstance zi;
					zi.SI = &(it->second);
					zi.WorldMat = instanceMatrix;
					instances.push_back(zi);
				}
			}
		}
	}
	//
	/*float delta = 5.f;
	Box.extend(Box.getMin() + CVector(- delta, - delta, - delta));
	Box.extend(Box.getMax() + CVector(delta, delta, delta));*/
	//
	CVector cornerMin = Box.getMin();
	CVector cornerMax = Box.getMax();
	//
	uint width = (uint) ceilf((cornerMax.x - cornerMin.x) / cellSize);
	uint height = (uint) ceilf((cornerMax.y - cornerMin.y) / cellSize);
	float depth = cornerMax.z - cornerMin.z;
	if (width == 0 || height == 0) return;
	Grid.init(width, height, -1);
	// 
	TVertexGrid   vertexGrid; // grid for fast sharing of vertices 
	TTriListGrid  triListGrid; // grid for list of tris per grid cell (before packing in a single vector)
	vertexGrid.init(width, height);
	triListGrid.init(width, height);
	CellSize = cellSize;
	//
	_Origin = cornerMin;
	_WorldToLocal.x = 1.f / (width * cellSize);
	_WorldToLocal.y = 1.f / (height * cellSize);
	_WorldToLocal.z = depth != 0  ? 1.f / depth : 0.f;
	//
	for(uint k = 0; k < leaves.size(); ++k)
	{
		CTriangle tri;
		tri.V0 = leaves[k]->VBase->EndPos;
		tri.V1 = leaves[k]->VLeft->EndPos;
		tri.V2 = leaves[k]->VRight->EndPos;
		addTri(tri, vertexGrid, triListGrid);		
	}	
	// add each ig	
	for(uint k = 0; k < instances.size(); ++k)
	{					
		addInstance(*instances[k].SI, instances[k].WorldMat, vertexGrid, triListGrid);
	}	
	// pack tri lists
	for (uint y = 0; y < height; ++y)
	{
		for (uint x = 0; x < width; ++x)
		{
			if (!triListGrid(x, y).empty())
			{
				Grid(x, y) = TriLists.size();
				std::copy(triListGrid(x, y).begin(), triListGrid(x, y).end(), std::back_inserter(TriLists));
				TriLists.push_back(~0); // mark the end of the list
			}
		}
	}
	//
	ZoneX = zoneX;
	ZoneX = zoneY;
	//
	int vertsSize = sizeof(CPackedVertex) * Verts.size();
	int trisSize = sizeof(CPackedTri) * Tris.size();
	int triListSize = sizeof(uint32) * TriLists.size();
	int gridSize = sizeof(uint32) * Grid.getWidth() * Grid.getHeight();
	/*printf("Verts Size = %d\n", vertsSize);
	printf("Tris Size = %d\n", trisSize);
	printf("Tri List Size = %d\n", triListSize);
	printf("Grid size = %d\n", gridSize);
	printf("Total = %d\n", vertsSize + trisSize + triListSize + gridSize);*/
}

//***************************************************************************************
void CPackedZone32::addTri(const CTriangle &tri, TVertexGrid &vertexGrid, TTriListGrid &triListGrid)
{
	CPackedTri pt;
	pt.V0 = allocVertex(tri.V0, vertexGrid);
	pt.V1 = allocVertex(tri.V1, vertexGrid);
	pt.V2 = allocVertex(tri.V2, vertexGrid);	
	// tmp
	/*
	_PackedLocalToWorld.set(1.f / (_WorldToLocal.x * 65535.f),
		                    1.f / (_WorldToLocal.y * 65535.f),
							1.f / (_WorldToLocal.z * 65535.f));	
	CTriangle tmpTri;
	unpackTri(pt, &tmpTri.V0);
	const float DELTA = 2.f;
	float delta0 = (tmpTri.V0 - tri.V0).norm();
	float delta1 = (tmpTri.V1 - tri.V1).norm();
	float delta2 = (tmpTri.V2 - tri.V2).norm();

	nlassert(delta0 < DELTA);
	nlassert(delta1 < DELTA);
	nlassert(delta2 < DELTA);
	*/

	//
	static CPolygon2D::TRasterVect  rasters;
	static CPolygon2D				polyTri;	
	sint							minY;
	polyTri.Vertices.resize(3);
	//	
	polyTri.Vertices[0].set((tri.V0.x - _Origin.x) / CellSize, (tri.V0.y - _Origin.y) / CellSize);
	polyTri.Vertices[1].set((tri.V1.x - _Origin.x) / CellSize, (tri.V1.y - _Origin.y) / CellSize);
	polyTri.Vertices[2].set((tri.V2.x - _Origin.x) / CellSize, (tri.V2.y - _Origin.y) / CellSize);	
	//
	polyTri.computeOuterBorders(rasters, minY);
	if (rasters.empty()) return;
	Tris.push_back(pt);
	//		
	for (sint y = 0; y < (sint) rasters.size(); ++y)
	{
		sint gridY = y + minY;
		if (gridY < 0) continue;
		if (gridY >= (sint) triListGrid.getHeight()) break;
		for (sint x = rasters[y].first; x <= rasters[y].second; ++x)
		{
			if (x < 0) continue;
			if (x >= (sint) triListGrid.getWidth()) break;
			triListGrid(x, gridY).push_back(Tris.size() - 1);
		}
	}
}



//***************************************************************************************
uint32 CPackedZone32::allocVertex(const CVector &src, TVertexGrid &vertexGrid)
{
	CVector local((src.x - _Origin.x) * _WorldToLocal.x,
		          (src.y - _Origin.y) * _WorldToLocal.y,
				  (src.z - _Origin.z) * _WorldToLocal.z);
	sint x = (sint) (local.x * vertexGrid.getWidth());
	sint y = (sint) (local.y * vertexGrid.getHeight());
	if (x == (sint) vertexGrid.getWidth()) x = (sint) vertexGrid.getWidth() - 1;
	if (y == (sint) vertexGrid.getHeight()) y = (sint) vertexGrid.getHeight() - 1;
	//
	CPackedVertex pv;
	sint32 ix = (sint32) (local.x * 65535);
	sint32 iy = (sint32) (local.y * 65535);
	sint32 iz = (sint32) (local.z * 65535);
	clamp(ix, 0, 65535);
	clamp(iy, 0, 65535);
	clamp(iz, 0, 65535);
	pv.X = (uint16) ix;
	pv.Y = (uint16) iy;
	pv.Z = (uint16) iz;
	//
	std::list<uint32> &vertList = vertexGrid(x, y);
	for(std::list<uint32>::iterator it = vertexGrid(x, y).begin(); it != vertexGrid(x, y).end(); ++it)
	{
		if (Verts[*it] == pv) return *it; // exists already
	}
	

	// create a new vertex
	Verts.push_back(pv);
	vertList.push_front(Verts.size() - 1);
	return Verts.size() - 1;
}

//***************************************************************************************
void CPackedZone32::render(CVertexBuffer &vb, IDriver &drv, CMaterial &material, CMaterial &wiredMaterial, const CMatrix &camMat, uint batchSize, const CVector localFrustCorners[8])
{
	_PackedLocalToWorld.set(1.f / (_WorldToLocal.x * 65535.f),
		                    1.f / (_WorldToLocal.y * 65535.f),
							1.f / (_WorldToLocal.z * 65535.f));	
	CVector frustCorners[8];
	for(uint k = 0; k < sizeofarray(frustCorners); ++k)
	{
		frustCorners[k] = camMat * localFrustCorners[k];
		frustCorners[k].x -= _Origin.x;
		frustCorners[k].y -= _Origin.y;		
	}
	// project frustum on x/y plane to see where to test polys		
	sint minY = INT_MAX;
	static CPolygon2D::TRasterVect silhouette;
	silhouette.clear();
	addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[2], frustCorners[3], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[1], frustCorners[5], frustCorners[6], frustCorners[2], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[4], frustCorners[5], frustCorners[6], frustCorners[7], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[0], frustCorners[4], frustCorners[7], frustCorners[3], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[5], frustCorners[4], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[3], frustCorners[7], frustCorners[6], frustCorners[2], silhouette, minY, CellSize);
	//	
	drv.setPolygonMode(IDriver::Line);		
	//		
	{		
		CVertexBufferReadWrite vba;	
		vb.setNumVertices(batchSize * 3);
		vb.lock(vba);
		CVector *dest = vba.getVertexCoordPointer(0);
		const CVector *endDest = dest + batchSize * 3;
		for(sint y = 0; y < (sint) silhouette.size(); ++y)
		{
			sint gridY = y + minY;
			if (gridY < 0) continue;
			if (gridY >= (sint) Grid.getHeight()) continue;
			sint minX = silhouette[y].first;
			sint maxX = silhouette[y].second;
			for (sint x = minX; x <= maxX; ++x)			
			{
				if (x < 0) continue;
				if (x >= (sint) Grid.getWidth()) break;
				uint32 triRefIndex = Grid(x, gridY);
				if (triRefIndex == (uint32) ~0) continue;
				for (;;)
				{
					uint32 triIndex = TriLists[triRefIndex];
					if (triIndex == ~0) break; // end of list					
					unpackTri(Tris[triIndex], dest);
					dest += 3;
					if (dest == endDest)
					{
						// flush batch
						vba.unlock();						
						drv.setPolygonMode(IDriver::Filled);
						drv.activeVertexBuffer(vb);
						drv.renderRawTriangles(material, 0, batchSize);						
						drv.setPolygonMode(IDriver::Line);
						//drv.renderRawTriangles(wiredMaterial, 0, batchSize);
						// reclaim a new batch
						vb.setNumVertices(batchSize * 3);
						vb.lock(vba);
						dest = vba.getVertexCoordPointer(0);
						endDest = dest + batchSize * 3;
					}
					++ triRefIndex;
				}				
			}
		}
		vba.unlock();
		uint numRemainingTris = batchSize - ((endDest - dest) / 3);
		if (numRemainingTris)
		{			
			drv.setPolygonMode(IDriver::Filled);
			drv.activeVertexBuffer(vb);
			drv.renderRawTriangles(material, 0, numRemainingTris);		
			drv.setPolygonMode(IDriver::Line);
			//drv.renderRawTriangles(wiredMaterial, 0, numRemainingTris);
		}
	}
}

//***************************************************************************************
void CPackedZone32::addInstance(const CShapeInfo &si, const NLMISC::CMatrix &matrix, TVertexGrid &vertexGrid, TTriListGrid &triListGrid)
{	
	for(uint k = 0; k < si.Tris.size(); ++k)
	{
		CTriangle worldTri;
		si.Tris[k].applyMatrix(matrix, worldTri);
		addTri(worldTri, vertexGrid, triListGrid);
	}	
}

//***************************************************************************************
CSmartPtr<CPackedZone16> CPackedZone32::buildPackedZone16()
{
	if (Verts.size() > 65535) return NULL;
	if (Tris.size() > 65534) return NULL; // NB : not 65535 here because -1 is used to mark the end of a list
	if (TriLists.size() > 65534) return NULL;
	// can convert
	CSmartPtr<CPackedZone16> dest = new CPackedZone16;
	dest->Box = Box;
	dest->Verts = Verts;
	dest->Tris.resize(Tris.size());
	for(uint k = 0; k < Tris.size(); ++k)
	{
		dest->Tris[k].V0 = (uint16) Tris[k].V0;
		dest->Tris[k].V1 = (uint16) Tris[k].V1;
		dest->Tris[k].V2 = (uint16) Tris[k].V2;
	}
	dest->TriLists.resize(TriLists.size());
	for(uint k = 0; k < (sint) TriLists.size(); ++k)
	{
		dest->TriLists[k] = (uint16) TriLists[k];
	}
	dest->CellSize = CellSize;
	dest->Grid.init(Grid.getWidth(), Grid.getHeight());
	for (sint y = 0; y < (sint) Grid.getHeight(); ++y)
	{
		for (sint x = 0; x < (sint) Grid.getWidth(); ++x)
		{
			dest->Grid(x, y) = (uint16) Grid(x, y);
		}
	}
	dest->_Origin = _Origin;
	dest->_WorldToLocal = _WorldToLocal;	
	dest->ZoneX = ZoneX;
	dest->ZoneY = ZoneY;	
	return dest;
}

//***************************************************************************************
void CPackedZone16::serial(NLMISC::IStream &f) throw (NLMISC::EStream)
{
	f.serialVersion(0);
	f.serial(Box);
	f.serialCont(Verts);
	f.serialCont(Tris);
	f.serialCont(TriLists);
	f.serial(Grid);
	f.serial(CellSize);
	f.serial(_Origin);
	f.serial(_WorldToLocal);
	f.serial(ZoneX);
	f.serial(ZoneY);
}

//***************************************************************************************
void CPackedZone16::render(CVertexBuffer &vb, IDriver &drv, CMaterial &material, CMaterial &wiredMaterial, const CMatrix &camMat, uint batchSize, const CVector localFrustCorners[8])
{
	// yes this is ugly code duplication of CPackedZone32::render but this code is temporary anyway...
	_PackedLocalToWorld.set(1.f / (_WorldToLocal.x * 65535.f),
		                    1.f / (_WorldToLocal.y * 65535.f),
							1.f / (_WorldToLocal.z * 65535.f));	
	CVector frustCorners[8];
	for(uint k = 0; k < sizeofarray(frustCorners); ++k)
	{
		frustCorners[k] = camMat * localFrustCorners[k];
		frustCorners[k].x -= _Origin.x;
		frustCorners[k].y -= _Origin.y;		
	}
	// project frustum on x/y plane to see where to test polys		
	sint minY = INT_MAX;
	static CPolygon2D::TRasterVect silhouette;
	silhouette.clear();
	addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[2], frustCorners[3], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[1], frustCorners[5], frustCorners[6], frustCorners[2], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[4], frustCorners[5], frustCorners[6], frustCorners[7], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[0], frustCorners[4], frustCorners[7], frustCorners[3], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[0], frustCorners[1], frustCorners[5], frustCorners[4], silhouette, minY, CellSize);
	addQuadToSilhouette(frustCorners[3], frustCorners[7], frustCorners[6], frustCorners[2], silhouette, minY, CellSize);
	//	
	drv.setPolygonMode(IDriver::Line);	
	//		
	{
		CVertexBufferReadWrite vba;		
		vb.setNumVertices(batchSize * 3);
		vb.lock(vba);
		CVector *dest = vba.getVertexCoordPointer(0);
		const CVector *endDest = dest + batchSize * 3;
		for(sint y = 0; y < (sint) silhouette.size(); ++y)
		{
			sint gridY = y + minY;
			if (gridY < 0) continue;
			if (gridY >= (sint) Grid.getHeight()) continue;
			sint minX = silhouette[y].first;
			sint maxX = silhouette[y].second;
			for (sint x = minX; x <= maxX; ++x)			
			{
				if (x < 0) continue;
				if (x >= (sint) Grid.getWidth()) break;
				uint32 triRefIndex = Grid(x, gridY);
				if (triRefIndex == (uint16) ~0) continue;
				for (;;)
				{
					uint16 triIndex = TriLists[triRefIndex];
					if (triIndex == (uint16) ~0) break; // end of list
					unpackTri(Tris[triIndex], dest);
					dest += 3;
					if (dest == endDest)
					{						
						// flush batch
						vba.unlock();						
						drv.setPolygonMode(IDriver::Filled);
						drv.activeVertexBuffer(vb);
						drv.renderRawTriangles(material, 0, batchSize);						
						drv.setPolygonMode(IDriver::Line);
						//drv.renderRawTriangles(wiredMaterial, 0, batchSize);
						// reclaim a new batch
						vb.setNumVertices(batchSize * 3);
						vb.lock(vba);
						dest = vba.getVertexCoordPointer(0);
						endDest = dest + batchSize * 3;
					}
					++ triRefIndex;
				}				
			}
		}
		vba.unlock();		
		uint numRemainingTris = batchSize - ((endDest - dest) / 3);
		if (numRemainingTris)
		{			
			drv.setPolygonMode(IDriver::Filled);
			drv.activeVertexBuffer(vb);
			drv.renderRawTriangles(material, 0, numRemainingTris);			
			drv.setPolygonMode(IDriver::Line);
			//drv.renderRawTriangles(wiredMaterial, 0, numRemainingTris);
		}		
	}
}





//***************************************************************************************
void CPackedZone16::unpackTri(const CPackedTri16 &src, CVector dest[3]) const
{	
	// yes this is ugly code duplication of CPackedZone16::unpackTri but this code is temporary anyway...
	// TODO: add 'multiply-add' operator
	dest[0].set(Verts[src.V0].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V0].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V0].Z * _PackedLocalToWorld.z + _Origin.z);
	dest[1].set(Verts[src.V1].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V1].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V1].Z * _PackedLocalToWorld.z + _Origin.z);
	dest[2].set(Verts[src.V2].X * _PackedLocalToWorld.x + _Origin.x,
		        Verts[src.V2].Y * _PackedLocalToWorld.y + _Origin.y,
				Verts[src.V2].Z * _PackedLocalToWorld.z + _Origin.z);

}

// raytrace code, common to CPackedZone32 & CPackedZone16
template <class T> bool raytrace(T &packedZone, const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<CTriangle> *testedTriangles = NULL)
{	
	if (packedZone.Grid.empty()) return false;
	CVector2f start2f((start.x - packedZone.Box.getMin().x) / packedZone.CellSize, (start.y - packedZone.Box.getMin().y) / packedZone.CellSize);	
	CVector2f dir2f((end.x - start.x) / packedZone.CellSize, (end.y - start.y) / packedZone.CellSize);
	sint x, y;
	CGridTraversal::startTraverse(start2f, x, y);
	do
	{		
		if (x < 0) continue;
		if (x >= (sint) packedZone.Grid.getWidth()) continue;
		if (y < 0) continue;
		if (y >= (sint) packedZone.Grid.getHeight()) continue;		
		uint32 triListIndex = packedZone.Grid(x, y);
		if (triListIndex != (T::TIndexType) ~0)
		{
			CTriangle tri;
			CPlane triPlane;			
			float bestInterDist = FLT_MAX;
			CVector currInter;
			do
			{			
				packedZone.unpackTri(packedZone.Tris[packedZone.TriLists[triListIndex]], &tri.V0);
				if (testedTriangles)
				{
					testedTriangles->push_back(tri);
				}
				triPlane.make(tri.V0, tri.V1, tri.V2);
				if (tri.intersect(start, end, currInter, triPlane))
				{
					float dist = (currInter - start).norm();
					if (dist < bestInterDist)
					{
						bestInterDist = dist;
						inter = currInter;						
					}
				}
				++ triListIndex;
			}
			while (packedZone.TriLists[triListIndex] != (T::TIndexType) ~0);
			if (bestInterDist != FLT_MAX) return true;
		}
	}
	while(CGridTraversal::traverse(start2f, dir2f, x, y));
	return false;
}

//***************************************************************************************
bool CPackedZone32::raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<CTriangle> *testedTriangles /*= NULL*/) const
{
	return NL3D::raytrace(*this, start, end, inter, testedTriangles);	
}

//***************************************************************************************
bool CPackedZone16::raytrace(const NLMISC::CVector &start, const NLMISC::CVector &end, NLMISC::CVector &inter, std::vector<CTriangle> *testedTriangles /*= NULL*/) const
{	
	return NL3D::raytrace(*this, start, end, inter, testedTriangles);
}

} // NL3D



