/** \file remote_entity.cpp
 * Remote-controlled entities
 *
 * $Id: remote_entity.cpp,v 1.3 2000/10/27 15:45:07 cado Exp $
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

namespace NLNET {


/*
 * Constructor
 */
CRemoteEntity::CRemoteEntity( const NLMISC::CVector pos,
							  const NLMISC::CVector hdg,
							  const NLMISC::CVector vec,
							  const TAngVelocity av ) :
	IMovingEntity( pos, hdg, vec, av )
{
}


/*
 * Alt. constructor with entity state
 */
CRemoteEntity::CRemoteEntity( const IMovingEntity& es ) :
	IMovingEntity( es )
{
}


/*
 * Change the current state
 */
void CRemoteEntity::changeStateTo( const IMovingEntity& es )
{
	IMovingEntity::operator=( es ); // at the moment, no particular convergence algorithm
}


} // NLNET
