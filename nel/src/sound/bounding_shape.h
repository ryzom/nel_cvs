/** \file bounding_shape.h
 * IBoundingShape: interface for bounding shapes
 *
 * $Id: bounding_shape.h,v 1.4 2001/07/25 08:40:58 cado Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_BOUNDING_SHAPE_H
#define NL_BOUNDING_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/vector.h"


namespace NLSOUND {


/**
 * Interface for bounding shapes (bounding spheres, bounding boxes...)
 *
 * These bounding shapes are used by EnvSounds and EnvEffects areas.
 * There is no common interface in NLMISC for performance purpose.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class IBoundingShape : public NLMISC::IStreamable
{
public:

	/// Return true if the shape includes the specified point
	virtual bool	include( const NLMISC::CVector& pos ) const = 0;
	/// Return the approximate diameter
	virtual float	getDiameter() const = 0;
	/// Return the radius at the intersection of the shape and the line between the specified position and the center of the shape
	virtual float	getRadiusAtIntersect( const NLMISC::CVector& pos ) { return 0.0f; /* method not required*/ }
	/// Return a reference to the center of the shape
	virtual const NLMISC::CVector& getCenter() const = 0;
	/// Move the shape
	virtual void	setCenter( const NLMISC::CVector& pos ) = 0;
	/** Return the ratio of a point between this shape and another inner shape.
	 * The point must be included in this shape (the outer one) and not
	 * in the inner shape.
	 * If the point is near this shape, the ratio is near 0.
	 * If the point is near the inner shape, the ratio is near 1.
	 */
	virtual float	getRatio( const NLMISC::CVector& pos, IBoundingShape *inner ) const { return 0.0f; /*method not required*/ };
	/// Serialize
	virtual void	serial( NLMISC::IStream& s ) = 0;


	/// Destructor
	virtual ~IBoundingShape() {}

protected:

	/// Constructor
	IBoundingShape() {}

};


} // NLSOUND


#endif // NL_BOUNDING_SHAPE_H

/* End of bounding_shape.h */
