/** \file source_user.cpp
 * CSourceUSer: implementation of USource
 *
 * $Id: source_user.cpp,v 1.29 2002/11/04 15:40:44 boucher Exp $
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
#include "simple_sound.h"

using namespace NLMISC;


namespace NLSOUND 
{


/*
 * Constructor
 */
CSourceUser::CSourceUser( CSimpleSound *simpleSound, bool spawn, TSpawnEndCallback cb, void *cbUserParam, CSoundContext *context, const std::string &buffername)
:	CSourceCommon(simpleSound, spawn, cb, cbUserParam, context),
	_Track(NULL)
{
	setSound( simpleSound, context, buffername );
}


/*
 * Destructor
 */
CSourceUser::~CSourceUser()
{
//	removeSource(this, _Buffername);
}
/*
void CSourceUser::storeSource(CSourceUser *psourceUser, const std::string &buffername)
{
	std::map<std::string, std::set<CSourceUser *> >::iterator it(_Sources.find(buffername);
	if (it == _Sources.end())
	{
		it = _Sources.insert(std::make_pair(buffername, std::set<CSourceUser*> ());
	}
	it->second.insert(psourceUser);
}

void CSourceUser::removeSource(CSourceUser *psourceUser, const std::string &buffername)
{
	std::map<std::string, std::set<CSourceUser *> >::iterator it(_Sources.find(buffername);
	// remove the buffer assoc.
	if (it != _Sources.end())
	{
		it->second.erase(psourceUser);

		if (it->second.empty())
			_Sources.erase(it);
	}
}

void CSourceUser::bufferUnloaded(const std::string &buffername)
{
	std::map<std::string, std::set<CSourceUser *> >::iterator it(_Sources.find(buffername);

	if (it != _Sources.end())
	{
		std::set<CSourceUser *>::iterator first(it->second.begin()), last(it->second.end());
		for (; first != last; ++first)
		{
			first->
		}
				
	}
}
*/

CSimpleSound	*CSourceUser::getSimpleSound()
{
	return _Sound;
}

void CSourceUser::setSound( TSoundId id, CSoundContext *context)
{
	nlassert(id->getSoundType() == CSound::SOUND_SIMPLE);

	setSound(static_cast<CSimpleSound*> (id), context, "");
}


/*
 * Change the sound binded to the source
 */
void					CSourceUser::setSound( CSimpleSound *simpleSound, CSoundContext *context, const std::string &buffername )
{
	if ( simpleSound == NULL )
	{
		stop();
	}
	_Sound = simpleSound;
	if ( _Sound != NULL )
	{
		_InitialGain = _Gain = _Sound->getGain();
		_Pitch = _Sound->getPitch();
		_Looping = _Sound->getLooping();
		_Priority = _Sound->getPriority();
		_Alpha = _Sound->getAlpha();
		_Direction = _Sound->getDirectionVector();

		// get the buffername with a specific context
		if(buffername.empty())
		{
			_Sound->getBuffername(_Buffername, context);
		}
		else
		{
			_Buffername = buffername;
		}
		if (_Sound->getBuffer(&_Buffername) == NULL)
		{
			nlwarning ("buffername '%s' is not found", _Buffername.c_str());
			_Sound = NULL;
		}
	}

	// Set the buffer
	if ( _Track != NULL )
	{
		if ( _Sound != NULL )
		{
			nlassert( _Sound->getBuffer() != NULL );
			nlassert( ! isPlaying() );
			_Track->DrvSource->setStaticBuffer( _Sound->getBuffer(&_Buffername) );

			// Take into account the static properties in _Sound
			_Track->DrvSource->setGain( _Gain );
			_Track->DrvSource->setPitch( _Pitch );
			_Track->DrvSource->setLooping( _Looping );
			_Track->DrvSource->setAlpha(_Alpha);
			_Track->DrvSource->setDirection(_Direction);
			if ( ! _Sound->getBuffer(&_Buffername)->isStereo() )
			{
				_Track->DrvSource->setMinMaxDistances( _Sound->getMinDistance(), _Sound->getMaxDistance() );
				_Track->DrvSource->setCone( _Sound->getConeInnerAngle(), _Sound->getConeOuterAngle(), _Sound->getConeOuterGain() );
				_Track->DrvSource->setDirection( _Direction ); // apply cone
			}
		}
	}

//	storeSource(this, _Buffername);
}

uint32	CSourceUser::getTime()
{
	if (_Track && _Track->DrvSource)
	{
		return _Track->DrvSource->getTime();
	}
	else
		return 0;
}


IBuffer					*CSourceUser::getBuffer()
{
	if (_Sound != NULL)
		return _Sound->getBuffer(&_Buffername);
	else
		return NULL;
}




/*
 * Set looping on/off for future playbacks (default: off)
 */
void					CSourceUser::setLooping( bool l )
{
	CSourceCommon::setLooping(l);
	if ( _Track != NULL )
	{
		_Track->DrvSource->setLooping( l );
	}
}


/*
 * Play
 */
void					CSourceUser::play()
{
	CAudioMixerUser::instance()->setPlaying(this);
	if ( _Sound == NULL || _Sound->getBuffer(&_Buffername) == NULL)
	{
//		nlwarning ("sound is available but buffer name '%s' is not found", _Buffername.c_str());
		_PlayStart = CTime::getLocalTime();
		_Playing = false;
		return;
	}

	if ( _Track != NULL )
	{
		if (_Track->DrvSource->getStaticBuffer() == NULL)
		{
			/// Restore an unloaded buffer ?
			_Track->DrvSource->setStaticBuffer(_Sound->getBuffer());
		}
		if (_Track->DrvSource->play())
		{
			CSourceCommon::play();
		}
		else
		{
//			nlwarning ("sound buffer '%s' can't be played", _Buffername.c_str());
			_PlayStart = CTime::getLocalTime();
			_Playing = false;
			return;
		}

//		nldebug( "AM: Playing source %s", getSound() && (getSound()->getName()!="") ? getSound()->getName().c_str() : "" );
//		nlinfo( "AM: Playing source %p", this);
	}
	else
	{
		// could not have track. Only set the playStart time.
		_PlayStart = CTime::getLocalTime();
	}

}


/*
 * Stop playing
 */
void					CSourceUser::stop()
{
	CAudioMixerUser::instance()->setPlaying(this);
	if ( _Track != NULL )
	{
		_Track->DrvSource->stop();
		nldebug( "AM: Source %s stopped", getSound() && (getSound()->getName()!="") ? getSound()->getName().c_str() : "" );
//		nlinfo( "AM: Stoping source %p", this);
	}
	CSourceCommon::stop();
}


/* Set the position vector (default: (0,0,0)).
 * 3D mode -> 3D position
 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
 */
void					CSourceUser::setPos( const NLMISC::CVector& pos )
{
	CSourceCommon::setPos(pos);

	// Set the position
	if ( _Track != NULL )
	{
		_Track->DrvSource->setPos( pos );
	}
}


/*
 * Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
 */
void					CSourceUser::setVelocity( const NLMISC::CVector& vel )
{
	CSourceCommon::setVelocity(vel);

	// Set the velocity
	if ( _Track != NULL )
	{
		// TODO : uncoment, test only	
		_Track->DrvSource->setVelocity( vel );
	}
}


/*
 * Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
 */
void					CSourceUser::setDirection( const NLMISC::CVector& dir )
{
	CSourceCommon::setDirection(dir);

	// Set the direction
	if ( _Track != NULL )
	{
		if ( ! _Sound->getBuffer(&_Buffername)->isStereo() )
		{
			static bool coneset = false;
			if ( dir.isNull() ) // workaround
			{
				_Track->DrvSource->setCone( float(Pi*2), float(Pi*2), 1.0f ); // because the direction with 0 is not enough for a non-directional source!
				_Track->DrvSource->setDirection( CVector::I );  // Don't send a 0 vector, DSound will complain. Send (1,0,0), it's omnidirectional anyway.
				coneset = false;
			}
			else
			{
//				if ( ! coneset )
				{
					_Track->DrvSource->setCone( _Sound->getConeInnerAngle(), _Sound->getConeOuterAngle(), _Sound->getConeOuterGain() );
					coneset = true;
				}
				_Track->DrvSource->setDirection( dir );
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
	CSourceCommon::setGain(gain);

	// Set the gain
	if ( _Track != NULL )
	{
		_Track->DrvSource->setGain( gain );
	}
}

void CSourceUser::setRelativeGain( float gain )
{
	CSourceCommon::setRelativeGain(gain);

	// Set the gain
	if ( _Track != NULL )
	{
		_Track->DrvSource->setGain( _Gain );
	}
}


/* Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
 * of one octave. 0 is not a legal value.
 */
void					CSourceUser::setPitch( float pitch )
{
	CSourceCommon::setPitch(pitch);

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
	CSourceCommon::setSourceRelativeMode(mode);

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

	nlassert( _Sound->getBuffer(&_Buffername) != NULL );
	_Track->DrvSource->setStaticBuffer( _Sound->getBuffer(&_Buffername) );

	_Track->DrvSource->setPos( _Position );
	if ( ! _Sound->getBuffer(&_Buffername)->isStereo() )
	{
		_Track->DrvSource->setMinMaxDistances( _Sound->getMinDistance(), _Sound->getMaxDistance() );
		setDirection( _Direction ); // because there is a workaround inside
		_Track->DrvSource->setVelocity( _Velocity );
	}
	_Track->DrvSource->setGain( _Gain );
	_Track->DrvSource->setSourceRelativeMode( _RelativeMode );
	_Track->DrvSource->setLooping( _Looping );
	_Track->DrvSource->setPitch( _Pitch );
	_Track->DrvSource->setAlpha( _Alpha );
}



/* Set the corresponding track	(NULL allowed, does the same as leaveTrack())
 * Don't set a non-null track if getSound() is null.
 */
void					CSourceUser::enterTrack( CTrack *track )
{
	if (track == _Track)
		return;
	else if (_Track != 0 && track == 0)
	{
		nlassert(_Track->getUserSource() == this);
		_Track->setUserSource(NULL);
		_Track = track;
		return;
	}

	_Track = track;

	// FIXME: SWAPTEST
	if ( _Track != NULL && _Track->DrvSource->isPlaying() )
	{
		nlstopex( ("buffer name = %s", _Buffername.c_str()) );
	}

	if ( _Track != NULL )
	{
		nlassertex( _Sound != NULL, ("buffer name = %s", _Buffername.c_str()) );

		_Track->setUserSource( this );
		if (_Sound->getBuffer(&_Buffername) != NULL)
		{
			copyToTrack(); // must always be synchronized, because the tracks may not have the default settings
			if ( _Playing )
			{
				// Play physically
				_Track->DrvSource->play();
			}
			//nldebug( "AM: Source %s selected for playing", getSound() && (getSound()->getName()!="") ? getSound()->getName().c_str() : "" );
		}
		else
		{
			// it means that the sample is not loaded (not found?) so don't play
			//nldebug( "AM: Source %s selected for playing but sample is not loaded", getSound() && (getSound()->getName()!="") ? getSound()->getName().c_str() : "" );
		}
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
//	_Track->setUserSource( NULL );

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
			else if ( _Looping || (CTime::getLocalTime()-_PlayStart < _Sound->getDuration(&_Buffername)) )
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
			if (CTime::getLocalTime()-_PlayStart < _Sound->getDuration(&_Buffername))
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
		if ( _Sound == 0)
			return true;

		if ( _PlayStart == 0 )
		{
			// Not played yet
			return false;
		}
		else if ( (!_Looping) && (CTime::getLocalTime()-_PlayStart > _Sound->getDuration(&_Buffername)) )
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
