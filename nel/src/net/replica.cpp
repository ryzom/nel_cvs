/** \file replica.cpp
 * <File description>
 *
 * $Id: replica.cpp,v 1.5 2001/01/19 14:17:03 cado Exp $
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

#include "nel/net/replica.h"
#include "nel/misc/vector.h"


using namespace NLMISC;


namespace NLNET {


/*
 * Constructor
 */
CReplica::CReplica( TEntityType t,
				    const NLMISC::CVector& pos,
					const NLMISC::CVector& hdg,
					const TAngle rollangle,
					const NLMISC::CVector& vec,
					const TAngVelocity av,
					bool groundmode ) :
	IMovingEntity( t, pos, hdg, rollangle, vec, av, groundmode )
{
}


/*
 * Alt. constructor with entity state
 */
CReplica::CReplica( const IMovingEntity& es ) :
	IMovingEntity( es )
{
}


/*
 * Change the current state
 */
void CReplica::changeStateTo( const IMovingEntity& es )
{
	if ( groundMode() )
	{
		CVector p = es.pos();
		CVector v = es.trajVector();
		CVector h = es.bodyHeading();
		p.z = pos().z;
		v.z = 0.0f; // the trajectory vector
		h.z = bodyHeading().z;
		setPos( p );
		setTrajVector( v );
		setBodyHeading( h );
	}
	/*if ( ! full3d() )
	{
		CVector p = es.pos();
		CVector v = es.trajVector();
		CVector h = es.bodyHeading();
		p.z = pos().z;
		v.z = trajVector().z;
		h.z = bodyHeading().z;
		setPos( p );
		setTrajVector( v );
		setBodyHeading( h );
	}*/
	else
	{
		setPos( es.pos() );
		setTrajVector( es.trajVector() );
		setBodyHeading( es.bodyHeading() );
	}
	/*
	setAngularVelocity( es.angularVelocity() );
	setRollAngle( es.rollAngle() );
	*/
}


} // NLNET
