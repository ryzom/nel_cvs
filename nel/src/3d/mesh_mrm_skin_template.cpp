/** \file mesh_mrm_skin_template.cpp
 * File not compiled. Included from mesh_mrm_skin.cpp. It is a "old school" template.
 *
 * $Id: mesh_mrm_skin_template.cpp,v 1.4 2002/08/05 12:17:29 berenguier Exp $
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


/*
Old School template: file is included 2 times, with NL_SKIN_SSE defined or not.

*/


// ***************************************************************************
// ***************************************************************************
// "Templates" for VertexSkinning with any input matrix type.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
#ifdef NL_SKIN_SSE
static void	applyArraySkinNormalT(uint numMatrixes, uint32 *infPtr, CMesh::CSkinWeight *srcSkinPtr, 
	CVector *srcVertexPtr, CVector *srcNormalPtr, uint normalOff,
	uint8 *destVertexPtr, CMatrix3x4SSEArray &boneMat3x4, uint vertexSize, uint nInf)
#else
static void	applyArraySkinNormalT(uint numMatrixes, uint32 *infPtr, CMesh::CSkinWeight *srcSkinPtr, 
	CVector *srcVertexPtr, CVector *srcNormalPtr, uint normalOff,
	uint8 *destVertexPtr, vector<CMatrix3x4> &boneMat3x4, uint vertexSize, uint nInf)
#endif
{
	/* Prefetch all vertex/normal before, it is to be faster.
	*/
#ifdef NL_OS_WINDOWS
	{
		uint	nInfTmp= nInf;
		uint32	*infTmpPtr= infPtr;
		for(;nInfTmp>0;nInfTmp--, infTmpPtr++)
		{
			uint	index= *infTmpPtr;
			CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
			CVector				*srcVertex= srcVertexPtr + index;
			CVector				*srcNormal= srcNormalPtr + index;

			__asm
			{
				mov eax, srcSkin
				mov ebx, srcVertex
				mov ecx, srcNormal
				mov edx, [eax]
				mov edx, [ebx]
				mov edx, [ecx]
			}

		}
	}
#endif

	// Process vertices.
	switch(numMatrixes)
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
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, *dstNormal);
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
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
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
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
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
			CVector				*srcNormal= srcNormalPtr + index;
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);


			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddPoint( *srcVertex, srcSkin->Weights[3], *dstVertex);
			// Normal.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddVector( *srcNormal, srcSkin->Weights[3], *dstNormal);
		}
		break;

	}
}



// ***************************************************************************
#ifdef NL_SKIN_SSE
static void	applyArraySkinTangentSpaceT(uint numMatrixes, uint32 *infPtr, CMesh::CSkinWeight *srcSkinPtr, 
	CVector *srcVertexPtr, CVector *srcNormalPtr, CVector *tgSpacePtr, uint normalOff, uint tgSpaceOff,
	uint8 *destVertexPtr, CMatrix3x4SSEArray &boneMat3x4, uint vertexSize, uint nInf)
#else
static void	applyArraySkinTangentSpaceT(uint numMatrixes, uint32 *infPtr, CMesh::CSkinWeight *srcSkinPtr, 
	CVector *srcVertexPtr, CVector *srcNormalPtr, CVector *tgSpacePtr, uint normalOff, uint tgSpaceOff,
	uint8 *destVertexPtr, vector<CMatrix3x4> &boneMat3x4, uint vertexSize, uint nInf)
#endif
{
	/* Prefetch all vertex/normal/tgSpace before, it is faster.
	*/
#ifdef NL_OS_WINDOWS
	{
		uint	nInfTmp= nInf;
		uint32	*infTmpPtr= infPtr;
		for(;nInfTmp>0;nInfTmp--, infTmpPtr++)
		{
			uint	index= *infTmpPtr;
			CMesh::CSkinWeight	*srcSkin= srcSkinPtr + index;
			CVector				*srcVertex= srcVertexPtr + index;
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;

			__asm
			{
				mov eax, srcSkin
				mov ebx, srcVertex
				mov ecx, srcNormal
				mov esi, srcTgSpace
				mov edx, [eax]
				mov edx, [ebx]
				mov edx, [ecx]
				mov edx, [esi]
			}

		}
	}
#endif

	// Process vertices.
	switch(numMatrixes)
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
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;
			//
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);
			CVector				*dstTgSpace= (CVector*)(dstVertexVB + tgSpaceOff);



			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, *dstVertex);
			// Normal.				
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, *dstNormal);
			// Tg space
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcTgSpace, *dstTgSpace);

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
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;
			//
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);
			CVector				*dstTgSpace= (CVector*)(dstVertexVB + tgSpaceOff);

			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			// Normal.				
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			// Tg space
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcTgSpace, srcSkin->Weights[0], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcTgSpace, srcSkin->Weights[1], *dstTgSpace);
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
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;
			//
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);
			CVector				*dstTgSpace= (CVector*)(dstVertexVB + tgSpaceOff);

			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			// Normal.				
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
			// Tg space
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcTgSpace, srcSkin->Weights[0], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcTgSpace, srcSkin->Weights[1], *dstTgSpace);				
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcTgSpace, srcSkin->Weights[2], *dstTgSpace);				
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
			CVector				*srcNormal= srcNormalPtr + index;
			CVector				*srcTgSpace= tgSpacePtr + index;
			//
			uint8				*dstVertexVB= destVertexPtr + index * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + normalOff);
			CVector				*dstTgSpace= (CVector*)(dstVertexVB + tgSpaceOff);

			// Vertex.
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetPoint( *srcVertex, srcSkin->Weights[0], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddPoint( *srcVertex, srcSkin->Weights[1], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddPoint( *srcVertex, srcSkin->Weights[2], *dstVertex);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddPoint( *srcVertex, srcSkin->Weights[3], *dstVertex);
			// Normal.				
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcNormal, srcSkin->Weights[0], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcNormal, srcSkin->Weights[1], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcNormal, srcSkin->Weights[2], *dstNormal);
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddVector( *srcNormal, srcSkin->Weights[3], *dstNormal);
			// Tg space
			boneMat3x4[ srcSkin->MatrixId[0] ].mulSetVector( *srcTgSpace, srcSkin->Weights[0], *dstTgSpace);
			boneMat3x4[ srcSkin->MatrixId[1] ].mulAddVector( *srcTgSpace, srcSkin->Weights[1], *dstTgSpace);				
			boneMat3x4[ srcSkin->MatrixId[2] ].mulAddVector( *srcTgSpace, srcSkin->Weights[2], *dstTgSpace);								
			boneMat3x4[ srcSkin->MatrixId[3] ].mulAddVector( *srcTgSpace, srcSkin->Weights[3], *dstTgSpace);								
		}
		break;

	}

}



// ***************************************************************************
// ***************************************************************************
// ApplySkin methods.
// ***************************************************************************
// ***************************************************************************


#ifdef NL_SKIN_SSE
#undef	NL_SKIN_MATRIX_ARRAY
#define	NL_SKIN_MATRIX_ARRAY	CMatrix3x4SSEArray
#else
#undef	NL_SKIN_MATRIX_ARRAY
#define	NL_SKIN_MATRIX_ARRAY	vector<CMatrix3x4>
#endif


// ***************************************************************************
#ifdef NL_SKIN_SSE
void	CMeshMRMGeom::applySkinWithNormalSSE(CLod &lod, const CSkeletonModel *skeleton)
#else
void	CMeshMRMGeom::applySkinWithNormal(CLod &lod, const CSkeletonModel *skeleton)
#endif
{
	nlassert(_Skinned);
	if(_SkinWeights.size()==0)
		return;

	// get vertexPtr / normalOff.
	//===========================
	uint8		*destVertexPtr= (uint8*)_VBufferFinal.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// must have XYZ and Normal.
	nlassert((flags & CVertexBuffer::PositionFlag) 
			 && (flags & CVertexBuffer::NormalFlag) 
			);


	// Compute offset of each component of the VB.
	sint32		normalOff;
	normalOff= _VBufferFinal.getNormalOff();


	// compute src array.
	CMesh::CSkinWeight	*srcSkinPtr;
	CVector				*srcVertexPtr;
	CVector				*srcNormalPtr= NULL;
	srcSkinPtr= &_SkinWeights[0];
	srcVertexPtr= &_OriginalSkinVertices[0];
	srcNormalPtr= &(_OriginalSkinNormals[0]);



	// Compute usefull Matrix for this lod.
	//===========================
	// Those arrays map the array of bones in skeleton.
	static	NL_SKIN_MATRIX_ARRAY			boneMat3x4;
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
		applyArraySkinNormalT(i, infPtr, srcSkinPtr, srcVertexPtr, srcNormalPtr, 
			normalOff, destVertexPtr, 
			boneMat3x4, vertexSize, nInf);
	}
}


// ***************************************************************************
#ifdef NL_SKIN_SSE
void	CMeshMRMGeom::applySkinWithTangentSpaceSSE(CLod &lod, const CSkeletonModel *skeleton, 
	uint tangentSpaceTexCoord)
#else
void	CMeshMRMGeom::applySkinWithTangentSpace(CLod &lod, const CSkeletonModel *skeleton, 
	uint tangentSpaceTexCoord)
#endif
{
	nlassert(_Skinned);
	if(_SkinWeights.size()==0)
		return;

	// get vertexPtr / normalOff / tangent space offset.
	//===========================
	uint8		*destVertexPtr= (uint8*)_VBufferFinal.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// must have XYZ.
	// if there's tangent space, there also must be a normal there.
	nlassert((flags & CVertexBuffer::PositionFlag) 
			 && (flags & CVertexBuffer::NormalFlag) 
			);


	// Compute offset of each component of the VB.
	sint32		normalOff;	
	normalOff= _VBufferFinal.getNormalOff();
	
	// tg space offset
	sint32		tgSpaceOff = _VBufferFinal.getTexCoordOff((uint8) tangentSpaceTexCoord);

	// compute src array.
	CMesh::CSkinWeight	*srcSkinPtr;
	CVector				*srcVertexPtr;
	CVector				*srcNormalPtr;
	CVector				*tgSpacePtr;
	//
	srcSkinPtr= &_SkinWeights[0];
	srcVertexPtr= &_OriginalSkinVertices[0];	
	srcNormalPtr= &(_OriginalSkinNormals[0]);
	tgSpacePtr = &(_OriginalTGSpace[0]);



	// Compute usefull Matrix for this lod.
	//===========================
	// Those arrays map the array of bones in skeleton.
	static	NL_SKIN_MATRIX_ARRAY			boneMat3x4;
	computeBoneMatrixes3x4(boneMat3x4, lod.MatrixInfluences, skeleton);


	// apply skinning (with tangent space added)
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
		applyArraySkinTangentSpaceT(i, infPtr, srcSkinPtr, srcVertexPtr, srcNormalPtr, tgSpacePtr, 
			normalOff, tgSpaceOff, destVertexPtr, 
			boneMat3x4, vertexSize, nInf);
	}
}



// ***************************************************************************
// ***************************************************************************
// Raw "Vertex/Normal only" ApplySkin methods.
// ***************************************************************************
// ***************************************************************************


#define	NL3D_RAWSKIN_NORMAL_OFF 12

// ***************************************************************************
#ifdef NL_SKIN_SSE
void		CMeshMRMGeom::applyArrayRawSkinNormal1(CRawVertexNormalSkin1 *src, uint8 *destVertexPtr, 
	CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf)
#else
void		CMeshMRMGeom::applyArrayRawSkinNormal1(CRawVertexNormalSkin1 *src, uint8 *destVertexPtr, 
	CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf)
#endif
{
	for(;nInf>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertexNormal1, nInf);
		// next block.
		nInf-= nBlockInf;

	#ifdef NL_SKIN_SSE
		// cache the data in L1 cache.
		CFastMem::precacheSSE(src, nBlockInf * sizeof(CRawVertexNormalSkin1));
	#else
		// slower precache but still usefull
		if( CSystemInfo::hasMMX() )
			CFastMem::precacheMMX(src, nBlockInf * sizeof(CRawVertexNormalSkin1));
	#endif

		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++)
		{
			uint8				*dstVertexVB= destVertexPtr + src->VertexId * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + NL3D_RAWSKIN_NORMAL_OFF);

			// Vertex.
			boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex, *dstVertex);
			// Normal.
			boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Normal, *dstNormal);
		}
	}


}

// ***************************************************************************
#ifdef NL_SKIN_SSE
void		CMeshMRMGeom::applyArrayRawSkinNormal2(CRawVertexNormalSkin2 *src, uint8 *destVertexPtr, 
	CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf)
#else
void		CMeshMRMGeom::applyArrayRawSkinNormal2(CRawVertexNormalSkin2 *src, uint8 *destVertexPtr, 
	CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf)
#endif
{
	for(;nInf>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertexNormal2, nInf);
		// next block.
		nInf-= nBlockInf;

	#ifdef NL_SKIN_SSE
		// cache the data in L1 cache.
		CFastMem::precacheSSE(src, nBlockInf * sizeof(CRawVertexNormalSkin2));
	#else
		// slower precache but still usefull
		if( CSystemInfo::hasMMX() )
			CFastMem::precacheMMX(src, nBlockInf * sizeof(CRawVertexNormalSkin2));
	#endif

		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++)
		{
			uint8				*dstVertexVB= destVertexPtr + src->VertexId * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + NL3D_RAWSKIN_NORMAL_OFF);

			// Vertex.
			boneMat3x4[ src->MatrixId[0] ].mulSetPoint( src->Vertex, src->Weights[0], *dstVertex);
			boneMat3x4[ src->MatrixId[1] ].mulAddPoint( src->Vertex, src->Weights[1], *dstVertex);
			// Normal.
			boneMat3x4[ src->MatrixId[0] ].mulSetVector( src->Normal, src->Weights[0], *dstNormal);
			boneMat3x4[ src->MatrixId[1] ].mulAddVector( src->Normal, src->Weights[1], *dstNormal);
		}
	}

}

// ***************************************************************************
#ifdef NL_SKIN_SSE
void		CMeshMRMGeom::applyArrayRawSkinNormal3(CRawVertexNormalSkin4 *src, uint8 *destVertexPtr, 
	CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf)
#else
void		CMeshMRMGeom::applyArrayRawSkinNormal3(CRawVertexNormalSkin4 *src, uint8 *destVertexPtr, 
	CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf)
#endif
{
	for(;nInf>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertexNormal4, nInf);
		// next block.
		nInf-= nBlockInf;

	#ifdef NL_SKIN_SSE
		// cache the data in L1 cache.
		CFastMem::precacheSSE(src, nBlockInf * sizeof(CRawVertexNormalSkin4));
	#else
		// slower precache but still usefull
		if( CSystemInfo::hasMMX() )
			CFastMem::precacheMMX(src, nBlockInf * sizeof(CRawVertexNormalSkin4));
	#endif

		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++)
		{
			uint8				*dstVertexVB= destVertexPtr + src->VertexId * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + NL3D_RAWSKIN_NORMAL_OFF);

			// Vertex.
			boneMat3x4[ src->SkinWeight.MatrixId[0] ].mulSetPoint( src->Vertex, src->SkinWeight.Weights[0], *dstVertex);
			boneMat3x4[ src->SkinWeight.MatrixId[1] ].mulAddPoint( src->Vertex, src->SkinWeight.Weights[1], *dstVertex);
			boneMat3x4[ src->SkinWeight.MatrixId[2] ].mulAddPoint( src->Vertex, src->SkinWeight.Weights[2], *dstVertex);
			// Normal.
			boneMat3x4[ src->SkinWeight.MatrixId[0] ].mulSetVector( src->Normal, src->SkinWeight.Weights[0], *dstNormal);
			boneMat3x4[ src->SkinWeight.MatrixId[1] ].mulAddVector( src->Normal, src->SkinWeight.Weights[1], *dstNormal);
			boneMat3x4[ src->SkinWeight.MatrixId[2] ].mulAddVector( src->Normal, src->SkinWeight.Weights[2], *dstNormal);
		}
	}
}

// ***************************************************************************
#ifdef NL_SKIN_SSE
void		CMeshMRMGeom::applyArrayRawSkinNormal4(CRawVertexNormalSkin4 *src, uint8 *destVertexPtr, 
	CMatrix3x4SSE *boneMat3x4, uint vertexSize, uint nInf)
#else
void		CMeshMRMGeom::applyArrayRawSkinNormal4(CRawVertexNormalSkin4 *src, uint8 *destVertexPtr, 
	CMatrix3x4 *boneMat3x4, uint vertexSize, uint nInf)
#endif
{
	for(;nInf>0;)
	{
		// number of vertices to process for this block.
		uint	nBlockInf= min(NumCacheVertexNormal4, nInf);
		// next block.
		nInf-= nBlockInf;

	#ifdef NL_SKIN_SSE
		// cache the data in L1 cache.
		CFastMem::precacheSSE(src, nBlockInf * sizeof(CRawVertexNormalSkin4));
	#else
		// slower precache but still usefull
		if( CSystemInfo::hasMMX() )
			CFastMem::precacheMMX(src, nBlockInf * sizeof(CRawVertexNormalSkin4));
	#endif

		//  for all InfluencedVertices only.
		for(;nBlockInf>0;nBlockInf--, src++)
		{
			uint8				*dstVertexVB= destVertexPtr + src->VertexId * vertexSize;
			CVector				*dstVertex= (CVector*)(dstVertexVB);
			CVector				*dstNormal= (CVector*)(dstVertexVB + NL3D_RAWSKIN_NORMAL_OFF);

			// Vertex.
			boneMat3x4[ src->SkinWeight.MatrixId[0] ].mulSetPoint( src->Vertex, src->SkinWeight.Weights[0], *dstVertex);
			boneMat3x4[ src->SkinWeight.MatrixId[1] ].mulAddPoint( src->Vertex, src->SkinWeight.Weights[1], *dstVertex);
			boneMat3x4[ src->SkinWeight.MatrixId[2] ].mulAddPoint( src->Vertex, src->SkinWeight.Weights[2], *dstVertex);
			boneMat3x4[ src->SkinWeight.MatrixId[3] ].mulAddPoint( src->Vertex, src->SkinWeight.Weights[3], *dstVertex);
			// Normal.
			boneMat3x4[ src->SkinWeight.MatrixId[0] ].mulSetVector( src->Normal, src->SkinWeight.Weights[0], *dstNormal);
			boneMat3x4[ src->SkinWeight.MatrixId[1] ].mulAddVector( src->Normal, src->SkinWeight.Weights[1], *dstNormal);
			boneMat3x4[ src->SkinWeight.MatrixId[2] ].mulAddVector( src->Normal, src->SkinWeight.Weights[2], *dstNormal);
			boneMat3x4[ src->SkinWeight.MatrixId[3] ].mulAddVector( src->Normal, src->SkinWeight.Weights[3], *dstNormal);
		}
	}
}


// ***************************************************************************
#ifdef NL_SKIN_SSE
void	CMeshMRMGeom::applyRawSkinWithNormalSSE(CLod &lod, CRawSkinNormalCache &rawSkinLod, const CSkeletonModel *skeleton)
#else
void	CMeshMRMGeom::applyRawSkinWithNormal(CLod &lod, CRawSkinNormalCache &rawSkinLod, const CSkeletonModel *skeleton)
#endif
{
	nlassert(_Skinned);
	if(_SkinWeights.size()==0)
		return;

	// get vertexPtr / normalOff.
	//===========================
	uint8		*destVertexPtr= (uint8*)_VBufferFinal.getVertexCoordPointer();
	uint		flags= _VBufferFinal.getVertexFormat();
	sint32		vertexSize= _VBufferFinal.getVertexSize();
	// must have XYZ and Normal.
	nlassert((flags & CVertexBuffer::PositionFlag) 
			 && (flags & CVertexBuffer::NormalFlag) 
			);


	// Compute offset of each component of the VB.
	sint32		normalOff;
	normalOff= _VBufferFinal.getNormalOff();
	// HardCoded for normalOff==12 (see applyArrayRawSkinNormal*)
	nlassert(normalOff==NL3D_RAWSKIN_NORMAL_OFF);


	// Compute usefull Matrix for this lod.
	//===========================
	// Those arrays map the array of bones in skeleton.
	static	NL_SKIN_MATRIX_ARRAY			boneMat3x4;
	computeBoneMatrixes3x4(boneMat3x4, lod.MatrixInfluences, skeleton);


	// apply skinning.
	//===========================
	// assert, code below is written especially for 4 per vertex.
	nlassert(NL3D_MESH_SKINNING_MAX_MATRIX==4);
	uint	nInf;
	// apply the skin to the vertices

	// 1 Matrix
	nInf= rawSkinLod.Vertices1.size();
	if(nInf>0)
		applyArrayRawSkinNormal1(&rawSkinLod.Vertices1[0], destVertexPtr, &boneMat3x4[0], vertexSize, nInf);
	// 2 Matrix
	nInf= rawSkinLod.Vertices2.size();
	if(nInf>0)
		applyArrayRawSkinNormal2(&rawSkinLod.Vertices2[0], destVertexPtr, &boneMat3x4[0], vertexSize, nInf);
	// 3 Matrix
	nInf= rawSkinLod.Vertices3.size();
	if(nInf>0)
		applyArrayRawSkinNormal3(&rawSkinLod.Vertices3[0], destVertexPtr, &boneMat3x4[0], vertexSize, nInf);
	// 4 Matrix
	nInf= rawSkinLod.Vertices4.size();
	if(nInf>0)
		applyArrayRawSkinNormal4(&rawSkinLod.Vertices4[0], destVertexPtr, &boneMat3x4[0], vertexSize, nInf);
}



