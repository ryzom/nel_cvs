/** \file landscape_face_vector_manager.cpp
 * <File description>
 *
 * $Id: landscape_face_vector_manager.cpp,v 1.2 2002/02/28 12:59:49 besson Exp $
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

#include "3d/landscape_face_vector_manager.h"
#include "nel/misc/debug.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


// ***************************************************************************
#define	NL3D_FACE_VECTOR_NUMBLOCK	33


// ***************************************************************************
CLandscapeFaceVector::CLandscapeFaceVector()
{
	TriPtr= NULL;
	NumTri= 0;
	_BlockId= -1;
	_Next= NULL;
}
// ***************************************************************************
CLandscapeFaceVector::CLandscapeFaceVector(const CLandscapeFaceVector &o)
{
	nlstop;
}
// ***************************************************************************
CLandscapeFaceVector &CLandscapeFaceVector::operator=(const CLandscapeFaceVector &o)
{
	nlstop;
	return *this;
}
// ***************************************************************************
CLandscapeFaceVector::~CLandscapeFaceVector()
{
	free(TriPtr);
}



// ***************************************************************************
CLandscapeFaceVectorManager::CLandscapeFaceVectorManager()
{
	// Allow 2^32 triangles at max. each list i has at max 2^i triangles.
	_Blocks.resize(NL3D_FACE_VECTOR_NUMBLOCK, NULL);
}

// ***************************************************************************
CLandscapeFaceVectorManager::~CLandscapeFaceVectorManager()
{
	purge();
}

// ***************************************************************************
void					CLandscapeFaceVectorManager::purge()
{
	for(uint i=0; i<NL3D_FACE_VECTOR_NUMBLOCK; i++)
	{
		CLandscapeFaceVector	*ptr= _Blocks[i];
		// For each node in list, delete.
		while(ptr)
		{
			CLandscapeFaceVector	*next= ptr->_Next;
			delete ptr;
			ptr= next;
		}
		// list is empty.
		_Blocks[i]= NULL;
	}
}

// ***************************************************************************
CLandscapeFaceVector	*CLandscapeFaceVectorManager::createFaceVector(uint numTri)
{
	uint	blockId= getPowerOf2(numTri);

	// If no more free FaceVector, allocate.
	if(_Blocks[blockId]==NULL)
	{
		_Blocks[blockId]= new CLandscapeFaceVector;
		// For debug, good filling only at creation.
		_Blocks[blockId]->_BlockId= -1;
		uint	numTriMax= 1<<blockId;
		// allocate the index buffer.
		_Blocks[blockId]->TriPtr= (uint32*)malloc(numTriMax*3* sizeof(uint32));
	}

	// Pop a FaceVector from the free list.
	CLandscapeFaceVector	*ret= _Blocks[blockId];
	_Blocks[blockId]= ret->_Next;
	ret->_Next= NULL;

	// There is numTri triangles.
	ret->NumTri= numTri;
	// Mark as allcoated.
	ret->_BlockId= blockId;

	return ret;
}

// ***************************************************************************
void					CLandscapeFaceVectorManager::deleteFaceVector(CLandscapeFaceVector	*fv)
{
	// check if not deleted.
	nlassert(fv->_BlockId>=0);

	// Append this block to the free list.
	fv->_Next= _Blocks[fv->_BlockId];
	_Blocks[fv->_BlockId]= fv;

	// Mark as deleted.
	fv->_BlockId= -1;
}


} // NL3D
