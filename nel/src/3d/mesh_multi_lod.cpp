/** \file mesh_multi_lod.cpp
 * Mesh with several LOD meshes.
 *
 * $Id: mesh_multi_lod.cpp,v 1.2 2001/07/03 08:33:39 corvazier Exp $
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


namespace NL3D 
{

// ***************************************************************************

void CMeshMultiLod::build(CMeshMultiLodBuild &mbuild)
{
	// Clear the mesh
	clear ();

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
		_MeshVector[slot].MeshGeom = mbuild.LodMeshes[slot].MeshGeom;
		
	}

	// Sort the slot by the distance...
	for (uint i=mbuild.LodMeshes.size()-1; i>0; i++)
	for (uint j=0; j<i; j++)
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
		// Get polygon count at the begining of the slot interval
		if (k==0)
			_MeshVector[k].BeginPolygonCount=_MeshVector[k].MeshGeom->getNumTriangles (0);
		else 
			_MeshVector[k].BeginPolygonCount=_MeshVector[k].MeshGeom->getNumTriangles (_MeshVector[k-1].DistMax);

		// Get polygon count at the end of the slot interval
		_MeshVector[k].EndPolygonCount=_MeshVector[k].MeshGeom->getNumTriangles (_MeshVector[k].DistMax);
	}
}

// ***************************************************************************

CTransformShape	*CMeshMultiLod::createInstance(CScene &scene)
{
	// Create a CMeshInstance, an instance of a multi lod mesh.
	CMeshMultiLodInstance *mi=(CMeshMultiLodInstance*)scene.createModel(NL3D::MeshMultiLodInstanceId);
	mi->Shape= this;

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

void CMeshMultiLod::render(IDriver *drv, CTransformShape *trans)
{
	// Render good meshes

	// Look in the table for good distances..
	uint meshCount=_MeshVector.size();

	// At least on mesh
	if (meshCount>0)
	{
		uint i=0;

		/// \toto hulud: get the wanted number of polygons
		float polygonCount=0;

		if (meshCount>1)
		{
			// Look for good i
			while ( _MeshVector[i+1].BeginPolygonCount < polygonCount )
			{
				i++;
				if (i==meshCount-1)
					break;
			}
		}

		// The slot
		CMeshSlot	&slot=_MeshVector[i];

		// Get the next slot perfect polygon count
		float realEndPolyCount;

		// Last slot ?
		if ( (i<meshCount-1) && _MeshVector[i+1].MeshGeom )
		{
			// Take end number polygon count in the next slot
			realEndPolyCount = _MeshVector[i+1].MeshGeom->getNumTriangles (slot.DistMax);
		}
		else
		{
			// Take end number polygon count in the this slot
			realEndPolyCount = slot.EndPolygonCount;
		}

		// Final polygon count
		float goodPolyCount = (polygonCount-slot.BeginPolygonCount) * (slot.EndPolygonCount-slot.BeginPolygonCount) / 
			(realEndPolyCount-slot.BeginPolygonCount) + slot.BeginPolygonCount;

		// Check count interval
		nlassert (slot.BeginPolygonCount>=goodPolyCount);
		nlassert (goodPolyCount>=slot.EndPolygonCount);
		nlassert (slot.BeginPolygonCount>=slot.StartBlendPolygonCount);
		nlassert (slot.StartBlendPolygonCount>=slot.EndPolygonCount);
		
		// Blend with other ?
		float blendFactor = (slot.EndPolygonCount-goodPolyCount) / (slot.EndPolygonCount-slot.StartBlendPolygonCount);
		if ( ( goodPolyCount <= slot.StartBlendPolygonCount ) && ( goodPolyCount >= slot.EndPolygonCount ) )
		{
			// Render next mesh
			if ( (i<meshCount-1) && _MeshVector[i+1].MeshGeom )
			{
				// The next slot
				CMeshSlot	&nextSlot=_MeshVector[i+1];

				if (nextSlot.Flags&BlendIn)
					// Render the geom mesh with alpha blending with nextSlot.BeginPolygonCount
					render (i+1, drv, trans, nextSlot.BeginPolygonCount, 1.f-blendFactor);
			}

			// Render this mesh
			if (slot.MeshGeom)
			{
				if (slot.Flags&BlendOut)
					// Render the geom mesh with alpha blending with goodPolyCount
					render (i, drv, trans, goodPolyCount, blendFactor);
				else
					// Render the geom mesh without alpha blending with goodPolyCount
					render (i, drv, trans, goodPolyCount, 1);
			}
		}
		else
		{
			// Render without blend with goodPolyCount
			render (i, drv, trans, goodPolyCount, 1);
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
			// Get the perfect polygon count in this slot for the asked distance
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
				(slot.EndPolygonCount-slot.BeginPolygonCount) + slot.BeginPolygonCount;
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

CMeshMultiLod::CMeshSlot::~CMeshSlot ()
{
	if (MeshGeom)
		delete MeshGeom;
}

// ***************************************************************************

void CMeshMultiLod::render (uint slot, IDriver *drv, CTransformShape *trans, float numPoylgons, float alpha)
{
	// Ref
	CMeshSlot &slotRef=_MeshVector[slot];

	// Coarse mesh ?
	if (slotRef.Flags&CoarseMesh)
	{
		/// \toto hulud: manage coarse meshes here..
		if ( (slotRef.Flags&CoarseMeshLoaded) == 0)
		{
			// Add in coarse mesh manager
		}
		// Set alpha
		// Set matrix
	}
	else
	{
		// Here
		if (slotRef.MeshGeom)
		{
			/// \toto hulud: manage alpha blending and pôlygon count here..
			slotRef.MeshGeom->render (drv, trans);
		}
	}
}

// ***************************************************************************

} // NL3D
