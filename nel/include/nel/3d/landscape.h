/** \file landscape.h
 * <File description>
 *
 * $Id: landscape.h,v 1.8 2000/11/30 10:57:13 berenguier Exp $
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


	/** Clip the landscape according to frustum. 
	 * Planes must be normalized.
	 */
	void			clip(const CVector &refineCenter, const std::vector<CPlane>	&pyramid);
	/// Refine/Geomorph the tesselation of the landscape.
	void			refine(const CVector &refineCenter);
	/// Render the landscape.
	void			render(IDriver *drv, const CVector &refineCenter, bool doTileAddPass=false);


	// TODO: landscape param setup (Transition etc...).
	// Store it by landscape, and not only globally in CTessFace statics.


	/** Update and refresh a patch texture.
	 * Usefull for Tile edition. Even if patch is in tile mode, it is refreshed...
	 * \param zoneId the zone of the update.
	 * \param numPatch the index of patch in zoneId which will receive his new texture. assert if bad id.
	 * \param tiles the patch texture. assert if not of good size (OrderS*OrderT).
	 * \return false if zone not loaded in landscape.
	 */
	bool			changePatchTexture(sint zoneId, sint numPatch, const std::vector<CTileElement> &tiles);


private:
	// Private part used by CPatch.
	friend class	CPatch;

	// TODO_TEXTURE.
	// dummy Far texture here.
	CPatchRdrPass	*getFarRenderPass() {return &FarRdrPass;}
	// Return the render pass for a tile Id.
	CPatchRdrPass	*getTileRenderPass(uint16 tileId, bool additiveRdrPass);
	// Return the UvScaleBias for a tile Id. uv.z has the scale info. uv.x has the BiasU, and uv.y has the BiasV.
	void			getTileUvScaleBias(sint tileId, bool additiveRdrPass, CVector &uvScaleBias);


private:
	TZoneMap		Zones;

	// The temp VB for tiles and far passes.
	CVertexBuffer	FarVB;
	CVertexBuffer	TileVB;
	// The temp prim block, for each rdrpass. re-allocation rarely occurs.
	CPrimitiveBlock	PBlock;


	// The map of tile texture.
	typedef	NLMISC::CSmartPtr<ITexture>	PTexture;
	typedef	std::map<std::string, PTexture>	TTileTextureMap;
	typedef	TTileTextureMap::iterator	ItTileTextureMap;
	TTileTextureMap		TileTextureMap;


	struct	CTileInfo
	{
		// Is this tile correctly loaded?
		bool			TileOk;
		// Should be a pointer, when/if tiles will be grouped in a multiple big square textures.
		CPatchRdrPass	RdrPass;
		// The scale/Bias to access this tile in those big texture.
		// uv.z has the scale info. uv.x has the BiasU, and uv.y has the BiasV.
		// Manages the demi-texel on tile border too.
		CVector			UvScaleBias;
	};
	struct	CTileKey
	{
		uint16	TileId;
		bool	Additive;
		bool	operator<(const CTileKey &k) const
		{
			if(Additive!=k.Additive)
				return !Additive;
			else
				return TileId<k.TileId;
		}
	};


	// The map of tile Rdr Pass.
	typedef	std::map<CTileKey, CTileInfo>	TTileRdrPassMap;
	typedef	TTileRdrPassMap::iterator	ItTileRdrPassMap;
	TTileRdrPassMap		TileRdrPassMap;

	// TODO_TEXTURE.
	// For test only. The only one Far material.
	CPatchRdrPass	FarRdrPass;


private:
	// Internal only. Force the insert in TileRdrPassMap of the tile (with TileBank).
	void			loadTile(const CTileKey &key);

};



} // NL3D


#endif // NL_LANDSCAPE_H

/* End of landscape.h */
