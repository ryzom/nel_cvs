/** \file sound_anim_player.h
 * A sound event marer on a sound track
 *
 * $Id: sound_anim_player.h,v 1.1 2002/06/18 16:02:32 hanappe Exp $
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

#ifndef NL_SOUND_ANIM_PLAYER_H
#define NL_SOUND_ANIM_PLAYER_H

#include "nel/sound/u_audio_mixer.h"
#include "nel/misc/vector.h"

namespace NLSOUND {

typedef enum { NLSOUND_SOUND_ANIM_PLAYING, NLSOUND_SOUND_ANIM_STOPPED } TSoundAnimPlayerState;

class CSoundAnimation;

/**

  CSoundAnimPlayer manages the playback of an animation. 

  */

class CSoundAnimPlayer
{
public:

	/**
     *  Constructor
     *  
     *  The player start the animation at a time offset and position given as argument to
	 *  the constructor. The time offset if the absolute time.  The offset will be substracted
	 *  from the time passed to the update() method to determine to local time of the 
	 *  animation. The position determines where the animation should be position in space.
	 *  The position of the animation can change but the pointer to the vector position should 
	 *  remain valid throughout the lifetime of the player.
     *
	 *  \param animation The animation to be played.
	 *  \param time The offset in time of the animation. 
	 *  \param position A pointer to a vector indicating the position of the animation.
     */
	CSoundAnimPlayer(CSoundAnimation* animation, float time, NLMISC::CVector *position, NLSOUND::UAudioMixer* mixer, sint32 id);

	virtual ~CSoundAnimPlayer() {}

	/** Update the track for playback. Sends all the events between the
	 *  last call and the current call
	 */
	virtual void				update(float lastTime, float curTime);

	/** Returns true if the player still playing */
	virtual bool				isPlaying()		{ return _Playing; }

	/** Get the unique id of this player instance */
	virtual sint32				getId()			{ return _Id; }

protected:

	/** The audio mixer */
	NLSOUND::UAudioMixer		*_Mixer;

	/** The start time of this animation */
	float						_StartTime;

	/** The animation to be played */
	CSoundAnimation*			_Animation;

	//uint32						_NextMarker;
	//uint32						_NumMarkers;

	/** Is the player still playing? */
	bool						_Playing;

	/** A auxiliary vector to store the sounds of an animation marker */
	std::vector<const char*>	_Sounds;

	/** The pointer to the position vector of the animation */
	NLMISC::CVector				*_Position;

	/** The ID of the player */
	sint32						_Id;
};

} // namespace NLSOUND

#endif // NL_SOUND_ANIM_TRACK_H
