/** \file rgba.h
 * ARGB pixel format
 *
 * $Id: rgba.h,v 1.3 2000/10/20 16:16:08 corvazier Exp $
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

#ifndef NL_RGBA_H
#define NL_RGBA_H

#include "nel/misc/types_nl.h"


namespace NLMISC 
{

class	IStream;

/**
 * <Class description>
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CRGBA
{
public:

	/// Constructor
	CRGBA() {};
	CRGBA(uint8 r, uint8 g, uint8 b, uint8 a=255) :
		R(r), G(g), B(b), A(a) {}
	uint	getPacked() const {return ((uint)R<<24) + ((uint)G<<16) + ((uint)B<<8) + A;}
	bool	operator<(const CRGBA &c) const {return getPacked()<c.getPacked();}
	bool	operator==(const CRGBA &c) const  
		{return R==c.R && G==c.G && B==c.B && A==c.A;}
	void    serial(class NLMISC::IStream &f);

	///
	uint8	R,G,B,A;
};


} // NLMISC


#endif // NL_RGBA_H

/* End of rgba.h */
