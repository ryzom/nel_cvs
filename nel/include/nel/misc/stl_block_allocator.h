/** \file stl_block_allocator.h
 * <File description>
 *
 * $Id: stl_block_allocator.h,v 1.2 2001/12/27 14:31:47 berenguier Exp $
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

#ifndef NL_STL_BLOCK_ALLOCATOR_H
#define NL_STL_BLOCK_ALLOCATOR_H


#include "nel/misc/types_nl.h"
#include "nel/misc/block_memory.h"


namespace NLMISC {


// ***************************************************************************
/**
 * This class is a STL block allocator which use CBlockMemory. see CBlockMemory for description
 *	of block memory management/advantages.
 *
 * This class works with STLPort. It implements __stl_alloc_rebind() (not C++ standard??) to work properly
 *	with list<>/set<> etc... node allocations.
 *
 * NB: if used with a vector<> or a deque<> (or if allocate(..,n) is called with n>1), it's still work, 
 *	but it's use malloc()/free() instead.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
template<class T, size_t blockSize=16>
class CSTLBlockAllocator
{
public:

	/// \name standard allocator interface.
	// @{

	typedef T			value_type;
	typedef value_type *pointer;
	typedef const T*	const_pointer;
	typedef T&			reference;
	typedef const T&	const_reference;
	typedef size_t		size_type;
	typedef ptrdiff_t	difference_type;


	pointer	adress(reference x) const
	{
		return &x;
	}

	const_pointer	adress(const_reference x) const
	{
		return &x;
	}

	void	construct(pointer p, const T &val)
	{
		new (p) T(val);
	}

	void	destroy(pointer p)
	{
		p->T::~T();
	}

	// @}

public:

	/// Constructor
	CSTLBlockAllocator() : _BlockMemory(blockSize)
	{
	}
	/// copy ctor
	CSTLBlockAllocator(const CSTLBlockAllocator<T, blockSize> &other) : _BlockMemory(blockSize)
	{
		// don't copy any setup from the other allocator.
	}
	/// dtor
	~CSTLBlockAllocator()
	{
	}


	pointer	allocate(size_type n, const_pointer hint= NULL)
	{
		if(n==0)
			return NULL;
		// If sizeof 1, use CBlockMemory allocation
		if(n==1)
		{
			// verify that we allocate with good size!! (verify __stl_alloc_rebind scheme).
			// ie an allocator can be used only to allocate a kind of element
			uint	eltSize= std::max(sizeof(T), sizeof(void*));
			nlassert( eltSize == _BlockMemory.__stl_alloc_getEltSize() );
			// and allocate.
			return _BlockMemory.allocate();
		}
		// else use std malloc
		else
			return (T*)malloc(n*sizeof(T));
	}

	void	deallocate(void *p, size_type n)
	{
		if(n==0)
			return;
		// If sizeof 1, use CBlockMemory allocation
		if(n==1)
			_BlockMemory.free((T*)p);
		// else use std free
		else
			free(p);
	}


	template <class T, class U>
	CSTLBlockAllocator<U, blockSize>& __stl_alloc_rebind(CSTLBlockAllocator<T, blockSize>& __a, const U*)
	{
		// must change the internal eltSize of __a.
		__a._BlockMemory.__stl_alloc_changeEltSize(sizeof(U));
		// and just typecast/return him
		return (CSTLBlockAllocator<U, blockSize>&)(__a); 
	}

	template <class T, class U>
	CSTLBlockAllocator<U, blockSize> __stl_alloc_create(const CSTLBlockAllocator<T, blockSize>&, const U*)
	{
		return CSTLBlockAllocator<U, blockSize>();
	}



// *******************
private:

	// The blockMemory used to allocate elements
	CBlockMemory<T, false>		_BlockMemory;

};



} // NLMISC


#endif // NL_STL_BLOCK_ALLOCATOR_H

/* End of stl_block_allocator.h */
