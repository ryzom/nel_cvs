/** \file ps_zone.cpp
 * <File description>
 *
 * $Id: ps_zone.cpp,v 1.5 2001/05/28 15:30:12 vizerie Exp $
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

#include "nel/3d/ps_zone.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/primitive_block.h"
#include "nel/3d/material.h"
#include "nel/3d/ps_util.h"
#include "nel/misc/plane.h"

#include <math.h>

namespace NL3D {


/*
 * Constructor
 */
	CPSZone::CPSZone() : _BounceFactor(1.f), _CollisionBehaviour(bounce)
{
}

// dtor

CPSZone::~CPSZone()
{	
	// release the collisionInfos we've querried
	
	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{		
		releaseTargetRsc(*it) ;
	}
}


/** Add a new type of located for this zone to apply on. 
* We override this to queery the target to allocate the CollisionInfo attribute
*/
void CPSZone::attachTarget(CPSLocated *ptr)
{
		
	CPSTargetLocatedBindable::attachTarget(ptr) ;
	ptr->queryCollisionInfo() ;
}




/// inherit from CPSTargetLocatedBindable. Its called when one of the targets has been detroyed
void CPSZone::releaseTargetRsc(CPSLocated *target)
{
	// tell the target that we were using collision infos and that we won't use them anymore
	target->releaseCollisionInfo() ;
}



void CPSZone::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	// for zone, the PSCollision pass and the PSToolRenderPass are processed
	switch(pass)
	{
		case PSCollision:
			performMotion(ellapsedTime) ;
		break ;
		case PSToolRender:
			show(ellapsedTime) ;
		break ;
	}
}


// build a basis with K = the normal of the plane
CMatrix CPSZonePlane::buildBasis(uint32 index) const
{
	CMatrix m ;
	m.translate(_Owner->getPos()[index]) ;	
	m = m * CPSUtil::buildSchmidtBasis(_Normal[index]) ;
	return m ;
}


/// this compute a new speed vector, so that the located will reach the correct position at the next speed integration
/// this create the illusion of collision
/*void CPSZone::bounce(uint32 locatedIndex, const CVector &bouncePoint, const CVector &surfNormal, float elasticity, CAnimationTime ellapsedTime)
{
	CVector &speed = _Owner->getSpeed()[locatedIndex] ;
	const CVector &pos   = _Owner->getPos()[locatedIndex]   ;
	CVector &bounceVect = elasticity  * (speed - 2.0f * (speed * surfNormal) * surfNormal) ; // speed vector after collision
	// now check where the located will be after integration
	CVector d = bouncePoint - pos ;
	CAnimationTime collideDelay = speed.norm() / d.norm() ;
	CVector finalPos = bouncePoint + (ellapsedTime - collideDelay) * bounceVect ;
	// now, we must have pos + ellapsedTime * newSpeed = finalPos 
	// newSpeed = alpha * (finalPos - pos)
	// so alpha = 1 / ellapsedTime

	speed = (1.0f / ellapsedTime) * (finalPos - pos) ;	
}*/


void CPSZonePlane::show(CAnimationTime)
{
	const float planeSize = 2.0f ;
	CVertexBuffer vb ;
	CMaterial mat ; 
	CPrimitiveBlock pb ;
	
    vb.setVertexFormat(IDRV_VF_XYZ) ;
    vb.setNumVertices(8) ;

	vb.setVertexCoord(0, CVector(-planeSize, -planeSize, 0)) ;
	vb.setVertexCoord(1, CVector(planeSize, -planeSize, 0)) ;
	vb.setVertexCoord(2, CVector(planeSize, planeSize, 0)) ;
	vb.setVertexCoord(3, CVector(-planeSize, planeSize, 0)) ;
	vb.setVertexCoord(4, CVector(0, 0, 0)) ;
	vb.setVertexCoord(5, CVector(0, 0, planeSize)) ;

	pb.reserveLine(6) ;

	pb.addLine(0, 1) ;
	pb.addLine(1, 2) ;
	pb.addLine(2, 3) ;
	pb.addLine(3, 0) ;
	pb.addLine(4, 5) ;

	setupDriverModelMatrix() ;

	IDriver *driver = getDriver() ;
	uint32 k  = 0 ;
	for (TPSAttribVector::const_iterator it = _Owner->getPos().begin() ; it != _Owner->getPos().end() ; ++it, ++k)
	{	
		driver->setupModelMatrix(getLocatedMat() * buildBasis(k)) ;
		driver->activeVertexBuffer(vb) ;
		driver->render(pb, mat) ;	
	}
	

}



void CPSZonePlane::resize(uint32 size)
{
	_Normal.resize(size) ;
}


void CPSZonePlane::newElement(void)
{
	nlassert(_Normal.getSize() != _Normal.getMaxSize()) ;
	_Normal.insert(CVector(0, 0, 1)) ;
}


void CPSZonePlane::deleteElement(uint32 index)
{
	_Normal.remove(index) ;
}



void CPSZonePlane::performMotion(CAnimationTime ellapsedTime)
{
	// for each target, we must check wether they are going through the plane
	// if so they must bounce
	TPSAttribVector::const_iterator planePosIt, planePosEnd, normalIt, targetPosIt, targetPosEnd ;
	CVector dest ;
	CPSCollisionInfo ci ;
	CVector startEnd ;
	uint32 k ;
	const TPSAttribVector *speedAttr ;
	float posSide, negSide ;

	// alpha is the ratio that gives the percent of endPos - startPos that hit the plane
	float alpha ; 

	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{

		speedAttr = &((*it)->getSpeed()) ;
		// cycle through the planes

		planePosEnd = _Owner->getPos().end() ;
		for (planePosIt = _Owner->getPos().begin(), normalIt = _Normal.begin() ; planePosIt != planePosEnd
				; ++planePosIt, ++normalIt)
		{
			
			// we must setup the plane in the good basis

			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner) ;

			

			NLMISC::CPlane p ;
			p.make(m.mulVector(*normalIt), m * (*planePosIt)) ;		
			
			// deals with each particle


			targetPosEnd = (*it)->getPos().end() ;
			for (targetPosIt = (*it)->getPos().begin(), k = 0 ; targetPosIt != targetPosEnd ; ++targetPosIt, ++k)
			{	
				const CVector &speed = (*speedAttr)[k] ;
				// check whether the located is going through the plane
				dest = *targetPosIt + ellapsedTime *  speed ;


				posSide = p * *targetPosIt ;
				negSide = p * dest ;
								
				if ((posSide * negSide) < 0.0f)
				{
				
						alpha = posSide / (posSide - negSide) ;
						startEnd = alpha * (dest - *targetPosIt) ;					
						ci.dist = startEnd.norm() ;
						// we translate the particle from an epsilon so that it won't get hooked to the plane
						ci.newPos = *targetPosIt  + startEnd + PSCollideEpsilon * p.getNormal() ;				
						ci.newSpeed = _BounceFactor * (speed - 2.0f * (speed * p.getNormal()) * p.getNormal()) ;

						ci.collisionZone = this ;						
						(*it)->collisionUpdate(ci, k) ;
					
				}
			}
		}
	}

}

void CPSZonePlane::applyMatrix(uint32 index, const CMatrix &m)
{
	nlassert(index < _Normal.getSize()) ;

	_Normal[index] = m.mulVector(_Normal[index]) ;
	CVector &p = _Owner->getPos()[index] ;
	p = m * p ;

}



CMatrix CPSZonePlane::getMatrix(uint32 index) const
{
	return buildBasis(index) ;
}


void CPSZonePlane::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'PSPL') ;
	CPSZone::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_Normal) ;
}



///////////////////////////
// sphere implementation //
///////////////////////////




void CPSZoneSphere::performMotion(CAnimationTime ellapsedTime)
{
	// for each target, we must check wether they are going through the plane
	// if so they must bounce

	TPSAttribRadiusPair::const_iterator radiusIt = _Radius.begin() ;
	TPSAttribVector::const_iterator spherePosIt, spherePosEnd, targetPosIt, targetPosEnd ;
	CVector dest ;
	CPSCollisionInfo ci ;
	CVector startEnd ;
	uint32 k ;
	const TPSAttribVector *speedAttr ;


	float rOut, rIn ;


	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{

		speedAttr = &((*it)->getSpeed()) ;


		// cycle through the spheres

		spherePosEnd = _Owner->getPos().end() ;
		for (spherePosIt = _Owner->getPos().begin() ; spherePosIt != spherePosEnd
				; ++spherePosIt, ++radiusIt)
		{
			
			// we must setup the sphere in the good basis

			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner) ;

			

			CVector center = m * *spherePosIt ;
						
			// deals with each particle


			targetPosEnd = (*it)->getPos().end() ;
			for (targetPosIt = (*it)->getPos().begin(), k = 0 ; targetPosIt != targetPosEnd ; ++targetPosIt, ++k)
			{	
				const CVector &speed = (*speedAttr)[k] ;
				const CVector &pos = *targetPosIt ;
				// check whether the located is going through the sphere

				// we don't use raytracing for now because it is too slow ...
					

				rOut = (pos - center) * (pos - center) ;

				// initial position outside the sphere ?
				if (rOut > radiusIt->R2)
				{
					dest = pos + ellapsedTime *  speed ;
					rIn = (dest - center) * (dest - center) ;

					const CVector D = dest - pos ;

					// final position inside the sphere ?

					if ( rIn <= radiusIt->R2)
					{				
						// discriminant of the intersection equation
						const float b = 2.f * (pos * D - D * center), a = D * D
									, c = (pos * pos) + (center * center) - 2.f * (pos * center) - radiusIt->R2  ;
						float d = b * b - 4 * a * c ;

						
						if (d <= 0.f) continue ; // should never happen, but we never know ...
						

						d = sqrtf(d) ;

						// roots of the equation, we take the smallest 


						const float invA = 1.f / a ;

						const float r1 = .5f * (-b - 2.f * d) * a
									, r2 = .5f * (-b - 2.f * d) * a ;

						const float  r = std::min(r1, r2) ;

						// collision point 

						const CVector C = pos  + r * D ;




				
				    
						const float alpha = ((C - pos) * D) * a ;

						const CVector startEnd = alpha * (dest - pos) ;					

						CVector normal = C - center ;
						normal = normal * (1.f / radiusIt->R) ;

					
						ci.dist = startEnd.norm() ;
						// we translate the particle from an epsilon so that it won't get hooked to the sphere
						ci.newPos = pos  + startEnd + PSCollideEpsilon * normal ;				
						ci.newSpeed = _BounceFactor * (speed - 2.0f * (speed * normal) * normal) ;

						ci.collisionZone = this ;						
						(*it)->collisionUpdate(ci, k) ;
						
					}					
				}
			}
		}
	}
}



void CPSZoneSphere ::show(CAnimationTime ellapsedTime)
{
	TPSAttribRadiusPair::const_iterator radiusIt = _Radius.begin() ;
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt = _Owner->getPos().end() ;
	setupDriverModelMatrix() ;	
	for ( ; posIt != endPosIt ; ++posIt, ++radiusIt) 
	{	
		CPSUtil::displaySphere(*getDriver(), radiusIt->R, *posIt, 5) ;
	}
}
	
void CPSZoneSphere::applyMatrix(uint32 index, const CMatrix &m)
{
	nlassert(index < _Radius.getSize()) ;
	// slow, but for edition purposes ...
	CVector I = _Radius[index].R * CVector::I ;
	I = m.mulVector(I) ;
	// compute new radius
	_Radius[index].R = I.norm() ;
	_Radius[index].R2 = _Radius[index].R * _Radius[index].R  ;

	// compute new pos
	CVector &p = _Owner->getPos()[index] ;
	p = m * p ;
}


CMatrix CPSZoneSphere::getMatrix(uint32 index) const
{
	nlassert(index < _Radius.getSize()) ;
	CMatrix m ;
	m.translate(_Owner->getPos()[index]) ;
	m.scale(CVector(_Radius[index].R, _Radius[index].R, _Radius[index].R)) ;
	return m ; 
}


void CPSZoneSphere::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'PSSP') ;
	CPSZone::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_Radius) ;	
}


		



void CPSZoneSphere::resize(uint32 size)
{
	_Radius.resize(size) ;
}

void CPSZoneSphere::newElement(void)
{
	CRadiusPair rp ;
	rp.R = rp.R2 = 1.f ;
	nlassert(_Radius.getSize() != _Radius.getMaxSize()) ;
	_Radius.insert(rp) ;
}

void CPSZoneSphere::deleteElement(uint32 index)
{
	_Radius.remove(index) ;
}


////////////////////////////////
// CPSZoneDisc implementation //
////////////////////////////////

void CPSZoneDisc::performMotion(CAnimationTime ellapsedTime)
{
	// for each target, we must check wether they are going through the disc
	// if so they must bounce
	TPSAttribVector::const_iterator discPosIt, discPosEnd, normalIt, targetPosIt, targetPosEnd ;
	TPSAttribRadiusPair::const_iterator radiusIt = _Radius.begin() ;
	CVector dest ;
	CPSCollisionInfo ci ;
	CVector startEnd ;
	uint32 k ;
	const TPSAttribVector *speedAttr ;
	float posSide, negSide ;

	// the square of radius at the hit point
	float hitRadius2 ; 

	// alpha is the ratio that gives the percent of endPos - startPos that hit the disc
	float alpha ; 

	CVector center ;

	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{

		speedAttr = &((*it)->getSpeed()) ;
		// cycle through the disc

		discPosEnd = _Owner->getPos().end() ;
		for (discPosIt = _Owner->getPos().begin(), normalIt = _Normal.begin() ; discPosIt != discPosEnd
				; ++discPosIt, ++normalIt, ++radiusIt)
		{
			
			// we must setup the disc in the good basis

			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner) ;

			

			NLMISC::CPlane p ;
			center = m * (*discPosIt) ;
			p.make(m.mulVector(*normalIt), center) ;		
			
			// deals with each particle


			targetPosEnd = (*it)->getPos().end() ;
			for (targetPosIt = (*it)->getPos().begin(), k = 0 ; targetPosIt != targetPosEnd ; ++targetPosIt, ++k)
			{	
				const CVector &speed = (*speedAttr)[k] ;
				// check whether the located is going through the disc
				dest = *targetPosIt + ellapsedTime *  speed ;


				posSide = p * *targetPosIt ;
				negSide = p * dest ;
								
				if ((posSide * negSide) < 0.0f)
				{									
						alpha = posSide / (posSide - negSide) ;
						startEnd = alpha * (dest - *targetPosIt) ;					
						ci.dist = startEnd.norm() ;
						// we translate the particle from an epsilon so that it won't get hooked to the disc
						ci.newPos = *targetPosIt  + startEnd + PSCollideEpsilon * p.getNormal() ;				


						// now, check the collision pos against radius

						hitRadius2 = (ci.newPos - center) * (ci.newPos - center) ;

						if (hitRadius2 < radiusIt->R2) // check collision against disc
						{
							ci.newSpeed = _BounceFactor * (speed - 2.0f * (speed * p.getNormal()) * p.getNormal()) ;						
							ci.collisionZone = this ;						
							(*it)->collisionUpdate(ci, k) ;
						}
					
				}
			}
		}
	}	
}

void CPSZoneDisc::show(CAnimationTime ellapsedTime)
{
	TPSAttribRadiusPair::const_iterator radiusIt = _Radius.begin() ;
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt = _Owner->getPos().end()
									, normalIt = _Normal.begin() ;
	setupDriverModelMatrix() ;	
	CMatrix mat ;
	for ( ; posIt != endPosIt ; ++posIt, ++radiusIt, ++normalIt) 
	{	
		mat = CPSUtil::buildSchmidtBasis(*normalIt) ;
		CPSUtil::displayDisc(*getDriver(), radiusIt->R, *posIt, mat) ;
	}	
}
	





void CPSZoneDisc::applyMatrix(uint32 index, const CMatrix &m)
{
	nlassert(index < _Radius.getSize()) ;
	// slow, but for edition purposes ...
	CVector I = _Radius[index].R * CVector::I ;
	I = m.mulVector(I) ;
	// compute new radius
	_Radius[index].R = I.norm() ;
	_Radius[index].R2 = _Radius[index].R * _Radius[index].R  ;

	// compute new pos
	CVector &p = _Owner->getPos()[index] ;
	p = m * p ;

	// compute new normal
	_Normal[index] = m.mulVector(_Normal[index]) ;
}



CMatrix CPSZoneDisc::getMatrix(uint32 index) const
{
	CMatrix m ;
	m.translate(_Owner->getPos()[index]) ;	
	m.scale(CVector(_Radius[index].R, _Radius[index].R, _Radius[index].R)) ;
	m = m * CPSUtil::buildSchmidtBasis(_Normal[index]) ;
	return m ;	
}


void CPSZoneDisc::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'PSSP') ;
	CPSZone::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_Normal) ;		
	f.serial(_Radius) ;		
}

void CPSZoneDisc::resize(uint32 size)
{
	_Radius.resize(size) ;
	_Normal.resize(size) ;
}

void CPSZoneDisc::newElement(void)
{
	CRadiusPair rp ;
	rp.R = rp.R2 = 1.f ;
	nlassert(_Radius.getSize() != _Radius.getMaxSize()) ;
	_Radius.insert(rp) ;
	_Normal.insert(CVector::K) ;
}

void CPSZoneDisc::deleteElement(uint32 index)
{
	_Radius.remove(index) ;
	_Normal.remove(index) ;
}




} // NL3D
