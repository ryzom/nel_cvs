/** \file linear_entity_interpolator.cpp
 * Linear interpolation of entity
 *
 * $Id: linear_entity_interpolator.cpp,v 1.1 2000/11/20 15:51:49 cado Exp $
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

#include "nel/net/linear_entity_interpolator.h"


namespace NLNET {


/*
 * Begins interpolation
 */
void CLinearEntityInterpolator::begin( const IMovingEntity& src, const IMovingEntity& dest, TDuration duration )
{
	_Src = src;
	_Dest = dest;
	_Duration = duration;
	_Elapsed = 0;
	_Active = true;
}


/*
 * Returns the actual state; \e deltatime is the time between two updates.
 */
void CLinearEntityInterpolator::getNextState( IMovingEntity& es, TDuration deltatime )
{
	_Elapsed += deltatime;
	float ratio = _Elapsed / _Duration;
	if ( ratio < 1.0 )
	{
		es.setPos( _Src.pos() + (_Dest.pos()-_Src.pos()) * ratio );
		es.setBodyHeading( _Src.bodyHeading() + (_Dest.bodyHeading()-_Src.bodyHeading()) * ratio );
	}
	else
	{
		es = _Dest;
		_Active = false;
	}
}


} // NLNET
