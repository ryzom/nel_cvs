/** \file audio_mixer_user.h
 * CAudioMixerUser: implementation of UAudioMixer
 *
 * $Id: audio_mixer_user.h,v 1.21 2002/06/04 10:06:01 hanappe Exp $
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
#include "nel/misc/time_nl.h"
#include "driver/source.h"
#include "listener_user.h"
#include "mixing_track.h"
#include "sound.h"
#include <vector>
#include <set>


namespace NLSOUND {


class CSourceUser;
class CEnvSoundUser;
class CEnvEffect;


/*
 * Max number of tracks (physical sources)
 *
 * Note: In the Windows implementation of OpenAL, AL_SOURCES_MAX is currently defined to 32
 * but seems unused. In alc.c (AL Context implementation), the function alcOpenDevice() contains:
 *     // Check how many Hardware 3D Streaming buffers can be created
 *     device->MaxNoOfSources = dsCaps.dwMaxHw3DStreamingBuffers;
 * and this value seems to be no more than 32 in DirectX8.
 */
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
	 *
	 * In case of failure, can throw one of these ESoundDriver (Exception) objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion.
	 *
	 * The sources will be auto-balanced every "balance_period" calls to update()
	 * (set 0 for "never auto-balance")
	 */
	virtual void				init( uint32 balance_period=AUTOBALANCE_DEFAULT_PERIOD );
	/// Resets the audio system (deletes all the sources, include envsounds)
	virtual void				reset();
	/// Disables or reenables the sound
	virtual void				enable( bool b );
	/// Load environmental effects
	virtual void				loadEnvEffects( const char *filename );
	/** Load buffers. Returns the number of buffers successfully loaded.
	 * If you specify a non null notfoundfiles vector, it is filled with the names of missing files if any.
	 * You can call this method several times, to load several sound banks.
	 */
	virtual uint32				loadSoundBuffers( const char *filename, std::vector<std::string> *notfoundfiles=NULL );
	/// Load environment sounds ; treeRoot can be null if you don't want an access to the envsounds
	virtual	void				loadEnvSounds( const char *filename,
											   UEnvSound **treeRoot=NULL );
	/// Get a TSoundId from a name (returns NULL if not found)
	virtual TSoundId			getSoundId( const char *name );


	/** Add a logical sound source (returns NULL if name not found).
	 * If spawn is true, the source will auto-delete after playing. If so, the return USource* pointer
	 * is valid only before the time when calling play() plus the duration of the sound. You can
	 * pass a callback function that will be called (if not NULL) just before deleting the spawned
	 * source.
	 */
	virtual USource				*createSource( const char *name, bool spawn=false, TSpawnEndCallback cb=NULL, void *cbUserParam = NULL );
	/// Add a logical sound source (by sound id). To remove a source, just delete it. See createSource(const char*)
	virtual USource				*createSource( TSoundId id, bool spawn=false, TSpawnEndCallback cb=NULL, void *cbUserParam = NULL );
	/// Add a source which was created by an EnvSound
	void						addSource( CSourceUser *source );
	/** Delete a logical sound source. If you don't call it, the source will be auto-deleted
	 * when deleting the audio mixer object
	 */
	virtual void				removeSource( USource *source );

	/// Put source into a track
	void						giveTrack( CSourceUser *source );
	/// Release track
	void						releaseTrack( CSourceUser *source );

	/// Return the listener interface
	virtual UListener			*getListener()	{ return &_Listener; }


	/// Choose the environmental effect(s) corresponding to tag
	virtual void				selectEnvEffects( const char *tag );
	/// Update audio mixer (call evenly)
	virtual void				update(); 


	/// Return the names of the sounds (call this method after loadSoundBuffers())
	virtual void				getSoundNames( std::vector<const char *>& names ) const;
	/// Return the number of mixing tracks (voices)
	virtual uint				getPolyphony() const { return _NbTracks; }
	/// Return the number of sources (slow)
	virtual uint				getSourcesNumber() const { return _Sources.size(); }
	/// Return the number of playing sources (slow)
	virtual uint				getPlayingSourcesNumber() const;
	/// Return the number of available tracks
	virtual uint				getNumberAvailableTracks() const;

	/// Return a string showing the playing sources (slow)
	virtual std::string			getSourcesStats() const;


	/// Take a listener's move into account
	void						applyListenerMove( const NLMISC::CVector& listenerpos );
	/// Return the root of the envsounds tree
	CEnvSoundUser				*getEnvSounds()							{ return _EnvSounds; }
	/// Return the listen pos vector
	const NLMISC::CVector&		getListenPosVector() const				{ return _ListenPosition; }
	/** Same as removeSource() but does not delete the object (e.g. when not allocated by new,
	 * as the CAmbiantSource channels)
	 */
	void						removeMySource( USource *source );
	/// Add ambiant sound pointer for later deletion
	void						addAmbiantSound( CSound *sound )		{ _AmbSounds.insert( sound ); }
	// Allow to load sound files (nss) when the corresponding wave file is missing (see CSound)
	//static void					allowMissingWave( bool b )				{ CSound::allowMissingWave( b ); }	

protected:

	/// Redispatch the sources into tracks if needed
	void						balanceSources()						{ if ( moreSourcesThanTracks() ) redispatchSourcesToTrack(); }
	/// Returns nb available tracks (or NULL)
	void						getFreeTracks( uint nb, CTrack **tracks );
	/// Select the appropriate environmental effect
	void						computeEnvEffect( const NLMISC::CVector& listenerpos, bool force=false );
	/// Return true if the number of user sources is higher than the number of tracks
	bool						moreSourcesThanTracks() const			{ return _NbTracks < _Sources.size(); }
	/// Redispatch the sources (call only if moreSourcesThanTracks() returns true)
	void						redispatchSourcesToTrack();
	/// See removeSource(USource*) and removeMySource(USource*)
	void						removeSource( std::set<CSourceUser*>::iterator ips, bool deleteit );

private:

	/// The audio mixer singleton instance
	static CAudioMixerUser		*_Instance;

	/// The sound driver instance
	ISoundDriver				*_SoundDriver;

	/// Sound buffers and static properties
	TSoundMap					_Sounds;

	/// Sound buffers used (and allocated but not deleted because shared) by ambiant sources
	TSoundSet					_AmbSounds;

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

	/// Max _NbTracks
	uint						_MaxNbTracks;

	/// Flag set in destructor
	bool						_Leaving;

	NLMISC::TTicks				_StartTime;

	uint32						curTime() { return (uint32) (NLMISC::CTime::getLocalTime() - _StartTime); }

};


/// Return the priority cstring (debug info)
const char *getPriorityStr( TSoundPriority p );


} // NLSOUND


#endif // NL_AUDIO_MIXER_USER_H

/* End of audio_mixer_user.h */
