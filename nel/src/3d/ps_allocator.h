/** \file ps_allocator.h
 *
 * $Id: ps_allocator.h,v 1.3 2004/03/19 10:45:19 vizerie Exp $
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

#ifndef NL_PS_ALLOCATOR_H
#define NL_PS_ALLOCATOR_H

#include "nel/misc/time_nl.h"
#include "nel/misc/contiguous_block_allocator.h"
//
#include <map>

namespace NL3D
{

#if	defined(NL_OS_WINDOWS) && !defined(NL_COMP_VC7)
	// fast mem alloc of particle systems only on windows for now
	//
	// NB : When using NL_MEMORY, we solve the redefinition of new errors as follow :	
	//
	// #if !defined (NL_USE_DEFAULT_MEMORY_MANAGER) && !defined (NL_NO_DEFINE_NEW)
	//	   #undef new
	// #endif
	// PS_FAST_ALLOC // for fast alloc
	// #if !defined (NL_USE_DEFAULT_MEMORY_MANAGER) && !defined (NL_NO_DEFINE_NEW)
	//	   #define new NL_NEW
	// #endif
	#define PS_FAST_ALLOC	
#endif


#ifndef PS_FAST_ALLOC	
	// the following macros, that redefines new & delete for classes of ps, is now empty
	// NB : When using NL_MEMORY, we solve the redefinition of new errors as follow :	
	//
	// #if !defined (NL_USE_DEFAULT_MEMORY_MANAGER) && !defined (NL_NO_DEFINE_NEW)
	//	   #undef new
	// #endif
	// PS_FAST_OBJ_ALLOC
	// #if !defined (NL_USE_DEFAULT_MEMORY_MANAGER) && !defined (NL_NO_DEFINE_NEW)
	//	   #define new NL_NEW
	// #endif
	#define PS_FAST_OBJ_ALLOC
	// Partial specialization tips for vectors. For non-windows version it just fallback on the default allocator
	// To define a vector of type T, one must do : CPSVector<T>::V MyVector;
	template <class T>
		struct CPSVector
	{
		typedef std::vector<T, std::allocator<T> > V;
	};
	// partial specialisation tips for multimap
	template <class T, class U, class Pr = std::less<T> >
		struct CPSMultiMap
	{
		typedef std::multimap<T, U, Pr, std::allocator<T> > M;
	};
	
#else

	// current contiguous block allocator to be used by the particle system allocator. std::allocator is used if no such allocator is provided
	extern NLMISC::CContiguousBlockAllocator *PSBlockAllocator;


	// a stl allocator for objects used in particle systems (vectors ..)
	template<class T>
	class CPSAllocator
	{
	public:
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T *pointer;
		typedef const T *const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		pointer address(reference x) const { return &x; }    
		const_pointer address(const_reference x) const { return &x; }
		CPSAllocator() {}
		CPSAllocator<T>& operator=(const CPSAllocator<T> &other) { *this = other; return *this; }
	private:
		typedef NLMISC::CContiguousBlockAllocator *TBlocAllocPtr;
	public:
		pointer allocate(size_type n, const void *hint) 
		{ 		
			++NumPSAlloc;
			TBlocAllocPtr *result;
			// prefix the block to say it comes from a contiguous block allocator or not
			if (PSBlockAllocator)
			{		
				result = (TBlocAllocPtr *) PSBlockAllocator->alloc(n * sizeof(T) + sizeof(TBlocAllocPtr *));
				*result = PSBlockAllocator; // mark as a block from block allocator
			}
			else
			{
				 result = (TBlocAllocPtr *) _Alloc.allocate(n * sizeof(T) + sizeof(TBlocAllocPtr *));
				 *result = NULL; // mark as a stl block
			}		
			return (pointer) (result + 1); // usable space starts after header
		}
		void deallocate(pointer p, size_type n)
		{ 
			++NumDealloc;
			--NumPSAlloc;
			if (!p) return;		
			pointer realAddress = (pointer) ((uint8 *) p - sizeof(TBlocAllocPtr *));
			if (* (TBlocAllocPtr *) realAddress)
			{	
				// block comes from a block allocator
				(*(TBlocAllocPtr *) realAddress)->free((void *) realAddress);						
			}
			else
			{
				// block comes from the stl allocator
				_Alloc.deallocate((uint8 *) realAddress);
			}		
		}
		void construct(pointer p, const T& val) { new (p) T(val); }    
		void destroy(pointer p) { p->~T(); }
		// stl rebind
		template <class _Tp, class U>
			CPSAllocator<U>& __stl_alloc_rebind(CPSAllocator<_Tp>& __a, const U*)
		{				
			return (CPSAllocator<U>&)(__a); 
		}
		
		template <class _Tp, class U>
			CPSAllocator<U> __stl_alloc_create(const CPSAllocator<_Tp>&, const U*)
		{
			return CPSAllocator<U>();
		}

	private:
		std::allocator<uint8> _Alloc;
	};
	

	// allocation of objects of ps (to be used inside base class declaration, replaces operator new & delete)
	#if !defined (NL_USE_DEFAULT_MEMORY_MANAGER) && !defined (NL_NO_DEFINE_NEW)
		// special version for nel memory
		#define PS_FAST_OBJ_ALLOC \
		void *operator new(size_t size, const char *filename, int line) { return PSFastMemAlloc((uint) size); }\
		void operator delete(void *block, const char *filename, int line) { PSFastMemFree(block); }            \
		void operator delete(void *block) { PSFastMemFree(block); }
	#else
		#define PS_FAST_OBJ_ALLOC \
		void *operator new(size_t size) { return PSFastMemAlloc((uint) size); }\
		void operator delete(void *block) { PSFastMemFree(block); }
	#endif


	// Partial specialization tips for vectors
	// To define a vector of type T, one must do : CPSVector<T>::V MyVector;
	template <class T>
	struct CPSVector
	{
		typedef std::vector<T, CPSAllocator<T> > V;
	};
	// partial specialisation tips for multimap
	template <class T, class U, class Pr = std::less<T> >
	struct CPSMultiMap
	{
		typedef std::multimap<T, U, Pr, CPSAllocator<T> > M;
	};



	extern uint NumPSAlloc;
	extern uint NumDealloc;


	// allocation of memory block for objects of a particle system
	void *PSFastMemAlloc(uint numBytes);
	void PSFastMemFree(void *block);

#endif


} // NL3D

#endif

