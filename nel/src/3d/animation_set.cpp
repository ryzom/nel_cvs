/** \file animation_set.cpp
 * <File description>
 *
 * $Id: animation_set.cpp,v 1.3 2001/03/08 13:29:07 corvazier Exp $
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

#include "nel/3d/animation_set.h"


namespace NL3D 
{

// ***************************************************************************

uint CAnimationSet::getNumChannelId () const
{
	return _ChannelIdByName.size ();
}

// ***************************************************************************

uint CAnimationSet::getChannelIdByName (const std::string& name) const
{
	// Look for an id with this name
	std::map <std::string, uint32>::const_iterator ite=_ChannelIdByName.find (name);
	if (ite!=_ChannelIdByName.end ())
		return ite->second;
	else
		return NotFound;
}

// ***************************************************************************

uint CAnimationSet::getAnimationIdByName (const std::string& name) const
{
	// Look for an id with this name
	std::map <std::string, uint32>::const_iterator ite=_AnimationIdByName.find (name);
	if (ite!=_AnimationIdByName.end ())
		return ite->second;
	else
		return NotFound;
}

// ***************************************************************************

uint CAnimationSet::addAnimation (const std::string& name)
{
	// Add an animation
	_Animation.resize (_Animation.size()+1);

	// Add an entry name / animation
	_AnimationIdByName.insert (std::map <std::string, uint32>::value_type (name, _Animation.size()-1));

	// Return animation id
	return _Animation.size()-1;
}

// ***************************************************************************

void CAnimationSet::build ()
{
	// Clear the channel map
	_ChannelIdByName.clear ();

	// Set of names
	std::set<std::string> channelNames;

	// For each animation in the set
	for (uint a=0; a<_Animation.size(); a++)
	{
		// Fill the set of channel names
		getAnimation (a)->getTrackNames (channelNames);
	}

	// Add this name in the map with there iD
	uint id=0;
	std::set<std::string>::iterator ite=channelNames.begin ();
	while (ite!=channelNames.end ())
	{
		// Insert an entry
		_ChannelIdByName.insert (std::map <std::string, uint32>::value_type (*ite, id++));

		// Next entry
		ite++;
	}
}

// ***************************************************************************

} // NL3D
