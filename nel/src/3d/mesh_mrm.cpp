/** \file mesh_mrm.cpp
 * <File description>
 *
 * $Id: mesh_mrm.cpp,v 1.5 2001/06/20 09:36:42 berenguier Exp $
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
void	CMeshMRM::applyGeomorph(std::vector<CMRMWedgeGeom>  &geoms, float alphaLod)
{
	// no geomorphs? quit.
	if(geoms.size()==0)
		return;

	uint		i;
	clamp(alphaLod, 0.f, 1.f);
	float		a= alphaLod;
	float		a1= 1 - alphaLod;
	uint		ua= (uint)(a*256);
	clamp(ua, (uint)0, (uint)256);
	uint		ua1= 256 - ua;


	// info from VBuffer.
	uint8		*vertexPtr= (uint8*)_VBuffer.getVertexCoordPointer();
	uint		flags= _VBuffer.getVertexFormat();
	sint32		vertexSize= _VBuffer.getVertexSize();
	// because of the unrolled code for 4 first UV, must assert this.
	nlassert(IDRV_VF_MAXSTAGES>=4);
	// must have XYZ.
	nlassert(flags & IDRV_VF_XYZ);


	// if an offset is 0, it means that the component is not in the VBuffer.
	sint32		normalOff;
	sint32		colorOff;
	sint32		specularOff;
	sint32		uvOff[IDRV_VF_MAXSTAGES];


	// Compute offset of each component of the VB.
	if(flags & IDRV_VF_NORMAL)
		normalOff= _VBuffer.getNormalOff();
	else
		normalOff= 0;
	if(flags & IDRV_VF_COLOR)
		colorOff= _VBuffer.getColorOff();
	else
		colorOff= 0;
	if(flags & IDRV_VF_SPECULAR)
		specularOff= _VBuffer.getSpecularOff();
	else
		specularOff= 0;
	for(i= 0; i<IDRV_VF_MAXSTAGES;i++)
	{
		if(flags & IDRV_VF_UV[i])
			uvOff[i]= _VBuffer.getTexCoordOff(i);
		else
			uvOff[i]= 0;
	}


	// For all geomorphs.
	uint			nGeoms= geoms.size();
	CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
	uint8			*destPtr= vertexPtr;
	/* NB: optimisation: lot of "if" in this Loop, but because of BTB, they always cost nothing (prediction is good).
	   NB: this also is why we unroll the 4 1st Uv. The other (if any), are done in the other loop.
	*/
	for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
	{
		uint8			*startPtr=	vertexPtr + ptrGeom->Start*vertexSize;
		uint8			*endPtr=	vertexPtr + ptrGeom->End*vertexSize;

		// Vertex.
		{
			CVector		*start=	(CVector*)startPtr;
			CVector		*end=	(CVector*)endPtr;
			CVector		*dst=	(CVector*)destPtr;
			*dst= *start * a + *end * a1;
		}

		// Normal.
		if(normalOff)
		{
			CVector		*start= (CVector*)(startPtr + normalOff);
			CVector		*end=	(CVector*)(endPtr   + normalOff);
			CVector		*dst=	(CVector*)(destPtr  + normalOff);
			*dst= *start * a + *end * a1;
		}
		// color.
		if(colorOff)
		{
			CRGBA		*start= (CRGBA*)(startPtr + colorOff);
			CRGBA		*end=	(CRGBA*)(endPtr   + colorOff);
			CRGBA		*dst=	(CRGBA*)(destPtr  + colorOff);
			dst->blendFromui(*start, *end,  ua1);
		}
		// specular.
		if(specularOff)
		{
			CRGBA		*start= (CRGBA*)(startPtr + specularOff);
			CRGBA		*end=	(CRGBA*)(endPtr   + specularOff);
			CRGBA		*dst=	(CRGBA*)(destPtr  + specularOff);
			dst->blendFromui(*start, *end,  ua1);
		}

		// Uvs.
		// uv[0].
		if(uvOff[0])
		{
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[0]);
			CUV			*end=	(CUV*)(endPtr   + uvOff[0]);
			CUV			*dst=	(CUV*)(destPtr  + uvOff[0]);
			*dst= *start * a + *end * a1;
		}
		// uv[1].
		if(uvOff[1])
		{
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[1]);
			CUV			*end=	(CUV*)(endPtr   + uvOff[1]);
			CUV			*dst=	(CUV*)(destPtr  + uvOff[1]);
			*dst= *start * a + *end * a1;
		}
		// uv[2].
		if(uvOff[2])
		{
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[2]);
			CUV			*end=	(CUV*)(endPtr   + uvOff[2]);
			CUV			*dst=	(CUV*)(destPtr  + uvOff[2]);
			*dst= *start * a + *end * a1;
		}
		// uv[3].
		if(uvOff[3])
		{
			// Uv.
			CUV			*start= (CUV*)(startPtr + uvOff[3]);
			CUV			*end=	(CUV*)(endPtr   + uvOff[3]);
			CUV			*dst=	(CUV*)(destPtr  + uvOff[3]);
			*dst= *start * a + *end * a1;
		}

	}


	// Process extra UVs (maybe never :)).
	// For all stages after 4.
	for(i=4;i<IDRV_VF_MAXSTAGES;i++)
	{
		uint			nGeoms= geoms.size();
		CMRMWedgeGeom	*ptrGeom= &(geoms[0]);
		uint8			*destPtr= vertexPtr;

		// For all geomorphs.
		for(; nGeoms>0; nGeoms--, ptrGeom++, destPtr+= vertexSize )
		{
			uint8			*startPtr=	vertexPtr + ptrGeom->Start*vertexSize;
			uint8			*endPtr=	vertexPtr + ptrGeom->End*vertexSize;

			// uv[i].
			if(uvOff[i])
			{
				// Uv.
				CUV			*start= (CUV*)(startPtr + uvOff[i]);
				CUV			*end=	(CUV*)(endPtr	+ uvOff[i]);
				CUV			*dst=	(CUV*)(destPtr	+ uvOff[i]);
				*dst= *start * a + *end * a1;
			}

		}
	}
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
		applyGeomorph(lod.Geomorphs, alphaLod);
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
