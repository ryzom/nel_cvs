/** \file sound_anim_marker.cpp
 * A sound event marker on a sound track
 *
 * $Id: sound_anim_marker.cpp,v 1.1 2002/06/18 16:02:46 hanappe Exp $
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
#include "nel/sound/sound_anim_marker.h"
#include "nel/misc/common.h"
/*
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
*/

using namespace std;
using namespace NLSOUND;
using namespace NLMISC;

namespace NLSOUND {

// ********************************************************

CSoundAnimMarker::~CSoundAnimMarker()
{
}

// ********************************************************

void CSoundAnimMarker::addSound(string& soundName)
{
	pair<TMarkerSoundSet::iterator, bool> inserted;
	inserted = _Sounds.insert(soundName);
	if (inserted.second == false)
	{
		nlwarning("Duplicate sound (%s)", soundName.c_str());
	}
}

// ********************************************************

void CSoundAnimMarker::removeSound(string& soundName)
{
	TMarkerSoundSet::const_iterator iter = _Sounds.find(soundName);
    if (iter != _Sounds.end())
	{
		_Sounds.erase(iter);
	}
	else
	{
		nlwarning("No sound was removed (%s)", soundName.c_str());		
	}
}

// ********************************************************

void CSoundAnimMarker::getSounds(vector<const char*>& sounds)
{
	set<string>::iterator iter;

	for (iter = _Sounds.begin(); iter != _Sounds.end(); iter++)
	{
		sounds.push_back((*iter).c_str());
	}
}

// ********************************************************
/*
void CSoundAnimMarker::save(xmlNodePtr parent)
{
	set<string>::iterator iter;

	char s[64];
	smprintf(s, 64, "%f", _Time);

	xmlNodePtr markerNode = xmlNewChild ( parent, NULL, (const xmlChar*)"MARKER", NULL );
	xmlSetProp (markerNode, (const xmlChar*)"time", (const xmlChar*) s);

	for (iter = _Sounds.begin(); iter != _Sounds.end(); iter++)
	{
		xmlNodePtr soundNode = xmlNewChild ( markerNode, NULL, (const xmlChar*)"SOUND", NULL );
		xmlSetProp (soundNode, (const xmlChar*)"name", (const xmlChar*) (*iter).c_str());
	}
}
*/

// ********************************************************
/*
void CSoundAnimMarker::load(CIXml& input, xmlNodePtr node)
{
	const char *time = (const char*) xmlGetProp(node, (xmlChar*) "time");
	if (time == 0)
	{
		throw exception("Invalid sound animation marker");
	}

	setTime((float) atof(time));
	xmlFree ((void*)time);



	xmlNodePtr sound = input.getFirstChildNode(node, "SOUND");

	while (sound != 0)
	{
		char *name = (char*) xmlGetProp(node, (xmlChar*) "name");
		if (name == 0)
		{
			throw exception("Invalid sound animation marker");
		}

		addSound(string(name));

		xmlFree ((void*)name);

		sound = input.getNextChildNode(node, "SOUND");
	}
}


*/


} // namespace NLSOUND
