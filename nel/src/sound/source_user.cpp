/** \file source_user.cpp
 * CSourceUSer: implementation of USource
 *
 * $Id: source_user.cpp,v 1.2 2001/07/13 09:47:11 cado Exp $
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

#include "source_user.h"
#include "nel/misc/vector.h"
#include "driver/buffer.h"
#include "driver/source.h"
#include "track.h"
#include "sound.h"
#include "audio_mixer_user.h"


using namespace NLMISC;


namespace NLSOUND {


/*
 * Constructor
 */
CSourceUser::CSourceUser( TSoundId id ) :
	_Priority(MidPri), _Playing(false),
	_Position(CVector::Null), _Velocity(CVector::Null), _Direction(CVector::Null),
	_Gain(1.0f), _RelativeMode(false), _Looping(false),
	_Track(NULL), _ParentSource(NULL)
{
	setSound( id );
}


/*
 * Destructor
 */
CSourceUser::~CSourceUser()
{
	if ( _Track != NULL )
	{
		if ( _Playing )
		{
			_Track->DrvSource->stop();
		}
		_Track->setUserSource( NULL );
	}
}


/*
 * Change the sound binded to the source
 */
void					CSourceUser::setSound( TSoundId id )
{
	_Sound = id;
	if ( _Sound != NULL )
	{
		_Gain = _Sound->getGain();
	}

	// Set the buffer
	if ( _Track != NULL )
	{
		if ( _Sound != NULL )
		{
			nlassert( _Sound->getBuffer() != NULL );
			nlassert( ! isPlaying() );
			_Track->DrvSource->setStaticBuffer( _Sound->getBuffer() );
			_Track->DrvSource->setGain( _Gain );
			_Track->DrvSource->setMinMaxDistances( _Sound->getMinDistance(), _Sound->getMaxDistance() );
			_Track->DrvSource->setCone( _Sound->getConeInnerAngle(), _Sound->getConeOuterAngle(), _Sound->getConeOuterGain() );
		}
	}
}


/*
 * Change the priority of the source
 */
void					CSourceUser::setPriority( TSoundPriority pr, bool redispatch )
{
	_Priority = pr;

	// Redispatch the tracks if needed
	if ( redispatch )
	{
		CAudioMixerUser::instance()->balanceSources();
	}
}


/*
 * Set looping on/off for future playbacks (default: off)
 */
void					CSourceUser::setLooping( bool l )
{
	_Looping = l;
	if ( _Track != NULL )
	{
		_Track->DrvSource->setLooping( true );
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
		nlassert( (_Sound->getBuffer() != NULL) );
	}
#endif
	
	if ( _Track != NULL )
	{
		_Track->DrvSource->play();
		nldebug( "AM: Playing source" );
	}
	_Playing = true;
}


/*
 * Stop playing
 */
void					CSourceUser::stop()
{
	if ( _Track != NULL )
	{
		_Track->DrvSource->stop();
		nldebug( "AM: Source stopped" );
	}
	_Playing = false;
}


/* Set the position vector (default: (0,0,0)).
 * 3D mode -> 3D position
 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
 */
void					CSourceUser::setPosition( const NLMISC::CVector& pos )
{
	_Position = pos;

	// Set the position
	if ( _Track != NULL )
	{
		_Track->DrvSource->setPosition( pos );
	}
}


/* Get the position vector.
 * If the parent source is not null, return its position.
 */
void					CSourceUser::getPosition( NLMISC::CVector& pos ) const
{
	if ( _ParentSource == NULL )
	{
		pos = _Position;
	}
	else
	{
		_ParentSource->getPosition( pos );
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
		_Track->DrvSource->setDirection( dir );
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

	_Track->DrvSource->setPosition( _Position );
	_Track->DrvSource->setVelocity( _Velocity );
	_Track->DrvSource->setDirection( _Direction );
	_Track->DrvSource->setGain( _Gain );
	_Track->DrvSource->setSourceRelativeMode( _RelativeMode );
	_Track->DrvSource->setLooping( _Looping );

	if ( ! _Sound->getBuffer()->isStereo() )
	{
		_Track->DrvSource->setMinMaxDistances( _Sound->getMinDistance(), _Sound->getMaxDistance() );
		_Track->DrvSource->setCone( _Sound->getConeInnerAngle(), _Sound->getConeOuterAngle(), _Sound->getConeOuterGain() );
	}
}



/* Set the corresponding track	(NULL allowed, does the same as leaveTrack())
 * Don't set a non-null track if getSound() is null.
 */
void					CSourceUser::enterTrack( CTrack *track )
{
	_Track = track;

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
		nldebug( "AM: Source selected for playing" );
	}
	else
	{
		nldebug( "AM: Source unselected" );
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


} // NLSOUND
