/** \file sound_process.h
 *
 * A sound process plays a sound pattern.
 *
 * $Id: sound_process.h,v 1.1 2002/06/28 20:38:54 hanappe Exp $
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

#ifndef NL_SOUND_PROCESS_H
#define NL_SOUND_PROCESS_H

#include "sound_pattern.h"

namespace NLSOUND
{


class CSoundPattern;
class UAudioMixer;
class USource;


typedef std::vector<USource*> TSourceVector;


/**
 * A sound process plays a sound pattern. The process can be triggered
 * explicitely by a client object. Alternatively, the process can be
 * spawned and trigger itself according to the pattern's time intervals.
 */

class CSoundProcess /* : public IPlayable */
{
public:

	CSoundProcess(UAudioMixer* mixer, CSoundPattern* pattern) : _Mixer(mixer), _Pattern(pattern);
	virtual ~CSoundProcess() {}

	virtual void					play();
	virtual void					trigger();
	virtual void					update(float time);

	static void						removeSource(USource *source, void *data)


protected:

	UAudioMixer*					_Mixer
	CSoundPattern*					_Pattern;
	CSoundPattern::PatternIterator	_PatternIterator;
	CSoundPattern::IntervalIterator	_IntervalIterator;
	TSourceVector					_Sources;
	uint							_NextTrigger;
};

} // namespace

#endif // NL_SOUND_PROCESS_H
