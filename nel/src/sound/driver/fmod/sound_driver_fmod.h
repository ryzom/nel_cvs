/** \file sound_dirver_fmod.h
 * DirectSound sound source
 *
 * $Id: sound_driver_fmod.h,v 1.5.2.1 2004/10/28 17:37:46 corvazier Exp $
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

#ifndef NL_SOUND_DRIVER_FMOD_H
#define NL_SOUND_DRIVER_FMOD_H

#include "nel/misc/log.h"
#include "sound/driver/sound_driver.h"
#include "source_fmod.h"
#include "buffer_fmod.h"
#include "../sound_driver.h"
#include <iostream>

namespace NLSOUND {

class IListener;
class ISource;
class IBuffer;
class CListenerFMod;
class CSourceFMod;
class CBufferFMod;

// ***************************************************************************
/*
	DRIVER FMOD LIMITATIONS:

	- EAX is not supported (setEnvironnement() / setEAXProperty() no-op)
	- ADPCM is not supported (decompressed and the format change internaly )
	- deffered param on ISource::setPos() etc... does not work. Always deffered.
	- No cooperative level change in FMod as in DSOUND (default???)
	
*/


// ***************************************************************************
class CSoundDriverFMod : public ISoundDriver
{
public:

    /// Constructor
    CSoundDriverFMod();

    virtual ~CSoundDriverFMod();

	/// Return the instance of the singleton
	static CSoundDriverFMod *instance() { return _Instance; }

	/// Create the listener instance
	virtual	IListener *createListener();

	/// Initialization
	void init(IStringMapperProvider *stringMapper, bool forceSoftwareBuffer);

	/// Create a sound buffer
	virtual	IBuffer *createBuffer();

	virtual bool readWavBuffer( IBuffer *destbuffer, const std::string &name, uint8 *wavData, uint dataSize);

	virtual bool readRawBuffer( IBuffer *destbuffer, const std::string &name, uint8 *rawData, uint dataSize, TSampleFormat format, uint32 frequency);


    // Source management

	/// Create a source
	virtual	ISource *createSource();

	/// Commit all the changes made to 3D settings of listener and sources
	virtual void commit3DChanges();

	/// Return the maximum number of sources that can created
	virtual uint countMaxSources();

	/// Count the number of sources that are actually playing.
    uint countPlayingSources();

	/// Update all the driver and its sources. To be called only by the timer callback.
	void update();

	/// Write information about the driver to the output stream.
	void writeProfile(std::string& out);


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

	// play the music
	virtual bool	playMusic(NLMISC::CIFile &file, uint xFadeTime, bool loop);
	
	// play the music async
	virtual bool	playMusicAsync(const std::string &path, uint xFadeTime= 0, uint fileOffset=0, uint fileSize= 0, bool loop=true);
	
	// stop the music
	virtual void	stopMusic(uint xFadeTime=0);
	
	// pause the music
	virtual void	pauseMusic();
	
	// resume the music
	virtual void	resumeMusic();
	
	// is the music playing
	virtual bool	isMusicEnded();

	// set music volume
	virtual void	setMusicVolume(float gain);

	// get a song title
	virtual bool	getSongTitle(const std::string &filename, std::string &result, uint fileOffset=0, uint fileSize=0);

	// Tool method to transform from Nel coords to FMod coords
	static void	toFModCoord(const NLMISC::CVector &in, float out[3]);

	bool	fmodOk() const {return _FModOk;}

	bool	forceSofwareBuffer() const {return _ForceSoftwareBuffer;}

private:

	// The refence to the singleton.
    static CSoundDriverFMod* _Instance;

	friend CBufferFMod::~CBufferFMod();
	friend CSourceFMod::~CSourceFMod();

 	/// Remove a buffer (should be called by the friend destructor of the buffer class)
	virtual void removeBuffer(IBuffer *buffer);

	/// Remove a source (should be called by the friend destructor of the source class)
	virtual void removeSource(ISource *source);


	virtual void	startBench();
	virtual void	endBench();
	virtual void	displayBench(NLMISC::CLog *log);


    // Array with the allocated sources
	std::set<CSourceFMod*>	_Sources;

	/// The string mapper provided by client code
	IStringMapperProvider	*_StringMapper;

	/// if correctly created
	bool					_FModOk;

	/// Master Volume [0,1]
	float					_MasterGain;

	/// If want to create buffer in software (no hardware)
	bool					_ForceSoftwareBuffer;

	
	/// Music
	class CMusicChannel
	{
	public:
		FSOUND_STREAM			*FModMusicStream;	// The FMod stream
		uint8					*FModMusicBuffer;	// the RAM buffer (representation of a MP3 file, only for sync play)
		sint					FModMusicChannel;	// channel played for music. CAN BE -1 while FModMusicStream!=NULL in case of Async Loading
		float					XFadeVolume;		// 0--1
		float					XFadeDVolume;		// dt

	public:
		CMusicChannel()
		{
			FModMusicStream= NULL;
			FModMusicBuffer= NULL;
			FModMusicChannel= -1;
			XFadeVolume= 0.f;
			XFadeDVolume= 0.f;
		}
	};
	// 2 musics channels for XFade
	enum	{MaxMusicChannel= 2};
	CMusicChannel			_MusicChannel[MaxMusicChannel];
	uint8					_ActiveMusicChannel;
	sint64					_LastXFadeTime;
	float					_FModMusicVolume;
	std::list<FSOUND_STREAM*>	_FModMusicStreamWaitingForClose;	// see stopMusicChannel()
	void	playMusicStartFade(uint xFadeTime);
	void	playMusicStartChannel(uint musicChannel);
	void	stopMusicChannel(uint musicChannel);
	void	updateMusicVolumeChannel(uint musicChannel);
	void	updateMusic();
	void	updateMusicFModStreamWaitingForClose();
	
};


} // NLSOUND

#endif // NL_SOUND_DRIVER_FMOD_H
