/** \file patch.h
 * <File description>
 *
 * $Id: patch.h,v 1.28 2001/01/19 14:25:49 berenguier Exp $
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
#include "nel/3d/tessellation.h"
#include "nel/3d/aabbox.h"
#include "nel/3d/bsphere.h"
#include "nel/3d/tile_element.h"
#include "nel/3d/tile_color.h"


namespace NL3D {

#define NL_MAX_TILES_BY_PATCH_EDGE_SHIFT 4										// max 16x16 tiles by patch (shift version)
#define NL_MAX_TILES_BY_PATCH_EDGE (1<<NL_MAX_TILES_BY_PATCH_EDGE_SHIFT)		// max 16x16 tiles by patch
#define NL_PATCH_FAR0_ROTATED 0x1												// Flags far0 rotated
#define NL_PATCH_FAR1_ROTATED 0x2												// Flags far1 rotated

using NLMISC::CVector;


class	CZone;
class	CBezierPatch;
class	ITexture;


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
		TODO_NOISE: - displacement map (ptr/index).
		TODO_UVCORRECT: - "UV correction" infos.
		
	*/

	class CStreamBit
	{
	public:
		// Init the pointer of the stream
		void setPtr (const std::vector<uint8>& buffer);

		// Push a bool
		void pushBackBool (bool bBoolean);

		// Push 4 bits
		void pushBack4bits (uint8 fourBits);

		// Pop a bool
		bool popBackBool ();

		// Pop 4 bits
		uint8 popBack4bits ();
	private:
		const std::vector<uint8>		*_Vector;
		uint							_Offset;
	};

	// Shadow array. Use CPatch::CStreamBit to read inside.
	std::vector<uint8>	Shadows;

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


	CZone			*getZone() const {return Zone;}
	uint8			getOrderS() const {return OrderS;}
	uint8			getOrderT() const {return OrderT;}
	float			getErrorSize() const {return ErrorSize;}

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


	/// Classify this patch as UnClipped.
	void			forceNoClip() {Clipped= false;}
	/// Classify this patch as Clipped.
	void			forceClip() {Clipped= true;}
	/// Classify this patch.
	void			clip(const std::vector<CPlane>	&pyramid);
	/// Refine / geomorph this patch. Even if clipped.
	void			refine();
	/// preRender this patch, if not clipped. Build RdrFace List ....
	void			preRender();
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


	// only usefull for CZone refine.
	bool			isClipped() const {return Clipped;}

// Private part.
private:
/*********************************/

	friend	class CTessFace;

	CZone			*Zone;
	// Tile Order for the patch.
	uint8			OrderS, OrderT;
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
	uint8			FarRotated;		// If the flag is set, the far texture of the patch is rotated of 1 (to the left of course)
									// Flags: NL_PATCH_FAR0_ROTATED for Far0, NL_PATCH_FAR1_ROTATED for Far1
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
	// The root for render.
	CTessFace		*RdrRoot;
	CTessFace		*RdrTileRoot[NL3D_MAX_TILE_PASS];
	// The N tess faces for this patch.
	sint			NCurrentFaces;


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

	// For rdr.
	void			appendFaceToRenderList(CTessFace *face);
	void			appendFaceToTileRdrList(CTessFace *face);
	void			removeFaceFromRenderList(CTessFace *face);

	/// Texture mgt.
	// @{
	// For CTessFace::computeMaterial(). Return the render pass for this material, given the number of the tile, and the
	// desired pass. NULL may be returned if the pass is not present (eg: no additive for this tile...).
	// The tile lightmap is also required, to get the good lightmapped tile...
	CPatchRdrPass	*getTileRenderPass(sint tileId, sint pass, ITexture *lightmap);
	// For CTessFace::computeMaterial(). Return the orient/scalebias for the tile in the patchtexture, and the
	// desired pass.
	void			getTileUvInfo(sint tileId, sint pass, uint8 &orient, CVector &uvScaleBias, bool &is256x256, uint8 &uvOff);
	// @}

	// Tile LightMap mgt.
	// @{
	// for a given tile (accessed from the (ts,tt) coordinates), compute and get a lightmapId.
	// lightmap returned is to be uses with getTileRenderPass(). The id returned must be stored.
	uint		getTileLightMap(sint ts, sint tt, ITexture *&lightmap);
	// tileLightMapId must be the id returned  by getTileLightMap().
	void		getTileLightMapUvInfo(uint tileLightMapId, CVector &uvScaleBias);
	// tileLightMapId must be the id returned  by getTileLightMap().
	void		releaseTileLightMap(uint tileLightMapId);
	// @}

	// For Render. Those methods compute the vertices for Driver (in CTessFace::CurrentVB).
	sint			getFarIndex0(CTessVertex *vert, CTessFace::CParamCoord  pc);
	sint			getFarIndex1(CTessVertex *vert, CTessFace::CParamCoord  pc);
	void			computeTileVertex(CTessVertex *vert, ITileUv *uv, sint idUv);

	static void		resetFarIndices(CTessFace *rdrRoot);


private:
	// NB: All global render info are stored in CTessFace class static members....

	// The Patch cache (may be a short list/vector later...).
	static	CBezierPatch	CachePatch;
	// For cahcing.
	static	const CPatch	*LastPatch;

	// unpack the patch into the cache.
	CBezierPatch	*unpackIntoCache() const;

};

// inline CPatch::CStreamBit **********************************************************************

// Init the pointer of the stream
/*void CPatch::CStreamBit::setPtr (const std::vector<uint8>& buffer)
{
	_Vector=&buffer;
	_Offset=0;
}

// Push a bool
void CPatch::CStreamBit::pushBackBool (bool bBoolean)
{
	// Size
	if ((_Offset>>5)>=_Vector.size())
		_Vector.resize ((_Offset>>5)+1);

	uint off=_Offset>>5;
	_Vector[off]&=~(1<<(_Offset&0x1f));
	_Vector[off]|=(((uint)bBoolean)<<(_Offset&0x1f));
	_Offset++;
}

// Push 4 bits
void CPatch::CStreamBit::pushBack4bits (uint8 fourBits)
{
	nlassert ((fourBits>=0)&&(fourBits<4));

	if (((_Offset+3)>>5)>=_Vector.size())
		_Vector.resize (((_Offset+3)>>5)+1);

	uint off0=_Offset>>5;
	uint off1=off0+1;
	_Vector[off0]&=~(0xf<<(_Offset&0x1f));
	_Vector[off0]|=((uint)fourBits)<<(_Offset&0x1f));
	_Vector[off1]&=~(0xf>>(32-(_Offset&0x1f)));
	_Vector[off1]|=(((uint)fourBits)>>(32-(_Offset&0x1f)));
	_Offset+=4;
}

// Pop a bool
bool CPatch::CStreamBit::popBackBool ()
{
	// Size
	nlassert ((_Offset>>5)<_Vector.size())

	uint off=_Offset>>5;
	_Offset++;
	return (_Vector[off]&(1<<(_Offset&0x1f)))!=0;
}

// Pop 4 bits
uint8 CPatch::CStreamBit::popBack4bits ()
{
	nlassert (((_Offset+3)>>5)<_Vector.size());

	uint off0=_Offset>>5;
	uint off1=off0+1;
	_Offset+=4;
	return 	((_Vector[off0]&(0xf<<(  _Offset&0x1f   )))>>(_Offset&0x1f))|
			((_Vector[off1]&(0xf>>(32-(_Offset&0x1f))))<<(32-(_Offset&0x1f)));
}*/


} // NL3D


#endif // NL_PATCH_H

/* End of patch.h */
