/** \file source_user.h
 * CSourceUSer: implementation of USource
 *
 * $Id: source_common.h,v 1.2 2002/11/25 14:11:41 boucher Exp $
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

#ifndef NL_SOURCE_COMMON_H
#define NL_SOURCE_COMMON_H

#include "nel/misc/types_nl.h"
#include "nel/sound/u_source.h"
#include "sound.h"


namespace NLSOUND {

class CSourceCommon : public USource
{
public:
	
	enum TSOURCE_TYPE
	{
		SOURCE_SIMPLE,
		SOURCE_COMPLEX,
		SOURCE_BACKGROUND
	};

	CSourceCommon(TSoundId id, bool spawn, TSpawnEndCallback cb, void *cbUserParam);

	~CSourceCommon();


	/// Get the type the the source.
	virtual TSOURCE_TYPE	getType() const = 0;

	void					setPriority( TSoundPriority pr);
	/// Return the priority
	TSoundPriority			getPriority() const								{ return _Priority; }
	void					setLooping( bool loop );
	bool					getLooping() const;
	void					play();
	void					stop();
	bool					isPlaying()									{ return _Playing; }
	void					setPos( const NLMISC::CVector& pos ) ;
	const NLMISC::CVector	&getPos() const;
	void					setVelocity( const NLMISC::CVector& vel );
	void					setDirection( const NLMISC::CVector& dir );
	void					setGain( float gain );
	void					setRelativeGain( float gain );
	float					getRelativeGain() const;
	void					setSourceRelativeMode( bool mode );
	/// return the user param for the user callback
	void							*getCallbackUserParam(void) const			{ return _CbUserParam; }
	/// Tells this source not to call its callbacks when it ends. This is valid for spawned sources only.
	virtual	void					unregisterSpawnCallBack()					{ _SpawnEndCb = NULL; }
	/// Get the velocity vector
	virtual void					getVelocity( NLMISC::CVector& vel ) const	{ vel = _Velocity; }
	/// Get the direction vector
	virtual void					getDirection( NLMISC::CVector& dir ) const	{ dir = _Direction; }
	/// Get the gain
	virtual float					getGain() const								{ return _Gain; }
	/// Get the pitch
	virtual float					getPitch() const							{ return _Pitch; }
	/// Get the source relative mode
	virtual bool					getSourceRelativeMode() const				{ return _RelativeMode; }
	/// Set the position vector to return for a stereo source (default: NULL)
	void							set3DPositionVector( const NLMISC::CVector *pos )	{ _3DPosition = pos; }
	/// Return the spawn state
	bool							isSpawn() const								{ return _Spawn; }
	/** Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
	 * of one octave. 0 is not a legal value.
	 */
	virtual void					setPitch( float pitch );

	virtual uint32					getTime();



/*	/// Set looping on/off for future playbacks (default: off)
	virtual void					setLooping( bool l );
	/// Return the looping state
	virtual bool					getLooping() const;

	/// Tells this source not to call its callbacks when it ends. This is valid for spawned sources only.
	virtual	void					unregisterSpawnCallBack();


	/// \name Source properties
	//@{
*/	/** Set the position vector (default: (0,0,0)).
	 * 3D mode -> 3D position
	 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
	 */
/*	virtual void					setPos( const NLMISC::CVector& pos );
	/// Get the position vector (3D mode only)
	virtual void					getPos( NLMISC::CVector& pos ) const;
	/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
	virtual void					setVelocity( const NLMISC::CVector& vel );
	/// Get the velocity vector
	virtual void					getVelocity( NLMISC::CVector& vel ) const;
	/// Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
	virtual void					setDirection( const NLMISC::CVector& dir );
	/// Get the direction vector
	virtual void					getDirection( NLMISC::CVector& dir ) const;
*/	/** Set the gain (volume value inside [0 , 1]). (default: 1)
	 * 0.0 -> silence
	 * 0.5 -> -6dB
	 * 1.0 -> no attenuation
	 * values > 1 (amplification) not supported by most drivers
	 */
/*	virtual void					setGain( float gain );
	/// Get the gain
	virtual float					getGain() const;
*/	/** Set the gain amount (value inside [0, 1]) to map between 0 and the nominal gain
	 * (which is getSource()->getGain()). Does nothing if getSource() is null.
	 */
/*	virtual void					setRelativeGain( float gain );
	/// Return the relative gain (see setRelativeGain()), or the absolute gain if getSource() is null.
	virtual float					getRelativeGain() const;
*/	/** Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
	 * of one octave. 0 is not a legal value.
	 */
/*	virtual void					setPitch( float pitch );
	/// Get the pitch
	virtual float					getPitch() const;
	/// Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
	virtual void					setSourceRelativeMode( bool mode );
	/// Get the source relative mode
	virtual bool					getSourceRelativeMode() const;
	//@}
*/
protected:
	// Dynamic properties
	TSoundPriority					_Priority;
	bool							_Playing;
	bool							_Looping;

	NLMISC::CVector					_Position;
	NLMISC::CVector					_Velocity;
	NLMISC::CVector					_Direction;
	float							_Gain;
	float							_Pitch;
	bool							_RelativeMode;

	/// Gain not affected by setRelativeGain and used to compute _Gain.
	float							_InitialGain;

	// Playing start time
	NLMISC::TTime					_PlayStart;

	// Position to return, for a stereo source
	const NLMISC::CVector			*_3DPosition;

	// Spawn state
	const bool						_Spawn;
	TSpawnEndCallback				_SpawnEndCb;
	void							*_CbUserParam;
};

} // NLSOUND


#endif //NL_SOURCE_COMMON_H