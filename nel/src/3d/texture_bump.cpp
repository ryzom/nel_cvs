/** \file texture_bump.cpp
 * <File description>
 *
 * $Id: texture_bump.cpp,v 1.1 2001/10/26 08:20:36 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "3d/texture_bump.h"


namespace NL3D {


/// create a DsDt texture from a height map (red component of a rgba bitmap)
static void BuildDsDt(uint32 *src, sint width, sint height, uint16 *dest)
{	
	#define GET_HGT(x, y) ((sint) (src[(uint) (x) % width + ((uint) (y) % height) * width] & 0x00ff00) >> 8)
	sint x, y;	
	for (x = 0; x < width; ++x)
	{
		for (y = 0; y < height; ++y)
		{			
			sint off = x + y * width;
			sint16 ds = (sint16) (GET_HGT(x + 1, y) - GET_HGT(x - 1, y));
			sint16 dt = (sint16) (GET_HGT(x, y + 1) - GET_HGT(x, y - 1));
			*(uint8 *) &dest[x + y * width] = (uint8) (ds + 127);
			((uint8 *) &dest[x + y * width])[1] = (uint8) (dt + 127);
		}
	}
}



/*
 * Constructor
 */
CTextureBump::CTextureBump()
{
}



void CTextureBump::setHeightMap(ITexture *heightMap)
{
	if (heightMap != _HeightMap)
	{
		_HeightMap = heightMap;
		touch();
	}
}

void CTextureBump::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(0);
	ITexture::serial(f);
	ITexture *tex = NULL;	
	if (f.isReading())
	{		
		f.serialPolyPtr(tex);
		_HeightMap = tex;
		touch();
	}
	else
	{
		tex = _HeightMap;
		f.serialPolyPtr(tex);
	}
}


void CTextureBump::doGenerate()
{
	nlassert(_HeightMap);
	// generate the height map
	_HeightMap->generate();
	if (!_HeightMap->convertToType(CBitmap::RGBA))
	{
		makeDummy();
		return;
	}
	releaseMipMaps();
	uint width = _HeightMap->getWidth();
	uint height = _HeightMap->getHeight();
	CBitmap::resize(_HeightMap->getWidth(), _HeightMap->getHeight(), CBitmap::DsDt);
	// mipmapping not supported for now, disable it
	ITexture::setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	// build the DsDt map
	BuildDsDt((uint32 *) &(_HeightMap->getPixels()[0]), width, height, (uint16 *) &(getPixels()[0]));
	if (_HeightMap->getReleasable())
	{
		_HeightMap->release();
	}
}

void CTextureBump::release()
{
	ITexture::release();
	if (_HeightMap != NULL)
	{
		if (_HeightMap->getReleasable())
		{
			_HeightMap->release();
		}
	}
}

} // NL3D
