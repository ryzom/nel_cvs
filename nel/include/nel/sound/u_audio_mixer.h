/** \file u_audio_mixer.h
 * UAudioMixer: game interface for audio
 *
 * $Id: u_audio_mixer.h,v 1.1 2001/07/10 16:51:20 cado Exp $
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


/**
 * Game interface for audio
 *
 * Removing does not mean deleting. The user is responsible for deleting the sources and envsounds
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
	/// Initialization
	virtual void		init() = 0;


	/// Load environment effects
	virtual void		loadEnvEffects( const char *filename ) = 0;
	/// Load buffers
	virtual	void		loadSoundBuffers( const char *filename,
										  const std::vector<TSoundId> **idvec ) = 0;
	/// Load environment sounds ; idvec can be null if you don't want an access to the envsounds
	virtual	void		loadEnvSounds( const char *filename,
									   const std::vector<UEnvSound*> **esvec=NULL ) = 0;
	/// Add logical sound source
	virtual USource		*createSource( TSoundId id ) = 0;
	/// Remove logical sound source
	virtual void		removeSource( USource *source ) = 0;
	// Remove environment sound
	//*virtual void		removeEnvSound( UEnvSound *envsound ) = 0;
	/// Return the listener interface
	virtual UListener	*getListener() = 0;


	/// Choose the environmental effect(s) corresponding to tag
	virtual void		selectEnvEffects( const std::string& tag ) = 0;
	/// Update audio mixer (call evenly)
	virtual void		update() = 0; 


	/// Destructor
	virtual				~UAudioMixer() {}

protected:

	/// Constructor
	UAudioMixer() {}

};


} // NLSOUND


#endif // NL_U_AUDIO_MIXER_H

/* End of u_audio_mixer.h */
