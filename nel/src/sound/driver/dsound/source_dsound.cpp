/** \file source_dsound.cpp
 * DirectSound sound source
 *
 * $Id: source_dsound.cpp,v 1.13 2002/11/04 15:40:44 boucher Exp $
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
#include "listener_dsound.h"



using namespace NLMISC;


namespace NLSOUND {


#if NLSOUND_PROFILE

	#define INITTIME(_var)	 TTicks _var = CTime::getPerformanceTime()

	#define DEBUG_POSITIONS  1

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
uint CSourceDSound::_DefaultChannels = 1;
uint CSourceDSound::_DefaultSampleRate = 22050;
uint CSourceDSound::_DefaultSampleSize = 16;



#define NLSOUND_MIN(_a,_b)	  (((_a) < (_b)) ? (_a) : (_b))
#define NLSOUND_DISTANCE(_from, _to, _period)	(((_to) > (_from)) ? (_to) - (_from) : (_period) + (_to) - (_from))


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



// ******************************************************************

CSourceDSound::CSourceDSound( uint sourcename ) : ISource(), _SourceName(sourcename)
{
	_BufferSize = 0;
	_SwapBuffer = 0;
	_SecondaryBuffer = 0;
	_SecondaryBufferState = NL_DSOUND_SILENCED;
	_3DBuffer = 0;
	_NextWritePos = 0;
	_BytesWritten = 0;
	_SilenceWritten = 0;
	_Loop = false;
	_EndPosition = 0;
	_EndState = NL_DSOUND_TAIL1;
	_UserState = NL_DSOUND_STOPPED;
	_Freq = 1.0f;
	_SampleRate = _DefaultSampleRate;
	_IsUsed = false;
	_Gain = 1.0f;
	_Volume = 0;
	_Alpha = 0.0;
	InitializeCriticalSection(&_CriticalSection);
}


// ******************************************************************

CSourceDSound::~CSourceDSound()
{
	nldebug("Destroying DirectSound source");

	CSoundDriverDSound::instance()->removeSource(this);

	EnterCriticalSection(&_CriticalSection); 

	// Release the DirectSound buffer within the critical zone
	// to avoid a call to update during deconstruction
	release();

	LeaveCriticalSection(&_CriticalSection); 
	DeleteCriticalSection(&_CriticalSection);
}


// ******************************************************************

void CSourceDSound::release()
{
	_Buffer = 0;

	if (_SecondaryBuffer != 0)
	{
		_SecondaryBuffer->Stop();
	}

	if (_3DBuffer != 0)
	{
		_3DBuffer->Release();
		_3DBuffer = 0;
	}

	if (_SecondaryBuffer != 0)
	{
		_SecondaryBuffer->Release();
		_SecondaryBuffer = 0;
	}

}

// ******************************************************************

void CSourceDSound::init(LPDIRECTSOUND directSound)
{

	// Initialize the buffer format
	WAVEFORMATEX format;

	format.cbSize = sizeof(WAVEFORMATEX);
	format.nChannels = _DefaultChannels;
	format.wBitsPerSample = _DefaultSampleSize;
	format.nSamplesPerSec = _DefaultSampleRate;
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
		//nldebug("Source: Allocating 3D buffer in hardware");
		desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 
						| DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
	} 
	else
	{
		//nldebug("Source: Allocating 3D buffer in software");
		desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 
						| DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_MUTE3DATMAXDISTANCE;
		desc.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
		//desc.guid3DAlgorithm = DS3DALG_HRTF_FULL;
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
			//nldebug("Source: Allocating 2D buffer in hardware");
			desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCHARDWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
		} 
		else
		{
			//nldebug("Source: Allocating 2D buffer in software");
			desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;
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

// ******************************************************************

void CSourceDSound::reset()
{
	setPitch(1.0f);
	setLooping(false);
	setGain(1.0f);
}


// ******************************************************************

void CSourceDSound::setStaticBuffer( IBuffer *buffer )
{
	EnterCriticalSection(&_CriticalSection); 

	// If the user calls setStaticBuffer with a null buffer,
	// stop the currently playing buffer and set it to null.
	// Otherwise, store the buffer in the swap buffer variable.
	// A crossfade between the current buffer and the swap buffer
	// will be done when the user calls play.
	if (buffer == 0)
	{
		stop();
		_Buffer = 0;
		_BufferSize = 0;
		_BytesWritten = 0;
	}

	_SwapBuffer = _Buffer = buffer;

	LeaveCriticalSection(&_CriticalSection); 
}



// ******************************************************************

void CSourceDSound::setLooping( bool l )
{
	_Loop = l;
}


// ******************************************************************

bool CSourceDSound::getLooping() const
{
	return _Loop;
}


// ******************************************************************

void CSourceDSound::swap()
{
	_Buffer = _SwapBuffer;
	_BufferSize = _Buffer->getSize();
	_BytesWritten = 0;
	_SwapBuffer = 0;
}

// ******************************************************************

bool CSourceDSound::play()
{
	EnterCriticalSection(&_CriticalSection); 

	if (_Buffer == 0 || ((CBufferDSound*) _Buffer)->getData() == 0)
	{
		// the sample has been unloaded, can't play!

		LeaveCriticalSection(&_CriticalSection); 
		return false;
	}


	DBGPOS(("[%p] PLAY: Enter, buffer state = %u", this, _SecondaryBufferState));
	switch (_SecondaryBufferState)
	{
	case NL_DSOUND_FILLING:
		if (_SwapBuffer != 0)
		{
			// crossfade to the new sound 
			DBGPOS(("[%p] PLAY: XFading 1", this));
			crossFade();
		}
		break;

	case NL_DSOUND_SILENCING:
		if (_SwapBuffer != 0)
		{
			if ((_Buffer != 0) && (_UserState == NL_DSOUND_PLAYING))
			{
				// crossfade to the new sound
				DBGPOS(("[%p] PLAY: XFading 2", this));
				crossFade();
			}
			else
			{
				DBGPOS(("[%p] PLAY: Swap & fadein 1", this));
				swap();
				fadeIn();
			}
		}
		else
		{
			DBGPOS(("[%p] PLAY: Fadein", this));
			_BytesWritten = 0;
			// start the old sound again
			fadeIn();
		}

		break;


	case NL_DSOUND_SILENCED:
		if (_SwapBuffer != 0)
		{
			// fade in to the new sound
				DBGPOS(("[%p] PLAY: Swap & fadein 2", this));
			swap();
			fadeIn();
		}
		else
		{
			DBGPOS(("[%p] PLAY: Fadein", this));
			_BytesWritten = 0;
			// start the old sound again
			fadeIn();
		}
	}

	_UserState = NL_DSOUND_PLAYING;
	DBGPOS(("[%p] PLAY: PLAYING", this));

	//nldebug ("NLSOUND: %p play", this);

	LeaveCriticalSection(&_CriticalSection); 

	return true;
}


// ******************************************************************

void CSourceDSound::stop()
{
	EnterCriticalSection(&_CriticalSection); 

	TSourceDSoundUserState old = _UserState;

	_UserState = NL_DSOUND_STOPPED;
	DBGPOS(("[%p] STOP: STOPPED", this));

	//nldebug ("NLSOUND: %p stop", this);

	if (old == NL_DSOUND_PLAYING)
	{
		fadeOut();
	}

	_BytesWritten = 0;

	LeaveCriticalSection(&_CriticalSection); 
}

// ******************************************************************

void CSourceDSound::pause()
{
	EnterCriticalSection(&_CriticalSection); 

	TSourceDSoundUserState old = _UserState;

	_UserState = NL_DSOUND_PAUSED;
	DBGPOS(("[%p] PAUZ: PAUSED", this));

	//nldebug ("NLOUND: pause %p", this);

	if (old == NL_DSOUND_PLAYING)
	{
		fadeOut();
	}

	LeaveCriticalSection(&_CriticalSection); 
}

// ******************************************************************

bool CSourceDSound::isPlaying() const
{
	return (_UserState == NL_DSOUND_PLAYING);
}


// ******************************************************************

bool CSourceDSound::isPaused() const
{
	return (_UserState == NL_DSOUND_PAUSED);
}


// ******************************************************************

bool CSourceDSound::isStopped() const
{
	return (_UserState == NL_DSOUND_STOPPED);
}


// ******************************************************************

bool CSourceDSound::needsUpdate()
{
	DWORD playPos, writePos;
	uint32 space;
	
	if (_SecondaryBuffer == 0)
	{
		return true;
	}

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



// ******************************************************************

void CSourceDSound::update()
{
	update2();
}

// ******************************************************************

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



// ******************************************************************

void CSourceDSound::setPos( const NLMISC::CVector& pos )
{
	_Pos = pos;
	// Coordinate system: conversion from NeL to OpenAL/GL:
	if (_3DBuffer != NULL)
	{
		if (_3DBuffer->SetPosition(pos.x, pos.z, pos.y, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning ("SetPosition failed");
		}
		else
		{
			//nlwarning ("%p set source NEL(p:%.2f/%.2f/%.2f) DS(p:%.2f/%.2f/%.2f)", this, pos.x, pos.y, pos.z, pos.x, pos.z, pos.y);
		}
	}
}


// ******************************************************************

const NLMISC::CVector &CSourceDSound::getPos() const
{
	return _Pos;
	// Coordinate system: conversion from NeL to OpenAL/GL:
/*	if (_3DBuffer != NULL)
	{
		D3DVECTOR v;
		HRESULT hr = _3DBuffer->GetPosition(&v);

		if (hr != DS_OK)
		{
			nlwarning ("GetPosition failed");
			pos.set(0, 0, 0);	
		}
		else
		{
			pos.set(v.x, v.z, v.y);
		}
	}
	else
	{
		pos.set(0, 0, 0);	
	}
*/
}


// ******************************************************************

void CSourceDSound::setVelocity( const NLMISC::CVector& vel )
{
	if (_3DBuffer != NULL)
	{
		if (_3DBuffer->SetVelocity(vel.x, vel.z, vel.y, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning ("SetVelocity failed");
		}
	}
}


// ******************************************************************

void CSourceDSound::getVelocity( NLMISC::CVector& vel ) const
{
	if (_3DBuffer != NULL)
	{
		D3DVECTOR v;

		if (_3DBuffer->GetVelocity(&v) != DS_OK)
		{
			nlwarning ("GetVelocity failed");
			vel.set(0, 0, 0);	
		}
		else
		{
			vel.set(v.x, v.z, v.y);
		}
	}
	else
	{
		vel.set(0, 0, 0);	
	}
}


// ******************************************************************

void CSourceDSound::setDirection( const NLMISC::CVector& dir )
{
	if (_3DBuffer != 0)
	{
		if (_3DBuffer->SetConeOrientation(dir.x, dir.z, dir.y, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning ("SetConeOrientation failed (x=%.2f, y=%.2f, z=%.2f)", dir.x, dir.y, dir.z);
		}
		else
		{
			//nlwarning ("NLSOUND: %p set source direction NEL(p:%.2f/%.2f/%.2f) DS(p:%.2f/%.2f/%.2f)", this, dir.x, dir.y, dir.z, dir.x, dir.z, dir.y);
		}
	}
}


// ******************************************************************

void CSourceDSound::getDirection( NLMISC::CVector& dir ) const
{
	if (_3DBuffer != NULL)
	{
		D3DVECTOR v;

		if (_3DBuffer->GetConeOrientation(&v) != DS_OK)
		{
			nlwarning("GetConeOrientation failed");
			dir.set(0, 0, 1);	
		}
		else
		{
			dir.set(v.x, v.z, v.y);
		}
	}
	else
	{
		dir.set(0, 0, 1);	
	}
}


// ******************************************************************

void CSourceDSound::setGain( float gain )
{
	clamp(gain, 0.00001f, 1.0f);
	_Gain = gain;

	/* convert from linear amplitude to hundredths of decibels */
	_Volume = (uint32)(100.0 * 20.0 * log10(gain));
	clamp(_Volume, DSBVOLUME_MIN, DSBVOLUME_MAX);

	//nlwarning ("set gain %f vol %d", gain, _Volume);

	/*
	if ((_SecondaryBuffer != 0) && (_SecondaryBuffer->SetVolume(_Volume) != DS_OK))
	{
		nlwarning("SetVolume failed");
	}
	*/
}


/*
 * Get the gain
 */
float CSourceDSound::getGain() const
{
	return _Gain;
}


// ******************************************************************

void CSourceDSound::setPitch( float coeff )
{
//	_Freq = coeff;

	if ((_Buffer != 0) && (_SecondaryBuffer != 0))
	{
		TSampleFormat format;
		uint freq;

		_Buffer->getFormat(format, freq);

		_SampleRate = (uint32) (coeff * (float) freq);

		//nlwarning("Freq=%d", newfreq);

		if (_SecondaryBuffer->SetFrequency(_SampleRate) != DS_OK)
		{
//			nlwarning("SetFrequency failed (buffer freq=%d, NeL freq=%.5f, DSound freq=%d)", freq, coeff, newfreq);
			nlwarning("SetFrequency");
		}
	}
}


// ******************************************************************

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
			nlwarning("GetFrequency failed");
			return 1.0;
		}

		return ((float) freq / (float) freq0);
	}

	return 1.0;
}


// ******************************************************************

void CSourceDSound::setSourceRelativeMode( bool mode )
{
	if (_3DBuffer != 0)
	{
		HRESULT hr;

		if (mode)
		{
			hr = _3DBuffer->SetMode(DS3DMODE_HEADRELATIVE, DS3D_DEFERRED);
		}
		else
		{
			hr = _3DBuffer->SetMode(DS3DMODE_NORMAL, DS3D_DEFERRED);
		}

		if (hr != DS_OK)
		{
			nlwarning("SetMode failed");
		}
	}
	else
	{
		nlwarning("Requested setSourceRelativeMode on a non-3D source");
	}
}


// ******************************************************************

bool CSourceDSound::getSourceRelativeMode() const
{
	if (_3DBuffer != 0)
	{
		DWORD mode;

		if (_3DBuffer->GetMode(&mode) != DS_OK)
		{
			nlwarning("GetMode failed");
			return false;
		}

		return (mode == DS3DMODE_HEADRELATIVE);
	}
	else
	{
		nlwarning("Requested setSourceRelativeMode on a non-3D source");
		return false;
	}
}


// ******************************************************************

void CSourceDSound::setMinMaxDistances( float mindist, float maxdist )
{
	if (_3DBuffer != 0)
	{
		if (_3DBuffer->SetMinDistance(mindist, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning("SetMinDistance (%f) failed", mindist);
		}
		if (_3DBuffer->SetMaxDistance(maxdist, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning("SetMaxDistance (%f) failed", maxdist);
		}
	}
	else
	{
		nlwarning("Requested setMinMaxDistances on a non-3D source");
	}
}


// ******************************************************************

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
			nlwarning("GetMinDistance or GetMaxDistance failed");
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
		nlwarning("Requested getMinMaxDistances on a non-3D source");
	}
}

// ******************************************************************

void CSourceDSound::updateVolume( const NLMISC::CVector& listener )
{
	CVector pos = getPos();
	pos -= listener;

	float sqrdist = pos.sqrnorm();
	float min, max;

	getMinMaxDistances(min, max);

	if (sqrdist < min * min) 
	{
		_SecondaryBuffer->SetVolume(_Volume);
		//nlwarning("VOLUME = %ddB, rolloff = %0.2f", _Volume/100, CListenerDSound::instance()->getRolloffFactor());
	}
	else if (sqrdist > max * max)
	{
		_SecondaryBuffer->SetVolume(DSBVOLUME_MIN);
		//nlwarning("VOLUME = %ddB, rolloff = %0.2f", DSBVOLUME_MIN/100, CListenerDSound::instance()->getRolloffFactor());
	}
	else
	{
		sint32 db = _Volume;

		double dist = (double) sqrt(sqrdist);

		// linearly descending volume on a dB scale
		double db1 = DSBVOLUME_MIN * (dist - min) / (max - min);

		if (_Alpha == 0.0) {
			db += (sint32) db1;

		} else if (_Alpha > 0.0) {
			double amp2 = 0.0001 + 0.9999 * (max - dist) / (max - min); // linear amp between 0.00001 and 1.0
			double db2 = 2000.0 * log10(amp2); // covert to 1/100th decibels
			db += (sint32) ((1.0 - _Alpha) * db1 + _Alpha * db2);

		} else if (_Alpha < 0.0) {
			double amp3 = min / dist; // linear amplitude is 1/distance
			double db3 = 2000.0 * log10(amp3); // covert to 1/100th decibels
			db += (sint32) ((1.0 + _Alpha) * db1 - _Alpha * db3);
		}

		clamp(db, DSBVOLUME_MIN, DSBVOLUME_MAX);

		_SecondaryBuffer->SetVolume(db);

/*		LONG tmp;
		_SecondaryBuffer->GetVolume(&tmp);
*/
		
		//nlwarning("VOLUME = %d dB, rolloff = %0.2f", db/100, CListenerDSound::instance()->getRolloffFactor());
	}

}

// ******************************************************************

void CSourceDSound::setCone( float innerAngle, float outerAngle, float outerGain )
{
	if (_3DBuffer != 0)
	{
		// Set the cone angles

		// Convert from radians to degrees
		DWORD inner = (DWORD)(180.0 * innerAngle / Pi);
		DWORD outer = (DWORD)(180.0 * outerAngle / Pi);


		// Sanity check: wrap the angles in the [0,360] interval
		if (outer < inner)
		{
			outer = inner;
		}

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

		if (_3DBuffer->SetConeAngles(inner, outer, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning("SetConeAngles failed");			
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

		if (_3DBuffer->SetConeOutsideVolume(volume, DS3D_DEFERRED) != DS_OK)
		{
			nlwarning("SetConeOutsideVolume failed"); 		
		}

	}
	else
	{
		nlwarning("Requested setCone on a non-3D source");
	}
}

// ******************************************************************

void CSourceDSound::getCone( float& innerAngle, float& outerAngle, float& outerGain ) const
{
	if (_3DBuffer != 0)
	{
		DWORD inner, outer;
		LONG volume;

		if (_3DBuffer->GetConeAngles(&inner, &outer) != DS_OK)
		{
			nlwarning("GetConeAngles failed");			
			innerAngle = outerAngle = (float)(2.0 * Pi);
		}
		else
		{
			innerAngle = (float)(Pi * inner / 180.0);
			outerAngle = (float)(Pi * outer / 180.0);
		}

		if (_3DBuffer->GetConeOutsideVolume(&volume) != DS_OK)
		{
			nlwarning("GetConeOutsideVolume failed"); 		
			outerGain = 0.0f;
		}
		else
		{
			outerGain = (float) pow(10, (double) volume / 20.0 / 100.0);
		}
	}
	else
	{
		nlwarning("Requested getCone on a non-3D source");
	}
}

// ******************************************************************

void CSourceDSound::setEAXProperty( uint prop, void *value, uint valuesize )
{
#ifdef EAX_AVAILABLE
	if ( EAXSetProp != NULL )
	{
		EAXSetProp( &DSPROPSETID_EAX_SourceProperties, prop, _SourceName, value, valuesize );
	}
#endif
}


// ******************************************************************

IBuffer *CSourceDSound::getBuffer()
{	
	return _Buffer;
}


// ******************************************************************

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

// ******************************************************************

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


			NW	   P   W
   +--------+------+---+-----------------------+
   |........|	   |xxx|.......................|
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

	if (_SecondaryBuffer == 0)
	{
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
					DBGPOS(("[%p] FILL: LOOP", this));

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
			DBGPOS(("[%p] FILL: LOOP", this));

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
			DBGPOS(("[%p] FILL: LOOP", this));
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

			DBGPOS(("[%p] FILL: SILENCING", this));
			DBGPOS(("[%p] FILL: ENDSTATE=%d, E=%d, P=%d", this, (int) _EndState, _EndPosition, playPos));
		}
	}


	// Update the write pointer
	_NextWritePos += bytes1 + bytes2;
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos  -= _SecondaryBufferSize;
	}
 
	DBGPOS(("[%p] FILL: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


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




// ******************************************************************

bool CSourceDSound::silence()
{
	uint8 *ptr1, *ptr2;
	DWORD bytes1, bytes2;
	DWORD playPos, writePos;
	uint32 space;

	if (_SecondaryBuffer == 0)
	{
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
		DBGPOS(("[%p] SLNC: ENDED", this));

		// If the buffer was playing, mark it as stopped
		if (_UserState == NL_DSOUND_PLAYING)
		{
			_UserState = NL_DSOUND_STOPPED;
			DBGPOS(("[%p] SLNC: STOPPED", this));
		}
	}
	else if ((playPos < _EndPosition) && (_EndState == NL_DSOUND_TAIL1))
	{
		// The play cursor wrapped around the buffer and is now before the end position
		_EndState = NL_DSOUND_TAIL2;
		DBGPOS(("[%p] FILL: ENDSTATE=%d, E=%d, P=%d", this, (int) _EndState, _EndPosition, playPos));
	}


	// Update the amount of silence written
	_SilenceWritten += bytes1 + bytes2;
	if (_SilenceWritten >= _SecondaryBufferSize)
	{
		// The buffer is now completely filled with silence
		_SecondaryBufferState = NL_DSOUND_SILENCED;
		DBGPOS(("[%p] SLNC: SILENCED", this));

		// If the buffer was playing, mark it as stopped
		if (_UserState == NL_DSOUND_PLAYING)
		{
			_UserState = NL_DSOUND_STOPPED;
			DBGPOS(("[%p] SLNC: STOPPED*", this));
		}
	}

	DBGPOS(("[%p] SLNC: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


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


// ******************************************************************

/** 
 *	Calculate the size of the crossfade and set the pointer in the sample buffer
 *	to the sample that comes after the write cursor in the DirectSound buffer.
 *	This method also returns the number of samples that have been written after
 *	the write cursor.
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

	DBGPOS(("[%p] FADE: TOO=%d", this, writtenTooMuch));


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

		DBGPOS(("[%p] FADE: END, TOO=%d, S=%d, F=%d", this, writtenTooMuch, _SilenceWritten, xfadeSize));
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

		DBGPOS(("[%p] FADE: LOOPED, TOO=%d, F=%d", this, writtenTooMuch, xfadeSize));

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

		DBGPOS(("[%p] FADE: STD, TOO=%d, F=%d", this, writtenTooMuch, xfadeSize));
	}
}



// ******************************************************************

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

	if (_SecondaryBuffer == 0)
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

#define MIX 0


#if MIX
	float incr, amp1, amp2;

	if (xfadeSize == 0)
	{
		amp1 = 0.0f;
		amp2 = 1.0f;
		incr = 0.0f;
	}
	else
	{
		amp1 = 1.0f;
		amp2 = 0.0f;
		incr = 1.0f / xfadeSize;
	}

#else
	float incr, amp1;

	if (xfadeSize == 0)
	{
		amp1 = 0.0f;
		incr = 0.0f;
	}
	else
	{
		amp1 = 1.0f;
		incr = 1.0f / xfadeSize;
	}

#endif


	// Start copying the samples
	

	// In the first case, the cross fade is completely contained in the first buffer
	// pointed to by ptr1.
	if (xfadeByteSize < bytes1)
	{

		// Do cross fade

		for (i = 0; i < xfadeSize; i++) 
		{
#if MIX
			out[i] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
			amp1 -= incr;
			amp2 += incr;
#else
			out[i] = (sint16) (amp1 * in1[i]);
			amp1 -= incr;
#endif
		}

		// Copy remaining samples

#if MIX
		CFastMem::memcpy(ptr1 + xfadeByteSize, data2 + xfadeByteSize, bytes1 - xfadeByteSize);
		_BytesWritten = bytes1;
#else
		CFastMem::memcpy(ptr1 + xfadeByteSize, data2, bytes1 - xfadeByteSize);
		_BytesWritten = bytes1 - xfadeByteSize;
#endif

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
#if MIX
			out[i] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
			amp1 -= incr;
			amp2 += incr;
#else
			out[i] = (sint16) (amp1 * in1[i]);
			amp1 -= incr;
#endif
		}
#if MIX
		_BytesWritten = bytes1;
#else
		_BytesWritten = 0;
#endif


		if (ptr2)
		{
			out = (sint16*) ptr2;

			// Part 2, ontinue at ptr2
			for (uint32 k = 0; i < xfadeSize; i++, k++) 
			{
#if MIX
				out[k] = (sint16) (amp1 * in1[i] + amp2 * in2[i]);
				amp1 -= incr;
				amp2 += incr;
#else
				out[k] = (sint16) (amp1 * in1[i]);
				amp1 -= incr;
#endif
			}

			// Copy remaining samples
#if MIX
			CFastMem::memcpy(ptr2 + 2 * k, data2 + _BytesWritten + 2 * k, bytes2 - 2 * k);
			_BytesWritten += bytes2;
#else
			CFastMem::memcpy(ptr2 + 2 * k, data2, bytes2 - 2 * k);
			_BytesWritten = bytes2 - 2 * k;
#endif
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

	setPitch(_Freq);

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


	DBGPOS(("[%p] XFADE", this));
	DBGPOS(("[%p] P=%d, W=%d, NW=%d, SZ=%d, BW=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, bytes1 + bytes2));

	

	LeaveCriticalSection(&_CriticalSection);


#if NLSOUND_PROFILE
	_LastSwapTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
	_TotalSwapTime += _LastSwapTime;
	_MaxSwapTime = (_LastSwapTime > _MaxSwapTime) ? _LastSwapTime : _MaxSwapTime;
	_MinSwapTime = (_LastSwapTime < _MinSwapTime) ? _LastSwapTime : _MinSwapTime;
	_SwapCount++;
#endif

}



// ******************************************************************

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

	if (_SecondaryBuffer == 0)
	{
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

	float amp1, incr;

	if (xfadeSize == 0)
	{
		amp1 = 0.0f;
		incr = 0.0f;
	}
	else
	{
		amp1 = 1.0f;
		incr = 1.0f / xfadeSize;
	}


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
	DBGPOS(("[%p] FDOU: SILENCING", this));

	// Keep track of where tha last sample was written and the position
	// of the play cursor relative to the end position. if the _EndState
	// is 0, the play cursor is after the end position, 1 otherwise.
	_EndPosition = writePos + xfadeSize;
	if (_EndPosition >= _SecondaryBufferSize)
	{
		_EndPosition -= _SecondaryBufferSize;
	}

	_EndState = (playPos > _EndPosition)? NL_DSOUND_TAIL1 : NL_DSOUND_TAIL2;
	DBGPOS(("[%p] FDOU: ENDSTATE=%d, E=%d, P=%d", this, (int) _EndState, _EndPosition, playPos));


	DBGPOS(("[%p] FDOU: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));



#if NLSOUND_PROFILE
	_LastSwapTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
	_TotalSwapTime += _LastSwapTime;
	_MaxSwapTime = (_LastSwapTime > _MaxSwapTime) ? _LastSwapTime : _MaxSwapTime;
	_MinSwapTime = (_LastSwapTime < _MinSwapTime) ? _LastSwapTime : _MinSwapTime;
	_SwapCount++;
#endif

}

// ******************************************************************

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

	if (_SecondaryBuffer == 0)
	{
		return;
	}

	INITTIME(startPos);

	// Set the correct pitch for this sound
	setPitch(_Freq);

	// Set the correct volume
	// FIXME: a bit of a hack
	const CVector &pos = CListenerDSound::instance()->getPos();
	updateVolume(pos);


	_SecondaryBuffer->GetCurrentPosition(&playPos, &writePos);

	uint8* data = ((CBufferDSound*) _Buffer)->getData();
	uint32 available = (_BytesWritten < _BufferSize) ? _BufferSize - _BytesWritten : 0;
	uint32 bytes = NLSOUND_MIN(_SwapCopySize, available);
//	uint32 clear = _SwapCopySize - available;


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
					DBGPOS(("[%p] FDIN: LOOP", this));

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
			DBGPOS(("[%p] FDIN: LOOP", this));

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
	DBGPOS(("[%p] FDIN: FILLING", this));


	// Check if we've reached the end of the file
	if (_BytesWritten == _BufferSize) 
	{
		if (_Loop)
		{
			// If we're looping, start all over again
			DBGPOS(("[%p] FDIN: LOOP", this));
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

			DBGPOS(("[%p] FDIN: SILENCING", this));
			DBGPOS(("[%p] FDIN: ENDSTATE=%d, E=%d, P=%d", this, (int) _EndState, _EndPosition, playPos));
		}
	}


	// Update the write pointer
	_NextWritePos = writePos + bytes1 + bytes2;
	if (_NextWritePos >= _SecondaryBufferSize)
	{
		_NextWritePos  -= _SecondaryBufferSize;
	}
 
	DBGPOS(("[%p] FDIN: P=%d, W=%d, NW=%d, SZ=%d, BW=%d, S=%d, B=%d", this, playPos, writePos, _NextWritePos, _BufferSize, _BytesWritten, _SilenceWritten, bytes1 + bytes2));


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
