/** \file sample_bank.cpp
 * CSampleBank: a set of sound samples
 *
 * $Id: sample_bank.cpp,v 1.6 2002/08/21 09:42:29 lecroart Exp $
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

using namespace std;
using namespace NLMISC;


namespace NLSOUND {


set<CSampleBank*> CSampleBank::_Banks;


// ********************************************************

IBuffer*		CSampleBank::get(const char* name)
{
	IBuffer* buffer;
	set<CSampleBank*>::iterator iter;

	for (iter = _Banks.begin(); iter != _Banks.end(); ++iter)
	{
		buffer = (*iter)->getSample(name);
		if (buffer != 0)
		{
			return buffer;
		}
	}
	
	//nlwarning ("Try to get an unknown sample '%s'", name);
	return 0;
}


// ********************************************************

CSampleBank::CSampleBank(const std::string& path, ISoundDriver *sd) : _SoundDriver(sd), _Path(path), _Loaded(false)
{
	_Name = CFile::getFilenameWithoutExtension(_Path);
	_Banks.insert(this);
}


// ********************************************************

CSampleBank::~CSampleBank()
{
	// remove the bank from the list of known banks
	set<CSampleBank*>::const_iterator iter = _Banks.find(static_cast<CSampleBank*>(this));
	if (iter == _Banks.end())
	{
		nlwarning( "AM: Cannot remove sample bank: not found" );
	}
	else
	{
		_Banks.erase(iter);
	}

	unload();
}


// ********************************************************

void				CSampleBank::load()
{
	vector<string> filenames;
	vector<string>::iterator iter;

	CPath::getPathContent(_Path, true, false, true, filenames);

	for (iter = filenames.begin(); iter != filenames.end(); iter++)
	{
		IBuffer* buffer = NULL;
		try
		{
			buffer = _SoundDriver->createBuffer();
			nlassert(buffer);
			_SoundDriver->loadWavFile(buffer, (*iter).c_str());
			_Samples.insert(make_pair(buffer->getName().c_str(), buffer));

//			nldebug("AM: SampleBank %s: loading sample %s", _Name.c_str(), buffer->getName().c_str());

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
}


// ********************************************************

void				CSampleBank::unload()
{
	vector<IBuffer*> vec;
	TSampleTable::iterator map_iter;

	for (map_iter = _Samples.begin(); map_iter != _Samples.end(); ++map_iter)
	{
		// We can't delete directly second because the map is based on second->getName()
		vec.push_back( (*map_iter).second );
	}
	
	_Samples.clear();

	vector<IBuffer*>::iterator vec_iter;

	for (vec_iter = vec.begin(); vec_iter != vec.end(); ++vec_iter)
	{
		delete (*vec_iter);
	}
}

// ********************************************************

bool				CSampleBank::isLoaded()
{
	return _Loaded;
}

// ********************************************************

IBuffer*			CSampleBank::getSample(const char* name)
{
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


} // namespace NLSOUND

