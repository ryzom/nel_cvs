/** \file font_generator.h
 * CFontGenerator class
 *
 * $Id: font_generator.h,v 1.2 2000/11/10 15:20:13 coutelas Exp $
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

#include <freetype/freetype.h>

#include "nel/misc/types_nl.h"
#include <string>

namespace NL3D {


struct CFontDescriptor 
{
	std::string FontFileName;
	ucchar C;
	uint32 Size;
	
	CFontDescriptor(std::string s, ucchar c, uint32 sz): FontFileName(s),C(c),Size(sz) { }
	bool operator< (const CFontDescriptor& desc) const
	{
		if (FontFileName<desc.FontFileName)
			return true;
		if (FontFileName>desc.FontFileName)
			return false;
		if (C<desc.C)
			return true;
		if (C>desc.C)
			return false;
		return Size<desc.Size;
	}
};



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
	CFontGenerator(const char *fontFileName);

	/** generate and return a bitmap
	 * \param c the unicode char
	 * \param size size of the generated font in ??? format
	 * \param width width of the generated bitmap, this value is set by this function
	 * \param height height of the generated bitmap, this value is set by this function
	 * \param pitch pitch of the generated bitmap (+ or - the number of bytes per row), this value is set by this function
	 */
	uint8 *getBitmap (ucchar c, uint32 size, uint32 &width, uint32 &height, uint32 &pitch);

private:

	const char *getFT2Error(FT_Error fte);

	static FT_Library	_Library;
	static bool			_LibraryInit;

	FT_Face		_Face;
};


} // NL3D


#endif // NL_FONT_GENERATOR_H

/* End of font_generator.h */
