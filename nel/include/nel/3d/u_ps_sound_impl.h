/** \file ps_sound_impl.h
 * <File description>
 *
 * $Id: u_ps_sound_impl.h,v 1.1 2001/08/29 14:26:41 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_PS_SOUND_IMPL_H
#define NL_PS_SOUND_IMPL_H

#include <string>
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/rgba.h"
#include "nel/sound/u_audio_mixer.h"
#include "nel/3d/u_particle_system_sound.h"


// WARNING : this file is not intended to be directly included by the client. 
// It is just used to avoid a dependencie between NL3D and NLSOUND



namespace NL3D 
{


/// This class implement a sound instance (a sound source)
class CPSSoundInstanceImpl : public UPSSoundInstance
{
public:
	/// construct this object from a nel sound source
	/** The system will call this method to set the parameters of the sound	  
	  */
	CPSSoundInstanceImpl(NLSOUND::USource *source) : _Source(source)
	{
		
	}

	/// change this sound source paramerters
	virtual void setSoundParams(float gain
						   , const NLMISC::CVector &pos
						   , const NLMISC::CVector &velocity
						   , float pitch
						  )
	{
		nlassert(_Source);
		_Source->setPos(pos);
		_Source->setVelocity(velocity);
		_Source->setGain(gain);
		_Source->setPitch(pitch);
	}

	/// start to play the sound
	virtual void play(void)
	{
		nlassert(_Source);
		_Source->play();
	}


	virtual bool isPlaying(void) const
	{
		return _Source->isPlaying();
	}

	/// stop the sound
	virtual void stop(void)
	{
		nlassert(_Source);
		_Source->stop();
	}

	/// release the sound source
	virtual void release(void)
	{
		delete _Source;
		delete this;
	}

protected:
	NLSOUND::USource *_Source;
};







/**
 * This class implements PS sound server. It warps the calls to NEL sound. Everything is in a .h file to avoid dependency
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSSoundServImpl : public UPSSoundServer
{
public:	
	/// construct this sound server; You must init it then
	CPSSoundServImpl() : _AudioMixer(NULL)
	{
	}

	

	/// init this particle system sound server, using the given audio mixer
	void init(NLSOUND::UAudioMixer *audioMixer)
	{
		nlassert(audioMixer);
		_AudioMixer = audioMixer;
	}


	/// get the audio mixer associated with that server
	NLSOUND::UAudioMixer *getAudioMixer(void) { return _AudioMixer;}
	const NLSOUND::UAudioMixer *getAudioMixer(void) const { return _AudioMixer;}


	/// inherited from IPSSoundServer
	UPSSoundInstance *createSound(const std::string &soundName)
	{		
		nlassert(_AudioMixer);
		NLSOUND::USource *source = _AudioMixer->createSource(soundName.c_str());
		if (source)
		{						
			return new CPSSoundInstanceImpl(source);
		}
		else
		{
			return NULL;
		}
	}

protected:

	NLSOUND::UAudioMixer  *_AudioMixer;

};





} // NL3D


#endif // NL_PS_SOUND_IMPL_H

/* End of ps_sound_impl.h */
