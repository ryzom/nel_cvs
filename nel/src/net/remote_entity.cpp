/** \file remote_entity.cpp
 * Remote-controlled entities
 *
 * $Id: remote_entity.cpp,v 1.8 2000/11/27 16:26:45 cado Exp $
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

#include "nel/net/remote_entity.h"
#include "nel/net/replica.h"
#include "nel/misc/debug.h"

using namespace NLMISC;


namespace NLNET {


/// Converge duration constant
const TDuration CRemoteEntity::ConvergeDuration = 0.5;


/*
 * Constructor
 */
CRemoteEntity::CRemoteEntity( const NLMISC::CVector& pos,
							  const NLMISC::CVector& hdg,
							  const TAngle rollangle,
							  const NLMISC::CVector& vec,
							  const TAngVelocity av,
							  bool groundmode ) :
	CReplica( pos, hdg, rollangle, vec, av, groundmode )
{
}


/*
 * Alt. constructor with entity state
 */
CRemoteEntity::CRemoteEntity( const IMovingEntity& es ) :
	CReplica( es )
{
}


/*
 * Update the entity state
 */
void CRemoteEntity::update( TDuration deltatime )
{
	if ( _Interpolator.active() )
	{
		_Interpolator.getNextState( *this, deltatime );
	}
	else
	{
		computePosAfterDuration( deltatime );
	}
}


/*
 * Converge to the specified state
 */
void CRemoteEntity::convergeTo( const IMovingEntity& dest_es )
{
	CReplica extrapolated_dest( *this );
	extrapolated_dest.changeStateTo( dest_es );
	extrapolated_dest.update( CRemoteEntity::ConvergeDuration );
	_Interpolator.begin( *this, extrapolated_dest, CRemoteEntity::ConvergeDuration );
	//changeStateTo( dest_es );
}


} // NLNET
