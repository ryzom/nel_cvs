/** \file water_pool_manager.cpp
 * <File description>
 *
 * $Id: water_pool_manager.cpp,v 1.4 2001/11/07 17:10:14 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "3d/water_pool_manager.h"
#include "3d/texture_blend.h"
#include "3d/water_shape.h"
#include "nel/misc/command.h"
#include "water_height_map.h"



namespace NL3D {


/*
NLMISC_COMMAND(setWaterPool, "Setup a pool of water in the water pool manager",
			   "<ID>,<Size>,<Damping>,<FilterWeight>,<UnitSize>,<WaveIntensity>,<WavePeriod>")
{
	CWaterPoolManager::CWaterHeightMapBuild whmb;
	const uint numArgs = args.size();
	if (numArgs == 0) return false;
	if (numArgs == 1)
	{
		whmb.ID = ::atoi(args[0].c_str());
	}
	if (numArgs == 2)
	{
		whmb.Size = ::atoi(args[1].c_str());
	}
	if (numArgs == 3)
	{
		whmb.FilterWeight = ::atof(args[2].c_str());
	}
	if (numArgs == 4)
	{
		whmb.UnitSize = ::atof(args[3].c_str());
	}
	if (numArgs == 5)
	{
		whmb.WaveIntensity = ::atof(args[4].c_str());
	}
	if (numArgs == 4)
	{
		whmb.WavePeriod = ::atof(args[5].c_str());
	}
	// create the water pool
	GetWaterPoolManager().createWaterPool(whmb);	
	return true;
}
*/

CWaterPoolManager &GetWaterPoolManager()
{
	static CWaterPoolManager singleton;
	return singleton;
}

CWaterHeightMap *CWaterPoolManager::createWaterPool(const CWaterHeightMapBuild &params)
{
	CWaterHeightMap *whm = _PoolMap.count(params.ID) == 0 ? new CWaterHeightMap : _PoolMap[params.ID];	
	whm->setDamping(params.Damping);
	whm->setFilterWeight(params.FilterWeight);
	whm->setSize(params.Size);
	whm->setUnitSize(params.UnitSize);
	whm->setWaves(params.WaveIntensity, params.WavePeriod);
	_PoolMap[params.ID] = whm; // in case it was just created
	return whm;
}

CWaterHeightMap &CWaterPoolManager::getPoolByID(uint ID)
{
	if(_PoolMap.count(ID))
	{
		return *_PoolMap[ID];
	}
	else
	{
		return *createWaterPool();	
	}
}
	

void CWaterPoolManager::reset()
{
	for (TPoolMap::iterator it = _PoolMap.begin(); it != _PoolMap.end(); ++it)
	{
		delete it->second;
	}
}
	

void CWaterPoolManager::registerWaterShape(CWaterShape *shape)
{	
	nlassert(std::find(_WaterShapes.begin(), _WaterShapes.end(), shape) == _WaterShapes.end()); // Shape registered twice!
	_WaterShapes.push_back(shape);
}

void CWaterPoolManager::unRegisterWaterShape(CWaterShape *shape)
{
	TWaterShapeVect::iterator it = std::find(_WaterShapes.begin(), _WaterShapes.end(), shape);
//	nlassert(it != _WaterShapes.end()); // shape not registered!
	if (it != _WaterShapes.end())
		_WaterShapes.erase(it);
}

void CWaterPoolManager::setBlendFactor(IDriver *drv, float factor)
{
	nlassert(factor >= 0 && factor <= 1);
	for (TWaterShapeVect::iterator it = _WaterShapes.begin(); it != _WaterShapes.end(); ++it)
	{
		CTextureBlend *tb = NLMISC::safe_cast<CTextureBlend *>((*it)->getEnvMap());
		if (tb->setBlendFactor((uint16) (256.f * factor)))
		{
			drv->setupTexture(*tb);
		}
	}
}

bool CWaterPoolManager::isWaterShapeObserver(const CWaterShape *shape) const
{
	return std::find(_WaterShapes.begin(), _WaterShapes.end(), shape) != _WaterShapes.end();
}

} // NL3D
