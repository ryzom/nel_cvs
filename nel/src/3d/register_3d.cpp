/** \file register_3d.cpp
 * <File description>
 *
 * $Id: register_3d.cpp,v 1.1 2000/12/12 10:04:48 berenguier Exp $
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


#include "nel/3d/register_3d.h"
#include "nel/3d/texture.h"


namespace NL3D 
{


//****************************************************************************
void	registerSerial3d()
{
	NLMISC_REGISTER_CLASS(CTextureFile);
	NLMISC_REGISTER_CLASS(CTextureMem);
	NLMISC_REGISTER_CLASS(CTextureFont);
	// Don't register CTextureCroos,, since local, and not designed to be serialised.
}


} // NL3D
