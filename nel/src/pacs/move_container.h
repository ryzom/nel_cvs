/** \file move_container.h
 * Container for movable object
 *
 * $Id: move_container.h,v 1.1 2001/06/08 15:10:18 legros Exp $
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

#ifndef NL_MOVE_CONTAINER_H
#define NL_MOVE_CONTAINER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/pool_memory.h"
#include "nel/pacs/move_cell.h"
#include "nel/pacs/collision_ot.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/collision_surface_temp.h"

#define NELPACS_CONTAINER_TRIGGER_DEFAULT_SIZE 100

namespace NLPACS 
{

class CMovePrimitive;
class CMoveElement;
class CGlobalRetriever;

/**
 * A container for movable objects
 * Some constraints:
 * * The move bounding box must be lower than the cell size
 * 
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */

class CMoveContainer: public UMoveContainer
{
	friend class CMovePrimitive;
public:
	/// Constructor
	CMoveContainer (double xmin, double ymin, double xmax, double ymax, uint widthCellCount, uint heightCellCount, double primitiveMaxSize, 
		uint maxIteration=NELPACS_DEFAULT_MAX_TEST_ITERATION, uint otSize=100)
	{
		init (xmin, ymin, xmax, ymax, widthCellCount, heightCellCount, primitiveMaxSize, maxIteration, otSize);
	}

	/// Init the container with a global retriever
	CMoveContainer (CGlobalRetriever* retriever, uint widthCellCount, uint heightCellCount, double primitiveMaxSize, 
		uint maxIteration=NELPACS_DEFAULT_MAX_TEST_ITERATION, uint otSize=100)
	{
		init (retriever, widthCellCount, heightCellCount, primitiveMaxSize, maxIteration, otSize);
	}

	/// Destructor
	~CMoveContainer ();

	/// Init the container without global retriever
	void init (double xmin, double ymin, double xmax, double ymax, uint widthCellCount, uint heightCellCount, double primitiveMaxSize, 
		uint maxIteration=NELPACS_DEFAULT_MAX_TEST_ITERATION, uint otSize=100);

	/// Init the container with a global retriever
	void init (CGlobalRetriever* retriever, uint widthCellCount, uint heightCellCount, double primitiveMaxSize, 
		uint maxIteration=NELPACS_DEFAULT_MAX_TEST_ITERATION, uint otSize=100);

	/// Add a primitive in the container. Return the pointer on the primitive.
	UMovePrimitive*				addPrimitive ();

	/// Remove a primitive from the container.
	void						removePrimitive (UMovePrimitive* primitive);

	/// Evaluation of the collision system
	void						evalCollision (double deltaTime);

	/// Make a move test
	bool						testMove (UMovePrimitive* primitive, const NLMISC::CVectorD& speed, double deltaTime);

	/// Allocate a move element
	CMoveElement				*allocateMoveElement ();

	/// Free move element
	void						freeMoveElement (CMoveElement *element);

	/// Get the retriever
	CGlobalRetriever			*getGlobalRetriever() const
	{
		return _Retriever;
	}

	/// Get number of trigger informations
	uint						getNumTriggerInfo() const
	{
		return _Triggers.size();
	}

	/// Get the n-th trigger informations
	const UTriggerInfo			&getTriggerInfo (uint id) const
	{
		// check
		nlassert (id<_Triggers.size());
		
		return _Triggers[id];
	}

private:
	/// Current test time
	uint32						_TestTime;

	/// Max test iterations
	uint32						_MaxTestIteration;

	/// Set of primitives
	std::set<CMovePrimitive*>	_PrimitiveSet;

	/// Root of modified primitive
	CMovePrimitive				*_ChangedRoot;

	/// The time ordered table size
	uint						_OtSize;

	/// The time ordered table
	std::vector<CCollisionOT>	_TimeOT;

	/// Previous collision node in the OT
	CCollisionOT				*_PreviousCollisionNode;

	/// Current deltaTime
	double						_DeltaTime;

	/// Max primitive size
	double						_PrimitiveMaxSize;

	/// Area size
	double						_Xmin;
	double						_Ymin;
	double						_Xmax;
	double						_Ymax;

	/// Cells width and height
	double						_CellWidth;
	double						_CellHeight;

	/// Cells count 
	uint						_CellCountWidth;
	uint						_CellCountHeight;

	/// Cells array
	std::vector<CMoveCell>		_VectorCell;

	/// Retriver pointner
	CGlobalRetriever			*_Retriever;
	CCollisionSurfaceTemp		_SurfaceTemp;

	/// Memory manager for CCollisionOTInfo
	std::vector <UTriggerInfo>						_Triggers;
	NLMISC::CPoolMemory<CCollisionOTDynamicInfo>	_AllocOTDynamicInfo;
	NLMISC::CPoolMemory<CCollisionOTStaticInfo>		_AllocOTStaticInfo;

private:

	// Clear the container
	void						clear ();

	// Update modified primitives bounding box
	void						updatePrimitives (double deltaTime);

	// Update cells list for this primitive
	void						updateCells (CMovePrimitive *primitive);

	// Clear the time ordered table
	void						clearOT ();

	// Check the OT is cleared and linked
	void						checkOT ();

	// Eval one collision
	bool						evalOneCollision (double beginTime, CMovePrimitive *primitive, bool testMove=false);

	// Eval all collision for modified primitives
	void						evalAllCollisions (double beginTime);

	// Add a collision in the time ordered table
	void						newCollision (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc, 
												bool collision, bool enter, bool exit);

	// Add a collision in the time ordered table
	void						newCollision (CMovePrimitive* first, const CCollisionSurfaceDesc& desc, const CVector& delta);

	// Add a trigger in the trigger array
	void						newTrigger (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc);

	// Clear modified primitive list
	void						clearModifiedList ();

	// Remove modified primitive from time ordered table
	void						removeModifiedFromOT ();

	// Check sorted list
	void						checkSortedList ();

	// Allocate ordered table info
	CCollisionOTDynamicInfo		*allocateOTDynamicInfo ();

	// Allocate ordered table info
	CCollisionOTStaticInfo		*allocateOTStaticInfo ();

	// Free all ordered table info
	void						freeAllOTInfo ();

	// Allocate a primitive
	CMovePrimitive				*allocatePrimitive ();

	// Free a primitive
	void						freePrimitive (CMovePrimitive *primitive);

	// Called by CMovePrimitive when a change occured on the primitive BB
	void						changed (CMovePrimitive* primitive);

	// Remove the primitive from the modified list
	void						removeFromModifiedList (CMovePrimitive* primitive);

	// Unlink this move element
	void						unlinkMoveElement  (CMoveElement *element);

	// Reaction of the collision between two primitives. Return true if one object has been modified.
	void						reaction (const CCollisionOTInfo& first);
};


} // NLPACS


#endif // NL_MOVE_CONTAINER_H

/* End of move_container.h */
