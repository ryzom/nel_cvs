/** \file bounding_sphere.cpp
 * <File description>
 *
 * $Id: bounding_sphere.cpp,v 1.1 2001/07/17 14:22:33 cado Exp $
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

#include "bounding_sphere.h"

using namespace NLMISC;


namespace NLSOUND {


/*
 * Constructor
 */
/*CBoundingSphere::CBoundingSphere()
{
}*/


/*
 * Return true if the box includes the specified point
 */
bool		CBoundingSphere::include( const NLMISC::CVector& pos ) const
{
	return _BSphere.include( pos );
}



/* Return the ratio of a point between this shape and another inner shape.
 * The point must be included in this shape (the outer one) and not
 * in the inner shape.
 * If the point is near this shape, the ratio is near 0.
 * If the point is near the inner shape, the ratio is near 1.
 */
float		CBoundingSphere::getRatio( const NLMISC::CVector& pos, IBoundingShape *inner ) const
{
	return (_BSphere.Radius-(pos-_BSphere.Center).norm()) / (_BSphere.Radius-inner->getRadiusAtIntersect(pos));
}


/*
 * Set properties (EDIT)
 */
void		CBoundingSphere::setProperties( const NLMISC::CVector& center, float radius )
{
	_BSphere.Center = center;
	_BSphere.Radius = radius;
}


} // NLSOUND
