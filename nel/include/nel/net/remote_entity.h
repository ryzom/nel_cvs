/** \file remote_entity.h
 * Remote-controlled entities
 *
 * $Id: remote_entity.h,v 1.3 2000/10/27 15:45:06 cado Exp $
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
#include "nel/net/moving_entity.h"


namespace NLNET {


/**
 * A moving entity that is controlled from elsewhere
 * \warning This class is test code and is highly subject to change.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CRemoteEntity : public IMovingEntity
{
public:

	/// Constructor
	CRemoteEntity() {};
	
	/// Alt. constructor
	CRemoteEntity( const NLMISC::CVector pos,
				   const NLMISC::CVector hdg,
				   const NLMISC::CVector vec,
				   const TAngVelocity av );

	/// Alt. constructor with entity state
	CRemoteEntity( const IMovingEntity& es );

	/// Update the entity state
	void	update( TDuration deltatime )
	{
		computePosAfterDuration( deltatime );
	}

	/// Change the current state
	void	changeStateTo( const IMovingEntity& es );

};


} // NLNET


#endif // NL_REMOTE_ENTITY_H

/* End of remote_entity.h */
