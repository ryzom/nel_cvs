/** \file water_pool_manager.h
 * <File description>
 *
 * $Id: water_pool_manager.h,v 1.1 2001/10/26 08:21:57 vizerie Exp $
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

#ifndef NL_WATER_POOL_MANAGER_H
#define NL_WATER_POOL_MANAGER_H

#include "nel/misc/types_nl.h"
#include <map>

namespace NL3D {

class CWaterHeightMap;

/**
 * This class helps managing various waters pools
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CWaterPoolManager
{
public:
	/// create a water pool with the given id and the given parameters. It replaces any existing pool with the same ID
	void createWaterPool(uint ID, uint size, float fluidity, float filterWeight, float unitSize);
	/// Get a water pool by its ID. If the ID doesn't exist, a new pool is created with default parameters
	CWaterHeightMap &getPoolByID(uint ID);
	
	/// delete all heightmaps
	void reset();
	
	// dtor
	~CWaterPoolManager() { reset(); }
private:	
	friend CWaterPoolManager &GetWaterPoolManager();	
	CWaterPoolManager() {}	
	typedef std::map<uint, CWaterHeightMap *> TPoolMap;
	TPoolMap _PoolMap;	
};


// get the only water pool manager
CWaterPoolManager &GetWaterPoolManager();


} // NL3D


#endif // NL_WATER_POOL_MANAGER_H

/* End of water_pool_manager.h */
