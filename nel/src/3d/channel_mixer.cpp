/** \file channel_mixer.cpp
 * class CChannelMixer
 *
 * $Id: channel_mixer.cpp,v 1.2 2001/02/12 14:18:40 corvazier Exp $
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
#include "nel/misc/debug.h"
#include "nel/misc/common.h"

using namespace NLMISC;
using namespace std;

namespace NL3D 
{

// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
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
		CChannel &entry=_Channels[iDInAnimationSet];

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

		// Erase all slot values for this channel
		for (uint slot=0; slot<NumAnimationSlot; slot++)
		{
			entry._Weights[slot]=1.f;

			// Empty slot
			entry._Tracks[slot]=NULL;

			// Slot in use ?
			if (_SlotArray[slot]._Animation)
			{
				// Track id in the animation
				uint iDTrack=_SlotArray[slot]._Animation->getIdTrackByName (channelName);
				if (iDTrack!=CAnimation::NotFound)
				{
					// Ok, get the track pointer for this animation for this channel
					entry._Tracks[slot]=&_SlotArray[slot]._Animation->getTrack (iDTrack);
				}
				else
				{
					// Set the default track
					entry._Tracks[slot]=entry._DefaultTracks;
				}
			}
		}
	}
}


// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
// ***************************************************************************
void CChannelMixer::eval ()
{
	// For purpose only, 'how to eval channels'
	// Final implementation should only eval preselected channels
	uint size=_Channels.size();
	for (uint channel=0; channel<size; channel++)
	{
		// First slot found
		bool bFirst=true;

		// The channel
		CChannel& entry=_Channels[channel];

		// Last blend factor
		float lastBlend;

		// Eval each slot
		for (uint slot=0; slot<NumAnimationSlot; slot++)
		{
			// Is this an active slot ?
			if (_SlotArray[slot]._Animation!=NULL)
			{
				// Current blend factor
				float blend=entry._Weights[slot]*_SlotArray[slot]._Weight;

				// Eval the track at this time
				entry._Tracks[slot]->eval (_SlotArray[slot]._Date);

				// First track to be eval ?
				if (bFirst)
				{
					// Copy the interpolated value
					entry._Value->affect (entry._Tracks[slot]->getValue());

					// First blend factor
					lastBlend=blend;

					// Not first anymore
					bFirst=false;
				}
				else
				{
					// Blend with this value and the previous sum
					entry._Value->blend (entry._Tracks[slot]->getValue(), lastBlend/(lastBlend+blend));

					// last blend update
					lastBlend+=blend;
				}

				// Touch the animated value and its owner to recompute them later.
				entry._Object->touch (entry._ValueId);
			}
		}
	}
}


} // NL3D
