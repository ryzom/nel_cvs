/** \file ps_particle.cpp
 * <File description>
 *
 * $Id: ps_particle.cpp,v 1.3 2001/04/27 09:32:03 vizerie Exp $
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
#include "nel/3d/vertex_buffer.h"


namespace NL3D {


/*
 * Constructor
 */
CPSParticle::CPSParticle()
{
}


CPSDot::CPSDot(const CRGBA &c) : _Color(c)
{
	init() ;
}

void CPSDot::init(void)
{
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
	_Mat.setZWrite(false) ;
	_Mat.setLighting(false) ;
	_Mat.setBlend(true) ;
}


void CPSDot::step(TPSProcessPass pass, CAnimationTime)
{
	if (pass != PSBlendRender) return  ;

	// we create a vertex buffer that contains all the particles before to show them
	uint32 size = _Owner->getSize() ;


	if (!size) return ;


	setupDriverModelMatrix() ;

	CVertexBuffer vb ;
	vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR) ;
	vb.setNumVertices(size) ;

	uint32 k = 0 ;
	TPSAttribVector::iterator it = _Owner->getPos().begin() ;
	TPSAttribFloat::iterator it2 = _Owner->getTime().begin() ;

	IDriver *driver = getDriver() ;

	uint32 *tab = new uint32[3 * size] ;

	float intensity ;

	CRGBA result ;


	// pas du tout opimise mais c pour tester ...

	while (k != size)
	{
		intensity = 1.0f - *it2 ;
		vb.setVertexCoord(k, *it) ;	
		result.blendFromui(CRGBA(0,0,0), _Color, (uint8) (255.0f * intensity)) ;
		vb.setColor(k,  result) ;
		

		tab[3 * k] = k ;
		tab[3 * k + 1] = k ;
		tab[3 * k + 2] = k ;
		++k ;
		++it ; ++it2 ;
	}

	driver->activeVertexBuffer(vb) ;
	driver->setPolygonMode(IDriver::Point) ;
	
	driver->renderTriangles(_Mat, tab, size) ;


	driver->setPolygonMode(IDriver::Filled) ;

	delete[] tab ;
}



void CPSDot::serial(NLMISC::IStream &f)
{
	f.serialCheck((uint32) 'PSDO') ;
	CPSParticle::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_Color) ;

	if (f.isReading())
	{
		init() ;
	}
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





	/// create the face look at by giving a texture and an optionnal color
CPSFaceLookAt::CPSFaceLookAt(CSmartPtr<ITexture> tex, const CRGBA &c) : CPSDot(c), _Tex(tex)
{
	init() ;
}

void CPSFaceLookAt::init(void)
{
	_Mat.setBlendFunc(CMaterial::one, CMaterial::one) ;
	_Mat.setZWrite(false) ;
	_Mat.setLighting(false) ;
	_Mat.setTexture(0, _Tex) ;
	_Mat.setBlend(true) ;
}


void CPSFaceLookAt::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	if (pass != PSBlendRender) return  ;

	setupDriverModelMatrix() ;
	CVector I = computeI() ;
	CVector K = computeK() ;

	// we create a vertex buffer that contains all the particles before to show them
	uint32 size = _Owner->getSize() ;

	if (!size) return ;

	CVertexBuffer vb ;
	vb.setVertexFormat(IDRV_VF_XYZ | IDRV_VF_COLOR | IDRV_VF_UV[0]) ;
	vb.setNumVertices(4 * size) ;

	uint32 k = 0 ;
	TPSAttribVector::iterator it = _Owner->getPos().begin() ;
	TPSAttribFloat::iterator it2 = _Owner->getTime().begin() ;

	IDriver *driver = getDriver() ;

	uint32 *tab = new uint32[6 * size] ;

	float intensity ;

	CRGBA result ;


	
	// pas du tout opimise mais c pour tester ...

	while (k != size)
	{
	//	intensity = 1.0f - *it2 ;
		intensity = 1.0f ;
		vb.setVertexCoord(k * 4 , *it  -FaceLookAtSize * I + FaceLookAtSize * K) ;	
		vb.setVertexCoord(k * 4 + 1, *it + FaceLookAtSize * I + FaceLookAtSize * K) ;	
		vb.setVertexCoord(k * 4 + 2, *it + FaceLookAtSize * I -FaceLookAtSize * K) ;	
		vb.setVertexCoord(k * 4 + 3, *it -FaceLookAtSize * I -FaceLookAtSize * K) ;	
		vb.setTexCoord(k * 4, 0, CUV(0,0)) ;
		vb.setTexCoord(k * 4 + 1, 0, CUV(1,0)) ;
		vb.setTexCoord(k * 4 + 2, 0, CUV(1,1)) ;
		vb.setTexCoord(k * 4 + 3, 0, CUV(0,1)) ;

		result.blendFromui(CRGBA(0,0,0), _Color, (uint8) (255.0f * intensity)) ;
		vb.setColor(4 * k,  result) ;
		vb.setColor(4 * k + 1,  result) ;
		vb.setColor(4 * k + 2,  result) ;
		vb.setColor(4 * k + 3,  result) ;

		tab[6 * k] = 4 * k + 2 ;
		tab[6 * k + 1] = 4 * k + 1 ;
		tab[6 * k + 2] = 4 * k ;		

		tab[6 * k + 3] = 4 * k  ;
		tab[6 * k + 4] = 4 * k + 3 ;
		tab[6 * k + 5] = 4 * k + 2;
		

		++k ;
		++it ; ++it2 ;
	}

	driver->activeVertexBuffer(vb) ;

	
	driver->renderTriangles(_Mat, tab, size * 2) ;




	delete[] tab ;

}
void CPSFaceLookAt::serial(NLMISC::IStream &f)
{
	f.serialCheck((uint32) 'PFLA') ;
	CPSDot::serial(f) ;

	ITexture *ptTex = _Tex ;

	f.serialPolyPtr(ptTex) ;
	

	if (f.isReading())
	{	
		_Tex = ptTex ;
		init() ;
	}
	
}
	



} // NL3D
