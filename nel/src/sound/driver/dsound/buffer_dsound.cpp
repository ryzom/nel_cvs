/** \file buffer_dsound.cpp
 * DirectSound sound buffer
 *
 * $Id: buffer_dsound.cpp,v 1.4 2002/06/11 09:36:09 hanappe Exp $
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
#include "buffer_dsound.h"
#include "nel/misc/path.h"


#include <windows.h>
#include <mmsystem.h>

using namespace NLMISC;
using namespace std;

namespace NLSOUND {


CBufferDSound::CBufferDSound()
{
    _Data = NULL;
    _Size = 0; 
    _Format = Mono16;
    _Freq = 0;
}

CBufferDSound::~CBufferDSound()
{
	nldebug("Destroying DirectSound buffer");

    if (_Data != NULL)
    {
        delete[] _Data;
    }
}


void CBufferDSound::setFormat( TSampleFormat format, uint freq )
{
    _Format = format;
    _Freq = freq;
}


bool CBufferDSound::fillBuffer( void *src, uint32 bufsize )
{
    return false;
}


float CBufferDSound::getDuration() const
{
    float frames = (float) _Size;

    switch (_Format) {
    case Mono8:
        break;
    case Mono16:
        frames /= 2.0f;
        break;
    case Stereo8:
        frames /= 2.0f;
        break;
    case Stereo16:
        frames /= 4.0f;
        break;
    }

    return 1000.0f * frames / (float) _Freq;
}


bool CBufferDSound::loadWavFile(const char* file) 
{
    sint error; 
    sint32 num;
    HMMIO hmmio;
    WAVEFORMATEX format;
    MMCKINFO riff_chunk;
    MMCKINFO data_chunk;
    MMCKINFO chunk;

  
    if (_Data != NULL)
    {
        free(_Data);
        _Data = NULL;
    }


    // Open the file 
    hmmio = mmioOpen((char*) file, NULL, MMIO_READ | MMIO_DENYWRITE);

    if (hmmio == NULL) 
    {
        throw ESoundDriver("Failed to open the file");
    }


    // Check it's a WAVE file 
    riff_chunk.ckid = FOURCC_RIFF;

    error = (sint) mmioDescend(hmmio, &riff_chunk, NULL, 0);

    if ((error != 0) || (riff_chunk.ckid != FOURCC_RIFF) || (riff_chunk.fccType != mmioFOURCC('W', 'A', 'V', 'E'))) 
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Not a WAVE file");
    }


    // Search the format chunk 
    chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');

    error = (sint) mmioDescend(hmmio, &chunk, &riff_chunk, MMIO_FINDCHUNK);

    if (error != 0) 
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Couldn't find the format chunk");
    }

    if (chunk.cksize < (long) sizeof(PCMWAVEFORMAT)) 
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Invalid format chunk size");
    }


    // read in the format data

    num = mmioRead(hmmio, (HPSTR) &format, (long) sizeof(format));
    if (num != (long) sizeof(format)) 
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Read failed");
    }

    format.cbSize = 0;

    // Get out of the format chunk

    if (mmioAscend(hmmio, &chunk, 0) != 0) 
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Read failed");
    }


    // copy the format data 

    if (format.wFormatTag != WAVE_FORMAT_PCM) 
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Unsupported sample format");
    }

    _Freq = format.nSamplesPerSec;

    if (format.nChannels == 1) 
    {
        if (format.wBitsPerSample == 8)
        {
            _Format = Mono8;
        } 
        else if (format.wBitsPerSample == 16)
        {
            _Format = Mono16;
        }
        else
        {
            mmioClose(hmmio, 0);
            throw ESoundDriver("Unsupported sample size");
        }
    }
    else if (format.nChannels == 2)
    {
        if (format.wBitsPerSample == 8)
        {
            _Format = Stereo8;
        } 
        else if (format.wBitsPerSample == 16)
        {
            _Format = Stereo16;
        }
        else
        {
            mmioClose(hmmio, 0);
            throw ESoundDriver("Unsupported sample size");
        }
    }
    else // Shouldn't normally happen
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Unsupported number of channels");
    }

    
    // Set the file position to the beginning of the data chunk */
 
    sint32 pos = mmioSeek(hmmio, riff_chunk.dwDataOffset + sizeof(FOURCC), SEEK_SET);

    if (pos < 0) 
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Read to set the read position");
    }

    data_chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');

    if (mmioDescend(hmmio, &data_chunk, &riff_chunk, MMIO_FINDCHUNK) != 0) 
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Read to set the read position");
    }


    // Allocate the sample buffer
    _Size = data_chunk.cksize;

    _Data = new uint8[_Size];

    if (_Data == NULL)
    {
        mmioClose(hmmio, 0);
        throw ESoundDriver("Out of memory");
    }


    // Read the sample data

    num = mmioRead(hmmio, (HPSTR) _Data, _Size);

    if (num < 0) 
    {
        throw ESoundDriver("Failed to read the samples");
    }
    else if ((uint32) num < _Size)
    {
        // FIXME: print warning or throw exception ???
        char* p = (char*) _Data;
        ZeroMemory(p + num, _Size - num);
    }


    mmioClose(hmmio, 0);


	_Name = CFile::getFilenameWithoutExtension(file);

	return true;

}


} // NLSOUND



