/** \file audio_mixer_user.h
 * CAudioMixerUser: implementation of UAudioMixer
 *
 * $Id: audio_mixer_user.h,v 1.4 2001/07/17 14:21:54 cado Exp $
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
#include <set>


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
 * the client's point of view.
 * The tracks (_Tracks) are the physical sources played by the sound driver. Their number
 * is small.
 *
 * When there are more sources than tracks, the process of choosing which sources go into
 * the tracks is called "balancing". The source are auto-balanced according to the
 * argument passed to init(). The sources are also balanced when
 * - Adding a new source
 * - Removing a new source
 * - Entering/Exiting from an envsound area
 *
 * Important: The user is responsible for deleting the sources that have been allocated by
 * createSource(), before deleting the audio mixer object.
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

	
	/** Initialization
	 * The sources will be auto-balanced every "balance_period" calls to update()
	 * (set 0 for "never auto-balance")
	 */
	virtual void				init( uint32 balance_period=AUTOBALANCE_DEFAULT_PERIOD );
	/// Load environmental effects
	virtual void				loadEnvEffects( const char *filename );
	/// Load buffers
	virtual void				loadSoundBuffers( const char *filename,
											  const std::vector<TSoundId> **idvec );
	/// Load environment sounds ; treeRoot can be null if you don't want an access to the envsounds
	virtual	void				loadEnvSounds( const char *filename,
											   UEnvSound **treeRoot=NULL );
	/// Add logical sound source
	virtual USource				*createSource( TSoundId id );
	/// Return the listener interface
	virtual UListener			*getListener()	{ return &_Listener; }


	/// Choose the environmental effect(s) corresponding to tag
	virtual void				selectEnvEffects( const std::string& tag );
	/// Update audio mixer (call evenly)
	virtual void				update(); 


	/// Remove logical sound source
	void						removeSource( USource *source );
	/// Add a source which was created by an EnvSound
	void						addSource( CSourceUser *source )		{ _Sources.insert( source ); }
	/// Put source into a track
	void						giveTrack( CSourceUser *source );
	/// Release track
	void						releaseTrack( CSourceUser *source );
	/// Take a listener's move into account
	void						applyListenerMove( const NLMISC::CVector& listenerpos );
	/// Redispatch the sources into tracks if needed
	void						balanceSources()						{ if ( moreSourcesThanTracks() ) redispatchSourcesToTrack(); }
	/// Return the root of the envsounds tree
	CEnvSoundUser				*getEnvSounds()							{ return _EnvSounds; }

protected:

	/// Returns nb available tracks (or NULL)
	void						getFreeTracks( uint nb, CTrack **tracks );
	/// Select the appropriate environmental effect
	void						computeEnvEffect( const NLMISC::CVector& listenerpos, bool force=false );
	/// Return true if the number of user sources is higher than the number of tracks
	bool						moreSourcesThanTracks() const			{ return _NbTracks < _Sources.size(); }
	/// Redispatch the sources (call only if moreSourcesThanTracks() returns true)
	void						redispatchSourcesToTrack();

private:

	/// The audio mixer singleton instance
	static CAudioMixerUser		*_Instance;

	/// The sound driver instance
	ISoundDriver				*_SoundDriver;

	/// Sound buffers and static properties
	std::vector<CSound*>		_Sounds;

	/// The listener instance
	CListenerUser				_Listener;

	/// Listener position vector
	NLMISC::CVector				_ListenPosition;

	/// Environment sounds tree
	CEnvSoundUser				*_EnvSounds;

	/// Auto-Balance period
	uint32						_BalancePeriod;

public: // Temp (EDIT)

	/// All Logical sources
	std::set<CSourceUser*>		_Sources;

	/// Environment effects
	std::vector<CEnvEffect*>	_EnvEffects;
	
private:

	/// Current effect
	CEnvEffect					*_CurEnvEffect;

public: // Temp (EDIT)
	
	/// Physical sources array
	CTrack						*_Tracks [MAX_TRACKS];

	/// Size of the physical sources array (must be <= MAX_TRACKS)
	uint						_NbTracks;
};


} // NLSOUND


#endif // NL_AUDIO_MIXER_USER_H

/* End of audio_mixer_user.h */
