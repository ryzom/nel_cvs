/** \file font_generator.h
 * CFontGenerator class
 *
 * $Id: font_generator.h,v 1.3 2001/12/27 10:19:16 lecroart Exp $
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

#ifndef NL_FONT_GENERATOR_H
#define NL_FONT_GENERATOR_H

#ifndef NL_DONT_USE_EXTERNAL_CODE
#	include <freetype/freetype.h>
#else // NL_DONT_USE_EXTERNAL_CODE

#endif // NL_DONT_USE_EXTERNAL_CODE

#include "nel/misc/types_nl.h"
#include <string>


namespace NL3D {


/**
 * Generate bitmap based on a true type font (using freetype2)
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CFontGenerator
{
public:

	/** Constructor
	 * \param fontFileName path+filename (ex: "c:\winnt\fonts\arial.ttf")
	 */
	CFontGenerator (const std::string &fontFileName, const std::string &fontExFileName = "");

	virtual ~CFontGenerator ();

	/** generate and return a bitmap
	 * \param c the unicode char
	 * \param size size of the generated font in ??? format
	 * \param width width of the generated bitmap, this value is set by this function
	 * \param height height of the generated bitmap, this value is set by this function
	 * \param pitch pitch of the generated bitmap (+ or - the number of bytes per row), this value is set by this function
	 */
	uint8	*getBitmap (ucchar c, uint32 size, uint32 &width, uint32 &height, uint32 &pitch, sint32 &left, sint32 &top, sint32 &advx, uint32 &glyphIndex);

	/** returns the width and height of a character using a specific size and
	 *
	 * \warning this function is not very fast (but faster than getBitmap()) because it has to load the char before.
	 */
	void	 getSizes (ucchar c, uint32 size, uint32 &width, uint32 &height);

	void	 getKerning (ucchar left, ucchar right, sint32 &kernx);

	uint32	 getCharIndex (ucchar c);

	std::string	FontFileName;
private:

#ifndef NL_DONT_USE_EXTERNAL_CODE
	const char			*getFT2Error(FT_Error fte);

	static FT_Library	_Library;
	static bool			_LibraryInit;

	FT_Face				_Face;
#else // NL_DONT_USE_EXTERNAL_CODE

#endif // NL_DONT_USE_EXTERNAL_CODE
};



/**
 * Descriptor of a character
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
struct CFontDescriptor 
{
	CFontGenerator *FontGen;
	ucchar C;
	uint32 Size;
	
	/** Constructor
	 * \param fg the font generator
	 * \param c the unicode char
	 * \param sz size of the font
	 */
	CFontDescriptor(CFontGenerator *fg, ucchar c, uint32 sz): FontGen(fg),C(c),Size(sz) { }
	
	/** operator<
	 * used for ordering generated char
	 */
	bool operator< (const CFontDescriptor& desc) const
	{
		if (FontGen->FontFileName<desc.FontGen->FontFileName)
			return true;
		if (FontGen->FontFileName>desc.FontGen->FontFileName)
			return false;
		if (C<desc.C)
			return true;
		if (C>desc.C)
			return false;
		return Size<desc.Size;
	}
};


} // NL3D


#endif // NL_FONT_GENERATOR_H

/* End of font_generator.h */
