/** \file tessellation.h
 * <File description>
 *
 * $Id: tessellation.h,v 1.8 2000/11/07 17:25:23 berenguier Exp $
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
#include "nel/3d/driver.h"
#include <vector>


namespace	NL3D
{


// Odd pass are always additive pass.
#define	NL3D_MAX_TILE_PASS 6


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	CPatch;


// ***************************************************************************
const	float	OO32768= 1.0f/0x8000;


// ***************************************************************************
/**
 * A render pass for a landscape material (tile or texture far).
 * A Primitive Block is not used, for speed improvement: must not realloc at each face or at each patch...
 *
 * How does it work: class CPatchRdrPass maintain a GlobalTriList array, where tri indices are stored.
 * Since Tris may be added in a interleaved fashion (33 tris indices for material 0, then 12 tris indices for material 1, 
 * then 60 tris indices for material 0 etc...), this GlobalTriList is so interleaved (with some indices used for interleaving).
 *
 * NB: GlobalTriList is a "grow only" vector, even across frames.... So reallocation never happens, but at begin of program.
 */
class	CPatchRdrPass
{
public:
	CMaterial		*Mat;
	// The current number of tris for this rdrpass.
	sint			NTris;
	// Where this RdrPass begin, in the GlobalTriList.
	sint			StartIndex;
	// The current/end index for this RdrPass GlobalTriList.
	sint			CurIndex;
	// The BlockLen index, to know what is the length of a block (in Triangles).
	sint			BlockLenIndex;

	// Format of a single block:
	// |LEN|ID0|ID1|ID2|ID0|ID1|ID2....|JMP|

public:
	CPatchRdrPass();
	void			addTri(uint32 idx0, uint32 idx1, uint32 idx2);
	void			resetTriList();
	void			buildPBlock(CPrimitiveBlock &pb);


public:
	// Must resetTriList() of all material using the GlobalTriList, before calling resetGlobalTriList.
	static void		resetGlobalTriList();

private:
	static sint					CurGlobalIndex;
	static std::vector<uint32>	GlobalTriList;

};


// ***************************************************************************
struct	CTileMaterial
{
	// The type of the tile.
	uint8			TileUvFmt;		// 4 first bits to know the number of Uv needed.  5th bit: 0 -> no bump. 16 -> bump. 
	// The LUT to know which Uv to be used, for which pass.
	uint8			PassToUv[NL3D_MAX_TILE_PASS];
	// The rendering passes materials.
	CPatchRdrPass	*Pass[NL3D_MAX_TILE_PASS];
	// Pass are:
	//  0: general Tile.
	//  1: additive pass for general Tile.
	//  2: 1st alpha tile.
	//  3: additive pass for 1st alpha tile.
	//  4: 2nd alpha tile.
	//  5: additive pass for 2nd alpha tile.

	// ptrs are Null by default.
	CTileMaterial();
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
 *
 * NB: why don't use a UvPasses ptr, and aloc it on demand??? For future allocation improvement, where we need to allocate
 * fixed size element.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	ITileUv
{
public:
	// The dtor should be virtual (if any), but since dtor is a no-op, don't add it.

	// The vertex index for the tile.
	uint32			TileIndex;
};



// Uv Passes.
// **************************************
struct	CPassUvNormal
{
	CUV		PUv0, PUv1;

	void	makeMiddle(const CPassUvNormal &uv0, const CPassUvNormal &uv1)
	{
		PUv0= (uv0.PUv0 + uv1.PUv0)*0.5;
		PUv1= (uv0.PUv1 + uv1.PUv1)*0.5;
	}
};
struct	CPassUvBump : public CPassUvNormal
{
	// Herited: CUV	PUv0, PUv1;
	CUV		PUv2;
	CUVW	PUvw3;

	void	makeMiddle(const CPassUvBump &uv0, const CPassUvBump &uv1)
	{
		PUv0= (uv0.PUv0 + uv1.PUv0)*0.5;
		PUv1= (uv0.PUv1 + uv1.PUv1)*0.5;
		PUv2= (uv0.PUv2 + uv1.PUv2)*0.5;
		PUvw3= (uv0.PUvw3 + uv1.PUvw3)*0.5;
	}
};


// TileUv format, without BumpMapping support.
// **************************************
class	ITileUvNormal : public ITileUv
{
public:
	CPassUvNormal	*UvPasses;
};
// ******
template<sint NPass>
class	CTileUvNormalT : public ITileUvNormal
{
public:
	CPassUvNormal	MyUvPasses[NPass];
public:
	CTileUvNormalT() {UvPasses= MyUvPasses;}
	// Create as the middle.
	CTileUvNormalT(const CTileUvNormalT *uva, const CTileUvNormalT *uvb)
	{
		UvPasses= MyUvPasses;
		for(sint i=0;i<NPass;i++)
			UvPasses[i].makeMiddle(uva->UvPasses[i], uvb->UvPasses[i]);
	}
};


typedef	CTileUvNormalT<1>	CTileUvNormal1;
typedef	CTileUvNormalT<2>	CTileUvNormal2;
typedef	CTileUvNormalT<3>	CTileUvNormal3;
typedef	CTileUvNormalT<4>	CTileUvNormal4;
typedef	CTileUvNormalT<5>	CTileUvNormal5;
typedef	CTileUvNormalT<6>	CTileUvNormal6;


// TileUv format, with BumpMapping support.
// **************************************
class	ITileUvBump : public ITileUv
{
public:
	CPassUvBump		*UvPasses;
};
// ******
template<sint NPass>
class	CTileUvBumpT : public ITileUvBump
{
private:
	CPassUvBump		MyUvPasses[NPass];

public:
	CTileUvBumpT() {UvPasses= MyUvPasses;}
	// Create as the middle.
	CTileUvBumpT(const CTileUvBumpT *uva, const CTileUvBumpT *uvb)
	{
		UvPasses= MyUvPasses;
		for(sint i=0;i<NPass;i++)
			UvPasses[i].makeMiddle(uva->UvPasses[i], uvb->UvPasses[i]);
	}
};


typedef	CTileUvBumpT<1>	CTileUvBump1;
typedef	CTileUvBumpT<2>	CTileUvBump2;
typedef	CTileUvBumpT<3>	CTileUvBump3;
typedef	CTileUvBumpT<4>	CTileUvBump4;
typedef	CTileUvBumpT<5>	CTileUvBump5;
typedef	CTileUvBumpT<6>	CTileUvBump6;



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
	CVector			Pos;
	CVector			StartPos, EndPos;
	// The index of current far vertex.
	uint32			FarIndex;

	CTessVertex()
	{
		// Must init to 0.
		FarIndex= 0;
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
		CParamCoord(uint16 s, uint16 t) {S=s; T=t;}
		CParamCoord	operator+(const CParamCoord &v) const	{return CParamCoord(S+v.S, T+v.T);}
		CParamCoord	operator-(const CParamCoord &v) const	{return CParamCoord(S-v.S, T-v.T);}
		CParamCoord	shifted() const	{return CParamCoord(S>>1, T>>1);}
		// Get s,t as floats. returned s,t E [0,1].
		float	getS() const {return S*OO32768;}
		float	getT() const {return T*OO32768;}
		// Set s,t as float. s,t E [0,1].
		void	setST(float s, float t) {S= (sint16)(s*32768);T= (sint16)(t*32768);}
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
	// The multi-pass tile Material.
	CTileMaterial	*TileMaterial;
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


	// For Render.
	CTessFace		*RenderNext;

public:
	CTessFace();
	// do nothing dtor.
	~CTessFace();

	// Utilities.
	bool			isLeaf() const {return SonLeft==NULL;}
	bool			hasVertex(CTessVertex *v) const {return VBase==v || VLeft==v || VRight==v;}
	bool			hasEdge(CTessVertex *v0, CTessVertex *v1) const {return hasVertex(v0) && hasVertex(v1);}
	void			changeNeighbor(CTessFace *from, CTessFace *to)
	{
		if(FBase==from) FBase=to;
		if(FLeft==from) FLeft=to;
		if(FRight==from) FRight=to;
	}

	// Fill all Tile Material infos (id, type...), according to CPatch tilemap, and CTessFace paramcoord.
	void			computeTileMaterial();
	// Release Tile Uvs.
	void			releaseTileMaterial();

	// update the error metric (even if !NeedCompute).
	float			updateErrorMetric();
	// can split leaf only.
	void			split();
	// can merge "short roots" only (roots which have leafs).
	bool			merge();
	// if NeedCompute, refine the node, and his sons.
	void			refine();

	// Build the render list. (insert leaves...).
	void			appendToRenderList(CTessFace *&root);


	// Used by CPatch::unbind(). isolate the tesselation from other patchs.
	void			unbind();
	// Used by CPatch::unbind(). force the merging of face.
	void			forceMerge();
	// Used by CPatch::bind(). Split if necessary, according to neighbors.
	bool			updateBindEdge(CTessFace	*&edgeFace);
	void			updateBind();



public:

	// LANDSCAPE RENDERING CONTEXT.  Landscape must setup it at the begining at refine()/render().
	// The current date of LandScape for refine only.
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


	// Render Global info. Used by Patch.
	// The distance transition for Far0 and Far1 (200m / 400m).
	static	float	Far0Dist, Far1Dist;
	// Distance for Alpha blend transition
	static	float	FarTransition;
	// The current VertexBuffer.
	static	CVertexBuffer	*CurrentVB;
	// The current Vertex Index for each pass of landscape render. Start at 1.
	static	sint	CurrentVertexIndex;

	// PATCH GLOBAL INTERFACE.  patch must setup them at the begining at refine()/render().
	// NO!!! REMIND: can't have any patch global, since a propagated split()/updateErrorMetric()
	// can arise. must use Patch pointer.


private:
	// Faces have the same tile???
	static bool		sameTile(const CTessFace *a, const CTessFace *b) 
		{return (a->Patch==b->Patch && a->TileId==b->TileId);}
	// Alloc a tile.
	static ITileUv	*allocTileUv(uint8 fmt);

private:
	// The fake face which indicates a "can't merge". Usefull for bind 2/4 or 1/4.
	// The fake face is the only one which has a NULL patch ptr.
	static	CTessFace	CantMergeFace;
};


} // NL3D


#endif // NL_TESSELLATION_H

/* End of tessellation.h */
