/** \file sound_system.cpp
 * This initilize the sound system
 *
 * $Id: sound_system.cpp,v 1.10 2002/06/20 08:39:54 hanappe Exp $
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

#include "std_afx.h"
#include "resource.h"
#include "sound_system.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/sound/u_listener.h"
#include "nel/sound/sound_anim_manager.h"
#include "nel/sound/sound_animation.h"
#include "nel/sound/sound_anim_player.h"
#include "nel/misc/path.h"
#include "edit_ps_sound.h"

using namespace std;
using namespace NLMISC;
using namespace NLSOUND;


UAudioMixer				*CSoundSystem::_AudioMixer = NULL;
set<string>				CSoundSystem::_SoundBanksFileName;
set<string>				CSoundSystem::_SampleBanksFileName;
static CVector			SoundListenerPos = CVector::Null;
CSoundAnimManager		*CSoundSystem::_AnimManager = NULL;
TSoundAnimId			CSoundSystem::_CurrentAnimation = CSoundAnimation::NoId;
sint32					CSoundSystem::_CurrentPlayback = -1;
CVector					CSoundSystem::_Zero = CVector::Null;
string					CSoundSystem::_SamplePath;
sint					CSoundSystem::_AnimIndex = -1;

void CSoundSystem::setListenerMatrix(const NLMISC::CMatrix &m)
{
	if (_AudioMixer)
	{	
		UListener *l = _AudioMixer->getListener();
		SoundListenerPos = m.getPos();
		l->setPos(SoundListenerPos);		
		CVector j = m.getJ(), k = m.getK();
		l->setOrientation(j, k);
	}
}


void CSoundSystem::initSoundSystem ()
{		

	_AudioMixer = NLSOUND::UAudioMixer::createAudioMixer();
	try
	{
		_AudioMixer->init(AUTOBALANCE_DEFAULT_PERIOD);
		_AudioMixer->setSamplePath(_SamplePath);
		_AnimManager = new CSoundAnimManager(_AudioMixer);
	}
	catch (NLMISC::Exception &e)
	{
		string mess = string("Unable to init sound :") + e.what();
		nlwarning ("Init sound: %s", mess.c_str());
		_AudioMixer = NULL;
		_AnimManager = NULL;
		return;
	}
	setPSSoundSystem(_AudioMixer);

		
	for (set<string>::const_iterator it1 = _SampleBanksFileName.begin();
		 it1 != _SampleBanksFileName.end();
		 ++it1)
	{
		try
		{
			//_AudioMixer->loadSampleBank(NLMISC::CPath::lookup(*it).c_str());
			_AudioMixer->loadSampleBank((*it1).c_str());
		}
		catch (NLMISC::Exception &e)
		{
			string mess = "Unable to load sound file :" + *it1
						+ "\n" + e.what();
			nlwarning ("Init sound: %s", mess.c_str());
		}
	}					

	for (set<string>::const_iterator it2 = _SoundBanksFileName.begin();
		 it2 != _SoundBanksFileName.end();
		 ++it2)
	{
		try
		{
			//_AudioMixer->loadSoundBank(NLMISC::CPath::lookup(*it).c_str());
			_AudioMixer->loadSoundBank((*it2).c_str());
		}
		catch (NLMISC::Exception &e)
		{
			string mess = "Unable to load sound file :" + *it2
						+ "\n" + e.what();
			nlwarning ("Init sound: %s", mess.c_str());
		}
	}					


}


void CSoundSystem::poll()
{
	if (_AudioMixer)
	{
		_AudioMixer->update();
	}
}



void CSoundSystem::releaseSoundSystem(void)
{
	setPSSoundSystem(NULL);
	if (_AnimManager)
	{
		delete _AnimManager;
		_AnimManager = NULL;
	}
	if (_AudioMixer)
	{
		delete _AudioMixer;
		_AudioMixer = NULL;
	}
}


void CSoundSystem::play(const string &soundName)
{
	if (_AudioMixer)
	{
		NLSOUND::USource *src =  _AudioMixer->createSource(soundName.c_str(), true);
		if (src)
		{
			src->setLooping(false);
			src->setPos(SoundListenerPos);
			src->play();
		}
	}
}

void CSoundSystem::playAnimation(string& name, sint index, float start, float lastTime, float curTime)
{
	if (_AnimManager == NULL)
	{
		return;
	}

	TSoundAnimId id = _AnimManager->getAnimationFromName(name);

	if ((id != _CurrentAnimation) || (_AnimIndex != index))
	{
		if (_CurrentPlayback != -1)
		{
			_AnimManager->stopAnimation(_CurrentPlayback);
		}
		_CurrentAnimation = CSoundAnimation::NoId;
		_CurrentPlayback = -1;
		_AnimIndex = index;

		if (id != CSoundAnimation::NoId)
		{
			_CurrentAnimation = id;
			_CurrentPlayback = _AnimManager->playAnimation(id, start, &_Zero);
		}
	}
	else if (!_AnimManager->isPlaying(_CurrentPlayback))
	{
		_CurrentPlayback = _AnimManager->playAnimation(_CurrentAnimation, start, &_Zero);
	}

	_AnimManager->update(lastTime, curTime);
}
