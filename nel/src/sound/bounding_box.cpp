/** \file bounding_box.cpp
 * CBoundingBox: Bounding box (different from the NL3D ones)
 *
 * $Id: bounding_box.cpp,v 1.2 2001/07/17 15:32:12 cado Exp $
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

#include "bounding_box.h"

using namespace NLMISC;

namespace NLSOUND {


/*
 * Constructor
 */
CBoundingBox::CBoundingBox() : _Center(CVector::Null), _Rx(0.0f), _Ry(0.0f), _Rz(0.0f)
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
