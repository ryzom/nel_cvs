/** \file sound_driver_dsound.cpp
 * DirectSound driver
 *
 * $Id: sound_driver_dsound.cpp,v 1.1 2002/05/24 16:50:48 hanappe Exp $
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


// The one and only INITGUID
#define INITGUID

#include "stddsound.h"


#include "sound_driver_dsound.h"
#include "listener_dsound.h"
#include "source_dsound.h"
#include "buffer_dsound.h"

using namespace std;
using namespace NLMISC;


namespace NLSOUND {

CSoundDriverDSound* CSoundDriverDSound::_Instance = NULL;
uint32 CSoundDriverDSound::_TimerPeriod = 100;
HINSTANCE CSoundDriverDllHandle = 0;
HWND CSoundDriverWnd = 0;


/* The main entry of the DLL. It's used to get a hold of the hModule handle. */
BOOL WINAPI DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  CSoundDriverDllHandle = (HINSTANCE) hModule;
  return TRUE;
}


/* The event handling procedure of the invisible window created below. */
long FAR PASCAL CSoundDriverCreateWindowProc(HWND hWnd, unsigned message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, message, wParam, lParam);
}



__declspec(dllexport) ISoundDriver *NLSOUND_createISoundDriverInstance()
{

	// Don't ask me why we have to create a window to do sound!
	// echo <your comment> | mail support@microsoft.com -s "F#%@cking window"
	WNDCLASS myClass;
	myClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	myClass.hIcon = NULL; 
	myClass.lpszMenuName = (LPSTR) NULL;
	myClass.lpszClassName = (LPSTR) "CSoundDriver";
	myClass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	myClass.hInstance = CSoundDriverDllHandle;
	myClass.style = CS_GLOBALCLASS;
	myClass.lpfnWndProc = CSoundDriverCreateWindowProc;
	myClass.cbClsExtra = 0;
	myClass.cbWndExtra = 0;

	if (!RegisterClass(&myClass)) 
	{
		nlwarning("Failed to initialize the sound driver (RegisterClass)");
		return 0;
	}

	CSoundDriverWnd = CreateWindow((LPSTR) "CSoundDriver", (LPSTR) "CSoundDriver", WS_OVERLAPPEDWINDOW,
									CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, (HWND) NULL, (HMENU) NULL, 
									CSoundDriverDllHandle, (LPSTR) NULL);  

	if (CSoundDriverWnd == NULL)
	{
		nlwarning("Failed to initialize the sound driver (CreateWindow)");
		return 0;
	}
	
	CSoundDriverDSound *driver = new CSoundDriverDSound();
	driver->init(CSoundDriverWnd);

	return driver;
}


__declspec(dllexport) uint32 NLSOUND_interfaceVersion()
{
	return ISoundDriver::InterfaceVersion;
}


__declspec(dllexport) void NLSOUND_outputProfile(ostream &out)
{
	CSoundDriverDSound::instance()->writeProfile(out);
}






/*
 * Constructor
 */
CSoundDriverDSound::CSoundDriverDSound() : ISoundDriver()
{
	if ( _Instance == NULL )
	{
		_Instance = this;

        _DirectSound = NULL;
        _PrimaryBuffer = NULL;
        _SourceCount = 0;
        _Sources = NULL;
        _TimerID = NULL;

#if NLSOUND_PROFILE
        _TimerIntervalCount = 0;
        _TotalTime = 0.0;
        _TotalUpdateTime = 0.0;
		_UpdateCount = 0;
		_UpdateSources = 0;
		_UpdateExec = 0;
#endif

    }
	else
	{
		nlerror("Sound driver singleton instanciated twice");
	}
}


/*
 * Destructor
 */
CSoundDriverDSound::~CSoundDriverDSound()
{
    if (_TimerID != NULL)
    {
        timeKillEvent(_TimerID);
        timeEndPeriod(_TimerResolution); 
    }

    if (_Sources != NULL) 
    {
        for (uint i = 0; i < _SourceCount; i++) 
        {
            if (_Sources[i] != NULL)
            {
                delete _Sources[i];
            }
        }

        delete[] _Sources;
        _Sources = NULL;
    }

    if (_PrimaryBuffer != NULL) 
    {
        _PrimaryBuffer->Release(); 
        _PrimaryBuffer = NULL;
    }

    if (_DirectSound != NULL) 
    {
        _DirectSound->Release(); 
        _DirectSound = NULL;
    }
}

class CDeviceDescription
{
public:

    static CDeviceDescription* _List;

    CDeviceDescription(LPGUID guid, const char* descr) 
    {
        _Guid = guid;
        _Description = strdup(descr);
        _Next = _List;
        _List = this;
    }

    virtual ~CDeviceDescription()
    {
        if (_Description) 
        {
            free(_Description);
        }
        if (_Next)
        {
            delete _Next;
        }
    }

    char* _Description;
    CDeviceDescription* _Next;
    LPGUID _Guid;
};

CDeviceDescription* CDeviceDescription::_List = 0;


BOOL CALLBACK CSoundDriverDSoundEnumCallback(LPGUID guid, LPCSTR description, PCSTR module, LPVOID context)
{
    new CDeviceDescription(guid, description);
    return TRUE;
}


/*
 * Initialization
 */
bool CSoundDriverDSound::init(HWND wnd)
{
    if (FAILED(DirectSoundEnumerate(CSoundDriverDSoundEnumCallback, this)))
    {
        throw ESoundDriver("Failed to enumerate the DirectSound devices");
    }

    // Create a DirectSound object and set the cooperative level.

    if (DirectSoundCreate(NULL, &_DirectSound, NULL) != DS_OK) 
    {
        throw ESoundDriver("Failed to create the DirectSound object");
    }


    if (_DirectSound->SetCooperativeLevel(wnd, DSSCL_PRIORITY) != DS_OK) 
    {
        throw ESoundDriver("Failed to set the cooperative level");
    }


    // Analyse the capabilities of the sound driver/device

    _Caps.dwSize = sizeof(_Caps); 

    if (_DirectSound->GetCaps(&_Caps) != DS_OK)  
    {
        throw ESoundDriver("Failed to query the sound device caps");
    }


    // Create primary buffer 
 
    DSBUFFERDESC desc;

    ZeroMemory(&desc, sizeof(DSBUFFERDESC));
    desc.dwSize = sizeof(DSBUFFERDESC);


    // First, try to allocate a 3D hardware buffer.
    // If we can't get a 3D hardware buffer, use a 2D hardware buffer.
    // As last option, use a 2D software buffer.

    if (countHw3DBuffers() > 0) 
    {
		nldebug("Primary buffer: Allocating 3D buffer in hardware");
        desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE | DSBCAPS_CTRL3D;
    } 
    else
    {
 		nldebug("Primary buffer: Allocating 3D buffer in software");
        desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRL3D;
        desc.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
    }


    if (_DirectSound->CreateSoundBuffer(&desc, &_PrimaryBuffer, NULL) != DS_OK) 
    {

 		nlwarning("Primary buffer: Failed to create a buffer with 3D capabilities.");

		ZeroMemory(&desc, sizeof(DSBUFFERDESC));
		desc.dwSize = sizeof(DSBUFFERDESC);

		if (countHw2DBuffers() > 0) 
		{
			nldebug("Primary buffer: Allocating 2D buffer in hardware");
			desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE;
		} 
		else
		{
 			nldebug("Primary buffer: Allocating 2D buffer in software");
			desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE;
		}

		if (_DirectSound->CreateSoundBuffer(&desc, &_PrimaryBuffer, NULL) != DS_OK) 
		{
			throw ESoundDriver("Failed to create the primary buffer");
		}
    }


    // Set the format of the primary buffer

    WAVEFORMATEX format;

    format.cbSize = sizeof(WAVEFORMATEX);

    // Make sure the sound card accepts the default settings.
    // For now, only the default settings are accepted. Fallback
    // strategy will be handled later.

    if ((_Caps.dwMinSecondarySampleRate > 22050) && (22050 > _Caps.dwMaxSecondarySampleRate)) {
        throw ESoundDriver("Unsupported sample rate range");        
    }   

    if (_Caps.dwFlags & DSCAPS_PRIMARY16BIT == 0) {
        throw ESoundDriver("Unsupported sample size [16bits]");        
    }   

    format.wBitsPerSample = 16;
    format.nChannels = 1;
    format.nSamplesPerSec = 22050;
    format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.wFormatTag = WAVE_FORMAT_PCM;


    if (_PrimaryBuffer->SetFormat(&format) != DS_OK) 
    {
        throw ESoundDriver("Failed to create set the format of the primary buffer");
    }


    uint32 numBuffers = countHw3DBuffers();
	if (numBuffers == 0)
	{
		numBuffers = 31;
	}

    _Sources = new CSourceDSound*[numBuffers];


	for (uint i = 0; i < numBuffers; i++) 
	{
		_Sources[i] = 0;
	}

	try
	{
	    for (i = 0; i < numBuffers; i++) 
		{
			_Sources[i] = new CSourceDSound(i);
			_Sources[i]->init(_DirectSound);
			_SourceCount++;
		}
	}
	catch (ESoundDriver& e)
	{
		// Okay, here's the situation: I'm listening to WinAmp while debugging.
		// The caps told me there were 31 buffers available. In reality, there were
		// only 30 available because WinAmp was using one. Somehow DirectSound didn't
		// notice. So when creating buffer 31, an exception was thrown. 
		// If something like this happens, don't bother and go on with the buffers we've 
		// got. If no buffers are created at all, throw the exception again.

		if (_Sources == 0)
		{
			throw e;
		}
	}


    TIMECAPS tcaps;
    
    timeGetDevCaps(&tcaps, sizeof(TIMECAPS));
    _TimerResolution = (tcaps.wPeriodMin > 10)? tcaps.wPeriodMin : 10;
    timeBeginPeriod(_TimerResolution); 



#if NLSOUND_PROFILE
    for (i = 0; i < 1024; i++)
    {
        _TimerInterval[i] = 0;
    }

    _TimerDate = CTime::getPerformanceTime();
#endif




    _TimerID = timeSetEvent(_TimerPeriod, 0, &CSoundDriverDSound::TimerCallback, (DWORD)this, TIME_CALLBACK_FUNCTION | TIME_PERIODIC);

    if (_TimerID == NULL)
    {
        throw ESoundDriver("Failed to create the timer");
    }

        


    return true;
}



void CSoundDriverDSound::writeProfile(ostream& out)
{
    // Write the available sound devices
    CDeviceDescription* list = CDeviceDescription::_List;
    while (list) {
		out << list->_Description << "\n";
        list = list->_Next;
    }

    out << "\n";

    // Write the buffers sizes
    out << "buffer size: " << CSourceDSound::_SecondaryBufferSize << "\n";
    out << "copy size: " << CSourceDSound::_UpdateCopySize << "\n";
    out << "swap size: " << CSourceDSound::_SwapCopySize << "\n";
    out << "\n";

    // Write the number of hardware buffers 
    DSCAPS caps;
    caps.dwSize = sizeof(caps); 
    _DirectSound->GetCaps(&caps);  

    cout << "3d hw buffers: " << caps.dwMaxHw3DAllBuffers << "\n";
	cout << "2d hw buffers: " << caps.dwMaxHwMixingAllBuffers << "\n";
    out << "\n";

    // Write the number of hardware buffers 
    out << "update time total --- " <<  getAverageUpdateTime()<< "\n";
	out << "update time source --- " << CSourceDSound::getAverageUpdateTime() << "\n";
	out << "update --- t: " << CSourceDSound::getAverageCumulTime();
	out << " - p: " << CSourceDSound::getAveragePosTime();
	out << " - l: " << CSourceDSound::getAverageLockTime();
	out << " - c: " << CSourceDSound::getAverageCopyTime();
	out << " - u: " << CSourceDSound::getAverageUnlockTime() << "\n";
	out << "update percentage: --- " << getUpdatePercentage() << "\n";
	out << "update num sources --- " << getAverageUpdateSources() << "\n";
	out << "update byte size --- " << CSourceDSound::getAverageUpdateSize() << "\n";
	out << "swap time --- " << CSourceDSound::getTestAverage() << "\n";
	out << "src --- " << countPlayingSources() << "\n";
}


void CALLBACK CSoundDriverDSound::TimerCallback(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    CSoundDriverDSound* driver = (CSoundDriverDSound*) dwUser;
    driver->update();
}



void CSoundDriverDSound::update()
{
#if NLSOUND_PROFILE
    TTicks now = CTime::getPerformanceTime();
#endif

	if (_Sources[0]->needsUpdate())
	{
#if NLSOUND_PROFILE
		_UpdateExec++;
#endif

		for (uint i = 0; i < _SourceCount; i++) 
		{
			if (_Sources[i] != NULL) 
			{
				if (_Sources[i]->update2()) {
#if NLSOUND_PROFILE
					_UpdateSources++;
#endif
				}
			}
		}
	}

#if NLSOUND_PROFILE
    _TotalUpdateTime += 1000.0 * CTime::ticksToSecond(CTime::getPerformanceTime() - now);
	_UpdateCount++;
#endif
}


/**
 *  Count the number of available 3D hardware buffers
 */
uint CSoundDriverDSound::countHw3DBuffers()
{
    DSCAPS caps;
    caps.dwSize = sizeof(caps); 

    if (_DirectSound->GetCaps(&caps) != DS_OK)  
    {
        throw ESoundDriver("Failed to query the sound device caps");
    }

    return caps.dwFreeHw3DStreamingBuffers;
}


/**
 *  Count the number of available 3D software buffers
 */
uint CSoundDriverDSound::countHw2DBuffers()
{
    DSCAPS caps;
    caps.dwSize = sizeof(caps); 

    if (_DirectSound->GetCaps(&caps) != DS_OK)  
    {
        throw ESoundDriver("Failed to query the sound device caps");
    }

    return caps.dwFreeHwMixingStreamingBuffers;
}


/**
 *  Create the listener
 */
IListener *CSoundDriverDSound::createListener()
{
    LPDIRECTSOUND3DLISTENER dsoundListener;

    if (CListenerDSound::instance() != NULL) 
    {
        return CListenerDSound::instance();
    }

    if (_PrimaryBuffer == 0) 
    {
        throw ESoundDriver("Corrupt driver");
    }

    if (FAILED(_PrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID *) &dsoundListener)))
    {
		nlwarning("The 3D listener interface is not available.");
        return new CListenerDSound(NULL);
    }

    return new CListenerDSound(dsoundListener);
}


/** 
 *  Create a sound buffer 
 */
IBuffer *CSoundDriverDSound::createBuffer()
{
    if (_PrimaryBuffer == 0) 
    {
        throw ESoundDriver("Corrupt driver");
    }


    // FIXME: set buffer ID
    return new CBufferDSound(0);   
}


/**
 *  Remove a buffer (should be called by the friend destructor of the buffer class)
 */
void CSoundDriverDSound::removeBuffer(IBuffer *buffer)
{
	delete buffer;
}



/**
 *  Load a wave file into the buffer
 */
bool CSoundDriverDSound::loadWavFile(IBuffer *destbuffer, const char *filename)
{
	return ((CBufferDSound*) destbuffer)->loadWavFile(filename);
}



/**
 *  Create a new source
 */
ISource *CSoundDriverDSound::createSource()
{
    if (_PrimaryBuffer == 0) 
    {
        throw ESoundDriver("Corrupt driver");
    }


    for (uint i = 0; i < _SourceCount; i++) 
    {
        if ((_Sources[i] != NULL) && _Sources[i]->isStopped())
        {
            return _Sources[i];
        }
    }
    
    return 0;
}


/** 
 *  Remove a source (should be called by the friend destructor of the source class) 
 */
void CSoundDriverDSound::removeSource(ISource *source)
{
	// Do nothing. All sources are allocated at init() and released in the deconstructor.
}


uint CSoundDriverDSound::countPlayingSources()
{
    uint n = 0;

    for (uint i = 0; i < _SourceCount; i++)
    {
        if ((_Sources[i] != NULL) && _Sources[i]->isPlaying())
        {
            n++;
        }
    }

    return n;
}




#if NLSOUND_PROFILE

uint CSoundDriverDSound::countTimerIntervals()
{
    return 1024;
}


uint CSoundDriverDSound::getTimerIntervals(uint index)
{           
    return _TimerInterval[index];
}


void CSoundDriverDSound::addTimerInterval(uint32 dt) 
{
    if (_TimerIntervalCount >= 1024)
    {
        _TimerIntervalCount = 0;
    }

    _TimerInterval[_TimerIntervalCount++] = dt;
}



double CSoundDriverDSound::getCPULoad()
{
    return (_TotalTime > 0.0)? 100.0 * _TotalUpdateTime / _TotalTime : 0.0;  
}



void CSoundDriverDSound::printDriverInfo(FILE* fp)
{
    CDeviceDescription* list = CDeviceDescription::_List;

    while (list) {
        fprintf(fp, "%s\n", list->_Description);
        list = list->_Next;
    }

    fprintf(fp, "\n");

    fprintf(fp, "buffer size: %d\n"
				"copy size: %d\n"
				"swap size: %d\n", 
			CSourceDSound::_SecondaryBufferSize,
			CSourceDSound::_UpdateCopySize,
			CSourceDSound::_SwapCopySize);

    fprintf(fp, "\n");

    DSCAPS caps;
    caps.dwSize = sizeof(caps); 

    if (_DirectSound->GetCaps(&caps) != DS_OK)  
    {
        throw ESoundDriver("Failed to query the sound device caps");
    }

    
    fprintf(fp, "3d hw buffers: %d\n" "2d hw buffers: %d\n\n", caps.dwMaxHw3DAllBuffers, caps.dwMaxHwMixingAllBuffers);
}

#endif


} // NLSOUND
