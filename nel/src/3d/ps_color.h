/** \file ps_color.h
 * <File description>
 *
 * $Id: ps_color.h,v 1.1 2001/06/15 16:24:43 corvazier Exp $
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

#ifndef NL_PS_COLOR_H
#define NL_PS_COLOR_H

#include "nel/misc/types_nl.h"
#include "3d/ps_attrib_maker_template.h"
#include "nel/misc/rgba.h"
#include "nel/3d/animation_time.h"






namespace NL3D {

using NLMISC::CRGBA ;

/**
 * Here, we got color maker
 * \see ps_attrib_maker.h, ps_attrib_maker_template.h
 */




/// these are some attribute makers for int

/// This is a int blender class. It just blend between 2 values. The blending is exact, ands thus slow...
class CPSColorBlenderExact : public CPSValueBlender<CRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBlenderExact) ;
	CPSColorBlenderExact(CRGBA startColor = CRGBA::White , CRGBA endColor = CRGBA::Black, float nbCycles = 1.0f) : CPSValueBlender<CRGBA>(nbCycles)
	{
		_F.setValues(startColor, endColor) ;
	}
	
	// F is serialized by base classes...

} ;



// an int blender class that perform 64 color sample between colors, it is faster
class CPSColorBlender : public CPSValueBlenderSample<CRGBA, 64>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBlender) ;
	CPSColorBlender(CRGBA startColor = CRGBA::White , CRGBA endColor = CRGBA::Black, float nbCycles = 1.0f) : CPSValueBlenderSample<CRGBA, 64>(nbCycles)
	{
		_F.setValues(startColor, endColor) ;
	}
	
	// F is serialized by base classes...

} ;



/// This is a color gradient class
class CPSColorGradient : public CPSValueGradient<CRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorGradient) ;

	/**	
	 *	Construct the value gradient blender by passing a pointer to a color table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSColorGradient(const CRGBA *colorTab = CPSColorGradient::_DefaultGradient
						, uint32 nbValues = 2, uint32 nbStages = 64, float nbCycles = 1.0f) ;


	static CRGBA _DefaultGradient[] ;
	
	// F is serialized by base classes...	

} ;

} // NL3D


#endif // NL_PS_COLOR_H

/* End of ps_color.h */
