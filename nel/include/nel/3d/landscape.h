/** \file landscape.h
 * <File description>
 *
 * $Id: landscape.h,v 1.40 2001/05/21 17:03:54 cado Exp $
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

#ifndef NL_LANDSCAPE_H
#define NL_LANDSCAPE_H


#include "nel/misc/types_nl.h"
#include "nel/misc/class_id.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/triangle.h"
#include "nel/3d/zone.h"
#include "nel/3d/tile_bank.h"
#include "nel/3d/patch_rdr_pass.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/primitive_block.h"
#include "nel/3d/material.h"
#include "nel/3d/tile_far_bank.h"
#include "nel/3d/texture_near.h"
#include <map>

#define NL_MAX_SIZE_OF_TEXTURE_EDGE_SHIFT (NL_MAX_TILES_BY_PATCH_EDGE_SHIFT+NL_NUM_PIXELS_ON_FAR_TILE_EDGE_SHIFT)
#define NL_MAX_SIZE_OF_TEXTURE_EDGE (1<<NL_MAX_SIZE_OF_TEXTURE_EDGE_SHIFT)		// Size max of a far texture edge in pixel


namespace NL3D 
{


class	CHeightMap;


// ***************************************************************************
// The maximum amount of different tiles in world.
const	sint	NbTilesMax= 65536;
// Size of a CTextureNear. 256 by default (works everywhere).
// Texures must be square, because of uvscalebias...
const	sint	TextureNearSize= 256;
const	sint	NbTilesByLine= TextureNearSize/NL_TILE_LIGHTMAP_SIZE;
const	sint	NbTilesByTexture= NbTilesByLine*NbTilesByLine;



// ***************************************************************************
/**
 * A landscape bind exception.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
struct EBadBind : public Exception
{
private:
	mutable	std::string		_Output;

public:
	struct	CBindError
	{
		CBindError(sint z, sint p) {ZoneId= z; PatchId= p;}
		sint	ZoneId;
		sint	PatchId;
	};


	// The result list of bind errors.
	std::list<CBindError>	BindErrors;

public:
	EBadBind() : Exception( "Landscape Bind Error in (3DSMax indices!! (+1) ): " ) {}

};



// ***************************************************************************
/**
 * A landscape. Use CZone to build zone, and use landscape to dynamically add/remove them, for render.
 *
 *	Limits:
 *		- 65535	zones max in whole world (16 bits ZoneId ).
 *		- 65535 patchs max by zone.
 *		- patch order 2x2 minimum.
 *		- patch order 16x16 maximum.
 *		- connectivity on a edge: 1/1, 1/2, or 1/4.
 *		- connectivity on a edge of a zone: 1/1 only.
 *		- The value of Noise amplitude is global and cannot go over 1 meter (+- 1m).
 *			Sorry, this is a FIXED (for ever) value which should NEVER change (because of Gfx database).
 *
 *	If you use the tiles mapped on the patches, load the near bank file (.bank) and the far bank file (.farbank)
 *  by seralizing TileBank and TileFarBank with those files. Then call initTileBanks.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CLandscape : public NLMISC::CRefCount
{
public:
	// The bank of tiles information.
	CTileBank		TileBank;
	CTileFarBank	TileFarBank;

public:

	/// Constructor
	CLandscape();
	/// Destructor. clear().
	~CLandscape();


	/// \name Init/Build.
	// @{
	/// init the landscape VBuffers, texture cache etc...
	void			init();

	/** Add a zone which should be builded (or loaded), but not compiled. CLandscape compile it.
	 * The contents of newZone are copied into the landscape.
	 * \param newZone the new zone.
	 * \return true if OK, false otherwise. As example, Fail if newZone is already connected.
	 */
	bool			addZone(const CZone	&newZone);
	
	/** remove a zone by its unique Id.
	 * The zone is release()-ed (disconnected), then deleted.
	 * \param zoneId the zone to be removed.
	 * \return true if OK, false otherwise. As example, Fail if zone is not connected.
	 */
	bool			removeZone(uint16 zoneId);
	
	/// Disconnect, and Delete all zones.
	void			clear();

	/// Verify the binding of patchs of all zones. throw EBadBind if error.
	void			checkBinds() throw(EBadBind);
	/// Verify the binding of patchs of one zone. throw EBadBind if error. nop if zone not loaded.
	void			checkBinds(uint16 zoneId) throw(EBadBind);

	/**
	  *  Build tileBank. Call this after loading the near and far tile banks.
	  *  
	  *  \return true if ok, false else. If false, far texture will be desactived.
	  */
	bool			initTileBanks ();
	// @}

	
	/// \name Landscape Parameters.
	// @{
	/// Set tile near distance. Default 50.f. maximized to length of Far alpha transition).
	void			setTileNear (float tileNear);
	/// Get tile near distance.
	float			getTileNear () const	{return _TileDistNear;}
	/// Set threshold for subdivsion quality. The lower is threshold, the more the landscape is subdivided. Default: 0.001.
	void			setThreshold (float thre)	{_Threshold= thre;}
	/// Get threshold.
	float			getThreshold () const	{return _Threshold;}
	void			setRefineMode(bool enabled) {_RefineMode= enabled;}
	bool			getRefineMode() const {return _RefineMode;}
	/// Set Maximum Tile subdivision. Valid values must be in [0..4]  (assert). Default is 0 (for now :) ).
	void			setTileMaxSubdivision (uint tileDiv);
	/// Get Maximum Tile subdivision.
	uint 			getTileMaxSubdivision ();

	// TODO: other landscape param setup (Transition etc...).
	// Store it by landscape, and not only globally in CTessFace statics.
	// @}


	/// \name Render methods.
	// @{
	/** Clip the landscape according to frustum. 
	 * Planes must be normalized.
	 */
	void			clip(const CVector &refineCenter, const std::vector<CPlane>	&pyramid);
	/// Refine/Geomorph the tesselation of the landscape.
	void			refine(const CVector &refineCenter);
	/// Render the landscape. A more precise clip is made on TessBlocks. pyramid should be the same as one passed to clip().
	void			render(IDriver *drv, const CVector &refineCenter, const std::vector<CPlane>	&pyramid, bool doTileAddPass=false);
	/// Refine/Geomorph ALL the tesselation of the landscape, from the view point refineCenter. Even if !RefineMode.
	void			refineAll(const CVector &refineCenter);
	// @}


	/// \name Collision methods.
	// @{
	/** Build the set of faces of landscape, which are IN a bbox. Usefull for collisions.
	 * The faces are builded at Tile level (2m*2m).
	 * \param bbox the bbox where faces are searched.
	 * \param faces the result of the build.
	 * \param faceSplit if true, Only faces which are IN or partialy IN the bbox are returned. Else the clipping is done
	 * on patch level. Worst, but faster.
	 */
	void			buildCollideFaces(const CAABBoxExt &bbox, std::vector<CTriangle> &faces, bool faceSplit);
	/** Build the set of faces of landscape, from a certain patch. Usefull for collisions. Triangles are built first in S 
	 *  then T order. There is two triangles by tiles. So the number of triangles for a patch is 2*OrderS*OrderT.
	 */
	void			buildCollideFaces(sint zoneId, sint patch, std::vector<CTriangle> &faces);
	// @}



	/// \name Accessors.
	// @{
	/** Get a zone pointer.
	 * 
	 * \param zoneId the zone of the update.
	 * \return Return a zone pointer. NULL if the zone doesn't exist or isn't loaded.
	 */
	CZone*			getZone (sint zoneId);
	/** Get a zone pointer.
	 * 
	 * \param zoneId the zone of the update.
	 * \return Return a zone pointer. NULL if the zone doesn't exist or isn't loaded.
	 */
	const CZone*	getZone (sint zoneId) const;
	// @}


	/// \name Tile mgt.
	// @{
	/// Force a range of tiles to be loaded in the driver...
	void			flushTiles(IDriver *drv, uint16 tileStart, uint16 nbTiles);
	/// Force a range of tiles to be loaded in the driver...
	void			releaseTiles(uint16 tileStart, uint16 nbTiles);

	/// Return the texture for a tile Id. UseFull for Tile edition.
	NLMISC::CSmartPtr<ITexture>		getTileTexture(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias);
	// @}

	/// \name Lighting.
	// @{
	/**
	  *  Setup the light color use for static illumination.
	  *
	  *  \param diffuse is the color of the diffuse componante of the lighting.
	  *  \param ambiant is the color of the ambiante componante of the lighting.
	  *  \param multiply is the multiply factor. Final color is (diffuse*multiply*shading+ambiant*(1.0-shading))
	  */
	void setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply);
	/**
	  *  Get the light color by shading table.
	  *
	  *  \return a CRGBA[256] array. It give the static light color for a shading value.
	  */
	const CRGBA* getStaticLight () const
	{
		return _LightValue;
	}
	// @}


	/// \name HeightField DeltaZ.
	// @{
	/// return the HeightField DeltaZ for the 2D position. (0,0,dZ) is returned.
	CVector		getHeightFieldDeltaZ(float x, float y) const;
	/** set the HeightField data. NB: take lot of place in memory.
	 * only one is possible. You should setup this heightfield around the zones which will be loaded.
	 * It is applied only when a zone is loaded, so you should setup it 2km around the user, each time you move too far 
	 * from a previous place (eg 160m from last setup).
	 */
	void		setHeightField(const CHeightMap &hf);
	// @}



// ********************************
private:
	// Private part used by CPatch.
	friend class	CPatch;
	friend class	CZone;

	// Return the render pass for a far texture here.
	CPatchRdrPass	*getFarRenderPass(CPatch* pPatch, uint farIndex, float& far1UScale, float& far1VScale, float& far1UBias, float& far1VBias, bool& bRot);
	// Free the render pass for a far texture here.
	void freeFarRenderPass (CPatch* pPatch, CPatchRdrPass* pass, uint farIndex);
	// Return the render pass for a tile Id, and a patch Lightmap.
	CPatchRdrPass	*getTileRenderPass(uint16 tileId, bool additiveRdrPass);
	// Return the UvScaleBias for a tile Id. uv.z has the scale info. uv.x has the BiasU, and uv.y has the BiasV.
	// if bitmap type is CTile::alpha, Return also the additionla rot for alpha (else 0).
	void			getTileUvScaleBiasRot(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias, uint8 &rotAlpha);

	// release Far render pass/reset Tile/Far render.
	void			resetRenderFar();
	/// For changing TileMaxSubdivision. force tesselation to be under tile.
	void			forceMergeAtTileLevel();

	// Update globals value to CTessFace
	void updateGlobals (const CVector &refineCenter) const;

private:
	TZoneMap		Zones;

	// Parameters.
	float			_TileDistNear;
	float			_Threshold;
	bool			_RefineMode;
	float			_FarTransition;
	uint			_TileMaxSubdivision;

	// The temp VB for tiles and far passes.
	CVertexBuffer	FarVB;
	CVertexBuffer	TileVB;


	// Tiles Types.
	//=============
	// Texture Map. Use a RefPtr because TileTextureMap must not reference the object, but the ptr.
	typedef	NLMISC::CRefPtr<ITexture>			RPTexture;
	typedef	std::map<std::string, RPTexture>	TTileTextureMap;
	typedef	TTileTextureMap::iterator			ItTileTextureMap;
	// RdrPass Set.
	typedef	std::set<CPatchRdrPass>				TTileRdrPassSet;
	typedef	TTileRdrPassSet::iterator			ItTileRdrPassSet;
	typedef NLMISC::CSmartPtr<CPatchRdrPass>	TSPRenderPass;

	// The additional realtime structure for a tile.
	struct	CTileInfo
	{
		// NB: CSmartPtr are not used for simplicity, and because of TTileRdrPassSet...
		// CPatchRdrPass::RefCount are excplictly incremented/decremented...
		// The rdrpass for diffuse+Alpha material.
		CPatchRdrPass	*DiffuseRdrPass;
		// The rdrpass for additive material (may be NULL if no additive part).
		CPatchRdrPass	*AdditiveRdrPass;
		// The scale/Bias to access those tiles in the big texture.
		// uv.z has the scale info. uv.x has the BiasU, and uv.y has the BiasV.
		// Manages the demi-texel on tile border too.
		CVector			DiffuseUvScaleBias;
		CVector			AlphaUvScaleBias;
		CVector			AdditiveUvScaleBias;
		// The additional rotation for this tile, in alpha.
		uint8			RotAlpha;
	};


	// Tiles Data.
	//=============
	// The map of tile texture loaded.
	TTileTextureMap				TileTextureMap;
	// The set of tile Rdr Pass.
	TTileRdrPassSet				TileRdrPassSet;
	// The parrallel array of tile of those existing in TileBank. size of NbTilesMax.
	std::vector<CTileInfo*>		TileInfos;
	// The Lightmap rdrpass for tiles.
	// must have a vector of pointer, because of vector reallocation.
	std::vector<TSPRenderPass>		_TextureNears;
	uint							_NFreeLightMaps;


	// The Tile material.
	CMaterial		TileMaterial;

	// The Far material.
	CMaterial		FarMaterial;


	// *** Far texture	

	// ** Some types

	// The vector of set of far render pass
	typedef std::set<TSPRenderPass>				TSPRenderPassSet;
	typedef TSPRenderPassSet::iterator			ItSPRenderPassSet;
	typedef	std::vector<TSPRenderPassSet>		TSPRdrPassSetVector;
	TSPRenderPassSet							_FarRdrPassSet;					// Contain all the render pass not empty for pass0 and pass1
	TSPRdrPassSetVector							_FarRdrPassSetVectorFree;		// Contain the render pass not filled yet sorted by size for pass0 and pass1
	bool										_FarInitialized;

	// Used internaly by initTileBanks
	bool										eraseTileFarIfNotGood (uint tileNumber, uint sizeOrder0, uint sizeOrder1, uint sizeOrder2);

	// ** Some private methods
	static uint									getRdrPassIndexWithSize (uint width, uint height);
	void										addPatch ();
	void										removePatch ();


	// *** Lighting
	CRGBA			_LightValue[256];

private:
	// Internal only. Force load of the tile (with TileBank).
	void			loadTile(uint16 tileId);
	void			releaseTile(uint16 tileId);
	ITexture		*findTileTexture(const std::string &textName);
	CPatchRdrPass	*findTileRdrPass(const CPatchRdrPass &pass);

	// Tile LightMap mgt.
	// @{
	// Compute and get a lightmapId/lightmap renderpass.
	// lightmap returned is to be uses with getTileRenderPass(). The id returned must be stored.
	uint		getTileLightMap(CRGBA  map[NL_TILE_LIGHTMAP_SIZE*NL_TILE_LIGHTMAP_SIZE], CPatchRdrPass *&lightmapRdrPass);
	// tileLightMapId must be the id returned  by getTileLightMap().
	void		getTileLightMapUvInfo(uint tileLightMapId, CVector &uvScaleBias);
	// tileLightMapId must be the id returned  by getTileLightMap().
	void		releaseTileLightMap(uint tileLightMapId);
	// @}


	// check a zone, adding error to exception.
	void			checkZoneBinds(CZone &curZone, EBadBind &bindError);


	/** A Bezier patch of One value only.
	 * NB: unlike CBezierPatch, layout is inverted on Y. (NB: same formulas...)
	 */
	struct	CBezierPatchZ
	{
		/// The vertices a,b,c,d of the quad patch.
		float		Vertices[4];		
		/// The tangents ab, ba, bc, cb, cd, dc, da, ad. NB: tangents are points, not vectors.
		float		Tangents[8];		
		/// The interiors, ia,ib,ic,id. NB: interiors are points, not vectors.
		float		Interiors[4];		

		/// make default Interiors, according to Vertices and Tangents.
		void		makeInteriors();
		/// Evaluate.
		float		eval(float s, float t) const;			// s,t coordinates for quad.
	};


	// HeightFields.
	struct	CHeightField
	{
		std::vector<CBezierPatchZ>		ZPatchs;

		/// The origin of the bottom-left corner of this heightmap.
		float			OriginX, OriginY;
		/// The size of one Element ot this HeightMap (eg: 160x160 for a zone).
		float			SizeX, SizeY;
		float			OOSizeX, OOSizeY;
		/// The size of this array. Heights.size
		uint			Width, Height;
	};
	CHeightField	_HeightField;

};



} // NL3D


#endif // NL_LANDSCAPE_H

/* End of landscape.h */
