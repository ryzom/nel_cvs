/** \file move_primitive.h
 * Description of movables primitives
 *
 * $Id: move_primitive.h,v 1.1 2001/05/04 14:36:59 corvazier Exp $
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
public:

	/// Primitive mode
	enum TMode
	{
		/**
		  * This is a static 2d oriented bounding box. It can be oriented only on the Z axis.
		  * It has a height. Collision can be performed only no its sides but not on its top and bottom
		  * planes. It doesn't mode.
		  */
		Static2DOrientedBox=0,

		/** 
		  * This is a movable 2d oriented bounding box. It can be oriented only on the Z axis.
		  * It has a height. Collision can be performed only no its sides but not on its top and bottom
		  * planes. It can move only with 3d translations. When it moves, it keeps the same orientation.
          */
		Movable2DOrientedBox=1,

		/**
		  * This is a static 2d oriented cylinder. The circle side is in the OZ plane.
		  * Collisions can be performed on its sides but not on its top and bottom planes.
		  * It doesn't mode.
		  */
		Static2DOrientedCylinder=2,

		/**
		  * This is a static 2d oriented cylinder. The circle side is in the OZ plane.
		  * Collisions can be performed on its sides but not on its top and bottom planes.
		  * It can move with 3d translations.
		  */
		Movable2DOrientedCylinder=3,
	};

private:
	
	// Some flags
	enum TFlags
	{
		// Mask for the primitive type
		PrimitiveMask=0xff,

		// The dirt flag. Precalculated data for the position must be recomputed.
		DirtPosFlag=0x100,

		// The dirt speed. Precalculated data for the speed must be recomputed.
		DirtSpeedFlag=0x100,

		// Force the size to uint32.
		ForceSize=0xffffffff
	};

public:

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
		nlassert ( ((((uint32)_Flags)&PrimitiveMask)==Static2DOrientedBox) ||
			((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedBox) );

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
		nlassert ( ((((uint32)_Flags)&PrimitiveMask)==Static2DOrientedBox) ||
			((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedBox) );

		// New position
		_OBData.Length[0]=width;
		_OBData.Length[1]=depth;

		// Position has changed
		dirtPos ();
	}

	/**
	  * Set the current time of the move primitive. Only for movable primitive.
	  *
	  * \param time is the current time of the primitive.
	  */
	void	setTime (float time)
	{
		// Checks
		nlassert ( ((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedBox) ||
			((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedCylinder) );

		// New time
		_MData.Time=time;
	}

	/**
	  * Set the speed vector for this primitive. Only for movable primitives.
	  *
	  * \param speed is the new speed vector.
	  */
	void	setSpeed (const NLMISC::CVector& speed)
	{
		// Checks
		nlassert ( ((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedBox) ||
			((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedCylinder) );

		// New time
		_MData.SpeedX=speed.x;
		_MData.SpeedY=speed.y;
		_MData.SpeedZ=speed.z;

		// Speed has changed
		dirtSpeed ();
	}

	/**
	  * Eval collisions with the other primitive.
	  *
	  * \param other is another move primitive to test collisions with.
	  * \param desc is a collision descriptor filled with information
	  * about the collision context if the method return true.
	  * \return true if a collision has been detected, else false.
	  */
	bool	evalCollision (CMovePrimitive& other, class CCollisionDesc& desc);

private:

	// Update precalculated data
	void update ()
	{
		// Pos dirt ?
		if (_Flags&DirtPosFlag)
		{
			// Compute precalculated data
			precalcPos ();

			// Clean
			_Flags&=~DirtPosFlag;
		}

		// Speed dirt ?
		if (_Flags&DirtSpeedFlag)
		{
			// Compute precalculated data
			precalcSpeed ();

			// Clean
			_Flags&=~DirtSpeedFlag;
		}
	}

	// Dirt the position flag. Position has changed.
	void	dirtPos ()
	{
		_Flags|=DirtPosFlag;
	}

	// Dirt the position flag. Position has changed.
	void	dirtSpeed ()
	{
		_Flags|=DirtSpeedFlag;
	}

	// Compute precalculated data for the position
	void precalcPos ();

	// Compute precalculated data for the speed
	void precalcSpeed ();

	/**
	  * Some notation for the different move primitives
	  *
	  * SOB: Static Oriented 2d Box
	  * MOB: Movable Oriented 2d Box
	  * SOC: Static Oriented 2d Cylinder
	  * MOC: Static Oriented 2d Cylinder
	  * SP:  Static 3d Point
	  * MP:  Movable 3d Point
	  * SS:  Static 2d segment with height
	  * MS:  Movable 2d segment with height
	  */

	// Some methods to eval collisions
	bool	evalCollisionSOBoverMOB (CMovePrimitive& other, CCollisionDesc& desc);
	bool	evalCollisionSOBoverMOC (CMovePrimitive& other, CCollisionDesc& desc);
	bool	evalCollisionMOBoverMOB (CMovePrimitive& other, CCollisionDesc& desc);
	bool	evalCollisionMOBoverSOC (CMovePrimitive& other, CCollisionDesc& desc);
	bool	evalCollisionMOBoverMOC (CMovePrimitive& other, CCollisionDesc& desc);
	bool	evalCollisionSOCoverMOC (CMovePrimitive& other, CCollisionDesc& desc);
	bool	evalCollisionMOCoverMOC (CMovePrimitive& other, CCollisionDesc& desc);
	bool	evalCollisionSSoverMP (CMovePrimitive& other, CCollisionDesc& desc, uint numSeg, uint numPoint);
	bool	evalCollisionMSoverSP (CMovePrimitive& other, CCollisionDesc& desc, uint numSeg, uint numPoint);
	
private:

	// This position is the central bottom position for the box or for the cylinder
	NLMISC::CVector		_Position;

	// This is the height of the box or of the cylinder.
	float				_Height;

	// Flags
	uint32				_Flags;

	// Union for data space shared by primitive type (cylinder or box)
	union 
	{
		// Data for Boxes
		struct
		{
			// 2d position of the 4 points of the box at initial time.
			float PointPosX[4];
			float PointPosY[4];

			// The constants of each 4 planes of the box.
			float PlaneConst[4];

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
		} _OCData;
	};

	// Union for data space shared by primitive mode (movable or static)
	union 
	{
		// Data for statics
		struct
		{
		} _SData;

		// Data for movables
		struct
		{
			// The speed of the primitive
			float SpeedX;
			float SpeedY;
			float SpeedZ;

			// The current time for the primitive
			float Time;
		} _MData;
	};

	// Union for data space shared by primitive type and mode (SOB, MOB, SOC, MOC)
	union 
	{
		// Data for static boxes
		struct
		{
		} _SOBData;

		// Data for movable boxes
		struct
		{
			float OneOverSpeedPlaneConst[4];
		} _MOBData;

		// Data for static cylinders
		struct
		{
		} _SOCData;

		// Data for movable cylinders
		struct
		{			
		} _MOCData;
	};
};


} // NLPACS


#endif // NL_MOVE_PRIMITIVE_H

/* End of move_primitive.h */
