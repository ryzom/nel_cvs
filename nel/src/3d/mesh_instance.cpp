/** \file mesh_instance.cpp
 * <File description>
 *
 * $Id: mesh_instance.cpp,v 1.19 2003/08/07 08:49:13 berenguier Exp $
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

#include "3d/mesh_instance.h"
#include "3d/mesh.h"
#include "3d/skeleton_model.h"
#include "nel/3d/u_scene.h"
#include "3d/scene.h"
#include <list>

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************
CMeshInstance::CMeshInstance()
{
	_ShadowMap= NULL;
	_ShadowGeom= NULL;

	// LoadBalancing is not usefull for Mesh, because meshs cannot be reduced in faces.
	// Override CTransformShape state.
	CTransform::setIsLoadbalancable(false);

	// Mesh support shadow map casting only
	CTransform::setIsShadowMapCaster(true);
}

// ***************************************************************************
CMeshInstance::~CMeshInstance()
{
	// Auto detach me from skeleton. Must do it here, not in ~CTransform().
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// hrc and clip hierarchy is modified.
		_FatherSkeletonModel->detachSkeletonSon(this);
		nlassert(_FatherSkeletonModel==NULL);
	}

	// delete the shadowMap
	if(_ShadowMap)
	{
		nlassert(_ShadowGeom);
		delete _ShadowMap;
		delete _ShadowGeom;
		_ShadowMap= NULL;
		_ShadowGeom= NULL;
	}
	nlassert(_ShadowGeom==NULL && _ShadowMap==NULL);
}


// ***************************************************************************
void		CMeshInstance::registerBasic()
{
	CScene::registerModel(MeshInstanceId, MeshBaseInstanceId, CMeshInstance::creator);
}

// ***************************************************************************
void		CMeshInstance::setApplySkin(bool state)
{
	// Call parents method
	CMeshBaseInstance::setApplySkin (state);

	// Get a pointer on the shape
	CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// Recompute the id
	if (state)
	{
		pMesh->computeBonesId (_FatherSkeletonModel);
	}

	// update the skeleton usage according to the mesh.
	pMesh->updateSkeletonUsage(_FatherSkeletonModel, state);
}


// ***************************************************************************
const std::vector<sint32>	*CMeshInstance::getSkinBoneUsage() const
{
	// Get a pointer on the shape
	CMesh	*pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// Recompute the id
	pMesh->computeBonesId (_FatherSkeletonModel);

	// get ids.
	return &pMesh->getMeshGeom().getSkinBoneUsage();
}


// ***************************************************************************
bool	CMeshInstance::isSkinnable() const
{
	if(Shape==NULL)
		return false;

	// Get a pointer on the shape
	CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

	// true if the mesh is skinned
	return pMesh->getMeshGeom().isSkinned();
}


// ***************************************************************************
void	CMeshInstance::renderSkin(float alphaMRM)
{
	// Don't setup lighting or matrix in Skin. Done by the skeleton

	if(Shape && getVisibility() != CHrcTrav::Hide)
	{
		// Get a pointer on the shape
		CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

		// render the meshGeom
		CMeshGeom	&meshGeom= const_cast<CMeshGeom&>(pMesh->getMeshGeom ());
		meshGeom.renderSkin( this, alphaMRM );
	}
}


// ***************************************************************************
void	CMeshInstance::initRenderFilterType()
{
	if(Shape)
	{
		// If the Shape has a VP or not...
		CMesh *pMesh = NLMISC::safe_cast<CMesh *>((IShape*)Shape);

		if( pMesh->getMeshGeom().hasMeshVertexProgram() )
			_RenderFilterType= UScene::FilterMeshVP;
		else
			_RenderFilterType= UScene::FilterMeshNoVP;
	}
}

// ***************************************************************************
// ***************************************************************************
// ShadowMapping
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CMeshInstance::generateShadowMap(const CVector &lightDir)
{
	// get the driver for Texture Render
	CScene			*scene= getOwnerScene();
	CRenderTrav		&renderTrav= scene->getRenderTrav();
	IDriver			*driver= renderTrav.getAuxDriver();

	if(!Shape)
		return;

	// update ShadowMap data if needed.
	// ****
	updateShadowMap(driver);

	if(_ShadowMap)
	{
		nlassert(_ShadowGeom);
	}
	if(!_ShadowMap || _ShadowGeom->CasterTriangles.empty())
		return;

	// compute the ProjectionMatrix.
	// ****

	// get the BBox and localPosMatrix
	CAABBox		bbShape;
	Shape->getAABBox(bbShape);
	CMatrix		localPosMatrix= getWorldMatrix();
	localPosMatrix.setPos(CVector::Null);

	// setup cameraMatrix with BBox and Enlarge For 1 pixel
	CMatrix		cameraMatrix;
	_ShadowMap->buildCasterCameraMatrix(lightDir, localPosMatrix, bbShape, cameraMatrix);


	// Render.
	// ****
	// setup the orhtogonal frustum and viewMatrix to include all the object.
	driver->setFrustum(0,1,0,1,0,1,false);
	driver->setupViewMatrix(cameraMatrix.inverted());
	driver->setupModelMatrix(localPosMatrix);

	// render the Cached VB/Primtives
	driver->activeVertexBuffer(_ShadowGeom->CasterVBuffer);
	CMaterial	&castMat= renderTrav.getShadowMapManager().getCasterShadowMaterial();
	driver->renderTriangles(castMat, &_ShadowGeom->CasterTriangles[0], _ShadowGeom->CasterTriangles.size()/3);

	// Infos.
	// ****

	// Compute the BackPoint: the first point to be shadowed. 
	CVector		backPoint= bbShape.getCenter();
	// get the 3/4 bottom of the shape
	backPoint.z-= bbShape.getHalfSize().z/2;
	backPoint= localPosMatrix * backPoint;
	// Use the 3/4 bottom of the BBox minus the light direction in XY. NB: little hack: 
	// suppose no Rotate (but Z) and no scale
	CVector	ldir= lightDir;
	ldir.z= 0;
	ldir.normalize();
	float	lenXY= (CVector(bbShape.getHalfSize().x, bbShape.getHalfSize().y, 0)).norm();
	backPoint-= ldir*lenXY;

	// Compute LocalProjectionMatrix and other infos from cameraMatrix and backPoint?
	_ShadowMap->buildProjectionInfos(cameraMatrix, backPoint, scene);
}

// ***************************************************************************
CShadowMap	*CMeshInstance::getShadowMap()
{
	return _ShadowMap;
}

// ***************************************************************************
void		CMeshInstance::updateShadowMap(IDriver *driver)
{
	// create the shadowMap if not already done.
	if(!_ShadowMap)
	{
		uint i;

		_ShadowMap= new CShadowMap;
		_ShadowGeom= new CShadowGeom;

		// create a VBuffer with only Position Data
		CMesh	*mesh= safe_cast<CMesh*>((IShape*)Shape);
		const CVertexBuffer	&vbSrc= mesh->getVertexBuffer();
		// init
		_ShadowGeom->CasterVBuffer.setVertexFormat(CVertexBuffer::PositionFlag);
		uint	numVerts= vbSrc.getNumVertices();
		_ShadowGeom->CasterVBuffer.setNumVertices(numVerts);
		// fill
		for(i=0;i<numVerts;i++)
		{
			_ShadowGeom->CasterVBuffer.setVertexCoord(i, *(CVector*)vbSrc.getVertexCoordPointer(i));
		}

		// TODO: OPTIM : VBHard
		/* TODO: OPTIM : must store the shadow Geometry in the mesh and not in the instance (in shadowMap)!
			Not very interesting to optimize because casters will be only skeletons 
			(even the items like swords won't be rendered in CMeshInstance since sticked to a skeleton)
		*/

		// Copy All triangles in the PB cache.
		uint	nbMB= mesh->getNbMatrixBlock();
		// count tris
		uint	numTris= 0;
		for(i=0;i<nbMB;i++)
		{
			uint	nbRP= mesh->getNbRdrPass(i);
			for(uint j=0;j<nbRP;j++)
			{
				numTris+= mesh->getRdrPassPrimitiveBlock(i, j).getNumTri();
			}
		}
		if(numTris)
		{
			// allocate
			_ShadowGeom->CasterTriangles.resize(numTris*3);
			// Copy.
			uint32	*triPtr= &_ShadowGeom->CasterTriangles[0];
			for(i=0;i<nbMB;i++)
			{
				uint	nbRP= mesh->getNbRdrPass(i);
				for(uint j=0;j<nbRP;j++)
				{
					uint	passNummTri= mesh->getRdrPassPrimitiveBlock(i, j).getNumTri();
					memcpy(triPtr, mesh->getRdrPassPrimitiveBlock(i, j).getTriPointer(), passNummTri*3*sizeof(uint32));
					triPtr+= passNummTri*3;
				}
			}
		}
	}

	// create/update texture
	if(_ShadowMap->getTextureSize()!=getOwnerScene()->getShadowMapTextureSize())
	{
		_ShadowMap->initTexture(getOwnerScene()->getShadowMapTextureSize());
	}
}


// ***************************************************************************
void	CMeshInstance::traverseRender()
{
	CMeshBaseInstance::traverseRender();

	/*
		Doing like this (and not like skeleton scheme) result in 2 problems:
			- MehsInstance ShadowMap Casting are not "Loded" ie they are computed each frame.
			- The shadow is displayed only if the mesh is, which is conceptually false.
		BUT this is just an easy demo of ShadowMap.
		Additionally, still do the correct test: if I am son of a SkeletonModel, then I don't have to cast my 
		shadowMap since my skeleton father will do it for me.
	*/
	if(canCastShadowMap() && _AncestorSkeletonModel==NULL )
	{
		// Since the mesh is rendered, add it to the list.
		getOwnerScene()->getRenderTrav().getShadowMapManager().addShadowCaster(this);
		// Compute each frame.
		getOwnerScene()->getRenderTrav().getShadowMapManager().addShadowCasterGenerate(this);
	}
}

// ***************************************************************************
bool	CMeshInstance::computeWorldBBoxForShadow(NLMISC::CAABBox &worldBB)
{
	// If even not visible or empty, no-op
	if(!isHrcVisible() || !Shape)
		return false;

	// get the shape bbox
	Shape->getAABBox(worldBB);
	// transform into world
	worldBB= CAABBox::transformAABBox(getWorldMatrix(), worldBB);

	return true;
}

// ***************************************************************************
void	CMeshInstance::renderIntoSkeletonShadowMap(CSkeletonModel *rootSkeleton, CMaterial	&castMat)
{
	/*
		Yoyo: Not Very Robuts here:
			- suppose the MeshInstance don't have sons.
			- suppose that the VertexBuffer doesn't have VertexColor (else castMat.color unused).
	*/

	// If even not visible or empty, no-op
	if(!isHrcVisible() || !Shape)
		return;

	// render into aux Driver
	IDriver			*driver= getOwnerScene()->getRenderTrav().getAuxDriver();

	// **** Render the Skeleton Skins
	// The model Matrix is special here. It must be the Skeleton World Matrix, minus The Root Skeleton pos.
	CMatrix		localPosMatrix;
	localPosMatrix.setRot( getWorldMatrix() );
	// NB: if this==rootSkeleton, then the final pos will be CVector::Null
	localPosMatrix.setPos( getWorldMatrix().getPos() - rootSkeleton->getWorldMatrix().getPos() );
	driver->setupModelMatrix(localPosMatrix);

	// render the Mesh
	CMesh	*mesh= (CMesh*)(IShape*)Shape;
	driver->activeVertexBuffer( const_cast<CVertexBuffer&>(mesh->getVertexBuffer()) );
	for(uint mb=0;mb<mesh->getNbMatrixBlock();mb++)
	{
		for(uint rp=0;rp<mesh->getNbRdrPass(mb);rp++)
		{
			const CPrimitiveBlock	&pb= mesh->getRdrPassPrimitiveBlock(mb, rp);
			driver->render(const_cast<CPrimitiveBlock&>(pb), castMat);
		}
	}

}


} // NL3D
