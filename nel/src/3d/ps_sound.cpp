/** \file ps_sound.cpp
 * <File description>
 *
 * $Id: ps_sound.cpp,v 1.3 2001/08/27 10:46:14 vizerie Exp $
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


// we batch computation of Gains and frequencies. Here is the buffer size:
static const uint SoundBufSize = 1024;


CPSSound::CPSSound() : _GainScheme(NULL), _PitchScheme(NULL)
					   ,_Gain(1.f)	  , _Pitch(1.f)
{
	_Name = std::string("sound");
}

CPSSound::~CPSSound()
{
	delete _GainScheme;	
	delete _PitchScheme;	
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

	float   Gains[SoundBufSize];
	float   frequencies[SoundBufSize];

	uint	GainPtInc    = _GainScheme ? 1 : 0;
	uint	frequencyPtInc = _PitchScheme ? 1 : 0;
	float   *currVol, *currFrequency;
	

	CPSAttrib<IPSSoundInstance *>::iterator it = _Sounds.begin()
												, endIt;
	CPSAttrib<NLMISC::CVector>::const_iterator posIt = _Owner->getPos().begin();
	CPSAttrib<NLMISC::CVector>::const_iterator speedIt = _Owner->getSpeed().begin();

	do
	{
		toProcess = leftToDo > SoundBufSize ? SoundBufSize : leftToDo;
		// compute Gain		
		currVol = _GainScheme ? (float *) _GainScheme->make(getOwner(), size - leftToDo, Gains, sizeof(float), toProcess, true)
								: &_Gain;
		// compute frequency
		currFrequency = _PitchScheme ? (float *) _PitchScheme->make(getOwner(), size - leftToDo, frequencies, sizeof(float), toProcess, true)
								: &_Pitch;

		endIt = it + toProcess;

		if (!_Owner->isInSystemBasis())
		{
			do
			{
				if (*it) // was this sound instanciated?
				{
					(*it)->setSoundParams(*currVol
										  , *posIt
										  , *speedIt
										  , *currFrequency);						  
				}
				currVol += GainPtInc;
				currFrequency += frequencyPtInc;
				++posIt;
				++speedIt;
				++it;
			}
			while (it != endIt);
		}
		else
		{
			const NLMISC::CMatrix m = _Owner->getOwner()->getSysMat();
			do
			{
				if (*it) // was this sound instanciated?
				{
					(*it)->setSoundParams(*currVol
										  , m * *posIt
										  , *speedIt
										  , *currFrequency);						  
				}
				currVol += GainPtInc;
				currFrequency += frequencyPtInc;
				++posIt;
				++speedIt;
				++it;
			}
			while (it != endIt);
		}

		
		leftToDo -= toProcess;
	}
	while (leftToDo);

}

void	CPSSound::setGain(float Gain)
{
	delete _GainScheme;
	_GainScheme = NULL;
	_Gain = Gain;	
}


void	CPSSound::setGainScheme(CPSAttribMaker<float> *Gain)
{
	delete _GainScheme;
	_GainScheme = Gain;	
}




void	CPSSound::setPitch(float pitch)
{
	delete _PitchScheme;
	_PitchScheme = NULL;
	_Pitch = pitch;
}

void	CPSSound::setPitchScheme(CPSAttribMaker<float> *pitch)
{
	delete _PitchScheme;	
	_PitchScheme = pitch;
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
		delete _GainScheme;
		_GainScheme = NULL;
		delete _PitchScheme;
		_PitchScheme = NULL;
	}
	// save Gain infos
	hasScheme = _GainScheme != NULL;
	f.serial(hasScheme);
	if (hasScheme)
	{
		f.serialPolyPtr(_GainScheme);
	}
	else
	{
		f.serial(_Gain);
	}
	// save frequency infos
	hasScheme = _PitchScheme != NULL;
	f.serial(hasScheme);
	if (hasScheme)
	{
		f.serialPolyPtr(_PitchScheme);
	}
	else
	{
		f.serial(_Pitch);
	}			
}
	

void			CPSSound::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	if (_GainScheme && _GainScheme->hasMemory()) _PitchScheme->newElement(emitterLocated, emitterIndex);
	if (_PitchScheme && _PitchScheme->hasMemory()) _PitchScheme->newElement(emitterLocated, emitterIndex);
	// if there's a sound server, we generate a new sound instance
	if (CParticleSystem::getSoundServer())
	{
		uint32 index = _Sounds.insert(CParticleSystem::getSoundServer()->createSound(_SoundName));
		/// set position and activate the sound
	
		if (_Sounds[index])
		{			
			const NLMISC::CMatrix &mat = _Owner->isInSystemBasis() ? _Owner->getOwner()->getSysMat() : NLMISC::CMatrix::Identity;
			_Sounds[index]->setSoundParams(0, mat * _Owner->getPos()[index], _Owner->getSpeed()[index], 1);
			_Sounds[index]->play();
		}
	
	}
	else
	{
		_Sounds.insert(NULL);
	}
}

void			CPSSound::deleteElement(uint32 index)
{
	if (_GainScheme && _GainScheme->hasMemory()) _PitchScheme->deleteElement(index);
	if (_PitchScheme && _PitchScheme->hasMemory()) _PitchScheme->deleteElement(index);
	if (_Sounds[index])
	{
		_Sounds[index]->release();
	}
	_Sounds.remove(index);
}

void			CPSSound::resize(uint32 size)
{
	if (_GainScheme && _GainScheme->hasMemory()) _GainScheme->resize(size, getOwner()->getSize());
	if (_PitchScheme && _PitchScheme->hasMemory()) _PitchScheme->resize(size, getOwner()->getSize());
	_Sounds.resize(size);
}

} // NL3D
