/** \file sound_process.cpp
 *
 * A sound process plays a sound pattern.
 *
 * $Id: sound_process.cpp,v 1.1 2002/06/28 20:38:54 hanappe Exp $
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


#include "sound_process.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_source.h"

using namespace std;
using namespace NLMISC;


namespace NLSOUND
{

// ********************************************************

void CSoundProcess::play()
{
 	_PatternIterator = _Pattern.beginSoundPattern();
	_IntervalIterator = _Pattern.beginIntervalPattern();
	_NextTrigger = 0;
}

// ********************************************************

void CSoundProcess::trigger()
{
	string& sound = *_PatternIterator;
	USource* source = _Mixer->createSource(sound.c_str(), true, CSoundProcess::removeSource, this);
	_Sources.push_back(source);
	_PatternIterator++;
}

// ********************************************************

void CSoundProcess::update(float time)
{
	float triggerTime = _NextTrigger / _Pattern->getTicksPerSecond();


}

// ********************************************************

void CSoundProcess::removeSource(USource *source, void *data)
{
	CSoundProcess* self = (CSoundProcess*) data;

	TSourceVector::iterator iter;

	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		if (*iter == source) 
		{
			_Sources.erase(iter);
			return;
		}
	}	
}


}