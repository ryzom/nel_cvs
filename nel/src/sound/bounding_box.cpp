/** \file bounding_box.cpp
 * CBoundingBox: Bounding box (different from the NL3D ones)
 *
 * $Id: bounding_box.cpp,v 1.5 2002/08/21 09:42:29 lecroart Exp $
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

#include "stdsound.h"

#include "bounding_box.h"
#include "nel/misc/plane.h"

using namespace NLMISC;

namespace NLSOUND {


/*
 * Constructor
 */
CBoundingBox::CBoundingBox() : _Rx(0.0f), _Ry(0.0f), _Rz(0.0f), _Center(CVector::Null)
{
}


/*
 * Does the box include a point ?
 */
bool			CBoundingBox::include( const CVector& pos ) const
{
	if ( pos.x < _Center.x - _Rx ) return false;
	if ( pos.x > _Center.x + _Rx ) return false;
	if ( pos.y < _Center.y - _Ry ) return false;
	if ( pos.y > _Center.y + _Ry ) return false;
	if ( pos.z < _Center.z - _Rz ) return false;
	if ( pos.z > _Center.z + _Rz ) return false;
	return true;
}


/*
 * Return the environment size
 */
float			CBoundingBox::getDiameter() const
{
	// Set it to the average of the three widths (?)
	return (_Rx+_Ry+_Rz) / 1.5f;
}


/*
 * Return the radius at the intersection of the shape and the line between the specified position and the center of the shape
 */
float			CBoundingBox::getRadiusAtIntersect( const NLMISC::CVector& pos ) const
{
	return 0.0f;
}


/*
 *
 */
CVector			CBoundingBox::getIntersectWithLine( const NLMISC::CVector& c, const NLMISC::CVector& p ) const
{
	// 1. Build 6 planes made up of the box
	CPlane pl[6];
	CVector ppos = _Center;
	ppos.x = _Center.x + _Rx;
	pl[0].make( CVector::I, ppos );
	ppos.x = _Center.x - _Rx;
	pl[1].make( -CVector::I, ppos );
	ppos.x = _Center.x;
	ppos.y = _Center.y + _Ry;
	pl[2].make( CVector::J, ppos );
	ppos.y = _Center.y - _Ry;
	pl[3].make( -CVector::J, ppos );
	ppos.y = _Center.y;
	ppos.z = _Center.z + _Rz;
	pl[4].make( CVector::K, ppos );
	ppos.z = _Center.z - _Rz;
	pl[5].make( -CVector::K, ppos );

	// 2. Maximize the length of the segment to be greater than the size of the box (>=sqrt(3))
	CVector v = (p-c).normed();
	v *= std::max( std::max( _Rx, _Ry ), _Rz ) * 10.0f;
	CVector pfar = c + v;

	// 3. Clip
	CVector center = c;
	uint i;
	for ( i=0; i!=6; ++i )
	{
		pl[i].clipSegmentBack( center, pfar );
	}

	return pfar;
}


/* Return the ratio of a point between this shape and another inner shape.
 * The point must be included in this shape (the outer one) and not
 * in the inner shape.
 * If the point is near this shape, the ratio is near 0.
 * If the point is near the inner shape, the ratio is near 1.
 */
float			CBoundingBox::getRatio( const NLMISC::CVector& pos, IBoundingShape *inner ) const
{
	/* TEMP: works only with 2 boxes
	 *
	 */

	// Let C be the center of the inner shape, P the position of the point

	// 1. Get the intersection A between [CP] and the inner shape
	CVector posa = inner->getIntersectWithLine( inner->getCenter(), pos );

	// 2. Get the intersection B between (CP) and the outer shape (this)
	CVector posb = getIntersectWithLine( inner->getCenter(), pos );

	// 3. Compute PB / AB
	float ab = (posa-posb).norm();
	if ( ab != 0 )
	{
		float pb = (pos-posb).norm();
		return pb / ab;
	}
	else
	{
		return 1.0f;
	}
}


/*
 * Return the corners (EDIT)
 */
void			CBoundingBox::getCorners( NLMISC::CVector& c1, NLMISC::CVector& c2 )
{
	c1.x = _Center.x - _Rx;
	c1.y = _Center.y - _Ry;
	c1.z = _Center.z - _Rz;
	c2.x = _Center.x + _Rx;
	c2.y = _Center.y + _Ry;
	c2.z = _Center.z + _Rz;
}

/*
 * Set the corners (the corners must be ordered: 1 has smaller x, y, z) (EDIT)
 */
void			CBoundingBox::setCorners( const NLMISC::CVector& c1, const NLMISC::CVector& c2 )
{
	_Center.x = (c1.x+c2.x)/2.0f;
	_Center.y = (c1.y+c2.y)/2.0f;
	_Center.z = (c1.z+c2.z)/2.0f;
	_Rx = (c2.x-c1.x)/2.0f;
	_Ry = (c2.y-c1.y)/2.0f;
	_Rz = (c2.z-c1.z)/2.0f;
}


} // NLSOUND
