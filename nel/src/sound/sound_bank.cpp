/** \file sound_bank.cpp
 * CSoundBank: a set of sounds
 *
 * $Id: sound_bank.cpp,v 1.6 2002/11/04 15:40:44 boucher Exp $
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
#include "simple_sound.h"

#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form.h"
#include "nel/misc/path.h"
#include "driver/buffer.h"

using namespace std;
using namespace NLMISC;
using namespace NLGEORGES;


namespace NLSOUND {

CSoundBank::TSoundBankContainer		CSoundBank::_Banks;
//map<std::string, std::vector<CSoundBank::TBufferAssoc> >	CSoundBank::_BufferAssoc;
CSoundBank::TBufferAssocContainer	CSoundBank::_BufferAssoc;


/** Return the name corresponding to a name. The sample is searched
 * in all the loaded sample banks.
 */
CSound*		CSoundBank::get(const std::string &name)
{
	CSound* sound;
	TSoundBankContainer::iterator iter;

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
void CSoundBank::getSoundNames( std::vector<std::string>& names )
{
	names.clear();

	TSoundBankContainer::iterator iter;
	for (iter = _Banks.begin(); iter != _Banks.end(); ++iter)
	{
		(*iter)->getNames(names);
	}
}

void CSoundBank::bufferUnloaded(const std::string bufferName)
{
//	std::map<std::string, std::vector<TBufferAssoc> >::iterator it(_BufferAssoc.find(bufferName));
	TBufferAssocContainer::iterator it(_BufferAssoc.find(bufferName));

	if (it != _BufferAssoc.end())
	{
		// ok, found some sound associated with this buffer.
		// update all sounds.

/*		while(!it->second.empty())
		{
			CSimpleSound *ss = const_cast<CSimpleSound*>(*(it->second.begin()));
			bool stop = it->second.size() == 1;
			nldebug("bufferUnloaded : remove buffer %s from sound %s", bufferName.c_str(), ss->getName().c_str());
			ss->setBuffer(NULL);

			if (stop)
				break;
		}
*/
		TSimpleSoundContainer::iterator first(it->second.begin()), last(it->second.end());
		for (; first != last; ++first)
		{
			// remove the associated buffer.
			CSimpleSound *ss = const_cast<CSimpleSound*>(*(first));
			ss->setBuffer(NULL);
		}
	}
}

void CSoundBank::bufferLoaded(const std::string bufferName, IBuffer *buffer)
{
//	std::map<std::string, std::vector<TBufferAssoc> >::iterator it(_BufferAssoc.find(buffer->getName()));
	TBufferAssocContainer::iterator it(_BufferAssoc.find(buffer->getName()));

	if (it != _BufferAssoc.end())
	{
		// ok, found some sound associated with this buffer.
		// update all sounds.
		TSimpleSoundContainer::iterator first(it->second.begin()), last(it->second.end());
		for (; first != last; ++first)
		{
			CSimpleSound *ss = const_cast<CSimpleSound*>(*(it->second.begin()));
			// restore the associated buffer.
			ss->setBuffer(buffer);
		}
	}

}

void CSoundBank::registerBufferAssoc(CSimpleSound *sound, IBuffer *buffer) 
{
	if (buffer != NULL)
	{
		const std::string &bufferName = buffer->getName();
//		nldebug("Register sound %s with buffer %s", sound->getName().c_str(), buffer->getName().c_str());
//		_BufferAssoc[bufferName].push_back(TBufferAssoc(NULL, sound));
		_BufferAssoc[bufferName].insert(sound);
	}
}

void CSoundBank::unregisterBufferAssoc(CSimpleSound *sound, IBuffer * buffer) 
{
	if (buffer != NULL)
	{
		const std::string &bufferName = buffer->getName();
//		std::map<std::string, std::vector<TBufferAssoc> >::iterator it(_BufferAssoc.find(bufferName));
		TBufferAssocContainer::iterator it(_BufferAssoc.find(bufferName));

		if (it != _BufferAssoc.end())
		{
			// search the sound in the vector.
//			std::vector<TBufferAssoc>::iterator first(it->second.begin()), last(it->second.end());

			TSimpleSoundContainer::iterator it2(it->second.find(sound));


			nlassert(it2 != it->second.end())
			it->second.erase(it2);

			if (it->second.empty())
			{
				// last sound refenrecing this buffer
				_BufferAssoc.erase(it);
			}

/*
#if defined(DEBUG)
			bool found = false;
#endif
			for (; first != last; ++first)
			{
				if (first->_Sound == sound)
				{
					nldebug("Unregister sound %s with buffer %s", sound->getName().c_str(), buffer->getName().c_str());
					it->second.erase(first);
#if defined(DEBUG)
					found = true;
#endif
					break;
				}
			}
#if defined(DEBUG)
			nlassert(found);
#endif
			// no more reference on this buffer ?
			if (it->second.empty())
				_BufferAssoc.erase(it);
*/		}
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
//						NLSOUND::CSound* sound = new CSound();
//						sound->importForm((*iter), form->getRootNode());
						CSound* sound = CSound::createSound((*iter), form->getRootNode());
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
	TSoundBankContainer::const_iterator iter = _Banks.find(static_cast<CSoundBank*>(this));
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
		CSound *sound = *vec_iter;
		delete sound;
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
CSound*			CSoundBank::getSound(const std::string &name)
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
void				CSoundBank::getNames( std::vector<std::string>& names )
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

void				CSoundBank::releaseAll()
{
	while (!_Banks.empty())
	{
		delete *(_Banks.begin());
	}
}


} // namespace NLSOUND

