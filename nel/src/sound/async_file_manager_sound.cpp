/** \file async_file_manager_sound.cpp
 * <File description>
 *
 * $Id: async_file_manager_sound.cpp,v 1.1 2002/11/04 15:40:43 boucher Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "stdsound.h"
#include "async_file_manager_sound.h"
#include "nel/misc/async_file_manager.h"
#include "driver/sound_driver.h"

using namespace NLMISC;

namespace NLSOUND
{

CAsyncFileManagerSound	*CAsyncFileManagerSound::_Singleton;


CAsyncFileManagerSound &CAsyncFileManagerSound::getInstance()
{
	if (_Singleton == NULL)
	{
		_Singleton = new CAsyncFileManagerSound();
	}
	return *_Singleton;
}


void	CAsyncFileManagerSound::terminate()
{
	if (_Singleton != NULL)
	{
		delete &getInstance();
		_Singleton = NULL;
	}
}

void	CAsyncFileManagerSound::loadWavFile(IBuffer *pdestBuffer, const std::string &filename)
{
	CAsyncFileManager::getInstance().addLoadTask(new CLoadWavFile(pdestBuffer, filename));
}


class CCancelLoadWavFile : public CAsyncFileManager::ICancelCallback
{
	std::string	_Filename;

	bool callback(const NLMISC::IRunnable *prunnable) const
	{
		const CAsyncFileManagerSound::CLoadWavFile *pLWF = dynamic_cast<const CAsyncFileManagerSound::CLoadWavFile*>(prunnable);

		if (pLWF != NULL)
		{
			if (pLWF->_Filename == _Filename)
				return true;
		}
		return false;
	}

public:
	CCancelLoadWavFile (const std::string &filename)
		: _Filename (filename)
	{}
};

void	CAsyncFileManagerSound::cancelLoadWaveFile(const std::string &filename)
{
//	CAsyncFileManager::getInstance().cancelLoadTask(CCancelLoadWavFile(filename));
}


// Do not use these methods with the bigfile manager
void CAsyncFileManagerSound::loadFile (const std::string &fileName, uint8 **pPtr)
{
	CAsyncFileManager::getInstance().loadFile(fileName, pPtr);
}

void CAsyncFileManagerSound::loadFiles (const std::vector<std::string> &vFileNames, const std::vector<uint8**> &vPtrs)
{
	CAsyncFileManager::getInstance().loadFiles(vFileNames, vPtrs);
}

void CAsyncFileManagerSound::signal (bool *pSgn)
{
	CAsyncFileManager::getInstance().signal(pSgn);
}

void CAsyncFileManagerSound::cancelSignal (bool *pSgn)
{
	CAsyncFileManager::getInstance().cancelSignal(pSgn);
}


// Load task.
CAsyncFileManagerSound::CLoadWavFile::CLoadWavFile (IBuffer *pdestBuffer, const std::string &filename)
:_Filename(filename), _pDestbuffer(pdestBuffer)
{}

void CAsyncFileManagerSound::CLoadWavFile::run (void)
{
	nldebug("Loading sample %s...", _Filename.c_str());
//	nlSleep(500);
	CAudioMixerUser::instance()->getSoundDriver()->loadWavFile(_pDestbuffer, _Filename.c_str());
}
	


} // NLSOUND