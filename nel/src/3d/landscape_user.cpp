/** \file landscape_user.cpp
 * <File description>
 *
 * $Id: landscape_user.cpp,v 1.28 2002/10/16 16:58:21 besson Exp $
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

#include "std3d.h"

#include "3d/landscape_user.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"

using namespace NLMISC;

namespace NL3D
{

H_AUTO_DECL( NL3D_UI_Landscape )
H_AUTO_DECL( NL3D_Render_Landscape_updateLightingAll )
H_AUTO_DECL( NL3D_Load_Landscape )

#define	NL3D_HAUTO_UI_LANDSCAPE						H_AUTO_USE( NL3D_UI_Landscape )
#define	NL3D_HAUTO_LANDSCAPE_UPDATE_LIGHTING_ALL	H_AUTO_USE( NL3D_Render_Landscape_updateLightingAll )
#define	NL3D_HAUTO_LOAD_LANDSCAPE					H_AUTO_USE( NL3D_Load_Landscape )


//****************************************************************************
void	CLandscapeUser::setZonePath(const std::string &zonePath)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_ZoneManager.setZonePath(zonePath);
}
//****************************************************************************
void	CLandscapeUser::loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	// First, load the banks.
	//=======================
	CIFile bankFile(CPath::lookup(tileBankFile));
	_Landscape->Landscape.TileBank.serial(bankFile);
	// All textures path are relative!
	_Landscape->Landscape.TileBank.makeAllPathRelative();
	// Use DDS!!!
	_Landscape->Landscape.TileBank.makeAllExtensionDDS();
	// No absolute path
	_Landscape->Landscape.TileBank.setAbsPath ("");

	// After loading the TileBank, and before initTileBanks(), must load the vegetables descritpor
	_Landscape->Landscape.TileBank.loadTileVegetableDescs();

	CIFile farbankFile(CPath::lookup(farBankFile));
	_Landscape->Landscape.TileFarBank.serial(farbankFile);
	if ( ! _Landscape->Landscape.initTileBanks() )
	{
		nlwarning( "You need to recompute bank.farbank for the far textures" );
	}
	bankFile.close();
	farbankFile.close();


	// Second, temporary, flushTiles.
	//===============================
	sint	ts;
	for (ts=0; ts<_Landscape->Landscape.TileBank.getTileSetCount (); ts++)
	{
		CTileSet *tileSet=_Landscape->Landscape.TileBank.getTileSet (ts);
		sint tl;
		for (tl=0; tl<tileSet->getNumTile128(); tl++)
			_Landscape->Landscape.flushTiles (_Scene->getDriver(), (uint16)tileSet->getTile128(tl), 1);
		for (tl=0; tl<tileSet->getNumTile256(); tl++)
			_Landscape->Landscape.flushTiles (_Scene->getDriver(), (uint16)tileSet->getTile256(tl), 1);
		for (tl=0; tl<CTileSet::count; tl++)
			_Landscape->Landscape.flushTiles (_Scene->getDriver(), (uint16)tileSet->getTransition(tl)->getTile (), 1);
	}

}


//****************************************************************************
void	CLandscapeUser::loadAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	zonesAdded.clear();

	_ZoneManager.checkZonesAround ((uint)pos.x, (uint)(-pos.y), (uint)radius);
	while (_ZoneManager.isWorking())
	{
		CZoneManager::SZoneManagerWork Work;
		if (_ZoneManager.isWorkComplete(Work))
		{
			nlassert(!Work.ZoneRemoved);
			if (Work.ZoneAdded)
			{
				_Landscape->Landscape.addZone (*Work.Zone);

				delete Work.Zone;
				std::string zoneadd = Work.NameZoneAdded;
				zoneadd = zoneadd.substr(0, zoneadd.find('.'));
				zonesAdded.push_back(zoneadd);
			}
		}
		else
		{
			nlSleep (1);
		}
		_ZoneManager.checkZonesAround ((uint)pos.x, (uint)(-pos.y), (uint)radius);
	}
	// Yoyo: must check the binds of the zones.
	_Landscape->Landscape.checkBinds();
}

//****************************************************************************
void	CLandscapeUser::refreshAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded, std::vector<std::string> &zonesRemoved)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	zonesAdded.clear();
	zonesRemoved.clear();
	std::string		za, zr;

	_ZoneManager.checkZonesAround ((uint)pos.x, (uint)(-pos.y), (uint)radius);
	while (_ZoneManager.isWorking())
	{
		refreshZonesAround (pos, radius, za, zr);

		// some zone added or removed??
		if(za != "")
			zonesAdded.push_back(za);
		if(zr != "")
			zonesRemoved.push_back(zr);
		
		_ZoneManager.checkZonesAround ((uint)pos.x, (uint)(-pos.y), (uint)radius);
	}
}

// ***************************************************************************
void	CLandscapeUser::getAllZoneLoaded(std::vector<std::string>	&zoneLoaded) const
{
	// Build the list of zoneId.
	std::vector<uint16>	zoneIds;
	_Landscape->Landscape.getZoneList(zoneIds);

	// transcript
	zoneLoaded.clear();
	zoneLoaded.resize(zoneIds.size());
	for(uint i=0;i<zoneLoaded.size();i++)
	{
		CLandscape::buildZoneName(zoneIds[i], zoneLoaded[i]);
	}
}

//****************************************************************************
void	CLandscapeUser::loadAllZonesAround(const CVector &pos, float radius)
{
	std::vector<std::string>	dummy;
	loadAllZonesAround(pos, radius, dummy);
}


//****************************************************************************
void	CLandscapeUser::refreshZonesAround(const CVector &pos, float radius)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	std::string	dummy1, dummy2;
	refreshZonesAround(pos, radius, dummy1, dummy2);
}
//****************************************************************************
void	CLandscapeUser::refreshZonesAround(const CVector &pos, float radius, std::string &zoneAdded, std::string &zoneRemoved)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;

	zoneRemoved= "";
	zoneAdded= "";
	CZoneManager::SZoneManagerWork Work;
	// Check if new zone must be added to landscape
	if (_ZoneManager.isWorkComplete(Work))
	{
		if (Work.ZoneAdded)
		{
			if (Work.Zone == (CZone*)-1)
			{
				std::string warn = std::string("Cant load zone ") + Work.NameZoneAdded;
				nlwarning (warn.c_str());
			}
			else
			{
				_Landscape->Landscape.addZone(*Work.Zone);

				// Yoyo: must check the binds of the new inserted zone.
				try
				{
					_Landscape->Landscape.checkBinds(Work.Zone->getZoneId());
				}
				catch (EBadBind &e)
				{
					nlwarning ("Bind error : %s", e.what());
				}

				delete Work.Zone;
				zoneAdded = Work.NameZoneAdded;
				zoneAdded = zoneAdded.substr(0, zoneAdded.find('.'));
			}
		}

		// Check if a zone must be removed from landscape
		if (Work.ZoneRemoved)
		{
			_Landscape->Landscape.removeZone (Work.IdZoneToRemove);
			zoneRemoved = Work.NameZoneRemoved;
			zoneRemoved = zoneRemoved.substr(0, zoneRemoved.find('.'));
		}
	}

	_ZoneManager.checkZonesAround((uint)pos.x, (uint)(-pos.y), (uint)radius);
}


//****************************************************************************
void	CLandscapeUser::setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setupStaticLight(diffuse, ambiant, multiply);
}



//****************************************************************************
void	CLandscapeUser::setThreshold (float thre)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setThreshold(thre);
}
//****************************************************************************
float	CLandscapeUser::getThreshold () const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getThreshold();
}
//****************************************************************************
void	CLandscapeUser::setTileNear (float tileNear)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setTileNear(tileNear);
}
//****************************************************************************
float	CLandscapeUser::getTileNear () const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getTileNear();
}
//****************************************************************************
void	CLandscapeUser::setTileMaxSubdivision (uint tileDiv)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setTileMaxSubdivision(tileDiv);
}
//****************************************************************************
uint	CLandscapeUser::getTileMaxSubdivision ()
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getTileMaxSubdivision();
}


//****************************************************************************
std::string	CLandscapeUser::getZoneName(const CVector &pos)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _ZoneManager.getZoneName((uint)pos.x, (uint)(-pos.y), 0, 0).first;
}


//****************************************************************************
CVector		CLandscapeUser::getHeightFieldDeltaZ(float x, float y) const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getHeightFieldDeltaZ(x,y);
}

//****************************************************************************
void		CLandscapeUser::setHeightField(const CHeightMap &hf)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setHeightField(hf);
}


//****************************************************************************
void		CLandscapeUser::enableVegetable(bool enable)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.enableVegetable(enable);
}

//****************************************************************************
void		CLandscapeUser::loadVegetableTexture(const std::string &textureFileName)
{
	NL3D_HAUTO_LOAD_LANDSCAPE;
	_Landscape->Landscape.loadVegetableTexture(textureFileName);
}

//****************************************************************************
void		CLandscapeUser::setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setupVegetableLighting(ambient, diffuse, directionalLight);
}

//****************************************************************************
void		CLandscapeUser::setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setVegetableWind(windDir, windFreq, windPower, windBendMin);
}

//****************************************************************************
void		CLandscapeUser::setVegetableUpdateLightingFrequency(float freq)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setVegetableUpdateLightingFrequency(freq);
}


//****************************************************************************
void		CLandscapeUser::setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setPointLightFactor(lightGroupName, nFactor);
}

//****************************************************************************
void		CLandscapeUser::setUpdateLightingFrequency(float freq)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setUpdateLightingFrequency(freq);
}


//****************************************************************************
void		CLandscapeUser::enableAdditive (bool enable)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->enableAdditive(enable);
}
//****************************************************************************
bool		CLandscapeUser::isAdditiveEnabled () const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->isAdditive ();
}

//****************************************************************************
void		CLandscapeUser::setPointLightDiffuseMaterial(CRGBA diffuse)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setPointLightDiffuseMaterial(diffuse);
}
//****************************************************************************
CRGBA		CLandscapeUser::getPointLightDiffuseMaterial () const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getPointLightDiffuseMaterial();
}

//****************************************************************************
void		CLandscapeUser::setDLMGlobalVegetableColor(CRGBA gvc)
{
	NL3D_HAUTO_UI_LANDSCAPE;
	_Landscape->Landscape.setDLMGlobalVegetableColor(gvc);
}
//****************************************************************************
CRGBA		CLandscapeUser::getDLMGlobalVegetableColor() const
{
	NL3D_HAUTO_UI_LANDSCAPE;
	return _Landscape->Landscape.getDLMGlobalVegetableColor();
}
//****************************************************************************
void		CLandscapeUser::updateLightingAll()
{
	NL3D_HAUTO_LANDSCAPE_UPDATE_LIGHTING_ALL;
	_Landscape->Landscape.updateLightingAll();
}



} // NL3D
