/** \file moving_entity.cpp
 * Interface for all moving entities
 *
 * $Id: moving_entity.cpp,v 1.10 2000/12/19 16:06:09 cado Exp $
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

#include "nel/net/moving_entity.h"
#include "nel/misc/matrix.h"
#include "nel/misc/debug.h"

using namespace NLMISC;


namespace NLNET {


TEntityId IMovingEntity::_MaxId = 1000001; //	avoid 0 and ids allocated by the LS


/*
 * Constructor
 */
IMovingEntity::IMovingEntity() :
	_Id( 0 ),
	_EntityType( 0 ),
	_Pos( 0,0,0 ),
	_BodyHdg( 0,1,0 ),
	_RollAngle( 0.0f ),
	_Vector( 0,0,0 ),
	_AngVel( 0.0f ),
	_GroundMode( true ),
	_PrevPos( 0,0,0 ),
	Tag( 0 )
{
}


/*
 * Alt. constructor
 */
IMovingEntity::IMovingEntity( TEntityType t,
							  const NLMISC::CVector& pos,
							  const NLMISC::CVector& hdg,
							  const TAngle rollangle,
							  const NLMISC::CVector& vec,
							  const TAngVelocity av,
							  bool groundmode ) :
	_Id( 0 ),
	_EntityType( t ),
	_Pos( pos ),
	_BodyHdg( hdg ),
	_RollAngle( rollangle ),
	_Vector( vec ),
	_AngVel( av ),
	_GroundMode( groundmode ),
	_PrevPos( 0,0,0) ,
	Tag( 0 )
{
}


/*
 * Copy constructor
 */
IMovingEntity::IMovingEntity( const IMovingEntity& other )
{
	operator=( other );
}


/*
 * Angle around z axis from x axis
 */
TAngle IMovingEntity::angleAroundZ()
{
	return (TAngle)atan2( _BodyHdg.y, _BodyHdg.x );
}



/*
 * Angle around x axis from y axis
 */
TAngle IMovingEntity::angleAroundX()
{
	return (TAngle)atan2( _BodyHdg.z, _BodyHdg.y );
}



/*
 * Angle around y axis from z axis
 */
TAngle IMovingEntity::angleAroundY()
{
	return (TAngle)atan2( _BodyHdg.x, _BodyHdg.z );
}


/*
 *Returns the rotations of the body around the three axes
 */
CVector IMovingEntity::bodyRot()
{
	// This works only because the norm of _BodyHdg is 1
	return CVector( (float)asin( _BodyHdg.z ), (float)asin( _BodyHdg.x ), (float)asin( _BodyHdg.y ) );
}


/*
 * Sets altitude (ground mode only)
 */
void IMovingEntity::setAltitude( TPosUnit z )
{
	nlassert( _GroundMode );
	_Pos.z = z;
}


/*
 * Computes position using heading and velocity
 */
void IMovingEntity::computePosAfterDuration( TDuration d )
{
	setPreviousPos( _Pos );

	// Compute position
	_Pos += _Vector * d;

	// Compute body heading if angular velocity used
	if ( _AngVel != 0.0 ) // check float comparison to zero
	{
		CMatrix m;
		m.identity();
		if ( groundMode() )
		{
			m.rotateZ( _AngVel * d ); // horizontal rotation
		}
		else
		{
			nlerror( "Not implemented" );
		}
		setBodyHeading( m * bodyHeading() );
	}
}




/*
 * Serialization
 */
void IMovingEntity::serial ( NLMISC::IStream &s )
{
	if ( groundMode() )
	{
		s.serial( _Id );
		s.serial( _EntityType );
		s.serial( _Pos.x );
		s.serial( _Pos.y );
		s.serial( _Vector.x );
		s.serial( _Vector.y );
		s.serial( _BodyHdg.x );
		s.serial( _BodyHdg.y );
		s.serial( _AngVel );
		s.serial( _RollAngle );
	}
	else
	{
		s.serial( _Id );
		s.serial( _EntityType );
		s.serial( _Pos );
		s.serial( _Vector );
		s.serial( _BodyHdg );
		s.serial( _AngVel );
		s.serial( _RollAngle );
	}
}


} // NLNET
