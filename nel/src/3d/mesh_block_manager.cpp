/** \file mesh_block_manager.cpp
 * <File description>
 *
 * $Id: mesh_block_manager.cpp,v 1.1 2002/06/19 08:42:10 berenguier Exp $
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

#include "std3d.h"

#include "3d/mesh_block_manager.h"


namespace NL3D 
{


// ***************************************************************************
CMeshBlockManager::CMeshBlockManager()
{
	_RenderCtx.Driver= NULL;
	_RenderCtx.Scene= NULL;
	_RenderCtx.RenderTrav= NULL;

	// some reserve, avoiding first reallocation.
	_Instances.reserve(500);
	_MeshGeoms.reserve(200);
}

// ***************************************************************************
void			CMeshBlockManager::addInstance(IMeshGeom *meshGeom, CMeshBaseInstance *inst, float polygonCount)
{
	// If the mesh geom is not added to this manager, add it.
	if(meshGeom->_RootInstanceId==-1)
	{
		_MeshGeoms.push_back(meshGeom);
	}

	// setup the instance.
	CInstanceInfo		instInfo;
	instInfo.MeshGeom= meshGeom;
	instInfo.MBI= inst;
	instInfo.PolygonCount= polygonCount;

	// link to the head of the list.
	instInfo.NextInstance= meshGeom->_RootInstanceId;
	meshGeom->_RootInstanceId= _Instances.size();

	// add this instance
	_Instances.push_back(instInfo);

}

// ***************************************************************************
void			CMeshBlockManager::flush(IDriver *drv, CScene *scene, CRenderTrav *renderTrav)
{
	uint	i;

	// setup the manager
	nlassert(drv && scene && renderTrav);
	_RenderCtx.Driver= drv;
	_RenderCtx.Scene= scene;
	_RenderCtx.RenderTrav= renderTrav;

	// render
	//==========

	// Always sort by MeshGeom.
	for(i=0; i<_MeshGeoms.size();i++)
	{
		// render the meshGeom and his instances
		render(_MeshGeoms[i]);
	}


	// reset.
	//==========

	// Parse all MehsGeoms, and flag them as Not Added to me
	for(i=0; i<_MeshGeoms.size();i++)
	{
		_MeshGeoms[i]->_RootInstanceId= -1;
	}

	// clear arrays
	_Instances.clear();
	_MeshGeoms.clear();
}


// ***************************************************************************
void			CMeshBlockManager::render(IMeshGeom *meshGeom)
{
	// Start for this mesh.
	meshGeom->beginMesh(_RenderCtx);

	// number of renderPasses for this mesh.
	uint	numRdrPass= meshGeom->getNumRdrPasses();

	// sort per material first?
	if( meshGeom->sortPerMaterial() )
	{
		// for all material.
		for(uint rdrPass=0;rdrPass<numRdrPass;rdrPass++)
		{
			// for all instance.
			sint32	instId= meshGeom->_RootInstanceId;
			while( instId!=-1 )
			{
				CInstanceInfo		&instInfo= _Instances[instId];

				// activate this instance
				meshGeom->activeInstance(_RenderCtx, instInfo.MBI, instInfo.PolygonCount);

				// render the pass.
				meshGeom->renderPass(_RenderCtx, instInfo.MBI, instInfo.PolygonCount, rdrPass);

				// next instance
				instId= instInfo.NextInstance;
			}
		}
	}
	// else sort per instance first
	else
	{
		// for all instance.
		sint32	instId= meshGeom->_RootInstanceId;
		while( instId!=-1 )
		{
			CInstanceInfo		&instInfo= _Instances[instId];

			// activate this instance
			meshGeom->activeInstance(_RenderCtx, instInfo.MBI, instInfo.PolygonCount);

			// for all material.
			for(uint rdrPass=0;rdrPass<numRdrPass;rdrPass++)
			{
				// render the pass.
				meshGeom->renderPass(_RenderCtx, instInfo.MBI, instInfo.PolygonCount, rdrPass);
			}

			// next instance
			instId= instInfo.NextInstance;
		}
	}

	// End for this mesh.
	meshGeom->endMesh(_RenderCtx);
}


} // NL3D
