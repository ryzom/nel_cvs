/** \file background_sound_manager.cpp
 * CBackgroundSoundManager
 *
 * $Id: background_sound_manager.cpp,v 1.15.2.2 2003/05/28 13:43:17 boucher Exp $
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
#include "3d/cluster.h"

#include "nel/sound/u_source.h"
#include "clustered_sound.h"
#include "sample_bank.h"

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
: _LastPosition(0,0,0), _Playing(false), _DoFade(false)
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


void CBackgroundSoundManager::addSound(const std::string &soundName, uint layerId, const std::vector<NLLIGO::CPrimVector> &points, bool isPath)
{
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	TSoundData	sd;

	sd.SoundName = CStringMapper::map(soundName);
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
		nlwarning ("The sound '%s' can't be loaded", CStringMapper::unmap(sd.SoundName).c_str());
	}
}

void CBackgroundSoundManager::addSound(const std::string &rawSoundName, const std::vector<NLLIGO::CPrimVector> &points, bool isPath)
{
	uint layerId = 0;
	uint n = 0;
	string name;
	// count the number of '-' in the string.
	n = std::count(rawSoundName.begin(), rawSoundName.end(), '-');

	if (n == 2)
	{
		// no layer spec, default to layer A
		uint32 pos1 = rawSoundName.find ("-");
		if(pos1 == string::npos)
		{
			nlwarning ("zone have the malformated name '%s' missing -name-", rawSoundName.c_str());
			return;
		}
		pos1++;

		uint32 pos2 = rawSoundName.find ("-", pos1);
		if(pos2 == string::npos)
		{
			nlwarning ("zone have the malformated name '%s' missing -name-", rawSoundName.c_str());
			return;
		}

		name = rawSoundName.substr(pos1, pos2-pos1);
	}
	else if (n == 3)
	{
		// layer spec !
		uint32 pos1 = rawSoundName.find ("-");
		uint32 pos2 = rawSoundName.find ("-", pos1+1);
		if(pos1 == string::npos || pos2 == string::npos)
		{
			nlwarning ("zone have the malformated name '%s' missing -layerId- or -name-", rawSoundName.c_str());
			return;
		}
		pos1++;

		uint32 pos3 = rawSoundName.find ("-", pos2+1);
		if(pos3 == string::npos)
		{
			nlwarning ("zone have the malformated name '%s' missing -name-", rawSoundName.c_str());
			return;
		}

		char id = rawSoundName[pos1];

		// check caps
		if (id < 'a')
			id = id + ('a' - 'A');

		layerId = id - 'a';

		NLMISC::clamp(layerId, 0u, BACKGROUND_LAYER-1);
		pos2++;

		name = rawSoundName.substr(pos2, pos3-pos2);
	}
	else
	{
		nlwarning ("zone have the malformated name '%s",  rawSoundName.c_str());
		return;
	}
		
	addSound(name, layerId, points, isPath);
/*
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
*/
}


void CBackgroundSoundManager::loadAudioFromPrimitives(const NLLIGO::IPrimitive &audioRoot)
{
	std::string className;
	if(audioRoot.getPropertyByName("class", className))
	{
		if (className == "audio")
		{
			// ok, it a root of the audio primitives
			
			// remember playing state
			bool oldState = _Playing;
			unload();

			for (uint i=0; i<audioRoot.getNumChildren(); ++i)
			{
				const NLLIGO::IPrimitive *child;

				audioRoot.getChild(child, i);

				if (child->getPropertyByName("class", className))
				{
					if (className == "sounds")
					{
						loadSoundsFromPrimitives(*child);
					}
					else if (className == "sample_banks")
					{
						loadSamplesFromPrimitives(*child);
					}
					else if (className == "env_fx")
					{
						loadEffectsFromPrimitives(*child);
					}
				}
			}

			if (oldState)
				play();
		}
	}
	else
	{
		// try to look in the first child level
		for (uint i=0; i<audioRoot.getNumChildren(); ++i)
		{
			const NLLIGO::IPrimitive *child;
			audioRoot.getChild(child, i);

			if (child->getPropertyByName("class", className))
			{
				if (className == "audio")
				{
					// recurse in this node
					loadAudioFromPrimitives(*child);
					// don't look any other primitives
					break;
				}
			}
		}
	}
}

void CBackgroundSoundManager::loadSoundsFromPrimitives(const NLLIGO::IPrimitive &soundRoot)
{
	std::string className;
	if (soundRoot.getPropertyByName("class", className))
	{
		if (className == "sounds" || className == "sound_folder")
		{
			// ok, it sounds or a sounds foilder
			for (uint i=0; i<soundRoot.getNumChildren(); ++i)
			{
				const NLLIGO::IPrimitive *child;
				std::string primName;
				soundRoot.getChild(child, i);


				if (child->getPropertyByName("class", className))
				{
					uint layerId = 0;
					std::string layerString;
					if (child->getPropertyByName("layer", layerString))
					{
						// extract layer number.
						if (!layerString.empty())
						{
							// TODO : handle special case for weather layer
							layerId = layerString[layerString.size()-1] - '0';
						}
						clamp(layerId, 0u, BACKGROUND_LAYER-1);
					}

					child->getPropertyByName("name", primName);
					if (className == "sound_zone")
					{
						if(child->getNumVector()>2)
						{
							addSound(primName, layerId, static_cast<const CPrimZone*>(child)->VPoints, false);
						}
						else
						{
							nlwarning ("A background sound patatoid have less than 3 points '%s'", primName.c_str());
						}
					}
					else if (className == "sound_path")
					{
						if(child->getNumVector() > 1)
						{
							addSound(primName, layerId, static_cast<const CPrimPath*>(child)->VPoints, true);
						}
						else
						{
							nlwarning ("A background sound path have less than 2 points '%s'", primName.c_str());
						}
					}
					else if (className == "sound_point")
					{
						std::vector<NLLIGO::CPrimVector>	points;
						points.push_back(static_cast<const CPrimPoint*>(child)->Point);

						addSound(primName, layerId, points, false);
					}
					else if (className == "sound_folder")
					{
						loadSoundsFromPrimitives(*child);
					}
				}
			}
		}
	}
}

void CBackgroundSoundManager::loadSamplesFromPrimitives(const NLLIGO::IPrimitive &sampleRoot)
{
	std::string className;
	_Banks.clear();
	if (sampleRoot.getPropertyByName("class", className))
	{
		if (className == "sample_banks")
		{
			for (uint i=0; i<sampleRoot.getNumChildren(); ++i)
			{
				const NLLIGO::IPrimitive *child;
				std::string primName;
				sampleRoot.getChild(child, i);

				if (child->getPropertyByName("class", className))
				{
					child->getPropertyByName("name", primName);
					if (className == "sample_bank_zone")
					{
						const std::vector<std::string> *names;
						if (child->getPropertyByName("bank_names", names))
						{
							addSampleBank(*names, static_cast<const CPrimZone*>(child)->VPoints);
						}
					}
				}
			}
		}
	}
}

void CBackgroundSoundManager::loadEffectsFromPrimitives(const NLLIGO::IPrimitive &fxRoot)
{
	std::string className;
	_FxZones.clear();

	if (fxRoot.getPropertyByName("class", className))
	{
		if (className == "env_fx")
		{
			for (uint i=0; i<fxRoot.getNumChildren(); ++i)
			{
				const NLLIGO::IPrimitive *child;
				std::string primName;
				fxRoot.getChild(child, i);

				if (child->getPropertyByName("class", className))
				{
					child->getPropertyByName("name", primName);
					if (className == "env_fx_zone")
					{
						std::string fxName;
						if (child->getPropertyByName("fx_name", fxName))
						{
							addFxZone(fxName, static_cast<const CPrimZone*>(child)->VPoints);
						}
					}
				}
			}
		}
	}
}

void CBackgroundSoundManager::addFxZone(const std::string &fxName, const std::vector<NLLIGO::CPrimVector> &points)
{
	TFxZone	fxZone;

	fxZone.FxName = CStringMapper::map(fxName);
	fxZone.Points.resize (points.size());
	for (uint j=0; j<points.size(); j++)
	{
		fxZone.Points[j] = points[j];
	}

	// compute bouding box.
	CVector	vmin(FLT_MAX, FLT_MAX, 0), vmax(-FLT_MAX, -FLT_MAX, 0);

	vector<CVector>::iterator first(fxZone.Points.begin()), last(fxZone.Points.end());
	for (; first != last; ++first)
	{
		vmin.x = min(first->x, vmin.x);
		vmin.y = min(first->y, vmin.y);
		vmax.x = max(first->x, vmax.x);
		vmax.y = max(first->y, vmax.y);
	}
	fxZone.MaxBox = vmax;
	fxZone.MinBox = vmin;

	_FxZones.push_back(fxZone);
}


void CBackgroundSoundManager::addSampleBank(const std::vector<std::string> &bankNames, const std::vector<CPrimVector> &points)
{
	TBanksData	bd;
//	uint pointCount = points.size ();
	bd.Points.resize (points.size());
	for (uint j=0; j<points.size(); j++)
	{
		bd.Points[j] = points[j];
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

	for(uint i=0; i<bankNames.size(); ++i)
	{
		if (!bankNames[i].empty())
			bd.Banks.push_back(bankNames[i]);
	}

	// ok, store it in the container.
	_Banks.push_back(bd);
}


void CBackgroundSoundManager::loadSamplesFromRegion(const NLLIGO::CPrimRegion &region)
{
	_Banks.clear();

	for (uint i=0; i< region.VZones.size(); ++i)
	{
		if (region.VZones[i].VPoints.size() > 2)
		{
			// parse the zone name to find the samples name.
			std::vector<std::string>	splitted = split(region.VZones[i].Name, '-');
			std::vector<std::string>	bankNames;

			if (splitted.size() > 2)
			{
				for (uint j=1; j<splitted.size()-1; ++j)
				{
					bankNames.push_back(splitted[j]);
				}

				addSampleBank(bankNames, region.VZones[i].VPoints);
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
	NL_ALLOC_CONTEXT(NLSOUND_CBackgroundSoundManager);
	// First, try to load from a .primitive file (contain everythink)
	{
		CIFile file;
//		CPrimRegion region;
		CPrimitives primitives;
		primitives.RootNode = new CPrimNode;
		string fn = continent+"_audio.primitive";

		nlinfo ("loading '%s'", fn.c_str());

		string path = CPath::lookup(fn, false);

		if(!path.empty() && file.open (path))
		{
			CIXml xml;
			xml.init (file);

			primitives.read(xml.getRootNode(), fn.c_str());
//			region.serial(xml);
			file.close ();

			loadAudioFromPrimitives(*primitives.RootNode);

			////////////////////////////////////////////////
			// Jobs done !
			return;
		}

	}

	// We reach this only if the new .primitive file format is not found
	// then, we try to load separate .prim file for sound, samples and fx

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

	// erase the sample banks zone
	_Banks.clear();

	// TODO : erase the fx zones
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

	// evalutate the current env fx
	{
		NL3D::CCluster *rootCluster = 0;
		if (mixer->getClusteredSound())
			rootCluster = mixer->getClusteredSound()->getRootCluster();

		std::vector<TFxZone>::iterator first(_FxZones.begin()), last(_FxZones.end());
		for (; first != last; ++first)
		{
			if (listener.x >= first->MinBox.x && listener.x <= first->MaxBox.x
				&& listener.y >= first->MinBox.y && listener.y <= first->MaxBox.y
				)
			{
				// bounding box ok, 
				if (CPrimZone::contains(listener, first->Points))
				{
					// stop at the first zone !
					if (rootCluster)
					{
						// use the cluster system
						rootCluster->setEnvironmentFx(first->FxName);
					}
					else
					{
						// no cluster system, set the env 'manualy'
						IListener *drvListener = static_cast<CListenerUser*>(mixer->getListener())->getListener();

						if (_LastEnv != first->FxName)
						{
							_LastEnv = first->FxName;
							uint envNum;
							if (first->FxName == CStringMapper::map("GENERIC"))
								envNum = 0;
							else if (first->FxName == CStringMapper::map("PADDEDCELL"))
								envNum = 1;
							else if (first->FxName == CStringMapper::map("ROOM"))
								envNum = 2;
							else if (first->FxName == CStringMapper::map("BATHROOM"))
								envNum = 3;
							else if (first->FxName == CStringMapper::map("LIVINGROOM"))
								envNum = 4;
							else if (first->FxName == CStringMapper::map("STONEROOM"))
								envNum = 5;
							else if (first->FxName == CStringMapper::map("AUDITORIUM"))
								envNum = 6;
							else if (first->FxName == CStringMapper::map("CONCERTHALL"))
								envNum = 7;
							else if (first->FxName == CStringMapper::map("CAVE"))
								envNum = 8;
							else if (first->FxName == CStringMapper::map("ARENA"))
								envNum = 9;
							else if (first->FxName == CStringMapper::map("HANGAR"))
								envNum = 10;
							else if (first->FxName == CStringMapper::map("CARPETEDHALLWAY"))
								envNum = 11;
							else if (first->FxName == CStringMapper::map("HALLWAY"))
								envNum = 12;
							else if (first->FxName == CStringMapper::map("STONECORRIDOR"))
								envNum = 13;
							else if (first->FxName == CStringMapper::map("ALLEY"))
								envNum = 14;
							else if (first->FxName == CStringMapper::map("FOREST"))
								envNum = 15;
							else if (first->FxName == CStringMapper::map("CITY"))
								envNum = 16;
							else if (first->FxName == CStringMapper::map("MOUNTAINS"))
								envNum = 16;
							else if (first->FxName == CStringMapper::map("QUARRY"))
								envNum = 17;
							else if (first->FxName == CStringMapper::map("PLAIN"))
								envNum = 18;
							else if (first->FxName == CStringMapper::map("PARKINGLOT"))
								envNum = 19;
							else if (first->FxName == CStringMapper::map("SEWERPIPE"))
								envNum = 20;
							else if (first->FxName == CStringMapper::map("UNDERWATER"))
								envNum = 21;
							else if (first->FxName == CStringMapper::map("DRUGGED"))
								envNum = 22;
							else if (first->FxName == CStringMapper::map("DIZZY"))
								envNum = 23;
							else if (first->FxName == CStringMapper::map("PSYCHOTIC"))
								envNum = 24;
							else
								envNum = 18;

							drvListener->setEnvironment(envNum, 10);
						}
					}
					break;
				}
			}
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

/*		{
			nldebug("-----------------------------");
			nldebug("Loaded sample banks (%u elements):", _LoadedBanks.size());
			set<string>::iterator first(_LoadedBanks.begin()), last(_LoadedBanks.end());
			for (; first != last; ++first)
			{
				const string &str = *first;
				nldebug("  %s", first->c_str());
			}
		}
		{
			nldebug("New Sample bank list (%u elements):", newBanks.size());
			set<string>::iterator first(newBanks.begin()), last(newBanks.end());
			for (; first != last; ++first)
			{
				const string &str = *first;
				nldebug("  %s", first->c_str());
			}
		}
*/
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
//				nldebug("Trying to load sample bank %s", first->c_str());
				mixer->loadSampleBank(true, *first);
			}
			_LoadedBanks.insert(loadList.begin(), loadList.end());
		}
		{
			std::set<std::string>::iterator first(unloadList.begin()), last(unloadList.end());
			for (; first != last; ++first)
			{
//				nldebug("Trying to unload sample bank %s", first->c_str());
				if (mixer->unloadSampleBank(*first))
				{
					// ok, the bank is unloaded
					_LoadedBanks.erase(*first);
				}
				else if (CSampleBank::findSampleBank(CStringMapper::map(*first)) == 0)
				{
					// ok, the bank is unavailable !
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
							ss.Position.z = _LastPosition.z + 5.0f;
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

