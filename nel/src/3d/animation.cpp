/** \file animation.cpp
 * <File description>
 *
 * $Id: animation.cpp,v 1.3 2001/03/08 13:29:07 corvazier Exp $
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

#include "nel/3d/animation.h"
#include "nel/3d/track.h"
#include "nel/misc/stream.h"


namespace NL3D 
{

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
}

// ***************************************************************************

void CAnimation::serial (NLMISC::IStream& f) throw (NLMISC::EStream)
{
	// Serial a header
	f.serialCheck ((uint32)"_LEN");
	f.serialCheck ((uint32)"MINA");

	// Serial a version
	sint version=f.serialVersion (0);

	// Serial the name/id map
	f.serialMap (_IdByName);

	// Serial the vector
	f.serialContPolyPtr (_TrackVector);
}

// ***************************************************************************

} // NL3D
