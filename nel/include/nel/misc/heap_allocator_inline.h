/** \file heap_allocator_inline.h
 * A Heap allocator, inline functions
 *
 * $Id: heap_allocator_inline.h,v 1.3 2003/07/01 15:33:14 corvazier Exp $
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

#ifndef NL_HEAP_ALLOCATOR_INLINE_H
#define NL_HEAP_ALLOCATOR_INLINE_H

#ifdef NL_HEAP_ALLOCATOR_INTERNAL_CHECKS
#define internalAssert(a) memory_assert(a)
#else // NL_HEAP_ALLOCATOR_INTERNAL_CHECKS
#define internalAssert(a) ((void)0)
#endif // NL_HEAP_ALLOCATOR_INTERNAL_CHECKS

#ifdef	NLMISC_HEAP_ALLOCATION_NDEBUG

#define NL_UPDATE_MAGIC_NUMBER(node) ((void)0)
#define NL_UPDATE_MAGIC_NUMBER_FREE_NODE(node) ((void)0)

#else // NLMISC_HEAP_ALLOCATION_NDEBUG

#define NL_UPDATE_MAGIC_NUMBER(node) {\
		uint32 crc = evalMagicNumber (node);\
		*(node->EndMagicNumber) = crc;\
	}

#define NL_UPDATE_MAGIC_NUMBER_FREE_NODE(node) {\
		if (node != &_NullNode.FreeNode) \
		{ \
			uint32 crc = evalMagicNumber (getNode (node));\
			*(getNode (node)->EndMagicNumber) = crc;\
		} \
	}

#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

#if defined (NL_OS_WINDOWS)
#define NL_ALLOC_STOP _asm { int 3 }
#else
#define NL_ALLOC_STOP abort()
#endif

// *********************************************************
// Set / Get methods
// *********************************************************

inline bool	CHeapAllocator::isNodeFree		(const CNodeBegin *node)
{
	return (node->SizeAndFlags & CNodeBegin::Free) != 0;
}

// *********************************************************

inline bool	CHeapAllocator::isNodeUsed		(const CNodeBegin *node)
{
	return (node->SizeAndFlags & CNodeBegin::Free) == 0;
}

// *********************************************************

inline bool	CHeapAllocator::isNodeLast		(const CNodeBegin *node)
{
	return (node->SizeAndFlags & CNodeBegin::Last) != 0;
}

// *********************************************************

inline bool	CHeapAllocator::isNodeSmall		(const CNodeBegin *node)
{
#ifdef NL_HEAP_NO_SMALL_BLOCK_OPTIMIZATION
	return false;
#else // NL_HEAP_NO_SMALL_BLOCK_OPTIMIZATION
	return getNodeSize (node) <= LastSmallBlock;
#endif // NL_HEAP_NO_SMALL_BLOCK_OPTIMIZATION
}

// *********************************************************

inline bool	CHeapAllocator::isNodeRed		(const CFreeNode *node)
{
	return (node->Flags & CFreeNode::Red) != 0;
}

// *********************************************************

inline bool	CHeapAllocator::isNodeBlack		(const CFreeNode *node)
{
	return (node->Flags & CFreeNode::Red) == 0;
}

// *********************************************************

inline void	CHeapAllocator::setNodeFree	(CNodeBegin *node)
{
	node->SizeAndFlags |= CNodeBegin::Free;
}

// *********************************************************

inline void	CHeapAllocator::setNodeUsed	(CNodeBegin *node)
{
	node->SizeAndFlags &= ~CNodeBegin::Free;
}

// *********************************************************

inline void	CHeapAllocator::setNodeLast	(CNodeBegin *node, bool last)
{
	if (last)
		node->SizeAndFlags |= CNodeBegin::Last;
	else
		node->SizeAndFlags &= ~CNodeBegin::Last;
}

// *********************************************************

inline void	CHeapAllocator::setNodeSize	(CNodeBegin *node, uint size)
{
	// Size must be < 1 Go
	internalAssert ((size&0xc0000000) == 0);

	// Set the size
	node->SizeAndFlags &= ~CNodeBegin::SizeMask;
	node->SizeAndFlags |= size;
}

// *********************************************************

inline void	CHeapAllocator::setNodeColor (CFreeNode *node, bool red)
{
	if (red)
		node->Flags |= CFreeNode::Red;
	else
		node->Flags &= ~CFreeNode::Red;
}

// *********************************************************

inline void	CHeapAllocator::setNodeRed (CFreeNode *node)
{
	node->Flags |= CFreeNode::Red;
}

// *********************************************************

inline void	CHeapAllocator::setNodeBlack (CFreeNode *node)
{
	node->Flags &= ~CFreeNode::Red;
}

// *********************************************************
// Free tree management
// *********************************************************

inline void CHeapAllocator::rotateLeft (CHeapAllocator::CFreeNode *x)
{   
	// Rotate node x to left

	// Establish x->Right link
	CHeapAllocator::CFreeNode *y = x->Right;
	x->Right = y->Left;
	if (y->Left != &_NullNode.FreeNode) 
	{
		y->Left->Parent = x;
		NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y->Left);
	}

	// Establish y->Parent link
	if (y != &_NullNode.FreeNode) 
		y->Parent = x->Parent;

	// x is the root ?
	if (x->Parent)
	{
		// Link its parent to y
		if (x == x->Parent->Left)
			x->Parent->Left = y;
		else
		{
			internalAssert (x == x->Parent->Right);
			x->Parent->Right = y;
		}
		NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x->Parent);
	}
	else
	{
		internalAssert (x == _FreeTreeRoot);
		_FreeTreeRoot = y;
	}

	// Link x and y
	y->Left = x;
	if (x != &_NullNode.FreeNode)
		x->Parent = y;
	NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x);
	NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y);
}

// *********************************************************

inline void CHeapAllocator::rotateRight (CHeapAllocator::CFreeNode *x)
{
	// Rotate node x to right

	// Establish x->Left link
	CHeapAllocator::CFreeNode *y = x->Left;
	x->Left = y->Right;
	if (y->Right != &_NullNode.FreeNode)
	{
		y->Right->Parent = x;
		NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y->Right);
	}

	// Establish y->parent link
	if (y != &_NullNode.FreeNode)
		y->Parent = x->Parent;

	// x is the root ?
	if (x->Parent)
	{
		// Link its parent to y
		if (x == x->Parent->Right)
		{
			x->Parent->Right = y;
		}
		else
		{
			internalAssert (x == x->Parent->Left);
			x->Parent->Left = y;
		}
		NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x->Parent);
	} 
	else 
	{        
		internalAssert (x == _FreeTreeRoot);
		_FreeTreeRoot = y;
	}

    // Link x and y
	y->Right = x;    
	if (x != &_NullNode.FreeNode) 
		x->Parent = y;

	// Crc
	NL_UPDATE_MAGIC_NUMBER_FREE_NODE (x);
	NL_UPDATE_MAGIC_NUMBER_FREE_NODE (y);
}

// *********************************************************

inline CHeapAllocator::CFreeNode *CHeapAllocator::find (uint size)
{
	CHeapAllocator::CFreeNode	*current = _FreeTreeRoot;
	CHeapAllocator::CFreeNode	*smallest = NULL;
	while (current != &_NullNode.FreeNode)
	{
		// Smaller than this node ?
		if (size <= getNodeSize (getNode (current)))
		{
			// This is good
			smallest = current;

			// Go left
			current = current->Left;
		}
		else
		{
			// Go right
			current = current->Right;
		}
	}
	return smallest;
}

// *********************************************************
// Node methods
// *********************************************************

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG

inline void CHeapAllocator::computeCRC32(uint32 &crc, const void* buffer, unsigned int count)
{
	internalAssert ( (count&3) == 0);
	count >>= 2;
	
	const uint32* ptr = (uint32*) buffer;
	while (count--)
	{
		crc ^= *(ptr++);
	}
/*	Real CRC, too slow
	
	const uint8* ptr = (uint8*) buffer;
	while (count--)
	{
		uint8 value = *(ptr++);
		crc ^= ((uint)value << 24);
		for (int i = 0; i < 8; i++)
		{
			if (crc & 0x80000000)
			{
				crc = (crc << 1) ^ 0x04C11DB7;
			}
			else
			{
				crc <<= 1;
			}
		}
	}
*/
}

#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

// *********************************************************

inline const CHeapAllocator::CNodeBegin *CHeapAllocator::getFirstNode (const CMainBlock *mainBlock)
{
	// Align the node pointer
	return (CNodeBegin*)(((uint32)mainBlock->Ptr&~(Align-1)) + ((((uint32)mainBlock->Ptr&(Align-1))==0)? 0 : Align));
}

// *********************************************************

inline CHeapAllocator::CNodeBegin *CHeapAllocator::getFirstNode (CMainBlock *mainBlock)
{
	// todo align
	return (CNodeBegin*)(((uint32)mainBlock->Ptr&~(Align-1)) + ((((uint32)mainBlock->Ptr&(Align-1))==0)? 0 : Align));
}

// *********************************************************

inline const CHeapAllocator::CNodeBegin *CHeapAllocator::getNextNode (const CNodeBegin *current)
{
	// Last ?
	if (isNodeLast (current))
		return NULL;

	// todo align
	return (const CNodeBegin*)((uint8*)current + sizeof (CNodeBegin) + NL_HEAP_NODE_END_SIZE + getNodeSize (current) );
}

// *********************************************************

inline CHeapAllocator::CNodeBegin *CHeapAllocator::getNextNode (CNodeBegin *current)
{
	// Last ?
	if (isNodeLast (current))
		return NULL;

	// todo align
	return (CNodeBegin*)((uint8*)current + sizeof (CNodeBegin) + NL_HEAP_NODE_END_SIZE + getNodeSize (current) );
}

// *********************************************************

inline const CHeapAllocator::CFreeNode *CHeapAllocator::getFreeNode	(const CNodeBegin *current)
{
	return (const CHeapAllocator::CFreeNode *)((uint8*)current + sizeof(CNodeBegin));
}

// *********************************************************

inline CHeapAllocator::CFreeNode *CHeapAllocator::getFreeNode (CNodeBegin *current)
{
	return (CHeapAllocator::CFreeNode *)((uint8*)current + sizeof(CNodeBegin));
}

// *********************************************************

inline const CHeapAllocator::CNodeBegin	*CHeapAllocator::getNode (const CFreeNode *current)
{
	return (const CHeapAllocator::CNodeBegin *)((uint8*)current - sizeof(CNodeBegin));
}

// *********************************************************

inline CHeapAllocator::CNodeBegin *CHeapAllocator::getNode (CFreeNode *current)
{
	return (CHeapAllocator::CNodeBegin *)((uint8*)current - sizeof(CNodeBegin));
}

// *********************************************************

inline uint CHeapAllocator::getNodeSize (const CNodeBegin *current)
{
	return current->SizeAndFlags & CNodeBegin::SizeMask;
}

// *********************************************************

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG

inline uint32 CHeapAllocator::evalMagicNumber (const CNodeBegin *node)
{
	uint32 crc = (uint32)node;

	// Make the CRC of the rest of the node header
	computeCRC32 (crc, node, sizeof(CNodeBegin));

	// If the node is free and LARGE, crc the free tree node
	if ( isNodeFree (node) && !isNodeSmall (node) )
		computeCRC32 (crc, getFreeNode (node), sizeof(CFreeNode));

	// Return the magic number
	return crc;
}

#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

// *********************************************************
// Integrity checks
// *********************************************************

inline bool CHeapAllocator::checkNodeSB (const CSmallBlockPool *mainBlock, const CNodeBegin *previous, const CNodeBegin *current, const CNodeBegin *next, bool stopOnError) const
{
#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	// Get the theorical CRC check
	uint32 crc = evalMagicNumber (current);

	// Compare the magic number
	if (*(current->EndMagicNumber) != crc)
	{
		// Stop on error ?
		if (stopOnError)
		{
			// ********
			// * STOP *
			// ********
			// * The bottom CRC32 of the current node is wrong: Check if a node has overflowed. Node by top and bottom,
			// * the next by the top and the previous by the bottom.
			// * overflow or the next node by the top.
			// ********
			// * (*previous):	previous node
			// * (*current):	current node
			// * (*next):		next node
			// ********
			NL_ALLOC_STOP;
		}

		// CRC is wrong
		return false;
	}
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	// *** Release node control

	// Check node
	if	(
			( (uint)current < ((uint)mainBlock) + sizeof (CSmallBlockPool)) ||
			( (uint)current + getNodeSize (current) + sizeof(CNodeBegin) + NL_HEAP_NODE_END_SIZE >
				((uint)mainBlock) + sizeof (CSmallBlockPool) + SmallBlockPoolSize * (sizeof(CNodeBegin)+ mainBlock->Size  + NL_HEAP_NODE_END_SIZE) )
		)
	{
		// Stop on error ?
		if (stopOnError)
		{
			// ********
			// * STOP *
			// ********
			// * The size value is corrupted: Check if the current node has
			// * overflow by the top or the previous node by the bottom.
			// ********
			// * (*previous):	previous node
			// * (*current):	current node
			// * (*next):		next node
			// ********
			NL_ALLOC_STOP;
		}

		// Node size is corrupted
		return false;
	}

	// Ok
	return true;
}

// *********************************************************

inline bool CHeapAllocator::checkNodeLB (const CMainBlock *mainBlock, const CNodeBegin *previous, const CNodeBegin *current, const CNodeBegin *next, bool stopOnError) const
{
#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG
	// Get the theorical CRC check
	uint32 crc = evalMagicNumber (current);

	// Compare the magic number
	if (*(current->EndMagicNumber) != crc)
	{
		// Stop on error ?
		if (stopOnError)
		{
			// ********
			// * STOP *
			// ********
			// * The bottom CRC32 of the current node is wrong: Check if a node has overflowed. Node by top and bottom,
			// * the next by the top and the previous by the bottom.
			// * overflow or the next node by the top.
			// ********
			// * (*previous):	previous node
			// * (*current):	current node
			// * (*next):		next node
			// ********
			NL_ALLOC_STOP;
		}

		// CRC is wrong
		return false;
	}
#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

	// *** Release node control

	// Check node
	if	(
			( (uint)current < (uint)mainBlock->Ptr ) ||
			( (uint)current + getNodeSize (current) + sizeof(CNodeBegin) + NL_HEAP_NODE_END_SIZE > 
				(uint)mainBlock->Ptr + mainBlock->Size )
		)
	{
		// Stop on error ?
		if (stopOnError)
		{
			// ********
			// * STOP *
			// ********
			// * The size value is corrupted: Check if the current node has
			// * overflow by the top or the previous node by the bottom.
			// ********
			// * (*previous):	previous node
			// * (*current):	current node
			// * (*next):		next node
			// ********
			NL_ALLOC_STOP;
		}

		// Node size is corrupted
		return false;
	}

	// Check Previous node pointer
	if ( !(current->Previous == NULL || 
		( ((uint)current->Previous <= (uint)current - sizeof (CNodeBegin) - NL_HEAP_NODE_END_SIZE) && 
		((uint)current->Previous >= (uint)mainBlock->Ptr) )
		) )
	{
		// Stop on error ?
		if (stopOnError)
		{
			// ********
			// * STOP *
			// ********
			// * The previous value is corrupted: Check if the current node has
			// * overflow by the top or the previous node by the bottom.
			// ********
			// * (*previous):	previous node
			// * (*current):	current node
			// * (*next):		next node
			// ********
			NL_ALLOC_STOP;
		}

		// Node flag is corrupted
		return false;
	}

	// Check FreeNode node pointer
	if (isNodeFree (current))
	{
		// Get only this free node
		const CFreeNode *freeNode = getFreeNode (current);
		checkFreeNode (freeNode, stopOnError, false);
	}

	// Ok
	return true;
}

// *********************************************************

inline bool CHeapAllocator::checkFreeNode (const CFreeNode *current, bool stopOnError, bool recurse) const
{
	// Not NULL ?
	if (current != &_NullNode.FreeNode)
	{
		// Get the node
		const CNodeBegin *node = getNode (current);

		// 1st rule: Node must be free
		if ( !isNodeFree(node) )
		{
			// Stop on error ?
			if (stopOnError)
			{
				// ********
				// * STOP *
				// ********
				// * Node should be free : Free tree is corrupted.
				// ********
				NL_ALLOC_STOP;
			}

			// Node Color is corrupted
			return false;
		}
		
		// 2nd rule: Node must be sorted 
		if	( 
				( current->Left != &_NullNode.FreeNode && getNodeSize (getNode (current->Left)) > getNodeSize (node) ) ||
				( current->Right != &_NullNode.FreeNode && getNodeSize (getNode (current->Right)) < getNodeSize (node) )
			)
		{
			// Stop on error ?
			if (stopOnError)
			{
				// ********
				// * STOP *
				// ********
				// * Node order is corrupted: Free tree is corrupted.
				// ********
				NL_ALLOC_STOP;
			}

			// Node Data is corrupted
			return false;
		}
		
		// 3rd rule: if red, must have two black nodes
		bool leftBlack = (current->Left == &_NullNode.FreeNode) || isNodeBlack(current->Left);
		bool rightBlack = (current->Right == &_NullNode.FreeNode) || isNodeBlack(current->Right);
		if ( !leftBlack && !rightBlack && isNodeRed (getFreeNode (node)) )
		{
			// Stop on error ?
			if (stopOnError)
			{
				// ********
				// * STOP *
				// ********
				// * Color is corrupted: Free tree is corrupted.
				// ********
				NL_ALLOC_STOP;
			}

			// Node Color is corrupted
			return false;
		}

		// If Parent NULL, must be the root
		if ( ( (current->Parent == NULL) && (_FreeTreeRoot != current) ) || ( (current->Parent != NULL) && (_FreeTreeRoot == current) ) )
		{
			// Stop on error ?
			if (stopOnError)
			{
				// ********
				// * STOP *
				// ********
				// * Parent pointer corrupted: Free tree is corrupted.
				// ********
				NL_ALLOC_STOP;
			}

			// Node Parent is corrupted
			return false;
		}

		// Recuse childern
		if (recurse)
		{
			if (!checkFreeNode (current->Left, stopOnError, recurse))
				return false;
			if (!checkFreeNode (current->Right, stopOnError, recurse))
				return false;
		}
	}

	// Ok
	return true;
}

// *********************************************************
// Synchronisation methods
// *********************************************************

inline void CHeapAllocator::enterCriticalSectionSB () const
{
	_MutexSB.enter ();
}

// *********************************************************

inline void CHeapAllocator::leaveCriticalSectionSB () const
{
	_MutexSB.leave ();
}

// *********************************************************

inline void CHeapAllocator::enterCriticalSectionLB () const
{
	_MutexLB.enter ();
}

// *********************************************************

inline void CHeapAllocator::leaveCriticalSectionLB () const
{
	_MutexLB.leave ();
}

// *********************************************************

inline void CHeapAllocator::enterCriticalSection () const
{
	enterCriticalSectionSB ();
	enterCriticalSectionLB ();
}

// *********************************************************

inline void CHeapAllocator::leaveCriticalSection () const
{
	leaveCriticalSectionLB ();
	leaveCriticalSectionSB ();
}

// *********************************************************






// *********************************************************
// *********************************************************

// Synchronized methods

// *********************************************************
// *********************************************************







// *********************************************************

inline uint CHeapAllocator::getMainBlockCount () const
{
	uint count = 0;
	CMainBlock *currentBlock = _MainBlockList;
	while (currentBlock)
	{
		currentBlock = currentBlock->Next;
		count++;
	}
	return count;
}

// *********************************************************
// Debug functions
// *********************************************************

#ifndef NLMISC_HEAP_ALLOCATION_NDEBUG

/*inline void CHeapAllocator::debugAddBreakpoint (uint32 allocateNumber)
{
	_Breakpoints.insert (allocateNumber);
}*/

// *********************************************************

/*inline void CHeapAllocator::debugRemoveBreakpoints ()
{
	_Breakpoints.clear ();
}*/

#endif // NLMISC_HEAP_ALLOCATION_NDEBUG

// *********************************************************

inline void	CHeapAllocator::setOutOfMemoryMode (TOutOfMemoryMode mode)
{
	enterCriticalSection ();

	_OutOfMemoryMode = mode;

	leaveCriticalSection ();
}

// *********************************************************

inline CHeapAllocator::TOutOfMemoryMode CHeapAllocator::getOutOfMemoryMode () const
{
	return _OutOfMemoryMode;
}

// *********************************************************

inline void	CHeapAllocator::setBlockAllocationMode (TBlockAllocationMode mode)
{
	enterCriticalSection ();

	_BlockAllocationMode = mode;

	leaveCriticalSection ();
}

// *********************************************************

inline CHeapAllocator::TBlockAllocationMode	CHeapAllocator::getBlockAllocationMode () const
{
	return _BlockAllocationMode;
}

// *********************************************************


inline const char *CHeapAllocator::getName () const
{
	return _Name;
}

// *********************************************************

inline uint CHeapAllocator::getMainBlockSize () const
{
	return _MainBlockSize;
}


// *********************************************************
// Small Block
// *********************************************************

inline CHeapAllocator::CNodeBegin *CHeapAllocator::getSmallBlock (CHeapAllocator::CSmallBlockPool *smallBlock, uint blockIndex)
{
	return (CHeapAllocator::CNodeBegin*)((uint8*)smallBlock + sizeof (CSmallBlockPool) + blockIndex * (sizeof(CNodeBegin) + smallBlock->Size + NL_HEAP_NODE_END_SIZE) );
}

// *********************************************************

inline CHeapAllocator::CNodeBegin *CHeapAllocator::getNextSmallBlock (CNodeBegin *previous)
{
	return previous->Previous;
}

// *********************************************************

inline void CHeapAllocator::setNextSmallBlock (CNodeBegin *previous, CNodeBegin *next)
{
	previous->Previous = next;
}

// *********************************************************

/*
void					*reallocate (void *ptr, uint size, const char *sourceFile, uint line, const char *category);
void					freeAll ();
void					releaseMemory ();
void					setMainBlockSize (uint mainBlockSize);
uint					getMainBlockSize () const;
bool					setMainBlockCount (uint blockCount);
uint					debugGetDebugInfoSize () const;
uint					debugGetAllocatedMemoryByCatgegory (const char* category) const;
void					debugReportMemoryLeak (std::vector<CMemoryLeakBlock> &result);
*/

#endif // NL_HEAP_ALLOCATOR_H
