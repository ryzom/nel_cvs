/** \file sample_bank.cpp
 * CSampleBank: a set of sound samples
 *
 * $Id: sample_bank.cpp,v 1.1 2002/06/04 10:02:52 hanappe Exp $
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


CSampleBank::CSampleBank(const std::string& path, ISoundDriver *sd) : _Loaded(false), _SoundDriver(sd), _Path(path) 
{
	_Name = CFile::getFilenameWithoutExtension(_Path);
}


CSampleBank::~CSampleBank()
{
	unload();
}


/** Load all the sound samples.
 *
 * Can throw EPathNotFound or ESoundFileNotFound (check Exception)
 */
void				CSampleBank::load()
{
	vector<string> filenames;
	vector<string>::iterator iter;

	CPath::getPathContent(_Path, true, false, true, filenames);

	for (iter = filenames.begin(); iter != filenames.end(); iter++)
	{
		try
		{
			IBuffer* buffer = _SoundDriver->createBuffer();
			_SoundDriver->loadWavFile(buffer, (*iter).c_str());
			string buffername = CFile::getFilenameWithoutExtension(*iter);
			_Buffers.insert(make_pair(buffername.c_str(), buffer));

			//nldebug("AM: SampleBank %s: loading sample %s", _Name.c_str(), buffername.c_str());

		}
		catch (ESoundDriver& e2)
		{
			string what2 = e2.what();
			nlwarning(what2.c_str());
			throw;
		}
	}
}


/*
 * Unload all the sound samples in this bank.
 */
void				CSampleBank::unload()
{
	vector<IBuffer*> vec;
	TSoundTable::iterator map_iter;

	for (map_iter = _Buffers.begin(); map_iter != _Buffers.end(); ++map_iter)
	{
		// We can't delete directly second because the map is based on second->getName()
		vec.push_back( (*map_iter).second );
	}
	
	_Buffers.clear();

	vector<IBuffer*>::iterator vec_iter;

	for (vec_iter = vec.begin(); vec_iter != vec.end(); ++vec_iter)
	{
		delete (*vec_iter);
	}
}

/*
 * Returns true if the samples in this bank have been loaded.
 */
bool				CSampleBank::isLoaded()
{
	return _Loaded;
}

/*
 * Return a sound sample corresponding to a name.
 */
IBuffer*			CSampleBank::getBuffer(const char* name)
{
	// Find sound
	TSoundTable::iterator iter = _Buffers.find(name);
	if ( iter == _Buffers.end() )
	{
		return 0;
	}
	else
	{
		return (*iter).second;
	}
}

/*
 * Return the number of buffers in this bank.
 */
uint				CSampleBank::countBuffers()
{
	return _Buffers.size();
}

/*
 * Return the size of this bank in bytes.
 */
uint				CSampleBank::getSize()
{
	uint size = 0;

	TSoundTable::const_iterator iter;
	for (iter = _Buffers.begin(); iter != _Buffers.end(); iter++)
	{
		size +=	(*iter).second->getSize();
	}

	return size;
}


} // namespace NLSOUND

