/** \file sound_anim_player.cpp
 * A sound event marer on a sound track
 *
 * $Id: sound_anim_player.cpp,v 1.2 2002/06/20 08:37:14 hanappe Exp $
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
#include "nel/sound/sound_anim_player.h"
#include "nel/sound/sound_anim_marker.h"
#include "nel/sound/sound_animation.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_source.h"
#include "nel/misc/vector.h"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;

namespace NLSOUND {

// ********************************************************

CSoundAnimPlayer::CSoundAnimPlayer(CSoundAnimation* anim, float time, NLMISC::CVector *position, NLSOUND::UAudioMixer* mixer, TSoundAnimPlayId id) 
{
	nlassert(anim);
	nlassert(position);
	nlassert(mixer);

	_StartTime = time;
	_Mixer = mixer;
	_Animation = anim;
	_Playing = true;
	_Position = position;
	_Id = id;
	//_NextMarker = 0; 
	//_NumMarkers = _Animation->countMarkers();
}

// ********************************************************

void CSoundAnimPlayer::update(float lastTime, float curTime)
{
	float end = 0.0f;
	uint nmarkers = _Animation->countMarkers();

	if (curTime < _StartTime)
	{
		_Playing = false;
		return;
	}

	for (uint i = 0; i < nmarkers; i++) 
	{
		CSoundAnimMarker* marker = _Animation->getMarker(i);
		nlassert(marker);

		float markerTime = _StartTime + marker->getTime(); 
		end = max(markerTime, end);

		if ((lastTime <= markerTime) && (markerTime < curTime))
		{
			marker->getSounds(_Sounds);
			vector<const char*>::iterator iter;
			for (iter = _Sounds.begin(); iter != _Sounds.end(); iter++)
			{
				USource* source = _Mixer->createSource(*iter, true);
				source->setPos(*_Position);
				source->play();
			}
			_Sounds.clear();
		}
	}

	if (curTime > end)
	{
		_Playing = false;
	}
}

} // namespace NLSOUND


