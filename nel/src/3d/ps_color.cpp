/** \file ps_color.cpp
 * <File description>
 *
 * $Id: ps_color.cpp,v 1.2 2001/05/08 13:37:09 vizerie Exp $
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

#include "nel/3d/ps_color.h"


namespace NL3D {


//////////////////////////////////////
// CPSColorFaderFunc implementation //
//////////////////////////////////////



void CPSColorFaderFunc::serial(NLMISC::IStream &f)
{
	f.serialCheck((uint32) 'FFUN') ;
	f.serial(_Tab[0], _Tab[63]) ; 
	if (f.isReading())
	{
		setColors(_Tab[0], _Tab[63]) ;
	}
}


void CPSColorFaderFunc::setColors(CRGBA c1, CRGBA c2)
{
	for (uint k = 0 ; k < 64 ; ++k)
	{
		CRGBA c ;
		c.blendFromui(c1, c2, (uint32) (256.0f * k * (1.0f / 63.0f))) ;
		_Tab[k] = c ;
	}
}


//////////////////////////////////////
// CPSColorGradientFunc implementation //
//////////////////////////////////////

void CPSColorGradientFunc::getColors(CRGBA *tab) const
{	
	nlassert(tab) ;
	for (uint32 k = 0 ; k < _NumCol ; ++k)
	{
		tab[k] = _Tab[k << 6] ;
	}	
}



void CPSColorGradientFunc::setColors(const CRGBA *colorTab, uint32 numCol)
{
	nlassert(numCol > 1) ;

	if (_Tab)
	{
		delete[] _Tab ;		
	}


	// we have a 64 colors gradient for each tansition
	_NumCol = ((numCol - 1) << 6) + 1 ;
	_Tab = new CRGBA[_NumCol] ;

	for (uint32 k = 0 ; k  < (numCol - 1) ; ++k)
	{
		for(uint32 l = 0 ; l < 64 ; ++l)
		{
			CRGBA c ;
			c.blendFromui(colorTab[k], colorTab[k + 1], (uint32) (256.0f * l * (1.0f / 64.0f))) ;
			_Tab[ (k << 6) + l ] = c ;
		}
	}
	_Tab[_NumCol - 1] = colorTab[numCol - 1] ;
}
	


/// serialization
void CPSColorGradientFunc::serial(NLMISC::IStream &f)
{
	f.serialCheck((uint32) 'GCF_') ;
	if (f.isReading())
	{
		uint32 numCol ;
		f.serial(numCol) ;
		_NumCol = ((numCol - 1) << 6) + 1 ;

		// create the table on the stack for small gradient
		if (numCol < 256)
		{
			CRGBA tab[256] ;
			for (uint32 k = 0 ; k < numCol ; ++k)
			{
				f.serial(tab[k]) ;
			}
			setColors(tab, numCol) ;
		}	
		else
		{
			CRGBA *tab = new CRGBA[numCol] ;
			for (uint32 k = 0 ; k < numCol ; ++k)
			{
				f.serial(tab[k]) ;
			}
			setColors(tab, numCol) ;
			delete[] tab ;
		}
	}
	else
	{
		uint32 numCol = getNumCol() ;
		f.serial(numCol) ;
		for (uint32 k = 0 ; k < numCol ; ++k)
		{
			f.serial(_Tab[k << 6]) ;
		}
	}
}


CPSColorGradientFunc::CPSColorGradientFunc() : _Tab(NULL), _NumCol(0)
{
}


CPSColorGradientFunc::~CPSColorGradientFunc()
{
	delete[] _Tab ;
}


CRGBA CPSColorGradient::_DefaultGradient[2] = { CRGBA(255, 255, 255), CRGBA(0, 0, 0) } ;


} // NL3D
