/** \file sound_anim_manager.cpp
 * The sound animation manager handles all request to load, play, and
 * update sound animations.
 *
 * $Id: sound_anim_manager.cpp,v 1.2 2002/06/20 08:36:24 hanappe Exp $
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
#include "nel/sound/sound_anim_manager.h"
#include "nel/sound/sound_animation.h"
#include "nel/sound/sound_anim_player.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLSOUND;
using namespace NLMISC;

namespace NLSOUND {

CSoundAnimManager*	CSoundAnimManager::_Instance = 0;

// ********************************************************

CSoundAnimManager::CSoundAnimManager(NLSOUND::UAudioMixer* mixer) : _Mixer(mixer) 
{
	if (_Instance != 0)
	{
		throw Exception("Duplicate instanciation of CSoundAnimManager singleton");
	}

	_Instance = this;
	_PlayerId = 0;
}

// ********************************************************

CSoundAnimManager::~CSoundAnimManager()
{
	set<CSoundAnimPlayer*>::iterator iter;

	for (iter = _Players.begin(); iter != _Players.end(); iter++)
	{
		CSoundAnimPlayer* player = *iter; 
		delete player;
	}

	_Players.clear();
}

// ********************************************************

TSoundAnimId CSoundAnimManager::loadAnimation(std::string& name)
{
	nlassert(!name.empty());

	string filename = name;
	filename.append(".sound_anim");

	// throws exception if file not found
	filename = CPath::lookup(filename, true, false, true);

	TSoundAnimId id = createAnimation(name);
	if (id == CSoundAnimation::NoId)
	{
		throw exception("Duplicate sound animation");
	}

	CSoundAnimation* anim = _Animations[id];
	anim->setFilename(filename);
	anim->load();

	/*
	CIFile file;

	// Open the file
	if (!file.open(filename.c_str()))
	{
		throw exception("Can't open the file for reading");
	}

	anim->setFilename(filename);

	// Create the XML stream
	CIXml input;

	// Init
	if (input.init (file))
	{
		xmlNodePtr root = input.getRootNode ();

		anim->load (input, root);
	}

	// Close the file
	file.close ();
*/

	return id;
}

// ********************************************************

TSoundAnimId CSoundAnimManager::createAnimation(std::string& name)
{
	nlassert(!name.empty());

	// create and insert animations
	TSoundAnimId id = _Animations.size();
	CSoundAnimation* anim = new CSoundAnimation(name, id);
	_Animations.push_back(anim);

	// insert the name and id in the id table
	pair<TSoundAnimMap::iterator, bool> inserted;
	inserted =_IdMap.insert(make_pair(anim->getName().c_str(), id));
	if (!inserted.second) 
	{
		nlwarning("Duplicate sound animation \"%s\"", name.c_str());
		delete anim;
		return CSoundAnimation::NoId;
	}

	return id;
}

// ********************************************************

CSoundAnimation* CSoundAnimManager::findAnimation(std::string& name)
{
	TSoundAnimMap::iterator iter = _IdMap.find(name.c_str());
	return (iter == _IdMap.end())? 0 : _Animations[(*iter).second];	
}

// ********************************************************

TSoundAnimId CSoundAnimManager::getAnimationFromName(std::string& name)
{
	TSoundAnimMap::iterator iter = _IdMap.find(name.c_str());
	return (iter == _IdMap.end())? CSoundAnimation::NoId : (*iter).second;	
}

// ********************************************************

void CSoundAnimManager::saveAnimation(CSoundAnimation* anim, std::string& filename)
{
	nlassert(anim);
	nlassert(!filename.empty());

	anim->setFilename(filename);
	anim->save ();

	/*
	// File stream
	COFile file;

	// Open the file
	if (!file.open(filename.c_str()))
	{
		throw exception("Can't open the file for writing");
	}

	anim->setFilename(filename);

	// Create the XML stream
	COXml output;

	// Init
	if (output.init (&file, "1.0"))
	{
		xmlDocPtr xmlDoc = output.getDocument();

		anim->save (xmlDoc);

		// Flush the stream, write all the output file
		output.flush ();
	}

	// Close the file
	file.close ();
	*/
}

// ********************************************************

TSoundAnimPlayId CSoundAnimManager::playAnimation(TSoundAnimId id, float time, CVector* position)
{
	nlassert(id != CSoundAnimation::NoId);
	nlassert((uint32) id < _Animations.size());
	nlassert(position);

	CSoundAnimation* anim = _Animations[id];
	nlassert(anim);

	_PlayerId++;
	CSoundAnimPlayer* player = new CSoundAnimPlayer(anim, time, position, _Mixer, _PlayerId);
	nlassert(player);

	_Players.insert(player);

	return _PlayerId;
}

// ********************************************************

TSoundAnimPlayId CSoundAnimManager::playAnimation(string& name, float time, CVector* position)
{
	nlassert(position);

	TSoundAnimId id = getAnimationFromName(name);
	return (id == CSoundAnimation::NoId)? -1 : _PlayerId;
}

// ********************************************************

void CSoundAnimManager::stopAnimation(TSoundAnimPlayId playbackId)
{
	nlassert(playbackId >= 0);

	set<CSoundAnimPlayer*>::iterator iter;

	for (iter = _Players.begin(); iter != _Players.end(); )
	{
		CSoundAnimPlayer* player = *iter; 
		if (player->getId() == playbackId)
		{
			_Players.erase(iter);
			break;
		}
	}
}

// ********************************************************

bool CSoundAnimManager::isPlaying(TSoundAnimPlayId playbackId)
{
	nlassert(playbackId >= 0);

	set<CSoundAnimPlayer*>::iterator iter;

	for (iter = _Players.begin(); iter != _Players.end(); )
	{
		CSoundAnimPlayer* player = *iter; 
		if (player->getId() == playbackId)
		{
			return player->isPlaying();
		}
	}

	return false;
}

// ********************************************************

void CSoundAnimManager::update(float lastTime, float curTime)
{
	set<CSoundAnimPlayer*>::iterator iter;

	_Garbage.clear();

	for (iter = _Players.begin(); iter != _Players.end(); iter++)
	{
		CSoundAnimPlayer* player = *iter; 
		if (player->isPlaying())
		{
			player->update(lastTime, curTime);
		}
		else
		{
			_Garbage.push_back(player);
		}
	}

	vector<CSoundAnimPlayer*>::iterator iter2;

	for (iter2 = _Garbage.begin(); iter2 != _Garbage.end(); iter2++)
	{
		iter = _Players.find(*iter2);
		if (iter != _Players.end())
		{
			_Players.erase(iter);
		}
	}
}

} // namespace NLSOUND 


