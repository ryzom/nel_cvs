
#include "stdsound.h"
#include "sound_pattern.h"
#include "sound_controller.h"
#include "nel/misc/common.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND
{

// ********************************************************

CSoundPattern::CSoundPattern()
{
	_TicksPerSeconds = 1.0;
	_Spawn = false;
	_VolumeEnvelope = 0;
	_FreqModulation = 0;
}

// ********************************************************

CSoundPattern::~CSoundPattern()
{
	if (_VolumeEnvelope != 0)
	{
		delete _VolumeEnvelope;
	}

	if (_FreqModulation != 0)
	{
		delete _FreqModulation;
	}
}

// ********************************************************

void CSoundPattern::removeSound(std::string& name)
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

// ********************************************************

void CSoundPattern::getSounds(std::vector<std::string>& sounds)
{
	vector<string>::iterator iter;

	for (iter = _Sounds.begin(); iter != _Sounds.end(); iter++)
	{
		sounds.push_back(*iter);
	}
}

// ********************************************************

string& CSoundPattern::getSound(PatternIterator* iter)
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

// ********************************************************

void CSoundPattern::setSoundPattern(std::string& l)
{
	parsePattern(l, _SoundPattern);
}

// ********************************************************

void CSoundPattern::getSoundPattern(std::string& l)
{
	concatenatePattern(l, _SoundPattern);
}

// ********************************************************

uint16 CSoundPattern::getInterval(IntervalIterator* iter)
{
	return _Intervals[iter->_Value];
}

// ********************************************************

void CSoundPattern::generateRandomPattern(uint length)
{
	generateRandomPattern(_SoundPattern, length, 0, _Sounds.size());
}

// ********************************************************

void CSoundPattern::generateRandomMin1Pattern(uint length)
{
	generateRandomMin1Pattern(_SoundPattern, length, 0, _Sounds.size());
}

// ********************************************************

void CSoundPattern::setIntervals(std::string& l)
{
	parsePattern(l, _Intervals);
}

// ********************************************************

void CSoundPattern::getIntervals(std::string& l)
{
	concatenatePattern(l, _Intervals);
}

// ********************************************************

void CSoundPattern::generateRandomIntervals(uint length, uint16 min, uint16 max)
{
	generateRandomPattern(_Intervals, length, min, max);
}

// ********************************************************

void CSoundPattern::generateRandomMin1Intervals(uint length, uint16 min, uint16 max)
{
	generateRandomMin1Pattern(_Intervals, length, min, max);
}

// ********************************************************

void CSoundPattern::parsePattern(std::string& l, uint16_string& p)
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

// ********************************************************

void CSoundPattern::concatenatePattern(std::string& l, uint16_string& p)
{
	uint16_string::iterator iter;
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

// ********************************************************

void CSoundPattern::generateRandomPattern(uint16_string& pattern, uint length, uint16 min, uint16 max)
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

// ********************************************************

void CSoundPattern::generateRandomMin1Pattern(uint16_string& pattern, uint length, uint16 min, uint16 max)
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

// ********************************************************

void CSoundPattern::expandString(std::string& s, std::string& buffer)
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

}