/** \file mesh_mrm_skin.cpp
 * Skin computation part for class CMeshMRM.
 *
 * $Id: mesh_mrm_skin.cpp,v 1.9 2002/08/05 12:17:29 berenguier Exp $
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
#include "nel/misc/fast_mem.h"
#include "nel/misc/system_info.h"
#include "3d/mesh_mrm.h"
#include "3d/mrm_builder.h"
#include "3d/mrm_parameters.h"
#include "3d/mesh_mrm_instance.h"
#include "3d/scene.h"
#include "3d/skeleton_model.h"
#include "3d/stripifier.h"
#include "3d/matrix_3x4.h"
#include "3d/raw_skin.h"


using namespace NLMISC;
using namespace std;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// CMatrix3x4SSE array correctly aligned
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
#define	NL3D_SSE_ALIGNEMENT		16
/**
 *	A CMatrix3x4SSE array correctly aligned
 */
class	CMatrix3x4SSEArray
{
private:
	void	*_AllocData;
	void	*_Data;
	uint	_Size;
	uint	_Capacity;

public:
	CMatrix3x4SSEArray()
	{
		_AllocData= NULL;
		_Data= NULL;
		_Size= 0;
		_Capacity= 0;
	}
	~CMatrix3x4SSEArray()
	{
		clear();
	}
	CMatrix3x4SSEArray(const CMatrix3x4SSEArray &other)
	{
		_AllocData= NULL;
		_Data= NULL;
		_Size= 0;
		_Capacity= 0;
		*this= other;
	}
	CMatrix3x4SSEArray &operator=(const CMatrix3x4SSEArray &other)
	{
		if( this == &other)
			return *this;
		resize(other.size());
		// copy data from aligned pointers to aligned pointers.
		memcpy(_Data, other._Data, size() * sizeof(CMatrix3x4SSE) );

		return *this;
	}


	CMatrix3x4SSE	*getPtr()
	{
		return (CMatrix3x4SSE*)_Data;
	}

	void	clear()
	{
		free(_AllocData);
		_AllocData= NULL;
		_Data= NULL;
		_Size= 0;
		_Capacity= 0;
	}

	void	resize(uint n)
	{
		// reserve ??
		if(n>_Capacity)
			reserve( max(2*_Capacity, n));
		_Size= n;
	}

	void	reserve(uint n)
	{
		if(n==0)
			clear();
		else if(n>_Capacity)
		{
			// Alloc new data.
			void	*newAllocData;
			void	*newData;

			// Alloc for alignement.
			newAllocData= malloc(n * sizeof(CMatrix3x4SSE) + NL3D_SSE_ALIGNEMENT-1);
			if(newAllocData==NULL)
				throw Exception("SSE Allocation Failed");

			// Align ptr
			newData= (void*) ( ((uint32)newAllocData+NL3D_SSE_ALIGNEMENT-1) & (~(NL3D_SSE_ALIGNEMENT-1)) );

			// copy valid data from old to new.
			memcpy(newData, _Data, size() * sizeof(CMatrix3x4SSE) );

			// change ptrs and capacity.
			_Data= newData;
			_AllocData= newAllocData;
			_Capacity= n;
		}
	}

	uint	size() const {return _Size;}


	CMatrix3x4SSE	&operator[](uint i) {return ((CMatrix3x4SSE*)_Data)[i];}
};



// ***************************************************************************
// ***************************************************************************
// Matrix manip
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
template <class TMatrixArray>
inline void	computeBoneMatrixes3x4(TMatrixArray &boneMat3x4, const vector<uint32> &matInfs, const CSkeletonModel *skeleton)
{
	// For all matrix this lod use.
	for(uint i= 0; i<matInfs.size(); i++)
	{
		// Get Matrix info.
		uint	matId= matInfs[i];
		const CMatrix		&boneMat= skeleton->getActiveBoneSkinMatrix(matId);

		// compute "fast" matrix 3x4.
		// resize Matrix3x4.
		if(matId>=boneMat3x4.size())
		{
			boneMat3x4.resize(matId+1);
		}
		boneMat3x4[matId].set(boneMat);
	}
}


// ***************************************************************************
// ***************************************************************************
// Simple (slow) skinning version with position only.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CMeshMRMGeom::applySkin(CLod &lod, const CSkeletonModel *skeleton)
{
	nlassert(_Skinned);
	if(_SkinWeights.size()==0)
		return;

	// get vertexPtr.
	//===========================
	uint8		*destVertexPtr= (uint8*)_VBufferFinal.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// must have XYZ.
	nlassert(flags & CVertexBuffer::PositionFlag);


	// compute src array.
	CMesh::CSkinWeight	*srcSkinPtr;
	CVector				*srcVertexPtr;
	srcSkinPtr= &_SkinWeights[0];
	srcVertexPtr= &_OriginalSkinVertices[0];



	// Compute usefull Matrix for this lod.
	//===========================
	// Those arrays map the array of bones in skeleton.
	static	vector<CMatrix3x4>			boneMat3x4;
	computeBoneMatrixes3x4(boneMat3x4, lod.MatrixInfluences, skeleton);


	// apply skinning.
	//===========================
	// assert, code below is written especially for 4 per vertex.
	nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);
	for(uint i=0;i<NL3D_MESH_SKINNING_MAX_MATRIX;i++)
	{
		uint		nInf= lod.InfluencedVertices[i].size();
		if( nInf==0 )
			continue;
		uint32		*infPtr= &(lod.InfluencedVertices[i][0]);

		// apply the skin to the vertices
		switch(i)
		{
		//=========
		case 0:
			// Special case for Vertices influenced by one matrix. Just copy result of mul.
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, *dstVertex);
			}
			break;

		//=========
		case 1:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			}
			break;

		//=========
		case 2:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			}
			break;

		//=========
		case 3:
			//  for all InfluencedVertices only.
			for(;nInf>0;nInf--, infPtr++)
			{
				uint	index= *infPtr;
				CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
				CVector				*srcVertex= srcVertexPtr + index;
				uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
				CVector				*dstVertex= (CVector*)(dstVertexVB);


				// Vertex.
				boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
				boneMat3x4[ srcSkin->MatrixId[3] ].mulAddPoint( *srcVertex, srcSkin->Weights[3], *dstVertex);
			}
			break;

		}
	}
}


// ***************************************************************************
// ***************************************************************************
// Old school Template skinning: SSE or not.
// ***************************************************************************
// ***************************************************************************


// RawSkin Cache constants
//===============
// The number of byte to process per block
const	uint	NL_BlockByteL1= 4096;

// Number of vertices per block to process with 1 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal1= NL_BlockByteL1 / sizeof(CRawVertexNormalSkin1);
// Number of vertices per block to process with 2 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal2= NL_BlockByteL1 / sizeof(CRawVertexNormalSkin2);
// Number of vertices per block to process with 3/4 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal4= NL_BlockByteL1 / sizeof(CRawVertexNormalSkin4);


// Old School template: include the same file with define switching
#undef NL_SKIN_SSE
#include "mesh_mrm_skin_template.cpp"
#define NL_SKIN_SSE
#include "mesh_mrm_skin_template.cpp"



// ***************************************************************************
// ***************************************************************************
// Misc.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
void				CMeshMRMGeom::fillAGPSkinPart(CLod &lod, IVertexBufferHard *currentVBHard)
{
	// if VBHard OK
	if(currentVBHard)
	{
		// lock buffer.
		uint8		*vertexDst= (uint8*)currentVBHard->lock();

		// do it.
		fillAGPSkinPartWithVBHardPtr(lod, vertexDst);

		// release
		currentVBHard->unlock();
	}
}


// ***************************************************************************
void				CMeshMRMGeom::fillAGPSkinPartWithVBHardPtr(CLod &lod, uint8 *vertexDst)
{
	// Fill AGP vertices used by this lod from RAM. (not geomorphed ones).
	if( lod.SkinVertexBlocks.size()>0 )
	{
		// Get VB info, and lock buffers.
		uint8		*vertexSrc= (uint8*)_VBufferFinal.getVertexCoordPointer();
		uint32		vertexSize= _VBufferFinal.getVertexSize();

		// big copy of all vertices and their data.
		// NB: this not help RAM bandwidth, but this help AGP write combiners.
		// For the majority of mesh (vertex/normal/uv), this is better (6/10).

		// Also, this is a requirement for MeshMorpher to work, because the MehsMorpher may modify
		// UV and color


		// For all block of vertices.
		CVertexBlock	*vBlock= &lod.SkinVertexBlocks[0];
		uint	n= lod.SkinVertexBlocks.size();

		/*
			It appears that it is not a so good idea to use CFastMem::memcpySSE() here, maybe because
			data is often already in cache (written by skinning), and maybe because movntq is not usefull here since
			AGP is already a write-combining memory.
		*/
		for(;n>0; n--, vBlock++)
		{
			// For all vertices of this block, copy it from RAM to VRAM.
			uint8		*src= vertexSrc + vertexSize * vBlock->VertexStart;
			uint8		*dst= vertexDst + vertexSize * vBlock->VertexStart;

			memcpy(dst, src, vBlock->NVertices * vertexSize);
		}
	}
}



} // NL3D

