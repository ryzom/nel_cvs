/** \file texture_font.h
 * <File description>
 *
 * $Id: texture_font.h,v 1.5 2001/04/19 11:10:06 berenguier Exp $
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

#ifndef NL_TEXTURE_FONT_H
#define NL_TEXTURE_FONT_H

#include "nel/misc/types_nl.h"
#include "nel/3d/texture.h"
#include "nel/3d/font_generator.h"


namespace NL3D
{


//****************************************************************************
/**
 * CTextureFont
 *
 * Rq : _Width and _Height are duplicated from CBitmap because the texture
 *      can be released by the driver, which sets these values to zero.
 *		But these values are necessary to compute strings.
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CTextureFont : public ITexture
{
	uint32 _CharWidth;
	uint32 _CharHeight;
	uint32 _Width;
	uint32 _Height;

	uint32 _Size;
	CFontGenerator *_FontGen;
public:

	/** Default constructor
	 * 
	 */	
	CTextureFont() 
	{ 
		// Default char. This ctor is usefull for polymorphic serialisation only.
		Char = ' ';
		_Size = 10;
		_CharWidth = 0;
		_CharHeight = 0;
		_FontGen = NULL;
		_Width = 0;
		_Height = 0;
		
		setWrapS(ITexture::Repeat);
		setWrapT(ITexture::Repeat);

		// Font are always Alpha only.
		setUploadFormat(Alpha);
	}

	
	/** Default constructor
	 * 
	 */	
	CTextureFont(const CFontDescriptor& desc) 
	{ 
		Char = desc.C;
		_Size = desc.Size;
		_CharWidth = 0;
		_CharHeight = 0;
		_FontGen = desc.FontGen;
		_Width = 0;
		_Height = 0;

		setWrapS(ITexture::Repeat);
		setWrapT(ITexture::Repeat);

		// Font are always Alpha only.
		setUploadFormat(Alpha);
	}

	/** constructor
	 * 
	 */	
	CTextureFont(CFontGenerator *fg, ucchar c, uint32 size) 
	{ 
		Char = c;
		_Size = size;
		_CharWidth = 0;
		_CharHeight = 0;
		_FontGen = fg;
		_Width = 0;
		_Height = 0;

		setWrapS(ITexture::Repeat);
		setWrapT(ITexture::Repeat);

		// Font are always Alpha only.
		setUploadFormat(Alpha);
	}

	uint32	getCharWidth() const {return _CharWidth;}
	uint32	getCharHeight() const {return _CharHeight;}

	uint32	getWidth() const {return _Width;}
	uint32	getHeight() const {return _Height;}

	/** return the descriptor of this letter
	 * /return CFontDescriptor letter descriptor
	 */
	CFontDescriptor getDescriptor() const
	{
		return CFontDescriptor(_FontGen, Char, _Size);
	}

	// Generate the texture
	void doGenerate();

	/// the unicode character
	ucchar Char;
	/// number of the character in the this font
	uint32 GlyphIndex;
	/// Distance between origin and top of the texture
	sint32 Top;
	/// Distance between origin and left of the texture
	sint32 Left;
	/// Advance to the next caracter
	sint32 AdvX;


	/// Todo: serialize a font texture.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream) {nlstop;}
	NLMISC_DECLARE_CLASS(CTextureFont);

};


} // NL3D


#endif // NL_TEXTURE_FONT_H

/* End of texture_font.h */
