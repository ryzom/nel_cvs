/** \file replica.h
 * A moving entity that is controlled from elsewhere
 *
 * $Id: replica.h,v 1.3 2000/11/30 17:03:10 cado Exp $
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

#ifndef NL_REPLICA_H
#define NL_REPLICA_H

#include "nel/misc/types_nl.h"
#include "nel/net/moving_entity.h"


namespace NLNET {


/**
 * A moving entity that is controlled from elsewhere.
 * All you can do is create a replica, update its state, and change it by copying an entity state.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CReplica : public IMovingEntity
{
public:

	/// Constructor
	CReplica() : IMovingEntity() {}

	/// Alt. constructor
	CReplica( TEntityType t,
			  const NLMISC::CVector& pos,
			  const NLMISC::CVector& hdg,
			  const TAngle rollangle,
			  const NLMISC::CVector& vec,
			  const TAngVelocity av,
			  bool groundmode );

	/// Alt. constructor with entity state
	CReplica( const IMovingEntity& es );

	/// Change the current state
	void	changeStateTo( const IMovingEntity& es );

};


} // NLNET


#endif // NL_REPLICA_H

/* End of replica.h */
