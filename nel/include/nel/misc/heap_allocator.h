/** \file heap_allocator.cpp
 * A Heap allocator
 *
 * $Id: heap_allocator.h,v 1.5 2003/07/01 15:33:14 corvazier Exp $
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

#ifndef NL_HEAP_ALLOCATOR_H
#define NL_HEAP_ALLOCATOR_H

/* 
 * Include "nel/misc/types_nl.h" but don't redefine new
 */
#include "nel/misc/types_nl.h"
#include "nel/misc/mutex.h"
#include "nel/misc/tds.h"

#include <vector>

namespace NLMISC 
{

/// \name Configuration

// Define this to disable debug features (use to trace buffer overflow and add debug informations in memory headers)
// #define NLMISC_HEAP_ALLOCATION_NDEBUG

// Define this to activate internal checks (use to debug the heap code)
// #define NLMISC_HEAP_ALLOCATOR_INTERNAL_CHECKS

// Define this to disable small block optimizations
//#define NLMISC_HEAP_NO_SMALL_BLOCK_OPTIMIZATION

// Stop when free a NULL pointer
//#define NLMISC_HEAP_STOP_NULL_FREE

// Mutex to use with the allocator
typedef CFastMutex CAllocatorMutex;			// Not fair, non-system, using sleep(), but very fast
//typedef CFairMutex CAllocatorMutex;		// Fair, fastest system mutex under Windows
//typedef CUnfairMutex CAllocatorMutex;		// Unfair, slowest system mutex under Windows

/// \name Macros

/*
 * A heap allocator with a lot of functionnality.
 * Used by the NeL memory manager as default allocator.
**/
class CHeapAllocator
{
public:

	enum 
	{ 
		ReleaseHeaderSize		=	8,
		CategoryStringLength	=	8,
		BeginNodeMarkers		=	'<',
		EndNodeMarkers			=	'>',
		UnallocatedMemory		=	0xba,
		UninitializedMemory		=	0xbc,
		DeletedMemory			=	0xbd,
		NameLength				=	32
	};

	/** 
	  * Block allocation mode.
	  */
	enum TBlockAllocationMode 
	{ 
		/// When the allocator runs out of memory, allocate a new block.
		Grow, 

		/// When the allocator runs out of memory, returns a out of memory signal.
		DontGrow 
	};
	
	/** 
	  * Out of memory signal
	  */
	enum TOutOfMemoryMode 
	{ 
		/// When the allocator runs out of memory, throw an exception.
		ThrowException, 

		/// When the allocator runs out of memory, returns NULL.
		ReturnNull 
	};
	
	typedef char TCategoryString[CategoryStringLength];

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG

	struct CMemoryLeakBlock
	{
	public:
		void			*Adress;
		uint			BlockSize;
		const char		*SourceFile;
		uint			Line;
		TCategoryString	Category;
	};

#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	// Constructor / Destructor
	CHeapAllocator (	uint mainBlockSize=1024*1024*10, 
			uint blockCount=1, 
			TBlockAllocationMode blockAllocationMode = Grow, 
			TOutOfMemoryMode outOfMemoryMode = ThrowException );
	virtual ~CHeapAllocator ();

	// Allocation / desallocation
#ifndef	NLMISC_HEAP_ALLOCATION_NDEBUG
	/* Use the NelAlloc macro */
	void					*allocate (uint size, const char *sourceFile, uint line, const char *category);
	/* Use the NelRealloc macro */
	void					*reallocate (void *ptr, uint size, const char *sourceFile, uint line, const char *category);
#else	// NLMISC_HEAP_ALLOCATION_NDEBUG
	void					*allocate (uint size);
	void					*realloc (void *ptr, uint size);
#endif	// NLMISC_HEAP_ALLOCATION_NDEBUG
	void					free (void *ptr);
	void					freeAll ();
	void					releaseMemory ();

	// Return an allocated block size
	static uint				getBlockSize (void *block);

	// Performance control

	// Returns false if the block size choosed i too big (>= 1 Go)
	bool					setMainBlockSize (uint mainBlockSize);
	uint					getMainBlockSize () const;
	bool					setMainBlockCount (uint blockCount);
	uint					getMainBlockCount () const;
	void					setBlockAllocationMode (TBlockAllocationMode mode);
	TBlockAllocationMode	getBlockAllocationMode () const;
	void					setOutOfMemoryMode (TOutOfMemoryMode mode);
	TOutOfMemoryMode		getOutOfMemoryMode () const;

	// Heap control
	bool					checkHeap (bool stopOnError) const;

	uint					getAllocatedMemory () const;

	/* Return the amount of free memoyr available in the allocator */
	uint					getFreeMemory () const;

	/* Return the total amount of memory allocated by the allocator */
	uint					getTotalMemoryUsed () const;

	/* Return the amount of memory used by the small block optimisation */
	uint					getSmallBlockMemory () const;

	/* Return the amount of allocated system memory */
	static uint				getAllocatedSystemMemory ();

	/* Return the amount of allocated system memory used by the allocator */
	uint					getAllocatedSystemMemoryByAllocator ();

	float					getFragmentationRatio () const;
	void					setName (const char* name);
	const char				*getName () const;

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
/*	void					debugAddBreakpoint (uint32 allocateNumber);
	void					debugRemoveBreakpoints ();*/
	uint					debugGetDebugInfoSize () const;
	uint					debugGetSBDebugInfoSize () const;
	uint					debugGetLBDebugInfoSize () const;
	uint					debugGetAllocatedMemoryByCategory (const char* category) const;
	bool					debugStatisticsReport (const char* stateFile, bool memoryMap);
	void					debugAlwaysCheckMemory (bool alwaysCheck);
	bool					debugIsAlwaysCheckMemory (bool alwaysCheck) const;

	// Heap debug
	void					debugReportMemoryLeak ();

#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	// Overridable
	virtual uint8			*allocateBlock (uint size);
	virtual void			freeBlock (uint8 *block);

private:

	// Constants for freeNode block size
	enum 
	{ 
		FreeNodeBlockSize = 128, 
		FreeNodeBlockSizeShift = 7, 
		FreeNodeBlockSizeMask = 0x7f 
	};

	// Minimal size and align
	enum 
	{ 
		Align = 8,
		BlockDataSizeMin = 1<<4
	};

	struct CNodeBegin;

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	struct CNodeEnd;
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	struct CFreeNode;

	struct CNodeBegin
	{
		enum	
		{ 
			MarkerSize=6
		};

		// Flags for the memory node
		enum	
		{ 
			Free=0x40000000,
			Last=0x80000000,
			SizeMask=0x3fffffff,
		};

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
		char		BeginMarkers[MarkerSize];			// <<<<<<<
		uint16		Line;								// Source line number
		char		Category[CategoryStringLength];		// Category name
		const char	*File;								// Source file name pointer
		CHeapAllocator		*Heap;						// Heap holder of this node
		uint32		*EndMagicNumber;					// Pointer on the end magic number
		uint32		AllocateNumber;						// Align structure to 80 bytes
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

		uint32		SizeAndFlags;						// Size of the user memory zone of 30 bits (1 Go max), and 2 flags (Free/Used and LastBlock)
		CNodeBegin	*Previous;							// Previous node in large block mode / Small block pointer in small block mode
	};

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	struct CNodeEnd
	{
		enum	
		{ 
			MarkerSize=4
		};

		uint32		MagicNumber;						// CRC32 of the node pointer and the node header. Can be move backward to fit the data
		char		EndMarkers[MarkerSize];				// >>>>
	};
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

#ifdef NLMISC_HEAP_ALLOCATION_NDEBUG

#define NL_HEAP_NODE_END_SIZE 0

#else // NLMISC_HEAP_ALLOCATION_NDEBUG

#define NL_HEAP_NODE_END_SIZE sizeof(CNodeEnd)

#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	static uint32			evalMagicNumber (const CNodeBegin *node);
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG
	static const CNodeBegin	*getNextNode	(const CNodeBegin *current);
	static CNodeBegin		*getNextNode	(CNodeBegin *current);
	static const CFreeNode	*getFreeNode	(const CNodeBegin *current);
	static CFreeNode		*getFreeNode	(CNodeBegin *current);
	static const CNodeBegin	*getNode		(const CFreeNode *current);
	static CNodeBegin		*getNode		(CFreeNode *current);
	static uint				getNodeSize		(const CNodeBegin *current);
	static bool				isNodeFree		(const CNodeBegin *current);
	static bool				isNodeUsed		(const CNodeBegin *current);
	static bool				isNodeLast		(const CNodeBegin *current);
	static bool				isNodeSmall		(const CNodeBegin *current);
	static bool				isNodeRed		(const CFreeNode *current);
	static bool				isNodeBlack		(const CFreeNode *current);
	static void				setNodeFree		(CNodeBegin *current);
	static void				setNodeUsed		(CNodeBegin *current);
	static void				setNodeLast		(CNodeBegin *current, bool last);
	static void				setNodeSize		(CNodeBegin *current, uint size);
	static void				setNodeColor	(CFreeNode *current, bool red);
	static void				setNodeRed		(CFreeNode *current);
	static void				setNodeBlack	(CFreeNode *current);

	struct CMainBlock
	{
		uint		Size;
		uint8		*Ptr;
		CMainBlock	*Next;
	};
	
	static const CNodeBegin	*getFirstNode	(const CMainBlock *mainBlock);
	static CNodeBegin		*getFirstNode	(CMainBlock *mainBlock);

	/* Integrity check of a single node. Called at each allocation / deallocation when NLMISC_HEAP_ALLOCATION_NDEBUG is not defined.
	   Call it inside a critical section. */
	bool		checkNodeLB (const CMainBlock *mainBlock, const CNodeBegin *previous, 
							const CNodeBegin *current, const CNodeBegin *next, bool stopOnError) const;

	struct CFreeNode
	{
		enum
		{
			Red = 1
		};
		CFreeNode	*Left;
		CFreeNode	*Right;
		CFreeNode	*Parent;
		uint32		Flags;
	};

	enum
	{
		UserDataBlockSizeMin = sizeof(CFreeNode)
	};

	// Manage freeNode
	inline void			rotateLeft (CFreeNode *x);
	inline void			rotateRight (CFreeNode *x);
	inline void			insert (CFreeNode *x);
	inline void			erase (CFreeNode *z);
	inline CFreeNode	*find (uint size);

	// Manage blocks
	inline static void			mergeNode (CNodeBegin *node);
	inline CNodeBegin			*splitNode (CNodeBegin *node, uint newSize);
	inline void					initEmptyBlock (CMainBlock& mainBlock);

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	inline static void			computeCRC32 (uint32 &crc, const void* buffer, unsigned int count);
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	// Checks
	bool				checkFreeNode (const CFreeNode *current, bool stopOnError, bool recurse) const;
#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	void				checkNode (const CNodeBegin *current, uint32 crc) const;
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	// Performe full integrity check of the heap, free list and smallblock. Call it outside critical sections.
	bool				internalCheckHeap (bool stopOnError) const;

	// Synchronisation
	void				enterCriticalSection () const;
	void				leaveCriticalSection () const;
	void				enterCriticalSectionSB () const;
	void				leaveCriticalSectionSB () const;
	void				enterCriticalSectionLB () const;
	void				leaveCriticalSectionLB () const;

	// The Null node
	struct CNullNode
	{
		uint8			NodeBeginBuffer[sizeof(CNodeBegin)];
		CFreeNode		FreeNode;
	};

	uint						_MainBlockSize;
	uint						_BlockCount;
	TBlockAllocationMode		_BlockAllocationMode;
	TOutOfMemoryMode			_OutOfMemoryMode;
#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	bool						_AlwaysCheck;
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	// List of main block.
	CMainBlock					*_MainBlockList;

	// Members
	CFreeNode					*_FreeTreeRoot;
	CNullNode					_NullNode;
	mutable CAllocatorMutex		_MutexLB;

	char						_Name[NameLength];
#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	uint32						_AllocateCount;
	// std::set<uint32>			_Breakpoints;
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	// *********************************************************

	/// \name Small Block

public:
	// Small block size
	enum
	{
		// Small block granularity
		SmallBlockGranularityShift = 3,

		// Small block granularity
		SmallBlockGranularity = 1<<SmallBlockGranularityShift,

		// Smallest block size
		FirstSmallBlock = 8,

		// Largest block size
		LastSmallBlock = 128,

		// Size of a smallblock pool
		SmallBlockPoolSize = 20
	};

	struct CSmallBlockPool
	{
	public:
		uint32				Size;
		CSmallBlockPool		*Next;
	};

private:

	// Some internal methods

	/* Integrity check of a single node. Called at each allocation / deallocation when NLMISC_HEAP_ALLOCATION_NDEBUG is not defined.
	   Call it inside a critical section. */
	bool		checkNodeSB (const CSmallBlockPool *mainBlock, const CNodeBegin *previous, const CNodeBegin *current, 
		const CNodeBegin *next, bool stopOnError) const;

	// Get a block from a small block
	static CNodeBegin	*getSmallBlock (CSmallBlockPool *smallBlock, uint blockIndex);

	// Get next free small block
	static CNodeBegin	*getNextSmallBlock (CNodeBegin *previous);

	// Set next free small block
	static void			setNextSmallBlock (CNodeBegin *previous, CNodeBegin *next);

	// Some macros
#define NL_SMALLBLOCK_COUNT (1+(LastSmallBlock - FirstSmallBlock)/SmallBlockGranularity)
#define NL_SIZE_TO_SMALLBLOCK_INDEX(size) ((size-1)>>SmallBlockGranularityShift)
#define NL_ALIGN_SIZE_FOR_SMALLBLOCK(size) (((size) + SmallBlockGranularity-1) & ~(SmallBlockGranularity-1))

	// The free smallblock array by size
	volatile CNodeBegin			*_FreeSmallBlocks[NL_SMALLBLOCK_COUNT];
	volatile CSmallBlockPool	*_SmallBlockPool;

	mutable CAllocatorMutex		_MutexSB;

	// *********************************************************

	/// \name Category control
public:

	/* Push the category string. Add the string at the top of the category string stack. The string str must be a static pointer.
	 * There is one stack per thread. */
	void	debugPushCategoryString (const char *str);

	// Remove the last pushed category string from the category stack
	void	debugPopCategoryString ();

private:

	/* Struct for the category stack */
	struct CCategory
	{
		const char	*Name;
		CCategory	*Next;
	};

	/* Thread dependant storage of category stack */
	CTDS	_CategoryStack;
};

/// \name Macros

/* Heap macro */
#ifdef	NLMISC_HEAP_ALLOCATION_NDEBUG

/* NelAlloc: category can be NULL. Then, category string will be the last pushed category string. */
#define NelAlloc(heap,size,category) ((heap).allocate (size))

/* NelRealloc: category can be NULL. Then, category string will be the last pushed category string. */
#define NelRealloc(heap,size,ptr,category) (heap.allocate (ptr, size))

#else // NLMISC_HEAP_ALLOCATION_NDEBUG

/* NelAlloc: category can be NULL. Then, category string will be the last pushed category string. */
#define NelAlloc(heap,size,category) ((heap).allocate (size, __FILE__, __LINE__, category))

/* NelRealloc: category can be NULL. Then, category string will be the last pushed category string. */
#define NelRealloc(heap,size,ptr,category) (heap.allocate (ptr, size, __FILE__, __LINE__, category))

#endif	//NLMISC_HEAP_ALLOCATION_NDEBUG

} // NLMISC 

#endif // NL_HEAP_ALLOCATOR_H
