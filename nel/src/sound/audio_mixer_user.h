/** \file audio_mixer_user.h
 * CAudioMixerUser: implementation of UAudioMixer
 *
 * $Id: audio_mixer_user.h,v 1.1 2001/07/10 16:48:03 cado Exp $
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

#ifndef NL_AUDIO_MIXER_USER_H
#define NL_AUDIO_MIXER_USER_H

#include "nel/misc/types_nl.h"
#include "nel/sound/u_audio_mixer.h"
#include "driver/source.h"
#include "listener_user.h"
#include "track.h"
#include <vector>


namespace NLSOUND {


class CSourceUser;
class CEnvSoundUser;
class CEnvEffect;


// Max number of tracks (physical sources)
#define MAX_TRACKS 32


/**
 * Implementation of UAudioMixer
 *
 * The logical sources (_Sources) are the sources representing all entities in the world, from
 * the client's point of vue.
 * The tracks (_Tracks) are the physical sources played by the sound driver. Their number
 * is small.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CAudioMixerUser : public UAudioMixer
{
public:

	/// Constructor
	CAudioMixerUser();
	/// Return the audio mixer object
	static CAudioMixerUser		*instance() { return _Instance; }
	/// Destructor
	virtual						~CAudioMixerUser();

	
	/// Initialization
	virtual void				init();
	/// Load environmental effects
	virtual void				loadEnvEffects( const char *filename );
	/// Load buffers
	virtual void				loadSoundBuffers( const char *filename,
											  const std::vector<TSoundId> **idvec );
	/// Load environment sounds ; idvec can be null if you don't want an access to the envsounds
	virtual	void				loadEnvSounds( const char *filename,
											   const std::vector<UEnvSound*> **esvec=NULL );
	/// Add logical sound source
	virtual USource				*createSource( TSoundId id );
	/// Remove logical sound source
	virtual void				removeSource( USource *source );
	// Remove environment sound
	//virtual void				removeEnvSound( UEnvSound *envsound );
	/// Return the listener interface
	virtual UListener			*getListener()	{ return &_Listener; }


	/// Choose the environmental effect(s) corresponding to tag
	virtual void				selectEnvEffects( const std::string& tag );
	/// Update audio mixer (call evenly)
	virtual void				update(); 


	/// Put source into a track
	bool						giveTrack( CSourceUser *source );
	/// Release track
	void						releaseTrack( CSourceUser *source );
	/// Take a listener's move into account
	void						applyListenerMove( const NLMISC::CVector& listenerpos );

protected:

	/// Returns nb available tracks (or NULL)
	void						getFreeTracks( uint nb, CTrack **tracks );
	/// Select the appropriate environmental effect
	void						computeEnvEffect( const NLMISC::CVector& listenerpos, bool force=false );

private:

	/// The audio mixer singleton instance
	static CAudioMixerUser		*_Instance;

	/// The sound driver instance
	ISoundDriver				*_SoundDriver;

	/// Sound buffers and static properties
	std::vector<CSound*>		_Sounds;

	/// The listener instance
	CListenerUser				_Listener;

	/// Logical 3d sources
	std::vector<CSourceUser*>	_Sources;

	/// Environment sounds
	std::vector<CEnvSoundUser*>	_EnvSounds;

public: // Temp (EDIT)

	/// Environment effects
	std::vector<CEnvEffect*>	_EnvEffects;
	
private:

	/// Current effect
	CEnvEffect					*_CurEnvEffect;

	/// Physical sources array
	CTrack						*_Tracks [MAX_TRACKS];

	/// Size of the physical sources array (must be <= MAX_TRACKS)
	uint						_NbTracks;
};


} // NLSOUND


#endif // NL_AUDIO_MIXER_USER_H

/* End of audio_mixer_user.h */
