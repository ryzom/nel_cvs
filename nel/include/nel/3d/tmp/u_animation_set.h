/** \file u_animation_set.h
 * <File description>
 *
 * $Id: u_animation_set.h,v 1.1 2001/03/28 15:18:54 berenguier Exp $
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

#ifndef NL_U_ANIMATION_SET_H
#define NL_U_ANIMATION_SET_H

#include "nel/misc/types_nl.h"


namespace NL3D
{


// ***************************************************************************
/**
 * An animation Set is a set of animation, loaded from file. It contains a set of Skeleton TempalteWeight too.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UAnimationSet
{
protected:

	/// Constructor
	UAnimationSet() {}
	/// Constructor
	virtual	~UAnimationSet() {}

public:

	enum { NotFound=0xffffffff };

public:


	/// \name Animations mgt.
	// @{
	/**
	  * Get animations count.
	  */
	uint getNumAnimation () const =0;

	/**
	  * Get a animation ID by name. If no animation is found, method returns NotFound.
	  */
	uint getAnimationIdByName (const std::string& name) const  =0;

	/**
	  * Get animation name.
	  */
	const std::string& getAnimationName (uint animationId) const =0;

	/**
	  * Get begin time of an animation
	  *
	  * \return the begin time.
	  */
	CAnimationTime getAnimationBeginTime (uint animationId) const;

	/**
	  * Get end time of an animation
	  *
	  * \return the end time.
	  */
	CAnimationTime getAnimationEndTime (uint animationId) const;

	// @}


	/// \name SkeletonWeight mgt.
	// @{
	/**
	  * Get skeleton weight count.
	  */
	uint getNumSkeletonWeight () const =0;

	/**
	  * Get a SkeletonWeight ID by name. If no SkeletonWeight is found, method returns NotFound.
	  */
	uint getSkeletonWeightIdByName (const std::string& name) const =0;

	/**
	  * Get skeleton template name.
	  */
	const std::string& getSkeletonWeightName (uint skeletonId) const =0;
	// @}



};


} // NL3D


#endif // NL_U_ANIMATION_SET_H

/* End of u_animation_set.h */
