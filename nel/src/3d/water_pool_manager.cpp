/** \file water_pool_manager.cpp
 * <File description>
 *
 * $Id: water_pool_manager.cpp,v 1.2 2001/10/26 09:21:21 vizerie Exp $
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
#include "water_height_map.h"


namespace NL3D {


CWaterPoolManager &GetWaterPoolManager()
{
	static CWaterPoolManager singleton;
	return singleton;
}



CWaterHeightMap &CWaterPoolManager::getPoolByID(uint ID)
{
	if(_PoolMap.count(ID))
	{
		return *_PoolMap[ID];
	}
	else
	{
		CWaterHeightMap *whm = new CWaterHeightMap;
		_PoolMap[ID] = whm;
		return *whm;
	}
}
	

void CWaterPoolManager::reset()
{
	for (TPoolMap::iterator it = _PoolMap.begin(); it != _PoolMap.end(); ++it)
	{
		delete it->second;
	}
}
	


} // NL3D
