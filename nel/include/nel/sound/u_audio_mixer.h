/** \file u_audio_mixer.h
 * UAudioMixer: game interface for audio
 *
 * $Id: u_audio_mixer.h,v 1.7 2001/08/24 12:44:54 cado Exp $
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
#include <vector>


namespace NLSOUND {

	
class UEnvSound;
class UListener;


#define AUTOBALANCE_DEFAULT_PERIOD 20


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

	/// Create the audio mixer singleton and return a pointer to its instance
	static UAudioMixer	*createAudioMixer();
	/** Initialization
	 *
	 * In case of failure, can throw one of these ESoundDriver (Exception) objects:
	 * ESoundDriverNotFound, ESoundDriverCorrupted, ESoundDriverOldVersion, ESoundDriverUnknownVersion.
	 *
	 * The sources will be auto-balanced every "balance_period" calls to update()
	 * (set 0 for "never auto-balance")
	 */
	virtual void		init( uint32 balance_period=AUTOBALANCE_DEFAULT_PERIOD ) = 0;


	/// Load environment effects
	virtual void		loadEnvEffects( const char *filename ) = 0;
	/** Load buffers. Returns the number of buffers successfully loaded.
	 *  If you specify a non null notfoundfiles vector, it is filled with the names of missing files if any.
	 */
	virtual uint32		loadSoundBuffers( const char *filename, std::vector<std::string> *notfoundfiles=NULL ) = 0;
	/// Load environment sounds ; treeRoot can be null if you don't want an access to the envsounds
	virtual	void		loadEnvSounds( const char *filename,
									   UEnvSound **treeRoot=NULL ) = 0;
	/// Get a TSoundId from a name (returns NULL if not found)
	virtual TSoundId	getSoundId( const char *name ) = 0;


	/// Add a logical sound source (returns NULL if name not found). To remove a source, just delete it.
	virtual USource		*createSource( const char *name ) = 0;
	/// Add a logical sound source (by sound id). To remove a source, just delete it.
	virtual USource		*createSource( TSoundId id ) = 0;


	/// Return the listener interface
	virtual UListener	*getListener() = 0;


	/// Choose the environmental effect(s) corresponding to tag
	virtual void		selectEnvEffects( const char *tag ) = 0;
	/// Update audio mixer (call evenly)
	virtual void		update() = 0;


	/// Return the number of mixing tracks (voices)
	virtual uint		getPolyphony() const = 0;
	/// Return the names of the sounds (call this method after loadSoundBuffers())
	virtual void		getSoundNames( std::vector<const char *>& names ) const = 0;


	/// Destructor
	virtual				~UAudioMixer() {}

protected:

	/// Constructor
	UAudioMixer() {}

};


} // NLSOUND


#endif // NL_U_AUDIO_MIXER_H

/* End of u_audio_mixer.h */
