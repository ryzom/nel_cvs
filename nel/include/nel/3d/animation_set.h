/** \file animation_set.h
 * class CAnimationSet
 *
 * $Id: animation_set.h,v 1.5 2001/03/08 13:29:07 corvazier Exp $
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

#ifndef NL_ANIMATION_SET_H
#define NL_ANIMATION_SET_H

#include "nel/misc/types_nl.h"
#include "nel/3d/animation.h"
#include <map>
#include <string>
#include <vector>


namespace NL3D {


/**
 * An CAnimationSet is set of CAnimation. It manages global channel ID for a set of
 * animations with many tracks. An Id is associed with each track name.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimationSet
{
public:

	enum { NotFound=0xffffffff };

	/**
	  * Get channel ID count. This number is the count of different track name in the animation set.
	  */
	uint getNumChannelId () const;

	/**
	  * Get a channel ID with its name. If no channel is found, method returns NotFound.
	  */
	uint getChannelIdByName (const std::string& name) const;

	/**
	  * Get a animation ID by name. If no channel is found, method returns NotFound.
	  */
	uint getAnimationIdByName (const std::string& name) const;

	/**
	  * Get a read only animation pointer.
	  */
	const CAnimation* getAnimation (uint animationId) const
	{
		return &_Animation[animationId];
	}

	/**
	  * Get a writable animation pointer.
	  */
	CAnimation* getAnimation (uint animationId)
	{
		return &_Animation[animationId];
	}

	/**
	  * Add an animation empty to the set.
	  *
	  * \return the id of the new animation.
	  */
	uint addAnimation (const std::string& name);

	/**
	  * Final build of the animation set.
	  *
	  * First, for each animation you want to add to the set, you must:
	  * 1) add an empty animation using the method addAnimation().
	  * 2) get a writable animation pointer on it with getAnimation().
	  * 3) build this animation.
	  *
	  * When all animations are built, call this method to finlize the set.
	  */
	void build ();

private:
	std::vector <CAnimation>		_Animation;
	std::map <std::string, uint32>	_ChannelIdByName;
	std::map <std::string, uint32>	_AnimationIdByName;
};


} // NL3D


#endif // NL_ANIMATION_SET_H

/* End of animation_set.h */
