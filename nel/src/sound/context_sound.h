/** \file context_sound.h
 *
 * $Id: context_sound.h,v 1.2.2.1 2003/04/24 14:05:44 boucher Exp $
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

#ifndef NL_CONTEXT_SOUND_H
#define NL_CONTEXT_SOUND_H

#include "sound.h"
#include <hash_map>
#include <nel/misc/fast_mem.h>

namespace NLSOUND {

class ISoundDriver;
class IBuffer;
class CSound;


template <uint NbJoker, bool UseRandom, uint Shift = 5>
struct CContextMatcher
{
	// speudo constante
	enum
	{
		// Size of array : special case for 0 joker because we can't declare array of 0 elements
		JOKER_ARRAY_SIZE = (NbJoker == 0 ? 1 : NbJoker)
	};

	CContextMatcher(uint32 *jokersValues, uint32 randomValue)
		: HashValue(0)
	{
		for (uint i=0; i<NbJoker; ++i)
		{
			JokersValues[i] = jokersValues[i];
			
			uint leftShift = (5*i)&0x1f;
			HashValue ^= JokersValues[i] << leftShift;
			HashValue ^= JokersValues[i] >> (32-leftShift);
		}
		if (UseRandom)
		{
			RandomValue = randomValue;
			uint leftShift = (5*i)&0x1f;
			HashValue ^= randomValue << leftShift;
			HashValue ^= randomValue >> (32-leftShift);
		}
		else
			RandomValue = 0;
	}

	bool operator ==(const CContextMatcher &other) const
	{
		if (HashValue != other.HashValue)
			return false;
		else if (UseRandom)
			return RandomValue == other.RandomValue && memcmp(JokersValues, other.JokersValues, sizeof(uint32)*NbJoker) == 0;
		else
			return memcmp(JokersValues, other.JokersValues, sizeof(uint32)*NbJoker) == 0;
	}

	size_t getHashValue() const
	{
		return size_t(HashValue);
	}

	uint32	HashValue;
	uint32	JokersValues[JOKER_ARRAY_SIZE];
	uint32	RandomValue;

	struct CHash : public std::unary_function<CContextMatcher, size_t>
	{
		size_t operator () (const CContextMatcher &patternMatcher) const
		{
			return patternMatcher.getHashValue();
		}
	};

};


class IContextSoundContainer
{
public:
	virtual				~IContextSoundContainer() {};
	virtual void		init(uint *contextArgsIndex) =0;
	virtual void		addSound(CSound *sound, const std::string &baseName) =0;
	virtual CSound		*getSound(const CSoundContext &context, uint32 randomValue) =0;
	virtual void		getSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const =0;
	virtual float		getMaxDistance() const =0;
};

template <uint NbJoker, bool UseRandom, uint Shift = 5>
class CContextSoundContainer : public IContextSoundContainer
{
	// speudo constante
	enum
	{
		// Size of array : special case for 0 joker because we can't declare array of 0 elements
		JOKER_ARRAY_SIZE = (NbJoker == 0 ? 1 : NbJoker)
	};

	typedef std::hash_map<CContextMatcher<NbJoker, UseRandom, Shift>, CSound *, CContextMatcher<NbJoker, UseRandom, Shift>::CHash>	THashContextSound;

	virtual void		init(uint *contextArgsIndex)
	{
		_MaxDist = 0;
		NLMISC::CFastMem::memcpy(_ContextArgsIndex, contextArgsIndex, sizeof(uint) * NbJoker);
	}

	virtual float		getMaxDistance() const
	{
		return _MaxDist;
	}

	virtual void		addSound(CSound *sound, const std::string &baseName)
	{
		const std::string &patternName = CStringMapper::unmap(sound->getName());
		nlassert(patternName.size() >= baseName.size());

		std::string arg;
		uint32		args[JOKER_ARRAY_SIZE];

		_MaxDist = std::max(sound->getMaxDistance(), _MaxDist);

		// extract the context values
		std::string::const_iterator	first(patternName.begin() + baseName.size()), last(patternName.end());
//		std::string::const_iterator	first2(baseName.begin()), last2(baseName.end());
		// 1st, skip the base name 
//		for (; first == first2; ++first, ++first2);

		// 2nd, read all the joker values
		for (uint i=0; i<NbJoker && first != last; ++first)
		{
			if (isdigit(int(*first)))
			{
				arg += *first;
			}
			else if (!arg.empty())
			{
				// end of the argument.
				args[i++] = atoi(arg.c_str());
				arg = "";
			}
		}
		// read the potential last arg.
		if (!arg.empty())
		{
			// end of the argument.
			args[i++] = atoi(arg.c_str());
			arg = "";
		}

		if (i != NbJoker)
			return;
		nlassert(i==NbJoker);

		int randomValue;
		if (UseRandom)
		{
			bool ok = false;
			// 3rd, read the random value (if any)
			for (uint i=0; first != last; ++first)
			{
				if (isdigit(int(*first)))
				{
					arg += *first;
				}
				else if (!arg.empty())
				{
					nlassert (ok == false);
					// end of the argument.
					randomValue = atoi(arg.c_str());
					arg = "";
					ok = true;
				}
			}
			// read the potential last arg.
			if (!arg.empty())
			{
				nlassert (ok == false);
				// end of the argument.
				randomValue = atoi(arg.c_str());
				arg = "";
				ok = true;
			}
			nlassert (ok == true);

		}
		else
		{
			randomValue = 0;
		}
			
		// ok, now create the key and store the sound.
		CContextMatcher<NbJoker, UseRandom, Shift>	cm(args, randomValue);

#ifdef _DEBUG
		char tmp[1024];
		char tmp2[1024];

		smprintf(tmp, 1024, "Adding sound : %s, args = ", patternName.c_str());

		for (uint k=0; k<NbJoker; ++k)
		{
			sprintf(tmp2, "%u", args[k]);
			strcat(tmp, tmp2);
		}

		nldebug(tmp);

#endif

		std::pair<THashContextSound::iterator, bool>	ret;
		ret = _ContextSounds.insert(std::make_pair(cm, sound));
		if (!ret.second)
		{
			THashContextSound::iterator it = _ContextSounds.find(cm);
			nlassert(it != _ContextSounds.end());

			nlwarning("Sound %s has the same context matcher as the sound %s", CStringMapper::unmap(sound->getName()).c_str(), CStringMapper::unmap(it->second->getName()).c_str());
		}
	}

	virtual CSound		*getSound(const CSoundContext &context, uint32 randomValue)
	{
		// create a key
		uint32		args[JOKER_ARRAY_SIZE];
		for (uint i=0; i<NbJoker; ++i)
			args[i] = context.Args[_ContextArgsIndex[i]];

		CContextMatcher<NbJoker, UseRandom, Shift>	cm(args, randomValue);

		THashContextSound::iterator it = _ContextSounds.find(cm);

		if (it != _ContextSounds.end())
			return it->second;
		else
			return 0;
	}

	void getSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const
	{
		THashContextSound::const_iterator first(_ContextSounds.begin()), last(_ContextSounds.end());
		for (; first != last; ++first)
		{
			subsounds.push_back(std::make_pair(CStringMapper::unmap(first->second->getName()), first->second));
		}
	}

private:
	uint32				_ContextArgsIndex[JOKER_ARRAY_SIZE];
	THashContextSound	_ContextSounds;
	float				_MaxDist;
};

class CContextSound : public CSound
{
public:
	/// Constructor
	CContextSound();
	/// Destructor
	~CContextSound();


	TSOUND_TYPE			getSoundType()					{ return CSound::SOUND_CONTEXT; };

	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// Return true if cone is meaningful
	virtual bool		isDetailed() const;
	/// Return the length of the sound in ms
	virtual uint32		getDuration();
	/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const;


	CSound				*getContextSound(CSoundContext &context);

	void				init();

	void				serial(NLMISC::IStream &s);

	float				getMaxDistance() const;



private:
	
	/// The context sound pattern name.
	std::string					_PatternName;
	/// The base name, that is the constante part of the name (before the first joker).
	std::string					_BaseName;

	/// The random lenght (0 mean no random)
	uint32						_Random;

	

	/// container for all the candidate sounds
	IContextSoundContainer		*_ContextSounds;
	
};

} // NLSOUND

#endif //NL_CONTEXT_SOUND_H