/** \file patch.h
 * <File description>
 *
 * $Id: patch.h,v 1.4 2000/10/24 14:18:28 lecroart Exp $
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

#ifndef NL_PATCH_H
#define NL_PATCH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"


namespace NL3D {


using NLMISC::CVector;


class	CZone;
class	CTessFace;
class	CBezierPatch;


// ***************************************************************************
class	CVector3s
{
public:
	sint16	x,y,z;

	void	pack(const CVector &v, float bias, float scale)
	{
		x= (sint16)(v.x/scale - bias);
		y= (sint16)(v.y/scale - bias);
		z= (sint16)(v.z/scale - bias);
	}
	void	unpack(CVector &v, float bias, float scale)
	{
		v.x= x*scale + bias;
		v.y= y*scale + bias;
		v.z= z*scale + bias;
	}
};


// ***************************************************************************
/**
 * A landscape patch.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPatch
{
public:
	CZone		*Zone;
	/// The patch coordinates (see CBezierPatch).
	CVector3s	Vertices[4];
	CVector3s	Tangents[8];
	CVector3s	Interiors[4];
	// Tile Order for the patch.
	uint8		OrderS, OrderT;
	// The Base Size*bumpiness of the patch (/2 at each subdivide).
	// May be setup to surface of patch, modulated by tangents and dispalcement map.
	float		ErrorSize;
	// The tesselation.
	CTessFace	*Son0, *Son1;
	/*
		TODO:
		- "UV correction" infos.
		- displacement map (ptr/index).
	*/

	// Local info for tile. CPatch must setup them at the begining at refine()/render().
	// Should we compute the error metric part for tile??
	bool		ComputeTileErrorMetric;
	// For this patch, which level is required to be a valid Tile??
	sint		TileLimitLevel;


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




private:
	// unpack the patch into a floating point one.
	CBezierPatch	*unpackIntoCache();
	// unpack the patch into a floating point one.
	void			unpack(CBezierPatch	&p);

private:
	// The cache (may be a short list/vector later...).
	static	CBezierPatch	CachePatch;
	// For cahcing.
	static	CPatch			*LastPatch;
};


} // NL3D


#endif // NL_PATCH_H

/* End of patch.h */
