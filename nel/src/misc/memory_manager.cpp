/** \file memory_manager.cpp
 * A new memory manager
 *
 * $Id: memory_manager.cpp,v 1.2 2002/10/29 12:52:53 besson Exp $
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

#include "stdmisc.h"

#include "nel/misc/memory_manager.h"
#include "nel/misc/heap_allocator.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif // NL_OS_WINDOWS

// *********************************************************

#define NL_MEMORY_MANAGER_SHARED_MEMORY_ID 0x3a732235
#define NL_MEMORY_MANAGER_SHARED_MEMORY_STR "0x3a732235"

// *********************************************************

namespace NLMISC 
{

// *********************************************************

// The global allocator
CHeapAllocator *GlobalHeapAllocator = NULL;

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG

#include <libxml/xmlmemory.h>

#ifdef NL_OS_WINDOWS
// Automatically add the xmllib library if you use nel
#pragma comment(lib, "libxml2.lib")
#endif

// Class to report memory leak at exit
class CReportMemoryLeak 
{
public:
	~CReportMemoryLeak ()
	{
		// If allocator exist
		if (GlobalHeapAllocator && MustReportMemoryLeak)
		{
			// Report memory leak
			GlobalHeapAllocator->debugReportMemoryLeak ();
		}
	}
	static bool	MustReportMemoryLeak;
};

// *********************************************************

// Singleton
bool				CReportMemoryLeak::MustReportMemoryLeak = true;
CReportMemoryLeak	ReportMemoryLeak;

// *********************************************************

void XmlFree4NeL (void *ptr)
{
	delete [] (uint8*)ptr;
}

// *********************************************************

void *XmlMalloc4NeL (size_t size)
{
	NL_ALLOC_CONTEXT (LibXML);
	return new uint8[size];
}

// *********************************************************

void *XmlRealloc4NeL (void *ptr, size_t size)
{
	NL_ALLOC_CONTEXT (LibXML);

	// Get the block size
	uint oldSize = CHeapAllocator::getBlockSize (ptr);

	void *newPtr = (void*)new uint8[size];
	memcpy (newPtr, ptr, (size<oldSize) ? size : oldSize);

	xmlFree (ptr);
	return newPtr;
}

// *********************************************************

char *XmlStrdup4NeL (const char *str)
{
	NL_ALLOC_CONTEXT (LibXML);
	nlassert (str);
	char *newStr = (char*)new uint8[strlen (str)+1];
	strcpy (newStr, str);
	return newStr;
}

// *********************************************************

// Method to get the heap allocator
inline CHeapAllocator *getGlobalHeapAllocator ()
{
	// Already created ?
	if (GlobalHeapAllocator)
		return GlobalHeapAllocator;
	else
	{
		// I can't use CSharedMemory here because it is not initilized yet and it uses heap allocation.
		// So, manual hard coded shared memory...
#ifdef NL_OS_WINDOWS

		// Open the existing file mapping by name
		HANDLE hMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, false, NL_MEMORY_MANAGER_SHARED_MEMORY_STR );
		if ( hMapFile == NULL )
		{
			// Create a file mapping backed by the virtual memory swap file (not a data file)
			hMapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(CHeapAllocator*), NL_MEMORY_MANAGER_SHARED_MEMORY_STR );

			// Map the file into memory address space
			CHeapAllocator **accessAddress = (CHeapAllocator **)MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );

			// Safe malloc, allocate by block of 10 Mg
			*accessAddress = (CHeapAllocator*) malloc (sizeof (CHeapAllocator));

			// Unlock
			UnmapViewOfFile( accessAddress );
		}
		else
		{
			// The allocator has been allocated by another module. We should be in a dll..

			// Tells the memory leak report to not report (it is not the main)
			CReportMemoryLeak::MustReportMemoryLeak = false;
		}

		// Map the file into memory address space
		CHeapAllocator **accessAddress = (CHeapAllocator **)MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
		// nlassert (accessAddress);

		// Set the global
		GlobalHeapAllocator = *accessAddress;

		// Unlock
		UnmapViewOfFile( accessAddress );

#else // NL_OS_WINDOWS

		// Open an existing shared memory segment
		int shmid = shmget( NL_MEMORY_MANAGER_SHARED_MEMORY_ID, 0, 0666 );
		if ( shmid == -1 )
		{
			// Create a shared memory segment
			int shmid = shmget( NL_MEMORY_MANAGER_SHARED_MEMORY_STR, size, IPC_CREAT | IPC_EXCL | 0666 );

			// Map the segment into memory address space
			CHeapAllocator **accessAddress = (CHeapAllocator **)shmat( shmid, 0, 0 );

			// Safe malloc, allocate by block of 10 Mg
			*accessAddress = (CHeapAllocator*) malloc (sizeof (CHeapAllocator));

			// Unlock
			shmdt( accessAddress );
		}

		// Map the file into memory address space
		CHeapAllocator **accessAddress = (CHeapAllocator **)shmat( shmid, 0, 0 );

		// Set the global
		GlobalHeapAllocator = *accessAddress;

		// Unlock
		shmdt( accessAddress );

#endif // NL_OS_WINDOWS
		
#undef new
		GlobalHeapAllocator = new (GlobalHeapAllocator) CHeapAllocator (1024*1024*10, 1);
#define new NL_NEW
		GlobalHeapAllocator->setName ("NeL memory manager");

		// Create debug now
		createDebug ();

		// Hook LibXML allocations
		nlverify (xmlMemSetup (XmlFree4NeL, XmlMalloc4NeL, XmlRealloc4NeL, XmlStrdup4NeL) == 0);

		return GlobalHeapAllocator;
	}
}

#endif // NL_HEAP_ALLOCATION_NDEBUG

} // NLMISC

// *********************************************************

#ifndef NL_USE_DEFAULT_MEMORY_MANAGER

// Need a debug new ?
#ifdef NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

#undef new

void* operator new(size_t size)
{
	(NLMISC::getGlobalHeapAllocator ())->allocate (size);
}

#define new NL_NEW

#else // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

NLMISC::CAllocContext::CAllocContext (const char* str)
{
	(NLMISC::getGlobalHeapAllocator ())->debugPushCategoryString (str);
}

// *********************************************************

NLMISC::CAllocContext::~CAllocContext ()
{
	(NLMISC::getGlobalHeapAllocator ())->debugPopCategoryString ();
}

// *********************************************************

#undef new

void* operator new(size_t size, const char *filename, int line)
{
	return (NLMISC::getGlobalHeapAllocator ())->allocate (size, filename, line, NULL);
}

// *********************************************************

void* operator new(size_t size)
{
	return (NLMISC::getGlobalHeapAllocator ())->allocate (size, "::new (size_t size) operator", 0, NULL);
}

#define new NL_NEW

#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

void operator delete(void* p)
{
	(NLMISC::getGlobalHeapAllocator ())->free (p);
}

// *********************************************************

void operator delete(void* p, const char *filename, int line)
{
	(NLMISC::getGlobalHeapAllocator ())->free (p);
}

// *********************************************************

void operator delete[](void* p)
{
	(NLMISC::getGlobalHeapAllocator ())->free (p);
}

// *********************************************************

void operator delete[](void* p, const char *filename, int line)
{
	(NLMISC::getGlobalHeapAllocator ())->free (p);
}

// *********************************************************

#endif // NL_USE_DEFAULT_MEMORY_MANAGER
