/** \file patch.h
 * <File description>
 *
 * $Id: patch.h,v 1.1 2000/10/23 12:13:42 berenguier Exp $
 */

/** Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_PATCH_H
#define NL_PATCH_H

#include "nel/misc/types_nl.h"


namespace NL3D {


/**
 * <Class description>
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPatch
{
public:
	// Tile Order for the patch.
	uint8	OrderS, OrderT;

public:

	/// Constructor
	CPatch();

	/** Compute a vertex.
	 * Compute a vertex according to:
	 *	- s,t
	 *  - Patch UV geometric correction.
	 *	- Patch noise (and noise of Patch neighboor).
	 */
	CVector	computeVertex(float s, float t);


public:
	// Local info for tile. CPatch must setup them at the begining at refine()/render().
	// Should we compute the error metric part for tile??
	bool	ComputeTileErrorMetric;
	// For this patch, which level is required to be a valid Tile??
	sint	TileLimitLevel;
};


} // NL3D


#endif // NL_PATCH_H

/* End of patch.h */
