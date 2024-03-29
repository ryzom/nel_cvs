/** \file contiguous_block_allocator.cpp
 *
 * $Id: contiguous_block_allocator.cpp,v 1.5 2004/11/15 10:25:03 lecroart Exp $
 */

/* Copyright, 2000, 2001, 2002, 2003 Nevrax Ltd.
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

#include "stdmisc.h"
#include "nel/misc/contiguous_block_allocator.h"

namespace NLMISC
{

//*********************************************************************************************************
CContiguousBlockAllocator::CContiguousBlockAllocator()
{
	_BlockStart = NULL;
	_NextAvailablePos = NULL;
	_BlockEnd = 0;
	_NumAllocatedBytes = 0;
	#ifdef NL_DEBUG
		_NumAlloc = 0;
		_NumFree = 0;
	#endif
}

//*********************************************************************************************************
CContiguousBlockAllocator::~CContiguousBlockAllocator()
{
	init(0);
}

//*********************************************************************************************************
void CContiguousBlockAllocator::init(uint numBytes /*=0*/)
{
	if (_BlockStart) _DefaultAlloc.deallocate(_BlockStart, _BlockEnd - _BlockStart);
	_BlockEnd =  NULL;
	_BlockStart = NULL;
	_NumAllocatedBytes = 0;
	_NextAvailablePos = NULL;
	if (numBytes != 0)
	{
		_BlockStart = _DefaultAlloc.allocate(numBytes);
		_NextAvailablePos = _BlockStart;
		_BlockEnd = _BlockStart + numBytes;
		_NumAllocatedBytes = 0;
	}
	#ifdef NL_DEBUG
		_NumAlloc = 0;
		_NumFree = 0;
	#endif
}

//*********************************************************************************************************
void *CContiguousBlockAllocator::alloc(uint numBytes)
{
	if (numBytes == 0) return NULL;
	_NumAllocatedBytes += numBytes;
	if (_BlockStart)
	{
		if (_NextAvailablePos + numBytes <= _BlockEnd)
		{
			uint8 *block = _NextAvailablePos;
			_NextAvailablePos += numBytes;
			#ifdef NL_DEBUG
				++ _NumAlloc;
			#endif
			return block;
		}
	}
	// just uses standard new
	#ifdef NL_DEBUG
		++ _NumAlloc;
	#endif
	return _DefaultAlloc.allocate(numBytes);
}

//*********************************************************************************************************
void CContiguousBlockAllocator::free(void *block, uint numBytes)
{
	if (!block) return;
	#ifdef NL_DEBUG
		++ _NumFree;
	#endif
	// no-op if block not inside the big block (sub-block are never deallocated until init(0) is encountered)
	if (block < _BlockStart || block >= _BlockEnd)
	{
		// the block was allocated with std allocator
		_DefaultAlloc.deallocate((uint8 *) block, numBytes);
	}
}

} // NLMISC
