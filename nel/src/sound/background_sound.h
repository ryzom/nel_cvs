/** \file background_sound.h
 *
 * $Id: background_sound.h,v 1.1 2002/11/04 15:40:43 boucher Exp $
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

#ifndef NL_BACKGROUND_SOUND_H
#define NL_BACKGROUND_SOUND_H

#include "sound.h"

namespace NLSOUND {

//class ISoundDriver;
//class IBuffer;
//class CSound;



/**
 * A background sound static properties
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2002
 */
class CBackgroundSound : public CSound
{
public:
	/// Constructor
	CBackgroundSound();
	/// Destructor
	~CBackgroundSound();

	TSOUND_TYPE getSoundType()							{ return SOUND_BACKGROUND;}

	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// Return true if cone is meaningful
	bool				isDetailed() const				{ return false;}
	/// Return the length of the sound in ms
	virtual uint32		getDuration(std::string *buffername = NULL);
	/// Return the name (must be unique)

	/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const;

	virtual float		getMaxDistance() const;




	/** Container for background filter.
	 *	If flag is not set, then the sound cannot be played when the environnement
	 *	status is on.
	 *	Ex : if Night is not set, then the sound is not played during the night.
	 */
/*	struct TBackgroundFilter
	{
		bool	Day;
		bool	Night;
		bool	Spring;
		bool	Summer;
		bool	Autumn;
		bool	Winter;
		bool	Sunny;
		bool	Cloudy;
		bool	Foggy;
		bool	Rainy;
		bool	Snow;
		bool	Storm;
		bool	BadAtmosphere;
		bool	GoodAtmosphere;
	};
*/
	/// Associtation clas for storage of sound / filter.
	struct TSoundInfo
	{
		std::string			SoundName;
		/// Fade in length (ms) after environnement filter end. 
		uint32				FilterFadeIn;
		/// Fade out length (ms) after environnement filter start.
		uint32				FilterFadeOut;
		UAudioMixer::TBackgroundFlags		Filter;
	};

	const std::vector<TSoundInfo> &getSounds() const			{ return _Sounds;}

private:


	/// The list of sound composing the background sound with there repective filter.
	std::vector<TSoundInfo>	_Sounds;
	

	// Duration of sound.
	uint32						_Duration;
	// flag for validity of duration (after first evaluation).
	bool						_DurationValid;
};

} // NLSOUND

#endif // NL_BACKGROUND_SOUND_H

