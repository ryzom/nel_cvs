/** \file memory_manager.cpp
 * A new memory manager
 *
 * $Id: memory_manager.cpp,v 1.6 2003/11/17 10:12:05 corvazier Exp $
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

// Include STLPort first
#include <stl/_site_config.h>
#include "nel/memory/memory_manager.h"
#include "memory_common.h"
#include "heap_allocator.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif // NL_OS_WINDOWS

#undef new

// *********************************************************

// Exported functions

namespace NLMEMORY
{

// Global allocator
CHeapAllocator *GlobalHeapAllocator = NULL;
sint32			GlobalHeapAllocatorSystemAllocated = 0;

// *********************************************************

#ifndef NL_USE_DEFAULT_MEMORY_MANAGER

#ifdef NL_HEAP_ALLOCATION_NDEBUG

MEMORY_API void* MemoryAllocate (unsigned int size)
{
	return GlobalHeapAllocator->allocate (size);
}

MEMORY_API void* MemoryAllocateDebug (uint size, const char *filename, uint line, const char *category)
{
	return GlobalHeapAllocator->allocate (size);
}

#else // NL_HEAP_ALLOCATION_NDEBUG

MEMORY_API void* MemoryAllocate (unsigned int size)
{
	return GlobalHeapAllocator->allocate (size, "", 0, 0);
}

MEMORY_API void* MemoryAllocateDebug (uint size, const char *filename, uint line, const char *category)
{
	return GlobalHeapAllocator->allocate (size, filename, line, category);
}

#endif // NL_HEAP_ALLOCATION_NDEBUG

MEMORY_API unsigned int GetAllocatedMemory ()
{
	return GlobalHeapAllocator->getAllocatedMemory ();
}

// *********************************************************

MEMORY_API unsigned int GetFreeMemory ()
{
	return GlobalHeapAllocator->getFreeMemory ();
}

// *********************************************************

MEMORY_API unsigned int GetTotalMemoryUsed ()
{
	return GlobalHeapAllocator->getTotalMemoryUsed ();
}

// *********************************************************

MEMORY_API unsigned int GetDebugInfoSize ()
{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	return GlobalHeapAllocator->debugGetDebugInfoSize ();
#else // NL_HEAP_ALLOCATION_NDEBUG
	return 0;
#endif // NL_HEAP_ALLOCATION_NDEBUG
}

// *********************************************************

MEMORY_API unsigned int GetAllocatedMemoryByCategory (const char *category)
{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	return GlobalHeapAllocator->debugGetAllocatedMemoryByCategory (category);
#else // NL_HEAP_ALLOCATION_NDEBUG
	return 0;
#endif // NL_HEAP_ALLOCATION_NDEBUG
}

// *********************************************************

MEMORY_API float GetFragmentationRatio ()
{
	return GlobalHeapAllocator->getFragmentationRatio ();
}

// *********************************************************

MEMORY_API unsigned int GetAllocatedSystemMemoryByAllocator ()
{
	return GlobalHeapAllocator->getAllocatedSystemMemoryByAllocator ();
}

// *********************************************************

MEMORY_API unsigned int GetAllocatedSystemMemory ()
{
	return GlobalHeapAllocator->getAllocatedSystemMemory ();
}

// *********************************************************

MEMORY_API unsigned int GetAllocatedSystemMemoryHook ()
{
	return GlobalHeapAllocatorSystemAllocated;
}

// *********************************************************

MEMORY_API bool CheckHeap (bool stopOnError)
{
	return GlobalHeapAllocator->checkHeap (stopOnError);
}

// *********************************************************

MEMORY_API bool StatisticsReport (const char *filename, bool memoryDump)
{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	return GlobalHeapAllocator->debugStatisticsReport (filename, memoryDump);
#else // NL_HEAP_ALLOCATION_NDEBUG
	return false;
#endif // NL_HEAP_ALLOCATION_NDEBUG
}

// *********************************************************

MEMORY_API void	ReportMemoryLeak ()
{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	GlobalHeapAllocator->debugReportMemoryLeak ();
#endif // NL_HEAP_ALLOCATION_NDEBUG
}

// *********************************************************

MEMORY_API void			AlwaysCheckMemory(bool alwaysCheck)
{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	GlobalHeapAllocator->debugAlwaysCheckMemory (alwaysCheck);
#endif // NL_HEAP_ALLOCATION_NDEBUG
}

// *********************************************************

MEMORY_API bool			IsAlwaysCheckMemory()
{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	return GlobalHeapAllocator->debugIsAlwaysCheckMemory ();
#endif // NL_HEAP_ALLOCATION_NDEBUG
	return false;
}

// *********************************************************

MEMORY_API void			SetOutOfMemoryHook (void (*outOfMemoryCallback)())
{
	GlobalHeapAllocator->setOutOfMemoryHook (outOfMemoryCallback);
}

// *********************************************************

MEMORY_API unsigned int GetBlockSize (void *pointer)
{
	return GlobalHeapAllocator->getBlockSize (pointer);
}

// *********************************************************

MEMORY_API void MemoryDeallocate (void *p)
{
	GlobalHeapAllocator->free (p);
}

// *********************************************************

MEMORY_API NLMEMORY::CHeapAllocator* GetGlobalHeapAllocator ()
{
	return GlobalHeapAllocator;
}

// *********************************************************

MEMORY_API void* NLMEMORY::MemoryReallocate (void *p, unsigned int size)
{
	// Get the block size
/*	uint oldSize = NLMEMORY::GetBlockSize (p);
	if (size > oldSize)
	{
		void *newPtr = MemoryAllocate (size);
		memcpy (newPtr, p, oldSize);
		MemoryDeallocate (p);
		p = newPtr;
	}
	return p;*/

	uint oldSize = NLMEMORY::GetBlockSize (p);
	
	void *newPtr = MemoryAllocate (size);
	memcpy (newPtr, p, (size<oldSize) ? size : oldSize);
	
	MemoryDeallocate (p);
	return newPtr;
	
}

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

// Class to report memory leak at exit
class CReportMemoryLeak 
{
public:
	~CReportMemoryLeak ()
	{
		// Report memory leak
		GlobalHeapAllocator->debugReportMemoryLeak ();
	}
};

// *********************************************************

// Singleton
CReportMemoryLeak	ReportMemoryLeakSingleton;

// *********************************************************

#endif // NL_HEAP_ALLOCATION_NDEBUG

#endif // NL_USE_DEFAULT_MEMORY_MANAGER

} // NLMEMORY

extern "C"
{

MEMORY_API void* NelMemoryAllocate (unsigned int size)
{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	return NLMEMORY::GlobalHeapAllocator->allocate (size, "", 0, 0);
#else NL_HEAP_ALLOCATION_NDEBUG
	return NLMEMORY::GlobalHeapAllocator->allocate (size);
#endif NL_HEAP_ALLOCATION_NDEBUG
}

MEMORY_API void* NelMemoryAllocateDebug (uint size, const char *filename, uint line, const char *category)
{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	return NLMEMORY::GlobalHeapAllocator->allocate (size, filename, line, category);
#else NL_HEAP_ALLOCATION_NDEBUG
	return NLMEMORY::GlobalHeapAllocator->allocate (size);
#endif NL_HEAP_ALLOCATION_NDEBUG
}

MEMORY_API void NelMemoryDeallocate (void *pointer)
{
	NLMEMORY::GlobalHeapAllocator->free (pointer);
}

}

// *********************************************************

#ifndef NL_USE_DEFAULT_MEMORY_MANAGER

// Need a debug new ?
#ifdef NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

#undef new

#define new NL_NEW

#else // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

NLMEMORY::CAllocContext::CAllocContext (const char* str)
{
	GlobalHeapAllocator->debugPushCategoryString (str);
}

// *********************************************************

NLMEMORY::CAllocContext::~CAllocContext ()
{
	GlobalHeapAllocator->debugPopCategoryString ();
}

// *********************************************************

#undef new

// *********************************************************

#define new NL_NEW

#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************
#endif // NL_USE_DEFAULT_MEMORY_MANAGER
