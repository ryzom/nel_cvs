/** \file source_dsound.cpp
 * DirectSound sound source
 *
 * $Id: source_dsound.cpp,v 1.2 2002/05/27 09:35:57 hanappe Exp $
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
#include "source_dsound.h"
#include "sound_driver_dsound.h"
#include "buffer_dsound.h"
#include "sound_exception.h"



using namespace NLMISC;


namespace NLSOUND {


#if NLSOUND_PROFILE

	#define INITTIME(_var)   TTicks _var = CTime::getPerformanceTime()

	#define DEBUG_POSITIONS  0

	#if DEBUG_POSITIONS
	#define DBGPOS(_a) nldebug ## _a
	#else
	#define DBGPOS(_a) 
	#endif

#else
	#define INITTIME(_var)
	#define DBGPOS(_a) 
#endif



uint32 CSourceDSound::_SecondaryBufferSize = 65536;
uint32 CSourceDSound::_SwapCopySize = 32768;
uint32 CSourceDSound::_UpdateCopySize = 16384;
uint32 CSourceDSound::_XFadeSize = 64;
uint CSourceDSound::_Channels = 1;
uint CSourceDSound::_SampleRate = 22050;
uint CSourceDSound::_SampleSize = 16;



#define NLSOUND_MIN(_a,_b)    (((_a) < (_b)) ? (_a) : (_b))
#define NLSOUND_DISTANCE(_from, _to, _period)   (((_to) > (_from)) ? (_to) - (_from) : (_period) + (_to) - (_from))


#if NLSOUND_PROFILE

// Static variables used for profiling
double CSourceDSound::_LastSwapTime = 0.0;
double CSourceDSound::_TotalSwapTime = 0.0;
double CSourceDSound::_MaxSwapTime = 0.0;
double CSourceDSound::_MinSwapTime = 1000000.0;
uint32 CSourceDSound::_SwapCount = 0;
double CSourceDSound::_PosTime = 0.0;
double CSourceDSound::_LockTime = 0.0;
double CSourceDSound::_CopyTime = 0.0;
double CSourceDSound::_UnlockTime = 0.0;
uint32 CSourceDSound::_CopyCount = 0;
double CSourceDSound::_TotalUpdateTime = 0.0;
double CSourceDSound::_MaxUpdateTime = 0.0;
double CSourceDSound::_MinUpdateTime = 1000000.0;
uint32 CSourceDSound::_UpdateCount = 0;
uint32 CSourceDSound::_TotalUpdateSize = 0;
#endif



/*
 * Constructor
 */
CSourceDSound::CSourceDSound( uint sourcename ) : _SourceName(sourcename)
{
    _Buffer = 0;
    _BufferSize = 0;
	_SwapBuffer = 0;
    _SecondaryBuffer = 0;
	_SecondaryBufferState = NL_DSOUND_SILENCED;
    _3DBuffer = NULL;
    _NextWritePos = 0;
    _BytesWritten = 0;
    _SilenceWritten = 0;
    _Loop = false;
    _EndPosition = 0;
	_EndState = NL_DSOUND_TAIL1;
	_UserState = NL_DSOUND_STOPPED;
    InitializeCriticalSection(&_CriticalSection);
}


/*
 * Destructor
 */
CSourceDSound::~CSourceDSound()
{
    if (_3DBuffer != NULL)
    {
        _3DBuffer->Release();
    }

    if (_SecondaryBuffer != NULL)
    {
        _SecondaryBuffer->Stop();
        _SecondaryBuffer->Release();
    }

    DeleteCriticalSection(&_CriticalSection);
}


/*
 * Initilize the DirectSound buffers
 */
void CSourceDSound::init(LPDIRECTSOUND directSound)
{

    // Initialize the buffer format
    WAVEFORMATEX format;

    format.cbSize = sizeof(WAVEFORMATEX);
    format.nChannels = _Channels;
    format.wBitsPerSample = _SampleSize;
    format.nSamplesPerSec = _SampleRate;
    format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.wFormatTag = WAVE_FORMAT_PCM;


    // Initialize the buffer description 

    DSBUFFERDESC desc;

	CSoundDriverDSound* driver = CSoundDriverDSound::instance();


    ZeroMemory(&desc, sizeof(DSBUFFERDESC));
    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.lpwfxFormat = &format;
    desc.dwBufferBytes = _SecondaryBufferSize;  
    desc.dwReserved = 0; 

	if (driver->countHw3DBuffers() > 0)
	{
		nldebug("Source: Allocating 3D buffer in hardware");
		desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 
						| DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
	} 
	else
	{
 		nldebug("Source: Allocating 3D buffer in software");
		desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 
						| DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
        desc.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
	}


    // Allocate the secondary buffer
 
    if (FAILED(directSound->CreateSoundBuffer(&desc, &_SecondaryBuffer, NULL))) 
    {
		nlwarning("Source: Failed to create a buffer with 3D capabilities.");

		ZeroMemory(&desc, sizeof(DSBUFFERDESC));
		desc.dwSize = sizeof(DSBUFFERDESC);
		desc.lpwfxFormat = &format;
		desc.dwBufferBytes = _SecondaryBufferSize;  
		desc.dwReserved = 0; 

		if (driver->countHw2DBuffers() > 0)
		{
			nldebug("Source: Allocating 2D buffer in hardware");
			desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME 
							| DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
		} 
		else
		{
 			nldebug("Source: Allocating 2D buffer in software");
			desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME 
							| DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
		}

		if (FAILED(directSound->CreateSoundBuffer(&desc, &_SecondaryBuffer, NULL))) 
		{
			throw ESoundDriver("Failed to allocate the DirectSound secondary buffer");
		}
	}


    // Fill the buffer with silence

    LPVOID ptr;
    DWORD bytes;

    if (FAILED(_SecondaryBuffer->Lock(0, 0, &ptr, &bytes, NULL, NULL, DSBLOCK_ENTIREBUFFER)))
    {
        throw ESoundDriver("Failed to lock the DirectSound secondary buffer");
    }

    memset(ptr, 0, bytes);
    
    _SecondaryBuffer->Unlock(ptr, bytes, 0, 0);

    // Allocate the 3D interface, if necessary

    if (FAILED(_SecondaryBuffer->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID *) &_3DBuffer)))
    {
       throw ESoundDriver("Failed to allocate the DirectSound 3D buffer");
    }


    if (FAILED(_SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING)))
    {
        throw ESoundDriver("Play failed");
    }

}

/* Set the buffer that will be played (no streaming)
 * If the buffer is stereo, the source mode becomes stereo and the source relative mode is on,
 * otherwise the source is considered as a 3D source.
 */

void CSourceDSound::setStaticBuffer( IBuffer *buffer )
{
	_SwapBuffer = buffer;
}



/*
 * Set looping on/off for future playbacks (default: off)
 */
void CSourceDSound::setLooping( bool l )
{
    _Loop = l;
}


/*
 * Return the looping state
 */
bool CSourceDSound::getLooping() const
{
    return _Loop;
}


/*
 * Play the static buffer (or stream in and play)
 */
void CSourceDSound::play()
{
    EnterCriticalSection(&_CriticalSection); 


	if (_SwapBuffer != 0)
	{

		_UserState = NL_DSOUND_PLAYING;
		DBGPOS(("PLAY: PLAYING"));

		if (_SecondaryBufferState == NL_DSOUND_SILENCED)
		{
			_Buffer = _SwapBuffer;
			_BufferSize = _Buffer->getSize();
			_SwapBuffer = 0;
			_BytesWritten = 0;

			fadeIn();
		}
		else 
		{
			crossFade();
		}
	}
	else if ((_UserState == NL_DSOUND_PAUSED) || (_UserState == NL_DSOUND_STOPPED))
	{
		_UserState = NL_DSOUND_PLAYING;
		DBGPOS(("PLAY: PLAYING"));

		fadeIn();
	}


    LeaveCriticalSection(&_CriticalSection); 
}


/*
 * Stop playing
 */
void CSourceDSound::stop()
{
    EnterCriticalSection(&_CriticalSection); 

	TSourceDSoundUserState old = _UserState;

	_UserState = NL_DSOUND_STOPPED;
	DBGPOS(("STOP: STOPPED"));

	if (old == NL_DSOUND_PLAYING)
	{
		fadeOut();
	}

	_BytesWritten = 0;

	LeaveCriticalSection(&_CriticalSection); 
}


/*
 * Pause. Call play() to resume.
 */
void CSourceDSound::pause()
{
    EnterCriticalSection(&_CriticalSection); 

	TSourceDSoundUserState old = _UserState;

	_UserState = NL_DSOUND_PAUSED;
	DBGPOS(("PAUZ: PAUSED"));

	if (old == NL_DSOUND_PLAYING)
	{
		fadeOut();
	}

    LeaveCriticalSection(&_CriticalSection); 
}


/*
 * Return the playing state
 */
bool CSourceDSound::isPlaying() const
{
	return (_UserState == NL_DSOUND_PLAYING);
}


/*
 * Return the paused state
 */
bool CSourceDSound::isPaused() const
{
	return (_UserState == NL_DSOUND_PAUSED);
}


/*
 * Return true if playing is finished or stop() has been called.
 */
bool CSourceDSound::isStopped() const
{
    return (_UserState == NL_DSOUND_STOPPED);
}


/*
 * Returns true is the play position has advanced enough to require an update
 */
bool CSourceDSound::needsUpdate()
{
    DWORD playPos, writePos;
	uint32 space;
	
	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

		
    if (playPos == _NextWritePos)
    {
        return false;
    }
    else if (playPos > _NextWritePos) 
    {
        space = playPos - _NextWritePos;
    }
    else
    {
        space = _SecondaryBufferSize + playPos - _NextWritePos;
    }
    
    // Don't bother if the number of samples is smaller than the copy size.
    return (space > _UpdateCopySize);
}



/*
 * Update the source (e.g. continue to stream the data in)
 */
void CSourceDSound::update()
{
	update2();
}

/*
 * Update the source (e.g. continue to stream the data in)
 * Returns an indication whether the source actually streamed data or not.
 */
bool CSourceDSound::update2()
{
	bool res = false;



	INITTIME(start);

    //
    // Enter critical region
    //
    EnterCriticalSection(&_CriticalSection);



	switch (_SecondaryBufferState)
    {


	case NL_DSOUND_SILENCED:
		{
			// Just pretend were writing silence by advancing the next 
			// write position.
		    _NextWritePos += _UpdateCopySize;
			if (_NextWritePos >= _SecondaryBufferSize)
			{
				_NextWritePos  -= _SecondaryBufferSize;
			}
		}
		break;


	case NL_DSOUND_FILLING:
		res = fill();	
		break;


	case NL_DSOUND_SILENCING:
		res = silence();	
		break;

	}


    //
    // Leave critical region
    //
    LeaveCriticalSection(&_CriticalSection); 


#if NLSOUND_PROFILE
    double dt = CTime::ticksToSecond(CTime::getPerformanceTime() - start);;
    _TotalUpdateTime += dt;
    _MaxUpdateTime = (dt > _MaxUpdateTime) ? dt : _MaxUpdateTime;
    _MinUpdateTime = (dt < _MinUpdateTime) ? dt : _MinUpdateTime;
    _UpdateCount++;
#endif

	return res;
}



/* Set the position vector.
 * 3D mode -> 3D position
 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
 */
void CSourceDSound::setPos( const NLMISC::CVector& pos )
{
	// Coordinate system: conversion from NeL to OpenAL/GL:
    if (_3DBuffer != NULL)
    {
		if (_3DBuffer->SetPosition(pos.x, pos.z, -pos.y, DS3D_IMMEDIATE) != DS_OK)
		{
			nldebug("SetPosition failed");
		}
    }
}


/* Get the position vector.
 * See setPos() for details.
 */
void CSourceDSound::getPos( NLMISC::CVector& pos ) const
{
	// Coordinate system: conversion from NeL to OpenAL/GL:
    if (_3DBuffer != NULL)
	{
		D3DVECTOR v;
        HRESULT hr = _3DBuffer->GetPosition(&v);

		if (hr != DS_OK)
		{
			nldebug("GetPosition failed");
			pos.set(0, 0, 0);	
		}
		else
		{
			pos.set(v.x, -v.z, v.y);
		}
    }
	else
	{
		pos.set(0, 0, 0);	
	}
}


/*
 * Set the velocity vector (3D mode only)
 */
void CSourceDSound::setVelocity( const NLMISC::CVector& vel )
{
    if (_3DBuffer != NULL)
    {
		if (_3DBuffer->SetVelocity(vel.x, vel.z, -vel.y, DS3D_IMMEDIATE) != DS_OK)
		{
			nldebug("SetVelocity failed");
		}
    }
}


/*
 * Get the velocity vector
 */
void CSourceDSound::getVelocity( NLMISC::CVector& vel ) const
{
    if (_3DBuffer != NULL)
	{
		D3DVECTOR v;

		if (_3DBuffer->GetVelocity(&v) != DS_OK)
		{
			nldebug("GetVelocity failed");
			vel.set(0, 0, 0);	
		}
		else
		{
			vel.set(v.x, -v.z, v.y);
		}
    }
	else
	{
		vel.set(0, 0, 0);	
	}
}


/*
 * Set the direction vector (3D mode only)
 */
void CSourceDSound::setDirection( const NLMISC::CVector& dir )
{
	if (_3DBuffer != 0)
	{
		if (_3DBuffer->SetConeOrientation(dir.x, dir.z, -dir.y, DS3D_IMMEDIATE) != DS_OK)
		{
			nldebug("SetConeOrientation failed");
		}
	}
}


/*
 * Get the direction vector
 */
void CSourceDSound::getDirection( NLMISC::CVector& dir ) const
{
    if (_3DBuffer != NULL)
	{
		D3DVECTOR v;

		if (_3DBuffer->GetConeOrientation(&v) != DS_OK)
		{
			nldebug("GetConeOrientation failed");
			dir.set(0, 1, 0);	
		}
		else
		{
			dir.set(v.x, -v.z, v.y);
		}
    }
	else
	{
		dir.set(0, 1, 0);	
	}
}


/* Set the gain (volume value inside [0 , 1]).
 * 0.0 -> silence
 * 0.5 -> -6dB
 * 1.0 -> no attenuation
 * values > 1 (amplification) not supported by most drivers
 */
void CSourceDSound::setGain( float gain )
{
	if (_SecondaryBuffer != 0)
	{
		if (gain < 0.00001f)
		{
			gain = 0.00001f;
		}

		/* convert from linear amplitude to hundredths of decibels */
		LONG volume = (LONG)(100.0 * 20.0 * log10(gain));

		if (volume < DSBVOLUME_MIN) 
		{
			volume = DSBVOLUME_MIN;
		}
		else if (volume > DSBVOLUME_MAX) 
		{
			volume = DSBVOLUME_MAX;
		}

		if (_SecondaryBuffer->SetVolume(volume) != DS_OK)
		{
			nldebug("SetVolume failed");
		}
	}
}


/*
 * Get the gain
 */
float CSourceDSound::getGain() const
{
	if (_SecondaryBuffer != 0)
	{
		/* convert from hundredths of decibels to linear amplitude */
		LONG volume;

		if (_SecondaryBuffer->GetVolume(&volume) != DS_OK)
		{
			nldebug("GetVolume failed");
			return 1.0;
		}

		return (float) pow(10, (double) volume / 20.0 / 100.0);
	}

	return 1.0;
}


/* Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
 * of one octave. 0 is not a legal value.
 */
void CSourceDSound::setPitch( float coeff )
{
	if ((_Buffer != 0) && (_SecondaryBuffer != 0))
	{
		TSampleFormat format;
		uint freq;

		_Buffer->getFormat(format, freq);

		if (_SecondaryBuffer->SetFrequency((DWORD)(coeff * (float) freq)) != DS_OK)
		{
			nldebug("SetFrequency failed");
		}
	}
}


/*
 * Get the pitch
 */
float CSourceDSound::getPitch() const
{
	if ((_Buffer != 0) && (_SecondaryBuffer != 0))
	{
		TSampleFormat format;
		uint freq0;
		DWORD freq;

		_Buffer->getFormat(format, freq0);

		if (_SecondaryBuffer->GetFrequency(&freq) != DS_OK)
		{
			nldebug("GetFrequency failed");
			return 1.0;
		}

		return ((float) freq / (float) freq0);
	}

	return 1.0;
}


/*
 * Set the source relative mode. If true, positions are interpreted relative to the listener position.
 */
void CSourceDSound::setSourceRelativeMode( bool mode )
{
	if (_3DBuffer != 0)
	{
		HRESULT hr;

		if (mode)
		{
			hr = _3DBuffer->SetMode(DS3DMODE_HEADRELATIVE, DS3D_IMMEDIATE);
		}
		else
		{
			hr = _3DBuffer->SetMode(DS3DMODE_NORMAL, DS3D_IMMEDIATE);
		}

		if (hr != DS_OK)
		{
			nldebug("SetMode failed");
		}
	}
	else
	{
		nldebug("Requested setSourceRelativeMode on a non-3D source");
	}
}


/*
 * Get the source relative mode (3D mode only)
 */
bool CSourceDSound::getSourceRelativeMode() const
{
	if (_3DBuffer != 0)
	{
		DWORD mode;

		if (_3DBuffer->GetMode(&mode) != DS_OK)
		{
			nldebug("GetMode failed");
			return false;
		}

		return (mode == DS3DMODE_HEADRELATIVE);
	}
	else
	{
		nldebug("Requested setSourceRelativeMode on a non-3D source");
		return false;
	}
}


/*
 * Set the min and max distances (3D mode only)
 */
void CSourceDSound::setMinMaxDistances( float mindist, float maxdist )
{
	if (_3DBuffer != 0)
	{
		if ((_3DBuffer->SetMinDistance(mindist, DS3D_IMMEDIATE) != DS_OK)
			|| (_3DBuffer->SetMaxDistance(maxdist, DS3D_IMMEDIATE) != DS_OK))
		{
			nldebug("SetMinDistance or SetMaxDistance failed");
		}
	}
	else
	{
		nldebug("Requested setMinMaxDistances on a non-3D source");
	}
}


/*
 * Get the min and max distances
 */
void CSourceDSound::getMinMaxDistances( float& mindist, float& maxdist ) const
{
	if (_3DBuffer != 0)
	{
		D3DVALUE min, max;

		if ((_3DBuffer->GetMinDistance(&min) != DS_OK)
			|| (_3DBuffer->GetMaxDistance(&max) != DS_OK))
		{
			mindist = 0.0f;
			maxdist = 0.0f;
			nldebug("GetMinDistance or GetMaxDistance failed");
		}
		else
		{
			mindist = min;
			maxdist = max;
		}
	}
	else
	{
		mindist = 0.0f;
		maxdist = 0.0f;
		nldebug("Requested getMinMaxDistances on a non-3D source");
	}
}


/*
 * Set the cone angles (in radian) and gain (in [0 , 1]) (3D mode only)
 */
void CSourceDSound::setCone( float innerAngle, float outerAngle, float outerGain )
{
	if (_3DBuffer != 0)
	{
		// Set the cone angles

		// Convert from radians to degrees
		DWORD inner = (DWORD)(Pi * innerAngle / 180.0);
		DWORD outer = (DWORD)(Pi * outerAngle / 180.0);

		// Sanity check: wrap the angles in the [0,360] interval
		while (inner < DS3D_MINCONEANGLE) 
		{
			inner += 360;
		}

		while (inner > DS3D_MAXCONEANGLE)
		{
			inner -= 360;
		}

		while (outer < DS3D_MINCONEANGLE) 
		{
			outer += 360;
		}

		while (outer > DS3D_MAXCONEANGLE)
		{
			outer -= 360;
		}

		if (_3DBuffer->SetConeAngles(inner, outer, DS3D_IMMEDIATE) != DS_OK)
		{
			nldebug("SetConeAngles failed");			
		}

		// Set the outside volume
		if (outerGain < 0.00001f)
		{
			outerGain = 0.00001f;
		}

		// convert from linear amplitude to hundredths of decibels 
		LONG volume = (LONG)(100.0 * 20.0 * log10(outerGain));

		if (volume < DSBVOLUME_MIN) 
		{
			volume = DSBVOLUME_MIN;
		}
		else if (volume > DSBVOLUME_MAX) 
		{
			volume = DSBVOLUME_MAX;
		}

		if (_3DBuffer->SetConeOutsideVolume(volume, DS3D_IMMEDIATE) != DS_OK)
		{
			nldebug("SetConeOutsideVolume failed");			
		}

	}
	else
	{
		nldebug("Requested setCone on a non-3D source");
	}
}


/*
 * Get the cone angles (in radian)
 */
void CSourceDSound::getCone( float& innerAngle, float& outerAngle, float& outerGain ) const
{
	if (_3DBuffer != 0)
	{
		DWORD inner, outer;
		LONG volume;

		if (_3DBuffer->GetConeAngles(&inner, &outer) != DS_OK)
		{
			nldebug("GetConeAngles failed");			
			innerAngle = outerAngle = (float)(2.0 * Pi);
		}
		else
		{
			innerAngle = (float)(Pi * inner / 180.0);
			outerAngle = (float)(Pi * outer / 180.0);
		}

		if (_3DBuffer->GetConeOutsideVolume(&volume) != DS_OK)
		{
			nldebug("GetConeOutsideVolume failed");			
			outerGain = 0.0f;
		}
		else
		{
			outerGain = (float) pow(10, (double) volume / 20.0 / 100.0);
		}
	}
	else
	{
		nldebug("Requested getCone on a non-3D source");
	}
}


/*
 * Set any EAX source property if EAX available
 */
void CSourceDSound::setEAXProperty( uint prop, void *value, uint valuesize )
{
#ifdef EAX_AVAILABLE
	if ( EAXSetProp != NULL )
	{
		EAXSetProp( &DSPROPSETID_EAX_SourceProperties, prop, _SourceName, value, valuesize );
	}
#endif
}


/*
 * Return the sound buffer
 */
IBuffer *CSourceDSound::getBuffer()
{   
    return _Buffer;
}


/*
 *  lock
 */
bool CSourceDSound::lock(uint32 writePos, uint32 size, uint8* &ptr1, DWORD &bytes1, uint8* &ptr2, DWORD &bytes2)
{
	HRESULT hr = _SecondaryBuffer->Lock(writePos, size, (LPVOID*) &ptr1, &bytes1, (LPVOID*) &ptr2, &bytes2, 0);

	if (hr == DSERR_BUFFERLOST)
	{
		// If the buffer got lost somehow, try to restore it.
		if (FAILED(_SecondaryBuffer->Restore()))
		{
			nlwarning("Lock failed (1)");
			return false;
		}
		if (FAILED(_SecondaryBuffer->Lock(_NextWritePos, _UpdateCopySize, (LPVOID*) &ptr1, &bytes1, (LPVOID*) &ptr2, &bytes2, 0)))
		{
			nlwarning("Lock failed (2)");
			return false;
		}
	} 
	else if (hr != DS_OK)
	{
		nlwarning("Lock failed (3)");
		return false;
	}

	return true;
}

/*
 *  unlock
 */
bool CSourceDSound::unlock(uint8* ptr1, DWORD bytes1, uint8* ptr2, DWORD bytes2)
{
	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);
	return true;
}



/***************************************************************************
 

  Buffer operations

  There are five buffer operation: fill, silence, fadeOut, crossFade
  and fadeIn. fill and silence are called by the update function. The
  others are called by the user state functions (play, stop, pause).


            NW     P   W
   +--------+------+---+-----------------------+
   |........|      |xxx|.......................|
   +--------+------+---+-----------------------+

  The source maintains the next write position (_NextWritePos, NW in figure
  above). That is the position at which new samples or silemce is written.
  DirectSound maintaines a play cursor and a write cursor (P and W in figure).
  The data between P and W is scheduled for playing and cannot be touched.
  The data between W and NW are unplayed sample data that the source copied
  into the DirectSound buffer.

  The update functions (fill, silence) refresh the buffer with new samples
  or silence. That insert the sample data at the next write position NW. This 
  write position is maintained as closely behind the DirectSound play cursor 
  as possible to keep the buffer filled with data.

  The user functions (the fades) modify the sample data that is right after
  the write cursor W maintained by DirectSound. The data has to be written 
  after W to hear the changes as soon as possible. When a fade is done, the
  date already written in the buffer has to be overwritten. The function
  getFadeOutSize() helps to found out how many samples are writen between
  W and NW and to what section of the original audio data they correspond.

  All the buffer functions below have the same pattern:

	- get current play and write cursors (P and W)
	- lock the buffer
	- copy samples
	- unlock buffer
	- update state variables

  The differences between the functions are due to different operation
  (fades), position and size of the buffer to lock, handling of silence
  and looping.

  Enjoy!

  PH


************************************************************************/

bool CSourceDSound::fill()
{
	bool res = false;
    uint8 *ptr1, *ptr2;
    DWORD bytes1, bytes2;
    DWORD playPos, writePos;
    uint32 space;


	if (_Buffer == NULL)
	{
		_SecondaryBufferState = NL_DSOUND_SILENCING;
		_UserState = NL_DSOUND_STOPPED;
		return false;
	}

	INITTIME(startPos);


	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

	if (playPos == _NextWritePos)
	{
		return false;
	}
	else if (playPos > _NextWritePos) 
	{
		space = playPos - _NextWritePos;
	}
	else
	{
		space = _SecondaryBufferSize + playPos - _NextWritePos;
	}

	// Don't bother if the number of samples that can be written is too small.
	if (space < _UpdateCopySize)
	{
		return false;
	}


	uint8* data = ((CBufferDSound*) _Buffer)->getData();
	uint32 available = (_BytesWritten < _BufferSize) ? _BufferSize - _BytesWritten : 0;


	// The number of samples bytes that will be copied. If bytes is 0
	// than write silence to the buffer.
	uint32 bytes = NLSOUND_MIN(_UpdateCopySize, available);
	uint32 clear = _UpdateCopySize - available;


	// Lock the buffer

	INITTIME(startLock);


	if (!lock(_NextWritePos, _UpdateCopySize, ptr1, bytes1, ptr2, bytes2))
	{
		return false;
	}


	INITTIME(startCopy);

	// Start copying the samples

	if (bytes1 <= bytes) {

		CFastMem::memcpy(ptr1, data + _BytesWritten, bytes1);
		_BytesWritten += bytes1;
		bytes -= bytes1;

		if (ptr2)
		{
			if (bytes > 0)
			{
				CFastMem::memcpy(ptr2, data + _BytesWritten, bytes);                    
				_BytesWritten += bytes;
			}

			if (bytes < bytes2)
			{
				if (_Loop)
				{
					DBGPOS(("FILL: LOOP"));

					CFastMem::memcpy(ptr2 + bytes, data, bytes2 - bytes); 
					_BytesWritten = bytes2 - bytes;
				}
				else
				{
					memset(ptr2 + bytes, 0, bytes2 - bytes); 
					_SilenceWritten = bytes2 - bytes;
				}
			}
		}
	}
	else
	{
		if (bytes > 0)
		{
			CFastMem::memcpy(ptr1, data + _BytesWritten, bytes);
			_BytesWritten += bytes;
		}

		if (_Loop)
		{
			DBGPOS(("FILL: LOOP"));

			CFastMem::memcpy(ptr1 + bytes, data, bytes1 - bytes);                    
			_BytesWritten = bytes1 - bytes;

			if (ptr2)
			{
				CFastMem::memcpy(ptr2, data + _BytesWritten, bytes2);                    
				_BytesWritten += bytes2;
			}

		} 
		else
		{
			memset(ptr1 + bytes, 0, bytes1 - bytes);                    
			_SilenceWritten = bytes1 - bytes;

			if (ptr2)
			{
				memset(ptr2, 0, bytes2);                    
				_SilenceWritten += bytes2;
			}
		}

	}




	INITTIME(startUnlock);

	// Unlock the buffer
	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);


	// Update the state variables

	// Check if we've reached the end of the file
	if (_BytesWritten == _BufferSize) 
	{
		if (_Loop)
		{
			// If we're looping, start all over again
			DBGPOS(("FILL: LOOP"));
			_BytesWritten = 0;
		}
		else
		{
			_SecondaryBufferState = NL_DSOUND_SILENCING;

			// Keep track of where tha last sample was written and the position
			// of the play cursor relative to the end position. if the _EndState
			// is 0, the play cursor is after the end position, 1 otherwise.
			_EndPosition = writePos + bytes;
			if (_EndPosition >= _SecondaryBufferSize)
			{
				_EndPosition -= _SecondaryBufferSize;
			}

			_EndState = (playPos > _EndPosition)? NL_DSOUND_TAIL1 : NL_DSOUND_TAIL2;

			DBGPOS(("FILL: SILENCING"));
			DBGPOS(("FILL: ENDSTATE=%d, E=%d, P=%d", (int) _EndState, _EndPosition, playPos));
		}
	}


	// Update the write pointer
	_NextWritePos += bytes1 + bytes2;
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos  -= _SecondaryBufferSize;
	}
 
	DBGPOS(("FILL: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


#if NLSOUND_PROFILE
	_TotalUpdateSize += bytes1 + bytes2;
	_PosTime += CTime::ticksToSecond(startLock - startPos);
	_LockTime += CTime::ticksToSecond(startCopy - startLock);
	_CopyTime += CTime::ticksToSecond(startUnlock - startCopy);
	_UnlockTime += CTime::ticksToSecond(CTime::getPerformanceTime() - startUnlock);
	_CopyCount++;
#endif


	return true;
}




/** 
 *   Fill the buffer with new silence
 */

bool CSourceDSound::silence()
{
    uint8 *ptr1, *ptr2;
    DWORD bytes1, bytes2;
    DWORD playPos, writePos;
    uint32 space;

		
	INITTIME(startPos);

	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

	if (playPos == _NextWritePos)
	{
		return false;
	}
	else if (playPos > _NextWritePos) 
	{
		space = playPos - _NextWritePos;
	}
	else
	{
		space = _SecondaryBufferSize + playPos - _NextWritePos;
	}

	// Don't bother if the number of samples that can be written is too small.
	if (space < _UpdateCopySize)
	{
		return false;
	}

		// Lock the buffer

	INITTIME(startLock);

	if (!lock(_NextWritePos, _UpdateCopySize, ptr1, bytes1, ptr2, bytes2))
	{
		return false;
	}


	INITTIME(startCopy);
   
	// Silence the buffer
	memset(ptr1, 0, bytes1);
	memset(ptr2, 0, bytes2);

		// Unlock the buffer

	INITTIME(startUnlock);

	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);


	// Update the next write position
	_NextWritePos += bytes1 + bytes2;
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos  -= _SecondaryBufferSize;
	}


	// Check if all the samples in the buffer are played
	if ((playPos > _EndPosition) && (_EndState == NL_DSOUND_TAIL2)) 
	{
		// The buffer played passed the last sample. Flag the source as stopped.
		_EndState = NL_DSOUND_ENDED;
		DBGPOS(("SLNC: ENDED"));

		// If the buffer was playing, mark it as stopped
		if (_UserState == NL_DSOUND_PLAYING)
		{
			_UserState = NL_DSOUND_STOPPED;
			DBGPOS(("SLNC: STOPPED"));
		}
	}
	else if ((playPos < _EndPosition) && (_EndState == NL_DSOUND_TAIL1))
	{
		// The play cursor wrapped around the buffer and is now before the end position
		_EndState = NL_DSOUND_TAIL2;
		DBGPOS(("FILL: ENDSTATE=%d, E=%d, P=%d", (int) _EndState, _EndPosition, playPos));
	}


	// Update the amount of silence written
	_SilenceWritten += bytes1 + bytes2;
	if (_SilenceWritten >= _SecondaryBufferSize)
	{
		// The buffer is now completely filled with silence
		_SecondaryBufferState = NL_DSOUND_SILENCED;
		DBGPOS(("SLNC: SILENCED"));

		// If the buffer was playing, mark it as stopped
		if (_UserState == NL_DSOUND_PLAYING)
		{
			_UserState = NL_DSOUND_STOPPED;
			DBGPOS(("SLNC: STOPPED*"));
		}
	}

	DBGPOS(("SLNC: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


#if NLSOUND_PROFILE
	_TotalUpdateSize += bytes1 + bytes2;
	_PosTime += CTime::ticksToSecond(startLock - startPos);
	_LockTime += CTime::ticksToSecond(startCopy - startLock);
	_CopyTime += CTime::ticksToSecond(startUnlock - startCopy);
	_UnlockTime += CTime::ticksToSecond(CTime::getPerformanceTime() - startUnlock);
	_CopyCount++;
#endif

	return true;
}



/** 
 *  Calculate the size of the crossfade and set the pointer in the sample buffer
 *  to the sample that comes after the write cursor in the DirectSound buffer.
 *  This method also returns the number of samples that have been written after
 *  the write cursor.
 *
 */

 void CSourceDSound::getFadeOutSize(uint32 writePos, uint32 &xfadeSize, sint16* &in1, uint32 &writtenTooMuch)
{
    // The number of samples over which we will do the crossfade
    xfadeSize = _XFadeSize;


    // The tricky part of this method is to figger out how many samples of the old
    // buffer were written after the write cursor and figger our with what position 
    // in the old buffer the write cursor corresponds. We have to consider the following
    // cases:
    //
    // - the old buffer just looped, 
    // - the old buffer is finished, but stil has samples in the DirectSound buffer
    // - the default case, i.e. the old buffer is playing somewhere in the middle.
    //

    // How many bytes did we write after the write position?

	writtenTooMuch = NLSOUND_DISTANCE(writePos, _NextWritePos, _SecondaryBufferSize);

    DBGPOS(("FADE: TOO=%d", writtenTooMuch));


	uint8* data = ((CBufferDSound*) _Buffer)->getData();

    // If the sound is finished and all the samples are written in the
    // buffer, it's possible that there are still samples after the write
    // position. If this is the case, we have to do a fade out. If there is
    // only silence, we only need to copy without fade.

    if (_SilenceWritten > 0)
    {
        
        if (writtenTooMuch > _SilenceWritten)
        {
            writtenTooMuch -= _SilenceWritten;
            in1 = (sint16*) (data + _BufferSize - writtenTooMuch) ;

            if (writtenTooMuch < 2 * xfadeSize)
            {
                xfadeSize = writtenTooMuch / 2;
            }
        }
        else
        {
            xfadeSize = 0;                            
        }

        DBGPOS(("FADE: END, TOO=%d, S=%d, F=%d", writtenTooMuch, _SilenceWritten, xfadeSize));
    }
    
    // If the sound looped, it's possible that the number of samples
    // written is small. In that case, the write cursor is still inside
    // the previous loop. All we have to do is fade out the last part
    // of the previous loop. 

	else if (writtenTooMuch >= _BytesWritten)
    {

        writtenTooMuch -= _BytesWritten;

        in1 = (sint16*) (data + _BufferSize - writtenTooMuch) ;

        if (writtenTooMuch < 2 * xfadeSize)
        {
            xfadeSize = writtenTooMuch / 2;
        }

        DBGPOS(("FADE: LOOPED, TOO=%d, F=%d", writtenTooMuch, xfadeSize));

    }

    // This is the default case. Simply fade from the previous to the next buffer.
    // The variable writtenTooMuch tells us how much of the previous sound has
    // been written after the current write cursor. We just have to check there
    // are enough samples available for the fade.

    else
    {
        in1 = (sint16*) (data + (sint32) _BytesWritten - writtenTooMuch);

        if (xfadeSize > _BufferSize - _BytesWritten)
        {
            xfadeSize = _BufferSize - _BytesWritten;
        }

        DBGPOS(("FADE: STD, TOO=%d, F=%d", writtenTooMuch, xfadeSize));
    }
}



/**
 *   Do a crossfade between the current buffer and the swap buffer.
 */

void CSourceDSound::crossFade()
{
    uint8 *ptr1, *ptr2;
    DWORD bytes1, bytes2;
    DWORD playPos, writePos;
    uint32 i;


	if (_Buffer == NULL)
	{
		return;
	}


    INITTIME(start); 
    

    EnterCriticalSection(&_CriticalSection); 



    // The source is currently playing an other buffer. We will do a hot
    // swap between the old and the new buffer. DirectSound maintains two
    // cursors into the buffer: the play cursor and the write cursor.
    // The write cursor indicates where we can start writing the new samples.
    // To avoid clicks, we have to do a cross fade between the old buffer and
    // the new buffer.
    
    _SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);


    // The number of bytes we will write to the DirectSound buffer
    uint32 bytes = _SwapCopySize;
    if (bytes > _SwapBuffer->getSize())
    {
        bytes = _SwapBuffer->getSize();
    }


    // Lock the DirectSound buffer

    if (FAILED(_SecondaryBuffer->Lock(writePos, bytes, (LPVOID*) &ptr1, &bytes1, (LPVOID*) &ptr2, &bytes2, 0)))
    {
        LeaveCriticalSection(&_CriticalSection);
        throw ESoundDriver("Failed to lock the DirectSound secondary buffer");
    }

    
    sint16* in1;
    uint8* data1 = ((CBufferDSound*) _Buffer)->getData();
    uint8* data2 = ((CBufferDSound*) _SwapBuffer)->getData();
    sint16* in2 = (sint16*) data2;
    sint16* out = (sint16*) ptr1;

	// The number of samples over which we will do the crossfade
	uint32 xfadeSize;
	uint32 xfadeByteSize;
	uint32 writtenTooMuch;

	getFadeOutSize(writePos, xfadeSize, in1, writtenTooMuch);
	xfadeByteSize = 2 * xfadeSize;


    float amp1 = 1.0f;
    float amp2 = 0.0f;
    float incr = 1.0f / xfadeSize;


    // Start copying the samples
    

    // In the first case, the cross fade is completely contained in the first buffer
    // pointed to by ptr1.
    if (xfadeByteSize < bytes1)
    {

        // Do cross fade

        for (i = 0; i < xfadeSize; i++) 
        {
            out[i] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
            amp1 -= incr;
            amp2 += incr;
        }

        // Copy remaining samples

        CFastMem::memcpy(ptr1 + xfadeByteSize, data2 + xfadeByteSize, bytes1 - xfadeByteSize);
        _BytesWritten = bytes1;

        if (ptr2)
        {
            CFastMem::memcpy(ptr2, data2 + _BytesWritten, bytes2);
            _BytesWritten += bytes2;
        }

    }

    // In the second case, the cross fade stretches over the first and the second buffers.
    else
    {

        uint32 fade1 = bytes1 / 2;
        uint32 fade2 = xfadeSize - fade1;

        // Do cross fade

        // Part 1, start at ptr1
        for (i = 0; i < fade1; i++) 
        {
            out[i] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
            amp1 -= incr;
            amp2 += incr;
        }
        _BytesWritten = bytes1;


        if (ptr2)
        {
            out = (sint16*) ptr2;

            // Part 2, ontinue at ptr2
            for (uint32 k = 0; i < xfadeSize; i++, k++) 
            {
                out[k] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
                amp1 -= incr;
                amp2 += incr;
            }

            // Copy remaining samples
            CFastMem::memcpy(ptr2 + 2 * k, data2 + _BytesWritten + 2 * k, bytes2 - 2 * k);
            _BytesWritten += bytes2;
        }

    }


    // Unlock the DirectSound buffer
    _SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);

    
	// Update the state variables

    _SilenceWritten = 0;

    _NextWritePos = (writePos + bytes1 + bytes2);
    if (_NextWritePos >= _SecondaryBufferSize)
    {
        _NextWritePos -= _SecondaryBufferSize;
    }



	_SecondaryBufferState = NL_DSOUND_FILLING;


	// Swap the two buffers
	_BufferSize = _SwapBuffer->getSize();
	_Buffer = _SwapBuffer;
	_SwapBuffer = 0;

	// Check if we've reached the end of the file
	if (_BytesWritten == _BufferSize) 
	{
		if (_Loop)
		{
			_BytesWritten = 0;
		}
		else
		{
			_SecondaryBufferState = NL_DSOUND_SILENCING;
		}
	}


    DBGPOS(("XFADE"));
    DBGPOS(("P=%d, W=%d, NW=%d, SZ=%d, BW=%d, B=%d", playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, bytes1 + bytes2));

	

    LeaveCriticalSection(&_CriticalSection);


#if NLSOUND_PROFILE
    _LastSwapTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
    _TotalSwapTime += _LastSwapTime;
    _MaxSwapTime = (_LastSwapTime > _MaxSwapTime) ? _LastSwapTime : _MaxSwapTime;
    _MinSwapTime = (_LastSwapTime < _MinSwapTime) ? _LastSwapTime : _MinSwapTime;
    _SwapCount++;
#endif

}




/**
 *   Fade out the current buffer
 */
void CSourceDSound::fadeOut()
{
    uint8 *ptr1, *ptr2;
    DWORD bytes1, bytes2;
    DWORD playPos, writePos;
    uint32 i;


	if (_Buffer == NULL)
	{
		_SecondaryBufferState = NL_DSOUND_SILENCING;
		_UserState = NL_DSOUND_STOPPED;
		return;
	}


    INITTIME(start);
    

    _SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);


    // Lock the DirectSound buffer

    if (FAILED(_SecondaryBuffer->Lock(writePos, _SwapCopySize, (LPVOID*) &ptr1, &bytes1, (LPVOID*) &ptr2, &bytes2, 0)))
    {
        throw ESoundDriver("Failed to lock the DirectSound secondary buffer");
    }

    

    // in1 points to the position in the old buffer where the fade out starts
	sint16* in1;
    sint16* out = (sint16*) ptr1;

    // The number of samples over which we will do the crossfade
    uint32 xfadeSize;
	uint32 xfadeByteSize;
	uint32 writtenTooMuch;

	getFadeOutSize(writePos, xfadeSize, in1, writtenTooMuch);
    xfadeByteSize = 2 * xfadeSize;

    float amp1 = 1.0f;
    float incr = 1.0f / xfadeSize;


	if (writtenTooMuch > _BytesWritten)
	{
		// The buffer looped. Count backwards from the end of the file.
		_BytesWritten = _BufferSize - writtenTooMuch;
	}
	else
	{
		_BytesWritten -= writtenTooMuch;
	}


    // Start copying the samples
    

    // In the first case, the fade out is completely contained in the first buffer
    // pointed to by ptr1.
    if (xfadeByteSize < bytes1)
    {

        // Do cross fade

        for (i = 0; i < xfadeSize; i++) 
        {
            out[i] = (sint16) (amp1 * in1[i]);
            amp1 -= incr;
        }

        // Copy remaining samples

        memset(ptr1 + xfadeByteSize, 0, bytes1 - xfadeByteSize);
        _SilenceWritten = bytes1 - xfadeByteSize;

        if (ptr2)
        {
            memset(ptr2, 0, bytes2);
            _SilenceWritten += bytes2;
        }

    }

    // In the second case, the fade out stretches over the first and the second buffers.
    else
    {

        uint32 fade1 = bytes1 / 2;
        uint32 fade2 = xfadeSize - fade1;

        // Do cross fade

        // Part 1, start at ptr1
        for (i = 0; i < fade1; i++) 
        {
            out[i] = (sint16) (amp1 * in1[i]);
            amp1 -= incr;
        }


        if (ptr2)
        {
            out = (sint16*) ptr2;

            // Part 2, continue at ptr2
            for (uint32 k = 0; i < xfadeSize; i++, k++) 
            {
                out[k] = (sint16) (amp1 * in1[i]);
                amp1 -= incr;
            }

            // Clear remaining samples
            memset(ptr2 + 2 * k, 0, bytes2 - 2 * k);
            _SilenceWritten = bytes2 - 2 * k;
        }

    }


    _BytesWritten += xfadeByteSize;


    // Unlock the DirectSound buffer
    _SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);

    
	// Update the next write position
    _NextWritePos = (writePos + bytes1 + bytes2);
    if (_NextWritePos >= _SecondaryBufferSize)
    {
        _NextWritePos -= _SecondaryBufferSize;
    }


	_SecondaryBufferState = NL_DSOUND_SILENCING;
	DBGPOS(("FDOU: SILENCING"));

	// Keep track of where tha last sample was written and the position
	// of the play cursor relative to the end position. if the _EndState
	// is 0, the play cursor is after the end position, 1 otherwise.
	_EndPosition = writePos + xfadeSize;
	if (_EndPosition >= _SecondaryBufferSize)
	{
		_EndPosition -= _SecondaryBufferSize;
	}

	_EndState = (playPos > _EndPosition)? NL_DSOUND_TAIL1 : NL_DSOUND_TAIL2;
	DBGPOS(("FDOU: ENDSTATE=%d, E=%d, P=%d", (int) _EndState, _EndPosition, playPos));


	DBGPOS(("FDOU: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));



#if NLSOUND_PROFILE
    _LastSwapTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
    _TotalSwapTime += _LastSwapTime;
    _MaxSwapTime = (_LastSwapTime > _MaxSwapTime) ? _LastSwapTime : _MaxSwapTime;
    _MinSwapTime = (_LastSwapTime < _MinSwapTime) ? _LastSwapTime : _MinSwapTime;
    _SwapCount++;
#endif

}



/**
 *   Fade in the current buffer
 */

void CSourceDSound::fadeIn()
{
	bool res = false;
    uint8 *ptr1, *ptr2;
    DWORD bytes1, bytes2;
    DWORD playPos, writePos;


	if (_Buffer == NULL)
	{
		_SecondaryBufferState = NL_DSOUND_SILENCING;
		_UserState = NL_DSOUND_STOPPED;
		return;
	}


	INITTIME(startPos);


	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

	uint8* data = ((CBufferDSound*) _Buffer)->getData();
	uint32 available = (_BytesWritten < _BufferSize) ? _BufferSize - _BytesWritten : 0;
	uint32 bytes = NLSOUND_MIN(_SwapCopySize, available);
	uint32 clear = _SwapCopySize - available;


	_SilenceWritten = 0;

	// Lock the buffer

	INITTIME(startLock);


	if (!lock(writePos, _SwapCopySize, ptr1, bytes1, ptr2, bytes2))
	{
		return;
	}


	INITTIME(startCopy);

	// Start copying the samples

	if (bytes1 <= bytes) {

		CFastMem::memcpy(ptr1, data + _BytesWritten, bytes1);
		_BytesWritten += bytes1;
		bytes -= bytes1;

		if (ptr2)
		{
			if (bytes > 0)
			{
				CFastMem::memcpy(ptr2, data + _BytesWritten, bytes);                    
				_BytesWritten += bytes;
			}

			if (bytes < bytes2)
			{
				if (_Loop)
				{
					DBGPOS(("FDIN: LOOP"));

					CFastMem::memcpy(ptr2 + bytes, data, bytes2 - bytes); 
					_BytesWritten = bytes2 - bytes;
				}
				else
				{
					memset(ptr2 + bytes, 0, bytes2 - bytes); 
					_SilenceWritten = bytes2 - bytes;
				}
			}
		}
	}
	else
	{
		if (bytes > 0)
		{
			CFastMem::memcpy(ptr1, data + _BytesWritten, bytes);
			_BytesWritten += bytes;
		}

		if (_Loop)
		{
			DBGPOS(("FDIN: LOOP"));

			CFastMem::memcpy(ptr1 + bytes, data, bytes1 - bytes);                    
			_BytesWritten = bytes1 - bytes;

			if (ptr2)
			{
				CFastMem::memcpy(ptr2, data + _BytesWritten, bytes2);                    
				_BytesWritten += bytes2;
			}
		} 
		else
		{
			memset(ptr1 + bytes, 0, bytes1 - bytes);                    
			_SilenceWritten = bytes1 - bytes;

			if (ptr2)
			{
				memset(ptr2, 0, bytes2);                    
				_SilenceWritten += bytes2;
			}
		}
	}


	INITTIME(startUnlock);

	// Unlock the buffer
	_SecondaryBuffer->Unlock(ptr1, bytes1, ptr2, bytes2);


	// Update the state variables

	_SecondaryBufferState = NL_DSOUND_FILLING;
	DBGPOS(("FDIN: FILLING"));


	// Check if we've reached the end of the file
	if (_BytesWritten == _BufferSize) 
	{
		if (_Loop)
		{
			// If we're looping, start all over again
			DBGPOS(("FDIN: LOOP"));
			_BytesWritten = 0;
		}
		else
		{
			_SecondaryBufferState = NL_DSOUND_SILENCING;

			// Keep track of where tha last sample was written and the position
			// of the play cursor relative to the end position. if the _EndState
			// is NL_DSOUND_TAIL1, the play cursor is after the end position, 
			// NL_DSOUND_TAIL2 otherwise.
			_EndPosition = writePos + bytes;
			if (_EndPosition >= _SecondaryBufferSize)
			{
				_EndPosition -= _SecondaryBufferSize;
			}

			_EndState = (playPos > _EndPosition)? NL_DSOUND_TAIL1 : NL_DSOUND_TAIL2;

			DBGPOS(("FDIN: SILENCING"));
			DBGPOS(("FDIN: ENDSTATE=%d, E=%d, P=%d", (int) _EndState, _EndPosition, playPos));
		}
	}


	// Update the write pointer
	_NextWritePos = writePos + bytes1 + bytes2;
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos  -= _SecondaryBufferSize;
	}
 
	DBGPOS(("FDIN: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


#if NLSOUND_PROFILE
	_TotalUpdateSize += bytes1 + bytes2;
	_PosTime += CTime::ticksToSecond(startLock - startPos);
	_LockTime += CTime::ticksToSecond(startCopy - startLock);
	_CopyTime += CTime::ticksToSecond(startUnlock - startCopy);
	_UnlockTime += CTime::ticksToSecond(CTime::getPerformanceTime() - startUnlock);
	_CopyCount++;
#endif


}



} // NLSOUND
