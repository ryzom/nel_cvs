/** \file source_user.h
 * CSourceUSer: implementation of USource
 *
 * $Id: background_source.h,v 1.1 2002/11/04 15:40:43 boucher Exp $
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

#ifndef NL_BACKGROUND_SOURCE_H
#define NL_BACKGROUND_SOURCE_H

#include "nel/misc/types_nl.h"
//#include "nel/sound/u_source.h"
#include "source_common.h"
#include "background_sound.h"


namespace NLSOUND {

class CBackgroundSound;

/** Implemetation class for background source.
 *	Complex source are source that use a CPatternSound object.
 * \author Boris Boucher.
 * \author Nevrax
 */
class CBackgroundSource : public CSourceCommon, public CAudioMixerUser::IMixerEvent, public CAudioMixerUser::IMixerUpdate
{
public:
	/// Constructor
	CBackgroundSource	(CBackgroundSound *backgroundSource=NULL, bool spawn=false, TSpawnEndCallback cb=0, void *cbUserParam = 0, CSoundContext *context=0);
	/// Destructor
	~CBackgroundSource	();

	/// Change the sound binded to the source
	void							setSound( TSoundId id, CSoundContext *context = 0);
	/// Return the sound binded to the source (or NULL if there is no sound)
	virtual TSoundId				getSound();

	virtual void					play();
	/// Stop playing
	virtual void					stop();

	TSOURCE_TYPE					getType() const								{return SOURCE_BACKGROUND;}

	void							setGain( float gain );
	void							setRelativeGain( float gain );

	void							setPos( const NLMISC::CVector& pos );
	void							setVelocity( const NLMISC::CVector& vel );
	void							setDirection( const NLMISC::CVector& dir );


private:

	/// Check the given filter agains current environnment status, return false is masking.
	bool	checkFilter(const UAudioMixer::TBackgroundFlags &filter);

	/// Mixer update implementation (for fade in/out).
	void onUpdate();
	/// Mixer event for env checking.
	void onEvent();

	/// Sub source possible status.
	enum TSubSourceStatus
	{
		/// The sub source is playing.
		SUB_STATUS_PLAY,
		/// The sub source is stopped : it is mask by environnemt status.
		SUB_STATUS_STOP,
		/// the sub source is being faded out then stop (masked by filter).
		SUB_STATUS_FADEOUT,
		/// The sub source is being faded in, then play (unmasked by filter).
		SUB_STATUS_FADEIN
	};

	/// Sub source info.
	struct TSubSource
	{
		/// Sub source instance.
		USource				*Source;
		/// Sub source status.
		TSubSourceStatus	Status;
		/// Sub source fade factor (when fading).
		NLMISC::TTime		FadeStart;
		/// Fade in length (ms) after environnement filter end. 
		uint32				FilterFadeIn;
		/// Fade out length (ms) after environnement filter start.
		uint32				FilterFadeOut;
		/// Sub source filter.
		UAudioMixer::TBackgroundFlags	Filter;
	};

	/// The sound static data.
	CBackgroundSound			*_BackgroundSound;

	/// The sub sources container.
	std::vector<TSubSource>		_Sources;

	/// Last time we check env variable.
	NLMISC::TTime				_LastCheck;
};



} // NLSOOUND

#endif // NL_BACKGROUND_SOURCE_H