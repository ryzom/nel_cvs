/** \file tile_element.cpp
 * <File description>
 *
 * $Id: tile_element.cpp,v 1.3 2000/12/05 18:13:42 berenguier Exp $
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
	sint	where= NL_TILE_ELM_SIZE_ROTATE*i+NL_TILE_ELM_OFFSET_ROTATE;
	Flags&= ~(NL_TILE_ELM_MASK_ROTATE<<where);
	Flags|= orient<<where;
}


uint8	CTileElement::getTileOrient(sint i) const
{
	nlassert(i>=0 && i<=2);
	sint	where= NL_TILE_ELM_SIZE_ROTATE*i+NL_TILE_ELM_OFFSET_ROTATE;
	return uint8 ((Flags>>where) & NL_TILE_ELM_MASK_ROTATE);
}


void	CTileElement::setTileUvInfo(bool is256x256, uint8 uvOff)
{
	nlassert(uvOff>=0 && uvOff<=3);
	sint	where= NL_TILE_ELM_OFFSET_UVINFO;
	sint	info= uvOff+(is256x256?4:0);
	Flags&= ~(NL_TILE_ELM_MASK_UVINFO<<where);
	Flags|= info<<where;
}


void	CTileElement::getTileUvInfo(bool &is256x256, uint8 &uvOff) const
{
	sint	where= NL_TILE_ELM_OFFSET_UVINFO;
	sint	info= ((Flags>>where) & NL_TILE_ELM_MASK_UVINFO);
	uvOff= info&3;
	is256x256= (info&4)?true:false;
}


void	CTileElement::serial(NLMISC::IStream &f)
{
	f.serial(Flags);
	f.serial(Tile[0]);
	f.serial(Tile[1]);
	f.serial(Tile[2]);
}


} // NL3D
