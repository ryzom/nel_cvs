/** \file remote_entity.h
 * Remote-controlled entities
 *
 * $Id: remote_entity.h,v 1.7 2000/11/30 17:03:10 cado Exp $
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

#ifndef NL_REMOTE_ENTITY_H
#define NL_REMOTE_ENTITY_H

#include "nel/misc/types_nl.h"
#include "nel/net/replica.h"
#include "nel/net/cubic_entity_interpolator.h"
#include "nel/net/linear_entity_interpolator.h"



namespace NLNET {


/**
 * A remote-controlled replica, with convergency.
 * All you can do is create a remote entity, update its state and change it (using a convergency
 * algorithm) from another entity state
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CRemoteEntity : public CReplica
{
public:

	/// Constructor
	CRemoteEntity() : CReplica() {};
	
	/// Alt. constructor
	CRemoteEntity( TEntityType t,
				   const NLMISC::CVector& pos,
				   const NLMISC::CVector& hdg,
				   const TAngle rollangle,
				   const NLMISC::CVector& vec,
				   const TAngVelocity av,
				   bool groundmode );

	/// Alt. constructor with entity state
	CRemoteEntity( const IMovingEntity& es );

	/// Update the entity state
	void	update( TDuration deltatime );

	/// Converge to the specified state
	void	convergeTo( const IMovingEntity& dest_es );

	/// Converge duration
	static const TDuration ConvergeDuration;

private:

	CCubicEntityInterpolator	_Interpolator;
};


} // NLNET


#endif // NL_REMOTE_ENTITY_H

/* End of remote_entity.h */
