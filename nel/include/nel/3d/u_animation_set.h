/** \file u_animation_set.h
 * <File description>
 *
 * $Id: u_animation_set.h,v 1.5 2001/08/01 09:38:25 berenguier Exp $
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
#include "nel/3d/animation_time.h"


namespace NL3D
{

class UAnimation;

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

	/// \name Set build.
	// @{
	/**
	  *  Add an animation in the animation set. After adding all your animations, call build().
	  *  This method use CPath to search the animation file.
	  *
	  * \param fileName is the animation filename
	  * \param animName is the name of the animation in the animation set.
	  * \return the id of the new animation or NotFound if the file is not found.
	  */
	virtual	uint addAnimation (const char* fileName, const char* animName) =0;

	/**
	  *  Build the animation set. Call build after adding all your animations.
	  */
	virtual	void build () =0;

	/**
	  *  Add a skeleton weight in the animation set.
	  *  This method use CPath to search the skeleton file.
	  *
	  * \param fileName is the skeleton weight filename
	  * \param skelName is the name of the skeleton weight in the animation set.
	  * \return the id of the new skeleton or NotFound if the file is not found.
	  */
	virtual	uint addSkeletonWeight (const char* fileName, const char* skelName) =0;

	// @}

	/// \name Animations mgt.
	// @{
	/**
	  * Get animations count.
	  */
	virtual	uint getNumAnimation () const =0;

	/**
	  * Get a animation ID by name. If no animation is found, method returns NotFound.
	  */
	virtual	uint getAnimationIdByName (const std::string& name) const  =0;

	/**
	  * Get animation name.
	  */
	virtual	const std::string& getAnimationName (uint animationId) const =0;

	/**
	  * Get a writable animation pointer.
	  *
	  * \return the end time.
	  */
	virtual UAnimation *getAnimation (uint animationId) =0;

	// @}


	/// \name SkeletonWeight mgt.
	// @{
	/**
	  * Get skeleton weight count.
	  */
	virtual	uint getNumSkeletonWeight () const =0;

	/**
	  * Get a SkeletonWeight ID by name. If no SkeletonWeight is found, method returns NotFound.
	  */
	virtual	uint getSkeletonWeightIdByName (const std::string& name) const =0;

	/**
	  * Get skeleton template name.
	  */
	virtual	const std::string& getSkeletonWeightName (uint skeletonId) const =0;
	// @}


	/// \name Channel mgt.
	// @{

	/**
	  * Get a channel ID with its name. If no channel is found, method returns NotFound.
	  */
	virtual	uint getChannelIdByName (const std::string& name) const =0;

	// @}


};


} // NL3D


#endif // NL_U_ANIMATION_SET_H

/* End of u_animation_set.h */
