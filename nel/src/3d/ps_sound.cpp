/** \file ps_sound.cpp
 * <File description>
 *
 * $Id: ps_sound.cpp,v 1.1 2001/08/07 14:20:52 vizerie Exp $
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

#include "3d/ps_sound.h"
#include "3d/particle_system.h"
#include "3d/ps_sound_interface.h"
#include "3d/ps_attrib_maker.h"


namespace NL3D 
{


// we batch computation of volumes and frequencies. Here is the buffer size:
static const uint SoundBufSize = 1024;


CPSSound::CPSSound() : _VolumeScheme(NULL), _FrequencyScheme(NULL)
					   ,_Volume(1.f)	  , _Frequency(1000.f)
{
	_Name = std::string("sound");
}

CPSSound::~CPSSound()
{
	delete _VolumeScheme;	
	delete _FrequencyScheme;	
}

uint32			CPSSound::getType(void) const
{ 
	return PSSound; 
}

void			CPSSound::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	if (pass != PSMotion) return;
	nlassert(_Owner);
	const uint32 size = _Owner->getSize();
	if (!size) return;
	uint32 toProcess, leftToDo = size;

	float   volumes[SoundBufSize];
	float   frequencies[SoundBufSize];

	uint	volumePtInc    = _VolumeScheme ? 1 : 0;
	uint	frequencyPtInc = _FrequencyScheme ? 1 : 0;
	float   *currVol, *currFrequency;
	

	CPSAttrib<IPSSoundInstance *>::iterator it = _Sounds.begin()
												, endIt;
	CPSAttrib<NLMISC::CVector>::const_iterator posIt = _Owner->getPos().begin();
	CPSAttrib<NLMISC::CVector>::const_iterator speedIt = _Owner->getSpeed().begin();

	do
	{
		toProcess = leftToDo > SoundBufSize ? SoundBufSize : leftToDo;
		// compute volume		
		currVol = _VolumeScheme ? (float *) _VolumeScheme->make(getOwner(), size - leftToDo, volumes, sizeof(float), toProcess, true)
								: &_Volume;
		// compute frequency
		currFrequency = _FrequencyScheme ? (float *) _FrequencyScheme->make(getOwner(), size - leftToDo, frequencies, sizeof(float), toProcess, true)
								: &_Frequency;

		endIt = it + toProcess;
		do
		{
			if (*it) // was this sound instanciated?
			{
				(*it)->setSoundParams(*currVol
									  , *posIt
									  , *speedIt
									  , *currFrequency);						  
			}
			currVol += volumePtInc;
			currFrequency += frequencyPtInc;
			++posIt;
			++speedIt;
			++it;
		}
		while (it != endIt);
		
		leftToDo -= toProcess;
	}
	while (leftToDo);

}

void	CPSSound::setVolume(float volume)
{
	delete _VolumeScheme;
	_VolumeScheme = NULL;
	_Volume = volume;	
}


void	CPSSound::setVolumeScheme(CPSAttribMaker<float> *volume)
{
	delete _VolumeScheme;
	_VolumeScheme = volume;	
}




void	CPSSound::setFrequency(float frequency)
{
	delete _FrequencyScheme;
	_FrequencyScheme = NULL;
	_Frequency = frequency;
}

void	CPSSound::setFrequencyScheme(CPSAttribMaker<float> *frequency)
{
	delete _FrequencyScheme;	
	_FrequencyScheme = frequency;
}

	
	

void			CPSSound::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSLocatedBindable::serial(f);
	sint ver = f.serialVersion(1);
	f.serial(_SoundName);
	sint32 nbSounds;
	bool hasScheme;
	if (f.isReading())
	{		
		f.serial(nbSounds); // we are very unlikely to save a system with sounds being played in it,
							// but we need to keep datas coherency.
		_Sounds.resizeNFill(nbSounds);
		for (sint k = 0; k < nbSounds; ++k)
		{
			/// we could also recreate an interface...
			_Sounds[k] = NULL;
		}		
	}
	else
	{
		nbSounds = _Sounds.getSize();
		f.serial(nbSounds);		
	}


	if (f.isReading())
	{
		delete _VolumeScheme;
		_VolumeScheme = NULL;
		delete _FrequencyScheme;
		_FrequencyScheme = NULL;
	}
	// save volume infos
	hasScheme = _VolumeScheme != NULL;
	f.serial(hasScheme);
	if (hasScheme)
	{
		f.serialPolyPtr(_VolumeScheme);
	}
	else
	{
		f.serial(_Volume);
	}
	// save frequency infos
	hasScheme = _FrequencyScheme != NULL;
	f.serial(hasScheme);
	if (hasScheme)
	{
		f.serialPolyPtr(_FrequencyScheme);
	}
	else
	{
		f.serial(_Frequency);
	}			
}
	

void			CPSSound::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	if (_VolumeScheme && _VolumeScheme->hasMemory()) _FrequencyScheme->newElement(emitterLocated, emitterIndex);
	if (_FrequencyScheme && _FrequencyScheme->hasMemory()) _FrequencyScheme->newElement(emitterLocated, emitterIndex);
	// if there's a sound server, we generate a new sound instance
	if (CParticleSystem::getSoundServer())
	{
		_Sounds.insert(CParticleSystem::getSoundServer()->createSound(_SoundName));
	}
	else
	{
		_Sounds.insert(NULL);
	}
}

void			CPSSound::deleteElement(uint32 index)
{
	if (_VolumeScheme && _VolumeScheme->hasMemory()) _FrequencyScheme->deleteElement(index);
	if (_FrequencyScheme && _FrequencyScheme->hasMemory()) _FrequencyScheme->deleteElement(index);
	_Sounds.remove(index);
}

void			CPSSound::resize(uint32 size)
{
	if (_VolumeScheme && _VolumeScheme->hasMemory()) _VolumeScheme->resize(size, getOwner()->getSize());
	if (_FrequencyScheme && _FrequencyScheme->hasMemory()) _FrequencyScheme->resize(size, getOwner()->getSize());
	_Sounds.resize(size);
}

} // NL3D
