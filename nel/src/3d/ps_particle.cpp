/** \file ps_particle.cpp
 * <File description>
 *
 * $Id: ps_particle.cpp,v 1.43 2001/09/12 13:20:10 vizerie Exp $
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

#include "3d/ps_particle.h"
#include "3d/driver.h"
#include "3d/ps_attrib_maker.h"
#include "3d/texture_grouped.h"
#include "3d/scene.h"
#include "3d/shape_bank.h"
#include "3d/transform_shape.h"
#include "3d/texture_mem.h"
#include "3d/fast_floor.h"
#include "3d/particle_system.h"

#include "nel/misc/common.h"
#include "nel/misc/quat.h"
#include "nel/misc/file.h"
#include "nel/misc/line.h"
#include "nel/misc/path.h"


#include "3d/dru.h"


#include <algorithm>




namespace NL3D {


/** this macro check wether a pointer is inside a given VB. Has memory in Vertex Buffer if accessed directly, 
  * we can't detect memory overwrite
  * the first parameter is the VB, the second is a pointrer into it
  */

#define CHECK_VERTEX_BUFFER(vb, pt) nlassert((uint8 *) (pt) >= (uint8 *) (vb).getVertexCoordPointer()  \
									&& (uint8 *) (pt) < ((uint8 *) (vb).getVertexCoordPointer() + (vb).getVertexSize() * (vb).getNumVertices()));


// this macro check the memory integrity (windows platform for now). It may be useful after violent vb access
#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
	#include <crtdbg.h>
	#define PARTICLES_CHECK_MEM nlassert(_CrtCheckMemory());
#else
	#define PARTICLES_CHECK_MEM
#endif



using NLMISC::CQuat; 							   

///////////////////////////
// constant definition   //
///////////////////////////


static const uint dotBufSize = 1024; // size used for point particles batching

static const uint quadBufSize = 800; // size used for quad particles batching. 
							   // If this is too high, the cache may be broken
							   // too small values will result in too much driver calls


static const uint shockWaveBufSize = 64; // number of shockwave to be processed at once


static const uint meshBufSize = 16; // number of meshs to be processed at once...

static const uint constraintMeshBufSize = 64; // number of meshs to be processed at once...


/////////////////////////////////
// CPSParticle implementation  //
/////////////////////////////////

void CPSParticle::showTool()
{
	PARTICLES_CHECK_MEM;

	CVector I = CVector::I;
	CVector J = CVector::J;

	const CVector tab[] = { 2 * J, I + J
							, I + J, 2 * I + J
							, 2 * I + J, I
							, I,  2 * I - J
							, 2 * I - J, - .5f * J
							, - .5f * J, -2 * I - J
							, -2 * I - J, - I
							, - I, -2 * I + J
							, -2 * I + J, - I + J
							, - I + J, 2 * J
						};
	const uint tabSize = sizeof(tab) / (2 * sizeof(CVector));

	const float sSize = 0.1f;
	displayIcon2d(tab, tabSize, sSize);

	PARTICLES_CHECK_MEM;
}


/*
 * Constructor
 */
CPSParticle::CPSParticle()
{
}



//////////////////////////////////////
// coloured particle implementation //
//////////////////////////////////////



void CPSColoredParticle::setColorScheme(CPSAttribMaker<CRGBA> *col)
{
	if (_UseColorScheme)
	{
		delete _ColorScheme;	
		_ColorScheme = NULL;
	}
	else
	{
		_UseColorScheme = true;
	}

	_ColorScheme = col;
	if (getColorOwner() && col->hasMemory()) col->resize(getColorOwner()->getMaxSize(), getColorOwner()->getSize());

	updateMatAndVbForColor();
}


		
void CPSColoredParticle::setColor(NLMISC::CRGBA col)
{
	if (_UseColorScheme)
	{
		delete _ColorScheme;
		_ColorScheme = NULL;
		_UseColorScheme = false;
	}
	_Color = col;

	updateMatAndVbForColor();
}

		
CPSColoredParticle::CPSColoredParticle() : _UseColorScheme(false), _ColorScheme(NULL), _Color(CRGBA(255, 255, 255))
{	
}



/// dtor

CPSColoredParticle::~CPSColoredParticle()
{
	if (_UseColorScheme)
	{
		delete _ColorScheme;
	}
}


/// serialization

void CPSColoredParticle::serialColorScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	f.serialVersion(1);	
	if (f.isReading())
	{
		if (_UseColorScheme)
		{
			delete _ColorScheme;
			_ColorScheme = NULL;
		}
	}

	f.serial(_UseColorScheme);

	if (_UseColorScheme)
	{
		f.serialPolyPtr(_ColorScheme);
	}
	else
	{
		f.serial(_Color);
	}
}


///////////////////////////////////
// sized particle implementation //
///////////////////////////////////

void CPSSizedParticle::setSizeScheme(CPSAttribMaker<float> *size)
{
	if (_UseSizeScheme)
	{
		delete _SizeScheme;
		_SizeScheme = NULL;
	}
	else
	{
		_UseSizeScheme = true;
	}

	_SizeScheme = size;
	if (getSizeOwner() && size->hasMemory()) size->resize(getSizeOwner()->getMaxSize(), getSizeOwner()->getSize());
}


		
void CPSSizedParticle::setSize(float size)
{
	if (_UseSizeScheme)
	{
		delete _SizeScheme;
		_SizeScheme = NULL;
		_UseSizeScheme = false;
	}
	_ParticleSize = size;
}



/// ctor : default are 0.3 sized particles
CPSSizedParticle::CPSSizedParticle() : _UseSizeScheme(false), _SizeScheme(NULL), _ParticleSize(0.3f)
{
}
	
CPSSizedParticle::~CPSSizedParticle()
{
	if (_UseSizeScheme)
	{
		delete _SizeScheme;
	}
}


		/// serialization
void CPSSizedParticle::serialSizeScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	if (f.isReading())
	{
		if (_UseSizeScheme)
		{
			delete _SizeScheme;
			_SizeScheme = NULL;
		}
	}


	f.serial(_UseSizeScheme);

	if (_UseSizeScheme)
	{		
		f.serialPolyPtr(_SizeScheme);
	}
	else
	{
		f.serial(_ParticleSize);
	}	
};

/////////////////////////////////////
// rotated particle implementation //
/////////////////////////////////////


float CPSRotated2DParticle::_RotTable[256 * 4];

//#ifdef NL_DEBUG
	bool CPSRotated2DParticle::_InitializedRotTab = false;
//#endif


void CPSRotated2DParticle::setAngle2DScheme(CPSAttribMaker<float> *angle2DScheme)
{
	if (_UseAngle2DScheme)
	{
		delete _Angle2DScheme;
		_Angle2DScheme = NULL;
	}
	else
	{
		_UseAngle2DScheme = true;
	}

	_Angle2DScheme = angle2DScheme;
	if (getAngle2DOwner() && angle2DScheme->hasMemory()) angle2DScheme->resize(getAngle2DOwner()->getMaxSize(), getAngle2DOwner()->getSize());
}


void CPSRotated2DParticle::setAngle2D(float angle2DScheme)
{
	if (_UseAngle2DScheme)
	{
		delete _Angle2DScheme;
		_Angle2DScheme = NULL;
		_UseAngle2DScheme = false;
	}
	_Angle2D = angle2DScheme;
}

		
CPSRotated2DParticle::CPSRotated2DParticle() : _Angle2D(0), _UseAngle2DScheme(false), _Angle2DScheme(NULL)
{
}

		
CPSRotated2DParticle::~CPSRotated2DParticle()
{
	if (_UseAngle2DScheme)
	{
		delete _Angle2DScheme;
		_UseAngle2DScheme = false;
	}
}

		
void CPSRotated2DParticle::serialAngle2DScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	if (f.isReading())
	{
		if (_UseAngle2DScheme)
		{
			delete _Angle2DScheme;
			_Angle2DScheme = NULL;
		}
	}

	f.serial(_UseAngle2DScheme);
	if (_UseAngle2DScheme)
	{
		f.serialPolyPtr(_Angle2DScheme);
	}
	else
	{		
		f.serial(_Angle2D);
	}	
}


void CPSRotated2DParticle::initRotTable(void)
{
	float *ptFloat = _RotTable;
	for (uint32 k = 0; k < 256; ++k)
	{
		const float ca = (float) cos(k * (1.0f / 256.0f) * 2.0f * NLMISC::Pi);
		const float sa = (float) sin(k * (1.0f / 256.0f) * 2.0f * NLMISC::Pi);

		*ptFloat++ = -ca - sa;
		*ptFloat++ = -sa + ca;

		*ptFloat++ = ca - sa;
		*ptFloat++ = sa + ca;
/*
		*ptFloat++ = 2 * ca;
		*ptFloat++ = 2 * sa;

		*ptFloat++ = 2 * ca;
		*ptFloat++ = 2 * sa;

		*ptFloat++ = 2 * sa;
		*ptFloat++ = -2 * ca;

		*ptFloat++ = -2 * ca;
		*ptFloat++ = -2 * sa; */
	}

	#ifdef NL_DEBUG
		_InitializedRotTab = true;
	#endif
}


//////////////////////////////////////
// textured particle implementation //
//////////////////////////////////////

void CPSTexturedParticle::setTextureIndexScheme(CPSAttribMaker<sint32> *animOrder)
{
	nlassert(animOrder);
	nlassert(_TexGroup); // setTextureGroup must have been called before this
	if (_UseTextureIndexScheme)
	{
		delete _TextureIndexScheme;
		_TextureIndexScheme = NULL;
	}
	else
	{
		_UseTextureIndexScheme = true;
	}

	_TextureIndexScheme = animOrder;
	if (getTextureIndexOwner() && animOrder->hasMemory()) animOrder->resize(getTextureIndexOwner()->getMaxSize(), getTextureIndexOwner()->getSize());


	updateMatAndVbForTexture();
}


/// set a constant index for the current texture. not very useful, but available...
void CPSTexturedParticle::setTextureIndex(sint32 index)
{
	if (_UseTextureIndexScheme)
	{
		delete _TextureIndexScheme;
		_TextureIndexScheme = NULL;
	}
	_TextureIndexScheme = NULL;
	_UseTextureIndexScheme = false;

	_TextureIndex = index;
}

void CPSTexturedParticle::setTextureGroup(NLMISC::CSmartPtr<CTextureGrouped> texGroup)
{
	nlassert(texGroup);
	if (_Tex)
	{
		_Tex = NULL;
	}
	_TexGroup = texGroup;
}


void CPSTexturedParticle::setTexture(CSmartPtr<ITexture> tex)
{
	if (_UseTextureIndexScheme)
	{
		delete _TextureIndexScheme;
		_TextureIndexScheme = NULL;
		_UseTextureIndexScheme = false;
	}
	_Tex = tex;
	_TexGroup = NULL; // release any grouped texture if one was set before

	updateMatAndVbForTexture();
}

		
CPSTexturedParticle::CPSTexturedParticle() : _UseTextureIndexScheme(false), _TextureIndex(0)
											 ,_TexGroup(NULL), _TextureIndexScheme(NULL)
{
}

CPSTexturedParticle::~CPSTexturedParticle()
{
	if (_UseTextureIndexScheme)
	{
		delete _TextureIndexScheme;
	}
}

		/// serialization. We choose a different name because of multiple-inheritance

void CPSTexturedParticle::serialTextureScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	if (f.isReading())
	{
		if (_UseTextureIndexScheme)
		{
			delete _TextureIndexScheme;
			_TextureIndexScheme = NULL;
			_UseTextureIndexScheme = false;
			_Tex = NULL;
			_TexGroup = NULL;
		}
	}

	bool useAnimatedTexture;
	
	if (!f.isReading())
	{
		useAnimatedTexture = (_TexGroup != NULL);
	}

	f.serial(useAnimatedTexture);

	



	if (useAnimatedTexture)
	{
		if (f.isReading())
		{
			CTextureGrouped *ptTex = NULL;
			f.serialPolyPtr(ptTex);
			_TexGroup = ptTex;
		}
		else
		{
			CTextureGrouped *ptTex = _TexGroup;
			f.serialPolyPtr(ptTex);
		}
		
		bool useTextureIndexScheme;

		if (!f.isReading())
		{
			useTextureIndexScheme = _UseTextureIndexScheme;
		}

		f.serial(useTextureIndexScheme);
		if (useTextureIndexScheme)
		{
			f.serialPolyPtr(_TextureIndexScheme);
			_TextureIndex = 0;
			_UseTextureIndexScheme = true;
		}
		else
		{
			_UseTextureIndexScheme = false;
			f.serial(_TextureIndex);
		}
	}
	else
	{
		if (f.isReading())
		{
			ITexture *ptTex = NULL;
			f.serialPolyPtr(ptTex);
			_Tex = ptTex;
		}
		else
		{
			ITexture *ptTex = _Tex;
			f.serialPolyPtr(ptTex);
		}

	}	
}



//////////////////////////////////////////
//       CPSRotated3DPlaneParticle      //
//////////////////////////////////////////



void CPSRotated3DPlaneParticle::setPlaneBasisScheme(CPSAttribMaker<CPlaneBasis> *basisMaker)
{
	nlassert(basisMaker);
	if (_UsePlaneBasisScheme)
	{
		delete _PlaneBasisScheme;
		_PlaneBasisScheme = NULL;
	}
	else
	{
		_UsePlaneBasisScheme = true;
	}
	_PlaneBasisScheme = basisMaker;
	if (getPlaneBasisOwner() && basisMaker->hasMemory()) basisMaker->resize(getPlaneBasisOwner()->getMaxSize(), getPlaneBasisOwner()->getSize());
}

void CPSRotated3DPlaneParticle::setPlaneBasis(const CPlaneBasis &basis)
{
	if (_UsePlaneBasisScheme)
	{
		delete _PlaneBasisScheme;		
		_PlaneBasisScheme = NULL;
		_UsePlaneBasisScheme = false;
	}
	_PlaneBasis = basis;
}


CPSRotated3DPlaneParticle::CPSRotated3DPlaneParticle() : _UsePlaneBasisScheme(false), _PlaneBasisScheme(NULL)														
{
	_PlaneBasis.X = CVector::I;
	_PlaneBasis.Y = CVector::J;
}


CPSRotated3DPlaneParticle::~CPSRotated3DPlaneParticle()
{
	if (_UsePlaneBasisScheme)
	{
		delete _PlaneBasisScheme;
	}
}

void CPSRotated3DPlaneParticle::serialPlaneBasisScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	

	f.serialPolyPtr(_PlaneBasisScheme);
	
	_UsePlaneBasisScheme = (_PlaneBasisScheme != NULL);

	// TODO : remove the use of bool _Use... =, it is useless...

	if (!_UsePlaneBasisScheme)
	{
		f.serial(_PlaneBasis);
	}
}

////////////////////////////////
// CPSMaterial implemantation //
////////////////////////////////


CPSMaterial::CPSMaterial()
{
	_Mat.setBlend(true);
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one);
	_Mat.setZWrite(false);	

}





void CPSMaterial::serialMaterial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	if (f.isReading())
	{
		TBlendingMode m = add; // initialization to avoid a warning only
		f.serialEnum(m);
		setBlendingMode(m);
	}
	else
	{
		TBlendingMode m = getBlendingMode();
		f.serialEnum(m);
	}
}


void CPSMaterial::setBlendingMode(CPSMaterial::TBlendingMode mode)
{
	switch (mode)
	{		
		case add:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::one, CMaterial::one);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case modulate:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::zero, CMaterial::srccolor);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case alphaBlend:
			_Mat.setBlend(true);
			_Mat.setBlendFunc(CMaterial::srcalpha, CMaterial::invsrcalpha);
			_Mat.setZWrite(false);
			_Mat.setAlphaTest(false);
		break;
		case alphaTest:
			_Mat.setBlend(false);
			_Mat.setZWrite(true);
			_Mat.setAlphaTest(true);
		break;
	}
}


CPSMaterial::TBlendingMode CPSMaterial::getBlendingMode(void) const
{
	if (_Mat.getBlend())
	{
		CMaterial::TBlend srcBlend = _Mat.getSrcBlend();	
		CMaterial::TBlend destBlend = _Mat.getDstBlend();

		if (srcBlend == CMaterial::one && destBlend == CMaterial::one) return add;
		if (srcBlend == CMaterial::zero && destBlend == CMaterial::srccolor) return modulate;
		if (srcBlend == CMaterial::srcalpha && destBlend == CMaterial::invsrcalpha) return alphaBlend;

		// unrecognized mode
		nlassert(0);
		return alphaTest; // to avoid a warning only ...
	}
	else
	{
		return alphaTest;
	}
}




///////////////////////////
// CPSDot implementation //
///////////////////////////

void CPSDot::init(void)
{		
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	
	updateMatAndVbForColor();
}

uint32 CPSDot::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize();
}

void CPSDot::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newColorElement(emitterLocated, emitterIndex);
}
void CPSDot::deleteElement(uint32 index)
{
	deleteColorElement(index);
}


void CPSDot::updateMatAndVbForColor(void)
{
	if (!_UseColorScheme)
	{
		_Vb.setVertexFormat(CVertexBuffer::PositionFlag);		
		_Mat.setColor(_Color);
	}
	else
	{
		_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag);
		_Mat.setColor(CRGBA::White);
	}

	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}
}



bool CPSDot::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}
bool CPSDot::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}


void CPSDot::resize(uint32 size)
{
	_Vb.setNumVertices(size < dotBufSize ? size : dotBufSize);
	resizeColor(size);
}

void CPSDot::draw(bool opaque)
{
	
	PARTICLES_CHECK_MEM;
	// we create a vertex buffer that contains all the particles before to show them
	uint32 size = _Owner->getSize();


	if (!size) return;


	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	

	
	
	setupDriverModelMatrix();
	
	IDriver *driver = getDriver();
	driver->activeVertexBuffer(_Vb);		



	uint32 leftToDo = size, toProcess;

	TPSAttribVector::iterator it = _Owner->getPos().begin();
	TPSAttribVector::iterator itEnd;

	do
	{
		
		toProcess = leftToDo < dotBufSize ? leftToDo : dotBufSize;

		if (_UseColorScheme)
		{
			// compute the colors
			_ColorScheme->make(_Owner, 0, _Vb.getColorPointer(), _Vb.getVertexSize(), toProcess);
			itEnd = it + toProcess;
				
		
			uint8    *currPos = (uint8 *) _Vb.getVertexCoordPointer();	
			uint32 stride = _Vb.getVertexSize();
			do
			{
				CHECK_VERTEX_BUFFER(_Vb, currPos);
				*((CVector *) currPos) =  *it;	
				++it ;
				currPos += stride;
			}
			while (it != itEnd);
		}
		else
		{
			// there's no color information in the buffer, so we can copy it directly
			memcpy(_Vb.getVertexCoordPointer(), &(*it), sizeof(CVector) * toProcess);
			it += toProcess;
		}
				
		driver->renderPoints(_Mat,toProcess);

		leftToDo -= toProcess;
	}
	while (leftToDo);

	PARTICLES_CHECK_MEM;
}



void CPSDot::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	
	f.serialVersion(1);	


	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);
	serialMaterial(f);
	if (f.isReading())
	{
		init();		
	}
}


//////////////////////////////////
// CPSQuad implementation       //
//////////////////////////////////


CPSQuad::CPSQuad(CSmartPtr<ITexture> tex)
{
	setTexture(tex);
	init();
	// we don't init the _IndexBuffer for now, as it will be when resize is called
	_Name = std::string("quad");
}


CPSQuad::~CPSQuad()
{
}

uint32 CPSQuad::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize() << 1;
}

bool CPSQuad::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}
bool CPSQuad::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}


void CPSQuad::init(void)
{	
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);


	updateMatAndVbForColor();
	updateMatAndVbForTexture();
}

void CPSQuad::updateMatAndVbForTexture(void)
{	
	_Mat.setTexture(0, _TexGroup ? (ITexture *) _TexGroup : (ITexture *) _Tex);	
}

bool CPSQuad::completeBBox(NLMISC::CAABBox &box) const  
{ 
	if (!_UseSizeScheme)
	{
		CPSUtil::addRadiusToAABBox(box, _ParticleSize);
	}
	else
	{
		CPSUtil::addRadiusToAABBox(box, _SizeScheme->getMaxValue());
	}


	return true ;	
}

void CPSQuad::resize(uint32 aSize)
{

	resizeSize(aSize);
	resizeColor(aSize);
	resizeTextureIndex(aSize);

	// the size used for buffer can't be higher that quad buf size
	// to have too large buffer will broke the cache

	const uint32 size = aSize > quadBufSize ? quadBufSize : aSize;

	_Vb.setNumVertices(size << 2);
		

	


	for (uint32 k = 0; k < size; ++k)
	{		

		_Vb.setTexCoord(k * 4, 0, CUV(0, 0));
		_Vb.setTexCoord(k * 4 + 1, 0, CUV(1, 0));
		_Vb.setTexCoord(k * 4 + 2, 0, CUV(1, 1));
		_Vb.setTexCoord(k * 4 + 3, 0, CUV(0, 1));	
	}		
}

void CPSQuad::updateMatAndVbForColor(void)
{
	if (!_UseColorScheme)
	{
		_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);		
		_Mat.setColor(_Color);
	}
	else
	{
		_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag);
		_Mat.setColor(CRGBA::White);
	}

	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}		
}

void CPSQuad::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);		
	CPSParticle::serial(f);	
	CPSSizedParticle::serialSizeScheme(f);
	CPSColoredParticle::serialColorScheme(f);
	CPSTexturedParticle::serialTextureScheme(f);
	serialMaterial(f);
}


void CPSQuad::updateVbColNUVForRender(uint32 startIndex, uint32 size)
{
nlassert(_Owner);

if (!size) return;

if (_UseColorScheme)
{
	// compute the colors, each color is replicated 4 times
	_ColorScheme->make4(_Owner, startIndex, _Vb.getColorPointer(), _Vb.getVertexSize(), size);
}


if (_TexGroup) // if it has a constant texture we are sure it has been setupped before...
{	
	sint32 textureIndex[quadBufSize];
	const uint32 stride = _Vb.getVertexSize(), stride2 = stride << 1, stride3 = stride2 + stride, stride4 = stride2 << 1;
	uint8 *currUV = (uint8 *) _Vb.getTexCoordPointer();				
	

	const sint32 *currIndex;		
	uint32 currIndexIncr; 

	if (_UseTextureIndexScheme)
	{
		currIndex = (sint32 *) _TextureIndexScheme->make(_Owner, startIndex, textureIndex, sizeof(sint32), size, true);			
		currIndexIncr = 1;
	}
	else
	{
		currIndex = &_TextureIndex;
		currIndexIncr  = 0;
	}

	while (size--)
	{
		// for now, we don't make texture index wrapping
		const CTextureGrouped::TFourUV &uvGroup = _TexGroup->getUVQuad((uint32) *currIndex);

		// copy the 4 uv's for this face
		*(CUV *) currUV = uvGroup.uv0;
		*(CUV *) (currUV + stride) = uvGroup.uv1;
		*(CUV *) (currUV + stride2) = uvGroup.uv2;
		*(CUV *) (currUV + stride3) = uvGroup.uv3;

		// point the next face
		currUV += stride4;
		currIndex += currIndexIncr;
	}		
}
}


//////////////////////////////////
// CPSFaceLookAt implementation //
//////////////////////////////////

/// create the face look at by giving a texture and an optionnal color
CPSFaceLookAt::CPSFaceLookAt(CSmartPtr<ITexture> tex) : CPSQuad(tex), _MotionBlurCoeff(0.f)
														, _Threshold(0.5f), _IndependantSizes(false)
{	
	_SecondSize.Owner = this;
	_Name = std::string("LookAt");
}


void CPSFaceLookAt::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	CPSQuad::newElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
}
	
void CPSFaceLookAt::deleteElement(uint32 index)
{
	CPSQuad::deleteElement(index);
	deleteAngle2DElement(index);
}

void CPSFaceLookAt::resize(uint32 capacity)
{
	CPSQuad::resize(capacity);
	resizeAngle2D(capacity);
}

void CPSFaceLookAt::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;

	nlassert(_Owner);

	const uint32 size = _Owner->getSize();
	if (!size) return;


	IDriver *driver = getDriver();

	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	


	setupDriverModelMatrix();
	driver->activeVertexBuffer(_Vb);	


	const CVector I = computeI();
	const CVector J = computeJ();
	const CVector K = computeK();

	
	TPSAttribVector::iterator it = _Owner->getPos().begin();
	const float *rotTable = CPSRotated2DParticle::getRotTable();	
	// for each the particle can be constantly rotated or have an independant rotation for each particle
		

	// number of face left, and number of face to process at once
	uint32 leftToDo = size, toProcess;

	float pSizes[quadBufSize]; // the sizes to use	
	float pSecondSizes[quadBufSize]; // the second sizes to use

	float *currentSize; 
	uint32 currentSizeStep = _UseSizeScheme ? 1 : 0;
	
	

	// point the vector part in the current vertex
	uint8 *ptPos;

	// strides to go from one vertex to another one
	const uint32 stride = _Vb.getVertexSize(), stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride << 2;					
	
	if (!_UseAngle2DScheme)
	{
		// constant rotation case
		

		do
		{			
				// restart at the beginning of the vertex buffer
				ptPos = (uint8 *) _Vb.getVertexCoordPointer();
				toProcess = leftToDo <= quadBufSize ? leftToDo : quadBufSize;

				if (_UseSizeScheme)
				{
					currentSize = (float *) _SizeScheme->make(_Owner, size- leftToDo, pSizes, sizeof(float), toProcess, true);									
				}
				else
				{
					currentSize = &_ParticleSize;
				}

				//
				updateVbColNUVForRender(size - leftToDo, toProcess);

						
				TPSAttribVector::iterator endIt = it + toProcess;

				/* version 1 : slow code because the CVector ctor can't be inlined !! */
				/*
				while (it != endIt)
				{
					*(CVector *) ptPos = *it  + *currentSize * v1;  			
					*(CVector *) (ptPos + stride) = *it + *currentSize * v2;	
					*(CVector *) (ptPos + stride2) = *it - *currentSize * v1;
					*(CVector *) (ptPos + stride3) = *it - *currentSize * v2;	

					++it;
					currentSize += currentSizeStep;
					ptPos += stride4;
				}
				*/

				/* version 2 : here, we avoid ctor calls, but we must perform the op ourself  */																

				if (_MotionBlurCoeff == 0.f)
				{
					if (!_IndependantSizes)
					{						
						const uint32 tabIndex = (((uint32) _Angle2D) & 0xff) << 2;
						const CVector v1 = rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K;
						const CVector v2 = rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K;

						if (currentSizeStep)
						{
							while (it != endIt)
							{
								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  + *currentSize * v1.x;  			
								((CVector *) ptPos)->y = it->y  + *currentSize * v1.y;  			
								((CVector *) ptPos)->z = it->z  + *currentSize * v1.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  + *currentSize * v2.x;  			
								((CVector *) ptPos)->y = it->y  + *currentSize * v2.y;  			
								((CVector *) ptPos)->z = it->z  + *currentSize * v2.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  - *currentSize * v1.x;  			
								((CVector *) ptPos)->y = it->y  - *currentSize * v1.y;  			
								((CVector *) ptPos)->z = it->z  - *currentSize * v1.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  - *currentSize * v2.x;  			
								((CVector *) ptPos)->y = it->y  - *currentSize * v2.y;  			
								((CVector *) ptPos)->z = it->z  - *currentSize * v2.z;  			
								ptPos += stride;
								

								++it;
								currentSize += currentSizeStep;					
							}
						}
						else
						{
							// constant size
							const CVector myV1 = *currentSize * v1;
							const CVector myV2 = *currentSize * v2;

							while (it != endIt)
							{
								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  + myV1.x;  			
								((CVector *) ptPos)->y = it->y  + myV1.y;  			
								((CVector *) ptPos)->z = it->z  + myV1.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  + myV2.x;  			
								((CVector *) ptPos)->y = it->y  + myV2.y;  			
								((CVector *) ptPos)->z = it->z  + myV2.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  - myV1.x;  			
								((CVector *) ptPos)->y = it->y  - myV1.y;  			
								((CVector *) ptPos)->z = it->z  - myV1.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  - myV2.x;  			
								((CVector *) ptPos)->y = it->y  - myV2.y;  			
								((CVector *) ptPos)->z = it->z  - myV2.z;  			
								ptPos += stride;
								

								++it;											
							}

						}
					}
					else // independant sizes
					{			
						const CVector v1 = CPSUtil::getCos((sint32) _Angle2D) * I  + CPSUtil::getSin((sint32) _Angle2D) * K;
						const CVector v2 = - CPSUtil::getSin((sint32) _Angle2D) * I + CPSUtil::getCos((sint32) _Angle2D) * K;

						float *currentSize2;
						float secondSize;
						uint32 currentSizeStep2;
						if (_SecondSize.getSizeScheme())
						{
							currentSize2 = (float *) _SecondSize.getSizeScheme()->make(_Owner, size- leftToDo, pSecondSizes, sizeof(float), toProcess, true);
							currentSizeStep2 = 1;
						}
						else
						{	
							secondSize = _SecondSize.getSize();
							currentSize2 = &secondSize;
							currentSizeStep2 = 0;
						}
						 

						while (it != endIt)
						{
							CHECK_VERTEX_BUFFER(_Vb, ptPos);
							((CVector *) ptPos)->x = it->x  - *currentSize * v1.x + *currentSize2 * v2.x;  			
							((CVector *) ptPos)->y = it->y  - *currentSize * v1.y + *currentSize2 * v2.y;  			
							((CVector *) ptPos)->z = it->z  - *currentSize * v1.z + *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(_Vb, ptPos);
							((CVector *) ptPos)->x = it->x  + *currentSize * v1.x + *currentSize2 * v2.x;  			
							((CVector *) ptPos)->y = it->y  + *currentSize * v1.y + *currentSize2 * v2.y;  			
							((CVector *) ptPos)->z = it->z  + *currentSize * v1.z + *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(_Vb, ptPos);
							((CVector *) ptPos)->x = it->x  + *currentSize * v1.x - *currentSize2 * v2.x;  			
							((CVector *) ptPos)->y = it->y  + *currentSize * v1.y - *currentSize2 * v2.y;  			
							((CVector *) ptPos)->z = it->z  + *currentSize * v1.z - *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(_Vb, ptPos);
							((CVector *) ptPos)->x = it->x  - *currentSize * v1.x - *currentSize2 * v2.x;  			
							((CVector *) ptPos)->y = it->y  - *currentSize * v1.y - *currentSize2 * v2.y;  			
							((CVector *) ptPos)->z = it->z  - *currentSize * v1.z - *currentSize2 * v2.z;
							ptPos += stride;
							++it;
							currentSize += currentSizeStep;
							currentSize2 += currentSizeStep2;
						}
						
					}
				}
				else
				{
					// perform motion, blur, we need an iterator on speed
					// independant sizes and rotation not supported for now with motion blur
					TPSAttribVector::const_iterator speedIt = _Owner->getSpeed().begin() + (size - leftToDo);

					
					const CVector v1 = I + K;
					const CVector v2 = K - I;

					
					
					CVector startV, endV, mbv1, mbv1n, mbv12, mbv2, speed;


					// norme of the v1 vect
					float n;

					const float epsilon  = 10E-5f;
					const float normEpsilon  = 10E-6f;



					
					CMatrix tMat =  _Owner->isInSystemBasis() ? getViewMat()  *  getSysMat()
															  : getViewMat();
			

					while (it != endIt)
					{
						// project the speed in the projection plane
						// this give us the v1 vect




						startV = tMat * *it ;						
						endV = tMat * (*it + *speedIt);

						
									
						if (startV.y > epsilon || endV.y > epsilon)																																		
						{	
							if (startV.y < epsilon)
							{
								if (fabsf(endV.y - startV.y) > normEpsilon)
								{
									startV = endV + (endV.y - epsilon) / (endV.y - startV.y) * (startV - endV);
								}
								startV.y = epsilon;
							}
							else if (endV.y < epsilon)
							{	
								if (fabsf(endV.y - startV.y) > normEpsilon)
								{
									endV = startV + (startV.y - epsilon) / (startV.y - endV.y) * (endV - startV);
								}
								endV.y = epsilon;
							}														

							mbv1 = (startV.x / startV.y - endV.x / endV.y) * I
											+ (startV.z / startV.y - endV.z / endV.y) * K ;


							
						
							n = mbv1.norm();

							if (n > _Threshold)
							{
								mbv1 *= _Threshold / n;
								n = _Threshold;								
							}
							
																					
							if (n > normEpsilon)												
							{															
									

								mbv1n = mbv1 / n;														
								mbv2 = *currentSize * (J ^ mbv1n);
								mbv12 = -*currentSize * mbv1n;							
								mbv1 *= *currentSize * (1 + _MotionBlurCoeff * n * n) / n;

						

								
									*(CVector *) ptPos = *it - mbv2;												
									*(CVector *) (ptPos + stride) = *it  + mbv1;  			
									*(CVector *) (ptPos + stride2) = *it + mbv2;	
									*(CVector *) (ptPos + stride3) = *it + mbv12;
								
								
									CHECK_VERTEX_BUFFER(_Vb, ptPos);
									((CVector *) ptPos)->x = it->x  - mbv2.x;  			
									((CVector *) ptPos)->y = it->y  - mbv2.y;
									((CVector *) ptPos)->z = it->z  - mbv2.z;
									
									CHECK_VERTEX_BUFFER(_Vb, ptPos + stride);
									((CVector *) (ptPos + stride))->x = it->x  + mbv1.x;  			
									((CVector *) (ptPos + stride))->y = it->y  + mbv1.y;  			
									((CVector *) (ptPos + stride))->z = it->z  + mbv1.z;  			

									CHECK_VERTEX_BUFFER(_Vb, ptPos + stride2);
									((CVector *) (ptPos + stride2))->x = it->x  + mbv2.x;  			
									((CVector *) (ptPos + stride2))->y = it->y  + mbv2.y;  			
									((CVector *) (ptPos + stride2))->z = it->z  + mbv2.z;  			


									CHECK_VERTEX_BUFFER(_Vb, ptPos + stride3);
									((CVector *) (ptPos + stride3))->x = it->x  + mbv12.x;  			
									((CVector *) (ptPos + stride3))->y = it->y  + mbv12.y;  			
									((CVector *) (ptPos + stride3))->z = it->z  + mbv12.z;  												
								
							}
							else // speed too small, we must avoid a zero divide
							{
								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								((CVector *) ptPos)->x = it->x  - *currentSize * v2.x;  			
								((CVector *) ptPos)->y = it->y  - *currentSize * v2.y;  			
								((CVector *) ptPos)->z = it->z  - *currentSize * v2.z;  			
								
								CHECK_VERTEX_BUFFER(_Vb, ptPos + stride);
								((CVector *) (ptPos + stride))->x = it->x  + *currentSize * v1.x;  			
								((CVector *) (ptPos + stride))->y = it->y  + *currentSize * v1.y;  			
								((CVector *) (ptPos + stride))->z = it->z  + *currentSize * v1.z;  			

								CHECK_VERTEX_BUFFER(_Vb, ptPos + stride2);
								((CVector *) (ptPos + stride2))->x = it->x  + *currentSize * v2.x;  			
								((CVector *) (ptPos + stride2))->y = it->y  + *currentSize * v2.y;  			
								((CVector *) (ptPos + stride2))->z = it->z  + *currentSize * v2.z;  			


								CHECK_VERTEX_BUFFER(_Vb, ptPos + stride3);
								((CVector *) (ptPos + stride3))->x = it->x  - *currentSize * v1.x;  			
								((CVector *) (ptPos + stride3))->y = it->y  - *currentSize * v1.y;  			
								((CVector *) (ptPos + stride3))->z = it->z  - *currentSize * v1.z;  							
							}
						}
						else
						{

							CHECK_VERTEX_BUFFER(_Vb, ptPos);
							((CVector *) ptPos)->x = it->x  - *currentSize * v2.x;  			
							((CVector *) ptPos)->y = it->y  - *currentSize * v2.y;  			
							((CVector *) ptPos)->z = it->z  - *currentSize * v2.z;  			
							
							CHECK_VERTEX_BUFFER(_Vb, ptPos + stride);
							((CVector *) (ptPos + stride))->x = it->x  + *currentSize * v1.x;  			
							((CVector *) (ptPos + stride))->y = it->y  + *currentSize * v1.y;  			
							((CVector *) (ptPos + stride))->z = it->z  + *currentSize * v1.z;  			

							CHECK_VERTEX_BUFFER(_Vb, ptPos + stride2);
							((CVector *) (ptPos + stride2))->x = it->x  + *currentSize * v2.x;  			
							((CVector *) (ptPos + stride2))->y = it->y  + *currentSize * v2.y;  			
							((CVector *) (ptPos + stride2))->z = it->z  + *currentSize * v2.z;  			


							CHECK_VERTEX_BUFFER(_Vb, ptPos + stride3);
							((CVector *) (ptPos + stride3))->x = it->x  - *currentSize * v1.x;  			
							((CVector *) (ptPos + stride3))->y = it->y  - *currentSize * v1.y;  			
							((CVector *) (ptPos + stride3))->z = it->z  - *currentSize * v1.z;  			
						
							

						
							/*

								CHECK_VERTEX_BUFFER(_Vb, ptPos);
								CHECK_VERTEX_BUFFER(_Vb, ptPos + stride);
								CHECK_VERTEX_BUFFER(_Vb, ptPos + stride2);
								CHECK_VERTEX_BUFFER(_Vb, ptPos + stride3);

								*(CVector *) ptPos = *it - *currentSize * v2;
								*(CVector *) (ptPos + stride) = *it  + *currentSize * v1;  			
								*(CVector *) (ptPos + stride2) = *it + *currentSize * v2;	
								*(CVector *) (ptPos + stride3) = *it - *currentSize * v1;	
							*/
						}
						

						
						

					
						ptPos += stride4;
						++it;
						++speedIt;
						currentSize += currentSizeStep;					
					}

				}
					
				
				driver->renderQuads(_Mat, 0, toProcess);
				leftToDo -= toProcess;				
		}
		while (leftToDo);
	}
	else
	{
		
		float pAngles[quadBufSize]; // the angles to use
		float *currentAngle;

		
		do
		{
			
				// restart at the beginning of the vertex buffer
				ptPos = (uint8 *) _Vb.getVertexCoordPointer();
				toProcess = leftToDo <= quadBufSize ? leftToDo : quadBufSize;

				if (_UseSizeScheme)
				{
					currentSize = (float *) _SizeScheme->make(_Owner, size - leftToDo, pSizes, sizeof(float), toProcess, true);														
				}
				else
				{
					currentSize = &_ParticleSize;
				}
				currentAngle = (float *) _Angle2DScheme->make(_Owner, size - leftToDo, pAngles, sizeof(float), toProcess, true);					

				//
				updateVbColNUVForRender(size - leftToDo, toProcess);

						
				TPSAttribVector::iterator endIt = it + toProcess;

				CVector v1, v2;

				OptFastFloorBegin();

				if (!_IndependantSizes)
				{				
					while (it != endIt)
					{
						const uint32 tabIndex = ((OptFastFloor(*currentAngle)) & 0xff) << 2;
					/*	v1 = *currentSize * (rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K);
						v2 = *currentSize * (rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K); */

						// lets avoid some ctor calls
						v1.x = *currentSize * (rotTable[tabIndex] * I.x + rotTable[tabIndex + 1] * K.x);
						v1.y = *currentSize * (rotTable[tabIndex] * I.y + rotTable[tabIndex + 1] * K.y);
						v1.z = *currentSize * (rotTable[tabIndex] * I.z + rotTable[tabIndex + 1] * K.z);

						v2.x = *currentSize * (rotTable[tabIndex + 2] * I.x + rotTable[tabIndex + 3] * K.x);
						v2.y = *currentSize * (rotTable[tabIndex + 2] * I.y + rotTable[tabIndex + 3] * K.y);
						v2.z = *currentSize * (rotTable[tabIndex + 2] * I.z + rotTable[tabIndex + 3] * K.z);
						
						CHECK_VERTEX_BUFFER(_Vb, ptPos);
						CHECK_VERTEX_BUFFER(_Vb, ptPos + stride);
						CHECK_VERTEX_BUFFER(_Vb, ptPos + stride2);
						CHECK_VERTEX_BUFFER(_Vb, ptPos + stride3);

						/*
						*(CVector *) ptPos = *it  + v1;  			
						*(CVector *) (ptPos + stride) = *it + v2;	
						*(CVector *) (ptPos + stride2) = *it - v1;
						*(CVector *) (ptPos + stride3) = *it - v2;	 */



						((CVector *) ptPos)->x  = it->x  + v1.x;		
						((CVector *) ptPos)->y  = it->y  + v1.y;
						((CVector *) ptPos)->z = it->z  + v1.z;  			

						ptPos += stride;

						((CVector *) ptPos)->x  = it->x  + v2.x;		
						((CVector *) ptPos)->y  = it->y  + v2.y;
						((CVector *) ptPos)->z = it->z  + v2.z;  			

						ptPos += stride;

						((CVector *) ptPos)->x  = it->x  - v1.x;		
						((CVector *) ptPos)->y  = it->y  - v1.y;
						((CVector *) ptPos)->z = it->z  - v1.z;  			

						ptPos += stride;

						((CVector *) ptPos)->x  = it->x  - v2.x;		
						((CVector *) ptPos)->y  = it->y  - v2.y;
						((CVector *) ptPos)->z = it->z  - v2.z;  			

						ptPos += stride;

						
						++it;
						currentSize += currentSizeStep;
						++currentAngle;					
					}
				}
				else // independant size, and non-constant rotation
				{
					
					float *currentSize2;
					float secondSize;
					uint32 currentSizeStep2;
					if (_SecondSize.getSizeScheme())
					{
						currentSize2 = (float *) _SecondSize.getSizeScheme()->make(_Owner, size- leftToDo, pSecondSizes, sizeof(float), toProcess, true);
						currentSizeStep2 = 1;
					}
					else
					{	
						secondSize = _SecondSize.getSize();
						currentSize2 = &secondSize;
						currentSizeStep2 = 0;
					}

					float cosAngle, sinAngle;

					while (it != endIt)
					{
						cosAngle = CPSUtil::getCos((sint32) *currentAngle);
						sinAngle = CPSUtil::getSin((sint32) *currentAngle);
						v1 = cosAngle * I  + sinAngle * K;
						v2 = - sinAngle * I + cosAngle * K;

						CHECK_VERTEX_BUFFER(_Vb, ptPos);
						((CVector *) ptPos)->x = it->x  - *currentSize * v1.x + *currentSize2 * v2.x;  			
						((CVector *) ptPos)->y = it->y  - *currentSize * v1.y + *currentSize2 * v2.y;  			
						((CVector *) ptPos)->z = it->z  - *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(_Vb, ptPos);
						((CVector *) ptPos)->x = it->x  + *currentSize * v1.x + *currentSize2 * v2.x;  			
						((CVector *) ptPos)->y = it->y  + *currentSize * v1.y + *currentSize2 * v2.y;  			
						((CVector *) ptPos)->z = it->z  + *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(_Vb, ptPos);
						((CVector *) ptPos)->x = it->x  + *currentSize * v1.x - *currentSize2 * v2.x;  			
						((CVector *) ptPos)->y = it->y  + *currentSize * v1.y - *currentSize2 * v2.y;  			
						((CVector *) ptPos)->z = it->z  + *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(_Vb, ptPos);
						((CVector *) ptPos)->x = it->x  - *currentSize * v1.x - *currentSize2 * v2.x;  			
						((CVector *) ptPos)->y = it->y  - *currentSize * v1.y - *currentSize2 * v2.y;  			
						((CVector *) ptPos)->z = it->z  - *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;
						++it;
						++currentAngle;
						currentSize  += currentSizeStep;
						currentSize2 += currentSizeStep2;			
					}
				}				
				OptFastFloorEnd();
					
			
				
				driver->renderQuads(_Mat, 0, toProcess);

				leftToDo -= toProcess;
		}
		while (leftToDo);



	}

	PARTICLES_CHECK_MEM;


}


void CPSFaceLookAt::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(2);

	CPSQuad::serial(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	
	f.serial(_MotionBlurCoeff);


	if (_MotionBlurCoeff != 0)
	{
		f.serial(_Threshold);
	}

	if (ver > 1)
	{
		f.serial(_IndependantSizes);
		if (_IndependantSizes)
		{
			_SecondSize.serialSizeScheme(f);
		}
	}

	if (f.isReading())
	{
		init();		
	}
}



//////////////////////////////
// fan light implementation //
//////////////////////////////


uint8 CPSFanLight::_RandomPhaseTab[32][128];

//#ifdef NL_DEBUG
	bool CPSFanLight::_RandomPhaseTabInitialized = false;
//#endif

	

// this blur a tab of bytes once
static void BlurBytesTab(const uint8 *src, uint8 *dest, uint size)
{
	std::vector<uint8> b(src, src + size);
	for (sint k = 1 ; k < (sint) (size - 1); ++k)
	{
		dest[k] = (uint8) (((uint16) b[k - 1] + (uint16) b[k + 1])>>1);
	}
}


void CPSFanLight::initFanLightPrecalc(void)
{
	// build several random tab, and linearly interpolate between l values
	float currPhase, nextPhase, phaseStep;
	for (uint l = 0; l < 32 ; l++)
	{
		nextPhase = (float) (uint8) rand();
		uint32 k = 0;
		while (k < 128)
		{
			currPhase = nextPhase;
			nextPhase = (float) (uint8) rand();			
			phaseStep = (nextPhase - currPhase) / (l + 1);

			for (uint32 m = 0; m <= l; ++m)
			{
				_RandomPhaseTab[l][k] = (uint8) currPhase;
				currPhase += phaseStep;
				++k;
				if (k >= 128) break;
			}
		}
		for (uint m = 0; m < 2 * l; ++m)
			BlurBytesTab(&_RandomPhaseTab[l][0], &_RandomPhaseTab[l][0], 128);
	}	
	//#ifdef NL_DEBUG
		_RandomPhaseTabInitialized = true;
	//#endif
}


uint32 CPSFanLight::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize() * _NbFans;
}


bool CPSFanLight::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}
bool CPSFanLight::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}


void CPSFanLight::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newColorElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
}
void CPSFanLight::deleteElement(uint32 index)
{
	deleteColorElement(index);
	deleteSizeElement(index);
	deleteAngle2DElement(index);
}



void CPSFanLight::setPhaseSpeed(float multiplier)
{
	_PhaseSpeed = 256.0f * multiplier;
}


void CPSFanLight::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;
	nlassert(_RandomPhaseTabInitialized);

	setupDriverModelMatrix();
	const CVector I = computeI();
	const CVector K = computeK();	
	const uint32 size = _Owner->getSize();

	if (!size) return; // nothing to draw
	uint8 *randomPhaseTab = &_RandomPhaseTab[_PhaseSmoothness][0];


	// compute individual colors if needed
	if (_UseColorScheme)
	{
		// we change the color at each fan light center
		_ColorScheme->make(_Owner, 0, _Vb.getColorPointer(), _Vb.getVertexSize() * (_NbFans + 1), size);	
	}

	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	




	const uint32 particleBunchSize = 512; // the size of a bunch of particle to deal with at a time


	float pSizes[particleBunchSize];
	float pAngles[particleBunchSize];

	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin()
									,endPosIt ;

	TPSAttribFloat::const_iterator timeIt = _Owner->getTime().begin();

	uint32 k; // helps to count the fans


	uint32 leftToGo = (_UseSizeScheme || _UseAngle2DScheme) ? size : 0; // useful if a size scheme is used
												 // if so, we need to deal process separatly group of particles	
	uint8 *ptVect = (uint8 *) _Vb.getVertexCoordPointer();
	const uint32 stride = _Vb.getVertexSize();

	float currentAngle;

	const float angleStep = 256.0f / _NbFans;	

	
	float *currentSizePt; // it points either the particle constant size, or a size in a table
	float *currentAnglePt; // it points either the particle constant angle, or an angle in a table

	
	const uint32 currentSizePtIncrement = _UseSizeScheme ? 1 : 0; // increment to get the next size for the size pointer. It is 0 if the size is constant
	const uint32 currentAnglePtIncrement = _UseAngle2DScheme ? 1 : 0; // increment to get the next angle for the angle pointer. It is 0 if the size is constant



	
	if (_UseSizeScheme || _UseAngle2DScheme)
	{
		endPosIt = posIt;
	}

	for (;;)
	{				

		if (_UseSizeScheme || _UseAngle2DScheme)
		{
			// there are over particleBunchSize particles left to draw, so we fill the whole size and / or angle tab
			if (leftToGo > particleBunchSize)
			{
				endPosIt = endPosIt + particleBunchSize;

				if (_UseSizeScheme)
				{
					currentSizePt  = (float *) _SizeScheme->make(_Owner, size - leftToGo, pSizes, sizeof(float), particleBunchSize, true);					
				}
				else
				{
					currentSizePt = &_ParticleSize;
				}

				if (_UseAngle2DScheme)
				{
					currentAnglePt  = (float *) _Angle2DScheme->make(_Owner, size - leftToGo, pAngles, sizeof(float), particleBunchSize, true);					
				}
				else
				{
					currentAnglePt = &_Angle2D;
				}

				leftToGo -= particleBunchSize;				
			}
			else
			{
				endPosIt = endPosIt + leftToGo;

				if (_UseSizeScheme)
				{
					currentSizePt  = (float *) (_SizeScheme->make(_Owner, size - leftToGo, pSizes, sizeof(float), leftToGo, true));
					currentSizePt = pSizes;
				}
				else
				{
					currentSizePt = &_ParticleSize;
				}

				if (_UseAngle2DScheme)
				{
					currentAnglePt = (float *) (_Angle2DScheme->make(_Owner, size - leftToGo, pAngles, sizeof(float), leftToGo, true));					
				}
				else
				{
					currentAnglePt = &_Angle2D;
				}
			
				leftToGo = 0;
			}
		
			
			
		}
		else
		{
			endPosIt = _Owner->getPos().end();
			currentSizePt = &_ParticleSize;			
			currentAnglePt = &_Angle2D;			
		}	

		for (;posIt != endPosIt; ++posIt, ++timeIt)
		{	
			
			CHECK_VERTEX_BUFFER(_Vb, ptVect);
			*(CVector *) ptVect = *posIt;
			
			// the start angle
			currentAngle = *currentAnglePt;

			const uint8 phaseAdd = (uint8) (_PhaseSpeed * (*timeIt));

			ptVect += stride;

			const float fanSize = *currentSizePt * 0.5f;
			const float moveIntensity = _MoveIntensity * fanSize;

			currentSizePt += currentSizePtIncrement;
			currentAnglePt += currentAnglePtIncrement;

			for (k = 0; k <= _NbFans; ++k)
			{
				const float fSize  = fanSize + (moveIntensity * CPSUtil::getCos(randomPhaseTab[k] + phaseAdd));
				*(CVector *) ptVect = (*posIt) + I * fSize * (CPSUtil::getCos((sint32) currentAngle))
									  + K * fSize * (CPSUtil::getSin((sint32) currentAngle));
				currentAngle += angleStep;
				ptVect += stride;
			}
		}

		if (leftToGo == 0)
		{
			break; 
		}
	}


	IDriver *driver = getDriver();

	driver->activeVertexBuffer(_Vb);	
	driver->renderTriangles(_Mat, _IndexBuffer, size * _NbFans);

	PARTICLES_CHECK_MEM;
}

void CPSFanLight::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(1);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);	
	CPSSizedParticle::serialSizeScheme(f);	
	CPSRotated2DParticle::serialAngle2DScheme(f);
	f.serial(_NbFans);
	serialMaterial(f);
	if (ver > 1)
	{
		f.serial(_PhaseSmoothness, _MoveIntensity);
		ITexture *tex = _Tex;
		f.serialPolyPtr(tex);
		if (f.isReading()) _Tex = tex ;
	}
	if (f.isReading())
	{
		init();		
	}	
}



bool CPSFanLight::completeBBox(NLMISC::CAABBox &box) const
{
	// TODO

	return false;
}



CPSFanLight::CPSFanLight(uint32 nbFans) : _IndexBuffer(NULL)
										, _NbFans(nbFans), _PhaseSpeed(256)
										, _MoveIntensity(1.5f), _PhaseSmoothness(0)
										, _Tex(NULL)
{
	nlassert(nbFans >= 3);


	init();
	_Name = std::string("FanLight");
}

CPSFanLight::~CPSFanLight()
{
	delete[] _IndexBuffer;
}

	
void CPSFanLight::setNbFans(uint32 nbFans)
{
	_NbFans = nbFans;

	resize(_Owner->getMaxSize());

	notifyOwnerMaxNumFacesChanged();
}


void CPSFanLight::resize(uint32 size)
{	
	resizeColor(size);
	resizeAngle2D(size);
	resizeSize(size);
	_Vb.setNumVertices(size * (2 + _NbFans));
		
	delete _IndexBuffer;		

	_IndexBuffer = new uint32[ size * (_NbFans + 1) * 3];
		


	// pointer on the current index to fill
	uint32 *ptIndex = _IndexBuffer;	
	
	// index of the first vertex of the current fanFilght
	uint32 currVertFan = 0;

	uint32 l; // the current fan in the current fanlight
	uint32 k; // the current fan light

	for (k = 0; k < size; ++k)
	{
		for (l = 0; l < _NbFans; ++l)
		{
			*ptIndex++ = currVertFan;
			*ptIndex++ = currVertFan + (l + 1);
			*ptIndex++ = currVertFan + (l + 2);
		}

		*ptIndex++ = currVertFan;
		*ptIndex++ = currVertFan + _NbFans;
		*ptIndex++ = currVertFan + 1;


		
		currVertFan += 2 + _NbFans;
	}

		
	for (k = 0; k < size; ++k)
	{			
		if (_Tex)
		{
			_Vb.setTexCoord(k * _NbFans + 2, 0, NLMISC::CUV(0, 0));
		}
		if (!_UseColorScheme)
		{
			_Vb.setColor(k * _NbFans + 2, _Color);			
		}		
		if (!_Tex)
		{
			for(l = 1; l <= _NbFans + 1; ++l)
			{
				_Vb.setColor(l + k * (_NbFans + 2), CRGBA(0, 0, 0));
			}
		}
		else
		{
			for(l = 1; l <= _NbFans; ++l)
			{
				_Vb.setColor(l + k * (_NbFans + 2), CRGBA(0, 0, 0));
				_Vb.setTexCoord(l + k * (_NbFans + 2), 0, NLMISC::CUV((l - 1) / (float) _NbFans, 1));
			}
		}
	}

	

}

void CPSFanLight::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);

	updateMatAndVbForColor();
}

	
void CPSFanLight::updateMatAndVbForColor(void)
{
	_Mat.setTexture(0, _Tex);
	_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | (_Tex ? CVertexBuffer::TexCoord0Flag : 0) );		
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}	
	
}


///////////////////////////////
// CPSTailDot implementation //
///////////////////////////////

// ctor
CPSTailDot::CPSTailDot(uint32 nbSegmentInTail) : _TailNbSeg(nbSegmentInTail), _ColorFading(true)
												 ,_SystemBasisEnabled(false)
{
	nlassert(_TailNbSeg <= 255);
	init();
	_Name = std::string("TailDot");
}



uint32 CPSTailDot::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize() * _TailNbSeg;
}


bool CPSTailDot::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}
bool CPSTailDot::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

void CPSTailDot::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);

	_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag );	
}
	


void CPSTailDot::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	if (
			(pass == PSBlendRender && hasTransparentFaces())
			|| (pass == PSSolidRender && hasOpaqueFaces())
			)
	{
		draw(pass == PSSolidRender);
	}
	else 
	if (pass == PSToolRender) // edition mode only
	{			
		showTool();
	}
	else if (pass == PSMotion)
	{
			PARTICLES_CHECK_MEM;
		// the number of particles
		const uint32 size = _Owner->getSize();

		if (!size) return;	

		// size of the vertices
		const uint32 vSize = _Vb.getVertexSize();

		// size of all the vertices for a tail
		const uint32 tailVSize = vSize * (_TailNbSeg + 1);
		
		// offset of the color in vertices
		const uint32 colorOff = _Vb.getColorOff();

		// offset for the head in a tail (the last vertex)
		const uint32 headOffset = vSize * _TailNbSeg;

		// address of the firstVertex

		uint8 *firstVertex = (uint8 *) _Vb.getVertexCoordPointer();

		// loop counters
		uint32 k, l;

		// pointer to the current vertex
		uint8 *currVertex;


		_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	


		TPSAttribVector::const_iterator posIt, endPosIt = _Owner->getPos().end();
			
		// decal the whole vertex buffer (but not the last vertex)
		
		// TODO : cache optimization

		if (_UseColorScheme && _ColorFading)
		{

			// the first color is set to black
			// other are modulated by the ratio of the intensity n and the intensity n + 1
			
			// the brightness increase between each segment in the tail
			const float lumiStep = 1.f / _TailNbSeg;
			float lumi = lumiStep;

			// set the first vertex to black				
			currVertex = firstVertex;
			for (k = 0; k < size; ++k)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVertex + colorOff);
				CHECK_VERTEX_BUFFER(_Vb, currVertex);


				*(CRGBA *) (currVertex + colorOff) = CRGBA::Black;
				// copy the next vertex pos
				*(CVector *) currVertex = *(CVector *) (currVertex + vSize);
				currVertex += tailVSize ;			
			}

			for (l = 1; l < _TailNbSeg; ++l)
			{
				currVertex = firstVertex + l * vSize;
				uint8 ratio = (uint8) (255.0f * lumi / (lumi +  lumiStep));
				lumi += lumiStep;			
				for (k = 0; k < size; ++k)
				{
					CHECK_VERTEX_BUFFER(_Vb, currVertex + colorOff);
					CHECK_VERTEX_BUFFER(_Vb, currVertex + vSize + colorOff);
					// get the color of the next vertex and modulate
					((CRGBA *) (currVertex + colorOff))->modulateFromui(*(CRGBA *) (currVertex + vSize + colorOff), ratio);					

					// copy the next vertex pos
					CHECK_VERTEX_BUFFER(_Vb, currVertex);
					*(CVector *) currVertex = *(CVector *) (currVertex + vSize);
					currVertex += tailVSize ;			
				}
				
			}			
		}
		else if ( !_UseColorScheme || _ColorFading)
		{		
			// we just decal the pos
			// we copy some extra pos, but we avoid 2 nested loops
			const uint32 nbVerts = (size * (_TailNbSeg + 1)) - 1;
			const uint8 *lastVert = firstVertex + nbVerts * vSize;

			for (currVertex = firstVertex; currVertex != lastVert; )
			{
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				*(CVector *) currVertex = *(CVector *) (currVertex + vSize);			
				currVertex += vSize;
			}	
		}
		else
		{		
			// we just copy the colors and the position
			// memcpy will copy some extra bytes, but it should remains fasters that 2 nested loops
			memcpy(firstVertex, firstVertex + vSize, size * tailVSize  - vSize); 		
		}




		// we fill the head of particles with the right color
		// With constant color, the setup was done in setupColor
		if (_UseColorScheme)
		{
			_ColorScheme->make(_Owner, 0,  firstVertex + headOffset + colorOff
								, tailVSize, _Owner->getSize());		
		}


		currVertex = firstVertex + headOffset;
		// If we are in the same basis than the located that hold us, we can copy coordinate directly
		if (_SystemBasisEnabled == _Owner->isInSystemBasis())
		{	
			for (posIt = _Owner->getPos().begin(); posIt != endPosIt; ++posIt)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				*(CVector *) currVertex = *posIt;
				currVertex += tailVSize;
			}
		}
		else
		{
			// the tail are not in the same basis, we need a conversion matrix
			const CMatrix &m = _SystemBasisEnabled ?   /*_Owner->getOwner()->*/getInvertedSysMat() 
													 : /*_Owner->getOwner()->*/getSysMat();
			// copy the position after transformation
			for (posIt = _Owner->getPos().begin(); posIt != endPosIt; ++posIt)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				*(CVector *) currVertex = m * (*posIt);
				currVertex += tailVSize;
			}
		}

	}

}

void CPSTailDot::draw(bool opaque)
{
	const uint32 size = _Owner->getSize();

		if (!size) return;	

	
	IDriver *driver = getDriver();
	if (_SystemBasisEnabled)
	{
		driver->setupModelMatrix(/*_Owner->getOwner()->*/getSysMat());
	}
	else
	{
		driver->setupModelMatrix(CMatrix::Identity);
	}

	
	driver->activeVertexBuffer(_Vb);		
	_Pb.setNumLine(size * _TailNbSeg);
	driver->render(_Pb, _Mat);

	PARTICLES_CHECK_MEM
}


void CPSTailDot::resize(uint32 size)
{	
	resizeColor(size);
	resizeVb(_TailNbSeg, size);	
}

void CPSTailDot::resizeVb(uint32 oldTailSize, uint32 size)
{
	
	if (!_Owner) return; // no attachement occured yet ...

	// calculate the primitive block : we got lines
		
	uint32 k, l;

	_Pb.reserveLine(_TailNbSeg * size);

	uint32 currIndex = 0;
	uint lineIndex = 0;

	for (k = 0; k < size; ++k)
	{
		for (l = 0; l < _TailNbSeg; ++l)
		{
			_Pb.setLine(lineIndex , currIndex + l, currIndex + l + 1);		
			++lineIndex;
		}

		currIndex += _TailNbSeg + 1;		
	}

	const uint32 oldSize = _Owner->getSize(); // number of used particles
	

	_Vb.setVertexFormat(CVertexBuffer::PrimaryColorFlag | CVertexBuffer::PositionFlag);
	_Vb.setNumVertices(size * (_TailNbSeg + 1));

	// fill the vertex buffer

	// particle that were present before
	// their old positions are copied

	// if the tail are not in the same basis, we need a conversion matrix

	const CMatrix *m;

	if (_SystemBasisEnabled == _Owner->isInSystemBasis())
	{
		m = &CMatrix::Identity;
	}
	else
	{
		m = _SystemBasisEnabled ?  & /*_Owner->getOwner()->*/getInvertedSysMat() 
								: &/*_Owner->getOwner()->*/getSysMat();
	}

	const TPSAttribVector &oldPos = _Owner->getPos();
	currIndex = 0;
	CVector currPos;

	for (k = 0; k < _Owner->getSize(); ++k)
	{
		currPos = *m * oldPos[k];
		for (l = 0; l <= _TailNbSeg; ++l)
		{
			_Vb.setVertexCoord( currIndex + l , currPos);			
		}
		currIndex += _TailNbSeg + 1;		
	}	
	// we don't need to setup the following vertices coordinates, they will be filled with newElement anyway...

	setupColor();
	
}

void CPSTailDot::setupColor(void)
{
	if (_Owner)
	{
		// we setup the WHOLE vb so, we need to get the max number of particles
		const uint32 size = _Owner->getMaxSize();	

		// size of the vertices
		const uint32 vSize = _Vb.getVertexSize();

		// offset of the color in vertices
		const uint32 colorOff = _Vb.getColorOff();

		// first vertex
		uint8 *firstVertex = (uint8 *) _Vb.getVertexCoordPointer();
		
		// point the current vertex color
		uint8 *currVertex =  firstVertex + colorOff;

		// loop counters
		uint k, l;


		if (_UseColorScheme || !_ColorFading)
		{
			// we can't precompute the colors, so at first, we fill all with black
			const CRGBA &color = _UseColorScheme ? CRGBA::Black : _Color;
			for (k = 0; k < size * (_TailNbSeg + 1); ++k)
			{
					CHECK_VERTEX_BUFFER(_Vb, currVertex);
					*(CRGBA *) currVertex = color;
					currVertex += vSize;
			}
		}
		else // constant color with color fading
		{
			const float lumiStep = 255.0f / _TailNbSeg;
			const  uint32 tailVSize = vSize * (_TailNbSeg + 1); // size of a whole tail in the vertex buffer
			float lumi = 0.f;
			for (l = 0; l <= _TailNbSeg; ++l)
			{
				currVertex = firstVertex + l * vSize + colorOff;
				CRGBA col;
				col.modulateFromui(_Color, (uint8) lumi);
				lumi += lumiStep;			
				for (k = 0; k < size; ++k)
				{				
					CHECK_VERTEX_BUFFER(_Vb, currVertex);
					*(CRGBA *)currVertex = col;				
					currVertex += tailVSize ;			
				}
				
			}

		}
	}	
}


void CPSTailDot::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	
		newColorElement(emitterLocated, emitterIndex);

	// if we got a constant color, everything has been setupped before

	
		const uint32 index = _Owner->getNewElementIndex(); // the index of the element to be created

		const uint32 vSize =_Vb.getVertexSize();	// vertex size
		uint8 *currVert = (uint8 *) _Vb.getVertexCoordPointer() + (index * (_TailNbSeg + 1)) * vSize; // 

		const CVector &pos = _Owner->getPos()[index];	
		if (_UseColorScheme)
		{
			// check wether the located is in the same basis than the tail
			// Otherwise, a conversion is required for the pos
			if (_SystemBasisEnabled == _Owner->isInSystemBasis())
			{
				for (uint32 k = 0; k < _TailNbSeg + 1; ++k)
				{
					CHECK_VERTEX_BUFFER(_Vb, currVert);
					CHECK_VERTEX_BUFFER(_Vb, currVert + _Vb.getColorOff());

					*(CVector *) currVert = pos;
					*(CRGBA *) (currVert + _Vb.getColorOff()) = CRGBA::Black;

					currVert += vSize; // go to next vertex
				}
			}
			else
			{
				const CMatrix &m = _SystemBasisEnabled ?   /*_Owner->getOwner()->*/getInvertedSysMat() 
												 : /*_Owner->getOwner()->*/getSysMat();
				for (uint32 k = 0; k < _TailNbSeg + 1; ++k)
				{
					CHECK_VERTEX_BUFFER(_Vb, currVert);
					*(CVector *) currVert = m * pos;
					CHECK_VERTEX_BUFFER(_Vb, currVert + _Vb.getColorOff());
					*(CRGBA *) (currVert + _Vb.getColorOff()) = CRGBA::Black;

					currVert += vSize; // go to next vertex
				}
			}

		}
		else
		{
			// color setup was done during setupColor()...

			// check wether the located is in the same basis than the tail
			// Otherwise, a conversion is required for the pos
			if (_SystemBasisEnabled == _Owner->isInSystemBasis())
			{
				for (uint32 k = 0; k < _TailNbSeg + 1; ++k)
				{
					CHECK_VERTEX_BUFFER(_Vb, currVert);
					*(CVector *) currVert = pos;			
					currVert += vSize; // go to next vertex
				}
			}
			else
			{
				const CMatrix &m = _SystemBasisEnabled ?  /*_Owner->getOwner()->*/getInvertedSysMat() 
												 : /*_Owner->getOwner()->*/getSysMat();
				for (uint32 k = 0; k < _TailNbSeg + 1; ++k)
				{
					CHECK_VERTEX_BUFFER(_Vb, currVert);
					*(CVector *) currVert = m * pos;			
					currVert += vSize; // go to next vertex
				}
			}
		}
}
		


void CPSTailDot::deleteElement(uint32 index)
{

	deleteColorElement(index);
	// we copy the last element datas to this one data
	
	// vertex size
	const uint32 vSize =_Vb.getVertexSize();


	// source
	const uint8 *currSrcVert = (uint8 *) _Vb.getVertexCoordPointer() + ((_Owner->getSize() - 1) * (_TailNbSeg + 1)) * vSize;

	// destination
	uint8 *currDestVert = (uint8 *) _Vb.getVertexCoordPointer() + (index * (_TailNbSeg + 1)) * vSize;

	// copy the vertices
	memcpy(currDestVert, currSrcVert, vSize * (_TailNbSeg + 1) );	
}



			
	
	
void CPSTailDot::setTailNbSeg(uint32 nbSeg)
{
	nlassert(nbSeg <= 255);
	uint32 oldTailSize = _TailNbSeg;
	_TailNbSeg = nbSeg;

	if (_Owner)
	{
		resizeVb(nbSeg, _Owner->getMaxSize());
	}

	notifyOwnerMaxNumFacesChanged();
}

	


void CPSTailDot::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	const uint oldTailNbSeg = _TailNbSeg;

	f.serialVersion(1);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);	
	f.serial(_TailNbSeg, _ColorFading, _SystemBasisEnabled);

	serialMaterial(f);

	// In this version we don't save the vb state, as we probably won't need it
	// we just rebuild the vb
	
	if (f.isReading())
	{
		resizeVb(oldTailNbSeg, _Owner->getMaxSize());
		// init();		
	}		

}

void CPSTailDot::updateMatAndVbForColor(void)
{
	setupColor();
}


///////////////////////////
// ribbon implementation //
///////////////////////////


// predifined shapes

const CVector CPSRibbon::Triangle[] = { CVector (0, 1, 0),
								 CVector (1, -1, 0),
								 CVector (-1, -1, 0),								 
								};

const CVector CPSRibbon::Losange[] = { CVector (0, 1.f, 0),
								 CVector (1.f, 0, 0),
								 CVector (0, -1.f, 0),
								 CVector (-1.f, 0, 0)
								};

const CVector  CPSRibbon::HeightSides[] = {  CVector(-0.5f, 1, 0)
											,CVector(0.5f, 1, 0)
											,CVector(1, 0.5f, 0)
											,CVector(1, -0.5f, 0)
											,CVector(0.5f, -1, 0)
											,CVector(-0.5f, -1, 0)
											,CVector(-1, -0.5f, 0)
											,CVector(-1, 0.5f, 0) };


const CVector CPSRibbon::Pentagram[] = { CVector(0, 1, 0), CVector(1, -1, 0)
										 , CVector(-1, 0, 0), CVector(1, 0, 0)
										 , CVector(-1, -1, 0)
										};


const uint32 CPSRibbon::NbVerticesInTriangle = sizeof(CPSRibbon::Triangle) / sizeof(CVector);


const uint32 CPSRibbon::NbVerticesInLosange = sizeof(Losange) / sizeof(CVector);


const uint32 CPSRibbon::NbVerticesInHeightSide = sizeof(CPSRibbon::HeightSides) / sizeof(CVector);

const uint32 CPSRibbon::NbVerticesInPentagram = sizeof(CPSRibbon::Pentagram) / sizeof(CVector);



// ctor
CPSRibbon::CPSRibbon(uint32 nbSegmentInTail
				  , const CVector *shape
				  , uint32 nbPointsInShape) 
				  : _ColorFading(true)
				  ,	_SystemBasisEnabled(false)	 
				  , _DyingRibbons(NULL)
				  , _ShapeNbSeg(0)
				  , _TailNbSeg(4)
				  , _NbDyingRibbons(0)
				  , _UFactor(1.f)
				  , _VFactor(1.f)
{

	init();
	nlassert(_TailNbSeg >= 2);
	setShape(shape, nbPointsInShape);
	setTailNbSeg(nbSegmentInTail); 	
	_Name = std::string("Ribbon");
}

uint32 CPSRibbon::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return (_Owner->getMaxSize() * _TailNbSeg * _ShapeNbSeg) << 1;
}


// dtor
CPSRibbon::~CPSRibbon()
{		
	delete _DyingRibbons;
}


bool CPSRibbon::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}
bool CPSRibbon::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

// querry for light persistence in ribbons
void CPSRibbon::setPersistAfterDeath(bool persist)
{
	if (persist)
	{
		if (_DyingRibbons) return;
		_DyingRibbons = new CRibbonsDesc;		
		if (_Owner)
		{
			_DyingRibbonsLifeLeft.resize(_Owner->getMaxSize());
		}
		_NbDyingRibbons = 0;
		init(*_DyingRibbons);
		setupVertexFormatNMat(*_DyingRibbons);
		resizeVb(*_DyingRibbons);
	}
	else
	{		
		delete _DyingRibbons;
		_DyingRibbons = NULL;	
	}
}


// set the number of segments in the tail		
void CPSRibbon::setTailNbSeg(uint32 nbSeg)
{	
	
	nlassert(nbSeg >= 2);
	_TailNbSeg = nbSeg;

	resizeVb(_AliveRibbons);	
	if (_DyingRibbons)
	{
		resizeVb(*_DyingRibbons);
	}

	// compute the ratio tab
	_SliceRatioTab.resize(_TailNbSeg);

	const float radiusStep = 1.f / _TailNbSeg;
	float radius = 0.f;

	for (std::vector<float>::iterator it = _SliceRatioTab.begin(); it != _SliceRatioTab.end(); ++it)
	{
		*it = radius / (radius + radiusStep);
		radius += radiusStep;
	}
	notifyOwnerMaxNumFacesChanged();
	
}

		
void CPSRibbon::setShape(const CVector *shape, uint32 nbPointsInShape)
{
	nlassert(nbPointsInShape >= 3);
	
	if (nbPointsInShape != _ShapeNbSeg)
	{
		_ShapeNbSeg = nbPointsInShape;
		_Shape.resize(nbPointsInShape);

		std::copy(shape, shape + nbPointsInShape, _Shape.begin());
		resizeVb(_AliveRibbons);
		if (_DyingRibbons)
		{
			resizeVb(*_DyingRibbons);
		}
		notifyOwnerMaxNumFacesChanged();
	}
	else
	{
		std::copy(shape, shape + nbPointsInShape, _Shape.begin());
	}	
	
}

void CPSRibbon::getShape(CVector *shape) const
{
	std::copy(_Shape.begin(), _Shape.end(), shape);
}

void CPSRibbon::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);
	
	init(_AliveRibbons);
	if (_DyingRibbons)
	{
		init(*_DyingRibbons);
	}
}


void CPSRibbon::init(CRibbonsDesc &rb)
{
	rb._Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag );	// no texture is the default
}



void CPSRibbon::decalRibbons(CRibbonsDesc &rb, const uint32 size)	
{
	if (!size) return;

	// size of the vertices
	const uint32 vSize = rb._Vb.getVertexSize();

	// size of a shape in bytes
	const uint32 sSize = vSize * _ShapeNbSeg;

	// size of all the vertices for a tail
	const uint32 tailVSize = sSize * (_TailNbSeg + 1);

	
	
	// offset of the color in vertices
	const uint32 colorOff = rb._Vb.getColorOff();

	// address of the firstVertex

	uint8 *firstVertex = (uint8 *) rb._Vb.getVertexCoordPointer();

	// loop counters
	uint32 k, l;

	// pointer to the current vertex
	uint8 *currVertex;

	

	// during the first pass, we resize each 'slice' in ribbons, and we decal them
	// the first slice is centered to the tail center


	// iterator on the center pos of each slice
	std::vector<CVector>::iterator posIt = rb._Pos.begin();
		
	
	// Iterator on each slice color, used to store slice color when fading is applied
	std::vector<CRGBA>::iterator colIt;


	if (!_UseColorScheme)
	{
		// we use colors in the vertex buffer as it
		currVertex = firstVertex;
		for (k = 0; k < size; ++k)
		{		
			std::vector<float>::const_iterator ratioIt = _SliceRatioTab.begin();
			for (; ratioIt != _SliceRatioTab.end(); ++ratioIt)
			{
				// decal the center of the slice
				
				*posIt = *(posIt + 1);
				for (l = 0; l < _ShapeNbSeg; ++l)
				{
					CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
					CHECK_VERTEX_BUFFER(rb._Vb, currVertex + sSize);

					CVector &v = *(CVector *) currVertex;				
					// points the same vertex in the next slice in the current ribbon
					const CVector &vNext = *(CVector *) (currVertex + sSize);				

					// rescale the point
					//v = *posIt + *ratioIt * (vNext - *posIt);

					v.x = posIt->x + *ratioIt * (vNext.x - posIt->x);
					v.y = posIt->y + *ratioIt * (vNext.y - posIt->y);
					v.z = posIt->z + *ratioIt * (vNext.z - posIt->z);

					// points next vertex in current slice
					currVertex += vSize;
				}
				++posIt;

			}
			// we didn't process the last slice
			currVertex += sSize;
			++posIt;
		}
	}
	else
	{
		if (!_ColorFading)
		{
			// we just decal colors in the vertex buffer
			currVertex = firstVertex;
			for (k = 0; k < size; ++k)
			{		
				std::vector<float>::const_iterator ratioIt = _SliceRatioTab.begin();
				for (; ratioIt != _SliceRatioTab.end(); ++ratioIt)
				{
					// decal the center of the slice

					CHECK_VERTEX_BUFFER(rb._Vb, currVertex + sSize + colorOff);
					const CRGBA nextCol = *(CRGBA *) (currVertex + sSize + colorOff);
					
					*posIt = *(posIt + 1);
					for (l = 0; l < _ShapeNbSeg; ++l)
					{
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex + sSize);

						CVector &v = *(CVector *) currVertex;				
						// points the same vertex in the next slice in the current ribbon
						const CVector &vNext = *(CVector *) (currVertex + sSize);				

						// rescale the point
						v = *posIt + *ratioIt * (vNext - *posIt);

						// decal color

						CHECK_VERTEX_BUFFER(rb._Vb, currVertex + colorOff);
						*(CRGBA *) (currVertex + colorOff) = nextCol ;

						// points next vertex in current slice
						currVertex += vSize;
					}
					++posIt;
				}
				// we didn't process the last slice
				currVertex += sSize;
				++posIt;
			}
		}
		else
		{
			// we must apply color fading allong the ribbon
			colIt = rb._ColTab.begin();
			currVertex = firstVertex;
			const float colorRatioStep = 255.f / (_TailNbSeg + 1);
			for (k = 0; k < size; ++k)
			{		
				std::vector<float>::const_iterator ratioIt = _SliceRatioTab.begin();
				float currColRatio = 0.f;
				CRGBA sliceCol;
				for (; ratioIt != _SliceRatioTab.end(); ++ratioIt)
				{
					// decal the center of the slice
					
					*colIt = *(colIt + 1); // decal colors
					sliceCol.modulateFromui(*colIt, (uint) currColRatio);

					*posIt = *(posIt + 1);
					for (l = 0; l < _ShapeNbSeg; ++l)
					{
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex + colorOff);
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex + sSize);

						CVector &v = *(CVector *) currVertex;				
						// points the same vertex in the next slice in the current ribbon
						const CVector &vNext = *(CVector *) (currVertex + sSize);				

						// rescale the point
						v = *posIt + *ratioIt * (vNext - *posIt);

						// copy color
						*(CRGBA *) (currVertex + colorOff) = sliceCol ;

						// points next vertex in current slice
						currVertex += vSize;
					}

					currColRatio += colorRatioStep;

					++colIt;
					++posIt;
				}
				// we didn't process the last slice
				currVertex += sSize;
				++posIt;
				++colIt;
			}
		}
	}
}


void CPSRibbon::computeLastSlice(CRibbonsDesc &rb, const uint32 size)
{	
	if (!size) return;

	// size of the vertices
	const uint32 vSize = rb._Vb.getVertexSize();

	// size of a shape in bytes
	const uint32 sSize = vSize * _ShapeNbSeg;

	// size of all the vertices for a tail
	const uint32 tailVSize = sSize * (_TailNbSeg + 1);

		
	// offset of the color in vertices
	const uint32 colorOff = rb._Vb.getColorOff();

	// address of the firstVertex

	uint8 *firstVertex = (uint8 *) rb._Vb.getVertexCoordPointer();

	// loop counters
	uint32 k, l;

	// pointer to the current vertex
	uint8 *currSliceFirstVertex, *currVertex;

	// iterator on speed
	// it is used to compute a basis for the shape to generate
	TPSAttribVector::const_iterator speedIt = _Owner->getSpeed().begin();

	// iterator on the center pos of each slice
	std::vector<CVector>::iterator posIt;

	// Iterator on each slice color, used to store slice color when fading is applied
	std::vector<CRGBA>::iterator colIt;


	// now, create the last slice for each ribbon
	posIt = rb._Pos.begin() + _TailNbSeg;

	TPSAttribVector::const_iterator  srcPosIt = _Owner->getPos().begin();
	
	// points the first vertex of the last slice in the first ribbon
	currSliceFirstVertex = firstVertex + sSize * _TailNbSeg;

	CMatrix shapeMat;
	std::vector<CVector>::const_iterator shapeIt, shapeEnd = _Shape.end();


	

	if (_ColorFading && _UseColorScheme)
	{
		 colIt = rb._ColTab.begin() + _TailNbSeg ;
	}

	k = 0;

	// check wether we are in the system or world basis

	
	CMatrix modelMat, rotModelMat;

	if (_Owner->isInSystemBasis() == _SystemBasisEnabled)
	{
		modelMat = CMatrix::Identity;
	}
	else
	{
		modelMat = _SystemBasisEnabled ?   /*_Owner->getOwner()->*/getInvertedSysMat() 
								:          /*_Owner->getOwner()->*/getSysMat();
	}

	rotModelMat = modelMat;
	rotModelMat.setPos(CVector::Null);

	 

	for (;;)
	{
				
		*posIt = modelMat * *srcPosIt;
		

		// compute a basis where to put the shape in
		shapeMat = rotModelMat * CPSUtil::buildSchmidtBasis(*speedIt);		


		currVertex = currSliceFirstVertex;	

		// compute the angles and 2x2 rot matrix, depending on the attributes

		const float scale = _UseSizeScheme ? _SizeScheme->get(_Owner, k) : _ParticleSize;
		const float ca = CPSUtil::getCos(	
											(sint32)
											(
												_UseAngle2DScheme ?
												_Angle2DScheme->get(_Owner, k) : _Angle2D
											)
										);

		const float sa = CPSUtil::getSin(
											(sint32)
											(
												_UseAngle2DScheme ?
												_Angle2DScheme->get(_Owner, k) : _Angle2D
											)
										);


		for (shapeIt = _Shape.begin(); shapeIt != shapeEnd; ++shapeIt )
		{
				
			const CVector &v = *shapeIt;
			CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
			*(CVector *) currVertex = *posIt + scale * (shapeMat * CVector( ca * v.x - sa * v.y 
																			,sa * v.x + ca * v.y
																			,v.z));
			currVertex += vSize;
		}

		if (_UseColorScheme)
		{			
			const CRGBA col = _ColorScheme->get(_Owner, k);
			currVertex -= sSize; // go back to the start of the slice
			for (l = 0; l < _ShapeNbSeg; ++l)
			{
				CHECK_VERTEX_BUFFER(rb._Vb, currVertex + colorOff);
				*(CRGBA *) (currVertex + colorOff) = col;
				currVertex += vSize;
			}
			if (_ColorFading)
			{
				*colIt = col;			
			}
		}
		// go to the first vertex of the last slice in the next ribbon
		currSliceFirstVertex += tailVSize;

		++k;

		if (k == size) break;

		++speedIt;		
		posIt += _TailNbSeg + 1;
		++srcPosIt;

		if (_UseColorScheme && _ColorFading)
		{		
			colIt += _TailNbSeg + 1;
		}
	}
}


void CPSRibbon::render(CRibbonsDesc &rb, const uint32 size)
{
	IDriver *driver = getDriver();	
	driver->activeVertexBuffer(rb._Vb);		
	driver->renderTriangles(_Mat, &rb._Ib[0], 2 * size * _TailNbSeg * _ShapeNbSeg);		
}







void CPSRibbon::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{	
	if (pass == PSMotion)
	{
		const uint32 size = _Owner->getSize();		
		decalRibbons(_AliveRibbons, size);
		computeLastSlice(_AliveRibbons, size);

		if (_DyingRibbons)
		{
			decalRibbons(*_DyingRibbons, _NbDyingRibbons);
			render(*_DyingRibbons, _NbDyingRibbons);
		}

	}
	if (
		(pass == PSBlendRender && hasTransparentFaces())
		|| (pass == PSSolidRender && hasOpaqueFaces())
		)
	{
		draw(pass == PSSolidRender);
	}
	else 
	if (pass == PSToolRender) // edition mode only
	{			
		showTool();
	}
}


void CPSRibbon::draw(bool opaque)
{	
	
	// we decal ribbon during the motion pass

	PARTICLES_CHECK_MEM;

	const uint32 size = _Owner->getSize();



   
	
	// now, draw the ribbons

   

	if (_SystemBasisEnabled)
	{
		getDriver()->setupModelMatrix(/*_Owner->getOwner()->*/getSysMat());
	}
	else
	{
		getDriver()->setupModelMatrix(CMatrix::Identity);
	}
	
	render(_AliveRibbons, size);

	// deals with dying ribbons
	if (_DyingRibbons)
	{		
		// now, check for dead ribbons
		uint k = 0;
		for (;;)
		{
			if (k >= _NbDyingRibbons)
			{
				break;
			}	
			_DyingRibbonsLifeLeft[k] ++;
			if (_DyingRibbonsLifeLeft[k] > (_TailNbSeg + 1))
			{
				copyElement(*_DyingRibbons, _NbDyingRibbons - 1, *_DyingRibbons, k); // replace this ribbon with the last
				nlassert(_NbDyingRibbons);
				_NbDyingRibbons --;
				_DyingRibbonsLifeLeft[k] = _DyingRibbonsLifeLeft[_NbDyingRibbons ];
			}
			else
			{
				++k;
			}
					
		}
	}

	PARTICLES_CHECK_MEM;
}


void CPSRibbon::resize(uint32 size)
{		
	resizeAngle2D(size);
	resizeSize(size);
	resizeColor(size);
	resizeVb(_AliveRibbons);	
	if (_DyingRibbons)
	{
		resizeVb(*_DyingRibbons);
		_DyingRibbonsLifeLeft.resize(size);
	}
}

void CPSRibbon::resizeVb(CRibbonsDesc &rb)
{
	
	if (!_Owner) return; // no attachement occured yet ...

	const uint32 size = _Owner->getMaxSize();

	// some loop counters
	uint32 k, l, m;

	// setup the index buffer			
	rb._Ib.resize(6 * _ShapeNbSeg * _TailNbSeg * size); // 2 triangles of 6 index for each slice part

	std::vector<uint32>::iterator ptCurrIndex = rb._Ib.begin();
	uint32 currIndex = 0;

	for (k = 0; k < size; ++k)
	{
		for (l = 0; l < _TailNbSeg; ++l)
		{
			
			for (m = 0; m < _ShapeNbSeg - 1; ++m)
			{

			
				*ptCurrIndex++ = currIndex;
				*ptCurrIndex++ = currIndex + _ShapeNbSeg;
				*ptCurrIndex++ = currIndex + 1;
						
				

				*ptCurrIndex++ = currIndex + 1;
				*ptCurrIndex++ = currIndex + _ShapeNbSeg;
				*ptCurrIndex++ = currIndex + _ShapeNbSeg + 1;

				
				++currIndex;
			}

			// loop around the slice

		
			*ptCurrIndex++ = currIndex;
			*ptCurrIndex++ = currIndex + _ShapeNbSeg;
			*ptCurrIndex++ = currIndex - _ShapeNbSeg + 1;
		

			*ptCurrIndex++ = currIndex - _ShapeNbSeg + 1;
			*ptCurrIndex++ = currIndex + _ShapeNbSeg;
			*ptCurrIndex++ = currIndex + 1;

			++currIndex;

		}
		currIndex += _ShapeNbSeg;
	
	}
	

	

	

	// if the tail are not in the same basis, we need a conversion matrix

	/* const CMatrix *m;

	if (_SystemBasisEnabled == _Owner->isInSystemBasis())
	{
		m = &CMatrix::Identity;
	}
	else
	{
		m = _SystemBasisEnabled ?  & _Owner->getOwner()->getInvertedSysMat() 
								: &_Owner->getOwner()->getSysMat();
	}*/

	
	// setup the vertex buffer and the pos vect



	rb._Pos.resize(size * (_TailNbSeg + 1));
	rb._Vb.setNumVertices(size * (_TailNbSeg + 1) * _ShapeNbSeg);


	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPos = _Owner->getPos().end();
	
	for ( k = 0; posIt != endPos; ++posIt, ++k)
	{
		initRibbonPos(rb, k, *posIt);
	}	
	

	setupColor(rb);
	setupUV(rb);

}

void CPSRibbon::initRibbonPos(CRibbonsDesc &rb, uint32 index, const CVector &aPos)
{
	// aPos is in the basis chosen for this particle (system or world)
	// we must expressed it in the tail basis

	CVector pos;
	if (_SystemBasisEnabled == _Owner->isInSystemBasis())
	{
		pos = aPos;
	}
	else
	{
		pos = _SystemBasisEnabled ?   getInvertedSysMat() * aPos 
								:     getSysMat()         * aPos;
	}
	
	const uint32 vSize = rb._Vb.getVertexSize();

	uint8 *currVertex = ((uint8 *) rb._Vb.getVertexCoordPointer())
						+ index * vSize * (_TailNbSeg + 1 ) * _ShapeNbSeg;

	std::vector<CVector>::iterator itPos = rb._Pos.begin() + (index * (_TailNbSeg + 1) );	

	std::fill(itPos, itPos + _TailNbSeg + 1, pos);	
	
	// at start, all pos are the same, e.g centered on the particle
	for (uint32 k = 0; k < ((_TailNbSeg + 1) * _ShapeNbSeg); ++k)
	{
		CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
		*(CVector *) currVertex = pos;		
		currVertex += vSize;
	}		
}

void CPSRibbon::setupColor(CRibbonsDesc &rb)
{
	if (!_Owner) return;
		// we setup the WHOLE vb so, we need to get the max number of particles
	const uint32 size = _Owner->getMaxSize();	

	// size of the vertices
	const uint32 vSize = rb._Vb.getVertexSize();

	// size of a slice

	const uint32 sSize = vSize * _ShapeNbSeg;

	// offset of the color in vertices
	const uint32 colorOff = rb._Vb.getColorOff();

	// first vertex
	uint8 *firstVertex = (uint8 *) rb._Vb.getVertexCoordPointer();
	
	// point the current vertex color
	uint8 *currVertex =  firstVertex + colorOff;

	// loop counters
	uint k, l, m;

	
	if (_UseColorScheme && _ColorFading)
	{
		// we must keep colors in a table in order to blend them correctly
		// If there's no color fading, color are just decaled

		rb._ColTab.resize(size * (_TailNbSeg + 1) );
		std::fill(rb._ColTab.begin(), rb._ColTab.end(), CRGBA::Black);		
	}
	else
	{
		if (rb._ColTab.size() != 0)
		{
			NLMISC::contReset(rb._ColTab);
		}
	}

	if (!_ColorFading)
	{
		
		for (k = 0; k < size * (_TailNbSeg + 1) * _ShapeNbSeg ; ++k)
		{
				CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
				*(CRGBA *) currVertex = _Color;
				currVertex += vSize;
		}
	}
	else // constant color with color fading
	{
		const float lumiStep = 255.0f / _TailNbSeg;
		const  uint32 tailVSize = sSize * (_TailNbSeg + 1); // size of a whole tail in the vertex buffer
		float lumi = 0.f;
		for (l = 0; l <= _TailNbSeg; ++l)
		{
			currVertex = firstVertex + l * sSize + colorOff;
			CRGBA col;
			col.modulateFromui(_Color, (uint8) lumi);
			lumi += lumiStep;			
			for (k = 0; k < size; ++k)
			{	
				for (m = 0; m < _ShapeNbSeg; ++m)
				{
					CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
					*(CRGBA *) currVertex = col;				
					currVertex += vSize;
				}
				currVertex += tailVSize  - sSize;			
			}
			
		}

	}




}


void CPSRibbon::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{	
	
	const uint32 index = _Owner->getNewElementIndex(); 

	initRibbonPos(_AliveRibbons, index, _Owner->getPos()[index] ); 
	newColorElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);

}
		


void CPSRibbon::deleteElement(uint32 index)
{
	deleteColorElement(index);
	deleteSizeElement(index);
	deleteAngle2DElement(index);

		// if the light must persist after death, we must copy it in the dying ribbon tab
	if (_DyingRibbons)
	{
		nlassert(_NbDyingRibbons != _Owner->getMaxSize());
		copyElement(_AliveRibbons, index, *_DyingRibbons, _NbDyingRibbons);
		_DyingRibbonsLifeLeft[_NbDyingRibbons] = 0;
		// now, we setup the last slice of the ribbon so that all points are centered
		const CVector &center = _DyingRibbons->_Pos[_NbDyingRibbons * (_TailNbSeg + 1) + _TailNbSeg];

		const uint32 vSize = _DyingRibbons->_Vb.getVertexSize(); // vertex size
		const uint32 sSize = vSize * _ShapeNbSeg;				// slice size
		const uint32 tSize = sSize * (_TailNbSeg + 1); // tail size
		const uint32 colorOff = _DyingRibbons->_Vb.getColorOff(); // offset to color

		// points the first vertex of the last slice in the ribbon that we copied
		uint8 *currVertex = (uint8 *) _DyingRibbons->_Vb.getVertexCoordPointer() 
						  + tSize * _NbDyingRibbons
						  + sSize * _TailNbSeg;

		// color must be set to black if there's a color scheme, and no color fading
		// otherwise, the color remains unchanged
		CHECK_VERTEX_BUFFER(_DyingRibbons->_Vb, currVertex + colorOff);
		const CRGBA col = _UseColorScheme && !_ColorFading ? CRGBA::Black : *(CRGBA *) (currVertex + colorOff);

		for (uint32 k = 0; k < _ShapeNbSeg; ++k)
		{
			CHECK_VERTEX_BUFFER(_DyingRibbons->_Vb, currVertex);
			CHECK_VERTEX_BUFFER(_DyingRibbons->_Vb, currVertex + colorOff);

			*(CVector *) currVertex = center;
			*(CRGBA *) (currVertex + colorOff) = col;
			currVertex += vSize;
		}

		// if there's color fading and a color scheme, color is stored in _ColTab...

		if (_UseColorScheme && _ColorFading)
		{
			_DyingRibbons->_ColTab[_NbDyingRibbons * (_TailNbSeg + 1) + _TailNbSeg] = CRGBA::Black;
		}
		

		++_NbDyingRibbons;
	}

	deleteElement(_AliveRibbons, index, _Owner->getSize());
}



void CPSRibbon::deleteElement(CRibbonsDesc &rb, uint32 index, const uint32 size)
{
	// copy the last element over this own
	copyElement(rb, size - 1 , rb, index);
}


void CPSRibbon::copyElement(CRibbonsDesc &rbSrc, uint32 srcIndex, CRibbonsDesc &rbDest, uint32 destIndex)
{
	if 	(&rbSrc == &rbDest && destIndex == srcIndex) return;

	// we copy the last element datas to this one data	
	// size of a ribbon in byte

	const uint32 ts = (_TailNbSeg + 1);

	const uint32 rSize = rbSrc._Vb.getVertexSize() * ts * _ShapeNbSeg;


	// destination
	uint8 *destVert = (uint8 *) rbDest._Vb.getVertexCoordPointer() + destIndex * rSize;

	// source
	const uint8 *srcVert = (uint8 *) rbSrc._Vb.getVertexCoordPointer() + srcIndex * rSize;


	// copy the vertices
	memcpy(destVert, srcVert, rSize );	




	// copy the pos tab;
	const std::vector<CVector>::iterator  destStart = rbDest._Pos.begin();
	const std::vector<CVector>::const_iterator  srcStart = rbSrc._Pos.begin();

	
	std::copy(srcStart + ts * srcIndex, srcStart + ts * (srcIndex + 1), destStart + ts * destIndex);

	// copy the color tab when present
	if (_UseColorScheme && _ColorFading)
	{
		const std::vector<CRGBA>::iterator  destColStart = rbDest._ColTab.begin();	
		const std::vector<CRGBA>::const_iterator  srcColStart = rbSrc._ColTab.begin();	
		std::copy(srcColStart + ts * srcIndex, srcColStart + ts * (srcIndex +1), destColStart + ts * destIndex);
	}
}



	


void CPSRibbon::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{

		
			


	f.serialVersion(1);

	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);	
	CPSSizedParticle::serialSizeScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	f.serial( _ColorFading, _SystemBasisEnabled);
	serialMaterial(f);


	// we don't save dying ribbon pos in this version
	bool drEnabled;
	ITexture *tex = NULL;

	if (!f.isReading())
	{
		drEnabled = (_DyingRibbons != NULL);
		f.serial(drEnabled);		
		f.serial(_TailNbSeg);
		tex = _Tex;
		f.serialPolyPtr(tex);
		if (_Tex)
		{
			f.serial(_UFactor, _VFactor) ;
		}
	}
	else
	{
		f.serial(drEnabled);
		setPersistAfterDeath(drEnabled);
		uint32 tnbs;
		f.serial(tnbs);
		setTailNbSeg(tnbs);
		f.serialPolyPtr(tex);
		_Tex = tex;				
		if (_Tex)
		{
			f.serial(_UFactor, _VFactor) ;
		}
	}
	

	// shape serialization
	{
		f.serialCont(_Shape);
		_ShapeNbSeg = _Shape.size();
	}
	

	// In this version we don't save the vb state, as we probably won't need it
	// we just rebuild the vb	
		if (f.isReading())
		{
			setupVertexFormatNMat(_AliveRibbons);
			resizeVb(_AliveRibbons);
			if (_DyingRibbons)
			{
				setupVertexFormatNMat(*_DyingRibbons);
				resizeVb(*_DyingRibbons);
			}		
		}
		

}

void CPSRibbon::updateMatAndVbForColor(void)
{	
	setupColor(_AliveRibbons);
	if (_DyingRibbons)
	{
			setupColor(*_DyingRibbons);
	}
/*	if (_PersistOnDeath)
	{
		setupColor(DyingRibbons);
	}*/
}

void CPSRibbon::updateMatAndVbForTexture(void)
{
	setupVertexFormatNMat(_AliveRibbons);
	
	if (_DyingRibbons)
	{
			setupVertexFormatNMat(*_DyingRibbons);
	}
	
	
	if (_Owner) 
	{		
		resizeVb(_AliveRibbons);
		if (_DyingRibbons)
		{
			resizeVb(*_DyingRibbons);
		}
	}
	
}

void  CPSRibbon::setupVertexFormatNMat(CRibbonsDesc &rb)
{
	if (_Tex == NULL) // no texture needed
	{
		rb._Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag );		
	}
	else
	{
		rb._Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag );	
		_Mat.setTexture(0, _Tex);
	}
}


void CPSRibbon::setupUV(CRibbonsDesc &rb)
{
	if (_Tex)
	{
		uint32 size = _Owner->getMaxSize(); 
		uint8 *currV = (uint8 *) rb._Vb.getVertexCoordPointer();
		// now, setup the uv's
		
		
		for (uint32 k = 0; k < size; ++k)
		{
			for (uint32 l = 0; l <= _TailNbSeg; ++l)
			{
				for (uint32 m = 0; m < _ShapeNbSeg; ++m)
				{
					*(CUV *) (currV + rb._Vb.getTexCoordOff()) = CUV(_UFactor * m / float(_ShapeNbSeg)
															   , _VFactor * l / float(_TailNbSeg + 1));
					currV += rb._Vb.getVertexSize();
				}
				
			}
		}
	}

}


////////////////////////////
// CPSFace implementation //
////////////////////////////




CPSFace::CPSFace(CSmartPtr<ITexture> tex) : CPSQuad(tex)
{   
	_Name = std::string("Face");	
}

void CPSFace::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;

	nlassert(_Owner);

	const uint32 size = _Owner->getSize();
	if (!size) return;


	setupDriverModelMatrix();

	

	const uint32 vSize = _Vb.getVertexSize();
	
	
	uint8 *currVertex ; 




	// number of left faces to draw, number of faces to process at once
	uint32 leftFaces = size, toProcess;


	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	


	IDriver *driver = getDriver();


	driver->activeVertexBuffer(_Vb);	

	float sizeBuf[quadBufSize];
	float *ptSize;
	std::vector<uint32>::const_iterator indexIt = _IndexInPrecompBasis.begin();
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt;

	// if constant size is used, the pointer points always the same float 
	uint32 ptSizeIncrement = _UseSizeScheme ? 1 : 0;


	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
			
		// rotate all precomputed basis

		// TODO : mettre la bonne valeur ici !!
		const float ellapsedTime = 0.01f;

		for (std::vector< CPlaneBasisPair >::iterator it = _PrecompBasis.begin(); it != _PrecompBasis.end(); ++it)
		{
			// not optimized at all, but this will apply to very few elements anyway...
			CMatrix mat;
			mat.rotate(CQuat(it->Axis, ellapsedTime * it->AngularVelocity));
			CVector n = mat * it->Basis.getNormal();
			it->Basis = CPlaneBasis(n);
		}

		do
		{
			
			toProcess = leftFaces > quadBufSize ? quadBufSize : leftFaces;



			currVertex = (uint8 *) _Vb.getVertexCoordPointer() ; 

			if (_UseSizeScheme)
			{				
				ptSize = (float *) (_SizeScheme->make(_Owner, size - leftFaces, sizeBuf, sizeof(float), toProcess, true));								
			}
			else
			{	
				ptSize = &_ParticleSize;			
			}
			
			
			updateVbColNUVForRender(size - leftFaces, toProcess);
			

			const uint32 stride = _Vb.getVertexSize(), stride2 = stride << 1, stride3 = stride2 + stride, stride4 = stride2 << 1;

			endPosIt = posIt + toProcess;

			/* version 1 : genere du code moyen ...*/
		/*	do		
			{
				// points the current basis
				<const CPlaneBasis &currBasis = _PrecompBasis[*indexIt].untrans;

				*(CVector *) currVertex	= *posIt + *ptSize * currBasis.X;
				*(CVector *) (currVertex + stride) = *posIt + *ptSize * currBasis.Y;
				*(CVector *) (currVertex + stride2) = *posIt - *ptSize * currBasis.X;
				*(CVector *) (currVertex + stride3) = *posIt - *ptSize * currBasis.Y;
			
				currVertex += stride4;
				++indexIt;
				++posIt;
			}
			while (posIt != endPosIt); */


				/* version 2 : here, we avoid ctor calls, but we must perform the op ourself  */
				
			do		
			{
			
				const CPlaneBasis &currBasis = _PrecompBasis[*indexIt].Basis;

				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				((CVector *) currVertex)->x = posIt->x  + *ptSize * currBasis.X.x;  			
				((CVector *) currVertex)->y = posIt->y  + *ptSize * currBasis.X.y;  			
				((CVector *) currVertex)->z = posIt->z  + *ptSize * currBasis.X.z;  			
				currVertex += stride;

				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				((CVector *) currVertex)->x = posIt->x  + *ptSize * currBasis.Y.x;  			
				((CVector *) currVertex)->y = posIt->y  + *ptSize * currBasis.Y.y;  			
				((CVector *) currVertex)->z = posIt->z  + *ptSize * currBasis.Y.z;  			
				currVertex += stride;

				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				((CVector *) currVertex)->x = posIt->x  - *ptSize * currBasis.X.x;  			
				((CVector *) currVertex)->y = posIt->y  - *ptSize * currBasis.X.y;  			
				((CVector *) currVertex)->z = posIt->z  - *ptSize * currBasis.X.z;  			
				currVertex += stride;

				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				((CVector *) currVertex)->x = posIt->x  - *ptSize * currBasis.Y.x;  			
				((CVector *) currVertex)->y = posIt->y  - *ptSize * currBasis.Y.y;  			
				((CVector *) currVertex)->z = posIt->z  - *ptSize * currBasis.Y.z;  			
				currVertex += stride;
				
			
				ptSize += ptSizeIncrement;

				++indexIt;
				++posIt;
			}
			while (posIt != endPosIt);
			

			
				
			

			driver->renderQuads(_Mat, 0, toProcess);

			leftFaces -= toProcess;

		}
		while (leftFaces);

	}
	else
	{
		// must compute each particle basis at each time

		static CPlaneBasis planeBasis[quadBufSize]; // buffer to compute each particle basis

		CPlaneBasis *currBasis;
		uint32    ptPlaneBasisIncrement = _UsePlaneBasisScheme ? 1 : 0;
		const uint32 vSize = _Vb.getVertexSize();


		do
		{
			
			toProcess = leftFaces > quadBufSize ? quadBufSize : leftFaces;



			currVertex = (uint8 *) _Vb.getVertexCoordPointer() ; 

			if (_UseSizeScheme)
			{				
				ptSize  = (float *) (_SizeScheme->make(_Owner, size - leftFaces, sizeBuf, sizeof(float), toProcess, true));								
			}
			else
			{	
				ptSize = &_ParticleSize;			
			}

			if (_UsePlaneBasisScheme)
			{
				currBasis = (CPlaneBasis *) (_PlaneBasisScheme->make(_Owner, size - leftFaces, planeBasis, sizeof(CPlaneBasis), toProcess, true));				
			}
			else
			{
				currBasis = &_PlaneBasis;
			}
			
			
			updateVbColNUVForRender(size - leftFaces, toProcess);
			

			

			endPosIt = posIt + toProcess;

					
			do		
			{
			

				// we use this instead of the + operator, because we avoid 4 constructor calls this way
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				((CVector *) currVertex)->x = posIt->x  + *ptSize * currBasis->X.x;  			
				((CVector *) currVertex)->y = posIt->y  + *ptSize * currBasis->X.y;  			
				((CVector *) currVertex)->z = posIt->z  + *ptSize * currBasis->X.z;  			
				currVertex += vSize;

				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				((CVector *) currVertex)->x = posIt->x  + *ptSize * currBasis->Y.x;  			
				((CVector *) currVertex)->y = posIt->y  + *ptSize * currBasis->Y.y;  			
				((CVector *) currVertex)->z = posIt->z  + *ptSize * currBasis->Y.z;  			
				currVertex += vSize;

				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				((CVector *) currVertex)->x = posIt->x  - *ptSize * currBasis->X.x;  			
				((CVector *) currVertex)->y = posIt->y  - *ptSize * currBasis->X.y;  			
				((CVector *) currVertex)->z = posIt->z  - *ptSize * currBasis->X.z;  			
				currVertex += vSize;

				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				((CVector *) currVertex)->x = posIt->x  - *ptSize * currBasis->Y.x;  			
				((CVector *) currVertex)->y = posIt->y  - *ptSize * currBasis->Y.y;  			
				((CVector *) currVertex)->z = posIt->z  - *ptSize * currBasis->Y.z;  			
				currVertex += vSize;
	
				ptSize += ptSizeIncrement;
			
			
				++posIt;
				currBasis += ptPlaneBasisIncrement;
			}
			while (posIt != endPosIt);
			

			
				
			

			driver->renderQuads(_Mat, 0, toProcess);

			leftFaces -= toProcess;

		}
		while (leftFaces);
	}



	PARTICLES_CHECK_MEM;
	


}

void CPSFace::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{

	f.serialVersion(1);
	CPSQuad::serial(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);

	if (f.isReading())
	{
		uint32 nbConfigurations;
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);		
		}
		hintRotateTheSame(nbConfigurations, _MinAngularVelocity, _MaxAngularVelocity);

		init();		
	}
	else	
	{				
		uint32 nbConfigurations = _PrecompBasis.size();
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);		
		}
	}


}
	
	

/// this produce a random unit vector
static CVector MakeRandomUnitVect(void)	
{
	CVector v((float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  ,(float) ((rand() % 20000) - 10000)
			  );
	v.normalize();
	return v;
}

void CPSFace::hintRotateTheSame(uint32 nbConfiguration
						, float minAngularVelocity
						, float maxAngularVelocity
					  )
{
	_MinAngularVelocity = minAngularVelocity;
	_MaxAngularVelocity = maxAngularVelocity;



	_PrecompBasis.resize(nbConfiguration);

	if (nbConfiguration)
	{
		// each precomp basis is created randomly;
		for (uint k = 0; k < nbConfiguration; ++k)
		{
			 CVector v = MakeRandomUnitVect();
			_PrecompBasis[k].Basis = CPlaneBasis(v);
			_PrecompBasis[k].Axis = MakeRandomUnitVect();
			_PrecompBasis[k].AngularVelocity = minAngularVelocity 
											   + (rand() % 20000) / 20000.f * (maxAngularVelocity - minAngularVelocity);

		}	

		// we need to do this because nbConfs may have changed
		fillIndexesInPrecompBasis();
	}
}


void CPSFace::fillIndexesInPrecompBasis(void)
{
	const uint32 nbConf = _PrecompBasis.size();
	if (_Owner)
	{
		_IndexInPrecompBasis.resize( _Owner->getMaxSize() );
	}	
	for (std::vector<uint32>::iterator it = _IndexInPrecompBasis.begin(); it != _IndexInPrecompBasis.end(); ++it)
	{
		*it = rand() % nbConf;
	}
}



void CPSFace::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	CPSQuad::newElement(emitterLocated, emitterIndex);
	newPlaneBasisElement(emitterLocated, emitterIndex);
	const uint32 nbConf = _PrecompBasis.size();
	if (nbConf) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis[_Owner->getNewElementIndex()] = rand() % nbConf;
	}
}
	
	
void CPSFace::deleteElement(uint32 index)
{
	CPSQuad::deleteElement(index);
	deletePlaneBasisElement(index);
	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
		// replace ourself by the last element...
		_IndexInPrecompBasis[index] = _IndexInPrecompBasis[_Owner->getSize() - 1];
	}
}
	
void CPSFace::resize(uint32 size)
{
	resizePlaneBasis(size);
	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis.resize(size);
	}
	CPSQuad::resize(size);

}



/////////////////////////////////
// CPSShockWave implementation //
/////////////////////////////////



CPSShockWave::CPSShockWave(uint nbSeg, float radiusCut, CSmartPtr<ITexture> tex) :  _NbSeg(nbSeg)
																					  , _RadiusCut(radiusCut)						
{
	nlassert(nbSeg > 2 && nbSeg <= 64);
	setTexture(tex);
	init();
	_Name = std::string("ShockWave");
}


uint32 CPSShockWave::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return (_Owner->getMaxSize() * _NbSeg) << 1 ;
}



bool CPSShockWave::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}
bool CPSShockWave::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}
	
void CPSShockWave::setNbSegs(uint nbSeg)
{
	nlassert(nbSeg > 2 && nbSeg <= 64);

	_NbSeg = nbSeg;
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
		notifyOwnerMaxNumFacesChanged();
	}
}


void CPSShockWave::setRadiusCut(float radiusCut)
{
	_RadiusCut = radiusCut;
	
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}

}


void CPSShockWave::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	
	f.serialVersion(1);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSTexturedParticle::serialTextureScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	serialMaterial(f);
	f.serial(_NbSeg, _RadiusCut);
	init();
	
}


void CPSShockWave::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;

	nlassert(_Owner);
	const uint32 size = _Owner->getSize();
	if (!size) return;




	const uint32 vSize = _Vb.getVertexSize();

	IDriver *driver = getDriver();

	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	


	setupDriverModelMatrix();
	driver->activeVertexBuffer(_Vb);	

	static CPlaneBasis planeBasis[shockWaveBufSize];
	float       sizes[shockWaveBufSize];
	float       angles[shockWaveBufSize];
	
	uint leftToDo  = size, toProcess;
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endIt ;
	uint8 *currVertex;
	uint k ;

	const float angleStep = 256.f / _NbSeg;
	float currAngle;


	CPlaneBasis *ptCurrBasis;
	uint32	ptCurrBasisIncrement = _UsePlaneBasisScheme ? 1 : 0;

	float *ptCurrSize;
	uint32 ptCurrSizeIncrement = _UseSizeScheme ? 1 : 0;

	float *ptCurrAngle;
	uint32 ptCurrAngleIncrement = _UseAngle2DScheme ? 1 : 0;

	CVector radVect, innerVect;

	float _RadiusRatio;

	do
	{
		currVertex = (uint8 *) _Vb.getVertexCoordPointer();

		toProcess = leftToDo > shockWaveBufSize ? shockWaveBufSize : leftToDo;

		endIt = posIt + toProcess;

		if (_UseSizeScheme)
		{
			ptCurrSize  = (float *) (_SizeScheme->make(_Owner, size - leftToDo, (void *) sizes, sizeof(float), toProcess, true));			
		}
		else
		{
			ptCurrSize = &_ParticleSize;
		}

		if (_UsePlaneBasisScheme)
		{
			ptCurrBasis  = (CPlaneBasis *) (_PlaneBasisScheme->make(_Owner, size - leftToDo, (void *) planeBasis, sizeof(CPlaneBasis), toProcess, true));			
		}
		else
		{
			ptCurrBasis = &_PlaneBasis;
		}

		if (_UseAngle2DScheme)
		{
			ptCurrAngle  = (float *) (_Angle2DScheme->make(_Owner, size - leftToDo, (void *) angles, sizeof(float), toProcess, true));			
		}
		else
		{
			ptCurrAngle = &_Angle2D;
		}

		
		if (_UseColorScheme)
		{
			_ColorScheme->makeN(_Owner, size - leftToDo, (uint8 *) _Vb.getVertexCoordPointer() + _Vb.getColorOff(), vSize, toProcess, (_NbSeg + 1) << 1 );
		}

		
		do
		{			
			currAngle = *ptCurrAngle;
			if (fabsf(*ptCurrSize) > 10E-6)
			{
				_RadiusRatio = (*ptCurrSize - _RadiusCut) / *ptCurrSize;
			}
			else
			{
				_RadiusRatio = 0.f;
			}

			for (k = 0; k <= _NbSeg; ++k)
			{
				radVect = *ptCurrSize * (CPSUtil::getCos((sint32) currAngle) * ptCurrBasis->X + CPSUtil::getSin((sint32) currAngle) * ptCurrBasis->Y);
				innerVect = _RadiusRatio * radVect;
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				* (CVector *) currVertex = *posIt + radVect;
				currVertex += vSize;
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				* (CVector *) currVertex = *posIt + innerVect;
				currVertex += vSize;

				currAngle += angleStep;				
			}
			
			++posIt;
			ptCurrBasis +=  ptCurrBasisIncrement;
			ptCurrSize  +=  ptCurrSizeIncrement;
			ptCurrAngle  +=  ptCurrAngleIncrement;
		}
		while (posIt != endIt);

	/*	printf("\n nb quad = %d \n", toProcess);
		fflush(stdout);
*/
		

		_Pb.setNumQuad(toProcess * _NbSeg);
		driver->render(_Pb, _Mat);
		leftToDo -= toProcess;

		
	}
	while (leftToDo);

	PARTICLES_CHECK_MEM;

}



bool CPSShockWave::completeBBox(NLMISC::CAABBox &box) const
{
	// TODO : implement this
	return false;
}

	
	
void CPSShockWave::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);

	updateMatAndVbForColor();
	updateMatAndVbForTexture();
}


void CPSShockWave::updateVbColNUVForRender(uint32 startIndex, uint32 size)
{
	nlassert(_Owner);

	if (!size) return;

	if (_UseColorScheme)
	{
		// compute the colors, each color is replicated n times...
		_ColorScheme->makeN(_Owner, startIndex, _Vb.getColorPointer(), _Vb.getVertexSize(), size, (_NbSeg + 1) << 1);
	}


	if (_TexGroup) // if it has a constant texture we are sure it has been setupped before...
	{	
		sint32 textureIndex[shockWaveBufSize];
		const uint32 stride = _Vb.getVertexSize(), stride2 = stride << 1;
		uint8 *currUV = (uint8 *) _Vb.getTexCoordPointer();				
		uint k;
		

		uint32 currIndexIncr;
		const sint32 *currIndex;		

		if (_UseTextureIndexScheme)
		{
			currIndex  = (sint32 *) (_TextureIndexScheme->make(_Owner, startIndex, textureIndex, sizeof(sint32), size, true));			
			currIndexIncr = 1;
		}
		else
		{	
			currIndex = &_TextureIndex;
			currIndexIncr = 0;
		}
		
		while (--size)
		{
			// for now, we don't make texture index wrapping
			const CTextureGrouped::TFourUV &uvGroup = _TexGroup->getUVQuad((uint32) *currIndex);

			k = _NbSeg + 1;

			for (k = 0; k <= _NbSeg; ++k)
			{
				
				*(CUV *) currUV = uvGroup.uv0 + CUV((float) k, 0);
				*(CUV *) (currUV + stride) = uvGroup.uv3 + CUV((float) k, 0);				
				// point the next quad
				currUV += stride2;
			}
			while (--k);

			currIndex += currIndexIncr;
		}		
	}	
}



void CPSShockWave::updateMatAndVbForColor(void)
{
	if (!_UseColorScheme)
	{
		_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);		
		_Mat.setColor(_Color);
	}
	else
	{
		_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag);
		_Mat.setColor(CRGBA::White);
	}

	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}	
}


void CPSShockWave::updateMatAndVbForTexture(void)
{
	_Mat.setTexture(0, _TexGroup ? (ITexture *) _TexGroup : (ITexture *) _Tex);	
}




void CPSShockWave::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newColorElement(emitterLocated, emitterIndex);
	newTextureIndexElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
}
	
void CPSShockWave::deleteElement(uint32 index)
{
	deleteColorElement(index);
	deleteTextureIndexElement(index);
	deleteSizeElement(index);
	deleteAngle2DElement(index);
}


void CPSShockWave::resize(uint32 aSize)
{
	resizeColor(aSize);
	resizeTextureIndex(aSize);
	resizeSize(aSize);
	resizeAngle2D(aSize);

	const uint32 size = aSize > shockWaveBufSize ? shockWaveBufSize : aSize;

	_Vb.setNumVertices((size * (_NbSeg + 1)) << 1 );
		

	_Pb.reserveQuad(size * _NbSeg);
	for (uint32 k = 0; k < size; ++k)
	{
		for (uint32 l = 0; l < _NbSeg; ++l)
		{
	
			const uint32 index = ((k * (_NbSeg + 1)) + l) << 1;
			
			
			_Pb.setQuad(l + (k * _NbSeg) , index , index + 2, index + 3, index + 1);
			
			_Vb.setTexCoord(index, 0, CUV((float) l, 0));
			_Vb.setTexCoord(index + 1, 0, CUV((float) l, 1));			
		}

		const uint32 index = ((k * (_NbSeg + 1)) + _NbSeg) << 1;

		_Vb.setTexCoord(index, 0, CUV((float) _NbSeg, 0));
		_Vb.setTexCoord(index + 1, 0, CUV((float) _NbSeg, 1));			

	}
}


////////////////////////////
// CPSMesh implementation //
////////////////////////////



/** a private function that create a dummy mesh :a cube with dummy textures
 */ 

static CMesh *CreateDummyShape(void)
{
	CMesh::CMeshBuild mb;
	CMeshBase::CMeshBaseBuild mbb;

	mb.VertexFlags = CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag;
	mb.Vertices.push_back(CVector(-.5f, -.5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, -.5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, -.5f, .5f));
	mb.Vertices.push_back(CVector(-.5f, -.5f, .5f));

	mb.Vertices.push_back(CVector(-.5f, .5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, .5f, -.5f));
	mb.Vertices.push_back(CVector(.5f, .5f, .5f));
	mb.Vertices.push_back(CVector(-.5f, .5f, .5f));

	// index for each face
	uint32 tab[] = { 4, 1, 0
					,4, 5, 1
					, 5, 2, 1
					, 5, 6, 2
					, 6, 3, 2
					, 6, 7, 3
					, 7, 0, 3
					, 7, 4, 0
					, 7, 5, 4
					, 7, 6, 5
					, 2, 0, 1
					, 2, 3, 0
					};
	for (uint k = 0; k < 6; ++k)
	{
		CMesh::CFace f;
		f.Corner[0].Vertex = tab[6 * k];
		f.Corner[0].Uvs[0] = CUV(0, 0);

		f.Corner[1].Vertex = tab[6 * k + 1];
		f.Corner[1].Uvs[0] = CUV(1, 1);

		f.Corner[2].Vertex = tab[6 * k + 2];
		f.Corner[2].Uvs[0] = CUV(0, 1);

		f.MaterialId = 0;

		mb.Faces.push_back(f);

		f.Corner[0].Vertex = tab[6 * k + 3];
		f.Corner[0].Uvs[0] = CUV(0, 0);

		f.Corner[1].Vertex = tab[6 * k + 4];
		f.Corner[1].Uvs[0] = CUV(1, 0);

		f.Corner[2].Vertex = tab[6 * k + 5];
		f.Corner[2].Uvs[0] = CUV(1, 1);

		f.MaterialId = 0;
		mb.Faces.push_back(f);
		
	}

	CMaterial mat;
	CTextureMem *tex = new CTextureMem;
	tex->makeDummy();
	mat.setTexture(0, tex);
	mat.setLighting(false);
	mat.setColor(CRGBA::White);

	mbb.Materials.push_back(mat);
	CMesh *m = new CMesh;
	m->build(mbb, mb);
	return m;
} 

void CPSMesh::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	

	f.serialVersion(1);	
	CPSParticle::serial(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);

	f.serial(_Shape);

	if (f.isReading())
	{
		invalidate();
	}


}


uint32 CPSMesh::getMaxNumFaces(void) const
{
	/// we don't draw any face ! (the meshs are drawn by the scene)
	return 0;
}



bool CPSMesh::hasTransparentFaces(void)
{
	/// we don't draw any face ! (the meshs are drawn by the scene)
	return false;
}
bool CPSMesh::hasOpaqueFaces(void)
{
	/// we don't draw any face ! (the meshs are drawn by the scene)
	return false;
}


void CPSMesh::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newPlaneBasisElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);

	nlassert(_Owner);
	nlassert(_Owner->getOwner());

	CScene *scene = _Owner->getScene();
	nlassert(scene); // the setScene method of the particle system should have been called
	//CTransformShape *instance = _Shape->createInstance(*scene);

	CTransformShape *instance = scene->createInstance(_Shape);

	if (!instance)
	{
		const std::string dummyShapeName("dummy mesh shape");
		// mesh not found ...
		IShape *is = CreateDummyShape();
		scene->getShapeBank()->add(dummyShapeName, is);
		instance = scene->createInstance(dummyShapeName);
		nlassert(instance);
	}


	instance->setTransformMode(CTransform::DirectMatrix);

	instance->hide(); // the object hasn't the right matrix yet so we hide it. It'll be shown once it is computed
	nlassert(instance);

	_Instances.insert(instance);
}
	
void CPSMesh::deleteElement(uint32 index)
{	
	deleteSizeElement(index);
	deleteAngle2DElement(index);
	deletePlaneBasisElement(index);

	// check wether CTransformShape have been instanciated
	if (_Invalidated) return;

	nlassert(_Owner);
	nlassert(_Owner->getOwner());

	CScene *scene = _Owner->getScene();
	nlassert(scene); // the setScene method of the particle system should have been called

	scene->deleteInstance(_Instances[index]);
	_Instances.remove(index);
}


void CPSMesh::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;

	nlassert(_Owner);
	const uint32 size = _Owner->getSize();
	if (!size) return;


	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	


	if (_Invalidated)
	{
		// need to rebuild all the transform shapes
		nlassert(_Owner);
		nlassert(_Owner->getOwner());

		CScene *scene = _Owner->getScene();
		nlassert(scene); // the setScene method of the particle system should have been called
	

		resize(_Owner->getMaxSize());

		for (uint k = 0; k < size; ++k)
		{
			CTransformShape *instance = scene->createInstance(_Shape);
			instance->setTransformMode(CTransform::DirectMatrix);
			instance->hide();
			_Instances.insert(instance);
		}

		_Invalidated = false;
	}
	
	float sizes[meshBufSize];
	float angles[meshBufSize];
	static CPlaneBasis planeBasis[meshBufSize];

	uint32 leftToDo = size, toProcess;


	float *ptCurrSize;
	const uint  ptCurrSizeIncrement = _UseSizeScheme ? 1 : 0;

	float *ptCurrAngle;
	const uint  ptCurrAngleIncrement = _UseAngle2DScheme ? 1 : 0;

	CPlaneBasis *ptBasis;
	const uint  ptCurrPlaneBasisIncrement = _UsePlaneBasisScheme ? 1 : 0;

	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt;


	TInstanceCont::iterator instanceIt = _Instances.begin();

	do
	{
		toProcess = leftToDo < meshBufSize ? leftToDo : meshBufSize;

		if (_UseSizeScheme)
		{
			ptCurrSize  = (float *) (_SizeScheme->make(_Owner, size - leftToDo, &sizes[0], sizeof(float), toProcess, true));			
		}
		else
		{
			ptCurrSize =& _ParticleSize;
		}

		if (_UseAngle2DScheme)
		{
			ptCurrAngle  = (float *) (_Angle2DScheme->make(_Owner, size - leftToDo, &angles[0], sizeof(float), toProcess, true));			
		}
		else
		{
			ptCurrAngle =& _Angle2D;
		}


		if (_UsePlaneBasisScheme)
		{
			ptBasis  = (CPlaneBasis *) (_PlaneBasisScheme->make(_Owner, size - leftToDo, &planeBasis[0], sizeof(CPlaneBasis), toProcess, true));			
		}
		else
		{
			ptBasis = &_PlaneBasis;
		}

		endPosIt = posIt + toProcess;


		
		CMatrix mat, tmat;
		

		// the matrix used to get in the right basis
		const CMatrix &transfo = _Owner->isInSystemBasis() ? /*_Owner->getOwner()->*/getSysMat() : CMatrix::Identity;

		do
		{
			(*instanceIt)->show();

			tmat.identity();
			mat.identity();

			tmat.translate(*posIt);

			

			mat.setRot( ptBasis->X * CPSUtil::getCos((sint32) *ptCurrAngle) + ptBasis->Y * CPSUtil::getSin((sint32) *ptCurrAngle)
						, ptBasis->X * CPSUtil::getCos((sint32) *ptCurrAngle + 64) + ptBasis->Y * CPSUtil::getSin((sint32) *ptCurrAngle + 64)
						, ptBasis->X ^ ptBasis->Y
					  );

			mat.scale(*ptCurrSize);			
			
			(*instanceIt)->setMatrix(transfo * tmat * mat);			

			++instanceIt;
			++posIt;
			ptCurrSize += ptCurrSizeIncrement;
			ptCurrAngle += ptCurrAngleIncrement;
			ptBasis += ptCurrPlaneBasisIncrement;
		}
		while (posIt != endPosIt);



		leftToDo -= toProcess;
	}
	while (leftToDo);

	PARTICLES_CHECK_MEM;
}

void CPSMesh::resize(uint32 size)
{
	resizeSize(size);
	resizeAngle2D(size);
	resizePlaneBasis(size);
	_Instances.resize(size);
}


CPSMesh::~CPSMesh()
{

	nlassert(_Owner);
	nlassert(_Owner->getOwner());

	CScene *scene = _Owner->getScene();
	nlassert(scene); // the setScene method of the particle system should have been called

	for (TInstanceCont::iterator it = _Instances.begin(); it != _Instances.end(); ++it)
	{
		scene->deleteInstance(*it);
	}
}

//////////////////////////////////////
// CPSConstraintMesh implementation //
//////////////////////////////////////

/// private : eval the number of triangles in a mesh
static uint getMeshNumTri(const CMesh &m)
{	
	uint numFaces = 0;
	for (uint k = 0 ; k < m.getNbMatrixBlock(); ++k)
	{
		for (uint l = 0; l  < m.getNbRdrPass(k); ++l)
		{
			const CPrimitiveBlock pb = m.getRdrPassPrimitiveBlock(k, l);
			numFaces += (pb.getNumLine() << 1) + pb.getNumTri() + (pb.getNumQuad() << 1);

		}
	}
	return numFaces;
}

/** This duplicate a primitive block n time in the destination primitive block
 *  This is used to draw several mesh at once
 *  For each duplication, vertices indices are shifted from the given offset
 */

static void DuplicatePrimitiveBlock(const CPrimitiveBlock &srcBlock, CPrimitiveBlock &destBlock, uint nbReplicate, uint vertOffset)
{
	PARTICLES_CHECK_MEM;

	// this must be update each time a new primitive is added
	
	// loop counters, and index of the current primitive in the dest pb
	uint k, l, index;

	// the current vertex offset.
	uint currVertOffset;


	// duplicate triangles

	uint numTri = srcBlock.getNumTri();
	destBlock.reserveTri(numTri * nbReplicate);
	
	index = 0;
	currVertOffset = 0;

	const uint32 *triPtr = srcBlock.getTriPointer();
	const uint32 *currTriPtr; // current Tri
	for (k = 0; k < nbReplicate; ++k)
	{
		currTriPtr = triPtr;
		for (l = 0; l < numTri; ++l)
		{
			destBlock.setTri(index, currTriPtr[0] + currVertOffset, currTriPtr[1] + currVertOffset, currTriPtr[2] + currVertOffset);
			currTriPtr += 3;
			++ index;
		}
		currVertOffset += vertOffset;
	}


	// duplicate quads

	uint numQuad = srcBlock.getNumQuad();
	destBlock.reserveQuad(numQuad * nbReplicate);
	
	index = 0;
	currVertOffset = 0;

	const uint32 *QuadPtr = srcBlock.getQuadPointer();
	const uint32 *currQuadPtr; // current Quad
	for (k = 0; k < nbReplicate; ++k)
	{
		currQuadPtr = QuadPtr;
		for (l = 0; l < numQuad; ++l)
		{
			destBlock.setQuad(index, currQuadPtr[0] + currVertOffset, currQuadPtr[1] + currVertOffset, currQuadPtr[2] + currVertOffset, currQuadPtr[3] + currVertOffset);
			currQuadPtr += 4;
			++ index;
		}
		currVertOffset += vertOffset;
	}

	// duplicate lines

	uint numLine = srcBlock.getNumLine();
	destBlock.reserveLine(numLine * nbReplicate);
	
	index = 0;
	currVertOffset = 0;

	const uint32 *LinePtr = srcBlock.getLinePointer();
	const uint32 *currLinePtr; // current Line
	for (k = 0; k < nbReplicate; ++k)
	{
		currLinePtr = LinePtr;
		for (l = 0; l < numLine; ++l)
		{
			destBlock.setLine(index, currLinePtr[0] + currVertOffset, currLinePtr[1] + currVertOffset);
			currLinePtr += 4;
			++ index;
		}
		currVertOffset += vertOffset;
	}

	// duplicate lines


	// TODO quad / strips duplication : (unimplemented in primitive blocks for now)

	PARTICLES_CHECK_MEM;
}



uint32 CPSConstraintMesh::getMaxNumFaces(void) const
{
//	nlassert(_ModelVb);
	return _NumFaces * _Owner->getMaxSize();
	
}



bool CPSConstraintMesh::hasTransparentFaces(void)
{
	if (!_Touched) return _HasTransparentFaces;
	/// we must construct the mesh to know wether it has transparent faces
	update();
	return _HasTransparentFaces;
}
bool CPSConstraintMesh::hasOpaqueFaces(void)
{
	if (!_Touched) return _HasOpaqueFaces;	
	update();
	return _HasOpaqueFaces;
}


void CPSConstraintMesh::setShape(const std::string &meshFileName)
{
	_MeshShapeFileName = meshFileName;
	_Touched = true;
}


void CPSConstraintMesh::update(void)
{
	if (!_Touched) return;

	clean();
	
	nlassert(_Owner->getScene());

	CScene *scene = _Owner->getScene();

	CShapeBank *sb = scene->getShapeBank();

	
	try
	{
		sb->load(_MeshShapeFileName);
	}	
	catch (NLMISC::EPathNotFound &)
	{
		// shape not found, create a dummy shape
	}

	IShape *is;

	if (!sb->isPresent(_MeshShapeFileName))
	{	
		is = CreateDummyShape();
		sb->add(std::string("dummy constraint mesh shape"), is);
	}
	else
	{
		is = sb->addRef(_MeshShapeFileName);
	}


	nlassert(dynamic_cast<CMesh *>(is));
	const CMesh &m  = * (CMesh *) is;

	/// update the number of faces
	_NumFaces = getMeshNumTri(m);
	notifyOwnerMaxNumFacesChanged();
	
	// we don't support skinning, so there must be only one matrix block
	// duplicate rendering pass

	nlassert(m.getNbMatrixBlock() == 1);  // SKINNING UNSUPPORTED

	_RdrPasses.resize(m.getNbRdrPass(0));

	const CVertexBuffer &srcVb = m.getVertexBuffer();

	_HasTransparentFaces = false;
	_HasOpaqueFaces = false;
	
	for (uint k = 0; k < m.getNbRdrPass(0); ++k)
	{
		_RdrPasses[k].Mat = m.getMaterial(m.getRdrPassMaterial(0, k));
		if (!_RdrPasses[k].Mat.getZWrite())
		{
			_HasTransparentFaces = true;
		}
		else // z-buffer write or no blending -> the face is opaque
		{
			_HasOpaqueFaces = true;
		}
			 
		DuplicatePrimitiveBlock(m.getRdrPassPrimitiveBlock(0, k), _RdrPasses[k].Pb, constraintMeshBufSize, srcVb.getNumVertices() );		
	}
	

	// make a reference to the original vbuffer

	_ModelVb = &m.getVertexBuffer();

	setupPreRotatedVb(_ModelVb->getVertexFormat());
	setupVb(_ModelVb->getVertexFormat());

	_ModelBank = sb;
	_ModelShape = is;

	_Touched = false;

	/// check for transparent faces

	
}




void CPSConstraintMesh::setupPreRotatedVb(sint vertexFlags)
{
	nlassert(vertexFlags & CVertexBuffer::PositionFlag);
	// in the prerotated Vb, we only need the normal and the position...
	if (_PrecompBasis.size())
	{
		// we need only the position and the normal (when present ...)
		_PreRotatedMeshVb.setVertexFormat(vertexFlags & (CVertexBuffer::NormalFlag | CVertexBuffer::PositionFlag));
		_PreRotatedMeshVb.setNumVertices(_ModelVb->getNumVertices() * _PrecompBasis.size() );


		const uint vSize = _ModelVb->getVertexSize();

		// duplicate position and normals
		uint index = 0;


		for (uint k = 0; k < _PrecompBasis.size(); ++k)
		{
			for (uint l = 0; l < _ModelVb->getNumVertices(); ++l)
			{
				_PreRotatedMeshVb.setVertexCoord(index
												 , *(CVector *) ((uint8 *) _ModelVb->getVertexCoordPointer() + l * vSize) 
												);

				// duplicate the normal if present
				if (vertexFlags  & CVertexBuffer::NormalFlag)
				{
					_PreRotatedMeshVb.setNormalCoord(index
													 , *(CVector *) ((uint8 *) _ModelVb->getNormalCoordPointer() + l * vSize)
													);	
				}
				++ index;
			}
		}
	}

}

	
void CPSConstraintMesh::setupVb(sint vertexFlags)
{
	PARTICLES_CHECK_MEM;

	nlassert(vertexFlags & CVertexBuffer::PositionFlag); // need the position at least

	_MeshBatchVb.setVertexFormat(vertexFlags);
	_MeshBatchVb.setNumVertices(_ModelVb->getNumVertices() * constraintMeshBufSize );
	


	// duplicate the model mesh vertices data (even position and normal, though they will be updated each time a mesh is drawn)

	const uint mSize = _MeshBatchVb.getVertexSize() * _ModelVb->getNumVertices();


	for (uint k = 0; k < constraintMeshBufSize; ++k)	
	{
		memcpy((uint8 *) _MeshBatchVb.getVertexCoordPointer() + k * mSize, _ModelVb->getVertexCoordPointer(), mSize);
	}

	PARTICLES_CHECK_MEM;
}


void CPSConstraintMesh::hintRotateTheSame(uint32 nbConfiguration
										, float minAngularVelocity
										, float maxAngularVelocity
										)
{

	// TODO : avoid code duplication with CPSFace ...
	_MinAngularVelocity = minAngularVelocity;
	_MaxAngularVelocity = maxAngularVelocity;



	_PrecompBasis.resize(nbConfiguration);

	if (nbConfiguration)
	{
		// each precomp basis is created randomly;
		for (uint k = 0; k < nbConfiguration; ++k)
		{
			 CVector v = MakeRandomUnitVect();
			_PrecompBasis[k].Basis = CPlaneBasis(v);
			_PrecompBasis[k].Axis = MakeRandomUnitVect();
			_PrecompBasis[k].AngularVelocity = minAngularVelocity 
											   + (rand() % 20000) / 20000.f * (maxAngularVelocity - minAngularVelocity);

		}	

		// we need to do this because nbConfs may have changed
		fillIndexesInPrecompBasis();
	}

	/** make sure build() was called
	 *  If this was'nt done, setupPreRotatedVb will be called during the build call
	 */


	if (_ModelVb)
	{
		setupPreRotatedVb(_ModelVb->getVertexFormat());
	}
}


void CPSConstraintMesh::fillIndexesInPrecompBasis(void)
{
	// TODO : avoid code duplication with CPSFace ...
	const uint32 nbConf = _PrecompBasis.size();
	if (_Owner)
	{
		_IndexInPrecompBasis.resize( _Owner->getMaxSize() );
	}	
	for (std::vector<uint32>::iterator it = _IndexInPrecompBasis.begin(); it != _IndexInPrecompBasis.end(); ++it)
	{
		*it = rand() % nbConf;
	}
}

	
/// serialisation. Derivers must override this, and call their parent version
void CPSConstraintMesh::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{

	f.serialVersion(1);
	if (f.isReading())
	{
		clean();
	}


	CPSParticle::serial(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);

	// prerotations ...

	if (f.isReading())
	{
		uint32 nbConfigurations;
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);		
		}
		hintRotateTheSame(nbConfigurations, _MinAngularVelocity, _MaxAngularVelocity);	
	}
	else	
	{				
		uint32 nbConfigurations = _PrecompBasis.size();
		f.serial(nbConfigurations);
		if (nbConfigurations)
		{
			f.serial(_MinAngularVelocity, _MaxAngularVelocity);		
		}
	}

	// saves the model file name, or an empty string if nothing has been set
	
	static std::string emptyStr("");

	if (!f.isReading())
	{
		if (_ModelShape)
		{
			f.serial(_MeshShapeFileName);			
		}
		else
		{
			f.serial(emptyStr); 
		}
	}
	else
	{	
		f.serial(_MeshShapeFileName);		
		_Touched = true;			
	}


}

CPSConstraintMesh::~CPSConstraintMesh() 
{
	clean();
}

void CPSConstraintMesh::clean(void)
{
	if (_ModelBank)
	{
		nlassert(_ModelShape);
		/// todo Nicolas: check this \\// (Hulud)
		if (_ModelShape)
		{
			_ModelBank->release(_ModelShape);
		}
		_ModelBank = NULL;
		_ModelShape = NULL;
		_ModelVb = NULL;
	}

	_RdrPasses.clear(); 
}

	
	

void CPSConstraintMesh::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;
	nlassert(_Owner);

	update(); // update mesh datas if needed

	if (!_ModelVb) return; // no mesh has been set for now

	const uint32 size = _Owner->getSize();

	if (!size) return;



	_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	


	// size for model vertices
	const uint vSize = _ModelVb->getVertexSize();



	const uint normalOff = _ModelVb->getVertexFormat() & CVertexBuffer::NormalFlag ? _ModelVb->getNormalOff() : 0;

	const uint nbVerticesInSource = _ModelVb->getNumVertices();

	IDriver *driver = getDriver();

	// a loop counter
	uint k ; 

	uint8 *currVertex, *currSrcVertex;

	setupDriverModelMatrix();	


	float sizes[constraintMeshBufSize];

	float *ptCurrSize;
	uint ptCurrSizeIncrement = _UseSizeScheme ? 1 : 0;

	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt;
	uint leftToDo = size, toProcess;

	driver->activeVertexBuffer(_MeshBatchVb);


	// do we use pre-rotated meshs ?
	if (_PrecompBasis.size() != 0)
	{
		// we do...

		// size for vertices in prerotated model
		const uint vpSize = _PreRotatedMeshVb.getVertexSize();

		// size of a complete prerotated model
		const uint prerotatedModelSize = vpSize * _ModelVb->getNumVertices();

		// offset of normals in vertices of the prerotated model
		const uint pNormalOff = _PreRotatedMeshVb.getVertexFormat() & CVertexBuffer::NormalFlag ? _PreRotatedMeshVb.getNormalOff() : 0;

		// TODO : mettre la bonne valeur ici !!
		const float ellapsedTime = 0.01f;

		// rotate basis
		// and compute the set of prerotated meshs that will then duplicated (with scale and translation) to create the Vb of what must be drawn

		currVertex = (uint8 *) _PreRotatedMeshVb.getVertexCoordPointer();

		for (std::vector< CPlaneBasisPair >::iterator it = _PrecompBasis.begin(); it != _PrecompBasis.end(); ++it)
		{
			// not optimized at all, but this will apply to very few elements anyway...
			CMatrix mat;
			mat.rotate(CQuat(it->Axis, ellapsedTime * it->AngularVelocity));
			CVector n = mat * it->Basis.getNormal();
			it->Basis = CPlaneBasis(n);

		
			mat.identity();
			mat.setRot(it->Basis.X, it->Basis.Y, it->Basis.X ^ it->Basis.Y);

			currSrcVertex = (uint8 *) _ModelVb->getVertexCoordPointer();

			k = nbVerticesInSource;

			// check wether we need to rotate normals as well...
			if (_ModelVb->getVertexFormat() & CVertexBuffer::NormalFlag)
			{
			
				do
				{
					CHECK_VERTEX_BUFFER(*_ModelVb, currSrcVertex);
					CHECK_VERTEX_BUFFER(*_ModelVb, currSrcVertex + normalOff);
					CHECK_VERTEX_BUFFER(_PreRotatedMeshVb, currVertex);
					CHECK_VERTEX_BUFFER(_PreRotatedMeshVb, currVertex + pNormalOff);

					* (CVector *) currVertex =  mat.mulVector(* (CVector *) currSrcVertex);
					* (CVector *) (currVertex + pNormalOff) =  mat.mulVector(* (CVector *) (currSrcVertex + normalOff) );
					currVertex += vpSize;
					currSrcVertex += vSize;
				}
				while (--k);		
			}
			else
			{
				// no normal included
				do
				{	
					
					CHECK_VERTEX_BUFFER(_PreRotatedMeshVb, currVertex);	
					CHECK_VERTEX_BUFFER(*_ModelVb, currSrcVertex);					

					* (CVector *) currVertex =  mat.mulVector(* (CVector *) currSrcVertex);
					currVertex += vpSize;
					currSrcVertex += vSize;
				}
				while (--k);	

			}
		}
		

	
		std::vector<uint32>::const_iterator indexIt = _IndexInPrecompBasis.begin();
		

		do
		{
			currVertex = (uint8 *) _MeshBatchVb.getVertexCoordPointer();

			toProcess = leftToDo  < constraintMeshBufSize ? leftToDo : constraintMeshBufSize;

			if (_UseSizeScheme)
			{
				ptCurrSize = (float *) (_SizeScheme->make(_Owner, size -leftToDo, &sizes[0], sizeof(float), toProcess, true));				
			}
			else
			{
				ptCurrSize = &_ParticleSize;
			}

			endPosIt = posIt + toProcess;

			do
			{
	
				currSrcVertex = (uint8 *) _PreRotatedMeshVb.getVertexCoordPointer() + prerotatedModelSize * *indexIt;
				k = nbVerticesInSource;

				// do we need a normal ?

				if (_ModelVb->getVertexFormat() & CVertexBuffer::NormalFlag)
				{
					// offset of normals in the prerotated mesh				
					do
					{
						CHECK_VERTEX_BUFFER(_PreRotatedMeshVb, currSrcVertex);	
						CHECK_VERTEX_BUFFER(_MeshBatchVb, currVertex);	
						CHECK_VERTEX_BUFFER(_PreRotatedMeshVb, currSrcVertex + pNormalOff);	
						CHECK_VERTEX_BUFFER(_MeshBatchVb, currVertex + normalOff);	

						// translate and resize the vertex (relatively to the mesh origin)
						*(CVector *) currVertex = *posIt + *ptCurrSize * *(CVector *) currSrcVertex ;										
						// copy the normal
						*(CVector *) (currVertex + normalOff) = *(CVector *) (currSrcVertex + pNormalOff);

						currSrcVertex += vpSize;
						currVertex += vSize;
					}
					while (--k);
				}
				else
				{
					// no normal ...

					do
					{
						CHECK_VERTEX_BUFFER(_PreRotatedMeshVb, currSrcVertex);	
						CHECK_VERTEX_BUFFER(_MeshBatchVb, currVertex);							
						
						// translate and resize the vertex (relatively to the mesh origin)
						*(CVector *) currVertex = *posIt + *ptCurrSize * *(CVector *) currSrcVertex ;																

						currSrcVertex += vpSize;
						currVertex += vSize;
					}
					while (--k);
				}

				++indexIt;
				++posIt;
				ptCurrSize += ptCurrSizeIncrement;
			}
			while (posIt != endPosIt);
			
			// render meshs : we process each rendering pass

			for (TRdrPassVect::iterator rdrPassIt = _RdrPasses.begin() 
				 ; rdrPassIt != _RdrPasses.end(); ++rdrPassIt)
			{
				// TODO : update this when new primitive will be added

				/// check wether this material has to be rendered
				if ((opaque && rdrPassIt->Mat.getZWrite()) || (!opaque && !rdrPassIt->Mat.getZWrite()))
				{
					rdrPassIt->Pb.setNumTri(rdrPassIt->Pb.capacityTri() * toProcess / constraintMeshBufSize);
					rdrPassIt->Pb.setNumQuad(rdrPassIt->Pb.capacityQuad() * toProcess / constraintMeshBufSize);
					rdrPassIt->Pb.setNumLine(rdrPassIt->Pb.capacityLine() * toProcess / constraintMeshBufSize);
				}

				driver->render(rdrPassIt->Pb, rdrPassIt->Mat);

			}

			leftToDo -= toProcess;

		}
		while (leftToDo);
	

		

	}
	else
	{
		// we don't have precomputed mesh there ... so each mesh must be transformed, which is the worst case

		
	
		CPlaneBasis planeBasis[constraintMeshBufSize];

		CPlaneBasis *ptBasis;

		uint ptBasisIncrement = _UsePlaneBasisScheme ? 1 : 0;

		CVector K; // the K vector of the current basis
		
		do
		{
			currVertex = (uint8 *) _MeshBatchVb.getVertexCoordPointer();

			toProcess = leftToDo  < constraintMeshBufSize ? leftToDo : constraintMeshBufSize;

			if (_UseSizeScheme)
			{
				ptCurrSize  = (float *) (_SizeScheme->make(_Owner, size -leftToDo, &sizes[0], sizeof(float), toProcess, true));				
			}
			else
			{
				ptCurrSize = &_ParticleSize;
			}

			if (_UsePlaneBasisScheme)
			{
				ptBasis = (CPlaneBasis *) (_PlaneBasisScheme->make(_Owner, size -leftToDo, &planeBasis[0], sizeof(CPlaneBasis), toProcess, true));				
			}
			else
			{
				ptBasis = &_PlaneBasis;
			}

			endPosIt = posIt + toProcess;

			// transfo matri & scaled transfo matrix;

			CMatrix  M, sM ;

			do
			{
	
				currSrcVertex = (uint8 *) _ModelVb->getVertexCoordPointer();
				k = nbVerticesInSource;

				


				// do we need a normal ?

				if (_ModelVb->getVertexFormat() & CVertexBuffer::NormalFlag)
				{
					M.identity();
					M.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
					sM = M;
					sM.scale(*ptCurrSize);

					// offset of normals in the prerotated mesh				
					do
					{
						CHECK_VERTEX_BUFFER(*_ModelVb, currSrcVertex);	
						CHECK_VERTEX_BUFFER(_MeshBatchVb, currVertex);	
						CHECK_VERTEX_BUFFER(*_ModelVb, currSrcVertex + normalOff);	
						CHECK_VERTEX_BUFFER(_MeshBatchVb, currVertex + normalOff);	

						// translate and resize the vertex (relatively to the mesh origin)
						*(CVector *) currVertex = *posIt + sM * *(CVector *) currSrcVertex ;										
						// copy the normal
						*(CVector *) (currVertex + normalOff) = M * *(CVector *) (currSrcVertex + normalOff);

						currSrcVertex += vSize;
						currVertex += vSize;
					}
					while (--k);
				}
				else
				{
					// no normal to transform
					sM.identity();
					sM.setRot(ptBasis->X, ptBasis->Y, ptBasis->X ^ ptBasis->Y);
					sM.scale(*ptCurrSize);

					do
					{
						CHECK_VERTEX_BUFFER(*_ModelVb, currSrcVertex);	
						CHECK_VERTEX_BUFFER(_MeshBatchVb, currVertex);	
						// translate and resize the vertex (relatively to the mesh origin)
						*(CVector *) currVertex = *posIt + sM * *(CVector *) currSrcVertex ;																

						currSrcVertex += vSize;
						currVertex += vSize;
					}
					while (--k);
				}

				
				++posIt;
				ptCurrSize += ptCurrSizeIncrement;
				ptBasis += ptBasisIncrement;
			}
			while (posIt != endPosIt);
			
			// render meshs

			// render meshs : we process each rendering pass

			for (TRdrPassVect::iterator rdrPassIt = _RdrPasses.begin() 
				 ; rdrPassIt != _RdrPasses.end(); ++rdrPassIt)
			{
				// TODO : update this when new primitive will be added

				// check wether this material has to be rendered
				if ((opaque && rdrPassIt->Mat.getZWrite()) || (!opaque && !rdrPassIt->Mat.getZWrite()))
				{
					rdrPassIt->Pb.setNumTri(rdrPassIt->Pb.capacityTri() * toProcess / constraintMeshBufSize);
					rdrPassIt->Pb.setNumQuad(rdrPassIt->Pb.capacityQuad() * toProcess / constraintMeshBufSize);
					rdrPassIt->Pb.setNumLine(rdrPassIt->Pb.capacityLine() * toProcess / constraintMeshBufSize);
				}
				driver->render(rdrPassIt->Pb, rdrPassIt->Mat);

			}

			leftToDo -= toProcess;

		}
		while (leftToDo);

	}

	PARTICLES_CHECK_MEM;
}


void CPSConstraintMesh::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newSizeElement(emitterLocated, emitterIndex);
	newPlaneBasisElement(emitterLocated, emitterIndex);
	// TODO : avoid code cuplication with CPSFace ...
	const uint32 nbConf = _PrecompBasis.size();
	if (nbConf) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis[_Owner->getNewElementIndex()] = rand() % nbConf;
	}
}
	
	
void CPSConstraintMesh::deleteElement(uint32 index)
{
	deleteSizeElement(index);
	deletePlaneBasisElement(index);
	// TODO : avoid code cuplication with CPSFace ...
	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
		// replace ourself by the last element...
		_IndexInPrecompBasis[index] = _IndexInPrecompBasis[_Owner->getSize() - 1];
	}
}
	
void CPSConstraintMesh::resize(uint32 size)
{
	resizeSize(size);
	resizePlaneBasis(size);
	// TODO : avoid code cuplication with CPSFace ...
	if (_PrecompBasis.size()) // do we use precomputed basis ?
	{
		_IndexInPrecompBasis.resize(size);
	}
}	




} // NL3D
