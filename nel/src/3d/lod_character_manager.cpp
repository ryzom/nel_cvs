/** \file lod_character_manager.cpp
 * <File description>
 *
 * $Id: lod_character_manager.cpp,v 1.10 2003/08/07 08:47:52 berenguier Exp $
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
#include "3d/lod_character_instance.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/fast_floor.h"
#include "3d/lod_character_texture.h"
#include "nel/misc/file.h"


using	namespace std;
using	namespace NLMISC;

namespace NL3D 
{


H_AUTO_DECL ( NL3D_CharacterLod_Render )


// ***************************************************************************
// Dest is without Normal because precomputed
#define	NL3D_CLOD_VERTEX_FORMAT	(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::PrimaryColorFlag)
#define	NL3D_CLOD_VERTEX_SIZE	24
#define	NL3D_CLOD_UV_OFF		12
#define	NL3D_CLOD_COLOR_OFF		20

// size (in block) of the big texture.
#define	NL3D_CLOD_TEXT_NLOD_WIDTH	16
#define	NL3D_CLOD_TEXT_NLOD_HEIGHT	16
#define	NL3D_CLOD_TEXT_NUM_IDS		NL3D_CLOD_TEXT_NLOD_WIDTH*NL3D_CLOD_TEXT_NLOD_HEIGHT
#define	NL3D_CLOD_BIGTEXT_WIDTH		NL3D_CLOD_TEXT_NLOD_WIDTH*NL3D_CLOD_TEXT_WIDTH
#define	NL3D_CLOD_BIGTEXT_HEIGHT	NL3D_CLOD_TEXT_NLOD_HEIGHT*NL3D_CLOD_TEXT_HEIGHT

// Default texture color. Alpha must be 255
#define	NL3D_CLOD_DEFAULT_TEXCOLOR	CRGBA(255,255,255,255)


// ***************************************************************************
CLodCharacterManager::CLodCharacterManager()
{
	_Driver= NULL;
	_MaxNumVertices= 3000;
	_Rendering= false;

	// Setup the format of render
	_VBuffer.setVertexFormat(NL3D_CLOD_VERTEX_FORMAT);

	// NB: addRenderCharacterKey() loop hardCoded for Vertex+UV+Normal+Color only.
	nlassert( NL3D_CLOD_UV_OFF == _VBuffer.getTexCoordOff());
	nlassert( NL3D_CLOD_COLOR_OFF == _VBuffer.getColorOff());

	// setup the texture.
	_BigTexture= new CTextureBlank;
	// The texture always reside in memory... This take 1Mo of RAM. (16*32*16*32 * 4)
	// NB: this is simplier like that, and this is not a problem, since only 1 or 2 Mo are allocated :o)
	_BigTexture->setReleasable(false);
	// create the bitmap.
	_BigTexture->resize(NL3D_CLOD_BIGTEXT_WIDTH, NL3D_CLOD_BIGTEXT_HEIGHT, CBitmap::RGBA);
	// Format of texture, 16 bits and no mipmaps.
	_BigTexture->setUploadFormat(ITexture::RGB565);
	_BigTexture->setFilterMode(ITexture::Linear, ITexture::LinearMipMapOff);
	_BigTexture->setWrapS(ITexture::Clamp);
	_BigTexture->setWrapT(ITexture::Clamp);

	// Alloc free Ids
	_FreeIds.resize(NL3D_CLOD_TEXT_NUM_IDS);
	for(uint i=0;i<_FreeIds.size();i++)
	{
		_FreeIds[i]= i;
	}

	// setup the material
	_Material.initUnlit();
	_Material.setAlphaTest(true);
	_Material.setDoubleSided(true);
	_Material.setTexture(0, _BigTexture);

	// setup for lighting, Default for Ryzom setup
	_LightCorrectionMatrix.rotateZ((float)Pi/2);
	_LightCorrectionMatrix.invert();
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

		// must unlock VBhard before. ATI: No need to update any vertices.
		_VBHard->unlock(0,0);

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
	H_AUTO_USE ( NL3D_CharacterLod_Render )

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
			_MaxNumVertices, IDriver::VBHardAGP, _VBuffer.getUVRouting());
		// Set Name For Profiling
		if(_VBHard)
			_VBHard->setName("CLodManagerVB");
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
	// NB: addRenderCharacterKey() loop hardCoded for Vertex+UV+Normal+Color only.
	nlassert( _VertexSize == NL3D_CLOD_VERTEX_SIZE );	// Vector + Normal + UV + RGBA


	// Alloc a minimum of primitives (2*vertices), to avoid as possible reallocation in addRenderCharacterKey
	if(_Triangles.size()<_MaxNumVertices * 2)
		_Triangles.resize(_MaxNumVertices * 2);

	// Local manager matrix
	_ManagerMatrixPos= managerPos;

	// Ok, start rendering
	_Rendering= true;
}


// ***************************************************************************
static inline CRGBA	computeLodLighting(const CVector &lightObjectSpace, const CVector &normalPtr, CRGBA ambient, CRGBA diffuse)
{
	// \todo yoyo: TODO_OPTIMIZE
	float	f= lightObjectSpace * normalPtr;
	f= max(0.f,f);
	sint	f8= NLMISC::OptFastFloor(f*255);
	CRGBA	lightRes;
	lightRes.modulateFromuiRGBOnly(diffuse, f8);
	lightRes.addRGBOnly(lightRes, ambient);
	return lightRes;
}


// ***************************************************************************
bool			CLodCharacterManager::addRenderCharacterKey(CLodCharacterInstance &instance, const CMatrix &worldMatrix, 
	CRGBA ambient, CRGBA diffuse, const CVector &lightDir)
{
	H_AUTO_USE ( NL3D_CharacterLod_Render )

	nlassert(_Driver);
	// we must be beewteen beginRender() and endRender()
	nlassert(isRendering());

	CVector		unPackScaleFactor;
	uint		numVertices;

	// Get the Shape and current key.
	//=============

	// get the shape
	const CLodCharacterShape	*clod= getShape(instance.ShapeId);
	// if not found quit, return true
	if(!clod)
		return true;

	// get UV/Normal array. NULL => error
	const CVector	*normalPtr= clod->getNormals();
	// get UV of the instance
	const CUV		*uvPtr= instance.getUVs();
	// uvPtr is NULL means that initInstance() has not been called!!
	nlassert(normalPtr && uvPtr);

	// get the anim key
	const CLodCharacterShape::CVector3s		*vertPtr;
	vertPtr= clod->getAnimKey(instance.AnimId, instance.AnimTime, instance.WrapMode, unPackScaleFactor);
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

	// verify colors.
	bool	vertexColor= instance.VertexColors.size() == numVertices;
	// if error, take white as global color.


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

	// get the light in object space.
	CVector		lightObjectSpace;
	// Multiply light dir with transpose of worldMatrix. This may be not exact (not uniform scale) but sufficient.
	lightObjectSpace.x= a00 * lightDir.x + a10 * lightDir.y + a20 * lightDir.z;
	lightObjectSpace.y= a01 * lightDir.x + a11 * lightDir.y + a21 * lightDir.z;
	lightObjectSpace.z= a02 * lightDir.x + a12 * lightDir.y + a22 * lightDir.z;
	// animation User correction
	lightObjectSpace= _LightCorrectionMatrix.mulVector(lightObjectSpace);
	// normalize, and neg for Dot Product.
	lightObjectSpace.normalize();
	lightObjectSpace= -lightObjectSpace;

	// multiply matrix with scale factor for Pos.
	a00*= unPackScaleFactor.x; a01*= unPackScaleFactor.y; a02*= unPackScaleFactor.z; 
	a10*= unPackScaleFactor.x; a11*= unPackScaleFactor.y; a12*= unPackScaleFactor.z; 
	a20*= unPackScaleFactor.x; a21*= unPackScaleFactor.y; a22*= unPackScaleFactor.z; 


	// get dst Array.
	uint8	*dstPtr;
	dstPtr= _VertexData + _CurrentVertexId * _VertexSize;

	// 2 modes, with or without per vertexColor
	if(vertexColor)
	{
		// get color array
		const CRGBA		*colorPtr= &instance.VertexColors[0];
		for(;numVertices>0;numVertices--)
		{
			// NB: order is important for AGP filling optimisation
			// transform vertex, and store.
			CVector		*dstVector= (CVector*)dstPtr;
			dstVector->x= a00 * vertPtr->x + a01 * vertPtr->y + a02 * vertPtr->z + matPos.x;
			dstVector->y= a10 * vertPtr->x + a11 * vertPtr->y + a12 * vertPtr->z + matPos.y;
			dstVector->z= a20 * vertPtr->x + a21 * vertPtr->y + a22 * vertPtr->z + matPos.z;
			// Copy UV
			*(CUV*)(dstPtr + NL3D_CLOD_UV_OFF)= *uvPtr;

			// Compute Lighting.
			CRGBA	lightRes= computeLodLighting(lightObjectSpace, *normalPtr, ambient, diffuse);
			// modulate color and store.
			((CRGBA*)(dstPtr + NL3D_CLOD_COLOR_OFF))->modulateFromColorRGBOnly(*colorPtr, lightRes);
			// Copy Alpha.
			((CRGBA*)(dstPtr + NL3D_CLOD_COLOR_OFF))->A= colorPtr->A;

			// next
			vertPtr++;
			uvPtr++;
			normalPtr++;
			colorPtr++;
			dstPtr+= NL3D_CLOD_VERTEX_SIZE;
		}
	}
	else
	{
		for(;numVertices>0;numVertices--)
		{
			// NB: order is important for AGP filling optimisation
			// transform vertex, and store.
			CVector		*dstVector= (CVector*)dstPtr;
			dstVector->x= a00 * vertPtr->x + a01 * vertPtr->y + a02 * vertPtr->z + matPos.x;
			dstVector->y= a10 * vertPtr->x + a11 * vertPtr->y + a12 * vertPtr->z + matPos.y;
			dstVector->z= a20 * vertPtr->x + a21 * vertPtr->y + a22 * vertPtr->z + matPos.z;
			// Copy UV
			*(CUV*)(dstPtr + NL3D_CLOD_UV_OFF)= *uvPtr;

			// Compute Lighting.
			CRGBA	lightRes= computeLodLighting(lightObjectSpace, *normalPtr, ambient, diffuse);
			lightRes.A= 255;
			// store global color
			*(CRGBA*)(dstPtr + NL3D_CLOD_COLOR_OFF)= lightRes;

			// next
			vertPtr++;
			uvPtr++;
			normalPtr++;
			dstPtr+= NL3D_CLOD_VERTEX_SIZE;
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
	H_AUTO_USE ( NL3D_CharacterLod_Render )

	nlassert(_Driver);
	// we must be beewteen beginRender() and endRender()
	nlassert(isRendering());

	// UnLock Buffer.
	if(_VBHard)
	{
		// ATI: copy only used vertices.
		_VBHard->unlock(0, _CurrentVertexId);
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

// ***************************************************************************
void			CLodCharacterManager::setupNormalCorrectionMatrix(const CMatrix &normalMatrix)
{
	_LightCorrectionMatrix= normalMatrix;
	_LightCorrectionMatrix.setPos(CVector::Null);
	_LightCorrectionMatrix.invert();
}


// ***************************************************************************
// ***************************************************************************
// Texturing.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CLodCharacterTmpBitmap::CLodCharacterTmpBitmap()
{
	reset();
}

// ***************************************************************************
void			CLodCharacterTmpBitmap::reset()
{
	// setup a 1*1 bitmap
	_Bitmap.resize(1);
	_Bitmap[0]= CRGBA::Black;
	_WidthPower=0;
	_UShift= 8;
	_VShift= 8;
}

// ***************************************************************************
void			CLodCharacterTmpBitmap::build(const NLMISC::CBitmap &bmpIn)
{
	uint	width= bmpIn.getWidth();
	uint	height= bmpIn.getHeight();
	nlassert(width>0 && width<=256);
	nlassert(height>0 && height<=256);

	// resize bitmap.
	_Bitmap.resize(width*height);
	_WidthPower= getPowerOf2(width);
	// compute shift
	_UShift= 8-getPowerOf2(width);
	_VShift= 8-getPowerOf2(height);

	// convert the bitmap.
	CBitmap		bmp= bmpIn;
	bmp.convertToType(CBitmap::RGBA);
	CRGBA	*src= (CRGBA*)&bmp.getPixels()[0];
	CRGBA	*dst= _Bitmap.getPtr();
	for(sint nPix= width*height;nPix>0;nPix--, src++, dst++)
	{
		*dst= *src;
	}
}

// ***************************************************************************
void			CLodCharacterTmpBitmap::build(CRGBA col)
{
	// setup a 1*1 bitmap and set it with col
	reset();
	_Bitmap[0]= col;
}


// ***************************************************************************
void			CLodCharacterManager::initInstance(CLodCharacterInstance &instance)
{
	// first release in (maybe) other manager.
	if(instance._Owner)
		instance._Owner->releaseInstance(instance);

	// get the shape
	const CLodCharacterShape	*clod= getShape(instance.ShapeId);
	// if not found quit
	if(!clod)
		return;
	// get Uvs.
	const CUV	*uvSrc= clod->getUVs();
	nlassert(uvSrc);


	// Ok, init header
	instance._Owner= this;
	instance._UVs.resize(clod->getNumVertices());

	// allocate an id. If cannot, then fill Uvs with 0 => filled with Black. (see endTextureCompute() why).
	if(_FreeIds.empty())
	{
		// set a "Not enough memory" id
		instance._TextureId= NL3D_CLOD_TEXT_NUM_IDS;
		CUV		uv(0,0);
		fill(instance._UVs.begin(), instance._UVs.end(), uv);
	}
	// else OK, can instanciate the Uvs.
	else
	{
		// get the id.
		instance._TextureId= _FreeIds.back();
		_FreeIds.pop_back();
		// get the x/y.
		uint	xId= instance._TextureId % NL3D_CLOD_TEXT_NLOD_WIDTH;
		uint	yId= instance._TextureId / NL3D_CLOD_TEXT_NLOD_WIDTH;
		// compute the scale/bias to apply to Uvs.
		float	scaleU= 1.0f / NL3D_CLOD_TEXT_NLOD_WIDTH;
		float	scaleV= 1.0f / NL3D_CLOD_TEXT_NLOD_HEIGHT;
		float	biasU= (float)xId / NL3D_CLOD_TEXT_NLOD_WIDTH;
		float	biasV= (float)yId / NL3D_CLOD_TEXT_NLOD_HEIGHT;
		// apply it to each UVs.
		CUV		*uvDst= &instance._UVs[0];
		for(uint i=0; i<instance._UVs.size();i++)
		{
			uvDst[i].U= biasU + uvSrc[i].U*scaleU;
			uvDst[i].V= biasV + uvSrc[i].V*scaleV;
		}
	}
}

// ***************************************************************************
void			CLodCharacterManager::releaseInstance(CLodCharacterInstance &instance)
{
	if(instance._Owner==NULL)
		return;
	nlassert(this==instance._Owner);

	// if the id is not a "Not enough memory" id, release it.
	if(instance._TextureId>=0 && instance._TextureId<NL3D_CLOD_TEXT_NUM_IDS)
		_FreeIds.push_back(instance._TextureId);

	// reset the instance
	instance._Owner= NULL;
	instance._TextureId= -1;
	contReset(instance._UVs);
}


// ***************************************************************************
CRGBA			*CLodCharacterManager::getTextureInstance(CLodCharacterInstance &instance)
{
	nlassert(instance._Owner==this);
	nlassert(instance._TextureId!=-1);
	// if the texture id is a "not enough memory", quit.
	if(instance._TextureId==NL3D_CLOD_TEXT_NUM_IDS)
		return NULL;

	// get the x/y.
	uint	xId= instance._TextureId % NL3D_CLOD_TEXT_NLOD_WIDTH;
	uint	yId= instance._TextureId / NL3D_CLOD_TEXT_NLOD_WIDTH;

	// get the ptr on the correct pixel.
	CRGBA	*pix= (CRGBA*)&_BigTexture->getPixels(0)[0];
	return pix + yId*NL3D_CLOD_TEXT_HEIGHT*NL3D_CLOD_BIGTEXT_WIDTH + xId*NL3D_CLOD_TEXT_WIDTH;
}


// ***************************************************************************
bool			CLodCharacterManager::startTextureCompute(CLodCharacterInstance &instance)
{
	CRGBA	*dst= getTextureInstance(instance);
	if(!dst)
		return false;

	// erase the texture with 0,0,0,255. Alpha is actually the min "Quality" part of the CTUVQ.
	CRGBA	col= NL3D_CLOD_DEFAULT_TEXCOLOR;
	for(uint y=0;y<NL3D_CLOD_TEXT_HEIGHT;y++)
	{
		// erase the line
		for(uint x=0;x<NL3D_CLOD_TEXT_WIDTH;x++)
			dst[x]= col;
		// Next line
		dst+= NL3D_CLOD_BIGTEXT_WIDTH;
	}

	return true;
}

// ***************************************************************************
void			CLodCharacterManager::addTextureCompute(CLodCharacterInstance &instance, const CLodCharacterTexture &lodTexture)
{
	CRGBA	*dst= getTextureInstance(instance);
	if(!dst)
		return;

	// get lookup ptr.
	nlassert(lodTexture.Texture.size()==NL3D_CLOD_TEXT_SIZE);
	const CLodCharacterTexture::CTUVQ		*lookUpPtr= &lodTexture.Texture[0];

	// apply the lodTexture, taking only better quality (ie nearer 0)
	for(uint y=0;y<NL3D_CLOD_TEXT_HEIGHT;y++)
	{
		// erase the line
		for(uint x=0;x<NL3D_CLOD_TEXT_WIDTH;x++)
		{
			CLodCharacterTexture::CTUVQ		lut= *lookUpPtr;
			// if this quality is better than the one stored
			if(lut.Q<dst[x].A)
			{
				// get what texture to read, and read the pixel.
				CRGBA	col= _TmpBitmaps[lut.T].getPixel(lut.U, lut.V);
				// set quality.
				col.A= lut.Q;
				// set in dest
				dst[x]= col;
			}

			// next lookup
			lookUpPtr++;
		}
		// Next line
		dst+= NL3D_CLOD_BIGTEXT_WIDTH;
	}
}

// ***************************************************************************
void			CLodCharacterManager::endTextureCompute(CLodCharacterInstance &instance, uint numBmpToReset)
{
	CRGBA	*dst= getTextureInstance(instance);
	if(!dst)
		return;

	// reset All Alpha values to 255 => no AlphaTest problems
	for(uint y=0;y<NL3D_CLOD_TEXT_HEIGHT;y++)
	{
		// erase the line
		for(uint x=0;x<NL3D_CLOD_TEXT_WIDTH;x++)
		{
			dst[x].A= 255;
		}
		// Next line
		dst+= NL3D_CLOD_BIGTEXT_WIDTH;
	}

	// If the id == 0 then must reset the 0,0 Pixel to black. for the "Not Enough memory" case in initInstance().
	if(instance._TextureId==0)
		*(CRGBA*)&_BigTexture->getPixels(0)[0]= NL3D_CLOD_DEFAULT_TEXCOLOR;

	// get the x/y.
	uint	xId= instance._TextureId % NL3D_CLOD_TEXT_NLOD_WIDTH;
	uint	yId= instance._TextureId / NL3D_CLOD_TEXT_NLOD_WIDTH;
	// touch the texture for Driver update.
	_BigTexture->touchRect(
		CRect(xId*NL3D_CLOD_TEXT_WIDTH, yId*NL3D_CLOD_TEXT_HEIGHT, NL3D_CLOD_TEXT_WIDTH, NL3D_CLOD_TEXT_HEIGHT) );

	// reset tmpBitmaps / free memory.
	for(uint i=0; i<numBmpToReset; i++)
	{
		_TmpBitmaps[i].reset();
	}

	// TestYoyo
	/*NLMISC::COFile	f("tam.tga");
	_BigTexture->writeTGA(f,32);*/
}


} // NL3D
