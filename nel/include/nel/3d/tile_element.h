/** \file tile_element.h
 * <File description>
 *
 * $Id: tile_element.h,v 1.1 2000/11/30 10:57:38 berenguier Exp $
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

#ifndef NL_TILE_ELEMENT_H
#define NL_TILE_ELEMENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"


namespace NL3D 
{


// ***************************************************************************
/**
 * An Element for CPatchTexture. Temporary! since CPatchTexture should be compressed...
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTileElement
{
private:
	uint16	Flags;	// Tile Orientation etc...

public:
	/** The three tile ident. 0xFFFF means no Tile for this pass. Tile[0] must be !=0xFFFF.
	 *
	 */
	uint16	Tile[3];
	/** Set the tile orientation of tile of pass i, to "orient".
	 * orient E [0,3]. The rotation is CCW.
	 */
	void	setTileOrient(sint i, uint8 orient);
	/** Get the tile orientation of tile of pass i.
	 * orient E [0,3]. The rotation is CCW.
	 */
	uint8	getTileOrient(sint i);


	void	serial(NLMISC::IStream &f);
};



} // NL3D


#endif // NL_TILE_ELEMENT_H

/* End of tile_element.h */
