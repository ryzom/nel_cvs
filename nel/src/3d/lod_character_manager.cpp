/** \file lod_character_manager.cpp
 * <File description>
 *
 * $Id: lod_character_manager.cpp,v 1.2 2002/05/13 16:45:55 berenguier Exp $
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

#include "nel/misc/common.h"
#include "3d/lod_character_manager.h"
#include "3d/lod_character_shape.h"
#include "3d/lod_character_shape_bank.h"


using	namespace std;
using	namespace NLMISC;

namespace NL3D 
{


// ***************************************************************************
#define	NL3D_CLOD_VERTEX_FORMAT	(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag)
#define	NL3D_CLOD_VERTEX_SIZE	16


// ***************************************************************************
CLodCharacterManager::CLodCharacterManager()
{
	_Driver= NULL;
	_MaxNumVertices= 3000;
	_Rendering= false;

	// Setup the format of render
	_VBuffer.setVertexFormat(NL3D_CLOD_VERTEX_FORMAT);


	// setup the material
	_Material.initUnlit();
	_Material.setAlphaTest(true);
}


// ***************************************************************************
CLodCharacterManager::~CLodCharacterManager()
{
	reset();
}

// ***************************************************************************
void			CLodCharacterManager::reset()
{
	// delete shapeBanks.
	for(uint i=0;i<_ShapeBankArray.size();i++)
	{
		if(_ShapeBankArray[i])
			delete _ShapeBankArray[i];
	}

	// clears containers
	contReset(_ShapeBankArray);
	contReset(_ShapeMap);

	// reset render part.
	deleteVertexBuffer();
	_Driver= NULL;
}

// ***************************************************************************
uint32			CLodCharacterManager::createShapeBank()
{
	// search a free entry
	for(uint i=0;i<_ShapeBankArray.size();i++)
	{
		// if ree, use it.
		if(_ShapeBankArray[i]==NULL)
		{
			_ShapeBankArray[i]= new CLodCharacterShapeBank;
			return i;
		}
	}

	// no free entrey, resize array.
	_ShapeBankArray.push_back(new CLodCharacterShapeBank);
	return _ShapeBankArray.size()-1;
}

// ***************************************************************************
const CLodCharacterShapeBank	*CLodCharacterManager::getShapeBank(uint32 bankId) const
{
	if(bankId>=_ShapeBankArray.size())
		return NULL;
	else
		return _ShapeBankArray[bankId];
}

// ***************************************************************************
CLodCharacterShapeBank	*CLodCharacterManager::getShapeBank(uint32 bankId)
{
	if(bankId>=_ShapeBankArray.size())
		return NULL;
	else
		return _ShapeBankArray[bankId];
}

// ***************************************************************************
void			CLodCharacterManager::deleteShapeBank(uint32 bankId)
{
	if(bankId>=_ShapeBankArray.size())
	{
		if(_ShapeBankArray[bankId])
		{
			delete _ShapeBankArray[bankId];
			_ShapeBankArray[bankId]= NULL;
		}
	}
}

// ***************************************************************************
sint32			CLodCharacterManager::getShapeIdByName(const std::string &name) const
{
	CstItStrIdMap	it= _ShapeMap.find(name);
	if(it==_ShapeMap.end())
		return -1;
	else
		return it->second;
}

// ***************************************************************************
const CLodCharacterShape	*CLodCharacterManager::getShape(uint32 shapeId) const
{
	// split the id
	uint	bankId= shapeId >> 16;
	uint	shapeInBankId= shapeId &0xFFFF;

	// if valid bankId
	const CLodCharacterShapeBank	*shapeBank= getShapeBank(bankId);
	if(shapeBank)
	{
		// return the shape from the bank
		return shapeBank->getShape(shapeInBankId);
	}
	else
		return NULL;
}

// ***************************************************************************
bool			CLodCharacterManager::compile()
{
	bool	error= false;

	// clear the map
	contReset(_ShapeMap);

	// build the map
	for(uint i=0; i<_ShapeBankArray.size(); i++)
	{
		if(_ShapeBankArray[i])
		{
			// Parse all Shapes
			for(uint j=0; j<_ShapeBankArray[i]->getNumShapes(); j++)
			{
				// build the shape Id
				uint	shapeId= (i<<16) + j;

				// get the shape
				const CLodCharacterShape	*shape= _ShapeBankArray[i]->getShape(j);
				if(shape)
				{
					const string &name= shape->getName();
					ItStrIdMap	it= _ShapeMap.find(name);
					if(it == _ShapeMap.end())
						// insert the id in the map
						_ShapeMap.insert(make_pair(name, shapeId));
					else
					{
						error= true;
						nlwarning("Found a Character Lod with same name in the manager: %s", name.c_str());
					}
				}
			}
		}
	}

	return error;
}

// ***************************************************************************
// ***************************************************************************
// Render
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CLodCharacterManager::setMaxVertex(uint32 maxVertex)
{
	// we must not be beewteen beginRender() and endRender()
	nlassert(!isRendering());
	_MaxNumVertices= maxVertex;
}

// ***************************************************************************
void			CLodCharacterManager::deleteVertexBuffer()
{
	// test (refptr) if the object still exist in memory.
	if(_VBHard!=NULL)
	{
		// A vbufferhard should still exist only if driver still exist.
		nlassert(_Driver!=NULL);

		// must unlock VBhard before.
		_VBHard->unlock();

		// delete it from driver.
		_Driver->deleteVertexBufferHard(_VBHard);
		_VBHard= NULL;
	}

	// delete the soft one.
	_VBuffer.deleteAllVertices();
}


// ***************************************************************************
void			CLodCharacterManager::beginRender(IDriver *driver, const CVector &managerPos)
{
	// we must not be beewteen beginRender() and endRender()
	nlassert(!isRendering());

	// Reset render
	//=================
	_CurrentVertexId=0;
	_CurrentTriId= 0;

	// update Driver.
	//=================
	nlassert(driver);

	// test change of driver.
	nlassert(driver);
	if( _Driver==NULL || driver!=_Driver )
	{
		// must restart build of VB.
		deleteVertexBuffer();
		_Driver= driver;
		_VBHardOk= _Driver->supportVertexBufferHard();
	}

	// If the vbhard exist, but size is different from cur max size, reset.
	if(_VBHard && _VBHard->getNumVertices()!=_MaxNumVertices)
		deleteVertexBuffer();

	// If VBHard is possible, and if not already allocated, try to create it.
	if(_VBHardOk && _VBHard==NULL)
	{
		_VBHard= driver->createVertexBufferHard(_VBuffer.getVertexFormat(), _VBuffer.getValueTypePointer(), 
			_MaxNumVertices, IDriver::VBHardAGP);
	}

	// Last try to create a std VertexBuffer if the VBHard does not exist.
	if(_VBHard==NULL)
	{
		// just resize the std VB.
		_VBuffer.setNumVertices(_MaxNumVertices);
	}

	// prepare for render.
	//=================

	// Lock Buffer.
	if(_VBHard)
	{
		_VertexData= (uint8*)_VBHard->lock();
		_VertexSize= _VBHard->getVertexSize();
	}
	else
	{
		_VertexData= (uint8*)_VBuffer.getVertexCoordPointer();
		_VertexSize= _VBuffer.getVertexSize();
	}

	// Alloc a minimum of primitives (2*vertices), to avoid as possible reallocation in addRenderCharacterKey
	if(_Triangles.size()<_MaxNumVertices * 2)
		_Triangles.resize(_MaxNumVertices * 2);

	// Local manager matrix
	_ManagerMatrixPos= managerPos;

	// Ok, start rendering
	_Rendering= true;
}

// ***************************************************************************
bool			CLodCharacterManager::addRenderCharacterKey(uint shapeId, uint animId, TGlobalAnimationTime time, bool wrapMode, 
	const CMatrix &worldMatrix, const std::vector<CRGBA> &colorVertex, CRGBA globalLighting)
{
	nlassert(_Driver);
	// we must be beewteen beginRender() and endRender()
	nlassert(isRendering());

	CVector		unPackScaleFactor;
	uint		numVertices;

	// Get the Shape and current key.
	//=============

	// get the shape
	const CLodCharacterShape	*clod= getShape(shapeId);
	// if not found quit, return true
	if(!clod)
		return true;

	// get the anim key
	const CLodCharacterShape::CVector3s		*vertPtr;
	vertPtr= clod->getAnimKey(animId, time, wrapMode, unPackScaleFactor);
	// if not found quit, return true
	if(!vertPtr)
		return true;
	// get num verts
	numVertices= clod->getNumVertices();

	// empty shape??
	if(numVertices==0)
		return true;

	// If too many vertices, quit, returning false.
	if(_CurrentVertexId+numVertices > _MaxNumVertices)
		return false;

	// vertify colors.
	bool	vertexColor= colorVertex.size() == numVertices;
	// if error, take gray as global color.
	CRGBA	globalColor;
	if(!vertexColor)
	{
		globalColor.set(128,128,128,255);
		// pre compute / modualte with instance lighting
		globalColor.modulateFromColorRGBOnly(globalColor, globalLighting);
	}


	// Transform and Add vertices.
	//=============

	// Get matrix pos.
	CVector		matPos= worldMatrix.getPos();
	// compute in manager space.
	matPos -= _ManagerMatrixPos;
	// Get rotation line vectors
	float	a00= worldMatrix.get()[0]; float	a01= worldMatrix.get()[4]; float	a02= worldMatrix.get()[8]; 
	float	a10= worldMatrix.get()[1]; float	a11= worldMatrix.get()[5]; float	a12= worldMatrix.get()[9]; 
	float	a20= worldMatrix.get()[2]; float	a21= worldMatrix.get()[6]; float	a22= worldMatrix.get()[10]; 
	// multiply with scale factor
	a00*= unPackScaleFactor.x; a01*= unPackScaleFactor.y; a02*= unPackScaleFactor.z; 
	a10*= unPackScaleFactor.x; a11*= unPackScaleFactor.y; a12*= unPackScaleFactor.z; 
	a20*= unPackScaleFactor.x; a21*= unPackScaleFactor.y; a22*= unPackScaleFactor.z; 


	// get dst Array.
	uint8	*dstPtr;
	dstPtr= _VertexData + _CurrentVertexId * _VertexSize;

	// NB: loop hardCoded for Vertex+Color only.
	nlassert( NL3D_CLOD_VERTEX_FORMAT == (CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag) );
	nlassert( _VertexSize == NL3D_CLOD_VERTEX_SIZE);	// Vector + RGBA

	// 2 modes, with or without per vertexColor
	if(vertexColor)
	{
		// get color array
		const CRGBA		*colorPtr= &colorVertex[0];
		for(;numVertices>0;numVertices--, vertPtr++, colorPtr++, dstPtr+= NL3D_CLOD_VERTEX_SIZE)
		{
			// NB: order is important for AGP filling optimisation
			// transform vertex, and store.
			CVector		*dstVector= (CVector*)dstPtr;
			dstVector->x= a00 * vertPtr->x + a01 * vertPtr->y + a02 * vertPtr->z + matPos.x;
			dstVector->y= a10 * vertPtr->x + a11 * vertPtr->y + a12 * vertPtr->z + matPos.y;
			dstVector->z= a20 * vertPtr->x + a21 * vertPtr->y + a22 * vertPtr->z + matPos.z;
			// modulate color and store.
			((CRGBA*)(dstPtr + 12))->modulateFromColorRGBOnly(*colorPtr, globalLighting);
		}
	}
	else
	{
		for(;numVertices>0;numVertices--, vertPtr++, dstPtr+= NL3D_CLOD_VERTEX_SIZE)
		{
			// NB: order is important for AGP filling optimisation
			// transform vertex, and store.
			CVector		*dstVector= (CVector*)dstPtr;
			dstVector->x= a00 * vertPtr->x + a01 * vertPtr->y + a02 * vertPtr->z + matPos.x;
			dstVector->y= a10 * vertPtr->x + a11 * vertPtr->y + a12 * vertPtr->z + matPos.y;
			dstVector->z= a20 * vertPtr->x + a21 * vertPtr->y + a22 * vertPtr->z + matPos.z;
			// store global color
			*(CRGBA*)(dstPtr + 12)= globalColor;
		}
	}


	// Add Primitives.
	//=============

	// get number of tri indexes
	uint	numTriIdxs= clod->getNumTriangles() * 3;

	// realloc tris if needed.
	if(_CurrentTriId+numTriIdxs > _Triangles.size())
	{
		_Triangles.resize(_CurrentTriId+numTriIdxs);
	}

	// reindex and copy tris
	const uint32	*srcIdx= clod->getTriangleArray();
	uint32			*dstIdx= &_Triangles[_CurrentTriId];
	for(;numTriIdxs>0;numTriIdxs--, srcIdx++, dstIdx++)
	{
		*dstIdx= *srcIdx + _CurrentVertexId;
	}


	// Next
	//=============

	// Inc Vertex count.
	_CurrentVertexId+= clod->getNumVertices();
	// Inc Prim count.
	_CurrentTriId+= clod->getNumTriangles() * 3;


	// key added
	return true;
}

// ***************************************************************************
void			CLodCharacterManager::endRender()
{
	nlassert(_Driver);
	// we must be beewteen beginRender() and endRender()
	nlassert(isRendering());

	// UnLock Buffer.
	if(_VBHard)
	{
		_VBHard->unlock();
	}

	// Render the VBuffer and the primitives.
	if(_CurrentTriId>0)
	{
		// setup matrix.
		CMatrix		managerMatrix; 
		managerMatrix.setPos(_ManagerMatrixPos);
		_Driver->setupModelMatrix(managerMatrix);

		// active VB
		if(_VBHard)
			_Driver->activeVertexBufferHard(_VBHard);
		else
			_Driver->activeVertexBuffer(_VBuffer);

		// render triangles
		_Driver->renderTriangles(_Material, &_Triangles[0], _CurrentTriId/3);
	}

	// Ok, end rendering
	_Rendering= false;
}


} // NL3D
