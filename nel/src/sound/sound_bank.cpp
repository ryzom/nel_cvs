/** \file sound_bank.cpp
 * CSoundBank: a set of sounds
 *
 * $Id: sound_bank.cpp,v 1.5 2002/07/16 13:16:37 lecroart Exp $
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

#include "sound_bank.h"
#include "sound.h"

#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


namespace NLSOUND {

set<CSoundBank*> CSoundBank::_Banks;


/** Return the name corresponding to a name. The sample is searched
 * in all the loaded sample banks.
 */
CSound*		CSoundBank::get(const char* name)
{
	CSound* sound;
	set<CSoundBank*>::iterator iter;

	for (iter = _Banks.begin(); iter != _Banks.end(); ++iter)
	{
		sound = (*iter)->getSound(name);
		if (sound != 0)
		{
			return sound;
		}
	}
	
	return 0;
}

/**
 *  Return the names of the sounds. The names of all the loaded sound banks are returned.
 */
void CSoundBank::getSoundNames( std::vector<const char *>& names )
{
	names.clear();

	set<CSoundBank*>::iterator iter;
	for (iter = _Banks.begin(); iter != _Banks.end(); ++iter)
	{
		(*iter)->getNames(names);
	}
}


/*
 * Destructor
 */
CSoundBank::~CSoundBank()
{
	unload();
}


/** Load all the sound samples.
 *
 * Can throw EPathNotFound or ESoundFileNotFound (check Exception)
 */
void				CSoundBank::load()
{
	vector<string> filenames;
	vector<string>::iterator iter;

	CPath::getPathContent(_Path, true, false, true, filenames);

	if (!filenames.empty())
	{
		UFormLoader* formLoader = UFormLoader::createLoader();

		for (iter = filenames.begin(); iter != filenames.end(); iter++)
		{
			// FIXME: should filter files based on file extension .sound

			if ((*iter).find(".sound") < (*iter).npos)
			{
				try
				{
					// load the form
					CSmartPtr<UForm> form = formLoader->loadForm((*iter).c_str());
					if (form)
					{
						// create a new sound and ask it to import the form
						NLSOUND::CSound* sound = new CSound();
						sound->importForm((*iter), form->getRootNode());
						_Sounds.insert(make_pair(sound->getName().c_str(), sound));
						//nldebug("Loaded %s.sound with sample %s.wav", sound->getName().c_str(), sound->getBuffername().c_str());

					} 
					else
					{
						nlwarning("Failed to load the form %s", (*iter).c_str());
					}
				}
				catch (exception& e2)
				{
					string what2 = e2.what();
					nlwarning(what2.c_str());
					throw;
				}
			}
		}

		UFormLoader::releaseLoader(formLoader);
	}

	_Banks.insert(this);
}


/*
 * Unload all the sound samples in this bank.
 */
void				CSoundBank::unload()
{
	vector<CSound*> vec;
	TSoundTable::iterator map_iter;

	// remove the bank from the list of known banks
	set<CSoundBank*>::const_iterator iter = _Banks.find(static_cast<CSoundBank*>(this));
	if (iter == _Banks.end())
	{
		nlwarning( "AM: Cannot remove sound bank: not found" );
	}
	else
	{
		_Banks.erase(iter);
	}


	for (map_iter = _Sounds.begin(); map_iter != _Sounds.end(); ++map_iter)
	{
		// We can't delete directly second because the map is based on second->getName()
		vec.push_back( (*map_iter).second );
	}
	
	_Sounds.clear();

	vector<CSound*>::iterator vec_iter;

	for (vec_iter = vec.begin(); vec_iter != vec.end(); ++vec_iter)
	{
		delete (*vec_iter);
	}
}

/*
 * Returns true if the samples in this bank have been loaded.
 */
bool				CSoundBank::isLoaded()
{
	return _Loaded;
}

/*
 * Return a sound sample corresponding to a name.
 */
CSound*			CSoundBank::getSound(const char* name)
{
	// Find sound
	TSoundTable::iterator iter = _Sounds.find(name);
	if ( iter == _Sounds.end() )
	{
		return 0;
	}
	else
	{
		return (*iter).second;
	}
}

/**
 *  Return the names of the sounds 
 */ 
void				CSoundBank::getNames( std::vector<const char *>& names )
{
	TSoundTable::const_iterator iter;
	for (iter = _Sounds.begin(); iter != _Sounds.end(); ++iter)
	{
		names.push_back((*iter).first);
		//nlwarning("getting sound %s", (*iter).first);
	}
}

/*
 * Return the number of buffers in this bank.
 */
uint				CSoundBank::countSounds()
{
	return _Sounds.size();
}




} // namespace NLSOUND

