/** \file ps_color.h
 * <File description>
 *
 * $Id: ps_color.h,v 1.2 2001/05/08 13:37:08 vizerie Exp $
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
#include "nel/3d/ps_attrib_maker.h"
#include "nel/misc/rgba.h"
#include "nel/3d/tmp/animation_time.h"


/**
 * Here, we got color maker
 * \see ps_attrib_maker.h
 */



namespace NL3D {

using NLMISC::CRGBA ;


/** this class is a color fader functor
 *  It is used by CPSColorFader
 *  \see CPScolorFader
 */
class CPSColorFaderFunc
{
public:
	inline CRGBA operator()(CAnimationTime time) const
	{
		return _Tab[(uint32) (time * 63.0f)] ;		
	}

	
	/// copie the start and end color in the referenced colors
	void getColors(CRGBA &c1, CRGBA &c2) const
	{
		c1 = _Tab[0] ;
		c2 = _Tab[63] ;
	}

	/// set the colors

	void setColors(CRGBA c1, CRGBA c2) ;
	

	/// serialization
	virtual void serial(NLMISC::IStream &f) ;

protected:	
	// a precomputed color tab
	CRGBA _Tab[64] ;
} ;


/// Affect this class to a particle to have a particle fader
class CPSColorFader : public CPSAttribMakerT<CRGBA, CPSColorFaderFunc>
{
public:
		NLMISC_DECLARE_CLASS(CPSColorFader) ;

		CPSColorFader(CRGBA c1 = CRGBA(255, 255, 255), CRGBA c2 = CRGBA(0, 0, 0), float nbCycles = 1.0f) 
					: CPSAttribMakerT<CRGBA, CPSColorFaderFunc>(nbCycles) 
		{
			_F.setColors(c1, c2) ;
		}

		// serialization is done by CPSAttribMakerT
} ;



/** this class is a color gradient functor
 *  It is used by CPSColorGradient
 *  \see CPScolorFader
 */
class CPSColorGradientFunc
{
public:
	inline CRGBA operator()(CAnimationTime time) const
	{
		nlassert(_Tab) ;
		return _Tab[(uint32) (time * _NumCol)] ;		
	}

	
	/// copie the colors in the specified table
	void getColors(CRGBA *tab) const ;	

	uint32 getNumCol(void) const { return ((_NumCol - 1) >> 6) + 1 ; }

	/** set the colors
	 *  \param numCol number of color, must be >= 2
	 *  \colorTab a table containing the colors. color will be blended, so you must only provide keyframe colors	 
	 */

	void setColors(const CRGBA *colorTab, uint32 numCol) ;
	

	/// serialization
	virtual void serial(NLMISC::IStream &f) ;

	// ctor
	CPSColorGradientFunc() ;

	// dtor
	~CPSColorGradientFunc() ;

protected:	
	uint32 _NumCol ;
	// a precomputed color tab
	CRGBA  *_Tab ;
} ;


/// Affect this class to a particle to have a particle fader
class CPSColorGradient : public CPSAttribMakerT<CRGBA, CPSColorGradientFunc>
{
public:
		NLMISC_DECLARE_CLASS(CPSColorGradient) ;

		CPSColorGradient(CRGBA *tab = CPSColorGradient::_DefaultGradient, uint32 numCol = 2
						 ,float nbCycles = 1.0f) : CPSAttribMakerT<CRGBA, CPSColorGradientFunc>(nbCycles) 
		{
			_F.setColors(tab, numCol) ;
		}

	static CRGBA _DefaultGradient[] ;

	// serialization is done by CPSAttribMakerT
} ;





} // NL3D


#endif // NL_PS_COLOR_H

/* End of ps_color.h */
