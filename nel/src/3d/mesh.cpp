/** \file mesh.cpp
 * <File description>
 *
 * $Id: mesh.cpp,v 1.4 2000/12/18 09:45:13 corvazier Exp $
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

#include "nel/3d/mesh.h"


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// Tools.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static	CAABBoxExt	makeBBox(const std::vector<CVector>	&Vertices)
{
	CAABBox		ret;
	nlassert(Vertices.size());
	ret.setCenter(Vertices[0]);
	for(sint i=0;i<(sint)Vertices.size();i++)
	{
		ret.extend(Vertices[i]);
	}

	return ret;
}

// ***************************************************************************
sint	CMesh::CCorner::Flags=0;


// ***************************************************************************
bool	CMesh::CCorner::operator<(const CCorner &c) const
{
	sint	i;

	// Vert first.
	if(Vertex!=c.Vertex)
		return Vertex<c.Vertex;

	// Order: normal, uvs, color0, color1, then weights
	if((CCorner::Flags & IDRV_VF_NORMAL) && Normal!=c.Normal)
		return Normal<c.Normal;
	for(i=0;i<IDRV_VF_MAXSTAGES;i++)
	{
		if(CCorner::Flags & IDRV_VF_UV[i] && Uvs[i]!=c.Uvs[i])
			return Uvs[i]<c.Uvs[i];
	}
	if((CCorner::Flags & IDRV_VF_COLOR) && Color!=c.Color)
		return Color<c.Color;
	if((CCorner::Flags & IDRV_VF_SPECULAR) && Specular!=c.Specular)
		return Specular<c.Specular;
	for(i=0;i<IDRV_VF_MAXW;i++)
	{
		if(CCorner::Flags & IDRV_VF_W[i] && Weights[i]!=c.Weights[i])
			return Weights[i]<c.Weights[i];
	}

	// All are equal!!
	return false;
}

// ***************************************************************************
CMesh::CCorner::CCorner()
{
	sint	i;
	VBId= 0;
	Vertex= 0;
	Normal= CVector::Null;
	for(i=0;i<IDRV_VF_MAXSTAGES;i++)
		Uvs[i]= CUV(0,0);
	Color.set(255,255,255,255);
	Specular.set(0,0,0,0);
	for(i=0;i<IDRV_VF_MAXW;i++)
		Weights[i]= 0;
}





// ***************************************************************************
// ***************************************************************************
// CMesh.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMesh::CMesh()
{
}


// ***************************************************************************
void	CMesh::build(const CMeshBuild &m)
{
	// Empty?
	if(m.Vertices.size()==0 || m.Faces.size()==0)
	{
		_VBuffer.setNumVertices(0);
		_VBuffer.reserve(0);
		_RdrPass.clear();
		_BBox.setCenter(CVector::Null);
		_BBox.setSize(CVector::Null);
		return;
	}
	nlassert(m.Materials.size()>0);


	/// 0. First, make bbox.
	//======================
	_BBox= makeBBox(m.Vertices);


	/// 1. Then, for all faces, resolve continuities, building VBuffer.
	//================================================
	// Setup VB.
	_VBuffer.setNumVertices(0);
	_VBuffer.reserve(0);
	_VBuffer.setVertexFormat(m.VertexFlags | IDRV_VF_XYZ);

	// Set local flags for corner comparison.
	CCorner::Flags= m.VertexFlags;
	// Setup locals.
	TCornerSet	corners;
	const CFace		*pFace= &(*m.Faces.begin());
	sint		N= m.Faces.size();
	sint		currentVBIndex=0;
	for(;N>0;N--, pFace++)
	{
		ItCornerSet	it;
		findVBId(corners, &pFace->Corner[0], currentVBIndex, m.Vertices[pFace->Corner[0].Vertex]);
		findVBId(corners, &pFace->Corner[1], currentVBIndex, m.Vertices[pFace->Corner[1].Vertex]);
		findVBId(corners, &pFace->Corner[2], currentVBIndex, m.Vertices[pFace->Corner[2].Vertex]);
	}


	/// 2. build the RdrPass material.
	//================================
	sint	i;
	_RdrPass.resize(m.Materials.size());
	// TODO: it should be interesting to sort the materials, depending of their attributes.
	for(i=0;i<(sint)_RdrPass.size(); i++)
	{
		_RdrPass[i].Material= m.Materials[i];
	}

	
	/// 3. Then, for all faces, build the RdrPass PBlock.
	//===================================================
	pFace= &(*m.Faces.begin());
	N= m.Faces.size();
	for(;N>0;N--, pFace++)
	{
		_RdrPass[pFace->MaterialId].PBlock.addTri(pFace->Corner[0].VBId, pFace->Corner[1].VBId, pFace->Corner[2].VBId);
	}

	// End!!
}


// ***************************************************************************
bool	CMesh::clip(const std::vector<CPlane>	&pyramid)
{
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		if(!_BBox.clipBack(pyramid[i]))
			return false;
	}

	return true;
}

// ***************************************************************************
void	CMesh::render(IDriver *drv)
{
	if(_RdrPass.size()==0)
		return;

	nlassert(drv);
	drv->activeVertexBuffer(_VBuffer);

	// Render all pass.
	for(sint i=0;i<(sint)_RdrPass.size();i++)
	{
		drv->render(_RdrPass[i].PBlock, _RdrPass[i].Material);
	}
}


// ***************************************************************************
void	CMesh::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Serial the shape
	IShape::serial (f);

	// Serial the mesh
	sint	ver= f.serialVersion(0);

	f.serial(_VBuffer);
	f.serialCont(_RdrPass);
	f.serial(_BBox);
}


} // NL3D
