/** \file material.cpp
 * CMaterial implementation
 *
 * $Id: material.cpp,v 1.2 2000/10/30 14:51:32 viau Exp $
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

#include "nel/3d/driver.h"

using std::vector;

namespace NL3D
{

// --------------------------------------------------

CTexture::CTexture(void)
{
	_Width=0;
	_Height=0;
}

CTexture::CTexture(uint16 width, uint16 height)
{
	_Width=width;
	_Height=height;
	_Data.resize(_Width*_Height);
}

// --------------------------------------------------

bool CTexture::fillData(const void* data)
{
	void*	ptr;

	ptr=&(*_Data.begin());
	memcpy(ptr,data,_Width*_Height*sizeof(CRGBA));
	return(true);
}

bool CTexture::fillData(const vector<CRGBA>& data)
{
	if ( (uint16)data.size()!=(_Width*_Height) )
	{
		return(false);
	}
	_Data=data;
	return(true);
}

// --------------------------------------------------
// --------------------------------------------------
// --------------------------------------------------
// --------------------------------------------------
// --------------------------------------------------

// --------------------------------------------------

}

