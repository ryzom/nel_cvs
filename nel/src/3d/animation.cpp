/** \file animation.cpp
 * <File description>
 *
 * $Id: animation.cpp,v 1.13 2002/04/12 16:16:38 vizerie Exp $
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

#include "3d/animation.h"
#include "3d/track.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"

namespace NL3D 
{

// ***************************************************************************

CAnimation::CAnimation() : _BeginTimeTouched(true), _EndTimeTouched(true)
{	
}

// ***************************************************************************

CAnimation::~CAnimation ()
{
	// Delete all the pointers in the array
	for (uint i=0; i<_TrackVector.size(); i++)
		// Delete
		delete _TrackVector[i];
}

// ***************************************************************************

void CAnimation::addTrack (const std::string& name, ITrack* pChannel)
{
	// Add an entry in the map
	_IdByName.insert (TMapStringUInt::value_type (name, (uint32)_TrackVector.size()));

	// Add an entry in the array
	_TrackVector.push_back (pChannel);

	// 
	_BeginTimeTouched = _EndTimeTouched = true;

}

// ***************************************************************************

void CAnimation::serial (NLMISC::IStream& f)
{
	// Serial a header
	f.serialCheck ((uint32)'_LEN');
	f.serialCheck ((uint32)'MINA');

	// Serial a version
	sint version=f.serialVersion (0);

	// Serial the name
	f.serial (_Name);

	// Serial the name/id map
	f.serialCont(_IdByName);

	// Serial the vector
	f.serialContPolyPtr (_TrackVector);

}

// ***************************************************************************

void CAnimation::getTrackNames (std::set<std::string>& setString) const
{
	// For each track name
	TMapStringUInt::const_iterator ite=_IdByName.begin();
	while (ite!=_IdByName.end())
	{
		// Add the name in the map
		setString.insert (ite->first);

		// Next track
		ite++;
	}
}

// ***************************************************************************

TAnimationTime CAnimation::getBeginTime () const
{
	if (_BeginTimeTouched)
	{
		// Track count
		uint trackCount=_TrackVector.size();

		// Track count empty ?
		if (trackCount==0)
			return 0.f;

		// Look for the lowest
		_BeginTime=_TrackVector[0]->getBeginTime ();

		// Scan all keys
		for (uint t=1; t<trackCount; t++)
		{
			if (_TrackVector[t]->getBeginTime ()<_BeginTime)
				_BeginTime=_TrackVector[t]->getBeginTime ();
		}

		_BeginTimeTouched = false;
	}

	return _BeginTime;
}

// ***************************************************************************

TAnimationTime CAnimation::getEndTime () const
{
	if (_EndTimeTouched)
	{
		// Track count
		uint trackCount=_TrackVector.size();

		// Track count empty ?
		if (trackCount==0)
			return 0.f;

		// Look for the highest
		_EndTime=_TrackVector[0]->getEndTime ();

		// Scan tracks keys
		for (uint t=1; t<trackCount; t++)
		{
			if (_TrackVector[t]->getEndTime ()>_EndTime)
				_EndTime=_TrackVector[t]->getEndTime ();
		}
		_EndTimeTouched = false;
	}

	return _EndTime;
}

// ***************************************************************************

UTrack* CAnimation::getTrackByName (const char* name)
{
	// Get track id
	uint id=getIdTrackByName (name);

	// Not found ?
	if (id==CAnimation::NotFound)
		// Error, return NULL
		return NULL;
	else
		// No error, return the track
		return getTrack (id);
}

// ***************************************************************************

void CAnimation::releaseTrack (UTrack* track)
{
	// Nothing to do
}

// ***************************************************************************

UAnimation* UAnimation::createAnimation (const char* sPath)
{
	// Allocate an animation
	std::auto_ptr<CAnimation> anim (new CAnimation);

	// Read it
	NLMISC::CIFile file;
	if (file.open ( NLMISC::CPath::lookup( sPath ) ) )
	{
		// Serial the animation
		file.serial (*anim);

		// Return pointer
		CAnimation *ret=anim.release ();

		// Return the animation interface
		return ret;
	}
	else 
		return NULL;
}

// ***************************************************************************

void UAnimation::releaseAnimation (UAnimation* animation)
{
	// Cast the pointer
	CAnimation* release=(CAnimation*)animation;

	// Delete it
	delete release;
}

// ***************************************************************************

} // NL3D
