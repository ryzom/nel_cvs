/** \file visual_collision_manager.cpp
 * <File description>
 *
 * $Id: visual_collision_manager.cpp,v 1.6 2002/06/25 09:45:03 corvazier Exp $
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

#include "3d/visual_collision_manager.h"
#include "3d/visual_collision_entity.h"
#include "3d/landscape.h"
#include "nel/misc/common.h"


namespace NL3D 
{


// ***************************************************************************
// Those blocks size are computed to be approximatively one block for 10 entities.
const	uint	TileDescNodeAllocatorBlockSize= 40000;
const	uint	PatchQuadBlockAllocatorBlockSize= 160;


// ***************************************************************************
CVisualCollisionManager::CVisualCollisionManager() : 
	_TileDescNodeAllocator(TileDescNodeAllocatorBlockSize), 
	_PatchQuadBlockAllocator(PatchQuadBlockAllocatorBlockSize)
{
	_Landscape= NULL;

	// Default.
	setSunContributionPower(0.5f, 0.5f);
}
// ***************************************************************************
CVisualCollisionManager::~CVisualCollisionManager()
{
	_Landscape= NULL;
}


// ***************************************************************************
void					CVisualCollisionManager::setLandscape(CLandscape *landscape)
{
	_Landscape= landscape;
}


// ***************************************************************************
CVisualCollisionEntity		*CVisualCollisionManager::createEntity()
{
	return new CVisualCollisionEntity(this);
}


// ***************************************************************************
void						CVisualCollisionManager::deleteEntity(CVisualCollisionEntity	*entity)
{
	delete entity;
}


// ***************************************************************************
CVisualTileDescNode		*CVisualCollisionManager::newVisualTileDescNode()
{
	return _TileDescNodeAllocator.allocate();
}

// ***************************************************************************
void					CVisualCollisionManager::deleteVisualTileDescNode(CVisualTileDescNode *ptr)
{
	_TileDescNodeAllocator.free(ptr);
}

// ***************************************************************************
CPatchQuadBlock			*CVisualCollisionManager::newPatchQuadBlock()
{
	return _PatchQuadBlockAllocator.allocate();
}

// ***************************************************************************
void					CVisualCollisionManager::deletePatchQuadBlock(CPatchQuadBlock *ptr)
{
	_PatchQuadBlockAllocator.free(ptr);
}


// ***************************************************************************
void					CVisualCollisionManager::setSunContributionPower(float power, float maxThreshold)
{
	NLMISC::clamp(power, 0.f, 1.f);

	for(uint i=0; i<256; i++)
	{
		float	f= i/255.f;
		f = powf(f/maxThreshold, power);
		sint	uf= (sint)floor(255*f);
		NLMISC::clamp(uf, 0, 255);
		_SunContributionLUT[i]= uf;
	}

}



} // NL3D
