/** \file context_sound.cpp
 *
 * $Id: context_sound.cpp,v 1.2.2.1 2003/04/24 14:05:44 boucher Exp $
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

#include "context_sound.h"
#include "sound_bank.h"

using namespace std;
using namespace NLMISC;



namespace NLSOUND {


//CContextSound	test;


/// Constructor
CContextSound::CContextSound()
:	_ContextSounds(0)
{
}
/// Destructor
CContextSound::~CContextSound()
{
	if (_ContextSounds != 0)
		delete _ContextSounds;
}

void		CContextSound::serial(NLMISC::IStream &s)
{
	CSound::serial(s);
	s.serial(_PatternName);

	if (s.isReading())
	{
		if (_ContextSounds)
			delete _ContextSounds;
	}
}


/// Load the sound parameters from georges' form
void		CContextSound::importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot)
{
	NLGEORGES::UFormElm *psoundType;
	std::string dfnName;

	// some basic checking.
	formRoot.getNodeByName(&psoundType, ".SoundType");
	nlassert(psoundType != NULL);
	psoundType->getDfnName(dfnName);
	nlassert(dfnName == "context_sound.dfn");

	// Call the base class
	CSound::importForm(filename, formRoot);

	// Read the pattern name
	formRoot.getValueByName(_PatternName, ".SoundType.PatternName");
}

/// Return true if cone is meaningful
bool		CContextSound::isDetailed() const
{
	return false;
}

float		CContextSound::getMaxDistance() const
{
	if (_ContextSounds == 0)
	{
		const_cast<CContextSound*>(this)->init();
		if (_ContextSounds == 0)
		{
			// invalid state
			return 0;
		}
	}
	return _ContextSounds->getMaxDistance();;
}

/// Return the length of the sound in ms
uint32		CContextSound::getDuration() 
{
	std::vector<std::pair<std::string, CSound*> > sounds;
	getSubSoundList(sounds);

	uint32 duration = 0;

	std::vector<std::pair<std::string, CSound*> >::iterator first(sounds.begin()), last(sounds.end());
	for (; first != last; ++first)
	{
		duration = std::max(duration, first->second->getDuration());
	}
	return duration;
}

/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
void		CContextSound::getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const 
{
	if (_ContextSounds == 0)
	{
		const_cast<CContextSound*>(this)->init();
	}

	if (_ContextSounds)
	{
		_ContextSounds->getSoundList(subsounds);
	}
}

CSound	*CContextSound::getContextSound(CSoundContext &context)
{
	if (_ContextSounds == 0)
	{
		// need to init the sound container.
		init();

		if (_ContextSounds == 0)
			return 0;
	}

	if (_Random != 0)
	{
		// compute the random value.
		uint32	r;
		do
		{
			r = rand()%_Random;
		} while (r == context.PreviousRandom);

		context.PreviousRandom = r;

		return _ContextSounds->getSound(context, r);
	}
	else
		return _ContextSounds->getSound(context, 0);


}

void CContextSound::init()
{
	uint	nbJoker = 0;
	uint	contextArgIndex[SoundContextNbArgs];
	bool	useRandom = false;
	bool	parseArg = false;
	_BaseName = "";

	nldebug("Init the context sound %s", _PatternName.c_str());

	string::iterator	first(_PatternName.begin()), last(_PatternName.end());
	// 1st loop until the first joker
	for(; first != last && *first != '%'; ++first)
		_BaseName += *first;

	// 2nd loop, now we read the context arg index for each joker.
	std::string index;
	for (; first != last; ++first)
	{
		if (parseArg)
		{
			if (isdigit(int(*first)))
			{
				index += *first;
			}
			else if (!index.empty())
			{
				if (useRandom)
				{
					_Random = atoi(index.c_str());
				}
				else
				{
					nlassert(nbJoker < SoundContextNbArgs);
					contextArgIndex[nbJoker++] = atoi(index.c_str());
					parseArg = false;
					index = "";
				}
			}
			else if (*first == 'r')
			{
				nlassert(useRandom == false);
				useRandom = true;
			}
		}
		else if (*first == '%')
		{
			nlassert(useRandom == false);
			parseArg = true;
		}
	}

	// read the last char...
	if (!index.empty())
	{
		if (useRandom)
		{
			_Random = atoi(index.c_str());
		}
		else
		{
			nlassert(nbJoker < SoundContextNbArgs);
			contextArgIndex[nbJoker++] = atoi(index.c_str());
			parseArg = false;
		}
	}

	if (_ContextSounds != 0)
		delete _ContextSounds;

	// A little macro to make life easier (LM stand for Local Macro)
#define LM_CASE_CONTAINER_CREATOR(size)	case (size):\
											if (useRandom)\
												_ContextSounds = new CContextSoundContainer<(size), true>;\
											else\
												_ContextSounds = new CContextSoundContainer<(size), false>;\
											break;

	// ok, now instantiate the correct container
	switch (nbJoker)
	{
		LM_CASE_CONTAINER_CREATOR(0)
		LM_CASE_CONTAINER_CREATOR(1)
		LM_CASE_CONTAINER_CREATOR(2)
		LM_CASE_CONTAINER_CREATOR(3)
		LM_CASE_CONTAINER_CREATOR(4)
		LM_CASE_CONTAINER_CREATOR(5)
		LM_CASE_CONTAINER_CREATOR(6)
		LM_CASE_CONTAINER_CREATOR(7)
		LM_CASE_CONTAINER_CREATOR(8)
		LM_CASE_CONTAINER_CREATOR(9)
		LM_CASE_CONTAINER_CREATOR(10)
	default:
		nlwarning("Unsuported number of context argument !");
		return;
	}
	// cleanup macro
#undef LM_CASE_CONTAINER_CREATOR

	_ContextSounds->init(contextArgIndex);

	// ok, we have the container, now fill it with the sound
	{
		std::vector<NLMISC::TStringId> allSounds;
//		CSoundBank::getSoundNames(allSounds);
		CAudioMixerUser::instance()->getSoundNames(allSounds);

		std::vector<NLMISC::TStringId>::iterator first(allSounds.begin()), last(allSounds.end());
		for (; first != last; ++first)
		{
			const std::string &soundName = CStringMapper::unmap(*first);
			if (soundName.size() > _BaseName.size())
			{
				uint i;
				for (i=0; i<_BaseName.size(); ++i)
				{
					if (soundName[i] != _BaseName[i])
						break;
				}
				if (i == _BaseName.size())
				{
					// The base name is ok, check that the next char is a digit (avoid conflit if some 
					// sound have a longeur base name with same begining)
					if (soundName[i] >= '0' && soundName[i] <= '9')
						_ContextSounds->addSound(CAudioMixerUser::instance()->getSoundId(*first), _BaseName);
				}
			}
		}
	}
}



} // NLSOUND