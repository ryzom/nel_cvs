/** \file mesh.cpp
 * <File description>
 *
 * $Id: mesh.cpp,v 1.45 2002/03/06 10:24:47 corvazier Exp $
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

#include "3d/mesh.h"
#include "3d/mesh_instance.h"
#include "3d/scene.h"
#include "3d/skeleton_model.h"
#include "3d/mesh_morpher.h"
#include "nel/misc/bsphere.h"
#include "3d/stripifier.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// MeshGeom Tools.
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
sint	CMeshGeom::CCornerTmp::Flags=0;


// ***************************************************************************
bool	CMeshGeom::CCornerTmp::operator<(const CCornerTmp &c) const
{
	sint	i;

	// Vert first.
	if(Vertex!=c.Vertex)
		return Vertex<c.Vertex;

	// Order: normal, uvs, color0, color1, skinning.
	if((CCornerTmp::Flags & CVertexBuffer::NormalFlag) && Normal!=c.Normal)
		return Normal<c.Normal;
	for(i=0; i<CVertexBuffer::MaxStage; i++)
	{
		if((CCornerTmp::Flags & (CVertexBuffer::TexCoord0Flag<<i)) && Uvs[i]!=c.Uvs[i])
			return Uvs[i]<c.Uvs[i];
	}
	if((CCornerTmp::Flags & CVertexBuffer::PrimaryColorFlag) && Color!=c.Color)
		return Color<c.Color;
	if((CCornerTmp::Flags & CVertexBuffer::SecondaryColorFlag) && Specular!=c.Specular)
		return Specular<c.Specular;

	if ((CCornerTmp::Flags & CVertexBuffer::PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag)
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
// CMeshGeom.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMeshGeom::CMeshGeom()
{
	_Skinned= false;
	_VertexBufferHardDirty= true;
	_MeshMorpher = new CMeshMorpher;
	_BoneIdComputed = false;
}


// ***************************************************************************
CMeshGeom::~CMeshGeom()
{
	// test (refptr) if the object still exist in memory.
	if(_VertexBufferHard!=NULL)
	{
		// A vbufferhard should still exist only if driver still exist.
		nlassert(_Driver!=NULL);

		// delete it from driver.
		_Driver->deleteVertexBufferHard(_VertexBufferHard);
		_VertexBufferHard= NULL;
	}
	delete _MeshMorpher;
}


// ***************************************************************************
void	CMeshGeom::optimizeTriangleOrder()
{
	CStripifier		stripifier;

	// for all rdrpass of all matrix blocks.
	for(uint mb= 0;mb<_MatrixBlocks.size();mb++)
	{
		for(uint  rp=0; rp<_MatrixBlocks[mb].RdrPass.size(); rp++ )
		{
			// stripify list of triangles of this pass.
			CRdrPass	&pass= _MatrixBlocks[mb].RdrPass[rp];
			stripifier.optimizeTriangles(pass.PBlock, pass.PBlock);
		}
	}

}


// ***************************************************************************
void	CMeshGeom::build (CMesh::CMeshBuild &m, uint numMaxMaterial)
{
	sint	i;

	// Dirt the VBuffer.
	_VertexBufferHardDirty= true;

	// Empty geometry?
	if(m.Vertices.size()==0 || m.Faces.size()==0)
	{
		_VBuffer.setNumVertices(0);
		_VBuffer.reserve(0);
		_MatrixBlocks.clear();
		_BBox.setCenter(CVector::Null);
		_BBox.setSize(CVector::Null);
		return;
	}
	nlassert(numMaxMaterial>0);


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

	_Skinned= ((m.VertexFlags & CVertexBuffer::PaletteSkinFlag)==CVertexBuffer::PaletteSkinFlag);
	// Skinning is OK only if SkinWeights are of same size as vertices.
	_Skinned= _Skinned && (m.Vertices.size()==m.SkinWeights.size());

	// If skinning is KO, remove the Skin option.
	uint	vbFlags= m.VertexFlags;
	if(!_Skinned)
		vbFlags&= ~CVertexBuffer::PaletteSkinFlag;
	// Force presence of vertex.
	vbFlags|= CVertexBuffer::PositionFlag;


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
	uint32		nFaceMB = 0;
	sint		N= tmpFaces.size();
	sint		currentVBIndex=0;

	m.VertLink.clear ();

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
		CMesh::CVertLink vl1(nFaceMB, 0, pFace->Corner[0].VBId);
		CMesh::CVertLink vl2(nFaceMB, 1, pFace->Corner[1].VBId);
		CMesh::CVertLink vl3(nFaceMB, 2, pFace->Corner[2].VBId);
		m.VertLink.push_back(vl1);
		m.VertLink.push_back(vl2);
		m.VertLink.push_back(vl3);
		++nFaceMB;
	}


	/// 3. build the RdrPass material.
	//================================
	uint	mb;

	// For each _MatrixBlocks, point on those materials.
	for(mb= 0;mb<_MatrixBlocks.size();mb++)
	{
		// Build RdrPass ids.
		_MatrixBlocks[mb].RdrPass.resize (numMaxMaterial);

		/// \todo yoyo: TODO_OPTIMIZE: it should be interesting to sort the materials, depending on their attributes. But must change next loop too...
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

	/// 6. Misc.
	//============================
	// BShapes
	this->_MeshMorpher->BlendShapes = m.BlendShapes;

	// sort triangles for better cache use.
	optimizeTriangleOrder();

	// SmartPtr Copy VertexProgram effect.
	this->_MeshVertexProgram= m.MeshVertexProgram;

	/// 7. Compact bones id and build bones name array.
	//=================================================	

	// If skinned
	if(_Skinned)
	{
		// Reserve some space
		_BonesName.reserve (m.BonesNames.size ());

		// Current local bone
		uint currentBone = 0;

		// For each matrix block
		uint matrixBlock;
		for (matrixBlock=0; matrixBlock<_MatrixBlocks.size(); matrixBlock++)
		{
			// Ref on the matrix block
			CMatrixBlock &mb = _MatrixBlocks[matrixBlock];

			// Remap the skeleton index in model index
			std::map<uint, uint> remap;

			// For each matrix
			uint matrix;
			for (matrix=0; matrix<mb.NumMatrix; matrix++)
			{
				// Get bone id in the skeleton
				std::map<uint, uint>::iterator ite = remap.find (mb.MatrixId[matrix]);

				// Not found
				if (ite == remap.end())
				{
					// Insert it
					remap.insert (std::map<uint, uint>::value_type (mb.MatrixId[matrix], currentBone));

					// Check the matrix id
					nlassert (mb.MatrixId[matrix] < m.BonesNames.size());

					// Set the bone name
					_BonesName.push_back (m.BonesNames[mb.MatrixId[matrix]]);

					// Set the id in local
					mb.MatrixId[matrix] = currentBone++;
				}
				else
				{
					// Set the id in local
					mb.MatrixId[matrix] = ite->second;
				}
			}
		}

		// Bone id in local
		_BoneIdComputed = false;
	}


	// End!!
}

// ***************************************************************************
void CMeshGeom::setBlendShapes(std::vector<CBlendShape>&bs)
{
	_MeshMorpher->BlendShapes = bs;
}


// ***************************************************************************
void	CMeshGeom::initInstance(CMeshBaseInstance *mbi)
{
	// init the instance with _MeshVertexProgram infos
	if(_MeshVertexProgram)
		_MeshVertexProgram->initInstance(mbi);
}

// ***************************************************************************
bool	CMeshGeom::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// Speed Clip: clip just the sphere.
	CBSphere	localSphere(_BBox.getCenter(), _BBox.getRadius());
	CBSphere	worldSphere;

	// transform the sphere in WorldMatrix (with nearly good scale info).
	localSphere.applyTransform(worldMatrix, worldSphere);

	// if out of only plane, entirely out.
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		// if SpherMax OUT return false.
		float	d= pyramid[i]*worldSphere.Center;
		if(d>worldSphere.Radius)
			return false;
	}
	return true;
}

// ***************************************************************************
void	CMeshGeom::updateVertexBufferHard(IDriver *drv)
{
	if(!drv->supportVertexBufferHard())
		return;


	// If the mesh is skinned and if the driver do not support hardware palette skinning, still use normal CVertexBuffer.
	if(_VertexBufferHardDirty && _Skinned && !drv->supportPaletteSkinning())
	{
		// delete possible old VBHard.
		if(_VertexBufferHard!=NULL)
		{
			// VertexBufferHard lifetime < Driver lifetime.
			nlassert(_Driver!=NULL);
			_Driver->deleteVertexBufferHard(_VertexBufferHard);
		}
		return;
	}

	// If the vbufferhard is not synced to the vbuffer.
	if(_VertexBufferHardDirty || _VertexBufferHard==NULL)
	{
		_VertexBufferHardDirty= false;

		// delete possible old VBHard.
		if(_VertexBufferHard!=NULL)
		{
			// VertexBufferHard lifetime < Driver lifetime.
			nlassert(_Driver!=NULL);
			_Driver->deleteVertexBufferHard(_VertexBufferHard);
		}

		// bkup drv in a refptr. (so we know if the vbuffer hard has to be deleted).
		_Driver= drv;
		// try to create new one, in AGP Ram
		_VertexBufferHard= _Driver->createVertexBufferHard(_VBuffer.getVertexFormat(), _VBuffer.getValueTypePointer (), _VBuffer.getNumVertices(), IDriver::VBHardAGP);

		// If KO, use normal VertexBuffer.
		if(_VertexBufferHard==NULL)
			return;
		// else, Fill it with VertexBuffer.
		else
		{
			void	*vertexPtr= _VertexBufferHard->lock();

			nlassert(_VBuffer.getVertexFormat() == _VertexBufferHard->getVertexFormat());
			nlassert(_VBuffer.getNumVertices() == _VertexBufferHard->getNumVertices());
			nlassert(_VBuffer.getVertexSize() == _VertexBufferHard->getVertexSize());

			// \todo yoyo: TODO_DX8 and DX8 ???
			// Because same internal format, just copy all block.
			memcpy(vertexPtr, _VBuffer.getVertexCoordPointer(), _VBuffer.getNumVertices() * _VBuffer.getVertexSize() );

			_VertexBufferHard->unlock();
		}

	}
}


// ***************************************************************************
void	CMeshGeom::render(IDriver *drv, CTransformShape *trans, bool opaquePass, float polygonCount, float globalAlpha)
{
	nlassert(drv);
	// get the mesh instance.
	nlassert(dynamic_cast<CMeshInstance*>(trans));
	CMeshInstance	*mi= (CMeshInstance*)trans;
	// get a ptr on scene
	CScene			*ownerScene= mi->getScene();
	// get a ptr on renderTrav
	CRenderTrav		*renderTrav= ownerScene->getRenderTrav();


	// get the skeleton model to which I am binded (else NULL).
	CSkeletonModel		*skeleton;
	skeleton= mi->getSkeletonModel();
	// Is this mesh skinned?? true only if mesh is skinned, skeletonmodel is not NULL, and isSkinApply().
	bool	skinOk= _Skinned && mi->isSkinApply() && skeleton;


	// If skinning, enable driver skinning.
	if(skinOk)
	{
		drv->setupVertexMode(NL3D_VERTEX_MODE_SKINNING);
	}
	// else setup instance matrix
	else
	{
		drv->setupModelMatrix(trans->getWorldMatrix());
	}


	// use MeshVertexProgram effect?
	bool	useMeshVP= _MeshVertexProgram != NULL && drv->isVertexProgramSupported() && !drv->isVertexProgramEmulated();
	bool	useMeshVPLightSetup= false;
	// Disable meshVP with Skinning, because incompatible (hardware use VertexProgram too :) ).
	if(skinOk)
		useMeshVP= false;
	// Test if must setup VP Lighting fragment
	if(useMeshVP)
	{
		bool	meshVPLightSpecular;
		uint	meshVPLightCteStart;
		useMeshVPLightSetup= _MeshVertexProgram->useSceneVPLightSetup(meshVPLightSpecular, meshVPLightCteStart);
		// If use VPLightSetup
		if(useMeshVPLightSetup)
		{
			// setup ctes for lighting
			renderTrav->beginVPLightSetup(meshVPLightCteStart, meshVPLightSpecular, trans->getWorldMatrix().inverted());
		}
	}


	// update the VBufferHard (create/delete), to maybe render in AGP memory.
	updateVertexBufferHard (drv);
	_MeshMorpher->init (&_VBufferOri, &_VBuffer, _VertexBufferHard);
	_MeshMorpher->update (mi->getBlendShapeFactors());

	// Global alpha used ?
	bool globalAlphaUsed=globalAlpha!=1;
	uint8 globalAlphaInt=(uint8)(globalAlpha*255);

	// For all _MatrixBlocks
	for(uint mb=0;mb<_MatrixBlocks.size();mb++)
	{
		CMatrixBlock	&mBlock= _MatrixBlocks[mb];
		if(mBlock.RdrPass.size()==0)
			continue;

		// If skinning, Setup matrixs (else worldmatrix setuped before).
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
				/* Use separate multiply for precision consideration:
					In OpenGL, The modelViewMatrix is first computed in setupModelMatrix(), and so
					the result is nearly identity (because in camera basis).
					If we do setupModelMatrix(skeleton->getWorldMatrix() * skeleton->Bones[curBoneId].getBoneSkinMatrix()),
					the result is worse.
				*/
				drv->setupModelMatrix(skeleton->getWorldMatrix(), idMat);
				drv->multiplyModelMatrix(skeleton->Bones[curBoneId].getBoneSkinMatrix(), idMat);
			}
		}

		// If use MeshGeom special VertexProgram.
		if(useMeshVP)
		{
			// Apply it.
			_MeshVertexProgram->begin(drv, ownerScene, mi);
		}

		// if VB Hard is here, use it.
		if(_VertexBufferHard != NULL)
		{
			// active VB Hard.
			drv->activeVertexBufferHard(_VertexBufferHard);
		}
		else
		{
			// active VB. SoftwareSkinning: reset flags for skinning.
			drv->activeVertexBuffer(_VBuffer);
		}


		// Global alpha ?
		if (globalAlphaUsed)
		{
			// Render all pass.
			for (uint i=0;i<mBlock.RdrPass.size();i++)
			{
				CRdrPass	&rdrPass= mBlock.RdrPass[i];
				// Render with the Materials of the MeshInstance.
				if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (opaquePass == true) ) ||
					 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (opaquePass == false) ) )
				{
					// CMaterial Ref
					CMaterial &material=mi->Materials[rdrPass.MaterialId];

					// Backup opacity
					uint8 opacity=material.getOpacity ();

					// New opacity
					material.setOpacity (globalAlphaInt);

					// Backup the zwrite
					bool zwrite=material.getZWrite ();

					// New zwrite
					material.setZWrite (false);

					// Backup blend
					bool blend=material.getBlend ();
					material.setBlend (true);

					// If use meshVertexProgram, and use VPLightSetup
					if(useMeshVPLightSetup)
					{
						// setup ctes for Vertex Program lighting which depend on material
						renderTrav->changeVPLightSetupMaterial(material);
					}

					// Render
					drv->render(rdrPass.PBlock, material);

					// Resetup backuped opacity
					material.setOpacity (opacity);

					// Resetup backuped zwrite
					material.setZWrite (zwrite);

					// Resetup backuped blend
					material.setBlend (blend);
				}
			}
		}
		else
		{
			// Render all pass.
			for(uint i=0;i<mBlock.RdrPass.size();i++)
			{
				CRdrPass	&rdrPass= mBlock.RdrPass[i];
				// Render with the Materials of the MeshInstance.
				if( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (opaquePass == true) ) ||
					( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (opaquePass == false) )		)
				{
					// If use meshVertexProgram, and use VPLightSetup
					if(useMeshVPLightSetup)
					{
						// setup ctes for Vertex Program lighting which depend on material
						renderTrav->changeVPLightSetupMaterial(mi->Materials[rdrPass.MaterialId]);
					}

					// render primitives
					drv->render(rdrPass.PBlock, mi->Materials[rdrPass.MaterialId]);
				}
			}
		}

		// End VertexProgram effect
		if(useMeshVP)
		{
			// Apply it.
			_MeshVertexProgram->end(drv);
		}
	}

	// disable driver skinning.
	if(skinOk)
	{
		drv->setupVertexMode(NL3D_VERTEX_MODE_NORMAL);
	}

}


// ***************************************************************************
void	CMeshGeom::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 4:
		- BonesName.
	Version 3:
		- MeshVertexProgram.
	Version 2:
		- precompute of triangle order. (nothing more to load).
	Version 1:
		- added blend shapes
	Version 0:
		- separate serialisation CMesh / CMeshGeom.
	*/
	sint ver = f.serialVersion (4);

	// Version 4+: Array of bone name
	if (ver >= 4)
	{
		f.serialCont (_BonesName);
	}

	if (f.isReading())
	{
		// Version3-: Bones index are in skeleton model id list
		_BoneIdComputed = (ver < 4);
	}
	else
	{
		// Warning, if you have skinned this shape, you can't write it anymore because skinning id have been changed! 
		nlassert (_BoneIdComputed==false);
	}

	// Version3+: MeshVertexProgram.
	if (ver >= 3)
	{
		IMeshVertexProgram	*mvp= NULL;
		if(f.isReading())
		{
			f.serialPolyPtr(mvp);
			_MeshVertexProgram= mvp;
		}
		else
		{
			mvp= _MeshVertexProgram;
			f.serialPolyPtr(mvp);
		}
	}
	else if(f.isReading())
	{
		// release vp
		_MeshVertexProgram= NULL;
	}

	// Version1+: _MeshMorpher.
	if (ver >= 1)
		f.serial (*_MeshMorpher);

	// serial geometry.
	f.serial (_VBuffer);
	f.serialCont (_MatrixBlocks);
	f.serial (_BBox);
	f.serial (_Skinned);


	// If _VertexBuffer changed, flag the VertexBufferHard.
	if(f.isReading())
	{
		_VertexBufferHardDirty = true;

		// if >= version 2, reorder of triangles is precomputed, else compute it now.
		if(ver < 2 )
		{
			optimizeTriangleOrder();
		}
	}

}


// ***************************************************************************
// ***************************************************************************
// Skinning.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CMeshGeom::buildSkin(CMesh::CMeshBuild &m, std::vector<CFaceTmp>	&tmpFaces)
{
	sint	i,j,k;
	TBoneMap		remainingBones;
	list<uint>		remainingFaces;


	// 0. normalize SkinWeights: for all weights at 0, copy the matrixId from 0th matrix => no random/bad use of matrix.
	//================================
	for(i=0;i<(sint)m.SkinWeights.size();i++)
	{
		CMesh::CSkinWeight	&sw= m.SkinWeights[i];

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
			CMesh::CSkinWeight	&sw= m.SkinWeights[face.Corner[j].Vertex];
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
						CMesh::CSkinWeight	&sw= m.SkinWeights[face.Corner[j].Vertex];

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
		for(j=0;j<(sint)mBlock.NumMatrix;j++)
		{
			remap.Remap[j]= mBlock.MatrixId[j];
		}

		// For all ids of this blocks, try to mirror them.
		for(j=0;j<(sint)mBlock.NumMatrix;j++)
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
		for(j=0;j<(sint)mBlock.NumMatrix;j++)
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
void	CMeshGeom::CFaceTmp::buildBoneUse(vector<uint>	&boneUse, vector<CMesh::CSkinWeight> &skinWeights)
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
sint	CMeshGeom::CMatrixBlock::getMatrixIdLocation(uint32 boneId) const
{
	for(uint i=0;i<NumMatrix;i++)
	{
		if(MatrixId[i]==boneId)
			return i;
	}

	// not found.
	return -1;
}


// ***************************************************************************
float	CMeshGeom::getNumTriangles (float distance)
{
	// Sum of triangles
	uint32 triCount=0;

	// For each matrix block
	uint mbCount=_MatrixBlocks.size();
	for (uint mb=0; mb<mbCount; mb++)
	{
		CMatrixBlock &block=_MatrixBlocks[mb];

		// Count of primitive block
		uint pCount=block.RdrPass.size();
		for (uint pb=0; pb<pCount; pb++)
		{
			// Ref on the primitive block
			CRdrPass &pass=block.RdrPass[pb];

			// Sum tri
			triCount+=pass.PBlock.getNumTriangles ();
		}
	}
	return (float)triCount;
}


// ***************************************************************************
void	CMeshGeom::computeBonesId (CSkeletonModel *skeleton)
{
	// Already computed ?
	if (!_BoneIdComputed)
	{
		// Get a pointer on the skeleton
		nlassert (skeleton);
		if (skeleton)
		{
			// For each matrix block
			uint matrixBlock;
			for (matrixBlock=0; matrixBlock<_MatrixBlocks.size(); matrixBlock++)
			{
				// Ref on the matrix block
				CMatrixBlock &mb = _MatrixBlocks[matrixBlock];

				// For each matrix
				uint matrix;
				for (matrix=0; matrix<mb.NumMatrix; matrix++)
				{
					// Get bone id in the skeleton
					nlassert (mb.MatrixId[matrix]<_BonesName.size());
					sint32 boneId = skeleton->getBoneIdByName (_BonesName[mb.MatrixId[matrix]]);

					// Bones found ?
					if (boneId != -1)
					{
						// Set the bone id
						mb.MatrixId[matrix] = (uint32)boneId;
					}
					else
					{
						// Put id 0
						mb.MatrixId[matrix] = 0;

						// Error
						nlwarning ("Bone %s not found in the skeleton.", _BonesName[mb.MatrixId[matrix]].c_str());
					}
				}
			}

			// Computed
			_BoneIdComputed = true;
		}
	}
}



// ***************************************************************************
// ***************************************************************************
// CMeshBuild components.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
CMesh::CCorner::CCorner()
{
	sint	i;
	Vertex= 0;
	Normal= CVector::Null;
	for(i=0;i<CVertexBuffer::MaxStage;i++)
		Uvs[i]= CUV(0,0);
	Color.set(255,255,255,255);
	Specular.set(0,0,0,0);
}


// ***************************************************************************
void CMesh::CCorner::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Vertex);
	f.serial(Normal);
	for(int i=0;i<CVertexBuffer::MaxStage;++i) f.serial(Uvs[i]);
	f.serial(Color);
	f.serial(Specular);
}

// ***************************************************************************
void CMesh::CFace::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	for(int i=0;i<3;++i) 
		f.serial(Corner[i]);
	f.serial(MaterialId);
}

// ***************************************************************************
void CMesh::CSkinWeight::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	for(int i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;++i)
	{
		f.serial(MatrixId[i]);
		f.serial(Weights[i]);
	}
}

// ***************************************************************************
/* Serialization is not used.
void CMesh::CMeshBuild::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint	ver= f.serialVersion(0);

	// Serial mesh base (material info).
	CMeshBaseBuild::serial(f);

	// Serial Geometry.
	f.serial( VertexFlags );
	f.serialCont( Vertices );
	f.serialCont( SkinWeights );
	f.serialCont( Faces );

}*/


// ***************************************************************************
// ***************************************************************************
// CMesh.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
CMesh::CMesh()
{
	// create the MeshGeom
	_MeshGeom= new CMeshGeom;
}
// ***************************************************************************
CMesh::~CMesh()
{
	// delete the MeshGeom
	delete _MeshGeom;
}


// ***************************************************************************
CMesh::CMesh(const CMesh &mesh)
{
	// create the MeshGeom
	_MeshGeom= new CMeshGeom(*mesh._MeshGeom);
}


// ***************************************************************************
CMesh	&CMesh::operator=(const CMesh &mesh)
{
	// Copy CMeshBase part
	(CMeshBase&)*this= (CMeshBase&)mesh;

	// copy content of meshGeom.
	*_MeshGeom= *mesh._MeshGeom;


	return *this;
}



// ***************************************************************************
void	CMesh::build (CMeshBase::CMeshBaseBuild &mbase, CMeshBuild &m)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase (mbase);

	// build the geometry.
	_MeshGeom->build (m, mbase.Materials.size());
}

// ***************************************************************************
void CMesh::setBlendShapes(std::vector<CBlendShape>&bs)
{
	_MeshGeom->setBlendShapes (bs);
}

// ***************************************************************************
void	CMesh::build(CMeshBase::CMeshBaseBuild &mbuild, CMeshGeom &meshGeom)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase(mbuild);

	// build the geometry.
	*_MeshGeom= meshGeom;
}


// ***************************************************************************
CTransformShape		*CMesh::createInstance(CScene &scene)
{
	// Create a CMeshInstance, an instance of a mesh.
	//===============================================
	CMeshInstance		*mi= (CMeshInstance*)scene.createModel(NL3D::MeshInstanceId);
	mi->Shape= this;


	// instanciate the material part of the Mesh, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi, &scene);


	// do some instance init for MeshGeom
	_MeshGeom->initInstance(mi);


	return mi;
}


// ***************************************************************************
bool	CMesh::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	return _MeshGeom->clip(pyramid, worldMatrix);
}


// ***************************************************************************
void	CMesh::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	_MeshGeom->render(drv, trans, passOpaque, 0);
}


// ***************************************************************************
void	CMesh::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 6:
		- cut in serialisation, because of:
			- bad ITexture serialisation (with no version....) => must cut.  (see CMeshBase serial).
			- because of this and to simplify, make a cut too in CMesh serialisation.
	NB : all old version code is dropped.
	*/
	sint	ver= f.serialVersion(6);


	if(ver<6)
		throw NLMISC::EStream(f, "Mesh in Stream is too old (Mesh version < 6)");


	// serial Materials infos contained in CMeshBase.
	CMeshBase::serialMeshBase(f);


	// serial geometry.
	_MeshGeom->serial(f);

}


// ***************************************************************************
const NLMISC::CAABBoxExt& CMesh::getBoundingBox() const
{
	return _MeshGeom->getBoundingBox();
}
// ***************************************************************************
const CVertexBuffer &CMesh::getVertexBuffer() const 
{ 
	return _MeshGeom->getVertexBuffer() ; 
}
// ***************************************************************************
uint CMesh::getNbMatrixBlock() const 
{ 
	return _MeshGeom->getNbMatrixBlock(); 
}
// ***************************************************************************
uint CMesh::getNbRdrPass(uint matrixBlockIndex) const 
{ 
	return _MeshGeom->getNbRdrPass(matrixBlockIndex) ; 
}
// ***************************************************************************
const CPrimitiveBlock &CMesh::getRdrPassPrimitiveBlock(uint matrixBlockIndex, uint renderingPassIndex) const
{
	return _MeshGeom->getRdrPassPrimitiveBlock(matrixBlockIndex, renderingPassIndex) ;
}
// ***************************************************************************
uint32 CMesh::getRdrPassMaterial(uint matrixBlockIndex, uint renderingPassIndex) const
{
	return _MeshGeom->getRdrPassMaterial(matrixBlockIndex, renderingPassIndex) ;
}
// ***************************************************************************
float	CMesh::getNumTriangles (float distance)
{
	// A CMesh do not degrad himself, so return 0, to not be taken into account.
	return 0;
}
// ***************************************************************************
const	CMeshGeom& CMesh::getMeshGeom () const
{
	return *_MeshGeom;
}
// ***************************************************************************
void	CMesh::computeBonesId (CSkeletonModel *skeleton)
{
	nlassert (_MeshGeom);
	_MeshGeom->computeBonesId (skeleton);
}


} // NL3D
