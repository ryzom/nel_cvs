/** \file visual_collision_entity.h
 * <File description>
 *
 * $Id: visual_collision_entity.h,v 1.2 2001/07/13 16:08:14 berenguier Exp $
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

#ifndef NL_VISUAL_COLLISION_ENTITY_H
#define NL_VISUAL_COLLISION_ENTITY_H

#include "nel/misc/types_nl.h"
#include "3d/patch.h"
#include "3d/landscape_collision_grid.h"


namespace NL3D 
{


class	CVisualCollisionManager;


// ***************************************************************************
/**
 * An entity created by CVisualCollisionManager.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVisualCollisionEntity
{
public:

	/// Constructor
	CVisualCollisionEntity(CVisualCollisionManager *owner);
	~CVisualCollisionEntity();


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 * and meshes.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	bool	snapToGround(CVector &pos);


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 * and meshes.
	 * \param normal the ret normal of where it is snapped. NB: if return false, not modified.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	bool	snapToGround(CVector &pos, CVector &normal);



// ***********************
private:
	CVisualCollisionManager		*_Owner;


	/// \name Landscape part.
	// @{
	/// This is the radius of the bbox around the entity where we have correct collisions.
	static const float					_BBoxRadius;
	/// Same as _BBoxRadius, but for z value. This later should be greater because of NLPACS surface quadtree imprecision.
	static const float					_BBoxRadiusZ;
	/// Default capacity of _PatchQuadBlocks.
	static const uint32					_StartPatchQuadBlockSize;
	/// This is the temp array of BlockIds filled by landscape search.
	static std::vector<CPatchBlockIdent>	_TmpBlockIds;
	/// This is the temp array of PatchBlocks ptr.
	static std::vector<CPatchQuadBlock*>	_TmpPatchQuadBlocks;

	/** Array of quadBlock which are around the entity. NB: plain vector, because not so big (ptrs).
	 * NB: reserve to a big size (64), so reallocation rarely occurs.
	 * NB: this array is sorted in ascending order (comparison of CPatchBlockIdent).
	 */
	std::vector<CPatchQuadBlock*>		_PatchQuadBlocks;
	/// A quadgrid of chainlist of tileId (CVisualTileDescNode), which are around the entity.
	CLandscapeCollisionGrid				_LandscapeQuadGrid;
	/// The current BBox where the entity lies.
	CAABBox								_CurrentBBox;


	/// Fast "2D" test of a triangle against ray P0 P1.
	bool		triangleIntersect(CTriangle &tri, const CVector &pos0, const CVector &pos1, CVector &hit);
	/// test if the new position is outside the preceding setuped bbox, and then compute tiles infos around the position.
	void		testComputeLandscape(const CVector &pos);
	/// compute tiles infos around the position.
	void		doComputeLandscape(const CVector &pos);

	/// snap to current rendered tesselation.
	void		snapToLandscapeCurrentTesselation(CVector &pos, const CTrianglePatch &tri);

	// @}


};


} // NL3D


#endif // NL_VISUAL_COLLISION_ENTITY_H

/* End of visual_collision_entity.h */
