/** \file env_sound_user.cpp
 * CEnvSoundUser: implementation of UEnvSound
 *
 * $Id: env_sound_user.cpp,v 1.1 2001/07/10 16:48:03 cado Exp $
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

#include "env_sound_user.h"
#include "sound.h"
#include "audio_mixer_user.h"
#include <stdlib.h>

using namespace std;
using namespace NLMISC;


namespace NLSOUND {


// If this flag is defined, one ambiant sound cannot be the next one to itself
#define ENVSOUND_DONT_DUPLICATE_AMBIANT


/*
 * Constructor
 */
CEnvSoundUser::CEnvSoundUser() : _InnerRadius(0.0f), _OuterRadius(0.0f), _Play(false),
	_RandomSoundChosen(false), _Sustain(false), _StereoGain(0.0f), _Listener(NULL),
	_CrossfadeTime(4000), _SustainTime(8000), _SparseAvgPeriod(20000), _NextSparseSoundTime(0)
{
	_CenterSource.setLooping( true );
	_StereoChannels[AMBIANT_CH1].setLooping( true );
	_StereoChannels[AMBIANT_CH2].setLooping( true );
	//_StereoChannels[SPARSE_CH].setLooping( false );

	srand( (uint32)CTime::getLocalTime() );
}


/*
 * Destructor
 */
CEnvSoundUser::~CEnvSoundUser()
{
	// TODO

	// Delete 3d sound
}


/*
 * Serialize
 */
void CEnvSoundUser::serial( NLMISC::IStream& s )
{
	// If you change this, increment the version number in load()

	// 3D position and sound
	CVector pos;
	CSound *sound;
	if ( ! s.isReading() )
	{
		_CenterSource.getPosition( pos );
		sound = _CenterSource.getSound();
	}
	s.serial( pos );
	s.serialPtr( sound );
	if ( s.isReading() )
	{
		_CenterSource.setPosition( pos );
		_CenterSource.setSound( sound );
	}

	// Area
	s.serial( _InnerRadius );
	s.serial( _OuterRadius );

	// Constants
	s.serial( _CrossfadeTime );
	s.serial( _SustainTime );
	s.serial( _SparseAvgPeriod );

	// Stereo sound banks
	s.serialContPtr( _AmbiantSounds );
	s.serialContPtr( _SparseSounds );

	// TEMP while update() is not coded (TO REMOVE)
	if ( s.isReading() )
	{
		if ( _AmbiantSounds.size() >= 2 )
		{
			_StereoChannels[AMBIANT_CH1].setSound( getRandomSound( _AmbiantSounds ) );
			_StereoChannels[AMBIANT_CH1].setGain( 0.0f );
			_StereoChannels[AMBIANT_CH2].setSound( getRandomSound( _AmbiantSounds ) );
			_StereoChannels[AMBIANT_CH2].setGain( 0.0f );
		}
		/*if ( ! _SparseSounds.empty() )
		{
			_StereoChannels[SPARSE_CH].setGain( 1.0f );
		}*/
	}
}


/*
 * Serialize file header
 */
void	CEnvSoundUser::serialFileHeader( NLMISC::IStream& s, uint32& nb )
{
	s.serialCheck( (uint32)'SEN' ); // NeL Environment Sounds
	s.serialVersion( 0 );
	s.serial( nb );
}


/*
 * Load several envsounds and return the number of envsounds loaded
 */
uint32 CEnvSoundUser::load( std::vector<CEnvSoundUser*>& container, NLMISC::IStream& s, CListenerUser *listener )
{
	if ( s.isReading() )
	{
		uint32 nb, i, notfound=0;
		serialFileHeader( s, nb );
		for ( i=0; i!=nb; i++ )
		{
			CEnvSoundUser *envsound = new CEnvSoundUser();
			s.serial( *envsound );
			envsound->setListener( listener );
			container.push_back( envsound );
		}
		return nb;
	}
	else
	{
		nlstop;
		return 0;
	}
}



/*
 * Play or stop the sources
 */
void CEnvSoundUser::recompute()
{
	// Collect all sources to play EnvSounds
	bool centersrc, stereosrc;
	CVector listenerpos;
	nlassert( _Listener != NULL );
	_Listener->getPosition( listenerpos );
	getCurrentSources( listenerpos, centersrc, stereosrc );

	// Calc position in cycle
	bool crossfade;
	uint32 leadchannel;
	calcPosInCycle( crossfade, leadchannel );

	// Play or stop the sources
	manageCenterSource( centersrc );
	manageStereoChannels( stereosrc, crossfade, leadchannel );
}


/*
 * Get the sources to play corresponding to the listener's position, set with the right volume
 */
void CEnvSoundUser::getCurrentSources( const CVector& listenerpos, bool& centersrc, bool& stereosrcs )
{
	// Check if we must play
	if ( ! _Play )
	{
		centersrc = false;
		stereosrcs = false;
		return;
	}

	bool between = true;
	CVector center;
	_CenterSource.getPosition( center );
	float distanceToCenter = (center-listenerpos).norm();

	// Add stereo channels ?
	if ( distanceToCenter <= _OuterRadius )
	{
		stereosrcs = true;
	}
	else
	{
		// If the listener is strictly outside the outer sphere
		stereosrcs = false;
		_CenterSource.setRelativeGain( 1.0f );
		_StereoGain = 0.0f;
		between = false;
	}

	// Add 3D source ?
	if ( distanceToCenter > _InnerRadius )
	{
		centersrc = true;
	}
	else
	{
		// If the listener is inside the inner sphere, or at its border
		centersrc = false;
		_StereoGain = 1.0f;
		between = false;
	}

	// Lower volume, between _InnerRadius and _OutRadius
	if ( between )
	{
		nlassert( _InnerRadius != _OuterRadius ); // not equal because between would be false
		_StereoGain = (_OuterRadius-distanceToCenter) / (_OuterRadius-_InnerRadius);
		_CenterSource.setRelativeGain( 1.0f - _StereoGain );
	}

	// Take changes into account
	//update();
}


/*
 * Calc pos in cycle
 */
TTime CEnvSoundUser::calcPosInCycle( bool& crossfade, uint32& leadchannel )
{
	TTime pos = CTime::getLocalTime();
	pos = pos % ((_CrossfadeTime+_SustainTime)*2);
	if ( pos < _CrossfadeTime+_SustainTime )
	{
		leadchannel = 0;
	}
	else
	{
		leadchannel = 1;
		pos = pos - (_CrossfadeTime+_SustainTime);
	}
	crossfade = ( pos < _CrossfadeTime );
	return pos;
}


/*
 * Update the stereo mix (call evenly)
 */
void CEnvSoundUser::update()
{
	/*nldebug( "AM: EnvSound: Center source %p has sound %p, buffer %p, track %p, buffer %p",
		&_CenterSource, _CenterSource.getSound(), _CenterSource.getSound()?_CenterSource.getSound()->getBuffer():NULL,
		_CenterSource.getTrack(), _CenterSource.getTrack()?_CenterSource.getTrack()->DrvSource->getStaticBuffer():NULL );
	nldebug( "AM: EnvSound: Channel #1    %p has sound %p, buffer %p, track %p, buffer %p",
		&_StereoChannels[0], _StereoChannels[0].getSound(), _StereoChannels[0].getSound()?_StereoChannels[0].getSound()->getBuffer():NULL,
		_StereoChannels[0].getTrack(), _StereoChannels[0].getTrack()?_StereoChannels[0].getTrack()->DrvSource->getStaticBuffer():NULL );
	nldebug( "AM: EnvSound: Channel #2    %p has sound %p, buffer %p, track %p, buffer %p",
		&_StereoChannels[1], _StereoChannels[1].getSound(), _StereoChannels[1].getSound()?_StereoChannels[1].getSound()->getBuffer():NULL,
		_StereoChannels[1].getTrack(), _StereoChannels[1].getTrack()?_StereoChannels[1].getTrack()->DrvSource->getStaticBuffer():NULL );
	nldebug( "AM: EnvSound: Channel #3    %p has sound %p, buffer %p, track %p, buffer %p",
		&_StereoChannels[2], _StereoChannels[2].getSound(), _StereoChannels[2].getSound()?_StereoChannels[2].getSound()->getBuffer():NULL,
		_StereoChannels[2].getTrack(), _StereoChannels[2].getTrack()?_StereoChannels[2].getTrack()->DrvSource->getStaticBuffer():NULL );*/

	if ( (!_Play) || (_StereoGain==0.0f) )
	{
		return;
	}

	bool crossfade;
	uint32 leadchannel, backchannel;
	TTime posInCycle = calcPosInCycle( crossfade, leadchannel );
	backchannel = 1 - leadchannel;
	
	// Crossfade the first two sources
	if ( crossfade )
	{
		// Attack
		float ratio = (float)posInCycle / (float)_CrossfadeTime;
		_StereoChannels[leadchannel].setRelativeGain( ratio*_StereoGain );
		_StereoChannels[backchannel].setRelativeGain( (1.0f - ratio)*_StereoGain );

		// Start next sound
		if ( _Sustain )
		{
			nldebug( "AM: EnvSound: Beginning crossfade: channel #%u rising", leadchannel );
			_StereoChannels[leadchannel].play();
			_Sustain = false;
		}
	}
	else
	{
		// Set sustain gain (takes into account the possible changes to _StereoGain)
		_StereoChannels[leadchannel].setRelativeGain( _StereoGain );

		// Prepare next sound
		if ( ! _Sustain )
		{
			_Sustain = true;
			TSoundId nextsound = getRandomSound( _AmbiantSounds );
#ifdef ENVSOUND_DONT_DUPLICATE_AMBIANT
			while ( nextsound == _StereoChannels[leadchannel].getSound() )
			{
				nldebug( "AM: EnvSound: Avoiding ambiant sound duplication..." );
				nextsound = getRandomSound( _AmbiantSounds );
			}
#endif
			nldebug( "AM: EnvSound: Sustain: channel #1" );
			_StereoChannels[backchannel].setRelativeGain( 0.0f );
			_StereoChannels[backchannel].stop();
			_StereoChannels[backchannel].setSound( nextsound );
		}
	}

	// Add a short random sound into the third source
	if ( ! _SparseSounds.empty() )
	{
		TTime now = CTime::getLocalTime();
		if ( now > _NextSparseSoundTime )
		{
			TSoundId nextsound = getRandomSound( _SparseSounds );
			_StereoChannels[SPARSE_CH].stop();
			_StereoChannels[SPARSE_CH].setSound( nextsound );
			_StereoChannels[SPARSE_CH].play();
			nldebug( "AM: EnvSound: Playing sparse sound" );
			if ( _StereoChannels[SPARSE_CH].getTrack() == NULL )
			{
				nldebug( "AM: Ensound: Switch on sparse channel" );
				nlverify( CAudioMixerUser::instance()->giveTrack( &_StereoChannels[SPARSE_CH] ) );
				nlassert( _StereoChannels[SPARSE_CH].getSound() != NULL );
			}
			// Does not leave the track at present time
			calcRandomSparseSoundTime( nextsound );
		}
	}
}


/*
 * Calculate the next time a sparse sound plays
 */
void		CEnvSoundUser::calcRandomSparseSoundTime( TSoundId currentsparesound )
{
	uint32 delay = (uint)((float)rand() * (float)(_SparseAvgPeriod*2) / (float)RAND_MAX);

	// Check the next sound will play after the current one
	if ( currentsparesound != NULL )
	{
		uint32 soundlength = currentsparesound->getDuration();
		if ( delay <= soundlength )
		{
			delay = soundlength+1;
		}
	}

	nldebug( "AM: EnvSound: Next sparse sound will play in %u ms", delay );
	_NextSparseSoundTime = CTime::getLocalTime() + delay;
}


/*
 * Start or stop the center source
 */
void		CEnvSoundUser::manageCenterSource( bool toplay )
{
	if ( _CenterSource.getSound() != NULL )
	{
		if ( toplay )
		{
			// Enter track and play
			if ( _CenterSource.getTrack() == NULL )
			{
				nlverify( CAudioMixerUser::instance()->giveTrack( &_CenterSource ) );
				_CenterSource.play();
			}
		}
		else
		{
			// Stop and remove useless source from track
			if ( _CenterSource.getTrack() != NULL )
			{
				_CenterSource.stop();
				CAudioMixerUser::instance()->releaseTrack( &_CenterSource );
			}
		}
	}
}


/*
 * Start or stop the stereo channels
 */
void		CEnvSoundUser::manageStereoChannels( bool toplay, bool crossfade, uint32 leadchannel )
{
	if ( toplay )
	{
		if ( _StereoChannels[leadchannel].getTrack() == NULL )
		{
			nldebug( "AM: Envsound: Switch on channel %u", leadchannel );
			nlverify( CAudioMixerUser::instance()->giveTrack( &_StereoChannels[leadchannel] ) );
			nlassert( _StereoChannels[leadchannel].getSound() != NULL );
			_StereoChannels[leadchannel].play();
		}
		if ( (_StereoChannels[1-leadchannel].getTrack() == NULL) && crossfade )
		{
			nldebug( "AM: Envsound: Switch on channel %u", 1-leadchannel );
			nlverify( CAudioMixerUser::instance()->giveTrack( &_StereoChannels[1-leadchannel] ) );
			nlassert( _StereoChannels[1-leadchannel].getSound() != NULL );
			_StereoChannels[1-leadchannel].play();
		}
		// The SPARSE_CH is added only when needed
	}
	else
	{
		if ( _StereoChannels[leadchannel].getTrack() != NULL )
		{
			nldebug( "AM: Envsound: Switch off channel %u", leadchannel );
			_StereoChannels[leadchannel].stop();
			CAudioMixerUser::instance()->releaseTrack( &_StereoChannels[leadchannel] );
		}
		if ( (_StereoChannels[1-leadchannel].getTrack() != NULL) && crossfade )
		{
			nldebug( "AM: Envsound: Switch off channel %u", 1-leadchannel );
			_StereoChannels[1-leadchannel].stop();
			CAudioMixerUser::instance()->releaseTrack( &_StereoChannels[1-leadchannel] );
		}
		if ( _StereoChannels[SPARSE_CH].getTrack() != NULL )
		{
			nldebug( "AM: Envsound: Switch off sparse channel" );
			_StereoChannels[SPARSE_CH].stop();
			CAudioMixerUser::instance()->releaseTrack( &_StereoChannels[SPARSE_CH] );
		}
	}
}


/*
 * Select a random sound in a bank
 */
TSoundId	CEnvSoundUser::getRandomSound( const std::vector<CSound*>& bank ) const
{
	nlassert( ! bank.empty() );
	// Note: does not work with a very big size (rand()*bank.size() would overflow)
	uint32 r = rand()*bank.size()/(RAND_MAX+1);
	nlassert( r < bank.size() );
	nldebug( "AM: EnvSound: Prepared random sound number %u of %u", r, bank.size()-1 );

	return bank[r];
}


/*
 * Set properties (EDIT)
 */
void		CEnvSoundUser::setProperties( CSourceUser& centersrc,
										   CSourceUser stchannels[MAX_ENV_CHANNELS],
										   float innerradius, float outerradius,
										   std::vector<TSoundId>& ambiantsounds,
										   std::vector<TSoundId>& sparsesounds,
										   uint32 crossfadeTimeMs, uint32 sustainTimeMs,
										   uint32 sparseAvgPeriodMs )
{
	_CenterSource = centersrc;
	_StereoChannels[0] = stchannels[0];
	_StereoChannels[1] = stchannels[1];
	_StereoChannels[2] = stchannels[2];
	_InnerRadius = innerradius;
	_OuterRadius = outerradius;
	_AmbiantSounds = ambiantsounds;
	_SparseSounds = sparsesounds;
	_CrossfadeTime = crossfadeTimeMs;
	_SustainTime = sustainTimeMs;
	_SparseAvgPeriod = sparseAvgPeriodMs;
}


/*
 * Save (output stream only) (EDIT)
 */
void CEnvSoundUser::save( const std::vector<CEnvSoundUser>& container, NLMISC::IStream& s )
{
	nlassert( ! s.isReading() );

	uint32 nb=container.size(), i;
	serialFileHeader( s, nb );
	for ( i=0; i!=nb; i++ )
	{
		s.serial( const_cast<CEnvSoundUser&>(container[i]) );
	}
}


/*
 * Play
 */
void CEnvSoundUser::play()
{
	_Play = true;
	calcRandomSparseSoundTime( NULL );
	recompute();
}


/*
 * Stop playing
 */
void CEnvSoundUser::stop()
{
	_Play = false;
	recompute();
	//_StereoChannels[SPARSE_CH].stop(); // if playing, emergency stop, before its end
}


} // NLSOUND
