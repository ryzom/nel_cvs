/** \file tessellation.h
 * <File description>
 *
 * $Id: tessellation.h,v 1.28 2001/02/28 14:21:00 berenguier Exp $
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
#include "nel/misc/uv.h"
#include "nel/misc/bsphere.h"
#include "nel/3d/tess_list.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;
using NLMISC::CUV;


class	CPatch;
class	CPatchRdrPass;
class	CVertexBuffer;
class	CTessFace;


// ***************************************************************************
// 4th pass is always the Lightmapped one (Lightmap*clouds).
#define	NL3D_MAX_TILE_PASS 5
// There is no Face for lightmap, since lightmap pass share the RGB0 face.
#define	NL3D_MAX_TILE_FACE	NL3D_MAX_TILE_PASS-1

#define	NL3D_TILE_PASS_RGB0		0
#define	NL3D_TILE_PASS_RGB1		1
#define	NL3D_TILE_PASS_RGB2		2
#define	NL3D_TILE_PASS_ADD		3
#define	NL3D_TILE_PASS_LIGHTMAP	4
// NB: RENDER ORDER: CLOUD*LIGHTMAP is done BEFORE ADDITIVE.


// ***************************************************************************
const	float	OO32768= 1.0f/0x8000;



// ***************************************************************************
/**
 * The parametric coordinates of the patch. 0x0000<=>0.0f. 0x8000<=> 1.0f.
 */
class	CParamCoord
{
public:
	uint16	S,T;

public:
	CParamCoord() {}
	CParamCoord(uint16 s, uint16 t) {S=s; T=t;}
	// Create at middle.
	CParamCoord(CParamCoord a, CParamCoord b) 
	{
		S= (uint16) (((sint)a.S + (sint)b.S)>>1);
		T= (uint16) (((sint)a.T + (sint)b.T)>>1);
	}
	// Get s,t as floats. returned s,t E [0,1].
	float	getS() const {return S*OO32768;}
	float	getT() const {return T*OO32768;}
	// Set s,t as float. s,t E [0,1].
	void	setST(float s, float t) {S= (sint16)(s*32768);T= (sint16)(t*32768);}
	// vertex on the border?
	bool	onBorder() const {return (S==0 || S==0x8000 || T==0 || T==0x8000);}
};



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
	// The swap, for geomorph "optim"
	bool			GeomDone;

	CTessVertex()
	{
		// Must init to 0.
		GeomDone= false;
	}
};


// ***************************************************************************
struct	CTessFarVertex : public CTessNodeList
{
	sint32			Index0;		// The index of Far0 in the Far VB.
	sint32			Index1;		// The index of Far1 in the Far VB.
	CTessVertex		*Src;		// The src vertex to copy Pos, and compute alpha.
	CParamCoord		PCoord;		// The patch coord, to compute far UV.
};


// ***************************************************************************
struct	CTessNearVertex : public CTessNodeList
{
	sint32			Index;		// The index in the Tile VB.
	CTessVertex		*Src;		// The src vertex to copy Pos.
	CUV				PUv0;
	CUV				PUv1;

public:
	// Init this near vertex UVs, at middle of a and b.
	void		initMiddleUv(CTessNearVertex &a, CTessNearVertex &b)
	{
		PUv0= (a.PUv0+b.PUv0)*0.5;
		PUv1= (a.PUv1+b.PUv1)*0.5;
	}
};


// ***************************************************************************
/** A tileface. There is one TileFace per pass (BUT Lightmap PASS!! same used for RGB0 and LIGHTMAP).
 *
 */
struct	CTileFace : public CTessNodeList
{
	CTessNearVertex		*VBase, *VLeft, *VRight;
};


// ***************************************************************************
struct	CTileMaterial
{
	// The coordinates of the tile in the patch.
	uint8			TileS, TileT;
	// The number of the tile relatively to the patch TileMap.
	uint8			TileId;
	// The rendering passes materials.
	CPatchRdrPass	*Pass[NL3D_MAX_TILE_PASS];
	// Pass are:
	//  0: RGB general Tile.
	//  1: 1st alpha tile. 0: RGB.  1: Alpha.
	//  2: 2nd alpha tile. 0: RGB.  1: Alpha.
	//  3: additive pass.
	//	4: Cloud*Lightmap.
	// NB: BIG TRICK: the pass 0 has 2 Uvs: Uv0: RGB, and Uv1: Lightmap!!!! even, if they are not used at the same pass :o).
	// This simplifies, save memory, and save from copies of v3f.
	// NB: RENDER ORDER: CLOUD*LIGHTMAP is done BEFORE ADDITIVE.

	// The list of faces, for each pass. NB: no faces for lightmaps!! since share same face/vertices from RGB0.
	// The only thing not shared is the renderpass (Pass[0] and Pass[3] are different).
	CTessList<CTileFace>	TileFaceList[NL3D_MAX_TILE_FACE];


	// The global id of the little lightmap part for this tile.
	uint			LightMapId;

	// ptrs are Null by default.
	CTileMaterial();
};


// ***************************************************************************
/**
 * A Landscape Triangle.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTessFace : public CTessNodeList
{
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
	// Mark of recursion (see canMerge() ...).
	bool			RecursMarkCanMerge;
	bool			RecursMarkForceMerge;
	//@}

	/// \name Tile Material Infos (uvs...).
	// @{
	// The multi-pass tile Material.
	CTileMaterial	*TileMaterial;
	// The Tile Faces. There is no Face for lightmap, since use the one from RGB0.
	CTileFace		*TileFaces[NL3D_MAX_TILE_FACE];
	// @}


	/// \name Error metric.
	// @{
	// Size could be computed from TileSize: Size= Patch->BaseSize / (1<<(TileSize-Patch->BaseTileSize)). (or optimized)
	// But used by computeErrorMetric(), and must be as fast as possible.
	sint			ProjectedSizeDate;	// The date of errormetric update.
	float			Size;				// /2 at each split.
	CVector			Center;				// Center of the face.
	float			ProjectedSize;		// The result of errormetric: the projected size of face.
	float			ErrorMetric;		// equal to ProjectedSize, but greater for the transition Far-Near.
	// @}


	// For Render.
	CTessFarVertex	*FVBase, *FVLeft, *FVRight;
	// NB: herit from CTessNodeList to use Prec and Next ptrs.


public:
	CTessFace();
	// do nothing dtor.
	~CTessFace();

	// Utilities.
	bool			isLeaf() const {return SonLeft==NULL;}
	bool			isRectangular() const;
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
	void			updateErrorMetric();
	// can split leaf only.
	void			split(bool propagateSplit=true);
	// can merge "short roots" only (roots which have leafs).
	bool			merge();
	// if NeedCompute, refine the node, and his sons.
	void			refine();


	// Used by CPatch::unbind(). isolate the tesselation from other patchs.
	void			unbind(CPatch *except[4]);
	// Used by CPatch::unbind(). force the merging of face.
	void			forceMerge();
	// Used by CPatch::bind(). Split if necessary, according to neighbors.
	bool			updateBindEdge(CTessFace	*&edgeFace, bool &splitWanted);
	void			updateBind();
	void			updateBindAndSplit();
	// Used by CPatch::forceMergeAtTileLevel(). force the merging of face (as possible).
	void			forceMergeAtTileLevel();


	// Used by CZone::changePatchTexture().
	void			deleteTileUvs();
	void			recreateTileUvs();


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
	// Limits for merge/split propagation. See refine(). Defaults:  1.1, 1.9, 2.1.
	static	float	ChildrenStartComputeLimit;
	static	float	SelfEndComputeLimit;


	// Tile Global Info.
	// What are the limit distances for Tile tesselation transition.
	static	float	TileDistNear, TileDistFar;
	// System, computed from prec.
	static	float	TileDistNearSqr, TileDistFarSqr;
	// System, computed from prec.
	static	float	OOTileDistDeltaSqr;
	// The tiles are not subdivided above this limit (but because of enforced splits). Default: 4 => 50cm.
	static	sint	TileMaxSubdivision;
	// The sphere for TileFar test.
	static	NLMISC::CBSphere	TileFarSphere;
	// The sphere for TileNear test.
	static	NLMISC::CBSphere	TileNearSphere;
	// The size of a 128x128 tile, in pixel. UseFull for HalfPixel Scale/Bias.
	static	float		TilePixelSize;


	// Render Global info. Used by Patch.
	// The distance transition for Far0 and Far1 (200m / 400m).
	static	float	Far0Dist, Far1Dist;
	// Distance for Alpha blend transition
	static	float	FarTransition;
	// The current VertexBuffer for Far.
	static	CVertexBuffer	*CurrentFarVB;
	// The current VertexBuffer for Tile.
	static	CVertexBuffer	*CurrentTileVB;
	// The current Vertex Index for each pass of landscape render. Start at 0.
	static	sint	CurrentFarIndex;
	// The current Vertex Index for each pass of landscape render. Start at 0.
	static	sint	CurrentTileIndex;

	// PATCH GLOBAL INTERFACE.  patch must setup them at the begining at refine()/render().
	// NO!!! REMIND: can't have any patch global, since a propagated split()/updateErrorMetric()
	// can arise. must use Patch pointer.


private:
	// Faces have the same tile???
	static bool		sameTile(const CTessFace *a, const CTessFace *b) 
		{return (a->Patch==b->Patch && a->TileMaterial==b->TileMaterial);}

	/// \name UV mgt.
	// @{
	enum	TTileUvId {IdUvBase=0, IdUvLeft, IdUvRight};
	// Allocate a CTessNearVertex "id" (base, left or right) for each not NULL TileFace of "this" face.
	// Init with good CTessVertex::Src, and then, insert it into Patch RenderList.
	void	allocTileUv(TTileUvId id);
	// delete a CTessNearVertex "id", removing it from Patch RenderList, for each not NULL TileFace of "this" face.
	void	deleteTileUv(TTileUvId id);
	// Just ptr-copy a CTessNearVertex "id" from an other face/vertex id. Do this for each not NULL TileFace.
	void	copyTileUv(TTileUvId id, CTessFace *srcFace, TTileUvId srcId);

	// The Base NearVertex must be allocated before.
	// for each not NULL TileFace of "this" face, make the Base NearVertex the middle of baseFace TileUvLeft, and
	// baseFace TileUvRight.
	void	heritTileUv(CTessFace *baseFace);


	// create The TileFaces, according to the TileMaterial passes. NO INSERTION IN RENDERLIST!!
	void	buildTileFaces();
	// delete The TileFaces, according to the TileMaterial passes. NO REMOVAL FROM RENDERLIST!!
	void	deleteTileFaces();
	// return true iff TileFaces are all NULL.
	bool	emptyTileFaces();

	// @}


	// Usefull for unbind.
	static bool	exceptPatch(CPatch *p, CPatch *except[4]);


	// see updateErrorMetric.
	void	computeTileErrorMetric();

	// See refine() and merge().
	bool	canMerge(bool testEm);

	// see split().
	void	splitRectangular(bool propagateSplit);
	void	doMerge();
	void	heritTileMaterial();

	// see computeTileMaterial().
	void	initTileUvRGBA(sint pass, bool alpha, CParamCoord pointCoord, CParamCoord middleCoord, CUV &uv);
	// The same, but for the lightmap pass.
	void	initTileUvLightmap(CParamCoord pointCoord, CParamCoord middleCoord, CUV &uv);

	// Far Vertices: update info (ptr, and PCoord).
	void			updateNearFarVertices();

private:
	// Fake face are the only ones which have a NULL patch ptr (with mult face).
	// The fake face which indicates a "can't merge". Usefull for bind 2/4 or 1/4.
	static	CTessFace	CantMergeFace;

public:
	// The fake face which indicates a multiple patch face. Used in updateBind(), for multiple bind.
	static	CTessFace	MultipleBindFace;
};


} // NL3D


#endif // NL_TESSELLATION_H

/* End of tessellation.h */
