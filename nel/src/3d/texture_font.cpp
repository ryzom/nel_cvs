/** \file texture_font.cpp
 * <File description>
 *
 * $Id: texture_font.cpp,v 1.1 2000/12/15 18:20:22 berenguier Exp $
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

#include "nel/3d/texture_font.h"
#include "nel/misc/common.h"


namespace NL3D
{


/*==================================================================*\
							CTEXTUREFONT
\*==================================================================*/

/*------------------------------------------------------------------*\
							generate()
\*------------------------------------------------------------------*/
void CTextureFont::generate()
{
	// getting bitmap infos
	uint32 pitch = 0;
	uint8 *bitmap = _FontGen->getBitmap(Char, _Size, _CharWidth, _CharHeight, pitch, Left, Top, AdvX, GlyphIndex);

	// computing new width and height as powers of 2
	if(!NLMISC::isPowerOf2(_CharWidth))
	{
		_Width = NLMISC::raiseToNextPowerOf2(_CharWidth);
		CBitmap::_Width = _Width;
	}
	else
	{
		_Width = _CharWidth;
		CBitmap::_Width = _Width;
	}
	if(!NLMISC::isPowerOf2(_CharHeight))
	{
		_Height = NLMISC::raiseToNextPowerOf2(_CharHeight);
		CBitmap::_Height = _Height;
	}
	else
	{
		_Height = _CharHeight;
		CBitmap::_Height = _Height;
	}
	

	// calculating memory size taken by the bitmap
	uint32 bitmapSize = _Width*_Height*4;
	_Data[0].resize(bitmapSize);

	// filling CBitmap buffer
	for(uint i=0; i<_Height; i++)
	{
		for(uint j=0; j<_Width; j++)
		{
			if(j<_CharWidth && i<_CharHeight)
			{
				_Data[0][(i*_Width + j)*4] = 255;
				_Data[0][(i*_Width + j)*4 + 1] = 255;
				_Data[0][(i*_Width + j)*4 + 2] = 255;
				_Data[0][(i*_Width + j)*4 + 3] = bitmap[i*pitch + j];
			}
			else
			{
				_Data[0][(i*_Width + j)*4] = 255;
				_Data[0][(i*_Width + j)*4 + 1] = 255;
				_Data[0][(i*_Width + j)*4 + 2] = 255;
				_Data[0][(i*_Width + j)*4 + 3] = 0;
			}
			/*
			if(j<_CharWidth && i<_CharHeight)
			{
				if (bitmap[i*pitch + j] == 0)
				{
					_Data[0][(i*_Width + j)*4] = 64;
					_Data[0][(i*_Width + j)*4 + 1] = 64;
					_Data[0][(i*_Width + j)*4 + 2] = 255;
					_Data[0][(i*_Width + j)*4 + 3] = 64;
				}
				else
				{
					_Data[0][(i*_Width + j)*4] = 255;
					_Data[0][(i*_Width + j)*4 + 1] = 255;
					_Data[0][(i*_Width + j)*4 + 2] = 255;
					_Data[0][(i*_Width + j)*4 + 3] = bitmap[i*pitch + j];
				}
			}
			else
			{
				_Data[0][(i*_Width + j)*4] = 255;
				_Data[0][(i*_Width + j)*4 + 1] = 0;
				_Data[0][(i*_Width + j)*4 + 2] = 0;
				_Data[0][(i*_Width + j)*4 + 3] = 64;
			}
			*/
		}
	}
}


} // NL3D
