/** \file visual_collision_entity.h
 * <File description>
 *
 * $Id: visual_collision_entity.h,v 1.4 2001/12/27 11:17:48 berenguier Exp $
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
	 * and meshes. see setSnapToRenderedTesselation() option
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	bool	snapToGround(CVector &pos);


	/** Snap the entity onto the ground. pos.z is modified so that it lies on the ground, according to rendered landscapes
	 * and meshes.
	 * \param normal the ret normal of where it is snapped. NB: if return false, not modified.
	 * \return true if pos.z has been modified (sometimes it may not find a solution).
	 */
	bool	snapToGround(CVector &pos, CVector &normal);



	/** If groundMode is true, the entity is snapped on faces with normal.z > 0. Default is true.
	 *	NB: if both groundMode and ceilMode are false, snapToGround is a no-op.
	 */
	void	setGroundMode(bool groundMode) {_GroundMode= groundMode;}


	/** If ceilMode is true, the entity is snapped on faces with normal.z < 0. Default is false.
	 *	NB: if both groundMode and ceilMode are false, snapToGround is a no-op.
	 */
	void	setCeilMode(bool ceilMode) {_CeilMode= ceilMode;}


	bool	getGroundMode() const {return _GroundMode;}
	bool	getCeilMode() const {return _CeilMode;}


	/** By default, the visual collision entity is snapped on rendered/geomorphed tesselation (true).
	 *  Use this method to change this behavior. if false, the entity is snapped to the tile level tesselation
	 *	according to noise etc...
	 */
	void	setSnapToRenderedTesselation(bool snapMode) {_SnapToRenderedTesselation= snapMode;}
	bool	getSnapToRenderedTesselation() const {return _SnapToRenderedTesselation;}


// ***********************
private:
	CVisualCollisionManager		*_Owner;

	bool	_CeilMode;
	bool	_GroundMode;
	bool	_SnapToRenderedTesselation;


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
	bool		triangleIntersect2DGround(CTriangle &tri, const CVector &pos0);
	bool		triangleIntersect2DCeil(CTriangle &tri, const CVector &pos0);
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
