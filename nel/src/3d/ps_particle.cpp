/** \file ps_particle.cpp
 * <File description>
 *
 * $Id: ps_particle.cpp,v 1.9 2001/05/11 17:17:22 vizerie Exp $
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

#include "nel/3d/ps_particle.h"
#include "nel/3d/driver.h"
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/texture_grouped.h"
#include "nel/misc/common.h"

#include <algorithm>


namespace NL3D {


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
		delete _ColorScheme ;
	}
	else
	{
		_UseColorScheme = true ;
	}

	_ColorScheme = col ;

	updateMatAndVbForColor() ;
}


		
void CPSColoredParticle::setColor(NLMISC::CRGBA col)
{
	if (_UseColorScheme)
	{
		delete _ColorScheme ;
		_UseColorScheme = false ;
	}
	_Color = col ;

	updateMatAndVbForColor() ;
}

		
CPSColoredParticle::CPSColoredParticle() : _UseColorScheme(false), _Color(CRGBA(255, 255, 255))
{	
}



/// dtor

CPSColoredParticle::~CPSColoredParticle()
{
	if (_UseColorScheme)
	{
		delete _ColorScheme ;
	}
}


/// serialization

void CPSColoredParticle::serialColorScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	if (f.isReading())
	{
		if (_UseColorScheme)
		{
			delete _ColorScheme ;
		}
	}

	f.serial(_UseColorScheme) ;

	if (_UseColorScheme)
	{
		f.serialPolyPtr(_ColorScheme) ;
	}
	else
	{
		f.serial(_Color) ;
	}
}


///////////////////////////////////
// sized particle implementation //
///////////////////////////////////

void CPSSizedParticle::setSizeScheme(CPSAttribMaker<float> *size)
{
	if (_UseSizeScheme)
	{
		delete _SizeScheme ;
	}
	else
	{
		_UseSizeScheme = true ;
	}

	_SizeScheme = size ;
}


		
void CPSSizedParticle::setSize(float size)
{
	if (_UseSizeScheme)
	{
		delete _SizeScheme ;
		_UseSizeScheme = false ;
	}
	_ParticleSize = size ;
}



/// ctor : default are 0.3 sized particles
CPSSizedParticle::CPSSizedParticle() : _UseSizeScheme(false), _ParticleSize(0.3f)
{
}
	
CPSSizedParticle::~CPSSizedParticle()
{
	if (_UseSizeScheme)
	{
		delete _SizeScheme ;
	}
}


		/// serialization
void CPSSizedParticle::serialSizeScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading())
	{
		if (_UseSizeScheme)
		{
			delete _SizeScheme ;
		}
	}


	f.serial(_UseSizeScheme) ;

	if (_UseSizeScheme)
	{		
		f.serialPolyPtr(_SizeScheme) ;
	}
	else
	{
		f.serial(_ParticleSize) ;
	}	
} ;

/////////////////////////////////////
// rotated particle implementation //
/////////////////////////////////////


float CPSRotated2DParticle::_RotTable[256 * 4] ;

#ifdef NL_DEBUG
	bool CPSRotated2DParticle::_InitializedRotTab = false ;
#endif


void CPSRotated2DParticle::setAngle2DScheme(CPSAttribMaker<float> *angle2DScheme)
{
	if (_UseAngle2DScheme)
	{
		delete _Angle2DScheme ;
	}
	else
	{
		_UseAngle2DScheme = true ;
	}

	_Angle2DScheme = angle2DScheme ;
}


void CPSRotated2DParticle::setAngle2D(float angle2DScheme)
{
	if (_UseAngle2DScheme)
	{
		delete _Angle2DScheme ;
		_UseAngle2DScheme = false ;
	}
	_Angle2D = angle2DScheme ;
}

		
CPSRotated2DParticle::CPSRotated2DParticle() : _Angle2D(0), _UseAngle2DScheme(false), _Angle2DScheme(NULL)
{
}

		
CPSRotated2DParticle::~CPSRotated2DParticle()
{
	if (_UseAngle2DScheme)
	{
		delete _Angle2DScheme ;
		_UseAngle2DScheme = false ;
	}
}

		
void CPSRotated2DParticle::serialAngle2DScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading())
	{
		if (_UseAngle2DScheme)
		{
			delete _Angle2DScheme ;
		}
	}

	f.serial(_UseAngle2DScheme) ;
	if (_UseAngle2DScheme)
	{
		f.serialPolyPtr(_Angle2DScheme) ;
	}
	else
	{
		f.serial(_Angle2D) ;
	}	
}


void CPSRotated2DParticle::initRotTable(void)
{
	float *ptFloat = _RotTable ;
	for (uint32 k = 0 ; k < 256 ; ++k)
	{
		const float ca = (float) cos(k * (1.0f / 256.0f) * 2.0f * NLMISC::Pi) ;
		const float sa = (float) sin(k * (1.0f / 256.0f) * 2.0f * NLMISC::Pi) ;

		*ptFloat++ = -ca - sa ;
		*ptFloat++ = -sa + ca ;

		*ptFloat++ = ca - sa ;
		*ptFloat++ = sa + ca ;
/*
		*ptFloat++ = 2 * ca ;
		*ptFloat++ = 2 * sa ;

		*ptFloat++ = 2 * ca ;
		*ptFloat++ = 2 * sa ;

		*ptFloat++ = 2 * sa ;
		*ptFloat++ = -2 * ca ;

		*ptFloat++ = -2 * ca ;
		*ptFloat++ = -2 * sa ; */
	}

	#ifdef NL_DEBUG
		_InitializedRotTab = true ;
	#endif
}


//////////////////////////////////////
// textured particle implementation //
//////////////////////////////////////

void CPSTexturedParticle::setTextureScheme(CSmartPtr<CTextureGrouped> textureGroup, CPSAttribMaker<sint32> *animOrder)
{
	nlassert(animOrder) ;
	if (_UseTextureScheme)
	{
		delete _TextureScheme ;
	}
	else
	{
		_UseTextureScheme = true ;
	}

	_TextureScheme = animOrder ;

	_TexGroup = textureGroup ;

	_Tex = NULL ; // release any single texture if one was set before

	updateMatAndVbForTexture() ;

}

void CPSTexturedParticle::setTexture(CSmartPtr<ITexture> tex)
{
	if (_UseTextureScheme)
	{
		delete _TextureScheme ;
		_UseTextureScheme = false ;
	}
	_Tex = tex ;
	_TexGroup = NULL ; // release any grouped texture if one was set before

	updateMatAndVbForTexture() ;
}

		
CPSTexturedParticle::CPSTexturedParticle() : _UseTextureScheme(false)
{
}

CPSTexturedParticle::~CPSTexturedParticle()
{
	if (_UseTextureScheme)
	{
		delete _TextureScheme ;
	}
}

		/// serialization. We choose a different name because of multiple-inheritance

void CPSTexturedParticle::serialTextureScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading())
	{
		if (_UseTextureScheme)
		{
			delete _TextureScheme ;
		}
	}

	f.serial(_UseTextureScheme) ;
	if (_UseTextureScheme)
	{
		if (f.isReading())
		{
			CTextureGrouped *ptTex = NULL ;
			f.serialPolyPtr(ptTex) ;
			_TexGroup = ptTex ;
		}
		else
		{
			CTextureGrouped *ptTex = _TexGroup ;
			f.serialPolyPtr(ptTex) ;
		}
		f.serialPolyPtr(_TextureScheme) ;
	}
	else
	{
		if (f.isReading())
		{
			ITexture *ptTex = NULL  ;
			f.serialPolyPtr(ptTex) ;
			_Tex = ptTex ;
		}
		else
		{
			ITexture *ptTex = _Tex ;
			f.serialPolyPtr(ptTex) ;
		}

	}	
}



///////////////////////////
// CPSDot implementation //
///////////////////////////

void CPSDot::init(void)
{	
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
	_Mat.setZWrite(false) ;
	_Mat.setLighting(false) ;
	_Mat.setBlend(true) ;
	_Mat.setZFunc(CMaterial::less) ;
	
	updateMatAndVbForColor() ;
}




void CPSDot::updateMatAndVbForColor(void)
{
	if (!_UseColorScheme)
	{
		_Vb.setVertexFormat(IDRV_VF_XYZ) ;		
		_Mat.setColor(_Color) ;
	}
	else
	{
		_Vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR) ;
		_Mat.setColor(CRGBA::White) ;
	}

	if (_Owner)
	{
		resize(_Owner->getMaxSize()) ;
	}
}



void CPSDot::draw(void)
{
	
	// we create a vertex buffer that contains all the particles before to show them
	uint32 size = _Owner->getSize() ;


	if (!size) return ;


	CParticleSystem::_NbParticlesDrawn += size ;
	
	setupDriverModelMatrix() ;
	
	
	if (_UseColorScheme)
	{
		// compute the colors
		_ColorScheme->make(_Owner, 0, _Vb.getColorPointer(), _Vb.getVertexSize(), size) ;
	}
	
	TPSAttribVector::iterator it = _Owner->getPos().begin() ;
	TPSAttribVector::iterator itEnd = _Owner->getPos().end() ;
	
	
	
	uint8    *currPos = (uint8 *) _Vb.getVertexCoordPointer() ;	
	uint32 stride = _Vb.getVertexSize() ;
	do
	{

		*((CVector *) currPos) =  *it ;	
		++it  ;
		currPos += stride ;
	}
	while (it != itEnd) ;

	
	IDriver *driver = getDriver() ;
	driver->activeVertexBuffer(_Vb) ;		
	driver->renderPoints(_Mat, size) ;

}



void CPSDot::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1) ;	
	f.serialCheck((uint32) 'PSDO') ;

	CPSParticle::serial(f) ;
	CPSColoredParticle::serialColorScheme(f) ;
	if (f.isReading())
	{
		init() ;		
	}
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





	/// create the face look at by giving a texture and an optionnal color
CPSFaceLookAt::CPSFaceLookAt(CSmartPtr<ITexture> tex) : _IndexBuffer(NULL)
{
	setTexture(tex) ;
	init() ;
	// we don't init the _IndexBuffer for now, as it will be when resize is called
}




CPSFaceLookAt::~CPSFaceLookAt()
{
	delete(_IndexBuffer) ;
}

void CPSFaceLookAt::resize(uint32 size)
{
	_Vb.setNumVertices(size << 2) ;
	
	delete _IndexBuffer ;		

	_IndexBuffer = new uint32[ size * 6] ;
	

	// we precompute the uv's and the index buffer because they won't change

	// TODO : fan optimisation

	for (uint32 k = 0 ; k < size ; ++k)
	{
		_IndexBuffer[6 * k] = 4 * k + 2 ;
		_IndexBuffer[6 * k + 1] = 4 * k + 1 ;
		_IndexBuffer[6 * k + 2] = 4 * k ;		
		_IndexBuffer[6 * k + 3] = 4 * k  ;
		_IndexBuffer[6 * k + 4] = 4 * k + 3 ;
		_IndexBuffer[6 * k + 5] = 4 * k + 2;			

		_Vb.setTexCoord(k * 4, 0, CUV(0,0)) ;
		_Vb.setTexCoord(k * 4 + 1, 0, CUV(1,0)) ;
		_Vb.setTexCoord(k * 4 + 2, 0, CUV(1,1)) ;
		_Vb.setTexCoord(k * 4 + 3, 0, CUV(0,1)) ;	
	}

		
}



void CPSFaceLookAt::init(void)
{	
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
	_Mat.setZWrite(false) ;
	_Mat.setLighting(false) ;	
	_Mat.setBlend(true) ;
	_Mat.setZFunc(CMaterial::less) ;


	updateMatAndVbForColor() ;
	updateMatAndVbForTexture() ;
}


void CPSFaceLookAt::updateMatAndVbForColor(void)
{
	if (!_UseColorScheme)
	{
		_Vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0]) ;		
		_Mat.setColor(_Color) ;
	}
	else
	{
		_Vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR | IDRV_VF_UV[0]) ;
		_Mat.setColor(CRGBA::White) ;
	}

	if (_Owner)
	{
		resize(_Owner->getMaxSize()) ;
	}		
}


void CPSFaceLookAt::updateMatAndVbForTexture(void)
{	
	_Mat.setTexture(0, _UseTextureScheme ? (ITexture *) _TexGroup : (ITexture *) _Tex) ;	
}





void CPSFaceLookAt::draw(void)
{

	const uint32 faceLookAtBufferSize = 1024 ;  // size of the buffer for temporary attributes

	setupDriverModelMatrix() ;
	const CVector I = computeI() ;
	const CVector K = computeK() ;

	const uint32 size = _Owner->getSize() ;

	if (!size) return ;

	CParticleSystem::_NbParticlesDrawn += size ; // for benchmark purpose

	if (_UseColorScheme)
	{
		// compute the colors, each color is replicated 4 times
		_ColorScheme->make4(_Owner, 0, _Vb.getColorPointer(), _Vb.getVertexSize(), size) ;
	}


	if (_UseTextureScheme) // if it has a constant texture we are sure it has been setupped before...
	{
		sint32 textureIndex[faceLookAtBufferSize] ;
		uint32 texturesLeft = size ;		
		
		
		uint8 *currUV = (uint8 *) _Vb.getTexCoordPointer() ;
		const uint8 stride = _Vb.getVertexSize(), stride2 = stride << 1, stride3 = stride2 + stride, stride4 = stride2 << 1 ;
		uint k ;
		const sint32 *currIndex ;

		while (texturesLeft)
		{			

			const uint32 nbTexToProcess = texturesLeft >= faceLookAtBufferSize ? faceLookAtBufferSize : texturesLeft ;

			_TextureScheme->make(_Owner, size - texturesLeft, textureIndex, sizeof(sint32), nbTexToProcess) ;

			currIndex = &textureIndex[0] ;
			for (k = 0 ; k < nbTexToProcess ; ++k, ++currIndex)
			{
				// for now, we don't make texture index wrapping
				const CTextureGrouped::TFourUV &uvGroup = _TexGroup->getUVQuad((uint32) *currIndex) ;

				// copy the 4 uv's for this face
				*(CUV *) currUV = uvGroup.uv0 ;
				*(CUV *) (currUV + stride) = uvGroup.uv1 ;
				*(CUV *) (currUV + stride2) = uvGroup.uv2 ;
				*(CUV *) (currUV + stride3) = uvGroup.uv3 ;

				// point the next face
				currUV += stride4 ;
			}


			texturesLeft -= nbTexToProcess ;

		}
	}


	
	TPSAttribVector::iterator it = _Owner->getPos().begin() ;

	IDriver *driver = getDriver() ;


	const float *rotTable = CPSRotated2DParticle::getRotTable() ;

	// we have 4 drawing version : one with fixed size, and the other, with variable size.
	// for each the particle can be constantly rotated or have an independant rotation for each particle
	
	if (!_UseSizeScheme)
	{
		if (!_UseAngle2DScheme)
		{			
			const uint32 tabIndex = (((uint32) _Angle2D) & 0xff) << 2 ;
			const CVector v1 = _ParticleSize * (rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K) ;
			const CVector v2 = _ParticleSize * (rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K) ;
			
			uint8 *ptPos = (uint8 *) _Vb.getVertexCoordPointer() ;
			const uint32 stride = _Vb.getVertexSize(), stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride << 2 ;
			TPSAttribVector::iterator endIt = _Owner->getPos().end() ;
		
			// first version : cste rotation

			do
			{
				*(CVector *) ptPos = *it  + v1 ;  			
				*(CVector *) (ptPos + stride) = *it + v2 ;	
				*(CVector *) (ptPos + stride2) = *it - v1 ;
				*(CVector *) (ptPos + stride3) = *it - v2 ;
						

				ptPos += stride4 ;			
				++it ;
			}
			while (it != endIt) ;
		}
		else
		{
			// there's a different rotation for each particle
			// we compute the effective rotation for each 'faceLookAtBufferSize' particle and then we draw

			uint32 leftToDo = size ;

			float pAngles[faceLookAtBufferSize] ; // the angles to use

			const float *currentAngle ;
			
			uint32 l ;

			uint8 *ptPos = (uint8 *) _Vb.getVertexCoordPointer() ;
			const uint32 stride = _Vb.getVertexSize(), stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride << 2 ;			

			for (;;)
			{
				if (leftToDo < faceLookAtBufferSize)
				{
					_Angle2DScheme->make(_Owner, size - leftToDo, pAngles, sizeof(float), leftToDo) ;				
					currentAngle = &pAngles[0] ;					
					TPSAttribVector::iterator endIt = _Owner->getPos().end() ;

					while (it != endIt)
					{
						const uint32 tabIndex = (((uint32) *currentAngle) & 0xff) << 2 ;
						const CVector v1 = _ParticleSize * (rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K) ;
						const CVector v2 = _ParticleSize * (rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K) ;

						*(CVector *) ptPos = *it  + v1 ;  			
						*(CVector *) (ptPos + stride) = *it + v2 ;	
						*(CVector *) (ptPos + stride2) = *it - v1 ;
						*(CVector *) (ptPos + stride3) = *it - v2 ;								

						++it ;
						++currentAngle ;
						ptPos += stride4 ;
					}				
					break ;
				}
				else
				{
					// we compute 'faceLookAtBufferSize' new sizes at a time

					_Angle2DScheme->make(_Owner, leftToDo, pAngles, sizeof(float), faceLookAtBufferSize) ;				

					currentAngle = &pAngles[0] ;
					for (l = 0 ; l < faceLookAtBufferSize ; ++l)
					{
						const uint32 tabIndex = (((uint32) _Angle2D) & 0xff) << 2 ;
						const CVector v1 = _ParticleSize * (rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K) ;
						const CVector v2 = _ParticleSize * (rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K) ;

						*(CVector *) ptPos = *it  + v1 ;  			
						*(CVector *) (ptPos + stride) = *it + v2 ;	
						*(CVector *) (ptPos + stride2) = *it - v1 ;
						*(CVector *) (ptPos + stride3) = *it - v2 ;		
						
						++it ;
						++currentAngle ;
						ptPos += stride4 ;
					}											
					leftToDo -= faceLookAtBufferSize ;
				}
			}

		}		
	}
	else
	{

		uint32 leftToDo = size ;

		float pSizes[faceLookAtBufferSize] ; // the sizes to use
		float *currentSize ; 

	
		uint32 l ;

		uint8 *ptPos = (uint8 *) _Vb.getVertexCoordPointer() ;
		const uint32 stride = _Vb.getVertexSize(), stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride << 2 ;					
		
		if (!_UseAngle2DScheme)
		{
			// constant rotation case

			const uint32 tabIndex = (((uint32) _Angle2D) & 0xff) << 2 ;
			const CVector v1 = _ParticleSize * (rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K) ;
			const CVector v2 = _ParticleSize * (rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K) ;

			for (;;)
			{
				if (leftToDo < faceLookAtBufferSize)
				{
					_SizeScheme->make(_Owner, size - leftToDo, pSizes, sizeof(float), leftToDo) ;				

					currentSize = &pSizes[0] ;				

					TPSAttribVector::iterator endIt = _Owner->getPos().end() ;

					while (it != endIt)
					{
						*(CVector *) ptPos = *it  + *currentSize * v1 ;  			
						*(CVector *) (ptPos + stride) = *it + *currentSize * v2 ;	
						*(CVector *) (ptPos + stride2) = *it - *currentSize * v1 ;
						*(CVector *) (ptPos + stride3) = *it - *currentSize * v2 ;	

						++it ;
						++currentSize ;
						ptPos += stride4 ;
					}				
					break ;
				}
				else
				{
					// we compute 'aceLookAtBufferSize'new size at a time

					_SizeScheme->make(_Owner, size - leftToDo, pSizes, sizeof(float), faceLookAtBufferSize) ;				

					currentSize = &pSizes[0] ;
					for (l = 0 ; l < faceLookAtBufferSize ; ++l)
					{
						*(CVector *) ptPos = *it  + *currentSize * v1 ;  			
						*(CVector *) (ptPos + stride) = *it + *currentSize * v2 ;	
						*(CVector *) (ptPos + stride2) = *it - *currentSize * v1 ;
						*(CVector *) (ptPos + stride3) = *it - *currentSize * v2 ;	

						++it ;
						++currentSize ;
						ptPos += stride4 ;
					}												
					leftToDo -= faceLookAtBufferSize ;
				}
			}
		}
		else
		{
			
			float pAngles[faceLookAtBufferSize] ; // the angles to use
			float *currentAngle ;
	
			for (;;)
			{
				if (leftToDo < faceLookAtBufferSize)
				{
					// compute sizes
					_SizeScheme->make(_Owner, size - leftToDo, pSizes, sizeof(float), leftToDo) ;				
					// compute rotations
					_Angle2DScheme->make(_Owner, size - leftToDo, pAngles, sizeof(float), leftToDo) ;				

					

					currentSize = &pSizes[0] ;				
					currentAngle = &pAngles[0] ;				

					TPSAttribVector::iterator endIt = _Owner->getPos().end() ;

					while (it != endIt)
					{
						const uint32 tabIndex = (((uint32) *currentAngle) & 0xff) << 2 ;
						const CVector v1 = *currentSize * (rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K) ;
						const CVector v2 = *currentSize * (rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K) ;

						*(CVector *) ptPos = *it  + v1 ;  			
						*(CVector *) (ptPos + stride) = *it + v2 ;	
						*(CVector *) (ptPos + stride2) = *it - v1 ;
						*(CVector *) (ptPos + stride3) = *it - v2 ;		


						++it ;
						++currentSize ;
						++currentAngle ;
						ptPos += stride4 ;
					}				
					break ;
				}
				else
				{
					// we compute 'aceLookAtBufferSize'new size and angles at a time
					_SizeScheme->make(_Owner, size - leftToDo, pSizes, sizeof(float), faceLookAtBufferSize) ;				
					_Angle2DScheme->make(_Owner, size - leftToDo, pAngles, sizeof(float), faceLookAtBufferSize) ;				


					currentSize = &pSizes[0] ;
					currentAngle = &pAngles[0] ;	

					for (l = 0 ; l < faceLookAtBufferSize ; ++l)
					{
						const uint32 tabIndex = (((uint32) *currentAngle) & 0xff) << 2 ;
						const CVector v1 = *currentSize * (rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K) ;
						const CVector v2 = *currentSize * (rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K) ;

						*(CVector *) ptPos = *it  + v1 ;  			
						*(CVector *) (ptPos + stride) = *it + v2 ;	
						*(CVector *) (ptPos + stride2) = *it - v1 ;
						*(CVector *) (ptPos + stride3) = *it - v2 ;	

						++it ;
						++currentSize ;
						++currentAngle ;
						ptPos += stride4 ;
					}												
					leftToDo -= faceLookAtBufferSize ;
				}
			}
		}
	}

	driver->activeVertexBuffer(_Vb) ;	
	driver->renderTriangles(_Mat, _IndexBuffer, size * 2) ;

}


void CPSFaceLookAt::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{

	f.serialCheck((uint32) 'PFLA') ;	
	
			
	CPSParticle::serial(f) ;
	CPSSizedParticle::serialSizeScheme(f) ;
	CPSColoredParticle::serialColorScheme(f) ;
	CPSRotated2DParticle::serialAngle2DScheme(f) ;
	CPSTexturedParticle::serialTextureScheme(f) ;

	if (f.isReading())
	{
		init() ;		
	}
}


bool CPSFaceLookAt::completeBBox(NLMISC::CAABBox &box) const  
{ 
	if (!_UseColorScheme)
	{
		CPSUtil::addRadiusToAABBox(box, _ParticleSize) ;
	}
	else
	{
		CPSUtil::addRadiusToAABBox(box, _SizeScheme->getMaxValue()) ;
	}


	return true  ;	
}

//////////////////////////////
// fan light implementation //
//////////////////////////////


uint8 CPSFanLight::_RandomPhaseTab[128] ;

#ifdef NL_DEBUG
	bool CPSFanLight::_RandomPhaseTabInitialized = false ;
#endif

	
void CPSFanLight::initFanLightPrecalc(void)
{
	for (uint32 k = 0 ; k < 128 ; ++k)
	{
		_RandomPhaseTab[k] = (uint8) rand() ;
	}
	#ifdef NL_DEBUG
		_RandomPhaseTabInitialized = true ;
	#endif
}



void CPSFanLight::setPhaseSpeed(float multiplier)
{
	_PhaseSpeed = 256.0f * multiplier ;
}


void CPSFanLight::draw(void)
{
	nlassert(_RandomPhaseTabInitialized) ;

	setupDriverModelMatrix() ;
	const CVector I = computeI() ;
	const CVector K = computeK() ;

	const uint32 size = _Owner->getSize() ;

	if (!size) return ; // nothing to draw



	// compute individual colors if needed
	if (_UseColorScheme)
	{
		// we change the color at each fan light center
		_ColorScheme->make(_Owner, 0, _Vb.getColorPointer(), _Vb.getVertexSize() * (_NbFans + 1), size) ;	
	}

	CParticleSystem::_NbParticlesDrawn += size ; // for benchmark purpose



	const uint32 particleBunchSize = 512 ; // the size of a bunch of particle to deal with at a time


	float pSizes[particleBunchSize] ;
	float pAngles[particleBunchSize] ;

	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin()
									,endPosIt  ;

	TPSAttribFloat::const_iterator timeIt = _Owner->getTime().begin() ;

	uint32 k ; // helps to count the fans


	uint32 leftToGo = (_UseSizeScheme || _UseAngle2DScheme) ? size : 0 ; // useful if a size scheme is used
												 // if so, we need to deal process separatly group of particles	
	uint8 *ptVect = (uint8 *) _Vb.getVertexCoordPointer() ;
	const uint32 stride = _Vb.getVertexSize() ;

	float currentAngle ;

	const float angleStep = 256.0f / _NbFans ;	

	
	float *currentSizePt ; // it points either the particle constant size, or a size in a table
	float *currentAnglePt ; // it points either the particle constant angle, or an angle in a table

	
	const uint32 currentSizePtIncrement = _UseSizeScheme ? 1 : 0 ; // increment to get the next size for the size pointer. It is 0 if the size is constant
	const uint32 currentAnglePtIncrement = _UseAngle2DScheme ? 1 : 0 ; // increment to get the next angle for the angle pointer. It is 0 if the size is constant



	
	if (_UseSizeScheme || _UseAngle2DScheme)
	{
		endPosIt = posIt ;
	}

	for (;;)
	{				

		if (_UseSizeScheme || _UseAngle2DScheme)
		{
			// there are over particleBunchSize particles left to draw, so we fill the whole size and / or angle tab
			if (leftToGo > particleBunchSize)
			{
				endPosIt = endPosIt + particleBunchSize ;

				if (_UseSizeScheme)
				{
					_SizeScheme->make(_Owner, size - leftToGo, pSizes, sizeof(float), particleBunchSize) ;
					currentSizePt = pSizes ;
				}
				else
				{
					currentSizePt = &_ParticleSize ;
				}

				if (_UseAngle2DScheme)
				{
					_Angle2DScheme->make(_Owner, size - leftToGo, pAngles, sizeof(float), particleBunchSize) ;
					currentAnglePt = pAngles ;
				}
				else
				{
					currentAnglePt = &_Angle2D ;
				}

				leftToGo -= particleBunchSize ;				
			}
			else
			{
				endPosIt = endPosIt + leftToGo ;

				if (_UseSizeScheme)
				{
					_SizeScheme->make(_Owner, size - leftToGo, pSizes, sizeof(float), leftToGo) ;
					currentSizePt = pSizes ;
				}
				else
				{
					currentSizePt = &_ParticleSize ;
				}

				if (_UseAngle2DScheme)
				{
					_Angle2DScheme->make(_Owner, size - leftToGo, pAngles, sizeof(float), leftToGo) ;
					currentAnglePt = pAngles ;
				}
				else
				{
					currentAnglePt = &_Angle2D ;
				}
			
				leftToGo = 0 ;
			}
		
			
			
		}
		else
		{
			endPosIt = _Owner->getPos().end() ;
			currentSizePt = &_ParticleSize ;			
			currentAnglePt = &_Angle2D ;			
		}	

		for (;posIt != endPosIt ; ++posIt, ++timeIt)
		{		
			*(CVector *) ptVect = *posIt ;
			
			// the start angle
			currentAngle = *currentAnglePt ;

			const uint8 phaseAdd = (uint8) (_PhaseSpeed * (*timeIt)) ;

			ptVect += stride ;

			const float fanSize = *currentSizePt * 0.5f ;

			currentSizePt += currentSizePtIncrement ;
			currentAnglePt += currentAnglePtIncrement ;

			for (k = 0 ; k < _NbFans ; ++k)
			{
				const float fSize  = fanSize * (CPSUtil::getCos(_RandomPhaseTab[k] + phaseAdd) + 1.5f) ;
				*(CVector *) ptVect = (*posIt) + I * fSize * (CPSUtil::getCos((sint32) currentAngle))
									  + K * fSize * (CPSUtil::getSin((sint32) currentAngle)) ;
				currentAngle += angleStep ;
				ptVect += stride ;
			}
		}

		if (leftToGo == 0)
		{
			break ; 
		}
	}


	IDriver *driver = getDriver() ;

	driver->activeVertexBuffer(_Vb) ;	
	driver->renderTriangles(_Mat, _IndexBuffer, size * _NbFans) ;
}

void CPSFanLight::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1) ;
	CPSParticle::serial(f) ;
	CPSColoredParticle::serialColorScheme(f) ;	
	CPSSizedParticle::serialSizeScheme(f) ;	
	CPSRotated2DParticle::serialAngle2DScheme(f) ;
	f.serial(_NbFans) ;
	if (f.isReading())
	{
		init() ;		
	}	
}



bool CPSFanLight::completeBBox(NLMISC::CAABBox &box) const
{
	// TODO

	return false ;
}



CPSFanLight::CPSFanLight(uint32 nbFans) : _IndexBuffer(NULL), _NbFans(nbFans), _PhaseSpeed(256)
{
	nlassert(nbFans >= 3) ;

	// TODO : remove this later, as it will be useless

	init() ;
}

CPSFanLight::~CPSFanLight()
{
	delete[] _IndexBuffer ;
}

	
void CPSFanLight::setNbFans(uint32 nbFans)
{
	_NbFans = nbFans ;

	resize(_Owner->getMaxSize()) ;
}


void CPSFanLight::resize(uint32 size)
{	
	_Vb.setNumVertices(size * (1 + _NbFans)) ;
		
	delete _IndexBuffer ;		

	_IndexBuffer = new uint32[ size * _NbFans * 3] ;
		

	// TODO : adapt this for real fans
	// we precompute the uv's and the index buffer because they won't change


	// pointer on the current index to fill
	uint32 *ptIndex = _IndexBuffer ;	
	
	// index of the first vertex of the current fanFilght
	uint32 currVertFan = 0 ;

	uint32 l ; // the current fan in the current fanlight
	uint32 k ; // the current fan light

	for (k = 0 ; k < size ; ++k)
	{
		for (l = 0 ; l < (_NbFans - 1) ; ++l)
		{
			*ptIndex++ = currVertFan ;
			*ptIndex++ = currVertFan + (l + 1) ;
			*ptIndex++ = currVertFan + (l + 2) ;
		}

		*ptIndex++ = currVertFan ;
		*ptIndex++ = currVertFan + _NbFans ;
		*ptIndex++ = currVertFan + 1 ;


		
		currVertFan += (1 + _NbFans) ;
	}

		
	for (k = 0 ; k < size ; ++k)
	{			
		if (!_UseColorScheme)
		{
			_Vb.setColor(k * (_NbFans + 1), _Color) ;
		}

		for(l = 1 ; l <= _NbFans ; ++l)
		{
			_Vb.setColor(l + k * (_NbFans + 1), CRGBA(0, 0, 0)) ;
		}
	}
}

void CPSFanLight::init(void)
{
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
	_Mat.setZWrite(false) ;
	_Mat.setLighting(false) ;	
	_Mat.setBlend(true) ;
	_Mat.setZFunc(CMaterial::less) ;


	updateMatAndVbForColor() ;
}

	
void CPSFanLight::updateMatAndVbForColor(void)
{
	_Vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR) ;

	
	if (_Owner)
	{
		resize(_Owner->getMaxSize()) ;
	}	
	
}


///////////////////////////////
// CPSTailDot implementation //
///////////////////////////////

// ctor
CPSTailDot::CPSTailDot(uint32 nbSegmentInTail) : _TailNbSeg(nbSegmentInTail), _ColorFading(true)
												 ,_SystemBasisEnabled(false)
{
	init() ;
}


void CPSTailDot::init(void)
{
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
	_Mat.setZWrite(false) ;
	_Mat.setLighting(false) ;
	_Mat.setBlend(true) ;
	_Mat.setZFunc(CMaterial::less) ;


	_Vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR ) ;	
}
	

void CPSTailDot::draw(void)
{
	// the number of particles
	const uint32 size = _Owner->getSize() ;

	if (!size) return ;	

	// size of the vertices
	const uint32 vSize = _Vb.getVertexSize() ;

	// size of all the vertices for a tail
	const uint32 tailVSize = vSize * (_TailNbSeg + 1) ;
	
	// offset of the color in vertices
	const uint32 colorOff = _Vb.getColorOff() ;

	// offset for the head in a tail (the last vertex)
	const uint32 headOffset = vSize * _TailNbSeg ;

	// address of the firstVertex

	uint8 *firstVertex = (uint8 *) _Vb.getVertexCoordPointer() ;

	// loop counters
	uint32 k, l ;

	// pointer to the current vertex
	uint8 *currVertex ;


	CParticleSystem::_NbParticlesDrawn += size ; // for benchmark purpose

	TPSAttribVector::const_iterator posIt, endPosIt = _Owner->getPos().end() ;
		
	// decal the whole vertex buffer (but not the last vertex)
	
	// TODO : cache optimization

	if (_UseColorScheme && _ColorFading)
	{

		// the first color is set to black
		// other are modulated by the ratio of the intensity n and the intensity n + 1
		
		// the brightness increase between each segment in the tail
		float lumiStep = 1.f / _TailNbSeg ;
		float lumi = lumiStep ;

		// set the first vertex to black				
		currVertex = firstVertex ;
		for (k = 0 ; k < size ; ++k)
		{
			*(CRGBA *) (currVertex + colorOff) = CRGBA::Black ;
			// copy the next vertex pos
			*(CVector *) currVertex = *(CVector *) (currVertex + vSize) ;
			currVertex += tailVSize  ;			
		}

		for (l = 1 ; l < _TailNbSeg ; ++l)
		{
			currVertex = firstVertex + l * vSize ;
			uint8 ratio = (uint8) (255.0f * lumi / (lumi +  lumiStep)) ;
			lumi += lumiStep ;			
			for (k = 0 ; k < size ; ++k)
			{
				// get the color of the next vertex and modulate
				((CRGBA *) (currVertex + colorOff))->modulateFromui(*(CRGBA *) (currVertex + vSize + colorOff), ratio) ;
				// copy the next vertex pos
				*(CVector *) currVertex = *(CVector *) (currVertex + vSize) ;
				currVertex += tailVSize  ;			
			}
			
		}			
	}
	else if ( !_UseColorScheme || _ColorFading)
	{		
		// we just decal the pos
		// we copy some extra pos, but we avoid 2 nested loops
		const uint32 nbVerts = (size * (_TailNbSeg + 1)) - 1 ;
		const uint8 *lastVert = firstVertex + nbVerts * vSize ;

		for (currVertex = firstVertex ; currVertex != lastVert ; )
		{
			*(CVector *) currVertex = *(CVector *) (currVertex + vSize) ;			
			currVertex += vSize ;
		}	
	}
	else
	{		
		// we just copy the colors and the position
		// memcpy will copy some extra bytes, but it should remains fasters that 2 nested loops
		memcpy(firstVertex, firstVertex + vSize, size * tailVSize  - vSize) ; 		
	}




	// we fill the head of particles with the right color
	// With constant color, the setup was done in setupColor
	if (_UseColorScheme)
	{
		_ColorScheme->make(_Owner, 0,  firstVertex + headOffset + colorOff
							, tailVSize, _Owner->getSize()) ;		
	}


	currVertex = firstVertex + headOffset ;
	// If we are in the same basis than the located that hold us, we can copy coordinate directly
	if (_SystemBasisEnabled == _Owner->isInSystemBasis())
	{	
		for (posIt = _Owner->getPos().begin() ; posIt != endPosIt ; ++posIt)
		{
			*(CVector *) currVertex = *posIt ;
			currVertex += tailVSize ;
		}
	}
	else
	{
		// the tail are not in the same basis, we need a conversion matrix
		const CMatrix &m = _SystemBasisEnabled ?   _Owner->getOwner()->getInvertedSysMat() 
												 : _Owner->getOwner()->getSysMat() ;
		// copy the position after transformation
		for (posIt = _Owner->getPos().begin() ; posIt != endPosIt ; ++posIt)
		{
			*(CVector *) currVertex = m * (*posIt) ;
			currVertex += tailVSize ;
		}
	}

	
	IDriver *driver = getDriver() ;
	if (_SystemBasisEnabled)
	{
		driver->setupModelMatrix(_Owner->getOwner()->getSysMat()) ;
	}
	else
	{
		driver->setupModelMatrix(CMatrix::Identity) ;
	}



	
	driver->activeVertexBuffer(_Vb) ;		
	_Pb.setNumLine(size * _TailNbSeg) ;
	driver->render(_Pb, _Mat) ;
}


void CPSTailDot::resize(uint32 size)
{	
	resizeVb(_TailNbSeg, size) ;	
}

void CPSTailDot::resizeVb(uint32 oldTailSize, uint32 size)
{
	
	if (!_Owner) return ; // no attachement occured yet ...

	// calculate the primitive block : we got lines
		
	uint32 k, l ;

	_Pb.reserveLine(_TailNbSeg * size) ;

	uint32 currIndex = 0 ;
	uint lineIndex = 0 ;

	for (k = 0 ; k < size ; ++k)
	{
		for (l = 0 ; l < _TailNbSeg ; ++l)
		{
			_Pb.setLine(lineIndex , currIndex + l, currIndex + l + 1) ;		
			++lineIndex ;
		}

		currIndex += _TailNbSeg + 1 ;		
	}

	const uint32 oldSize = _Owner->getSize() ; // number of used particles
	

	_Vb.setVertexFormat(IDRV_VF_COLOR | IDRV_VF_XYZ) ;
	_Vb.setNumVertices(size * (_TailNbSeg + 1)) ;

	// fill the vertex buffer

	// particle that were present before
	// their old positions are copied

	// if the tail are not in the same basis, we need a conversion matrix

	const CMatrix *m;

	if (_SystemBasisEnabled == _Owner->isInSystemBasis())
	{
		m = &CMatrix::Identity ;
	}
	else
	{
		m = _SystemBasisEnabled ?  & _Owner->getOwner()->getInvertedSysMat() 
								: &_Owner->getOwner()->getSysMat() ;
	}

	const TPSAttribVector &oldPos = _Owner->getPos() ;
	currIndex = 0 ;
	CVector currPos ;

	for (k = 0 ; k < _Owner->getSize() ; ++k)
	{
		currPos = *m * oldPos[k] ;
		for (l = 0 ; l <= _TailNbSeg ; ++l)
		{
			_Vb.setVertexCoord( currIndex + l , currPos) ;			
		}
		currIndex += _TailNbSeg + 1 ;		
	}	
	// we don't need to setup the following vertices coordinates, they will be filled with newElement anyway...

	setupColor() ;
	
}

void CPSTailDot::setupColor(void)
{
	// we setup the WHOLE vb so, we need to get the max number of particles
	const uint32 size = _Owner->getMaxSize() ;	

	// size of the vertices
	const uint32 vSize = _Vb.getVertexSize() ;

	// offset of the color in vertices
	const uint32 colorOff = _Vb.getColorOff() ;

	// first vertex
	uint8 *firstVertex = (uint8 *) _Vb.getVertexCoordPointer() ;
	
	// point the current vertex color
	uint8 *currVertex =  firstVertex + colorOff ;

	// loop counters
	uint k, l ;


	if (_UseColorScheme || !_ColorFading)
	{
		// we can't precompute the colors, so at first, we fill all with black
		const CRGBA &color = _UseColorScheme ? CRGBA::Black : _Color ;
		for (k = 0 ; k < size * (_TailNbSeg + 1) ; ++k)
		{
				*(CRGBA *) currVertex = color ;
				currVertex += vSize ;
		}
	}
	else // constant color with color fading
	{
		const float lumiStep = 255.0f / _TailNbSeg ;
		const  uint32 tailVSize = vSize * (_TailNbSeg + 1) ; // size of a whole tail in the vertex buffer
		float lumi = 0.f ;
		for (l = 0 ; l <= _TailNbSeg ; ++l)
		{
			currVertex = firstVertex + l * vSize + colorOff ;
			CRGBA col ;
			col.modulateFromui(_Color, (uint8) lumi) ;
			lumi += lumiStep ;			
			for (k = 0 ; k < size ; ++k)
			{				
				*(CRGBA *)currVertex = col ;				
				currVertex += tailVSize  ;			
			}
			
		}

	}


	

}


void CPSTailDot::newElement(void)
{
	nlassert(_Owner->getSize() != _Owner->getMaxSize()) ;


	// if we got a constant color, everything has been setupped before

	
		const uint32 index = _Owner->getNewElementIndex() ; // the index of the element to be created

		const uint32 vSize =_Vb.getVertexSize() ;	// vertex size
		uint8 *currVert = (uint8 *) _Vb.getVertexCoordPointer() + (index * (_TailNbSeg + 1)) * vSize ; // 

		const CVector &pos = _Owner->getPos()[index] ;	
		if (_UseColorScheme)
		{
			// check wether the located is in the same basis than the tail
			// Otherwise, a conversion is required for the pos
			if (_SystemBasisEnabled == _Owner->isInSystemBasis())
			{
				for (uint32 k = 0 ; k < _TailNbSeg + 1 ; ++k)
				{
					*(CVector *) currVert = pos ;
					*(CRGBA *) (currVert + _Vb.getColorOff()) = CRGBA::Black ;

					currVert += vSize ; // go to next vertex
				}
			}
			else
			{
				const CMatrix &m = _SystemBasisEnabled ?   _Owner->getOwner()->getInvertedSysMat() 
												 : _Owner->getOwner()->getSysMat() ;
				for (uint32 k = 0 ; k < _TailNbSeg + 1 ; ++k)
				{
					*(CVector *) currVert = m * pos ;
					*(CRGBA *) (currVert + _Vb.getColorOff()) = CRGBA::Black ;

					currVert += vSize ; // go to next vertex
				}
			}

		}
		else
		{
			// color setup was done during setup color

			// check wether the located is in the same basis than the tail
			// Otherwise, a conversion is required for the pos
			if (_SystemBasisEnabled == _Owner->isInSystemBasis())
			{
				for (uint32 k = 0 ; k < _TailNbSeg + 1 ; ++k)
				{
					*(CVector *) currVert = pos ;			
					currVert += vSize ; // go to next vertex
				}
			}
			else
			{
				const CMatrix &m = _SystemBasisEnabled ?   _Owner->getOwner()->getInvertedSysMat() 
												 : _Owner->getOwner()->getSysMat() ;
				for (uint32 k = 0 ; k < _TailNbSeg + 1 ; ++k)
				{
					*(CVector *) currVert = m * pos ;			
					currVert += vSize ; // go to next vertex
				}
			}
		}
}
		


void CPSTailDot::deleteElement(uint32 index)
{
	// we copy the last element datas to this one data
	
	// vertex size
	const uint32 vSize =_Vb.getVertexSize() ;


	// source
	const uint8 *currSrcVert = (uint8 *) _Vb.getVertexCoordPointer() + ((_Owner->getSize() - 1) * (_TailNbSeg + 1)) * vSize ;

	// destination
	uint8 *currDestVert = (uint8 *) _Vb.getVertexCoordPointer() + (index * (_TailNbSeg + 1)) * vSize ;


	// copy the vertices
	memcpy(currDestVert, currSrcVert, vSize * (_TailNbSeg + 1) ) ;	
}



			
	
	
void CPSTailDot::setTailNbSeg(uint32 nbSeg)
{
	uint32 oldTailSize = _TailNbSeg ;
	_TailNbSeg = nbSeg ;

	if (_Owner)
	{
		resizeVb(nbSeg, _Owner->getMaxSize()) ;
	}
}

	


void CPSTailDot::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	const uint oldTailNbSeg = _TailNbSeg ;

	f.serialVersion(1) ;
	CPSParticle::serial(f) ;
	CPSColoredParticle::serialColorScheme(f) ;	
	f.serial(_TailNbSeg, _ColorFading, _SystemBasisEnabled) ;

	// In this version we don't save the vb state, as we probably won't need it
	// we just rebuild the vb
	
	if (f.isReading())
	{
		resizeVb(oldTailNbSeg, _Owner->getMaxSize()) ;
		// init() ;		
	}		

}

void CPSTailDot::updateMatAndVbForColor(void)
{
	setupColor() ;
}




} // NL3D
