/** \file patch.h
 * <File description>
 *
 * $Id: patch.h,v 1.4 2001/07/05 11:37:48 berenguier Exp $
 * \todo yoyo:
		- "UV correction" infos.
		- NOISE, or displacement map (ptr/index).
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
#include "3d/tessellation.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/bsphere.h"
#include "nel/misc/triangle.h"
#include "nel/misc/geom_ext.h"
#include "3d/tile_element.h"
#include "3d/tile_color.h"
#include "3d/tile_lumel.h"
#include "3d/tess_block.h"


namespace NL3D {

#define NL_MAX_TILES_BY_PATCH_EDGE_SHIFT 4										// max 16x16 tiles by patch (shift version)
#define NL_MAX_TILES_BY_PATCH_EDGE (1<<NL_MAX_TILES_BY_PATCH_EDGE_SHIFT)		// max 16x16 tiles by patch
#define NL_PATCH_FAR0_ROTATED 0x1												// Flags far0 rotated
#define NL_PATCH_FAR1_ROTATED 0x2												// Flags far1 rotated
#define NL_PATCH_SMOOTH_FLAG_SHIFT 0x3											// Smooth flags shift
#define NL_PATCH_SMOOTH_FLAG_MASK 0x3c											// Smooth flags mask

#define NL_LUMEL_BY_TILE_SHIFT 2												// 4 lumels by tile
#define NL_LUMEL_BY_TILE (1<<NL_LUMEL_BY_TILE_SHIFT)							// 4 lumels by tile


#define NL_PATCH_BLOCK_MAX_QUAD 4												// Max quad per CPatchQuadBlock.
#define NL_PATCH_BLOCK_MAX_VERTEX (NL_PATCH_BLOCK_MAX_QUAD+1)					// Max vertex per CPatchQuadBlock.


using NLMISC::CVector;


class	CLandscape;
class	CZone;
class	CBezierPatch;
class	ITexture;


// ***************************************************************************
#define	NL3D_NOISE_MAX	1


// ***************************************************************************
class	CVector3s
{
public:
	sint16	x,y,z;

public:
	void	pack(const CVector &v, const CVector &bias, float scale)
	{
		float	xr,yr,zr;
		xr= (v.x - bias.x)/scale;
		yr= (v.y - bias.y)/scale;
		zr= (v.z - bias.z)/scale;
		NLMISC::clamp(xr, -32768, 32767);
		NLMISC::clamp(yr, -32768, 32767);
		NLMISC::clamp(zr, -32768, 32767);
		x= (sint16)xr;
		y= (sint16)yr;
		z= (sint16)zr;
	}
	void	unpack(CVector &v, const CVector &bias, float scale) const
	{
		v.x= x*scale + bias.x;
		v.y= y*scale + bias.y;
		v.z= z*scale + bias.z;
	}
	void	serial(NLMISC::IStream &f)
	{
		f.serial(x,y,z);
	}
};


// ***************************************************************************
/**
 * A landscape patch identifier (zone/patch).
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CPatchIdent
{
	sint32		ZoneId;		// From which zone this patch come from...
	uint16		PatchId;	// Id of this patch.

public:
	bool			operator<(const CPatchIdent &p) const
	{
		if(ZoneId!=p.ZoneId) return ZoneId<p.ZoneId;
		return PatchId<p.PatchId;
	}

	bool			operator==(const CPatchIdent &p) const
	{
		return ZoneId==p.ZoneId && PatchId==p.PatchId;
	}
	bool			operator!=(const CPatchIdent &p) const
	{
		return !(*this==p);
	}

};


// ***************************************************************************
/**
 * A triangle from a patch identifier.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CTrianglePatch : public NLMISC::CTriangleUV
{
	/// from which patch this triangle comes from.
	CPatchIdent		PatchId;
};


// ***************************************************************************
/**
 * An descriptor of a group of tiles in a patch.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CPatchBlockIdent
{
public:
	/// the patch.
	CPatchIdent		PatchId;
	/// size of the patch, in tile coordinates.
	uint8			OrderS,OrderT;
	/// coordinate of the SubPart of the patch, in tile coordinates.
	uint8			S0,S1,T0,T1;

public:
	/// \name Operators.
	// @{
	bool			operator==(const CPatchBlockIdent &pb) const
	{
		return PatchId==pb.PatchId &&
			S0==pb.S0 && S1==pb.S1 &&
			T0==pb.T0 && T1==pb.T1;
	}
	bool			operator!=(const CPatchBlockIdent &pb) const
	{
		return !(*this==pb);
	}

	bool			operator<(const CPatchBlockIdent &pb) const
	{
		if(PatchId!=pb.PatchId)
			return PatchId<pb.PatchId;
		if(S0!=pb.S0)	return S0<pb.S0;
		if(S1!=pb.S1)	return S1<pb.S1;
		if(T0!=pb.T0)	return T0<pb.T0;
		return T1<pb.T1;
	}
	bool			operator<=(const CPatchBlockIdent &pb) const
	{
		return (*this<pb) || (*this==pb);
	}
	bool			operator>(const CPatchBlockIdent &pb) const
	{
		return !(*this<=pb);
	}
	bool			operator>=(const CPatchBlockIdent &pb) const
	{
		return !(*this<pb);
	}

	// @}
};


// ***************************************************************************
/**
 * A group of tiles faces in a patch. For speed/mem, max size of a CPatchQuadBlock is fixed.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CPatchQuadBlock
{
public:
	/// from what subPart of the patch those data comes from.
	CPatchBlockIdent	PatchBlockId;

	/// evaluated Vertices of this patch.
	CVector				Vertices[NL_PATCH_BLOCK_MAX_VERTEX*NL_PATCH_BLOCK_MAX_VERTEX];

public:

	/** build the 2 triangles from a quad of the CPatchQuadBlock. 
	 * quadId is the number of the quad, relatively to the PatchQuadBlock.
	 */
	void		buildTileTriangles(uint8 quadId, CTrianglePatch  triangles[2]) const;
};



// ***************************************************************************
/**
 * A landscape patch.
 * QuadPatch layout (same notations as 3ds Max SDK).
 * 
 *   A---> ad ---- da <---D
 *   |                    |
 *   |                    |
 *   v                    v
 *   ab    ia      id     dc
 *
 *   |                    |
 *   |                    |
 *
 *   ba    ib      ic     cd
 *   ^                    ^
 *   |                    |
 *   |                    |
 *   B---> bc ---- cb <---C
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

		CPatch			*Next[4];	// The neighbor patch i.
		sint			Edge[4];	// On which edge of Nexti we are binded.
	};

public:
	/// The patch coordinates (see CBezierPatch).
	CVector3s		Vertices[4];
	CVector3s		Tangents[8];
	CVector3s		Interiors[4];
	/*
		\todo yoyo: TODO_NOISE: - displacement map (ptr/index).
		\todo yoyo: TODO_UVCORRECT: - "UV correction" infos.
		
	*/

	// Lumel array compressed.
	std::vector<uint8>	CompressedLumels;

	// Lumel array uncompressed.
	std::vector<CTileLumel>	UncompressedLumels;

	// There is OrderS*OrderT tiles. CZone build it at build() time.
	std::vector<CTileElement>	Tiles;

	// There is OrderS*OrderT tiles color. CZone build it at build() time.
	std::vector<CTileColor>		TileColors;

public:

	/// Constructor
	CPatch();
	/// dtor
	~CPatch();

	/** compile a valid patch. (init)
	 * Call this method before any other. Zone and Control points must be valid before calling compile(). \n
	 * This is an \b ERROR to call compile() two times. \n
	 * \param z zone where the patch must be binded.
	 * \param orderS the Tile order in S direction: 2,4,8,16.
	 * \param orderT the Tile order in T direction: 2,4,8,16.
	 * \param errorSize if 0, setup() compute himself the errormetric of the patch. May be setup to surface of patch, 
	 *  modulated by tangents and displacement map.
	 */
	void			compile(CZone *z, uint8 orderS, uint8 orderT, CTessVertex *baseVertices[4], float errorSize=0);
	/// Un-compile a patch. Tesselation is deleted. if patch is not compiled, no - op.
	void			release();


	///  Get the landscape in which is placed this patch.
	CLandscape		*getLandscape () const;
	CZone			*getZone() const {return Zone;}
	uint8			getOrderS() const {return OrderS;}
	uint8			getOrderT() const {return OrderT;}
	float			getErrorSize() const {return ErrorSize;}
	sint			getFar0() const {return Far0;}
	sint			getFar1() const {return Far1;}

	/// Build the bbox of the patch, according to ctrl points, and displacement map max value.
	CAABBox			buildBBox() const;

	/// Return the bounding sphere. Work only when zone compiled.
	const CBSphere	&getBSphere() const {return BSphere;}

	/** Compute a vertex.
	 * Compute a vertex according to:
	 *	- s,t
	 *	- patch control points uncompressed with zone Bias/Scale.
	 *  - Patch UV geometric correction.
	 *	- Patch noise (and noise of Patch neighbor).
	 */
	CVector			computeVertex(float s, float t) const;


	/** unbind the patch from All neighbors. neighbors patchs links are modified too. The tesselation is forcemerged.
	 * unbind from patchs except those in except (usefull for bind 2/1 or 4/1).
	 */
	void			unbind(CPatch *except[4]);

	/** bind the patch to 4 neighbors, given in this patch edge order (0,1,2,3). Tesselation is reseted (patch unbound first).
	 * NB: this patch and his neighborood must be compiled...
	 * NB: neighbor patchs must not be NULL (but according to NPatchs).
	 */
	void			bind(CBindInfo	Edges[4]);

	/// For changing TileMaxSubdivision. force tesselation to be under tile.
	void			forceMergeAtTileLevel();


	/// Classify this patch as UnClipped.
	void			forceNoClip() {Clipped= false;}
	/// Classify this patch as Clipped.
	void			forceClip() {Clipped= true;}
	/// Classify this patch.
	void			clip(const std::vector<CPlane>	&pyramid);
	/// Refine / geomorph this patch. Even if clipped.
	void			refine();
	/// preRender this patch, if not clipped. Build RdrFace List ....
	void			preRender(const std::vector<CPlane>	&pyramid);
	/// Render this patch, if not clipped (append to VertexBuffers / materials primitive block).
	void			renderFar0();
	void			renderFar1();
	void			renderTile(sint pass);
	// release Far render pass/reset Tile/Far render.
	void			resetRenderFar();



	// For CZone changePatchTexture only.
	void			deleteTileUvs();
	void			recreateTileUvs();


	// Serial just the un-compiled part.
	void			serial(NLMISC::IStream &f);

	// unpack the patch into a floating point one.
	void			unpack(CBezierPatch	&p) const;

	/// \name Lumels methods

	/**
	  *  Expand the shading part of the patch in bilinear. Array must have a size of ((OrderS*4/ratio)+1)*((OrderT*4/ratio)+1)
	  *
	  *  \param Shading is a pointer on the destination shading buffer. Size must be ((OrderS*4/ratio)+1)*((OrderT*4/ratio)+1).
	  *  \param ratio is the one over the ratio of the texture destination. Must be 1 or 2.
	  *  \see unpackShadowMap(), packShadowMap(), resetCompressedLumels(), clearUncompressedLumels()
	  */
	void			expandShading (uint8 * Shading, uint ratio=1);

	/**
	  *  Unpack the lumels of the patches. Lumels are classes that describe the lighting environnement at a given texel
	  *  of the lightmap. It is used to compute the shadow map of the patch, compress it and uncompress it.
	  *  This method uncompress the lumels stored in its Lumels member.
	  *
	  *  \param pShadow is a pointer on the destination lumel buffer. Size must be ((OrderS*4/ratio)+1)*((OrderT*4/ratio)+1).
	  *  \param ratio is the one over the ratio of the texture destination. Must be 1 or 2.
	  *  \see expandShading(), packShadowMap(), resetCompressedLumels(), clearUncompressedLumels()
	  */
	void			unpackShadowMap (class CTileLumel *pShadow, uint ratio=1);

	/**
	  *  Pack the lumels of the patches. Lumels are classes that describe the lighting environnement at a given texel
	  *  of the lightmap. It is used to compute the shadow map of the patch, compress it and uncompress it.
	  *  This method compress the lumels passed in parameter and stored them in its Lumels member.
	  *
	  *  \param pShadow is a pointer on the destination lumel buffer. Size must be (OrderS*4+1)*(OrderS*4+1).
	  *  \see expandShading(), unpackShadowMap(), resetCompressedLumels(), clearUncompressedLumels()
	  */
	void			packShadowMap (const class CTileLumel *pLumel);

	/**
	  *  Rebuild the packed lumels without shadow. Only the interpolated color will be used.
	  *
	  *  \see expandShading(), packShadowMap(), unpackShadowMap(), clearUncompressedLumels()
	  */
	void			resetCompressedLumels ();

	/**
	  *  Clear the uncompressed lumel of the patch.
	  *
	  *  \see expandShading(), packShadowMap(), unpackShadowMap()
	  */
	void			clearUncompressedLumels ();

	/// \name Smooth flags methods

	/**
	  * Set the smooth flag for the n-th edge. flag is false if this edge must by smoothed, true else.
	  */
	void setSmoothFlag (uint edge, bool flag)
	{
		// Erase it
		Flags&=~(1<<(edge+NL_PATCH_SMOOTH_FLAG_SHIFT));
		
		// Set it
		Flags|=(((uint)flag)<<(edge+NL_PATCH_SMOOTH_FLAG_SHIFT));
	}

	/**
	  * Get the smooth flag for the n-th edge. Return false if this edge must by smoothed, true else.
	  */
	bool getSmoothFlag (uint edge)
	{
		// Test it
		return ((Flags&(1<<(edge+NL_PATCH_SMOOTH_FLAG_SHIFT)))!=0);
	}


	/// \name Subdivision / ForCollision.
	// @{
	/** Add triangles to triangles array which intersect the bbox.
	 * NB: this method use a convex hull subdivion to search in O(logn) what part of the patch to insert.
	 * \param patchId the id of this patch, used to fill triangles.
	 * \param bbox the bbox to test against.
	 * \param triangles array to be filled (no clear performed, elements added).
	 * \param tileTessLevel 0,1 or 2  size of the triangles (2*2m, 1*1m or 0.5*0.5m). Level of subdivision of a tile.
	 */
	void		addTrianglesInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tileTessLevel) const;

	/** Fill a CPatchQuadBlock, from its required PatchId.
	 * nlassert(PatchId size is less than NL_PATCH_BLOCK_MAX_QUAD)
	 */
	void		fillPatchQuadBlock(CPatchQuadBlock &quadBlock) const;

	/** Add CPatchBlockIdent to CPatchBlockIdent array which intersect the bbox.
	 * NB: this method use a convex hull subdivion to search in O(logn) what part of the patch to insert.
	 * \param patchId the id of this patch, used to fill PatchBlockIdent.
	 * \param bbox the bbox to test against.
	 * \param paBlockIds array to be filled (no clear performed, elements added).
	 */
	void		addPatchBlocksInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds) const;


	/** From the current tesselation of this patch, and a UV in this patch, return tesselated position.
	 */
	CVector		getTesselatedPos(CUV uv) const;


	/** From the current tesselation of this patch, append to the list of leaves faces.
	 */
	void		appendTessellationLeaves(std::vector<const CTessFace*>  &leaves) const;


	// @}


public:

	// only usefull for CZone refine.
	bool			isClipped() const {return Clipped;}

// Private part.
private:
/*********************************/

	friend	class CTessFace;
	friend	class CZone;

	CZone			*Zone;
	// Tile Order for the patch.
	uint8			OrderS, OrderT;
	// For this patch, which level is required for a face to be inserted in the secondary TessBlocks (and not the masterblock)??
	sint			TessBlockLimitLevel;
	// For this patch, which level is required for a face to be a valid Tile??
	sint			TileLimitLevel;
	// For this patch, which level is required for a face to be a "square" face (not rectangular)??
	sint			SquareLimitLevel;
	// The Base Size*bumpiness of the patch (/2 at each subdivide).
	float			ErrorSize;
	// The root for tesselation.
	CTessFace		*Son0, *Son1;
	// The base vertices.
	CTessVertex		*BaseVertices[4];
	// The base Far vertices (always here!!).
	CTessFarVertex	BaseFarVertices[4];
	// BSphere.
	CBSphere		BSphere;


	// Local info for CTessFace tiles. CPatch must setup them at the begining at refine()/render().
	// For Far Texture coordinates.
	sint			Far0;			// The level of First Far: 0,1,2 or 3. 0 means Tile.
	sint			Far1;			// The level of second Far, for transition: 1,2 or 3. 0 means none.
	float			Far0UScale, Far0VScale, Far0UBias, Far0VBias;
	float			Far1UScale, Far1VScale, Far1UBias, Far1VBias;

	// Pack 4 bytes
	// {
	/**
	  * Flags NL_PATCH_FAR0_ROTATED and NL_PATCH_FAR1_ROTATED
	  *		NL_PATCH_FAR0_ROTATED for Far0, NL_PATCH_FAR1_ROTATED for Far1
	  *		If the flag is set, the far texture of the patch is rotated of 1 
	  *		(to the left of course)
	  *
	  * Flags NL_PATCH_SMOOTH_FLAG_MASK
	  *		4 flag for smooth edge. Same as CPatchInfo::CBindInfo shifted by (<<NL_PATCH_SMOOTH_FLAG_SHIFT).
	  *		See CPatchInfo::CBindInfo::Flags for details. 
	  */
	uint8			Flags;
								
	// are we cliped?
	bool			Clipped;

	// Do we must compute the Tile errormetric part??
	bool			ComputeTileErrorMetric;
	
	// Are we in the Tile/Far transition. if ComputeTileErrorMetric==true, and TileFarTransition==false, we are 
	// TOTALY IN the Tile zone sphere.
	bool			TileFarTransition;
	// }
	
	// The render Pass of Far0 and Far1.
	CPatchRdrPass	*Pass0, *Pass1;
	// Info for alpha transition with Far1.
	float			TransitionSqrMin;
	float			OOTransitionSqrDelta;

	/// \name Block renders.
	// @{
	// The block render of far only. Only Far faces bigger than a block are inserted here.
	CTessBlock					MasterBlock;
	// The 2*2 block render. For memory optimisation, none is allocated when no faces need it.
	// There is (OrderT/2)*(OrderS/2) TessBlocks.
	std::vector<CTessBlock>		TessBlocks;
	// The counter of faces which need TessBlocks (FarFaces, TileMaterial and FarVertices). When 0, the vector is contReset()-ed.
	sint						TessBlockRefCount;
	// @}


	/**
	  *  Static buffer used to expand shading in expandShading. Its size is 
	  *  (NL_MAX_TILES_BY_PATCH_EDGE*NL_LUMEL_BY_TILE+1)*(NL_MAX_TILES_BY_PATCH_EDGE*NL_LUMEL_BY_TILE+1)
	  */
	static uint8	_ShadingBuffer[];

	/**
	  *  Stream version of the class. 
	  */
	static uint32	_Version;

private:
	// Guess.
	void			computeDefaultErrorSize();
	// based on BaseVertices, recompute positions, and Make Face roots Son0 and Son1.
	void			makeRoots();
	// Guess. For bind() reasons.
	CTessFace		*getRootFaceForEdge(sint edge) const;
	// Guess. For bind() reasons. return the vertex 0 of edge.
	CTessVertex		*getRootVertexForEdge(sint edge) const;
	void			changeEdgeNeighbor(sint edge, CTessFace *to);


	/// \name RenderList mgt.
	// @{

	// reset all list of MasterBlock.
	void			resetMasterBlock();
	// simply clear the tessnbloc array and reset cout to 0.
	void			clearTessBlocks();
	// add a ref to tess blocks, allocate them if necessary.
	void			addRefTessBlocks();
	// dec a ref to tess blocks, destroy them if necessary.
	void			decRefTessBlocks();
	// UGLY SIDE EFFECT: when refcount TessBlocks RefCount reach 0, tessblockas are deleted. think of it in tesselation.cpp.


	// Retrieve the tessblockId, depending on face info.
	uint			getNumTessBlock(CTessFace *face);
	// FarVertType.
	enum			TFarVertType {FVMasterBlock=0, FVTessBlock, FVTessBlockEdge};
	// Retrieve the tessblockId, depending on a ParamCoord.
	void			getNumTessBlock(CParamCoord pc, TFarVertType &type, uint &numtb);


	// For rdr. Insert in the GOOD TessBlock the face (depending on level, patchcoordinates etc...)
	// call appendFaceToTileRenderList() to insert his TileFaces into the good renderList.
	void			appendFaceToRenderList(CTessFace *face);
	// Remove a face and his tileface from the patch renderlist.
	// call removeFaceFromTileRenderList() to insert his TileFaces into the good renderList.
	void			removeFaceFromRenderList(CTessFace *face);
	// for changePatchTexture, insert just the TileFace into the good render List.
	void			appendFaceToTileRenderList(CTessFace *face);
	void			removeFaceFromTileRenderList(CTessFace *face);

	// Set/Unset (to NULL) a TileMaterial from the TessBlocks. Material must exist for both functions.
	// And TileS/TileT must be OK.
	void			appendTileMaterialToRenderList(CTileMaterial *tm);
	void			removeTileMaterialFromRenderList(CTileMaterial *tm);

	// Add/Remove FarVertices. Use fv->PCoord to know where.
	void			appendFarVertexToRenderList(CTessFarVertex *fv);
	void			removeFarVertexFromRenderList(CTessFarVertex *fv);
	// Add/Remove NearVertices. Use tileMat to know where.
	void			appendNearVertexToRenderList(CTileMaterial *tileMat, CTessNearVertex *nv);
	void			removeNearVertexFromRenderList(CTileMaterial *tileMat, CTessNearVertex *nv);

	// @}



	/// \name Texture mgt.
	// @{
	// For CTessFace::computeMaterial(). Return the render pass for this material, given the number of the tile, and the
	// desired pass. NULL may be returned if the pass is not present (eg: no additive for this tile...).
	CPatchRdrPass	*getTileRenderPass(sint tileId, sint pass);
	// For CTessFace::computeMaterial(). Return the orient/scalebias for the tile in the patchtexture, and the
	// desired pass (and the desired stage: RGB/Alpha).
	void			getTileUvInfo(sint tileId, sint pass, bool alpha, uint8 &orient, CVector &uvScaleBias, bool &is256x256, uint8 &uvOff);
	// @}

	// Tile LightMap mgt.
	// @{
	// for a given tile (accessed from the (ts,tt) coordinates), compute and get a lightmapId, and a RenderPass.
	// The id returned must be stored, for getTileLightMapUvInfo() and releaseTileLightMap().
	uint		getTileLightMap(sint ts, sint tt, CPatchRdrPass *&rdrpass);
	// tileLightMapId must be the id returned  by getTileLightMap().
	void		getTileLightMapUvInfo(uint tileLightMapId, CVector &uvScaleBias);
	// tileLightMapId must be the id returned  by getTileLightMap().
	void		releaseTileLightMap(uint tileLightMapId);
	// @}

	// For Render. Those methods compute the vertices for Driver (in CTessFace::Current*VB).
	void		fillFar0VB(CTessList<CTessFarVertex>  &vertList);
	void		fillFar1VB(CTessList<CTessFarVertex>  &vertList);
	void		fillTileVB(CTessList<CTessNearVertex> &vertList);
	// For Render. Those methods compute the primitives for Driver (in pass).
	void		addFar0TriList(CPatchRdrPass *pass, CTessList<CTessFace> &flist);
	void		addFar1TriList(CPatchRdrPass *pass, CTessList<CTessFace> &flist);
	void		addTileTriList(CPatchRdrPass *pass, CTessList<CTileFace> &flist);


	/// \name Subdivision private.
	// @{
	/// build a bbox from the convex hull of a bezier patch, enlarged with noise.
	void		buildBBoxFromBezierPatch(const CBezierPatch &p, CAABBox &ret) const;
	/** recurse subdivide of the bezierPatch.
	 * 3 1st parameters are the parameter of addTrianglesInBBox(). \n
	 * pa is the bezier patch for this subdivision of this patch. \n
	 * s0, s1, t0, t1 represent the part of the bezier patch subdivided. At start, s0=0, s1=OrderS, t0=0, t1=OrderT.
	 */
	void		addTrianglesInBBoxRecurs(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tessLevel, 
		const CBezierPatch &pa, uint8 s0, uint8 s1, uint8 t0, uint8 t1) const;
	/** called by addTrianglesInBBoxRecurs(). effective fill the array of triangles from 1 tile at tile coordinates s0,t0.
	 * depending of tessLevel (0,1,2), 2, 8 or 32 triangles are added.  (2*2m, 1*1*m or 0.5*0.5m).
	 * NB: only triangles of quad included in the bbox are added.
	 */
	void		addTileTrianglesInBBox(CPatchIdent paId, const CAABBox &bbox, std::vector<CTrianglePatch> &triangles, uint8 tessLevel, 
		uint8 s0, uint8 t0) const;

	/** recurse method of addPatchBlocksInBBox.
	 */
	void		addPatchBlocksInBBoxRecurs(CPatchIdent paId, const CAABBox &bbox, std::vector<CPatchBlockIdent> &paBlockIds, 
		const CBezierPatch &pa, uint8 s0, uint8 s1, uint8 t0, uint8 t1) const;
	// @}


private:
	// NB: All global render info are stored in CTessFace class static members....

	// The Patch cache (may be a short list/vector later...).
	static	CBezierPatch	CachePatch;
	// For cahcing.
	static	const CPatch	*LastPatch;

	// unpack the patch into the cache.
	CBezierPatch	*unpackIntoCache() const;

};


} // NL3D


#endif // NL_PATCH_H

/* End of patch.h */
