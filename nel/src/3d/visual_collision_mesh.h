/** \file visual_collision_mesh.h
 * <File description>
 *
 * $Id: visual_collision_mesh.h,v 1.2 2004/05/07 14:41:42 corvazier Exp $
 */

/* Copyright, 2000-2003 Nevrax Ltd.
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

#ifndef NL_VISUAL_COLLISION_MESH_H
#define NL_VISUAL_COLLISION_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/object_vector.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/aabbox.h"

namespace NL3D 
{


// ***************************************************************************
/**
 * Collision mesh used for camera collision for instance
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CVisualCollisionMesh : public NLMISC::CRefCount
{
public:

	/// Constructor
	CVisualCollisionMesh();

	/// build. NB: fails if too much vertices/triangles (>=65536) or if 0 vertices / triangles
	bool		build(const std::vector<NLMISC::CVector> &vertices, const std::vector<uint32> &triangles);

	/// get collision with camera. [0,1] value
	float		getCameraCollision(const NLMISC::CMatrix &instanceMatrix, class CCameraCol &camCol);

	/// compute the world bbox of an instance
	NLMISC::CAABBox	computeWorldBBox(const NLMISC::CMatrix &instanceMatrix);

// *********************
private:

	// A Static Grid Container. Only 65535 elements max can be inserted
	class	CStaticGrid
	{
	public:
		// create
		void	create(uint nbQuads, uint nbElts, const NLMISC::CAABBox &gridBBox);
		
		// add an element (bbox shoudl be included in gridBBox from create() )
		void	add(uint16 id, const NLMISC::CAABBox &bbox);
		
		// compile
		void	compile();
		
		// return the list of elements intersected. NB: the vector is enlarged to max, but real selection size is in the  return value
		uint	select(const NLMISC::CAABBox &bbox, std::vector<uint16> &res);
		
	private:
		struct CEltBuild
		{
			uint32	X0,Y0;
			uint32	X1,Y1;
		};
		// point to GridData
		struct CCase 
		{
			uint32	Start, NumElts;
		};
		uint32										_GridSizePower;
		uint32										_GridSize;
		NLMISC::CVector								_GridPos;
		NLMISC::CVector								_GridScale;
		// The Grid
		NLMISC::CObjectVector<CCase, false>			_Grid;
		// The raw elt data
		NLMISC::CObjectVector<uint16, false>		_GridData;
		// Used at build only
		NLMISC::CObjectVector<CEltBuild, false>		_EltBuild;
		uint32										_GridDataSize;
		
		// For Fast selection
		uint32										_ItSession;
		// For each element the session id. if same than _ItSession, then already inserted
		NLMISC::CObjectVector<uint32, false>		_Sessions;
	};

private:

	// Data
	std::vector<NLMISC::CVector>	Vertices;
	std::vector<uint16>		Triangles;
	// The Local Triangle Quadgrid
	CStaticGrid				QuadGrid;
	
};


} // NL3D


#endif // NL_VISUAL_COLLISION_MESH_H

/* End of visual_collision_mesh.h */
