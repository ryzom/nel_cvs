/** \file font_generator.cpp
 * CFontGenerator class
 *
 * $Id: font_generator.cpp,v 1.9 2000/12/18 14:16:56 lecroart Exp $
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


#include <freetype/freetype.h>

// for freetype 2.0
#ifdef FTERRORS_H
#undef FTERRORS_H
#endif

// for freetype 2.0.1
#ifdef __FTERRORS_H__
#undef __FTERRORS_H__
#endif

#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST  {
#define FT_ERROR_END_LIST    { 0, 0 } };

const struct
{
int          err_code;
const char*  err_msg;
} ft_errors[] = 

#include <freetype/fterrors.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"

#include "nel/3d/font_generator.h"

using namespace NLMISC;

namespace NL3D {

FT_Library	CFontGenerator::_Library;
bool		CFontGenerator::_LibraryInit = false;




const char *CFontGenerator::getFT2Error(FT_Error fte)
{
	static char ukn[1024];

	for (uint32 i = 0; ft_errors[i].err_code != 0 || ft_errors[i].err_msg != 0; i++)
	{
		if (ft_errors[i].err_code == fte)
			return ft_errors[i].err_msg;
	}
	smprintf (ukn, 1024, "Unknown freetype2 error, errcode: 0x%x", fte);
	return ukn;
}



/*
 * Constructor
 */
CFontGenerator::CFontGenerator (const std::string &fontFileName, const std::string &fontExFileName)
{
	FT_Error error;

	if (!_LibraryInit)
	{
		error = FT_Init_FreeType (&_Library);
		if (error)
		{
			nlerror ("FT_Init_FreeType() failed: %s", getFT2Error(error));
		}
		_LibraryInit = true;
	}

	error = FT_New_Face (_Library, fontFileName.c_str (), 0, &_Face);
	if (error)
	{
		nlerror ("FT_New_Face() failed: %s", getFT2Error(error));
	}

	if (fontExFileName != "")
	{
		error = FT_Attach_File (_Face, fontExFileName.c_str ());
		if (error)
		{
			nlerror ("FT_Attach_File() failed: %s", getFT2Error(error));
		}
	}
}



uint8 *CFontGenerator::getBitmap (ucchar c, uint32 size, uint32 &width, uint32 &height, uint32 &pitch, sint32 &left, sint32 &top, sint32 &advx, uint32 &glyphIndex)
{
	FT_Error error;
	
	error = FT_Set_Pixel_Sizes (_Face, size, size);
	if (error)
	{
		nlerror ("FT_Set_Pixel_Sizes() failed: %s", getFT2Error(error));
	}

	// retrieve glyph index from character code
	FT_UInt glyph_index = FT_Get_Char_Index (_Face, c);

	// load glyph image into the slot (erase previous one)
	error = FT_Load_Glyph (_Face, glyph_index, FT_LOAD_DEFAULT);
	if (error)
	{
		nlerror ("FT_Load_Glyph() failed: %s", getFT2Error(error));
	}

	// convert to an anti-aliased bitmap
	error = FT_Render_Glyph (_Face->glyph, ft_render_mode_normal);
	if (error)
	{
		nlerror ("FT_Render_Glyph() failed: %s", getFT2Error(error));
	}

	width = _Face->glyph->bitmap.width;
	height = _Face->glyph->bitmap.rows;
	pitch = _Face->glyph->bitmap.pitch;

	left = _Face->glyph->bitmap_left;
	top = _Face->glyph->bitmap_top;

	advx = _Face->glyph->advance.x >> 6;

	glyphIndex = glyph_index;

	return (uint8 *) _Face->glyph->bitmap.buffer;
}



void CFontGenerator::getKerning (ucchar left, ucchar right, sint32 &kernx)
{
	if (!FT_HAS_KERNING(_Face))
	{
		kernx = 0;
	}
	else
	{
		FT_Vector  kerning;
		FT_Error error = FT_Get_Kerning (_Face, left, right, ft_kerning_default, &kerning);
		if (error)
		{
			nlerror ("FT_Get_Kerning() failed: %s", getFT2Error(error));
		}
		kernx = kerning.x;
	}
}



uint32	 CFontGenerator::getCharIndex (ucchar c)
{
	return FT_Get_Char_Index (_Face, c);
}


} // NL3D
