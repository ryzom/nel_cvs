/** \file bsphere.h
 * <File description>
 *
 * $Id: bsphere.h,v 1.1 2000/11/02 14:03:00 berenguier Exp $
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

#ifndef NL_BSPHERE_H
#define NL_BSPHERE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"


namespace NL3D 
{

using NLMISC::CVector;
using NLMISC::CPlane;


/**
 * A bounding Sphere.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CBSphere
{
public:
	CVector			Center;
	float			Radius;

	/// Empty Constructor.
	CBSphere() {}

	/// \name Clip
	// @{
	/// Is the bsphere partially in front of the plane??
	bool	clip(const CPlane &p) const;
	/// same as clip(), but assume p is normalized.
	bool	clipUnitPlane(const CPlane &p) const;
	// @}

};


} // NL3D


#endif // NL_BSPHERE_H

/* End of bsphere.h */
