/** \file env_sound_user.h
 * CEnvSoundUser: implementation of UEnvSound
 *
 * $Id: env_sound_user.h,v 1.1 2001/07/10 16:48:03 cado Exp $
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

#ifndef NL_ENV_SOUND_USER_H
#define NL_ENV_SOUND_USER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/sound/u_env_sound.h"
#include "source_user.h"
#include <vector>


namespace NLSOUND {


class CListenerUser;


// Number of environment channels
#define MAX_ENV_CHANNELS 3
#define AMBIANT_CH1 0
#define AMBIANT_CH2 1
#define SPARSE_CH 2


/**
 * Implementation of UEnvSound
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CEnvSoundUser : public UEnvSound
{
public:

	/// Constructor
	CEnvSoundUser();

	/// Destructor
	virtual					~CEnvSoundUser();

	
	/// Serialize
	void					serial( NLMISC::IStream& s );
	/// Serialize file header
	static void				serialFileHeader( NLMISC::IStream& s, uint32& nb );
	/// Load several envsounds and return the number of envsounds loaded
	static uint32			load( std::vector<CEnvSoundUser*>& container, NLMISC::IStream& s, CListenerUser *listener  );

	/// Return the corresponding localized 3D sound source
	virtual USource			*get3DSource()			{ return &_CenterSource; }
	/// Return the non-localized stereo sound source (TEMP)
	USource					*getStereoSources()		{ return _StereoChannels; }
	/// Return the inner radius
	float					getInnerRadius()		{ return _InnerRadius; }
	/// Return the outer radius
	float					getOuterRadius()		{ return _OuterRadius; }


	/// Set the listener
	void					setListener( CListenerUser *listener )	{ _Listener = listener; }
	/// Recompute the EnvSound. Call this method after having moved the 3D source.
	virtual void			recompute();
	/// Get the sources to play corresponding to the listener's position, set with the right volume
	void					getCurrentSources( const NLMISC::CVector& listenerpos,
											   bool& centersrc, bool& stereosrcs );
	/// Update the stereo mix (call evenly)
	void					update();


	/// Play
	virtual void			play();
	/// Stop playing
	virtual void			stop();
	/// Return the play flag
	bool					mustPlay() const		{ return _Play; }


	/// Set properties (EDIT)
	void					setProperties( CSourceUser& centersrc,
										   CSourceUser stchannels[MAX_ENV_CHANNELS],
										   float innerradius, float outerradius,
										   std::vector<TSoundId>& ambiantsounds,
										   std::vector<TSoundId>& sparsesounds,
										   uint32 crossfadeTimeMs=4000, uint32 sustainTimeMs=8000,
										   uint32 sparseAvgPeriodMs=20000 );
	/// Save (output stream only) (EDIT)
	static void				save( const std::vector<CEnvSoundUser>& container, NLMISC::IStream& s );

protected:

	/// Calc pos in cycle
	NLMISC::TTime			calcPosInCycle( bool& crossfade, uint32& leadchannel );
	/// Select a random sound in a bank
	TSoundId				getRandomSound( const std::vector<CSound*>& bank ) const;
	/// Calculate the next time a sparse sound plays (set NULL for no current sound)
	void					calcRandomSparseSoundTime( TSoundId currentsparesound );
	/// Start or stop the center source
	void					manageCenterSource( bool toplay );
	/// Start or stop the stereo channels
	void					manageStereoChannels( bool toplay, bool crossfade, uint32 leadchannel );

private:

	// Must play or not
	bool					_Play;

	// Localized 3D sound source
	CSourceUser				_CenterSource;
	CListenerUser			*_Listener;

	// Area
	float					_InnerRadius;
	float					_OuterRadius;

	// Non-localized stereo source
	CSourceUser				_StereoChannels [MAX_ENV_CHANNELS];
	float					_StereoGain;

	// Sound banks
	std::vector<CSound*>	_AmbiantSounds, _SparseSounds;

	// Crossfade control
	NLMISC::TTime			_StartTime;
	bool					_Sustain;
	bool					_RandomSoundChosen;
	NLMISC::TTime			_NextSparseSoundTime;

	// Constants
	uint32					_CrossfadeTime;
	uint32					_SustainTime;
	uint32					_SparseAvgPeriod;
};


} // NLSOUND


#endif // NL_ENV_SOUND_USER_H

/* End of env_sound_user.h */
