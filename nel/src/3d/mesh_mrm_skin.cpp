/** \file mesh_mrm_skin.cpp
 * Skin computation part for class CMeshMRM.
 *
 * $Id: mesh_mrm_skin.cpp,v 1.3 2002/04/10 15:57:02 berenguier Exp $
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
#include "3d/fast_mem.h"
#include "nel/misc/cpu_info.h"


using namespace NLMISC;
using namespace std;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// STD Matrix
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
// For fast vector/point multiplication.
class	CMatrix3x4
{
public:
	// Order them in memory line first, for faster memory access.
	float	a11, a12, a13, a14;
	float	a21, a22, a23, a24;
	float	a31, a32, a33, a34;

	// Copy from a matrix.
	void	set(const CMatrix &mat)
	{
		const float	*m =mat.get();
		a11= m[0]; a12= m[4]; a13= m[8] ; a14= m[12]; 
		a21= m[1]; a22= m[5]; a23= m[9] ; a24= m[13]; 
		a31= m[2]; a32= m[6]; a33= m[10]; a34= m[14]; 
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &in, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z);
		out.y= (a21*in.x + a22*in.y + a23*in.z);
		out.z= (a31*in.x + a32*in.y + a33*in.z);
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &in, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z + a14);
		out.y= (a21*in.x + a22*in.y + a23*in.z + a24);
		out.z= (a31*in.x + a32*in.y + a33*in.z + a34);
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &in, float scale, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z) * scale;
		out.y= (a21*in.x + a22*in.y + a23*in.z) * scale;
		out.z= (a31*in.x + a32*in.y + a33*in.z) * scale;
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &in, float scale, CVector &out)
	{
		out.x= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
		out.y= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
		out.z= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
	}


	// mulAddvector. NB: in should be different as v!! (else don't work).
	void	mulAddVector(const CVector &in, float scale, CVector &out)
	{
		out.x+= (a11*in.x + a12*in.y + a13*in.z) * scale;
		out.y+= (a21*in.x + a22*in.y + a23*in.z) * scale;
		out.z+= (a31*in.x + a32*in.y + a33*in.z) * scale;
	}
	// mulAddpoint. NB: in should be different as v!! (else don't work).
	void	mulAddPoint(const CVector &in, float scale, CVector &out)
	{
		out.x+= (a11*in.x + a12*in.y + a13*in.z + a14) * scale;
		out.y+= (a21*in.x + a22*in.y + a23*in.z + a24) * scale;
		out.z+= (a31*in.x + a32*in.y + a33*in.z + a34) * scale;
	}



};


// ***************************************************************************
// ***************************************************************************
// SSE Matrix
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
#ifdef NL_OS_WINDOWS

//#define NL_DebugSSE
//#define NL_DebugSSENoSkin


// For fast vector/point multiplication.
class	CMatrix3x4SSE
{
public:
	// Order them in memory column first, for SSE column multiplication.
	float	a11, a21, a31, a41;
	float	a12, a22, a32, a42;
	float	a13, a23, a33, a43;
	float	a14, a24, a34, a44;

	// Copy from a matrix.
	void	set(const CMatrix &mat)
	{
		const float	*m =mat.get();
		a11= m[0]; a12= m[4]; a13= m[8] ; a14= m[12]; 
		a21= m[1]; a22= m[5]; a23= m[9] ; a24= m[13]; 
		a31= m[2]; a32= m[6]; a33= m[10]; a34= m[14]; 
		// not used.
		//a41= 0   ; a42= 0   ; a43= 0    ; a44= 1; 
	}


	// mulSetvector. NB: in should be different as v!! (else don't work).
	void	mulSetVector(const CVector &vin, CVector &vout)
	{
	#ifndef NL_DebugSSE
		__asm
		{
			mov		eax, vin
			mov		ebx, this
			mov		edi, vout
			// Load in vector in op[0]
			movss	xmm0, [eax]vin.x
			movss	xmm1, [eax]vin.y
			movss	xmm2, [eax]vin.z
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			shufps	xmm2, xmm2, 0
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2

			// write the result.
			movss	[edi]vout.x, xmm0
			shufps	xmm0, xmm0, 33
			movss	[edi]vout.y, xmm0
			movhlps	xmm0, xmm0
			movss	[edi]vout.z, xmm0
		}
	#elif !defined (NL_DebugSSENoSkin)
		vout.x= (a11*vin.x + a12*vin.y + a13*vin.z);
		vout.y= (a21*vin.x + a22*vin.y + a23*vin.z);
		vout.z= (a31*vin.x + a32*vin.y + a33*vin.z);
	#else
		vout= vin;
	#endif
	}
	// mulSetpoint. NB: in should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &vin, CVector &vout)
	{
	#ifndef NL_DebugSSE
		__asm
		{
			mov		eax, vin
			mov		ebx, this
			mov		edi, vout
			// Load in vector in op[0]
			movss	xmm0, [eax]vin.x
			movss	xmm1, [eax]vin.y
			movss	xmm2, [eax]vin.z
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			shufps	xmm2, xmm2, 0
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2
			// Add Matrix translate column vector
			addps	xmm0, [ebx]this.a14

			// write the result.
			movss	[edi]vout.x, xmm0
			shufps	xmm0, xmm0, 33
			movss	[edi]vout.y, xmm0
			movhlps	xmm0, xmm0
			movss	[edi]vout.z, xmm0
		}
	#elif !defined (NL_DebugSSENoSkin)
		vout.x= (a11*vin.x + a12*vin.y + a13*vin.z + a14);
		vout.y= (a21*vin.x + a22*vin.y + a23*vin.z + a24);
		vout.z= (a31*vin.x + a32*vin.y + a33*vin.z + a34);
	#else
		vout= vin;
	#endif
	}


	// mulSetvector. NB: vin should be different as v!! (else don't work).
	void	mulSetVector(const CVector &vin, float scale, CVector &vout)
	{
	#ifndef NL_DebugSSE
		__asm
		{
			mov		eax, vin
			mov		ebx, this
			mov		edi, vout
			// Load in vector in op[0]
			movss	xmm0, [eax]vin.x
			movss	xmm1, [eax]vin.y
			movss	xmm2, [eax]vin.z
			// Load scale in op[0]
			movss	xmm3, scale
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			shufps	xmm2, xmm2, 0
			shufps	xmm3, xmm3, 0
			// Store vertex column in other regs.
			movaps	xmm5, xmm0
			movaps	xmm6, xmm1
			movaps	xmm7, xmm2
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2

			// mul final result with scale
			mulps	xmm0, xmm3

			// store it in xmm4 for future use.
			movaps	xmm4, xmm0
		}
	#elif !defined (NL_DebugSSENoSkin)
		vout.x= (a11*vin.x + a12*vin.y + a13*vin.z) * scale;
		vout.y= (a21*vin.x + a22*vin.y + a23*vin.z) * scale;
		vout.z= (a31*vin.x + a32*vin.y + a33*vin.z) * scale;
	#else
		vout= vin;
	#endif
	}
	// mulSetpoint. NB: vin should be different as v!! (else don't work).
	void	mulSetPoint(const CVector &vin, float scale, CVector &vout)
	{
	#ifndef NL_DebugSSE
		__asm
		{
			mov		eax, vin
			mov		ebx, this
			mov		edi, vout
			// Load in vector in op[0]
			movss	xmm0, [eax]vin.x
			movss	xmm1, [eax]vin.y
			movss	xmm2, [eax]vin.z
			// Load scale in op[0]
			movss	xmm3, scale
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm0, xmm0, 0
			shufps	xmm1, xmm1, 0
			shufps	xmm2, xmm2, 0
			shufps	xmm3, xmm3, 0
			// Store vertex column in other regs.
			movaps	xmm5, xmm0
			movaps	xmm6, xmm1
			movaps	xmm7, xmm2
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2
			// Add Matrix translate column vector
			addps	xmm0, [ebx]this.a14

			// mul final result with scale
			mulps	xmm0, xmm3

			// store it in xmm4 for future use.
			movaps	xmm4, xmm0
		}
	#elif !defined (NL_DebugSSENoSkin)
		vout.x= (a11*vin.x + a12*vin.y + a13*vin.z + a14) * scale;
		vout.y= (a21*vin.x + a22*vin.y + a23*vin.z + a24) * scale;
		vout.z= (a31*vin.x + a32*vin.y + a33*vin.z + a34) * scale;
	#else
		vout= vin;
	#endif
	}


	// mulAddvector. NB: vin should be different as v!! (else don't work).
	void	mulAddVector(const CVector &vin, float scale, CVector &vout)
	{
	#ifndef NL_DebugSSE
		__asm
		{
			mov		ebx, this
			mov		edi, vout
			// Load vin vector loaded in mulSetVector
			movaps	xmm0, xmm5
			movaps	xmm1, xmm6
			movaps	xmm2, xmm7
			// Load scale in op[0]
			movss	xmm3, scale
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm3, xmm3, 0
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2

			// mul final result with scale
			mulps	xmm0, xmm3

			// Add result, with prec sum.
			addps	xmm0, xmm4

			// store it in xmm4 for future use.
			movaps	xmm4, xmm0

			// write the result.
			movss	[edi]vout.x, xmm0
			shufps	xmm0, xmm0, 33
			movss	[edi]vout.y, xmm0
			movhlps	xmm0, xmm0
			movss	[edi]vout.z, xmm0
		}
	#elif !defined (NL_DebugSSENoSkin)
		vout.x+= (a11*vin.x + a12*vin.y + a13*vin.z) * scale;
		vout.y+= (a21*vin.x + a22*vin.y + a23*vin.z) * scale;
		vout.z+= (a31*vin.x + a32*vin.y + a33*vin.z) * scale;
	#else
		vout= vin;
	#endif
	}
	// mulAddpoint. NB: vin should be different as v!! (else don't work).
	void	mulAddPoint(const CVector &vin, float scale, CVector &vout)
	{
	#ifndef NL_DebugSSE
		__asm
		{
			mov		ebx, this
			mov		edi, vout
			// Load vin vector loaded in mulSetPoint
			movaps	xmm0, xmm5
			movaps	xmm1, xmm6
			movaps	xmm2, xmm7
			// Load scale in op[0]
			movss	xmm3, scale
			// Expand op[0] to op[1], op[2], op[3]
			shufps	xmm3, xmm3, 0
			// Mul each vector with 3 Matrix column
			mulps	xmm0, [ebx]this.a11
			mulps	xmm1, [ebx]this.a12
			mulps	xmm2, [ebx]this.a13
			// Add each column vector.
			addps	xmm0, xmm1
			addps	xmm0, xmm2
			// Add Matrix translate column vector
			addps	xmm0, [ebx]this.a14

			// mul final result with scale
			mulps	xmm0, xmm3

			// Add result, with prec sum.
			addps	xmm0, xmm4

			// store it in xmm4 for future use.
			movaps	xmm4, xmm0

			// write the result.
			movss	[edi]vout.x, xmm0
			shufps	xmm0, xmm0, 33
			movss	[edi]vout.y, xmm0
			movhlps	xmm0, xmm0
			movss	[edi]vout.z, xmm0
		}
	#elif !defined (NL_DebugSSENoSkin)
		vout.x+= (a11*vin.x + a12*vin.y + a13*vin.z + a14) * scale;
		vout.y+= (a21*vin.x + a22*vin.y + a23*vin.z + a24) * scale;
		vout.z+= (a31*vin.x + a32*vin.y + a33*vin.z + a34) * scale;
	#else
		vout= vin;
	#endif
	}

};

#else // NL_OS_WINDOWS
/// dummy CMatrix3x4SSE for non windows platform
class CMatrix3x4SSE : public  CMatrix3x4 { };
#endif


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
uint	CMeshMRMGeom::NumCacheVertexNormal1= NL_BlockByteL1 / sizeof(CMeshMRMGeom::CRawVertexNormalSkin1);
// Number of vertices per block to process with 2 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal2= NL_BlockByteL1 / sizeof(CMeshMRMGeom::CRawVertexNormalSkin2);
// Number of vertices per block to process with 3/4 matrix.
uint	CMeshMRMGeom::NumCacheVertexNormal4= NL_BlockByteL1 / sizeof(CMeshMRMGeom::CRawVertexNormalSkin4);

// Same for TgSpace Raw vertices
uint	CMeshMRMGeom::NumCacheVertexTgSpace1= NL_BlockByteL1 / sizeof(CMeshMRMGeom::CRawVertexTgSpaceSkin1);
uint	CMeshMRMGeom::NumCacheVertexTgSpace2= NL_BlockByteL1 / sizeof(CMeshMRMGeom::CRawVertexTgSpaceSkin2);
uint	CMeshMRMGeom::NumCacheVertexTgSpace4= NL_BlockByteL1 / sizeof(CMeshMRMGeom::CRawVertexTgSpaceSkin4);


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
void				CMeshMRMGeom::fillAGPSkinPart(CLod &lod)
{
	// Fill AGP vertices used by this lod from RAM. (not geomorphed ones).
	if(_VBHard && lod.SkinVertexBlocks.size()>0 )
	{
		// Get VB info, and lock buffers.
		uint8		*vertexSrc= (uint8*)_VBufferFinal.getVertexCoordPointer();
		uint8		*vertexDst= (uint8*)_VBHard->lock();
		uint32		vertexSize= _VBufferFinal.getVertexSize();
		nlassert(vertexSize == _VBHard->getVertexSize());

		// big copy of all vertices and their data.
		// NB: this not help RAM bandwidth, but this help AGP write combiners.
		// For the majority of mesh (vertex/normal/uv), this is better (6/10).


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


		_VBHard->unlock();
	}
}



} // NL3D

