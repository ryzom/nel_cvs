/** \file audio_mixer_user.cpp
 * CAudioMixerUser: implementation of UAudioMixer
 *
 * $Id: audio_mixer_user.cpp,v 1.1 2001/07/10 16:48:03 cado Exp $
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

#include "audio_mixer_user.h"
#include "source_user.h"
#include "env_sound_user.h"
#include "env_effect.h"
#include "sound.h"

#include "nel/misc/file.h"
using namespace NLMISC;

using namespace std;


namespace NLSOUND {


// The audio mixer singleton instance
CAudioMixerUser		*CAudioMixerUser::_Instance = NULL;


/*
 * Create the audio mixer singleton
 */
UAudioMixer	*UAudioMixer::createAudioMixer()
{
	return new CAudioMixerUser();
}


/*
 * Constructor
 */
CAudioMixerUser::CAudioMixerUser() : _SoundDriver(NULL), _NbTracks(0), _CurEnvEffect(NULL)
{
	if ( _Instance == NULL )
	{
		_Instance = this;
	}
	else
	{
		nlerror( "Audio mixer singleton instanciated twice" );
	}
}


/*
 * Destructor
 */
CAudioMixerUser::~CAudioMixerUser()
{
	nldebug( "AM: Releasing..." );

	// Sources and Env. sounds
	// TODO

	// Tracks
	uint i;
	for ( i=0; i!=_NbTracks; i++ )
	{
		delete _Tracks[i];
	}
	
	// Sound driver
	delete _SoundDriver;

	_Instance = NULL;

	nldebug( "AM: Released" );
}


/*
 * Initialization
 */
void				CAudioMixerUser::init()
{
	nldebug( "AM: Init..." );
	
	// Init sound driver
	_SoundDriver = ISoundDriver::createDriver();
	CSound::init( _SoundDriver );

	// Init listener
	_Listener.init( _SoundDriver );
	
	// Init tracks (physical sources)
	_NbTracks = MAX_TRACKS; // could be chosen by the user, or according to the capabilities of the sound card
	uint i;
	for ( i=0; i!=_NbTracks; i++ )
	{
		_Tracks[i] = new CTrack();
		_Tracks[i]->DrvSource = _SoundDriver->createSource();
		_Tracks[i]->setAvailable( true );
#ifdef NL_DEBUG
		_Tracks[i]->UserSource = NULL;
#endif
	}
	for ( i=_NbTracks+1; i<MAX_TRACKS; i++ )
	{
		_Tracks[i] = NULL;
	}

	nldebug( "AM: Initialized" );
}


/*
 * Set the appropriate environmental effect
 */
void				CAudioMixerUser::computeEnvEffect( const CVector& listenerpos, bool force )
{
	// Find the first matching, linear search
	vector<CEnvEffect*>::iterator ipe;
	for ( ipe=_EnvEffects.begin(); ipe!=_EnvEffects.end(); ++ipe )
	{
		if ( (*ipe)->include( listenerpos ) )
		{
			// Set the effect only if it has changed
			if ( (_CurEnvEffect != *ipe) || force )
			{
				_CurEnvEffect = *ipe;
				_Listener.getListener()->setEnvironment( _CurEnvEffect->getEnvNum(), _CurEnvEffect->getEnvSize() );
				nldebug( "AM: Listener environmental effect changed to %u", _CurEnvEffect->getEnvNum() );
			}
			return;
		}
	}

	// If not found, set the default (only if it wasn't the default before)
	if ( _CurEnvEffect != NULL )
	{
		_Listener.getListener()->setEnvironment( ENVFX_DEFAULT_NUM );
		_CurEnvEffect = NULL;
		nldebug( "AM: Listener environmental effect reset" );
	}
}


/*
 * Put source into a track
 */
bool				CAudioMixerUser::giveTrack( CSourceUser *source )
{
	if ( source->isPlaying() )
	{
		source->stop();
	}
	CTrack *track;
	getFreeTracks( 1, &track );
	source->enterTrack( track );
	//nldebug( "Source %p entered track %p", source, track );
	return (track != NULL);
}


/*
 * Release track
 */
void				CAudioMixerUser::releaseTrack( CSourceUser *source )
{
	if ( source->isPlaying() )
	{
		source->stop();
	}
	source->getTrack()->setAvailable( true );
#ifdef NL_DEBUG
	source->getTrack()->UserSource = source;
#endif
	//nldebug( "Source %p left track %p", source, source->getTrack() );
	source->leaveTrack();
}

 
/*
 * Take a listener's move into account
 */
void				CAudioMixerUser::applyListenerMove( const NLMISC::CVector& listenerpos )
{
	// Environmental effect
	computeEnvEffect( listenerpos );

	// Environment sounds
	vector<CEnvSoundUser*>::iterator ipe;
	for ( ipe=_EnvSounds.begin(); ipe!=_EnvSounds.end(); ++ipe )
	{
		(*ipe)->recompute();
	}
}


/*
 * Update audio mixer (call evenly)
 */
void				CAudioMixerUser::update()
{
	// Update envsounds
	vector<CEnvSoundUser*>::iterator ipe;
	for ( ipe=_EnvSounds.begin(); ipe!=_EnvSounds.end(); ++ipe )
	{
		(*ipe)->update();
	}

	// Debug info
	/*uint32 i;
	nldebug( "List of the %u tracks", _NbTracks );
	for ( i=0; i!=_NbTracks; i++ )
	{
#ifdef NL_DEBUG
		CSourceUser *su;
		if ( su = _Tracks[i]->UserSource )
		{
			nldebug( "%u: %p %s %s %s, vol %u",
				    i, &_Tracks[i]->DrvSource, _Tracks[i]->available()?"FREE":"USED",
					_Tracks[i]->available()?"":(su->isPlaying()?"PLAYING":"STOPPED"),
					_Tracks[i]->available()?"":(su->getSound()?su->getSound()->getFilename().c_str():""),
					(uint)(su->getGain()*100.0f) );
		}
#else
			nldebug( "%u: %p %s", i, &_Tracks[i]->DrvSource, _Tracks[i]->available()?"FREE":"USED" );
#endif
	}*/
}


/*
 * Add sound source
 */
USource				*CAudioMixerUser::createSource( TSoundId id )
{
	// Create source
	CSourceUser *source = new CSourceUser( id );
	_Sources.push_back( source );

	// Give it a free track
	if ( source->getSound() != NULL )
	{
		CTrack *track;
		getFreeTracks( 1, &track );
		source->enterTrack( track );
	}
	nldebug( "AM: Source created" ); 
	return source;
}


/*
 * Add environment sound
 */
/*UEnvSound			*CAudioMixerUser::createEnvSound( TSoundId id3d, TSoundId idstereo )
{
	// Create env. sound
	CEnvSoundUser *envsound = new CEnvSoundUser( id3d, idstereo );
	_EnvSounds.push_back( envsound );

	// Give it 2 free tracks
	CTrack *tracks [2];
	getFreeTracks( 2, tracks );
	if ( static_cast<CSourceUser*>(envsound->get3DSource())->getSound() != NULL )
	{
		static_cast<CSourceUser*>(envsound->get3DSource())->enterTrack( tracks[0] );
	}
	if ( static_cast<CSourceUser*>(envsound->getStereoSource())->getSound() != NULL )
	{
		static_cast<CSourceUser*>(envsound->getStereoSource())->enterTrack( tracks[1] );
	}
	nldebug( "AM: EnvSound created" ); 
	return envsound;
}*/


/*
 * Remove sound source
 */
void				CAudioMixerUser::removeSource( USource *source )
{
	nlassert( source != NULL );
	vector<CSourceUser*>::iterator ips = find( _Sources.begin(), _Sources.end(), source );
	if ( ips == _Sources.end() )
	{
		nlwarning( "AM: Cannot remove source: not found" );
	}
	else
	{
		_Sources.erase( ips );
		nldebug( "AM: Source removed" );
	}
}


/*
 * Remove environment sound
 */
/*void				CAudioMixerUser::removeEnvSound( UEnvSound *envsound )
{
	nlassert( envsound != NULL );
	vector<CEnvSoundUser*>::iterator ipe = find( _EnvSounds.begin(), _EnvSounds.end(), envsound );
	if ( ipe == _EnvSounds.end() )
	{
		nlwarning( "AM: Cannot remove EnvSound: not found" );
	}
	else
	{
		_EnvSounds.erase( ipe );
		nldebug( "AM: EnvSound removed" );
	}
}*/


/*
 * Returns nb available tracks (or NULL)
 */
void				CAudioMixerUser::getFreeTracks( uint nb, CTrack **tracks )
{
	nlassert( nb <= _NbTracks );
	
	uint found=0, i;
	for ( found=0, i=0; (found!=nb) && (i!=_NbTracks); i++ )
	{
		if ( _Tracks[i]->available() )
		{
			tracks[found] = _Tracks[i];
			_Tracks[i]->setAvailable( false );
			found++;
			//nlinfo( "AM: Acquiring track %p (number %u)", _Tracks[i], i );
		}
	}
	for ( i=found; i!=nb; i++ )
	{
		tracks[i] = NULL;
	}
	nldebug( "AM: Requested %u tracks, obtained %u", nb, found );
}


/*
 * Choose the environmental effect(s) corresponding to tag
 */
void				CAudioMixerUser::selectEnvEffects( const std::string& tag )
{
	// Select Env
	vector<CEnvEffect*>::iterator ipe;
	for ( ipe=_EnvEffects.begin(); ipe!=_EnvEffects.end(); ++ipe )
	{
		(*ipe)->selectEnv( tag );
	}

	// Compute
	CVector pos;
	_Listener.getPosition( pos );
	computeEnvEffect( pos, true );
}


/*
 * Load environment effects
 */
void				CAudioMixerUser::loadEnvEffects( const char *filename )
{
	nlassert( filename != NULL );
	nldebug( "AM: Loading environmental effects..." );

	// Unload previous env effects
	vector<CEnvEffect*>::iterator ipe;
	for ( ipe=_EnvEffects.begin(); ipe!=_EnvEffects.end(); ++ipe )
	{
		delete (*ipe);
	}
	_EnvEffects.clear();

	// Load env effects
	CIFile file;
	if ( file.open( filename ) )
	{
		uint32 n = CEnvEffect::load( _EnvEffects, file );
		nldebug( "AM: Loaded %u environmental effects", n );
	}
	else
	{
		nlwarning( "AM: Environmental effects file not found" );
	}
}


/*
 * Load buffers
 */
void			CAudioMixerUser::loadSoundBuffers( const char *filename, const vector<TSoundId> **idvec )
{
	nlassert( filename != NULL );
	nldebug( "AM: Loading sound buffers..." );


	CIFile file;
	if ( file.open( filename ) )
	{
		uint32 n = CSound::load( _Sounds, file );
		nldebug( "AM: Loaded %u sound buffers", n );
	}
	else
	{
		nlwarning( "AM: Sound description file not found: %s", filename );
	}
	nlassert( idvec != NULL );
	*idvec = &_Sounds;
}


/*
 * Load environment sounds
 */
void			CAudioMixerUser::loadEnvSounds( const char *filename, const std::vector<UEnvSound*> **esvec )
{
	nlassert( filename != NULL );
	nldebug( "AM: Loading environment sounds..." );

	CIFile file;
	if ( file.open( filename ) )
	{
		uint32 n = CEnvSoundUser::load( _EnvSounds, file, &_Listener );
		nldebug( "AM: Loaded %u environment sounds", n );
	}
	else
	{
		nlwarning( "AM: Environment sounds file not found: %s", filename );
	}
	if ( esvec != NULL )
	{
		*esvec = (const std::vector<UEnvSound*>*)&_EnvSounds;
	}
}


} // NLSOUND
