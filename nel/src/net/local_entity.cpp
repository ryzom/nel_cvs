/** \file local_entity.cpp
 * Locally-controlled entities
 *
 * $Id: local_entity.cpp,v 1.20 2001/01/03 16:38:20 cado Exp $
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
#include "nel/net/local_area.h"
#include "nel/net/msg_socket.h"
#include "nel/misc/matrix.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"

using namespace NLMISC;


namespace NLNET {



/*
 * Constructor
 */
CLocalEntity::CLocalEntity() :
	IMovingEntity(),
	_FrontVel( 0.0 ),
	_StrafeVel( 0.0 ),
	_VertVel( 0.0 ),
	_DeltaTime( 0 ),
	_DRThresholdPos( 0.5 ),
	_DRTestBodyHeading( true ),
	_DRThresholdHeading( 0.5 ),
	ViewPitch( 0.0 ),
	ViewRoll( 0.0 )
{
	_DRReplica = *this;
}


/*
 * Alt. constructor with entity state
 */
CLocalEntity::CLocalEntity( const IMovingEntity& es ) :
	IMovingEntity( es ),
	_FrontVel( 0.0 ),
	_StrafeVel( 0.0 ),
	_VertVel( 0.0 ),
	_DeltaTime( 0 ),
	_DRThresholdPos( 0.5 ),
	_DRTestBodyHeading( true ),
	_DRThresholdHeading( 0.5 ),
	ViewPitch( 0.0 ),
	ViewRoll( 0.0 )
{
	_DRReplica = *this;
}


/* Update the entity state.
 * The heading vector multiplied by the velocity is added to the current position. Don't forget
 * to call commitPos() after update to propagate the entity state.
 */
void CLocalEntity::update( TDuration deltatime )
{
	// Local entity
	_DeltaTime = deltatime;
	IMovingEntity::update( deltatime );

	// Local replica
	_DRReplica.update( deltatime );

	/* Now this is done in commitPos()
	// Compare the entity and its replica
	if ( drDivergeTest() )
	{
		//nlinfo( "Pos: %f", (pos()-_DRReplica.pos()).norm() );
		propagateState();
	}*/
}


// Distance ignoring Z
TPosUnit distanceXY( const CVector& p1, const CVector& p2 )
{
	TPosUnit x = p2.x - p1.x;
	TPosUnit y = p2.y - p1.y;
	return (TPosUnit)sqrt( x*x + y*y );
}


/*
 * Dead Reckoning divergence test: returns true if the replica needs to converge
 */
bool CLocalEntity::drDivergeTest()
{
	bool bh = false;

	// At the moment, test heading (orientation) divergence computing vector difference.
	// Because bodyHeading() is normed(), i.e. its norm is 1.0, if _DRThresholdHeading
	// equals 1.0, the corresponding angle is PI/3.
	if ( groundMode() )
	{
		if ( _DRTestBodyHeading )
		{
			bh = distanceXY( bodyHeading(), _DRReplica.bodyHeading() ) > _DRThresholdHeading;
		}
		// Position divergence test
		return bh || ( distanceXY( pos(), _DRReplica.pos() ) > _DRThresholdPos );
	}
	else
	{
		if ( _DRTestBodyHeading )
		{
			bh = ( (bodyHeading()-_DRReplica.bodyHeading()).norm() > _DRThresholdHeading );
		}
		// Position divergence test
		return bh || ( (pos()-_DRReplica.pos()).norm() > _DRThresholdPos );
	}
	
	/*CVector p1 = pos();
	CVector p2 = _DRReplica.pos();
	TPosUnit diff = (p1-p2).norm();
	bh = diff > _DRThresholdPos;
	if ( bh )
	{
		return true;
	}*/
	return bh;
}


/*
 * Sets dead reckoning threshold for heading divergence test (angle in radian)
 */
void CLocalEntity::setThresholdForHeading( TAngle a )
{
	_DRThresholdHeading = a * 3.0f / (float)Pi; // see drDivergenceTest()
}


/*
 * Computes trajectory vector
 */
void CLocalEntity::computeVector()
{
#define SQUARE_ANGLE (Pi/2.0)
	CVector strafevect, vertvect;
	CMatrix m;

	// Lateral axis
	if ( _StrafeVel == 0.0 ) // TODO: Check float comparison
	{
		strafevect = CVector(0,0,0);
	}
	else
	{
		if ( groundMode() )
		{
			m.identity();
			m.rotateZ( (float)SQUARE_ANGLE ); // Strafe: rotate around the vertical axis
			strafevect = (m * bodyHeading()) * _StrafeVel;
		}
		else
		{
			strafevect = (CVector(0,0,1) ^ bodyHeading()) * _StrafeVel;
		}
	}

	// Vertical axis
	vertvect = CVector(0,0,_VertVel);

	// Add all three axis
	setTrajVector( (bodyHeading() * _FrontVel) + strafevect + vertvect );
}


/*
 * Sends update to all replicas, including local replica
 */
void CLocalEntity::propagateState()
{
	// Send
	if ( (CLocalArea::Instance->ClientSocket != NULL) && CLocalArea::Instance->ClientSocket->connected() && (id() != 0) )
	{
		CMessage msgout;
		if ( groundMode() )
		{
			msgout.setType( "GES" ); // Entity State, Ground mode
		}
		else
		{
			msgout.setType( "FES" ); // Full Entity State
		}
		msgout.serial( *this );
		CLocalArea::Instance->ClientSocket->send( msgout );
		nlinfo( "Entity State sent, with id %u", id() );
		if ( ! groundMode() )
		{
			setGroundMode( true ); // enter ground mode after sending a full entity state
		}
	}

	// Update local replica
	_DRReplica.changeStateTo( *this );
}


/* Corrects the entity position (and updates trajectory vector) using external information
 * such as collision detection. The dead reckoning diverge test and state propagation are then done.
 * Usage :
 * -# Update the entity
 * -# Submit the new pos to the landscape (for example)
 * -# Commit the position.
 */
void CLocalEntity::commitPos( const NLMISC::CVector& p )
{
	if ( p != pos() )
	{
		// Compute trajectory vector
		setTrajVector( (p-previousPos())/_DeltaTime );
		setPos( p );
	}

	// Compare the entity and its replica
	if ( drDivergeTest() )
	{
		//nlinfo( "Pos: %f", (pos()-_DRReplica.pos()).norm() );
		propagateState();
	}
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
	IMovingEntity::setAngularVelocity( v );
}


//
void CLocalEntity::yaw( TAngle delta )
{
	CMatrix m;
	m.identity();
	if ( groundMode() )
	{
		m.rotateZ( delta );
	}
	else
	{
		nlerror( "Not implemented" );
	}
	setBodyHeading( (m * bodyHeading()).normed() );
	computeVector();
}


//
void CLocalEntity::roll( TAngle delta )
{
	setRollAngle( rollAngle() + delta );
}


//
void CLocalEntity::pitch( TAngle delta )
{
	/*CVector v1 = CVector(0,0,1) ^ _BodyHdg;
	CVector v2 = _BodyHdg ^ v1;*/

	CMatrix m;
	m.identity();
	m.rotateX( delta ); // TODO

	//nlerror( "Not implemented" );

	setBodyHeading( m * bodyHeading() );
	computeVector();
}


} // NLNET
