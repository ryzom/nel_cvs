/** \file channel_mixer.cpp
 * class CChannelMixer
 *
 * $Id: channel_mixer.cpp,v 1.18 2002/02/28 12:59:49 besson Exp $
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

#include "std3d.h"

#include "3d/channel_mixer.h"
#include "3d/track.h"
#include "3d/animatable.h"
#include "3d/skeleton_weight.h"
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
	_FirstChannelGlobal=NULL;
	_FirstChannelDetail=NULL;

	// No animation set
	_AnimationSet=NULL;

	// Mixer no dirty
	_Dirt=false;

	// never evaluated.
	_LastEvalDetailDate= -1;
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

void CChannelMixer::eval (bool detail, uint64 evalDetailDate)
{
	// Setup an array of animation that are not empty and stay
	uint numActive=0;
	uint activeSlot[NumAnimationSlot];

	if(detail && (sint64)evalDetailDate== _LastEvalDetailDate)
		return;

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
	CChannel* pChannel;
	if(detail)
	{
		pChannel=_FirstChannelDetail;
		// eval the animation only one time per scene traversal.
		_LastEvalDetailDate= evalDetailDate;
	}
	else
		pChannel=_FirstChannelGlobal;


	while (pChannel)
	{
		// if the channel is enabled, eval all active slot.
		if(pChannel->_Enabled)
		{
			// For Quat animated value only.
			CQuat	firstQuat;

			// First slot found
			bool bFirst=true;

			// Last blend factor
			float lastBlend;

			// Eval each slot
			for (uint a=0; a<numActive; a++)
			{
				// Slot number
				uint slot=activeSlot[a];

				// Is this an active slot ?
				if (_SlotArray[slot]._Animation!=NULL)
				{
					// Current blend factor
					float blend=pChannel->_Weights[slot]*_SlotArray[slot]._Weight;

					if(blend!=0.0f)
					{
						// Eval the track at this time
						((ITrack*)pChannel->_Tracks[slot])->eval (_SlotArray[slot]._Time);

						// First track to be eval ?
						if (bFirst)
						{
							// If channel is a Quaternion animated Value, must store the first Quat.
							if (pChannel->_IsQuat)
							{
								CAnimatedValueBlendable<NLMISC::CQuat>	*pQuatValue=(CAnimatedValueBlendable<NLMISC::CQuat>*)&pChannel->_Tracks[slot]->getValue();
								firstQuat=pQuatValue->Value;
							}

							// Copy the interpolated value
							pChannel->_Value->affect (pChannel->_Tracks[slot]->getValue());

							// First blend factor
							lastBlend=blend;

							// Not first anymore
							bFirst=false;
						}
						else
						{
							// If channel is a Quaternion animated Value, must makeClosest the ith result of the track, from firstQuat.
							if (pChannel->_IsQuat)
							{
								CAnimatedValueBlendable<NLMISC::CQuat>	*pQuatValue=(CAnimatedValueBlendable<NLMISC::CQuat>*)&pChannel->_Tracks[slot]->getValue();
								pQuatValue->Value.makeClosest (firstQuat);
							}

							// Blend with this value and the previous sum
							pChannel->_Value->blend (pChannel->_Tracks[slot]->getValue(), lastBlend/(lastBlend+blend));

							// last blend update
							lastBlend+=blend;
						}
					}

					// NB: if all weights are 0, the AnimatedValue is not modified...

					// Touch the animated value and its owner to recompute them later.
					pChannel->_Object->touch (pChannel->_ValueId, pChannel->_OwnerValueId);
				}
			}
		}

		// Next channel
		pChannel=pChannel->_Next;
	}
}

// ***************************************************************************

void CChannelMixer::addChannel (const string& channelName, IAnimatable* animatable, IAnimatedValue* value, ITrack* defaultValue, uint32 valueId, uint32 ownerValueId, bool detail)
{
	// Check the animationSet has been set
	nlassert (_AnimationSet);

	// Check args
	nlassert (animatable);
	nlassert (value);
	nlassert (defaultValue);

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

		// Is this a CQuat animated value???
		entry._IsQuat= (typeid (*(entry._Value))==typeid (CAnimatedValueBlendable<NLMISC::CQuat>))!=0;


		// Set the default track pointer
		entry._DefaultTracks=defaultValue;

		// Set the value ID in the object
		entry._ValueId=valueId;

		// Set the First value ID in the object
		entry._OwnerValueId=ownerValueId;

		// in what mode is the channel?
		entry._Detail= detail;

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

		// Affect the default value in the animated value
		entry._Value->affect (entry._DefaultTracks->getValue());

		// Touch the animated value and its owner to recompute them later.
		entry._Object->touch (entry._ValueId, entry._OwnerValueId);
	}
}

// ***************************************************************************

void CChannelMixer::resetChannels ()
{
	_Channels.clear();
	dirtAll ();
}


// ***************************************************************************
void CChannelMixer::enableChannel (uint channelId, bool enable)
{
	std::map<uint, CChannel>::iterator	it= _Channels.find(channelId);
	if(it!=_Channels.end())
	{
		it->second._Enabled= enable;
	}
}


// ***************************************************************************
bool CChannelMixer::isChannelEnabled (uint channelId) const
{
	std::map<uint, CChannel>::const_iterator	it= _Channels.find(channelId);
	if(it!=_Channels.end())
	{
		return it->second._Enabled;
	}
	else
		return false;
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
				std::map<uint, CChannel>::iterator ite=_Channels.find(channelId);
				if (ite!=_Channels.end())
					ite->second._Weights[slot]=invert?1.f-weight:weight;
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
	CChannel **lastPointerGlobal=&_FirstChannelGlobal;
	CChannel **lastPointerDetail=&_FirstChannelDetail;


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

				// Still in use?
				if (!add)
				{
					// Set it's value to default and touch it's object
					channel._Value->affect (channel._DefaultTracks->getValue());
					channel._Object->touch (channel._ValueId, channel._OwnerValueId);
				}
			}
		}

		// Do i have to add the channel to the list
		if (add)
		{
			// It is in the list
			channel._InTheList=true;

			if(channel._Detail)
			{
				// Set the last pointer value
				*lastPointerDetail=&channel;
				// Change last pointer
				lastPointerDetail=&channel._Next;
			}
			else
			{
				// Set the last pointer value
				*lastPointerGlobal=&channel;
				// Change last pointer
				lastPointerGlobal=&channel._Next;
			}

		}
		else
		{
			// It is not in the list
			channel._InTheList=false;
		}
	}

	// End of the list
	*lastPointerGlobal=NULL;
	*lastPointerDetail=NULL;
}

// ***************************************************************************

} // NL3D
