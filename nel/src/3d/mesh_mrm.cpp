/** \file mesh_mrm.cpp
 * <File description>
 *
 * $Id: mesh_mrm.cpp,v 1.4 2001/06/19 16:58:13 berenguier Exp $
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
#include "nel/misc/common.h"


using namespace NLMISC;

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


	// TempYoyo.
	static	float	testTime= 0;
	testTime+= 0.001f;
	float	testAlpha= (1+(float)sin(testTime))/2;


	// Choose what Lod to draw.
	float	alpha= testAlpha;
	clamp(alpha, 0, 1);
	alpha*= _Lods.size()-1;
	sint	numLod= (sint)ceil(alpha);
	float	alphaLod;
	if(numLod==0)
	{
		numLod= 0;
		alphaLod= 0;
	}
	else
	{
		// Lerp beetween lod i-1 and lod i.
		alphaLod= alpha-(numLod-1);
	}


	// Render the choosen Lod.
	CLod	&lod= _Lods[numLod];
	if(lod.RdrPass.size()==0)
		return;

	// Geomorph the choosen Lod (if not the coarser mesh).
	if(numLod>0)
	{
		float	a= alphaLod;
		float	a1= 1 - alphaLod;
		for(uint i=0; i<lod.Geomorphs.size(); i++)
		{
			CMRMWedgeGeom	&geom= lod.Geomorphs[i];
			// TODODO.
			{
				// Vertex.
				CVector		*start=	(CVector*)_VBuffer.getVertexCoordPointer(geom.Start);
				CVector		*end=	(CVector*)_VBuffer.getVertexCoordPointer(geom.End);
				CVector		*dst=	(CVector*)_VBuffer.getVertexCoordPointer(i);
				*dst= *start * a + *end * a1;
			}
			{
				// Normal.
				CVector		*start= (CVector*)_VBuffer.getNormalCoordPointer(geom.Start);
				CVector		*end=	(CVector*)_VBuffer.getNormalCoordPointer(geom.End);
				CVector		*dst=	(CVector*)_VBuffer.getNormalCoordPointer(i);
				*dst= *start * a + *end * a1;
			}
			{
				// Uv.
				CUV		*start= (CUV*)_VBuffer.getTexCoordPointer(geom.Start, 0);
				CUV		*end=	(CUV*)_VBuffer.getTexCoordPointer(geom.End, 0);
				CUV		*dst=	(CUV*)_VBuffer.getTexCoordPointer(i, 0);
				*dst= *start * a + *end * a1;
			}
		}
	}


	// force normalisation of normals..
	bool	bkupNorm= drv->isForceNormalize();
	drv->forceNormalize(true);

	// active VB.
	drv->activeVertexBuffer(_VBuffer);

	// Render all pass.
	for(uint i=0;i<lod.RdrPass.size();i++)
	{
		CRdrPass	&rdrPass= lod.RdrPass[i];
		// Render with the Materials of the MeshInstance.
		drv->render(rdrPass.PBlock, mi->Materials[rdrPass.MaterialId]);
	}


	// bkup force normalisation.
	drv->forceNormalize(bkupNorm);

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
