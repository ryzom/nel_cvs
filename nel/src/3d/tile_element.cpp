/** \file tile_element.cpp
 * <File description>
 *
 * $Id: tile_element.cpp,v 1.2 2000/12/01 16:35:34 corvazier Exp $
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

#include "nel/3d/tile_element.h"
#include "nel/misc/debug.h"


namespace NL3D 
{


void	CTileElement::setTileOrient(sint i, uint8 orient)
{
	nlassert(i>=0 && i<=2);
	nlassert(orient>=0 && orient<=3);
	Flags&= ~(NL_TILE_ELM_MASK_ROTATE<<(NL_TILE_ELM_SIZE_ROTATE*i));
	Flags|= orient<<(NL_TILE_ELM_SIZE_ROTATE*i+NL_TILE_ELM_OFFSET_ROTATE);
}


uint8	CTileElement::getTileOrient(sint i) const
{
	nlassert(i>=0 && i<=2);
	return uint8((Flags>>(NL_TILE_ELM_SIZE_ROTATE*i+NL_TILE_ELM_OFFSET_ROTATE)) & ((1<<NL_TILE_ELM_SIZE_ROTATE)-1));
}


void	CTileElement::serial(NLMISC::IStream &f)
{
	f.serial(Flags);
	f.serial(Tile[0]);
	f.serial(Tile[1]);
	f.serial(Tile[2]);
}


} // NL3D
