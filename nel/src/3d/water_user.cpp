/** \file water_user.cpp
 * implementation of the user interface for water
 *
 * $Id: water_user.cpp,v 1.6 2004/02/20 14:36:05 vizerie Exp $
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

#include "std3d.h"

#include "nel/3d/u_water.h"
#include "3d/water_pool_manager.h"
#include "3d/water_height_map.h"
#include "3d/water_model.h"
#include "3d/water_user.h"
#include "3d/driver_user.h"


namespace NL3D 
{

//===========================================================================
UWaterHeightMap &UWaterHeightMapManager::getWaterHeightMapFromID(uint32 ID)
{
	NL3D_MEM_WATER
	nlassert(GetWaterPoolManager().hasPool(ID)); // unknown pool ID!
	return  GetWaterPoolManager().getPoolByID(ID);
}


//===========================================================================
void	UWaterHeightMapManager::setBlendFactor(UDriver *drv, float value)
{
	NL3D_MEM_WATER
	NLMISC::clamp(value, 0.f, 1.f);
	GetWaterPoolManager().setBlendFactor(NLMISC::safe_cast<CDriverUser *>(drv)->getDriver(), value);
}

//===========================================================================
void UWaterHeightMapManager::releaseBlendTextures()
{
	GetWaterPoolManager().releaseBlendTextures();
}

//===========================================================================
uint32	CWaterInstanceUser::getWaterHeightMapID() const
{
	NL3D_MEM_WATER
	CWaterModel *wm = NLMISC::safe_cast<CWaterModel *>(_Instance);
	return wm->getWaterHeightMapID();
}


//===========================================================================
float	CWaterInstanceUser::getHeightFactor() const
{
	NL3D_MEM_WATER
	CWaterModel *wm = NLMISC::safe_cast<CWaterModel *>(_Instance);
	return wm->getHeightFactor();
}

//===========================================================================
float   CWaterInstanceUser::getHeight(const NLMISC::CVector2f &pos)
{
	NL3D_MEM_WATER
	CWaterModel *wm = NLMISC::safe_cast<CWaterModel *>(_Instance);
	return wm->getHeight(pos);
}

//===========================================================================
float   CWaterInstanceUser::getAttenuatedHeight(const NLMISC::CVector2f &pos, const NLMISC::CVector &viewer)
{
	NL3D_MEM_WATER
	CWaterModel *wm = NLMISC::safe_cast<CWaterModel *>(_Instance);
	return wm->getAttenuatedHeight(pos, viewer);
}

} // NL3D