/** \file tile_color.h
 * <File description>
 *
 * $Id: tile_color.h,v 1.2 2001/08/21 16:18:55 corvazier Exp $
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

#ifndef NL_TILE_COLOR_H
#define NL_TILE_COLOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"



namespace NL3D {


// ***************************************************************************
/**
 * A basic color Element for CPatchTexture.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTileColor
{
public:
	/// The RGB user color.
	uint16			Color565;
	/// For Bump: The Light Vector in Patch Tangent Space.
	uint8			LightX,LightY,LightZ;

public:

	/// Constructor
	CTileColor() {}

	void	serial(NLMISC::IStream &f)
	{
		f.serial(Color565);
		f.serial(LightX,LightY,LightZ);
	}
};


} // NL3D


#endif // NL_TILE_COLOR_H

/* End of tile_color.h */
