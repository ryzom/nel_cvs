// mem_alloc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define __sgi_alloc TOTO

#include <windows.h>
#include <winbase.h>

#include <nel/misc/debug.h>
#include <nel/misc/heap_allocator.h>
#include <nel/misc/thread.h>
#include <stdlib.h>
#include <conio.h>


#include <set>
#include <map>
#include <vector>

using namespace NLMISC;
using namespace std;

// Choose the tests to perform
//#define TEST_NEL_ALLOCATORS			// Test the NeL allocator
//#define TEST_NEL_ALLOCATORS_SMALL	// Test the NeL allocator only small blocks
//#define TEST_NEL_MULTI_THREAD		// Test multi thread
#define TEST_MEMORY_ALLOCATOR		// Test memory allocator
//#define BENCH_ALLOCATORS			// Bench the differents allocators
// Define this to get verbose informations
#define VERBOSE 

// Define this to get step by step
// #define STEP_BY_STEP 

inline uint64 rdtsc()
{
	uint64 ticks;
#	ifndef NL_OS_WINDOWS		
		__asm__ volatile(".byte 0x0f, 0x31" : "=a" (ticks.low), "=d" (ticks.high));				
#	else 		
		__asm	rdtsc
		__asm	mov		DWORD PTR [ticks], eax
		__asm	mov		DWORD PTR [ticks + 4], edx		
#	endif
	return ticks;	
}

// Output strings
void Output (const char *format, ... )
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	sint ret = vsnprintf( buffer, 1024, format, args );
	printf (buffer);
	va_end( args );
}

// Verbose output strings
void VOutput (const char *format, ... )
{
#ifdef VERBOSE
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	sint ret = vsnprintf( buffer, 1024, format, args );
	printf (buffer);
	va_end( args );
#endif // VERBOSE
}

// An allocator interface for test 
class IAllocator
{
public:
	virtual uint8	*allocate (uint size) = 0;
	virtual void	free (uint8 *ptr, uint size) = 0;
	virtual uint	getAllocatedMemory () = 0;
	virtual bool	checkMemory (bool stopOnError) = 0;
	virtual const char* getName () const = 0;
};

// Base class for standard allocator implementations
class CStandardAllocator : public IAllocator
{
public:
	CStandardAllocator ()
	{
		_AllocatedSize = 0;
	}

protected:
	uint8	*allocate (uint size)
	{
		_AllocatedSize += size;
		return NULL;
	}
	void	free (uint8 *ptr, uint size)
	{
		_AllocatedSize -= size;
	}
	uint	getAllocatedMemory ()
	{
		return _AllocatedSize;
	}
	bool	checkMemory (bool stopOnError)
	{
		return true;
	}
	uint	_AllocatedSize;
};

// Allocator using new
class CNewAllocator : public CStandardAllocator
{
private:
	uint8	*allocate (uint size)
	{
		CStandardAllocator::allocate (size);
		return new uint8[size];
	}
	void	free (uint8 *ptr, uint size)
	{
		CStandardAllocator::free (ptr, size);
		delete [] ptr;
	}
	const char* getName () const
	{
		return "New allocator";
	}
};

// Allocator using malloc
class CMallocAllocator : public CStandardAllocator
{
private:
	uint8	*allocate (uint size)
	{
		CStandardAllocator::allocate (size);
		return (uint8*)::malloc (size);
	}
	void	free (uint8 *ptr, uint size)
	{
		CStandardAllocator::free (ptr, size);
		::free (ptr);
	}
	const char* getName () const
	{
		return "Malloc allocator";
	}
};

// Allocator using stl::alloc
class CSTLAllocator : public CStandardAllocator
{
private:
	uint8	*allocate (uint size)
	{
		CStandardAllocator::allocate (size);
		return _Allocator.allocate (size, NULL);
	}
	void	free (uint8 *ptr, uint size)
	{
		CStandardAllocator::free (ptr, size);		
		_Allocator.deallocate (ptr, size);
	}
	const char* getName () const
	{
		return "STL allocator";
	}

	std::allocator<uint8>	_Allocator;
};

// Allocator using NLMISC::CHeapAllocator
class CNeLAllocator : public IAllocator
{
public:
	CNeLAllocator (uint blockSize, bool checkAlways) : _Allocator (blockSize)
	{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
		_Allocator.debugAlwaysCheckMemory (checkAlways);
#endif // NL_HEAP_ALLOCATION_NDEBUG
	}
private:
	uint8	*allocate (uint size)
	{
		return (uint8*) NelAlloc (_Allocator, size, "CNeLAllocator");
	}
	void	free (uint8 *ptr, uint size)
	{
		_Allocator.free (ptr);
	}
	uint	getAllocatedMemory ()
	{
		return _Allocator.getAllocatedMemory ();
	}
	bool	checkMemory (bool stopOnError)
	{
		return _Allocator.checkHeap (stopOnError);
	}
	const char* getName () const
	{
		return "NeL allocator";
	}

	CHeapAllocator	_Allocator;
};

// A bench interface
class IBench
{
public:
	IBench (const std::string &name)
	{
		Name = name;
	}
	virtual void bench (IAllocator &allocator, uint threadId) = 0;

protected:
	std::string Name;
};

// A random alloc bench
class CRandomAllocBench : public IBench
{
public:
	CRandomAllocBench (const std::string &name, uint blockMin, uint blockMax, uint allocCount, uint sizeMax, uint incrementSizeMax, bool checkMemory) : IBench (name)
	{
		_BlockMin = blockMin;
		_BlockMax = blockMax;
		_AllocCount = allocCount;
		_SizeMax = sizeMax;
		_CheckMemory = checkMemory;
		_IncrementSizeMax = incrementSizeMax;
	}

private:

	class CAllocation
	{
	public:
		CAllocation () {}
		CAllocation (uint8 *ptr, uint size)
		{
			Pointer = ptr;
			Size = size;
		}
		uint8	*Pointer;
		uint	Size;
	};

	class CAllocThread : public IRunnable
	{
	public:
		CAllocThread (CRandomAllocBench *bench, IAllocator *allocator, uint threadId)
		{
			_Bench = bench;
			_Allocator = allocator;
			_ThreadId = threadId;
		}
	private:
		void run ()
		{
			_Bench->bench (*_Allocator, _ThreadId);
		}

		CRandomAllocBench	*_Bench;
		IAllocator			*_Allocator;
		uint				_ThreadId;
	};

public:
	void multi_thread_bench (IAllocator &allocator, uint threadCount)
	{
		while (threadCount)
		{
			threadCount--;
			IThread *thread = IThread::create (new CAllocThread (this, &allocator, threadCount));
			thread->start ();
		}
	}

	void bench (IAllocator &allocator, uint threadId = 0)
	{
		// Set thread priority
		HANDLE threadHandle = GetCurrentThread ();
		SetThreadPriority (threadHandle, THREAD_PRIORITY_NORMAL);

		// Init random
		srand(1);

		// Zero perf
		_AllocateTime = 0;
		_FreeTime = 0;

		// No memory
		std::vector<CAllocation>	allocations;
		allocations.reserve (1000);
		uint allocatedMemory = 0;

		// Go up or down ?
		uint currentSizeMax = 0;
		bool goUp = true;

		// Main loop
		uint count = _AllocCount;
		while (count != 0)
		{
			// Output
			if ((count &0xff) == 0)
				Output ("(%d) %d test to go\n", threadId, count);
			VOutput ("(%d) %d test to go\n", threadId, count);
			
			if (goUp)
			{
				currentSizeMax += _IncrementSizeMax;
				if (currentSizeMax >= _SizeMax)
				{
					goUp = false;
					currentSizeMax = _SizeMax;
				}
			}
			else
			{
				if (currentSizeMax <= _IncrementSizeMax)
				{
					goUp = true;
					currentSizeMax = _IncrementSizeMax;
				}
				else
					currentSizeMax -= _IncrementSizeMax;
			}

			bool bestAlloc = allocatedMemory < (currentSizeMax / 2);
			
			// Alloc or free ?
			bool alloc;
			if (allocatedMemory == 0)
				alloc = true;
			else
			{
				if (allocatedMemory >= currentSizeMax)
					alloc = false;
				else
				{
					if (bestAlloc)
						alloc = (rand () & 3) != 0;
					else
						alloc = (rand () & 3) == 3;
				}
			}

			// Alloc or free ?
			if (alloc)
			{
				// Size to alloc
				uint size = _BlockMin + ((rand () + 1) * (_BlockMax - _BlockMin)) / (RAND_MAX + 1);
				allocatedMemory += size;

				// Allocate the pointer
				uint64 startTick = rdtsc();
				uint8 *ptr = allocator.allocate (size);
				memset (ptr, 0xff, size);
				uint64 endTick = rdtsc();
				endTick -= startTick;

				// Update amout of memory
				_AllocateTime += endTick;

				// Add the allocation
				allocations.push_back (CAllocation (ptr, size));

				// Output
				VOutput ("(%d) Alloc : %d\tTotal : %d\tBlock count : %d\n", threadId, size, allocatedMemory, allocations.size ());
			}
			else
			{
				nlassert (!allocations.empty ());
				
				// Which block to free
				uint blockId = rand () * (allocations.size ()-1) / RAND_MAX;

				// Free the block
				uint8 *ptr = allocations[blockId].Pointer;
				uint size = allocations[blockId].Size;
				uint64 startTick = rdtsc();
				allocator.free (ptr, size);
				uint64 endTick = rdtsc();
				endTick -= startTick;
				_FreeTime += endTick;
	
				// Update amout of memory
				allocatedMemory -= size;

				// Remove the allocation
				if (allocations.size ()>1)
					allocations[blockId] = allocations[allocations.size ()-1];
				allocations.resize (allocations.size ()-1);

				// Output
				VOutput ("(%d) Free  : %d\tTotal : %d\tBlock count : %d\tBlock index : %d\n", threadId, size, allocatedMemory, allocations.size (), blockId);
			}

			// Check memory
			if (_CheckMemory)
			{
				// Check heap integrity
				if (allocator.checkMemory (true))
				{
					VOutput ("(%d) Check OK\n", threadId);
				}
				else
				{
					VOutput ("(%d) Check Failed\n", threadId);
				}

				// Check allocated memory is the same
				// nlassert (allocator.getAllocatedMemory () == allocatedMemory);
			}

#ifdef STEP_BY_STEP
			// Step by step ?
			getch ();
#endif // STEP_BY_STEP
			count--;
		}

		// Output report
		Output ("Result for the bench \"%s\" with the allocator \"%s\" : \n"
				"\tAlloc kticks : %d\n"
				"\tFree kticks : %d\n", Name.c_str (), allocator.getName (), 
				(uint32)(_AllocateTime/(uint64)1024), (uint32)(_FreeTime/(uint64)1024));
	}
private:
	uint			_BlockMin, _BlockMax;
	uint			_AllocCount;
	uint			_SizeMax;
	uint			_CheckMemory;
	uint			_IncrementSizeMax;
	uint64			_AllocateTime;
	uint64			_FreeTime;	
};


// Generate 

void alloc (int toto)
{
	int *ptr = new int (toto);
	// ptr = new (ptr) int;
}


#define ALLOC_COUNT 1024*10

int main(int argc, char* argv[])
{
	NL_ALLOC_CONTEXT (Main);
#ifdef TEST_NEL_ALLOCATORS
	CRandomAllocBench test ("Test", 1, 10000, 10000000, 1000000, 10, true);

	CNeLAllocator nelAlloc (10000, false);
	
	test.bench (nelAlloc);
#endif // TEST_NEL_ALLOCATORS

#ifdef TEST_NEL_ALLOCATORS_SMALL
	CRandomAllocBench testSmall ("TestSmall", 1, 128, 10000000, 1000000, 10000, true);

	CNeLAllocator nelAlloc (10000, true);
	
	testSmall.bench (nelAlloc);
#endif // TEST_NEL_ALLOCATORS_SMALL

#ifdef TEST_NEL_MULTI_THREAD
	CRandomAllocBench testSmall ("TestMultiThread", 1, 10000, 10000000, 1000000, 10000, true);

	CNeLAllocator nelAlloc (10000, false);
	//CSTLAllocator nelAlloc;
	//CNewAllocator nelAlloc;
	
	testSmall.multi_thread_bench (nelAlloc, 2);
#endif // TEST_NEL_MULTI_THREAD

#ifdef BENCH_ALLOCATORS
	CRandomAllocBench bench ("Bench", 1, 128, 1000000, 1000000, 10000, false);

	CNewAllocator newAlloc;
	CMallocAllocator mallocAlloc;
	CSTLAllocator stlAlloc;
	CNeLAllocator nelAlloc (10*1024*1024, false);
	
	bench.bench (newAlloc);
	bench.bench (mallocAlloc);
	bench.bench (nelAlloc);
	bench.bench (stlAlloc);
#endif // BENCH_ALLOCATORS

#ifdef TEST_MEMORY_ALLOCATOR

	// Should not call NeL allocator
	nlinfo ("toto");
	
	set<int> setAlloc;

	multiset<int> multisetAlloc;
	
	list<int> listAlloc;
	
	map<int, int> mapAlloc;
	
	multimap<int, int> multimapAlloc;

	int	*mallocAlloc = (int*)malloc (sizeof(int)*1000);
	
	// Should call NeL allocator

	{
		NL_ALLOC_CONTEXT (Main2);

		string stringAlloc = "toto";			// String doesn't work. String methods are imported from the stlPort dll.
		
		new vector<int> (1000);
		
		new vector<char> (1000);
	}
	int	*newAlloc = new int[1000];
	
	{
		NL_ALLOC_CONTEXT (Main3);

		new int[1000];
		new deque<int> (1000);
	}
	
	deque<int> dequeAlloc (1000);

	setAlloc.insert (0);

	multisetAlloc.insert (0);
	
	listAlloc.push_back (0);
	
	mapAlloc.insert (map<int,int>::value_type(0,0));
	
	multimapAlloc.insert (map<int,int>::value_type(0,0));

	// Output stats
	printf ("Allocated memory:\t%d\n", getGlobalHeapAllocator ()->getAllocatedMemory ());
	printf ("Free memory:\t%d\n", getGlobalHeapAllocator ()->getFreeMemory ());
	printf ("Total memory:\t%d\n", getGlobalHeapAllocator ()->getTotalMemoryUsed ());
	printf ("Debug memory:\t%d\n", getGlobalHeapAllocator ()->debugGetDebugInfoSize ());
	printf ("Main2 category memory:\t%d\n", getGlobalHeapAllocator ()->debugGetAllocatedMemoryByCategory ("Main2"));
	printf ("Fragmentation:\t%f\n", getGlobalHeapAllocator ()->getFragmentationRatio ());

	// Check 
	getGlobalHeapAllocator ()->debugStatisticsReport ("test.csv");

#endif TEST_MEMORY_ALLOCATOR

	// Pause
	getch ();

	return 0;
}

