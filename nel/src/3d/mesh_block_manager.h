/** \file mesh_block_manager.h
 * <File description>
 *
 * $Id: mesh_block_manager.h,v 1.1 2002/06/19 08:42:10 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_MESH_BLOCK_MANAGER_H
#define NL_MESH_BLOCK_MANAGER_H

#include "nel/misc/types_nl.h"
#include "3d/mesh_geom.h"


namespace NL3D 
{


class	CMeshBaseInstance;
class	IDriver;
class	CScene;
class	CRenderTrav;

// ***************************************************************************
/**
 * A class used to render instances sorted by MeshGeom first, then per material, where possible.
 *	This allow optimisation because less renderState swapping are needed.
 *	WARNING: if you add MeshGeom to 2 different CMeshBlockManager at same times, it won't work, and will
 *	certainly crashes (not checked/assert).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CMeshBlockManager
{
public:

	/// Constructor
	CMeshBlockManager();

	/** Add an instance of a MeshGeom to render. Only CMeshBaseInstance can be added.
	 *	For now, only CMeshGeom and CMeshMRMGeom are known to work.
	 */
	void			addInstance(IMeshGeom *meshGeom, CMeshBaseInstance *inst, float polygonCount);

	/** Flush the manager and effectively render.
	 */
	void			flush(IDriver *drv, CScene *scene, CRenderTrav *renderTrav);


// ************************
private:

	// An instance information.
	struct	CInstanceInfo
	{
		IMeshGeom			*MeshGeom;
		CMeshBaseInstance	*MBI;
		float				PolygonCount;
		// Next instance to render in the list. -1 if end of list.
		sint32				NextInstance;
	};

	/// List of instances. small realloc are performed, since same vector used each frame.
	std::vector<CInstanceInfo>	_Instances;

	/// List of MeshGeom. small realloc are performed, since same vector used each frame.
	std::vector<IMeshGeom*>		_MeshGeoms;

	// render all instance of this meshGeoms, sorting by material, VP etc....
	void			render(IMeshGeom *meshGeom);

	// current flush setup
	CMeshGeomRenderContext		_RenderCtx;
};


} // NL3D


#endif // NL_MESH_BLOCK_MANAGER_H

/* End of mesh_block_manager.h */
