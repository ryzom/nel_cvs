/** \file move_container.h
 * Container for movable object
 *
 * $Id: move_container.h,v 1.1 2001/05/22 08:24:49 corvazier Exp $
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

namespace NLPACS 
{

class CMovePrimitive;
class CMoveElement;

/**
 * A container for movable objects
 * Some constraints:
 * * The move bounding box must be lower than the cell size
 * 
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */

class CMoveContainer
{
	friend class CMovePrimitive;
	friend class ICollisionCallback;
public:
	/// Constructor
	CMoveContainer (float xmin, float ymin, float xmax, float ymax, uint widthCellCount, uint heightCellCount, float primitiveMaxSize, uint otSize=100);

	/// Destructor
	~CMoveContainer ();

	/// Init the container
	void init (float xmin, float ymin, float xmax, float ymax, uint widthCellCount, uint heightCellCount, float primitiveMaxSize, uint otSize=100);

	/// Add a primitive in the container. Return the pointer on the primitive.
	CMovePrimitive*				addPrimitive ();

	/// Remove a primitive from the container.
	void						removePrimitive (CMovePrimitive* primitive);

	/// Begin evaluation of collisions
	void						evalBegin (float deltaTime, ICollisionCallback& callBack);

	/* 
	 * Evaluation of a collision
	 *
	 * You must call evalBegin first.
	 * If it returns true, tehh system has found a collision, call the callback and then resolve the collision.
	 * Recall evalCollision () still it returns false.
	 */
	bool						evalCollision ();

	/// Allocate a move element
	CMoveElement				*allocateMoveElement ();

	/// Free move element
	void						freeMoveElement (CMoveElement *element);

private:
	// Set of primitives
	std::set<CMovePrimitive*>	_PrimitiveSet;

	// Root of modified primitive
	CMovePrimitive				*_ChangedRoot;

	// The time ordered table
	uint						_OtSize;
	std::vector<CCollisionOT>	_TimeOT;
	CCollisionOTInfo			*_NextCollision;

	float						_DeltaTime;

	bool						_Begin;

	ICollisionCallback			*_Callback;

	// Max primitive size
	float						_PrimitiveMaxSize;

	float						_Xmin;
	float						_Ymin;
	float						_Xmax;
	float						_Ymax;
	float						_CellWidth;
	float						_CellHeight;
	uint						_CellCountWidth;
	uint						_CellCountHeight;
	std::vector<CMoveCell>		_VectorCell;

	// Memory manager for CCollisionOTInfo
	NLMISC::CPoolMemory<CCollisionOTInfo>	_AllocOTInfo;

private:

	// Clear the container
	void						clear ();

	// Update modified primitives bounding box
	void						updatePrimitives (float deltaTime);

	// Update cells list for this primitive
	void						updateCells (CMovePrimitive *primitive);

	// Clear the time ordered table
	void						clearOT ();

	// Clear modified primitive collision list
	void						clearModifiedPrimitiveCollisionList ();

	// Eval all collision for modified primitives
	void						evalAllCollisions (float beginTime);

	// Add a collision in the time ordered table
	void						newCollision (CMovePrimitive* first, CMovePrimitive* second, const CCollisionDesc& desc);


	// Clear modified primitive list
	void						clearModifiedList ();

	// Remove modified primitive from time ordered table
	void						removeModifiedFromOT (float beginTime);

	// Check sorted list
	void						checkSortedList ();

	// Allocate ordered table info
	CCollisionOTInfo			*allocateOTInfo ();

	// Free all ordered table info
	void						freeAllOTInfo ();

	// Allocate a primitive
	CMovePrimitive				*allocatePrimitive ();

	// Free a primitive
	void						freePrimitive (CMovePrimitive *primitive);

	// Called by CMovePrimitive when a change occured on the primitive BB
	void						changed (CMovePrimitive* primitive);
};


} // NLPACS


#endif // NL_MOVE_CONTAINER_H

/* End of move_container.h */
