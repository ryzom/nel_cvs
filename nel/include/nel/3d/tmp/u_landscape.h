/** \file u_landscape.h
 * <File description>
 *
 * $Id: u_landscape.h,v 1.2 2001/04/24 09:36:52 berenguier Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_U_LANDSCAPE_H
#define NL_U_LANDSCAPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include <string>
// TODO_COLLISION: This is temporary!!!!
#include "nel/3d/mini_col.h"


namespace NL3D
{


using	NLMISC::CVector;
using	NLMISC::CRGBA;


// ***************************************************************************
/**
 * Game Interface for manipulate Landscape.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class ULandscape
{
protected:

	/// \name Object
	/// protected because created/deleted by UScene.
	// @{
	ULandscape() {}
	virtual	~ULandscape() {}
	// @}


public:
	// TODO_COLLISION: This is temporary!!!!
	// This is the collision Manager for this landscape. Temp!!!
	// It is inited in ULandscapte ctor.
	CMiniCol		CollisionManager;
	


public:

	/// \name Load
	/// All those load methods use CPath to search files.
	// @{
	/// Set the zonePath from where zones are loaded.
	virtual	void	setZonePath(const std::string &zonePath) =0;
	/// Load the tile banks:  the ".bank" and the  ".farbank".
	virtual	void	loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile) =0;
	/// Load all Zones around a position. This is a blocking call.
	virtual	void	loadAllZonesAround(const CVector &pos, float radius) =0;
	/// Delete old zones, or load new zones, around a position. new Zones are loaded async.
	virtual	void	refreshZonesAround(const CVector &pos, float radius) =0;
	// @}


	/// \name Lighting
	// @{
	/**
	  *  Setup the light color use for static illumination.
	  *  NB: This setup will be visible only for new texture far/near computed (when player move or see dynamic lighting).
	  *
	  *  \param diffuse is the color of the diffuse componante of the lighting.
	  *  \param ambiant is the color of the ambiante componante of the lighting.
	  *  \param multiply is the multiply factor. Final color is (diffuse*multiply*shading+ambiant*(1.0-shading))
	  */
	virtual	void	setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply) =0;
	// @}


	/// \name Parameters
	// @{
	/// Set threshold for subdivsion quality. The lower is threshold, the more the landscape is subdivided. Default: 0.001.
	virtual	void	setThreshold (float thre) =0;
	/// Get threshold.
	virtual	float	getThreshold () const =0;
	/// Set tile near distance. Default 50.f. maximized to length of Far alpha transition).
	virtual	void	setTileNear (float tileNear) =0;
	/// Get tile near distance.
	virtual	float	getTileNear () const =0;
	/// Set Maximum Tile subdivision. Valid values must be in [0..4]  (assert). Default is 0 (for now :) ).
	virtual	void	setTileMaxSubdivision (uint tileDiv) =0;
	/// Get Maximum Tile subdivision.
	virtual	uint 	getTileMaxSubdivision () =0;
	// @}


	/// \name Misc
	// @{
	/// Return the name of the zone around a particular position (in NL3D basis!).
	virtual	std::string	getZoneName(const CVector &pos) =0;
	// @}


	/// \name HeightField DeltaZ.
	// @{
	/// return the HeightField DeltaZ for the 2D position. (0,0,dZ) is returned.
	virtual	CVector		getHeightFieldDeltaZ(float x, float y) const =0;
	/** set the HeightField data. NB: take lot of place in memory.
	 * only one is possible. You should setup this heightfield around the zones which will be loaded.
	 * It is applied only when a zone is loaded, so you should setup it 2km around the user, each time you move too far 
	 * from a previous place (eg 160m from last setup).
	 */
	virtual	void		setHeightField(const CHeightMap &hf) =0;
	// @}

};


} // NL3D


#endif // NL_U_LANDSCAPE_H

/* End of u_landscape.h */
