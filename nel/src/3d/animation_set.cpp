/** \file animation_set.cpp
 * <File description>
 *
 * $Id: animation_set.cpp,v 1.2 2001/03/08 12:57:40 corvazier Exp $
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

const CAnimation* CAnimationSet::getAnimation (uint animationId) const
{
	// Look for an id with this name
	return &_Animation[animationId];
}

// ***************************************************************************

} // NL3D
