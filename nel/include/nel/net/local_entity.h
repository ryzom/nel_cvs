/** \file local_entity.h
 * Locally-controlled entities
 *
 * $Id: local_entity.h,v 1.12 2000/12/22 13:46:16 cado Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_LOCAL_ENTITY_H
#define NL_LOCAL_ENTITY_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/net/moving_entity.h"
#include "nel/net/replica.h"


namespace NLNET {


/**
 * A moving entity that is locally controlled.
 * This class represents a user.
 * Motion (e.g. vertical motion) does not work properly yet
 * \warning This class is test code and is highly subject to change.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CLocalEntity : public IMovingEntity
{
public:

	/// Constructor
	CLocalEntity();

	/// Alt. constructor with entity state
	CLocalEntity( const IMovingEntity& es );

	/** Update the entity state.
	 * The heading vector multiplied by the velocity is added to the current position. Don't forget
	 * to call commitPos() after update to propagate the entity state.
	 */
	void			update( TDuration deltatime );

	/** Corrects the entity position (and updates trajectory vector) using external information
	 * such as collision detection. The dead reckoning diverge test and state propagation are then done.
	 * Usage :
	 * -# Update the entity
	 * -# Submit the new pos to the landscape (for example)
	 * -# Commit the position.
	 */
	void			commitPos( const NLMISC::CVector& p );

	/* Sends an update to all replicas, including local replica.
	 * Call it yourself only if you explicitly want to send an update
	 * (e.g. just after the creation and setup of the local entity, or when the user
	 * begins or stops a movement)
	 */
	void			propagateState();

	///	Sets the initial position
	void			resetPos( const NLMISC::CVector& p )
	{
		setPos( p );
	}

	/// Sets the initial heading
	void			resetBodyHeading( const NLMISC::CVector& hdg )
	{
		setBodyHeading( hdg );
	}

	/// Sets dead reckoning threshold for position divergence test
	void			setThresholdForPos( TPosUnit th )
	{
		_DRThresholdPos = th;
	}

	/// Sets if heading must be included in dead reckoning divergence test
	void			setTestBodyHeading( bool t )
	{
		_DRTestBodyHeading = t;
	}

	/// Sets dead reckoning threshold for heading divergence test (angle in radian)
	void			setThresholdForHeading( TAngle a );

	/// @name Input controls
	//@{

	/// Sets the front velocity. Positive value for forward motion; negative value for backward motion.
	void			setFrontVelocity( TVelocity v );

	/// Sets the lateral velocity. Positive value for left motion; negative for right motion.
	void			setStrafeVelocity( TVelocity v );

	/// Sets the vertical velocity. Positive value for up motion; negative for down motion.
	void			setVerticalVelocity( TVelocity v );

	/// Sets the angular velocity (yaw). A positive value means left turn, in radian per second.
	/// You can use either this method or yaw(), whether the rotation is atomic or not.
	void			setAngularVelocity( TAngVelocity v );

	/// Yaws. Positive value for left rotation; negative for right rotation.
	void			yaw( TAngle delta );

	/// Rolls. Positive value for right rotation; negative value for left rotation.
	void			roll( TAngle delta );

	/** Pitches. Positive value for up rotation; negative value for down rotation.
	 * This is not just a head pitching.
	 */
	void			pitch( TAngle delta );

	//@}

	/// @name Read accessors

	//@{

	TVelocity		frontVelocity() const	{ return _FrontVel; }
	TVelocity		strafeVelocity() const	{ return _StrafeVel; }
	TVelocity		vertVelocity() const	{ return _VertVel; }

	const CReplica&	drReplica() const		{ return _DRReplica; }

	//@}

	/// Pitch of the camera (or of the head)
	TAngle			ViewPitch;

	/// Roll of the camera (or of the head)
	TAngle			ViewRoll;

protected:

	/// Computes trajectory vector.
	void			computeVector();

	/// Dead reckoning divergence test: returns true if the replica needs to converge
	bool			drDivergeTest();

private:

	/// Front velocity
	TVelocity		_FrontVel;

	/// Strafe velocity
	TVelocity		_StrafeVel;

	/// Vertical velocity;
	TVelocity		_VertVel;

	/// Delta time that was passed to update()
	TDuration		_DeltaTime;

///@name Dead Reckoning properties
//@{

	// Dead reckoning replica
	CReplica		_DRReplica;

	// Threshold for dead reckoning divergence test on position
	TPosUnit		_DRThresholdPos;

	// True if the body heading must be included in divergence test
	bool			_DRTestBodyHeading;

	// Threshold for dead reckoning divergence test on heading (see also drDivergenceTest())
	TPosUnit		_DRThresholdHeading;

//@}

};


}// NLNET


#endif // NL_LOCAL_ENTITY_H

/* End of local_entity.h */
