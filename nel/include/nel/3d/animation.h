/** \file animation.h
 * <File description>
 *
 * $Id: animation.h,v 1.3 2001/03/07 17:11:46 corvazier Exp $
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

#ifndef NL_ANIMATION_H
#define NL_ANIMATION_H

#include "nel/misc/types_nl.h"
#include <memory>
#include <map>
#include <vector>


namespace NL3D 
{

class ITrack;

/**
 * This class describes animations for several tracks. Each track works
 * with a IChannel for the IAnimatable objects.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimation
{
public:
	// Typedef to avoid stupid VC++ warning

	class iS : public std::string {};
	class mI : public std::map<iS, uint> {};

	/// \name Public interface.

	enum { NotFound=0xffffffff };

	/** Get track with its name.
	  *
	  * \param name is the name of the desired track.
	  * \return CAnimation::NotFound if the track doesn't exist else the the id of the track.
	  */
	uint getIdTrackByName (const std::string& name) const;

	/** Get a track pointer
	  *
	  * \param channelId is the id of the desired channel.
	  */
	const ITrack* getTrack (uint trackId) const
	{
		return _TrackVector[trackId].get();
	}

	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/** Add a track at the end of the track list.
	  * 
	  * This method is used to insert tracks in the animation.
	  * Tracks must be allocated with new. The pointer is then maintained
	  * by the CAnimation.
	  */
	void addTrack (const std::string& name, ITrack* pChannel);

private:
	/// \name Members

	// Map to get a channel id with a name.
	mI									_IdByName;

	// Vector of channel pointer.
	std::vector<std::auto_ptr<ITrack> >	_TrackVector;
};


} // NL3D


#endif // NL_ANIMATION_H

/* End of animation.h */
