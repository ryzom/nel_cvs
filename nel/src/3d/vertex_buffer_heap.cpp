/** \file vertex_buffer_heap.cpp
 * TODO: File description
 *
 * $Id: vertex_buffer_heap.cpp,v 1.8 2005/02/22 10:19:13 besson Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "vertex_buffer_heap.h"
#include "nel/misc/common.h"

using namespace NLMISC;

// This code is not used actually and doesn't compile
// just preproc comment it
#if 0

namespace NL3D
{

// ***************************************************************************
CVertexBufferHeap::CVertexBufferHeap()
{
	_Enabled= false;
	_HardMode= false;

	_HeapStart= NULL;
	_VertexFormat= 0;
	_VertexSize= 0;
	_MaxVertices= 0;
}

// ***************************************************************************
CVertexBufferHeap::~CVertexBufferHeap()
{
	release();
}

// ***************************************************************************
void			CVertexBufferHeap::init(IDriver *driver, uint vertexFormat, uint maxVertices)
{
	nlassert(driver);
	// clean before.
	release();

	// setup
	_Driver= driver;

	// setup the vertexBuffer soft with queried info.
	_VBSoft.setVertexFormat(vertexFormat);
	// VertexSize must be a multitple of 4 (Heap alignement ...)
	nlassert( (_VBSoft.getVertexSize()&3) == 0);

	// create the VBHard, if possible
	_VBHard= driver->createVertexBufferHard(_VBSoft.getVertexFormat(), _VBSoft.getValueTypePointer(), maxVertices, IDriver::VBHardAGP, _VBSoft.getUVRouting());

	// Ok
	_Enabled= true;
	_VertexFormat= _VBSoft.getVertexFormat();
	_VertexSize= _VBSoft.getVertexSize();
	_MaxVertices= maxVertices;

	// Use hard or soft ???
	if(_VBHard)
	{
		_HardMode= true;
		// setup heap start with good AGP ptr.
		_HeapStart= (uint8*)_VBHard->lock();
		// just a gestion lock, no vertices have changed.
		_VBHard->unlock(0,0);
		// In essence, the VBHeap is rarely modified (but on mesh loading). => set it static
		_VBHard->lockHintStatic(true);
	}
	else
	{
		_HardMode= false;
		// => allocate soft one.
		_VBSoft.setNumVertices(maxVertices);
		// setup heap start with good ptr.
		_HeapStart= (uint8*)_VBSoft.getVertexCoordPointer();
	}

	// setup heap Manager with good ptr.
	_HeapManager.initHeap(_HeapStart, _MaxVertices*_VertexSize);
}
// ***************************************************************************
void			CVertexBufferHeap::release()
{
	if(_VBHard)
	{
		nlassert(_Driver);
		_Driver->deleteVertexBufferHard(_VBHard);
	}
	_VBHard= NULL;
	_Driver= NULL;
	_HeapStart= NULL;
	// release all memory
	contReset(_VBSoft);
	contReset(_HeapManager);

	_Enabled= false;
	_HardMode= false;
	_VertexFormat= 0;
	_VertexSize= 0;
	_MaxVertices= 0;
}

// ***************************************************************************
bool			CVertexBufferHeap::allocate(uint numVertices, uint &indexStart)
{
	nlassert(enabled());

	// allocate into the heap ?
	uint8	*ptr= (uint8*)_HeapManager.allocate(numVertices*getVertexSize());
	if(!ptr)
		return false;
	else
	{
		// compute vertex index
		indexStart= (uint)(ptr-_HeapStart);
		indexStart/= _VertexSize;
		return true;
	}
}
// ***************************************************************************
void			CVertexBufferHeap::free(uint indexStart)
{
	nlassert(enabled());

	// compute ptr to free
	uint8	*ptr= _HeapStart + indexStart*_VertexSize;
	// free it.
	_HeapManager.free(ptr);
}

// ***************************************************************************
uint8			*CVertexBufferHeap::lock(uint indexStart)
{
	nlassert(enabled());

	uint8	*ptr;
	if(_HardMode)
	{
		// lock the vbHard
		ptr= (uint8*)_VBHard->lock();
		nlassert(ptr==_HeapStart);
	}
	else
		ptr= _HeapStart;

	// index it with index
	return ptr + indexStart*_VertexSize;
}
// ***************************************************************************
void			CVertexBufferHeap::unlock(uint startVert, uint endVert)
{
	nlassert(enabled());

	if(_HardMode)
		_VBHard->unlock(startVert, endVert);
}
// ***************************************************************************
void			CVertexBufferHeap::activate()
{
	nlassert(enabled());

	if(_HardMode)
		_Driver->activeVertexBufferHard(_VBHard);
	else
		_Driver->activeVertexBuffer(_VBSoft);
}


} // NL3D

#endif // 0
