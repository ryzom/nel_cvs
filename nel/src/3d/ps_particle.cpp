/** \file ps_particle.cpp
 * <File description>
 *
 * $Id: ps_particle.cpp,v 1.5 2001/05/02 11:48:46 vizerie Exp $
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

	updateMatAndVb() ;
}


		
void CPSColoredParticle::setColor(NLMISC::CRGBA col)
{
	if (_UseColorScheme)
	{
		delete _ColorScheme ;
		_UseColorScheme = false ;
	}
	_Color = col ;

	updateMatAndVb() ;
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

void CPSColoredParticle::serialColorScheme(NLMISC::IStream &f)
{	
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
void CPSSizedParticle::serialSizeScheme(NLMISC::IStream &f)
{
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



///////////////////////////
// CPSDot implementation //
///////////////////////////

void CPSDot::init(void)
{	
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
	_Mat.setZWrite(false) ;
	_Mat.setLighting(false) ;
	_Mat.setBlend(true) ;
	
	updateMatAndVb() ;
}




void CPSDot::updateMatAndVb(void)
{
	if (!_UseColorScheme)
	{
		_Vb.setVertexFormat(IDRV_VF_XYZ) ;		
		_Mat.setColor(_Color) ;
	}
	else
	{
		_Vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR) ;
		_Mat.setColor(CRGBA(255, 255, 255)) ;
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
		_ColorScheme->make(_Owner->getTime().begin(), _Vb.getColorPointer(), _Vb.getVertexSize(), size) ;
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



void CPSDot::serial(NLMISC::IStream &f)
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
CPSFaceLookAt::CPSFaceLookAt(CSmartPtr<ITexture> tex) : _Tex(tex), _IndexBuffer(NULL)
{
	init() ;

	// we don't init the _IndexBuffer for now, as it will be when resize is called
}

CPSFaceLookAt::CPSFaceLookAt() : _IndexBuffer(NULL)
{
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
	_Mat.setTexture(0, _Tex) ;
	_Mat.setBlend(true) ;
	updateMatAndVb() ;
}


void CPSFaceLookAt::updateMatAndVb(void)
{
	if (!_UseColorScheme)
	{
		_Vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_UV[0]) ;		
		_Mat.setColor(_Color) ;
	}
	else
	{
		_Vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR | IDRV_VF_UV[0]) ;
		_Mat.setColor(CRGBA(255, 255, 255)) ;
	}

	if (_Owner)
	{
		resize(_Owner->getMaxSize()) ;
	}

		
}




void CPSFaceLookAt::draw(void)
{


	setupDriverModelMatrix() ;
	CVector I = computeI() ;
	CVector K = computeK() ;

	const uint32 size = _Owner->getSize() ;

	if (!size) return ;

	CParticleSystem::_NbParticlesDrawn += size ;

	if (_UseColorScheme)
	{
		// compute the colors, each color is replicated 4 times
		_ColorScheme->make4(_Owner->getTime().begin(), _Vb.getColorPointer(), _Vb.getVertexSize(), size) ;
	}


	uint32 k = 0 ;
	TPSAttribVector::iterator it = _Owner->getPos().begin() ;

	IDriver *driver = getDriver() ;


	// we have 2 drawing version : one with fixed size, and the other, with variable size
	
	if (!_UseSizeScheme)
	{
		const CVector v1 = - _ParticleSize * I + _ParticleSize * K ;
		const CVector v2 = + _ParticleSize * I + _ParticleSize * K ;
		const CVector v3 = + _ParticleSize * I - _ParticleSize * K ;
		const CVector v4 = -_ParticleSize * I - _ParticleSize * K ;

		while (k != size)
		{
			_Vb.setVertexCoord(k * 4 , *it  + v1) ;
			_Vb.setVertexCoord(k * 4 + 1, *it + v2) ;	
			_Vb.setVertexCoord(k * 4 + 2, *it + v3) ;	
			_Vb.setVertexCoord(k * 4 + 3, *it + v4) ;					
			++k ;
			++it ;
		}
	}
	else
	{
		const CVector v1 = - I + K ;
		const CVector v2 = I + K ;
		const CVector v3 = I - K ;
		const CVector v4 = - I - K ;	

		uint32 leftToDo = size ;

		float pSizes[1024] ; // the sizes to use

		float *currentSize ;

		TPSAttribTime::const_iterator itTime = _Owner->getTime().begin() ;
		uint32 l ;

		for (;;)
		{
			if (leftToDo < 1024)
			{
				_SizeScheme->make(itTime, pSizes, sizeof(float), leftToDo) ;				

				currentSize = &pSizes[0] ;				
				for (; k < size ; ++k)
				{
					_Vb.setVertexCoord(k * 4 , *it  + *currentSize * v1) ;
					_Vb.setVertexCoord(k * 4 + 1, *it + *currentSize * v2) ;	
					_Vb.setVertexCoord(k * 4 + 2, *it + *currentSize * v3) ;	
					_Vb.setVertexCoord(k * 4 + 3, *it + *currentSize * v4) ;										
					++it ;
					++currentSize ;
				}				
				break ;
			}
			else
			{
				// we compute 1024 new size at a time

				_SizeScheme->make(itTime, pSizes, sizeof(float), 1024) ;				

				currentSize = &pSizes[0] ;
				for (l = 0 ; l < 1024 ; ++l)
				{
					_Vb.setVertexCoord(k * 4 , *it  + *currentSize * v1) ;
					_Vb.setVertexCoord(k * 4 + 1, *it + *currentSize * v2) ;	
					_Vb.setVertexCoord(k * 4 + 2, *it + *currentSize * v3) ;	
					_Vb.setVertexCoord(k * 4 + 3, *it + *currentSize * v4) ;					
					++k ;
					++it ;
					++currentSize ;
				}				
				itTime = itTime + 1024 ;				
				leftToDo -= 1024 ;
			}
		}
	}

	driver->activeVertexBuffer(_Vb) ;	
	driver->renderTriangles(_Mat, _IndexBuffer, size * 2) ;

}


void CPSFaceLookAt::serial(NLMISC::IStream &f)
{

	f.serialCheck((uint32) 'PFLA') ;	
	ITexture *ptTex = _Tex ;
	f.serialPolyPtr(ptTex) ;
	if (f.isReading())
	{	
		_Tex = ptTex ;				
	}	
			
	CPSParticle::serial(f) ;
	CPSSizedParticle::serialSizeScheme(f) ;
	CPSColoredParticle::serialColorScheme(f) ;
	if (f.isReading())
	{
		init() ;		
	}
}


bool CPSFaceLookAt::completeBBox(NLMISC::CAABBox &box) const  
{ 
	if (_UseColorScheme)
	{
		CPSUtil::addRadiusToAABBox(box, _ParticleSize) ;
	}
	else
	{
		CPSUtil::addRadiusToAABBox(box, _SizeScheme->getMaxValue()) ;
	}


	return true  ;	
}

	



} // NL3D
