/** \file channel_mixer.cpp
 * class CChannelMixer
 *
 * $Id: channel_mixer.cpp,v 1.6 2001/03/19 09:33:15 berenguier Exp $
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

#include "nel/3d/channel_mixer.h"
#include "nel/3d/track.h"
#include "nel/3d/animatable.h"
#include "nel/3d/skeleton_weight.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"

using namespace NLMISC;
using namespace std;

namespace NL3D 
{

// ***************************************************************************

CChannelMixer::CChannelMixer()
{
	// No channel in the list
	_FirstChannel=NULL;

	// No animation set
	_AnimationSet=NULL;

	// Mixer no dirty
	_Dirt=false;
}

// ***************************************************************************

void CChannelMixer::setAnimationSet (const CAnimationSet* animationSet)
{
	// Set the animationSet Pointer
	_AnimationSet=animationSet;

	// clear the channels.
	resetChannels();
}

// ***************************************************************************

const CAnimationSet* CChannelMixer::getAnimationSet () const
{
	// Return the animationSet Pointer
	return _AnimationSet;
}

// ***************************************************************************

void CChannelMixer::eval ()
{
	// Setup an array of animation that are not empty and stay
	uint numActive=0;
	uint activeSlot[NumAnimationSlot];

	if(_Dirt)
	{
		refreshList();
		cleanAll();
	}

	// Setup it up
	for (uint s=0; s<NumAnimationSlot; s++)
	{
		// Dirt and not empty ? (add)
		if (!_SlotArray[s].isEmpty())
			// Add a dirt slot
			activeSlot[numActive++]=s;
	}

	// For each selected channel
	CChannel* pChannel=_FirstChannel;

	while (pChannel)
	{
		// First slot found
		bool bFirst=true;

		// Last blend factor
		float lastBlend;

		// Eval each slot
		for (uint a=0; a<numActive; a++)
		{
			// Slot number
			uint slot=activeSlot[s];

			// Is this an active slot ?
			if (_SlotArray[slot]._Animation!=NULL)
			{
				// Current blend factor
				float blend=pChannel->_Weights[slot]*_SlotArray[slot]._Weight;

				// Eval the track at this time
				((ITrack*)pChannel->_Tracks[slot])->eval (_SlotArray[slot]._Time);

				// First track to be eval ?
				if (bFirst)
				{
					// Copy the interpolated value
					pChannel->_Value->affect (pChannel->_Tracks[slot]->getValue());

					// First blend factor
					lastBlend=blend;

					// Not first anymore
					bFirst=false;
				}
				else
				{
					// Blend with this value and the previous sum
					pChannel->_Value->blend (pChannel->_Tracks[slot]->getValue(), lastBlend/(lastBlend+blend));

					// last blend update
					lastBlend+=blend;
				}

				// Touch the animated value and its owner to recompute them later.
				pChannel->_Object->touch (pChannel->_ValueId);
			}
		}
	}
}

// ***************************************************************************

void CChannelMixer::addChannel (const string& channelName, IAnimatable* animatable, IAnimatedValue* value, ITrack* defaultTrack, uint32 valueId)
{
	// Check the animationSet has been set
	nlassert (_AnimationSet);

	// Check args
	nlassert (animatable);
	nlassert (value);
	nlassert (defaultTrack);

	// Get the channel Id having the same name than the tracks in this animation set.
	uint iDInAnimationSet=_AnimationSet->getChannelIdByName (channelName);

	// Tracks exist in this animation set?
	if (iDInAnimationSet!=CAnimationSet::NotFound)
	{
		// The channel entry
		CChannel	entry;

		// Set the channel name
		entry._ChannelName=channelName;

		// Set the object pointer
		entry._Object=animatable;

		// Set the pointer on the value in the object
		entry._Value=value;

		// Set the default track pointer
		entry._DefaultTracks=defaultTrack;

		// Set the value ID in the object
		entry._ValueId=valueId;

		// All weights default to 1. All Tracks default to defaultTrack.
		for(sint s=0;s<NumAnimationSlot;s++)
		{
			entry._Weights[s]= 1.0f;
			entry._Tracks[s]= entry._DefaultTracks;
		}

		// add (if not already done) the entry in the map.
		_Channels[iDInAnimationSet]= entry;

		// Dirt all the slots
		dirtAll ();
	}
}

// ***************************************************************************

void CChannelMixer::resetChannels ()
{
	_Channels.clear();
}

// ***************************************************************************

void CChannelMixer::setSlotAnimation (uint slot, uint animation)
{
	// Check alot arg
	nlassert (slot<NumAnimationSlot);

	// Check an animationSet as been set.
	nlassert (_AnimationSet);

	// Find the animation pointer for this animation
	const CAnimation* pAnimation=_AnimationSet->getAnimation (animation);

	// Does this animation change ?
	if (_SlotArray[slot]._Animation!=pAnimation)
	{
		// Change it
		_SlotArray[slot]._Animation=pAnimation;

		// Dirt it
		_SlotArray[slot]._Dirt=true;

		// Dirt the mixer
		_Dirt=true;
	}
}

// ***************************************************************************

void CChannelMixer::emptySlot (uint slot)
{
	// Check alot arg
	nlassert (slot<NumAnimationSlot);

	// Does this animation already empty ?
	if (!_SlotArray[slot].isEmpty ())
	{
		// Change it
		_SlotArray[slot].empty ();

		// Dirt it
		_SlotArray[slot]._Dirt=true;

		// Dirt the mixer
		_Dirt=true;
	}
}

// ***************************************************************************

void CChannelMixer::resetSlots ()
{
	// Empty all slots
	for (uint s=0; s<NumAnimationSlot; s++)
		// Empty it
		emptySlot (s);
}

// ***************************************************************************

void CChannelMixer::applySkeletonWeight (uint slot, uint skeleton, bool invert)
{
	// Check alot arg
	nlassert (slot<NumAnimationSlot);

	// Check the animationSet has been set
	nlassert (_AnimationSet);

	// Get the skeleton weight
	const CSkeletonWeight *pSkeleton=_AnimationSet->getSkeletonWeight (skeleton);

	// Something to change ?
	if ((pSkeleton!=_SlotArray[slot]._SkeletonWeight)||(invert!=_SlotArray[slot]._InvertedSkeletonWeight))
	{
		// Set the current skeleton
		_SlotArray[slot]._SkeletonWeight=pSkeleton;
		_SlotArray[slot]._InvertedSkeletonWeight=invert;

		// Get number of node in the skeleton weight
		uint sizeSkel=pSkeleton->getNumNode ();

		// For each entry of the skeleton weight
		for (uint n=0; n<sizeSkel; n++)
		{
			// Get the name of the channel for this node
			const string& channelName=pSkeleton->getNodeName (n);

			// Get the channel Id having the same name than the tracks in this animation set.
			uint channelId=_AnimationSet->getChannelIdByName (channelName);

			// Tracks exist in this animation set?
			if (channelId!=CAnimationSet::NotFound)
			{
				// Get the weight of the channel for this node
				float weight=pSkeleton->getNodeWeight (n);

				// Set the weight of this channel for this slot (only if channel setuped!!)
				if(_Channels.find(channelId)!=_Channels.end())
					_Channels[channelId]._Weights[slot]=invert?1.f-weight:weight;
			}
		}
	}
}

// ***************************************************************************

void CChannelMixer::resetSkeletonWeight (uint slot)
{
	// Check alot arg
	nlassert (slot<NumAnimationSlot);

	// Something to change ?
	if (_SlotArray[slot]._SkeletonWeight!=NULL)
	{
		// Set skeleton
		_SlotArray[slot]._SkeletonWeight=NULL;
		_SlotArray[slot]._InvertedSkeletonWeight=false;

		// For each channels
		map<uint, CChannel>::iterator		itChannel;
		for(itChannel= _Channels.begin(); itChannel!=_Channels.end();itChannel++)
		{
			// Reset
			(*itChannel).second._Weights[slot]=1.f;
		}
	}
}

// ***************************************************************************

void CChannelMixer::cleanAll ()
{
	// For each slot
	for (uint s=0; s<NumAnimationSlot; s++)
	{
		// Clean it
		_SlotArray[s]._Dirt=false;
	}

	// Clean the mixer
	_Dirt=false;
}

// ***************************************************************************

void CChannelMixer::dirtAll ()
{
	// For each slot
	for (uint s=0; s<NumAnimationSlot; s++)
	{
		// Dirt
		if (!_SlotArray[s].isEmpty())
		{
			// Dirt it
			_SlotArray[s]._Dirt=true;

			// Dirt the mixer
			_Dirt=true;
		}
	}
}

// ***************************************************************************

void CChannelMixer::refreshList ()
{
	// Setup an array of animation to add
	uint numAdd=0;
	uint addSlot[NumAnimationSlot];

	// Setup an array of animation that are not empty and stay
	uint numStay=0;
	uint staySlot[NumAnimationSlot];

	// Setup it up
	uint s;
	for (s=0; s<NumAnimationSlot; s++)
	{
		// Dirt and not empty ? (add)
		if ((_SlotArray[s]._Dirt)&&(!_SlotArray[s].isEmpty()))
			// Add a dirt slot
			addSlot[numAdd++]=s;

		// Not empty and not dirt ? (stay)
		if ((!_SlotArray[s]._Dirt)&&(!_SlotArray[s].isEmpty()))
			// Add a dirt slot
			staySlot[numStay++]=s;
	}

	// Last channel pointer
	CChannel **lastPointer=&_FirstChannel;

	// Now scan each channel
	map<uint, CChannel>::iterator		itChannel;
	for(itChannel= _Channels.begin(); itChannel!=_Channels.end();itChannel++)
	{
		CChannel	&channel= (*itChannel).second;

		// Add this channel to the list if true
		bool add=false;

		// For each slot to add
		for (s=0; s<numAdd; s++)
		{
			// Find the index of the channel track in the animation set
			uint iDTrack=_SlotArray[addSlot[s]]._Animation->getIdTrackByName (channel._ChannelName);

			// If this track exist
			if (iDTrack!=CAnimation::NotFound)
			{
				// Set the track
				channel._Tracks[addSlot[s]]=_SlotArray[addSlot[s]]._Animation->getTrack (iDTrack);

				// Add this channel to the list
				add=true;
			}
			else
			{
				// Set the default track
				channel._Tracks[addSlot[s]]=channel._DefaultTracks;
			}
		}

		// Add this channel to the list ?
		if (!add)
		{
			// Was it in the list ?
			if (channel._InTheList)
			{
				// Check if this channel is still in use

				// For each slot in the stay list
				for (s=0; s<numStay; s++)
				{
					// Use anything interesting ?
					if (channel._Tracks[staySlot[s]]!=channel._DefaultTracks)
					{
						// Ok, add it to the list
						add=true;

						// Stop
						break;
					}
				}
			}
		}

		// Do i have to add the channel to the list
		if (add)
		{
			// It is in the list
			channel._InTheList=true;

			// Set the last pointer value
			*lastPointer=&channel;

			// Change last pointer
			lastPointer=&channel._Next;
		}
		else
		{
			// It is not in the list
			channel._InTheList=false;
		}
	}

	// End of the list
	*lastPointer=NULL;
}

// ***************************************************************************

} // NL3D
