/** \file animatable.h
 * Class IAnimatable
 *
 * $Id: animatable.h,v 1.1 2001/02/05 16:52:44 corvazier Exp $
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

#ifndef NL_ANIMATABLE_H
#define NL_ANIMATABLE_H

#include "nel/misc/types_nl.h"
#include <string>
#include <vector>
#include <map>


namespace NL3D 
{

class IChannel;

/**
 * An animatable object. This object can have a set of animation channels. 
 * Each channels handle a value. Channels are animated by a CChannelMixer.
 * Each channel have a name.
 *
 * When a class derives from IAnimatable, it must add all its channels by
 * calling IAnimatable::addChannel(name, channelPointer).
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class IAnimatable
{
public:

	/// \name Public interface.

	enum { NotFound=0xffffffff };

	/** Get channel with its name.
	  *
	  * \param name is the name of the desired channel.
	  * \return IAnimatable::NotFound if the channel doesn't exist else the the id of the channel.
	  */
	uint getIdChannelByName (const std::string& name) const;

	/** Get a read only channel
	  *
	  * \param channelId is the id of the desired channel.
	  */
	const IChannel& getChannel (uint channelId) const
	{
		return *_ChannelVector[channelId];
	}

	/** Add a channel at the end of the channel list.
	  * 
	  * This method is used by the derived IAnimatable classes to insert
	  * there channel's pointers in the name/channel map and in the vector
	  * of channel when they are created.
	  */
	void addChannel (const std::string& name, IChannel* pChannel);

private:
	/// \name Members

	// Map to get a channel id with a name.
	std::map<std::string, uint>	_IdByName;

	// Vector of channel pointer.
	std::vector<IChannel*>		_ChannelVector;
};


} // NL3D


#endif // NL_ANIMATABLE_H

/* End of animatable.h */
