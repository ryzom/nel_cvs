/** \file texture.cpp
 * ITexture & CTextureFile
 *
 * $Id: texture.cpp,v 1.1 2000/11/07 16:22:24 coutelas Exp $
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
#include "nel/misc/file.h"


namespace NL3D {


/*==================================================================*\
							ITEXTURE
\*==================================================================*/





/*==================================================================*\
							CTEXTUREFILE
\*==================================================================*/

/*------------------------------------------------------------------*\
							generate
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
							generate
\*------------------------------------------------------------------*/
void CTextureFont::generate()
{
	
}



} // NL3D
