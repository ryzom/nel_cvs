/** \file entity_interpolator.h
 * Base class for entity interpolation
 *
 * $Id: entity_interpolator.h,v 1.1 2000/11/20 15:51:49 cado Exp $
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

#ifndef NL_ENTITY_INTERPOLATOR_H
#define NL_ENTITY_INTERPOLATOR_H

#include "nel/misc/types_nl.h"
#include "nel/net/moving_entity.h"


namespace NLNET {


/**
 * Base class for entity interpolation
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class IEntityInterpolator
{
public:

	/// Constructor
	IEntityInterpolator() :	_Active( false ) {}

	/// Begins interpolation
	void		begin( const IMovingEntity& src, const IMovingEntity& dest, TDuration duration ) {}

	/// Returns the actual state; \e deltatime is the time between two updates.
	void		getNextState( IMovingEntity& es, TDuration deltatime ) {}

	/// Returns true if interpolation is active
	bool		active()		{ return _Active; }

	/// Interpolation duration
	TDuration	duration()		{ return _Duration; }

	/// Time elapsed according to the deltas passed in getNextState()
	TDuration	elapsed()		{ return _Elapsed; }

protected:

	bool		_Active;
	TDuration	_Duration;
	TDuration	_Elapsed;
};


} // NLNET


#endif // NL_ENTITY_INTERPOLATOR_H

/* End of entity_interpolator.h */
