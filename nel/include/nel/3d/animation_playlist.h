/** \file animation_playlist.h
 * <File description>
 *
 * $Id: animation_playlist.h,v 1.2 2001/03/19 14:03:57 berenguier Exp $
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

#ifndef NL_ANIMATION_PLAYLIST_H
#define NL_ANIMATION_PLAYLIST_H

#include "nel/misc/types_nl.h"
#include "nel/3d/channel_mixer.h"


namespace NL3D 
{


/**
 * This class 
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimationPlaylist : public NLMISC::CRefCount
{
public:
	enum { empty=0xffffffff };

	/// Constructor
	CAnimationPlaylist();

	/**
	  * Empty the playlist. Each slot is set to its default value.
	  */
	void emptyPlayList ();

	/**
	  * Set the animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param animation is the animation number to use in this slot. To empty the slot, use CAnimationPlaylist::empty.
	  */
	void setAnimation (uint8 slot, uint animation);

	/**
	  * Get the animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \return the animation number in use in this slot. Return CAnimationPlaylist::empty if the slot is empty.
	  */
	uint getAnimation (uint8 slot) const;

	/**
	  * Set the skeleton weight animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param skeletonId is the skeleton weight number to use in this slot. To empty the slot, use CAnimationPlaylist::empty.
	  * \param inverted is false if the weights must be used as they are, true if the weights to used are 1.f - weight.
	  */
	void setSkeletonWeight (uint8 slot, uint skeletonId, bool inverted=false);

	/**
	  * Get the skeleton weight of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param inverted will receive the invert flag.
	  * \return the skeleton weight number in use in this slot. Return CAnimationPlaylist::empty if the slot is empty.
	  */
	uint getSkeletonWeight (uint8 slot, bool &inverted) const;

	/**
	  * Set animation time origin, ie, the time in the playlist for which slot time is the startTime of the slot animation.
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param timeOrigin time origin to use in the slot.
	  */
	void setTimeOrigin (uint8 slot, CAnimationTime timeOrigin);

	/**
	  * Get animation time origin, ie, the time in the playlist for which slot time is the startTime of the slot animation.
	  * Default value is 0.f.
	  *
	  * \param slot is the id to get the slot.
	  * \return time origin used in the slot.
	  */
	CAnimationTime getTimeOrigin (uint8 slot) const;

	/**
	  * Set animation speed factor.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param speedFactor is the factor to use in this slot. The animation will go speedFactor* faster.
	  */
	void setSpeedFactor (uint8 slot, float speedFactor);

	/**
	  * Get animation speed factor.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \return the speed factor used in the slot.
	  */
	float getSpeedFactor (uint8 slot) const;

	/**
	  * Set animation start weight. This is the weight for this animation use at the beginning of the animation slot.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param startWeight is the factor to use in this slot.
	  * \param time is the playlist time for which the start weight is set. Default is 0.f.
	  */
	void setStartWeight (uint8 slot, float startWeight, CAnimationTime time);

	/**
	  * Get animation start weight.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \param time will get the playlist time for which the start weight is set. Default is 0.f.
	  * \return the start weight used in the slot.
	  */
	float getStartWeight (uint8 slot, CAnimationTime& time) const;

	/**
	  * Set animation end weight. This is the weight for this animation use at the end of the animation slot.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param time is the playlist time for which the start weight is set. Default is 1.f.
	  * \param endWeight is the factor to use in this slot.
	  */
	void setEndWeight (uint8 slot, float endWeight, CAnimationTime time);

	/**
	  * Get animation end weight.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \param time will get the playlist time for which the start weight is set. Default is 0.f.
	  * \return the end weight used in the slot.
	  */
	float getEndWeight (uint8 slot, CAnimationTime& time) const;

	/**
	  * Set weight smoothness. 
	  * This is the smoothness of the weight interpolation. 
	  * Must be between 0.f and 1.f. 0.f is a sharp interpolation (linear), 1.f is a smooth interpolation (quadratic).
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param smoothness is the smoothness value.
	  */
	void setWeightSmoothness (uint8 slot, float smoothness);

	/**
	  * Get weight smoothness. 
	  * This is the smoothness of the weight interpolation. 
	  * Must be between 0.f and 1.f. 0.f is a sharp interpolation (linear), 1.f is a smooth interpolation (quadratic).
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \return the smoothness value.
	  */
	float getWeightSmoothness (uint8 slot) const;

	/**
	  * Setup a channel mixer.
	  *
	  * For each slot, it sets :
	  * * the animation id used by the slot.
	  * * the animation time according with the begin time of each slot, the speed factor of each slot and the current time passed in parameter.
	  * * the skeleton weight id.
	  * * the weight interpolated with the start and end values. The weight value are clamped before and after the animation.
	  */
	void setupMixer (CChannelMixer& mixer, CAnimationTime time) const;

	/**
	  * Static interpolation method of blend value
	  *
	  * \param startBlendTime is the time when the blend starts.
	  * \param endBlendTime is the time when the blend ends.
	  * \param time is the current time.
	  * \param startBlend is the blend value at start.
	  * \param endBlend is the blend value at end.
	  * \param smoothness is the smoothnes value. (Must be between 0 and 1)
	  */
	static float getWeightValue (float startWeightTime, float endWeightTime, float time, float startWeight, float endWeight, float smoothness);

private:

	// Animation in slot
	uint				_Animations[CChannelMixer::NumAnimationSlot];

	// Skeleton weight in slot
	uint				_SkeletonWeight[CChannelMixer::NumAnimationSlot];

	// Invert Skeleton weight in slot
	bool				_InvertWeight[CChannelMixer::NumAnimationSlot];

	// Time origin in slot
	CAnimationTime		_TimeOrigin[CChannelMixer::NumAnimationSlot];

	// Speed factor in slot
	float				_SpeedFactor[CChannelMixer::NumAnimationSlot];

	// Start weight
	float				_StartWeight[CChannelMixer::NumAnimationSlot];

	// Start weight time
	CAnimationTime		_StartWeightTime[CChannelMixer::NumAnimationSlot];

	// End weight
	float				_EndWeight[CChannelMixer::NumAnimationSlot];

	// End weight time
	CAnimationTime		_EndWeightTime[CChannelMixer::NumAnimationSlot];

	// Smoothness of the 
	float				_Smoothness[CChannelMixer::NumAnimationSlot];
};


} // NL3D


#endif // NL_ANIMATION_PLAYLIST_H

/* End of animation_playlist.h */
