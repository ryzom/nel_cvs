/** \file ps_color.h
 * <File description>
 *
 * $Id: ps_color.h,v 1.8 2004/02/19 09:49:44 vizerie Exp $
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
#include "3d/ps_attrib_maker_bin_op.h"
#include "3d/ps_attrib_maker_helper.h"
#include "nel/misc/rgba.h"
#include "nel/3d/animation_time.h"






namespace NL3D {



const char *CPSAttribMaker<NLMISC::CRGBA>::getType() { return "CRGBA"; }


/**
 * Here, we got color maker
 * \see ps_attrib_maker.h, ps_attrib_maker_template.h
 */




/// these are some attribute makers for colors

/// This is a int blender class. It just blend between 2 values. The blending is exact, and thus slow...
class CPSColorBlenderExact : public CPSValueBlender<NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBlenderExact);
	CPSColorBlenderExact(NLMISC::CRGBA startColor = NLMISC::CRGBA::White , NLMISC::CRGBA endColor = NLMISC::CRGBA::Black, float nbCycles = 1.0f) : CPSValueBlender<NLMISC::CRGBA>(nbCycles)
	{
		_F.setValues(startColor, endColor);
	}		
	CPSAttribMakerBase *clone() const { return new CPSColorBlenderExact(*this); }

};



// an int blender class that perform 64 color sample between colors, it is faster than CPSColorBlenderExact
class CPSColorBlender : public CPSValueBlenderSample<NLMISC::CRGBA, 64>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorBlender);
	CPSColorBlender(NLMISC::CRGBA startColor = NLMISC::CRGBA::White , NLMISC::CRGBA endColor = NLMISC::CRGBA::Black, float nbCycles = 1.0f) : CPSValueBlenderSample<NLMISC::CRGBA, 64>(nbCycles)
	{
		_F.setValues(startColor, endColor);
	}		
	CPSAttribMakerBase *clone() const { return new CPSColorBlender(*this); }
};



/** This is a color gradient class
  * NB: a non null gradient must be set before use
  */
class CPSColorGradient : public CPSValueGradient<NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS(CPSColorGradient);

	/** default ctor
	  * NB: a non null gradient must be set before use
	  */
	CPSColorGradient();

	/**	
	 *	Construct the value gradient blender by passing a pointer to a color table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSColorGradient(const NLMISC::CRGBA *colorTab, uint32 nbValues, uint32 nbStages, float nbCycles = 1.0f);
	static NLMISC::CRGBA _DefaultGradient[];	
	CPSAttribMakerBase *clone() const { return new CPSColorGradient(*this); }
};


/** this memorize value by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSColorMemory : public CPSAttribMakerMemory<NLMISC::CRGBA>
{
public:
	CPSColorMemory() { setDefaultValue(NLMISC::CRGBA::White); }
	NLMISC_DECLARE_CLASS(CPSColorMemory);
	CPSAttribMakerBase *clone() const { return new CPSColorMemory(*this); }
};


/** An attribute maker whose output if the result of a binary op on colors
  *
  */
class CPSColorBinOp : public CPSAttribMakerBinOp<NLMISC::CRGBA>
{
	public:
	NLMISC_DECLARE_CLASS(CPSColorBinOp);
	CPSAttribMakerBase *clone() const { return new CPSColorBinOp(*this); }
};

} // NL3D
#endif // NL_PS_COLOR_H

/* End of ps_color.h */
