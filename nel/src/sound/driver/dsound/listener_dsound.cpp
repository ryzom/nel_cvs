/** \file listener_dsound.cpp
 * DirectSound listener
 *
 * $Id: listener_dsound.cpp,v 1.2 2002/05/27 09:35:57 hanappe Exp $
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

#include "stddsound.h"

#include "listener_dsound.h"
#include "sound_driver_dsound.h"


using namespace NLMISC;


namespace NLSOUND {


// The instance of the singleton
CListenerDSound	*CListenerDSound::_Instance = NULL;


/*
 * Constructor
 */
CListenerDSound::CListenerDSound(LPDIRECTSOUND3DLISTENER dsoundListener) //: IListener()
{
	if ( _Instance == NULL )
	{
		_Instance = this;
        _Listener = dsoundListener;
	}
	else
	{
		//nlerror( "Listener singleton instanciated twice" );
	}
}


CListenerDSound::~CListenerDSound()
{
    if (_Listener != NULL)
    {
        _Listener->Release();
    }

	_Instance = NULL;
}


/*
 * Set the position vector (default: (0,0,0)) (3D mode only)
 */
void CListenerDSound::setPos( const NLMISC::CVector& pos )
{
	// Coordinate system: conversion from NeL to OpenAL/GL:
    if (_Listener != NULL)
    {
        _Listener->SetPosition(pos.x, pos.z, -pos.y, DS3D_IMMEDIATE);
    }
}


/** Get the position vector.
 * See setPos() for details.
 */
void CListenerDSound::getPos( NLMISC::CVector& pos ) const
{
	// Coordinate system: conversion from NeL to OpenAL/GL:
    if (_Listener != NULL)
	{
		D3DVECTOR v;
        _Listener->GetPosition(&v);
		pos.set(v.x, -v.z, v.y);
    }
	else
	{
		pos.set(0, 0, 0);
	}
}


/*
 * Set the velocity vector (3D mode only)
 */
void CListenerDSound::setVelocity( const NLMISC::CVector& vel )
{
    if (_Listener != NULL)
    {
        _Listener->SetVelocity(vel.x, vel.z, -vel.y, DS3D_IMMEDIATE);
    }
}


/*
 * Get the velocity vector
 */
void CListenerDSound::getVelocity( NLMISC::CVector& vel ) const
{
    if (_Listener != NULL)
	{
		D3DVECTOR v;
        _Listener->GetVelocity(&v);
		vel.set(v.x, -v.z, v.y);
    }
	else
	{
		vel.set(0, 0, 0);	
	}
}


/*
 * Set the orientation vectors (3D mode only)
 */
void CListenerDSound::setOrientation( const NLMISC::CVector& front, const NLMISC::CVector& up )
{
    if (_Listener != NULL)
    {
        _Listener->SetOrientation(front.x, front.z, -front.y, up.x, up.z, -up.y, DS3D_IMMEDIATE);
    }
}


/*
 * Get the orientation vectors
 */
void CListenerDSound::getOrientation( NLMISC::CVector& front, NLMISC::CVector& up ) const
{
    if (_Listener != NULL)
	{
		D3DVECTOR vfront, vtop;
        _Listener->GetOrientation(&vfront, &vtop);
		front.set(vfront.x, -vfront.z, vfront.y);
		up.set(vtop.x, -vtop.z, vtop.y);
    }
	else
	{
		front.set(0, -1, 0);	
		up.set(0, 0, 1);	
	}
}


/* Set the gain (volume value inside [0 , 1]). (default: 1)
 * 0.0 -> silence
 * 0.5 -> -6dB
 * 1.0 -> no attenuation
 * values > 1 (amplification) not supported by most drivers
 */
void CListenerDSound::setGain( float gain )
{
	CSoundDriverDSound::instance()->setGain(gain);
}


/*
 * Get the gain
 */
float CListenerDSound::getGain() const
{
    return CSoundDriverDSound::instance()->getGain();
}


/*
 * Set the doppler factor (default: 1) to exaggerate or not the doppler effect
 */
void CListenerDSound::setDopplerFactor( float f )
{
    if (_Listener != NULL)
    {
        _Listener->SetDopplerFactor(f, DS3D_IMMEDIATE);
    }
}


/*
 * Set the rolloff factor (default: 1) to scale the distance attenuation effect
 */
void CListenerDSound::setRolloffFactor( float f )
{
    if (_Listener != NULL)
    {
        _Listener->SetRolloffFactor(f, DS3D_IMMEDIATE);
    }
}


/*
 * Set DSPROPERTY_EAXLISTENER_ENVIRONMENT and DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE if EAX available (see EAX listener properties)
 */
void CListenerDSound::setEnvironment( uint env, float size )
{
#ifdef EAX_AVAILABLE
	if ( EAXSetProp != NULL )
	{
		EAXSetProp( &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &env, sizeof(unsigned long) );
		EAXSetProp( &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE, 0, &size, sizeof(float) );
	}
#endif
}


/*
 * Set any EAX listener property if EAX available
 */
void CListenerDSound::setEAXProperty( uint prop, void *value, uint valuesize )
{
#ifdef EAX_AVAILABLE
	if ( EAXSetProp != NULL )
	{
		EAXSetProp( &DSPROPSETID_EAX_ListenerProperties, prop, 0, value, valuesize );
	}
#endif
}


} // NLSOUND
