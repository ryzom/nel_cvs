/** \file u_move_primitive.h
 * Description of movables primitives.
 *
 * $Id: u_move_primitive.h,v 1.1 2001/05/31 14:21:39 corvazier Exp $
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

#ifndef NL_U_MOVE_PRIMITIVE_H
#define NL_U_MOVE_PRIMITIVE_H

#include "nel/misc/types_nl.h"

namespace NLMISC
{
	class CVectorD;
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
class UMovePrimitive
{
public:

	/// Type of the collision mask.
	typedef uint32 TCollisionMask;

	/// Primitive mode
	enum TType
	{
		/**
		  * This is a static 2d oriented bounding box. It can be oriented only on the Z axis.
		  * It has a height. Collision can be performed only no its sides but not on its top and bottom
		  * planes. It doesn't mode. Default value.
		  */
		_2DOrientedBox=0x0,

		/** 
		  * This is a movable 2d oriented cylinder. It can be oriented only on the Z axis.
		  * It has a height. Collision can be performed only no its sides but not on its top and bottom
		  * planes. It can move only with 3d translations.
          */
		_2DOrientedCylinder=0x1,
	};


	/// Reaction mode
	enum TReaction
	{
		/**
		  * No reaction. For static objects or not influanced objects. Default value.
		  */
		DoNothing=0x0,

		/**
		  * This object slids over surfaces.
		  */
		Slide=0x10,

		/**
		  * This object reflects over surfaces.
		  */
		Reflexion=0x20,

		/**
		  * This object stops over surfaces.
		  */
		Stop=0x40,
	};

	/**
	  * User data.
	  */
	void			*UserPointer;

	/**
	  * Set the primitive type.
	  *
	  * \param type is the new primitive type.
	  */
	virtual void	setPrimitiveType (TType type) =0;

	/**
	  * Set the reaction type.
	  *
	  * \param type is the new reaction type.
	  */
	virtual void	setReactionType (TReaction type) =0;

	/**
	  * Set the collision mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new collision mask.
	  */
	virtual void	setCollisionMask (TCollisionMask mask) =0;

	/**
	  * Set the obstacle flag.
	  *
	  * \param obstacle is true if this primitive is an obstacle, else false.
	  */
	virtual void	setObstacle (bool obstacle) =0;

	/**
	  * Set the position of the move primitive. For movable primitives, this is
	  * the position for time = 0.
	  *
	  * \param pos is the new position of the primitive.
	  */
	virtual void	setPosition (const NLMISC::CVectorD& pos) =0;

	/**
	  * Get the position of the move primitive. For movable primitives, this is
	  * the position for time = 0.
	  *
	  * \return the new position of the primitive.
	  */
	virtual const NLMISC::CVectorD&	getPosition ()  const=0;

	/**
	  * Set the new orientation of the move primitive. Only for the box primitives.
	  *
	  * \param rot is the new OZ rotation in radian.
	  */
	virtual void	setOrientation (double rot) =0;

	/**
	  * Set the attenuation of collision for this object. Default value is 1. Should be between 0~1.
	  * 0, all the enrgy is attenuated by the collision. 1, all the energy stay in the object.
	  * Used only with the flag Reflexion.
	  *
	  * \param attenuation is the new attenuation for the primitive.
	  */
	virtual void	setAbsorbtion (float attenuation) =0;

	/**
	  * Set the box size. Only for boxes.
	  *
	  * \param width is the new width size of the box. It the size of the sides aligned on OX.
	  * \param depth is the new depth size of the box. It the size of the sides aligned on OY.
	  */
	virtual void	setSize (float width, float depth) =0;

	/**
	  * Set the height. For boxes or cylinder.
	  *
	  * \param height is the new height size of the box. It the size of the sides aligned on OZ.
	  */
	virtual void	setHeight (float height) =0;

	/**
	  * Set the cylinder size. Only for cylinder.
	  *
	  * \param radius is the new radius size of the cylinder.
	  */
	virtual void	setRadius (float radius) =0;

	/**
	  * Set the speed vector for this primitive. Only for movable primitives.
	  *
	  * \param speed is the new speed vector.
	  */
	virtual void	setSpeed (const NLMISC::CVectorD& speed) =0;

	/**
	  * Get the speed vector for this primitive.
	  *
	  * \Return the new speed vector.
	  */
	virtual const NLMISC::CVectorD&	getSpeed () const =0;
};


} // NLPACS


#endif // NL_U_MOVE_PRIMITIVE_H

/* End of u_move_primitive.h */
