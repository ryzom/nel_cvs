/** \file sound.cpp
 * CSound: a generic sound and its static properties
 *
 * $Id: sound.cpp,v 1.28.2.1 2003/05/28 13:49:49 boucher Exp $
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

#include "sound.h"
#include "nel/misc/path.h"
#include "sound_bank.h"

#include "simple_sound.h"
#include "complex_sound.h"
#include "background_sound.h"
#include "context_sound.h"

using namespace std;
using namespace NLMISC;


namespace NLSOUND {

CSound *CSound::createSound(const std::string &filename, NLGEORGES::UFormElm& formRoot)
{
	NL_ALLOC_CONTEXT(NLSOUND_CSound);
	CSound *ret = NULL;
	string	soundType;

	NLGEORGES::UFormElm *psoundType;

	if (!formRoot.getNodeByName(&psoundType, ".SoundType"))
	{
		nlwarning("No SoundType in : %s", filename.c_str());
		return 0;
	}

	if (psoundType != NULL)
	{
		std::string dfnName;
		psoundType->getDfnName(dfnName);

		if (dfnName == "simple_sound.dfn")
		{
			ret = new CSimpleSound();
			ret->importForm(filename, formRoot);
		}
		else if (dfnName == "complex_sound.dfn")
		{
			ret = new CComplexSound();
			ret->importForm(filename, formRoot);
		}
		else if (dfnName == "background_sound.dfn")
		{
			ret = new CBackgroundSound();
			ret->importForm(filename, formRoot);
		}
		else if (dfnName == "context_sound.dfn")
		{
			ret = new CContextSound();
			ret->importForm(filename, formRoot);
		}
		else
		{
			nlassertex(false, ("SoundType unsuported : %s", dfnName.c_str()));
		}
			
	}

	return ret;
}



/*
 * Constructor
 */
CSound::CSound() : 
	_Gain(1.0f), 
	_Pitch(1.0f),
	_MaxDist(1000000.0f),
	_Priority(MidPri), 
	_Looping(false),
	_ConeInnerAngle(6.283185f), 
	_ConeOuterAngle(6.283185f), 
	_ConeOuterGain( 1.0f ),
	_UserVarControler(CStringMapper::emptyId())
{
}

CSound::~CSound()
{}


void	CSound::serial(NLMISC::IStream &s)
{
	s.serial(_Gain);
	s.serial(_Pitch);
	s.serialEnum(_Priority);
	s.serial(_ConeInnerAngle, _ConeOuterAngle, _ConeOuterGain);
	s.serial(_Direction);
	s.serial(_Looping);
	s.serial(_MaxDist);
	if (s.isReading())
	{
		std::string name;
		s.serial(name);
		_Name = CStringMapper::map(name);
	}
	else
	{
		std::string name = CStringMapper::unmap(_Name);
		s.serial(name);
	}
}


/**
 * 	Load the sound parameters from georges' form
 */
void				CSound::importForm(const std::string& filename, NLGEORGES::UFormElm& root)
{
	// Name
	_Name = CStringMapper::map(CFile::getFilenameWithoutExtension(filename));	

	// InternalConeAngle
	uint32 inner;
	root.getValueByName(inner, ".InternalConeAngle");
	if (inner > 360)
	{
		inner = 360;
	}
	_ConeInnerAngle = (float) (Pi * inner / 180.0f);  // convert to radians

	// ExternalConeAngle
	uint32 outer;
	root.getValueByName(outer, ".ExternalConeAngle");
	if (outer > 360)
	{
		outer = 360;
	}
	_ConeOuterAngle= (float) (Pi * outer / 180.0f);  // convert to radians

	// Loop
	root.getValueByName(_Looping, ".Loop");

	// Gain
	sint32 gain;
	root.getValueByName(gain, ".Gain");
	if (gain > 0)
	{
		gain = 0;
	}
	if (gain < -100)
	{
		gain = -100;
	}
	_Gain = (float) pow(10.0, gain / 20.0); // convert dB to linear gain

	// External gain
	root.getValueByName(gain, ".ExternalGain");
	if (gain > 0)
	{
		gain = 0;
	}
	if (gain < -100)
	{
		gain = -100;
	}
	_ConeOuterGain = (float) pow(10.0, gain / 20.0); // convert dB to linear gain

	// Direction
	float x, y, z;

	root.getValueByName(x, ".Direction.X");
	root.getValueByName(y, ".Direction.Y");
	root.getValueByName(z, ".Direction.Z");
	_Direction = CVector(x, y, z);


	// Pitch
	sint32 trans;
	root.getValueByName(trans, ".Transpose");
	_Pitch =  (float) pow(Sqrt12_2, trans); // convert semi-tones to playback speed

	// Priority
	uint32 prio = 0;
 	root.getValueByName(prio, ".Priority");
	switch (prio)
	{
	case 0:		
		_Priority = LowPri; 
		break;
	case 1:		
		_Priority = MidPri; 
		break;
	case 2:		
		_Priority = HighPri; 
		break;
	case 3:		
		_Priority = HighestPri;	
		break;
	default:	
		_Priority = MidPri;
	}
}



} // NLSOUND
