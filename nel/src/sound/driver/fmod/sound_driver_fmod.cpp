/** \file sound_driver_fmod.cpp
 * DirectSound driver
 *
 * $Id: sound_driver_fmod.cpp,v 1.1.2.1 2004/09/09 14:02:32 berenguier Exp $
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

__declspec(dllexport) ISoundDriver *NLSOUND_createISoundDriverInstance(bool useEax, ISoundDriver::IStringMapperProvider *stringMapper)
{
	NL_ALLOC_CONTEXT(NLSOUND_ISoundDriver);

	CSoundDriverFMod *driver = new CSoundDriverFMod();
	driver->init(stringMapper);

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
    }
	else
	{
		nlerror("Sound driver singleton instanciated twice");
	}
}


// ******************************************************************

void	CSoundDriverFMod::init(IStringMapperProvider *stringMapper)
{
	_StringMapper = stringMapper;

	// Init with 32 channels, and deferred sound
	if(!FSOUND_Init(22050, 32, FSOUND_INIT_DSOUND_DEFERRED))
	{
		throw ESoundDriver("Failed to create the FMod driver object");
	}

	// succeed
	_FModOk= true;
	
	// Display Hardware Support
	int		num2D, num3D, numTotal;
	FSOUND_GetNumHWChannels(&num2D, &num3D, &numTotal);
	nlinfo("FMod Hardware Support: %d 2D channels, %d 3D channels, %d Total Channels", num2D, num3D, numTotal);
}

// ******************************************************************

CSoundDriverFMod::~CSoundDriverFMod()
{
	nldebug("Destroying FMOD");

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



} // NLSOUND
