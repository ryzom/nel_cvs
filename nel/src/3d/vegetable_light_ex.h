/** \file vegetable_light_ex.h
 * <File description>
 *
 * $Id: vegetable_light_ex.h,v 1.1 2002/03/15 12:11:32 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_VEGETABLE_LIGHT_EX_H
#define NL_VEGETABLE_LIGHT_EX_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"


namespace NL3D {


/**
 * Additional information to light Vegetables. 2 "precomputed" pointLights can 
 *	additionally light the vegetables.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CVegetableLightEx
{
public:

	/// Must not change (hardcoded)
	enum	{MaxNumLight= 2};

	/// Number Of light currently really enabled.
	uint			NumLights;
	/// Direction of the light. If pointLight, the direction to the instance should be precomputed.
	CVector			Direction[MaxNumLight];
	/// Color of the light. If pointLight, the attenuation to the instance should be precomputed .
	NLMISC::CRGBA	Color[MaxNumLight];

};


} // NL3D


#endif // NL_VEGETABLE_LIGHT_EX_H

/* End of vegetable_light_ex.h */
