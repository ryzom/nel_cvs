/** \file sound_driver_fmod.cpp
 * DirectSound driver
 *
 * $Id: sound_driver_fmod.cpp,v 1.5 2004/10/07 14:42:26 berenguier Exp $
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


#include "stdfmod.h"
#include "../sound_driver.h"

#include <math.h>
#include <eax.h>

#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "sound_driver_fmod.h"
#include "listener_fmod.h"

#include "fmod.h"


using namespace std;
using namespace NLMISC;


namespace NLSOUND {

CSoundDriverFMod* CSoundDriverFMod::_Instance = NULL;
HINSTANCE CSoundDriverDllHandle = 0;


// ******************************************************************
// The main entry of the DLL. It's used to get a hold of the hModule handle.

BOOL WINAPI DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  CSoundDriverDllHandle = (HINSTANCE) hModule;
  return TRUE;
}


// ******************************************************************

__declspec(dllexport) ISoundDriver *NLSOUND_createISoundDriverInstance(bool useEax, ISoundDriver::IStringMapperProvider *stringMapper, bool forceSoftwareBuffer)
{
	NL_ALLOC_CONTEXT(NLSOUND_ISoundDriver);

	CSoundDriverFMod *driver = new CSoundDriverFMod();
	driver->init(stringMapper, forceSoftwareBuffer);

	return driver;
}

// ******************************************************************

__declspec(dllexport) uint32 NLSOUND_interfaceVersion()
{
	return ISoundDriver::InterfaceVersion;
}

// ******************************************************************

__declspec(dllexport) void NLSOUND_outputProfile(string &out)
{
	CSoundDriverFMod::instance()->writeProfile(out);
}


// ******************************************************************

CSoundDriverFMod::CSoundDriverFMod()
:	_StringMapper(0)
{
	if ( _Instance == NULL )
	{
		_Instance = this;
		_FModOk= false;
		_MasterGain= 1.f;
		_ForceSoftwareBuffer= false;
		_ActiveMusicChannel= 0;
		_LastXFadeTime= 0;
		_FModMusicVolume= 1.f;
    }
	else
	{
		nlerror("Sound driver singleton instanciated twice");
	}
}


// ******************************************************************

void	CSoundDriverFMod::init(IStringMapperProvider *stringMapper, bool forceSoftwareBuffer)
{
	_StringMapper = stringMapper;

	// Init with 32 channels, and deferred sound
	if(!FSOUND_Init(22050, 32, FSOUND_INIT_DSOUND_DEFERRED))
	{
		throw ESoundDriver("Failed to create the FMod driver object");
	}

	// succeed
	_FModOk= true;
	
	// Allocate buffer in software?
	_ForceSoftwareBuffer= forceSoftwareBuffer;

	// Display Hardware Support
	int		num2D, num3D, numTotal;
	FSOUND_GetNumHWChannels(&num2D, &num3D, &numTotal);
	nlinfo("FMod Hardware Support: %d 2D channels, %d 3D channels, %d Total Channels", num2D, num3D, numTotal);
}

// ******************************************************************

CSoundDriverFMod::~CSoundDriverFMod()
{
	nldebug("Destroying FMOD");

	// Stop any played music
	stopMusic();

	
	// Assure that the remaining sources have released all their channels before closing
	set<CSourceFMod*>::iterator iter;
	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		(*iter)->release();
	}


	// Assure that the listener has released all resources before closing down FMod
	if (CListenerFMod::instance() != 0)
	{
		CListenerFMod::instance()->release();
	}

	// Close FMod
	if(_FModOk)
	{
		FSOUND_Close();
		_FModOk= false;
	}

	_Instance = 0;
}
// ******************************************************************

uint CSoundDriverFMod::countMaxSources()
{
	int		num2D, num3D, numTotal;
	FSOUND_GetNumHWChannels(&num2D, &num3D, &numTotal);

	// Try the hardware 3d buffers first
	if (num3D > 0) 
	{
		return num3D;
	}
	
	// If not, try the hardware 2d buffers first
	if (num2D > 0)
	{
		return num2D;
	}
	
	// Okay, we'll use 32 software buffers
	return 32;
}

// ******************************************************************

void CSoundDriverFMod::writeProfile(string& out)
{
	out+= "\tFMod Driver\n";

    // Write the number of hardware buffers 
	int		num2D, num3D, numTotal;
	FSOUND_GetNumHWChannels(&num2D, &num3D, &numTotal);
	
    out += "\t3d hw buffers: " + toString ((uint32)num3D) + "\n";
	out += "\t2d hw buffers: " + toString ((uint32)num2D) + "\n";
}


// ******************************************************************

void CSoundDriverFMod::update()
{
	H_AUTO(NLSOUND_FModUpdate)
	NLMISC::TTime now = NLMISC::CTime::getLocalTime();

	set<CSourceFMod*>::iterator first(_Sources.begin()), last(_Sources.end());
	for (;first != last; ++first)
	{
		if ((*first)->needsUpdate())
		{
			(*first)->update();
		}
	}
}

// ******************************************************************

IListener *CSoundDriverFMod::createListener()
{
	NL_ALLOC_CONTEXT(NLSOUND_CSoundDriverFMod);

    if (CListenerFMod::instance() != NULL) 
    {
        return CListenerFMod::instance();
    }

    if ( !_FModOk ) 
        throw ESoundDriver("Corrupt driver");
	
    return new CListenerFMod();
}

// ******************************************************************

IBuffer *CSoundDriverFMod::createBuffer()
{
	NL_ALLOC_CONTEXT(NLSOUND_CSoundDriverFMod);

    if ( !_FModOk ) 
        throw ESoundDriver("Corrupt driver");
	
    return new CBufferFMod();
}

// ******************************************************************

void CSoundDriverFMod::removeBuffer(IBuffer *buffer)
{
}

// ***************************************************************************
bool CSoundDriverFMod::readWavBuffer( IBuffer *destbuffer, const std::string &name, uint8 *wavData, uint dataSize)
{
	return ((CBufferFMod*) destbuffer)->readWavBuffer(name, wavData, dataSize);
}

bool CSoundDriverFMod::readRawBuffer( IBuffer *destbuffer, const std::string &name, uint8 *rawData, uint dataSize, TSampleFormat format, uint32 frequency)
{
	return ((CBufferFMod*) destbuffer)->readRawBuffer(name, rawData, dataSize, format, frequency);
}


// ******************************************************************

ISource *CSoundDriverFMod::createSource()
{
	NL_ALLOC_CONTEXT(NLSOUND_CSoundDriverFMod);

    if ( !_FModOk ) 
        throw ESoundDriver("Corrupt driver");
	
	CSourceFMod* src = new CSourceFMod(0);
	src->init();
	_Sources.insert(src);

	return src;
}


// ******************************************************************

void CSoundDriverFMod::removeSource(ISource *source)
{
	_Sources.erase((CSourceFMod*) source);
}

// ******************************************************************

void CSoundDriverFMod::commit3DChanges()
{
    if ( !_FModOk ) 
		return;
	
#if MANUAL_ROLLOFF == 1
	// We handle the volume of the source according to the distance
	// ourselves. Call updateVolume() to, well..., update the volume
	// according to, euh ..., the new distance!
	CListenerFMod* listener = CListenerFMod::instance();
	if(listener)
	{
		const CVector &origin = listener->getPos();
		set<CSourceFMod*>::iterator iter;
		for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
		{
			if ((*iter)->isPlaying()) 
			{
				(*iter)->updateVolume(origin);
			}
		}
	}
#endif
	
	// We handle the "SourceRelative state" ourselves. Updates sources according to current listener position/velocity
	set<CSourceFMod*>::iterator iter;
	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		(*iter)->updateFModPosIfRelative();
	}

	// update sources state each frame though
	update();

	// update the music (XFade etc...)
	updateMusic();

	// update 3D change in FMod
	FSOUND_Update();
}


// ******************************************************************

uint CSoundDriverFMod::countPlayingSources()
{
    uint n = 0;
	set<CSourceFMod*>::iterator iter;

	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		if ((*iter)->isPlaying()) 
		{
			n++;
		}
	}

    return n;
}


// ******************************************************************

void CSoundDriverFMod::setGain( float gain )
{
	clamp(gain, 0.f, 1.f);
	_MasterGain= gain;

	// set FMod volume
    if ( _FModOk ) 
	{
		uint	volume255= (uint)floor(_MasterGain*255);
		FSOUND_SetSFXMasterVolume(volume255);
	}
}

// ******************************************************************

float CSoundDriverFMod::getGain()
{
	return _MasterGain;
}


// ***************************************************************************
void	CSoundDriverFMod::startBench()
{
	NLMISC::CHTimer::startBench();
}
void	CSoundDriverFMod::endBench()
{
	NLMISC::CHTimer::endBench();
}
void	CSoundDriverFMod::displayBench(CLog *log)
{
	NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
	NLMISC::CHTimer::displayHierarchical(log, true, 48, 2);
	NLMISC::CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
	NLMISC::CHTimer::display(log, CHTimer::TotalTime);
}


// ***************************************************************************
void	CSoundDriverFMod::toFModCoord(const CVector &in, float out[3])
{
	out[0]= in.x;
	out[1]= in.z;
	out[2]= in.y;
}

// ***************************************************************************
void	CSoundDriverFMod::playMusicStartFade(uint xFadeTime)
{
	if(xFadeTime==0)
	{
		// stop all channels
		stopMusic();
		_ActiveMusicChannel= 0;
		// setup no cross fade
		_MusicChannel[_ActiveMusicChannel].XFadeVolume= 1.f;
		_MusicChannel[_ActiveMusicChannel].XFadeDVolume= 0.f;
	}
	else
	{
		// setup fade out for old music channel (don't modify previous XFadeVolume, if for instance previous XFade not ended)
		_MusicChannel[_ActiveMusicChannel].XFadeDVolume= -1000.f/xFadeTime;
		
		// get new active channel
		_ActiveMusicChannel++;
		_ActiveMusicChannel= _ActiveMusicChannel%MaxMusicChannel;
		
		// stop this channel if was still played (eg: playMusic() of a third music comes in while old XFade not ended)
		stopMusicChannel(_ActiveMusicChannel);
		
		// setup fade in for new music channel
		_MusicChannel[_ActiveMusicChannel].XFadeVolume= 0.f;
		_MusicChannel[_ActiveMusicChannel].XFadeDVolume= 1000.f/xFadeTime;
	}
	
}

// ***************************************************************************
void	CSoundDriverFMod::playMusicStartChannel(uint musicChannel)
{
	CMusicChannel	&chan= _MusicChannel[musicChannel];

	// if stopped, abort
	if(!chan.FModMusicStream)
		return;

	// if already start played, no op
	if(chan.FModMusicChannel!=-1)
		return;

	// try to start playing
	chan.FModMusicChannel= FSOUND_Stream_PlayEx(FSOUND_FREE, chan.FModMusicStream, NULL, true);
	// may failed for async case
	if(chan.FModMusicChannel!=-1)
	{
		// stereo pan (as reccomended)
		FSOUND_SetPan(chan.FModMusicChannel, FSOUND_STEREOPAN);
		// update this channel volume (xFaded or not)
		updateMusicVolumeChannel(musicChannel);
		// unpause
		FSOUND_SetPaused(chan.FModMusicChannel, false);
	}
}

// ***************************************************************************
bool	CSoundDriverFMod::playMusic(NLMISC::CIFile &fileIn, uint xFadeTime)
{
	if(!_FModOk)
		return false;
	
	// Stop old channels, according to cross fade or not
	playMusicStartFade(xFadeTime);
	
	// start trying to play the corresponding channel
	nlassert(_ActiveMusicChannel<MaxMusicChannel);
	CMusicChannel	&chan= _MusicChannel[_ActiveMusicChannel];
	// should be stoped in playMusicStartFade()
	nlassert(chan.FModMusicStream==NULL);
	
	// try to load the new one in memory
	uint32	fs= fileIn.getFileSize();
	if(fs==0)
		return false;
	
	// read Buffer
	chan.FModMusicBuffer= new uint8 [fs];
	try
	{
		fileIn.serialBuffer(chan.FModMusicBuffer, fs);
	}
	catch(...)
	{
		nlwarning("Sound FMOD: Error While reading music file");
		delete[] chan.FModMusicBuffer;
		chan.FModMusicBuffer= NULL;
		return false;
	}

	// Load to a stream FMOD sample
	chan.FModMusicStream= FSOUND_Stream_Open((const char*)chan.FModMusicBuffer, 
		FSOUND_2D|FSOUND_LOADMEMORY|FSOUND_LOOP_NORMAL, 0, fs);
	// not succeed?
	if(!chan.FModMusicStream)
	{
		nlwarning("Sound FMOD: Error While creating the FMOD stream for music file");
		delete[] chan.FModMusicBuffer;
		chan.FModMusicBuffer= NULL;
		return false;
	}
	
	// start to play this stream (if ok)
	playMusicStartChannel(_ActiveMusicChannel);

	return true;
}

// ***************************************************************************
bool	CSoundDriverFMod::playMusicAsync(const std::string &path, uint xFadeTime, uint fileOffset, uint fileSize)
{
	if(!_FModOk)
		return false;
	
	// Stop old channels, according to cross fade or not
	playMusicStartFade(xFadeTime);
	
	// start trying to play the corresponding channel
	nlassert(_ActiveMusicChannel<MaxMusicChannel);
	CMusicChannel	&chan= _MusicChannel[_ActiveMusicChannel];
	// should be stoped in playMusicStartFade()
	nlassert(chan.FModMusicStream==NULL);
	
	// Start FMod
	chan.FModMusicStream= FSOUND_Stream_Open((const char*)path.c_str(), 
		FSOUND_2D|FSOUND_LOOP_NORMAL|FSOUND_NONBLOCKING, fileOffset, fileSize);
	// with FSOUND_NONBLOCKING, should always succeed
	nlassert(chan.FModMusicStream);
	
	// with FSOUND_NONBLOCKING, the file is surely not ready, but still try now (will retry to replay at each updateMusic())
	playMusicStartChannel(_ActiveMusicChannel);

	return true;
}

// ***************************************************************************
void	CSoundDriverFMod::stopMusic(uint xFadeTime)
{
	if(!_FModOk)
		return;
	
	// if no cross fade
	if(xFadeTime==0)
	{
		// stop Music on all channels
		for(uint i=0;i<MaxMusicChannel;i++)
			stopMusicChannel(i);
	}
	// else don't stop now, but fade out
	else
	{
		// setup all playing channels to fade out
		for(uint i=0;i<MaxMusicChannel;i++)
		{
			CMusicChannel	&chan= _MusicChannel[i];
			
			// if playing, setup fade out (don't modify previous XFadeVolume, if for instance previous XFade not ended)
			if(chan.FModMusicStream)
				chan.XFadeDVolume= -1000.f/xFadeTime;
		}	
	}
}

// ***************************************************************************
void	CSoundDriverFMod::stopMusicChannel(uint musicChannel)
{
	nlassert(musicChannel<MaxMusicChannel);
	CMusicChannel	&chan= _MusicChannel[musicChannel];

	// if playing some music
	if(chan.FModMusicStream)
	{
		/* just append this channel for closing. We have to maintain such a list because in case of async playing,
			FMod FSOUND_Stream_Stop() and FSOUND_Stream_Close() calls fail if the file is not ready (hapens if music stoped
			in the 50 ms after the play for instance)
		*/
		_FModMusicStreamWaitingForClose.push_back(chan.FModMusicStream);
		// force stop now (succeed in 99% of case, and if not succeed, it means the play has not yet begun, so no problem)
		updateMusicFModStreamWaitingForClose();
		
		// reset
		chan.FModMusicChannel= -1;
		chan.FModMusicStream= NULL;
		// delete the music buffer (only != NULL if !async)
		delete[] chan.FModMusicBuffer;
		chan.FModMusicBuffer= NULL;
		// just to be clean
		chan.XFadeVolume= 0.f;
		chan.XFadeDVolume= 0.f;
	}
}


// ***************************************************************************
void	CSoundDriverFMod::setMusicVolume(float gain)
{
	if(!_FModOk)
		return;

	// bkup the volume
	clamp(gain, 0.f, 1.f);
	_FModMusicVolume= gain;

	// update volume of all music channels
	for(uint i=0;i<MaxMusicChannel;i++)
		updateMusicVolumeChannel(i);
}

// ***************************************************************************
void	CSoundDriverFMod::updateMusicVolumeChannel(uint musicChannel)
{
	nlassert(musicChannel<MaxMusicChannel);
	CMusicChannel	&chan= _MusicChannel[musicChannel];

	// the channel must be really playing (may not be the case for async play)
	if(chan.FModMusicStream && chan.FModMusicChannel!=-1)
	{
		float	realVol= chan.XFadeVolume*_FModMusicVolume;
		clamp(realVol, 0.f, 1.f);
		uint8	vol255= uint(realVol*255);
		FSOUND_SetVolumeAbsolute(chan.FModMusicChannel, vol255);
	}
}

// ***************************************************************************
void	CSoundDriverFMod::updateMusic()
{
	// get the dt for fade
	sint64	t1= CTime::getLocalTime();
	float	dt;
	if(_LastXFadeTime==0)
		dt= 0;
	else
		dt= (t1-_LastXFadeTime)/1000.f;
	_LastXFadeTime= t1;

	// update all channel
	for(uint i=0;i<MaxMusicChannel;i++)
	{
		CMusicChannel	&chan= _MusicChannel[i];
		
		// **** may start now the play (async playing)
		// if this channel is playing an async music, may retry to start the music each frame
		if(chan.FModMusicStream && chan.FModMusicBuffer==NULL)
		{
			if(chan.FModMusicChannel==-1)
			{
				playMusicStartChannel(i);
			}
		}

		// **** update fading
		// if playing and some fadein/fadeout
		if(chan.FModMusicStream && chan.XFadeDVolume)
		{
			// update volume
			chan.XFadeVolume+= chan.XFadeDVolume*dt;

			// if fade in and max reached, stop fade in
			if(chan.XFadeDVolume>0 && chan.XFadeVolume>1.f)
			{
				chan.XFadeVolume= 1.f;
				chan.XFadeDVolume= 0;
			}
			// if fade out and min reached, stop music and fade out!
			else if(chan.XFadeDVolume<0 && chan.XFadeVolume<0.f)
			{
				chan.XFadeVolume= 0.f;
				chan.XFadeDVolume= 0;
				stopMusicChannel(i);
			}

			// update the actual volume (NB: if just stoped, will work, see updateMusicVolumeChannel)
			updateMusicVolumeChannel(i);
		}
	}

	// flush the FMod stream waiting for close
	updateMusicFModStreamWaitingForClose();
}

// ***************************************************************************
void	CSoundDriverFMod::updateMusicFModStreamWaitingForClose()
{
	std::list<FSOUND_STREAM*>::iterator	it= _FModMusicStreamWaitingForClose.begin();
	while(it!=_FModMusicStreamWaitingForClose.end())
	{
		bool	ok= FSOUND_Stream_Stop(*it)!=0;
		if(ok)
			ok= FSOUND_Stream_Close(*it)!=0;
		// erase from list, or next
		if(ok)
			it= _FModMusicStreamWaitingForClose.erase(it);
		else
			it++;
	}
}


} // NLSOUND
