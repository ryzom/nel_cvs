/** \file mesh_mrm.cpp
 * <File description>
 *
 * $Id: mesh_mrm.cpp,v 1.65 2003/08/14 08:52:27 corvazier Exp $
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

#include "nel/misc/bsphere.h"
#include "nel/misc/system_info.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/fast_mem.h"
#include "3d/mesh_mrm.h"
#include "3d/mrm_builder.h"
#include "3d/mrm_parameters.h"
#include "3d/mesh_mrm_instance.h"
#include "3d/scene.h"
#include "3d/skeleton_model.h"
#include "3d/stripifier.h"
#include "3d/mesh_blender.h"
#include "3d/render_trav.h"
#include "nel/misc/fast_floor.h"
#include "3d/raw_skin.h"
#include "3d/shifted_triangle_cache.h"
#include "3d/texture_file.h"


using namespace NLMISC;
using namespace std;


namespace NL3D 
{


H_AUTO_DECL( NL3D_MeshMRMGeom_RenderSkinned )
H_AUTO_DECL( NL3D_MeshMRMGeom_RenderShadow )


// ***************************************************************************
// ***************************************************************************
// CMeshMRMGeom::CLod
// ***************************************************************************
// ***************************************************************************

	
// ***************************************************************************
void		CMeshMRMGeom::CLod::serial(NLMISC::IStream &f)
{
	/*
	Version 2:
		- precompute of triangle order. (nothing more to load).
	Version 1:
		- add VertexBlocks;
	Version 0:
		- base vdrsion.
	*/

	sint	ver= f.serialVersion(2);
	uint	i;

	f.serial(NWedges);
	f.serialCont(RdrPass);
	f.serialCont(Geomorphs);
	f.serialCont(MatrixInfluences);

	// Serial array of InfluencedVertices. NB: code written so far for NL3D_MESH_SKINNING_MAX_MATRIX==4 only.
	nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);
	for(i= 0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
	{
		f.serialCont(InfluencedVertices[i]);
	}

	if(ver>=1)
		f.serialCont(SkinVertexBlocks);
	else
		buildSkinVertexBlocks();

	// if >= version 2, reorder of triangles is precomputed, else compute it now.
	if(ver<2)
		optimizeTriangleOrder();

}


// ***************************************************************************
void		CMeshMRMGeom::CLod::buildSkinVertexBlocks()
{
	contReset(SkinVertexBlocks);


	// The list of vertices. true if used by this lod.
	vector<bool>		vertexMap;
	vertexMap.resize(NWedges, false);


	// from InfluencedVertices, aknoledge what vertices are used.
	uint	i;
	for(i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
	{
		uint		nInf= InfluencedVertices[i].size();
		if( nInf==0 )
			continue;
		uint32		*infPtr= &(InfluencedVertices[i][0]);

		//  for all InfluencedVertices only.
		for(;nInf>0;nInf--, infPtr++)
		{
			uint	index= *infPtr;
			vertexMap[index]= true;
		}
	}

	// For all vertices, test if they are used, and build the according SkinVertexBlocks;
	CVertexBlock	*vBlock= NULL;
	for(i=0; i<vertexMap.size();i++)
	{
		if(vertexMap[i])
		{
			// preceding block?
			if(vBlock)
			{
				// yes, extend it.
				vBlock->NVertices++;
			}
			else
			{
				// no, append a new one.
				SkinVertexBlocks.push_back(CVertexBlock());
				vBlock= &SkinVertexBlocks[SkinVertexBlocks.size()-1];
				vBlock->VertexStart= i;
				vBlock->NVertices= 1;
			}
		}
		else
		{
			// Finish the preceding block (if any).
			vBlock= NULL;
		}
	}

}


// ***************************************************************************
void		CMeshMRMGeom::CLod::optimizeTriangleOrder()
{
	CStripifier		stripifier;

	// for all rdrpass
	for(uint  rp=0; rp<RdrPass.size(); rp++ )
	{
		// stripify list of triangles of this pass.
		CRdrPass	&pass= RdrPass[rp];
		stripifier.optimizeTriangles(pass.PBlock, pass.PBlock);
	}

}


// ***************************************************************************
// ***************************************************************************
// CMeshMRMGeom.
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
CMeshMRMGeom::CMeshMRMGeom()
{
	_VertexBufferHardDirty= true;
	_Skinned= false;
	_NbLodLoaded= 0;
	_BoneIdComputed = false;
	_BoneIdExtended = false;
	_PreciseClipping= false;
	_SupportSkinGrouping= false;
	_MeshDataId= 0;
	_SupportMeshBlockRendering=  false;
	_MBRCurrentLodId= 0;
	_SupportShadowSkinGrouping= false;
}


// ***************************************************************************
CMeshMRMGeom::~CMeshMRMGeom()
{
	deleteVertexBufferHard();
}


// ***************************************************************************
void			CMeshMRMGeom::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	// check input.
	if(distanceFinest<0)	return;
	if(distanceMiddle<=distanceFinest)	return;
	if(distanceCoarsest<=distanceMiddle)	return;

	// Change.
	_LevelDetail.DistanceFinest= distanceFinest;
	_LevelDetail.DistanceMiddle= distanceMiddle;
	_LevelDetail.DistanceCoarsest= distanceCoarsest;

	// compile 
	_LevelDetail.compileDistanceSetup();
}


// ***************************************************************************
void			CMeshMRMGeom::build(CMesh::CMeshBuild &m, std::vector<CMesh::CMeshBuild*> &bsList,
									uint numMaxMaterial, const CMRMParameters &params)
{

	// Dirt the VBuffer.
	_VertexBufferHardDirty= true;

	// Empty geometry?
	if(m.Vertices.size()==0 || m.Faces.size()==0)
	{
		_VBufferFinal.setNumVertices(0);
		_VBufferFinal.reserve(0);
		_Lods.clear();
		_BBox.setCenter(CVector::Null);
		_BBox.setSize(CVector::Null);
		return;
	}
	nlassert(numMaxMaterial>0);


	// SmartPtr Copy VertexProgram effect.
	//================================================	
	this->_MeshVertexProgram= m.MeshVertexProgram;


	/// 0. First, make bbox.
	//======================
	// NB: this is equivalent as building BBox from MRM VBuffer, because CMRMBuilder create new vertices 
	// which are just interpolation of original vertices.
	_BBox= makeBBox(m.Vertices);


	/// 1. Launch the MRM build process.
	//================================================
	CMRMBuilder			mrmBuilder;
	CMeshBuildMRM		meshBuildMRM;

	mrmBuilder.compileMRM(m, bsList, params, meshBuildMRM, numMaxMaterial);

	// Then just copy result!
	//================================================
	_VBufferFinal= meshBuildMRM.VBuffer;
	_Lods= meshBuildMRM.Lods;
	_Skinned= meshBuildMRM.Skinned;
	_SkinWeights= meshBuildMRM.SkinWeights;

	// Compute degradation control.
	//================================================
	_LevelDetail.DistanceFinest= meshBuildMRM.DistanceFinest;
	_LevelDetail.DistanceMiddle= meshBuildMRM.DistanceMiddle;
	_LevelDetail.DistanceCoarsest= meshBuildMRM.DistanceCoarsest;
	nlassert(_LevelDetail.DistanceFinest>=0);
	nlassert(_LevelDetail.DistanceMiddle > _LevelDetail.DistanceFinest);
	nlassert(_LevelDetail.DistanceCoarsest > _LevelDetail.DistanceMiddle);
	// Compute OODistDelta and DistancePow
	_LevelDetail.compileDistanceSetup();


	// Build the _LodInfos.
	//================================================
	_LodInfos.resize(_Lods.size());
	uint32	precNWedges= 0;
	uint	i;
	for(i=0;i<_Lods.size();i++)
	{
		_LodInfos[i].StartAddWedge= precNWedges;
		_LodInfos[i].EndAddWedges= _Lods[i].NWedges;
		precNWedges= _Lods[i].NWedges;
		// LodOffset is filled in serial() when stream is input.
	}
	// After build, all lods are present in memory. 
	_NbLodLoaded= _Lods.size();


	// For load balancing.
	//================================================
	// compute Max Face Used
	_LevelDetail.MaxFaceUsed= 0;
	_LevelDetail.MinFaceUsed= 0;
	// Count of primitive block
	if(_Lods.size()>0)
	{
		uint	pb;
		// Compute MinFaces.
		CLod	&firstLod= _Lods[0];
		for (pb=0; pb<firstLod.RdrPass.size(); pb++)
		{
			CRdrPass &pass= firstLod.RdrPass[pb];
			// Sum tri
			_LevelDetail.MinFaceUsed+= pass.PBlock.getNumTriangles ();
		}
		// Compute MaxFaces.
		CLod	&lastLod= _Lods[_Lods.size()-1];
		for (pb=0; pb<lastLod.RdrPass.size(); pb++)
		{
			CRdrPass &pass= lastLod.RdrPass[pb];
			// Sum tri
			_LevelDetail.MaxFaceUsed+= pass.PBlock.getNumTriangles ();
		}
	}


	// For skinning.
	//================================================
	if( _Skinned )
	{
		bkupOriginalSkinVertices();
	}
	// Inform that the mesh data has changed
	dirtMeshDataId();


	// For AGP SKinning optim, and for Render optim
	//================================================
	for(i=0;i<_Lods.size();i++)
	{
		_Lods[i].buildSkinVertexBlocks();
		// sort triangles for better cache use.
		_Lods[i].optimizeTriangleOrder();
	}

	// Copy Blend Shapes
	//================================================	
	_MeshMorpher.BlendShapes = meshBuildMRM.BlendShapes;
	

	// Compact bone id and build a bone id names
	//================================================	

	// Skinned ?
	if (_Skinned)
	{
		// Remap
		std::map<uint, uint> remap;

		// Current bone
		uint currentBone = 0;

		// Reserve memory
		_BonesName.reserve (m.BonesNames.size());

		// For each vertices
		uint vert;
		for (vert=0; vert<_SkinWeights.size(); vert++)
		{
			// Found one ?
			bool found=false;
			
			// For each weight
			uint weight;
			for (weight=0; weight<NL3D_MESH_SKINNING_MAX_MATRIX; weight++)
			{
				// Active ?
				if ((_SkinWeights[vert].Weights[weight]>0)||(weight==0))
				{
					// Look for it
					std::map<uint, uint>::iterator ite = remap.find (_SkinWeights[vert].MatrixId[weight]);

					// Find ?
					if (ite == remap.end())
					{
						// Insert it
						remap.insert (std::map<uint, uint>::value_type (_SkinWeights[vert].MatrixId[weight], currentBone));

						// Check the id
						nlassert (_SkinWeights[vert].MatrixId[weight]<m.BonesNames.size());

						// Set the bone name
						_BonesName.push_back (m.BonesNames[_SkinWeights[vert].MatrixId[weight]]);

						// Set the local bone id
						_SkinWeights[vert].MatrixId[weight] = currentBone++;
					}
					else
					{
						// Set the local bone id
						_SkinWeights[vert].MatrixId[weight] = ite->second;
					}

					// Found one
					found = true;
				}
			}

			// Found one ?
			nlassert (found);
		}

		// Remap the vertex influence by lods
		uint lod;
		for (lod=0; lod<_Lods.size(); lod++)
		{
			// For each matrix used
			uint matrix;
			for (matrix=0; matrix<_Lods[lod].MatrixInfluences.size(); matrix++)
			{
				// Remap
				std::map<uint, uint>::iterator ite = remap.find (_Lods[lod].MatrixInfluences[matrix]);

				// Find ?
				nlassert (ite != remap.end());

				// Remap
				_Lods[lod].MatrixInfluences[matrix] = ite->second;
			}
		}
	}

	// Misc.
	//===================
	// Some runtime not serialized compilation
	compileRunTime();

}

// ***************************************************************************
void	CMeshMRMGeom::applyMaterialRemap(const std::vector<sint> &remap)
{
	for(uint lod=0;lod<getNbLod();lod++)
	{
		for(uint rp=0;rp<getNbRdrPass(lod);rp++)
		{
			// remap
			uint32	&matId= _Lods[lod].RdrPass[rp].MaterialId;
			nlassert(remap[matId]>=0);
			matId= remap[matId];
		}
	}
}

// ***************************************************************************
void	CMeshMRMGeom::applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod, IVertexBufferHard *currentVBHard)
{
	if(currentVBHard!=NULL)
	{
		// must write into it
		uint8	*vertexDestPtr= (uint8*)currentVBHard->lock();
		nlassert(_VBufferFinal.getVertexSize() == currentVBHard->getVertexSize());

		// apply the geomorph
		applyGeomorphWithVBHardPtr(geoms, alphaLod, vertexDestPtr);

		// unlock. ATI: copy only geomorphed vertices.
		currentVBHard->unlock(0, geoms.size());
	}
	else
		applyGeomorphWithVBHardPtr(geoms, alphaLod, NULL);

}


// ***************************************************************************
void	CMeshMRMGeom::applyGeomorphWithVBHardPtr(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod, uint8 *vertexDestPtr)
{
	// no geomorphs? quit.
	if(geoms.size()==0)
		return;

	uint		i;
	clamp(alphaLod, 0.f, 1.f);
	float		a= alphaLod;
	float		a1= 1 - alphaLod;
	uint		ua= (uint)(a*256);
	clamp(ua, (uint)0, (uint)256);
	uint		ua1= 256 - ua;


	// info from VBuffer.
	uint8		*vertexPtr= (uint8*)_VBufferFinal.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// because of the unrolled code for 4 first UV, must assert this.
	nlassert(CVertexBuffer::MaxStage>=4);
	// must have XYZ.
	nlassert(flags & CVertexBuffer::PositionFlag);


	// If VBuffer Hard disabled
	if(vertexDestPtr==NULL)
	{
		// write into vertexPtr.
		vertexDestPtr= vertexPtr;
	}


	// if it is a common format
	if( flags== (CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag | CVertexBuffer::TexCoord0Flag) &&
		_VBufferFinal.getValueType(CVertexBuffer::TexCoord0) == CVertexBuffer::Float2 )
	{
		// use a faster method
		applyGeomorphPosNormalUV0(geoms, vertexPtr, vertexDestPtr, vertexSize, a, a1);
	}
	else
	{
		// if an offset is 0, it means that the component is not in the VBuffer.
		sint32		normalOff;
		sint32		colorOff;
		sint32		specularOff;
		sint32		uvOff[CVertexBuffer::MaxStage];
		bool		has3Coords[CVertexBuffer::MaxStage];


		// Compute offset of each component of the VB.
		if(flags & CVertexBuffer::NormalFlag)
			normalOff= _VBufferFinal.getNormalOff();
		else
			normalOff= 0;
		if(flags & CVertexBuffer::PrimaryColorFlag)
			colorOff= _VBufferFinal.getColorOff();
		else
			colorOff= 0;
		if(flags & CVertexBuffer::SecondaryColorFlag)
			specularOff= _VBufferFinal.getSpecularOff();
		else
			specularOff= 0;
			
		for(i= 0; i<CVertexBuffer::MaxStage;i++)
		{
			if(flags & (CVertexBuffer::TexCoord0Flag<<i))
			{
				uvOff[i]= _VBufferFinal.getTexCoordOff(i);			
				has3Coords[i] = (_VBufferFinal.getValueType(i + CVertexBuffer::TexCoord0) == CVertexBuffer::Float3);
			}
			else
			{
				uvOff[i]= 0;
			}
		}


		// For all geomorphs.
		uint			nGeoms= geoms.size();
		CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
		uint8			*destPtr= vertexDestPtr;
		/* NB: optimisation: lot of "if" in this Loop, but because of BTB, they always cost nothing (prediction is good).
		   NB: this also is why we unroll the 4 1st Uv. The other (if any), are done in the other loop.
		   NB: optimisation for AGP write cominers: the order of write (vertex, normal, uvs...) is important for good
		   use of AGP write combiners.
		   We have 2 version : one that tests for 3 coordinates texture coords, and one that doesn't
		*/

		if (!has3Coords[0] && !has3Coords[1] && !has3Coords[2] && !has3Coords[3])
		{
			// there are no texture coordinate of dimension 3
			for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
			{
				uint8			*startPtr=	vertexPtr + ptrGeom->Start*vertexSize;
				uint8			*endPtr=	vertexPtr + ptrGeom->End*vertexSize;

				// Vertex.
				{
					CVector		*start=	(CVector*)startPtr;
					CVector		*end=	(CVector*)endPtr;
					CVector		*dst=	(CVector*)destPtr;
					*dst= *start * a + *end * a1;
				}

				// Normal.
				if(normalOff)
				{
					CVector		*start= (CVector*)(startPtr + normalOff);
					CVector		*end=	(CVector*)(endPtr   + normalOff);
					CVector		*dst=	(CVector*)(destPtr  + normalOff);
					*dst= *start * a + *end * a1;
				}


				// Uvs.
				// uv[0].
				if(uvOff[0])
				{				
					// Uv.
					CUV			*start= (CUV*)(startPtr + uvOff[0]);
					CUV			*end=	(CUV*)(endPtr   + uvOff[0]);
					CUV			*dst=	(CUV*)(destPtr  + uvOff[0]);
					*dst= *start * a + *end * a1;				
				}
				// uv[1].
				if(uvOff[1])
				{				
					// Uv.
					CUV			*start= (CUV*)(startPtr + uvOff[1]);
					CUV			*end=	(CUV*)(endPtr   + uvOff[1]);
					CUV			*dst=	(CUV*)(destPtr  + uvOff[1]);
					*dst= *start * a + *end * a1;				
				}
				// uv[2].
				if(uvOff[2])
				{				
					CUV			*start= (CUV*)(startPtr + uvOff[2]);
					CUV			*end=	(CUV*)(endPtr   + uvOff[2]);
					CUV			*dst=	(CUV*)(destPtr  + uvOff[2]);
					*dst= *start * a + *end * a1;				
				}
				// uv[3].
				if(uvOff[3])
				{				
					// Uv.
					CUV			*start= (CUV*)(startPtr + uvOff[3]);
					CUV			*end=	(CUV*)(endPtr   + uvOff[3]);
					CUV			*dst=	(CUV*)(destPtr  + uvOff[3]);
					*dst= *start * a + *end * a1;				
				}
			}
		}
		else // THERE ARE TEXTURE COORDINATES OF DIMENSION 3
		{
			for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
			{
				uint8			*startPtr=	vertexPtr + ptrGeom->Start*vertexSize;
				uint8			*endPtr=	vertexPtr + ptrGeom->End*vertexSize;

				// Vertex.
				{
					CVector		*start=	(CVector*)startPtr;
					CVector		*end=	(CVector*)endPtr;
					CVector		*dst=	(CVector*)destPtr;
					*dst= *start * a + *end * a1;
				}

				// Normal.
				if(normalOff)
				{
					CVector		*start= (CVector*)(startPtr + normalOff);
					CVector		*end=	(CVector*)(endPtr   + normalOff);
					CVector		*dst=	(CVector*)(destPtr  + normalOff);
					*dst= *start * a + *end * a1;
				}
				// Uvs.
				// uv[0].
				if(uvOff[0])
				{
					if (!has3Coords[0])
					{
						// Uv.
						CUV			*start= (CUV*)(startPtr + uvOff[0]);
						CUV			*end=	(CUV*)(endPtr   + uvOff[0]);
						CUV			*dst=	(CUV*)(destPtr  + uvOff[0]);
						*dst= *start * a + *end * a1;
					}
					else
					{
						// Uv.
						CUVW		*start= (CUVW*)(startPtr + uvOff[0]);
						CUVW		*end=	(CUVW*)(endPtr   + uvOff[0]);
						CUVW		*dst=	(CUVW*)(destPtr  + uvOff[0]);
						*dst= *start * a + *end * a1;
					}
				}
				// uv[1].
				if(uvOff[1])
				{
					if (!has3Coords[1])
					{
						// Uv.
						CUV			*start= (CUV*)(startPtr + uvOff[1]);
						CUV			*end=	(CUV*)(endPtr   + uvOff[1]);
						CUV			*dst=	(CUV*)(destPtr  + uvOff[1]);
						*dst= *start * a + *end * a1;
					}
					else
					{
						// Uv.
						CUVW		*start= (CUVW*)(startPtr + uvOff[1]);
						CUVW		*end=	(CUVW*)(endPtr   + uvOff[1]);
						CUVW		*dst=	(CUVW*)(destPtr  + uvOff[1]);
						*dst= *start * a + *end * a1;
					}
				}
				// uv[2].
				if(uvOff[2])
				{
					if (!has3Coords[2])
					{
						// Uv.
						CUV			*start= (CUV*)(startPtr + uvOff[2]);
						CUV			*end=	(CUV*)(endPtr   + uvOff[2]);
						CUV			*dst=	(CUV*)(destPtr  + uvOff[2]);
						*dst= *start * a + *end * a1;
					}
					else
					{
						// Uv.
						CUVW		*start= (CUVW*)(startPtr + uvOff[2]);
						CUVW		*end=	(CUVW*)(endPtr   + uvOff[2]);
						CUVW		*dst=	(CUVW*)(destPtr  + uvOff[2]);
						*dst= *start * a + *end * a1;
					}
				}
				// uv[3].
				if(uvOff[3])
				{
					if (!has3Coords[3])
					{
						// Uv.
						CUV			*start= (CUV*)(startPtr + uvOff[3]);
						CUV			*end=	(CUV*)(endPtr   + uvOff[3]);
						CUV			*dst=	(CUV*)(destPtr  + uvOff[3]);
						*dst= *start * a + *end * a1;
					}
					else
					{
						// Uv.
						CUVW		*start= (CUVW*)(startPtr + uvOff[3]);
						CUVW		*end=	(CUVW*)(endPtr   + uvOff[3]);
						CUVW		*dst=	(CUVW*)(destPtr  + uvOff[3]);
						*dst= *start * a + *end * a1;
					}
				}
				// color.
				if(colorOff)
				{
					CRGBA		*start= (CRGBA*)(startPtr + colorOff);
					CRGBA		*end=	(CRGBA*)(endPtr   + colorOff);
					CRGBA		*dst=	(CRGBA*)(destPtr  + colorOff);
					dst->blendFromui(*start, *end,  ua1);
				}
				// specular.
				if(specularOff)
				{
					CRGBA		*start= (CRGBA*)(startPtr + specularOff);
					CRGBA		*end=	(CRGBA*)(endPtr   + specularOff);
					CRGBA		*dst=	(CRGBA*)(destPtr  + specularOff);
					dst->blendFromui(*start, *end,  ua1);
				}
			}
		}


		// Process extra UVs (maybe never, so don't bother optims :)).
		// For all stages after 4.
		for(i=4;i<CVertexBuffer::MaxStage;i++)
		{
			uint			nGeoms= geoms.size();
			CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
			uint8			*destPtr= vertexDestPtr;

			if(uvOff[i]==0)
				continue;

			// For all geomorphs.
			for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
			{
				uint8			*startPtr=	vertexPtr + ptrGeom->Start*vertexSize;
				uint8			*endPtr=	vertexPtr + ptrGeom->End*vertexSize;

				// uv[i].
				// Uv.
				if (!has3Coords[i])
				{
					CUV			*start= (CUV*)(startPtr + uvOff[i]);
					CUV			*end=	(CUV*)(endPtr	+ uvOff[i]);
					CUV			*dst=	(CUV*)(destPtr	+ uvOff[i]);
					*dst= *start * a + *end * a1;
				}
				else
				{
					CUVW		*start= (CUVW*)(startPtr + uvOff[i]);
					CUVW		*end=	(CUVW*)(endPtr	+ uvOff[i]);
					CUVW		*dst=	(CUVW*)(destPtr	+ uvOff[i]);
					*dst= *start * a + *end * a1;
				}
			}
		}
	}
}


// ***************************************************************************
void	CMeshMRMGeom::applyGeomorphPosNormalUV0(std::vector<CMRMWedgeGeom>  &geoms, uint8 *vertexPtr, uint8 *vertexDestPtr, sint32 vertexSize, float a, float a1)
{
	nlassert(vertexSize==32);


	// For all geomorphs.
	uint			nGeoms= geoms.size();
	CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
	uint8			*destPtr= vertexDestPtr;
	for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
	{
		// Consider the Pos/Normal/UV as an array of 8 float to interpolate.
		float			*start=	(float*)(vertexPtr + (ptrGeom->Start<<5));
		float			*end=	(float*)(vertexPtr + (ptrGeom->End<<5));
		float			*dst=	(float*)(destPtr);

		// unrolled
		dst[0]= start[0] * a + end[0]* a1;
		dst[1]= start[1] * a + end[1]* a1;
		dst[2]= start[2] * a + end[2]* a1;
		dst[3]= start[3] * a + end[3]* a1;
		dst[4]= start[4] * a + end[4]* a1;
		dst[5]= start[5] * a + end[5]* a1;
		dst[6]= start[6] * a + end[6]* a1;
		dst[7]= start[7] * a + end[7]* a1;
	}
}


// ***************************************************************************
void	CMeshMRMGeom::initInstance(CMeshBaseInstance *mbi)
{
	// init the instance with _MeshVertexProgram infos
	if(_MeshVertexProgram)
		_MeshVertexProgram->initInstance(mbi);
}


// ***************************************************************************
bool	CMeshMRMGeom::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
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

	// test if must do a precise clip, according to mesh size.
	if( _PreciseClipping )
	{
		CPlane	localPlane;

		// if out of only plane, entirely out.
		for(sint i=0;i<(sint)pyramid.size();i++)
		{
			// Transform the pyramid in Object space.
			localPlane= pyramid[i]*worldMatrix;
			// localPlane must be normalized, because worldMatrix mya have a scale.
			localPlane.normalize();
			// if the box is not partially inside the plane, quit
			if( !_BBox.clipBack(localPlane) )
				return false;
		}
	}

	return true;
}


// ***************************************************************************
inline sint	CMeshMRMGeom::chooseLod(float alphaMRM, float &alphaLod)
{
	// Choose what Lod to draw.
	alphaMRM*= _Lods.size()-1;
	sint	numLod= (sint)ceil(alphaMRM);
	if(numLod==0)
	{
		numLod= 0;
		alphaLod= 0;
	}
	else
	{
		// Lerp beetween lod i-1 and lod i.
		alphaLod= alphaMRM-(numLod-1);
	}


	// If lod chosen is not loaded, take the best loaded.
	if(numLod>=(sint)_NbLodLoaded)
	{
		numLod= _NbLodLoaded-1;
		alphaLod= 1;
	}

	return numLod;
}


// ***************************************************************************
void	CMeshMRMGeom::render(IDriver *drv, CTransformShape *trans, float polygonCount, uint32 rdrFlags, float globalAlpha)
{
	nlassert(drv);
	if(_Lods.size()==0)
		return;


	// get the meshMRM instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);
	// get a ptr on scene
	CScene				*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav			*renderTrav= &ownerScene->getRenderTrav();


	// get the result of the Load Balancing.
	float	alphaMRM= _LevelDetail.getLevelDetailFromPolyCount(polygonCount);

	// choose the lod.
	float	alphaLod;
	sint	numLod= chooseLod(alphaMRM, alphaLod);


	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];
	if(lod.RdrPass.size()==0)
		return;


	// Update the vertexBufferHard (if possible).
	// \toto yoyo: TODO_OPTIMIZE: allocate only what is needed for the current Lod (Max of all instances, like
	// the loading....) (see loadHeader()).
	updateVertexBufferHard(drv, _VBufferFinal.getNumVertices());
	/* currentVBHard is NULL if must disable it temporarily
		For now, never disable it, but switch of VBHard may be VERY EXPENSIVE if NV_vertex_array_range2 is not
		supported (old drivers).
	*/
	IVertexBufferHard		*currentVBHard= _VBHard;


	// get the skeleton model to which I am binded (else NULL).
	CSkeletonModel *skeleton;
	skeleton = mi->getSkeletonModel();
	// The mesh must not be skinned for render()
	nlassert(!(_Skinned && mi->isSkinned() && skeleton));
	bool bMorphApplied = _MeshMorpher.BlendShapes.size() > 0;
	bool useTangentSpace = _MeshVertexProgram && _MeshVertexProgram->needTangentSpace();


	// Profiling
	//===========
	H_AUTO( NL3D_MeshMRMGeom_RenderNormal );


	// Morphing
	// ========
	if (bMorphApplied)
	{
		// If _Skinned (NB: the skin is not applied) and if lod.OriginalSkinRestored, then restoreOriginalSkinPart is
		// not called but mush morpher write changed vertices into VBHard so its ok. The unchanged vertices
		// are written in the preceding call to restoreOriginalSkinPart.
		if (_Skinned)
		{
			_MeshMorpher.initSkinned(&_VBufferOriginal,
								 &_VBufferFinal,
								 currentVBHard,
								 useTangentSpace,
								 &_OriginalSkinVertices,
								 &_OriginalSkinNormals,
								 useTangentSpace ? &_OriginalTGSpace : NULL,
								 false );
			_MeshMorpher.updateSkinned (mi->getBlendShapeFactors());
		}
		else // Not even skinned so we have to do all the stuff
		{
			_MeshMorpher.init(&_VBufferOriginal,
								 &_VBufferFinal,
								 currentVBHard,
								 useTangentSpace);
			_MeshMorpher.update (mi->getBlendShapeFactors());
		}
	}

	// Skinning.
	//===========
	// if mesh is skinned (but here skin not applied), we must copy vertices/normals from original vertices.
	if (_Skinned)
	{
		// do it for this Lod only, and if cache say it is necessary.
		if (!lod.OriginalSkinRestored)
			restoreOriginalSkinPart(lod, currentVBHard);
	}


	// set the instance worldmatrix.
	drv->setupModelMatrix(trans->getWorldMatrix());


	// Geomorph.
	//===========
	// Geomorph the choosen Lod (if not the coarser mesh).
	if(numLod>0)
	{
		applyGeomorph(lod.Geomorphs, alphaLod, currentVBHard);
	}


	// force normalisation of normals..
	bool	bkupNorm= drv->isForceNormalize();
	drv->forceNormalize(true);			


	// Setup meshVertexProgram
	//===========

	// use MeshVertexProgram effect?
	bool	useMeshVP= _MeshVertexProgram != NULL;
	if( useMeshVP )
	{
		CMatrix		invertedObjectMatrix;
		invertedObjectMatrix = trans->getWorldMatrix().inverted();
		// really ok if success to begin VP
		useMeshVP= _MeshVertexProgram->begin(drv, mi->getOwnerScene(), mi, invertedObjectMatrix, renderTrav->CamPos);
	}
	

	// Render the lod.
	//===========
	// active VB.
	if(currentVBHard)
		drv->activeVertexBufferHard(currentVBHard);
	else
		drv->activeVertexBuffer(_VBufferFinal);


	// Global alpha used ?
	uint32	globalAlphaUsed= rdrFlags & IMeshGeom::RenderGlobalAlpha;
	uint8	globalAlphaInt=(uint8)NLMISC::OptFastFloor(globalAlpha*255);

	// Render all pass.
	if (globalAlphaUsed)
	{
		bool	gaDisableZWrite= (rdrFlags & IMeshGeom::RenderGADisableZWrite)?true:false;

		// for all passes
		for(uint i=0;i<lod.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= lod.RdrPass[i];

			if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
				 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) ) )
			{
				// CMaterial Ref
				CMaterial &material=mi->Materials[rdrPass.MaterialId];

				// Use a MeshBlender to modify material and driver.
				CMeshBlender	blender;
				blender.prepareRenderForGlobalAlpha(material, drv, globalAlpha, globalAlphaInt, gaDisableZWrite);

				// Setup VP material
				if (useMeshVP)
				{
					if(currentVBHard)
						_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, currentVBHard);
					else
						_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, &_VBufferFinal);
				}

				// Render
				drv->render(rdrPass.PBlock, material);

				// Resetup material/driver
				blender.restoreRender(material, drv, gaDisableZWrite);
			}
		}
	}
	else
	{
		for(uint i=0;i<lod.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= lod.RdrPass[i];

			if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
				 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) ) )
			{
				// CMaterial Ref
				CMaterial &material=mi->Materials[rdrPass.MaterialId];

				// Setup VP material
				if (useMeshVP)
				{
					if(currentVBHard)
						_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, currentVBHard);
					else
						_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, &_VBufferFinal);
				}	

				// Render with the Materials of the MeshInstance.
				drv->render(rdrPass.PBlock, material);
			}
		}
	}


	// End VertexProgram effect
	if(useMeshVP)
	{
		// end it.
		_MeshVertexProgram->end(drv);
	}


	// bkup force normalisation.
	drv->forceNormalize(bkupNorm);

}


// ***************************************************************************
void	CMeshMRMGeom::renderSkin(CTransformShape *trans, float alphaMRM)
{
	if(_Lods.size()==0)
		return;


	// get the meshMRM instance. only CMeshMRMInstance is possible when skinned (not MultiLod)
	CMeshMRMInstance	*mi= safe_cast<CMeshMRMInstance*>(trans);
	// get a ptr on scene
	CScene				*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav			*renderTrav= &ownerScene->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= renderTrav->getDriver();
	nlassert(drv);


	// choose the lod.
	float	alphaLod;
	sint	numLod= chooseLod(alphaMRM, alphaLod);


	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];
	if(lod.RdrPass.size()==0)
		return;


	/*
		YOYO: renderSkin() no more support vertexBufferHard()!!! for AGP Memory optimisation concern.
		AGP Skin rendering is made when supportSkinGrouping() is true
		Hence if a skin is to be rendered here, because it doesn't have a good vertex format, or it has
		MeshVertexProgram etc..., it will be rendered WITHOUT VBHard => slower.
	*/


	// get the skeleton model to which I am skinned
	CSkeletonModel *skeleton;
	skeleton = mi->getSkeletonModel();
	// must be skinned for renderSkin()
	nlassert(_Skinned && mi->isSkinned() && skeleton);
	bool bMorphApplied = _MeshMorpher.BlendShapes.size() > 0;
	bool useNormal= (_VBufferFinal.getVertexFormat() & CVertexBuffer::NormalFlag)!=0;
	bool useTangentSpace = _MeshVertexProgram && _MeshVertexProgram->needTangentSpace();


	// Profiling
	//===========
	H_AUTO_USE( NL3D_MeshMRMGeom_RenderSkinned );


	// Morphing
	// ========
	if (bMorphApplied)
	{
		// Since Skinned we must update original skin vertices and normals because skinning use it
		_MeshMorpher.initSkinned(&_VBufferOriginal,
							 &_VBufferFinal,
							 NULL,
							 useTangentSpace,
							 &_OriginalSkinVertices,
							 &_OriginalSkinNormals,
							 useTangentSpace ? &_OriginalTGSpace : NULL,
							 true );
		_MeshMorpher.updateSkinned (mi->getBlendShapeFactors());
	}

	// Skinning.
	//===========

	// Never use RawSkin. Actually used in skinGrouping.
	updateRawSkinNormal(false, mi, numLod);

	// applySkin.
	//--------

	// If skin without normal (rare/usefull?) always simple (slow) case.
	if(!useNormal)
	{
		// skinning with just position
		applySkin (lod, skeleton);
	}
	else
	{
		// Use SSE when possible
		if( CSystemInfo::hasSSE() )
		{
			// apply skin for this Lod only.
			if (!useTangentSpace)
			{
				// skinning with normal, but no tangent space
				applySkinWithNormalSSE (lod, skeleton);
			}
			else
			{
				// Tangent space stored in the last texture coordinate
				applySkinWithTangentSpaceSSE(lod, skeleton, _VBufferFinal.getNumTexCoordUsed() - 1);
			}
		}
		// Standard FPU skinning
		else
		{
			// apply skin for this Lod only.
			if (!useTangentSpace)
			{
				// skinning with normal, but no tangent space
				applySkinWithNormal (lod, skeleton);
			}
			else
			{
				// Tangent space stored in the last texture coordinate
				applySkinWithTangentSpace(lod, skeleton, _VBufferFinal.getNumTexCoordUsed() - 1);
			}
		}
	}

	// endSkin.
	//--------
	// dirt this lod part. (NB: this is not optimal, but sufficient :) ).
	lod.OriginalSkinRestored= false;


	// NB: the skeleton matrix has already been setuped by CSkeletonModel
	// NB: the normalize flag has already been setuped by CSkeletonModel


	// Geomorph.
	//===========
	// Geomorph the choosen Lod (if not the coarser mesh).
	if(numLod>0)
	{
		applyGeomorph(lod.Geomorphs, alphaLod, NULL);
	}


	// Setup meshVertexProgram
	//===========

	// use MeshVertexProgram effect?
	bool	useMeshVP= _MeshVertexProgram != NULL;
	if( useMeshVP )
	{
		CMatrix		invertedObjectMatrix;
		invertedObjectMatrix = skeleton->getWorldMatrix().inverted();
		// really ok if success to begin VP
		useMeshVP= _MeshVertexProgram->begin(drv, mi->getOwnerScene(), mi, invertedObjectMatrix, renderTrav->CamPos);
	}
	

	// Render the lod.
	//===========
	// active VB.
	drv->activeVertexBuffer(_VBufferFinal);


	// Render all pass.
	for(uint i=0;i<lod.RdrPass.size();i++)
	{
		CRdrPass	&rdrPass= lod.RdrPass[i];

		// CMaterial Ref
		CMaterial &material=mi->Materials[rdrPass.MaterialId];

		// Setup VP material
		if (useMeshVP)
		{
			_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, &_VBufferFinal);
		}

		// Render with the Materials of the MeshInstance.
		drv->render(rdrPass.PBlock, material);
	}

	// End VertexProgram effect
	if(useMeshVP)
	{
		// end it.
		_MeshVertexProgram->end(drv);
	}
}


// ***************************************************************************
bool	CMeshMRMGeom::supportSkinGrouping() const
{
	return _SupportSkinGrouping;
}

// ***************************************************************************
sint	CMeshMRMGeom::renderSkinGroupGeom(CMeshMRMInstance	*mi, float alphaMRM, uint remainingVertices, uint8 *vbDest)
{
	// get a ptr on scene
	CScene				*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav			*renderTrav= &ownerScene->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= renderTrav->getDriver();
	nlassert(drv);


	// choose the lod.
	float	alphaLod;
	sint	numLod= chooseLod(alphaMRM, alphaLod);
	_LastLodComputed= numLod;


	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];
	if(lod.RdrPass.size()==0)
		// return no vertices added
		return 0;

	// If the Lod is too big to render in the VBufferHard
	if(lod.NWedges>remainingVertices)
		// return Failure
		return -1;

	// get the skeleton model to which I am skinned
	CSkeletonModel *skeleton;
	skeleton = mi->getSkeletonModel();
	// must be skinned for renderSkin()
	nlassert(_Skinned && mi->isSkinned() && skeleton);
	bool bMorphApplied = _MeshMorpher.BlendShapes.size() > 0;
	bool useNormal= (_VBufferFinal.getVertexFormat() & CVertexBuffer::NormalFlag)!=0;
	nlassert(useNormal);


	// Profiling
	//===========
	H_AUTO_USE( NL3D_MeshMRMGeom_RenderSkinned );


	// Morphing
	// ========
	if (bMorphApplied)
	{
		// Since Skinned we must update original skin vertices and normals because skinning use it
		// NB: no need to setup vertexBufferHard, because not update in SkinningMode, but if the skin is not applied,
		// which is not the case here
		_MeshMorpher.initSkinned(&_VBufferOriginal,
							 &_VBufferFinal,
							 NULL,
							 false,
							 &_OriginalSkinVertices,
							 &_OriginalSkinNormals,
							 NULL,
							 true );
		_MeshMorpher.updateSkinned (mi->getBlendShapeFactors());
	}

	// Skinning.
	//===========

	// Use RawSkin if possible: only if no morph, and only Vertex/Normal
	updateRawSkinNormal(!bMorphApplied, mi, numLod);

	// NB: the skeleton matrix has already been setuped by CSkeletonModel
	// NB: the normalize flag has already been setuped by CSkeletonModel

	// dirt this lod part. (NB: this is not optimal, but sufficient :) ).
	lod.OriginalSkinRestored= false;

	// applySkin with RawSkin.
	//--------
	if(mi->_RawSkinCache)
	{
		// RawSkin do all the job in optimized way: Skinning, copy to VBHard and Geomorph.

		// Use SSE when possible
		if( CSystemInfo::hasSSE() )
		{
			// skinning with normal, but no tangent space
			applyRawSkinWithNormalSSE(lod, *(mi->_RawSkinCache), skeleton, vbDest, alphaLod);
		}
		// Standard FPU skinning
		else
		{
			// skinning with normal, but no tangent space
			applyRawSkinWithNormal (lod, *(mi->_RawSkinCache), skeleton, vbDest, alphaLod);
		}

		// Vertices are packed in RawSkin mode (ie no holes due to MRM!)
		return	mi->_RawSkinCache->Geomorphs.size() + 
				mi->_RawSkinCache->TotalSoftVertices + 
				mi->_RawSkinCache->TotalHardVertices;
	}
	// applySkin STD.
	//--------
	else
	{
		// Use SSE when possible
		if( CSystemInfo::hasSSE() )
		{
			// skinning with normal, but no tangent space
			applySkinWithNormalSSE (lod, skeleton);
		}
		// Standard FPU skinning
		else
		{
			// skinning with normal, but no tangent space
			applySkinWithNormal (lod, skeleton);
		}

		// endSkin.
		//--------
		// Fill the usefull AGP memory (if any one loaded/Used).
		fillAGPSkinPartWithVBHardPtr(lod, vbDest);

		// Geomorph.
		//===========
		// Geomorph the choosen Lod (if not the coarser mesh).
		if(numLod>0)
		{
			applyGeomorphWithVBHardPtr(lod.Geomorphs, alphaLod, vbDest);
		}

		// How many vertices are added to the VBuffer ???
		return lod.NWedges;
	}
}

// ***************************************************************************
void	CMeshMRMGeom::renderSkinGroupPrimitives(CMeshMRMInstance	*mi, uint baseVertex, std::vector<CSkinSpecularRdrPass> &specularRdrPasses, uint skinIndex)
{
	// get a ptr on scene
	CScene				*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav			*renderTrav= &ownerScene->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= renderTrav->getDriver();
	nlassert(drv);

	// Get the lod choosen in renderSkinGroupGeom()
	CLod	&lod= _Lods[_LastLodComputed];


	// Profiling
	//===========
	H_AUTO_USE( NL3D_MeshMRMGeom_RenderSkinned );

	// must update primitive cache
	updateShiftedTriangleCache(mi, _LastLodComputed, baseVertex);
	nlassert(mi->_ShiftedTriangleCache);


	// Render Triangles with cache
	//===========
	for(uint i=0;i<lod.RdrPass.size();i++)
	{
		CRdrPass	&rdrPass= lod.RdrPass[i];

		// CMaterial Ref
		CMaterial &material=mi->Materials[rdrPass.MaterialId];

		// TestYoyo. Material Speed Test
		/*if( material.getDiffuse()!=CRGBA(250, 251, 252) )
		{
			material.setDiffuse(CRGBA(250, 251, 252));
			// Set all texture the same.
			static CSmartPtr<ITexture>	pTexFile= new CTextureFile("fy_hom_visage_c1_fy_e1.tga");
			material.setTexture(0, pTexFile );
			// Remove Specular.
			if(material.getShader()==CMaterial::Specular)
			{
				CSmartPtr<ITexture>	tex= material.getTexture(0);
				material.setShader(CMaterial::Normal);
				material.setTexture(0, tex );
			}
			// Remove MakeUp
			material.setTexture(1, NULL);
		}*/

		// If the material is a specular material, don't render it now!
		if(material.getShader()==CMaterial::Specular)
		{
			// Add it to the rdrPass to sort!
			CSkinSpecularRdrPass	specRdrPass;
			specRdrPass.SkinIndex= skinIndex;
			specRdrPass.RdrPassIndex= i;
			// Get the handle of the specular Map as the sort Key
			ITexture	*specTex= material.getTexture(1);
			if(!specTex)
				specRdrPass.SpecId= 0;
			else
				specRdrPass.SpecId= drv->getTextureHandle( *specTex );
			// Append it to the list
			specularRdrPasses.push_back(specRdrPass);
		}
		else
		{
			// Get the shifted triangles.
			CShiftedTriangleCache::CRdrPass		&shiftedRdrPass= mi->_ShiftedTriangleCache->RdrPass[i];

			// This speed up 4 ms for 80K polys.
			uint	memToCache= shiftedRdrPass.NumTriangles*12;
			memToCache= min(memToCache, 4096U);
			CFastMem::precache(shiftedRdrPass.Triangles, memToCache);

			// Render with the Materials of the MeshInstance.
			drv->renderTriangles(material, shiftedRdrPass.Triangles, shiftedRdrPass.NumTriangles);
		}
	}
}


// ***************************************************************************
void	CMeshMRMGeom::renderSkinGroupSpecularRdrPass(CMeshMRMInstance	*mi, uint rdrPassId)
{
	// get a ptr on scene
	CScene				*ownerScene= mi->getOwnerScene();
	// get a ptr on renderTrav
	CRenderTrav			*renderTrav= &ownerScene->getRenderTrav();
	// get a ptr on the driver
	IDriver				*drv= renderTrav->getDriver();
	nlassert(drv);

	// Get the lod choosen in renderSkinGroupGeom()
	CLod	&lod= _Lods[_LastLodComputed];


	// Profiling
	//===========
	H_AUTO_USE( NL3D_MeshMRMGeom_RenderSkinned );

	// _ShiftedTriangleCache must have been computed in renderSkinGroupPrimitives
	nlassert(mi->_ShiftedTriangleCache);


	// Render Triangles with cache
	//===========
	CRdrPass	&rdrPass= lod.RdrPass[rdrPassId];

	// CMaterial Ref
	CMaterial &material=mi->Materials[rdrPass.MaterialId];

	// Get the shifted triangles.
	CShiftedTriangleCache::CRdrPass		&shiftedRdrPass= mi->_ShiftedTriangleCache->RdrPass[rdrPassId];

	// This speed up 4 ms for 80K polys.
	uint	memToCache= shiftedRdrPass.NumTriangles*12;
	memToCache= min(memToCache, 4096U);
	CFastMem::precache(shiftedRdrPass.Triangles, memToCache);

	// Render with the Materials of the MeshInstance.
	drv->renderTriangles(material, shiftedRdrPass.Triangles, shiftedRdrPass.NumTriangles);
}


// ***************************************************************************
void	CMeshMRMGeom::updateShiftedTriangleCache(CMeshMRMInstance *mi, sint curLodId, uint baseVertex)
{
	// if the instance has a cache, but not sync to us, delete it.
	if( mi->_ShiftedTriangleCache && (
		mi->_ShiftedTriangleCache->MeshDataId != _MeshDataId ||
		mi->_ShiftedTriangleCache->LodId != curLodId ||
		mi->_ShiftedTriangleCache->BaseVertex != baseVertex) )
	{
		mi->clearShiftedTriangleCache();
	}

	// If the instance has not a valid cache, must create it.
	if( !mi->_ShiftedTriangleCache )
	{
		mi->_ShiftedTriangleCache= new CShiftedTriangleCache;
		// Fill the cache Key.
		mi->_ShiftedTriangleCache->MeshDataId= _MeshDataId;
		mi->_ShiftedTriangleCache->LodId= curLodId;
		mi->_ShiftedTriangleCache->BaseVertex= baseVertex;

		// Build list of PBlock. From Lod, or from RawSkin cache.
		static	vector<CPrimitiveBlock*>	pbList;
		pbList.clear();
		if(mi->_RawSkinCache)
		{
			pbList.resize(mi->_RawSkinCache->RdrPass.size());
			for(uint i=0;i<pbList.size();i++)
			{
				pbList[i]= &mi->_RawSkinCache->RdrPass[i];
			}
		}
		else
		{
			CLod	&lod= _Lods[curLodId];
			pbList.resize(lod.RdrPass.size());
			for(uint i=0;i<pbList.size();i++)
			{
				pbList[i]= &lod.RdrPass[i].PBlock;
			}
		}

		// Build RdrPass
		mi->_ShiftedTriangleCache->RdrPass.resize(pbList.size());

		// First pass, count number of triangles, and fill header info
		uint	totalTri= 0;
		uint	i;
		for(i=0;i<pbList.size();i++)
		{
			mi->_ShiftedTriangleCache->RdrPass[i].NumTriangles= pbList[i]->getNumTri();
			totalTri+= pbList[i]->getNumTri();
		}

		// Allocate triangles indices.
		mi->_ShiftedTriangleCache->RawIndices.resize(totalTri*3);
		uint32		*rawPtr= mi->_ShiftedTriangleCache->RawIndices.getPtr();

		// Second pass, fill ptrs, and fill Arrays
		uint	indexTri= 0;
		for(i=0;i<pbList.size();i++)
		{
			CShiftedTriangleCache::CRdrPass	&dstRdrPass= mi->_ShiftedTriangleCache->RdrPass[i];
			dstRdrPass.Triangles= rawPtr + indexTri*3;

			// Fill the array
			uint	numTris= pbList[i]->getNumTri();
			if(numTris)
			{
				uint	nIds= numTris*3;
				// index, and fill
				uint32	*pSrcTri= pbList[i]->getTriPointer();
				uint32	*pDstTri= dstRdrPass.Triangles;
				for(;nIds>0;nIds--,pSrcTri++,pDstTri++)
					*pDstTri= *pSrcTri + baseVertex;
			}

			// Next
			indexTri+= dstRdrPass.NumTriangles;
		}
	}
}


// ***************************************************************************
void	CMeshMRMGeom::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// because of complexity, serial is separated in save / load.

	if(f.isReading())
		load(f);
	else
		save(f);

}



// ***************************************************************************
sint	CMeshMRMGeom::loadHeader(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 5:
		- Shadow Skinning
	Version 4:
		- serial SkinWeights per MRM, not per Lod
	Version 3:
		- Bones names.
	Version 2:
		- Mesh Vertex Program.
	Version 1:
		- added blend shapes
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(5);


	// if >= version 3, serial boens names
	if(ver>=3)
	{
		f.serialCont (_BonesName);
	}

	// Version3-: Bones index are in skeleton model id list
	_BoneIdComputed = (ver < 3);
	// Must always recompute usage of parents of bones used.
	_BoneIdExtended = false;

	// Mesh Vertex Program.
	if (ver >= 2)
	{
		IMeshVertexProgram	*mvp= NULL;
		f.serialPolyPtr(mvp);
		_MeshVertexProgram= mvp;
	}
	else
	{
		// release vp
		_MeshVertexProgram= NULL;
	}

	// blend shapes
	if (ver >= 1)
		f.serial (_MeshMorpher);

	// serial Basic info.
	// ==================
	f.serial(_Skinned);
	f.serial(_BBox);
	f.serial(_LevelDetail.MaxFaceUsed);
	f.serial(_LevelDetail.MinFaceUsed);
	f.serial(_LevelDetail.DistanceFinest);
	f.serial(_LevelDetail.DistanceMiddle);
	f.serial(_LevelDetail.DistanceCoarsest);
	f.serial(_LevelDetail.OODistanceDelta);
	f.serial(_LevelDetail.DistancePow);
	// preload the Lods.
	f.serialCont(_LodInfos);

	// read/save number of wedges.
	/* NB: prepare memory space too for vertices.
		\todo yoyo: TODO_OPTIMIZE. for now there is no Lod memory profit with vertices / skinWeights.
		But resizing arrays is a problem because of reallocation...
	*/
	uint32	nWedges;
	f.serial(nWedges);
	// Prepare the VBuffer.
	_VBufferFinal.serialHeader(f);
	// If skinned, must allocate skinWeights.
	contReset(_SkinWeights);
	if(_Skinned)
	{
		_SkinWeights.resize(nWedges);
	}


	// If new version, serial SkinWeights in header, not in lods.
	if(ver >= 4)
	{
		f.serialCont(_SkinWeights);
	}


	// if >= version 5, serial Shadow Skin Information
	if(ver>=5)
	{
		f.serialCont (_ShadowSkinVertices);
		f.serialCont (_ShadowSkinTriangles);
	}


	// Serial lod offsets.
	// ==================
	// This is the reference pos, to load / save relative offsets.
	sint32			startPos = f.getPos();
	// Those are the lodOffsets, relative to startPos.
	vector<sint32>	lodOffsets;
	lodOffsets.resize(_LodInfos.size(), 0);

	// read all relative offsets, and build the absolute offset of LodInfos.
	for(uint i=0;i<_LodInfos.size(); i++)
	{
		f.serial(lodOffsets[i]);
		_LodInfos[i].LodOffset= startPos + lodOffsets[i];
	}


	// resest the Lod arrays. NB: each Lod is empty, and ready to receive Lod data.
	// ==================
	contReset(_Lods);
	_Lods.resize(_LodInfos.size());

	// Flag the fact that no lod is loaded for now.
	_NbLodLoaded= 0;

	// Inform that the mesh data has changed
	dirtMeshDataId();


	// Some runtime not serialized compilation
	compileRunTime();

	// return version of the header
	return ver;
}


// ***************************************************************************
void	CMeshMRMGeom::load(NLMISC::IStream &f) throw(NLMISC::EStream)
{

	// because loading, flag the VertexBufferHard.
	_VertexBufferHardDirty= true;


	// Load the header of the stream.
	// ==================
	sint	verHeader= loadHeader(f);

	// Read All lod subsets.
	// ==================
	for(uint i=0;i<_LodInfos.size(); i++)
	{
		// read the lod face data.
		f.serial(_Lods[i]);
		// read the lod vertex data.
		serialLodVertexData(f, _LodInfos[i].StartAddWedge, _LodInfos[i].EndAddWedges);
		// if reading, must bkup all original vertices from VB.
		// this is done in serialLodVertexData(). by subset
	}

	// Now, all lods are loaded.
	_NbLodLoaded= _Lods.size();

	// If version doen't have boneNames, must build BoneId now.
	if(verHeader <= 2)
	{
		buildBoneUsageVer2 ();
	}
}


// ***************************************************************************
void	CMeshMRMGeom::save(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 5:
		- Shadow Skinning
	Version 4:
		- serial SkinWeights per MRM, not per Lod
	Version 3:
		- Bones names.
	Version 2:
		- Mesh Vertex Program.
	Version 1:
		- added blend shapes
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(5);
	uint	i;

	// if >= version 3, serial bones names
	f.serialCont (_BonesName);

	// Warning, if you have skinned this shape, you can't write it anymore because skinning id have been changed! 
	nlassert (_BoneIdComputed==false);

	// Mesh Vertex Program.
	if (ver >= 2)
	{
		IMeshVertexProgram	*mvp= NULL;
		mvp= _MeshVertexProgram;
		f.serialPolyPtr(mvp);
	}

	// blend shapes
	if (ver >= 1)
		f.serial (_MeshMorpher);

	// must have good original Skinned Vertex before writing.
	if( _Skinned )
	{
		restoreOriginalSkinVertices();
	}


	// serial Basic info.
	// ==================
	f.serial(_Skinned);
	f.serial(_BBox);
	f.serial(_LevelDetail.MaxFaceUsed);
	f.serial(_LevelDetail.MinFaceUsed);
	f.serial(_LevelDetail.DistanceFinest);
	f.serial(_LevelDetail.DistanceMiddle);
	f.serial(_LevelDetail.DistanceCoarsest);
	f.serial(_LevelDetail.OODistanceDelta);
	f.serial(_LevelDetail.DistancePow);
	f.serialCont(_LodInfos);

	// save number of wedges.
	uint32	nWedges;
	nWedges= _VBufferFinal.getNumVertices();
	f.serial(nWedges);
	// Save the VBuffer header.
	_VBufferFinal.serialHeader(f);


	// If new version, serial SkinWeights in header, not in lods.
	if(ver >= 4)
	{
		f.serialCont(_SkinWeights);
	}

	// if >= version 5, serial Shadow Skin Information
	if(ver>=5)
	{
		f.serialCont (_ShadowSkinVertices);
		f.serialCont (_ShadowSkinTriangles);
	}

	// Serial lod offsets.
	// ==================
	// This is the reference pos, to load / save relative offsets.
	sint32			startPos = f.getPos();
	// Those are the lodOffsets, relative to startPos.
	vector<sint32>	lodOffsets;
	lodOffsets.resize(_LodInfos.size(), 0);

	// write all dummy offset. For now (since we don't know what to set), compute the offset of 
	// the sint32 to come back in serial lod parts below.
	for(i=0;i<_LodInfos.size(); i++)
	{
		lodOffsets[i]= f.getPos();
		f.serial(lodOffsets[i]);
	}

	// Serial lod subsets.
	// ==================

	// Save all the lods.
	for(i=0;i<_LodInfos.size(); i++)
	{
		// get current absolute position.
		sint32	absCurPos= f.getPos();

		// come back to "relative lodOffset" absolute position in the stream. (temp stored in lodOffset[i]).
		f.seek(lodOffsets[i], IStream::begin);

		// write the relative position of the lod to the stream.
		sint32	relCurPos= absCurPos - startPos;
		f.serial(relCurPos);

		// come back to absCurPos, to save the lod.
		f.seek(absCurPos, IStream::begin);

		// And so now, save the lod.
		// write the lod face data.
		f.serial(_Lods[i]);
		// write the lod vertex data.
		serialLodVertexData(f, _LodInfos[i].StartAddWedge, _LodInfos[i].EndAddWedges);
	}


}



// ***************************************************************************
void	CMeshMRMGeom::serialLodVertexData(NLMISC::IStream &f, uint startWedge, uint endWedge)
{
	/*
	Version 1:
		- serial SkinWeights per MRM, not per Lod
	*/
	sint	ver= f.serialVersion(1);

	// VBuffer part.
	_VBufferFinal.serialSubset(f, startWedge, endWedge);

	// SkinWeights.
	if(_Skinned)
	{
		// Serialize SkinWeight per lod only for old versions.
		if(ver<1)
		{
			for(uint i= startWedge; i<endWedge; i++)
			{
				f.serial(_SkinWeights[i]);
			}
		}
		// if reading, must copy original vertices from VB.
		if( f.isReading())
		{
			bkupOriginalSkinVerticesSubset(startWedge, endWedge);
		}
	}
}



// ***************************************************************************
void	CMeshMRMGeom::loadFirstLod(NLMISC::IStream &f)
{

	// because loading, flag the VertexBufferHard.
	_VertexBufferHardDirty= true;

	// Load the header of the stream.
	// ==================
	sint	verHeader= loadHeader(f);


	// If empty MRM, quit.
	if(_LodInfos.size()==0)
		return;

	/* If the version is <4, then SkinWeights are serialised per Lod.
		But for computebonesId(), we must have all SkinWeights RIGHT NOW.
		Hence, if too old version (<4), serialize all the MRM....
	*/
	uint	numLodToLoad;
	if(verHeader<4)
		numLodToLoad= _LodInfos.size();
	else
		numLodToLoad= 1;


	// Read lod subset(s).
	// ==================
	for(uint i=0;i<numLodToLoad; i++)
	{
		// read the lod face data.
		f.serial(_Lods[i]);
		// read the lod vertex data.
		serialLodVertexData(f, _LodInfos[i].StartAddWedge, _LodInfos[i].EndAddWedges);
		// if reading, must bkup all original vertices from VB.
		// this is done in serialLodVertexData(). by subset
	}

	// Now, just first lod is loaded (but if too old file)
	_NbLodLoaded= numLodToLoad;

	// If version doen't have boneNames, must build BoneId now.
	if(verHeader <= 2)
	{
		buildBoneUsageVer2 ();
	}
}


// ***************************************************************************
void	CMeshMRMGeom::loadNextLod(NLMISC::IStream &f)
{

	// because loading, flag the VertexBufferHard.
	_VertexBufferHardDirty= true;

	// If all is loaded, quit.
	if(getNbLodLoaded() == getNbLod())
		return;

	// Set pos to good lod.
	f.seek(_LodInfos[_NbLodLoaded].LodOffset, IStream::begin);

	// Serial this lod data.
	// read the lod face data.
	f.serial(_Lods[_NbLodLoaded]);
	// read the lod vertex data.
	serialLodVertexData(f, _LodInfos[_NbLodLoaded].StartAddWedge, _LodInfos[_NbLodLoaded].EndAddWedges);
	// if reading, must bkup all original vertices from VB.
	// this is done in serialLodVertexData(). by subset


	// Inc LodLoaded count.
	_NbLodLoaded++;
}


// ***************************************************************************
void	CMeshMRMGeom::unloadNextLod(NLMISC::IStream &f)
{
	// If just first lod remain (or no lod), quit
	if(getNbLodLoaded() <= 1)
		return;

	// Reset the entire Lod object. (Free Memory).
	contReset(_Lods[_NbLodLoaded-1]);


	// Dec LodLoaded count.
	_NbLodLoaded--;
}


// ***************************************************************************
void	CMeshMRMGeom::bkupOriginalSkinVertices()
{
	nlassert(_Skinned);

	// bkup the entire array.
	bkupOriginalSkinVerticesSubset(0, _VBufferFinal.getNumVertices());
}


// ***************************************************************************
void	CMeshMRMGeom::bkupOriginalSkinVerticesSubset(uint wedgeStart, uint wedgeEnd)
{
	nlassert(_Skinned);

	// Copy VBuffer content into Original vertices normals.
	if(_VBufferFinal.getVertexFormat() & CVertexBuffer::PositionFlag)
	{
		// copy vertices from VBuffer. (NB: unusefull geomorphed vertices are still copied, but doesn't matter).
		_OriginalSkinVertices.resize(_VBufferFinal.getNumVertices());
		for(uint i=wedgeStart; i<wedgeEnd;i++)
		{
			_OriginalSkinVertices[i]= *(CVector*)_VBufferFinal.getVertexCoordPointer(i);
		}
	}
	if(_VBufferFinal.getVertexFormat() & CVertexBuffer::NormalFlag)
	{
		// copy normals from VBuffer. (NB: unusefull geomorphed normals are still copied, but doesn't matter).
		_OriginalSkinNormals.resize(_VBufferFinal.getNumVertices());
		for(uint i=wedgeStart; i<wedgeEnd;i++)
		{
			_OriginalSkinNormals[i]= *(CVector*)_VBufferFinal.getNormalCoordPointer(i);
		}
	}

	// is there tangent space added ?
	if (_MeshVertexProgram && _MeshVertexProgram->needTangentSpace())
	{
		// yes, backup it
		nlassert(_VBufferFinal.getNumTexCoordUsed() > 0);
		uint tgSpaceStage = _VBufferFinal.getNumTexCoordUsed() - 1;
		_OriginalTGSpace.resize(_VBufferFinal.getNumVertices());
		for(uint i=wedgeStart; i<wedgeEnd;i++)
		{
			_OriginalTGSpace[i]= *(CVector*)_VBufferFinal.getTexCoordPointer(i, tgSpaceStage);
		}
	}
}


// ***************************************************************************
void	CMeshMRMGeom::restoreOriginalSkinVertices()
{
	nlassert(_Skinned);

	// Copy VBuffer content into Original vertices normals.
	if(_VBufferFinal.getVertexFormat() & CVertexBuffer::PositionFlag)
	{
		// copy vertices from VBuffer. (NB: unusefull geomorphed vertices are still copied, but doesn't matter).
		for(uint i=0; i<_VBufferFinal.getNumVertices();i++)
		{
			*(CVector*)_VBufferFinal.getVertexCoordPointer(i)= _OriginalSkinVertices[i];
		}
	}
	if(_VBufferFinal.getVertexFormat() & CVertexBuffer::NormalFlag)
	{
		// copy normals from VBuffer. (NB: unusefull geomorphed normals are still copied, but doesn't matter).
		for(uint i=0; i<_VBufferFinal.getNumVertices();i++)
		{
			*(CVector*)_VBufferFinal.getNormalCoordPointer(i)= _OriginalSkinNormals[i];
		}
	}
	if (_MeshVertexProgram && _MeshVertexProgram->needTangentSpace())
	{
		uint numTexCoords = _VBufferFinal.getNumTexCoordUsed();
		nlassert(numTexCoords >= 2);
		nlassert(_OriginalTGSpace.size() == _VBufferFinal.getNumVertices());
		// copy tangent space vectors
		for(uint i = 0; i < _VBufferFinal.getNumVertices(); ++i)
		{
			*(CVector*)_VBufferFinal.getTexCoordPointer(i, numTexCoords - 1)= _OriginalTGSpace[i];
		}
	}
}


// ***************************************************************************
void	CMeshMRMGeom::restoreOriginalSkinPart(CLod &lod, IVertexBufferHard *currentVBHard)
{
	nlassert(_Skinned);


	/*
		YOYO: _Skinned mrms no more support vertexBufferHard
		see note in renderSkin()
	*/

	// get vertexPtr / normalOff.
	//===========================
	uint8		*destVertexPtr= (uint8*)_VBufferFinal.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// must have XYZ.
	nlassert(flags & CVertexBuffer::PositionFlag);

	// Compute offset of each component of the VB.
	sint32		normalOff;
	if(flags & CVertexBuffer::NormalFlag)
		normalOff= _VBufferFinal.getNormalOff();
	else
		normalOff= 0;


	// compute src array.
	CVector				*srcVertexPtr;
	CVector				*srcNormalPtr= NULL;
	srcVertexPtr= &_OriginalSkinVertices[0];
	if(normalOff)
		srcNormalPtr= &(_OriginalSkinNormals[0]);


	// copy skinning.
	//===========================
	for(uint i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
	{
		uint		nInf= lod.InfluencedVertices[i].size();
		if( nInf==0 )
			continue;
		uint32		*infPtr= &(lod.InfluencedVertices[i][0]);

		//  for all InfluencedVertices only.
		for(;nInf>0;nInf--, infPtr++)
		{
			uint	index= *infPtr;
			CVector				*srcVertex= srcVertexPtr + index;
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			*dstVertex= *srcVertex;
			// Normal.
			if(normalOff)
				*dstNormal= *srcNormal;
		}
	}


	// clean this lod part. (NB: this is not optimal, but sufficient :) ).
	lod.OriginalSkinRestored= true;
}

// ***************************************************************************

float CMeshMRMGeom::getNumTriangles (float distance)
{
	// NB: this is an approximation, but this is continious.
	return _LevelDetail.getNumTriangles(distance);
}



// ***************************************************************************
void				CMeshMRMGeom::deleteVertexBufferHard()
{
	// test (refptr) if the object still exist in memory.
	if(_VBHard!=NULL)
	{
		// A vbufferhard should still exist only if driver still exist.
		nlassert(_Driver!=NULL);

		// delete it from driver.
		_Driver->deleteVertexBufferHard(_VBHard);
		_VBHard= NULL;
	}
}

// ***************************************************************************
void				CMeshMRMGeom::updateVertexBufferHard(IDriver *drv, uint32 numVertices)
{
	if(!drv->supportVertexBufferHard() || numVertices==0)
		return;


	/** If the mesh is skinned, still use normal CVertexBuffer.
	 *	Doens't use VBuffer hard when Skinned, for AGP memory optimisation. See renderSkin() note.
	 *	It's because most of the MRM skins are rendered through renderSkinGroup*() methods, which use a global VBHard
	 *	NB: meshs which are skinned but not skin applied are not optimized too. But this case is not a "realtime" game
	 *	situation
	 *
	 *	Also, if the driver has slow VBhard unlock()  (ie ATI gl extension), avoid use of them if MeshMorpher 
	 *	is used.
	 */
	bool	avoidVBHard;
	avoidVBHard= _Skinned || ( _MeshMorpher.BlendShapes.size()>0 && drv->slowUnlockVertexBufferHard() );
	if( _VertexBufferHardDirty && avoidVBHard )
	{
		// delete possible old VBHard.
		if(_VBHard!=NULL)
		{
			// VertexBufferHard lifetime < Driver lifetime.
			nlassert(_Driver!=NULL);
			_Driver->deleteVertexBufferHard(_VBHard);
		}
		return;
	}


	// If the vbufferhard is not here, or if dirty, or if do not have enough vertices.
	if(_VBHard==NULL || _VertexBufferHardDirty || _VBHard->getNumVertices() < numVertices)
	{
		_VertexBufferHardDirty= false;

		// delete possible old _VBHard.
		if(_VBHard!=NULL)
		{
			// VertexBufferHard lifetime < Driver lifetime.
			nlassert(_Driver!=NULL);
			_Driver->deleteVertexBufferHard(_VBHard);
		}

		// bkup drv in a refptr. (so we know if the vbuffer hard has to be deleted).
		_Driver= drv;
		// try to create new one, in AGP Ram
		_VBHard= _Driver->createVertexBufferHard(_VBufferFinal.getVertexFormat(), _VBufferFinal.getValueTypePointer (), numVertices, IDriver::VBHardAGP, _VBufferFinal.getUVRouting());


		// If KO, use normal VertexBuffer, else, Fill it with VertexBuffer.
		if(_VBHard!=NULL)
		{
			void	*vertexPtr= _VBHard->lock();

			nlassert(_VBufferFinal.getVertexFormat() == _VBHard->getVertexFormat());
			nlassert(_VBufferFinal.getNumVertices() >= numVertices);
			nlassert(_VBufferFinal.getVertexSize() == _VBHard->getVertexSize());

			// \todo yoyo: TODO_DX8 and DX8 ???
			// Because same internal format, just copy all block.
			memcpy(vertexPtr, _VBufferFinal.getVertexCoordPointer(), numVertices * _VBufferFinal.getVertexSize() );

			_VBHard->unlock();
		}
	}

}

// ***************************************************************************
void	CMeshMRMGeom::computeBonesId (CSkeletonModel *skeleton)
{
	// Already computed ?
	if (!_BoneIdComputed)
	{
		// Get a pointer on the skeleton
		nlassert (skeleton);
		if (skeleton)
		{
			// Resize boneId to the good size.
			_BonesId.resize(_BonesName.size());

			// Remap bones id table
			std::vector<uint> remap (_BonesName.size());

			// For each bones
			uint bone;
			for (bone=0; bone<remap.size(); bone++)
			{
				// Look for the bone
				sint32 boneId = skeleton->getBoneIdByName (_BonesName[bone]);

				// Setup the _BoneId.
				_BonesId[bone]= boneId;

				// Bones found ?
				if (boneId != -1)
				{
					// Set the bone id
					remap[bone] = (uint32)boneId;
				}
				else
				{
					// Put id 0
					remap[bone] = 0;

					// Error
					nlwarning ("Bone %s not found in the skeleton.", _BonesName[bone].c_str());
				}
			}

			// Remap the vertex
			uint vert;
			for (vert=0; vert<_SkinWeights.size(); vert++)
			{
				// For each weight
				uint weight;
				for (weight=0; weight<NL3D_MESH_SKINNING_MAX_MATRIX; weight++)
				{
					// Active ?
					if ((_SkinWeights[vert].Weights[weight]>0)||(weight==0))
					{
						// Check id
						nlassert (_SkinWeights[vert].MatrixId[weight] < remap.size());
						_SkinWeights[vert].MatrixId[weight] = remap[_SkinWeights[vert].MatrixId[weight]];
					}
				}				
			}

			// Remap the vertex influence by lods
			uint lod;
			for (lod=0; lod<_Lods.size(); lod++)
			{
				// For each matrix used
				uint matrix;
				for (matrix=0; matrix<_Lods[lod].MatrixInfluences.size(); matrix++)
				{
					// Check
					nlassert (_Lods[lod].MatrixInfluences[matrix]<remap.size());

					// Remap
					_Lods[lod].MatrixInfluences[matrix] = remap[_Lods[lod].MatrixInfluences[matrix]];
				}
			}

			// Remap Shadow Vertices.
			for(vert=0;vert<_ShadowSkinVertices.size();vert++)
			{
				CShadowVertex	&v= _ShadowSkinVertices[vert];
				// Check id
				nlassert (v.MatrixId < remap.size());
				v.MatrixId= remap[v.MatrixId];
			}

			// Computed
			_BoneIdComputed = true;
		}
	}

	// Already extended ?
	if (!_BoneIdExtended)
	{
		nlassert (skeleton);
		if (skeleton)
		{
			// the total bone Usage of the mesh.
			vector<bool>	boneUsage;
			boneUsage.resize(skeleton->Bones.size(), false);

			// for all Bones marked as valid.
			uint	i;
			for(i=0; i<_BonesId.size(); i++)
			{
				// if not a valid boneId, skip it.
				if(_BonesId[i]<0)
					continue;

				// mark him and his father in boneUsage.
				skeleton->flagBoneAndParents(_BonesId[i], boneUsage);
			}

			// fill _BonesIdExt with bones of _BonesId and their parents.
			_BonesIdExt.clear();
			for(i=0; i<boneUsage.size();i++)
			{
				// if the bone is used by the mesh, add it to BoneIdExt.
				if(boneUsage[i])
					_BonesIdExt.push_back(i);
			}

		}

		// Extended
		_BoneIdExtended= true;
	}

}


// ***************************************************************************
void	CMeshMRMGeom::buildBoneUsageVer2 ()
{
	if(_Skinned)
	{
		// parse all vertices, couting MaxBoneId used.
		uint32	maxBoneId= 0;
		// for each vertex
		uint vert;
		for (vert=0; vert<_SkinWeights.size(); vert++)
		{
			// For each weight
			for (uint weight=0; weight<NL3D_MESH_SKINNING_MAX_MATRIX; weight++)
			{
				// Active ?
				if ((_SkinWeights[vert].Weights[weight]>0)||(weight==0))
				{
					maxBoneId= max(_SkinWeights[vert].MatrixId[weight], maxBoneId);
				}
			}				
		}

		// alloc an array of maxBoneId+1, reset to 0.
		std::vector<uint8>		boneUsage;
		boneUsage.resize(maxBoneId+1, 0);

		// reparse all vertices, counting usage for each bone.
		for (vert=0; vert<_SkinWeights.size(); vert++)
		{
			// For each weight
			for (uint weight=0; weight<NL3D_MESH_SKINNING_MAX_MATRIX; weight++)
			{
				// Active ?
				if ((_SkinWeights[vert].Weights[weight]>0)||(weight==0))
				{
					// mark this bone as used.
					boneUsage[_SkinWeights[vert].MatrixId[weight]]= 1;
				}
			}				
		}

		// For each bone used
		_BonesId.clear();
		for(uint i=0; i<boneUsage.size();i++)
		{
			// if the bone is used by the mesh, add it to BoneId.
			if(boneUsage[i])
				_BonesId.push_back(i);
		}
	}
}


// ***************************************************************************
void	CMeshMRMGeom::updateSkeletonUsage(CSkeletonModel *sm, bool increment)
{
	// For all Bones used.
	for(uint i=0; i<_BonesIdExt.size();i++)
	{
		uint	boneId= _BonesIdExt[i];
		// Some explicit Error.
		if(boneId>=sm->Bones.size())
			nlerror(" Skin is incompatible with Skeleton: tries to use bone %d", boneId);
		// increment or decrement not Forced, because CMeshGeom use getActiveBoneSkinMatrix().
		if(increment)
			sm->incBoneUsage(boneId, CSkeletonModel::UsageNormal);
		else
			sm->decBoneUsage(boneId, CSkeletonModel::UsageNormal);
	}
}


// ***************************************************************************
void	CMeshMRMGeom::compileRunTime()
{
	_PreciseClipping= _BBox.getRadius() >= NL3D_MESH_PRECISE_CLIP_THRESHOLD;

	// Compute if can support SkinGrouping rendering
	if(_Lods.size()==0 || !_Skinned)
	{
		_SupportSkinGrouping= false;
		_SupportShadowSkinGrouping= false;
	}
	else
	{
		// The Mesh must follow those restrictions, to support group skinning
		_SupportSkinGrouping=
			_VBufferFinal.getVertexFormat() == NL3D_MESH_SKIN_MANAGER_VERTEXFORMAT &&
			_VBufferFinal.getNumVertices() < NL3D_MESH_SKIN_MANAGER_MAXVERTICES &&
			!_MeshVertexProgram;
		// Additionally, NONE of the RdrPass should have Quads or lines...
		for(uint i=0;i<_Lods.size();i++)
		{
			for(uint j=0;j<_Lods[i].RdrPass.size();j++)
			{
				CPrimitiveBlock		&pb= _Lods[i].RdrPass[j].PBlock;
				if( pb.getNumQuad() || pb.getNumLine() )
				{
					_SupportSkinGrouping= false;
					break;
				}
			}
		}

		// Support Shadow SkinGrouping if Shadow setuped, and if not too many vertices.
		_SupportShadowSkinGrouping= !_ShadowSkinVertices.empty() &&
			NL3D_SHADOW_MESH_SKIN_MANAGER_VERTEXFORMAT==CVertexBuffer::PositionFlag &&
			_ShadowSkinVertices.size() <= NL3D_SHADOW_MESH_SKIN_MANAGER_MAXVERTICES;
	}

	// Support MeshBlockRendering only if not skinned/meshMorphed.
	_SupportMeshBlockRendering= !_Skinned && _MeshMorpher.BlendShapes.size()==0;

	// \todo yoyo: support later MeshVertexProgram 
	_SupportMeshBlockRendering= _SupportMeshBlockRendering && _MeshVertexProgram==NULL;
}


// ***************************************************************************
void	CMeshMRMGeom::profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, float polygonCount, uint32 rdrFlags)
{
	// get the result of the Load Balancing.
	float	alphaMRM= _LevelDetail.getLevelDetailFromPolyCount(polygonCount);

	// choose the lod.
	float	alphaLod;
	sint	numLod= chooseLod(alphaMRM, alphaLod);

	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];

	// get the mesh instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);

	// Profile all pass.
	uint	triCount= 0;
	for (uint i=0;i<lod.RdrPass.size();i++)
	{
		CRdrPass	&rdrPass= lod.RdrPass[i];
		// Profile with the Materials of the MeshInstance.
		if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (rdrFlags & IMeshGeom::RenderOpaqueMaterial) ) ||
			 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (rdrFlags & IMeshGeom::RenderTransparentMaterial) ) )
		{
			triCount+= rdrPass.PBlock.getNumTri();
		}
	}

	// Profile
	if(triCount)
	{
		// tri per VBFormat
		rdrTrav->Scene->incrementProfileTriVBFormat(rdrTrav->Scene->BenchRes.MeshMRMProfileTriVBFormat, 
			_VBufferFinal.getVertexFormat(), triCount);

		// VBHard
		if(_VBHard)
			rdrTrav->Scene->BenchRes.NumMeshMRMVBufferHard++;
		else
			rdrTrav->Scene->BenchRes.NumMeshMRMVBufferStd++;

		// rendered in BlockRendering, only if not transparent pass (known it if RenderTransparentMaterial is set)
		if(supportMeshBlockRendering() && (rdrFlags & IMeshGeom::RenderTransparentMaterial)==0 )
		{
			if(isMeshInVBHeap())
			{
				rdrTrav->Scene->BenchRes.NumMeshMRMRdrBlockWithVBHeap++;
				rdrTrav->Scene->BenchRes.NumMeshMRMTriRdrBlockWithVBHeap+= triCount;
			}
			else
			{
				rdrTrav->Scene->BenchRes.NumMeshMRMRdrBlock++;
				rdrTrav->Scene->BenchRes.NumMeshMRMTriRdrBlock+= triCount;
			}
		}
		else
		{
			rdrTrav->Scene->BenchRes.NumMeshMRMRdrNormal++;
			rdrTrav->Scene->BenchRes.NumMeshMRMTriRdrNormal+= triCount;
		}
	}
}



// ***************************************************************************
// ***************************************************************************
// Mesh Block Render Interface
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMeshMRMGeom::supportMeshBlockRendering () const
{
	/*
		Yoyo: Don't Support It for MRM because too Slow!!
		The problem is that lock() unlock() on each instance, on the same VBHeap IS AS SLOWER AS
		VB switching.

		TODO_OPTIMIZE: find a way to optimize MRM.
	*/
	return false;
	//return _SupportMeshBlockRendering;
}

// ***************************************************************************
bool	CMeshMRMGeom::sortPerMaterial() const
{
	// Can't do it per material since 2 lods may not have the same number of RdrPass!!
	return false;
}
// ***************************************************************************
uint	CMeshMRMGeom::getNumRdrPassesForMesh() const 
{
	// not used...
	return 0;

}
// ***************************************************************************
uint	CMeshMRMGeom::getNumRdrPassesForInstance(CMeshBaseInstance *inst) const 
{
	return _Lods[_MBRCurrentLodId].RdrPass.size();
}
// ***************************************************************************
void	CMeshMRMGeom::beginMesh(CMeshGeomRenderContext &rdrCtx) 
{
	if(_Lods.empty())
		return;

	IDriver	*drv= rdrCtx.Driver;

	if(rdrCtx.RenderThroughVBHeap)
	{
		// Don't setup VB in this case, since use the VBHeap setuped one.
	}
	else
	{
		updateVertexBufferHard(drv, _VBufferFinal.getNumVertices());

		// active VB.
		if(_VBHard)
		{
			drv->activeVertexBufferHard(_VBHard);
		}
		else
		{
			drv->activeVertexBuffer(_VBufferFinal);
		}
	}


	// force normalisation of normals..
	_MBRBkupNormalize= drv->isForceNormalize();
	drv->forceNormalize(true);			

}
// ***************************************************************************
void	CMeshMRMGeom::activeInstance(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount, void *vbDst) 
{
	H_AUTO( NL3D_MeshMRMGeom_RenderNormal );

	if(_Lods.empty())
		return;

	// get the result of the Load Balancing.
	float	alphaMRM= _LevelDetail.getLevelDetailFromPolyCount(polygonCount);

	// choose the lod.
	float	alphaLod;
	_MBRCurrentLodId= chooseLod(alphaMRM, alphaLod);

	// Geomorph the choosen Lod (if not the coarser mesh).
	if(_MBRCurrentLodId>0)
	{
		if(rdrCtx.RenderThroughVBHeap)
			applyGeomorphWithVBHardPtr(_Lods[_MBRCurrentLodId].Geomorphs, alphaLod, (uint8*)vbDst);
		else
			applyGeomorph(_Lods[_MBRCurrentLodId].Geomorphs, alphaLod, _VBHard);
	}

	// set the instance worldmatrix.
	rdrCtx.Driver->setupModelMatrix(inst->getWorldMatrix());

	// setupLighting.
	inst->changeLightSetup(rdrCtx.RenderTrav);
}
// ***************************************************************************
void	CMeshMRMGeom::renderPass(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *mi, float polygonCount, uint rdrPassId) 
{
	if(_Lods.empty())
		return;

	CLod		&lod= _Lods[_MBRCurrentLodId];
	CRdrPass	&rdrPass= lod.RdrPass[rdrPassId];

	if ( mi->Materials[rdrPass.MaterialId].getBlend() == false )
	{
		// CMaterial Ref
		CMaterial &material=mi->Materials[rdrPass.MaterialId];

		// Render with the Materials of the MeshInstance.
		if(rdrCtx.RenderThroughVBHeap)
			// render shifted primitives
			rdrCtx.Driver->render(rdrPass.VBHeapPBlock, material);
		else
			rdrCtx.Driver->render(rdrPass.PBlock, material);
	}
}

// ***************************************************************************
void	CMeshMRMGeom::endMesh(CMeshGeomRenderContext &rdrCtx) 
{
	if(_Lods.empty())
		return;

	// bkup force normalisation.
	rdrCtx.Driver->forceNormalize(_MBRBkupNormalize);
}


// ***************************************************************************
bool	CMeshMRMGeom::getVBHeapInfo(uint &vertexFormat, uint &numVertices)
{
	// CMeshMRMGeom support VBHeap rendering, assuming _SupportMeshBlockRendering is true
	vertexFormat= _VBufferFinal.getVertexFormat();
	numVertices= _VBufferFinal.getNumVertices();
	return _SupportMeshBlockRendering;
}

// ***************************************************************************
void	CMeshMRMGeom::computeMeshVBHeap(void *dst, uint indexStart)
{
	// Fill dst with Buffer content.
	memcpy(dst, _VBufferFinal.getVertexCoordPointer(), _VBufferFinal.getNumVertices()*_VBufferFinal.getVertexSize() );

	// For All Lods
	for(uint lodId=0; lodId<_Lods.size();lodId++)
	{
		CLod	&lod= _Lods[lodId];

		// For all rdrPass.
		for(uint i=0;i<lod.RdrPass.size();i++)
		{
			// shift the PB
			CPrimitiveBlock	&srcPb= lod.RdrPass[i].PBlock;
			CPrimitiveBlock	&dstPb= lod.RdrPass[i].VBHeapPBlock;
			uint j;

			// Lines.
			dstPb.setNumLine(srcPb.getNumLine());
			uint32			*srcLinePtr= srcPb.getLinePointer();
			uint32			*dstLinePtr= dstPb.getLinePointer();
			for(j=0; j<dstPb.getNumLine()*2;j++)
			{
				dstLinePtr[j]= srcLinePtr[j]+indexStart;
			}
			// Tris.
			dstPb.setNumTri(srcPb.getNumTri());
			uint32			*srcTriPtr= srcPb.getTriPointer();
			uint32			*dstTriPtr= dstPb.getTriPointer();
			for(j=0; j<dstPb.getNumTri()*3;j++)
			{
				dstTriPtr[j]= srcTriPtr[j]+indexStart;
			}
			// Quads.
			dstPb.setNumQuad(srcPb.getNumQuad());
			uint32			*srcQuadPtr= srcPb.getQuadPointer();
			uint32			*dstQuadPtr= dstPb.getQuadPointer();
			for(j=0; j<dstPb.getNumQuad()*4;j++)
			{
				dstQuadPtr[j]= srcQuadPtr[j]+indexStart;
			}
		}
	}
}

// ***************************************************************************
bool	CMeshMRMGeom::isActiveInstanceNeedVBFill() const
{
	// Yes, need it for geomorph
	return true;
}


// ***************************************************************************
// ***************************************************************************
// CMeshMRM.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
CMeshMRM::CMeshMRM()
{
}
// ***************************************************************************
void			CMeshMRM::build (CMeshBase::CMeshBaseBuild &mBase, CMesh::CMeshBuild &m, 
								 std::vector<CMesh::CMeshBuild*> &listBS,
								 const CMRMParameters &params)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase (mBase);

	// Then build the geom.
	_MeshMRMGeom.build (m, listBS, mBase.Materials.size(), params);
}
// ***************************************************************************
void			CMeshMRM::build (CMeshBase::CMeshBaseBuild &m, const CMeshMRMGeom &mgeom)
{
	/// copy MeshBase info: materials ....
	CMeshBase::buildMeshBase(m);

	// Then copy the geom.
	_MeshMRMGeom= mgeom;
}


// ***************************************************************************
void			CMeshMRM::optimizeMaterialUsage(std::vector<sint> &remap)
{
	// For each material, count usage.
	vector<bool>	materialUsed;
	materialUsed.resize(CMeshBase::_Materials.size(), false);
	for(uint lod=0;lod<getNbLod();lod++)
	{
		for(uint rp=0;rp<getNbRdrPass(lod);rp++)
		{
			uint	matId= getRdrPassMaterial(lod, rp);
			// flag as used.
			materialUsed[matId]= true;
		}
	}

	// Apply it to meshBase
	CMeshBase::applyMaterialUsageOptim(materialUsed, remap);

	// Apply lut to meshGeom.
	_MeshMRMGeom.applyMaterialRemap(remap);
}


// ***************************************************************************
CTransformShape		*CMeshMRM::createInstance(CScene &scene)
{
	// Create a CMeshMRMInstance, an instance of a mesh.
	//===============================================
	CMeshMRMInstance		*mi= (CMeshMRMInstance*)scene.createModel(NL3D::MeshMRMInstanceId);
	mi->Shape= this;

	// instanciate the material part of the MeshMRM, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi, &scene);


	// do some instance init for MeshGeom
	_MeshMRMGeom.initInstance(mi);

	// init the FilterType
	mi->initRenderFilterType();

	return mi;
}


// ***************************************************************************
bool	CMeshMRM::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	return _MeshMRMGeom.clip(pyramid, worldMatrix);
}


// ***************************************************************************
void	CMeshMRM::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	// 0 or 0xFFFFFFFF
	uint32	mask= (0-(uint32)passOpaque);
	uint32	rdrFlags;
	// select rdrFlags, without ifs.
	rdrFlags=	mask & (IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderPassOpaque);
	rdrFlags|=	~mask & (IMeshGeom::RenderTransparentMaterial);
	// render the mesh
	_MeshMRMGeom.render(drv, trans, trans->getNumTrianglesAfterLoadBalancing(), rdrFlags, 1);
}


// ***************************************************************************
void	CMeshMRM::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);

	// serial Materials infos contained in CMeshBase.
	CMeshBase::serialMeshBase(f);


	// serial the geometry.
	_MeshMRMGeom.serial(f);
}


// ***************************************************************************
float	CMeshMRM::getNumTriangles (float distance)
{
	return _MeshMRMGeom.getNumTriangles (distance);
}


// ***************************************************************************
const CMeshMRMGeom& CMeshMRM::getMeshGeom () const
{
	return _MeshMRMGeom;
}


// ***************************************************************************
void	CMeshMRM::computeBonesId (CSkeletonModel *skeleton)
{
	_MeshMRMGeom.computeBonesId (skeleton);
}

// ***************************************************************************
void	CMeshMRM::updateSkeletonUsage (CSkeletonModel *skeleton, bool increment)
{
	_MeshMRMGeom.updateSkeletonUsage(skeleton, increment);
}

// ***************************************************************************
void	CMeshMRM::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	_MeshMRMGeom.changeMRMDistanceSetup(distanceFinest, distanceMiddle, distanceCoarsest);
}

// ***************************************************************************
IMeshGeom	*CMeshMRM::supportMeshBlockRendering (CTransformShape *trans, float &polygonCount ) const
{
	// Ok if meshGeom is ok.
	if(_MeshMRMGeom.supportMeshBlockRendering())
	{
		polygonCount= trans->getNumTrianglesAfterLoadBalancing();
		return (IMeshGeom*)&_MeshMRMGeom;
	}
	else
		return NULL;
}

// ***************************************************************************
void	CMeshMRM::profileSceneRender(CRenderTrav *rdrTrav, CTransformShape *trans, bool passOpaque)
{
	// 0 or 0xFFFFFFFF
	uint32	mask= (0-(uint32)passOpaque);
	uint32	rdrFlags;
	// select rdrFlags, without ifs.
	rdrFlags=	mask & (IMeshGeom::RenderOpaqueMaterial | IMeshGeom::RenderPassOpaque);
	rdrFlags|=	~mask & (IMeshGeom::RenderTransparentMaterial);
	// profile the mesh
	_MeshMRMGeom.profileSceneRender(rdrTrav, trans, trans->getNumTrianglesAfterLoadBalancing(), rdrFlags);
}



// ***************************************************************************
// ***************************************************************************
// CMeshMRMGeom RawSkin optimisation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CMeshMRMGeom::dirtMeshDataId()
{
	// see updateRawSkinNormal()
	_MeshDataId++;
}


// ***************************************************************************
void		CMeshMRMGeom::updateRawSkinNormal(bool enabled, CMeshMRMInstance *mi, sint curLodId)
{
	if(!enabled)
	{
		// if the instance cache is not cleared, must clear.
		if(mi->_RawSkinCache)
			mi->clearRawSkinCache();
	}
	else
	{
		// If the instance has no RawSkin, or has a too old RawSkin cache, must delete it, and recreate
		if( !mi->_RawSkinCache || mi->_RawSkinCache->MeshDataId!=_MeshDataId)
		{
			// first delete if too old.
			if(mi->_RawSkinCache)
				mi->clearRawSkinCache();
			// Then recreate, and use _MeshDataId to verify that the instance works with same data.
			mi->_RawSkinCache= new CRawSkinNormalCache;
			mi->_RawSkinCache->MeshDataId= _MeshDataId;
			mi->_RawSkinCache->LodId= -1;
		}


		/* If the instance rawSkin has a different Lod (or if -1), then must recreate it.
			NB: The lod may change each frame per instance, but suppose not so many change, so we can cache those data.
		*/
		if( mi->_RawSkinCache->LodId != curLodId )
		{
			H_AUTO( NL3D_CMeshMRMGeom_updateRawSkinNormal );

			CRawSkinNormalCache	&skinLod= *mi->_RawSkinCache;
			CLod				&lod= _Lods[curLodId];
			uint				i;
			sint				rawIdx;

			// Clear the raw skin mesh.
			skinLod.clearArrays();

			// Cache this lod
			mi->_RawSkinCache->LodId= curLodId;

			// For each matrix influence.
			nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);

			// For each vertex, acknowledge if it is a src for geomorph.
			static	vector<uint8>	softVertices;
			softVertices.clear();
			softVertices.resize( _VBufferFinal.getNumVertices(), 0 );
			for(i=0;i<lod.Geomorphs.size();i++)
			{
				softVertices[lod.Geomorphs[i].Start]= 1;
				softVertices[lod.Geomorphs[i].End]= 1;
			}

			// The remap from old index in _VBufferFinal to RawSkin vertices (without Geomorphs).
			static	vector<uint32>	vertexRemap;
			vertexRemap.resize( _VBufferFinal.getNumVertices() );
			sint	softSize[4];
			sint	hardSize[4];
			sint	softStart[4];
			sint	hardStart[4];
			// count vertices
			skinLod.TotalSoftVertices= 0;
			skinLod.TotalHardVertices= 0;
			for(i=0;i<4;i++)
			{
				softSize[i]= 0;
				hardSize[i]= 0;
				// Count.
				for(uint j=0;j<lod.InfluencedVertices[i].size();j++)
				{
					uint	vid= lod.InfluencedVertices[i][j];
					if(softVertices[vid])
						softSize[i]++;
					else
						hardSize[i]++;
				}
				skinLod.TotalSoftVertices+= softSize[i];
				skinLod.TotalHardVertices+= hardSize[i];
				skinLod.SoftVertices[i]= softSize[i];
				skinLod.HardVertices[i]= hardSize[i];
			}
			// compute offsets
			softStart[0]= 0;
			hardStart[0]= skinLod.TotalSoftVertices;
			for(i=1;i<4;i++)
			{
				softStart[i]= softStart[i-1]+softSize[i-1];
				hardStart[i]= hardStart[i-1]+hardSize[i-1];
			}
			// compute remap
			for(i=0;i<4;i++)
			{
				uint	softIdx= softStart[i];
				uint	hardIdx= hardStart[i];
				for(uint j=0;j<lod.InfluencedVertices[i].size();j++)
				{
					uint	vid= lod.InfluencedVertices[i][j];
					if(softVertices[vid])
						vertexRemap[vid]= softIdx++;
					else
						vertexRemap[vid]= hardIdx++;
				}
			}


			// Resize the dest array.
			skinLod.Vertices1.resize(lod.InfluencedVertices[0].size());
			skinLod.Vertices2.resize(lod.InfluencedVertices[1].size());
			skinLod.Vertices3.resize(lod.InfluencedVertices[2].size());
			skinLod.Vertices4.resize(lod.InfluencedVertices[3].size());

			// 1 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices1.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[0][i];
				// where to store?
				rawIdx= vertexRemap[vid];
				if(softVertices[vid])
					rawIdx-= softStart[0];
				else
					rawIdx+= softSize[0]-hardStart[0];
				// fill raw struct
				skinLod.Vertices1[rawIdx].MatrixId[0]= _SkinWeights[vid].MatrixId[0];
				skinLod.Vertices1[rawIdx].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices1[rawIdx].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices1[rawIdx].UV= *(CUV*)_VBufferFinal.getTexCoordPointer(vid);
			}

			// 2 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices2.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[1][i];
				// where to store?
				rawIdx= vertexRemap[vid];
				if(softVertices[vid])
					rawIdx-= softStart[1];
				else
					rawIdx+= softSize[1]-hardStart[1];
				// fill raw struct
				skinLod.Vertices2[rawIdx].MatrixId[0]= _SkinWeights[vid].MatrixId[0];
				skinLod.Vertices2[rawIdx].MatrixId[1]= _SkinWeights[vid].MatrixId[1];
				skinLod.Vertices2[rawIdx].Weights[0]= _SkinWeights[vid].Weights[0];
				skinLod.Vertices2[rawIdx].Weights[1]= _SkinWeights[vid].Weights[1];
				skinLod.Vertices2[rawIdx].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices2[rawIdx].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices2[rawIdx].UV= *(CUV*)_VBufferFinal.getTexCoordPointer(vid);
			}

			// 3 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices3.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[2][i];
				// where to store?
				rawIdx= vertexRemap[vid];
				if(softVertices[vid])
					rawIdx-= softStart[2];
				else
					rawIdx+= softSize[2]-hardStart[2];
				// fill raw struct
				skinLod.Vertices3[rawIdx].MatrixId[0]= _SkinWeights[vid].MatrixId[0];
				skinLod.Vertices3[rawIdx].MatrixId[1]= _SkinWeights[vid].MatrixId[1];
				skinLod.Vertices3[rawIdx].MatrixId[2]= _SkinWeights[vid].MatrixId[2];
				skinLod.Vertices3[rawIdx].Weights[0]= _SkinWeights[vid].Weights[0];
				skinLod.Vertices3[rawIdx].Weights[1]= _SkinWeights[vid].Weights[1];
				skinLod.Vertices3[rawIdx].Weights[2]= _SkinWeights[vid].Weights[2];
				skinLod.Vertices3[rawIdx].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices3[rawIdx].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices3[rawIdx].UV= *(CUV*)_VBufferFinal.getTexCoordPointer(vid);
			}

			// 4 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices4.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[3][i];
				// where to store?
				rawIdx= vertexRemap[vid];
				if(softVertices[vid])
					rawIdx-= softStart[3];
				else
					rawIdx+= softSize[3]-hardStart[3];
				// fill raw struct
				skinLod.Vertices4[rawIdx].MatrixId[0]= _SkinWeights[vid].MatrixId[0];
				skinLod.Vertices4[rawIdx].MatrixId[1]= _SkinWeights[vid].MatrixId[1];
				skinLod.Vertices4[rawIdx].MatrixId[2]= _SkinWeights[vid].MatrixId[2];
				skinLod.Vertices4[rawIdx].MatrixId[3]= _SkinWeights[vid].MatrixId[3];
				skinLod.Vertices4[rawIdx].Weights[0]= _SkinWeights[vid].Weights[0];
				skinLod.Vertices4[rawIdx].Weights[1]= _SkinWeights[vid].Weights[1];
				skinLod.Vertices4[rawIdx].Weights[2]= _SkinWeights[vid].Weights[2];
				skinLod.Vertices4[rawIdx].Weights[3]= _SkinWeights[vid].Weights[3];
				skinLod.Vertices4[rawIdx].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices4[rawIdx].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices4[rawIdx].UV= *(CUV*)_VBufferFinal.getTexCoordPointer(vid);
			}

			// Remap Geomorphs.
			//========
			uint	numGeoms= lod.Geomorphs.size();
			skinLod.Geomorphs.resize( numGeoms );
			for(i=0;i<numGeoms;i++)
			{
				// NB: don't add "numGeoms" to the index because RawSkin look in a TempArray in RAM, wich start at 0...
				skinLod.Geomorphs[i].Start= vertexRemap[lod.Geomorphs[i].Start];
				skinLod.Geomorphs[i].End= vertexRemap[lod.Geomorphs[i].End];
			}

			// Remap RdrPass.
			//========
			skinLod.RdrPass.resize(lod.RdrPass.size());
			for(i=0;i<skinLod.RdrPass.size();i++)
			{
				// NB: since RawSkin is possible only with SkinGrouping, and since SkniGrouping is 
				// possible only with no Quads/Lines, we should have only Tris here.
				nlassert( lod.RdrPass[i].PBlock.getNumQuad()== 0);
				nlassert( lod.RdrPass[i].PBlock.getNumLine()== 0);
				// remap tris.
				skinLod.RdrPass[i].setNumTri(lod.RdrPass[i].PBlock.getNumTri());
				uint32	*srcTriPtr= lod.RdrPass[i].PBlock.getTriPointer();
				uint32	*dstTriPtr= skinLod.RdrPass[i].getTriPointer();
				uint32	numIndices= lod.RdrPass[i].PBlock.getNumTri()*3;
				for(uint j=0;j<numIndices;j++, srcTriPtr++, dstTriPtr++)
				{
					uint	vid= *srcTriPtr;
					// If this index refers to a Geomorphed vertex, don't modify!
					if(vid<numGeoms)
						*dstTriPtr= vid;
					else
						*dstTriPtr= vertexRemap[vid] + numGeoms;
				}
			}
		}
	}
}


// ***************************************************************************
// ***************************************************************************
// CMeshMRMGeom Shadow Skin Rendering 
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CMeshMRMGeom::setShadowMesh(const std::vector<CShadowVertex> &shadowVertices, const std::vector<uint32> &triangles)
{
	_ShadowSkinVertices= shadowVertices;
	_ShadowSkinTriangles= triangles;
	// update flag. Support Shadow SkinGrouping if Shadow setuped, and if not too many vertices.
	_SupportShadowSkinGrouping= !_ShadowSkinVertices.empty() &&
		NL3D_SHADOW_MESH_SKIN_MANAGER_VERTEXFORMAT==CVertexBuffer::PositionFlag &&
		_ShadowSkinVertices.size() <= NL3D_SHADOW_MESH_SKIN_MANAGER_MAXVERTICES;
}

// ***************************************************************************
uint			CMeshMRMGeom::getNumShadowSkinVertices() const
{
	return _ShadowSkinVertices.size();
}

// ***************************************************************************
sint			CMeshMRMGeom::renderShadowSkinGeom(CMeshMRMInstance	*mi, uint remainingVertices, uint8 *vbDest)
{
	uint	numVerts= _ShadowSkinVertices.size();

	if(numVerts==0)
		return 0;

	// If the Lod is too big to render in the VBufferHard
	if(numVerts>remainingVertices)
		// return Failure
		return -1;

	// get the skeleton model to which I am skinned
	CSkeletonModel *skeleton;
	skeleton = mi->getSkeletonModel();
	// must be skinned for renderSkin()
	nlassert(skeleton);


	// Profiling
	//===========
	H_AUTO_USE( NL3D_MeshMRMGeom_RenderShadow );


	// Skinning.
	//===========

	// skinning with normal, but no tangent space
	applyArrayShadowSkin(&_ShadowSkinVertices[0], (CVector*)vbDest, skeleton, numVerts);


	// How many vertices are added to the VBuffer ???
	return numVerts;
}

// ***************************************************************************
void			CMeshMRMGeom::renderShadowSkinPrimitives(CMeshMRMInstance	*mi, CMaterial &castMat, IDriver *drv, uint baseVertex)
{
	nlassert(drv);

	if(_ShadowSkinTriangles.empty())
		return;

	// Profiling
	//===========
	H_AUTO_USE( NL3D_MeshMRMGeom_RenderShadow );

	// NB: the skeleton matrix has already been setuped by CSkeletonModel
	// NB: the normalize flag has already been setuped by CSkeletonModel

	// TODO_SHADOW: optim: Special triangle cache for shadow!
	static	vector<uint32>		shiftedTris;
	if(shiftedTris.size()<_ShadowSkinTriangles.size())
	{
		shiftedTris.resize(_ShadowSkinTriangles.size());
	}
	uint32	*src= &_ShadowSkinTriangles[0];
	uint32	*dst= &shiftedTris[0];
	for(uint n= _ShadowSkinTriangles.size();n>0;n--, src++, dst++)
	{
		*dst= *src + baseVertex;
	}

	// Render Triangles with cache
	//===========

	uint	numTris= _ShadowSkinTriangles.size()/3;

	// This speed up 4 ms for 80K polys.
	uint	memToCache= numTris*12;
	memToCache= min(memToCache, 4096U);
	CFastMem::precache(&shiftedTris[0], memToCache);

	// Render with the Materials of the MeshInstance.
	drv->renderTriangles(castMat, &shiftedTris[0], numTris);
}



} // NL3D

