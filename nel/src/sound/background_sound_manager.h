/** \file background_sound_manager.h
 * CBackgroundSoundManager
 *
 * $Id: background_sound_manager.h,v 1.2 2002/07/30 14:25:42 miller Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

#ifndef NL_BACKGROUND_SOUND_MANAGER_H
#define NL_BACKGROUND_SOUND_MANAGER_H

#include "nel/misc/types_nl.h"
#include <vector>


namespace NLSOUND {


class CListenerUser;
class IPlayable;
class IBoundingShape;


/**
 * This system replaces the old olivier cado system (ambiant_source + env sound)
 * It's not the final system, just a quick and dirty coded.
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2002
 */
class CBackgroundSoundManager
{
public:

	// 
	static	void init (UAudioMixer *am);

	static	void release ();

	/** Load background sound for a continent. It'll automatically unload the old continent before loading the new one
	 * contient is for example "matis" or "fyros". It'll add .prim to the continent name and lookup() to find zones.
	 * So, don't forget to add sound .prim in the CPath system for the lookup
	 */
	static	void load (const std::string &continent);

	/** Load a background sound, only when the distance is little to avoid allocated too many sources.
	 */
	static	void load (uint32 i);

	/** Unoad a background sound, only when the distance is too far. avoid allocated of too many sources.
	 */
	static	void unload (uint32 i);

	/** Remove all data about the current continents
	 */
	static	void unload ();

	/// Call this function when the listener position change
	static	void setListenerPosition (const NLMISC::CVector &listenerPosition);

	// Call this function evenly to update the stuffs
	static	void update ();

	static	void play ();
	static	void stop ();

	static	void setDayNightRatio(float ratio);

	
	// These functions are for debug purpose

	// Return the number of zone loaded (patatoid)
	static	uint32 getZoneNumber ();

	// Return a patatoid
	static const std::vector<NLMISC::CVector> &getZone(uint32 zone);

	// Return the position of the 3d source for a zone
	static NLMISC::CVector getZoneSourcePos(uint32 zone);
	
private:

	/// Constructor
	CBackgroundSoundManager() {}

	/// Destructor
	virtual					~CBackgroundSoundManager() {}

	static UAudioMixer *_AudioMixer;
};


} // NLSOUND


#endif // NL_BACKGROUND_SOUND_MANAGER_H

/* End of background_sound_manager.h */
