/** \file cubic_entity_interpolator.h
 * Cubic interpolation of entity
 *
 * $Id: cubic_entity_interpolator.h,v 1.5 2000/11/29 17:24:08 cado Exp $
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

#ifndef NL_CUBIC_ENTITY_INTERPOLATOR_H
#define NL_CUBIC_ENTITY_INTERPOLATOR_H

#include "nel/misc/types_nl.h"
#include "nel/net/entity_interpolator.h"
#include "nel/misc/matrix.h"
#include "nel/misc/vector.h"


namespace NLNET {


/**
 * Cubic interpolation of entity.
 * See IEntityInterpolator for an explanation.
 * This class uses Bezier splines for curve-fitting.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CCubicEntityInterpolator : public IEntityInterpolator
{
public:

	/// Constructor
	CCubicEntityInterpolator();

	/// Begins interpolation
	void	begin( const IMovingEntity& src, const IMovingEntity& dest, TDuration duration );

	/// Returns the actual state; \e deltatime is the time between two updates.
	void	getNextState( IMovingEntity& es, TDuration deltatime );

private:

	NLMISC::CMatrix			_CubicMatrix;
	NLMISC::CVector			_SrcHeading;
	IMovingEntity			_Dest;

	static NLMISC::CMatrix	_BezierMatrix;
	static bool				_Initialized;

};


} // NLNET


#endif // NL_CUBIC_ENTITY_INTERPOLATOR_H

/* End of cubic_entity_interpolator.h */
