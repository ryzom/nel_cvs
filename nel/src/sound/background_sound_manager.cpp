/** \file background_sound_manager.cpp
 * CBackgroundSoundManager
 *
 * $Id: background_sound_manager.cpp,v 1.13 2003/01/08 15:48:11 boucher Exp $
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
#include "clustered_sound.h"

#include "background_sound_manager.h"
#include "source_common.h"
#include "clustered_sound.h"
#include <algorithm>
#include "background_source.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;


namespace NLSOUND {

// external sound are cliping after 10 meter inside the inner patate
const float	INSIDE_FALLOF = 10.0f; 

//UAudioMixer *CBackgroundSoundManager::_AudioMixer = NULL;


CBackgroundSoundManager::CBackgroundSoundManager()
: _LastPosition(0,0,0), _Playing(false)
{
	for (uint i=0; i<UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
	{
		_BackgroundFlags.Flags[i] = false;
		_FilterFadesStart[i] = 0;
		_FilterFadeValues[i] = 1.0f;
	}
}

CBackgroundSoundManager::~CBackgroundSoundManager()
{
	unload();
}

const UAudioMixer::TBackgroundFlags &CBackgroundSoundManager::getBackgroundFlags()
{
	return  _BackgroundFlags;
}


void CBackgroundSoundManager::setBackgroundFilterFades(const UAudioMixer::TBackgroundFilterFades &backgroundFilterFades)
{
	_BackgroundFilterFades = backgroundFilterFades;
}

const UAudioMixer::TBackgroundFilterFades &CBackgroundSoundManager::getBackgroundFilterFades()
{
	return _BackgroundFilterFades;
}


void CBackgroundSoundManager::addSound(const std::string &soundName, const std::vector<NLLIGO::CPrimVector> &points, bool isPath)
{
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	uint layerId = 0;
	uint n = 0;
	string name;
	// count the number of '-' in the string.
	n = std::count(soundName.begin(), soundName.end(), '-');

	if (n == 2)
	{
		// no layer spec, default to layer A
		uint32 pos1 = soundName.find ("-");
		if(pos1 == string::npos)
		{
			nlwarning ("zone have the malformated name '%s' missing -name-", soundName.c_str());
			return;
		}
		pos1++;

		uint32 pos2 = soundName.find ("-", pos1);
		if(pos2 == string::npos)
		{
			nlwarning ("zone have the malformated name '%s' missing -name-", soundName.c_str());
			return;
		}

		name = soundName.substr(pos1, pos2-pos1);
	}
	else if (n == 3)
	{
		// layer spec !
		uint32 pos1 = soundName.find ("-");
		uint32 pos2 = soundName.find ("-", pos1+1);
		if(pos1 == string::npos || pos2 == string::npos)
		{
			nlwarning ("zone have the malformated name '%s' missing -layerId- or -name-", soundName.c_str());
			return;
		}
		pos1++;

		uint32 pos3 = soundName.find ("-", pos2+1);
		if(pos3 == string::npos)
		{
			nlwarning ("zone have the malformated name '%s' missing -name-", soundName.c_str());
			return;
		}

		char id = soundName[pos1];

		// check caps
		if (id < 'a')
			id = id + ('a' - 'A');

		layerId = id - 'a';

		NLMISC::clamp(layerId, 0u, BACKGROUND_LAYER-1);
		pos2++;

		name = soundName.substr(pos2, pos3-pos2);
	}
	else
	{
		nlwarning ("zone have the malformated name '%s",  soundName.c_str());
		return;
	}
		

	TSoundData	sd;

	sd.SoundName = name;
	sd.Sound = mixer->getSoundId(sd.SoundName);
	sd.Source = 0;

	// Copy the points
	sd.Points.resize (points.size ());
	for (uint i=0; i<points.size (); i++)
		sd.Points[i] = points[i];

	sd.Selected = false;
	sd.IsPath = isPath;

	if (sd.Sound != 0)
	{
		// the sound is available !
		// compute bouding box/
		CVector	vmin(FLT_MAX, FLT_MAX, 0), vmax(-FLT_MAX, -FLT_MAX, 0);

		vector<CVector>::iterator first(sd.Points.begin()), last(sd.Points.end());
		for (; first != last; ++first)
		{
			vmin.x = min(first->x, vmin.x);
			vmin.y = min(first->y, vmin.y);
			vmax.x = max(first->x, vmax.x);
			vmax.y = max(first->y, vmax.y);
		}
		sd.MaxBox = vmax;
		sd.MinBox = vmin;

		// compute the surface without the sound distance
		sd.Surface = (vmax.x - vmin.x) * (vmax.y - vmin.y);

		// add the eard distance of the sound.
		float	dist = sd.Sound->getMaxDistance();
		sd.MaxBox.x += dist;
		sd.MaxBox.y += dist;
		sd.MinBox.x -= dist;
		sd.MinBox.y -= dist;

		sd.MaxDist = dist;

		// store the sound. 
		// TODO : handle the three layer.
		_Layers[layerId].push_back(sd);
	}
	else
	{
		nlwarning ("The sound '%s' can't be loaded", sd.SoundName.c_str());
	}
}


void CBackgroundSoundManager::loadSamplesFromRegion(const NLLIGO::CPrimRegion &region)
{
	_Banks.clear();

	for (uint i=0; i< region.VZones.size(); ++i)
	{
		if (region.VZones[i].VPoints.size() > 2)
		{
			TBanksData	bd;
			uint pointCount = region.VZones[i].VPoints.size ();
			bd.Points.resize (pointCount);
			for (uint j=0; j<pointCount; j++)
			{
				bd.Points[j] = region.VZones[i].VPoints[j];
			}

			// compute bouding box.
			CVector	vmin(FLT_MAX, FLT_MAX, 0), vmax(-FLT_MAX, -FLT_MAX, 0);

			vector<CVector>::iterator first(bd.Points.begin()), last(bd.Points.end());
			for (; first != last; ++first)
			{
				vmin.x = min(first->x, vmin.x);
				vmin.y = min(first->y, vmin.y);
				vmax.x = max(first->x, vmax.x);
				vmax.y = max(first->y, vmax.y);
			}
			bd.MaxBox = vmax;
			bd.MinBox = vmin;

			// parse the zone name to find the samples name.
			std::vector<std::string>	splitted = split(region.VZones[i].Name, '-');

			if (splitted.size() > 2)
			{
				for (uint j=1; j<splitted.size()-1; ++j)
				{
					bd.Banks.push_back(splitted[j]);
				}

				// ok, store it in the container.
				_Banks.push_back(bd);
			}
			else
			{
				nlwarning ("A sample bank patatoid name did'nt contains banks name '%s'", region.VZones[i].Name.c_str());
			}
		}
		else
		{
			nlwarning ("A sample bank patatoid have less than 3 points '%s'", region.VZones[i].Name.c_str());
		}
	}
}

void CBackgroundSoundManager::loadEffecsFromRegion(const NLLIGO::CPrimRegion &region)
{
}

void CBackgroundSoundManager::loadSoundsFromRegion(const CPrimRegion &region)
{
	uint i;
	// remember playing state
	bool oldState = _Playing;
	unload();

	for (i = 0; i < region.VZones.size(); i++)
	{
		if(region.VZones[i].VPoints.size()>2)
		{
			addSound(region.VZones[i].Name, region.VZones[i].VPoints, false);
		}
		else
		{
			nlwarning ("A background sound patatoid have less than 3 points '%s'", region.VZones[i].Name.c_str());
		}
	}

	for (i = 0; i < region.VPaths.size(); i++)
	{
		if(region.VPaths[i].VPoints.size() > 1)
		{
			addSound(region.VPaths[i].Name, region.VPaths[i].VPoints, true);
		}
		else
		{
			nlwarning ("A background sound path have less than 2 points '%s'", region.VPaths[i].Name.c_str());
		}
	}
	for (i = 0; i < region.VPoints.size(); i++)
	{
		std::vector<CPrimVector>	points;
		points.push_back(region.VPoints[i].Point);

		addSound(region.VPoints[i].Name, points, false);
	}


	// restart playing ?
	if (oldState)
		play();
}

void CBackgroundSoundManager::load (const string &continent)
{
	// load the sound.
	{
		CIFile file;
		CPrimRegion region;
		string fn = continent+"_audio.prim";

		nlinfo ("loading '%s'", fn.c_str());

		string path = CPath::lookup(fn, false);

		if(!path.empty() && file.open (path))
		{
			CIXml xml;
			xml.init (file);
			region.serial(xml);
			file.close ();

			nlinfo ("Region '%s' contains %d zones for the background sounds", continent.c_str(), region.VZones.size());

			loadSoundsFromRegion(region);
		}
	}
	// load the effect.
	{
		CIFile file;
		CPrimRegion region;
		string fn = continent+"_effects.prim";

		nlinfo ("loading '%s'", fn.c_str());

		string path = CPath::lookup(fn, false);

		if(!path.empty() && file.open (path))
		{
			CIXml xml;
			xml.init (file);
			region.serial(xml);
			file.close ();

			nlinfo ("Region '%s' contains %d zones for the background effetcs", continent.c_str(), region.VZones.size());

			loadEffecsFromRegion(region);
		}
	}
	// load the samples banks.
	{
		CIFile file;
		CPrimRegion region;
		string fn = continent+"_samples.prim";

		nlinfo ("loading '%s'", fn.c_str());

		string path = CPath::lookup(fn, false);

		if(!path.empty() && file.open (path))
		{
			CIXml xml;
			xml.init (file);
			region.serial(xml);
			file.close ();

			nlinfo ("Region '%s' contains %d zones for the background samples banks", continent.c_str(), region.VZones.size());

			loadSamplesFromRegion(region);
		}
	}
}


void CBackgroundSoundManager::play ()
{
	if (_Playing)
		return;

	_Playing = true;

	CAudioMixerUser::instance()->registerUpdate(this);

	// init the filter value and filter start time
	for (uint i =0; i<UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
	{
		_FilterFadesStart[i] = 0;
		_FilterFadeValues[i] = 1.0f * !_BackgroundFlags.Flags[i];
	}
	// force an initial filtering 
	_DoFade = true;
	updateBackgroundStatus();

	
}


void CBackgroundSoundManager::stop ()
{
	if(!_Playing)
		return;
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	
	for (uint i=0; i<BACKGROUND_LAYER; ++i)
	{
		// stop all playing source
		std::vector<TSoundData>::iterator first(_Layers[i].begin()), last(_Layers[i].end());
		for (; first != last; ++first)
		{
			if (first->Source != 0 && first->Source->isPlaying())
				first->Source->stop();
		}
	}

	CAudioMixerUser::instance()->unregisterUpdate(this);

	_Playing = false;
}

void CBackgroundSoundManager::unload ()
{
	stop();

	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	
	for (uint i=0; i<BACKGROUND_LAYER; ++i)
	{
		// delete all created source
		std::vector<TSoundData>::iterator first(_Layers[i].begin()), last(_Layers[i].end());
		for (; first != last; ++first)
		{
			if (first->Source)
//				mixer->removeSource(first->Source);
				delete first->Source;
		}

		// and free the layer.
		_Layers[i].clear();
	}
}

void CBackgroundSoundManager::setListenerPosition (const CVector &listenerPosition)
{
	if (_LastPosition == listenerPosition)
	{
		return;
	}
	_LastPosition = listenerPosition;

	updateBackgroundStatus();
}

void CBackgroundSoundManager::updateBackgroundStatus()
{
	if (!_Playing)
		return;

	CAudioMixerUser *mixer = CAudioMixerUser::instance();


	// it s on 2d so we don't have z
	CVector listener = _LastPosition;
	listener.z = 0.0f;

	// special case for clustered sound management. If the listener is not
	// in the global cluster, it's background listening place could be different
	CClusteredSound *clusteredSound = mixer->getClusteredSound();
	if (clusteredSound != 0)
	{
		const CClusteredSound::CClusterSoundStatus *css = clusteredSound->getClusterSoundStatus(clusteredSound->getRootCluster());
		if (css != 0)
		{
			listener = css->Position;
			listener.z = 0.0f;
		}
	}


	// compute the list of load/unload banks.
	{
		// set of bank that must be in ram.
		std::set<std::string>	newBanks;

		std::vector<TBanksData>::iterator first(_Banks.begin()), last(_Banks.end());
		for (; first != last; ++first)
		{
			if (listener.x >= first->MinBox.x && listener.x <= first->MaxBox.x
				&& listener.y >= first->MinBox.y && listener.y <= first->MaxBox.y
				)
			{
				// bounding box ok, 
				if (CPrimZone::contains(listener, first->Points))
				{
					// add the banks of this zone in the n
					newBanks.insert(first->Banks.begin(), first->Banks.end());
				}
			}
		}
		// ok, now compute to set : the set of bank to load, and the set of banks to unload.
		std::set<std::string>	noChange;
		std::set_intersection(_LoadedBanks.begin(), _LoadedBanks.end(), newBanks.begin(), newBanks.end(), std::inserter(noChange, noChange.end()));

		std::set<std::string>	loadList; 
		std::set_difference(newBanks.begin(), newBanks.end(), noChange.begin(), noChange.end(), std::inserter(loadList, loadList.end()));

		std::set<std::string>	unloadList;
		std::set_difference(_LoadedBanks.begin(), _LoadedBanks.end(), newBanks.begin(), newBanks.end(), std::inserter(unloadList, unloadList.end()));

		// and now, load and unload....
		{
			std::set<std::string>::iterator first(loadList.begin()), last(loadList.end());
			for (; first != last; ++first)
			{
				mixer->loadSampleBank(true, *first);
			}
			_LoadedBanks.insert(loadList.begin(), loadList.end());
		}
		{
			std::set<std::string>::iterator first(unloadList.begin()), last(unloadList.end());
			for (; first != last; ++first)
			{
				if (mixer->unloadSampleBank(*first))
				{
					// ok, the bank is unloaded
					_LoadedBanks.erase(*first);
				}
			}
		}
	}

	// retreive the root cluster...
	NL3D::CCluster *rootCluster = 0;
	if (mixer->getClusteredSound() != 0)
		rootCluster = mixer->getClusteredSound()->getRootCluster();

	// Apply the same algo for each sound layer.
	for (uint i=0; i<BACKGROUND_LAYER; ++i)
	{
		vector<TSoundData> &layer = _Layers[i];
		vector<uint> selectedIndex;
		vector<uint> leaveIndex;

		selectedIndex.reserve(layer.size());
		leaveIndex.reserve(layer.size());

		// extract the list of selected/unselected box
		vector<TSoundData>::iterator first(layer.begin()), last(layer.end());
		for (uint count = 0; first != last; ++first, ++count)
		{
			if (listener.x >= first->MinBox.x && listener.x <= first->MaxBox.x
				&& listener.y >= first->MinBox.y && listener.y <= first->MaxBox.y
//				&& listener.z >= first->MinBox.z && listener.z <= first->MaxBox.z
				)
			{
//				nldebug("patat %u is selected by box (%s)", count, first->SoundName.c_str());
				selectedIndex.push_back(count);
			}
			else
			{
//				nldebug("patat %u is rejected  by box (%s)", count, first->SoundName.c_str());
				// listener out of this box.
				if (first->Selected && first->Source != 0)
				{
					// we leave this box.
					leaveIndex.push_back(count);
				}
			}
		}

		// stop all the sound that are leaved.
		{
			vector<uint>::iterator first(leaveIndex.begin()), last(leaveIndex.end());
			for (; first != last; ++first)
			{
				TSoundData &sd = layer[*first];
				sd.Selected = false;
				if (sd.Source->isPlaying())
					sd.Source->stop();
			}
		}
		// Compute new source mixing in this layer
		{
			/// Status of all selected sound ordered by surface.
			map<float, TSoundStatus>	status;

			// first loop to compute selected sound gain and position and order the result by surface..
			{
				vector<uint>::iterator first(selectedIndex.begin()), last(selectedIndex.end());
				for (; first != last; ++first)
				{
					TSoundData &sd = layer[*first];
					CVector pos;
					float	gain = 1.0f;
					float	distance;

					// inside the patat ?
					
					if(CPrimZone::contains(listener, sd.Points, distance, pos, sd.IsPath))
					{
						pos = _LastPosition;	// use the real listener position, not the 0 z centered
						gain = 1.0f;
//						nlinfo ("inside patate %d name '%s' ", *first, sd.SoundName.c_str());
					}
					else
					{
						if (distance < sd.MaxDist)
						{
							// compute the gain.
//							gain = (sd.MaxDist - distance) / sd.MaxDist;
						}
						else
						{
							// too far
							gain = 0;
						}
						//nlinfo ("near patate %d name '%s' from %f ", *first, sd.SoundName.c_str(), distance);
					}

					// store the status.
					status.insert(make_pair(sd.Surface, TSoundStatus(sd, pos, gain, distance)));
				}
			}
			// second loop thrue the surface ordered selected sound.
			{
				// Sound mixing strategie :
				// The smallest zone sound mask bigger one

				float	maskFactor = 1.0f;

				map<float, TSoundStatus>::iterator first(status.begin()), last(status.end());
				for (; first != last; ++first)
				{
					TSoundStatus &ss = first->second;

					if (maskFactor > 0.0f && ss.Gain > 0)
					{
						float gain = maskFactor * ss.Gain;
//						maskFactor -= ss.Gain;

						ss.SoundData.Selected = true;

//						if (ss.Gain == 1)
						if (ss.Distance == 0)
						{
							// inside a pattate, then decrease the mask factor will we are more inside the patate
							maskFactor -= first->second.Distance / INSIDE_FALLOF;
							clamp(maskFactor, 0.0f, 1.0f);
						}

						// start the soond (if needed) and update the volume.

						if (ss.SoundData.Source == 0)
						{
							// try to create the source.
							ss.SoundData.Source = static_cast<CSourceCommon*>(mixer->createSource(ss.SoundData.Sound, false, 0, 0, rootCluster));
						}
						if (ss.SoundData.Source != 0)
						{
							// set the volume
							ss.SoundData.Source->setRelativeGain(gain);
							// and the position
							ss.Position.z += 5.0f;
							ss.SoundData.Source->setPos(ss.Position);

//							nldebug("Setting source %s at %f", ss.SoundData.SoundName.c_str(), gain);
							if (!ss.SoundData.Source->isPlaying())
							{
								// start the sound is needed.
								ss.SoundData.Source->play();
							}
						}
					}
					else if (ss.SoundData.Source != 0 && ss.SoundData.Source->isPlaying())
					{
						// stop this too far source.
						ss.SoundData.Source->stop();
					}
				}
			} 
		} // compute source mixing
	} // for each layer

	
	// update the fade in / out
	if (_DoFade)
	{
		TTime now = NLMISC::CTime::getLocalTime();
		_DoFade = false;
		uint i;

		//for each filter
		for (i=0; i< UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
		{
			if (_FilterFadesStart[i] != 0)
			{
				// this filter is fading
				if (_BackgroundFlags.Flags[i])
				{
					// fading out
					TTime delta = now - _FilterFadesStart[i];
					if (delta > _BackgroundFilterFades.FadeOuts[i])
					{
						// the fade is terminated
						_FilterFadeValues[i] = 0;
						// stop the fade for this filter
						_FilterFadesStart[i] = 0;
					}
					else
					{
						_FilterFadeValues[i] = 1 - (float(delta) / _BackgroundFilterFades.FadeOuts[i]);
						// continue to fade (at least for this filter.
						_DoFade |= true;
					}
				}
				else
				{
					// fading in
					TTime delta = now - _FilterFadesStart[i];
					if (delta > _BackgroundFilterFades.FadeIns[i])
					{
						// the fade is terminated
						_FilterFadeValues[i] = 1;
						// stop the fade for this filter
						_FilterFadesStart[i] = 0;
					}
					else
					{
						_FilterFadeValues[i] = float(delta) / _BackgroundFilterFades.FadeIns[i];
						// continue to fade (at least for this filter.
						_DoFade |= true;
					}
				}
			}
		}

		// update all playing background source that filter value has changed
		// for each layer
		for (i=0; i<BACKGROUND_LAYER; ++i)
		{
			// for each patat
			std::vector<TSoundData>::iterator first(_Layers[i].begin()), last(_Layers[i].end());
			for (; first != last; ++first)
			{
				if (first->Selected)
				{
					// update this playing sound
					if (first->Source != 0 && first->Source->getType() == CSourceCommon::SOURCE_BACKGROUND)
						static_cast<CBackgroundSource*>(first->Source)->updateFilterValues(_FilterFadeValues);
				}
			}

		}

		if (!_DoFade)
		{
			// we can remove the update.
			mixer->unregisterUpdate(this);
		}
	}
}

void CBackgroundSoundManager::setBackgroundFlags(const UAudioMixer::TBackgroundFlags &backgroundFlags)
{
	for (uint i=0; i<UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
	{
		if (_BackgroundFlags.Flags[i] != backgroundFlags.Flags[i])
		{
			// the filter flags has changed ! 
			if (backgroundFlags.Flags[i])
			{
				// the filter is activated, to a fade out
				_FilterFadesStart[i] = uint64(NLMISC::CTime::getLocalTime() - (1-_FilterFadeValues[i]) * _BackgroundFilterFades.FadeOuts[i]);
				_DoFade = true;
			}
			else
			{
				// the filter is cleared, do a fade in
				_FilterFadesStart[i] = uint64(NLMISC::CTime::getLocalTime() - (_FilterFadeValues[i]) * _BackgroundFilterFades.FadeIns[i]);
				_DoFade = true;
			}
		}

		_BackgroundFlags.Flags[i] = backgroundFlags.Flags[i];
	}

	if (_DoFade)
		CAudioMixerUser::instance()->registerUpdate(this);
}


void CBackgroundSoundManager::onUpdate()
{
	updateBackgroundStatus();
}


/*
void CBackgroundSoundManager::update ()
{

}
*/
/*
uint32 CBackgroundSoundManager::getZoneNumber ()
{
//	return BackgroundSounds.size();
	return 0;
}
*/
/*
const vector<CVector> &CBackgroundSoundManager::getZone(uint32 zone)
{
//	nlassert (zone< BackgroundSounds.size());
//	return BackgroundSounds[zone].Points;
	static vector<CVector> v;
	return v;
}
*/
CVector CBackgroundSoundManager::getZoneSourcePos(uint32 zone)
{
/*	nlassert (zone< BackgroundSounds.size());
	CVector pos;
	if (BackgroundSounds[zone].SourceDay != NULL)
		BackgroundSounds[zone].SourceDay->getPos(pos);
	return pos;
*/
	return CVector();
}


/*
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
				BackgroundSounds[i].SourceDay->setRelativeGain(1.0f);

				if (!BackgroundSounds[i].SourceDay->isPlaying())
					BackgroundSounds[i].SourceDay->play();
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
				BackgroundSounds[i].SourceNight->setRelativeGain(1.0f);

				if (!BackgroundSounds[i].SourceNight->isPlaying())
					BackgroundSounds[i].SourceNight->play();
			}
		}
		else
		{
			if(BackgroundSounds[i].SourceDay != NULL)
			{
				BackgroundSounds[i].SourceDay->setRelativeGain((1.0f-ratio));

				if (!BackgroundSounds[i].SourceDay->isPlaying())
					BackgroundSounds[i].SourceDay->play();
			}

			if(BackgroundSounds[i].SourceNight != NULL)
			{
				BackgroundSounds[i].SourceNight->setRelativeGain(ratio);

				if (!BackgroundSounds[i].SourceNight->isPlaying())
					BackgroundSounds[i].SourceNight->play();
			}
		}
	}

}
*/
} // NLSOUND

