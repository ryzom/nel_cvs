/** \file patch_rdr_pass.cpp
 * <File description>
 *
 * $Id: patch_rdr_pass.cpp,v 1.4 2001/01/08 17:58:30 corvazier Exp $
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

#include "nel/3d/patch_rdr_pass.h"
#include "nel/3d/primitive_block.h"


namespace NL3D 
{


// ***************************************************************************
// Primitives Indices reallocation. must be >16 (see below..)
static sint		GlobalTriListBlockRealloc= 1024;


// ***************************************************************************
sint			CPatchRdrPass::CurGlobalIndex=0;
std::vector<uint32>	CPatchRdrPass::GlobalTriList;

	
// ***************************************************************************
CPatchRdrPass::CPatchRdrPass()
{
	resetTriList();
	RefCount= 0;
	BlendType= Alpha;
}

// ***************************************************************************
void			CPatchRdrPass::resetTriList()
{
	NTris=0;
	StartIndex=0;
	CurIndex=0;
	BlockLenIndex=0;
}

// ***************************************************************************
void			CPatchRdrPass::addTri(uint32 idx0, uint32 idx1, uint32 idx2)
{
	// An error may occurs if resetGlobalTriList() called, but not resetTriList().
	nlassert(CurIndex<=CurGlobalIndex);

	// Realloc if necessary.
	// Keep a security of 16 spaces (think that 5/6 only is needed).
	// Nb: 6, because we need one more space for the last JMP of the last block of the last material...
	// Don't bother, 16 is cool....
	if((sint)GlobalTriList.size() < CurGlobalIndex+16)
	{
		GlobalTriList.resize(GlobalTriList.size() + NL3D::GlobalTriListBlockRealloc);
	}

	// First, if current material interleaved, jump.
	if(CurIndex!=CurGlobalIndex)
	{
		// Leave a "jump" space for old material.
		CurGlobalIndex++;
		// This material "jump" to current index.
		if(NTris!=0)	// Only if the list is not empty.
		{
			// Old block (with CurIndex which points to the jump space) must point to cur.
			GlobalTriList[CurIndex]= CurGlobalIndex;
			// Start of a new block!!
			BlockLenIndex= CurGlobalIndex++;
			GlobalTriList[BlockLenIndex]=0;
		}
	}

	// insert!!
	if(NTris==0)
	{
		// Start of a new block!!
		StartIndex= BlockLenIndex= CurGlobalIndex++;
		GlobalTriList[BlockLenIndex]=0;
	}
	GlobalTriList[CurGlobalIndex++]= idx0;
	GlobalTriList[CurGlobalIndex++]= idx1;
	GlobalTriList[CurGlobalIndex++]= idx2;

	// CurIndex point to the next global index...
	CurIndex= CurGlobalIndex;
	NTris++;
	GlobalTriList[BlockLenIndex]++;
}

// ***************************************************************************
void			CPatchRdrPass::buildPBlock(CPrimitiveBlock &pb) const
{
	sint	idx, n, blocklen;

	pb.setNumTri(NTris);
	uint32	*pi= pb.getTriPointer();

	// Run the list of block.
	n= NTris;
	idx= StartIndex;
	while(n>0)
	{
		// size of block (in tris).
		blocklen= GlobalTriList[idx];
		// Copy the indices (jump the BlockLenIndex).
		memcpy(pi, &GlobalTriList[idx+1], blocklen*3*sizeof(uint32));
		// Jump to the next block!! (not valid if last block, but doesn't matter...)
		idx= GlobalTriList[idx+1+blocklen*3];
		pi+= blocklen*3;
		n-= blocklen;
	}
}

// ***************************************************************************
void			CPatchRdrPass::resetGlobalTriList()
{
	CurGlobalIndex= 0;
}





} // NL3D
