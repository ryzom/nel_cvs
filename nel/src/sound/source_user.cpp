/** \file source_user.cpp
 * CSourceUSer: implementation of USource
 *
 * $Id: source_user.cpp,v 1.24 2002/07/10 17:08:56 lecroart Exp $
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

#include "source_user.h"
#include "driver/buffer.h"
#include "driver/source.h"
#include "mixing_track.h"
#include "sound.h"

using namespace NLMISC;


namespace NLSOUND 
{


/*
 * Constructor
 */
CSourceUser::CSourceUser( TSoundId id, bool spawn, TSpawnEndCallback cb, void *cbUserParam ) :
	_Priority(MidPri), _Playing(false),
	_Position(CVector::Null), _Velocity(CVector::Null), _Direction(CVector::Null),
	_Gain(1.0f), _Pitch(1.0f), _RelativeMode(false), _Looping(false),
	_Track(NULL), _3DPosition(NULL), _PlayStart(0), _Spawn(spawn), _SpawnEndCb(cb), _CbUserParam(cbUserParam)
{
	setSound( id );
}


/*
 * Destructor
 */
CSourceUser::~CSourceUser()
{
	/*if ( _Track != NULL )
	{
		if ( _Playing )
		{
			_Track->DrvSource->stop();
		}
		_Track->setUserSource( NULL );
	}
	CAudioMixerUser::instance()->removeSource( this );*/
}


/*
 * Change the sound binded to the source
 */
void					CSourceUser::setSound( TSoundId id )
{
	if ( id == NULL )
	{
		stop();
	}
	_Sound = id;
	if ( _Sound != NULL )
	{
		_Gain = _Sound->getGain();
		_Pitch = _Sound->getPitch();
		_Looping = _Sound->getLooping();
		_Priority = _Sound->getPriority();
	}

	// Set the buffer
	if ( _Track != NULL )
	{
		if ( _Sound != NULL )
		{
			nlassert( _Sound->getBuffer() != NULL );
			nlassert( ! isPlaying() );
			_Track->DrvSource->setStaticBuffer( _Sound->getBuffer() );

			// Take into account the static properties in _Sound
			_Track->DrvSource->setGain( _Gain );
			_Track->DrvSource->setPitch( _Pitch );
			_Track->DrvSource->setLooping( _Looping );
			if ( ! _Sound->getBuffer()->isStereo() )
			{
				_Track->DrvSource->setMinMaxDistances( _Sound->getMinDistance(), _Sound->getMaxDistance() );
				_Track->DrvSource->setCone( _Sound->getConeInnerAngle(), _Sound->getConeOuterAngle(), _Sound->getConeOuterGain() );
				_Track->DrvSource->setDirection( _Direction ); // apply cone
			}
		}
	}
}


/*
 * Change the priority of the source
 */
void					CSourceUser::setPriority( TSoundPriority pr, bool redispatch )
{
	_Priority = pr;

	// The AudioMixer redispatches as necessary in the update() function [PH]
	// Redispatch the tracks if needed
	//if ( redispatch )
	//{
	//	CAudioMixerUser::instance()->balanceSources();
	//}
}


/*
 * Set looping on/off for future playbacks (default: off)
 */
void					CSourceUser::setLooping( bool l )
{
	_Looping = l;
	if ( _Track != NULL )
	{
		_Track->DrvSource->setLooping( l );
	}
}


/*
 * Return the looping state
 */
bool					CSourceUser::getLooping() const
{
	return _Looping;
}


/*
 * Play
 */
void					CSourceUser::play()
{

#ifdef NL_DEBUG
	if ( _Sound != NULL )
	{
		nlassert( _Sound->getBuffer() != NULL );
	}
#endif

	if ( _Track != NULL )
	{
		_Track->DrvSource->play();
//		nldebug( "AM: Playing source %s", getSound() && (getSound()->getName()!="") ? getSound()->getName().c_str() : "" );
//		nlinfo( "AM: Playing source %p", this);
	}
	_Playing = true;
	_PlayStart = CTime::getLocalTime();
}


/*
 * Stop playing
 */
void					CSourceUser::stop()
{
	if ( _Track != NULL )
	{
		_Track->DrvSource->stop();
		nldebug( "AM: Source %s stopped", getSound() && (getSound()->getName()!="") ? getSound()->getName().c_str() : "" );
		nlinfo( "AM: Stoping source %p", this);
	}
	_Playing = false;
}


/* Set the position vector (default: (0,0,0)).
 * 3D mode -> 3D position
 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
 */
void					CSourceUser::setPos( const NLMISC::CVector& pos )
{
	_Position = pos;

	// Set the position
	if ( _Track != NULL )
	{
		_Track->DrvSource->setPos( pos );
	}
}


/* Get the position vector.
 * If the source is stereo, return the position vector which reference was passed to set3DPositionVector()
 */
void					CSourceUser::getPos( NLMISC::CVector& pos ) const
{
	if ( _3DPosition == NULL )
	{
		pos = _Position;
	}
	else
	{
		pos = *_3DPosition;
	}
	
}


/*
 * Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
 */
void					CSourceUser::setVelocity( const NLMISC::CVector& vel )
{
	_Velocity = vel;

	// Set the velocity
	if ( _Track != NULL )
	{
		_Track->DrvSource->setVelocity( vel );
	}
}


/*
 * Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
 */
void					CSourceUser::setDirection( const NLMISC::CVector& dir )
{
	_Direction = dir;

	// Set the direction
	if ( _Track != NULL )
	{
		if ( ! _Sound->getBuffer()->isStereo() )
		{
			static bool coneset = false;
			if ( dir.isNull() ) // workaround
			{
				_Track->DrvSource->setCone( 6.283185f, 6.283185f, 1.0f ); // because the direction with 0 is not enough for a non-directional source!
				_Track->DrvSource->setDirection( CVector::I );  // Don't send a 0 vector, DSound will complain. Send (1,0,0), it's omnidirectional anyway.
				coneset = false;
			}
			else
			{
				if ( ! coneset )
				{
					_Track->DrvSource->setCone( _Sound->getConeInnerAngle(), _Sound->getConeOuterAngle(), _Sound->getConeOuterGain() );
					_Track->DrvSource->setDirection( dir );
					coneset = true;
				}
			}
		}
	}
}


/* Set the gain (volume value inside [0 , 1]). (default: 1)
 * 0.0 -> silence
 * 0.5 -> -6dB
 * 1.0 -> no attenuation
 * values > 1 (amplification) not supported by most drivers
 */
void					CSourceUser::setGain( float gain )
{
	nlassert( (gain >= 0.0f) && (gain <= 1.0f ) );
	_Gain = gain;

	// Set the gain
	if ( _Track != NULL )
	{
		_Track->DrvSource->setGain( gain );
	}
}


/* Set the gain amount (value inside [0, 1]) to map between 0 and the nominal gain
 * (which is getSource()->getGain()). Does nothing if getSource() is null.
 */
void					CSourceUser::setRelativeGain( float gain )
{
	nlassert( (gain >= 0.0f) && (gain <= 1.0f ) );
	if ( _Sound != NULL )
	{
		setGain( gain * _Sound->getGain() );
	}
}


/*
 * Return the relative gain (see setRelativeGain()), or the absolute gain if getSource() is null.
 */
float					CSourceUser::getRelativeGain() const
{
	if ( _Sound != NULL )
	{
		float gain = getGain();
		if ( gain == 0.0f )
			return 0.0f;
		else
			return _Gain / gain;
	}
	else
	{
		return _Gain;
	}
}


/* Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
 * of one octave. 0 is not a legal value.
 */
void					CSourceUser::setPitch( float pitch )
{
	nlassert( (pitch > 0) && (pitch <= 1.0f ) );
	_Pitch = pitch;

	// Set the pitch
	if ( _Track != NULL )
	{
		_Track->DrvSource->setPitch( pitch );
	}
}


/*
 * Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
 */
void					CSourceUser::setSourceRelativeMode( bool mode )
{
	_RelativeMode = mode;

	// Set the relative mode
	if ( _Track != NULL )
	{
		_Track->DrvSource->setSourceRelativeMode( mode );
	}
}



/*
 * Copy the source data into a track
 */
void					CSourceUser::copyToTrack()
{
	nlassert( _Track != NULL ); 

	nlassert( _Sound->getBuffer() != NULL );
	_Track->DrvSource->setStaticBuffer( _Sound->getBuffer() );

	_Track->DrvSource->setPos( _Position );
	if ( ! _Sound->getBuffer()->isStereo() )
	{
		_Track->DrvSource->setMinMaxDistances( _Sound->getMinDistance(), _Sound->getMaxDistance() );
		setDirection( _Direction ); // because there is a workaround inside
		_Track->DrvSource->setVelocity( _Velocity );
	}
	_Track->DrvSource->setGain( _Gain );
	_Track->DrvSource->setSourceRelativeMode( _RelativeMode );
	_Track->DrvSource->setLooping( _Looping );
	_Track->DrvSource->setPitch( _Pitch );
}



/* Set the corresponding track	(NULL allowed, does the same as leaveTrack())
 * Don't set a non-null track if getSound() is null.
 */
void					CSourceUser::enterTrack( CTrack *track )
{
	_Track = track;

	// FIXME: SWAPTEST
	if ( _Track != NULL && _Track->DrvSource->isPlaying() )
	{
		nlassert(0);
	}

	if ( _Track != NULL )
	{
		nlassert( _Sound != NULL );
		copyToTrack(); // must always be synchronized, because the tracks may not have the default settings
		_Track->setUserSource( this );
		if ( _Playing )
		{
			// Play physically
			_Track->DrvSource->play();
		}
		//nldebug( "AM: Source %s selected for playing", getSound() && (getSound()->getName()!="") ? getSound()->getName().c_str() : "" );
	}
	else
	{
		//nldebug( "AM: Source %s unselected", getSound() && (getSound()->getName()!="") ? getSound()->getName().c_str() : "" );
	}
}


/*
 * Unset the corresponding track
 */
void					CSourceUser::leaveTrack()
{	
	if ( _Track != NULL )
	{
		_Track->setUserSource( NULL );

		if ( _Playing )
		{
			// Stop to play physically
			_Track->DrvSource->stop();
		}
	}
	enterTrack( NULL );
}


/*
 * Enable (play with high priority) or disable (stop and set low priority)
 */
void					CSourceUser::enable( bool toplay, float gain )
{
	if ( getSound() != NULL )
	{
		if ( toplay )
		{
			// Set gain
			setRelativeGain( gain );

			// Start playing
			if ( getPriority() == LowPri )
			{
				setPriority( HighPri );
				play();
			}
		}
		else
		{
			// Stop playing
			if ( getPriority() != LowPri )
			{
				stop();
				setPriority( LowPri );
			}
		}
	}
}


/*
 * Serial sound and looping state (warning: partial serial, for CEnvSoundUser)
 */
/*
void					CSourceUser::serial( NLMISC::IStream& s )
{
	// If you change this, increment the version number in CEnvSoundUser::load() !

	// 3D position and sound (allocated here if once per pointer,
	// deleted in CAudioMixerUser::~CAudioMixerUser() (registered by addSource() below))
	s.serialPtr( _Sound );
	s.serial( _Looping );

	if ( s.isReading() )
	{
		if ( _Sound != NULL )
		{
			// Put into tracks
			CAudioMixerUser::instance()->addSource( this );
			CAudioMixerUser::instance()->giveTrack( this );
		}
	}
}
*/


/*
 * Get playing state. Return false if the source has stopped on its own.
 */
bool					CSourceUser::isPlaying()
{
	if ( _Playing )
	{
		if ( getTrack() != NULL )
		{
			// Check if "online" source is still playing
			if ( getTrack()->DrvSource->isPlaying() )
			{
				return true;
			}
			else
			{
				_Playing = false;
				return false;
			}
		}
		else
		{
			// Check if "offline" source should have stopped playing
			nlassert( _Sound );
			if ( _PlayStart == 0 )
			{
				// Not played yet
				return false;
			}
			else if ( _Looping || (CTime::getLocalTime()-_PlayStart < _Sound->getDuration()) )
			{
				return true;
			}
			else
			{
				_Playing = false;
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}


/*
 * Return true if playing is finished or stop() has been called.
 */
bool					CSourceUser::isStopped()
{
	if ( getTrack() != NULL )
	{
		if ( getTrack()->DrvSource->isStopped() )
		{
			if (CTime::getLocalTime()-_PlayStart < _Sound->getDuration())
			{
				//nlwarning ("openal bug (think that the sample [%p] is finished but not : %u %u", getTrack()->DrvSource, (uint32)(CTime::getLocalTime()-_PlayStart), _Sound->getDuration());
			}
		}
	}

/*	if ( getTrack() != NULL )
	{
		if ( getTrack()->DrvSource->isStopped() )
		{
			if (CTime::getLocalTime()-_PlayStart < _Sound->getDuration())
			{
/// open al thinks that the sound is finish but is not!!!
///				nlinfo ("%u %u", (uint32)(CTime::getLocalTime()-_PlayStart), _Sound->getDuration());
////				nlstop;
			}

			_Playing = false;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
*/	{
		nlassert( _Sound );
		if ( _PlayStart == 0 )
		{
			// Not played yet
			return false;
		}
		else if ( (!_Looping) && (CTime::getLocalTime()-_PlayStart > _Sound->getDuration()) )
		{
			_Playing = false;
			return true;
		}
		else
		{
			return false;
		}
	}
}


} // NLSOUND
