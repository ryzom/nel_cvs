/** \file landscape_user.cpp
 * <File description>
 *
 * $Id: landscape_user.cpp,v 1.4 2001/06/15 16:24:43 corvazier Exp $
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

#include "3d/landscape_user.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"


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
void	CLandscapeUser::loadAllZonesAround(const CVector &pos, float radius)
{
	_ZoneManager.loadAllZonesAround((uint)pos.x, (uint)(-pos.y), (uint)radius, true);
	while(_ZoneManager.getTaskListSize() != 0)
	{
		if(!_ZoneManager.ZoneAdded)
		{
			_Landscape->Landscape.addZone(*_ZoneManager.Zone);
			// TODO_COLLISION: This is temporary!!
			CollisionManager.addZone(_ZoneManager.Zone->getZoneId());
			delete _ZoneManager.Zone;
			_ZoneManager.ZoneAdded = true;
		}
		else
		{
			nlSleep(0);
		}
	}

	// Yoyo: must check the binds of the zones.
	_Landscape->Landscape.checkBinds();
}
//****************************************************************************
void	CLandscapeUser::refreshZonesAround(const CVector &pos, float radius)
{
	// Check if new zone must be added to landscape
	if(!_ZoneManager.ZoneAdded)
	{
		_Landscape->Landscape.addZone(*_ZoneManager.Zone);
		// TODO_COLLISION: This is temporary!!
		CollisionManager.addZone(_ZoneManager.Zone->getZoneId());

		// Yoyo: must check the binds of the new inserted zone.
		_Landscape->Landscape.checkBinds(_ZoneManager.Zone->getZoneId());

		delete _ZoneManager.Zone;
		_ZoneManager.ZoneAdded = true;
	}

	// Check if a zone must be removed from landscape
	if(!_ZoneManager.ZoneRemoved)
	{
		// TODO_COLLISION: This is temporary!!
		CollisionManager.removeZone(_ZoneManager.IdZoneToRemove);
		_Landscape->Landscape.removeZone(_ZoneManager.IdZoneToRemove);
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


} // NL3D
