/** \file u_move_container.h
 * A container for movable objects
 *
 * $Id: u_move_container.h,v 1.1 2001/05/31 14:21:39 corvazier Exp $
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

	/// Add a primitive in the container. Return the pointer on the primitive.
	virtual UMovePrimitive		*addPrimitive () =0;

	/// Remove a primitive from the container.
	virtual void				removePrimitive (UMovePrimitive* primitive) =0;

	/// Begin evaluation of collisions
	virtual void				evalBegin (double deltaTime) =0;

	/* 
	 * Evaluation of a collision
	 *
	 * You must call evalBegin first.
	 * If it returns true, tehh system has found a collision, call the callback and then resolve the collision.
	 * Recall evalCollision () still it returns false.
	 */
	virtual bool				evalCollision () =0;

	// Create method
	static 	UMoveContainer		*createMoveContainer (double xmin, double ymin, double xmax, double ymax, 
		uint widthCellCount, uint heightCellCount, double primitiveMaxSize, uint otSize=100);

	// Delete method
	static 	void				deleteMoveContainer (UMoveContainer	*container);
};


} // NLPACS


#endif // NL_U_MOVE_CONTAINER_H

/* End of u_move_container.h */
