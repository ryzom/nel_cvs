/** \file background_sound_manager.cpp
 * CBackgroundSoundManager
 *
 * $Id: background_sound_manager.cpp,v 1.2 2002/07/19 15:07:12 miller Exp $
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

#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"

#include "nel/ligo/primitive.h"

#include "nel/sound/u_source.h"

#include "background_sound_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;


namespace NLSOUND {

UAudioMixer *CBackgroundSoundManager::_AudioMixer = NULL;

class CBackgroundSound
{
public:

	CBackgroundSound () : SourceDay(NULL), SourceNight(NULL) {}

	// name of the background sound
	string				Name;

	// point that delimit the zone (patate)
	vector<CVector>		Points;

	USource				*SourceDay;
	USource				*SourceNight;
};

static vector<CBackgroundSound> BackgroundSounds;

static float OldRatio = 10.0f;

void CBackgroundSoundManager::init (UAudioMixer *am)
{
	_AudioMixer = am;
}

void CBackgroundSoundManager::release ()
{
	unload ();
	_AudioMixer = NULL;
}


void CBackgroundSoundManager::load (const string &continent)
{
	CIFile file;
	CPrimRegion region;

	string fn = continent+"_audio.prim";

	nlinfo ("loading '%s'", fn.c_str());

	if (!file.open (CPath::lookup(fn, false)))
		return;
	
	unload();

	CIXml xml;
	xml.init (file);
	region.serial(xml);
	file.close ();

	nlinfo ("Region '%s' contains %d zones for the background sounds", continent.c_str(), region.VZones.size());

	BackgroundSounds.resize(region.VZones.size());
	for (uint i = 0; i < region.VZones.size(); i++)
	{
		if(region.VZones[i].VPoints.size()>2)
		{
			CVector srcpos( region.VZones[i].VPoints[0].x, region.VZones[i].VPoints[0].y, region.VZones[i].VPoints[0].z );

			BackgroundSounds[i].Points = region.VZones[i].VPoints;
			BackgroundSounds[i].Name = region.VZones[i].Name;

			sint32 pos1 = BackgroundSounds[i].Name.find ("-");
			if(pos1 == string::npos)
			{
				nlwarning ("zone %d have the malformated name '%s' missing -name-", i, BackgroundSounds[i].Name.c_str());
				return;
			}
			pos1++;

			sint32 pos2 = BackgroundSounds[i].Name.find ("-", pos1);
			if(pos2 == string::npos)
			{
				nlwarning ("zone %d have the malformated name '%s' missing -name-", i, BackgroundSounds[i].Name.c_str());
				return;
			}

			string sample = BackgroundSounds[i].Name.substr(pos1, pos2-pos1);
			
			BackgroundSounds[i].SourceDay = _AudioMixer->createSource( string(sample+"_day").c_str() );
			if(BackgroundSounds[i].SourceDay == NULL)
			{
				nlwarning ("Can't load '%s'", string(sample+"_day").c_str());
			}
			else
			{
				BackgroundSounds[i].SourceDay->setPos( srcpos );
				BackgroundSounds[i].SourceDay->setGain( 1.0f );
				BackgroundSounds[i].SourceDay->stop();
			}

			BackgroundSounds[i].SourceNight = _AudioMixer->createSource( string(sample+"_night").c_str() );
			if(BackgroundSounds[i].SourceNight == NULL)
			{
				nlwarning ("Can't load '%s'", string(sample+"_night").c_str());
			}
			else
			{
				BackgroundSounds[i].SourceNight->setPos( srcpos );
				BackgroundSounds[i].SourceNight->setGain( 0.0f );
				BackgroundSounds[i].SourceNight->stop();
			}
		}
	}
	OldRatio = 10.0f;
}


void CBackgroundSoundManager::play ()
{
	for (uint i = 0; i < BackgroundSounds.size(); i++)
	{
		if(BackgroundSounds[i].SourceDay != NULL)
		{
			BackgroundSounds[i].SourceDay->play();
		}
		if(BackgroundSounds[i].SourceNight != NULL)
		{
			BackgroundSounds[i].SourceNight->play();
		}
	}
}

void CBackgroundSoundManager::stop ()
{
	for (uint i = 0; i < BackgroundSounds.size(); i++)
	{
		if(BackgroundSounds[i].SourceDay != NULL && BackgroundSounds[i].SourceDay->isPlaying())
		{
			BackgroundSounds[i].SourceDay->stop();
		}
		if(BackgroundSounds[i].SourceNight != NULL && BackgroundSounds[i].SourceNight->isPlaying())
		{
			BackgroundSounds[i].SourceNight->stop();
		}
	}
}

void CBackgroundSoundManager::unload ()
{
	stop();

	for (uint i = 0; i < BackgroundSounds.size(); i++)
	{
		if(BackgroundSounds[i].SourceDay != NULL)
		{
			if(BackgroundSounds[i].SourceDay->isPlaying())
				BackgroundSounds[i].SourceDay->stop();
			delete BackgroundSounds[i].SourceDay;
			BackgroundSounds[i].SourceDay = NULL;
		}
		if(BackgroundSounds[i].SourceNight != NULL)
		{
			if(BackgroundSounds[i].SourceNight->isPlaying())
				BackgroundSounds[i].SourceNight->stop();
			delete BackgroundSounds[i].SourceNight;
			BackgroundSounds[i].SourceNight = NULL;
		}
	}

	BackgroundSounds.clear();
}

void CBackgroundSoundManager::setListenerPosition (const CVector &listenerPosition)
{
	CVector nearestPoint;
	float nearestDist;
	
	// it s on 2d so we don't have z
	CVector listener = listenerPosition;
	listener.z = 0.0f;

//	nlinfo ("listener is now %f %f %f", listener.x, listener.y, listener.z);

	/// \todo compute the position of source only every second and only if necessary, not for each listener position changes
	for (uint i = 0; i < BackgroundSounds.size(); i++)
	{
		if (BackgroundSounds[i].Points.size()==0)
			continue;

		if(CPrimZone::contains(listener, BackgroundSounds[i].Points))
		{
			nearestPoint = listenerPosition;	// use the real listener position, not the 0 z centered
			nearestDist = 0.0f;
			//nlinfo ("inside patate %d name '%s' ", i, BackgroundSounds[i].Name.c_str());
		}
		else
		{
			nearestDist = 10000000.0f;
			for (uint j = 0; j < BackgroundSounds[i].Points.size(); j++)
			{
				CVector A = BackgroundSounds[i].Points[j];
				CVector B = BackgroundSounds[i].Points[(j+1)%BackgroundSounds[i].Points.size()];
				CVector V = (B-A).normed();
				float t = ((listener-A)*V)/(B-A).norm();
				float dist;
				CVector point;
				if (t < 0.0f)
				{
					point = A;
					dist = (A-listener).norm();
				}
				else if (t > 1.0f)
				{
					point = B;
					dist = (B-listener).norm();
				}
				else
				{
					point = A + t*(B-A);
					dist = (listener-point).norm();
				}

				if (dist < nearestDist)
				{
					nearestPoint = point;
					nearestDist = dist;
				}
			}
			//nlinfo ("near patate %d name '%s' from %f ", i, BackgroundSounds[i].Name.c_str(), nearestDist);
		}

		nearestPoint.z = listenerPosition.z;

//nlinfo ("nearest dist = %f pos = %f %f %f lis %f %f %f", nearestDist, nearestPoint.x, nearestPoint.y, nearestPoint.z, listenerPosition.x, listenerPosition.y, listenerPosition.z);

		if(BackgroundSounds[i].SourceDay != NULL)
			BackgroundSounds[i].SourceDay->setPos (nearestPoint);
		if(BackgroundSounds[i].SourceNight != NULL)
			BackgroundSounds[i].SourceNight->setPos (nearestPoint);
	}
}

void CBackgroundSoundManager::update ()
{

}

uint32 CBackgroundSoundManager::getZoneNumber ()
{
	return BackgroundSounds.size();
}

const vector<CVector> &CBackgroundSoundManager::getZone(uint32 zone)
{
	nlassert (zone< BackgroundSounds.size());
	return BackgroundSounds[zone].Points;
}

CVector CBackgroundSoundManager::getZoneSourcePos(uint32 zone)
{
	nlassert (zone< BackgroundSounds.size());
	CVector pos;
	if (BackgroundSounds[zone].SourceDay != NULL)
		BackgroundSounds[zone].SourceDay->getPos(pos);
	return pos;
}


void CBackgroundSoundManager::setDayNightRatio(float ratio)
{
	// 0 is day
	// 1 is night

	nlassert (ratio>=0.0f && ratio<=1.0f);

	if (OldRatio == ratio)
		return;
	else
		OldRatio = ratio;


	// recompute all source volume

	for (uint i = 0; i < BackgroundSounds.size(); i++)
	{
		if(ratio == 0.0f)
		{
			if(BackgroundSounds[i].SourceDay != NULL)
			{
				if (!BackgroundSounds[i].SourceDay->isPlaying())
					BackgroundSounds[i].SourceDay->play();
				
				BackgroundSounds[i].SourceDay->setGain(1.0f);
			}
			
			if(BackgroundSounds[i].SourceNight != NULL)
			{
				if (BackgroundSounds[i].SourceNight->isPlaying())
					BackgroundSounds[i].SourceNight->stop();
			}
		}
		else if (ratio == 1.0f)
		{
			if(BackgroundSounds[i].SourceDay != NULL)
			{
				if (BackgroundSounds[i].SourceDay->isPlaying())
					BackgroundSounds[i].SourceDay->stop();
			}

			if(BackgroundSounds[i].SourceNight != NULL)
			{
				if (!BackgroundSounds[i].SourceNight->isPlaying())
					BackgroundSounds[i].SourceNight->play();

				BackgroundSounds[i].SourceNight->setGain(1.0f);
			}
		}
		else
		{
			if(BackgroundSounds[i].SourceDay != NULL)
			{
				if (!BackgroundSounds[i].SourceDay->isPlaying())
					BackgroundSounds[i].SourceDay->play();
				BackgroundSounds[i].SourceDay->setGain(1.0f-ratio);
			}

			if(BackgroundSounds[i].SourceNight != NULL)
			{
				if (!BackgroundSounds[i].SourceNight->isPlaying())
					BackgroundSounds[i].SourceNight->play();
				BackgroundSounds[i].SourceNight->setGain(ratio);
			}
		}
	}



}

} // NLSOUND
