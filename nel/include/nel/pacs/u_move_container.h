/** \file u_move_container.h
 * A container for movable objects
 *
 * $Id: u_move_container.h,v 1.3 2001/06/07 12:42:55 corvazier Exp $
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

#ifndef NL_U_MOVE_CONTAINER_H
#define NL_U_MOVE_CONTAINER_H

#include "nel/misc/types_nl.h"

namespace NLPACS 
{

class UMovePrimitive;
class UTriggerInfo;
class UGlobalRetriever;

#define NELPACS_DEFAULT_OT_SIZE 100
#define NELPACS_DEFAULT_MAX_TEST_ITERATION 100

/**
 * A container for movable objects
 * Some constraints:
 * * The move bounding box must be lower than the cell size
 * 
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UMoveContainer
{
public:

	/// \name Manage primitives.

	/// Add a primitive in the container. Return the pointer on the primitive.
	virtual UMovePrimitive		*addPrimitive () =0;

	/// Remove a primitive from the container.
	virtual void				removePrimitive (UMovePrimitive* primitive) =0;

	/// \name System functions.

	/// Evaluation of the collision system
	virtual void				evalCollision (double deltaTime) =0;

	/// Make a move test
	virtual bool				testMove (UMovePrimitive* primitive, const NLMISC::CVectorD& speed, double deltaTime) =0;

	/// \name Triggers info.

	/// Get number of trigger informations
	virtual uint				getNumTriggerInfo() const=0;

	/// Get the n-th trigger informations
	virtual const UTriggerInfo &getTriggerInfo (uint id) const=0;

	/// \name Create methods.

	// Create method
	static 	UMoveContainer		*createMoveContainer (double xmin, double ymin, double xmax, double ymax, 
		uint widthCellCount, uint heightCellCount, double primitiveMaxSize, uint maxIteration=NELPACS_DEFAULT_MAX_TEST_ITERATION, 
		uint otSize=NELPACS_DEFAULT_OT_SIZE);

	// Create method
	static 	UMoveContainer		*createMoveContainer (UGlobalRetriever* retriever, uint widthCellCount, 
		uint heightCellCount, double primitiveMaxSize, uint maxIteration=NELPACS_DEFAULT_MAX_TEST_ITERATION, 
		uint otSize=NELPACS_DEFAULT_OT_SIZE);

	// Delete method
	static 	void				deleteMoveContainer (UMoveContainer	*container);
};


} // NLPACS


#endif // NL_U_MOVE_CONTAINER_H

/* End of u_move_container.h */
