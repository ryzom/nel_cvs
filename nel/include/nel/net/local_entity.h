/** \file local_entity.h
 * Locally-controlled entities
 *
 * $Id: local_entity.h,v 1.4 2000/11/07 16:44:44 cado Exp $
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
#include "nel/net/remote_entity.h"


namespace NLNET {


/**
 * A moving entity that is locally controlled
 * 
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

	/** Update the entity state.
	 * The heading vector multiplied by the velocity is added to the current position.
	 */
	void			update( TDuration deltatime );

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

	/** Corrects the entity position (and updates trajectory vector) using external information
	 * such as collision detection.
	 * Usage :
	 * -# Update the entity
	 * -# Submit the new pos to the landscape (for example)
	 * -# Correct the position.
	 */
	void			correctPos( const NLMISC::CVector& p );

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

	/// @name Input controls.
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

protected:

	/// Computes trajectory vector. \todo Cado: rotateZ() for NeL instead of rotateY() for GLTest (also in yaw() and roll())
	void			computeVector();

	/// Sends update to all replicas, including local replica
	void			propagateState();

	/// Dead reckoning divergence test: returns true if the replica needs to converge
	bool			drDivergeTest();

	/// R/W Access to dead reckoning replica
	CRemoteEntity&	drReplica()
	{
		return _DRReplica;
	}

private:

	/// Front velocity
	TVelocity		_FrontVel;

	/// Strafe velocity
	TVelocity		_StrafeVel;

	/// Vertical velocity;
	TVelocity		_VertVel;

	/// Previous position (that was valid before the latest update)
	NLMISC::CVector	_PrevPos;

	/// Delta time that was passed to update()
	TDuration		_DeltaTime;

///@name Dead Reckoning properties
//@{

	// Dead reckoning replica
	CRemoteEntity	_DRReplica;

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
