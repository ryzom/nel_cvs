// memory.cpp : Defines the entry point for the DLL application.
//

#include <new>
#include "heap_allocator.h"

namespace NLMEMORY
{
	extern CHeapAllocator *GlobalHeapAllocator;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			NLMEMORY::GlobalHeapAllocator = (NLMEMORY::CHeapAllocator*)malloc (sizeof (NLMEMORY::CHeapAllocator));
#undef new
			new (NLMEMORY::GlobalHeapAllocator) NLMEMORY::CHeapAllocator (1024*1024*10, 1);
			NLMEMORY::GlobalHeapAllocator->setName ("NeL memory manager");
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
