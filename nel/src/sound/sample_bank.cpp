/** \file sample_bank.cpp
 * CSampleBank: a set of sound samples
 *
 * $Id: sample_bank.cpp,v 1.8 2002/11/25 14:11:41 boucher Exp $
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

#include "stdsound.h"

#include "sample_bank.h"
#include "driver/sound_driver.h"
#include "driver/buffer.h"
#include "nel/misc/path.h"
#include "async_file_manager_sound.h"
#include "sound_bank.h"

//#include <crtdbg.h>

using namespace std;
using namespace NLMISC;


namespace NLSOUND {

/// Constante for the number of file to load asynchronously at a time.
uint32		ASYNC_LOADING_SPLIT = 10;		// 10 file by 10 file

CSampleBank::TSampleBankContainer CSampleBank::_Banks;
uint	CSampleBank::_LoadedSize = 0;



// ********************************************************
CSampleBank *CSampleBank::findSampleBank(const std::string &filename)
{
	TSampleBankContainer::iterator	it(_Banks.find(filename));

	if (it != _Banks.end())
		return it->second;

	return NULL;
}

// ********************************************************

IBuffer*		CSampleBank::get(const std::string &name)
{
	IBuffer* buffer;
	TSampleBankContainer::iterator iter;

	for (iter = _Banks.begin(); iter != _Banks.end(); ++iter)
	{
		buffer = iter->second->getSample(name);
		if (buffer != 0)
		{
			return buffer;
		}
	}
	
	//nlwarning ("Try to get an unknown sample '%s'", name);
	return 0;
}

void	 CSampleBank::reload(bool async)
{
	TSampleBankContainer::iterator first(_Banks.begin()), last(_Banks.end());

	for (; first != last; ++first)
	{
		first->second->unload();
		first->second->load(async);
	}
}



// ********************************************************

CSampleBank::CSampleBank(const std::string& path, ISoundDriver *sd) 
	: _SoundDriver(sd), _Path(path), _Loaded(false), _LoadingDone(true), _ByteSize(0)
{
//_CrtCheckMemory();
	_Name = CFile::getFilenameWithoutExtension(_Path);
	_Banks.insert(make_pair(path, this));
//_CrtCheckMemory();
}


// ********************************************************

CSampleBank::~CSampleBank()
{
//_CrtCheckMemory();
	while (!_LoadingDone)
	{
		// need to wait for loading end.
		nlSleep(100);
	}
//_CrtCheckMemory();

	// remove the bank from the list of known banks
	TSampleBankContainer::iterator iter(_Banks.begin()), end(_Banks.end());
//_CrtCheckMemory();

	for (; iter != end; ++iter)
	{
		if (iter->second == this)
		{
			_Banks.erase(iter);
			break;
		}
	}
//_CrtCheckMemory();


	// delete all the samples.
	while (!_Samples.empty())
	{
//_CrtCheckMemory();
		delete _Samples.begin()->second;
		_Samples.erase(_Samples.begin());
//_CrtCheckMemory();
	}
}


// ********************************************************

void				CSampleBank::load(bool async)
{
	nldebug("Loading sample bank %s %", _Name.c_str(), async?"":"Asynchronously");

	vector<string> filenames;
	vector<string>::iterator iter;

	if (_Loaded)
	{
		nlwarning("Trying to load an already loaded bank : %s", _Path);
		return;
	}

	_LoadingDone = false;

	CPath::getPathContent(_Path, true, false, true, filenames);

	for (iter = filenames.begin(); iter != filenames.end(); iter++)
	{
		IBuffer* buffer = NULL;
		try
		{
			buffer = _SoundDriver->createBuffer();
			nlassert(buffer);

			std::string sampleName(CFile::getFilenameWithoutExtension(*iter));

			if (async)
			{
				buffer->presetName(sampleName);
				nldebug("Preloading sample [%s]", sampleName.c_str());
			}
			else
			{
				_SoundDriver->loadWavFile(buffer, (*iter).c_str());
				_ByteSize += buffer->getSize();
			}
			_Samples[sampleName] = buffer ;

			// Warn the sound bank that the sample are available.
			CSoundBank::instance()->bufferLoaded(sampleName, buffer);
		}
		catch (ESoundDriver &e)
		{
			if (buffer != NULL) {
				delete buffer;
				buffer = NULL;
			}
			nlwarning("Problem with file '%s': %s", (*iter).c_str(), e.what());
		}
	}

	_Loaded = true;

	if (!async)
	{
		_LoadingDone = true;
		// compute the sample bank size.
		_LoadedSize += _ByteSize;
	}
	else
	{
		// fill the loading list.
		TSampleTable::iterator first(_Samples.begin()), last(_Samples.end());
		for (; first != last; ++first)
		{
			_LoadList.push_back(make_pair(first->second, first->first));
		}
		_SplitLoadDone = false;
		// send the first files
		for (uint i=0; i<ASYNC_LOADING_SPLIT && !_LoadList.empty(); ++i)
		{
			CAsyncFileManagerSound::getInstance().loadWavFile(_LoadList.front().first, _Path+"/"+_LoadList.front().second+".wav");
			_LoadList.pop_front();
		}
		// add a end loading event...
		CAsyncFileManagerSound::getInstance().signal(&_SplitLoadDone);
		// and register for update on the mixer
		CAudioMixerUser::instance()->registerUpdate(this);
	}
//_CrtCheckMemory();
}

void CSampleBank::onUpdate()
{
//_CrtCheckMemory();
	if (_SplitLoadDone)
	{
		nldebug("Some samples have been loaded");
		if (_LoadList.empty())
		{
			// all the samples are loaded, we can compute the bank size.
			TSampleTable::iterator	first(_Samples.begin()), last(_Samples.end());
			for (; first != last; ++first)
			{
				_ByteSize += first->second->getSize();
			}
		
			_LoadedSize += _ByteSize;

			// stop the update.
			CAudioMixerUser::instance()->unregisterUpdate(this);
			_LoadingDone = true;

			// Force an update in the background manager (can restar stoped sound).
			CAudioMixerUser::instance()->getBackgroundSoundManager()->updateBackgroundStatus();

			nldebug("Sample bank %s loaded.", _Name.c_str());
		}
		else
		{
			_SplitLoadDone = false;
			for (uint i=0; i<ASYNC_LOADING_SPLIT && !_LoadList.empty(); ++i)
			{
				CAsyncFileManagerSound::getInstance().loadWavFile(_LoadList.front().first, _Path+"/"+_LoadList.front().second+".wav");
				_LoadList.pop_front();
			}
			// add a end loading event...
			CAsyncFileManagerSound::getInstance().signal(&_SplitLoadDone);
		}
	}
//_CrtCheckMemory();
}

// ********************************************************

bool				CSampleBank::unload()
{
//_CrtCheckMemory();
	vector<IBuffer*> vec;
	TSampleTable::iterator it;

	if (!_Loaded)
	{
		nlwarning("Trying to unload an already unloaded bank : %s", _Path);
	}

	// need to wait end of load ?
	if (!_LoadingDone)
		return false;

	nldebug("Unloading sample bank %s", _Name.c_str());

	for (it = _Samples.begin(); it != _Samples.end(); ++it)
	{
		IBuffer *buffer = it->second;
		if (buffer)
		{
			const std::string & bufferName = buffer->getName();

			// Warn the mixer to stop any track playing this buffer.
			CAudioMixerUser::instance()->bufferUnloaded(buffer);
			// Warn the sound banks abount this buffer.
			CSoundBank::instance()->bufferUnloaded(bufferName);

			// delete
			it->second = NULL;
			delete buffer;
		}
	}
//_CrtCheckMemory();

	_Loaded = false;

	_LoadedSize -= _ByteSize;
	_ByteSize = 0;

	return true;
}

// ********************************************************

bool				CSampleBank::isLoaded()
{
	return _Loaded;
}

// ********************************************************

IBuffer*			CSampleBank::getSample(const std::string &name)
{
	{
/*		// dump the sample list.
		TSampleTable::iterator it (_Samples.begin()), last(_Samples.end());
		std::string s;

//		while (first != last)
		for (it = _Samples.begin(); it != _Samples.end(); ++it)
		{
			s += std::string(" [")+it->first+"] ";
			//first++;
		}

		nldebug("getSample(%s) : sample list = [%s]", name, s.c_str());
*/
	}

	// Find sound
	TSampleTable::iterator iter = _Samples.find(name);
	if ( iter == _Samples.end() )
	{
		return 0;
	}
	else
	{
		return (*iter).second;
	}
}

// ********************************************************

uint				CSampleBank::countSamples()
{
	return _Samples.size();
}

// ********************************************************

uint				CSampleBank::getSize()
{
	uint size = 0;

	TSampleTable::const_iterator iter;
	for (iter = _Samples.begin(); iter != _Samples.end(); iter++)
	{
		size +=	(*iter).second->getSize();
	}

	return size;
}

void				CSampleBank::releaseAll()
{
	nldebug( "SampleBanks: Releasing..." );

	while (!_Banks.empty())
	{
//_CrtCheckMemory();
		delete _Banks.begin()->second;
//_CrtCheckMemory();
	}
	nldebug( "SampleBanks: Released" );
}



} // namespace NLSOUND

