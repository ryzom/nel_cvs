/** \file local_entity.cpp
 * Locally-controlled entities
 *
 * $Id: local_entity.cpp,v 1.1 2000/10/23 14:18:51 cado Exp $
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

using namespace NLMISC;

//extern NLNET::CMsgSocket ClientSocket;


namespace NLNET {



/*
 * Constructor
 */
CLocalEntity::CLocalEntity() :
	IMovingEntity()
{
}


/* Update the entity state.
 * The heading vector multiplied by the velocity is added to the current position.
 */
void CLocalEntity::update( TDuration deltatime )
{
	computePosAfterDuration( deltatime );
	_Replica.update( deltatime );

	// Compare the entity and its replica
	if ( (pos()-_Replica.pos()).norm() > _Threshold )
	{
		propagateState();
	}
}


/*
 * Computes trajectory vector
 */
void CLocalEntity::computeVector()
{
	if ( _StrafeVel == 0.0 ) // TODO: Check float comparison
	{
		setTrajVector( bodyHeading() * _FrontVel );
	}
	else
	{
		CMatrix m;
		m.identity();
		m.rotateZ( PI/2.0 );
		setTrajVector( (bodyHeading() * _FrontVel) + ((m * bodyHeading()) * _StrafeVel) );
	}
}


/*
 * Sends update to all replicas, including local replica
 */
void CLocalEntity::propagateState()
{
	// Send
	CMessage msgout( "ES" );
	msgout.serial( *this );
	//ClientSocket.send( msgout );
	// Update local replica
	_Replica.changeStateTo( *this );
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


//
void CLocalEntity::yaw( TAngle delta )
{
	CMatrix m;
	m.identity();
	m.rotateZ( delta );
	setBodyHeading( m * bodyHeading() );
	setTrajVector( m * trajVector() ); // no need for a call to computerVector()
}


//
void CLocalEntity::roll( TAngle delta )
{
	CMatrix m;
	m.identity();
	m.rotateY( delta );
	setBodyHeading( m * bodyHeading() );
	setTrajVector( m * trajVector() ); // no need for a call to computerVector()
}


//
void CLocalEntity::pitch( TAngle delta )
{
	CMatrix m;
	m.identity();
	m.rotateX( delta );
	setBodyHeading( m * bodyHeading() );
	setTrajVector( m * trajVector() ); // no need for a call to computerVector()
}


} // NLNET
