/** \file rgba.cpp
 * ARGB pixel format
 *
 * $Id: rgba.cpp,v 1.10 2001/05/09 10:10:06 vizerie Exp $
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

#include "nel/misc/rgba.h"
#include "nel/misc/stream.h"


namespace NLMISC {


// ***************************************************************************
// ***************************************************************************
// CRGBA
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CRGBA::serial(class NLMISC::IStream &f)
{
	f.serial (R);
	f.serial (G);
	f.serial (B);
	f.serial (A);
}
// ***************************************************************************
void CRGBA::set(uint8 r, uint8 g, uint8 b, uint8 a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}



// ***************************************************************************
// ***************************************************************************
// CBGRA
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CBGRA::serial(class NLMISC::IStream &f)
{
	f.serial (B);
	f.serial (G);
	f.serial (R);
	f.serial (A);
}
// ***************************************************************************
void CBGRA::set(uint8 r, uint8 g, uint8 b, uint8 a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}
// ***************************************************************************
void CBGRA::blendFromui(CBGRA &c0, CBGRA &c1, uint coef) // coef must be in [0,256]
{
	int	a1 = coef;
	int	a2 = 256-a1;
	R = (c0.R*a2 + c1.R*a1) >>8;
	G = (c0.G*a2 + c1.G*a1) >>8;
	B = (c0.B*a2 + c1.B*a1) >>8;
	A = (c0.A*a2 + c1.A*a1) >>8;
}


// ***************************************************************************
// ***************************************************************************
// CRGBAF
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void CRGBAF::serial(class NLMISC::IStream &f)
{
	f.serial (R);
	f.serial (G);
	f.serial (B);
	f.serial (A);
}
// ***************************************************************************
void CRGBAF::set(float r, float g, float b, float a)
{
	R = r;
	G = g;
	B = b;
	A = a;
}


} // NLMISC
