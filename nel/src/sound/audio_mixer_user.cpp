/** \file audio_mixer_user.cpp
 * CAudioMixerUser: implementation of UAudioMixer
 *
 * $Id: audio_mixer_user.cpp,v 1.33 2002/11/04 15:40:43 boucher Exp $
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

#include "stdsound.h"

//#include "env_sound_user.h"
//#include "env_effect.h"
#include "simple_sound.h"
#include "complex_sound.h"
//#include "ambiant_source.h"
//#include "bounding_sphere.h"
//#include "bounding_box.h"
#include "driver/buffer.h"
#include "sample_bank.h"
#include "sound_bank.h"
#include "background_sound_manager.h"
#include "source_user.h"
#include "complex_source.h"
#include "background_source.h"


#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"

//#include <crtdbg.h>

using namespace NLMISC;

using namespace std;


namespace NLSOUND {

#define NL_TRACE_MIXER 0

#if NL_TRACE_MIXER
#define _profile(_a) nldebug ## _a
#else
#define _profile(_a) 
#endif

// The audio mixer singleton instance
CAudioMixerUser		*CAudioMixerUser::_Instance = NULL;

// Return the priority cstring (debug info)
const char *PriToCStr [NbSoundPriorities] = { "XH", "HI", "MD", "LO" };


// ******************************************************************

const char *getPriorityStr( TSoundPriority p )
{
	nlassert( ((uint)p) < NbSoundPriorities );
	return PriToCStr[p];
}


// ******************************************************************

UAudioMixer	*UAudioMixer::createAudioMixer()
{
	return new CAudioMixerUser();
}


// ******************************************************************

CAudioMixerUser::CAudioMixerUser() : _SoundDriver(NULL),
									 _ListenPosition(CVector::Null),
//									 _EnvSounds(NULL),
									 _BalancePeriod(0),
									 _CurEnvEffect(NULL),
									 _NbTracks(0),
									 _MaxNbTracks(0),
									 _Leaving(false)
{
	if ( _Instance == NULL )
	{
		_Instance = this;

#if NL_PROFILE_MIXER
		_UpdateTime = 0.0;
		_CreateTime = 0.0;
		_UpdateCount = 0;
		_CreateCount = 0;
#endif

	}
	else
	{
		nlerror( "Audio mixer singleton instanciated twice" );
	}
}


// ******************************************************************

CAudioMixerUser::~CAudioMixerUser()
{
	nldebug( "AM: Releasing..." );

	delete _BackgroundSoundManager;
//	CBackgroundSoundManager::release();

	reset();

	_Leaving = true;

/*	// EnvEffects
	vector<CEnvEffect*>::iterator ipee;
	for ( ipee=_EnvEffects.begin(); ipee!=_EnvEffects.end(); ++ipee )
	{
		delete (*ipee);
	}
*/
	// Sounds allocated by ambiant sources
/*	set<CSound*>::iterator ipss;
	for ( ipss=_AmbSounds.begin(); ipss!=_AmbSounds.end(); ++ipss )
	{
		delete (*ipss);
	}
*/
	// Release all SoundBanks
	CSoundBank::releaseAll();
	// Release all the SampleBanks
	CSampleBank::releaseAll();

	// Tracks
	uint i;
	for ( i=0; i!=_NbTracks; i++ )
	{
		if ( _Tracks[i] )
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

// ******************************************************************

void				CAudioMixerUser::writeProfile(std::ostream& out)
{
	// compute number of muted source
	uint nb = 0;	
	
/*	TSourceContainer::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		CSourceUser *psu = *first;
		if (psu->getTrack() == NULL)
		{
			++nb;
		}
	}
*/
/*	hash_set<CSourceUser*>::const_iterator ips;
	for ( ips=_Sources.begin(); ips!=_Sources.end(); ++ips )
	{
		CSourceUser *psu = *ips;
		if (psu->getTrack() == NULL)
		{
			++nb;
		}
	}
*/
	out << "Mixer: \n";
	out << "Playing sources: " << getPlayingSourcesNumber() << " \n";
	out << "Available tracks: " << getNumberAvailableTracks() << " \n";
//	out << "Muted sources: " << nb << " \n";
	out << "Muted sources: " << max(0, sint(getPlayingSourcesNumber())-sint(_NbTracks)) << " \n";
	out << "Average update time: " << (1000.0 * _UpdateTime / _UpdateCount) << " msec\n";
	out << "Average create time: " << (1000.0 * _CreateTime / _CreateCount) << " msec\n";
	out << "Estimated CPU: " << (100.0 * 1000.0 * (_UpdateTime + _CreateTime) / curTime()) << "%\n";

	if (_SoundDriver)
	{
		out << "\n";
		out << "Driver: \n";
		_SoundDriver->writeProfile(out);
	}
}

// ******************************************************************
void			CAudioMixerUser::setPlaying(CSourceUser *source)
{
	_PlayingSources.insert(source);
	// try to give it a track...
	if (source->getTrack() == 0)
	{
		giveTrack(source);
	}
}
// ******************************************************************
void			CAudioMixerUser::unsetPlaying(CSourceUser *source)
{
	_PlayingSources.erase(source);
}


// ******************************************************************

void				CAudioMixerUser::reset()
{
	_Leaving = true;

	// Stop tracks
	uint i;
	for ( i=0; i!=_NbTracks; i++ )
		if ( _Tracks[i] )
		{
			CSourceUser* src = _Tracks[i]->getUserSource();

			if ( ! _Tracks[i]->isAvailable() && src )
			{
				src->stop();
			}
			_Tracks[i]->DrvSource->setStaticBuffer( NULL );
		}

	// Env. sounds tree
/*	if ( _EnvSounds != NULL )
	{
		//_EnvSounds->stop( true );
		delete _EnvSounds;
		_EnvSounds = NULL;
	}
*/
	// Sources
	while (!_Sources.empty())
	{
		removeSource( _Sources.begin(), true ); // 3D sources, the envsounds were removed above
	}
/*
	hash_set<CSourceUser*>::iterator ipsrc, ipOld;
	for ( ipsrc=_Sources.begin(); ipsrc!=_Sources.end(); )
	{
		ipOld = ipsrc;
		++ipsrc;
		// Erasing an element from a set also does not invalidate any other iterators
		removeSource(( ipOld, true ); // 3D sources, the envsounds were removed above
	}
*/
	_Leaving = false;
}

// ******************************************************************

void				CAudioMixerUser::init( uint32 balance_period )
{
	nldebug( "AM: Init..." );

	_profile(( "AM: ---------------------------------------------------------------" ));
	_profile(( "AM: DRIVER: %s", NLSOUND_DLL_NAME ));
	
	// Init sound driver
	try
	{
		_SoundDriver = ISoundDriver::createDriver();
	}
	catch(...)
	{
		delete this;
		_Instance = NULL;
		throw;
	}

	// Init registrable classes
	static bool initialized = false;
	if (!initialized)
	{
		CSourceUser::init();
//		CAmbiantSource::init();
//		CBoundingSphere::init();
//		CBoundingBox::init();
//		CBackgroundSoundManager::init (this);
		initialized = true;
	}

	// Init listener
	_Listener.init( _SoundDriver );

	// Init tracks (physical sources)
	_NbTracks = MAX_TRACKS; // could be chosen by the user, or according to the capabilities of the sound card
	uint i;
	for ( i=0; i<MAX_TRACKS; i++ )
	{
		_Tracks[i] = NULL;
	}
	try
	{
		for ( i=0; i!=_NbTracks; i++ )
		{
			_Tracks[i] = new CTrack();
			_Tracks[i]->init( _SoundDriver );
		}
	}
	catch ( ESoundDriver & )
	{
		// If the source generation failed, keep only the generated number of sources
		_NbTracks = i;
		//delete _Tracks[i]; // Bug: the desctructor would not work because the source's name is invalid
	}

	_MaxNbTracks = _NbTracks;
	_BalancePeriod = balance_period;
	_StartTime = CTime::getLocalTime();

	// Create the background sound manager.
	_BackgroundSoundManager = new CBackgroundSoundManager();

	nlinfo( "Initialized audio mixer with %u voices", _NbTracks );
}


// ******************************************************************
void	CAudioMixerUser::bufferUnloaded(IBuffer *buffer)
{
	// check all track to find a track playing this buffer.
	uint i;
	for ( i=0; i!=_NbTracks; ++i )
	{
		CTrack	*track = _Tracks[i];
		if ( track && track->getUserSource())
		{
			if (track->getUserSource()->getBuffer() == buffer)
			{
				track->getUserSource()->stop();
				track->getUserSource()->leaveTrack();
//				nlassert(_PlayingSources.find(track->getUserSource()) != _PlayingSources.end());
				_PlayingSources.erase(track->getUserSource());
				track->DrvSource->stop();
				track->DrvSource->setStaticBuffer( NULL );
/*				nldebug("Stoping track %i playing sample %s (%p)", i, buffer->getName(), buffer);
				track->getUserSource()->leaveTrack();
				// stop the track.
				track->DrvSource->stop();
				// and assign an empty buffer
				track->DrvSource->setStaticBuffer( NULL );
//				break;
*/
			}
		}
	}

}


// ******************************************************************

void				CAudioMixerUser::enable( bool b )
{
	if ( b )
	{
		// Reenable
		_NbTracks = _MaxNbTracks;
	}
	else
	{
		// Disable
		uint i;
		for ( i=0; i!=_NbTracks; i++ )
		{
			if ( _Tracks[i] && ! _Tracks[i]->isAvailable() )
			{
				_Tracks[i]->getUserSource()->leaveTrack();
//				nlassert(_PlayingSources.find(_Tracks[i]->getUserSource()) != _PlayingSources.end());
				_PlayingSources.erase(_Tracks[i]->getUserSource());
			}
		}
		_NbTracks = 0;
	}
}

// ******************************************************************

ISoundDriver*		CAudioMixerUser::getSoundDriver()
{
	return _SoundDriver;
}

// ******************************************************************

/*void				CAudioMixerUser::computeEnvEffect( const CVector& listenerpos, bool force )
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
*/

// ******************************************************************

void				CAudioMixerUser::giveTrack( CSourceUser *source )
{
	nlassert( ! source->isPlaying() );

	CTrack *track;
	getFreeTracks( 1, &track );
	if ( track != NULL )
	{
		// Free tracks remain
		source->enterTrack( track );
//		nlassert(_PlayingSources.find(source) == _PlayingSources.end());
		_PlayingSources.insert(source);
		//nlwarning("AM: give: found free track");
	}
	else
	{
		// All track used
		_profile(("AM: GIVETRACK: CALLING REDISPATCH"));
		redispatchSourcesToTrack();
	}
}


// ******************************************************************

void				CAudioMixerUser::releaseTrack( CSourceUser *source )
{
	bool recomp = ( _NbTracks <= _PlayingSources.size() );

	// Release track
	source->leaveTrack();
//	nlassert(_PlayingSources.find(source) != _PlayingSources.end());
	_PlayingSources.erase(source);

	// Dispatch if needed
	if ( recomp && (! _Leaving) )
	{
		_profile(("AM: RELEASETRACK: CALLING REDISPATCH"));
		redispatchSourcesToTrack();
	}
}


// ******************************************************************

void				CAudioMixerUser::getFreeTracks( uint nb, CTrack **tracks )
{
	uint found=0, i;
	if ( nb <= _NbTracks )
	{
		for ( found=0, i=0; (found!=nb) && (i!=_NbTracks); i++ )
		{
			if ( _Tracks[i] && _Tracks[i]->isAvailable() )
			{
				tracks[found] = _Tracks[i];
				found++;
				//nldebug( "AM: Acquiring track number %u", i );
			}
		}
	}
	for ( i=found; i!=nb; i++ )
	{
		tracks[i] = NULL;
	}
	//nlwarning( "AM: Requested %u tracks, obtained %u", nb, found );
}

 
// ******************************************************************

void				CAudioMixerUser::applyListenerMove( const NLMISC::CVector& listenerpos )
{
	// Store position
	_ListenPosition = listenerpos;

	// Environmental effect
//	computeEnvEffect( listenerpos );

/*	// Environment sounds
	if ( _EnvSounds != NULL )
	{
		_EnvSounds->recompute();
	}
*/
}

// ******************************************************************

void				CAudioMixerUser::reloadSampleBanks(bool async)
{
	CSampleBank::reload(async);
}

// ******************************************************************

void				CAudioMixerUser::update()
{
#if NL_PROFILE_MIXER
	TTicks start = CTime::getPerformanceTime();
#endif

	// update the object.
	{
		// 1st, add registered fill the list
		{
			std::vector<IMixerUpdate*>::iterator first(_UpdateAddList.begin()), last(_UpdateAddList.end());
			for (; first != last; ++first)
			{
				_UpdateList.insert(*first);
			}
			_UpdateAddList.clear();
		}
		// 2nd, remove unregistered from the list
		{
			std::vector<IMixerUpdate*>::iterator first(_UpdateRemoveList.begin()), last(_UpdateRemoveList.end());
			for (; first != last; ++first)
			{
				_UpdateList.erase(*first);
			}
			_UpdateRemoveList.clear();
		}

		// 3rd, do the update
		{
			TMixerUpdateContainer::iterator first(_UpdateList.begin()), last(_UpdateList.end());
			for (; first != last; ++first)
			{
				// call the update method.
				const_cast<IMixerUpdate*>(*first)->onUpdate();
			}
		}
	}
	// send the event.
	{
		TTime now = NLMISC::CTime::getLocalTime();

		while (!_EventList.empty() && _EventList.begin()->first <= now)
		{
			_EventList.begin()->second->onEvent();
			_EventList.erase(_EventList.begin());
		}
	}


//	_BackgroundSoundManager->update();

	vector<CSourceUser*>	toRemove;
	vector<CSourceUser*>	playingToRemove;
	// Manage spawned sources
	TSourceContainer::iterator ips, ipOld;
	for ( ips=_PlayingSources.begin(); ips!=_PlayingSources.end(); )
	{
		ipOld = ips;
		++ips;
		// Check if source is spawned and stopped
		if ((*ipOld)->isStopped() )
		{
			if ((*ipOld)->isSpawn())
			{
				// Remove source (possibly, call callback before)
				TSpawnEndCallback cb = (*ipOld)->getSpawnEndCallback();
				if ( cb != NULL )
				{
					cb( *ipOld, (*ipOld)->getCallbackUserParam());
				}
				_profile(( "AM: [%u]---------------------------------------------------------------", curTime() ));
				_profile(( "AM: UPDATE: REMOVESOURCE" ));
				//removeSource( ipOld, true );
				toRemove.push_back(*ipOld);
			}
			else
			{
				// remove from playing sources.
				playingToRemove.push_back(*ipOld);
			}
		}
	}

	while (!toRemove.empty())
	{
		removeSource(toRemove.back());
		toRemove.pop_back();
	}
	while (!playingToRemove.empty())
	{
		_PlayingSources.erase(playingToRemove.back());
		playingToRemove.pop_back();
	}

/*	// Update envsounds
	if ( _EnvSounds != NULL )
	{
		_EnvSounds->update();
	}
*/
	// Balance sources
	if ( _BalancePeriod != 0 )
	{
		/*static uint32 update_counter = 1;
		if ( update_counter == _BalancePeriod )*/
		static TTime lastupd = CTime::getLocalTime();
		TTime delta = CTime::getLocalTime() - lastupd;

		if ( delta > _BalancePeriod )
		{
			lastupd += delta;
			//update_counter = 1;

			// Auto-Balance
			//balanceSources();
			//if (moreSourcesThanTracks())
			// FIXME: SWAPTEST
			if (getPlayingSourcesNumber() < _NbTracks) //getSourcesNumber())
			{
				_profile(("AM: UPDATE: CALLING REDISPATCH"));
				redispatchSourcesToTrack(); 
			}
		}
		/*else
		{
			update_counter++;
		}*/
/*		// Remove the last not played source.
		{
			TSourceContainer::iterator first(_PlayingSources.begin()), last(_PlayingSources.end());
			vector<USource*> toRemove;
			for (; first != last; ++first)
			{
				CSourceUser* psu = (CSourceUser*)*first;
				if (psu->getTrack() == 0 && psu->isSpawn())
				{
					toRemove.push_back(psu);
				}
			}
			while (!toRemove.empty())
			{
				removeSource(toRemove.back());
				toRemove.pop_back();
			}
		}
*/
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

	_SoundDriver->commit3DChanges();

#if NL_PROFILE_MIXER
	_UpdateTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
	_UpdateCount++;
#endif

/*	// display the track using...
	{
		char tmp[2048] = "";
		string str;

		for (uint i=0; i<_NbTracks/2; ++i)
		{
			sprintf(tmp, "[%2u]%8p ", i, _Tracks[i]->getUserSource());
			str += tmp;
		}
		nldebug((string("Status1: ")+str).c_str());
		str = "";
		for (i=_NbTracks/2; i<_NbTracks; ++i)
		{
			sprintf(tmp, "[%2u]%8p ", i, _Tracks[i]->getUserSource());
			str += tmp;
		}
//		nldebug((string("Status2: ")+str).c_str());
	}
*/
}


// ******************************************************************

TSoundId			CAudioMixerUser::getSoundId( const std::string &name )
{
	return CSoundBank::get(name);
}

// ******************************************************************

void				CAudioMixerUser::addSource( CSourceUser *source )
{ 
	nlassert(_Sources.find(source) == _Sources.end());
	_Sources.insert( source ); 

	_profile(( "AM: ADDSOURCE, SOUND: %d, TRACK: %p, NAME=%s", source->getSound(), source->getTrack(),
			source->getSound() && (source->getSound()->getName()!="") ? source->getSound()->getName().c_str() : "" ));

}


// ******************************************************************

USource				*CAudioMixerUser::createSource( TSoundId id, bool spawn, TSpawnEndCallback cb, void *userParam, CSoundContext *context )
{
#if NL_PROFILE_MIXER
	TTicks start = CTime::getPerformanceTime();
#endif

	_profile(( "AM: [%u]---------------------------------------------------------------", curTime() ));
	_profile(( "AM: CREATESOURCE: SOUND=%p, NAME=%s, TIME=%d", id, id->getName().c_str(), curTime() ));
	_profile(( "AM: SOURCES: %d, PLAYING: %d, TRACKS: %d", getSourcesNumber(), getPlayingSourcesNumber(), getNumberAvailableTracks() ));

	if ( id == NULL )
	{
		_profile(("AM: FAILED CREATESOURCE"));
		nldebug( "AM: Sound not created: invalid sound id" );
		return NULL;
	}

	USource *ret = NULL;

//	CBackgoundSound	*bgSound;

	if (id->getSoundType() == CSound::SOUND_SIMPLE)
	{
		CSimpleSound	*simpleSound = static_cast<CSimpleSound	*>(id);
		// This is a simple sound
		string bn;
		simpleSound->getBuffername(bn, context);
		if (bn.empty() || simpleSound->getBuffer(&bn) == NULL)
		{
			nlwarning ("Can't create the sound '%s'", bn.c_str());
			return NULL;
		}

		// Create source
		CSourceUser *source = new CSourceUser( simpleSound, spawn, cb, userParam, context, bn );
		addSource( source );

		if (source->getBuffer() != 0)
		{
			// Link the position to the listener position if it'a stereo source
			if ( source->getBuffer()->isStereo() )
			{
				source->set3DPositionVector( &_ListenPosition );
			}

			// Give it a free track
//			giveTrack( source );

			// remember buffer=>source assoctiation
	//		_BufferToSources.insert(make_pair(source->getBuffer(), source));
		}
		else
		{
			nlassert(false); // FIXME
		}
		ret = source;
	}
	else if (id->getSoundType() == CSound::SOUND_COMPLEX)
	{
		CComplexSound	*complexSound = static_cast<CComplexSound*>(id);
		// This is a pattern sound.
		ret =  new CComplexSource(complexSound, spawn, cb, userParam, context);
	}
	else if (id->getSoundType() == CSound::SOUND_BACKGROUND)
	{
		// This is a background sound.
		CBackgroundSound	*bgSound = static_cast<CBackgroundSound	*>(id);
		ret = new CBackgroundSource(bgSound, spawn, cb, userParam, context);
	}
	else
	{
		nlassertex(false, ("Unknown sound class !"));
	}

#if NL_PROFILE_MIXER
	_CreateTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
	_CreateCount++;
#endif

	//nldebug( "AM: Source created" ); 
	return ret;						
}


// ******************************************************************

USource				*CAudioMixerUser::createSource( const std::string &name, bool spawn, TSpawnEndCallback cb, void *userParam, CSoundContext *context)
{
	return createSource( getSoundId( name ), spawn, cb, userParam, context );
}


// ******************************************************************

void				CAudioMixerUser::removeSource( USource *source )
{
	nlassert( source != NULL );
	TSourceContainer::const_iterator ips = _Sources.find( static_cast<CSourceUser*>(source) );
	if ( ips == _Sources.end() )
	{
		nlwarning( "AM: Cannot remove source: not found" );
		nlassert(((CSourceUser*)source)->getTrack() == NULL);
		//releaseTrack( source );
		delete source;
	}
	else
	{
		removeSource( ips, true );
	}
}


// ******************************************************************

void				CAudioMixerUser::removeMySource( USource *source )
{
	nlassert( source != NULL );
	TSourceContainer::const_iterator ips = _Sources.find( static_cast<CSourceUser*>(source) );
	if ( ips == _Sources.end() )
	{
		nlwarning( "AM: Cannot remove source: not found" );
		//releaseTrack( source );
	}
	else
	{
		removeSource( ips, false );
	}
}


// ******************************************************************

void				CAudioMixerUser::removeSource( TSourceContainer::iterator ips, bool deleteit )
{
	nlassert( ips != _Sources.end() );

	CSourceUser *src = *ips;

	_profile(( "AM: REMOVESOURCE: SOUND=%d, TRACK=%p, NAME=%s, TIME=%d, PLAYED=%d, DUR=%d", src->getSound(), src->getTrack(),
				src->getSound() && (src->getSound()->getName()!="") ? src->getSound()->getName().c_str() : "" ,
				curTime(), (uint32) src->getPlayTime(), src->getSound()->getDuration()));

	_Sources.erase( ips );
	releaseTrack( src );

	if ( deleteit )
	{
		delete src;
	}
}


// ******************************************************************

void				CAudioMixerUser::selectEnvEffects( const std::string &tag )
{
	nlassertex(false, ("Not implemented yet"));
/*	// Select Env
	vector<CEnvEffect*>::iterator ipe;
	for ( ipe=_EnvEffects.begin(); ipe!=_EnvEffects.end(); ++ipe )
	{
		(*ipe)->selectEnv( tag );
	}

	// Compute
	CVector pos;
	_Listener.getPos( pos );
	computeEnvEffect( pos, true );
*/
}


// ******************************************************************

/*
void				CAudioMixerUser::loadEnvEffects( const char *filename )
{
	nlassert( filename != NULL );
	nlinfo( "Loading environmental effects from %s...", filename );

	// Unload previous env effects
	vector<CEnvEffect*>::iterator ipe;
	for ( ipe=_EnvEffects.begin(); ipe!=_EnvEffects.end(); ++ipe )
	{
		delete (*ipe);
	}
	_EnvEffects.clear();

	string str = CPath::lookup( filename, false );

	// Load env effects
	CIFile file;
	if ( !str.empty() && file.open(str) )
	{
		uint32 n = CEnvEffect::load( _EnvEffects, file );
		nldebug( "AM: Loaded %u environmental effects", n );
	}
	else
	{
		nlwarning( "AM: Environmental effects file not found" );
	}
}
*/

// ******************************************************************

uint32			CAudioMixerUser::loadSampleBank(bool async, const std::string &filename, std::vector<std::string> *notfoundfiles )
{
//	nlassert( filename != NULL );

	string path = _SamplePath;
	path.append("/").append(filename);

	nldebug( "Loading samples from %s...", path.c_str() );

	CSampleBank* bank = CSampleBank::findSampleBank(path);
	if (bank == NULL)
	{
		// create a new sample bank
//_CrtCheckMemory();
		bank = new CSampleBank(path, _SoundDriver);
//_CrtCheckMemory();
	}

	try 
	{
		bank->load(async);
	}
	catch (Exception& e)
	{
		if (notfoundfiles) {
			notfoundfiles->push_back(path);
		}
		string reason = e.what();
		nlwarning( "AM: Failed to load the samples: %s", reason.c_str() );
	}


	return bank->countSamples();
}

bool CAudioMixerUser::unloadSampleBank( const std::string &filename)
{
	string path = _SamplePath;
	path.append("/").append(filename);

	nldebug( "Unloading samples from %s...", path.c_str() );
	CSampleBank *pbank = CSampleBank::findSampleBank(path);

	if (pbank != NULL)
	{
		// ok, the bank exist.
		return pbank->unload();
	}
	else
		return false;

}


// ******************************************************************

void			CAudioMixerUser::loadSoundBank( const std::string &directory )
{
//	nlassert( directory != NULL );

	nlinfo( "Loading sounds from %s...", directory );

	CSoundBank* bank = new CSoundBank(directory);

	try 
	{
		bank->load();
	}
	catch (Exception& e)
	{
		string reason = e.what();
		nlwarning( "AM: Failed to load the samples: %s", reason.c_str() );
	}
}

// ******************************************************************

void			CAudioMixerUser::getSoundNames( std::vector<std::string>& names ) const
{
	CSoundBank::getSoundNames(names);
}


// ******************************************************************

uint			CAudioMixerUser::getPlayingSourcesNumber() const
{
/*	uint nb = 0;	
	TSourceContainer::const_iterator ips;
	for ( ips=_Sources.begin(); ips!=_Sources.end(); ++ips )
	{
		if ( !(*ips)->isStopped() )
		{
			++nb;
		}
	}
	return nb;
*/
	return _PlayingSources.size();
}

// ******************************************************************

uint			CAudioMixerUser::getNumberAvailableTracks() const
{
	uint nb = 0;	
	for ( uint i=0; i!=_NbTracks; i++ )
	{
		if ( _Tracks[i] && _Tracks[i]->isAvailable() )
		{
			++nb;
		}
	}
	return nb;
}


// ******************************************************************

string			CAudioMixerUser::getSourcesStats() const
{
	string s;
	TSourceContainer::iterator ips;
	for ( ips=_Sources.begin(); ips!=_Sources.end(); ++ips )
	{
		if ( (*ips)->isPlaying() )
		{
			char line [80];

			nlassert( (*ips)->getSound() && (*ips)->getSimpleSound()->getBuffer() );
			smprintf( line, 80, "%s: %u%% %s %s",
					  (*ips)->getSound()->getName().c_str(),
					  (uint32)((*ips)->getGain()*100.0f),
					  (*ips)->getBuffer()->isStereo()?"ST":"MO",
					  PriToCStr[(*ips)->getPriority()] );
			s += string(line) + "\n";
		}
	}
	return s;

}

// ******************************************************************
/*
void			CAudioMixerUser::loadEnvSounds( const char *filename, UEnvSound **treeRoot )
{
	nlassert( filename != NULL );
	nlinfo( "Loading environment sounds from %s...", filename );

	string str = CPath::lookup( filename, false );

	CIFile file;
	if ( !str.empty() && file.open( str ) )
	{
		uint32 n = 0; //CEnvSoundUser::load( _EnvSounds, file );
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
*/

// ******************************************************************

struct CompareSources : public binary_function<const CSourceUser*, const CSourceUser*, bool>
{
	// Constructor
	CompareSources( const CVector &pos ) : _Pos(pos) {}

	// Operator()
	bool operator()( const CSourceUser *s1, const CSourceUser *s2 )
	{
		if (s1->getPriority() < s2->getPriority())
		{
			return true;
		}
		else if (s1->getPriority() == s2->getPriority())
		{
			// Equal priority, test distances to the listener
			const CVector &src1pos = s1->getPos();
			const CVector &src2pos = s2->getPos();;
			return ( (src1pos-_Pos).sqrnorm() < (src2pos-_Pos).sqrnorm() );
		}
		else
		{
			return false;
		}
	}

	// Listener pos
	const CVector &_Pos;
};


// ******************************************************************
uint32			CAudioMixerUser::getLoadedSampleSize()
{
	return CSampleBank::getTotalByteSize();
}


// ******************************************************************

void			CAudioMixerUser::redispatchSourcesToTrack()
{
	if ( _NbTracks == 0 )
	{
		return;
	}

	_profile(( "AM: [%u]---------------------------------------------------------------", curTime() ));
	_profile(( "AM: Redispatching sources" ));
	
	const CVector &listenerpos = _Listener.getPos();

	// Get a copy of the sources set (we will modify it)
	TSourceContainer sources_copy = _PlayingSources;
	// FIXME: SWAPTEST
	//nlassert( sources_copy.size() >= _NbTracks );

	// Select the nbtracks "smallest" sources (the ones that have the higher priorities)
	TSourceContainer::iterator ips;
	TSourceContainer selected_sources;
	uint32 i;

	// Select the sources

	// Select the nbtracks "smallest" sources (the ones that have the higher priorities)
	// FIXME: SWAPTEST
	//for ( i=0; i!=_NbTracks; i++ )
	// TODO : optimize : this is a very BAD PERFORMANCE code
	while (!sources_copy.empty() && (selected_sources.size() < _NbTracks))
	{
		ips = min_element( sources_copy.begin(), sources_copy.end(), CompareSources( listenerpos ) );

		if ((*ips)->isPlaying())
		{
			selected_sources.insert( *ips );
		}

		sources_copy.erase( ips );
	}

	// Clear the current tracks where the sources are not selected anymore
	_profile(( "AM: Total sources: %u", _PlayingSources.size() ));
	_profile(( "AM: Selected sources: %u", selected_sources.size() ));
	for ( i=0; i!=_NbTracks; i++ )
	{
		// FIXME: SWAPTEST
		if ( _Tracks[i] && ! _Tracks[i]->isAvailable() )
		{
			// Optimization note: instead of searching the source in selected_sources, we could have
			// set a boolean in the source object and tested it.
			if ( (ips = selected_sources.find( _Tracks[i]->getUserSource() )) == selected_sources.end() )
			{
				// There will be a new source in this track
				_profile(( "AM: TRACK: %p: REPLACED, SOURCE: %p", _Tracks[i], _Tracks[i]->getUserSource() ));
				if (_Tracks[i]->getUserSource() != 0)
				{
					_Tracks[i]->getUserSource()->leaveTrack();
//					nlassert(_PlayingSources.find(_Tracks[i]->getUserSource()) != _PlayingSources.end());
					_PlayingSources.erase(_Tracks[i]->getUserSource());
				}
			}
			else
			{
				// The track will remain unchanged
				selected_sources.erase( ips );
				_profile(( "AM: TRACK: %p: UNCHANGED, SOURCE: %p", _Tracks[i], _Tracks[i]->getUserSource() ));
			}
		}
		else
		{
			_profile(( "AM: TRACK: %p: FREE", _Tracks[i] ));
		}
	}

	if (!selected_sources.empty())
	{
		// Now, only the sources to add into the tracks remain in selected_sources
		CTrack *track [MAX_TRACKS]; // a little bit more than needed (avoiding a "new")
		getFreeTracks( selected_sources.size(), track );

		_profile(( "AM: Remaining sources: %u", selected_sources.size() ));

		for (i=0, ips=selected_sources.begin(); ips!=selected_sources.end(); ++ips )
		{
			// FIXME: SWAPTEST
			(*ips)->enterTrack( track[i] );
//			nlassert(_PlayingSources.find(*ips) == _PlayingSources.end());
			_PlayingSources.insert(*ips);
	//		_profile(( "AM: TRACK: %p: ASSIGNED, SOURCE: %p", track[i], track[i]->getUserSource() ));
			_profile(( "AM: TRACK: %p: ASSIGNED, SOURCE: %p", track[i], *ips ));
			i++;
		}
	}
}

void CAudioMixerUser::setListenerPos (const NLMISC::CVector &pos)
{
	_Listener.setPos(pos);
	_BackgroundSoundManager->setListenerPosition(pos);
}

NLMISC_COMMAND (displaySoundInfo, "Display information about the audio mixer", "")
{
	if(args.size() != 0) return false;

	if (CAudioMixerUser::instance() == NULL)
	{
		log.displayNL ("No audio mixer available");
		return true;
	}

	log.displayNL ("%d tracks, MAX_TRACKS = %d, contains:", CAudioMixerUser::instance()->_NbTracks, MAX_TRACKS);

	for (uint i = 0; i < CAudioMixerUser::instance()->_NbTracks; i++)
	{
		if (CAudioMixerUser::instance()->_Tracks[i] == NULL)
		{
			log.displayNL ("Track %d is NULL", i);
		}
		else
		{
			log.displayNL ("Track %d %s available and %s playing.", i, (CAudioMixerUser::instance()->_Tracks[i]->isAvailable()?"is":"is not"), (CAudioMixerUser::instance()->_Tracks[i]->isPlaying()?"is":"is not"));
			if (CAudioMixerUser::instance()->_Tracks[i]->getUserSource() == NULL)
			{
				log.displayNL ("    CUserSource is NULL");
			}
			else
			{
				const CVector &pos = CAudioMixerUser::instance()->_Tracks[i]->getUserSource()->getPos();
				string bufname;
				if (CAudioMixerUser::instance()->_Tracks[i]->getUserSource()->getBuffer())
					bufname = CAudioMixerUser::instance()->_Tracks[i]->getUserSource()->getBuffer()->getName();
				log.displayNL ("    CUserSource is id %d buffer name '%s' pos %f %f %f", CAudioMixerUser::instance()->_Tracks[i]->getUserSource()->getSound(), bufname.c_str(), pos.x, pos.y, pos.z);
			}
		}
	}

	return true;
}

void CAudioMixerUser::registerBufferAssoc(CSound *sound, IBuffer *buffer)
{
	_BufferToSources[buffer].push_back(sound);
}

void CAudioMixerUser::unregisterBufferAssoc(CSound *sound, IBuffer *buffer)
{
	TBufferToSourceContainer::iterator it(_BufferToSources.find(buffer));
	if (it != _BufferToSources.end())
	{
		std::vector<CSound*>::iterator first(it->second.begin()), last(it->second.end());

		for (; first != last; ++first)
		{
			if (*first == sound)
			{
				it->second.erase(first);
				break;
			}
		}
	}
}


/// Register an object in the update list.
void CAudioMixerUser::registerUpdate(CAudioMixerUser::IMixerUpdate *pmixerUpdate)
{
	_UpdateAddList.push_back(pmixerUpdate);
}
/// Unregister an object from the update list.
void CAudioMixerUser::unregisterUpdate(CAudioMixerUser::IMixerUpdate *pmixerUpdate)
{
	_UpdateRemoveList.push_back(pmixerUpdate);
}

/// Add an event in the future.
void CAudioMixerUser::addEvent( CAudioMixerUser::IMixerEvent *pmixerEvent, const NLMISC::TTime &date)
{
	_EventList.insert(make_pair(date, pmixerEvent));
}

/// Remove any event programmed for this object.
void CAudioMixerUser::removeEvent( CAudioMixerUser::IMixerEvent *pmixerEvent)
{
	// need to walk throw the container
	multimap<NLMISC::TTime, IMixerEvent*>::iterator first, last;

	// TODO : rewrite an optimlized version.

restart:
	for (first = _EventList.begin(), last = _EventList.end();
			first != last; 
			++first
		)
	{
		if (first->second == pmixerEvent)
		{
			_EventList.erase(first);
			goto restart;
		}
	}
}

void CAudioMixerUser::setBackgroundFlags(const TBackgroundFlags &backgroundFlags)
{
	_BackgroundSoundManager->setBackgroundFlags(backgroundFlags);
}

void CAudioMixerUser::loadBackgroundSoundFromRegion (const NLLIGO::CPrimRegion &region)
{
	_BackgroundSoundManager->loadSoundsFromRegion(region);
}

void CAudioMixerUser::loadBackgroundEffectsFromRegion (const NLLIGO::CPrimRegion &region)
{
	_BackgroundSoundManager->loadEffecsFromRegion(region);
}
void CAudioMixerUser::loadBackgroundSamplesFromRegion (const NLLIGO::CPrimRegion &region)
{
	_BackgroundSoundManager->loadSamplesFromRegion(region);
}


void CAudioMixerUser::playBackgroundSound () 
{ 
	_BackgroundSoundManager->play (); 
}

void CAudioMixerUser::stopBackgroundSound () 
{ 
	_BackgroundSoundManager->stop (); 
}

void CAudioMixerUser::loadBackgroundSound (const std::string &continent) 
{ 
	_BackgroundSoundManager->load (continent); 
}

} // NLSOUND
