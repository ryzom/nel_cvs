/** \file sound_system.cpp
 * This initilize the sound system
 *
 * $Id: sound_system.cpp,v 1.9 2002/02/26 17:30:23 corvazier Exp $
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
#include "nel/misc/path.h"
#include "edit_ps_sound.h"

NLSOUND::UAudioMixer *CSoundSystem::_AudioMixer = NULL;
std::set<std::string>		 CSoundSystem::_SoundBanksFileName;
static				 NLMISC::CVector SoundListenerPos = NLMISC::CVector::Null;


void CSoundSystem::setListenerMatrix(const NLMISC::CMatrix &m)
{
	if (_AudioMixer)
	{	
		NLSOUND::UListener *l = _AudioMixer->getListener();
		SoundListenerPos = m.getPos();
		l->setPos(SoundListenerPos);		
		NLMISC::CVector j = m.getJ(), k = m.getK();
		l->setOrientation(j, k);
	}
}


void CSoundSystem::initSoundSystem ()
{		
	
	_AudioMixer = NLSOUND::UAudioMixer::createAudioMixer();
	try
	{
		_AudioMixer->init(AUTOBALANCE_DEFAULT_PERIOD);
	}
	catch (NLMISC::Exception &e)
	{
		std::string mess = std::string("Unable to init sound :") + e.what();
		nlwarning ("Init sound: %s", mess.c_str());
		_AudioMixer = NULL;
		return;
	}
	setPSSoundSystem(_AudioMixer);

	if (_SoundBanksFileName.size())
	{
		
			for (std::set<std::string>::const_iterator it = _SoundBanksFileName.begin();
				 it != _SoundBanksFileName.end();
				 ++it)
			{
				try
				{
					_AudioMixer->loadSoundBuffers(NLMISC::CPath::lookup(*it).c_str());
				}
				catch (NLMISC::Exception &e)
				{
					std::string mess = "Unable to load sound file :" + *it
								+ "\n" + e.what();
					nlwarning ("Init sound: %s", mess.c_str());
				}
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
	delete _AudioMixer;
	_AudioMixer = NULL;
}


void CSoundSystem::play(const std::string &soundName)
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

