/** \file texture.cpp
 * ITexture & CTextureFile
 *
 * $Id: texture.cpp,v 1.3 2000/11/10 15:19:47 coutelas Exp $
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

#include "nel/3d/texture.h"
#include "nel/3d/font_generator.h"
#include "nel/misc/file.h"
#include <vector>


namespace NL3D {


/*==================================================================*\
							ITEXTURE
\*==================================================================*/





/*==================================================================*\
							CTEXTUREFILE
\*==================================================================*/

/*------------------------------------------------------------------*\
							generate()
\*------------------------------------------------------------------*/
void CTextureFile::generate()
{
	NLMISC::CIFile f;
	f.open(_FileName);
	load(f);
}


/*==================================================================*\
							CTEXTUREFONT
\*==================================================================*/

/*------------------------------------------------------------------*\
							generate()
\*------------------------------------------------------------------*/
void CTextureFont::generate()
{
	// constructing generator
	CFontGenerator generator(_FontFileName.c_str());
	
	// getting bitmap infos
	uint32 pitch;
	uint8 * bitmap = generator.getBitmap(_Char, _Size, _CharWidth, _CharHeight, pitch);

	// computing new width and height as powers of 2
	if(!isPowerOf2(_CharWidth))
		_Width = getNextPowerOf2(_CharWidth);
	else
		_Width = _CharWidth;
	if(!isPowerOf2(_CharHeight))
		_Height = getNextPowerOf2(_CharHeight);
	else
		_Height = _CharHeight;
	

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
				_Data[0][(i*_Width + j)*4] = bitmap[i*pitch + j];
				_Data[0][(i*_Width + j)*4 + 1] = bitmap[i*pitch + j];
				_Data[0][(i*_Width + j)*4 + 2] = bitmap[i*pitch + j];
				_Data[0][(i*_Width + j)*4 + 3] = 255;
			}
			else
			{
				_Data[0][(i*_Width + j)*4] = 0;
				_Data[0][(i*_Width + j)*4 + 1] = 0;
				_Data[0][(i*_Width + j)*4 + 2] = 0;
				_Data[0][(i*_Width + j)*4 + 3] = 255;

			}
			
		}
	}


}



} // NL3D
