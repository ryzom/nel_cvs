/** \file texture.cpp
 * ITexture & CTextureFile
 *
 * $Id: texture.cpp,v 1.12 2000/12/15 18:20:22 berenguier Exp $
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
#include <vector>


namespace NL3D 
{


/*==================================================================*\
							ITEXTURE
\*==================================================================*/

ITexture &ITexture::operator=(const ITexture &tex)
{
	// The operator= do not copy drv info
	// set touched=true. _Releasable is copied.
	_Touched= true;
	_Releasable= tex._Releasable;
	return *this;
}


ITexture::~ITexture()
{
	// Must kill the drv mirror of this texture.
	TextureDrvShare.kill();
}




} // NL3D
