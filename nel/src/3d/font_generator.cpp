/** \file font_generator.cpp
 * CFontGenerator class
 *
 * $Id: font_generator.cpp,v 1.1 2000/11/09 17:42:21 lecroart Exp $
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

#undef FTERRORS_H
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
	sprintf (ukn, "Unknown freetype2 error, errcode: 0x%x", fte);
	return ukn;
}

/*
 * Constructor
 */
CFontGenerator::CFontGenerator(const char *fontFileName)
{
	FT_Error error;

	if (!_LibraryInit)
	{
		error = FT_Init_FreeType (&_Library);
		if (error)
		{
			nlerror ("FT_Init_FreeType() failed: %s", getFT2Error(error));
		}
		else		_LibraryInit = true;
	}

	error = FT_New_Face (_Library, fontFileName, 0, &_Face);
	if (error)
	{
		nlerror ("FT_New_Face() failed: %s", getFT2Error(error));
	}
}

uint8 *CFontGenerator::getBitmap (ucchar c, uint32 size, uint32 &width, uint32 &height, uint32 &pitch)
{
	FT_Error error;

	error = FT_Set_Pixel_Sizes (_Face, size, size);
	if (error)
	{
		nlerror ("FT_Set_Pixel_Sizes() failed: %s", getFT2Error(error));
	}

	error = FT_Load_Char (_Face, c, FT_LOAD_RENDER);
	if (error)
	{
		nlerror ("FT_Load_Char() failed: %s", getFT2Error(error));
	}

	width = _Face->glyph->bitmap.width;
	height = _Face->glyph->bitmap.rows;
	pitch = _Face->glyph->bitmap.pitch;

	return (uint8 *) _Face->glyph->bitmap.buffer;
}


} // NL3D
