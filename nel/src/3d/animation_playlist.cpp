/** \file animation_playlist.cpp
 * <File description>
 *
 * $Id: animation_playlist.cpp,v 1.1 2001/03/16 16:29:28 corvazier Exp $
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

#include "nel/3d/animation_playlist.h"


namespace NL3D 
{

// ***************************************************************************

CAnimationPlaylist::CAnimationPlaylist()
{
	// Empty the playlist
	emptyPlayList ();
}

// ***************************************************************************

void CAnimationPlaylist::emptyPlayList ()
{
	// Empty each slot
	for (uint i=0; i<CChannelMixer::NumAnimationSlot; i++)
	{
		_Animations[i]=empty;
		_SkeletonWeight[i]=empty;
		_InvertWeight[i]=false;
		_TimeOrigin[i]=0.f;
		_SpeedFactor[i]=1.f;
		_StartWeight[i]=1.f;
		_EndWeight[i]=1.f;
	}
}

// ***************************************************************************

void CAnimationPlaylist::setAnimation (uint8 slot, uint animation)
{
	_Animations[slot]=animation;
}

// ***************************************************************************

uint CAnimationPlaylist::getAnimation (uint8 slot) const
{
	return _Animations[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setSkeletonWeight (uint8 slot, uint skeletonId, bool inverted)
{
	_SkeletonWeight[slot]=skeletonId;
	_InvertWeight[slot]=inverted;
}

// ***************************************************************************

uint CAnimationPlaylist::getSkeletonWeight (uint8 slot, bool &inverted) const
{
	inverted=_InvertWeight[slot];
	return _SkeletonWeight[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setTimeOrigin (uint8 slot, CAnimationTime timeOrigin)
{
	_TimeOrigin[slot]=timeOrigin;
}

// ***************************************************************************

CAnimationTime CAnimationPlaylist::getTimeOrigin (uint8 slot) const
{
	return _TimeOrigin[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setSpeedFactor (uint8 slot, float speedFactor)
{
	_SpeedFactor[slot]=speedFactor;
}

// ***************************************************************************

float CAnimationPlaylist::getSpeedFactor (uint8 slot) const
{
	return _SpeedFactor[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setStartWeight (uint8 slot, float startWeight, CAnimationTime time)
{
	_StartWeight[slot]=startWeight;
	_StartWeightTime[slot]=time;
}

// ***************************************************************************

float CAnimationPlaylist::getStartWeight (uint8 slot, CAnimationTime& time) const
{
	time=_StartWeightTime[slot];
	return _StartWeight[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setEndWeight (uint8 slot, float endWeight, CAnimationTime time)
{
	_EndWeight[slot]=endWeight;
	_EndWeightTime[slot]=time;
}

// ***************************************************************************

float CAnimationPlaylist::getEndWeight (uint8 slot, CAnimationTime& time) const
{
	time=_EndWeightTime[slot];
	return _EndWeight[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setWeightSmoothness (uint8 slot, float smoothness)
{
	_Smoothness[slot]=smoothness;
}

// ***************************************************************************

float CAnimationPlaylist::getWeightSmoothness (uint8 slot) const
{
	return _Smoothness[slot];
}

// ***************************************************************************

void CAnimationPlaylist::setupMixer (CChannelMixer& mixer, CAnimationTime time) const
{
	// For each slot
	for (uint8 s=0; s<CChannelMixer::NumAnimationSlot; s++)
	{
		// *** Set the animation

		// empty ?
		if (_Animations[s]==empty)
			// Empty the slot
			mixer.emptySlot (s);
		else
			// Set the animation id
			mixer.setSlotAnimation (s, _Animations[s]);

		// *** Set the skeleton weight

		// empty ?
		if (_SkeletonWeight[s]==empty)
			// Empty the slot
			mixer.resetSkeletonWeight (s);
		else
			// Set the animation id
			mixer.applySkeletonWeight (s, _SkeletonWeight[s], 	_InvertWeight[s]);

		// *** Set the time

		// Get the animationSet pointer from the mixer
		const CAnimationSet *animSet=mixer.getAnimationSet ();

		// If is exists
		if (animSet)
		{
			// Get the animation
			const CAnimation *pAnimation=animSet->getAnimation (_Animations[s]);

			// If this animation exists
			if (pAnimation)
			{
				// Compute time
				CAnimationTime animTime=pAnimation->getBeginTime ()+(time-_TimeOrigin[s])/_SpeedFactor[s];

				// Set the time
				mixer.setSlotTime (s, animTime);
			}
		}

		// *** Set the weight
		mixer.setSlotWeight (s, getWeightValue (_StartWeightTime[s], _EndWeightTime[s], time, _StartWeight[s], _EndWeight[s], _Smoothness[s]));
	}
}

// ***************************************************************************

float CAnimationPlaylist::getWeightValue (float startWeightTime, float endWeightTime, float time, float startWeight, float endWeight, float smoothness)
{
	// Clamp left
	if (time<=startWeightTime)
		return startWeight;
	// Clamp left
	if (time>=endWeightTime)
		return endWeight;
	
	// *** Interpolate
	
	// Linear value
	float linear=startWeight+(endWeight-startWeight)*(time-startWeightTime)/(endWeightTime-startWeightTime);

	// Linear ?
	if (smoothness<0.0001f)
		return linear;

	// Quadratic value
	float a=2.f*startWeight-2.f*endWeight;
	float b=3.f*endWeight-3.f*startWeight;
	float x=(time-startWeightTime)/(endWeightTime-startWeightTime);
	float xSquare=x*x;
	float xCube=x*xSquare;
	float quad=a*xCube+b*xSquare+startWeight;

	// Interpolate between linear and quadratic
	return smoothness*quad+(1.f-smoothness)*linear;
}

// ***************************************************************************

} // NL3D
