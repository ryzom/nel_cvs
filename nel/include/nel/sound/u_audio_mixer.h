/** \file u_audio_mixer.h
 * UAudioMixer: game interface for audio
 *
 * $Id: u_audio_mixer.h,v 1.21 2003/02/06 09:09:25 boucher Exp $
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

#ifndef NL_U_AUDIO_MIXER_H
#define NL_U_AUDIO_MIXER_H

#include "nel/misc/types_nl.h"
#include "nel/sound/u_source.h"
#include "nel/ligo/primitive.h"
#include <vector>

namespace NL3D
{
	class CCluster;
	class UScene;
	class CScene;
}

namespace NLMISC
{
	class IProgressCallback;
}

namespace NLSOUND {

	
class UEnvSound;
class UListener;


//const uint32 AUTOBALANCE_DEFAULT_PERIOD = 20;


/**
 * Game interface for audio
 *
 * The logical sources represent all entities in the world, from the client's point of view.
 * Their number can be higher than the number of simultaneous playable sound on the soundcard.
 *
 * When there are more sources than tracks, the process of choosing which sources go into
 * the tracks is called "balancing". The source are auto-balanced according to the
 * argument passed to init(). The sources are also balanced when
 * - Adding a new source
 * - Removing a new source
 * - Entering/Exiting from an envsound area
 *
 * Important: The user is responsible for deleting the sources that have been allocated by
 * createSource(), before deleting the audio mixer object.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class UAudioMixer
{
public:


	/** Structure that contain the background flags.*/
	struct TBackgroundFlags
	{
		// speudo enum
		enum 
		{
			/// Number of filter flags in the background.
			NB_BACKGROUND_FLAGS = 32
		};

		bool	Flags[NB_BACKGROUND_FLAGS];

		void serial(NLMISC::IStream &s)
		{
			for (uint i = 0; i<NB_BACKGROUND_FLAGS; ++i)
				s.serial(Flags[i]);
		}
	};

	/** Structure that contain the background filter fadein and fade out
	 *  This are configuration data.
	 */
	struct TBackgroundFilterFades
	{
		NLMISC::TTime	FadeIns[TBackgroundFlags::NB_BACKGROUND_FLAGS];
		NLMISC::TTime	FadeOuts[TBackgroundFlags::NB_BACKGROUND_FLAGS];

		TBackgroundFilterFades()
		{
			for (uint i=0; i<TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
			{
				FadeIns[i] = 0;
				FadeOuts[i] = 0;
			}
		}
	};

	/// Create the audio mixer singleton and return a pointer to its instance
	static UAudioMixer	*createAudioMixer();
	/** Initialization
	 *
	 * In case of failure, can throw one of these ESoundDriver (Exception) objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion.
	 *
	 * You can ask for EAX support. If EAX support is requested, then the mixer will try to allocate
	 * hardware accelerated audio tracks. 
	 * If the total of available harware track is less than 10, then EAX is automaticaly 
	 * deactivated.
	 */
	virtual void		init(bool useEax = true, NLMISC::IProgressCallback *progressCallBack = NULL) = 0;

	/** Initialisation of the clustered sound system.
	  */
	virtual void		initClusteredSound(NL3D::UScene *uscene, float minGain, float maxDistance, float portalInterpolate = 20.0f) = 0;
	/** Initialisation of the clustered sound system. CNELU version
	 */
	virtual void		initClusteredSound(NL3D::CScene *scene, float minGain, float maxDistance, float portalInterpolate = 20.0f) = 0;
	/** Set the priority channel reserve.
	 *	Each priority channel can be assign a restrictive reserve value.
	 *	This value is used when the number free track available for playing drop
	 *	under the low water mark value (see setLowWaterMark).
	 *	The mixer count the number of playing source in each priority channel. 
	 *	A priority channel can orverflow it's reserve value only if the low water
	 *	mark is not reach.
	 *	In other word, when the number of played source increase, you can control
	 *	a 'smooth' cut in priority layer. The idea is to try to keep some free track
	 *	for the HighestPri source.
	 *	By default, reserve are set for each channel to the number of available tracks.
	 */
	virtual void		setPriorityReserve(TSoundPriority priorityChannel, uint reserve) = 0;
	/** Set the Low water mark.
	 *	This value is use to mute sound source that try to play when there priority
	 *	channel is full (see setPriorityReserve).
	 *	Set a value 1 to 4 to keep some extra track available when a
	 *	HighestPri source need to play.
	 *	By default, the value is set to 0, witch mean no special treatment is done
	 *	and the mixer will mute sound with no user control at all.
	 *	Note also that the availability of a track is not guarantie if the sum of
	 *	the priority reserve (see setPriorityReserve) is grater than the number od
	 *	available tracks (witch is almos alwais the case). But this value will help
	 *	the mixer make it's best.
	 */
	virtual void		setLowWaterMark(uint value) = 0;


	 /// Resets the audio system (deletes all the sources, include envsounds)
	virtual void		reset() = 0;
	/// Disables or reenables the sound
	virtual void		enable( bool b ) = 0;

	/** Load environment effects
	 *	\deprecated
	 */
//	virtual void		loadEnvEffects( const char *filename ) = 0;
	/** Load buffers. Returns the number of buffers successfully loaded.
	 *  If you specify a non null notfoundfiles vector, it is filled with the names of missing files if any.
	 *	\param async If true, the sample are loaded in a background thread.
	 *	\param filename Name of the directory that contains the samples to load.
	 *	\param notfoundfiles An optionnal pointer to a vector that will be filled with the list of not found files.
	 */
	virtual uint32		loadSampleBank(bool async, const std::string &filename, std::vector<std::string> *notfoundfiles=NULL ) = 0;
	/** Unload buffers. Return false if the bank can't be unloaded because an async loading is running.
	*/
	virtual bool		unloadSampleBank( const std::string &filename) = 0;
	/** Reload all the sample bank.
	 *	Thid method use provided for use in a sound editor or sound tool to update the list of available samples.
	 *	\async If true, the samples are loaded in a background thread.
	 */
	virtual void		reloadSampleBanks(bool async) =0;
	/** Return the total size in byte of loaded samples.
	 */
	virtual uint32		getLoadedSampleSize() =0;

	/// Get a TSoundId from a name (returns NULL if not found)
	virtual TSoundId	getSoundId( const std::string &name ) = 0;

	/** Add a logical sound source (returns NULL if name not found).
	 * If spawn is true, the source will auto-delete after playing. If so, the return USource* pointer
	 * is valid only before the time when calling play() plus the duration of the sound. You can
	 * pass a callback function that will be called (if not NULL) just before deleting the spawned
	 * source.
	 */
	virtual USource		*createSource( const std::string &name, bool spawn=false, TSpawnEndCallback cb=NULL, void *callbackUserParam = NULL, NL3D::CCluster *cluster = 0, CSoundContext *context=0) = 0;
	/// Add a logical sound source (by sound id). To remove a source, just delete it. See createSource(const char*)
	virtual USource		*createSource( TSoundId id, bool spawn=false, TSpawnEndCallback cb=NULL, void *callbackUserParam  = NULL, NL3D::CCluster *cluster = 0, CSoundContext *context=0 ) = 0;

	/** Use this method to set the listener position instead of using getListener->setPos();
	 * It's because we have to update the background sounds in this case.
	 */
	virtual void		setListenerPos (const NLMISC::CVector &pos) = 0;

	/// Return the listener interface
	virtual UListener	*getListener() = 0;


	/** Choose the environmental effect(s) corresponding to tag
	 *	\deprecated
	 */
	virtual void		selectEnvEffects( const std::string &tag ) = 0;
	/// Update audio mixer (call evenly)
	virtual void		update() = 0;


	/// Return the names of the sounds (call this method after loadSounds())
	virtual void		getSoundNames( std::vector<std::string>& names ) const = 0;
	/// Return the number of mixing tracks (voices)
	virtual uint		getPolyphony() const = 0;
	/// Return the number of sources
	virtual uint		getSourcesNumber() const = 0;
	/// Return the number of playing sources
	virtual uint		getPlayingSourcesNumber() const = 0;
	/// Return the number of available tracks
	virtual uint		getNumberAvailableTracks() const = 0;
	/// Return a string showing the playing sources
	virtual std::string	getSourcesStats() const = 0;

	/// Set the global path to the sample banks
	virtual void		setSamplePath(const std::string& path) = 0;

	/** Write profiling information about the mixer to the output stream.
	 *  \param out The output stream to which to write the information
	 */
	virtual void		writeProfile(std::ostream& out) = 0;

	virtual void		setBackgroundFlagName(uint flagIndex, const std::string &flagName) = 0;
	virtual void		setBackgroundFlagShortName(uint flagIndex, const std::string &flagShortName) = 0;
	virtual const std::string &getBackgroundFlagName(uint flagIndex) =0;
	virtual const std::string &getBackgroundFlagShortName(uint flagIndex) =0;
	virtual void		setBackgroundFlags(const TBackgroundFlags &backgroundFlags) = 0;
	virtual const TBackgroundFlags &getBackgroundFlags() =0;
	virtual void		setBackgroundFilterFades(const TBackgroundFilterFades &backgroundFilterFades) = 0;
	virtual const TBackgroundFilterFades &getBackgroundFilterFades() = 0;

	/// Deprecated
	virtual void		loadBackgroundSoundFromRegion (const NLLIGO::CPrimRegion &region) = 0;
	/// Deprecated
	virtual void		loadBackgroundEffectsFromRegion (const NLLIGO::CPrimRegion &region) = 0;
	/// Deprecated
	virtual void		loadBackgroundSamplesFromRegion (const NLLIGO::CPrimRegion &region) = 0;
	virtual void		loadBackgroundAudioFromPrimitives(const NLLIGO::IPrimitive &audioRoot) =0;
	virtual void		loadBackgroundSound (const std::string &continent) = 0;
	virtual void		playBackgroundSound () = 0;
	virtual void		stopBackgroundSound () = 0;

	virtual void		getPlayingSoundsPos(bool virtualPos, std::vector<std::pair<bool, NLMISC::CVector> > &pos) =0;

	/// Destructor
	virtual				~UAudioMixer() {}

protected:

	/// Constructor
	UAudioMixer() {}

};


} // NLSOUND


#endif // NL_U_AUDIO_MIXER_H

/* End of u_audio_mixer.h */
