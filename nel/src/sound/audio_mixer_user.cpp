/** \file audio_mixer_user.cpp
 * CAudioMixerUser: implementation of UAudioMixer
 *
 * $Id: audio_mixer_user.cpp,v 1.43 2003/03/24 17:09:25 boucher Exp $
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

//#include "env_sound_user.h"
//#include "env_effect.h"
#include "simple_sound.h"
#include "complex_sound.h"
//#include "ambiant_source.h"
//#include "bounding_sphere.h"
//#include "bounding_box.h"
#include "driver/buffer.h"
#include "sample_bank.h"
#include "sound_bank.h"
#include "background_sound_manager.h"
#include "simple_source.h"
#include "complex_source.h"
#include "background_source.h"
#include "clustered_sound.h"
#include "background_sound_manager.h"

#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"


#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"
#include "nel/misc/progress_callback.h"
#include "3d/scene_user.h"
#include "driver/sound_driver.h"

#include "context_sound.h"
#include <iomanip.h>

#if EAX_AVAILABLE == 1
# include <eax.h>
#endif



using namespace NLMISC;

using namespace std;


namespace NLSOUND {


#ifdef _DEBUG
CAudioMixerUser::IMixerEvent	*CurrentEvent = 0;
#endif


#define NL_TRACE_MIXER 0

#if NL_TRACE_MIXER
#define _profile(_a) nldebug ## _a
#else
#define _profile(_a) 
#endif

// The audio mixer singleton instance
CAudioMixerUser		*CAudioMixerUser::_Instance = NULL;

// Return the priority cstring (debug info)
const char *PriToCStr [NbSoundPriorities] = { "XH", "HI", "MD", "LO" };

const std::string	CAudioMixerUser::SampleBankListExt(".sample_bank_list");



// ******************************************************************

const char *getPriorityStr( TSoundPriority p )
{
	nlassert( ((uint)p) < NbSoundPriorities );
	return PriToCStr[p];
}


// ******************************************************************

UAudioMixer	*UAudioMixer::createAudioMixer()
{
	return new CAudioMixerUser();
}


// ******************************************************************

CAudioMixerUser::CAudioMixerUser() : _SoundDriver(NULL),
									 _ListenPosition(CVector::Null),
									 _NbTracks(0),
									 _MaxNbTracks(0),
									 _Leaving(false),
									 _BackgroundSoundManager(0),
									 _PlayingSources(0),
									 _ClusteredSound(0),
									_PackedSheetPath("")
{
	if ( _Instance == NULL )
	{
		_Instance = this;

#if NL_PROFILE_MIXER
		_UpdateTime = 0.0;
		_CreateTime = 0.0;
		_UpdateCount = 0;
		_CreateCount = 0;
#endif

	}
	else
	{
		nlerror( "Audio mixer singleton instanciated twice" );
	}


	// init the filter names and short names
	for (uint i=0; i<TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
	{
		char tmp[1024];
		sprintf(tmp, "Filter%2u", i);
		_BackgroundFilterNames[i] = tmp;
		sprintf(tmp, "%u", i);
		_BackgroundFilterShortNames[i] = tmp;
	}
}


// ******************************************************************

CAudioMixerUser::~CAudioMixerUser()
{
	nldebug( "AM: Releasing..." );

	if (_ClusteredSound != 0)
		delete _ClusteredSound;

	if (_BackgroundSoundManager != 0)
		delete _BackgroundSoundManager;
//	CBackgroundSoundManager::release();

	reset();

	_Leaving = true;

	// Release the sound bank
	CSoundBank::release();
	// Release all the SampleBanks
	CSampleBank::releaseAll();

	// Tracks
	uint i;
	for ( i=0; i!=_NbTracks; i++ )
	{
		if ( _Tracks[i] )
			delete _Tracks[i];
	}

	// Sound driver
	if ( _SoundDriver != NULL )
		delete _SoundDriver;

	_Instance = NULL;

	nldebug( "AM: Released" );
}


void CAudioMixerUser::initClusteredSound(NL3D::UScene *uscene, float minGain, float maxDistance, float portalInterpolate)
{
	NL3D::CScene *scene = 0;
	if (uscene != 0)
		scene = &(static_cast<NL3D::CSceneUser*>(uscene)->getScene());

	initClusteredSound(scene, minGain, maxDistance, portalInterpolate);
}

void CAudioMixerUser::initClusteredSound(NL3D::CScene *scene, float minGain, float maxDistance, float portalInterpolate = 20.0f)
{
	if (_ClusteredSound == 0)
		_ClusteredSound = new CClusteredSound;

	_ClusteredSound->init(scene, portalInterpolate, maxDistance, minGain);
}


void CAudioMixerUser::setPriorityReserve(TSoundPriority priorityChannel, uint reserve)
{
	_PriorityReserve[priorityChannel] = min(_NbTracks, reserve);
}

void CAudioMixerUser::setLowWaterMark(uint value)
{
	_LowWaterMark = min(_NbTracks, value);
}


// ******************************************************************

void				CAudioMixerUser::writeProfile(std::ostream& out)
{
	// compute number of muted source
	uint nb = 0;	
	
/*	TSourceContainer::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		CSimpleSource *psu = *first;
		if (psu->getTrack() == NULL)
		{
			++nb;
		}
	}
*/
/*	hash_set<CSimpleSource*>::const_iterator ips;
	for ( ips=_Sources.begin(); ips!=_Sources.end(); ++ips )
	{
		CSimpleSource *psu = *ips;
		if (psu->getTrack() == NULL)
		{
			++nb;
		}
	}
*/
	out << "Mixer: \n";
	out << "Playing sources: " << getPlayingSourcesNumber() << " \n";
	out << "Available tracks: " << getNumberAvailableTracks() << " \n";
//	out << "Muted sources: " << nb << " \n";
//	out << "Muted sources: " << max(0, sint(_PlayingSources.size())-sint(_NbTracks)) << " \n";
	out << "Muted sources: " << max(0, sint(_PlayingSources)-sint(_NbTracks)) << " \n";
	out << "Sources waiting for play: " << _SourceWaitingForPlay.size() << " \n";
	out << "HighestPri: " << _ReserveUsage[HighestPri] << " / " << _PriorityReserve[HighestPri] << " \n";
	out << "HighPri:    " << _ReserveUsage[HighPri] << " / " << _PriorityReserve[HighPri] << "\n";
	out << "MidPri:     " << _ReserveUsage[MidPri] << " / " << _PriorityReserve[MidPri] << " \n";
	out << "LowPri:     " << _ReserveUsage[LowPri] << " / " << _PriorityReserve[LowPri] << " \n";
	out << "FreeTracks: " << _FreeTracks.size() << " / " << _NbTracks << " \n";
	out << "Average update time: " << std::setw(10) << (1000.0 * _UpdateTime / _UpdateCount) << " msec\n";
	out << "Average create time: " << std::setw(10) <<(1000.0 * _CreateTime / _CreateCount) << " msec\n";
	out << "Estimated CPU: " << std::setiosflags(ios::right) << std::setprecision(6) << std::setw(10) << (100.0 * 1000.0 * (_UpdateTime + _CreateTime) / curTime()) << "%\n";

	if (_SoundDriver)
	{
		out << "\n";
		out << "Driver: \n";
		_SoundDriver->writeProfile(out);
	}
}

// ******************************************************************

void	CAudioMixerUser::addSourceWaitingForPlay(CSimpleSource *source)
{
	_SourceWaitingForPlay.push_back(source);
}


// ******************************************************************

void				CAudioMixerUser::reset()
{
	_Leaving = true;

	_SourceWaitingForPlay.clear();

	// Stop tracks
	uint i;
	for ( i=0; i!=_NbTracks; i++ )
	{
		if ( _Tracks[i] )
		{
			CSimpleSource* src = _Tracks[i]->getSource();

			if (src && src->isPlaying())
			{
				src->stop();
			}
		}
	}

	// Do a first multipass travesal to stop all playing source
	// We can't do the work in 1 pass because stoping a source can lead to
	// destruction of sub source, invalidating the iterators !
	bool again;
	do
	{
		again = false;
		TSourceContainer::iterator first(_Sources.begin()), last(_Sources.end());
		for (; first != last; ++first)
		{
			if ((*first)->isPlaying())
			{
				(*first)->stop();
				again = true;
				break;
			}
		}

	} while (again);

	// Sources
	while (!_Sources.empty())
	{
		//removeSource( _Sources.begin(), true ); // 3D sources, the envsounds were removed above
		CSourceCommon *source = *(_Sources.begin());
		if (source->isPlaying())
			source->stop();
		else
			delete source;
	}

	_Leaving = false;
}

void	CAudioMixerUser::setPackedSheetOption(const std::string &path, bool update)
{
	_PackedSheetPath = CPath::standardizePath(path, false);
	_UpdatePackedSheet = update;
}


// ******************************************************************

void				CAudioMixerUser::init(bool useEax, IProgressCallback *progressCallBack)
{
	nldebug( "AM: Init..." );

	_profile(( "AM: ---------------------------------------------------------------" ));
	_profile(( "AM: DRIVER: %s", NLSOUND_DLL_NAME ));
	
	// Init sound driver
	try
	{
		_SoundDriver = ISoundDriver::createDriver(useEax, this);
	}
	catch(...)
	{
		// TODO : is this logic to auto destruct this object in case of failing to create the driver ?
		delete this;
		_Instance = NULL;
		throw;
	}

	uint i;


	// Init registrable classes
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
	}

	// Init listener
	_Listener.init( _SoundDriver );

	// Init tracks (physical sources)
	_NbTracks = MAX_TRACKS; // could be chosen by the user, or according to the capabilities of the sound card
	for ( i=0; i<MAX_TRACKS; i++ )
	{
		_Tracks[i] = NULL;
	}
	try
	{
		for ( i=0; i!=_NbTracks; i++ )
		{
			_Tracks[i] = new CTrack();
			_Tracks[i]->init( _SoundDriver );
			// insert in front because the last inserted wan be sofware buffer...
			_FreeTracks.insert(_FreeTracks.begin(), _Tracks[i]);
		}
	}
	catch ( ESoundDriver & )
	{
		// If the source generation failed, keep only the generated number of sources
		_NbTracks = i;
	}

	_MaxNbTracks = _NbTracks;

	// Init the reserve stuff.
	_LowWaterMark = 0;
	for (i=0; i<NbSoundPriorities; ++i)
	{
		_PriorityReserve[i] = _NbTracks;
		_ReserveUsage[i] = 0;
	}
	
	_StartTime = CTime::getLocalTime();

	// if needed (update == true), build the sample bank list
	if (_UpdatePackedSheet)
	{
		// regenerate the sample banks list
		const std::string &sp = _SamplePath;

		std::vector <std::string> dirList;
		CPath::getPathContent(sp, false, true, false, dirList);

		while (!dirList.empty())
		{
			nldebug("Generating sample bank list for %s", dirList.back().c_str());
			std::vector<std::string> sampleList;
			CPath::getPathContent(dirList.back(), true, false, true, sampleList);

			for (uint i=0; i< sampleList.size(); ++i)
			{
				sampleList[i] = CFile::getFilename(sampleList[i]);
				nldebug("+- Adding sample %s to bank", sampleList[i].c_str());
			}

			std::vector<std::string> temp;
			NLMISC::explode(dirList.back(), "/", temp, true);
			nlassert(!temp.empty());
			std::string listName(temp.back());

			COFile file(_PackedSheetPath+listName+SampleBankListExt);
			file.serialCont(sampleList);
			dirList.pop_back();
		}
	}



	// Create the background sound manager.
	_BackgroundSoundManager = new CBackgroundSoundManager();

	// Load the sound bank singleton
	CSoundBank::instance()->load();
	nlinfo( "Initialized audio mixer with %u voices", _NbTracks );

	// try to load default configuration from george sheet
	
	NLGEORGES::UFormLoader *formLoader = NULL;

	try
	{
		std::string mixerConfigFile = NLMISC::CPath::lookup("default.mixer_config", false);
		if (!mixerConfigFile.empty())
		{
			formLoader = NLGEORGES::UFormLoader::createLoader();

			NLMISC::CSmartPtr<NLGEORGES::UForm> form;
			form = formLoader->loadForm(mixerConfigFile.c_str());

			NLGEORGES::UFormElm &root = form->getRootNode();


			// read track reserve
			uint32 highestRes, highRes, midRes, lowRes;
			root.getValueByName(highestRes, ".HighestPriorityReserve");
			root.getValueByName(highRes, ".HighPriorityReserve");
			root.getValueByName(midRes, ".MidPriorityReserve");
			root.getValueByName(lowRes, ".LowPriorityReserve");

			setPriorityReserve(HighestPri, highestRes);
			setPriorityReserve(HighPri, highRes);
			setPriorityReserve(MidPri, midRes);
			setPriorityReserve(LowPri, lowRes);

			uint32 lowWater;
			root.getValueByName(lowWater, ".LowWaterMark");
			setLowWaterMark(lowWater);

			// preload sample bank
			NLGEORGES::UFormElm *sampleBanks;
			root.getNodeByName(&sampleBanks, ".SampleBanks");

			if (sampleBanks != NULL)
			{
				uint size;
				sampleBanks->getArraySize(size);
				for (uint i=0; i<size; ++i)
				{
					std::string name;
					sampleBanks->getArrayValue(name, i);
					
					if (!name.empty())
						loadSampleBank(false, name);

					if (progressCallBack != 0)
						progressCallBack->progress(float(i) / size);
				}
			}

			// configure background flags names, fades and state
			NLGEORGES::UFormElm *bgFlags;
			root.getNodeByName(&bgFlags, ".BackgroundFlags");
			if (bgFlags != NULL)
			{
				TBackgroundFlags		flags;
				TBackgroundFilterFades	fades;

				uint size;
				bgFlags->getArraySize(size);
				uint i;
				for (i=0; i<min(size, uint(TBackgroundFlags::NB_BACKGROUND_FLAGS)); ++i)
				{
					NLGEORGES::UFormElm *flag;
					bgFlags->getArrayNode(&flag, i);

					flag->getValueByName(flags.Flags[i], ".InitialState");

					uint32 fadeIn, fadeOut;
					flag->getValueByName(fadeIn, ".FadeIn");
					flag->getValueByName(fadeOut, ".FadeOut");

					fades.FadeIns[i] = fadeIn;
					fades.FadeOuts[i] = fadeOut;

					flag->getValueByName(_BackgroundFilterNames[i], ".Name");
					flag->getValueByName(_BackgroundFilterShortNames[i], ".ShortName");
				}
				for (; i< TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
				{
					uint32 fadeIn, fadeOut;
					NLGEORGES::UFormElm::TWhereIsValue where = NLGEORGES::UFormElm::ValueDefaultDfn;
					root.getValueByName(fadeIn, ".BackgroundFlags[0].FadeIn", NLGEORGES::UFormElm::Eval, &where);
					root.getValueByName(fadeOut, ".BackgroundFlags[0].FadeOut", NLGEORGES::UFormElm::Eval, &where);
					root.getValueByName(flags.Flags[i], ".BackgroundFlags[0].InitialState", NLGEORGES::UFormElm::Eval, &where);

					fades.FadeIns[i] = fadeIn;
					fades.FadeOuts[i] = fadeOut;
				}
				setBackgroundFilterFades(fades);
				setBackgroundFlags(flags);
			}
		}
	}
	catch(...)
	{
		delete formLoader;
	}

	// init the user var bindings
	initUserVar();
}

void				CAudioMixerUser::setBackgroundFlagName(uint flagIndex, const std::string &flagName)
{
	if (flagIndex < TBackgroundFlags::NB_BACKGROUND_FLAGS)
		_BackgroundFilterNames[flagIndex] = flagName;
}
void				CAudioMixerUser::setBackgroundFlagShortName(uint flagIndex, const std::string &flagShortName)
{
	if (flagIndex < TBackgroundFlags::NB_BACKGROUND_FLAGS)
		_BackgroundFilterShortNames[flagIndex] = flagShortName;
}
const std::string	&CAudioMixerUser::getBackgroundFlagName(uint flagIndex)
{
	static std::string bad("");
	if (flagIndex < TBackgroundFlags::NB_BACKGROUND_FLAGS)
		return _BackgroundFilterNames[flagIndex];
	else
		return bad;
}
const std::string	&CAudioMixerUser::getBackgroundFlagShortName(uint flagIndex)
{
	static std::string bad("");
	if (flagIndex < TBackgroundFlags::NB_BACKGROUND_FLAGS)
		return _BackgroundFilterShortNames[flagIndex];
	else
		return bad;
}

const UAudioMixer::TBackgroundFlags		&CAudioMixerUser::getBackgroundFlags()
{
	return _BackgroundSoundManager->getBackgroundFlags();
}
const UAudioMixer::TBackgroundFilterFades &CAudioMixerUser::getBackgroundFilterFades()
{
	return _BackgroundSoundManager->getBackgroundFilterFades();
}


class CUserVarSerializer
{
public:
	std::vector<CAudioMixerUser::CControledSources>		Controlers;
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const std::string &name)
	{
		try
		{
			std::string varname, soundName, paramId;
			NLGEORGES::UFormElm &root = form->getRootNode();
			NLGEORGES::UFormElm *items;
			uint size;

			CAudioMixerUser::CControledSources	cs;

			// preset the default value
			cs.Value = 1.0f;

			root.getValueByName(varname, ".Name");
			root.getValueByName(paramId, ".ParamId");

			cs.Name = CStringMapper::map(varname);
			if (paramId == "Gain")
				cs.ParamId = CAudioMixerUser::gain_control;
			else if (paramId == "Pitch")
				cs.ParamId = CAudioMixerUser::pitch_control;
			else
				return;
		
			root.getNodeByName(&items, ".Sounds");
			items->getArraySize(size);

			for (uint i=0; i<size; ++i)
			{
				items->getArrayValue(soundName, i);
				soundName = soundName.substr(0, soundName.find(".sound"));

				cs.SoundNames.push_back(CStringMapper::map(soundName));
			}

			if (!cs.SoundNames.empty())
				Controlers.push_back(cs);
		}
		catch(...)
		{}
	}

	void serial (NLMISC::IStream &s)
	{
		s.serialCont(Controlers);
	}

	void removed()
	{}

	static uint getVersion () { return 2; }


};

void CAudioMixerUser::initUserVar()
{
	/// Temporary container.
	std::map<std::string, CUserVarSerializer> Container;

	// read all *.user_var_binding sheet in data/sound/user_var folder

	// load the sound_group sheets
	::loadForm("user_var_binding", _PackedSheetPath+"user_var_binding.packed_sheets", Container, _UpdatePackedSheet);
	// fill the real container.
	std::map<std::string, CUserVarSerializer>::iterator first(Container.begin()), last(Container.end());
	for (; first != last; ++first)
	{
		for (uint i=0; i<first->second.Controlers.size(); ++i)
		{
			_UserVarControls.insert(make_pair(first->second.Controlers[i].Name, first->second.Controlers[i]));
		}
	}

	// update all the sounds to refer to the controler.
	{
		TUserVarControlsContainer::iterator first(_UserVarControls.begin()), last(_UserVarControls.end());
		for(;  first != last; ++first)
		{
			std::vector<NLMISC::TStringId>::iterator first2(first->second.SoundNames.begin()), last2(first->second.SoundNames.end());
			for (; first2 != last2; ++first2)
			{
				CSound *sound = getSoundId(*first2);
				if (sound != 0)
				{
					// ok, the sound exist !
					sound->_UserVarControler = first->second.Name;
				}
			}
		}
	}
	
}

void CAudioMixerUser::CControledSources::serial (NLMISC::IStream &s)
{
	std::string name, soundName;
	if (s.isReading())
	{
		s.serial(name);
		Name = CStringMapper::map(name);
		s.serialEnum(ParamId);

		uint32 size;
		s.serial(size);
		for (uint i=0; i<size; ++i)
		{
			s.serial(soundName);
			SoundNames.push_back(CStringMapper::map(soundName));
		}
	}
	else
	{
		name = CStringMapper::unmap(Name);
		s.serial(name);
		s.serialEnum(ParamId);

		uint32 size = SoundNames.size();
		s.serial(size);

		for (uint i=0; i<size; ++i)
		{
			soundName = CStringMapper::unmap(SoundNames[i]);
			s.serial(soundName);
		}
	}

	// Default value to 1.
	Value = 1.0f;
}




void CAudioMixerUser::setUserVar(NLMISC::TStringId varName, float value)
{
	TUserVarControlsContainer::iterator it(_UserVarControls.find(varName));
	if (it != _UserVarControls.end())
	{
		// ok we found the var !
		// do some work only if the value is different (we don't trust client for
		// being smart ;) )
		if (it->second.Value != value)
		{
			it->second.Value = value;
			// update all sources
			std::set<CSourceCommon*>::iterator first(it->second.Sources.begin()), last(it->second.Sources.end());
			for (; first != last; ++first)
			{
				if (it->second.ParamId == gain_control)
				{
					float relGain = (*first)->getRelativeGain();
					float gain = (*first)->getSound()->getGain();
					(*first)->setGain(gain * value);
					(*first)->setRelativeGain(relGain);
				}
				else
				{
					(*first)->setPitch(value);
				}
			}
		}
	}
}

float CAudioMixerUser::getUserVar(NLMISC::TStringId varName)
{
	TUserVarControlsContainer::iterator it(_UserVarControls.find(varName));
	if (it != _UserVarControls.end())
	{
		return it->second.Value;
	}
	// return a default value.
	return 1.0f;
}

void CAudioMixerUser::addUserControledSource(CSourceCommon *source, NLMISC::TStringId varName)
{
	TUserVarControlsContainer::iterator it(_UserVarControls.find(varName));
	if (it != _UserVarControls.end())
	{
		// ok, the var exist, insert this source
		it->second.Sources.insert(source);
		// update the controled parameter
		if (it->second.ParamId == gain_control)
		{
			float relGain = source->getRelativeGain();
			float gain = source->getSound()->getGain();
			source->setGain(gain * it->second.Value);
			source->setRelativeGain(relGain);
		}
		else
		{
			source->setPitch(it->second.Value);
		}
	}
}

void CAudioMixerUser::removeUserControledSource(CSourceCommon *source, NLMISC::TStringId varName)
{
	TUserVarControlsContainer::iterator it(_UserVarControls.find(varName));
	if (it != _UserVarControls.end())
	{
		// ok, the var exist, remove this source
		it->second.Sources.erase(source);
	}
}



// ******************************************************************
void	CAudioMixerUser::bufferUnloaded(IBuffer *buffer)
{
	// check all track to find a track playing this buffer.
	uint i;
	for ( i=0; i!=_NbTracks; ++i )
	{
		CTrack	*track = _Tracks[i];
		if ( track && track->getSource())
		{
			if (track->getSource()->getBuffer() == buffer)
			{
				track->getSource()->stop();
			}
		}
	}

}


// ******************************************************************

void				CAudioMixerUser::enable( bool b )
{
	// TODO :  rewrite this method

	nlassert(false);
/*	if ( b )
	{
		// Reenable
		_NbTracks = _MaxNbTracks;
	}
	else
	{
		// Disable
		uint i;
		for ( i=0; i!=_NbTracks; i++ )
		{
			if ( _Tracks[i] && ! _Tracks[i]->isAvailable() )
			{
				_Tracks[i]->getSource()->leaveTrack();
//				nlassert(_PlayingSources.find(_Tracks[i]->getSource()) != _PlayingSources.end());
//				_PlayingSources.erase(_Tracks[i]->getSource());
			}
		}
		_NbTracks = 0;
	}
*/
}

// ******************************************************************

ISoundDriver*		CAudioMixerUser::getSoundDriver()
{
	return _SoundDriver;
}

// ******************************************************************

void				CAudioMixerUser::getFreeTracks( uint nb, CTrack **tracks )
{
	std::vector<CTrack*>::iterator first(_FreeTracks.begin()), last(_FreeTracks.end());
	for (nb =0; first != last; ++first, ++nb)
	{
		tracks[nb] = *first;
	}
}

 
// ******************************************************************

void				CAudioMixerUser::applyListenerMove( const NLMISC::CVector& listenerpos )
{
	// Store position
	_ListenPosition = listenerpos;

	_BackgroundSoundManager->updateBackgroundStatus();

	// Environmental effect
//	computeEnvEffect( listenerpos );

/*	// Environment sounds
	if ( _EnvSounds != NULL )
	{
		_EnvSounds->recompute();
	}
*/
}

// ******************************************************************

void				CAudioMixerUser::reloadSampleBanks(bool async)
{
	CSampleBank::reload(async);
}

// ******************************************************************

CTrack *CAudioMixerUser::getFreeTrack(CSimpleSource *source)
{
//	nldebug("There are %d free tracks", _FreeTracks.size() );
	// at least some track free ?
	if	(!_FreeTracks.empty())
	{
		// under the low water mark or  under the reserve
		if (_FreeTracks.size() > _LowWaterMark		
				|| _ReserveUsage[source->getPriority()] < _PriorityReserve[source->getPriority()] )
		{
			// non discardable track  or not too many waiting source
			if (source->getPriority() == HighestPri		
				|| _FreeTracks.size() > _SourceWaitingForPlay.size())
			{
				CTrack *ret = _FreeTracks.back();
				_FreeTracks.pop_back();
				ret->setSource(source);
				_ReserveUsage[source->getPriority()]++;
//				nldebug("Track %p assign to source %p", ret, ret->getSource());
				return ret;
			}
		}
	}
	// try to find a track with a source cuttable
	{
		float d1, d2, t1, t2;
		d1 = (source->getPos() - _ListenPosition).norm();
		t1 = max(0.0f, 1-((d1-source->getSimpleSound()->getMinDistance()) / (source->getSimpleSound()->getMaxDistance() - source->getSimpleSound()->getMinDistance())));

		for (uint i=0; i<_NbTracks; ++i)
		{
			CSimpleSource *src2 = _Tracks[i]->getSource();
			if (src2 != 0)
			{
				d2 = (src2->getPos() - _ListenPosition).norm();
				t2 = max(0.0f, 1-((d2-src2->getSimpleSound()->getMinDistance()) / (src2->getSimpleSound()->getMaxDistance() - src2->getSimpleSound()->getMinDistance())));

				const float tfactor = 1.3f;
				if (t1 > t2 * tfactor)
//				if (d1 < d2)
				{
					nldebug("Cutting source %p with source %p (%f > %f*%f)", src2, source, t1, tfactor, t2);
					// on peut cuter cette voie !
					src2->stop();
					if (_FreeTracks.empty())
					{
						nlwarning("No free track after cutting a playing sound source !");
					}
					else
					{
						CTrack *ret = _FreeTracks.back();
						_FreeTracks.pop_back();
						ret->setSource(source);
						_ReserveUsage[source->getPriority()]++;
						nldebug("Track %p assign to source %p", ret, ret->getSource());
						return ret;
					}
				}
			}
		}
	}

	return 0;
}

void CAudioMixerUser::freeTrack(CTrack *track)
{
	nlassert(track != 0);
	nlassert(track->getSource() != 0);

//	nldebug("Track %p free by source %p", track, track->getSource());

	_ReserveUsage[track->getSource()->getPriority()]--;
	track->setSource(0);
	_FreeTracks.push_back(track);
}


void CAudioMixerUser::getPlayingSoundsPos(bool virtualPos, std::vector<std::pair<bool, NLMISC::CVector> > &pos)
{
	int nbplay = 0;
	int	nbmute = 0;
	int	nbsrc = 0;

	TSourceContainer::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		CSourceCommon *ps = *first;
		if (ps->getType() == CSourceCommon::SOURCE_SIMPLE)
		{
			CSimpleSource *source = static_cast<CSimpleSource*>(*first);
			nbsrc++;

			if (source->isPlaying())
			{
				if (virtualPos)
					pos.push_back(make_pair(source->getTrack() == 0, source->getVirtualPos()));
				else
					pos.push_back(make_pair(source->getTrack() == 0, source->getPos()));

				if (source->getTrack() == 0)
					nbmute++;
				else
				{
//					nldebug ("Source %p playing on track %p", source, source->getTrack());
					nbplay ++;
				}
			}
		}
	}

//	nldebug("Total source : %d, playing : %d, muted : %d", nbsrc, nbplay, nbmute);
}



void				CAudioMixerUser::update()
{
/*	static NLMISC::TTime lastUpdate = NLMISC::CTime::getLocalTime();
	NLMISC::TTime now = NLMISC::CTime::getLocalTime();

	nldebug("Mixer update : %u ms", uint(now - lastUpdate));
	lastUpdate = now;
*/
#if NL_PROFILE_MIXER
	TTicks start = CTime::getPerformanceTime();
#endif

	// update the object.
	{
		// 1st, update the event list
		{
			std::vector<std::pair<IMixerUpdate*, bool> >::iterator first(_UpdateEventList.begin()), last(_UpdateEventList.end());
			for (; first != last; ++first)
			{
				if (first->second)
					_UpdateList.insert(first->first);
				else
					_UpdateList.erase(first->first);
			}
			_UpdateEventList.clear();
		}
		// 2nd, do the update
		{
			TMixerUpdateContainer::iterator first(_UpdateList.begin()), last(_UpdateList.end());
			for (; first != last; ++first)
			{
				// call the update method.
				const_cast<IMixerUpdate*>(*first)->onUpdate();
			}
		}
	}
	// send the event.
	{
		// 1st, update the event list
		{
			std::vector<std::pair<NLMISC::TTime, IMixerEvent*> >::iterator first(_EventListUpdate.begin()), last(_EventListUpdate.end());
			for (; first != last; ++first)
			{
				if (first->first != 0)
				{
					// add an event
//					nldebug ("Add event %p", first->second);
					TTimedEventContainer::iterator it(_EventList.insert(*first));
					_Events.insert(make_pair(first->second, it));
				}
				else
				{
					// remove the events
					pair<TEventContainer::iterator, TEventContainer::iterator> range = _Events.equal_range(first->second);
					TEventContainer::iterator first2(range.first), last2(range.second);
					for (; first2 != last2; ++first2)
					{
						// remove the event
						nldebug("Remove event %p", first2->second->second);
						_EventList.erase(first2->second);
					}
					_Events.erase(range.first, range.second);
				}
			}

			_EventListUpdate.clear();
		}
		// 2nd, call the events
		TTime now = NLMISC::CTime::getLocalTime();
		while (!_EventList.empty() && _EventList.begin()->first <= now)
		{
#ifdef _DEBUG
			CurrentEvent = _EventList.begin()->second;
#endif
//			nldebug("Sending Event %p", _EventList.begin()->second);
			_EventList.begin()->second->onEvent();
			TEventContainer::iterator it(_Events.lower_bound(_EventList.begin()->second));
			while (it->first == _EventList.begin()->second)
			{
				if (it->second == _EventList.begin())
				{
					_Events.erase(it);
					break;
				}
				it++;
			}
			_EventList.erase(_EventList.begin());
#ifdef _DEBUG
			CurrentEvent = 0;
#endif
		}
	}

	// update the background sound
	_BackgroundSoundManager->updateBackgroundStatus();

	uint i;
	// Check all playing track and stop any terminated buffer.
	for (i=0; i<_NbTracks; ++i)
	{
		if (!_Tracks[i]->isPlaying())
		{
			if (_Tracks[i]->getSource() != 0)
			{
				CSimpleSource *source = _Tracks[i]->getSource();
				source->stop();
			}
	
			// try to play any waiting source.
			if (!_SourceWaitingForPlay.empty())
			{
				// check if the source still exist before trying to play it
				if (_Sources.find(_SourceWaitingForPlay.front()) != _Sources.end())
					_SourceWaitingForPlay.front()->play();
				nldebug("Before POP Sources waiting : %u", _SourceWaitingForPlay.size());
				_SourceWaitingForPlay.pop_front();
				nldebug("After POP Sources waiting : %u", _SourceWaitingForPlay.size());
			}
		}
	}

	if (_ClusteredSound)
	{
		// update the clustered sound...
		CVector view, up;
		_Listener.getOrientation(view, up);
		_ClusteredSound->update(_ListenPosition, view, up);

		// update all playng track according to there cluster status
		for (i=0; i<_NbTracks; ++i)
		{
			if (_Tracks[i]->isPlaying())
			{
				if (_Tracks[i]->getSource() != 0)
				{
					CSimpleSource *source = _Tracks[i]->getSource();
					if (source->getCluster() != 0)
					{
						// need to check the cluster status
						const CClusteredSound::CClusterSoundStatus *css = _ClusteredSound->getClusterSoundStatus(source->getCluster());
						if (css != 0)
						{
							// there is some data here, update the virtual position of the sound.
							float dist = (css->Position - source->getPos()).norm();
							CVector vpos(_ListenPosition + css->Direction * (css->Dist + dist));
//							_Tracks[i]->DrvSource->setPos(source->getPos() * (1-css->PosAlpha) + css->Position*(css->PosAlpha));
							_Tracks[i]->DrvSource->setPos(source->getPos() * (1-css->PosAlpha) + vpos*(css->PosAlpha));
							// update the relative gain
							_Tracks[i]->DrvSource->setGain(source->getRelativeGain()*source->getGain()*css->Gain);
#if EAX_AVAILABLE == 1
							// update the occlusion parameters
							_Tracks[i]->DrvSource->setEAXProperty(DSPROPERTY_EAXBUFFER_OCCLUSION, (void*)&css->Occlusion, sizeof(css->Occlusion));
							_Tracks[i]->DrvSource->setEAXProperty(DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO, (void*)&css->OcclusionLFFactor, sizeof(css->OcclusionLFFactor));
//							if (lastRatio[i] != css->OcclusionRoomRatio)
//							{
								_Tracks[i]->DrvSource->setEAXProperty(DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO, (void*)&css->OcclusionRoomRatio, sizeof(css->OcclusionRoomRatio));
//								lastRatio[i] = css->OcclusionRoomRatio;
//								nldebug("Setting room ration.");
//							}
							_Tracks[i]->DrvSource->setEAXProperty(DSPROPERTY_EAXBUFFER_OBSTRUCTION, (void*)&css->Obstruction, sizeof(css->Obstruction));
#endif
						}
					}
				}
			}
		}
	}


	// Debug info
	/*uint32 i;
	nldebug( "List of the %u tracks", _NbTracks );
	for ( i=0; i!=_NbTracks; i++ )
	{
		CSimpleSource *su;
		if ( su = _Tracks[i]->getSource() )
		{
			nldebug( "%u: %p %s %s %s %s, vol %u",
				    i, &_Tracks[i]->DrvSource, _Tracks[i]->isAvailable()?"FREE":"USED",
					_Tracks[i]->isAvailable()?"":(su->isPlaying()?"PLAYING":"STOPPED"),
					_Tracks[i]->isAvailable()?"":PriToCStr[su->getPriority()],
					_Tracks[i]->isAvailable()?"":(su->getSound()?su->getSound()->getFilename().c_str():""),
					(uint)(su->getGain()*100.0f) );
		}
	}*/

	_SoundDriver->commit3DChanges();

#if NL_PROFILE_MIXER
	_UpdateTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
	_UpdateCount++;
#endif

/*	// display the track using...
	{
		char tmp[2048] = "";
		string str;

		for (uint i=0; i<_NbTracks/2; ++i)
		{
			sprintf(tmp, "[%2u]%8p ", i, _Tracks[i]->getSource());
			str += tmp;
		}
		nldebug((string("Status1: ")+str).c_str());
		str = "";
		for (i=_NbTracks/2; i<_NbTracks; ++i)
		{
			sprintf(tmp, "[%2u]%8p ", i, _Tracks[i]->getSource());
			str += tmp;
		}
//		nldebug((string("Status2: ")+str).c_str());
	}
*/
}


// ******************************************************************

TSoundId			CAudioMixerUser::getSoundId( const NLMISC::TStringId &name )
{
	return CSoundBank::instance()->getSound(name);
}

// ******************************************************************

void				CAudioMixerUser::addSource( CSourceCommon *source )
{ 
	nlassert(_Sources.find(source) == _Sources.end());
	_Sources.insert( source ); 

//	_profile(( "AM: ADDSOURCE, SOUND: %d, TRACK: %p, NAME=%s", source->getSound(), source->getTrack(),
//			source->getSound() && (source->getSound()->getName()!="") ? source->getSound()->getName().c_str() : "" ));

}


// ******************************************************************

USource				*CAudioMixerUser::createSource( TSoundId id, bool spawn, TSpawnEndCallback cb, void *userParam, NL3D::CCluster *cluster, CSoundContext *context )
{
#if NL_PROFILE_MIXER
	TTicks start = CTime::getPerformanceTime();
#endif

	_profile(( "AM: [%u]---------------------------------------------------------------", curTime() ));
	_profile(( "AM: CREATESOURCE: SOUND=%p, NAME=%s, TIME=%d", id, id->getName().c_str(), curTime() ));
	_profile(( "AM: SOURCES: %d, PLAYING: %d, TRACKS: %d", getSourcesNumber(), getPlayingSourcesNumber(), getNumberAvailableTracks() ));

	if ( id == NULL )
	{
		_profile(("AM: FAILED CREATESOURCE"));
		nldebug( "AM: Sound not created: invalid sound id" );
		return NULL;
	}

	USource *ret = NULL;

	if (id->getSoundType() == CSound::SOUND_SIMPLE)
	{
		CSimpleSound	*simpleSound = static_cast<CSimpleSound	*>(id);
		// This is a simple sound
		if (simpleSound->getBuffer() == NULL)
		{
			nlwarning ("Can't create the sound '%s'", CStringMapper::unmap(simpleSound->getBuffername()).c_str());
			return NULL;
		}

		// Create source
		CSimpleSource *source = new CSimpleSource( simpleSound, spawn, cb, userParam, cluster);

//		nldebug("Mixer : source %p created", source);

		if (source->getBuffer() != 0)
		{
			// Link the position to the listener position if it'a stereo source
			if ( source->getBuffer()->isStereo() )
			{
				source->set3DPositionVector( &_ListenPosition );
			}
		}
		else
		{
			nlassert(false); // FIXME
		}
		ret = source;
	}
	else if (id->getSoundType() == CSound::SOUND_COMPLEX)
	{
		CComplexSound	*complexSound = static_cast<CComplexSound*>(id);
		// This is a pattern sound.
		ret =  new CComplexSource(complexSound, spawn, cb, userParam, cluster);
	}
	else if (id->getSoundType() == CSound::SOUND_BACKGROUND)
	{
		// This is a background sound.
		CBackgroundSound	*bgSound = static_cast<CBackgroundSound	*>(id);
		ret = new CBackgroundSource(bgSound, spawn, cb, userParam, cluster);
	}
	else if (id->getSoundType() == CSound::SOUND_CONTEXT)
	{
		// This is a context sound.
		if (context != 0)
		{
			CContextSound	*ctxSound = static_cast<CContextSound	*>(id);
	//		nlassert(context != 0);
			CSound			*sound = ctxSound->getContextSound(*context);
			if (sound != 0)
			{
				ret = createSource(sound, spawn, cb, userParam, cluster);
				// Set the volume of the source according to the context volume
				if (ret != 0)
					ret->setGain(ret->getGain() * ctxSound->getGain());
			}
			else 
				ret = 0;
		}
		else
			ret = 0;
	}
	else
	{
//		nlassertex(false, ("Unknown sound class !"));
		nlwarning("Unknow sound class : %u", id->getSoundType());
	}

#if NL_PROFILE_MIXER
	_CreateTime = CTime::ticksToSecond(CTime::getPerformanceTime() - start);
	_CreateCount++;
#endif

	//nldebug( "AM: Source created" ); 
	return ret;						
}


// ******************************************************************

USource				*CAudioMixerUser::createSource( const NLMISC::TStringId &name, bool spawn, TSpawnEndCallback cb, void *userParam, NL3D::CCluster *cluster, CSoundContext *context)
{
	return createSource( getSoundId( name ), spawn, cb, userParam, cluster, context);
}


// ******************************************************************

void				CAudioMixerUser::removeSource( CSourceCommon *source )
{
	nlassert( source != NULL );
	
	size_t n = _Sources.erase(source);
	nlassert(n == 1);
}


// ******************************************************************

void				CAudioMixerUser::selectEnvEffects( const std::string &tag )
{
	nlassertex(false, ("Not implemented yet"));
/*	// Select Env
	vector<CEnvEffect*>::iterator ipe;
	for ( ipe=_EnvEffects.begin(); ipe!=_EnvEffects.end(); ++ipe )
	{
		(*ipe)->selectEnv( tag );
	}

	// Compute
	CVector pos;
	_Listener.getPos( pos );
	computeEnvEffect( pos, true );
*/
}


// ******************************************************************

/*
void				CAudioMixerUser::loadEnvEffects( const char *filename )
{
	nlassert( filename != NULL );
	nlinfo( "Loading environmental effects from %s...", filename );

	// Unload previous env effects
	vector<CEnvEffect*>::iterator ipe;
	for ( ipe=_EnvEffects.begin(); ipe!=_EnvEffects.end(); ++ipe )
	{
		delete (*ipe);
	}
	_EnvEffects.clear();

	string str = CPath::lookup( filename, false );

	// Load env effects
	CIFile file;
	if ( !str.empty() && file.open(str) )
	{
		uint32 n = CEnvEffect::load( _EnvEffects, file );
		nldebug( "AM: Loaded %u environmental effects", n );
	}
	else
	{
		nlwarning( "AM: Environmental effects file not found" );
	}
}
*/

// ******************************************************************

uint32			CAudioMixerUser::loadSampleBank(bool async, const std::string &name, std::vector<std::string> *notfoundfiles )
{
//	nlassert( filename != NULL );

//	string path = _SamplePath;
//	path.append("/").append(filename);

	nldebug( "Loading samples bank %s...", name.c_str() );

	CSampleBank* bank = CSampleBank::findSampleBank(CStringMapper::map(name));
	if (bank == NULL)
	{
		// create a new sample bank
		bank = new CSampleBank(name, _SoundDriver);
	}

	try 
	{
		bank->load(async);
	}
	catch (Exception& e)
	{
		if (notfoundfiles) 
		{
			notfoundfiles->push_back(name);
		}
		string reason = e.what();
		nlwarning( "AM: Failed to load the samples: %s", reason.c_str() );
	}


	return bank->countSamples();
}

bool CAudioMixerUser::unloadSampleBank( const std::string &name)
{
//	string path = _SamplePath;
//	path.append("/").append(filename);

	nldebug( "Unloading samples bank %s...", name.c_str() );
	CSampleBank *pbank = CSampleBank::findSampleBank(CStringMapper::map(name));

	if (pbank != NULL)
	{
		// ok, the bank exist.
		return pbank->unload();
	}
	else
		return false;

}

// ******************************************************************

void			CAudioMixerUser::getSoundNames( std::vector<NLMISC::TStringId> &names ) const
{
	CSoundBank::instance()->getNames(names);
}


// ******************************************************************

uint			CAudioMixerUser::getPlayingSourcesNumber() const
{
	return _PlayingSources;
}

// ******************************************************************

uint			CAudioMixerUser::getNumberAvailableTracks() const
{
	return _FreeTracks.size();
}


// ******************************************************************

string			CAudioMixerUser::getSourcesStats() const
{
	// TODO : rewrite log output

	string s;
	TSourceContainer::iterator ips;
	for ( ips=_Sources.begin(); ips!=_Sources.end(); ++ips )
	{
		if ( (*ips)->isPlaying() )
		{
//			char line [80];

/*			nlassert( (*ips)->getSound() && (*ips)->getSimpleSound()->getBuffer() );
			smprintf( line, 80, "%s: %u%% %s %s",
					  (*ips)->getSound()->getName().c_str(),
					  (uint32)((*ips)->getGain()*100.0f),
					  (*ips)->getBuffer()->isStereo()?"ST":"MO",
					  PriToCStr[(*ips)->getPriority()] );
			s += string(line) + "\n";
*/		}
	}
	return s;

}

// ******************************************************************
/*
void			CAudioMixerUser::loadEnvSounds( const char *filename, UEnvSound **treeRoot )
{
	nlassert( filename != NULL );
	nlinfo( "Loading environment sounds from %s...", filename );

	string str = CPath::lookup( filename, false );

	CIFile file;
	if ( !str.empty() && file.open( str ) )
	{
		uint32 n = 0; //CEnvSoundUser::load( _EnvSounds, file );
		nldebug( "AM: Loaded %u environment sounds", n );
	}
	else
	{
		nlwarning( "AM: Environment sounds file not found: %s", filename );
	}
	if ( treeRoot != NULL )
	{
		*treeRoot = _EnvSounds;
	}
}
*/

// ******************************************************************

struct CompareSources : public binary_function<const CSimpleSource*, const CSimpleSource*, bool>
{
	// Constructor
	CompareSources( const CVector &pos ) : _Pos(pos) {}

	// Operator()
	bool operator()( const CSimpleSource *s1, const CSimpleSource *s2 )
	{
		if (s1->getPriority() < s2->getPriority())
		{
			return true;
		}
		else if (s1->getPriority() == s2->getPriority())
		{
			// Equal priority, test distances to the listener
			const CVector &src1pos = s1->getPos();
			const CVector &src2pos = s2->getPos();;
			return ( (src1pos-_Pos).sqrnorm() < (src2pos-_Pos).sqrnorm() );
		}
		else
		{
			return false;
		}
	}

	// Listener pos
	const CVector &_Pos;
};


// ******************************************************************
uint32			CAudioMixerUser::getLoadedSampleSize()
{
	return CSampleBank::getTotalByteSize();
}

void			CAudioMixerUser::getLoadedSampleBankInfo(std::vector<std::pair<std::string, uint> > &result)
{
	CSampleBank::getLoadedSampleBankInfo(result);
}


// ******************************************************************
/*
void			CAudioMixerUser::redispatchSourcesToTrack()
{
*/	// TODO : rewrite ?
/*
	if ( _NbTracks == 0 )
	{
		return;
	}

	_profile(( "AM: [%u]---------------------------------------------------------------", curTime() ));
	_profile(( "AM: Redispatching sources" ));
	
	const CVector &listenerpos = _Listener.getPos();

	// Get a copy of the sources set (we will modify it)
	static TSourceContainer sources_copy; 
	sources_copy = _PlayingSources;
	// FIXME: SWAPTEST
	//nlassert( sources_copy.size() >= _NbTracks );

	// Select the nbtracks "smallest" sources (the ones that have the higher priorities)
	TSourceContainer::iterator ips;
	static TSourceContainer selected_sources;
	uint32 i;

	selected_sources.clear();

	// Select the sources

	// Select the nbtracks "smallest" sources (the ones that have the higher priorities)
	// FIXME: SWAPTEST
	//for ( i=0; i!=_NbTracks; i++ )
	// TODO : optimize : this is a very BAD PERFORMANCE code
	while (!sources_copy.empty() && (selected_sources.size() < _NbTracks))
	{
		ips = min_element( sources_copy.begin(), sources_copy.end(), CompareSources( listenerpos ) );

		if ((*ips)->isPlaying())
		{
			selected_sources.insert( *ips );
		}

		sources_copy.erase( ips );
	}

	// Clear the current tracks where the sources are not selected anymore
	_profile(( "AM: Total sources: %u", _PlayingSources.size() ));
	_profile(( "AM: Selected sources: %u", selected_sources.size() ));
	for ( i=0; i!=_NbTracks; i++ )
	{
		// FIXME: SWAPTEST
		if ( _Tracks[i] && ! _Tracks[i]->isAvailable() )
		{
			// Optimization note: instead of searching the source in selected_sources, we could have
			// set a boolean in the source object and tested it.
			if ( (ips = selected_sources.find( _Tracks[i]->getSource() )) == selected_sources.end() )
			{
				// There will be a new source in this track
				_profile(( "AM: TRACK: %p: REPLACED, SOURCE: %p", _Tracks[i], _Tracks[i]->getSource() ));
				if (_Tracks[i]->getSource() != 0)
				{
					_Tracks[i]->getSource()->leaveTrack();
//					nlassert(_PlayingSources.find(_Tracks[i]->getSource()) != _PlayingSources.end());
					_PlayingSources.erase(_Tracks[i]->getSource());
				}
			}
			else
			{
				// The track will remain unchanged
				selected_sources.erase( ips );
				_profile(( "AM: TRACK: %p: UNCHANGED, SOURCE: %p", _Tracks[i], _Tracks[i]->getSource() ));
			}
		}
		else
		{
			_profile(( "AM: TRACK: %p: FREE", _Tracks[i] ));
		}
	}

	if (!selected_sources.empty())
	{
		// Now, only the sources to add into the tracks remain in selected_sources
		CTrack *track [MAX_TRACKS]; // a little bit more than needed (avoiding a "new")
		getFreeTracks( selected_sources.size(), track );

		_profile(( "AM: Remaining sources: %u", selected_sources.size() ));

		for (i=0, ips=selected_sources.begin(); ips!=selected_sources.end(); ++ips )
		{
			// FIXME: SWAPTEST
			(*ips)->enterTrack( track[i] );
//			nlassert(_PlayingSources.find(*ips) == _PlayingSources.end());
//			_PlayingSources.insert(*ips);
	//		_profile(( "AM: TRACK: %p: ASSIGNED, SOURCE: %p", track[i], track[i]->getSource() ));
			_profile(( "AM: TRACK: %p: ASSIGNED, SOURCE: %p", track[i], *ips ));
			i++;
		}
	}
*/
//}

void CAudioMixerUser::setListenerPos (const NLMISC::CVector &pos)
{
	_Listener.setPos(pos);
	_BackgroundSoundManager->setListenerPosition(pos);
}

NLMISC_COMMAND (displaySoundInfo, "Display information about the audio mixer", "")
{
	if(args.size() != 0) return false;

	if (CAudioMixerUser::instance() == NULL)
	{
		log.displayNL ("No audio mixer available");
		return true;
	}

	log.displayNL ("%d tracks, MAX_TRACKS = %d, contains:", CAudioMixerUser::instance()->_NbTracks, MAX_TRACKS);

	for (uint i = 0; i < CAudioMixerUser::instance()->_NbTracks; i++)
	{
		if (CAudioMixerUser::instance()->_Tracks[i] == NULL)
		{
			log.displayNL ("Track %d is NULL", i);
		}
		else
		{
			log.displayNL ("Track %d %s available and %s playing.", i, (CAudioMixerUser::instance()->_Tracks[i]->isAvailable()?"is":"is not"), (CAudioMixerUser::instance()->_Tracks[i]->isPlaying()?"is":"is not"));
			if (CAudioMixerUser::instance()->_Tracks[i]->getSource() == NULL)
			{
				log.displayNL ("    CUserSource is NULL");
			}
			else
			{
				const CVector &pos = CAudioMixerUser::instance()->_Tracks[i]->getSource()->getPos();
				string bufname;
				if (CAudioMixerUser::instance()->_Tracks[i]->getSource()->getBuffer())
					bufname = CAudioMixerUser::instance()->_Tracks[i]->getSource()->getBuffer()->getName();
				log.displayNL ("    CUserSource is id %d buffer name '%s' pos %f %f %f", CAudioMixerUser::instance()->_Tracks[i]->getSource()->getSound(), bufname.c_str(), pos.x, pos.y, pos.z);
			}
		}
	}

	return true;
}

void CAudioMixerUser::registerBufferAssoc(CSound *sound, IBuffer *buffer)
{
	_BufferToSources[buffer].push_back(sound);
}

void CAudioMixerUser::unregisterBufferAssoc(CSound *sound, IBuffer *buffer)
{
	TBufferToSourceContainer::iterator it(_BufferToSources.find(buffer));
	if (it != _BufferToSources.end())
	{
		std::vector<CSound*>::iterator first(it->second.begin()), last(it->second.end());

		for (; first != last; ++first)
		{
			if (*first == sound)
			{
				it->second.erase(first);
				break;
			}
		}
	}
}


/// Register an object in the update list.
void CAudioMixerUser::registerUpdate(CAudioMixerUser::IMixerUpdate *pmixerUpdate)
{
	_UpdateEventList.push_back(make_pair(pmixerUpdate, true));
}
/// Unregister an object from the update list.
void CAudioMixerUser::unregisterUpdate(CAudioMixerUser::IMixerUpdate *pmixerUpdate)
{
	_UpdateEventList.push_back(make_pair(pmixerUpdate, false));
}

/// Add an event in the future.
void CAudioMixerUser::addEvent( CAudioMixerUser::IMixerEvent *pmixerEvent, const NLMISC::TTime &date)
{
//	nldebug("Adding event %p", pmixerEvent);
	_EventListUpdate.push_back(make_pair(date, pmixerEvent));
}
/// Remove any event programmed for this object.
void CAudioMixerUser::removeEvents( CAudioMixerUser::IMixerEvent *pmixerEvent)
{
//	nldebug("Removing event %p", pmixerEvent);
	// store the pointer fot future removal.
	_EventListUpdate.push_back(make_pair(0, pmixerEvent));
}

void CAudioMixerUser::setBackgroundFlags(const TBackgroundFlags &backgroundFlags)
{
	_BackgroundSoundManager->setBackgroundFlags(backgroundFlags);
}

void CAudioMixerUser::setBackgroundFilterFades(const TBackgroundFilterFades &backgroundFilterFades)
{
	_BackgroundSoundManager->setBackgroundFilterFades(backgroundFilterFades);
}


void CAudioMixerUser::loadBackgroundSoundFromRegion (const NLLIGO::CPrimRegion &region)
{
	_BackgroundSoundManager->loadSoundsFromRegion(region);
}

void CAudioMixerUser::loadBackgroundEffectsFromRegion (const NLLIGO::CPrimRegion &region)
{
	_BackgroundSoundManager->loadEffecsFromRegion(region);
}
void CAudioMixerUser::loadBackgroundSamplesFromRegion (const NLLIGO::CPrimRegion &region)
{
	_BackgroundSoundManager->loadSamplesFromRegion(region);
}

void CAudioMixerUser::loadBackgroundAudioFromPrimitives(const NLLIGO::IPrimitive &audioRoot)
{
	_BackgroundSoundManager->loadAudioFromPrimitives(audioRoot);
}

void CAudioMixerUser::playBackgroundSound () 
{ 
	_BackgroundSoundManager->play (); 
}

void CAudioMixerUser::stopBackgroundSound () 
{ 
	_BackgroundSoundManager->stop (); 
}

void CAudioMixerUser::loadBackgroundSound (const std::string &continent) 
{ 
	_BackgroundSoundManager->load (continent); 
}

} // NLSOUND
