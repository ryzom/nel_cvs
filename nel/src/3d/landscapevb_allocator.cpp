/** \file landscapevb_allocator.cpp
 * <File description>
 *
 * $Id: landscapevb_allocator.cpp,v 1.1 2001/09/10 10:06:56 berenguier Exp $
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

#include "3d/landscapevb_allocator.h"
#include "3d/driver.h"


namespace NL3D 
{

/*
	Once a reallocation of a VBHard occurs, how many vertices we add to the re-allocation, to avoid 
	as possible reallocations.
*/
#define	NL3D_LANDSCAPE_VERTEX_ALLOCATE_SECURITY		1024
/*
	The start size of the array.
*/
#define	NL3D_LANDSCAPE_VERTEX_ALLOCATE_START		4048


#define	NL3D_VERTEX_FREE_MEMORY_RESERVE	1024
// 65000 is a maximum because of GeForce limitations.
#define	NL3D_VERTEX_MAX_VERTEX_VBHARD	40000


// ***************************************************************************
CLandscapeVBAllocator::CLandscapeVBAllocator(TType type)
{
	_Type= type;
	_VertexFreeMemory.reserve(NL3D_VERTEX_FREE_MEMORY_RESERVE);

	_ReallocationOccur= false;
	_NumVerticesAllocated= 0;
	_VBHardOk= false;
	_BufferLocked= false;

	if(_Type==Far0)
		// v3f/t2f
		_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
	else if(_Type==Far1)
		// v3f/t2f/c4ub
		_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::PrimaryColorFlag );
	else
		// v3f/t2f0/t2f1
		_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag);

}

// ***************************************************************************
CLandscapeVBAllocator::~CLandscapeVBAllocator()
{
	clear();
}


// ***************************************************************************
void			CLandscapeVBAllocator::updateDriver(IDriver *driver)
{
	// test change of driver.
	nlassert(driver);
	if( _Driver==NULL || driver!=_Driver )
	{
		deleteVertexBuffer();
		_Driver= driver;
		_VBHardOk= _Driver->supportVertexBufferHard();

		// must reallocate the VertexBuffer.
		if( _NumVerticesAllocated>0 )
			allocateVertexBuffer(_NumVerticesAllocated);
	}
	else
	{
		// if VBHard possible, and if vbHardDeleted but space needed, reallocate.
		if( _VBHardOk && _VBHard==NULL && _NumVerticesAllocated>0 )
			allocateVertexBuffer(_NumVerticesAllocated);
	}

}


// ***************************************************************************
void			CLandscapeVBAllocator::clear()
{
	// clear list.
	_VertexFreeMemory.clear();
	_NumVerticesAllocated= 0;

	// delete the VB.
	deleteVertexBuffer();

	// clear other states.
	_ReallocationOccur= false;
	_Driver= NULL;
	_VBHardOk= false;
}



// ***************************************************************************
void			CLandscapeVBAllocator::resetReallocation()
{
	_ReallocationOccur= false;
}



// ***************************************************************************
// ***************************************************************************
// allocation.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
uint			CLandscapeVBAllocator::allocateVertex()
{
	// if no more free, allocate.
	if( _VertexFreeMemory.size()==0 )
	{
		// enlarge capacity.
		uint	newResize;
		if(_NumVerticesAllocated==0)
			newResize= NL3D_LANDSCAPE_VERTEX_ALLOCATE_START;
		else
			newResize= NL3D_LANDSCAPE_VERTEX_ALLOCATE_SECURITY;
		_NumVerticesAllocated+= newResize;
		// re-allocate VB.
		allocateVertexBuffer(_NumVerticesAllocated);
		// resize infos on vertices.
		_VertexInfos.resize(_NumVerticesAllocated);

		// Fill list of free elements.
		for(uint i=0;i<newResize;i++)
		{
			// create a new entry which points to this vertex.
			// the list is made so allocation is in growing order.
			_VertexFreeMemory.push_back( _NumVerticesAllocated - (i+1) );

			// Mark as free the new vertices. (Debug).
			_VertexInfos[_NumVerticesAllocated - (i+1)].Free= true;
		}
	}

	// get a vertex (pop_back).
	uint	id= _VertexFreeMemory.back();
	// delete this vertex free entry.
	_VertexFreeMemory.pop_back();

	// check and Mark as not free the vertex. (Debug).
	nlassert(id<_NumVerticesAllocated);
	nlassert(_VertexInfos[id].Free);
	_VertexInfos[id].Free= false;

	return id;
}

// ***************************************************************************
void			CLandscapeVBAllocator::deleteVertex(uint vid)
{
	// check and Mark as free the vertex. (Debug).
	nlassert(vid<_NumVerticesAllocated);
	nlassert(!_VertexInfos[vid].Free);
	_VertexInfos[vid].Free= true;

	// Add this vertex to the free list.
	// create a new entry which points to this vertex.
	_VertexFreeMemory.push_back( vid );
}


// ***************************************************************************
void			CLandscapeVBAllocator::lockBuffer(CFarVertexBufferInfo &farVB)
{
	nlassert( _Type==Far0 || _Type==Far1 );
	// force unlock
	unlockBuffer();

	if(_VBHard)
	{
		void	*data= _VBHard->lock();
		farVB.setupVertexBufferHard(*_VBHard, data);
	}
	else
	{
		farVB.setupVertexBuffer(_VB);
	}

	_BufferLocked= true;
}
// ***************************************************************************
void			CLandscapeVBAllocator::lockBuffer(CNearVertexBufferInfo &tileVB)
{
	nlassert(_Type==Tile);
	// force unlock
	unlockBuffer();

	if(_VBHard)
	{
		void	*data= _VBHard->lock();
		tileVB.setupVertexBufferHard(*_VBHard, data);
	}
	else
	{
		tileVB.setupVertexBuffer(_VB);
	}

	_BufferLocked= true;
}
// ***************************************************************************
void			CLandscapeVBAllocator::unlockBuffer()
{
	if(_BufferLocked)
	{
		if(_VBHard)
			_VBHard->unlock();
		_BufferLocked= false;
	}
}


// ***************************************************************************
// ***************************************************************************
// VertexBuffer mgt.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLandscapeVBAllocator::activate()
{
	nlassert(_Driver);
	nlassert(!_BufferLocked);

	if(_VBHard)
		_Driver->activeVertexBufferHard(_VBHard);
	else
		_Driver->activeVertexBuffer(_VB);
}


// ***************************************************************************
void				CLandscapeVBAllocator::deleteVertexBuffer()
{
	// must unlock VBhard before.
	unlockBuffer();

	// test (refptr) if the object still exist in memory.
	if(_VBHard!=NULL)
	{
		// A vbufferhard should still exist only if driver still exist.
		nlassert(_Driver!=NULL);

		// delete it from driver.
		_Driver->deleteVertexBufferHard(_VBHard);
		_VBHard= NULL;
	}

	// delete the soft one.
	_VB.deleteAllVertices();
}


// ***************************************************************************
void				CLandscapeVBAllocator::allocateVertexBuffer(uint32 numVertices)
{
	// no allocation must be done if the Driver is not setuped, or if the driver has been deleted by refPtr.
	nlassert(_Driver);

	// allocate() =>_ReallocationOccur= true;
	_ReallocationOccur= true;
	// must unlock VBhard before.
	unlockBuffer();

	// trye to allocate a vbufferhard if possible.
	if( _VBHardOk )
	{
		// delete possible old _VBHard.
		if(_VBHard!=NULL)
		{
			// VertexBufferHard lifetime < Driver lifetime.
			nlassert(_Driver!=NULL);
			_Driver->deleteVertexBufferHard(_VBHard);
		}

		// try to create new one, in AGP Ram
		// If too many vertices wanted, abort VBHard.
		if(numVertices <= NL3D_VERTEX_MAX_VERTEX_VBHARD)
			_VBHard= _Driver->createVertexBufferHard(_VB.getVertexFormat(), _VB.getValueTypePointer(), numVertices, IDriver::VBHardAGP);
		else
			_VBHard= NULL;

		// If KO, never try again.
		if(_VBHard==NULL)
			_VBHardOk= false;
	}

	// else, or if last fails, allocate a standard VB.
	if(!_VBHardOk)
	{
		// This always works.
		_VB.setNumVertices(numVertices);
	}


	//nlinfo("Alloc a LandVB %s of %d vertices in %s", _Type==Far0?"Far0":(_Type==Far1?"Far1":"Tile"), numVertices, _VBHardOk?"VBHard":"VBSoft");
}



} // NL3D
