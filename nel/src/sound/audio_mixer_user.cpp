/** \file audio_mixer_user.cpp
 * CAudioMixerUser: implementation of UAudioMixer
 *
 * $Id: audio_mixer_user.cpp,v 1.5 2001/07/18 17:14:35 cado Exp $
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
#include "ambiant_source.h"
#include "bounding_sphere.h"
#include "bounding_box.h"
#include "driver/buffer.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"
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
CAudioMixerUser::CAudioMixerUser() : _SoundDriver(NULL), _NbTracks(0), _CurEnvEffect(NULL),
	_BalancePeriod(0), _ListenPosition(CVector::Null), _Leaving(false)
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
	_Leaving = true;

	// Stop tracks
	uint i;
	for ( i=0; i!=_NbTracks; i++ )
	{
		if ( ! _Tracks[i]->isAvailable() )
		{
			_Tracks[i]->getUserSource()->stop();
		}
		_Tracks[i]->DrvSource->setStaticBuffer( NULL );
	}

	// Env. sounds tree
	_EnvSounds->stop();
	delete _EnvSounds;

	// Remaining sources (should have been removed and deleted by the user !)
	set<CSourceUser*>::iterator ips;
	for ( ips=_Sources.begin(); ips!=_Sources.end(); ++ips )
	{
		delete (*ips); // 3D sources, the envsounds were removed above
	}

	// EnvEffects
	vector<CEnvEffect*>::iterator ipee;
	for ( ipee=_EnvEffects.begin(); ipee!=_EnvEffects.end(); ++ipee )
	{
		delete (*ipee);
	}

	// Sounds
	vector<CSound*>::iterator ipsnds;
	for ( ipsnds=_Sounds.begin(); ipsnds!=_Sounds.end(); ++ipsnds )
	{
		delete (*ipsnds);
	}

	// Tracks
	for ( i=0; i!=_NbTracks; i++ )
	{
		delete _Tracks[i];
	}

	// Sound driver
	if ( _SoundDriver != NULL )
	{
		delete _SoundDriver;
	}

	_Instance = NULL;

	nldebug( "AM: Released" );
}


/*
 * Initialization
 */
void				CAudioMixerUser::init( uint32 balance_period )
{
	nldebug( "AM: Init..." );
	
	// Init sound driver
	_SoundDriver = ISoundDriver::createDriver();
	CSound::init( _SoundDriver );

	// Init registrable classes
	CSourceUser::init();
	CAmbiantSource::init();
	CBoundingSphere::init();
	CBoundingBox::init();

	// Init listener
	_Listener.init( _SoundDriver );

	// Init tracks (physical sources)
	_NbTracks = MAX_TRACKS; // could be chosen by the user, or according to the capabilities of the sound card
	uint i;
	for ( i=0; i!=_NbTracks; i++ )
	{
		_Tracks[i] = new CTrack();
		_Tracks[i]->init( _SoundDriver );
	}
	for ( i=_NbTracks+1; i<MAX_TRACKS; i++ )
	{
		_Tracks[i] = NULL;
	}

	_BalancePeriod = balance_period;

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
void				CAudioMixerUser::giveTrack( CSourceUser *source )
{
	nlassert( ! source->isPlaying() );
	/*if ( source->isPlaying() )
	{
		source->stop();
	}*/
	CTrack *track;
	getFreeTracks( 1, &track );
	if ( track != NULL )
	{
		// Free tracks remain
		source->enterTrack( track );
	}
	else
	{
		// All track used
		redispatchSourcesToTrack();
	}
}


/*
 * Release track
 */
void				CAudioMixerUser::releaseTrack( CSourceUser *source )
{
	bool recomp = ( _NbTracks <= _Sources.size() );

	/*if ( source->isPlaying() )
	{
		source->stop();
	}*/

	// Release track
	source->leaveTrack();

	// Dispatch if needed
	if ( recomp && (! _Leaving) )
	{
		redispatchSourcesToTrack();
	}
}


/*
 * Returns nb available tracks (or NULL)
 */
void				CAudioMixerUser::getFreeTracks( uint nb, CTrack **tracks )
{
	nlassert( nb <= _NbTracks );
	
	uint found=0, i;
	for ( found=0, i=0; (found!=nb) && (i!=_NbTracks); i++ )
	{
		if ( _Tracks[i]->isAvailable() )
		{
			tracks[found] = _Tracks[i];
			found++;
			//nldebug( "AM: Acquiring track number %u", i );
		}
	}
	for ( i=found; i!=nb; i++ )
	{
		tracks[i] = NULL;
	}
	nldebug( "AM: Requested %u tracks, obtained %u", nb, found );
}

 
/*
 * Take a listener's move into account
 */
void				CAudioMixerUser::applyListenerMove( const NLMISC::CVector& listenerpos )
{
	// Store position
	_ListenPosition = listenerpos;

	// Environmental effect
	computeEnvEffect( listenerpos );

	// Environment sounds
	_EnvSounds->recompute();
}


//const char *PriToCStr [3] = { "Hi", "Md ", "Lo" };


/*
 * Update audio mixer (call evenly)
 */
void				CAudioMixerUser::update()
{
	// Update envsounds
	_EnvSounds->update();

	// Balance sources
	if ( _BalancePeriod != 0 )
	{
		static uint32 update_counter = 1;
		if ( update_counter == _BalancePeriod )
		{
			update_counter = 1;

			// If source not a looping source,
			// check source playing state to false if the playing stopped on its own
			uint i;
			for ( i=0; i!=_NbTracks; i++ )
			{
				if ( (_Tracks[i]!=NULL) && (!_Tracks[i]->isPlaying()) )
				{
					// Synchronize user source (assumes the source is either playing or stopped)
					if ( (_Tracks[i]->getUserSource() != NULL) && ( _Tracks[i]->getUserSource()->isPlaying() ) )
					{
						_Tracks[i]->getUserSource()->stop();
					}
				}
			}

			// Auto-Balance
			balanceSources();
		}
		else
		{
			update_counter++;
		}
	}

	// Debug info
	/*uint32 i;
	nldebug( "List of the %u tracks", _NbTracks );
	for ( i=0; i!=_NbTracks; i++ )
	{
		CSourceUser *su;
		if ( su = _Tracks[i]->getUserSource() )
		{
			nldebug( "%u: %p %s %s %s %s, vol %u",
				    i, &_Tracks[i]->DrvSource, _Tracks[i]->isAvailable()?"FREE":"USED",
					_Tracks[i]->isAvailable()?"":(su->isPlaying()?"PLAYING":"STOPPED"),
					_Tracks[i]->isAvailable()?"":PriToCStr[su->getPriority()],
					_Tracks[i]->isAvailable()?"":(su->getSound()?su->getSound()->getFilename().c_str():""),
					(uint)(su->getGain()*100.0f) );
		}
	}*/
}


/*
 * Add sound source
 */
USource				*CAudioMixerUser::createSource( TSoundId id )
{
	// Create source
	CSourceUser *source = new CSourceUser( id );
	_Sources.insert( source );

	// Link the position to the listener position if it'a stereo source
	if ( id->getBuffer()->isStereo() )
	{
		source->set3DPositionVector( &_ListenPosition );
	}

	// Give it a free track
	if ( source->getSound() != NULL )
	{
		giveTrack( source );
	}
	nldebug( "AM: Source created" ); 
	return source;
}


/*
 * Remove sound source
 */
void				CAudioMixerUser::removeSource( USource *source )
{
	nlassert( source != NULL );
	set<CSourceUser*>::const_iterator ips = _Sources.find( static_cast<CSourceUser*>(source) );
	if ( ips == _Sources.end() )
	{
		nlwarning( "AM: Cannot remove source: not found" );
	}
	else
	{
		CSourceUser *src = *ips;
		_Sources.erase( ips );
		releaseTrack( src );
		nldebug( "AM: Source removed" );
	}
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
	_Listener.getPos( pos );
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
	if ( file.open( NLMISC::CPath::lookup( filename ) ) )
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
	if ( file.open( NLMISC::CPath::lookup( filename ) ) )
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
void			CAudioMixerUser::loadEnvSounds( const char *filename, UEnvSound **treeRoot )
{
	nlassert( filename != NULL );
	nldebug( "AM: Loading environment sounds..." );

	CIFile file;
	if ( file.open( NLMISC::CPath::lookup( filename ) ) )
	{
		uint32 n = CEnvSoundUser::load( _EnvSounds, file );
		nldebug( "AM: Loaded %u environment sounds", n );
	}
	else
	{
		nlwarning( "AM: Environment sounds file not found: %s", filename );
	}
	if ( treeRoot != NULL )
	{
		*treeRoot = _EnvSounds;
	}
}


/*
 * Functor that compares sources, including priority and distance to the listener
 */
struct CompareSources : public binary_function<CSourceUser*,CSourceUser*,bool>
{
	// Constructor
	CompareSources( CVector *pos ) : _Pos(pos) {}

	// Operator()
	bool operator()( CSourceUser *s1, CSourceUser *s2 )
	{
		if (s1->getPriority() < s2->getPriority())
		{
			return true;
		}
		else if (s1->getPriority() == s2->getPriority())
		{
			// Equal priority, test distances to the listener
			CVector src1pos, src2pos;
			s1->getPos( src1pos );
			s2->getPos( src2pos );
			return ( (src1pos-(*_Pos)).norm() < (src2pos-(*_Pos)).norm() );
		}
		else
		{
			return false;
		}
	}

	// Listener pos
	CVector *_Pos;
};


/*
 *
 */
void			CAudioMixerUser::redispatchSourcesToTrack()
{
	nldebug( "AM: Redispatching sources" );
	
	CVector listenerpos;
	_Listener.getPos( listenerpos );

	// Get a copy of the sources set (we will modify it)
	set<CSourceUser*> sources_copy = _Sources;
	nlassert( sources_copy.size() >= _NbTracks );

	// Select the nbtracks "smallest" sources (the ones that have the higher priorities)
	set<CSourceUser*> selected_sources;
	set<CSourceUser*>::iterator ips;
	uint32 i;
	for ( i=0; i!=_NbTracks; i++ )
	{
		ips = min_element( sources_copy.begin(), sources_copy.end(), CompareSources( &listenerpos ) );
		selected_sources.insert( *ips );
		sources_copy.erase( ips );
	}

	// Clear the current tracks where the sources are not selected anymore
	//nldebug( "Selected sources: %u", selected_sources.size() );
	for ( i=0; i!=_NbTracks; i++ )
	{
		if ( ! _Tracks[i]->isAvailable() )
		{
			// Optimization note: instead of searching the source in selected_sources, we could have
			// set a boolean in the source object and tested it.
			if ( (ips = selected_sources.find( _Tracks[i]->getUserSource() )) == selected_sources.end() )
			{
				// There will be a new source in this track
				_Tracks[i]->getUserSource()->leaveTrack();
				//nldebug( "%u: -> REPLACED", i );
			}
			else
			{
				// The track will remain unchanged
				selected_sources.erase( ips );
				//nldebug( "%u: UNCHANGED", i );
			}
		}
		else
		{
			//nldebug( "%u: FREE", i );
		}
	}

	// Now, only the sources to add into the tracks remain in selected_sources
	//nldebug( "Remaining selected sources: %u", selected_sources.size() );
	CTrack *track [MAX_TRACKS]; // a little bit more than needed (avoiding a "new")
	getFreeTracks( selected_sources.size(), track );
	i=0;
	for ( ips=selected_sources.begin(); ips!=selected_sources.end(); ++ips )
	{
		nlassert( track[i] != NULL );
		(*ips)->enterTrack( track[i] );
		i++;
	}
}


} // NLSOUND
