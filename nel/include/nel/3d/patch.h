/** \file patch.h
 * <File description>
 *
 * $Id: patch.h,v 1.5 2000/10/27 14:29:42 berenguier Exp $
 * \todo yoyo:
		- "UV correction" infos.
		- displacement map (ptr/index).
		- bind with multiple patchs.
		- rectangular subdivsion.
 *
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
#include "nel/3d/tessellation.h"
#include "nel/3d/aabbox.h"


namespace NL3D {


using NLMISC::CVector;


class	CZone;
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
	void	unpack(CVector &v, float bias, float scale) const
	{
		v.x= x*scale + bias;
		v.y= y*scale + bias;
		v.z= z*scale + bias;
	}
};


// ***************************************************************************
/**
 * A landscape patch.
 * QuadPatch layout (same notations as Max).
 * 
 *   A---> ad ----- da <---D
 *   |                     |
 *   |                     |
 *   v                     v
 *   ab    ia       id     dc
 *
 *   |                     |
 *   |                     |
 *
 *   ba    ib       ic     cd
 *   ^                     ^
 *   |                     |
 *   |                     |
 *   B---> bc ----- cb <---C
 *
 * NB: Patch 1x1 or 1xX are illegal: lot of problem: rectangular geomoprh, Son0 and Son1 must be setup as tile at beginning ...
 *
 * NB: Edges number are:
 *	- 0: AB.
 *	- 1: BC.
 *	- 2: CD.
 *	- 3: DA.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CPatch
{
public:

	struct	CBindInfo
	{
		sint			NPatchs;	// The number of patchs on this edge. 0,1, 2 or 4.
		CPatch			*Next0;		// The neighbor patch.
		sint			Edge0;		// On which edge of Next0 we are binded.

		// only usefull for Bind One/Two
		CPatch			*Next1;		// ....
		sint			Edge1;

		// only usefull for Bind One/Four
		CPatch			*Next2;		// ....
		sint			Edge2;
		CPatch			*Next3;
		sint			Edge3;
	};

public:
	/// The patch coordinates (see CBezierPatch).
	CVector3s		Vertices[4];
	CVector3s		Tangents[8];
	CVector3s		Interiors[4];
	/*
		TODO:
		- "UV correction" infos.
		- displacement map (ptr/index).
	*/


public:

	/// Constructor
	CPatch();
	/// dtor
	~CPatch();

	/** compile a valid patch. (init)
	 * Call this method before any other. Zone and Control points must be valid before calling compile(). \n
	 * This is an \b ERROR to call compile() two times. \n
	 * NB: zone loading must call compile() too, using the order and the errorSize saved.
	 * \param z zone where the patch must be binded.
	 * \param orderS the Tile order in S direction: 2,4,8,16.
	 * \param orderT the Tile order in T direction: 2,4,8,16.
	 * \param errorSize if 0, setup() compute himself the errormetric of the patch. May be setup to surface of patch, 
	 *  modulated by tangents and displacement map.
	 */
	void			compile(CZone *z, uint8 orderS, uint8 orderT, CTessVertex *baseVertices[4], float errorSize=0);


	CZone			*getZone() const {return Zone;}
	uint8			getOrderS() const {return OrderS;}
	uint8			getOrderT() const {return OrderT;}
	float			getErrorSize() const {return ErrorSize;}

	/// Build the bbox of the patch, according to ctrl points, and displacement map max value.
	CAABBox			buildBBox() const;

	/** Compute a vertex.
	 * Compute a vertex according to:
	 *	- s,t
	 *	- patch control points uncompressed with zone Bias/Scale.
	 *  - Patch UV geometric correction.
	 *	- Patch noise (and noise of Patch neighbor).
	 */
	CVector			computeVertex(float s, float t);


	/// unbind the patch from neighbors.
	void			unbind();

	/// bind the patch to 4 neighbors, given in this patch edge order (0,1,2,3). Tesselation is reseted (patch unbound first).
	void			bind(CBindInfo	Edges[4]);


	/// Refine / geomorph this patch.
	void			refine();
	/// Render this patch (append to VertexBuffers / materials primitive block).
	void			render();



// Private part.
private:
/*********************************/

	friend	class CTessFace;

	CZone			*Zone;
	// Tile Order for the patch.
	uint8			OrderS, OrderT;
	// For this patch, which level is required to be a valid Tile??
	sint			TileLimitLevel;
	// The Base Size*bumpiness of the patch (/2 at each subdivide).
	float			ErrorSize;
	// The root for tesselation.
	CTessFace		*Son0, *Son1;
	// The base vertices.
	CTessVertex		*BaseVertices[4];


	// Local info for CTessFace tiles. CPatch must setup them at the begining at refine()/render().
	// For Far Texture coordinates.
	float			Far0UVScale, Far0UBias, Far0VBias;
	float			Far1UVScale, Far1UBias, Far1VBias;
	// TODOR: add the texture (material) pointers of texture fars.


private:
	// Guess.
	void			computeDefaultErrorSize();
	// based on BaseVertices, recompute positions, and Make Face roots Son0 and Son1.
	void			makeRoots();
	// Guess. For bind() reasons.
	CTessFace		*getRootFaceForEdge(sint edge) const;
	void			changeEdgeNeighbor(sint edge, CTessFace *to);


private:
	// The Patch cache (may be a short list/vector later...).
	static	CBezierPatch	CachePatch;
	// For cahcing.
	static	const CPatch	*LastPatch;

	// unpack the patch into a floating point one.
	CBezierPatch	*unpackIntoCache() const;
	// unpack the patch into a floating point one.
	void			unpack(CBezierPatch	&p) const;

};


} // NL3D


#endif // NL_PATCH_H

/* End of patch.h */
