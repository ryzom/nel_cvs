/** \file mesh_multi_lod.cpp
 * Mesh with several LOD meshes.
 *
 * $Id: mesh_multi_lod.cpp,v 1.16 2002/02/28 12:59:50 besson Exp $
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

#include "3d/mesh_multi_lod.h"
#include "3d/mesh_multi_lod_instance.h"
#include "3d/mesh_instance.h"
#include "3d/mesh_mrm.h"
#include "3d/scene.h"
#include "3d/coarse_mesh_manager.h"
#include "3d/skeleton_model.h"

#include "nel/misc/debug.h"

using namespace NLMISC;

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

	// Number of coarse meshes
	uint coarse=0;

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
			_MeshVector[slot].Flags|=CMeshSlot::BlendIn;

		// Blend out ?
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::BlendOut)
			_MeshVector[slot].Flags|=CMeshSlot::BlendOut;

		// Coarse mesh ?
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::CoarseMesh)
		{
			// Warning: no more than 2 coarse meshes in a lod mesh!
			nlassert (coarse<=1);

			// Flag
			_MeshVector[slot].Flags|=CMeshSlot::CoarseMesh;

			// Flag coarse ID
			if (coarse==1)
				_MeshVector[slot].Flags|=CMeshSlot::CoarseMeshId;

			// One more
			coarse++;
		}

		// Is opaque
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::IsOpaque)
			_MeshVector[slot].Flags|=CMeshSlot::IsOpaque;

		// Is transparent
		if (mbuild.LodMeshes[slot].Flags & CMeshMultiLodBuild::CBuildSlot::IsTransparent)
			_MeshVector[slot].Flags|=CMeshSlot::IsTransparent;

		// MeshGeom
		nlassert (mbuild.LodMeshes[slot].MeshGeom);

		// Valid pointer ?
		if (_MeshVector[slot].Flags&CMeshSlot::CoarseMesh)
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
	for (int i=mbuild.LodMeshes.size()-1; i>0; i--)
	for (int j=0; j<i; j++)
	{
		// Bad sort ?
		if (_MeshVector[j].DistMax>_MeshVector[j+1].DistMax)
		{
			// Exchange slots
			CMeshSlot tmp=_MeshVector[j];
			_MeshVector[j]=_MeshVector[j+1];
			_MeshVector[j+1]=tmp;
			tmp.MeshGeom=NULL;
		}
	}

	// Calc start and end polygon count
	for (uint k=0; k<mbuild.LodMeshes.size(); k++)
	{
		// Get start distance
		float startDist;
		if (k==0)
			startDist=0;
		else 
			startDist=_MeshVector[k-1].DistMax;

		// Get start poly count
		float startPolyCount;
		startPolyCount=_MeshVector[k].MeshGeom->getNumTriangles (startDist);

		// Get end distance
		float endDist=_MeshVector[k].DistMax;

		// Get end poly count
		if (k==mbuild.LodMeshes.size()-1)
		{
			_MeshVector[k].EndPolygonCount=_MeshVector[k].MeshGeom->getNumTriangles (endDist);
			if (startPolyCount==_MeshVector[k].EndPolygonCount)
				_MeshVector[k].EndPolygonCount=startPolyCount/2;
		}
		else 
			_MeshVector[k].EndPolygonCount=_MeshVector[k+1].MeshGeom->getNumTriangles (endDist);

		// Calc A
		if (endDist==startDist)
			_MeshVector[k].A=0;
		else
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
	mi->_LastLodMatrixDate=0;

	// instanciate the material part of the Mesh, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi, &scene);


	// For all lods, do some instance init for MeshGeom
	for(uint i=0; i<_MeshVector.size(); i++)
	{
		if(_MeshVector[i].MeshGeom)
			_MeshVector[i].MeshGeom->initInstance(mi);
	}


	return mi;
}

// ***************************************************************************

bool CMeshMultiLod::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
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
			return slot.MeshGeom->clip (pyramid, worldMatrix);
		}
	}
	return true;
}

// ***************************************************************************

void CMeshMultiLod::render(IDriver *drv, CTransformShape *trans, bool passOpaque)
{
	// Render good meshes
	CMeshMultiLodInstance *instance=safe_cast<CMeshMultiLodInstance*>(trans);

	// Second lod ?
	if ( (instance->Lod1!=0xffffffff) && (passOpaque==false) )
	{
		// Render second lod in blend mode
		render (instance->Lod1, drv, instance, instance->PolygonCountLod1, 1.f-instance->BlendFactor, _StaticLod, true);
		render (instance->Lod1, drv, instance, instance->PolygonCountLod1, 1.f-instance->BlendFactor, _StaticLod, false);
	}

	// Have an opaque pass ?
	if ( (instance->Flags&CMeshMultiLodInstance::Lod0Blend) == 0)
	{
		// Only render the normal way the first lod
		render (instance->Lod0, drv, instance, instance->PolygonCountLod0, 1, _StaticLod, passOpaque);
	}
	else
	{
		// Should not be in opaque
		nlassert (passOpaque==false);

		// Render first lod in blend mode
		render (instance->Lod0, drv, instance, instance->PolygonCountLod0, instance->BlendFactor, _StaticLod, true);

		// Then render transparent 
		render (instance->Lod0, drv, instance, instance->PolygonCountLod0, instance->BlendFactor, _StaticLod, false);
	}

	// *** Remove unused coarse meshes
	uint meshCount=_MeshVector.size();
	for (uint j=0; j<meshCount; j++)
	{
		// Not a slot used and coarse mesh loaded ?
		if ( (j!=instance->Lod0)&&(j!=instance->Lod1)&&(_MeshVector[j].Flags&CMeshSlot::CoarseMesh) )
		{
			// Get coarse id
			uint coarseId=(_MeshVector[j].Flags&CMeshSlot::CoarseMeshId)?1:0;
			uint flag=CMeshMultiLodInstance::Coarse0Loaded<<coarseId;
			
			// Coarse mesh loaded ?
			if ( instance->Flags&flag )
			{
				// Yes, remove it..

				// Static or dynamic coarse mesh ?
				CCoarseMeshManager *manager;
				if (_StaticLod)
				{
					// Get the static coarse mesh manager
					manager=instance->getScene()->getStaticCoarseMeshManager();
				}
				else
				{
					// Get the dynamic coarse mesh manager
					manager=instance->getScene()->getDynamicCoarseMeshManager();
				}

				// Manager must exist beacuse a mesh has been loaded...
				if (manager)
				{
					// Remove the lod
					manager->removeMesh (instance->CoarseMeshId[coarseId]);

					// Clear the flag
					instance->Flags&=~flag;
				}
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

	// Static lod flag
	f.serial (_StaticLod);

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

void CMeshMultiLod::getAABBox(NLMISC::CAABBox &bbox) const
{
	// Get count
	uint count=_MeshVector.size();
	for (uint slot=0; slot<count; slot++)
	{
		// Shape ?
		if (_MeshVector[slot].MeshGeom)
		{
			// Get the bounding box
			bbox=_MeshVector[slot].MeshGeom->getBoundingBox().getAABBox();

			// ok
			break;
		}
	}
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
	f.serial (A);
	f.serial (B);
	f.serial (DistMax);
	f.serial (EndPolygonCount);
	f.serial (BlendLength);
	f.serial (Flags);

	if (f.isReading())
	{
	}
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

void CMeshMultiLod::render (uint slot, IDriver *drv, CMeshMultiLodInstance *trans, float numPoylgons, float alpha, bool staticLod, bool passOpaque)
{
	// Ref
	CMeshSlot &slotRef=_MeshVector[slot];

	// Coarse mesh ?
	if (slotRef.Flags&CMeshSlot::CoarseMesh)
	{
		// Mask
		uint coarseId=(slotRef.Flags&CMeshSlot::CoarseMeshId)?1:0;
		uint maskFlag = CMeshMultiLodInstance::Coarse0Loaded<<coarseId;

		// Only in opaque pass
		if (passOpaque)
		{
			// Static or dynamic coarse mesh ?
			CCoarseMeshManager *manager;
			if (staticLod)
			{
				// Get the static coarse mesh manager
				manager=trans->getScene()->getStaticCoarseMeshManager();
			}
			else
			{
				// Get the dynamic coarse mesh manager
				manager=trans->getScene()->getDynamicCoarseMeshManager();
			}

			// Manager  exist?
			if (manager)
			{
				// Get a pointer on the geom mesh
				nlassert (dynamic_cast<CMeshGeom*>(slotRef.MeshGeom));
				CMeshGeom *meshGeom=(CMeshGeom*)slotRef.MeshGeom;

				// Added in the manager ?
				
				if ( (trans->Flags&maskFlag) == 0)
				{
					// Add to the manager
					trans->CoarseMeshId[coarseId]=manager->addMesh (*meshGeom);
						
					// Added ?
					if (trans->CoarseMeshId[coarseId]!=CCoarseMeshManager::CantAddCoarseMesh)
						// Flag it
						trans->Flags|=maskFlag;

					// Dirt the matrix
					trans->_LastLodMatrixDate=0;
				}

				// Finally loaded ?
				if (trans->Flags&maskFlag)
				{
					// Matrix has changed ?
					if ( trans->ITransformable::compareMatrixDate (trans->_LastLodMatrixDate) )
					{
						// Get date
						trans->_LastLodMatrixDate = trans->ITransformable::getMatrixDate();

						// Set matrix
						manager->setMatrixMesh ( trans->CoarseMeshId[coarseId], *meshGeom, trans->getMatrix() );
					}
				}
			}
		}
	}
	else
	{
		// Here
		if (slotRef.MeshGeom)
		{
			/// Render the geom mesh
			slotRef.MeshGeom->render (drv, trans, passOpaque, numPoylgons, alpha);
		}
	}
}

// ***************************************************************************
float CMeshMultiLod::getDistMax () const
{
	// Last element
	std::vector<CMeshSlot>::const_iterator ite=_MeshVector.end();
	ite--;
	if (ite!=_MeshVector.end())
		return ite->DistMax;
	else
		return -1;
}

// ***************************************************************************
const IMeshGeom& CMeshMultiLod::getMeshGeom (uint slot) const
{
	// Checks
	nlassert (slot<getNumSlotMesh ());

	return *_MeshVector[slot].MeshGeom;
}

} // NL3D
