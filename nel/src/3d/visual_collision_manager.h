/** \file visual_collision_manager.h
 * <File description>
 *
 * $Id: visual_collision_manager.h,v 1.6 2004/03/23 15:38:43 berenguier Exp $
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

#ifndef NL_VISUAL_COLLISION_MANAGER_H
#define NL_VISUAL_COLLISION_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/block_memory.h"
#include "3d/patch.h"
#include "3d/landscape.h"
#include "3d/quad_grid.h"
#include "3d/visual_collision_mesh.h"


namespace NL3D 
{


class CVisualCollisionEntity;
class CLandscapeCollisionGrid;


// ***************************************************************************
/**
 * Server to Client collision manager. Snap logic position to Visual position.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CVisualTileDescNode
{
public:
	/// If od the patchblock containing this tile.
	uint16		PatchQuadBlocId;
	/// id in this patchblock of the tile.
	uint16		QuadId;
	/// for the quadgrid of chain list of CVisualTileDescNode.
	CVisualTileDescNode		*Next;
};


// ***************************************************************************
/**
 * Server to Client collision manager. Snap logic position to Visual position.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVisualCollisionManager
{
public:

	/// Constructor
	CVisualCollisionManager();
	~CVisualCollisionManager();


	/** setup the landscape used for this collision manager. ptr is kept, and manager and all entities must be cleared
	 * when the landscape is deleted.
	 */
	void						setLandscape(CLandscape *landscape);


	/** create an entity. NB: CVisualCollisionManager do not owns this ptr for now, and you must delete it with deleteEntity().
	 * NB: CVisualCollisionEntity are no more valid when this manager is deleted.
	 */
	CVisualCollisionEntity		*createEntity();

	/** delete an entity.
	 */
	void						deleteEntity(CVisualCollisionEntity	*entity);


	/** for CVisualCollisionEntity::getStaticLightSetup(). 
	 *  \see CVisualCollisionEntity::getStaticLightSetup()
	 *
	 *  Build a lighting table to remap sun contribution from landscape to sun contribution for objects.
	 *  The value remap the landscape sun contribution (0 ~ 1) to an object sun contribution (0 ~1)
	 *  using the following formula:
	 *
	 *  objectSunContribution = min ( powf ( landscapeSunContribution / maxThreshold, power ), 1 );
	 *
	 *	Default is 0.5 (=> sqrt) for power and 0.5 for maxThreshold.
	 */
	void						setSunContributionPower(float power, float maxThreshold);


	/** Get Typical Camera 3rd person collision.
	 *	For landscape, it is done only against TileFaces (ie only under approx 50 m)
	 *	return a [0,1] value. 0 => collision at start. 1 => no collision.
	 *	\param radius is the radius of the 'cylinder'
	 *	\param cone if true, the object tested is a cone (radius goes to end)
	 */
	float						getCameraCollision(const CVector &start, const CVector &end, float radius, bool cone);


	/** Add a Mesh Instance to the collision manager. For now it is used only for Camera Collision
	 *	\param mesh the collision mesh (keep a refptr on it)
	 *	\param instanceMatrix the matrix instance to apply to this mesh
	 *	\return the id used for remove, 0 if not succeed
	 */
	uint						addMeshInstanceCollision(CVisualCollisionMesh *mesh, const CMatrix &instanceMatrix);

	/** Remove a Mesh from the collision manager.
	 */
	void						removeMeshCollision(uint id);

// ***************************
private:
	/// The landscape used to generate tiles, and to snap position to tesselated ground.
	NLMISC::CRefPtr<CLandscape>			_Landscape;

	/// Allocators.
	CBlockMemory<CVisualTileDescNode>	_TileDescNodeAllocator;
	CBlockMemory<CPatchQuadBlock>		_PatchQuadBlockAllocator;

	// An instance of a Mesh collision
	class CMeshInstanceCol
	{
	public:
		// A ref to the visual col mesh
		CRefPtr<CVisualCollisionMesh>		Mesh;
		// The Matrix to apply to the mesh to get instance
		CMatrix								WorldMatrix;
		// The World BBox
		NLMISC::CAABBox						WorldBBox;
		// The pos in Mesh Instance quadgrid
		CQuadGrid<CMeshInstanceCol*>::CIterator	QuadGridIt;

	public:
		/// get collision with camera. [0,1] value
		float		getCameraCollision(class CCameraCol &camCol);
	};

	// The map of Meshes Instance
	typedef	std::map<uint32, CMeshInstanceCol>	TMeshColMap;
	TMeshColMap						_Meshs;
	// The QuadGrid of Collision meshs
	CQuadGrid<CMeshInstanceCol*>	_MeshQuadGrid;
	// The pool of id.
	uint32							_MeshIdPool;


private:
	friend class CVisualCollisionEntity;
	friend class CLandscapeCollisionGrid;

	// Allocators.
	CVisualTileDescNode			*newVisualTileDescNode();
	void						deleteVisualTileDescNode(CVisualTileDescNode *ptr);

	CPatchQuadBlock				*newPatchQuadBlock();
	void						deletePatchQuadBlock(CPatchQuadBlock *ptr);


	// setSunContributionPower Table.
	uint8						_SunContributionLUT[256];

};


} // NL3D


#endif // NL_VISUAL_COLLISION_MANAGER_H

/* End of visual_collision_manager.h */
