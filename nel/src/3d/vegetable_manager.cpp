/** \file vegetable_manager.cpp
 * <File description>
 *
 * $Id: vegetable_manager.cpp,v 1.2 2001/11/05 16:26:45 berenguier Exp $
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


#include "3d/vegetable_manager.h"
#include "3d/driver.h"
#include "3d/texture_file.h"


using namespace std;
using namespace NLMISC;


namespace NL3D 
{


#define	NL3D_VEGETABLE_CLIP_BLOCK_BLOCKSIZE			16
#define	NL3D_VEGETABLE_INSTANCE_GROUP_BLOCKSIZE		128


// ***************************************************************************
CVegetableManager::CVegetableManager() : 
	_ClipBlockMemory(NL3D_VEGETABLE_CLIP_BLOCK_BLOCKSIZE),
	_InstanceGroupMemory(NL3D_VEGETABLE_INSTANCE_GROUP_BLOCKSIZE)
{
	// Init all the allocators with the appropriate pass.
	for(uint i=0; i <NL3D_VEGETABLE_NRDRPASS; i++)
	{
		_VBAllocator[i].init(i);
	}

	// setup the material. Unlit (doesn't matter, lighting in VP) Alpha Test.
	_VegetableMaterial.initUnlit();
	_VegetableMaterial.setAlphaTest(true);

	// default light.
	_DirectionalLight= (CVector(0,1, -1)).normed();

	// Wind.
	_WindDirection.set(1,0,0);
	_WindFrequency= 1;
	_WindPower= 1;
	_WindTime= 0;
	_WindPrecRenderTime= 0;
	_WindAnimTime= 0;
}


// ***************************************************************************
CVegetableClipBlock			*CVegetableManager::createClipBlock()
{
	// create a clipblock
	CVegetableClipBlock	*ret;
	ret= _ClipBlockMemory.allocate();

	// append to list.
	_EmptyClipBlockList.append(ret);

	return ret;
}

// ***************************************************************************
void						CVegetableManager::deleteClipBlock(CVegetableClipBlock *clipBlock)
{
	if(!clipBlock)
		return;

	// verify no more IGs in this clipblock
	nlassert(clipBlock->_InstanceGroupList.size() == 0);

	// unlink from _EmptyClipBlockList, because _InstanceGroupList.size() == 0 ...
	_EmptyClipBlockList.remove(clipBlock);

	// delete
	_ClipBlockMemory.free(clipBlock);
}

// ***************************************************************************
CVegetableInstanceGroup		*CVegetableManager::createIg(CVegetableClipBlock *clipBlock)
{
	nlassert(clipBlock);

	// create an IG
	CVegetableInstanceGroup	*ret;
	ret= _InstanceGroupMemory.allocate();

	// if the clipBlock is empty, change list, because won't be no more.
	if(clipBlock->_InstanceGroupList.size()==0)
	{
		// remove from empty list
		_EmptyClipBlockList.remove(clipBlock);
		// and append to not empty one.
		_ClipBlockList.append(clipBlock);
	}

	// link ig to clipBlock.
	ret->_Owner= clipBlock;
	clipBlock->_InstanceGroupList.append(ret);

	return ret;
}

// ***************************************************************************
void						CVegetableManager::deleteIg(CVegetableInstanceGroup *ig)
{
	if(!ig)
		return;


	// For all render pass of this instance, delete his vertices
	for(sint rdrPass=0; rdrPass < NL3D_VEGETABLE_NRDRPASS; rdrPass++)
	{
		// which allocator?
		CVegetableVBAllocator	*allocator= &_VBAllocator[rdrPass];

		// For all vertices of this rdrPass, delete it
		sint	numVertices;
		numVertices= ig->_RdrPass[rdrPass].Vertices.size();
		for(sint i=0; i<numVertices;i++)
		{
			allocator->deleteVertex(ig->_RdrPass[rdrPass].Vertices[i]);
		}
		ig->_RdrPass[rdrPass].Vertices.clear();
	}


	// unlink from clipBlock, and delete.
	CVegetableClipBlock		*clipBlock= ig->_Owner;
	clipBlock->_InstanceGroupList.remove(ig);
	_InstanceGroupMemory.free(ig);


	// if the clipBlock is now empty, change list
	if(clipBlock->_InstanceGroupList.size()==0)
	{
		// remove from normal list
		_ClipBlockList.remove(clipBlock);
		// and append to empty list.
		_EmptyClipBlockList.append(clipBlock);
	}

}


// ***************************************************************************
CVegetableShape				*CVegetableManager::getVegetableShape(const std::string &shape)
{
	ItShapeMap	it= _ShapeMap.find(shape);
	// if found
	if(it != _ShapeMap.end())
		return &it->second;
	// else insert
	{
		// insert.
		CVegetableShape		*ret;
		it= ( _ShapeMap.insert(make_pair(shape, CVegetableShape()) ) ).first;
		ret= &it->second;

		// fill.
		ret->loadShape(shape);

		return ret;
	}
}


// ***************************************************************************
void			CVegetableManager::addInstance(CVegetableInstanceGroup *ig, 
		CVegetableShape	*shape, const NLMISC::CMatrix &mat, 
		const NLMISC::CRGBAF &ambientColor, const NLMISC::CRGBAF &diffuseColor, 
		float	bendFactor, float bendPhase)
{
	sint	i;

	// Some setup.
	//--------------------
	bool	instanceLighted= shape->Lighted;
	bool	instanceDoubleSided= shape->DoubleSided;

	// get correct rdrPass / allocator where we insert vertices/faces
	uint	rdrPass;
	// get according to lighted / doubleSided state
	if(instanceLighted)
	{
		if(instanceDoubleSided)
			rdrPass= NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED;
		else
			rdrPass= NL3D_VEGETABLE_RDRPASS_LIGHTED;
	}
	else
	{
		if(instanceDoubleSided)
			rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED;
		else
			rdrPass= NL3D_VEGETABLE_RDRPASS_UNLIT;
	}

	// get correct allocator
	CVegetableVBAllocator	*allocator;
	allocator= &_VBAllocator[rdrPass];

	// get correct dstVB
	const CVertexBuffer	&dstVBInfo= allocator->getSoftwareVertexBuffer();

	// color.
	CRGBA		primaryRGBA= diffuseColor;
	CRGBA		secondaryRGBA= ambientColor;

	// if the instance is not lighted, then suppose full lighting => add ambient and diffuse
	if(!instanceLighted)
	{
		primaryRGBA.R= min(255, primaryRGBA.R + secondaryRGBA.R);
		primaryRGBA.G= min(255, primaryRGBA.G + secondaryRGBA.G);
		primaryRGBA.B= min(255, primaryRGBA.B + secondaryRGBA.B);
		// useFull if 2Sided
		secondaryRGBA= primaryRGBA;
	}


	// Transform vertices to a vegetable instance, and enlarge clipBlock
	//--------------------
	// compute matrix to multiply normals, ie (M-1)t
	CMatrix		normalMat;
	// need just rotation scale matrix.
	normalMat.setRot(mat);
	normalMat.invert();
	normalMat.transpose();
	// compute Instance position
	CVector		instancePos;
	mat.getPos(instancePos);


	// At least, the bbox of the clipBlock must include the center of the shape.
	ig->_Owner->extendSphere(instancePos);


	// Vertex Info.
	sint	numVertices= shape->VB.getNumVertices();

	// src info.
	uint	srcVertexSize= shape->VB.getVertexSize();
	uint	srcNormalOff= (instanceLighted? shape->VB.getNormalOff() : 0);
	uint	srcTex0Off= shape->VB.getTexCoordOff(0);
	uint	srcTex1Off= shape->VB.getTexCoordOff(1);

	// dst info
	uint	dstVertexSize= dstVBInfo.getVertexSize();
	uint	dstNormalOff= (instanceLighted? dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_NORMAL) : 0);
	// got 2nd color if lighted (for ambient) or if 2Sided.
	uint	dstColor1Off= ( (instanceLighted||instanceDoubleSided)? 
		dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_COLOR1) : 0);
	uint	dstColor0Off= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_COLOR0);
	uint	dstTex0Off= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_TEX0);
	uint	dstBendOff= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_BENDINFO);
	uint	dstCenterOff= dstVBInfo.getValueOffEx(NL3D_VEGETABLE_VPPOS_CENTER);


	// For all vertices of shape, transform and store manager indices in temp shape.
	for(i=0; i<numVertices;i++)
	{
		// allocate a Vertex
		uint	vid= allocator->allocateVertex();
		// store in tmp shape.
		shape->InstanceVertices[i]= vid;

		// Fill this vertex.
		uint8	*srcPtr= (uint8*)shape->VB.getVertexCoordPointer(i);
		uint8	*dstPtr= (uint8*)allocator->getVertexPointer(vid);

		// Pos.
		//-------
		// Separate Center and relative pos.
		CVector	relPos= mat.mulVector(*(CVector*)srcPtr);	// mulVector, because translation in v[center]
		// compute bendCenterPos
		CVector	bendCenterPos;
		if(shape->BendCenterMode == CVegetableShapeBuild::BendCenterNull)
			bendCenterPos= CVector::Null;
		else
		{
			CVector	v= *(CVector*)srcPtr;
			v.z= 0;
			bendCenterPos= mat.mulVector(v);				// mulVector, because translation in v[center]
		}
		// copy
		*(CVector*)dstPtr= relPos-bendCenterPos;
		*(CVector*)(dstPtr + dstCenterOff)= instancePos + bendCenterPos;

		// Enlarge the clipBlock of the IG.
		// Since small shape, enlarge with each vertices. simpler and maybe faster.
		// TODO_VEGET: bend and clipping ...
		ig->_Owner->extendBBoxOnly(instancePos + relPos);

		// If ligthed
		//-------
		if(instanceLighted)
		{
			// normal and ambient
			*(CVector*)(dstPtr + dstNormalOff)= normalMat.mulVector( *(CVector*)(srcPtr + srcNormalOff) );
			*(CRGBA*)(dstPtr + dstColor1Off)= secondaryRGBA;
		}
		else if(instanceDoubleSided)
		{
			// secondary color computed.
			*(CRGBA*)(dstPtr + dstColor1Off)= secondaryRGBA;
		}


		// Color / Texture.
		//-------
		*(CRGBA*)(dstPtr + dstColor0Off)= primaryRGBA;
		*(CUV*)(dstPtr + dstTex0Off)= *(CUV*)(srcPtr + srcTex0Off);

		// Bend.
		//-------
		CUV		*dstBendPtr= (CUV*)(dstPtr + dstBendOff);
		// setup bend Weight.
		dstBendPtr->U= ((CUV*)(srcPtr + srcTex1Off))->U * bendFactor;
		// setup bend Phase.
		dstBendPtr->V= bendPhase;


		// fill the vertex in AGP.
		//-------
		allocator->flushVertex(vid);
	}


	// must recompute the sphere according to the bbox.
	ig->_Owner->updateSphere();


	// re-index triangles to manager indices
	//--------------------
	sint	numIndices= shape->TriangleIndices.size();
	// for all indices, get manager index
	for(i=0; i<numIndices; i++)
	{
		// get the index of the vertex in the shape
		uint	vid= shape->TriangleIndices[i];
		// re-direction, using InstanceVertices;
		shape->InstanceTriangleIndices[i]= shape->InstanceVertices[vid];
	}


	// Append list of indices and list of triangles to the IG
	//--------------------

	// TODO_VEGET_OPTIM: system reallocation of array is very bad...

	// insert list of vertices to delete in ig vertices.
	ig->_RdrPass[rdrPass].Vertices.insert(ig->_RdrPass[rdrPass].Vertices.end(), 
		shape->InstanceVertices.begin(), shape->InstanceVertices.end());
	// insert array of triangles in ig.
	ig->_RdrPass[rdrPass].TriangleIndices.insert(ig->_RdrPass[rdrPass].TriangleIndices.end(), 
		shape->InstanceTriangleIndices.begin(), shape->InstanceTriangleIndices.end() );
	// new triangle size.
	ig->_RdrPass[rdrPass].NTriangles= ig->_RdrPass[rdrPass].TriangleIndices.size() / 3;

}


// ***************************************************************************
// ***************************************************************************
// Render
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool			CVegetableManager::doubleSidedRdrPass(uint rdrPass)
{
	nlassert(rdrPass<NL3D_VEGETABLE_NRDRPASS);
	return (rdrPass == NL3D_VEGETABLE_RDRPASS_LIGHTED_2SIDED) || 
		(rdrPass == NL3D_VEGETABLE_RDRPASS_UNLIT_2SIDED);
}

// ***************************************************************************
void			CVegetableManager::updateDriver(IDriver *driver)
{
	// update all driver
	for(uint i=0; i <NL3D_VEGETABLE_NRDRPASS; i++)
	{
		_VBAllocator[i].updateDriver(driver);
	}
}


// ***************************************************************************
void			CVegetableManager::loadTexture(const string &texName)
{
	// setup a CTextureFile (smartPtr-ized).
	loadTexture(new CTextureFile(texName));
}

// ***************************************************************************
void			CVegetableManager::loadTexture(ITexture *itex)
{
	// setup a ITexture (smartPtr-ized).
	_VegetableMaterial.setTexture(0, itex);
}

// ***************************************************************************
void			CVegetableManager::setDirectionalLight(const CVector &light)
{
	_DirectionalLight= light;
	_DirectionalLight.normalize();
}

// ***************************************************************************
void			CVegetableManager::lockBuffers()
{
	// lock all buffers
	for(uint i=0; i <NL3D_VEGETABLE_NRDRPASS; i++)
	{
		_VBAllocator[i].lockBuffer();
	}
}

// ***************************************************************************
void			CVegetableManager::unlockBuffers()
{
	// unlock all buffers
	for(uint i=0; i <NL3D_VEGETABLE_NRDRPASS; i++)
	{
		_VBAllocator[i].unlockBuffer();
	}
}


// ***************************************************************************
void			CVegetableManager::render(const std::vector<CPlane> &pyramid, IDriver *driver)
{
	CVegetableClipBlock		*rootToRender= NULL;


	// Clip.
	//--------------------
	// For all current not empty clipBlocks, clip against pyramid, and insert visibles in list.
	CVegetableClipBlock		*ptrClipBlock= _ClipBlockList.begin();
	while(ptrClipBlock)
	{
		// if the clipBlock is visible and not empty
		if(ptrClipBlock->clip(pyramid))
		{
			// insert into visible list.
			ptrClipBlock->_RenderNext= rootToRender;
			rootToRender= ptrClipBlock;
		}

		// next
		ptrClipBlock= (CVegetableClipBlock*)ptrClipBlock->Next;
	}



	// Render
	//--------------------


	// set model matrix to identity.
	driver->setupModelMatrix(CMatrix::Identity);

	// set the driver for all allocators
	updateDriver(driver);


	// Compute Bend Anim.
	double	timeBend;

	// AnimFrequency factor.
	// Doing it incrementaly allow change of of frequency each frame with good results.
	_WindAnimTime+= (_WindTime - _WindPrecRenderTime)*_WindFrequency;
	_WindAnimTime= fmod(_WindAnimTime, 1.0f);
	timeBend= _WindAnimTime;
	// For incremental computing.
	_WindPrecRenderTime= _WindTime;

	// Freq to Angle.
	timeBend*= 2*Pi;
	timeBend-= Pi;

	// compute the angleAxis corresponding to direction
	CVector	angleAxis;
	// perform a 90° rotation to get correct angleAxis
	angleAxis.set(-_WindDirection.y,_WindDirection.x,0);


	// Standard
	// setup VertexProgram constants.
	// c[0..3] take the ModelViewProjection Matrix. After setupModelMatrix();
	driver->setConstantMatrix(0, IDriver::ModelViewProjection, IDriver::Identity);
	// c[4..7] take the ModelView Matrix. After setupModelMatrix();
	driver->setConstantMatrix(4, IDriver::ModelView, IDriver::Identity);
	// c[8] take usefull constants.
	driver->setConstant(8, 0, 1, 0.5f, 2);
	// c[9] take normalized directional light
	driver->setConstant(9, &_DirectionalLight);

	// Bend.
	// c[16]= quaternion axis. w==1, and z must be 0
	driver->setConstant( 16, angleAxis.x, angleAxis.y, angleAxis.z, 1);
	// c[17]=	{timeAnim (angle), WindPower, 0, 0)}
	driver->setConstant( 17, (float)timeBend, _WindPower, 0, 0 );
	// c[18]=	High order Taylor cos coefficient: { -1/2, 1/24, -1/720, 1/40320 }
	driver->setConstant( 18, -1/2.f, 1/24.f, -1/720.f, 1/40320.f );
	// c[19]=	Low order Taylor cos coefficient: { 1, -1/2, 1/24, -1/720 }
	driver->setConstant( 19, 1, -1/2.f, 1/24.f, -1/720.f );
	// c[20]=	Low order Taylor sin coefficient: { 1, -1/6, 1/120, -1/5040 }
	driver->setConstant( 20, 1, -1/6.f, 1/120.f, -1/5040.f );
	// c[21]=	Special constant vector for quatToMatrix: { 0, 1, -1, 0 }
	driver->setConstant( 21, 0.f, 1.f, -1.f, 0.f);
	// c[22]=	{0.5f, Pi, 2*Pi, 1/(2*Pi)}
	driver->setConstant( 22, 0.5f, (float)Pi, (float)(2*Pi), (float)(1/(2*Pi)) );



	// For all renderPass.
	for(sint rdrPass=0; rdrPass < NL3D_VEGETABLE_NRDRPASS; rdrPass++)
	{
		// additional setup to the material
		bool	doubleSided= doubleSidedRdrPass(rdrPass);
		// set the 2Sided flag in the material
		_VegetableMaterial.setDoubleSided( doubleSided );
		// must enable VP DoubleSided coloring
		if(doubleSided)
			driver->enableVertexProgramDoubleSidedColor(true);
		else
			driver->enableVertexProgramDoubleSidedColor(false);


		// Activate the unique material.
		driver->setupMaterial(_VegetableMaterial);

		// Activate the good VBuffer, and VertexProgram.
		_VBAllocator[rdrPass].activate();

		// For all visibles clipBlock, render their instance groups.
		ptrClipBlock= rootToRender;
		while(ptrClipBlock)
		{
			// For all igs of the clipBlock
			CVegetableInstanceGroup		*ptrIg= ptrClipBlock->_InstanceGroupList.begin();
			while(ptrIg)
			{
				// Render the faces of the good renderPass.
				if(ptrIg->_RdrPass[rdrPass].NTriangles)
				{
					driver->renderSimpleTriangles(&ptrIg->_RdrPass[rdrPass].TriangleIndices[0], 
						ptrIg->_RdrPass[rdrPass].NTriangles);
				}

				// next ig.
				ptrIg= (CVegetableInstanceGroup*)ptrIg->Next;
			}

			// next clipBlock to render 
			ptrClipBlock= ptrClipBlock->_RenderNext;
		}

	}

	// disable VertexProgram.
	driver->activeVertexProgram(NULL);

	// reset state to default.
	driver->enableVertexProgramDoubleSidedColor(false);


}




// ***************************************************************************
void		CVegetableManager::setWind(const CVector &windDir, float windFreq, float windPower)
{
	// Keep only XY component of the Wind direction (because VP only support z==0 quaternions).
	_WindDirection= windDir;
	_WindDirection.z= 0;
	_WindDirection.normalize();
	// copy setup
	_WindFrequency= windFreq;
	_WindPower= windPower;
}

// ***************************************************************************
void		CVegetableManager::setWindAnimationTime(double windTime)
{
	// copy time
	_WindTime= windTime;
}


} // NL3D
