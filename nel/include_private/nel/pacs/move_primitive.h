/** \file move_primitive.h
 * Description of movables primitives
 *
 * $Id: move_primitive.h,v 1.3 2001/06/06 09:34:03 corvazier Exp $
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
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/global_retriever.h"

#define NELPACS_DIST_BACK 0.002

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
class CMovePrimitive: public UMovePrimitive
{
private:

	// Some flags
	enum TFlags
	{
		// Mask for the primitive type
		PrimitiveMask=0x0f,

		// Mask for the primitive type
		ReactionMask=0xf0,

		// Mask for the trigger type
		TriggerMask=0xf00,

		// The dirt flag. Precalculated data for the position must be recomputed.
		DirtPosFlag=0x10000,

		// The dirt bounding box. Precalculated data for the bounding box must be recomputed.
		DirtBBFlag=0x20000,

		// In modified list.
		InModifiedListFlag=0x40000,

		// Obstacle flag. This flag tells that this object is an obstacle for others objects.
		ObstacleFlag=0x80000,

		// Force the size to uint32.
		ForceSize=0xffffffff
	};

public:

	/// Constructor
	CMovePrimitive (CMoveContainer* container);

	/// Destructor
	~CMovePrimitive ();

	/**
	  * Set the primitive type.
	  *
	  * \param type is the new primitive type.
	  */
	void	setPrimitiveType (TType type)
	{
		// New position
		_Flags&=~(uint32)PrimitiveMask;
		_Flags|=type;

		// Something has changed
		dirtPos ();
	}

	/**
	  * Set the reaction type.
	  *
	  * \param type is the new reaction type.
	  */
	void	setReactionType (TReaction type)
	{
		// New position
		_Flags&=~(uint32)ReactionMask;
		_Flags|=type;
	}

	/**
	  * Set the trigger type. Default type is NotATrigger.
	  *
	  * \param type is the new trigger type.
	  */
	void	setTriggerType (TTrigger type)
	{
		// New position
		_Flags&=~(uint32)TriggerMask;
		_Flags|=type;
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
	  * Set the global position of the move primitive. Setting the global position 
	  * can take a long time if you use a UGlobalRetriever. Set the position with
	  * this method only the first time or for teleporting.
	  *
	  * \param pos is the new global position of the primitive.
	  */
	void	setGlobalPosition (const NLMISC::CVectorD& pos, const UMoveContainer& container);

	/**
	  * Set the position of the move primitive. For movable primitives, this is
	  * the position at the primitive current time.
	  *
	  * \param pos is the new position of the primitive.
	  */
/*	void	setPosition (const NLMISC::CVectorD& pos)
	{
		// New position
		_Position=pos;

		// Something has changed
		dirtPos ();
	}*/

	/**
	  * Get the position of the move primitive at the end of the movement.
	  *
	  * \return the new position of the primitive.
	  */
	NLMISC::CVectorD	getFinalPosition () const
	{
		return _Position.getPos();
	}

	/**
	  * Set the new orientation of the move primitive. Only for the box primitives.
	  *
	  * \param rot is the new OZ rotation in radian.
	  */
	void	setOrientation (double rot)
	{
		// Checks
		nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);

		// New position
		_OBData.Orientation=rot;

		// Position has changed
		dirtPos ();
	}

	/**
	  * Set the collision mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new collision mask.
	  */
	void	setCollisionMask (TCollisionMask mask)
	{
		_CollisionMask=mask;
	}

	/**
	  * Set the occlusion mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new occlusion mask.
	  */
	void	setOcclusionMask (TCollisionMask mask)
	{
		_OcclusionMask=mask;
	}

	/**
	  * Set the attenuation of collision for this object. Default value is 1. Should be between 0~1.
	  * 0, all the enrgy is attenuated by the collision. 1, all the energy stay in the object.
	  * Used only with the flag Reflexion.
	  *
	  * \param attenuation is the new attenuation for the primitive.
	  */
	void	setAbsorbtion (float attenuation)
	{
		// New flag
		_Attenuation=attenuation;
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
	  * Set the height. For boxes or cylinder.
	  *
	  * \param height is the new height size of the box. It the size of the sides aligned on OZ.
	  */
	void	setHeight (float height)
	{
		// New size
		_Height=height;

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
		nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedCylinder);

		// New position
		_OCData.Radius=radius;

		// Position has changed
		dirtPos ();
	}

	/**
	  * Set the speed vector for this primitive. Only for movable primitives.
	  *
	  * \param speed is the speed of the primitive.
	  */
	void	move (const NLMISC::CVectorD& speed);

	/**
	  * Set the speed vector for this primitive.
	  *
	  * \param speed is the new speed vector.
	  */
	void	setSpeed (const NLMISC::CVectorD& speed)
	{
		// New time
		_Speed=speed;

		// Speed has changed
		dirtPos ();
	}

	/**
	  * Get the speed vector for this primitive.
	  *
	  * \Return the new speed vector.
	  */
	const NLMISC::CVectorD&	getSpeed () const
	{
		// New time
		return _Speed;
	}

	/**
	  * Return true if this primitive is an obstacle else false.
	  */
	bool	isObstacle () const
	{
		return ((_Flags&(uint32)ObstacleFlag)!=0);
	}

	/// Is in modified list ?
	bool	isInModifiedListFlag ()
	{
		return (_Flags&InModifiedListFlag) != 0;
	}

	/// Clear the inModifiedList flag.
	void	setInModifiedListFlag (bool itis)
	{
		if (itis)
			_Flags|=InModifiedListFlag;
		else
			_Flags&=~InModifiedListFlag;
	}

	// Link into modified list
	void	linkInModifiedList (CMovePrimitive* next)
	{
		_NextModified=next;
	}

	/// Get next modified primitive
	CMovePrimitive	*getNextModified () const
	{
		return _NextModified;
	}

	/**
	  * Return min of the bounding box in X.
	  */
	double	getBBXMin () const
	{
		return _BBXMin;
	}

	/**
	  * Return min of the bounding box in Y.
	  */
	double	getBBYMin () const
	{
		return _BBYMin;
	}

	/**
	  * Return max of the bounding box in X.
	  */
	double	getBBXMax () const
	{
		return _BBXMax;
	}

	/**
	  * Return max of the bounding box in Y.
	  */
	double	getBBYMax () const
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
	bool	evalCollision (CMovePrimitive& other, class CCollisionDesc& desc, double timeMin, double timeMax, uint32 testTime, 
							uint32 maxTestIteration, double &firstContactTime, double &lastContactTime);

	// Check trigger flag
	bool	isTriggered (CMovePrimitive& second, bool enter, bool exit);

	/**
	  * Eval collisions with the global retriever.
	  *
	  * \param retriever is the global retriever used to test collision
	  * \param timeMin is the time you want to clip collision result in the past.
	  * \param timeMax is the time you want to clip collision result in the futur.
	  *
	  * \return true if a collision has been detected in the time range, else false.
	  */
	const TCollisionSurfaceDescVector *CMovePrimitive::evalCollision (CGlobalRetriever &retriever, CCollisionSurfaceTemp& surfaceTemp, 
																  const NLMISC::CVector& delta, uint32 testTime, uint32 maxTestIteration);

	// Make a move with globalRetriever. Must be call after a free collision evalCollision call.
	void	doMove (CGlobalRetriever &retriever, CCollisionSurfaceTemp& surfaceTemp, double timeMax);

	// Make a move wihtout globalRetriever.
	void	doMove (double timeMax);

	/// Return the nieme MoveElement. The primitive can have 4 move elements. Can be NULL if the ineme elment is not in a list
	CMoveElement	*getMoveElement (uint i)
	{
		return _MoveElement[i];
	}

	/// Remove the nieme MoveElement.
	void removeMoveElement (uint i);

	/// Add the primitive in the cell
	void addMoveElement (CMoveCell& cell, uint16 x, uint16 y, double centerX, double centerY);

	/// Add a collision time ordered table element
	void addCollisionOTInfo (CCollisionOTInfo *info)
	{
		// Link to the list
		info->primitiveLink (this, _RootOTInfo);
		_RootOTInfo=info;
	}

	/// Remove one collision time ordered table element.
	void removeCollisionOTInfo (CCollisionOTInfo *toRemove);

	/// Remove all collision time ordered table element.
	void removeCollisionOTInfo ();

	/// Update precalculated data
	void update (double beginTime, double endTime)
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

	// Reaction between two primitives. Return true if one object has been modified.
	void reaction (CMovePrimitive& second, const CCollisionDesc& desc, CGlobalRetriever* retriver,
					CCollisionSurfaceTemp& surfaceTemp, bool collision);

	// Reaction with a static collision. Return true if one object has been modified.
	void reaction (const CCollisionSurfaceDesc&	surfaceDesc, const CGlobalRetriever::CGlobalPosition& globalPosition,
					const CGlobalRetriever& retriever, double deltaTime);

	// Return the global position of the primitive
	const CGlobalRetriever::CGlobalPosition& getGlobalPosition()
	{
		return _Position.getGlobalPos();
	}

private:

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
	void precalcBB (double beginTime, double endTime);

	// *** Some methods to eval collisions
	/* * NOTES:		BB for bounding box
					OB for oriented Box
					OC for oriented cylinder
					P for box point
					S for box segment
	*/

	// *** Get mass
	
	float getMass () const
	{
		// Box ?
		if ( (_Flags&PrimitiveMask) == _2DOrientedBox )
			return _OBData.Length[0]*_OBData.Length[1]*_Height;
		// Cylinder ?
		else
		{
			nlassert ( (_Flags&PrimitiveMask) == _2DOrientedCylinder );
			return _OCData.Radius*(float)NLMISC::Pi*_Height;
		}
	}

	// Test time. Return true if tetst can be perform, false if too many test have been computed for this primitive
	bool checkTestTime (uint32 testTime, uint32 maxTestIteration)
	{
		// Already checked for this test time ?
		if (testTime!=_LastTestTime)
		{
			// First time this primitive is visited at this test
			_LastTestTime=testTime;

			// Test counter
			_IterationCount=maxTestIteration;
		}
		else
		{
			// Too many test this primitive ?
			if (_IterationCount<=0)
				return false;
			else
				_IterationCount--;
		}
		// Ok, test can be performed
		return true;
	}


	// *** Primitive over primitive

	// Box over box
	bool	evalCollisionOBoverOB (CMovePrimitive& other, CCollisionDesc& desc, double timeMin, 
									double timeMax, double &firstContactTime, double &lastContactTime);

	// Box over cylinder
	bool	evalCollisionOBoverOC (CMovePrimitive& other, CCollisionDesc& desc, double timeMin, 
									double timeMax, double &firstContactTime, double &lastContactTime);

	// Cylinder over cylinder
	bool	evalCollisionOCoverOC (CMovePrimitive& other, CCollisionDesc& desc, double timeMin, 
									double timeMax, double &firstContactTime, double &lastContactTime);

	// *** Subprimitive over subprimitive

	// Point over segment in OB/OB test
	bool	evalCollisionPoverS (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, uint numSeg);

	// Point over cylinder
	bool	evalCollisionPoverOC (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, 
								double &firstContactTime, double &lastContactTime);

	// Segment over cylinder
	bool	evalCollisionSoverOC (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint);
	
private:
	// Last primitive test time
	uint32				_LastTestTime;

	// Iteration count
	sint32				_IterationCount;

	// Occlusion mask
	TCollisionMask		_OcclusionMask;

	// Collision mask
	TCollisionMask		_CollisionMask;

	// Container of this primitive
	CMoveContainer		*_Container;

	// This position is the central bottom position for the box or for the cylinder
	class CPosition
	{
	private:
		// 3d position
		NLMISC::CVectorD					_3dPosition;

		// Global position
		CGlobalRetriever::CGlobalPosition	_GlobalPosition;
	public:
		// Return the 3d position
		const NLMISC::CVectorD&						getPos () const
		{
			return _3dPosition;
		}

		// Set the 3d position
		void										setPos (const NLMISC::CVectorD& newPos)
		{
			_3dPosition=newPos;
		}

		// Return the global position
		const CGlobalRetriever::CGlobalPosition&	getGlobalPos () const
		{
			return _GlobalPosition;
		}

		// Set the global position
		void										setGlobalPos (const CGlobalRetriever::CGlobalPosition& globalPosition,
																  const CGlobalRetriever& globalRetriver)
		{
			// Get position with global position
			_GlobalPosition=globalPosition;
			_3dPosition=globalRetriver.getDoubleGlobalPosition (globalPosition);
		}
	}					_Position;

	// 3d position at t=0
	NLMISC::CVectorD	_3dInitPosition;

	// Current speed
	NLMISC::CVectorD	_Speed;

	// Time for valid getPos () position.  _Position.getPos () == _3dInitPosition + _Speed * _InitTime;
	double				_InitTime;

	// This is the height of the box or of the cylinder.
	float				_Height;

	// Attenuation 
	float				_Attenuation;

	// Flags
	uint32				_Flags;

	// Movable bounding box
	double				_BBXMin;
	double				_BBYMin;
	double				_BBXMax;
	double				_BBYMax;

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
			double PointPosX[4];
			double PointPosY[4];

			// The normalized direction vector of the 4 edges of the box.
			double EdgeDirectionX[4];
			double EdgeDirectionY[4];

			// The length of the 4 edges. The first is the width, the second is the depth
			float Length[2];

			// The box orientation
			double Orientation;
		} _OBData;

		// Data for Cylinders
		struct
		{
			// Radius of the cylinder
			float Radius;
		} _OCData;
	};
};

} // NLPACS

#include "nel/pacs/move_container_inline.h"

#endif // NL_MOVE_PRIMITIVE_H

/* End of move_primitive.h */
