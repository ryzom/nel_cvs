/** \file local_entity.h
 * Locally-controlled entities
 *
 * $Id: local_entity.h,v 1.1 2000/10/23 14:18:51 cado Exp $
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
#include "nel/net/moving_entity.h"
#include "nel/net/remote_entity.h"


namespace NLNET {


/**
 * A moving entity that is locally controlled
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

	/// Sets dead reckoning threshold
	void			setThreshold( TPosUnit th )
	{
		_Threshold = th;
	}

	/// @name Input controls.
	//@{

	/// Sets the front velocity. Positive value for forward motion; negative value for backward motion.
	void			setFrontVelocity( TVelocity v );

	/// Sets the lateral velocity. Positive value for left motion; negative for right motion.
	void			setStrafeVelocity( TVelocity v );

	/// Yaws. Positive value for left rotation; negative for right rotation.
	void			yaw( TAngle delta );

	/// Rolls. Positive value for right rotation; negative value for left rotation.
	void			roll( TAngle delta );

	/// Pitches. Positive value for up rotation; negative value for down rotation.
	void			pitch( TAngle delta );

	//@}

protected:

	/// Computes trajectory vector
	void			computeVector();

	/// Sends update to all replicas, including local replica
	void			propagateState();

private:

	/// Front velocity
	TVelocity		_FrontVel;

	/// Strafe velocity
	TVelocity		_StrafeVel;

	// Dead reckoning replica
	CRemoteEntity	_Replica;

	// Threshold for dead reckoning divergence test
	TPosUnit		_Threshold;

};


} // NLNET


#endif // NL_LOCAL_ENTITY_H

/* End of local_entity.h */
