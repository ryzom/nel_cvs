/** \file sound_anim_manager.h
 * The sound animation manager handles all request to load, play, and
 * update sound animations.
 *
 * $Id: sound_anim_manager.h,v 1.1 2002/06/18 16:02:32 hanappe Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_SOUND_ANIM_MANAGER_H
#define NL_SOUND_ANIM_MANAGER_H

#include "sound_animation.h"
#include "sound_anim_player.h"

#include "nel/misc/vector.h"
#include <hash_map>


namespace NLSOUND {


// Comparision for const char*
struct eqsndanim
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};


/// Animation name-to-id hash map
typedef std::hash_map<const char*, TSoundAnimId, std::hash<const char*>, eqsndanim> TSoundAnimMap;

/// Animation names vector
typedef std::vector<CSoundAnimation*> TSoundAnimVector;

/// The set of players
typedef std::set<CSoundAnimPlayer*> TPlayerSet;


class CSoundAnimManager
{
public:

	static CSoundAnimManager* instance() { return _Instance; }

	CSoundAnimManager(NLSOUND::UAudioMixer* mixer);
	virtual ~CSoundAnimManager();

	/** Load the sound animation with the specified name. 
	 *  Throws ESoundAnimationNotFound if the file could not be found.
	 *  \param name The name of the animation to load.
	 */
	virtual TSoundAnimId			loadAnimation(std::string& name);

	/** Create a new sound animation with the specified name. 
	 *  Returns CSoundAnimation::NoId if the creation fails (duplicate name).
	 *  \param name The name of the animation to load.
	 */
	virtual TSoundAnimId			createAnimation(std::string& name);

	/** Save the sound animation in the specified file. 
	 *  \param filename The name of the file to save the animation in.
	 */
	virtual void					saveAnimation(CSoundAnimation* anim, std::string& filname);

	/** Start playing a sound animation. Returns true is the animation was
	 *  found and is playing.
 	 *  \param name The name of the animation to load.
	 */
	virtual TSoundAnimId			getAnimationFromName(std::string& name);

	/** Returns the animation corresponding to a name.
	 *  \param name The name of the animation to load.
	 */
	virtual CSoundAnimation*		findAnimation(std::string& name);

	/** Start playing a sound animation. Returns an id number of this playback instance
	 *  or -1 if the animation was not found.
 	 *  \param name The id of the animation to play.
	 */
	virtual sint32					playAnimation(TSoundAnimId id, float time, NLMISC::CVector* position);

	/** Start playing a sound animation. Returns an id number of this playback instance
	 *  or -1 if the animation was not found.
 	 *  \param name The name of the animation to play.
	 */
	virtual sint32					playAnimation(std::string& name, float time, NLMISC::CVector* position);

	/** Stop the playing of a sound animation. 
 	 *  \param name The playback id that was returned by playAnimation.
	 */
	virtual void					stopAnimation(sint32 playbackId);

	/** Returns true is the animation with the specified playback ID is playing
 	 *  \param name The playback id that was returned by playAnimation.
	 */
	//virtual bool				isPlaying(sint32 playbackId);

	/** Update all the sound animations during playback. 
	 */
	virtual void					update(float lastTime, float curTime);


protected:

	/** The one and only singleton instance */
	static CSoundAnimManager*		_Instance;

	/** The mixer */
	NLSOUND::UAudioMixer			*_Mixer;

	/** The conversion table from animation name to id */
	TSoundAnimMap					_IdMap;

	/** The vector of all defined animations */
	TSoundAnimVector				_Animations;

	/** The set of all active players */
	TPlayerSet						_Players;

	/** The id of the next player */
	sint32							_PlayerId;

	/** An auxilary vector to help remove players from the active set */
	std::vector<CSoundAnimPlayer*>	_Garbage;

};

/**
 * ESoundFileNotFound
 */

class ESoundAnimationNotFound : public NLMISC::Exception
{
public:
	ESoundAnimationNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sound animation not found: ")+filename).c_str() ) {}
};

} // namespace NLSOUND

#endif // NL_SOUND_ANIM_MANAGER_H
