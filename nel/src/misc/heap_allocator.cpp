/** \file heap_allocator.cpp
 * A Heap allocator
 *
 * $Id: heap_allocator.cpp,v 1.5 2002/11/13 15:45:53 lecroart Exp $
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

/*	This file can't use Visual precompilated headers because
	the precompilated header ("nel/misc/stdmisc.h") includes 
	"nel/misc/types_nl.h". Before including the file 
	"nel/misc/types_nl.h", we need to define NL_HEAP_ALLOCATOR_H
	for this file to avoid new overriding. */

#include "stdmisc.h"

#include "nel/misc/heap_allocator.h"
#include "nel/misc/debug.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif // NL_OS_WINDOWS

#include <set>

namespace NLMISC 
{

// Include inlines functions
#include "nel/misc/heap_allocator_inline.h"

#define NL_HEAP_SB_CATEGORY "_SmlBlk"
#define NL_HEAP_CATEGORY_BLOCK_CATEGORY "_MemCat"
#define NL_HEAP_MEM_DEBUG_CATEGORY "_MemDb"
#define NL_HEAP_UNKNOWN_CATEGORY "Unknown"

void CHeapAllocatorOutputError (const char *str)
{
	fprintf (stderr, str);
#ifdef NL_OS_WINDOWS
	OutputDebugString (str);
#endif // NL_OS_WINDOWS
}

// *********************************************************
// Constructors / desctrutors
// *********************************************************

CHeapAllocator::CHeapAllocator (uint mainBlockSize, uint blockCount, TBlockAllocationMode blockAllocationMode, 
				  TOutOfMemoryMode outOfMemoryMode)
{
	// Critical section
	enterCriticalSection ();

	// Allocator name
	_Name[0] = 0;

	// Check size of structures must be aligned
	internalAssert ((sizeof (CNodeBegin) & (Align-1)) == 0);
	internalAssert ((NL_HEAP_NODE_END_SIZE & (Align-1)) == 0);
	internalAssert ((sizeof (CFreeNode) & (Align-1)) == 0);

	// Check small block sizes
	internalAssert ((FirstSmallBlock&(SmallBlockGranularity-1)) == 0);
	internalAssert ((LastSmallBlock&(SmallBlockGranularity-1)) == 0);

	_MainBlockList = NULL;
	_MainBlockSize = mainBlockSize;
	_BlockCount = blockCount;
	_BlockAllocationMode = blockAllocationMode;
	_OutOfMemoryMode = outOfMemoryMode;
	_FreeTreeRoot = &_NullNode.FreeNode;
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	_AlwaysCheck = false;
#endif // NL_HEAP_ALLOCATION_NDEBUG

	_NullNode.FreeNode.Left = &_NullNode.FreeNode;
	_NullNode.FreeNode.Right = &_NullNode.FreeNode;
	_NullNode.FreeNode.Parent = NULL;

	setNodeBlack (&_NullNode.FreeNode);

#ifndef NL_HEAP_ALLOCATION_NDEBUG
	_AllocateCount = 0;
#endif // NL_HEAP_ALLOCATION_NDEBUG

	// *********************************************************
	// Small Block
	// *********************************************************

	// The free smallblock array by size
	const uint smallBlockSizeCount = NL_SMALLBLOCK_COUNT;
	uint smallBlockSize;
	for (smallBlockSize=0; smallBlockSize<smallBlockSizeCount; smallBlockSize++)
	{
		_FreeSmallBlocks[smallBlockSize] = NULL;
	}

	// No small block
	_SmallBlockPool = NULL;

	leaveCriticalSection ();
}

// *********************************************************

CHeapAllocator::~CHeapAllocator ()
{
	// Release all memory used
	releaseMemory ();
}

// *********************************************************

void CHeapAllocator::insert (CHeapAllocator::CFreeNode *x)
{
    CHeapAllocator::CFreeNode *current, *parent;

    // Find future parent
	current = _FreeTreeRoot;
	parent = NULL;
	while (current != &_NullNode.FreeNode)
	{
		parent = current;
		current = (getNodeSize (getNode (x)) <= getNodeSize (getNode (current)) ) ? current->Left : current->Right;
	}

	// Setup new node
	x->Parent = parent;
	x->Left = &_NullNode.FreeNode;
	x->Right = &_NullNode.FreeNode;
	setNodeRed (x);

	// Insert node in tree
	if (parent)
	{
		if(getNodeSize (getNode (x)) <= getNodeSize (getNode (parent)))
			parent->Left = x;
		else
			parent->Right = x;
		NL_UPDATE_MAGIC_NUMBER_FREE_NODE (parent);
	}
	else
	{
		_FreeTreeRoot = x;
	}

	NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x);

    // Maintain Red-Black tree balance
    // After inserting node x
	// Check Red-Black properties

	while (x != _FreeTreeRoot && isNodeRed (x->Parent))
	{
		// We have a violation
		if (x->Parent == x->Parent->Parent->Left)
		{
			CHeapAllocator::CFreeNode *y = x->Parent->Parent->Right;
			if (isNodeRed (y))
			{
				// Uncle is RED
                setNodeBlack (x->Parent);
				setNodeBlack (y);
				setNodeRed (x->Parent->Parent);

				// Crc node
				NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y);
				NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x->Parent);
				NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x->Parent->Parent);

				x = x->Parent->Parent;
			}
			else
			{
                // Uncle is Black
				if (x == x->Parent->Right)
				{
                    // Make x a left child
					x = x->Parent;
                    rotateLeft(x);
				}

				// Recolor and rotate
				setNodeBlack (x->Parent);
				setNodeRed (x->Parent->Parent);

				rotateRight (x->Parent->Parent);

				// Crc node
				NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x->Parent);
			}
		}
		else
		{
			// Mirror image of above code
			CHeapAllocator::CFreeNode *y = x->Parent->Parent->Left;
			if (isNodeRed (y))
			{                
				// Uncle is Red
				setNodeBlack (x->Parent);
				setNodeBlack (y);
				setNodeRed (x->Parent->Parent);

				// Crc node
				NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y);
				NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x->Parent);
				NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x->Parent->Parent);

				x = x->Parent->Parent;
			}
			else
			{
                // Uncle is Black
                if (x == x->Parent->Left) 
				{
                    x = x->Parent;                    
					rotateRight(x);
                }
				setNodeBlack (x->Parent);
                setNodeRed (x->Parent->Parent);

                rotateLeft (x->Parent->Parent);

				// Crc node
				NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x->Parent);
			}        
		}    
	}
    setNodeBlack (_FreeTreeRoot);

	// Crc node
	NL_UPDATE_MAGIC_NUMBER_FREE_NODE (_FreeTreeRoot);
}

// *********************************************************

void CHeapAllocator::erase (CHeapAllocator::CFreeNode *z)
{
	CFreeNode *x, *y;
	if (z->Left == &_NullNode.FreeNode || z->Right == &_NullNode.FreeNode)
	{
		// y has a NULL node as a child
		y = z;
	}
	else
	{
		// Find tree successor with a &_NullNode.FreeNode node as a child
		y = z->Right;
		while (y->Left != &_NullNode.FreeNode)
			y = y->Left;
	}
	
	// x is y's only child
	if (y->Left != &_NullNode.FreeNode)
		x = y->Left;
	else
		x = y->Right;

	// Remove y from the parent chain
	x->Parent = y->Parent;

	if (y->Parent)
	{
		if (y == y->Parent->Left)
			y->Parent->Left = x;
		else
		{
			internalAssert (y == y->Parent->Right);
			y->Parent->Right = x;
		}
		NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y->Parent);
	}
	else
		_FreeTreeRoot = x;

	bool yRed = isNodeRed (y);

	if (y != z)
	{
		// Replace y by z
		*y = *z;
		setNodeColor (y, isNodeRed (z));
		if (y->Parent)
		{
			if (y->Parent->Left == z)
			{
				y->Parent->Left = y;
			}
			else
			{
				internalAssert (y->Parent->Right == z);
				y->Parent->Right = y;
			}
		}
		else
		{
			internalAssert (_FreeTreeRoot == z);
			_FreeTreeRoot = y;
		}

		if (y->Left)
		{
			internalAssert (y->Left->Parent == z);
			y->Left->Parent = y;
			
			NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y->Left);
		}
		if (y->Right)
		{
			internalAssert (y->Right->Parent == z);
			y->Right->Parent = y;
	
			NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y->Right);
		}
	}

	NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x);
	NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y);
	if (y->Parent)
		NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y->Parent);

	if (!yRed)
	{
		// Maintain Red-Black tree balance
		// After deleting node x
		while (x != _FreeTreeRoot && isNodeBlack (x))
		{
			if (x == x->Parent->Left)
			{
				CFreeNode *w = x->Parent->Right;
				if (isNodeRed (w))
				{
					setNodeBlack (w);
					setNodeRed (x->Parent);
					rotateLeft (x->Parent);

					NL_UPDATE_MAGIC_NUMBER_FREE_NODE (w);

					w = x->Parent->Right;
				}
				if (isNodeBlack (w->Left) && isNodeBlack (w->Right))
				{
					setNodeRed (w);
					x = x->Parent;

					NL_UPDATE_MAGIC_NUMBER_FREE_NODE (w);
				}
				else
				{
					if (isNodeBlack (w->Right))
					{
						setNodeBlack (w->Left);
						setNodeRed (w);
						rotateRight (w);
						w = x->Parent->Right;
					}
					setNodeColor (w, isNodeRed (x->Parent));
					setNodeBlack (x->Parent);
					setNodeBlack (w->Right);
					rotateLeft (x->Parent);

					NL_UPDATE_MAGIC_NUMBER_FREE_NODE (w);
					NL_UPDATE_MAGIC_NUMBER_FREE_NODE (w->Right);

					x = _FreeTreeRoot;
				}
			}
			else
			{
				CFreeNode *w = x->Parent->Left;
				if (isNodeRed (w))
				{
					setNodeBlack (w);
					setNodeRed (x->Parent);
					rotateRight (x->Parent);

					NL_UPDATE_MAGIC_NUMBER_FREE_NODE (w);

					w = x->Parent->Left;
				}
				if ( isNodeBlack (w->Right) && isNodeBlack (w->Left) )
				{
					setNodeRed (w);
					x = x->Parent;

					NL_UPDATE_MAGIC_NUMBER_FREE_NODE (w);
				}
				else
				{
					if ( isNodeBlack (w->Left) )
					{
						setNodeBlack (w->Right);
						setNodeRed (w);
						rotateLeft (w);
						w = x->Parent->Left;
					}
					setNodeColor (w, isNodeRed (x->Parent) );
					setNodeBlack (x->Parent);
					setNodeBlack (w->Left);

					rotateRight (x->Parent);

					NL_UPDATE_MAGIC_NUMBER_FREE_NODE (w);
					NL_UPDATE_MAGIC_NUMBER_FREE_NODE (w->Left);

					x = _FreeTreeRoot;
				}
			}
		}
		setNodeBlack (x);
		NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x);
	}
}

// *********************************************************
// Node methods
// *********************************************************

CHeapAllocator::CNodeBegin *CHeapAllocator::splitNode (CNodeBegin *node, uint newSize)
{
	// Should be smaller than node size
	internalAssert (newSize <= getNodeSize (node));

	// Align size
	uint allignedSize = (newSize&~(Align-1)) + (( (newSize&(Align-1))==0 ) ? 0 : Align);
	if (allignedSize <= UserDataBlockSizeMin)
		allignedSize = UserDataBlockSizeMin;

#ifndef NL_HEAP_ALLOCATION_NDEBUG
	// End magic number aligned on new size
	node->EndMagicNumber = (uint32*)((uint8*)node + newSize + sizeof (CNodeBegin));
#endif // NL_HEAP_ALLOCATION_NDEBUG

	// Rest is empty ?
	if ( getNodeSize (node) - allignedSize < UserDataBlockSizeMin + sizeof (CNodeBegin) + NL_HEAP_NODE_END_SIZE )
		// No split
		return NULL;

	// New node begin structure
	CNodeBegin *newNode = (CNodeBegin*)((uint8*)node + sizeof (CNodeBegin) + allignedSize + NL_HEAP_NODE_END_SIZE );

	// Fill the new node header

	// Size
	setNodeSize (newNode, getNodeSize (node) - allignedSize - sizeof (CNodeBegin) - NL_HEAP_NODE_END_SIZE);

	// Set the node free
	setNodeFree (newNode);

	// Set the previous node pointer
	newNode->Previous = node;

	// Last flag
	setNodeLast (newNode, isNodeLast (node));

#ifndef NL_HEAP_ALLOCATION_NDEBUG
	// Begin markers
	memset (newNode->BeginMarkers, BeginNodeMarkers, CNodeBegin::MarkerSize-1);
	newNode->BeginMarkers[CNodeBegin::MarkerSize-1] = 0;

	// End pointer
	newNode->EndMagicNumber = (uint32*)((uint8*)newNode + getNodeSize (newNode) + sizeof (CNodeBegin));

	// End markers
	CNodeEnd *endNode = (CNodeEnd*)((uint8*)newNode + getNodeSize (newNode) + sizeof (CNodeBegin));
	memset (endNode->EndMarkers, EndNodeMarkers, CNodeEnd::MarkerSize-1);
	endNode->EndMarkers[CNodeEnd::MarkerSize-1] = 0;

	// No source informations
	newNode->File = NULL;
	newNode->Line = 0xffff;
	node->AllocateNumber = 0xffffffff;
	memset (newNode->Category, 0, CategoryStringLength);

	// Heap pointer
	newNode->Heap = this;
#endif // NL_HEAP_ALLOCATION_NDEBUG

	// Get next node
	CNodeBegin *next = getNextNode (node);
	if (next)
	{
		// Set previous
		next->Previous = newNode;

		NL_UPDATE_MAGIC_NUMBER (next);
	}

	// Should be big enough
	internalAssert (getNodeSize (newNode) >= UserDataBlockSizeMin);

	// New size of the first node
	setNodeSize (node, allignedSize);

	// No more the last
	setNodeLast (node, false);

	// Return new node
	return newNode;
}

// *********************************************************

void CHeapAllocator::mergeNode (CNodeBegin *node)
{
	// Get the previous node to merge with
	CNodeBegin *previous = node->Previous;
	internalAssert (getNextNode (previous) == node);
	internalAssert (previous);
	internalAssert (isNodeFree (previous));

	// New size
	setNodeSize (previous, getNodeSize (previous) + getNodeSize (node) + sizeof (CNodeBegin) + NL_HEAP_NODE_END_SIZE);

#ifndef NL_HEAP_ALLOCATION_NDEBUG
	// Set end pointers
	previous->EndMagicNumber = (uint32*)((uint8*)previous + getNodeSize (previous) + sizeof (CNodeBegin));
#endif // NL_HEAP_ALLOCATION_NDEBUG

	// Get the next node to relink
	CNodeBegin *next = getNextNode (node);
	if (next)
	{
		// Relink
		next->Previous = previous;

		NL_UPDATE_MAGIC_NUMBER (next);
	}

	// Get the last flag
	setNodeLast (previous, isNodeLast (node));

#ifndef NL_HEAP_ALLOCATION_NDEBUG

	// todo align

	// Clear the node informations
	memset (((uint8*)node + getNodeSize (node) + sizeof (CNodeBegin)), DeletedMemory, NL_HEAP_NODE_END_SIZE);
	memset (node, DeletedMemory, sizeof (CNodeBegin));
#endif // NL_HEAP_ALLOCATION_NDEBUG
}


// *********************************************************
// *********************************************************

// Synchronized methods

// *********************************************************
// *********************************************************


void CHeapAllocator::initEmptyBlock (CMainBlock& mainBlock)
{
	// Get the node pointer
	CNodeBegin *node = getFirstNode (&mainBlock);

	// Allocated size remaining after alignment
	internalAssert ((uint32)node - (uint32)mainBlock.Ptr >= 0);
	uint allocSize = mainBlock.Size - ((uint32)node - (uint32)mainBlock.Ptr);

	// *** Fill the new node header

	// User data size
	setNodeSize (node, allocSize-sizeof (CNodeBegin)-NL_HEAP_NODE_END_SIZE);

	// Node is free
	setNodeFree (node);

	// Node is last
	setNodeLast (node, true);

	// No previous node
	node->Previous = NULL;

	// Debug info
#ifndef NL_HEAP_ALLOCATION_NDEBUG
	// End magic number
	node->EndMagicNumber = (uint32*)((uint8*)node + getNodeSize (node) + sizeof (CNodeBegin));

	// Begin markers
	memset (node->BeginMarkers, BeginNodeMarkers, CNodeBegin::MarkerSize-1);
	node->BeginMarkers[CNodeBegin::MarkerSize-1] = 0;

	// End markers
	CNodeEnd *endNode = (CNodeEnd*)node->EndMagicNumber;
	memset (endNode->EndMarkers, EndNodeMarkers, CNodeEnd::MarkerSize-1);
	endNode->EndMarkers[CNodeEnd::MarkerSize-1] = 0;

	// Unallocated memory
	memset ((uint8*)node + sizeof(CNodeBegin), UnallocatedMemory, getNodeSize (node) );

	// No source file
	memset (node->Category, 0, CategoryStringLength);
	node->File = NULL;
	node->Line = 0xffff;
	node->AllocateNumber = 0xffffffff;

	// Heap pointer
	node->Heap = this;

	NL_UPDATE_MAGIC_NUMBER (node);
#endif // NL_HEAP_ALLOCATION_NDEBUG
}

// *********************************************************

uint CHeapAllocator::getBlockSize (void *block)
{
	// Get the node pointer
	CNodeBegin *node = (CNodeBegin*) ((uint)block - sizeof (CNodeBegin));

	return getNodeSize (((CNodeBegin*) ((uint)block - sizeof (CNodeBegin))));
}

// *********************************************************

#ifdef NL_HEAP_ALLOCATION_NDEBUG
void *CHeapAllocator::allocate (uint size)
#else // NL_HEAP_ALLOCATION_NDEBUG
void *CHeapAllocator::allocate (uint size, const char *sourceFile, uint line, const char *category)
#endif // NL_HEAP_ALLOCATION_NDEBUG
{
	// Check size is valid
	if (size != 0)
	{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
		// If category is NULL
		if (category == NULL)
		{
			// Get the current category
			CCategory *cat = (CCategory*)_CategoryStack.getPointer ();
			if (cat)
			{
				category = cat->Name;
			}
			else
			{
				// Not yet initialised
				category = NL_HEAP_UNKNOWN_CATEGORY;
			}
		}

		// Checks ?
		if (_AlwaysCheck)
		{
			// Check heap integrity
			internalCheckHeap (true);
		}

		// Check breakpoints
		/*if (_Breakpoints.find (_AllocateCount) != _Breakpoints.end())
		{
			// ********
			// * STOP *
			// ********
			// * Breakpoints allocation
			// ********
			NL_ALLOC_STOP;
		}*/
#endif // NL_HEAP_ALLOCATION_NDEBUG

		// Small or largs block ?
#ifdef NL_HEAP_NO_SMALL_BLOCK_OPTIMIZATION
		if (0)
#else // NL_HEAP_NO_SMALL_BLOCK_OPTIMIZATION
		if (size <= LastSmallBlock)
#endif// NL_HEAP_NO_SMALL_BLOCK_OPTIMIZATION
		{
			// *******************
			// Small block
			// *******************
			
			enterCriticalSectionSB ();

			// Get pointer on the free block list
			CNodeBegin **freeNode = (CNodeBegin **)_FreeSmallBlocks+NL_SIZE_TO_SMALLBLOCK_INDEX (size);

			// Not found ?
			if (*freeNode == NULL)
			{
				leaveCriticalSectionSB ();

				// Size must be aligned
				uint alignedSize = NL_ALIGN_SIZE_FOR_SMALLBLOCK (size);

				// Use internal allocator
				CSmallBlockPool *smallBlock = (CSmallBlockPool *)NelAlloc (*this, sizeof(CSmallBlockPool) + SmallBlockPoolSize * (sizeof(CNodeBegin) + alignedSize + 
					NL_HEAP_NODE_END_SIZE), NL_HEAP_SB_CATEGORY);

				enterCriticalSectionSB ();

				// Link this new block
				smallBlock->Size = alignedSize;
				smallBlock->Next = (CSmallBlockPool*)_SmallBlockPool;
				_SmallBlockPool = smallBlock;

				// Initialize the block
				uint pool;
				CNodeBegin *nextNode = *freeNode;
				for (pool=0; pool<SmallBlockPoolSize; pool++)
				{
					// Get the pool
					CNodeBegin *node = getSmallBlock (smallBlock, pool);

					// Set as free
					node->SizeAndFlags = alignedSize;

					// Insert in the list
					setNextSmallBlock (node, nextNode);
					nextNode = node;

					// Set debug informations
#ifndef NL_HEAP_ALLOCATION_NDEBUG
					// Set node free
					setNodeFree (node);
					
					// End magic number
					node->EndMagicNumber = (uint32*)(CNodeEnd*)((uint8*)node + getNodeSize (node) + sizeof (CNodeBegin));

					// Begin markers
					memset (node->BeginMarkers, BeginNodeMarkers, CNodeBegin::MarkerSize-1);
					node->BeginMarkers[CNodeBegin::MarkerSize-1] = 0;

					// End markers
					CNodeEnd *endNode = (CNodeEnd*)((uint8*)node + getNodeSize (node) + sizeof (CNodeBegin));
					memset (endNode->EndMarkers, EndNodeMarkers, CNodeEnd::MarkerSize-1);
					endNode->EndMarkers[CNodeEnd::MarkerSize-1] = 0;

					// Unallocated memory
					memset ((uint8*)node + sizeof(CNodeBegin), UnallocatedMemory, getNodeSize (node) );

					// No source file
					memset (node->Category, 0, CategoryStringLength);
					node->File = NULL;
					node->Line = 0xffff;
					node->AllocateNumber = 0xffffffff;

					// Heap pointer
					node->Heap = this;

					NL_UPDATE_MAGIC_NUMBER (node);

#endif // NL_HEAP_ALLOCATION_NDEBUG
				}

				// Link the new blocks
				*freeNode = nextNode;
			}

			// Check allocation as been done
			internalAssert (*freeNode);

			// Get a node
			CNodeBegin *node = *freeNode;

			// Checks
			internalAssert (size <= getNodeSize (node));
			internalAssert ((NL_SIZE_TO_SMALLBLOCK_INDEX (size)) < (NL_SMALLBLOCK_COUNT));

			// Relink
			*freeNode = getNextSmallBlock (node);

#ifndef NL_HEAP_ALLOCATION_NDEBUG		
			// Check the node CRC
			checkNode (node, evalMagicNumber (node));

			// Set node free for checks
			setNodeUsed (node);

			// Fill category
			strncpy (node->Category, category, CategoryStringLength-1);

			// Source filename
			node->File = sourceFile;

			// Source line
			node->Line = line;

			// Allocate count
			node->AllocateNumber = _AllocateCount++;

			// End magic number aligned on new size
			node->EndMagicNumber = (uint32*)((uint8*)node + size + sizeof (CNodeBegin));

			// Uninitialised memory
			memset ((uint8*)node + sizeof(CNodeBegin), UninitializedMemory, (uint32)(node->EndMagicNumber) - ( (uint32)node + sizeof(CNodeBegin) ) );

			// Crc node
			NL_UPDATE_MAGIC_NUMBER (node);
#endif // NL_HEAP_ALLOCATION_NDEBUG

			leaveCriticalSectionSB ();

			// Return the user pointer
			return (void*)((uint)node + sizeof (CNodeBegin));
		}
		else
		{
			// *******************
			// Large block
			// *******************

			// Check size
			if ( (size & ~CNodeBegin::SizeMask) != 0)
			{
				// ********
				// * STOP *
				// ********
				// * Attempt to allocate more than 1 Go
				// ********
				NL_ALLOC_STOP

				// Select outofmemory mode
				if (_OutOfMemoryMode == ReturnNull)
					return NULL;
				else
					throw std::bad_alloc();
			}

			enterCriticalSectionLB ();

			// Find a free node
			CHeapAllocator::CFreeNode *freeNode = CHeapAllocator::find (size);

			// The node
			CNodeBegin *node;

			// Node not found ?
			if (freeNode == NULL)
			{
				// Block allocation mode
				if ((_BlockAllocationMode == DontGrow) && (_MainBlockList != NULL))
				{
					// Select outofmemory mode
					if (_OutOfMemoryMode == ReturnNull)
						return NULL;
					else
						throw std::bad_alloc();
				}

				// The node
				uint8 *buffer;

				// Alloc size
				uint allocSize;

				// Aligned size
				uint allignedSize = (size&~(Align-1)) + (( (size&(Align-1))==0 ) ? 0 : Align);
				if (allignedSize < BlockDataSizeMin)
					allignedSize = BlockDataSizeMin;

				// Does the node bigger than mainNodeSize ?
				if (allignedSize > (_MainBlockSize-sizeof (CNodeBegin)-NL_HEAP_NODE_END_SIZE))
					// Allocate a specific block
					allocSize = allignedSize + sizeof (CNodeBegin) + NL_HEAP_NODE_END_SIZE;
				else
					// Allocate a new block
					allocSize = _MainBlockSize;

				// Allocate the buffer
				buffer = allocateBlock (allocSize+Align);

				// Add the buffer
				CMainBlock *mainBlock = (CMainBlock*)allocateBlock (sizeof(CMainBlock)); 
				mainBlock->Size = allocSize+Align;
				mainBlock->Ptr = buffer;
				mainBlock->Next = _MainBlockList;
				_MainBlockList = mainBlock;

				// Init the new block
				initEmptyBlock (*mainBlock);

				// Get the first node
				node = getFirstNode (mainBlock);
			}
			else
			{
				// Get the node
				node = getNode (freeNode);

				// Remove the node from free blocks and get the removed block
				erase (freeNode);
			}

#ifndef NL_HEAP_ALLOCATION_NDEBUG
			// Check the node CRC
			checkNode (node, evalMagicNumber (node));
#endif // NL_HEAP_ALLOCATION_NDEBUG

			// Split the node
			CNodeBegin *rest = splitNode (node, size);

			// Fill informations for the first part of the node

			// Clear free flag
			setNodeUsed (node);

#ifndef NL_HEAP_ALLOCATION_NDEBUG
			// Fill category
			strncpy (node->Category, category, CategoryStringLength-1);

			// Source filename
			node->File = sourceFile;

			// Source line
			node->Line = line;

			// Allocate count
			node->AllocateNumber = _AllocateCount++;

			// Crc node
			NL_UPDATE_MAGIC_NUMBER (node);

			// Uninitialised memory
			memset ((uint8*)node + sizeof(CNodeBegin), UninitializedMemory, (uint32)(node->EndMagicNumber) - ( (uint32)node + sizeof(CNodeBegin) ) );
#endif // NL_HEAP_ALLOCATION_NDEBUG

			// Node has been splited ?
			if (rest)
			{
				// Fill informations for the second part of the node

				// Get the freeNode
				freeNode = getFreeNode (rest);

				// Insert the free node
				insert (freeNode);

				// Crc node
				NL_UPDATE_MAGIC_NUMBER (rest);
			}

			// Check the node size
			internalAssert ( size <= getNodeSize (node) );
			internalAssert ( std::max ((uint)BlockDataSizeMin, size + (uint)Align) + sizeof (CNodeBegin) + sizeof (CNodeEnd) + sizeof (CNodeBegin) + sizeof (CNodeEnd) + BlockDataSizeMin >= getNodeSize (node) );

			// Check pointer alignment
			internalAssert (((uint32)node&(Align-1)) == 0);
			internalAssert (((uint32)((char*)node + sizeof(CNodeBegin))&(Align-1)) == 0);

			// Check size
			internalAssert ((uint32)node->EndMagicNumber <= (uint32)((uint8*)node+sizeof(CNodeBegin)+getNodeSize (node) ));
			internalAssert ((uint32)node->EndMagicNumber > (uint32)(((uint8*)node+sizeof(CNodeBegin)+getNodeSize (node) ) - BlockDataSizeMin - BlockDataSizeMin - sizeof(CNodeBegin) - sizeof(CNodeEnd)));

			leaveCriticalSectionLB ();

			// Return the user pointer
			return (void*)((uint)node + sizeof (CNodeBegin));
		}
	}
	else
	{
		// ********
		// * STOP *
		// ********
		// * Attempt to allocate 0 bytes
		// ********
		NL_ALLOC_STOP;
		return NULL;
	}
}

// *********************************************************

void CHeapAllocator::free (void *ptr)
{
	// Delete a null pointer ?
	if (ptr == NULL)
	{
		// ********
		// * STOP *
		// ********
		// * Attempt to delete a NULL pointer
		// ********
#ifdef NL_HEAP_STOP_NULL_FREE
		NL_ALLOC_STOP;
#endif // NL_HEAP_STOP_NULL_FREE
	}
	else
	{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
		// Checks ?
		if (_AlwaysCheck)
		{
			// Check heap integrity
			internalCheckHeap (true);
		}

		// Get the node pointer
		CNodeBegin *node = (CNodeBegin*) ((uint)ptr - sizeof (CNodeBegin));

		// Check the node CRC
		enterCriticalSectionSB ();
		enterCriticalSectionLB ();
		checkNode (node, evalMagicNumber (node));
		leaveCriticalSectionLB ();
		leaveCriticalSectionSB ();
#endif // NL_HEAP_ALLOCATION_NDEBUG

		// Large or small block ?
#ifdef NL_HEAP_ALLOCATION_NDEBUG
		uint size = (((CNodeBegin*) ((uint)ptr - sizeof (CNodeBegin))))->SizeAndFlags;
#else // NL_HEAP_ALLOCATION_NDEBUG
		uint size = getNodeSize (((CNodeBegin*) ((uint)ptr - sizeof (CNodeBegin))));
#endif // NL_HEAP_ALLOCATION_NDEBUG
		if (size <= LastSmallBlock)
		{
			// *******************
			// Small block
			// *******************

			// Check the node has not been deleted
			if (isNodeFree (node))
			{
				// ********
				// * STOP *
				// ********
				// * Attempt to delete a pointer already deleted
				// ********
				// * (*node):	the already deleted node
				// ********
				NL_ALLOC_STOP;
			}
			else
			{
				enterCriticalSectionSB ();

#ifndef NL_HEAP_ALLOCATION_NDEBUG
				// Uninitialised memory
				memset ((uint8*)node + sizeof(CNodeBegin), DeletedMemory, size );

				// Set end pointers
				node->EndMagicNumber = (uint32*)((uint8*)node + size + sizeof (CNodeBegin));

				// Mark has free
				setNodeFree (node);
#endif // NL_HEAP_ALLOCATION_NDEBUG

				// Add in the free list
				CNodeBegin **freeNode = (CNodeBegin **)_FreeSmallBlocks+NL_SIZE_TO_SMALLBLOCK_INDEX (size);
				((CNodeBegin*) ((uint)ptr - sizeof (CNodeBegin)))->Previous = *freeNode;
				*freeNode = ((CNodeBegin*) ((uint)ptr - sizeof (CNodeBegin)));

				// Update smallblock crc
				NL_UPDATE_MAGIC_NUMBER (node);

				leaveCriticalSectionSB ();
			}
		}
		else
		{
#ifdef NL_HEAP_ALLOCATION_NDEBUG
			// Get the real size
			size = getNodeSize (((CNodeBegin*) ((uint)ptr - sizeof (CNodeBegin))));
#endif // NL_HEAP_ALLOCATION_NDEBUG

			// Get the node pointer
			CNodeBegin *node = (CNodeBegin*) ((uint)ptr - sizeof (CNodeBegin));

			// Check the node has not been deleted
			if (isNodeFree (node))
			{
				// ********
				// * STOP *
				// ********
				// * Attempt to delete a pointer already deleted
				// ********
				// * (*node):	the already deleted node
				// ********
				NL_ALLOC_STOP;
			}
			else
			{
				enterCriticalSectionLB ();

#ifndef NL_HEAP_ALLOCATION_NDEBUG
				// Uninitialised memory
				memset ((uint8*)node + sizeof(CNodeBegin), DeletedMemory, size );

				// Set end pointers
				node->EndMagicNumber = (uint32*)((uint8*)node + size + sizeof (CNodeBegin));
#endif // NL_HEAP_ALLOCATION_NDEBUG

				// Mark has free
				setNodeFree (node);

				// *******************
				// Large block
				// *******************

				// A free node
				CHeapAllocator::CFreeNode *freeNode = NULL;

				// Previous node
				CNodeBegin *previous = node->Previous;
				if (previous)
				{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
					// Check the previous node
					checkNode (previous, evalMagicNumber (previous));
#endif // NL_HEAP_ALLOCATION_NDEBUG

					// Is it free ?
					if (isNodeFree (previous))
					{
						// Merge the two nodes
						mergeNode (node);

						// Get its free node
						erase (getFreeNode (previous));

						// Curent node
						node = previous;
					}
				}

				// Mark has free
				setNodeFree (node);

				// Next node
				CNodeBegin *next = getNextNode (node);
				if (next)
				{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
					// Check the next node
					checkNode (next, evalMagicNumber (next));
#endif // NL_HEAP_ALLOCATION_NDEBUG

					// Is it free ?
					if (isNodeFree (next))
					{
						// Free the new one
						erase (getFreeNode (next));

						// Merge the two nodes
						mergeNode (next);
					}
				}

				// Insert it into the tree
				insert (getFreeNode (node));

				NL_UPDATE_MAGIC_NUMBER (node);
				
				leaveCriticalSectionLB ();
			}
		}
	}
}

// *********************************************************
// Statistics
// *********************************************************

uint CHeapAllocator::getAllocatedMemory () const
{
	enterCriticalSection ();

	// Sum allocated memory
	uint memory = 0;

	// For each small block
	CSmallBlockPool *currentSB = (CSmallBlockPool *)_SmallBlockPool;
	while (currentSB)
	{
		// For each node in this small block pool
		uint block;
		for (block=0; block<SmallBlockPoolSize; block++)
		{
			// Get the node
			const CNodeBegin *current = getSmallBlock (currentSB, block);

			// Node allocated ?
			if (isNodeUsed (current))
				memory += getNodeSize (current) + ReleaseHeaderSize;
		}

		// Next block
		currentSB = currentSB->Next;
	}

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Get the first node
		const CNodeBegin *current = getFirstNode (currentBlock);
		while (current)
		{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
			// Check node
			checkNode (current, evalMagicNumber (current));
#endif // NL_HEAP_ALLOCATION_NDEBUG

			// Node allocated ? Don't sum small blocks..
			if (isNodeUsed (current) && (strcmp (current->Category, NL_HEAP_SB_CATEGORY) != 0))
				memory += getNodeSize (current) + ReleaseHeaderSize;

			// Next node
			current = getNextNode (current);
		}

		// Next block
		currentBlock = currentBlock->Next;
	}

	leaveCriticalSection ();

	// Return memory used
	return memory;
}

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG
uint CHeapAllocator::debugGetAllocatedMemoryByCategory (const char* category) const
{
	enterCriticalSection ();

	// Sum allocated memory
	uint memory = 0;

	// For each small block
	CSmallBlockPool *currentSB = (CSmallBlockPool *)_SmallBlockPool;
	while (currentSB)
	{
		// For each node in this small block pool
		uint block;
		for (block=0; block<SmallBlockPoolSize; block++)
		{
			// Get the node
			const CNodeBegin *current = getSmallBlock (currentSB, block);

			// Node allocated ?
			if ((isNodeUsed (current)) && (strcmp (current->Category, category)==0))
				memory += getNodeSize (current);

			memory += getNodeSize (current);
		}

		// Next block
		currentSB = currentSB->Next;
	}

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Get the first node
		const CNodeBegin *current = getFirstNode (currentBlock);
		while (current)
		{
			// Node allocated ?
			if ((isNodeUsed (current)) && (strcmp (current->Category, category)==0))
				memory += getNodeSize (current);

			// Next node
			current = getNextNode (current);
		}

		// Next block
		currentBlock = currentBlock->Next;
	}

	leaveCriticalSection ();

	// Return memory used
	return memory;
}
#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG
uint CHeapAllocator::debugGetDebugInfoSize () const
{
	// Return memory used
	return debugGetSBDebugInfoSize () + debugGetLBDebugInfoSize ();
}

uint CHeapAllocator::debugGetLBDebugInfoSize () const
{
	enterCriticalSection ();

	// Sum memory used by debug header
	uint memory = 0;

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Get the first node
		const CNodeBegin *current = getFirstNode (currentBlock);
		while (current)
		{
			// Node allocated ?
			memory  += sizeof(CNodeBegin) - ReleaseHeaderSize + sizeof(CNodeEnd);

			// Next node
			current = getNextNode (current);
		}

		// Next block
		currentBlock = currentBlock->Next;
	}

	leaveCriticalSection ();

	// Return memory used
	return memory;
}

uint CHeapAllocator::debugGetSBDebugInfoSize () const
{
	enterCriticalSection ();

	// Sum memory used by debug header
	uint memory = 0;

	// For each small blocks
	CSmallBlockPool	*pool = (CSmallBlockPool*)_SmallBlockPool;
	while (pool)
	{
		memory  += SmallBlockPoolSize * (sizeof(CNodeBegin) - ReleaseHeaderSize + sizeof(CNodeEnd));

		// Next pool
		pool = pool->Next;
	}

	leaveCriticalSection ();

	// Return memory used
	return memory;
}
#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

void fprintf_int (uint value)
{
	
}

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG

class CCategoryMap
{
public:
	CCategoryMap ()
	{
		BlockCount = 0;
		Size = 0;
		Min = 0xffffffff;
		Max = 0;
	}
	uint	BlockCount;
	uint	Size;
	uint	Min;
	uint	Max;
};

bool CHeapAllocator::debugStatisticsReport (const char* stateFile, bool memoryMap)
{
	// Status
	bool status = false;

	debugPushCategoryString (NL_HEAP_MEM_DEBUG_CATEGORY);

	// Open files
	FILE *file = fopen (stateFile, "wt");

	// Block map
	typedef std::map<std::string, CCategoryMap> TBlockMap;
	TBlockMap blockMap;

	// Both OK
	if (file)
	{
		// **************************

		// For each small block
		uint smallBlockCount = 0;
		uint largeBlockCount = 0;
		CSmallBlockPool *currentSB = (CSmallBlockPool *)_SmallBlockPool;
		while (currentSB)
		{
			// For each node in this small block pool
			uint block;
			for (block=0; block<SmallBlockPoolSize; block++)
			{
				// Get the node
				const CNodeBegin *current = getSmallBlock (currentSB, block);

				// Node allocated ?
				if (isNodeUsed (current))
				{
					// Find the node
					TBlockMap::iterator ite = blockMap.find ((const char*)current->Category);

					// Found ?
					if (ite == blockMap.end ())
					{
						ite = blockMap.insert (TBlockMap::value_type (current->Category, CCategoryMap ())).first;
					}
					uint size = getNodeSize (current) + ReleaseHeaderSize;
					ite->second.BlockCount++;
					ite->second.Size += size;
					if (size < ite->second.Min)
						ite->second.Min = size;
					if (size > ite->second.Max)
						ite->second.Max = size;
				}

				// Next node
				smallBlockCount++;
			}

			// Next block
			currentSB = currentSB->Next;
		}

		// For each main block
		CMainBlock *currentBlock = _MainBlockList;
		while (currentBlock)
		{
			// Get the first node
			const CNodeBegin *current = getFirstNode (currentBlock);
			while (current)
			{
				// Node is used ?
				if (isNodeUsed (current))
				{
					// Find the node
					TBlockMap::iterator ite = blockMap.find ((const char*)current->Category);

					// Found ?
					if (ite == blockMap.end ())
					{
						ite = blockMap.insert (TBlockMap::value_type (current->Category, CCategoryMap ())).first;
					}
					uint size = getNodeSize (current) + ReleaseHeaderSize;
					ite->second.BlockCount++;
					ite->second.Size += size;
					if (size < ite->second.Min)
						ite->second.Min = size;
					if (size > ite->second.Max)
						ite->second.Max = size;
				}

				// Next node
				current = getNextNode (current);
				largeBlockCount++;
			}

			// Next block
			currentBlock = currentBlock->Next;
		}

		// Write the heap info file
		fprintf (file, "HEAP STATISTICS\n");
		fprintf (file, "HEAP, TOTAL MEMORY USED, ALLOCATED MEMORY, FREE MEMORY, FRAGMENTATION RATIO, MAIN BLOCK SIZE, MAIN BLOCK COUNT\n");

		fprintf (file, "%s, %d, %d, %d, %f%%, %d, %d\n", _Name, getTotalMemoryUsed (),
			getAllocatedMemory (), getFreeMemory (), 100.f*getFragmentationRatio (), getMainBlockSize (), getMainBlockCount ());

		fprintf (file, "\n\nHEAP BLOCKS\n");
		fprintf (file, "SMALL BLOCK MEMORY, SMALL BLOCK COUNT, LARGE BLOCK COUNT\n");

		fprintf (file, "%d, %d, %d\n", getSmallBlockMemory (), smallBlockCount, largeBlockCount);

		fprintf (file, "\n\nHEAP DEBUG INFOS\n");
		fprintf (file, "SB DEBUG INFO, LB DEBUG INFO, TOTAL DEBUG INFO\n");

		fprintf (file, "%d, %d, %d\n", debugGetSBDebugInfoSize (), debugGetLBDebugInfoSize (), debugGetDebugInfoSize ());

		// **************************

		// Write the system heap info file
		uint systemMemory = getAllocatedSystemMemory ();
		uint nelSystemMemory = getAllocatedSystemMemoryByAllocator ();

		fprintf (file, "\n\nSYSTEM HEAP STATISTICS\n");
		fprintf (file, "TOTAL ALLOCATED MEMORY, NEL ALLOCATED MEMORY, OTHER ALLOCATED MEMORY\n");
		fprintf (file, "%d, %d, %d\n", systemMemory, nelSystemMemory, systemMemory-nelSystemMemory);

		// Write the category map file
		fprintf (file, "\n\n\nCATEGORY STATISTICS\n");
		fprintf (file, "CATEGORY, BLOCK COUNT, MEMORY ALLOCATED, MIN BLOCK SIZE, MAX BLOCK SIZE, AVERAGE BLOCK SIZE, SB COUNT 8, SB COUNT 16, SB COUNT 24, SB COUNT 32, SB COUNT 40, SB COUNT 48, SB COUNT 56, SB COUNT 64, SB COUNT 72, SB COUNT 80, SB COUNT 88, SB COUNT 96, SB COUNT 104, SB COUNT 112, SB COUNT 120, SB COUNT 128\n");

		TBlockMap::iterator ite = blockMap.begin();
		while (ite != blockMap.end())
		{
			// Number of small blocks
			uint smallB[NL_SMALLBLOCK_COUNT];

			// Clean
			uint smallBlock;
			for (smallBlock=0; smallBlock<NL_SMALLBLOCK_COUNT; smallBlock++)
			{
				smallB[smallBlock] = 0;
			}
			
			// Scan small block for this category
			currentSB = (CSmallBlockPool *)_SmallBlockPool;
			while (currentSB)
			{
				// For each node in this small block pool
				uint block;
				for (block=0; block<SmallBlockPoolSize; block++)
				{
					// Get the node
					const CNodeBegin *current = getSmallBlock (currentSB, block);

					// Node allocated ?
					if (isNodeUsed (current))
					{
						// Good node ?
						if (current->Category == ite->first)
						{
							// Get the small block index
							uint index = NL_SIZE_TO_SMALLBLOCK_INDEX (getNodeSize (current));

							// One more node
							smallB[index]++;
						}
					}
				}

				// Next block
				currentSB = currentSB->Next;
			}

			// Average
			uint average = ite->second.Size / ite->second.BlockCount;

			// Print the line
			fprintf (file, "%s, %d, %d, %d, %d, %d", ite->first.c_str(), ite->second.BlockCount, ite->second.Size, 
				ite->second.Min, ite->second.Max, average);

			// Print small blocks
			for (smallBlock=0; smallBlock<NL_SMALLBLOCK_COUNT; smallBlock++)
			{
				fprintf (file, ", %d", smallB[smallBlock]);
			}

			fprintf (file, "\n");

			ite++;
		}

		// **************************

		// Write the small block statistics
		fprintf (file, "\n\n\nSMALL BLOCK STATISTICS\n");
		fprintf (file, "SIZE, BLOCK COUNT, BLOCK FREE, BLOCK USED, TOTAL MEMORY USED\n");

		// Number of small blocks
		uint count[NL_SMALLBLOCK_COUNT];
		uint free[NL_SMALLBLOCK_COUNT];

		uint smallBlock;
		for (smallBlock=0; smallBlock<NL_SMALLBLOCK_COUNT; smallBlock++)
		{
			count[smallBlock] = 0;
			free[smallBlock] = 0;
		}

		// For each small block
		currentSB = (CSmallBlockPool *)_SmallBlockPool;
		while (currentSB)
		{
			// For each node in this small block pool
			uint block;
			for (block=0; block<SmallBlockPoolSize; block++)
			{
				// Get the node
				const CNodeBegin *current = getSmallBlock (currentSB, block);

				// Get the small block index
				uint index = NL_SIZE_TO_SMALLBLOCK_INDEX (getNodeSize (current));

				// Add a block
				count[index]++;

				// Node allocated ?
				if (isNodeFree (current))
				{
					// Add a free block
					free[index]++;
				}

				// Next node
				current = getNextNode (current);
			}

			// Next block
			currentSB = currentSB->Next;
		}

		// Print stats
		for (smallBlock=0; smallBlock<NL_SMALLBLOCK_COUNT; smallBlock++)
		{
			uint size = (smallBlock+1)*SmallBlockGranularity;
			fprintf (file,"%d, %d, %d, %d, %d\n",size, count[smallBlock], free[smallBlock], 
				count[smallBlock]-free[smallBlock], count[smallBlock]*(sizeof (CNodeBegin) + size + NL_HEAP_NODE_END_SIZE));
		}
		
		// **************************

		// Write the memory map file
		if (memoryMap)
		{
			fprintf (file, "\n\n\nHEAP LARGE BLOCK DUMP\n");
			fprintf (file, "ADDRESS, SIZE, CATEGORY, HEAP, STATE, SOURCE, LINE\n");

			// For each main block
			currentBlock = _MainBlockList;
			while (currentBlock)
			{
				// Get the first node
				const CNodeBegin *current = getFirstNode (currentBlock);
				while (current)
				{
					// Write the entry
					fprintf (file, "0x%08x, %d, %s, %s, %s, %s, %d\n", (uint)current + sizeof(CNodeBegin),
						getNodeSize (current), current->Category, _Name, 
						isNodeFree (current)?"free":"used", current->File, current->Line);

					// Next node
					current = getNextNode (current);
				}

				// Next block
				currentBlock = currentBlock->Next;
			}
		}

		// File created successfuly
		status = true;
	}

	// Close
	if (file)
		fclose (file);

	debugPopCategoryString ();

	return status;
}
#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

uint CHeapAllocator::getFreeMemory () const
{
	enterCriticalSection ();

	// Sum free memory
	uint memory = 0;

	// For each small block
	CSmallBlockPool *currentSB = (CSmallBlockPool *)_SmallBlockPool;
	while (currentSB)
	{
		// For each node in this small block pool
		uint block;
		for (block=0; block<SmallBlockPoolSize; block++)
		{
			// Get the node
			const CNodeBegin *current = getSmallBlock (currentSB, block);

			// Node allocated ?
			if (isNodeFree (current))
				memory  += getNodeSize (current) + ReleaseHeaderSize;

			// Next node
			current = getNextNode (current);
		}

		// Next block
		currentSB = currentSB->Next;
	}

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Get the first node
		const CNodeBegin *current = getFirstNode (currentBlock);
		while (current)
		{
#ifndef NL_HEAP_ALLOCATION_NDEBUG
			// Check node
			checkNode (current, evalMagicNumber (current));
#endif // NL_HEAP_ALLOCATION_NDEBUG

			// Node allocated ?
			if (isNodeFree (current))
				memory  += getNodeSize (current) + ReleaseHeaderSize;

			// Next node
			current = getNextNode (current);
		}

		// Next block
		currentBlock = currentBlock->Next;
	}

	leaveCriticalSection ();

	// Return memory used
	return memory;
}

// *********************************************************

uint CHeapAllocator::getTotalMemoryUsed () const
{
	enterCriticalSection ();

	// Sum total memory
	uint memory = 0;

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Get block size
		memory += currentBlock->Size;

		// Sum the arrays
		memory += sizeof (CMainBlock);

		// Next block
		currentBlock = currentBlock->Next;
	}

	leaveCriticalSection ();

	// Return the memory
	return memory;
}

// *********************************************************

uint CHeapAllocator::getSmallBlockMemory () const
{
	enterCriticalSection ();

	// Sum total memory
	uint memory = 0;

	// For each small block
	CSmallBlockPool *currentSB = (CSmallBlockPool *)_SmallBlockPool;
	while (currentSB)
	{
		// Get block size
		memory += sizeof(CSmallBlockPool) + SmallBlockPoolSize * (sizeof(CNodeBegin) + currentSB->Size + 
					NL_HEAP_NODE_END_SIZE);

		// Next block
		currentSB = currentSB->Next;
	}

	leaveCriticalSection ();

	// Return the memory
	return memory;
}

// *********************************************************

float CHeapAllocator::getFragmentationRatio () const
{
	enterCriticalSection ();

	// Sum free and used node
	float free = 0;
	float used = 0;

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Get the first node
		const CNodeBegin *current = getFirstNode (currentBlock);
		while (current)
		{
			// Node allocated ?
			if (isNodeUsed (current))
				used++;
			else
				free++;

			// Next node
			current = getNextNode (current);
		}

		// Next block
		currentBlock = currentBlock->Next;
	}

	leaveCriticalSection ();

	// Return the memory
	if (used != 0)
		return free / used;
	else
		return 0;
}

// *********************************************************

void	CHeapAllocator::freeAll ()
{
	enterCriticalSection ();

	// Sum free memory
	uint memory = 0;

	// Clear the free tree
	_FreeTreeRoot = &_NullNode.FreeNode;

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Reinit this block
		initEmptyBlock (*currentBlock);

		// Get first block
		CNodeBegin *node = getFirstNode (currentBlock);

		// Insert the free node
		insert (getFreeNode (node));

		NL_UPDATE_MAGIC_NUMBER (node);

		// Next block
		currentBlock = currentBlock->Next;
	}

	leaveCriticalSection ();
}

// *********************************************************

void	CHeapAllocator::releaseMemory ()
{
	enterCriticalSection ();

	// Clear the free tree
	_FreeTreeRoot = &_NullNode.FreeNode;

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		freeBlock (currentBlock->Ptr);

		// Next block
		CMainBlock *toDelete = currentBlock;
		currentBlock = toDelete->Next;
		::free (toDelete);
	}

	// Erase block node
	_MainBlockList = NULL;

	leaveCriticalSection ();
}

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG

struct CLeak
{
	uint	Count;
	uint	Memory;
};

typedef std::map<std::string, CLeak> TLinkMap;

void CHeapAllocator::debugReportMemoryLeak ()
{
	// enterCriticalSection ();

	debugPushCategoryString (NL_HEAP_MEM_DEBUG_CATEGORY);

	// Sum allocated memory
	uint memory = 0;

	// Leak map
	TLinkMap leakMap;

	// Header
	char report[2048];
	smprintf (report, 2048, "Report Memory leak for allocator \"%s\"\n", _Name);
	CHeapAllocatorOutputError (report);

	// For each small block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Get the first node
		const CNodeBegin *current = getFirstNode (currentBlock);
		while (current)
		{
			// Check node
			checkNode (current, evalMagicNumber (current));

			// Node allocated ?
			if (isNodeUsed (current) && ( (current->Category == NULL) || (current->Category[0] != '_')) )
			{
				// Make a report
				smprintf (report, 2048, "%s(%d)\t: \"%s\"", current->File, current->Line, current->Category);

				// Look for this leak
				TLinkMap::iterator ite = leakMap.find (report);

				// Not found ?
				if (ite == leakMap.end ())
				{
					ite = leakMap.insert (TLinkMap::value_type (report, CLeak ())).first;
					ite->second.Count = 0;
					ite->second.Memory = 0;
				}

				// One more leak
				ite->second.Count++;
				ite->second.Memory += getNodeSize (current);

				memory += getNodeSize (current);
			}

			// Next node
			current = getNextNode (current);
		}

		// Next block
		currentBlock = currentBlock->Next;
	}

	// For each small block
	CSmallBlockPool *currentSB = (CSmallBlockPool *)_SmallBlockPool;
	while (currentSB)
	{
		// For each node in this small block pool
		uint block;
		for (block=0; block<SmallBlockPoolSize; block++)
		{
			// Get the node
			const CNodeBegin *current = getSmallBlock (currentSB, block);
			// Check node
			checkNode (current, evalMagicNumber (current));

			// Node allocated ?
			if (isNodeUsed (current) && ( (current->Category == NULL) || (current->Category[0] != '_')) )
			{
				// Make a report
				smprintf (report, 2048, "%s(%d)\t: \"%s\"",	current->File, current->Line, current->Category);

				// Look for this leak
				TLinkMap::iterator ite = leakMap.find (report);

				// Not found ?
				if (ite == leakMap.end ())
				{
					ite = leakMap.insert (TLinkMap::value_type (report, CLeak ())).first;
					ite->second.Count = 0;
					ite->second.Memory = 0;
				}

				// One more leak
				ite->second.Count++;
				ite->second.Memory += getNodeSize (current);

				memory += getNodeSize (current);
			}
		}

		// Next block
		currentSB = currentSB->Next;
	}

	// Look for this leak
	TLinkMap::iterator ite = leakMap.begin ();
	while (ite != leakMap.end ())
	{
		// Make a report
		smprintf (report, 2048, "%s,\tLeak count : %d,\tMemory allocated : %d\n", ite->first.c_str (), ite->second.Count, ite->second.Memory);

		// Report on stderr
		CHeapAllocatorOutputError (report);

		ite++;
	}

	// Make a report
	if (memory)
	{
		smprintf (report, 2048, "%d byte(s) found\n", memory);
	}
	else
	{
		smprintf (report, 2048, "No memory leak\n");
	}
	CHeapAllocatorOutputError (report);

	debugPopCategoryString ();

	// leaveCriticalSection ();
}
#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

bool CHeapAllocator::checkHeap (bool stopOnError) const
{
	bool res = internalCheckHeap (stopOnError);

	return res;
}

// *********************************************************

uint8 *CHeapAllocator::allocateBlock (uint size)
{
#undef malloc
	return (uint8*)::malloc (size);
}

// *********************************************************

void CHeapAllocator::freeBlock (uint8 *block)
{
	::free (block);
}

// *********************************************************

bool CHeapAllocator::internalCheckHeap (bool stopOnError) const
{
	enterCriticalSection ();

	// For each small blocks
	CSmallBlockPool	*pool = (CSmallBlockPool*)_SmallBlockPool;
	while (pool)
	{
		// For each small block
		uint smallBlock;
		CNodeBegin	*previous = NULL;
		for (smallBlock=0; smallBlock<SmallBlockPoolSize; smallBlock++)
		{
			// Get the small block
			CNodeBegin	*node = getSmallBlock (pool, smallBlock);
			CNodeBegin	*next = (smallBlock+1<SmallBlockPoolSize) ? getSmallBlock (pool, smallBlock+1) : NULL;

			// Check node
			checkNodeSB (pool, previous, node, next, stopOnError);

			previous = node;
		}

		// Next pool
		pool = pool->Next;
	}
	
	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Get the nodes
		const CNodeBegin *previous = NULL;
		const CNodeBegin *current = getFirstNode (currentBlock);
		internalAssert (current);	// Should have at least one block in the main block
		const CNodeBegin *next;
		
		// For each node
		while (current)
		{
			// Get next
			next = getNextNode (current);

			// Return Error ?
			if (!checkNodeLB (currentBlock, previous, current, next, stopOnError))
				return false;

			// Next
			previous = current;
			current = next;
		}

		// Next block
		currentBlock = currentBlock->Next;
	}

	// Check free tree
	if (!checkFreeNode (_FreeTreeRoot, stopOnError, true))
		return false;

	leaveCriticalSection ();

	// Ok, no problem
	return true;
}

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG
void CHeapAllocator::debugAlwaysCheckMemory (bool alwaysCheck)
{
	_AlwaysCheck = alwaysCheck;
}
#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG
bool CHeapAllocator::debugIsAlwaysCheckMemory (bool alwaysCheck) const
{
	return _AlwaysCheck;
}
#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

void	CHeapAllocator::setName (const char* name)
{
	enterCriticalSection ();

	strncpy (_Name, name, NameLength-1);

	leaveCriticalSection ();
}

// *********************************************************
// Category control
// *********************************************************

void CHeapAllocator::debugPushCategoryString (const char *str)
{
	// Get the category stack pointer
	CCategory *last = (CCategory*)_CategoryStack.getPointer ();

	// New category node
	CCategory *_new = (CCategory *)NelAlloc (*this, sizeof(CCategory), NL_HEAP_CATEGORY_BLOCK_CATEGORY);
	_new->Name = str;
	_new->Next = last;

	/* Push it, no need to be thread safe here, because we use thread specifc storage */
	_CategoryStack.setPointer (_new);
}

// *********************************************************

void CHeapAllocator::debugPopCategoryString ()
{
	// Get the category stack pointer
	CCategory *last = (CCategory*)_CategoryStack.getPointer ();
	// nlassertex (last, ("(CHeapAllocator::debugPopCategoryString ()) Pop category wihtout Push"));
	if (last)
	{
		CCategory *next = last->Next;

		// Free last node, no need to be thread safe here, because we use thread specifc storage
		free (last);

		/* Push it, no need to be thread safe here, because we use thread specifc storage */
		_CategoryStack.setPointer (next);
	}
}

// *********************************************************

#ifndef NL_HEAP_ALLOCATION_NDEBUG

#ifdef NL_OS_WINDOWS
#pragma optimize( "", off )
#endif // NL_OS_WINDOWS

void CHeapAllocator::checkNode (const CNodeBegin *node, uint32 crc) const
{
	// Check the bottom CRC of the node 
	if (crc != *(node->EndMagicNumber))
	{
		// ********
		// * STOP *
		// ********
		// * The bottom CRC32 of the current node is wrong. Use checkMemory() to debug the heap.
		// ********
		// * (*node) Check for more informations
		// ********
		NL_ALLOC_STOP;
	}

	// Check the node is hold by this heap
	if (node->Heap != this)
	{
		// ********
		// * STOP *
		// ********
		// * This node is not hold by this heap. It has been allocated with another heap.
		// ********
		// * (*node) Check for more informations
		// ********
		NL_ALLOC_STOP;
	}
}

#ifdef NL_OS_WINDOWS
#pragma optimize( "", on )
#endif // NL_OS_WINDOWS

#endif // NL_HEAP_ALLOCATION_NDEBUG

// *********************************************************

uint CHeapAllocator::getAllocatedSystemMemory ()
{
	uint systemMemory = 0;
#ifdef NL_OS_WINDOWS
	// Get system memory informations
	HANDLE hHeap[100];
	DWORD heapCount = GetProcessHeaps (100, hHeap);

	uint heap;
	for (heap = 0; heap < heapCount; heap++)
	{
		PROCESS_HEAP_ENTRY entry;
		entry.lpData = NULL;
		while (HeapWalk (hHeap[heap], &entry))
		{
			if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
			{
				systemMemory += entry.cbData + entry.cbOverhead;
			}
		}
	}

#elif defined NL_OS_UNIX
	
	char buffer[4096], *p;
	int fd, len;
	
	fd = open("/proc/self/stat", O_RDONLY);
	len = read(fd, buffer, sizeof(buffer)-1);
	close(fd);
	
	buffer[len] = '\0';
	
	p = buffer;
	p = strchr(p, ')')+1;			/* skip pid */
	p = skipWS(p);
	p++;
	
	p = skipToken(p);				/* skip ppid */
	p = skipToken(p);				/* skip pgrp */
	p = skipToken(p);				/* skip session */
	p = skipToken(p);				/* skip tty */
	p = skipToken(p);				/* skip tty pgrp */
	p = skipToken(p);				/* skip flags */
	p = skipToken(p);				/* skip min flt */
	p = skipToken(p);				/* skip cmin flt */
	p = skipToken(p);				/* skip maj flt */
	p = skipToken(p);				/* skip cmaj flt */
	p = skipToken(p);				/* utime */
	p = skipToken(p);				/* stime */
	p = skipToken(p);				/* skip cutime */
	p = skipToken(p);				/* skip cstime */
	p = skipToken(p);				/* priority */
	p = skipToken(p);				/* nice */
	p = skipToken(p);				/* skip timeout */
	p = skipToken(p);				/* skip it_real_val */
	p = skipToken(p);				/* skip start_time */
	
	systemMemory = strtoul(p, &p, 10);	/* vsize in bytes */

#endif // NL_OS_WINDOWS
	return systemMemory;
}

// *********************************************************

uint CHeapAllocator::getAllocatedSystemMemoryByAllocator ()
{
	uint nelSystemMemory = 0;

	// Build a set of allocated system memory pointers
	std::set<void*> ptrInUse;

	// For each main block
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		// Save pointers
		ptrInUse.insert ((void*)currentBlock);
		ptrInUse.insert ((void*)(currentBlock->Ptr));

		// Next block
		currentBlock = currentBlock->Next;
	}

#ifdef NL_OS_WINDOWS
	// Get system memory informations
	HANDLE hHeap[100];
	DWORD heapCount = GetProcessHeaps (100, hHeap);

	uint heap;
	for (heap = 0; heap < heapCount; heap++)
	{
		PROCESS_HEAP_ENTRY entry;
		entry.lpData = NULL;
		while (HeapWalk (hHeap[heap], &entry))
		{
			if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
			{
				// This pointer is already used ?
				if ( (ptrInUse.find ((void*)((char*)entry.lpData)) != ptrInUse.end ()) || 
					(ptrInUse.find ((void*)((char*)entry.lpData+32)) != ptrInUse.end ()) )
					nelSystemMemory += entry.cbData + entry.cbOverhead;
			}
		}
	}
#endif // NL_OS_WINDOWS
	return nelSystemMemory;
}

// *********************************************************

} // NLMISC
