/** \file source_user.h
 * CSourceUSer: implementation of USource
 *
 * $Id: source_common.cpp,v 1.6.4.2 2003/08/14 08:02:20 boucher Exp $
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

#include "source_common.h"
#include "audio_mixer_user.h"


using namespace NLMISC;

namespace NLSOUND 
{

CSourceCommon::CSourceCommon(TSoundId id, bool spawn, TSpawnEndCallback cb, void *cbUserParam, NL3D::CCluster *cluster)
:	_Priority(MidPri),
	_Playing(false),
	_Looping(false),
	_Position(CVector::Null),
	_Velocity(CVector::Null),
	_Direction(CVector::Null),
	_Gain(1.0f),
	_InitialGain(1.0f),
	_Pitch(1.0f),
	_RelativeMode(false),
	_3DPosition(NULL),
	_PlayStart(0),
	_Spawn(spawn),
	_SpawnEndCb(cb),
	_CbUserParam(cbUserParam),
	_Cluster(cluster),
	_UserVarControler(id->getUserVarControler())
{
	CAudioMixerUser::instance()->addSource(this);

	// get a local copy of the sound parameter
	_InitialGain = _Gain = id->getGain();
	_Pitch = id->getPitch();
	_Looping = id->getLooping();
	_Priority = id->getPriority();
	_Direction = id->getDirectionVector();

}

CSourceCommon::~CSourceCommon()
{
	CAudioMixerUser::instance()->removeSource(this);
}


/*
 * Change the priority of the source
 */
void CSourceCommon::setPriority( TSoundPriority pr)
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
void					CSourceCommon::setLooping( bool l )
{
	_Looping = l;
}

/*
 * Return the looping state
 */
bool					CSourceCommon::getLooping() const
{
	return _Looping;
}

/*
 * Play
 */
void					CSourceCommon::play()
{
	CAudioMixerUser::instance()->incPlayingSource();
	_Playing = true;
	_PlayStart = CTime::getLocalTime();

	if (_UserVarControler != CStringMapper::emptyId())
		CAudioMixerUser::instance()->addUserControledSource(this, _UserVarControler);
}

/*
 * Stop playing
 */
void					CSourceCommon::stop()
{
	CAudioMixerUser::instance()->decPlayingSource();
	_Playing = false;

	if (_UserVarControler != CStringMapper::emptyId())
		CAudioMixerUser::instance()->removeUserControledSource(this, _UserVarControler);
}

/* Set the position vector (default: (0,0,0)).
 * 3D mode -> 3D position
 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
 */
void					CSourceCommon::setPos( const NLMISC::CVector& pos )
{
	_Position = pos;
}

/* Get the position vector.
 * If the source is stereo, return the position vector which reference was passed to set3DPositionVector()
 */
const NLMISC::CVector &CSourceCommon::getPos() const
{
	if ( _3DPosition == NULL )
	{
		return _Position;
	}
	else
	{
		return *_3DPosition;
	}
	
}

/* Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
 * of one octave. 0 is not a legal value.
 */
void					CSourceCommon::setPitch( float pitch )
{
//	nlassert( (pitch > 0) && (pitch <= 1.0f ) );
	_Pitch = pitch;
}


/*
 * Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
 */
void					CSourceCommon::setVelocity( const NLMISC::CVector& vel )
{
	_Velocity = vel;
}

/*
 * Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
 */
void					CSourceCommon::setDirection( const NLMISC::CVector& dir )
{
	_Direction = dir;
}

/* Set the gain (volume value inside [0 , 1]). (default: 1)
 * 0.0 -> silence
 * 0.5 -> -6dB
 * 1.0 -> no attenuation
 * values > 1 (amplification) not supported by most drivers
 */
void					CSourceCommon::setGain( float gain )
{
	nlassert( (gain >= 0.0f) && (gain <= 1.0f ) );
	_InitialGain = _Gain = gain;
}

/* Set the gain amount (value inside [0, 1]) to map between 0 and the nominal gain
 * (which is getSource()->getGain()). Does nothing if getSource() is null.
 */
void					CSourceCommon::setRelativeGain( float gain )
{
	nlassert( (gain >= 0.0f) && (gain <= 1.0f ) );

	_Gain = _InitialGain * gain;
}

/*
 * Return the relative gain (see setRelativeGain()), or the absolute gain if getSource() is null.
 */
float					CSourceCommon::getRelativeGain() const
{
	if (_InitialGain == 0.0f)
		return 0.0f;
	else
		return _Gain / _InitialGain;
}

/*
 * Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
 */
void					CSourceCommon::setSourceRelativeMode( bool mode )
{
	_RelativeMode = mode;
}


uint32					CSourceCommon::getTime()
{
	if (!_Playing)
		return 0;
	// default implementation
	return uint32(CTime::getLocalTime() - _PlayStart);
}


} // NLSOUND