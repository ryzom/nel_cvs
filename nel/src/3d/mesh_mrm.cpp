/** \file mesh_mrm.cpp
 * <File description>
 *
 * $Id: mesh_mrm.cpp,v 1.43 2002/06/26 16:48:58 berenguier Exp $
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

#include "3d/mesh_mrm.h"
#include "3d/mrm_builder.h"
#include "3d/mrm_parameters.h"
#include "3d/mesh_mrm_instance.h"
#include "3d/scene.h"
#include "3d/skeleton_model.h"
#include "nel/misc/bsphere.h"
#include "3d/stripifier.h"
#include "nel/misc/system_info.h"
#include "nel/misc/hierarchical_timer.h"
#include "3d/mesh_blender.h"


using namespace NLMISC;
using namespace std;


namespace NL3D 
{



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
	_NumLodRawSkin= 0;
	_PreciseClipping= false;
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
	_DistanceFinest= distanceFinest;
	_DistanceMiddle= distanceMiddle;
	_DistanceCoarsest= distanceCoarsest;

	// compile 
	compileDistanceSetup();
}


// ***************************************************************************
void			CMeshMRMGeom::compileDistanceSetup()
{
	// Compute _OODistDelta.
	_OODistanceDelta= 1.0f / (_DistanceCoarsest - _DistanceFinest);
	/* Compute exponent pow, such that 0.5= dMiddle^pow;
		ie 0.5= e(ln dMiddle * pow)
	*/
	float	dMiddle= (_DistanceCoarsest - _DistanceMiddle) * _OODistanceDelta;
	_DistancePow= (float)(log(0.5) / log(dMiddle));
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
	_DistanceFinest= meshBuildMRM.DistanceFinest;
	_DistanceMiddle= meshBuildMRM.DistanceMiddle;
	_DistanceCoarsest= meshBuildMRM.DistanceCoarsest;
	nlassert(_DistanceFinest>=0);
	nlassert(_DistanceMiddle > _DistanceFinest);
	nlassert(_DistanceCoarsest > _DistanceMiddle);
	// Compute _OODistDelta and _DistancePow
	compileDistanceSetup();


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
	_MaxFaceUsed= 0;
	_MinFaceUsed= 0;
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
			_MinFaceUsed+= pass.PBlock.getNumTriangles ();
		}
		// Compute MaxFaces.
		CLod	&lastLod= _Lods[_Lods.size()-1];
		for (pb=0; pb<lastLod.RdrPass.size(); pb++)
		{
			CRdrPass &pass= lastLod.RdrPass[pb];
			// Sum tri
			_MaxFaceUsed+= pass.PBlock.getNumTriangles ();
		}
	}


	// For skinning.
	//================================================
	if( _Skinned )
	{
		bkupOriginalSkinVertices();
	}
	// reset any RawSkin applied.
	clearRawSkin();


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
float	CMeshMRMGeom::getLevelDetailFromDist(float dist)
{
	if(dist <= _DistanceFinest)
		return 1;
	if(dist >= _DistanceCoarsest)
		return 0;

	float	d= (_DistanceCoarsest - dist) * _OODistanceDelta;
	return  (float)pow(d, _DistancePow);
}


// ***************************************************************************
void	CMeshMRMGeom::applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod)
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


	// If VBuffer Hard present
	uint8		*vertexDestPtr;
	if(_VBHard!=NULL)
	{
		// must write into it
		vertexDestPtr= (uint8*)_VBHard->lock();
		nlassert(vertexSize == _VBHard->getVertexSize());
	}
	else
	{
		// else write into vertexPtr.
		vertexDestPtr= vertexPtr;
	}


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


	// If _VBHard here, unlock it.
	if(_VBHard)
	{
		_VBHard->unlock();
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
			// if the box is not partially inside the plane, quit
			if( !_BBox.clipBack(localPlane) )
				return false;
		}
	}

	return true;
}


// ***************************************************************************
void	CMeshMRMGeom::render(IDriver *drv, CTransformShape *trans, bool passOpaque, float polygonCount, float globalAlpha, bool gaDisableZWrite)
{
	nlassert(drv);
	if(_Lods.size()==0)
		return;


	// get the meshMRM instance.
	CMeshBaseInstance	*mi= safe_cast<CMeshBaseInstance*>(trans);
	// get a ptr on scene
	CScene			*ownerScene= mi->getScene();
	// get a ptr on renderTrav
	CRenderTrav		*renderTrav= ownerScene->getRenderTrav();


	// get the result of the Load Balancing.
	float	alphaMRM;
	if(_MaxFaceUsed > _MinFaceUsed)
	{
		// compute the level of detail we want.
		alphaMRM= (polygonCount - _MinFaceUsed) / (_MaxFaceUsed - _MinFaceUsed);
		clamp(alphaMRM, 0, 1);
	}
	else
		alphaMRM= 1;


	// Choose what Lod to draw.
	alphaMRM*= _Lods.size()-1;
	sint	numLod= (sint)ceil(alphaMRM);
	float	alphaLod;
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


	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];
	if(lod.RdrPass.size()==0)
		return;


	// Update the vertexBufferHard (if possible).
	// \toto yoyo: TODO_OPTIMIZE: allocate only what is needed for the current Lod (Max of all instances, like
	// the loading....) (see loadHeader()).
	updateVertexBufferHard(drv, _VBufferFinal.getNumVertices());

	// get the skeleton model to which I am binded (else NULL).
	CSkeletonModel *skeleton;
	skeleton = mi->getSkeletonModel();
	// Is this mesh skinned?? true only if mesh is skinned, skeletonmodel is not NULL, and instance isSkinned()
	bool bMorphApplied = _MeshMorpher.BlendShapes.size() > 0;
	bool bSkinApplied = _Skinned && mi->isSkinned() && skeleton;
	bool useNormal= (_VBufferFinal.getVertexFormat() & CVertexBuffer::NormalFlag)!=0;
	bool useTangentSpace = _MeshVertexProgram && _MeshVertexProgram->needTangentSpace();


	// Profiling
	//===========
	// Special profile: Split between Skinned or not.
#ifdef  ALLOW_TIMING_MEASURES
	static NLMISC::CHTimer	NL3D_MeshMRMGeom_Render_Normal_timer( "NL3D_MeshMRMGeom_RenderNormal" ); 
	static NLMISC::CHTimer	NL3D_MeshMRMGeom_Render_Skinned_timer( "NL3D_MeshMRMGeom_RenderSkinned" ); 
	// choose what to time according to skin mode.
	NLMISC::CAutoTimer	NL3D_MeshMRMGeom_Render_auto( 
		bSkinApplied? &NL3D_MeshMRMGeom_Render_Skinned_timer : &NL3D_MeshMRMGeom_Render_Normal_timer);
#endif


	// Morphing
	// ========
	if (bMorphApplied)
	{
		// If Skinned we must update original skin vertices and normals because skinning use it
		// If Skinned and not bSkinApplied and lod.OriginalSkinRestored restoreOriginalSkinPart is
		// not called but mush morpher write changed vertices into VBHard so its ok. The unchanged vertices
		// are written in the preceding call to restoreOriginalSkinPart.
		if (_Skinned)
		{
			_MeshMorpher.initSkinned(&_VBufferOriginal,
								 &_VBufferFinal,
								 _VBHard,
								 useTangentSpace,
								 &_OriginalSkinVertices,
								 &_OriginalSkinNormals,
								 useTangentSpace ? &_OriginalTGSpace : NULL,
								 bSkinApplied );
			_MeshMorpher.updateSkinned (mi->getBlendShapeFactors());
		}
		else // Not even skinned so we have to do all the stuff
		{
			_MeshMorpher.init(&_VBufferOriginal,
								 &_VBufferFinal,
								 _VBHard,
								 useTangentSpace);
			_MeshMorpher.update (mi->getBlendShapeFactors());
		}
	}

	// Skinning.
	//===========
	// if ready to skin.
	if (bSkinApplied)
	{
		// Use RawSkin if possible: only if no morph, and only Vertex/Normal
		if ( !bMorphApplied && !useTangentSpace && useNormal )
			updateRawSkinNormal();
		// Use RawSkin TgSpace if possible: only if no morph, and Vertex/Normal/TgSpace
		else if ( !bMorphApplied && useTangentSpace && useNormal )
			updateRawSkinTgSpace();
		// Use none
		else
			clearRawSkin();

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
					if(_NumLodRawSkin>0)
						// Use faster RawSkin if possible.
						applyRawSkinWithNormalSSE(lod, _RawSkinNormalLods[numLod], skeleton);
					else
						applySkinWithNormalSSE (lod, skeleton);
				}
				else
				{
					// Tangent space stored in the last texture coordinate
					if(_NumLodRawSkin>0)
						// Use faster RawSkin if possible.
						applyRawSkinWithTangentSpaceSSE(lod, _RawSkinTgSpaceLods[numLod], 
							skeleton, _VBufferFinal.getNumTexCoordUsed() - 1);
					else
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
					if(_NumLodRawSkin>0)
						// Use faster RawSkin if possible.
						applyRawSkinWithNormal (lod, _RawSkinNormalLods[numLod], skeleton);
					else
						applySkinWithNormal (lod, skeleton);
				}
				else
				{
					// Tangent space stored in the last texture coordinate
					if(_NumLodRawSkin>0)
						// Use faster RawSkin if possible.
						applyRawSkinWithTangentSpace(lod, _RawSkinTgSpaceLods[numLod], 
							skeleton, _VBufferFinal.getNumTexCoordUsed() - 1);
					else
						applySkinWithTangentSpace(lod, skeleton, _VBufferFinal.getNumTexCoordUsed() - 1);
				}
			}
		}

		// endSkin.
		//--------
		// Fill the usefull AGP memory (if any one loaded).
		fillAGPSkinPart(lod);
		// dirt this lod part. (NB: this is not optimal, but sufficient :) ).
		lod.OriginalSkinRestored= false;
	}
	// if instance skin is invalid but mesh is skinned , we must copy vertices/normals from original vertices.
	else if (!bSkinApplied && _Skinned)
	{
		// do it for this Lod only, and if cache say it is necessary.
		if (!lod.OriginalSkinRestored)
			restoreOriginalSkinPart(lod);
	}

	// If skinning, Setup the skeleton matrix
	if (bSkinApplied)
	{
		drv->setupModelMatrix(skeleton->getWorldMatrix());
	}
	// else set the instance worldmatrix.
	else
	{
		drv->setupModelMatrix(trans->getWorldMatrix());
	}


	// Geomorph.
	//===========
	// Geomorph the choosen Lod (if not the coarser mesh).
	if(numLod>0)
	{
		applyGeomorph(lod.Geomorphs, alphaLod);
	}


	// Setup meshVertexProgram
	//===========
	CMatrix		invertedObjectMatrix;
	if (bSkinApplied)
		invertedObjectMatrix = skeleton->getWorldMatrix().inverted();
	else
		invertedObjectMatrix = trans->getWorldMatrix().inverted();


	// force normalisation of normals..
	bool	bkupNorm= drv->isForceNormalize();
	drv->forceNormalize(true);			

	// use MeshVertexProgram effect?
	bool	useMeshVP= _MeshVertexProgram != NULL ? _MeshVertexProgram->begin(drv, mi->getScene(), mi, invertedObjectMatrix, renderTrav->CamPos) : false;
	
	// Render the lod.
	//===========
	// active VB.
	if(_VBHard)
		drv->activeVertexBufferHard(_VBHard);
	else
		drv->activeVertexBuffer(_VBufferFinal);


	// Global alpha used ?
	bool globalAlphaUsed=globalAlpha!=1;
	uint8 globalAlphaInt=(uint8)(globalAlpha*255);

	// Render all pass.
	if (globalAlphaUsed)
	{
		for(uint i=0;i<lod.RdrPass.size();i++)
		{
			CRdrPass	&rdrPass= lod.RdrPass[i];

			if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (passOpaque == true) ) ||
				 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (passOpaque == false) ) )
			{
				// CMaterial Ref
				CMaterial &material=mi->Materials[rdrPass.MaterialId];

				// Use a MeshBlender to modify material and driver.
				CMeshBlender	blender;
				blender.prepareRenderForGlobalAlpha(material, drv, globalAlpha, globalAlphaInt, gaDisableZWrite);

				// If use meshVertexProgram, and use VPLightSetup
				if (useMeshVP)
				{
					if(_VBHard)
					{
						_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, _VBHard);
					}
					else
					{
						_MeshVertexProgram->setupForMaterial(material, drv, ownerScene, &_VBufferFinal);
					}
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

			if ( ( (mi->Materials[rdrPass.MaterialId].getBlend() == false) && (passOpaque == true) ) ||
				 ( (mi->Materials[rdrPass.MaterialId].getBlend() == true) && (passOpaque == false) ) )
			{
				if (useMeshVP)
				{
					if(_VBHard)
					{
						_MeshVertexProgram->setupForMaterial(mi->Materials[rdrPass.MaterialId], drv, ownerScene, _VBHard);
					}
					else
					{
						_MeshVertexProgram->setupForMaterial(mi->Materials[rdrPass.MaterialId], drv, ownerScene, &_VBufferFinal);
					}
				}	

				// Render with the Materials of the MeshInstance.
				drv->render(rdrPass.PBlock, mi->Materials[rdrPass.MaterialId]);
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
	sint	ver= f.serialVersion(4);

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
	f.serial(_MaxFaceUsed);
	f.serial(_MinFaceUsed);
	f.serial(_DistanceFinest);
	f.serial(_DistanceMiddle);
	f.serial(_DistanceCoarsest);
	f.serial(_OODistanceDelta);
	f.serial(_DistancePow);
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

	// reset any RawSkin applied.
	clearRawSkin();


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
	sint	ver= f.serialVersion(4);
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
	f.serial(_MaxFaceUsed);
	f.serial(_MinFaceUsed);
	f.serial(_DistanceFinest);
	f.serial(_DistanceMiddle);
	f.serial(_DistanceCoarsest);
	f.serial(_OODistanceDelta);
	f.serial(_DistancePow);
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
void	CMeshMRMGeom::restoreOriginalSkinPart(CLod &lod)
{
	nlassert(_Skinned);


	/* NB: this copies into RAM, and not AGP.
		This is because Geomorph needs to read Data in RAM. So the easiest way is to copy all date into RAM,
		then duplicate  (see fillAGPSkinPart()) in AGP.
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


	// Fill the usefull AGP memory (if any one loaded).
	fillAGPSkinPart(lod);


	// clean this lod part. (NB: this is not optimal, but sufficient :) ).
	lod.OriginalSkinRestored= true;
}

// ***************************************************************************

float CMeshMRMGeom::getNumTriangles (float distance)
{
	// NB: this is an approximation, but this is continious.
	// return the lod detail [0,1].
	float	ld= getLevelDetailFromDist(distance);
	// return in nb face.
	return _MinFaceUsed + ld * (_MaxFaceUsed - _MinFaceUsed);
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


	// If the vbufferhard is not here, or if diryt, or if do not have enough vertices.
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
		_VBHard= _Driver->createVertexBufferHard(_VBufferFinal.getVertexFormat(), _VBufferFinal.getValueTypePointer (), numVertices, IDriver::VBHardAGP);


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
		// increment or decrement not Forced, because CMeshGeom use getActiveBoneSkinMatrix().
		if(increment)
			sm->incBoneUsage(_BonesIdExt[i], CSkeletonModel::UsageNormal);
		else
			sm->decBoneUsage(_BonesIdExt[i], CSkeletonModel::UsageNormal);
	}
}


// ***************************************************************************
void	CMeshMRMGeom::compileRunTime()
{
	_PreciseClipping= _BBox.getRadius() >= NL3D_MESH_PRECISE_CLIP_THRESHOLD;
}


// ***************************************************************************
// ***************************************************************************
// Mesh Block Render Interface
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMeshMRMGeom::supportMeshBlockRendering () const
{
	// TODODO
	return false;
}

// ***************************************************************************
bool	CMeshMRMGeom::sortPerMaterial() const
{
	return false;
}
// ***************************************************************************
uint	CMeshMRMGeom::getNumRdrPasses() const 
{
	return 0;
}
// ***************************************************************************
void	CMeshMRMGeom::beginMesh(CMeshGeomRenderContext &rdrCtx) 
{
}
// ***************************************************************************
void	CMeshMRMGeom::activeInstance(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *inst, float polygonCount) 
{
}
// ***************************************************************************
void	CMeshMRMGeom::renderPass(CMeshGeomRenderContext &rdrCtx, CMeshBaseInstance *mi, float polygonCount, uint rdrPassId) 
{
}
// ***************************************************************************
void	CMeshMRMGeom::endMesh(CMeshGeomRenderContext &rdrCtx) 
{
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


	// LoadBalancing backward compatibility: if the mesh is skinned, add it to the 
	// "Global" Load Balancing Group.
	if(_MeshMRMGeom.isSkinned())
	{
		mi->setLoadBalancingGroup("Global");
	}

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
	_MeshMRMGeom.render(drv, trans, passOpaque, trans->getNumTrianglesAfterLoadBalancing(), 1, false);
}


// ***************************************************************************
void	CMeshMRM::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(0);

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
// ***************************************************************************
// CMeshMRMGeom RawSkin optimisation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CMeshMRMGeom::clearRawSkin()
{
	if(_NumLodRawSkin>0)
	{
		contReset(_RawSkinNormalLods);
		contReset(_RawSkinTgSpaceLods);
		_NumLodRawSkin= 0;
	}
}


// ***************************************************************************
void		CMeshMRMGeom::updateRawSkinNormal()
{
	// If need to load new lods.
	if(_NumLodRawSkin<_NbLodLoaded)
	{
		uint	i;

		// TODO_OPTIM
		// Alloc space, big memcpy here...
		_RawSkinNormalLods.resize(_NbLodLoaded);

		// Build new lods.
		for(;_NumLodRawSkin<_NbLodLoaded;_NumLodRawSkin++)
		{
			CRawSkinNormalLod	&skinLod= _RawSkinNormalLods[_NumLodRawSkin];
			CLod				&lod= _Lods[_NumLodRawSkin];

			// For each matrix influence.
			nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);

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
				// fill raw struct
				skinLod.Vertices1[i].MatrixId[0]= _SkinWeights[vid].MatrixId[0];
				skinLod.Vertices1[i].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices1[i].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices1[i].VertexId= vid;
			}

			// 2 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices2.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[1][i];
				// fill raw struct
				skinLod.Vertices2[i].MatrixId[0]= _SkinWeights[vid].MatrixId[0];
				skinLod.Vertices2[i].MatrixId[1]= _SkinWeights[vid].MatrixId[1];
				skinLod.Vertices2[i].Weights[0]= _SkinWeights[vid].Weights[0];
				skinLod.Vertices2[i].Weights[1]= _SkinWeights[vid].Weights[1];
				skinLod.Vertices2[i].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices2[i].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices2[i].VertexId= vid;
			}

			// 3 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices3.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[2][i];
				// fill raw struct
				skinLod.Vertices3[i].SkinWeight= _SkinWeights[vid];
				skinLod.Vertices3[i].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices3[i].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices3[i].VertexId= vid;
			}

			// 4 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices4.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[3][i];
				// fill raw struct
				skinLod.Vertices4[i].SkinWeight= _SkinWeights[vid];
				skinLod.Vertices4[i].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices4[i].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices4[i].VertexId= vid;
			}

		}
	}
}


// ***************************************************************************
void		CMeshMRMGeom::updateRawSkinTgSpace()
{
	// If need to load new lods.
	if(_NumLodRawSkin<_NbLodLoaded)
	{
		uint	i;

		// TODO_OPTIM
		// Alloc space, big memcpy here...
		_RawSkinTgSpaceLods.resize(_NbLodLoaded);

		// Build new lods.
		for(;_NumLodRawSkin<_NbLodLoaded;_NumLodRawSkin++)
		{
			CRawSkinTgSpaceLod	&skinLod= _RawSkinTgSpaceLods[_NumLodRawSkin];
			CLod				&lod= _Lods[_NumLodRawSkin];

			// For each matrix influence.
			nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);

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
				// fill raw struct
				skinLod.Vertices1[i].MatrixId[0]= _SkinWeights[vid].MatrixId[0];
				skinLod.Vertices1[i].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices1[i].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices1[i].TgSpace= _OriginalTGSpace[vid];
				skinLod.Vertices1[i].VertexId= vid;
			}

			// 2 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices2.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[1][i];
				// fill raw struct
				skinLod.Vertices2[i].MatrixId[0]= _SkinWeights[vid].MatrixId[0];
				skinLod.Vertices2[i].MatrixId[1]= _SkinWeights[vid].MatrixId[1];
				skinLod.Vertices2[i].Weights[0]= _SkinWeights[vid].Weights[0];
				skinLod.Vertices2[i].Weights[1]= _SkinWeights[vid].Weights[1];
				skinLod.Vertices2[i].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices2[i].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices2[i].TgSpace= _OriginalTGSpace[vid];
				skinLod.Vertices2[i].VertexId= vid;
			}

			// 3 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices3.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[2][i];
				// fill raw struct
				skinLod.Vertices3[i].SkinWeight= _SkinWeights[vid];
				skinLod.Vertices3[i].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices3[i].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices3[i].TgSpace= _OriginalTGSpace[vid];
				skinLod.Vertices3[i].VertexId= vid;
			}

			// 4 Matrix skinning.
			//========
			for(i=0;i<skinLod.Vertices4.size();i++)
			{
				// get the dest vertex.
				uint	vid= lod.InfluencedVertices[3][i];
				// fill raw struct
				skinLod.Vertices4[i].SkinWeight= _SkinWeights[vid];
				skinLod.Vertices4[i].Vertex= _OriginalSkinVertices[vid];
				skinLod.Vertices4[i].Normal= _OriginalSkinNormals[vid];
				skinLod.Vertices4[i].TgSpace= _OriginalTGSpace[vid];
				skinLod.Vertices4[i].VertexId= vid;
			}

		}
	}
}



} // NL3D

