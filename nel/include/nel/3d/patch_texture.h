/** \file patch_texture.h
 * <File description>
 *
 * $Id: patch_texture.h,v 1.1 2000/11/30 10:57:28 berenguier Exp $
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

#ifndef NL_PATCH_TEXTURE_H
#define NL_PATCH_TEXTURE_H

#include "nel/misc/types_nl.h"
#include <vector>


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
	/** The three tile ident. 0 means no Tile for this pass. Tile[0] must be !=0.
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
};


// ***************************************************************************
/**
 * An Array2D of multiple tile with UV orientations info, etc... Used to put on a patch.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPatchTexture
{
public:
	// The tiles
	vector<CTileElement>	Tiles;

	/// Constructor
	CPatchTexture() {}

};


} // NL3D


#endif // NL_PATCH_TEXTURE_H

/* End of patch_texture.h */
