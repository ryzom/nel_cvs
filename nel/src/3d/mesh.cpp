/** \file mesh.cpp
 * <File description>
 *
 * $Id: mesh.cpp,v 1.13 2001/04/12 13:57:41 berenguier Exp $
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
#include "nel/3d/skeleton_model.h"


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
void	CMesh::build(CMeshBuild &m)
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
	// Skinning is OK only if SkinWeights are of same size as vertices.
	_Skinned= _Skinned && (m.Vertices.size()==m.SkinWeights.size());

	// If skinning is KO, remove the Skin option.
	uint	vbFlags= m.VertexFlags;
	if(!_Skinned)
		vbFlags&= ~IDRV_VF_PALETTE_SKIN;
	// Force presence of vertex.
	vbFlags|= IDRV_VF_XYZ;


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
		// reset matrix blocks.
		_MatrixBlocks.clear();
		// build matrix blocks, and link faces to good matrix blocks.
		buildSkin(m, tmpFaces);
	}


	/// 2. Then, for all faces, resolve continuities, building VBuffer.
	//================================================
	// Setup VB.
	_VBuffer.setNumVertices(0);
	_VBuffer.reserve(0);
	_VBuffer.setVertexFormat(vbFlags);

	// Set local flags for corner comparison.
	CCornerTmp::Flags= vbFlags;
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
		// TODO_OPTIMIZE: it should be interesting to sort the materials, depending on their attributes. But must change next loop too...
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
		nlassert(mbId>=0 && mbId<(sint)_MatrixBlocks.size());
		// Insert the face in good MatrixBlock/RdrPass.
		_MatrixBlocks[mbId].RdrPass[pFace->MaterialId].PBlock.addTri(pFace->Corner[0].VBId, pFace->Corner[1].VBId, pFace->Corner[2].VBId);
	}


	/// 5. Remove empty RdrPasses.
	//============================
	for(mb= 0;mb<_MatrixBlocks.size();mb++)
	{
		// NB: slow process (erase from a vector). Doens't matter since made at build.
		vector<CRdrPass>::iterator	itPass;
		for( itPass=_MatrixBlocks[mb].RdrPass.begin(); itPass!=_MatrixBlocks[mb].RdrPass.end(); )
		{
			// If this pass is empty, remove it.
			if( itPass->PBlock.getNumTri()==0 )
				itPass= _MatrixBlocks[mb].RdrPass.erase(itPass);
			else
				itPass++;
		}
	}


	/// 6. Copy default position values
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
	nlassert(drv);
	// get the mesh instance.
	nlassert(dynamic_cast<CMeshInstance*>(trans));
	CMeshInstance	*mi= (CMeshInstance*)trans;

	// get the skeleton model to which I am binded (else NULL).
	CSkeletonModel		*skeleton;
	skeleton= mi->_FatherSkeletonModel;
	// Is this mesh skinned?? true only if mesh is skinned, skeletonmodel is not NULL, and _ApplySkinOk.
	bool	skinOk= _Skinned && mi->_ApplySkinOk && skeleton;


	// enable driver skinning.
	if(skinOk)
	{
		drv->setupVertexMode(NL3D_VERTEX_MODE_SKINNING);
	}


	// For all _MatrixBlocks
	for(uint mb=0;mb<_MatrixBlocks.size();mb++)
	{
		CMatrixBlock	&mBlock= _MatrixBlocks[mb];
		if(mBlock.RdrPass.size()==0)
			continue;

		// If skinning, Setup matrixs (else MeshInstance has computed the worldmatrix for me).
		if(skinOk)
		{
			uint idMat;

			// Get previous block.
			CMatrixBlock	*mPrevBlock=NULL;
			if(mb>0)
				mPrevBlock= &_MatrixBlocks[mb-1];

			// For all matrix of this mBlock.
			for(idMat=0;idMat<mBlock.NumMatrix;idMat++)
			{
				uint	curBoneId= mBlock.MatrixId[idMat];

				// If same matrix binded as previous block, no need to bind!!
				if(mPrevBlock && idMat<mPrevBlock->NumMatrix && mPrevBlock->MatrixId[idMat]== curBoneId)
					continue;

				// Else, we must setup the matrix computed in skeleton to the driver.
				drv->setupModelMatrix(skeleton->Bones[curBoneId].getBoneSkinMatrix(), idMat);
			}
		}


		// active VB. SoftwareSkinning: reset flags for skinning.
		drv->activeVertexBuffer(_VBuffer);


		// Render all pass.
		for(uint i=0;i<mBlock.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= mBlock.RdrPass[i];
			// Render with the Materials of the MeshInstance.
			drv->render(rdrPass.PBlock, mi->Materials[rdrPass.MaterialId]);
		}
	}

	// disable driver skinning.
	if(skinOk)
	{
		drv->setupVertexMode(NL3D_VERTEX_MODE_NORMAL);
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
		vector<CRdrPassOldV2>	oldRdrPass;
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


// ***************************************************************************
// ***************************************************************************
// Skinning.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CMesh::buildSkin(CMeshBuild &m, std::vector<CFaceTmp>	&tmpFaces)
{
	sint	i,j,k;
	TBoneMap		remainingBones;
	list<uint>		remainingFaces;


	// 0. normalize SkinWeights: for all weights at 0, copy the matrixId from 0th matrix => no random/bad use of matrix.
	//================================
	for(i=0;i<(sint)m.SkinWeights.size();i++)
	{
		CSkinWeight	&sw= m.SkinWeights[i];

		// 0th weight must not be 0.
		nlassert(sw.Weights[0]!=0);

		// Begin at 1, tests all other weights.
		for(j=1;j<NL3D_MESH_SKINNING_MAX_MATRIX;j++)
		{
			// We don't use this entry??
			if(sw.Weights[j]==0)
			{
				// Setup MatrixId so that this vertex do no use more matrix than it really wants.
				sw.MatrixId[j]= sw.MatrixId[0];
			}
		}
	}


	// 1. build the list of used/remaining bones, in ascending order. (so we use the depth-first topolgy of hierarchy).
	//================================
	for(i=0;i<(sint)tmpFaces.size();i++)
	{
		CFaceTmp	&face= tmpFaces[i];

		for(j=0;j<3;j++)
		{
			CSkinWeight	&sw= m.SkinWeights[face.Corner[j].Vertex];
			for(k=0;k<NL3D_MESH_SKINNING_MAX_MATRIX;k++)
			{
				// insert (if not already here) the used bone in the set.
				// and insert his refcount. (NB: ctor() init it to 0 :) ).
				remainingBones[sw.MatrixId[k]].RefCount++;
			}
		}
	}


	// 2. Create the list of un-inserted faces.
	//================================
	for(i=0;i<(sint)tmpFaces.size();i++)
	{
		remainingFaces.push_back(i);
	}



	// 3. Create as many Blocks as necessary.
	//================================
	// Which bones a face use (up to 12).
	vector<uint>	boneUse;
	boneUse.reserve(NL3D_MESH_SKINNING_MAX_MATRIX*3);

	// While still exist faces.
	while(!remainingFaces.empty())
	{
		// create a new matrix block.
		_MatrixBlocks.push_back();
		CMatrixBlock	&matrixBlock= _MatrixBlocks[_MatrixBlocks.size()-1];
		matrixBlock.NumMatrix=0;

		// a. reset remainingBones as not inserted in the current matrixBlock.
		//============================
		ItBoneMap	itBone;
		for(itBone= remainingBones.begin();itBone!=remainingBones.end();itBone++)
		{
			itBone->second.Inserted= false;
		}


		// b. while still exist bones, try to insert faces which use them in matrixBlock.
		//============================
		while(!remainingBones.empty())
		{
			// get the first bone from the map. (remind: depth-first order).
			uint		currentBoneId= remainingBones.begin()->first;

			// If no more faces in the remainingFace list use this bone, remove it, and continue.
			if(remainingBones.begin()->second.RefCount==0)
			{
				remainingBones.erase(remainingBones.begin());
				continue;
			}

			// this is a marker, to know if a face insertion will occurs.
			bool		faceAdded= false;

			// traverse all faces, trying to insert them in current MatrixBlock processed.
			list<uint>::iterator	itFace;
			for(itFace= remainingFaces.begin(); itFace!=remainingFaces.end();)
			{
				bool	useCurrentBoneId;
				uint	newBoneAdded;

				// i/ Get info on current face.
				//-----------------------------

				// build which bones this face use.
				tmpFaces[*itFace].buildBoneUse(boneUse, m.SkinWeights);

				// test if this face use the currentBoneId.
				useCurrentBoneId= false;
				for(i=0;i<(sint)boneUse.size();i++)
				{
					// if this face use the currentBoneId
					if(boneUse[i]==currentBoneId)
					{
						useCurrentBoneId= true;
						break;
					}
				}
				// compute how many bones that are not in the current matrixblock this face use.
				newBoneAdded=0;
				for(i=0;i<(sint)boneUse.size();i++)
				{
					// if this bone is not inserted in the current matrix block, inform it.
					if(!remainingBones[boneUse[i]].Inserted)
						newBoneAdded++;
				}
				
				
				// ii/ insert/reject face.
				//------------------------

				// If this face do not add any more bone, we can insert it into the current matrixblock.
				// If it use the currentBoneId, and do not explode max count, we allow insert it too in the current matrixblock.
				if( newBoneAdded==0 || 
					(useCurrentBoneId && newBoneAdded+matrixBlock.NumMatrix < IDriver::MaxModelMatrix) )
				{
					// Insert this face in the current matrix block

					CFaceTmp	&face= tmpFaces[*itFace];

					// for all vertices of this face.
					for(j=0;j<3;j++)
					{
						CSkinWeight	&sw= m.SkinWeights[face.Corner[j].Vertex];

						// for each corner weight (4)
						for(k=0;k<NL3D_MESH_SKINNING_MAX_MATRIX;k++)
						{
							// get the global boneId this corner weight use.
							uint		boneId= sw.MatrixId[k];
							// get the CBoneTmp this corner weight use.
							CBoneTmp	&bone= remainingBones[boneId];

							// decRef the bone .
							bone.RefCount--;

							// Is this bone already inserted in the MatrixBlock ?
							if( !bone.Inserted )
							{
								// No, insert it.
								bone.Inserted= true;
								// link it to the MatrixId in the current matrixBlock.
								bone.MatrixIdInMB= matrixBlock.NumMatrix;

								// modify the matrixBlock
								matrixBlock.MatrixId[matrixBlock.NumMatrix]= boneId;
								// increment the number of matrix in the matrixBlock.
								matrixBlock.NumMatrix++;
							}

							// Copy Weight info for this Corner.
							// Set what matrix in the current matrix block this corner use.
							face.Corner[j].Palette.MatrixId[k]= bone.MatrixIdInMB;
							// Set weight.
							face.Corner[j].Weights[k]= sw.Weights[k];
						}
					}

					// to Which matrixblock this face is inserted.
					face.MatrixBlockId= _MatrixBlocks.size()-1;
					
					// remove the face from remain face list.
					itFace= remainingFaces.erase(itFace);

					// inform the algorithm that a face has been added.
					faceAdded= true;
				}
				else
				{
					// do not append this face to the current matrix block, skip to the next
					itFace++;
				}
			}

			// If no faces have been added during this pass, we are blocked, and either the MatrixBlock may be full,
			// or there is no more face. So quit this block and process a new one.
			if(!faceAdded)
				break;
		}

	}
	// NB: at the end of this loop, remainingBones may not be empty(), but all remainingBones should have RefCount==0.



	// 4. Re-order matrix use in MatrixBlocks, for minimum matrix change between MatrixBlocks.
	//================================
	vector<CMatrixBlockRemap>	blockRemaps;
	blockRemaps.resize(_MatrixBlocks.size());


	// For all MatrixBlocks > first, try to "mirror" bones from previous.
	for(i=1;i<(sint)_MatrixBlocks.size();i++)
	{
		CMatrixBlock		&mBlock= _MatrixBlocks[i];
		CMatrixBlock		&mPrevBlock= _MatrixBlocks[i-1];
		CMatrixBlockRemap	&remap= blockRemaps[i];

		// First bkup the bone ids in remap table.
		for(j=0;j<(sint)mBlock.NumMatrix;)
		{
			remap.Remap[j]= mBlock.MatrixId[j];
		}

		// For all ids of this blocks, try to mirror them.
		for(j=0;j<(sint)mBlock.NumMatrix;)
		{
			// get the location of this bone in the prev bone.
			sint	idLoc= mPrevBlock.getMatrixIdLocation(mBlock.MatrixId[j]);
			// If not found, or if bigger than current array, fails (cant be mirrored).
			// Or if already mirrored.
			if(idLoc==-1 || idLoc>=(sint)mBlock.NumMatrix || idLoc==j)
			{
				// next id.
				j++;
			}
			else
			{
				// puts me on my mirrored location. and swap with the current one at this mirrored location.
				swap(mBlock.MatrixId[j], mBlock.MatrixId[idLoc]);
				// mBlock.MatrixId[j] is now a candidate for mirror.
			}
		}

		// Then build the Remap table, to re-order faces matrixId which use this matrix block.
		for(j=0;j<(sint)mBlock.NumMatrix;)
		{
			// get the boneid which was at this position j before.
			uint	boneId= remap.Remap[j];
			// search his new position, and store the result in the remap table.
			remap.Remap[j]= mBlock.getMatrixIdLocation(boneId);
		}

		// NB: this matrixBlock is re-ordered. next matrixBlock use this state.
	}


	// For all faces/corners/weights, remap MatrixIds.
	for(i=0;i<(sint)tmpFaces.size();i++)
	{
		CFaceTmp	&face= tmpFaces[i];
		// do it but for matrixblock0.
		if(face.MatrixBlockId!=0)
		{
			CMatrixBlockRemap	&remap= blockRemaps[face.MatrixBlockId];
			// For all corners.
			for(j=0;j<3;j++)
			{
				for(k=0;k<NL3D_MESH_SKINNING_MAX_MATRIX;k++)
				{
					uint	oldId= face.Corner[j].Palette.MatrixId[k];
					face.Corner[j].Palette.MatrixId[k]= (uint8)remap.Remap[oldId];
				}
			}
		}
	}

}


// ***************************************************************************
void	CMesh::CFaceTmp::buildBoneUse(vector<uint>	&boneUse, vector<CMesh::CSkinWeight> &skinWeights)
{
	boneUse.clear();

	// For the 3 corners of the face.
	for(sint i=0;i<3;i++)
	{
		// get the CSkinWeight of this vertex.
		CMesh::CSkinWeight	&sw= skinWeights[Corner[i].Vertex];

		// For all skin weights of this vertex,
		for(sint j=0;j<NL3D_MESH_SKINNING_MAX_MATRIX;j++)
		{
			uint	boneId= sw.MatrixId[j];
			// insert (if not in the array) this bone.
			if( find(boneUse.begin(), boneUse.end(), boneId)==boneUse.end() )
				boneUse.push_back(boneId);
		}
	}


}



// ***************************************************************************
sint	CMesh::CMatrixBlock::getMatrixIdLocation(uint32 boneId) const
{
	for(uint i=0;i<NumMatrix;i++)
	{
		if(MatrixId[i]==boneId)
			return i;
	}

	// not found.
	return -1;
}



} // NL3D
