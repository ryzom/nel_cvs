/** \file tessellation.h
 * <File description>
 *
 * $Id: tessellation.h,v 1.3 2000/10/24 14:18:28 lecroart Exp $
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

#ifndef NL_TESSELLATION_H
#define NL_TESSELLATION_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/debug.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	CPatch;
class	CTileMaterial;


// ***************************************************************************
const	float	OO16384= 1.0f/0x8000;


// ***************************************************************************
/**
 * A Landscape Vertex
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTessVertex
{
public:
	// The current position, + geomorph.
	CVector	Pos;
	CVector	StartPos, EndPos;
	// The index of current far vertex.
	uint32			FarIndex;
};


// ***************************************************************************
class	CUV
{
public:
	float	U,V;

public:
	CUV() {}
	CUV(float u, float v) : U(u), V(v) {}
	CUV	operator+(const CUV &v) const
		{ return CUV(U+v.U, V+v.V);}
	CUV	operator-(const CUV &v) const
		{ return CUV(U-v.U, V-v.V);}
	CUV	operator*(float f) const
		{ return CUV(U*f, V*f);}
};


class	CUVW
{
public:
	float	U,V,W;

public:
	CUVW() {}
	CUVW(float u, float v, float w) : U(u), V(v), W(w) {}
	CUVW	operator+(const CUVW &v) const
		{ return CUVW(U+v.U, V+v.V, W+v.W);}
	CUVW	operator-(const CUVW &v) const
		{ return CUVW(U-v.U, V-v.V, W-v.W);}
	CUVW	operator*(float f) const
		{ return CUVW(U*f, V*f, W*f);}
};


// ***************************************************************************
/**
 * A Landscape Base TileUv.
 * virtual inheritance would have be usefull, for automatic clone(), middle() etc... but too heavy (a vftable...).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	ITileUv
{
public:
	// The dtor should be virtual (if any), but since dtor is a no-op, don't add it.
};


// TileUv format, without BumpMapping support.
// Format; PXUvY: PassX / UvY.
// **************************************
class	CTileUv1Tf2 : public ITileUv
{
public:
	CUV		P0Uv0, P0Uv1;
public:
	CTileUv1Tf2() {}
	// Create as the middle of two CTileUv1Tf2.
	CTileUv1Tf2(const CTileUv1Tf2 *uv0, const CTileUv1Tf2 *uv1)
	{
		P0Uv0= (uv0->P0Uv0 + uv1->P0Uv0)*0.5;
		P0Uv1= (uv0->P0Uv1 + uv1->P0Uv1)*0.5;
	}
};
// **************************************
class	CTileUv2Tf2 : public CTileUv1Tf2
{
public:
	// Herited: CUV		P0Uv0, P0Uv1;
	CUV		P1Uv0, P1Uv1;
public:
	CTileUv2Tf2() {}
	// Create as the middle of two CTileUv2Tf2.
	CTileUv2Tf2(const CTileUv2Tf2 *uv0, const CTileUv2Tf2 *uv1) : CTileUv1Tf2(uv0,uv1)
	{
		P1Uv0= (uv0->P1Uv0 + uv1->P1Uv0)*0.5;
		P1Uv1= (uv0->P1Uv1 + uv1->P1Uv1)*0.5;
	}
};
// **************************************
class	CTileUv3Tf2 : public CTileUv2Tf2
{
public:
	// Herited: CUV		P0Uv0, P0Uv1;
	// Herited: CUV		P1Uv0, P1Uv1;
	CUV		P2Uv0, P2Uv1;
public:
	CTileUv3Tf2() {}
	// Create as the middle of two CTileUv3Tf2.
	CTileUv3Tf2(const CTileUv3Tf2 *uv0, const CTileUv3Tf2 *uv1) : CTileUv2Tf2(uv0,uv1)
	{
		P2Uv0= (uv0->P2Uv0 + uv1->P2Uv0)*0.5;
		P2Uv1= (uv0->P2Uv1 + uv1->P2Uv1)*0.5;
	}
};



// TileUv format, with BumpMapping support.
// Format; PXUvY: PassX / UvY.
// **************************************
class	CTileUv1Tf4 : public ITileUv
{
public:
	CUV		P0Uv0, P0Uv1, P0Uv2;
	CUVW	P0Uvw3;
public:
	CTileUv1Tf4() {}
	// Create as the middle of two CTileUv1Tf4.
	CTileUv1Tf4(const CTileUv1Tf4 *uv0, const CTileUv1Tf4 *uv1)
	{
		P0Uv0= (uv0->P0Uv0 + uv1->P0Uv0)*0.5;
		P0Uv1= (uv0->P0Uv1 + uv1->P0Uv1)*0.5;
		P0Uv2= (uv0->P0Uv2 + uv1->P0Uv2)*0.5;
		P0Uvw3= (uv0->P0Uvw3 + uv1->P0Uvw3)*0.5;
	}
};
// **************************************
class	CTileUv2Tf4 : public CTileUv1Tf4
{
public:
	// Herited: CUV		P0Uv0, P0Uv1, P0Uv2;
	// Herited: CUVW	P0Uvw3;
	CUV		P1Uv0, P1Uv1, P1Uv2;
	CUVW	P1Uvw3;
public:
	CTileUv2Tf4() {}
	// Create as the middle of two CTileUv2Tf4.
	CTileUv2Tf4(const CTileUv2Tf4 *uv0, const CTileUv2Tf4 *uv1) : CTileUv1Tf4(uv0, uv1)
	{
		P1Uv0= (uv0->P1Uv0 + uv1->P1Uv0)*0.5;
		P1Uv1= (uv0->P1Uv1 + uv1->P1Uv1)*0.5;
		P1Uv2= (uv0->P1Uv2 + uv1->P1Uv2)*0.5;
		P1Uvw3= (uv0->P1Uvw3 + uv1->P1Uvw3)*0.5;
	}
};
// **************************************
class	CTileUv3Tf4 : public CTileUv2Tf4
{
public:
	// Herited: CUV		P0Uv0, P0Uv1, P0Uv2;
	// Herited: CUVW	P0Uvw3;
	// Herited: CUV		P1Uv0, P1Uv1, P1Uv2;
	// Herited: CUVW	P1Uvw3;
	CUV		P2Uv0, P2Uv1, P2Uv2;
	CUVW	P2Uvw3;
public:
	CTileUv3Tf4() {}
	// Create as the middle of two CTileUv3Tf4.
	CTileUv3Tf4(const CTileUv3Tf4 *uv0, const CTileUv3Tf4 *uv1) : CTileUv2Tf4(uv0, uv1)
	{
		P2Uv0= (uv0->P2Uv0 + uv1->P2Uv0)*0.5;
		P2Uv1= (uv0->P2Uv1 + uv1->P2Uv1)*0.5;
		P2Uv2= (uv0->P2Uv2 + uv1->P2Uv2)*0.5;
		P2Uvw3= (uv0->P2Uvw3 + uv1->P2Uvw3)*0.5;
	}
};



// ***************************************************************************
/**
 * A Landscape Triangle.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTessFace
{
public:
	class	CParamCoord
	{
	public:
		// The parametric coordinates of the patch. 0x0000<=>0.0f. 0x8000<=> 1.0f.
		uint16	S,T;

	public:
		CParamCoord() {}
		CParamCoord(uint16 s, uint16 t);
		CParamCoord	operator+(const CParamCoord &v) const	{return CParamCoord(S+v.S, T+v.T);}
		CParamCoord	operator-(const CParamCoord &v) const	{return CParamCoord(S-v.S, T-v.T);}
		CParamCoord	shifted() const	{return CParamCoord(S>>1, T>>1);}
		float	getS() const {return S*OO16384;}
		float	getT() const {return T*OO16384;}
		// vertex on the border?
		bool	onBorder() const {return (S==0 || S==0x8000 || T==0 || T==0x8000);}
	};

public:

	/// \name geometric tesselation.
	//@{
	// Can't be global, because of propagated splits.
	CPatch			*Patch;
	CTessVertex		*VBase, *VLeft, *VRight;
	CTessFace		*FBase, *FLeft, *FRight;
	CTessFace		*Father, *SonLeft, *SonRight;
	// The parametric coordinates of the patch.
	CParamCoord		PVBase, PVLeft, PVRight;
	// To know if we can split or not, and to compute tile errormetric.
	uint8			Level;			// Level of Detail of this face. (++ at each split).
	// See ErrorMetric part. NeedCompute says if this face need to compute his errorMetric.
	bool			NeedCompute;
	//@}

	/// \name Tile Material Infos (uvs...).
	// @{
	// The number of the tile relatively to the patch TileMap.
	uint8			TileId;
	// The type of the tile.
	uint8			TileFmt:5;		// (1,2 or 3 t2f0/t2f1,  1,2 or 3 t2f0/t2f1/t2f2/t3f3).
	uint8			TileMat:3;		// Normal / Spec / Alpha / AlphaSpec / SpecAlpha
	// The tile Materials. Each material=1 pass.
	CTileMaterial	*TilePass0;				//  general Tile.
	CTileMaterial	*TilePass1;				// (may be NULL) 2nd Tile Alpha. Or specular tile for TilePass0.
	CTileMaterial	*TilePass2;				// (may be NULL) 2nd Tile Alpha. Or specular tile for TilePass1 (2nd tile alpha).
	// Uv for tiles. ITileUv is either: 1,2 or 3 t2f0/t2f1,  either:  1,2 or 3 t2f0/t2f1/t2f2/t3f3.
	// color4ub are computed each frame (alpha= global night alpha, RGB= clouds color).
	ITileUv			*TileUvBase, *TileUvLeft, *TileUvRight;
	// @}


	/// \name Error metric.
	// @{
	// Size could be computed from TileSize: Size= Patch->BaseSize / (1<<(TileSize-Patch->BaseTileSize)). (or optimized)
	// But used by computeErrorMetric(), and must be as fast as possible.
	float			Size;				// /2 at each split.
	CVector			Center;				// Center of the face.
	float			ProjectedSize;		// The result of errormetric: the projected size of face.
	sint			ProjectedSizeDate;	// The date of errormetric update.
	// @}


public:
	CTessFace();
	// do nothing dtor.
	~CTessFace() {}

	// Utilities.
	bool	isLeaf() const {return SonLeft==NULL;}
	bool	hasVertex(CTessVertex *v) const {return VBase==v || VLeft==v || VRight==v;}
	bool	hasEdge(CTessVertex *v0, CTessVertex *v1) const {return hasVertex(v0) && hasVertex(v1);}
	void	changeNeighboor(CTessFace *from, CTessFace *to)
	{
		if(FBase==from) FBase=to;
		if(FLeft==from) FLeft=to;
		if(FRight==from) FRight=to;
	}

	// Fill all Tile Material infos (id, type...), according to CPatch tilemap, and CTessFace paramcoord.
	void	computeTileMaterial();
	// Release Tile Uvs.
	void	releaseTileMaterial();

	// update the error metric (even if !NeedCompute).
	float	updateErrorMetric();
	// can split leaf only.
	void	split();
	// can merge "short roots" only (roots which have leafs).
	bool	merge();
	// if NeedCompute, refine the node, and his sons.
	void	refine();


public:

	// LANDSCAPE GLOBAL INTERFACE.  Landscape must setup them at the begining at refine()/render().
	// The current date of LandScape draw/refine.
	static	sint	CurrentDate;
	// The center view for refinement.
	static	CVector RefineCenter;
	// What is the treshold for tessellation.
	static	float	RefineThreshold;
	// Guess.
	static	float	OORefineThreshold;
	// Limits for merge/split propagation. See refine(). Defaults:  1.1, 0.9,  1.9, 2.1.
	static	float	FatherStartComputeLimit, SelfEndComputeMin;
	static	float	ChildrenStartComputeLimit, SelfEndComputeMax;


	// Tile Global Info.
	// What are the limit distances for Tile tesselation transition.
	static	float	TileDistNear, TileDistFar;
	// Must be set as   1.0f/(CTessFace::TileDistFar - CTessFace::TileDistNear);
	static	float	OOTileDistDelta;
	// The tiles are not subdivided above this limit (but because of enforced splits). Default: 4 => 50cm.
	static	sint	TileMaxSubdivision;



	// PATCH GLOBAL INTERFACE.  patch must setup them at the begining at refine()/render().
	// NO!!! REMIND: can't have any patch global, since a propagated split()/updateErrorMetric()
	// can arise. must use Patch pointer.


private:
	// Faces have the same tile???
	static bool			sameTile(const CTessFace *a, const CTessFace *b) 
		{return (a->Patch==b->Patch && a->TileId==b->TileId);}
	// Alloc a tile.
	static ITileUv		*allocTileUv(uint8 fmt);
};


} // NL3D


#endif // NL_TESSELLATION_H

/* End of tessellation.h */
