/** \file landscape_user.cpp
 * <File description>
 *
 * $Id: landscape_user.cpp,v 1.23 2002/04/23 14:38:12 berenguier Exp $
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

using namespace NLMISC;

namespace NL3D
{


//****************************************************************************
void	CLandscapeUser::setZonePath(const std::string &zonePath)
{
	_ZoneManager.setZonePath(zonePath);
}
//****************************************************************************
void	CLandscapeUser::loadBankFiles(const std::string &tileBankFile, const std::string &farBankFile)
{

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
	zonesAdded.clear();

	_ZoneManager.loadAllZonesAround((uint)pos.x, (uint)(-pos.y), (uint)radius, true);
	while(_ZoneManager.getTaskListSize() != 0 || !_ZoneManager.ZoneAdded)
	{
		// Must do it at init only.
		nlassert(_ZoneManager.ZoneRemoved);

		if(!_ZoneManager.ZoneAdded)
		{
			_Landscape->Landscape.addZone(*_ZoneManager.Zone);

			delete _ZoneManager.Zone;
			zonesAdded.push_back(_ZoneManager.NameZoneAdded);
			_ZoneManager.ZoneAdded = true;
		}
		else
		{
			nlSleep(1);
		}
	}

	// Yoyo: must check the binds of the zones.
	_Landscape->Landscape.checkBinds();
}

//****************************************************************************
void	CLandscapeUser::refreshAllZonesAround(const CVector &pos, float radius, std::vector<std::string> &zonesAdded, std::vector<std::string> &zonesRemoved)
{
	zonesAdded.clear();
	zonesRemoved.clear();
	std::string		za, zr;

	_ZoneManager.loadAllZonesAround((uint)pos.x, (uint)(-pos.y), (uint)radius, true);

	// refresh until finished
	do
	{
		// refresh zone around me.
		refreshZonesAround(pos, radius, za, zr);

		// some zone added or removed??
		if(za != "")
			zonesAdded.push_back(za);
		if(zr != "")
			zonesRemoved.push_back(zr);
	}
	while(_ZoneManager.getTaskListSize() != 0 || !_ZoneManager.ZoneAdded || !_ZoneManager.ZoneRemoved );

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
	std::string	dummy1, dummy2;
	refreshZonesAround(pos, radius, dummy1, dummy2);
}
//****************************************************************************
void	CLandscapeUser::refreshZonesAround(const CVector &pos, float radius, std::string &zoneAdded, std::string &zoneRemoved)
{
	zoneRemoved= "";
	zoneAdded= "";

	// Check if new zone must be added to landscape
	if(!_ZoneManager.ZoneAdded)
	{
		_Landscape->Landscape.addZone(*_ZoneManager.Zone);

		// Yoyo: must check the binds of the new inserted zone.
		try
		{
			_Landscape->Landscape.checkBinds(_ZoneManager.Zone->getZoneId());
		}
		catch (EBadBind &e)
		{
			nlwarning ("Bind error : %s", e.what());
		}

		delete _ZoneManager.Zone;
		zoneAdded= _ZoneManager.NameZoneAdded;
		_ZoneManager.ZoneAdded = true;
	}

	// Check if a zone must be removed from landscape
	if(!_ZoneManager.ZoneRemoved)
	{
		_Landscape->Landscape.removeZone(_ZoneManager.IdZoneToRemove);

		zoneRemoved= _ZoneManager.NameZoneRemoved;
		_ZoneManager.ZoneRemoved = true;
	}

	_ZoneManager.loadAllZonesAround((uint)pos.x, (uint)(-pos.y), (uint)radius, false);
}


//****************************************************************************
void	CLandscapeUser::setupStaticLight (const CRGBA &diffuse, const CRGBA &ambiant, float multiply)
{
	_Landscape->Landscape.setupStaticLight(diffuse, ambiant, multiply);
}



//****************************************************************************
void	CLandscapeUser::setThreshold (float thre)
{
	_Landscape->Landscape.setThreshold(thre);
}
//****************************************************************************
float	CLandscapeUser::getThreshold () const
{
	return _Landscape->Landscape.getThreshold();
}
//****************************************************************************
void	CLandscapeUser::setTileNear (float tileNear)
{
	_Landscape->Landscape.setTileNear(tileNear);
}
//****************************************************************************
float	CLandscapeUser::getTileNear () const
{
	return _Landscape->Landscape.getTileNear();
}
//****************************************************************************
void	CLandscapeUser::setTileMaxSubdivision (uint tileDiv)
{
	_Landscape->Landscape.setTileMaxSubdivision(tileDiv);
}
//****************************************************************************
uint	CLandscapeUser::getTileMaxSubdivision ()
{
	return _Landscape->Landscape.getTileMaxSubdivision();
}


//****************************************************************************
std::string	CLandscapeUser::getZoneName(const CVector &pos)
{
	return _ZoneManager.getZoneName((uint)pos.x, (uint)(-pos.y), 0, 0).first;
}


//****************************************************************************
CVector		CLandscapeUser::getHeightFieldDeltaZ(float x, float y) const
{
	return _Landscape->Landscape.getHeightFieldDeltaZ(x,y);
}

//****************************************************************************
void		CLandscapeUser::setHeightField(const CHeightMap &hf)
{
	_Landscape->Landscape.setHeightField(hf);
}


//****************************************************************************
void		CLandscapeUser::enableVegetable(bool enable)
{
	_Landscape->Landscape.enableVegetable(enable);
}

//****************************************************************************
void		CLandscapeUser::loadVegetableTexture(const std::string &textureFileName)
{
	_Landscape->Landscape.loadVegetableTexture(textureFileName);
}

//****************************************************************************
void		CLandscapeUser::setupVegetableLighting(const CRGBA &ambient, const CRGBA &diffuse, const CVector &directionalLight)
{
	_Landscape->Landscape.setupVegetableLighting(ambient, diffuse, directionalLight);
}

//****************************************************************************
void		CLandscapeUser::setVegetableWind(const CVector &windDir, float windFreq, float windPower, float windBendMin)
{
	_Landscape->Landscape.setVegetableWind(windDir, windFreq, windPower, windBendMin);
}

//****************************************************************************
void		CLandscapeUser::setVegetableUpdateLightingFrequency(float freq)
{
	_Landscape->Landscape.setVegetableUpdateLightingFrequency(freq);
}


//****************************************************************************
void		CLandscapeUser::setPointLightFactor(const std::string &lightGroupName, NLMISC::CRGBA nFactor)
{
	_Landscape->Landscape.setPointLightFactor(lightGroupName, nFactor);
}

//****************************************************************************
void		CLandscapeUser::setUpdateLightingFrequency(float freq)
{
	_Landscape->Landscape.setUpdateLightingFrequency(freq);
}


//****************************************************************************
void		CLandscapeUser::enableAdditive (bool enable)
{
	_Landscape->enableAdditive(enable);
}
//****************************************************************************
bool		CLandscapeUser::isAdditiveEnabled () const
{
	return _Landscape->isAdditive ();
}

//****************************************************************************
void		CLandscapeUser::setPointLightDiffuseMaterial(CRGBA diffuse)
{
	_Landscape->Landscape.setPointLightDiffuseMaterial(diffuse);
}
//****************************************************************************
CRGBA		CLandscapeUser::getPointLightDiffuseMaterial () const
{
	return _Landscape->Landscape.getPointLightDiffuseMaterial();
}

//****************************************************************************
void		CLandscapeUser::setDLMGlobalVegetableColor(CRGBA gvc)
{
	_Landscape->Landscape.setDLMGlobalVegetableColor(gvc);
}
//****************************************************************************
CRGBA		CLandscapeUser::getDLMGlobalVegetableColor() const
{
	return _Landscape->Landscape.getDLMGlobalVegetableColor();
}



} // NL3D
