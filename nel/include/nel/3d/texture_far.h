/** \file texture_far.h
 * <File description>
 *
 * $Id: texture_far.h,v 1.3 2000/12/22 10:42:05 corvazier Exp $
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

#ifndef NL_TEXTURE_FAR_H
#define NL_TEXTURE_FAR_H

#include "nel/misc/types_nl.h"
#include "nel/3d/texture.h"
#include "nel/misc/rect.h"


namespace NL3D {

class CPatch;

/**
 * A CTextureFar is a set of texture used to map a whole patch when it is in far Mode. (ie not in tile mode).
 * A CTextureFar handle several tile
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTextureFar : public ITexture
{
public:

	/// Patch identifier
	struct CPatchIdent
	{
		/// Default constructor. do nothing, only for vector
		CPatchIdent () {};

		/// Constructor
		CPatchIdent (CPatch* patch, uint8 order)
		{
			Patch=patch;
			Order=order;
		}

		// Data

		// Patch pointer
		CPatch*	Patch;

		// Zone Id of the patch
		uint8	Order;
	};

	/// Constructor
	CTextureFar()
	{
		// This texture is not releasable. It stays in standard memory.
		setReleasable (false);
	}

	/// Number of patches in the width
	uint8						_WidthPatches;

	/// Number of patches in the height
	uint8						_HeightPatches;

	/**
	 *  Vector of patches which texture far is stored in this CTextureFar
	 *  Should be == to _WidthPatches*_HeightPatches
	 */
	std::vector<CPatchIdent>	_Patches;

	/**
	 *  Generate the texture. See ITexture::generate().
	 *
	 *  \see ITexture::generate()
	 */
	virtual void				generate();
};


} // NL3D


#endif // NL_TEXTURE_FAR_H

/* End of texture_far.h */
