
#include "stdsound.h"
#include "complex_sound.h"
#include "nel/misc/path.h"
//#include "sound_controller.h"
#include "nel/misc/common.h"
#include <numeric>

using namespace std;
using namespace NLMISC;

namespace NLSOUND
{

bool CComplexSound::isDetailed() const
{
	return false;
}

void CComplexSound::parseSequence(const std::string &str, std::vector<uint> &seq, uint scale)
{
	seq.clear();

	string	val;
	string::const_iterator first(str.begin()), last(str.end());

	for (; first != last; ++first)
	{
		if (*first != ';')
			val += *first;
		else
		{
			seq.push_back(atoi(val.c_str()) * scale);
			val.clear();
		}
	}

	// parse the last value
	if (!val.empty())
		seq.push_back(atoi(val.c_str()) * scale);

}

void CComplexSound::getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const
{
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	std::vector<std::string>::const_iterator first(_Sounds.begin()), last(_Sounds.end());
	for (; first != last; ++first)
	{
		CSound *sound = mixer->getSoundId(*first);
		subsounds.push_back(make_pair(*first, sound));
	}
}


uint32 CComplexSound::getDuration(std::string *buffername)
{
	// evaluate the duration of the sound...

	if (_DurationValid)
		return _Duration;

	// catch the duration of all sub sound.
	CAudioMixerUser *mixer = CAudioMixerUser::instance();

	vector<sint32>	durations;
	std::vector<std::string>::iterator first(_Sounds.begin()), last(_Sounds.end());
	for (; first != last; ++first)
	{
		CSound *sound = mixer->getSoundId(*first);
		if (sound != NULL)
		{
			durations.push_back(sint32(sound->getDuration()));
		}
		else
			durations.push_back(0);
			
	}

	_Duration = 0;
	switch (_PatternMode)
	{
	case MODE_CHAINED:
		{
			// sum the duration minus the xfade time (this is an aproximation if sample are shorter than 2 xfade time)
			vector<uint>::iterator first(_SoundSeq.begin()), last(_SoundSeq.end()), prev;
			for (; first != last; ++first)
			{
				if (first != _SoundSeq.begin())
				{
					// remove a xfade value
					_Duration -= minof<uint32>(uint32(_XFadeLenght / _TicksPerSeconds), durations[*first % durations.size()] / 2, durations[*prev % durations.size()] /2);
				}
				if (!durations.empty())
					_Duration += durations[*first % durations.size()];
				prev = first;
			}
//			_Duration -= max(sint(0), sint(_XFadeLenght * (_SoundSeq.size()-2)  ));
		}
		break;
	case MODE_SPARSE:
		{
			if (_SoundSeq.empty())
				_Duration = 0;
			else
			{
				uint soundIndex = 0;
				_Duration = durations[soundIndex++];

				std::vector<uint>::iterator first(_DelaySeq.begin()), last(_DelaySeq.end());

				for (; first != last; ++first)
				{
					// add the delay
					_Duration += uint32(*first / _TicksPerSeconds);
					// add the sound lenght
					_Duration += durations[soundIndex++ % durations.size()];
				}
			}
		}
		break;
	case MODE_ALL_IN_ONE:
		// only find the longueur sound.
		_Duration = *(std::max_element(durations.begin(), durations.end()));
		break;
	default:
		return 0;
	}

	_DurationValid = true;
	return _Duration;

}


// ********************************************************

CComplexSound::CComplexSound()
:	_MaxDistValid(false),
	_TicksPerSeconds(1.0f),
	_PatternMode(CComplexSound::MODE_UNDEFINED),
	_DurationValid(false),
	_Duration(0),
	_XFadeLenght(3000)		// defualt to 3000 sec.
{
}

// ********************************************************

CComplexSound::~CComplexSound()
{
/*	if (_VolumeEnvelope != 0)
	{
		delete _VolumeEnvelope;
	}

	if (_FreqModulation != 0)
	{
		delete _FreqModulation;
	}
*/
}

float CComplexSound::getMaxDistance() const
{
	if (!_MaxDistValid)
	{
		// compute the max distance by checking the max distance of all sounds.
		CAudioMixerUser *mixer = CAudioMixerUser::instance();

		// Hum, getMaxDistance is const, but we must compute the real max dist and update it !
		CComplexSound *This = const_cast<CComplexSound*>(this);

		This->_MaxDist = 0.0f;
		std::vector<std::string>::const_iterator first(_Sounds.begin()), last(_Sounds.end());

		for (; first != last; ++first)
		{
			CSound *sound = mixer->getSoundId(*first);
			if( sound != NULL)
			{
				This->_MaxDist = max(_MaxDist, sound->getMaxDistance());
			}
		}
		// security check.
		if (_MaxDist == 0.0f)
			This->_MaxDist = 1000000.0f;
	}

	_MaxDistValid = true;
	return _MaxDist;
}



/// Load the sound parameters from georges' form
void	CComplexSound::importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot)
{
	NLGEORGES::UFormElm *psoundType;
	std::string dfnName;

	_DurationValid = false;

	// some basic checking.
	formRoot.getNodeByName(&psoundType, ".SoundType");
	nlassert(psoundType != NULL);
	psoundType->getDfnName(dfnName);
	nlassert(dfnName == "complex_sound.dfn");

	// Call the base class
	CSound::importForm(filename, formRoot);

	// Beat per second.
	formRoot.getValueByName(_TicksPerSeconds, ".SoundType.Beat");
	//beat can't be null or negative! 
	if (_TicksPerSeconds <= 0.0f)
		_TicksPerSeconds = 1.0f;



	// List of sound int this pattern
	NLGEORGES::UFormElm	*psoundsArray;
	_Sounds.clear();
	formRoot.getNodeByName(&psoundsArray, ".SoundType.SoundList");

	if (psoundsArray != NULL)
	{
		uint size;
		psoundsArray->getArraySize(size);
		for (uint i=0; i<size; ++i)
		{
			string soundname;
			if (psoundsArray->getArrayValue(soundname, i))
			{
				soundname = CFile::getFilenameWithoutExtension(soundname);
				_Sounds.push_back(soundname);
			}
		}
	}


	// Mode of the complex sound.
	string	mode;
	formRoot.getValueByName(mode, ".SoundType.Mode");

	if (mode == "Chained" || mode == "Sparse")
	{
		// XFade lenght
		formRoot.getValueByName(_XFadeLenght, ".SoundType.XFadeLenght");
		// Fade in/out flag.
		formRoot.getValueByName(_DoFadeIn, ".SoundType.DoFadeIn");
		formRoot.getValueByName(_DoFadeOut, ".SoundType.DoFadeOut");

		// convert xfade to millisec.
		_XFadeLenght *= 1000;
		_PatternMode = MODE_CHAINED;
		// just read the sequence
		_SoundSeq.clear();

		string	str;
		formRoot.getValueByName(str, ".SoundType.SoundSeq");
		parseSequence(str, _SoundSeq);

		if (mode == "Sparse")
		{
			_PatternMode = MODE_SPARSE;
			// also read the delay sequence
			_DelaySeq.clear();

			string	str;
			formRoot.getValueByName(str, ".SoundType.DelaySeq");
			// parse the delay and premult by 1000 (for millisec).
			parseSequence(str, _DelaySeq, 1000);
		}
	}
	else if (mode == "AllInOne")
	{
		_PatternMode = MODE_ALL_IN_ONE;
		// nothing special to read.
	}
	else
		nlassertex(false, ("Unsupported mode : %s", mode.c_str()));
	
}

// ********************************************************

/*void CComplexSound::removeSound(std::string& name)
{
	vector<string>::iterator iter;

	for (iter = _Sounds.begin(); iter != _Sounds.end(); iter++)
	{
		if (*iter == name)
		{
			_Sounds.erase(iter);
			return;
		}
	}
}
*/
// ********************************************************
/*
void CComplexSound::getSounds(std::vector<std::string>& sounds)
{
	vector<string>::iterator iter;

	for (iter = _Sounds.begin(); iter != _Sounds.end(); iter++)
	{
		sounds.push_back(*iter);
	}
}
*/
// ********************************************************

/*string& CComplexSound::getSound(PatternIterator* iter)
{
	uint16 i = _SoundPattern[iter->_Value];
	if (_Sounds[i].find('%') == _Sounds[i].npos)
	{
		return _Sounds[i];
	}
	else
	{
		expandString(_Sounds[i], _StringBuffer);
		return _StringBuffer;
	}
}
*/
// ********************************************************

/*void CComplexSound::setSoundPattern(const std::string& l)
{
//	parsePattern(l, _SoundPattern);
}
*/
// ********************************************************
/*
void CComplexSound::getSoundPattern(std::string& l)
{
//	concatenatePattern(l, _SoundPattern);
}
*/
// ********************************************************
/*
uint16 CComplexSound::getInterval(IntervalIterator* iter)
{
	return _Intervals[iter->_Value];
}
*/
// ********************************************************
/*
void CComplexSound::generateRandomPattern(uint length)
{
//	generateRandomPattern(_SoundPattern, length, 0, _Sounds.size());
}
*/
// ********************************************************
/*
void CComplexSound::generateRandomMin1Pattern(uint length)
{
//	generateRandomMin1Pattern(_SoundPattern, length, 0, _Sounds.size());
}
*/
// ********************************************************
/*
void CComplexSound::setIntervals(const std::string& l)
{
//	parsePattern(l, _Intervals);
}
*/
// ********************************************************
/*
void CComplexSound::getIntervals(std::string& l)
{
//	concatenatePattern(l, _Intervals);
}
*/
// ********************************************************
/*
void CComplexSound::generateRandomIntervals(uint length, uint16 min, uint16 max)
{
//	generateRandomPattern(_Intervals, length, min, max);
}
*/
// ********************************************************
/*
void CComplexSound::generateRandomMin1Intervals(uint length, uint16 min, uint16 max)
{
//	generateRandomMin1Pattern(_Intervals, length, min, max);
}
*/
// ********************************************************
/*
void CComplexSound::parsePattern(const std::string& l, uint16_string& p)
{
	string buf;
	enum { number, comma } state = comma;
	char c;

	for (uint index = 0; index <= l.size(); index++)
	{
		c = l.at(index);

		switch (state)
		{
		case comma:
			if (isdigit(c))
			{
				buf.append(1, c);
				state = number;
			}
			else
			{
				throw exception();
			}
			break;

		case number:
			if (isdigit(c))
			{
				buf.append(1, c);
			}
			else if (c == ',')
			{
				uint16 n = atoi(buf.c_str());
				p.push_back(n);
				buf.erase();
				state = comma;
			}
			else
			{
				throw exception();
			}
			break;
		}
	}
}
*/
// ********************************************************
/*
void CComplexSound::concatenatePattern(std::string& l, const uint16_string& p)
{
	uint16_string::const_iterator iter;
	char s[64];

	iter = p.begin();
	if (iter == p.end())
	{
		return;
	}

	smprintf(s, 64, "%d", *iter);
	l.append(s);

	while (iter != p.end())
	{
		smprintf(s, 64, "%d", *iter);
		l.append(",").append(s);
		iter++;
	}
}
*/
// ********************************************************
/*
void CComplexSound::generateRandomPattern(uint16_string& pattern, uint length, uint16 min, uint16 max)
{
	pattern.erase();
	pattern.reserve(length);

	for (uint i = 0; i < length; i++)
	{
		uint16 value;
		value = rand() % length;
		pattern.push_back(value);
	}
}
*/
// ********************************************************
/*
void CComplexSound::generateRandomMin1Pattern(uint16_string& pattern, uint length, uint16 min, uint16 max)
{
	uint16 value;
	sint32 prev = -1;

	pattern.erase();
	pattern.reserve(length);

	for (uint i = 0; i < length; i++)
	{
		value = rand() % (length - 1);
		if (prev != -1)
		{
			value = (prev + 1 + value) % length;
		}
		pattern.push_back(value);
	}
}
*/
// ********************************************************
/*
void CComplexSound::expandString(const std::string& s, std::string& buffer)
{
	buffer.erase();
	buffer.reserve(s.size() + 128);

	for (uint i = 0; i < s.size(); i++)
	{
		char c = s.at(i);

		if (c == '%')
		{
			// get next character
			if (++i == s.size())
			{
				throw exception("Invalid format string");
			}

			// replace the variable with corresponding string
			switch (s.at(i))
			{
			case 's':
				buffer.append("TATA");
				break;

			default:
				throw exception("Invalid format string");
			}
		}
		else
		{
			buffer.append(1, c);
		}
	}
}
*/
}