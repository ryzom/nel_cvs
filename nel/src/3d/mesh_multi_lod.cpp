/** \file mesh_multi_lod.cpp
 * Mesh with several LOD meshes.
 *
 * $Id: mesh_multi_lod.cpp,v 1.6 2001/07/06 12:51:23 corvazier Exp $
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

#include "3d/mesh_multi_lod.h"
#include "3d/mesh_multi_lod_instance.h"
#include "3d/mesh_instance.h"
#include "3d/mesh_mrm.h"
#include "3d/scene.h"
#include "3d/coarse_mesh_manager.h"



namespace NL3D 
{




// ***************************************************************************

void CMeshMultiLod::build(CMeshMultiLodBuild &mbuild)
{
	// Clear the mesh
	clear ();

	// Build the base mesh
	CMeshBase::buildMeshBase (mbuild.BaseMesh);

	// Static flag
	_StaticLod=mbuild.StaticLod;

	// Resize the array
	_MeshVector.resize (mbuild.LodMeshes.size());

	// For each slots
	for (uint slot=0; slot<mbuild.LodMeshes.size(); slot++)
	{
		// Dist max
		_MeshVector[slot].DistMax=mbuild.LodMeshes[slot].DistMax;

		// BlendLength
		_MeshVector[slot].BlendLength=mbuild.LodMeshes[slot].BlendLength;

		// Flags
		_MeshVector[slot].Flags=0;

		// Blend in ?
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::BlendIn)
			_MeshVector[slot].Flags|=BlendIn;

		// Blend out ?
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::BlendOut)
			_MeshVector[slot].Flags|=BlendOut;

		// Coarse mesh ?
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::CoarseMesh)
			_MeshVector[slot].Flags|=CoarseMesh;

		// MeshGeom
		nlassert (mbuild.LodMeshes[slot].MeshGeom);

		// Valid pointer ?
		if (_MeshVector[slot].Flags&CoarseMesh)
		{
			// If it is a coarse mesh, it must be a CMeshGeom.
			if (dynamic_cast<CMeshGeom*>(mbuild.LodMeshes[slot].MeshGeom)==NULL)
			{
				// If it is a coarse mesh, it must be a CMeshGeom.
				_MeshVector[slot].MeshGeom = NULL;
				delete mbuild.LodMeshes[slot].MeshGeom;
			}
			else
				// Ok, no prb
				_MeshVector[slot].MeshGeom = mbuild.LodMeshes[slot].MeshGeom;
		}
		else
			// Ok, no prb
			_MeshVector[slot].MeshGeom = mbuild.LodMeshes[slot].MeshGeom;	
	}

	// Sort the slot by the distance...
	for (int i=mbuild.LodMeshes.size()-2; i>0; i--)
	for (int j=0; j<i; j++)
	{
		// Bad sort ?
		if (_MeshVector[i].DistMax>_MeshVector[i+1].DistMax)
		{
			// Exchange slots
			CMeshSlot tmp=_MeshVector[i];
			_MeshVector[i]=_MeshVector[i+1];
			_MeshVector[i+1]=tmp;
		}
	}

	// Calc start and end polygon count
	for (uint k=0; k<mbuild.LodMeshes.size(); k++)
	{
		// Get end distance
		float endDist=_MeshVector[k].DistMax;

		// Get end poly count
		if (k==mbuild.LodMeshes.size()-1)
			_MeshVector[k].EndPolygonCount=_MeshVector[k].MeshGeom->getNumTriangles (endDist);
		else 
			_MeshVector[k].EndPolygonCount=_MeshVector[k+1].MeshGeom->getNumTriangles (endDist);

		// Get start distance
		float startDist;
		if (k==0)
			startDist=0;
		else 
			startDist=_MeshVector[k-1].DistMax;

		// Get start poly count
		float startPolyCount;
		startPolyCount=_MeshVector[0].MeshGeom->getNumTriangles (startDist);

		// Calc A
		_MeshVector[k].A=(_MeshVector[k].EndPolygonCount-startPolyCount)/(endDist-startDist);

		// Calc A
		_MeshVector[k].B=_MeshVector[k].EndPolygonCount-_MeshVector[k].A*endDist;
	}
}

// ***************************************************************************

CTransformShape	*CMeshMultiLod::createInstance(CScene &scene)
{
	// Create a CMeshInstance, an instance of a multi lod mesh.
	CMeshMultiLodInstance *mi=(CMeshMultiLodInstance*)scene.createModel(NL3D::MeshMultiLodInstanceId);
	mi->Shape= this;
	mi->Scene= &scene;
	mi->_LastLodMatrixDate=0;

	// instanciate the material part of the Mesh, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi);

	return mi;
}

// ***************************************************************************

bool CMeshMultiLod::clip(const std::vector<CPlane>	&pyramid)
{
	// Look for the biggest mesh
	uint meshCount=_MeshVector.size();
	for (uint i=0; i<meshCount; i++)
	{
		// Ref on slot
		CMeshSlot &slot=_MeshVector[i];

		// Is mesh present ?
		if (slot.MeshGeom)
		{
			// Clip this mesh
			return slot.MeshGeom->clip (pyramid);
		}
	}
	return true;
}

// ***************************************************************************

void CMeshMultiLod::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	// Render good meshes
	nlassert (dynamic_cast<CMeshMultiLodInstance*>(trans));
	CMeshMultiLodInstance *instance=static_cast<CMeshMultiLodInstance*>(trans);

	// Look in the table for good distances..
	uint meshCount=_MeshVector.size();

	// At least on mesh
	if (meshCount>0)
	{
		uint i=0;

		// Get the wanted number of polygons
		float polygonCount=trans->getNumTrianglesAfterLoadBalancing ();

		if (meshCount>1)
		{
			// Look for good i
			while ( polygonCount < _MeshVector[i].EndPolygonCount )
			{
				i++;
				if (i==meshCount-1)
					break;
			}
		}

		// The slot
		CMeshSlot	&slot=_MeshVector[i];

		// Get the distance with polygon count
		float distance=(polygonCount-slot.B)/slot.A;

		// Get the final polygon count
		float goodPolyCount = getNumTriangles (distance);

		// Check count interval
/*		nlassert (slot.BeginPolygonCount>=goodPolyCount);
		nlassert (goodPolyCount>=slot.EndPolygonCount);
		nlassert (slot.BeginPolygonCount>=slot.StartBlendPolygonCount);
		nlassert (slot.StartBlendPolygonCount>=slot.EndPolygonCount);*/

		// Slot in use...
		uint slot0=i;
		uint slot1=0xffffffff;

		// Blend with other ?
/*		float blendFactor = (slot.EndPolygonCount-goodPolyCount) / (slot.EndPolygonCount-slot.StartBlendPolygonCount);
		if ( ( goodPolyCount <= slot.StartBlendPolygonCount ) && ( goodPolyCount >= slot.EndPolygonCount ) )
		{
			// Render next mesh
			if ( (i<meshCount-1) && _MeshVector[i+1].MeshGeom )
			{
				// The next slot
				CMeshSlot	&nextSlot=_MeshVector[i+1];

				if (nextSlot.Flags&BlendIn)
				{
					// Render the geom mesh with alpha blending with nextSlot.BeginPolygonCount
					render (i+1, drv, instance, nextSlot.BeginPolygonCount, 1.f-blendFactor, _StaticLod);
					slot1=i+1;
				}
			}

			// Render this mesh
			if (slot.MeshGeom)
			{
				if (slot.Flags&BlendOut)
					// Render the geom mesh with alpha blending with goodPolyCount
					render (i, drv, instance, goodPolyCount, blendFactor, _StaticLod);
				else
					// Render the geom mesh without alpha blending with goodPolyCount
					render (i, drv, instance, goodPolyCount, 1, _StaticLod);
			}
		}
		else*/
		{
			// Render without blend with goodPolyCount
			render (i, drv, instance, goodPolyCount, 1, _StaticLod);
		}

		// *** Remove unused coarse meshes
		for (uint j=0; j<meshCount; j++)
		{
			// Not a slot used and coarse mesh loaded ?
			if ( (j!=slot0)&&(j!=slot1)&&( (_MeshVector[j].Flags&CoarseMeshLoaded) != 0) )
			{
				// Yes, remove it..

				// Static or dynamic coarse mesh ?
				CCoarseMeshManager *manager;
				if (_StaticLod)
				{
					// Get the static coarse mesh manager
					manager=instance->Scene->getStaticCoarseMeshManager();
				}
				else
				{
					// Get the dynamic coarse mesh manager
					manager=instance->Scene->getDynamicCoarseMeshManager();
				}

				// Manager must exist beacuse a mesh has been loaded...
				nlassert (manager);

				// Remove the lod
				manager->removeMesh (_MeshVector[j].CoarseMeshId);

				// Clear the flag
				_MeshVector[j].Flags&=~CoarseMeshLoaded;
			}
		}
	}
}

// ***************************************************************************

void CMeshMultiLod::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Serial a version number
	sint ver=f.serialVersion (0);

	// serial Materials infos contained in CMeshBase.
	CMeshBase::serialMeshBase(f);

	// Serial the values
	f.serialCont (_MeshVector);
}

// ***************************************************************************

float CMeshMultiLod::getNumTriangles (float distance)
{
	// Look in the table for good distances..
	uint meshCount=_MeshVector.size();

	// At least on mesh
	if (meshCount>0)
	{
		uint i=0;

		// Look for good i
		while ( _MeshVector[i].DistMax<distance )
		{
			if (i==meshCount-1)
				// Abort if last one
				break;
			i++;
		}

		// Ref on slot
		CMeshSlot &slot=_MeshVector[i];

		// Is mesh present ?
		if (slot.MeshGeom)
		{
			// Get the polygon count with the distance
			float polyCount=slot.A * distance + slot.B;

			/*// Get the perfect polygon count in this slot for the asked distance
			float goodPolyCount=slot.MeshGeom->getNumTriangles (distance);

			// Get the next slot perfect polygon count
			float realEndPolyCount;

			// Last slot ?
			if ( (i<meshCount-1) && _MeshVector[i+1].MeshGeom )
				// Take end number polygon count in the next slot
				realEndPolyCount=_MeshVector[i+1].MeshGeom->getNumTriangles (slot.DistMax);
			else
				// Take end number polygon count in the this slot
				realEndPolyCount=slot.EndPolygonCount;

			// Return blended polygon count to have a continous decreasing function
			return (goodPolyCount-slot.BeginPolygonCount) * (realEndPolyCount-slot.BeginPolygonCount) / 
				(slot.EndPolygonCount-slot.BeginPolygonCount) + slot.BeginPolygonCount;*/
			return polyCount;
		}
	}

	return 0;
}

// ***************************************************************************

void CMeshMultiLod::clear ()
{
	_MeshVector.clear ();
}

// ***************************************************************************

void CMeshMultiLod::CMeshSlot::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Check version
	sint ver=f.serialVersion (0);

	f.serialPolyPtr (MeshGeom);
	f.serial (DistMax);
	f.serial (BlendLength);
	f.serial (Flags);
}

// ***************************************************************************

CMeshMultiLod::CMeshSlot::CMeshSlot ()
{
	MeshGeom=NULL;
}

// ***************************************************************************

CMeshMultiLod::CMeshSlot::~CMeshSlot ()
{
	if (MeshGeom)
		delete MeshGeom;
}

// ***************************************************************************

void CMeshMultiLod::render (uint slot, IDriver *drv, CMeshMultiLodInstance *trans, float numPoylgons, float alpha, bool staticLod)
{
	// Ref
	CMeshSlot &slotRef=_MeshVector[slot];

	// Coarse mesh ?
	if (slotRef.Flags&CoarseMesh)
	{
		// Static or dynamic coarse mesh ?
		CCoarseMeshManager *manager;
		if (staticLod)
		{
			// Get the static coarse mesh manager
			manager=trans->Scene->getStaticCoarseMeshManager();
		}
		else
		{
			// Get the dynamic coarse mesh manager
			manager=trans->Scene->getDynamicCoarseMeshManager();
		}

		// Manager  exist?
		if (manager)
		{
			// Get a pointer on the geom mesh
			nlassert (dynamic_cast<CMeshGeom*>(slotRef.MeshGeom));
			CMeshGeom *meshGeom=(CMeshGeom*)slotRef.MeshGeom;

			// Added in the manager ?
			if ( (slotRef.Flags&CoarseMeshLoaded) == 0)
			{
				// Add to the manager
				slotRef.CoarseMeshId=manager->addMesh (*meshGeom);
					
				// Added ?
				if (slotRef.CoarseMeshId!=CCoarseMeshManager::CantAddCoarseMesh)
					// Flag it
					slotRef.Flags|=CoarseMeshLoaded;

				// Dirt the matrix
				trans->_LastLodMatrixDate=0;
			}

			// Finally loaded ?
			if (slotRef.Flags&CoarseMeshLoaded)
			{
				// Matrix has changed ?
				if ( trans->ITransformable::compareMatrixDate (trans->_LastLodMatrixDate) )
				{
					// Get date
					trans->_LastLodMatrixDate = trans->ITransformable::getMatrixDate();

					// Set matrix
					manager->setMatrixMesh ( slotRef.CoarseMeshId, *meshGeom, trans->getMatrix() );
				}
			}
		}
	}
	else
	{
		// Here
		if (slotRef.MeshGeom)
		{
			/// \todo hulud: manage alpha blending and polygon count here..
			slotRef.MeshGeom->render (drv, trans, true);
		}
	}
}

// ***************************************************************************

} // NL3D
