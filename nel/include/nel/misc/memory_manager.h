/** \file memory_manager.h
 * A new memory manager
 *
 * $Id: memory_manager.h,v 1.1 2002/10/28 17:32:12 corvazier Exp $
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

#ifndef NL_MEMORY_MANAGER_H
#define NL_MEMORY_MANAGER_H

/// \name Configuration

/* Define this to use the default memory allocator for new, delete and STL allocator. It is defined only if _STLP_USE_NEL
 * is not defined in the STlport site config file "stl/_site_config.h". To use the NeL memory manager, you need a modified 
 * version of STLport */
#ifndef _STLP_USE_NEL
#define NL_USE_DEFAULT_MEMORY_MANAGER
#endif // _STLP_USE_NEL

#ifdef NL_USE_DEFAULT_MEMORY_MANAGER

#define NL_NEW new
#define NL_ALLOC_CONTEXT(str) ((void)0);

#else // NL_USE_DEFAULT_MEMORY_MANAGER

namespace NLMISC
{
	class CHeapAllocator *getGlobalHeapAllocator ();
}

// Need a debug new ?
#ifdef NL_HEAP_ALLOCATION_NDEBUG

void* operator new(size_t size);

#define NL_NEW new
#define NL_ALLOC_CONTEXT(str) ((void)0);

#else // NL_HEAP_ALLOCATION_NDEBUG

void* operator new(size_t size);
void* operator new(size_t size, const char *filename, int line);

namespace NLMISC
{

class CAllocContext
{
public:
	CAllocContext (const char* str);
	~CAllocContext ();
};

};

/* New operator in debug mode
 * Doesn't work with placement new. To do a placement new, undef new, make your placement new, and redefine new with the macro NL_NEW */
#define NL_NEW new(__FILE__, __LINE__)
#define new NL_NEW
#define NL_ALLOC_CONTEXT(__name) NLMISC::CAllocContext _##__name##MemAllocContext##__name (#__name);

#endif // NL_HEAP_ALLOCATION_NDEBUG

void operator delete(void* p);
void operator delete(void* p, const char *filename, int line);
void operator delete[](void* p);
void operator delete[](void* p, const char *filename, int line);

#endif // NL_USE_DEFAULT_MEMORY_MANAGER

/// \name STL allocator

namespace NLMISC
{
	class __nel_alloc
	{
	public:
	  // this one is needed for proper simple_alloc wrapping
	  typedef char value_type;
# if defined (__STL_MEMBER_TEMPLATE_CLASSES)  && defined(__STL_USE_RAW_SGI_ALLOCATORS)
	  template <class _Tp1> struct rebind {
		typedef __allocator<_Tp1, __nel_alloc > other;
	  };
# endif
	  static void* allocate(size_t size) 
	  {
		  return new char[size];
	  }
	  static void deallocate(void* __p, size_t) 
	  {
		delete ((char*)__p);
	  }
	};
}

#endif // NL_MEMORY_MANAGER_H
