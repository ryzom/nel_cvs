/** \file memory_manager.h
 * A new memory manager
 *
 * $Id: memory_manager.h,v 1.8 2003/03/13 15:06:54 corvazier Exp $
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

// CONFIGURATION

// Define this to use the allocator in release mode (no statistics, no checks, maximum speed, minimal memory used)
#define NL_HEAP_ALLOCATION_NDEBUG

#undef MEMORY_API
#ifdef MEMORY_EXPORTS
 #ifdef WIN32
  #define MEMORY_API __declspec(dllexport)
 #else 
  #define MEMORY_API
 #endif // NL_OS_WINDOWS
#else
 #ifdef WIN32
  #define MEMORY_API __declspec(dllimport)
 #else 
  #define MEMORY_API
 #endif // NL_OS_WINDOWS
#endif

#ifdef __cplusplus

/* Define this to use the default memory allocator for new, delete and STL allocator. It is defined only if _STLP_USE_NEL
 * is not defined in the STlport site config file "stl/_site_config.h". To use the NeL memory manager, you need a modified 
 * version of STLport */

/* To active Nel allocator in STLport, add the following code at the end of "stl/_site_cfg.h" and rebuild STLPort */

#if 0
// ********* Cut here

/*
 * Set _STLP_NEL_ALLOC to use NEL allocator that perform memory debugging,
 * such as padding/checking for memory consistency, a lot of debug features
 * and new/stl allocation statistics. 
 * Changing default will require you to rebuild STLPort library !
 *
 * To build STLPort library with Nelallocator, you should have build
 * all the nelmemory libraries.
 *
 * When you rebuild STLPort library, you must add to the environnement
 * variable the path of the nel library in the command promp with the
 * commands:
 * SET LIB=%LIB%;YOUR_NEL_LIB_DIRECTORY
 * SET INCLUDE=%INCLUDE%;YOUR_NEL_INCLUDE_DIRECTORY
 *
 * Most of the time YOUR_NEL_LIB_DIRECTORY is r:\code\nel\lib
 * Most of the time YOUR_NEL_INCLUDE_DIRECTORY is r:\code\nel\include
 *
 * Only tested under Visual 6
 */
#define   _STLP_USE_NEL 1

#ifdef _STLP_USE_NEL
# if !defined (MEMORY_EXPORTS)
#  include "nel/memory/memory_manager.h"
# endif
# undef new
# define _STLP_USE_NEWALLOC 1
# undef  _STLP_USE_MALLOC
# undef  _STLP_DEBUG_ALLOC
# if defined (WIN32) && ! defined (MEMORY_EXPORTS)
#  if defined (NDEBUG)
#   pragma message("Using NeL memory manager in Release")
#   pragma comment( lib, "nlmemory_rd" )
#  elif defined (_STLP_DEBUG)
#   pragma message("Using NeL memory manager in STLDebug")
#   pragma comment( lib, "nlmemory_d" )
#  else
#   pragma message("Using NeL memory manager in Debug")
#   pragma comment( lib, "nlmemory_df" )
#  endif
# endif // WIN32
#endif // _STLP_USE_NEL

// ********* Cut here
#endif // 0







#ifndef _STLP_USE_NEL
#define NL_USE_DEFAULT_MEMORY_MANAGER
#endif // _STLP_USE_NEL

#ifdef NL_USE_DEFAULT_MEMORY_MANAGER

#define NL_NEW new
#define NL_ALLOC_CONTEXT(str) ((void)0);

namespace NLMEMORY
{

MEMORY_API void*		MemoryAllocate (unsigned int size);
MEMORY_API void*		MemoryAllocateDebug (unsigned int size, const char *filename, unsigned int line, const char *category);
MEMORY_API void			MemoryDeallocate (void *p);
inline unsigned int GetAllocatedMemory () { return 0;}
inline unsigned int GetFreeMemory () { return 0;}
inline unsigned int GetTotalMemoryUsed () { return 0;}
inline unsigned int GetDebugInfoSize () { return 0;}
inline unsigned int GetAllocatedMemoryByCategory (const char *category) { return 0;}
inline unsigned int GetBlockSize (void *pointer) { return 0;}
inline float		GetFragmentationRatio () { return 0.0f;}
inline unsigned int GetAllocatedSystemMemoryByAllocator () { return 0;}
inline unsigned int GetAllocatedSystemMemory () { return 0;}
inline bool			CheckHeap (bool stopOnError) { return true;}
inline bool StatisticsReport (const char *filename, bool memoryDump) { return true; }

}

#else // NL_USE_DEFAULT_MEMORY_MANAGER
// *********************************************************

namespace NLMEMORY
{

MEMORY_API void*		MemoryAllocate (unsigned int size);
MEMORY_API void*		MemoryAllocateDebug (unsigned int size, const char *filename, unsigned int line, const char *category);
MEMORY_API void			MemoryDeallocate (void *p);
MEMORY_API unsigned int GetAllocatedMemory ();
MEMORY_API unsigned int GetFreeMemory ();
MEMORY_API unsigned int GetTotalMemoryUsed ();
MEMORY_API unsigned int GetDebugInfoSize ();
MEMORY_API unsigned int GetAllocatedMemoryByCategory (const char *category);
MEMORY_API unsigned int GetBlockSize (void *pointer);
MEMORY_API float		GetFragmentationRatio ();
MEMORY_API unsigned int GetAllocatedSystemMemoryByAllocator ();
MEMORY_API unsigned int GetAllocatedSystemMemory ();
MEMORY_API bool			CheckHeap (bool stopOnError);
MEMORY_API bool			StatisticsReport (const char *filename, bool memoryDump);

}

// Need a debug new ?
#ifdef NL_HEAP_ALLOCATION_NDEBUG

inline void* operator new(size_t size)
{
	return NLMEMORY::MemoryAllocate (size);
}

#define NL_NEW new
#define NL_ALLOC_CONTEXT(str) ((void)0);

#else // NL_HEAP_ALLOCATION_NDEBUG

inline void* operator new(size_t size, const char *filename, int line)
{
	return NLMEMORY::MemoryAllocateDebug (size, filename, line, 0);
}

inline void* operator new(size_t size)
{
	return NLMEMORY::MemoryAllocateDebug (size, "::new (size_t size) operator", 0, 0);
}

namespace NLMEMORY
{

class CAllocContext
{
public:
	MEMORY_API CAllocContext::CAllocContext (const char* str);
	MEMORY_API CAllocContext::~CAllocContext ();
};

};

/* New operator in debug mode
 * Doesn't work with placement new. To do a placement new, undef new, make your placement new, and redefine new with the macro NL_NEW */
#define NL_NEW new(__FILE__, __LINE__)
#define new NL_NEW
#define NL_ALLOC_CONTEXT(__name) NLMEMORY::CAllocContext _##__name##MemAllocContext##__name (#__name);

#endif // NL_HEAP_ALLOCATION_NDEBUG

namespace NLMEMORY
{
	class CHeapAllocator;
};

// *********************************************************

inline void operator delete(void* p)
{
	NLMEMORY::MemoryDeallocate (p);
}

// *********************************************************

inline void operator delete(void* p, const char *filename, int line)
{
	NLMEMORY::MemoryDeallocate (p);
}

// *********************************************************

inline void operator delete[](void* p)
{
	NLMEMORY::MemoryDeallocate (p);
}

// *********************************************************

inline void operator delete[](void* p, const char *filename, int line)
{
	NLMEMORY::MemoryDeallocate (p);
}

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	MEMORY_API void*		NelMemoryAllocate (unsigned int size);
	MEMORY_API void*		NelMemoryAllocateDebug (unsigned int size, const char *filename, unsigned int line, const char *category);
	MEMORY_API void			NelMemoryDeallocate (void *pointer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // NL_USE_DEFAULT_MEMORY_MANAGER


#endif // __cplusplus

#endif // NL_MEMORY_MANAGER_H
