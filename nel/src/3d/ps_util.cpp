/** \file ps_util.cpp
 * <File description>
 *
 * $Id: ps_util.cpp,v 1.19 2001/07/12 15:40:40 vizerie Exp $
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

#include "3d/ps_util.h"
#include "3d/driver.h"
#include "nel/misc/aabbox.h"
#include "3d/vertex_buffer.h"
#include "3d/primitive_block.h"
#include "3d/material.h"
#include "3d/nelu.h"

#include "3d/font_generator.h"
#include "3d/font_manager.h"
#include "3d/computed_string.h"

#include "nel/misc/matrix.h"

#include "3d/dru.h"

#include "3d/ps_emitter.h"
#include "3d/ps_located.h"
#include "3d/ps_particle.h"
#include "3d/ps_force.h"
#include "3d/ps_zone.h"
#include "3d/ps_color.h"
#include "3d/ps_float.h"
#include "3d/ps_int.h"
#include "3d/ps_plane_basis_maker.h"

#include "3d/particle_system_shape.h"



namespace NL3D {


using NLMISC::CVector ;


#ifdef NL_DEBUG
	bool CPSUtil::_CosTableInitialized = false ;
	bool CPSUtil::_PerlinNoiseTableInitialized = false ;
#endif

float CPSUtil::_CosTable[256] ;
float CPSUtil::_SinTable[256] ;


float CPSUtil::_PerlinNoiseTab[1024] ;


void CPSUtil::initPerlinNoiseTable(void)
{
	for (uint32 k = 0 ; k < 1024 ; ++k)
	{
		_PerlinNoiseTab[k] = (rand() % 30000) / 30000.f ; 
	}
	#ifdef NL_DEBUG
		_PerlinNoiseTableInitialized = true ;
	#endif
}


void CPSUtil::initFastCosNSinTable(void)
{
	for (uint32 k = 0 ; k < 256 ; k++)
	{
		const float angle =  k / 256.0f * 2.0f * float(NLMISC::Pi) ; 
		_CosTable[k] = (float) cos( angle ) ;
		_SinTable[k] = (float) sin( angle ) ;
	}
	#ifdef NL_DEBUG
		_CosTableInitialized = true ;
	#endif
}


void CPSUtil::registerSerialParticleSystem(void)
{
		NLMISC_REGISTER_CLASS(CPSEmitterOmni) ;
		NLMISC_REGISTER_CLASS(CPSEmitterDirectionnal) ;
		NLMISC_REGISTER_CLASS(CPSEmitterRectangle) ;
		NLMISC_REGISTER_CLASS(CPSEmitterConic) ;
		NLMISC_REGISTER_CLASS(CPSSphericalEmitter) ;
		NLMISC_REGISTER_CLASS(CPSDirectionnalForce) ; 
		NLMISC_REGISTER_CLASS(CPSGravity) ; 
		NLMISC_REGISTER_CLASS(CPSBrownianForce) ;
		NLMISC_REGISTER_CLASS(CPSCentralGravity) ; 
		NLMISC_REGISTER_CLASS(CPSFluidFriction) ;
		NLMISC_REGISTER_CLASS(CPSTurbul) ;
		NLMISC_REGISTER_CLASS(CPSCylindricVortex) ;
		NLMISC_REGISTER_CLASS(CPSLocated) ; 
		NLMISC_REGISTER_CLASS(CPSDot) ;
		NLMISC_REGISTER_CLASS(CPSFaceLookAt) ;
		NLMISC_REGISTER_CLASS(CPSZonePlane) ;
		NLMISC_REGISTER_CLASS(CPSZoneSphere) ;
		NLMISC_REGISTER_CLASS(CPSZoneDisc) ;
		NLMISC_REGISTER_CLASS(CPSZoneRectangle) ;
		NLMISC_REGISTER_CLASS(CPSZoneCylinder) ;
		NLMISC_REGISTER_CLASS(CPSColorBlender) ;
		NLMISC_REGISTER_CLASS(CPSColorMemory) ;
		NLMISC_REGISTER_CLASS(CPSColorBinOp) ;
		NLMISC_REGISTER_CLASS(CPSColorBlenderExact) ;
		NLMISC_REGISTER_CLASS(CPSColorGradient) ;
		NLMISC_REGISTER_CLASS(CPSFloatBlender) ;		
		NLMISC_REGISTER_CLASS(CPSFloatGradient) ;
		NLMISC_REGISTER_CLASS(CPSFloatMemory) ;
		NLMISC_REGISTER_CLASS(CPSFloatBinOp) ;
		NLMISC_REGISTER_CLASS(CPSIntBlender) ;		
		NLMISC_REGISTER_CLASS(CPSIntMemory) ;		
		NLMISC_REGISTER_CLASS(CPSIntBinOp) ;		
		NLMISC_REGISTER_CLASS(CPSIntGradient) ;
		NLMISC_REGISTER_CLASS(CPSUIntBlender) ;		
		NLMISC_REGISTER_CLASS(CPSUIntMemory) ;		
		NLMISC_REGISTER_CLASS(CPSUIntBinOp) ;		
		NLMISC_REGISTER_CLASS(CPSUIntGradient) ;
		NLMISC_REGISTER_CLASS(CPSSpring) ;
		NLMISC_REGISTER_CLASS(CPSFanLight) ;
		NLMISC_REGISTER_CLASS(CPSTailDot) ;
		NLMISC_REGISTER_CLASS(CPSRibbon) ;
		NLMISC_REGISTER_CLASS(CPSShockWave) ;
		NLMISC_REGISTER_CLASS(CPSFace) ;
		NLMISC_REGISTER_CLASS(CPSMesh) ;
		NLMISC_REGISTER_CLASS(CPSConstraintMesh) ;
		NLMISC_REGISTER_CLASS(CParticleSystemShape) ;
		NLMISC_REGISTER_CLASS(CPSPlaneBasisBlender) ;
		NLMISC_REGISTER_CLASS(CPSPlaneBasisGradient) ;
		NLMISC_REGISTER_CLASS(CPSPlaneBasisMemory) ;
		NLMISC_REGISTER_CLASS(CPSPlaneBasisBinOp) ;
		NLMISC_REGISTER_CLASS(CPSPlaneBasisFollowSpeed) ;

		// while we are here, we perform some important inits
		CPSRotated2DParticle::initRotTable() ; // init the precalc rot table for face lookat
		initFastCosNSinTable() ; // init fast cosine lookup table
		initPerlinNoiseTable() ; // init perlin noise table
		CPSFanLight::initFanLightPrecalc() ;
}


void CPSUtil::displayBBox(IDriver *driver, const NLMISC::CAABBox &box)
{	
	CVector max = box.getMax()
			,min = box.getMin() ;
	CVertexBuffer vb ;
	vb.setVertexFormat(IDRV_VF_XYZ) ;
	vb.setNumVertices(8) ;

	vb.setVertexCoord(0, min) ;
	vb.setVertexCoord(1, CVector(max.x, min.y, min.z)) ;
	vb.setVertexCoord(2, CVector(min.x, max.y, min.z)) ;
	vb.setVertexCoord(3, CVector(max.x, max.y, min.z)) ;
	vb.setVertexCoord(4, CVector(min.x, min.y, max.z)) ;
	vb.setVertexCoord(5, CVector(max.x, min.y, max.z)) ;
	vb.setVertexCoord(6, CVector(min.x, max.y, max.z)) ;
	vb.setVertexCoord(7, max) ;


	CMaterial material ;

	material.setColor(CRGBA(127, 127, 127)) ;
	material.setLighting(false) ;
	material.setBlendFunc(CMaterial::one, CMaterial::one) ;
	material.setZWrite(false) ;
	material.setBlend(true) ;



	CPrimitiveBlock pb ;
	pb.reserveLine(12) ;
	pb.addLine(0, 1) ; 
	pb.addLine(1, 5) ; 
	pb.addLine(5, 4) ; 
	pb.addLine(4, 0) ; 
	pb.addLine(0, 2) ; 
	pb.addLine(1, 3) ; 
	pb.addLine(4, 6) ; 
	pb.addLine(5, 7) ; 
	pb.addLine(6, 7) ; 
	pb.addLine(7, 3) ; 
	pb.addLine(3, 2) ; 
	pb.addLine(2, 6) ; 




	CMatrix mat ;
	mat.identity() ;

	driver->setupModelMatrix(mat) ;
	driver->activeVertexBuffer(vb) ;
	driver->render(pb, material) ;

}



void CPSUtil::displayArrow(IDriver *driver, const CVector &start, const CVector &v, float size, CRGBA col1, CRGBA col2)
{

	const float coneSize = size * 0.1f ;

	uint32 vTab[] = { 1, 2, 4,
						  4, 2, 3,
						  1, 2, 0,
						  2, 3, 0,
						  3, 4, 0,
						  4, 1, 0 } ;

	CVector end = start + size * v ;
	CDRU::drawLine(start, end, col1, *driver) ;
	CMatrix m = buildSchmidtBasis(v) ;

	CVertexBuffer vb ;
	vb.setVertexFormat(IDRV_VF_XYZ) ;
	vb.setNumVertices(5) ; 
	
	

	vb.setVertexCoord(0, end + m * CVector(0, 0, 3.0f * coneSize) ) ;
	vb.setVertexCoord(1, end + m * CVector(-coneSize, -coneSize, 0) ) ;
	vb.setVertexCoord(2, end + m * CVector(coneSize, -coneSize, 0) ) ;
	vb.setVertexCoord(3, end + m * CVector(coneSize, coneSize, 0) ) ;
	vb.setVertexCoord(4, end + m * CVector(-coneSize, coneSize, 0) ) ;

	CMaterial material  ;

	material.setColor(col2) ;
	material.setLighting(false) ;
	material.setBlendFunc(CMaterial::one, CMaterial::one) ;
	material.setZWrite(false) ;
	material.setBlend(true) ;
	material.setDoubleSided(true) ;

	driver->activeVertexBuffer(vb, 0, 5) ;
	driver->renderTriangles(material,  vTab, 6) ;


}

void CPSUtil::displayBasis(IDriver *driver, const CMatrix &modelMat, const NLMISC::CMatrix &m, float size, CFontGenerator &fg, CFontManager &fm)
{
	CMaterial material  ;

	driver->setupModelMatrix(modelMat) ;
	


	displayArrow(driver, m.getPos(), m.getI(), size, CRGBA(127, 127, 127), CRGBA(0, 0, 80)) ;
	displayArrow(driver, m.getPos(), m.getJ(), size, CRGBA(127, 127, 127), CRGBA(0, 0, 80)) ;
	displayArrow(driver, m.getPos(), m.getK(), size, CRGBA(127, 127, 127), CRGBA(200, 0, 80)) ;

	
	// draw the letters

	CPSUtil::print(driver, std::string("x"), fg, fm, modelMat * m * CVector(1.4f * size, 0, 0), 15.0f * size) ;
	CPSUtil::print(driver, std::string("y"), fg, fm, modelMat * m * CVector(0, 1.4f  * size, 0), 15.0f * size) ;
	CPSUtil::print(driver, std::string("z"), fg, fm, modelMat * m * CVector(0, 0, 1.4f  * size), 15.0f * size) ;

} ;



void CPSUtil::print(IDriver *driver, const std::string &text, CFontGenerator &fg, CFontManager &fm, const CVector &pos, float size)
{
	nlassert((&fg) && (&fm)) ;	
	CComputedString cptedString ;	
	fm.computeString ( text,
						&fg, 
						CRGBA(255,255,255),
						16, 
						driver,
						cptedString);
			
		
	CMatrix mat = driver->getViewMatrix() ;	
	mat.setPos(CVector::Null) ;	
	mat.scale(CVector(size, size, size)) ;		
	mat.transpose() ;
	mat.setPos(pos) ;	 


	cptedString.render3D(*driver, mat) ;
}



/**
* Compute the union of 2 aabboxes, that is the  aabbox that contains the 2.
* Should end up in NLMISC
*/


NLMISC::CAABBox CPSUtil::computeAABBoxUnion(const NLMISC::CAABBox &b1, const NLMISC::CAABBox &b2)
{	
	NLMISC::CAABBox result ;
	CVector min, max ;
	CVector min1 = b1.getMin() 
		    ,max1 = b1.getMax() 
			,min2 = b2.getMin() 
		    ,max2 = b2.getMax() ;
	max.maxof(max1, max2) ;
	min.minof(min1, min2) ;
	result.setMinMax(min, max) ;
	return result ;
}


NLMISC::CAABBox CPSUtil::transformAABBox(const NLMISC::CMatrix &mat, const NLMISC::CAABBox &box)
{
	// TODO : optimize this a bit if possible
	NLMISC::CAABBox result ;	

	const CVector &m = mat * box.getCenter() ;
	const CVector &h = mat.mulVector(box.getHalfSize()) ;

	CVector tmp, min, max ;


	min = max = m ;
	tmp = m + CVector(h.x, h.y, -h.z) ; min.minof(min, tmp) ; max.maxof(max, tmp) ;
	tmp = m + CVector(h.x, -h.y, h.z) ; min.minof(min, tmp) ; max.maxof(max, tmp) ;
	tmp = m + CVector(h.x, -h.y, -h.z) ; min.minof(min, tmp) ; max.maxof(max, tmp) ;
	tmp = m + CVector(-h.x, h.y, h.z) ; min.minof(min, tmp) ; max.maxof(max, tmp) ;
	tmp = m + CVector(-h.x, h.y, -h.z) ; min.minof(min, tmp) ; max.maxof(max, tmp) ;
	tmp = m + CVector(-h.x, -h.y, h.z) ; min.minof(min, tmp) ; max.maxof(max, tmp) ;
	tmp = m + CVector(-h.x, -h.y, -h.z) ; min.minof(min, tmp) ; max.maxof(max, tmp) ;

	result.setMinMax(min, max) ;
	
	return result ;	
}


CMatrix CPSUtil::buildSchmidtBasis(const CVector &k_)
{
	const float epsilon = 10E-4f ;


	CVector k = k_ ;
	k.normalize() ;
	CVector i ;
	if ((1.0f - fabsf(k * CVector::I)) > epsilon)
	{
		i = k ^ CVector::I ;
	}
	else
	if ((1.0f - fabs(k * CVector::J)) > epsilon)
	{
		i = k ^ CVector::J ;
	}
	else
	{
		i = k ^ CVector::K ;
	}

	i = i - (k * i) * k ;
	i.normalize() ;

	CMatrix result ;
	result.setRot(i, k ^ i, k, true) ;
	return result ;
}



void CPSUtil::displaySphere(IDriver &driver, float radius, const CVector &center, uint nbSubdiv, CRGBA color)
{
	uint x, y, k ;
	CVector p, p1, p2 ;
	
	static const CVector lK[] = { CVector::I, -CVector::I
								,CVector::J, -CVector::J
								,CVector::K, -CVector::K } ;

/*	static const CVector lI = { CVector::J, -CVector::J
								,CVector::K, -CVector::K
								,CVector::I, -CVector::I } ;*/



	for (k = 0 ; k < 6 ; ++k)
	{
		const CVector &I = lK[(k + 2) % 6] ;
		const CVector &K = lK[k] ;
		const CVector J = K ^ I ;

		for (x = 0; x < nbSubdiv ; ++x)
		{
			for (y = 0; y < nbSubdiv ; ++y)
			{
				p = ((2.f * x / float(nbSubdiv) ) - 1.f) * I + ((2.f * y / float(nbSubdiv) ) - 1.f) * J + K ;
				p1 = p + 2.f / float(nbSubdiv) * I ;
				p2 = p + 2.f / float(nbSubdiv) * J ;

				p.normalize() ; 
				p1.normalize() ;
				p2.normalize() ;

				p = center + radius * p ;
				p1 = center + radius * p1 ;
				p2 = center + radius * p2 ;

				CDRU::drawLine(p, p1, color, driver) ;
				CDRU::drawLine(p, p2, color, driver) ;
			}
		}
	}
}



void CPSUtil::displayDisc(IDriver &driver, float radius, const CVector &center, const CMatrix &mat, uint nbSubdiv, CRGBA color)
{
	// not optimized, but for edition only
	float thetaDelta = (float) NLMISC::Pi * 2.f / nbSubdiv ;
	float theta = 0.f ;
	const CVector &I = mat.getI() ;
	const CVector &J = mat.getJ() ;
	for (uint k = 0 ; k < nbSubdiv ; ++k)
	{
		
		CDRU::drawLine(center + radius * ((float) cos(theta) * I + (float) sin(theta) * J)
					   , center + radius * ((float) cos(theta + thetaDelta) * I + (float) sin(theta + thetaDelta) * J)
					   , color, driver) ;
		theta += thetaDelta ;
	}

}


void CPSUtil::displayCylinder(IDriver &driver, const CVector &center, const CMatrix &mat, const CVector &dim, uint nbSubdiv, CRGBA color)
{
	// not optimized, but for edition only
	float thetaDelta = (float) NLMISC::Pi * 2.f / nbSubdiv ;
	float theta = 0.f ;
	const CVector &I = mat.getI() ;
	const CVector &J = mat.getJ() ;
	const CVector &K = mat.getK() ;

	for (uint k = 0 ; k < nbSubdiv ; ++k)
	{
		
		CDRU::drawLine(center + dim.z * K + dim.x * cosf(theta) * I + dim.y * sinf(theta) * J
					   , center + dim.z * K + dim.x * cosf(theta + thetaDelta) * I + dim.y * sinf(theta + thetaDelta) * J
					   , color, driver) ;

		CDRU::drawLine(center - dim.z * K + dim.x * cosf(theta) * I + dim.y * sinf(theta) * J
					   , center - dim.z * K + dim.x * cosf(theta + thetaDelta) * I + dim.y * sinf(theta + thetaDelta) * J
					   , color, driver) ;

		CDRU::drawLine(center + dim.z * K + dim.x * cosf(theta) * I + dim.y * sinf(theta) * J
					   , center - dim.z * K + dim.x * cosf(theta) * I + dim.y * sinf(theta) * J
					   , color, driver) ;


		
		
		theta += thetaDelta ;
	}	
}


void CPSUtil::display3DQuad(IDriver &driver, const CVector &c1, const CVector &c2
								,const CVector &c3,  const CVector &c4, CRGBA color)
{
	CDRU::drawLine(c1, c2, color, driver) ;
	CDRU::drawLine(c2, c3, color, driver) ;
	CDRU::drawLine(c3, c4, color, driver) ;
	CDRU::drawLine(c4, c1, color, driver) ;
}




} // NL3D
