/** \file landscape_user.h
 * <File description>
 *
 * $Id: landscape_user.h,v 1.2 2001/02/28 16:19:51 berenguier Exp $
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

#ifndef NL_LANDSCAPE_USER_H
#define NL_LANDSCAPE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_landscape.h"
#include "nel/3d/landscape_model.h"
#include "nel/3d/zone_manager.h"
#include "nel/3d/scene.h"


namespace NL3D
{


//****************************************************************************
/**
 * ULandscape Implementation
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CLandscapeUser : public ULandscape
{
private:
	// The Scene.
	CScene				*_Scene;
	// The landscape, owned by the scene.
	CLandscapeModel		*_Landscape;
	// The zoneManager.
	CZoneManager		_ZoneManager;

public:

	/// \name Object
	// @{
	CLandscapeUser(CScene *scene)
	{
		nlassert(scene);
		_Scene= scene;
		_Landscape= (CLandscapeModel*)_Scene->createModel(LandscapeModelId);
		// TODO_COLLISION: This is temporary!!!!
		CollisionManager.init(&(_Landscape->Landscape), 200);
	}
	virtual	~CLandscapeUser()
	{
		_Scene->deleteModel(_Landscape);
		_Landscape= NULL;
	}
	// @}


	/// \name Load
	/// All those load methods use CPath to search files.
	// @{
	/// Set the zonePath from where zones are loaded.
	virtual	void	setZonePath(const std::string &zonePath);
	/// Load the tile banks:  the ".bank" and the  ".farbank".
	virtual	void	loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile);
	/// Load all Zones around a position. This is a blocking call.
	virtual	void	loadAllZonesAround(const CVector &pos, float radius);
	/// Delete old zones, or load new zones, around a position. new Zones are loaded async.
	virtual	void	refreshZonesAround(const CVector &pos, float radius);
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
	virtual	void	setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply);
	// @}


	/// \name Parameters
	// @{
	/// Set threshold for subdivsion quality. The lower is threshold, the more the landscape is subdivided. Default: 0.001.
	virtual	void	setThreshold (float thre);
	/// Get threshold.
	virtual	float	getThreshold () const;
	/// Set tile near distance. Default 50.f. maximized to length of Far alpha transition).
	virtual	void	setTileNear (float tileNear);
	/// Get tile near distance.
	virtual	float	getTileNear () const;
	/// Set Maximum Tile subdivision. Valid values must be in [0..4]  (assert). Default is 0 (for now :) ).
	virtual	void	setTileMaxSubdivision (uint tileDiv);
	/// Get Maximum Tile subdivision.
	virtual	uint 	getTileMaxSubdivision ();
	// @}


	/// \name Misc
	// @{
	/// Return the name of the zone around a particular position (in NL3D basis!).
	virtual	std::string	getZoneName(const CVector &pos);
	// @}

};


} // NL3D


#endif // NL_LANDSCAPE_USER_H

/* End of landscape_user.h */
