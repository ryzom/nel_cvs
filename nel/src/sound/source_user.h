/** \file source_user.h
 * CSourceUSer: implementation of USource
 *
 * $Id: source_user.h,v 1.14 2002/07/25 13:35:10 lecroart Exp $
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

#ifndef NL_SOURCE_USER_H
#define NL_SOURCE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/sound/u_source.h"
#include "nel/misc/vector.h"
#include "nel/misc/time_nl.h"
#include "playable.h"


namespace NLSOUND {


class ISource;
class CTrack;
class CSound;


/**
 * Implementation of USource
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CSourceUser : public USource, public IPlayable
{
public:

	/// Constructor
	CSourceUser( TSoundId id=NULL, bool spawn=false, TSpawnEndCallback cb=0, void *cbUserParam = 0, CSoundContext *context=0 );
	/// Destructor
	virtual ~CSourceUser();

	/// Static init (call at the very beginning)
	static void						init() 
	{ 
		//NLMISC_REGISTER_CLASS(CSourceUser); 
	}

	
	/// Change the sound binded to the source
	virtual void					setSound( TSoundId id, CSoundContext *context = 0 );
	/// Return the sound binded to the source (or NULL if there is no sound)
	virtual TSoundId				getSound()									{ return _Sound; }
	/// Change the priority of the source
	virtual void					setPriority( TSoundPriority pr, bool redispatch=true );
	/// Return the priority
	TSoundPriority					getPriority()								{ return _Priority; }

	/// \name Playback control
	//@{
	/// Set looping on/off for future playbacks (default: off)
	virtual void					setLooping( bool l );
	/// Return the looping state
	virtual bool					getLooping() const;
	/// Play
	virtual void					play();
	/// Stop playing
	virtual void					stop();
	/// Get playing state. Return false even if the source has stopped on its own.
	virtual bool					isPlaying();
	/// Tells this source not to call its callbacks when it ends. This is valid for spawned sources only.
	virtual	void					unregisterSpawnCallBack()					{ _SpawnEndCb = NULL; }
	///
	virtual NLMISC::TTicks			getPlayTime()								{ return NLMISC::CTime::getLocalTime() - _PlayStart; }
	//@}


	/// \name Source properties
	//@{
	/** Set the position vector (default: (0,0,0)).
	 * 3D mode -> 3D position
	 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
	 */
	virtual void					setPos( const NLMISC::CVector& pos );
	/** Get the position vector.
	 * If the source is stereo, return the position vector which reference was passed to set3DPositionVector()
	 */
	virtual void					getPos( NLMISC::CVector& pos ) const;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void					setVelocity( const NLMISC::CVector& vel );
	/// Get the velocity vector
	virtual void					getVelocity( NLMISC::CVector& vel ) const	{ vel = _Velocity; }
	/// Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
	virtual void					setDirection( const NLMISC::CVector& dir );
	/// Get the direction vector
	virtual void					getDirection( NLMISC::CVector& dir ) const	{ dir = _Direction; }
	/** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
	virtual void					setGain( float gain );
	/// Get the gain
	virtual float					getGain() const								{ return _Gain; }
	/** Set the gain amount (value inside [0, 1]) to map between 0 and the nominal gain
	 * (which is getSource()->getGain()). Does nothing if getSource() is null.
	 */
	virtual void					setRelativeGain( float gain );
	/// Return the relative gain (see setRelativeGain()), or the absolute gain if getSource() is null.
	virtual float					getRelativeGain() const;
	/** Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
	 * of one octave. 0 is not a legal value.
	 */
	virtual void					setPitch( float pitch );
	/// Get the pitch
	virtual float					getPitch() const							{ return _Pitch; }
	/// Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
	virtual void					setSourceRelativeMode( bool mode );
	/// Get the source relative mode
	virtual bool					getSourceRelativeMode() const				{ return _RelativeMode; }


	/// Set the position vector to return for a stereo source (default: NULL)
	void							set3DPositionVector( const NLMISC::CVector *pos )	{ _3DPosition = pos; }
	/// Return a pointer to the position vector (3D mode only)
	const NLMISC::CVector			*getPositionP() const						{ return &_Position; }
	/** Set the corresponding track	(NULL allowed, sets no track)
	 * Don't set a non-null track if getSound() is null.
	 */
	void							enterTrack( CTrack *track );
	/// Unset the corresponding track
	void							leaveTrack();
	/// Return the track
	CTrack							*getTrack()									{ return _Track; }
	/// Return true if playing is finished or stop() has been called.
	bool							isStopped();
	/// Return the spawn state
	bool							isSpawn() const								{ return _Spawn; }
	/// Return the spawn end callback
	TSpawnEndCallback				getSpawnEndCallback() const					{ return _SpawnEndCb; }
	/// return the user param for the user callback
	void							*getCallbackUserParam(void) const			{ return _CbUserParam; }

	// From IPlayable

	/// Enable (play with high priority) or disable (stop and set low priority)
	virtual void					enable( bool toplay, float gain );
	/// Move (set position)
	virtual void					moveTo( const NLMISC::CVector& pos )		{ setPos( pos ); }
	/// Serial sound and looping state (warning: partial serial)
	//virtual void					serial( NLMISC::IStream& s );

	//NLMISC_DECLARE_CLASS(CSourceUser);

	virtual IBuffer					*getBuffer();
	
protected:

	/// Copy the source data into the corresponding track (_Track must be not null)
	void							copyToTrack();

private:

	// These data are copied to a track when the source selected is for playing

	CSound							*_Sound;
	// name of a buffer get by getBuffer()
	std::string						_Buffername;
	TSoundPriority					_Priority;
	bool							_Playing;
	bool							_Looping;

	NLMISC::CVector					_Position;
	NLMISC::CVector					_Velocity;
	NLMISC::CVector					_Direction;
	float							_Gain;
	float							_Pitch;
	bool							_RelativeMode;

	// Corresponding track (if selected for playing)
	CTrack							*_Track;

	// Position to return, for a stereo source
	const NLMISC::CVector			*_3DPosition;

	// Playing start time
	NLMISC::TTime					_PlayStart;

	// Spawn state
	const bool						_Spawn;
	TSpawnEndCallback				_SpawnEndCb;
	void							*_CbUserParam;
};


} // NLSOUND


#endif // NL_SOURCE_USER_H

/* End of source_user.h */
