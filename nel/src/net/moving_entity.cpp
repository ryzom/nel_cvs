/** \file moving_entity.cpp
 * Interface for all moving entities
 *
 * $Id: moving_entity.cpp,v 1.3 2000/10/24 16:39:42 cado Exp $
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


namespace NLNET {


TEntityId IMovingEntity::_MaxId = 1; //	avoid 0


/*
 * Constructor
 */
IMovingEntity::IMovingEntity()
{
	_Id = 0; //getNewId();
}


/*
 * Alt. constructor
 */
IMovingEntity::IMovingEntity( const NLMISC::CVector pos,
							  const NLMISC::CVector hdg,
							  const NLMISC::CVector vec )
{
	_Id = 0; //getNewId();
	_Pos = pos;
	_BodyHdg = hdg;
	_Vector = vec;
}


/*
 * Copy constructor
 */
IMovingEntity::IMovingEntity( const IMovingEntity& other )
{
	operator=( other );
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
