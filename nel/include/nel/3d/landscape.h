/** \file landscape.h
 * <File description>
 *
 * $Id: landscape.h,v 1.13 2000/12/04 16:58:20 berenguier Exp $
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
#include "nel/3d/zone.h"
#include "nel/3d/tile_bank.h"
#include "nel/3d/patch_rdr_pass.h"
#include <map>


namespace NL3D 
{


// ***************************************************************************
// The maximum amount of different tiles in world.
const	sint	NbTilesMax= 65536;


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
 *		- The value of Noise amplitude is global and cannot go over 10 meters.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CLandscape
{
public:
	// The bank of tiles information.
	CTileBank		TileBank;

public:

	/// Constructor
	CLandscape();
	/// Destructor. clear().
	~CLandscape();


	/// init the landscape VBuffers, texture cache etc...
	void			init(bool bumpTiles=false);


	/** Add a zone which should be builded (or loaded), but not compiled. CLandscape compile it.
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

	
	/// Set tile near distance. Default 50.f.
	void setTileNear (float tileNear)
	{
		_TileDistNear=tileNear;
	}
	/// Get tile near distance.
	float getTileNear () const
	{
		return _TileDistNear;
	}
	// TODO: other landscape param setup (Transition etc...).
	// Store it by landscape, and not only globally in CTessFace statics.


	/** Clip the landscape according to frustum. 
	 * Planes must be normalized.
	 */
	void			clip(const CVector &refineCenter, const std::vector<CPlane>	&pyramid);
	/// Refine/Geomorph the tesselation of the landscape.
	void			refine(const CVector &refineCenter);
	/// Render the landscape.
	void			render(IDriver *drv, const CVector &refineCenter, bool doTileAddPass=false);


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


	/// Force a range of tiles to be loaded in the driver...
	void			flushTiles(IDriver *drv, uint16 tileStart, uint16 nbTiles);
	/// Force a range of tiles to be loaded in the driver...
	void			releaseTiles(uint16 tileStart, uint16 nbTiles);


	/// Return the texture for a tile Id. UseFull for Tile edition.
	NLMISC::CSmartPtr<ITexture>		getTileTexture(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias);


private:
	// Private part used by CPatch.
	friend class	CPatch;
	friend class	CZone;

	// TODO_TEXTURE.
	// dummy Far texture here.
	CPatchRdrPass	*getFarRenderPass() {return &FarRdrPass;}
	// Return the render pass for a tile Id.
	CPatchRdrPass	*getTileRenderPass(uint16 tileId, bool additiveRdrPass);
	// Return the UvScaleBias for a tile Id. uv.z has the scale info. uv.x has the BiasU, and uv.y has the BiasV.
	void			getTileUvScaleBias(uint16 tileId, CTile::TBitmap bitmapType, CVector &uvScaleBias);

	// Update globals value to CTessFace
	void updateGlobals () const;

private:
	TZoneMap		Zones;

	// Parameters.
	float			_TileDistNear;

	// The temp VB for tiles and far passes.
	CVertexBuffer	FarVB;
	CVertexBuffer	TileVB;
	// The temp prim block, for each rdrpass. re-allocation rarely occurs.
	CPrimitiveBlock	PBlock;


	// Shortcuts.
	// Use a RefPtr because TileTextureMap must not reference the object, but the ptr.
	typedef	NLMISC::CRefPtr<ITexture>			RPTexture;


	// The map of tile texture loaded.
	typedef	std::map<std::string, RPTexture>	TTileTextureMap;
	typedef	TTileTextureMap::iterator			ItTileTextureMap;
	TTileTextureMap		TileTextureMap;


	// The additional realtime structure for a tile.
	struct	CTileInfo
	{
		// NB: CSmartPtr are not used for simplicity, and because of TTileRdrPassSet...
		// CPatchRdrPass::RefCount are excplictly incremented/decremented...
		// The rdrpass for diffuse+bump material.
		CPatchRdrPass	*DiffuseRdrPass;
		// The rdrpass for additive material (may be NULL if no additive part).
		CPatchRdrPass	*AdditiveRdrPass;
		// The scale/Bias to access those tiles in the big texture.
		// uv.z has the scale info. uv.x has the BiasU, and uv.y has the BiasV.
		// Manages the demi-texel on tile border too.
		CVector			DiffuseUvScaleBias;
		CVector			BumpUvScaleBias;
		CVector			AdditiveUvScaleBias;
	};

	// The parrallel array of tile of those existing in TileBank. size of NbTilesMax.
	std::vector<CTileInfo*>				TileInfos;


	// The set of tile Rdr Pass.
	typedef	std::set<CPatchRdrPass>		TTileRdrPassSet;
	typedef	TTileRdrPassSet::iterator	ItTileRdrPassSet;
	TTileRdrPassSet		TileRdrPassSet;


	// The Tile material.
	CMaterial		TileMaterial;


	// TODO_TEXTURE.
	// For test only. The only one Far material.
	CMaterial		FarMaterial;
	CPatchRdrPass	FarRdrPass;


private:
	// Internal only. Force load of the tile (with TileBank).
	void			loadTile(uint16 tileId);
	void			releaseTile(uint16 tileId);
	ITexture		*findTileTexture(const std::string &textName);
	CPatchRdrPass	*findTileRdrPass(const CPatchRdrPass &pass);

};



} // NL3D


#endif // NL_LANDSCAPE_H

/* End of landscape.h */
