/** \file complex_sound.h
 *
 * A sound pattern represents a sequence of sounds. The sounds can
 * be selected randomly from a set, or have a composed sequence.
 * The intervals can be determined or selected randomly. 
 *
 * $Id: complex_sound.h,v 1.1 2002/11/04 15:40:44 boucher Exp $
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

#ifndef NL_COMPLEX_SOUND_H
#define NL_COMPLEX_SOUND_H

#include "nel/misc/types_nl.h"
#include "sound.h"
#include <string>

namespace NLSOUND
{

class ISoundController;


typedef std::basic_string<uint16>	uint16_string;


class CComplexSound : public CSound
{
public:

	enum TPATTERN_MODE
	{
		MODE_UNDEFINED,
		MODE_CHAINED,
		MODE_ALL_IN_ONE,
		MODE_SPARSE
	};

	bool							isDetailed() const;
	uint32							getDuration(std::string *buffername = NULL);


	TPATTERN_MODE					getPatternMode()							{ return _PatternMode;}
	void							setPatternMode(TPATTERN_MODE patternMode)	{ _PatternMode = patternMode;}

	const std::vector<uint>			&getSoundSeq() const						{ return _SoundSeq;}
	const std::vector<uint>			&getDelaySeq() const						{ return _DelaySeq;}
	const std::string				&getSound(uint index) const					{ return _Sounds[index%_Sounds.size()];}
	const std::vector<std::string>	&getSounds() const							{ return _Sounds;}

	uint32							getFadeLenght() const						{ return _XFadeLenght;} 

/*	class Iterator
	{
	public:
		uint16 _Max;
		uint16 _Value;
		CComplexSound* _Pattern;

		Iterator(CComplexSound* pattern, uint16 max) : _Pattern(pattern), _Value(0), _Max(max) {}

		Iterator& operator++() 
		{
			_Value++;
			if (_Value >= _Max) 
			{
				_Value -= _Max;
			}

			return *this;
		}
	};

	class PatternIterator : public Iterator
	{
	public:
		PatternIterator() : Iterator(0, 0) {}
		PatternIterator(CComplexSound* pattern, uint16 max) : Iterator(pattern, max) {}

		std::string& operator*() {
			return _Pattern->getSound(this);
		}
	};

	class IntervalIterator : public Iterator
	{
	public:
		IntervalIterator() : Iterator(0, 0) {}
		IntervalIterator(CComplexSound* pattern, uint16 max) : Iterator(pattern, max) {}

		uint32 operator*() {
			return _Pattern->getInterval(this);
		}
	};
*/
	/** Constructor */
	CComplexSound();

	/** Destructor */
	virtual ~CComplexSound();

	/// Load the sound parameters from georges' form
	virtual void					importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);



	/// \name Sounds list management
	//@{

	/** \def addSound(name)
	 *  Add a sound to the list of sounds used in this pattern 
	 */

	/** \def removeSound(name)
	 *  Remove a sound from the list of sounds used in this pattern
	 */

	/** \def getSounds(sounds)
	 *  Get the list of all sounds in this pattern
	 */

/*	virtual void				addSound(std::string& name)					{ _Sounds.push_back(name); }
	virtual void				removeSound(std::string& name);
	virtual void				getSounds(std::vector<std::string>& sounds);
*/	//@}


	/// \name Iterating through the sound pattern 
	//@{

	/** \def beginSoundPattern()
	 *  Get the beginning of the pattern. The pattern has infinite length. The code to run 
	 *  through the list of elements ressembles the iteration in STL classes:
	 *
	 *\code
 
	 	PatternIterator iterator = pattern.beginSoundPattern();
	 
	 	while (true)
	 	{
	 		string& sound = *iterator;
	 		iterator++;
	 	}

	 *\endcode
	 */

	/** \def getSound(iterator)
	 *  Returns the sound corresponding to the iterator value in the sound pattern. 
	 */

//	virtual PatternIterator		beginSoundPattern()							{ return PatternIterator(this, _SoundPattern.size()); }
//	virtual std::string&		getSound(PatternIterator* iterator);
	//@}


	/// \name Iterating through the sound pattern 
	//@{

	/** \def beginIntervalPattern()
	 *  Get the beginning of the interval pattern. The pattern has infinite length. The code 
	 *  to run through the list of elements ressembles the iteration in STL classes. See
	 *  code example in beginSoundPattern().
  	 * \see beginSoundPattern
	 */

	/** \def getInterval(iterator)
	 *  Returns the interval corresponding to the iterator value in the sound pattern. 
	 */

//	virtual IntervalIterator	beginIntervalPattern()						{ return IntervalIterator(this, _Intervals.size()); }
//	virtual uint16				getInterval(IntervalIterator* iterator);
	//@}
	
	/// \name Sound pattern editing and generation
	//@{
	/** \def setSoundPattern(list)
	 *  Set the pattern of the intervals. The list should be a string of numbers
	 *  separated by a comma. For example, "100,120,100,5,10,50"
	 */

	/** \def getIntervals(list)
	 *  Returns the pattern of the intervals as a string. The returned list has
	 *  the same format as expected by the setIntervals function.
	 *  \see setIntervals
	 */

/*	virtual void				setSoundPattern(const std::string& list);
	virtual void				getSoundPattern(std::string& list);
	virtual void				generateRandomPattern(uint length);
	virtual void				generateRandomMin1Pattern(uint length);
*/	//@}

	/// \name Interval pattern editing and generation
	//@{

	/** \def setIntervals(list)
	 *  Set the pattern of the intervals. The list should be a string of numbers
	 *  separated by a comma. For example, "100,120,100,5,10,50"
	 */

	/** \def getIntervals(list)
	 *  Returns the pattern of the intervals as a string. The returned list has
	 *  the same format as expected by the setIntervals function.
	 *  \see setIntervals
	 */

/*	virtual void				setIntervals(const std::string& list);
	virtual void				getIntervals(std::string& list);
	virtual void				generateRandomIntervals(uint length, uint16 min, uint16 max);
	virtual void				generateRandomMin1Intervals(uint length, uint16 min, uint16 max);
*/	//@}

	/// \name Tempo
	//@{
	virtual float					getTicksPerSecond()							{ return _TicksPerSeconds; }
	virtual void					setTicksPerSecond(float ticks)				{ _TicksPerSeconds = ticks; }
	//@}

	/// \name Spawning
	//@{
//	virtual void				setSpawn(bool v)							{ _Spawn = v; }
//	virtual bool				getSpawn()									{ return _Spawn; }
	//@}

	/// \name Continuous controllers
	//@{
/*	virtual void				setVolumeEnvelope(ISoundController* env)	{ _VolumeEnvelope = env; }
	virtual ISoundController*	getVolumeEnvelope()							{ return _VolumeEnvelope; }
	virtual void				setFreqModulation(ISoundController* mod)	{ _FreqModulation = mod; }
	virtual ISoundController*	getFreqModulation()							{ return _FreqModulation; }
*/	//@}

	
	TSOUND_TYPE						getSoundType() {return SOUND_COMPLEX;};

	void							getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const;
	bool							doFadeIn()								{ return _DoFadeIn; }
	bool							doFadeOut()								{ return _DoFadeOut; }

private:

	void							parseSequence(const std::string &str, std::vector<uint> &seq, uint scale = 1);
	virtual float					getMaxDistance() const;


/*	virtual void				parsePattern(const std::string& list, uint16_string& pattern);
	virtual void				concatenatePattern(std::string& list, const uint16_string& pattern);
	virtual void				generateRandomPattern(uint16_string& pattern, uint length, uint16 min, uint16 max);
	virtual void				generateRandomMin1Pattern(uint16_string& pattern, uint length, uint16 min, uint16 max);
	virtual void				expandString(const std::string& s, std::string& buffer);
*/	

	TPATTERN_MODE				_PatternMode;
	std::vector<std::string>	_Sounds;
	float						_TicksPerSeconds;
	std::vector<uint>			_SoundSeq;
	/// Sequence of delay in millisec.
	std::vector<uint>			_DelaySeq;

	/// Duration of xfade in millisec.
	uint32						_XFadeLenght;
	/// Flag for fade in
	bool						_DoFadeIn;
	/// Flag for fade out (only on normal termination, not explicit stop).
	bool						_DoFadeOut;

	mutable bool				_MaxDistValid;

	// Duration of sound.
	uint32						_Duration;
	// flag for validity of duration (after first evaluation).
	bool						_DurationValid;

/*	uint16_string				_SoundPattern;
	uint16_string				_Intervals;
	bool						_Spawn;
	ISoundController			*_VolumeEnvelope;
	ISoundController			*_FreqModulation;
	std::string					_StringBuffer;
*/
};

} // namespace

#endif // NL_COMPLEX_SOUND_H

