/** \file listener_al.cpp
 * OpenAL sound listener
 *
 * $Id: listener_al.cpp,v 1.3 2001/07/13 09:42:54 cado Exp $
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

#include "listener_al.h"
#include "sound_driver_al.h"
#include "al/al.h"
#include "nel/misc/vector.h"

using namespace NLMISC;


namespace NLSOUND {


// The instance of the singleton
CListenerAL	*CListenerAL::_Instance = NULL;


/*
 * Constructor
 */
CListenerAL::CListenerAL() : IListener()
{
	if ( _Instance == NULL )
	{
		_Instance = this;
	}
	else
	{
		nlerror( "Listener singleton instanciated twice" );
	}
}


/*
 * Set the position vector (default: (0,0,0)) (3D mode only)
 */
void					CListenerAL::setPosition( const NLMISC::CVector& pos )
{
	alListener3f( AL_POSITION, pos.x, pos.y, pos.z );
	nlassert( alGetError() == AL_NO_ERROR );
}


/** Get the position vector.
 * See setPosition() for details.
 */
void					CListenerAL::getPosition( NLMISC::CVector& pos ) const
{
	alGetListener3f( AL_POSITION, &pos.x, &pos.y, &pos.z );
	nlassert( alGetError() == AL_NO_ERROR );
}


/*
 * Set the velocity vector (3D mode only)
 */
void					CListenerAL::setVelocity( const NLMISC::CVector& vel )
{
	alListener3f( AL_VELOCITY, vel.x, vel.y, vel.z );
	nlassert( alGetError() == AL_NO_ERROR );
}


/*
 * Get the velocity vector
 */
void				 	CListenerAL::getVelocity( NLMISC::CVector& vel ) const
{
	alGetListener3f( AL_VELOCITY, &vel.x, &vel.y, &vel.z );
	nlassert( alGetError() == AL_NO_ERROR );
}


/*
 * Set the orientation vectors (3D mode only)
 */
void					CListenerAL::setOrientation( const NLMISC::CVector& front, const NLMISC::CVector& up )
{
	// Forward then up
	ALfloat v[6];
	v[0] = front.x;
	v[1] = front.y;
	v[2] = front.z;
	v[3] = up.x;
	v[4] = up.y;
	v[5] = up.z;
	alListenerfv( AL_ORIENTATION, v );
	nlassert( alGetError() == AL_NO_ERROR );
}


/*
 * Get the orientation vectors
 */
void					CListenerAL::getOrientation( NLMISC::CVector& front, NLMISC::CVector& up ) const
{
	// Forward then up
	ALfloat v[6];
	alGetListenerfv( AL_ORIENTATION, v );
	nlassert( alGetError() == AL_NO_ERROR );
	front.set( v[0], v[1], v[2] );
	up.set( v[3], v[4], v[5] );
}


/* Set the gain (volume value inside [0 , 1]). (default: 1)
 * 0.0 -> silence
 * 0.5 -> -6dB
 * 1.0 -> no attenuation
 * values > 1 (amplification) not supported by most drivers
 */
void					CListenerAL::setGain( float gain )
{
	alListenerf( AL_GAIN, gain );
	nlassert( alGetError() == AL_NO_ERROR );
}


/*
 * Get the gain
 */
float					CListenerAL::getGain() const
{
	ALfloat gain;
	alGetListenerf( AL_GAIN, &gain );
	nlassert( alGetError() == AL_NO_ERROR );
	return gain;
}


/*
 * Set the doppler factor (default: 1) to exaggerate or not the doppler effect
 */
void					CListenerAL::setDopplerFactor( float f )
{
	alDopplerFactor( f );
	nlassert( alGetError() == AL_NO_ERROR );
}


/*
 * Set the rolloff factor (default: 1) to scale the distance attenuation effect
 */
void					CListenerAL::setRolloffFactor( float f )
{
	nlassert( CSoundDriverAL::instance() != NULL );
	CSoundDriverAL::instance()->applyRolloffFactor( f );
}


/*
 * Set DSPROPERTY_EAXLISTENER_ENVIRONMENT and DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE if EAX available (see EAX listener properties)
 */
void					CListenerAL::setEnvironment( uint env, float size )
{
	if ( EAXSetProp != NULL )
	{
		EAXSetProp( &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &env, sizeof(unsigned long) );
		EAXSetProp( &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE, 0, &size, sizeof(float) );
	}
}


/*
 * Set any EAX listener property if EAX available
 */
void					CListenerAL::setEAXProperty( uint prop, void *value, uint valuesize )
{
	if ( EAXSetProp != NULL )
	{
		EAXSetProp( &DSPROPSETID_EAX_ListenerProperties, prop, 0, value, valuesize );
	}
}


} // NLSOUND
