/** \file visual_collision_manager.cpp
 * <File description>
 *
 * $Id: visual_collision_manager.cpp,v 1.1 2001/06/08 16:12:52 berenguier Exp $
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

#include "nel/3d/visual_collision_manager.h"
#include "nel/3d/visual_collision_entity.h"
#include "nel/3d/landscape.h"


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



} // NL3D
