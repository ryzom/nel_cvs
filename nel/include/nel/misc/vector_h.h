/** \file vector_h.h
 * Homogeneous vector
 *
 * $Id: vector_h.h,v 1.2 2000/11/23 16:53:29 cado Exp $
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

#ifndef NL_VECTOR_H_H
#define NL_VECTOR_H_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NLMISC {


/**
 * Homogeneus vector.
 * A CVectorH has a fourth value, w.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CVectorH
{
public:

	/// Attributes
	float	x, y, z, w;

	/// Constructor
	CVectorH() {}
	/// Constructor .
	CVectorH(float	_x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	/// Copy Constructor.
	CVectorH(const CVectorH &v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
	/// Constructor using a vector
	CVectorH(const CVector &v) : x(v.x), y(v.y), z(v.z), w(1.0f) {}

	/// Set
	void	set(float _x, float _y, float _z, float _w)
	{
		x=_x; y=_y; z=_z; w=_w;
	}

	/// Cast operator to a vector (ignoring w)
	operator CVector() const
	{
		return CVector( x, y, z );
	}

};


} // NLMISC


#endif // NL_VECTOR_H_H

/* End of vector_h.h */
