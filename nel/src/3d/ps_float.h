/** \file ps_size.h
 * <File description>
 *
 * $Id: ps_float.h,v 1.2 2001/07/04 12:32:18 vizerie Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_PS_FLOAT_H
#define NL_PS_FLOAT_H

#include "nel/misc/types_nl.h"
#include "3d/ps_attrib_maker_template.h"
#include "nel/3d/animation_time.h"
#include <algorithm>

namespace NL3D {


/// these are some attribute makers for float

/// This is a float blender class. It just blend between 2 values

class CPSFloatBlender : public CPSValueBlender<float>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatBlender) ;
	CPSFloatBlender(float startFloat = 0.1f , float endFloat = 0.0f, float nbCycles = 1.0f) : CPSValueBlender<float>(nbCycles)
	{
		_F.setValues(startFloat, endFloat) ;
	}
	
	// F is serialized by base classes...

} ;


/// This is a float gradient class
class CPSFloatGradient : public CPSValueGradient<float>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatGradient) ;

	/**	
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSFloatGradient(const float *floatTab = CPSFloatGradient::_DefaultGradient
						, uint32 nbValues = 2, uint32 nbStages = 16, float nbCycles = 1.0f) ;



	static float _DefaultGradient[] ;
	
	// F is serialized by base classes...	
} ;

/** this memorize float by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSFloatMemory : public CPSAttribMakerMemory<float>
{
public:
	CPSFloatMemory() { setDefaultValue(0.f) ; }
	NLMISC_DECLARE_CLASS(CPSFloatMemory) ;
} ;



} // NL3D


#endif // NL_PS_FLOAT_H

/* End of ps_size.h */
