/** \file background_sound_manager.h
 * CBackgroundSoundManager
 *
 * $Id: background_sound_manager.h,v 1.3 2002/11/04 15:40:43 boucher Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

#ifndef NL_BACKGROUND_SOUND_MANAGER_H
#define NL_BACKGROUND_SOUND_MANAGER_H

#include "nel/misc/types_nl.h"
#include "background_sound.h"
#include <vector>
#include <set>


namespace NLLIGO
{
	class CPrimRegion;
}

namespace NLSOUND {


class CListenerUser;
class IPlayable;
class IBoundingShape;

/// Number of background layer. Layer are identified in .prim by a letter starting from 'a' (for layer 0)
const uint32	BACKGROUND_LAYER = 3;	// 3 layer


/**
 * This manager handle the background sound :
 * - primitive positioned sound (point, path and patatoid supported)
 * - primitive positioned effect (patatoid only)
 * - primitive positioned sample bank (patatoid only)
 * - A set of 16 application definable flag that can be used in
 *	background sound to filter the sub sounds of a background sound.
 *	
 *	 
 *
 * Sounds can be put in three separate layer (named a, b and c). In each
 * layer, sounds patatoid are concurent and the smaller in surface muffle the
 * other sounds.
 *
 * Effets are managed according to the EAX capacity of the driver.
 *
 * Sample bank patatoid are used to dynamicaly load/unload the sample banks will the
 * player walk the univers.
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2002
 */
class CBackgroundSoundManager
{
public:
	/** Load the background sounds from a CPrimRegion class.
	 */
	void		loadSoundsFromRegion(const NLLIGO::CPrimRegion &region);
	/** Load the effects from a CPrimRegion class.
	 */
	void		loadEffecsFromRegion(const NLLIGO::CPrimRegion &region);
	/** Load the samples banks from a CPrimRegion class.
	 */
	void		loadSamplesFromRegion(const NLLIGO::CPrimRegion &region);

	/** Load background sound for a continent. It'll automatically unload the old continent before loading the new one.
	 * This method load the 'audio' regions (specifying the sounds), the 'effect' regions and the 'sample' regions.
	 * Continent is for example "matis" or "fyros". It'll add .prim to the continent name and lookup() to find zones.
	 * So, don't forget to add sound .prim in the CPath system for the lookup
	 */
	void		load (const std::string &continent);

	/** Remove all data about the current continents
	 */
	void		unload ();

	/// Start to play the background sound.
	void		play();
	/// Stop the background sound.
	void		stop();

	/// Call this method when the listener position change
	void		setListenerPosition (const NLMISC::CVector &listenerPosition);

	/// Call this method to update the bacground sound (sub method od setListenerPosition)
	void		updateBackgroundStatus();

	// Call this function evenly to update the stuffs
//	void		update ();

	/// Return a patatoid. If isPath is not null, set it.
	const std::vector<NLMISC::CVector> &getZone(uint32 zone, bool *isPath = 0);

	/// Return the position of the 3d source for a zone
	NLMISC::CVector getZoneSourcePos(uint32 zone);

	/// Set the background flags.
	const UAudioMixer::TBackgroundFlags &getBackgroundFlags();
	/// Get the background flags.
	void		setBackgroundFlags(const UAudioMixer::TBackgroundFlags &backgroundFlags);

private:

	// CAudioMixerUser will call private constructor and destructor, so, it is our friend ;)
	friend class CAudioMixerUser;

	/// Constructor
	CBackgroundSoundManager();
	/// Destructor
	virtual					~CBackgroundSoundManager();

	/// Internal use only for loading.
	void addSound(const std::string &soundName, const std::vector<NLMISC::CVector> &points, bool isPath);

	/// TODO : Utility... should be in NLMISC ?
	template <class CharType>
		std::vector<std::basic_string<CharType> >	split(const std::basic_string<CharType> &str, CharType splitTag)
	{
		std::vector<std::basic_string<CharType> >	splitted;
		std::basic_string<CharType>::size_type pos = 0, nextPos = 0, size = 0;

		while ((nextPos = str.find(splitTag, nextPos)) != std::basic_string<CharType>::npos)
		{
			size = nextPos - pos;
			if (size > 0)
				splitted.push_back(std::basic_string<CharType>(str, pos, nextPos - pos));
			// skip the tag
			nextPos += 1;
			pos = nextPos;
		}
		// is there a last part ?
		size = nextPos - pos;
		if (pos != str.size())
			splitted.push_back(std::basic_string<CharType>(str, pos, str.size() - pos));

		return splitted;
	}


	/// Flag for playing background sounds.
	bool					_Playing;

	/// Environnement flags.
	UAudioMixer::TBackgroundFlags _BackgroundFlags;

	/// The last position of the listener.
	NLMISC::CVector			_LastPosition;

	//@{
	//\name Samples banks related thinks

	/// Storage for a samples banks zone
	struct TBanksData
	{
		/// The list of samples banks that are active on this patatoid
		std::vector<std::string>		Banks;
		/// The min vector of the bounding box
		NLMISC::CVector		MinBox;
		/// The max vector of the bounding box
		NLMISC::CVector		MaxBox;
		/// The vector of points composing the patatoid.
		std::vector<NLMISC::CVector>	Points;
	};
	/// Container for the banks primitive.
	std::vector<TBanksData>		_Banks;
	/// Container for the list of currently loaded banks.
	std::set<std::string>		_LoadedBanks;
	//@}


	//@{
	//\name Sounds related thinks
	/// Storage for all a sound in a layer.
	struct TSoundData
	{
		/// The name of the sound.
		std::string			SoundName;
		/// The reference to the sound.
		CSound				*Sound;
		/// A source instance of the sound (may be NULL).
		USource				*Source;

		/// The min vector of the bounding box
		NLMISC::CVector		MinBox;
		/// The max vector of the bounding box
		NLMISC::CVector		MaxBox;
		/// The surface of the bounding box (used for patatoid competition)
		float				Surface;
		/// The max earing distance of the sound.
		float				MaxDist;
		/// Flag for path/patatoid sound.
		bool				IsPath;
		/// The vector of points compositing the primitive
		std::vector<NLMISC::CVector>	Points;
		/// Flag telling if this sound is currently selected for play by bounding box.
		bool				Selected;
	};
	/// Array of vector of sound data.
	std::vector<TSoundData>		_Layers[BACKGROUND_LAYER];
	// utility structure for audio mixing computation (see cpp)
	struct TSoundStatus
	{
		/// The data of the sound.
		TSoundData			&SoundData;
		/// The position of the source.
		NLMISC::CVector		Position;
		/** The relative gain of the source. This is used for patatoid competition.when
		  * a smaller patatoid mute bigger one.
		  */
		float				Gain;
		/// The distance beween listener and source.
		float				Distance;
		/// Constructor.
		TSoundStatus(TSoundData &sd, NLMISC::CVector position, float gain, float distance)
			: SoundData(sd), Position(position), Gain(gain), Distance(distance)
		{}
	};
	//@}
};


} // NLSOUND


#endif // NL_BACKGROUND_SOUND_MANAGER_H

/* End of background_sound_manager.h */
