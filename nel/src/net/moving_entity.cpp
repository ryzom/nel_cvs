/** \file moving_entity.cpp
 * Interface for all moving entities
 *
 * $Id: moving_entity.cpp,v 1.4 2000/10/27 15:45:07 cado Exp $
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

using namespace NLMISC;


namespace NLNET {


TEntityId IMovingEntity::_MaxId = 1; //	avoid 0


/*
 * Constructor
 */
IMovingEntity::IMovingEntity() :
	_AngVel( 0.0f )
{
	_Id = 0; //getNewId();
}


/*
 * Alt. constructor
 */
IMovingEntity::IMovingEntity( const NLMISC::CVector pos,
							  const NLMISC::CVector hdg,
							  const NLMISC::CVector vec,
							  const TAngVelocity av )
{
	_Id = 0; //getNewId();
	_Pos = pos;
	_BodyHdg = hdg;
	_Vector = vec;
	_AngVel = av;
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
	return atan2( _BodyHdg.y, _BodyHdg.x );
}



/*
 * Angle around x axis from y axis
 */
TAngle IMovingEntity::angleAroundX()
{
	return atan2( _BodyHdg.z, _BodyHdg.y );
}



/*
 * Angle around y axis from z axis
 */
TAngle IMovingEntity::angleAroundY()
{
	return atan2( _BodyHdg.x, _BodyHdg.z );
}


/*
 * Computes position using heading and velocity
 */
void IMovingEntity::computePosAfterDuration( TDuration d )
{
	// Compute position
	_Pos += _Vector * d;

	// Compute body heading if angular velocity used
	if ( _AngVel != 0.0 ) // check float comparison to zero
	{
		CMatrix m;
		m.identity();
		m.rotateZ( _AngVel * d ); // ? Y for the test, will be Z in NeL
		setBodyHeading( m * bodyHeading() );
	}
}




/*
 * Serialization
 */
void IMovingEntity::serial ( NLMISC::IStream &s )
{
	s.serial( _Id );
	s.serial( _Pos );
	s.serial( _BodyHdg );
	s.serial( _Vector );
}


} // NLNET
