/** \file sound_dirver_dsound.h
 * DirectSound sound source
 *
 * $Id: sound_driver_dsound.h,v 1.9.2.1 2003/04/24 16:19:45 boucher Exp $
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

#ifndef NL_SOUND_DRIVER_DSOUND_H
#define NL_SOUND_DRIVER_DSOUND_H

#include "sound/driver/sound_driver.h"
#include "source_dsound.h"
#include "buffer_dsound.h"
#include "../sound_driver.h"
#include <iostream>

namespace NLSOUND {

class IListener;
class ISource;
class IBuffer;
class CListenerDSound;
class CSourceDSound;
class CBufferDSound;

class CSoundDriverDSound : public ISoundDriver
{
public:

    /// Constructor
    CSoundDriverDSound();

    virtual ~CSoundDriverDSound();

	/// Return the instance of the singleton
	static CSoundDriverDSound *instance() { return _Instance; }

	/// Create the listener instance
	virtual	IListener *createListener();

	/// Initialization
	virtual bool init(HWND wnd, bool useEax, IStringMapperProvider *stringMapper);

	/// Create a sound buffer
	virtual	IBuffer *createBuffer();

	/// Load a wave file in to the buffer
//	virtual bool loadWavFile(IBuffer *destbuffer, const char *filename);
	virtual bool readWavBuffer( IBuffer *destbuffer, const std::string &name, uint8 *wavData, uint dataSize);

	virtual bool readRawBuffer( IBuffer *destbuffer, const std::string &name, uint8 *rawData, uint dataSize, TSampleFormat format, uint32 frequency);


    // Source management

	/// Create a source
	virtual	ISource *createSource();

	/// Commit all the changes made to 3D settings of listener and sources
	virtual void commit3DChanges();

	/// Return the maximum number of sources that can created
	virtual uint countMaxSources();

	/// Count the number of available hardware streaming 3D buffers
    uint countHw3DBuffers();

	/// Count the number of available hardware streaming 2D buffers
    uint countHw2DBuffers();

	/// Count the number of sources that are actually playing.
    uint countPlayingSources();

	/// Update all the driver and its sources. To be called only by the timer callback.
	void update();

	/// Write information about the driver to the output stream.
	void writeProfile(std::ostream& out);


    /** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
    void setGain( float gain );

    /// Get the gain
	float getGain();

	/// Return the string mapper
	IStringMapperProvider	*getStringMapper()	{return _StringMapper;}

#if EAX_AVAILABLE == 1
	LPKSPROPERTYSET		createPropertySet(CSourceDSound *source);
#endif

private:

	// The callback for the multimedia timer
    static void CALLBACK TimerCallback(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

	// The refence to the singleton.
    static CSoundDriverDSound* _Instance;

	// The period of the timer.
    static uint32 _TimerPeriod;

	friend CBufferDSound::~CBufferDSound();
	friend CSourceDSound::~CSourceDSound();

 	/// Remove a buffer (should be called by the friend destructor of the buffer class)
	virtual void removeBuffer(IBuffer *buffer);

	/// Remove a source (should be called by the friend destructor of the source class)
	virtual void removeSource(ISource *source);

	// The DirectSound object
    LPDIRECTSOUND8			_DirectSound;

    // The application-wide primary buffer
    LPDIRECTSOUNDBUFFER	_PrimaryBuffer;

    // The capabilities of the driver
    DSCAPS _Caps;

    // Array with the allocated sources
    //CSourceDSound** _Sources;
	std::set<CSourceDSound*> _Sources;

	// The number of allocated sources
    uint _SourceCount;

	// The Windows ID of the multimedia timer used in the update.
    UINT _TimerID;

	// The timer resolution.
    uint32 _TimerResolution;

	/// The EAX support is requested and accepted (ie, there is enougth hardware 3D buffer)
	bool	_UseEAX;
	/// The string mapper provided by client code
	IStringMapperProvider	*_StringMapper;

#if NLSOUND_PROFILE
protected:


    uint _TimerInterval[1024];
    uint _TimerIntervalCount;
    NLMISC::TTicks _TimerDate;
    double _TotalTime;
    double _TotalUpdateTime;
	uint32 _UpdateCount;
	uint32 _UpdateSources;
	uint32 _UpdateExec;


public:

    void printDriverInfo(FILE* fp);
    uint countTimerIntervals();
    uint getTimerIntervals(uint index);
    void addTimerInterval(uint32 dt);
    double getCPULoad();
	double getTotalTime()            { return _TotalTime; };
	double getAverageUpdateTime()    { return (_UpdateCount) ?  _TotalUpdateTime / _UpdateCount : 0.0; }
	uint32 getAverageUpdateSources() { return (_UpdateExec) ?  _UpdateSources / _UpdateExec : 0; }
	double getUpdatePercentage()     { return (_UpdateCount) ? (double) _UpdateExec / (double) _UpdateCount: 0; }
#endif



};

} // NLSOUND

#endif // NL_SOUND_DRIVER_DSOUND_H
