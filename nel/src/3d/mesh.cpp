/** \file mesh.cpp
 * <File description>
 *
 * $Id: mesh.cpp,v 1.11 2001/04/09 14:26:51 berenguier Exp $
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
#include "nel/3d/mesh_instance.h"
#include "nel/3d/scene.h"


using namespace std;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// Tools.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static	NLMISC::CAABBoxExt	makeBBox(const std::vector<CVector>	&Vertices)
{
	NLMISC::CAABBox		ret;
	nlassert(Vertices.size());
	ret.setCenter(Vertices[0]);
	for(sint i=0;i<(sint)Vertices.size();i++)
	{
		ret.extend(Vertices[i]);
	}

	return ret;
}


// ***************************************************************************
CMesh::CCorner::CCorner()
{
	sint	i;
	Vertex= 0;
	Normal= CVector::Null;
	for(i=0;i<IDRV_VF_MAXSTAGES;i++)
		Uvs[i]= CUV(0,0);
	Color.set(255,255,255,255);
	Specular.set(0,0,0,0);
}


// ***************************************************************************
sint	CMesh::CCornerTmp::Flags=0;


// ***************************************************************************
bool	CMesh::CCornerTmp::operator<(const CCornerTmp &c) const
{
	sint	i;

	// Vert first.
	if(Vertex!=c.Vertex)
		return Vertex<c.Vertex;

	// Order: normal, uvs, color0, color1, skinning.
	if((CCornerTmp::Flags & IDRV_VF_NORMAL) && Normal!=c.Normal)
		return Normal<c.Normal;
	for(i=0;i<IDRV_VF_MAXSTAGES;i++)
	{
		if(CCornerTmp::Flags & IDRV_VF_UV[i] && Uvs[i]!=c.Uvs[i])
			return Uvs[i]<c.Uvs[i];
	}
	if((CCornerTmp::Flags & IDRV_VF_COLOR) && Color!=c.Color)
		return Color<c.Color;
	if((CCornerTmp::Flags & IDRV_VF_SPECULAR) && Specular!=c.Specular)
		return Specular<c.Specular;

	if( (CCornerTmp::Flags & IDRV_VF_PALETTE_SKIN) == IDRV_VF_PALETTE_SKIN)
	{
		for(i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
		{
			if(Palette.MatrixId[i] != c.Palette.MatrixId[i])
				return Palette.MatrixId[i] < c.Palette.MatrixId[i];
			if(Weights[i] != c.Weights[i])
				return Weights[i] < c.Weights[i];
		}
	}


	// All are equal!!
	return false;
}




// ***************************************************************************
// ***************************************************************************
// CMesh.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMesh::CMesh()
{
	_Skinned= false;
}


// ***************************************************************************
void	CMesh::build(const CMeshBuild &m)
{
	sint	i;

	// clear the animated materials.
	_AnimatedMaterials.clear();

	// Empty?
	if(m.Vertices.size()==0 || m.Faces.size()==0)
	{
		_VBuffer.setNumVertices(0);
		_VBuffer.reserve(0);
		_MatrixBlocks.clear();
		_BBox.setCenter(CVector::Null);
		_BBox.setSize(CVector::Null);
		return;
	}
	nlassert(m.Materials.size()>0);


	/// 0. First, make bbox.
	//======================
	_BBox= makeBBox(m.Vertices);


	/// 1. If skinning, group by matrix Block the vertices.
	//================================================

	// First, copy Face array.
	vector<CFaceTmp>	tmpFaces;
	tmpFaces.resize(m.Faces.size());
	for(i=0;i<(sint)tmpFaces.size();i++)
		tmpFaces[i]= m.Faces[i];

	_Skinned= (m.VertexFlags & IDRV_VF_PALETTE_SKIN)==IDRV_VF_PALETTE_SKIN;


	// If the mesh is not skinned, we have just 1 _MatrixBlocks.
	if(!_Skinned)
	{
		_MatrixBlocks.resize(1);
		// For each faces, assign it to the matrix block 0.
		for(i=0;i<(sint)tmpFaces.size();i++)
			tmpFaces[i].MatrixBlockId= 0;
	}
	// Else We must group/compute the matrixs blocks.
	else
	{
		// TODODO.
	}


	/// 2. Then, for all faces, resolve continuities, building VBuffer.
	//================================================
	// Setup VB.
	_VBuffer.setNumVertices(0);
	_VBuffer.reserve(0);
	_VBuffer.setVertexFormat(m.VertexFlags | IDRV_VF_XYZ);

	// Set local flags for corner comparison.
	CCornerTmp::Flags= m.VertexFlags;
	// Setup locals.
	TCornerSet	corners;
	const CFaceTmp		*pFace= &(*tmpFaces.begin());
	sint		N= tmpFaces.size();
	sint		currentVBIndex=0;

	// process each face, building up the VB.
	for(;N>0;N--, pFace++)
	{
		ItCornerSet	it;
		sint	v0= pFace->Corner[0].Vertex;
		sint	v1= pFace->Corner[1].Vertex;
		sint	v2= pFace->Corner[2].Vertex;
		findVBId(corners, &pFace->Corner[0], currentVBIndex, m.Vertices[v0]);
		findVBId(corners, &pFace->Corner[1], currentVBIndex, m.Vertices[v1]);
		findVBId(corners, &pFace->Corner[2], currentVBIndex, m.Vertices[v2]);
	}


	/// 3. build the RdrPass material.
	//================================
	uint	mb;
	// copy the materials.
	_Materials= m.Materials;

	// For each _MatrixBlocks, point on those materials.
	for(mb= 0;mb<_MatrixBlocks.size();mb++)
	{
		// Build RdrPass ids.
		_MatrixBlocks[mb].RdrPass.resize(m.Materials.size());
		// TODO: it should be interesting to sort the materials, depending of their attributes.
		for(i=0;i<(sint)_MatrixBlocks[mb].RdrPass.size(); i++)
		{
			_MatrixBlocks[mb].RdrPass[i].MaterialId= i;
		}
	}

	
	/// 4. Then, for all faces, build the RdrPass PBlock.
	//===================================================
	pFace= &(*tmpFaces.begin());
	N= tmpFaces.size();
	for(;N>0;N--, pFace++)
	{
		sint	mbId= pFace->MatrixBlockId;
		nlassert(mbId>=0 && mbId<_MatrixBlocks.size());
		// Insert the face in good MatrixBlock/RdrPass.
		_MatrixBlocks[mbId].RdrPass[pFace->MaterialId].PBlock.addTri(pFace->Corner[0].VBId, pFace->Corner[1].VBId, pFace->Corner[2].VBId);
	}

	/// 5. Copy default position values
	//===================================================
	_DefaultPos.setValue (m.DefaultPos);
	_DefaultPivot.setValue (m.DefaultPivot);
	_DefaultRotEuler.setValue (m.DefaultRotEuler);
	_DefaultRotQuat.setValue (m.DefaultRotQuat);
	_DefaultScale.setValue (m.DefaultScale);

	// End!!
}



// ***************************************************************************
CTransformShape		*CMesh::createInstance(CScene &scene)
{
	// Create a CMeshInstance, an instance of a mesh.
	//===============================================
	CMeshInstance		*mi= (CMeshInstance*)scene.createModel(NL3D::MeshInstanceId);
	mi->Shape= this;

	// setup materials.
	//=================
	mi->Materials= _Materials;

	// setup animated materials.
	//==========================
	TAnimatedMaterialMap::iterator	it;
	mi->_AnimatedMaterials.reserve(_AnimatedMaterials.size());
	for(it= _AnimatedMaterials.begin(); it!= _AnimatedMaterials.end(); it++)
	{
		CAnimatedMaterial	aniMat(&it->second);

		// set the target instance material.
		nlassert(it->first < mi->Materials.size());
		aniMat.setMaterial(&mi->Materials[it->first]);

		// Must set the Animatable father of the animated material (the mesh_instance!).
		aniMat.setFather(mi, CMeshInstance::OwnerBit);

		// Append this animated material.
		mi->_AnimatedMaterials.push_back(aniMat);
	}
	

	return mi;
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
void	CMesh::render(IDriver *drv, CTransformShape *trans)
{
	// TODODO. skinning and good renderpass.

	if(_MatrixBlocks.size()==0)
		return;
	if(_MatrixBlocks[0].RdrPass.size()==0)
		return;

	nlassert(drv);
	drv->activeVertexBuffer(_VBuffer);

	// get the mesh instance.
	nlassert(dynamic_cast<CMeshInstance*>(trans));
	CMeshInstance	*mi= (CMeshInstance*)trans;

	// Render all pass.
	for(sint i=0;i<(sint)_MatrixBlocks[0].RdrPass.size();i++)
	{
		// Render with the Mateirals of the MeshInstance.
		drv->render(_MatrixBlocks[0].RdrPass[i].PBlock, mi->Materials[i]);
	}
}


// ***************************************************************************
void	CMesh::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 3:
		- skinning.
	Version 2:
		- Default track for position.
	Version 1:
		- Animated Materials.
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(3);

	f.serial(_VBuffer);

	// New Architecture for V3+ meshs.
	if(ver>=3)
	{
		f.serialCont(_Materials);
		f.serialCont(_MatrixBlocks);
	}
	else
	{
		// Old versions: must read RdrPassV0 struct, then fill _Materials and RdrPass.
		vector<CRdrPassV2>	oldRdrPass;
		f.serialCont(oldRdrPass);

		// copy to MatrixBlock 0.
		_MatrixBlocks.resize(1);
		_Materials.resize(oldRdrPass.size());
		_MatrixBlocks[0].RdrPass.resize(oldRdrPass.size());
		// fill V3+ rdrPass.
		for(uint i=0;i<oldRdrPass.size(); i++)
		{
			_Materials[i]= oldRdrPass[i].Material;
			_MatrixBlocks[0].RdrPass[i].PBlock= oldRdrPass[i].PBlock;
			_MatrixBlocks[0].RdrPass[i].MaterialId= i;
		}

		// V2- => We are not skinned.
		_Skinned= false;
	}

	f.serial(_BBox);

	// _AnimatedMaterials
	if(ver>=1)
		f.serialMap(_AnimatedMaterials);
	else
	{
		if(f.isReading())
			_AnimatedMaterials.clear();
	}

	// New features
	switch (ver)
	{
	case 3:
		f.serial (_Skinned);
	case 2:
		f.serial (_DefaultPos);
		f.serial (_DefaultPivot);
		f.serial (_DefaultRotEuler);
		f.serial (_DefaultRotQuat);
		f.serial (_DefaultScale);
	}
}


// ***************************************************************************
// ***************************************************************************
// Animated material.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMesh::setAnimatedMaterial(uint id, const std::string &matName)
{
	if(id<_Materials.size())
	{
		// add / replace animated material.
		_AnimatedMaterials[id].Name= matName;
		// copy Material default.
		_AnimatedMaterials[id].copyFromMaterial(&_Materials[id]);
	}
}

// ***************************************************************************
CMaterialBase	*CMesh::getAnimatedMaterial(uint id)
{
	TAnimatedMaterialMap::iterator	it;
	it= _AnimatedMaterials.find(id);
	if(it!=_AnimatedMaterials.end())
		return &it->second;
	else
		return NULL;
}


} // NL3D
