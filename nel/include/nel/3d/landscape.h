/** \file landscape.h
 * <File description>
 *
 * $Id: landscape.h,v 1.4 2000/11/10 09:57:34 berenguier Exp $
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


namespace NL3D 
{


// ***************************************************************************
/**
 * A landscape. Use CZone to build zone, and use landscape to dynamically add/remove them, for render.
 *
 *	Limits:
 *		- 65535	zones max in whole dans le monde entier (ZoneId sur 16 bits).
 *		- 65535 patchs maxi par zone.
 *		- patch d'ordre 2x2 minimum.
 *		- patch d'ordre 16x16 maximum.
 *		- conectivité sur un edge à 1, 2, ou 4 patchs.
 *		- la valeur globale du noise est globale, et ne peut pas dépasser 10 mètres.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CLandscape
{
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


private:
	// Private part used by CPatch.
	friend class	CPatch;

	// TODO_TEXTURE.
	// dummy texture here.
	CPatchRdrPass	*getFarRenderPass() {return &FarRdrPass;}
	CPatchRdrPass	*getTileRenderPass() {return &TileRdrPass;}


private:
	TZoneMap		Zones;

	// The temp VB for tiles and far passes.
	CVertexBuffer	FarVB;
	CVertexBuffer	TileVB;
	// The temp prim block, for each rdrpass. re-allocation rarely occurs.
	CPrimitiveBlock	PBlock;

	// TODO_TEXTURE.
	// For test only. The only two material (far and tile).
	CPatchRdrPass	FarRdrPass;
	CPatchRdrPass	TileRdrPass;
	CMaterial		FarMat;
	CMaterial		TileMat;
	CSmartPtr<CTexture>		FarText;
	CSmartPtr<CTexture>		TileText;

};



} // NL3D


#endif // NL_LANDSCAPE_H

/* End of landscape.h */
