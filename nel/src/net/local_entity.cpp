/** \file local_entity.cpp
 * Locally-controlled entities
 *
 * $Id: local_entity.cpp,v 1.4 2000/10/27 15:45:06 cado Exp $
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

#include "nel/net/local_entity.h"
#include "nel/net/msg_socket.h"
#include "nel/misc/matrix.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"

using namespace NLMISC;

extern NLNET::CMsgSocket *ClientSocket;


namespace NLNET {



/*
 * Constructor
 */
CLocalEntity::CLocalEntity() :
	IMovingEntity(),
	_DRThresholdPos( 0.05 ),
	_FrontVel( 0.0 ),
	_StrafeVel( 0.0 ),
	_VertVel( 0.0 ),
	_DRReplica( *this )
{
}


/* Update the entity state.
 * The heading vector multiplied by the velocity is added to the current position.
 */
void CLocalEntity::update( TDuration deltatime )
{
	computePosAfterDuration( deltatime );
	_DRReplica.update( deltatime );

	// Compare the entity and its replica
	if ( drDivergeTest() )
	{
		nlinfo( "Pos: %f", (pos()-_DRReplica.pos()).norm() );
		propagateState();
	}
}



/*
 * Dead Reckoning divergence test: returns true if the replica needs to converge
 */
bool CLocalEntity::drDivergeTest()
{
	bool bh = false;
	if ( _DRTestBodyHeading )
	{
		// At the moment, test heading (orientation) divergence computing vector difference.
		// Because bodyHeading() is normed(), i.e. its norm is 1.0, if _DRThresholdHeading
		// equals 1.0, the corresponding angle is PI/3.
		// At the moment, the divergence test returns true if the orientation changes in
		// any direction, not only horizontally (for avatars such as persons, which are
		// always vertical, the appearance may not change when looking up/down).

		bh = ( (bodyHeading()-_DRReplica.bodyHeading()).norm() > _DRThresholdHeading );
	}

	// Position divergence test
	return bh || ( (pos()-_DRReplica.pos()).norm() > _DRThresholdPos );
}


/*
 * Sets dead reckoning threshold for heading divergence test (angle in radian)
 */
void CLocalEntity::setThresholdForHeading( TAngle a )
{
	_DRThresholdHeading = a * 3.0 / PI; // see drDivergenceTest()
}


/*
 * Computes trajectory vector
 */
void CLocalEntity::computeVector()
{
#define SQUARE_ANGLE PI/2.0
	CVector strafevect, vertvect;
	CMatrix m;

	// Lateral axis
	if ( _StrafeVel == 0.0 ) // TODO: Check float comparison
	{
		strafevect = CVector(0,0,0);
	}
	else
	{
		m.identity();
		m.rotateY( SQUARE_ANGLE ); // Y for the test, will be Z in NeL
		strafevect = (m * bodyHeading()) * _StrafeVel;
	}

	// Vertical axis
	if ( _VertVel == 0.0 )
	{
		vertvect = CVector(0,0,0);
	}
	else
	{
		m.identity();
		m.rotateX( SQUARE_ANGLE );
		vertvect = (m * bodyHeading()) * _VertVel;
	}

	// Add all three axis
	setTrajVector( (bodyHeading() * _FrontVel) + strafevect + vertvect );
}


/*
 * Sends update to all replicas, including local replica
 */
void CLocalEntity::propagateState()
{
	// Send
	CMessage msgout( "ES" );
	msgout.serial( *this );
	ClientSocket->send( msgout );
	nlinfo( "Entity State sent, with id %u", id() );

	// Update local replica
	_DRReplica.changeStateTo( *this );
}


/*
 * Sets front velocity
 */
void CLocalEntity::setFrontVelocity( TVelocity v )
{
	_FrontVel = v;
	computeVector();
}


/*
 * Sets lateral velocity (positive values correspond to the left)
 */
void CLocalEntity::setStrafeVelocity( TVelocity v )
{
	_StrafeVel = v;
	computeVector();
}


/*
 * Sets the vertical velocity. Positive value for up motion; negative for down motion.
 */
void CLocalEntity::setVerticalVelocity( TVelocity v )
{
	_VertVel = v;
	computeVector();
}


/*
 * Sets the angular velocity (yaw). A positive value means left turn, in radian per second.
 */
void CLocalEntity::setAngularVelocity( TAngVelocity v )
{
	setAngularVelocity( v );
}


//
void CLocalEntity::yaw( TAngle delta )
{
	CMatrix m;
	m.identity();
	m.rotateZ( delta ); // ? Y for the test, will be Z in NeL
	setBodyHeading( m * bodyHeading() );
	computeVector();
}


//
void CLocalEntity::roll( TAngle delta )
{
	CMatrix m;
	m.identity();
	m.rotateY( delta ); // -Z for the test, will be Y in NeL
	setBodyHeading( m * bodyHeading() );
	computeVector();
}


//
void CLocalEntity::pitch( TAngle delta )
{
	CMatrix m;
	m.identity();
	m.rotateX( delta );
	setBodyHeading( m * bodyHeading() );
	computeVector();
}


} // NLNET
