/** \file sound_system.h
 * This initilize the sound system
 *
 * $Id: sound_system.h,v 1.4 2001/09/05 15:44:05 vizerie Exp $
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


#ifndef OV_SOUND_SYTEM_H
#define OV_SOUND_SYTEM_H


#include <nel/misc/types_nl.h>
#include <string>
#include <set>

namespace NLSOUND
{
	class UAudioMixer;
}

namespace NLMISC
{
	class CMatrix;
}


/// this class init the sound system used by the object viewer
class CSoundSystem
{
public:
	/// set the name of the file containing the sound bank
	static void addSoundBank(const std::string &soundBankFileName)
	{
		_SoundBanksFileName.insert(soundBankFileName);
	}
	/** Init the sound system this also load the sound bank
	  * See setSoundBank
	  */
	static void initSoundSystem(void);

	/// release the sound system
	static void releaseSoundSystem(void);

	/// set the listener matrix.
	static void setListenerMatrix(const NLMISC::CMatrix &m);

	/// poll sound. Must be called periodically
	static void poll(void);

	// spawn a sound at the user position
	static void play(const std::string &soundName);

	// get the audio mixer, or null if init failed
	static NLSOUND::UAudioMixer *getAudioMixer(void) { return _AudioMixer; }	

private:
	static NLSOUND::UAudioMixer *_AudioMixer;
	static std::set<std::string>	_SoundBanksFileName;
};


#endif