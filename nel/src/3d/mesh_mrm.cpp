/** \file mesh_mrm.cpp
 * <File description>
 *
 * $Id: mesh_mrm.cpp,v 1.3 2001/06/19 10:22:33 berenguier Exp $
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

#include "3d/mesh_mrm.h"
#include "3d/mrm_builder.h"
#include "3d/mrm_parameters.h"
#include "3d/mesh_mrm_instance.h"
#include "3d/scene.h"


namespace NL3D 
{


// ***************************************************************************
static	NLMISC::CAABBoxExt	makeBBox(const std::vector<CVector>	&Vertices)
{
	NLMISC::CAABBox		ret;
	nlassert(Vertices.size());
	ret.setCenter(Vertices[0]);
	for(sint i=0;i<(sint)Vertices.size();i++)
	{
		ret.extend(Vertices[i]);
	}

	return ret;
}


// ***************************************************************************
CMeshMRM::CMeshMRM()
{
}


// ***************************************************************************
void			CMeshMRM::build(CMesh::CMeshBuild &m)
{
	/// First, copy MeshBase info: materials ....
	//======================
	CMeshBase::buildMeshBase(m);


	// Empty geometry?
	if(m.Vertices.size()==0 || m.Faces.size()==0)
	{
		_VBuffer.setNumVertices(0);
		_VBuffer.reserve(0);
		_Lods.clear();
		_BBox.setCenter(CVector::Null);
		_BBox.setSize(CVector::Null);
		return;
	}
	nlassert(m.Materials.size()>0);


	/// 0. First, make bbox.
	//======================
	// NB: this is equivalent as building BBox from MRM VBuffer, because CMRMBuilder create new vertices 
	// which are just interpolation of original vertices.
	_BBox= makeBBox(m.Vertices);


	/// 1. Launch the MRM build process.
	//================================================
	CMRMBuilder			mrmBuilder;
	CMeshBuildMRM		meshBuildMRM;

	mrmBuilder.compileMRM(m, CMRMParameters(), meshBuildMRM);

	// Then just copy result!
	_VBuffer= meshBuildMRM.VBuffer;
	_Lods= meshBuildMRM.Lods;


}


// ***************************************************************************
CTransformShape		*CMeshMRM::createInstance(CScene &scene)
{
	// Create a CMeshMRMInstance, an instance of a mesh.
	//===============================================
	CMeshMRMInstance		*mi= (CMeshMRMInstance*)scene.createModel(NL3D::MeshMRMInstanceId);
	mi->Shape= this;


	// instanciate the material part of the MeshMRM, ie the CMeshBase.
	CMeshBase::instanciateMeshBase(mi);


	return mi;
}


// ***************************************************************************
bool	CMeshMRM::clip(const std::vector<CPlane>	&pyramid)
{
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// We are sure that pyramid has normalized plane normals.
		if(!_BBox.clipBack(pyramid[i]))
			return false;
	}

	return true;
}


// ***************************************************************************
void	CMeshMRM::render(IDriver *drv, CTransformShape *trans)
{
	nlassert(drv);
	// get the meshMRM instance.
	nlassert(dynamic_cast<CMeshMRMInstance*>(trans));
	CMeshMRMInstance	*mi= (CMeshMRMInstance*)trans;



	CLod	&lod= _Lods[0];
	if(lod.RdrPass.size()==0)
		return;

	// active VB.
	drv->activeVertexBuffer(_VBuffer);

	// Render all pass.
	for(uint i=0;i<lod.RdrPass.size();i++)
	{
		CRdrPass	&rdrPass= lod.RdrPass[i];
		// Render with the Materials of the MeshInstance.
		drv->render(rdrPass.PBlock, mi->Materials[rdrPass.MaterialId]);
	}

}


// ***************************************************************************
void	CMeshMRM::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/*
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(0);

	// serial Materials infos contained in CMeshBase.
	CMeshBase::serialMeshBase(f);

	// serial geometry.
	f.serial(_VBuffer);
	f.serialCont(_Lods);
	f.serial(_BBox);

}


} // NL3D
