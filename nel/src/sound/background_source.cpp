/** \file source_user.cpp
 * CSourceUSer: implementation of USource
 *
 * $Id: background_source.cpp,v 1.1 2002/11/04 15:40:43 boucher Exp $
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

#include "background_source.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND 
{

/// Time interval between environnment checking (millisec). 
const TTime		ENV_CHK_INTERVAL = 3000;	


CBackgroundSource::CBackgroundSource(CBackgroundSound *backgroundSource, bool spawn, TSpawnEndCallback cb, void *cbUserParam, CSoundContext *context)
:	CSourceCommon(backgroundSource, spawn, cb, cbUserParam, context)
{
	_BackgroundSound = backgroundSource;
}

CBackgroundSource::~CBackgroundSource()
{
	if (_Playing)
		stop();
}

void CBackgroundSource::setSound( TSoundId id, CSoundContext *context)
{
	nlassert(false);
}

TSoundId CBackgroundSource::getSound()
{
	return NULL;
}
	
void CBackgroundSource::setGain( float gain )
{
	CSourceCommon::setGain(gain);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setRelativeGain(_Gain);
	}
}
void CBackgroundSource::setRelativeGain( float gain )
{
	CSourceCommon::setRelativeGain(gain);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setRelativeGain(_Gain);
	}
}

void CBackgroundSource::setPos( const NLMISC::CVector& pos )
{
	CSourceCommon::setPos(pos);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setPos(pos);
	}
}

void CBackgroundSource::setVelocity( const NLMISC::CVector& vel )
{
	CSourceCommon::setVelocity(vel);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setVelocity(vel);
	}
}
void CBackgroundSource::setDirection( const NLMISC::CVector& dir )
{
	CSourceCommon::setDirection(dir);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setDirection(dir);
	}
}



void CBackgroundSource::play()
{
	if (_Playing)
		stop();

	CAudioMixerUser *mixer = CAudioMixerUser::instance();
		
	const vector<CBackgroundSound::TSoundInfo> &sounds = _BackgroundSound->getSounds();
	vector<CBackgroundSound::TSoundInfo>::const_iterator first(sounds.begin()), last(sounds.end());

	for (; first != last; ++first)
	{
		TSubSource subSource;
		subSource.Source = mixer->createSource(first->SoundName, false);
		subSource.Filter = first->Filter;
		subSource.FadeStart = 0;
		subSource.FilterFadeIn = first->FilterFadeIn;
		subSource.FilterFadeOut = first->FilterFadeOut;

		if (subSource.Source && checkFilter(subSource.Filter))
		{
			subSource.Status = SUB_STATUS_PLAY;
			subSource.Source->setRelativeGain(_Gain);
			subSource.Source->setPos(_Position);
			subSource.Source->play();
		}
		else
		{
			subSource.Status = SUB_STATUS_STOP;
		}

		_Sources.push_back(subSource);
	}

	_LastCheck = NLMISC::CTime::getLocalTime();

	// Set an event for next env checking.
	mixer->addEvent(this, _LastCheck+ENV_CHK_INTERVAL);

	CSourceCommon::play();
}

void CBackgroundSource::stop()
{
	if (_Playing)
	{
		CAudioMixerUser *mixer = CAudioMixerUser::instance();
		mixer->unregisterUpdate(this);
		mixer->removeEvent(this);
	
		while (!_Sources.empty())
		{
			TSubSource &subSource = _Sources.back();
			if (subSource.Source != NULL)
			{
				subSource.Source->stop();
				mixer->removeSource(subSource.Source);

			}
			_Sources.pop_back();
		}
	}

	CSourceCommon::stop();
}

void CBackgroundSource::onUpdate()
{
	// this is called for fade in/out.
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	bool doFade = false;
	TTime now = NLMISC::CTime::getLocalTime();
	float gain;

	vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != NULL)
		{
			if (first->Status == SUB_STATUS_FADEIN)
			{
				if (first->FilterFadeIn > 0)
					gain = (now - first->FadeStart) / float(first->FilterFadeIn);
				else
					gain = 1.0f;

				if (gain < 1.0f)
				{
					first->Source->setRelativeGain(gain);
					doFade = true;
				}
				else
				{
					// fade end
					first->Source->setRelativeGain(1.0f * _Gain);
					first->Status = SUB_STATUS_PLAY;
				}
			}
			else if (first->Status == SUB_STATUS_FADEOUT)
			{
				if (first->FilterFadeOut > 0)
					gain = 1.0f - (now - first->FadeStart) / float(first->FilterFadeOut);
				else
					gain = 0.0f;
				if (gain > 0.0f)
				{
					first->Source->setRelativeGain(gain * _Gain);
					doFade = true;
				}
				else
				{
					first->Source->stop();
					first->Status = SUB_STATUS_STOP;
				}
			}
		}
	}

	// any fade to update ?
	if (!doFade)
		mixer->unregisterUpdate(this);
}

void CBackgroundSource::onEvent()
{
	// check all sounds and test for start or stop.
	CAudioMixerUser *mixer = CAudioMixerUser::instance();

	bool doFade = false;
	TTime now = NLMISC::CTime::getLocalTime();

	vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != NULL)
		{
			if (checkFilter(first->Filter))
			{
				// source ok
				if (first->Status == SUB_STATUS_STOP)
				{
					first->Status = SUB_STATUS_FADEIN;
					first->Source->setRelativeGain(0.0f);
					first->Source->setPos(_Position);
					first->Source->play();
					first->FadeStart = now;
					doFade = true;
				}
				else if (first->Status == SUB_STATUS_FADEOUT)
				{
					first->Status = SUB_STATUS_FADEIN;
					// Replace fade start date to smoosly reverse the fade.
					first->FadeStart = now - (first->FadeStart - first->FilterFadeOut);
					doFade = true;
				}
			}
			else
			{
				// source masked.
				if (first->Status == SUB_STATUS_PLAY)
				{
					first->Status = SUB_STATUS_FADEOUT;
					first->FadeStart = now;
					doFade = true;
				}
				else if (first->Status == SUB_STATUS_FADEIN)
				{
					first->Status = SUB_STATUS_FADEOUT;
					// Replace fade start date to smoosly reverse the fade.
					first->FadeStart = now - (first->FadeStart - first->FilterFadeIn);
					doFade = true;
				}
			}
		}
	}

	if (doFade)
	{
		// register for fade in/out
		mixer->registerUpdate(this);
	}

	_LastCheck = NLMISC::CTime::getLocalTime();

	// set the next event.
	mixer->addEvent(this, _LastCheck + ENV_CHK_INTERVAL);
}


bool CBackgroundSource::checkFilter(const UAudioMixer::TBackgroundFlags &filter)
{
	// Get the current environnement from the bakgound sound manager.
	const UAudioMixer::TBackgroundFlags &bg = CAudioMixerUser::instance()->getBackgroundSoundManager()->getBackgroundFlags();

	bool ok = true;

	for (uint i=0; i<16; ++i)
		ok &= filter.Flags[i] || !bg.Flags[i];

	return ok;
}


} // NLSOUND