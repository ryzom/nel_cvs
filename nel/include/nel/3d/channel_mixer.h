/** \file channel_mixer.h
 * class CChannelMixer
 *
 * $Id: channel_mixer.h,v 1.2 2001/02/12 14:20:24 corvazier Exp $
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

#ifndef NL_CHANNEL_MIXER_H
#define NL_CHANNEL_MIXER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/animation_set.h"
#include <map>
#include <vector>

namespace NL3D 
{

class CAnimation;
class IAnimatable;
class IAnimatedValue;
class ITrack;
class CSkeletonWeight;
class CAnimationSet;

// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
// ***************************************************************************

/**
 * A channel mixer. Animated value are registred in it. 
 * Each animated value create a channel in the mixer. Then, mixer animates
 * all those channels with 1 to CChannelMixer::NumAnimationSlot animations.
 *
 * Animation are referenced in an animation slot (CSlot).
 * 
 * Each slot have an IAnimation pointer, a weight for this animation
 * between [0.f ~ 1.f] and a date for this animation.
 *
 * Each CChannel have a weight on each animation slot between [0.f ~ 1.f].
 *
 * Blending is normalized internaly so, weight sum have not to be == 1.f.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CChannelMixer
{
public:

	enum 
	{ 
		/// Number of animation slot in the CChannelMixer
		NumAnimationSlot=8 
	};

	/// An animation slot.
	class CSlot
	{
		friend class CChannelMixer;
	private:
		// Animation pointer in the animation set.
		CAnimation*		_Animation;

		// Date to use in the animation
		CAnimationTime	_Date;

		// Weight to apply to this slot
		float			_Weight;
	};

	/**
	 * An animated channel of the CChannelMixer. This class is used internaly in the 
	 * CChannelMixer.
	 *
	 * \author Cyril 'Hulud' Corvazier
	 * \author Nevrax France
	 * \date 2001
	 */
	class CChannel
	{
		friend class CChannelMixer;
	private:

		/// Name of the channel in the channel mixer
		std::string			_ChannelName;

		/// A pointer on the IAnimatable object
		IAnimatable*		_Object;

		/// A pointer on the IAnimatedValue
		IAnimatedValue*		_Value;

		/// The id of the value in the IAnimatable object.
		uint32				_ValueId;

		/// The default track used when track are missing in the animation.
		ITrack*				_DefaultTracks;

		/** 
		  * A track pointer on each slot CAnimation. Can't be NULL. If no track found for this 
		  * channel, the pionter is _DefaultTracks.
		  */
		ITrack*				_Tracks[NumAnimationSlot];

		/// A weight array for to blend each slot.
		float				_Weights[NumAnimationSlot];

		/// Pointer on the next channel selected for the animations selected in the slots
		CChannel*			_Next;

	};

	/// Constructor. The default constructor resets the slots and the channels.
	CChannelMixer();

	/// \name Acces to the slots

	/** 
	  * Set slot animation.
	  *
	  * Calling this method will dirt the slot, ie, all channels will be visited
	  * to check if they are used by the new animation. If they are, they
	  * will be linked in the internal IChannel list.
	  * You must set an animationSet in the channel mixer before calling this.
	  * 
	  * \param slot is the slot number to change the animation.
	  * \param animation is the new animation index in the animationSet use by this slot.
	  * The slot keep this pointer until it is reseted or changed.
	  */
	void setSlotAnimation (uint slot, uint animation)
	{}

	/** 
	  * Set slot date.
	  * 
	  * \param slot is the slot number to change the date.
	  * \param date is the new date to use in the slot.
	  */
	void setSlotDate (uint slot, CAnimationTime date)
	{}

	/** 
	  * Set slot weight.
	  * 
	  * \param slot is the slot number to change the weight.
	  * \param weight is the new weight to use in the slot.
	  */
	void setSlotWeight (uint slot, float weigth)
	{}

	/** 
	  * Empty a slot.
	  * 
	  * \param slot is the slot number to empty.
	  */
	void emptySlot (uint slot)
	{}

	/// Reset the slot of the mixer. All slot will be empty.
	void resetSlots ()
	{}

	/// \name Channel access

	/** 
	  * Add a channel for a specific value of an IAnimatable object.
	  * 
	  * \param channelName is the name of the channel.
	  * \param animatable is a pointer on the IAnimatable object in which the value is stored. It will be kept
	  * by the CChannelMixer until it is removed from the channel.
	  * \param value is a pointer on the value the channel works with. It will be kept
	  * by the CChannelMixer until it is removed from the channel.
      * \param defaultValue is a track used by default if a track is not presents in the animation for this channel. 
	  * It will be kept by the CChannelMixer until it is removed from the channel.
	  * \param valueId is the value ID in the IAnimatable object.
	  */
	void addChannel (const std::string& channelName, IAnimatable* animatable, IAnimatedValue* value, ITrack* defaultValue, uint32 valueId);

	/**
	  * Apply a skelette weight template on a specific slot.
	  * This method apply the weight of each node contains in skelWeight to the channel's slot weight.
	  *
	  * \param skelWeight is a skeleton weight template.
	  */
	void applySkeletonWeight (uint slot, const CSkeletonWeight& skelWeight)
	{}

	/// Reset the channel list if the mixer. All channels are removed from the mixer.
	void resetChannels ()
	{}

	/**
	  * Set the animation set used by this channel mixer.
	  * The pointer is hold by the channel mixer until it changes.
	  */
	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	// ***************************************************************************
	void setAnimationSet (const CAnimationSet* animationSet)
	{
		// Set the animationSet Pointer
		_AnimationSet=animationSet;

		// Resize the channel array this the count of tracks
		_Channels.resize (_AnimationSet->getNumChannelId ());
	}

	/** 
	  * Launch evaluation of channels.
	  * 
	  * Only the channels that are animated by animations selected in the slots are evaluated.
	  * They are stored in a linked list managed by the channel array.
	  * Others are initialized the eval time with the default channel value.
	  */
	void eval ();

private:
	// The slot array
	CSlot							_SlotArray[NumAnimationSlot];

	// The animation set
	const CAnimationSet*			_AnimationSet;

	// The vector of IChannel infos.
	std::vector<CChannel>			_Channels;
};


} // NL3D


#endif // NL_CHANNEL_MIXER_H

/* End of channel_mixer.h */
