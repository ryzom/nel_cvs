/** \file move_primitive.h
 * Description of movables primitives
 *
 * $Id: move_primitive.h,v 1.1 2001/05/22 08:24:49 corvazier Exp $
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

#ifndef NL_MOVE_PRIMITIVE_H
#define NL_MOVE_PRIMITIVE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/pacs/move_container.h"

namespace NLMISC
{
}

namespace NLPACS 
{

/**
 * Description of movables primitives.
 *
 * This primitive can be a 2d oriented box or a 2d oriented cylinder.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CMovePrimitive
{
	/*friend class CMoveContainer;
	friend class CMoveElement;*/
public:
	/// Primitive mode
	enum TMode
	{
		/**
		  * This is a static 2d oriented bounding box. It can be oriented only on the Z axis.
		  * It has a height. Collision can be performed only no its sides but not on its top and bottom
		  * planes. It doesn't mode.
		  */
		_2DOrientedBox=0,

		/** 
		  * This is a movable 2d oriented cylinder. It can be oriented only on the Z axis.
		  * It has a height. Collision can be performed only no its sides but not on its top and bottom
		  * planes. It can move only with 3d translations.
          */
		_2DOrientedCylinder=1,
	};

	// Some flags
	enum TFlags
	{
		// Mask for the primitive type
		PrimitiveMask=0xff,

		// The dirt flag. Precalculated data for the position must be recomputed.
		DirtPosFlag=0x100,

		// The dirt bounding box. Precalculated data for the bounding box must be recomputed.
		DirtBBFlag=0x200,

		// In modified list.
		InModifiedListFlag=0x400,

		// Obstacle flag. This flag tells that this object is an obstacle for athers objects.
		ObstacleFlag=0x800,

		/* Primitive in the linked list. This flag is set when the primitive is in a linked list.
		   _NextX and _PreviousX pointers are valid or NULL at the queues of the list.
		   If this flag is not set, _PreviousX and _NextX points on an entry of the list but this
		   primitive is not linked in the list. */
		InLinkedListFlag=0x1000,

		// Primitive is being removed from the time ordered table.
		BeingRemovedFromTOTFlag=0x2000,

		// Force the size to uint32.
		ForceSize=0xffffffff
	};

	/// Constructor
	CMovePrimitive ();

	/// Destructor
	~CMovePrimitive ();
	/**
	  * Set the primitive type.
	  *
	  * \param type is the new primitive type.
	  */
	void	setType (TMode type)
	{
		// New position
		_Flags&=~(uint32)PrimitiveMask;
		_Flags|=type;

		// Something has changed
		dirtPos ();
	}

	/**
	  * Set the obstacle flag.
	  *
	  * \param obstacle is true if this primitive is an obstacle, else false.
	  */
	void	setObstacle (bool obstacle)
	{
		// New flag
		if (obstacle)
			_Flags|=ObstacleFlag;
		else
			_Flags&=~(uint32)ObstacleFlag;

		// Something has changed
		dirtPos ();
	}

	/**
	  * Set the position of the move primitive. For movable primitives, this is
	  * the position at the primitive current time.
	  *
	  * \param pos is the new position of the primitive.
	  */
	void	setPosition (const NLMISC::CVector& pos)
	{
		// New position
		_Position=pos;

		// Something has changed
		dirtPos ();
	}

	/**
	  * Set the new orientation of the move primitive. Only for the box primitives.
	  *
	  * \param rot is the new OZ rotation in radian.
	  */
	void	setOrientation (float rot)
	{
		// Checks
		nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);

		// New position
		_OBData.Orientation=rot;

		// Position has changed
		dirtPos ();
	}

	/**
	  * Set the box size. Only for boxes.
	  *
	  * \param width is the new width size of the box. It the size of the sides aligned on OX.
	  * \param depth is the new depth size of the box. It the size of the sides aligned on OY.
	  */
	void	setSize (float width, float depth)
	{
		// Checks
		nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);

		// New position
		_OBData.Length[0]=width;
		_OBData.Length[1]=depth;

		// Position has changed
		dirtPos ();
	}

	/**
	  * Set the cylinder size. Only for cylinder.
	  *
	  * \param radius is the new radius size of the cylinder.
	  */
	void	setRadius (float radius)
	{
		// Checks
		nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);

		// New position
		_OCData.Radius=radius;

		// Position has changed
		dirtPos ();
	}

	/**
	  * Set the speed vector for this primitive. Only for movable primitives.
	  *
	  * \param speed is the new speed vector.
	  */
	void	setSpeed (const NLMISC::CVector& speed)
	{
		// New time
		_Speed=speed;

		// Speed has changed
		dirtPos ();
	}

	/**
	  * Return true if this primitive is an obstacle else false.
	  */
	bool	isObstacle () const
	{
		return ((_Flags&(uint32)ObstacleFlag)!=0);
	}

	/**
	  * Return true if this primitive is an obstacle else false.
	  */
	bool	isLinkedInList () const
	{
		return ((_Flags&(uint32)InLinkedListFlag)!=0);
	}

	/// Clear the inModifiedList flag.
	void	clearInModifiedListFlag ()
	{
		_Flags&=~InModifiedListFlag;
	}

	/**
	  * Return min of the bounding box in X.
	  */
	float	getBBXMin () const
	{
		return _BBXMin;
	}

	/**
	  * Return min of the bounding box in Y.
	  */
	float	getBBYMin () const
	{
		return _BBYMin;
	}

	/**
	  * Return max of the bounding box in X.
	  */
	float	getBBXMax () const
	{
		return _BBXMax;
	}

	/**
	  * Return max of the bounding box in Y.
	  */
	float	getBBYMax () const
	{
		return _BBYMax;
	}

	/**
	  * Eval collisions with the other primitive.
	  *
	  * \param other is another move primitive to test collisions with.
	  * \param desc is a collision descriptor filled with information
	  * about the collision context if the method return true.
	  * \param timeMin is the time you want to clip collision result in the past.
	  * \param timeMax is the time you want to clip collision result in the futur.
	  *
	  * \return true if a collision has been detected in the time range, else false.
	  */
	bool	evalCollision (CMovePrimitive& other, class CCollisionDesc& desc, float timeMin, float timeMax);

	/// Return the nieme MoveElement. The primitive can have 4 move elements. Can be NULL if the ineme elment is not in a list
	CMoveElement	*getMoveElement (uint i)
	{
		return _MoveElement[i];
	}

	/// Remove the nieme MoveElement.
	void removeMoveElement (uint i);

	/// Add the primitive in the cell
	void addMoveElement (CMoveCell& cell, float centerX, float centerY);

	/// Get next modified primitive
	CMovePrimitive	*getNextModified () const
	{
		return _NextModified;
	}

	/// Add a collision time ordered table element
	void addCollisionOTInfo (CCollisionOTInfo *info)
	{
		// Link to the list
		info->link (_RootOTInfo);
		_RootOTInfo=info;
	}

	/// Remove all collision time ordered table element higher than beginTime.
	void removeCollisionOTInfo (float beginTime);

	/// Clear the collision list.
	void clearCollisionList ()
	{
		_RootOTInfo=NULL;
	}

	/// Update precalculated data
	void update (float beginTime, float endTime)
	{
		// Pos dirt ?
		if (_Flags&DirtPosFlag)
		{
			// Compute precalculated data
			precalcPos ();

			// Clean
			_Flags&=~DirtPosFlag;
		}

		// Bounding box dirt ?
		if (_Flags&DirtBBFlag)
		{
			// Compute precalculated data
			precalcBB (beginTime, endTime);

			// Clean
			_Flags&=~DirtBBFlag;
		}
	}

	/// Check sorted lists
	void checkSortedList ();

private:

	// Unique constructor
	CMovePrimitive (CMoveContainer* container)
	{
		_Container=container;
	}

	// Dirt the position flag. Position has changed.
	void	dirtPos ()
	{
		_Flags|=DirtPosFlag;
		dirtBB ();
	}

	// Dirt the bounding box flag.
	void	dirtBB ()
	{
		// Warn container that BB has changed
		_Container->changed (this);

		_Flags|=DirtBBFlag;
	}

	// Compute precalculated data for the position
	void precalcPos ();

	// Compute precalculated data for the speed
	void precalcSpeed ();

	// Compute precalculated bounding box
	void precalcBB (float beginTime, float endTime);

	// Update sorted lists in X and Y
	void updateSortedLists ();

	// *** Some methods to eval collisions
	/* * NOTES:		BB for bounding box
					OB for oriented Box
					OC for oriented cylinder
					P for box point
					S for box segment
	*/

	// Bounding box test. Return true if there is a potential collision, else false.
	bool	evalCollisionBB (CMovePrimitive& other)
	{
		// Bounding box test
		return (	(_BBXMin<=other._BBXMax) && 
					(_BBXMax>=other._BBXMin) && 
					(_BBYMin<=other._BBYMax) && 
					(_BBYMax>=other._BBYMin)	);
	}

	// *** Primitive over primitive

	// Box over box
	bool	evalCollisionOBoverOB (CMovePrimitive& other, CCollisionDesc& desc, float timeMin, float timeMax);

	// Box over cylinder
	bool	evalCollisionOBoverOC (CMovePrimitive& other, CCollisionDesc& desc, float timeMin, float timeMax);

	// Cylinder over cylinder
	bool	evalCollisionOCoverOC (CMovePrimitive& other, CCollisionDesc& desc, float timeMin, float timeMax);

	// *** Subprimitive over subprimitive

	// Point over segment in OB/OB test
	bool	evalCollisionPoverS (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, uint numSeg, float timeMin, float timeMax);

	// Point over cylinder
	bool	evalCollisionPoverOC (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, float timeMin, float timeMax);

	// Segment over cylinder
	bool	evalCollisionSoverOC (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, float timeMin, float timeMax);
	
private:

	// Container of this primitive
	CMoveContainer		*_Container;

	// This position is the central bottom position for the box or for the cylinder
	NLMISC::CVector		_Position;
	NLMISC::CVector		_Speed;

	// This is the height of the box or of the cylinder.
	float				_Height;

	// Flags
	uint32				_Flags;

	// Movable bounding box
	float				_BBXMin;
	float				_BBYMin;
	float				_BBXMax;
	float				_BBYMax;

	// Pointer into the 4 possibles sorted lists of movable primitives. Can be NULL if not in the list
	CMoveElement		*_MoveElement[4];

	// Sorted list of modified primitives. This pointer is managed by CMoveContainer.
	CMovePrimitive		*_NextModified;

	// List of time ordered table element using this primitive
	CCollisionOTInfo	*_RootOTInfo;

	// Union for data space shared by primitive type (cylinder or box)
	union 
	{
		// Data for Boxes
		struct
		{
			// 2d position of the 4 points of the box at initial time.
			float PointPosX[4];
			float PointPosY[4];

			// The normalized direction vector of the 4 edges of the box.
			float EdgeDirectionX[4];
			float EdgeDirectionY[4];

			// The length of the 4 edges. The first is the width, the second is the depth
			float Length[2];

			// The box orientation
			float Orientation;
		} _OBData;

		// Data for Cylinders
		struct
		{
			// Radius of the cylinder
			float Radius;

			// Square radius
			float SquareRadius;
		} _OCData;
	};
};

} // NLPACS


#endif // NL_MOVE_PRIMITIVE_H

/* End of move_primitive.h */
