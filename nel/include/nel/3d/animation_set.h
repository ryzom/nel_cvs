/** \file animation_set.h
 * class CAnimationSet
 *
 * $Id: animation_set.h,v 1.2 2001/02/12 14:20:24 corvazier Exp $
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

	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/**
	  * Get channel ID count. This number is the count of different track name in the animation set.
	  */
	uint getNumChannelId () const
	{
		return _ChannelIdByName.size ();
	}

	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/**
	  * Get a channel ID with its name.
	  */
	uint getChannelIdByName (const std::string& name) const
	{
		// Look for an id with this name
		std::map <std::string, uint>::const_iterator ite=_ChannelIdByName.find (name);
		if (ite!=_ChannelIdByName.end ())
			return ite->second;
		else
			return NotFound;
	}

	/**
	  * Get an animation pointer
	  */
	CAnimation* getChannelIdByName (uint animationId)
	{
		// Look for an id with this name
		return &_Animation[animationId];
	}


private:
	std::vector <CAnimation>		_Animation;
	std::map <std::string, uint>	_ChannelIdByName;
};


} // NL3D


#endif // NL_ANIMATION_SET_H

/* End of animation_set.h */
