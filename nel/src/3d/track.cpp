/** \file track.cpp
 * <File description>
 *
 * $Id: track.cpp,v 1.17 2002/02/28 12:59:52 besson Exp $
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

#include "3d/track.h"

#include "nel/misc/rgba.h"

using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

/*
// Some compilation check: force Visual to compile to template
CTrackDefaultFloat ttoto10;
CTrackDefaultVector ttoto11;
CTrackDefaultQuat ttoto12;
CTrackDefaultInt ttoto13;
CTrackDefaultRGBA ttoto16;
CTrackDefaultString ttoto14;
CTrackDefaultBool ttoto15;
*/

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, float& res)
{
	// Evaluate it 
	eval (time);

	// Get a pointer on the value
	const CAnimatedValueFloat *value=dynamic_cast<const CAnimatedValueFloat*>(&getValue ());

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, sint32& res)
{
	// Evaluate it 
	eval (time);

	// Get a pointer on the value
	const CAnimatedValueInt *value=dynamic_cast<const CAnimatedValueInt*>(&getValue ());

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, CRGBA& res)
{
	// Evaluate it 
	eval (time);

	// Get a pointer on the value
	const CAnimatedValueRGBA *value=dynamic_cast<const CAnimatedValueRGBA*>(&getValue ());

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, CVector& res)
{
	// Evaluate it 
	eval (time);

	// Get a pointer on the value
	const CAnimatedValueVector *value=dynamic_cast<const CAnimatedValueVector*>(&getValue ());

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, CQuat& res)
{
	// Evaluate it 
	eval (time);

	// Get a pointer on the value
	const CAnimatedValueQuat *value=dynamic_cast<const CAnimatedValueQuat*>(&getValue ());

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, std::string& res)
{
	// Evaluate it 
	eval (time);

	// Get a pointer on the value
	const CAnimatedValueString *value=dynamic_cast<const CAnimatedValueString*>(&getValue ());

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

bool ITrack::interpolate (TAnimationTime time, bool& res)
{
	// Evaluate it 
	eval (time);

	// Get a pointer on the value
	const CAnimatedValueBool *value=dynamic_cast<const CAnimatedValueBool*>(&getValue ());

	// Type is good ?
	if (value)
	{
		// Ok, return the value
		res=value->Value;
		return true;
	}
	else
		// No, return false
		return false;
}

// ***************************************************************************

} // NL3D
