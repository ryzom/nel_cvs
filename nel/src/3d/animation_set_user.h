/** \file animation_set_user.h
 * <File description>
 *
 * $Id: animation_set_user.h,v 1.1 2001/06/15 16:24:42 corvazier Exp $
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

#ifndef NL_ANIMATION_SET_USER_H
#define NL_ANIMATION_SET_USER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "3d/animation_set.h"
#include "nel/3d/u_animation_set.h"
#include "nel/misc/smart_ptr.h"


namespace NL3D 
{


class	CPlayListManagerUser;


// ***************************************************************************
/**
 * UAnimationSet implementation
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CAnimationSetUser : public UAnimationSet
{
private:
	NLMISC::CSmartPtr<CAnimationSet>		_AnimationSet;
	friend class CPlayListManagerUser;


public:

	/// Constructor
	CAnimationSetUser(NLMISC::IStream	&f)
	{
		nlassert((uint)UAnimationSet::NotFound == (uint)CAnimationSet::NotFound );

		// create a smartptred animation set.
		_AnimationSet= new CAnimationSet;

		_AnimationSet->serial(f);
	}



	/// \name Animations mgt.
	// @{
	/**
	  * Get animations count.
	  */
	virtual	uint getNumAnimation () const 
	{
		return _AnimationSet->getNumAnimation();
	}

	/**
	  * Get a animation ID by name. If no animation is found, method returns NotFound.
	  */
	virtual	uint getAnimationIdByName (const std::string& name) const  
	{
		return _AnimationSet->getAnimationIdByName(name);
	}

	/**
	  * Get animation name.
	  */
	virtual	const std::string& getAnimationName (uint animationId) const 
	{
		if(animationId>=getNumAnimation())
			nlerror("getAnimation*(): bad animation Id");
		return _AnimationSet->getAnimationName(animationId);
	}

	/**
	  * Get begin time of an animation
	  *
	  * \return the begin time.
	  */
	virtual	CAnimationTime getAnimationBeginTime (uint animationId) const
	{
		if(animationId>=getNumAnimation())
			nlerror("getAnimation*(): bad animation Id");
		return _AnimationSet->getAnimation(animationId)->getBeginTime();
	}

	/**
	  * Get end time of an animation
	  *
	  * \return the end time.
	  */
	virtual	CAnimationTime getAnimationEndTime (uint animationId) const
	{
		if(animationId>=getNumAnimation())
			nlerror("getAnimation*(): bad animation Id");
		return _AnimationSet->getAnimation(animationId)->getEndTime();
	}

	// @}


	/// \name SkeletonWeight mgt.
	// @{
	/**
	  * Get skeleton weight count.
	  */
	virtual	uint getNumSkeletonWeight () const 
	{
		return _AnimationSet->getNumSkeletonWeight();
	}

	/**
	  * Get a SkeletonWeight ID by name. If no SkeletonWeight is found, method returns NotFound.
	  */
	virtual	uint getSkeletonWeightIdByName (const std::string& name) const 
	{
		return _AnimationSet->getSkeletonWeightIdByName(name);
	}

	/**
	  * Get skeleton template name.
	  */
	virtual	const std::string& getSkeletonWeightName (uint skeletonId) const 
	{
		if(skeletonId>=getNumSkeletonWeight())
			nlerror("getSkeletonWeight*(): bad SkeletonWeight Id");
		return _AnimationSet->getSkeletonWeightName(skeletonId);
	}

	// @}


};


} // NL3D


#endif // NL_ANIMATION_SET_USER_H

/* End of animation_set_user.h */
