/** \file ps_zone.cpp
 * <File description>
 *
 * $Id: ps_zone.cpp,v 1.8 2001/06/15 16:24:44 corvazier Exp $
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

#include "3d/ps_zone.h"
#include "3d/vertex_buffer.h"
#include "3d/primitive_block.h"
#include "3d/material.h"
#include "3d/ps_util.h"
#include "3d/dru.h"
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
	setupDriverModelMatrix() ;

	IDriver *driver = getDriver() ;
	uint k  = 0 ;

	setupDriverModelMatrix() ;

	for (TPSAttribVector::const_iterator it = _Owner->getPos().begin() ; it != _Owner->getPos().end() ; ++it, ++k)
	{	
		CMatrix mat = buildBasis(k) ;

		CPSUtil::displayBasis(getLocatedMat(), mat, 1.f, *getFontGenerator(), *getFontManager()) ;


		setupDriverModelMatrix() ;
	
		CDRU::drawLine(*it + (planeSize  + 3) * mat.getI() + planeSize * mat.getJ() 
						, *it - (planeSize + 3) * mat.getI() + planeSize * mat.getJ()
						, CRGBA::White
						, *driver) ;

		CDRU::drawLine(*it + (planeSize  + 3) * mat.getI() - planeSize * mat.getJ() 
						, *it - (planeSize + 3) * mat.getI() - planeSize * mat.getJ()
						, CRGBA::White
						, *driver) ;

		CDRU::drawLine(*it + planeSize  * mat.getI() + (planeSize + 3) * mat.getJ() 
						, *it + planeSize * mat.getI() - (planeSize + 3) * mat.getJ()
						, CRGBA::White
						, *driver) ;
		CDRU::drawLine(*it - planeSize  * mat.getI() + (planeSize + 3) * mat.getJ() 
						, *it - planeSize * mat.getI() - (planeSize + 3) * mat.getJ()
						, CRGBA::White
						, *driver) ;
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

						const float r1 = .5f * (-b + 2.f * d) * a
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

		mat.setPos(*posIt) ;
		CPSUtil::displayBasis(getLocatedMat(), mat, 1.f, *getFontGenerator(), *getFontManager()) ;				
		setupDriverModelMatrix() ;			
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


////////////////////////////////////
// CPSZoneCylinder implementation //
////////////////////////////////////


/*
void CPSZoneCylinder::performMotion(CAnimationTime ellapsedTime)
{
	TPSAttribVector::const_iterator dimIt = _Dim.begin() ;
	CPSAttrib<CPlaneBasis>::const_iterator basisIt = _Basis.begin() ;
	TPSAttribVector::const_iterator cylinderPosIt, cylinderPosEnd, targetPosIt, targetPosEnd ;
	CVector dest ;
	CPSCollisionInfo ci ;
	CVector startEnd ;
	uint32 k ;
	const TPSAttribVector *speedAttr ;



	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{

		speedAttr = &((*it)->getSpeed()) ;


		// cycle through the cylinders

		cylinderPosEnd = _Owner->getPos().end() ;
		for (cylinderPosIt = _Owner->getPos().begin() ; cylinderPosIt != cylinderPosEnd
				; ++cylinderPosIt, ++dimIt, ++basisIt)
		{
			
			// we must setup the cylinder in the good basis

			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner) ;

			

			// compute the new center pos
			CVector center = m * *cylinderPosIt ;

			// compute a basis for the cylinder
			CVector I = m.mulVector(basisIt->X) ;
			CVector J = m.mulVector(basisIt->Y) ;
			CVector K = m.mulVector(basisIt->X ^ basisIt->Y) ;

						
			// the pos projected (and scale) over the plane basis of the cylinder, the pos minus the center
			CVector projectedPos, tPos ;

			// the same, but with the final position
			CVector destProjectedPos, destTPos ;


			// deals with each particle
			targetPosEnd = (*it)->getPos().end() ;
			for (targetPosIt = (*it)->getPos().begin(), k = 0 ; targetPosIt != targetPosEnd ; ++targetPosIt, ++k)
			{	
				const CVector &speed = (*speedAttr)[k] ;
				const CVector &pos = *targetPosIt ;

				
				
				// check wether current pos was outside the cylinder


				tPos = pos - center ;
				projectedPos = (1 / dimIt->x) * (I * tPos) * I + (1 / dimIt->y)  * (J * tPos) * J ;
				
				if (!
					 (
						((tPos * K) < dimIt->z)
						&& ((tPos * K) > -dimIt->z)
						&& (projectedPos * projectedPos < 1.f) 
					 )
				   )
				{
					dest = pos + ellapsedTime *  speed ;
					destTPos = dest - center ;
					destProjectedPos = (1.f / dimIt->x) * (I * tPos) * I + (1.f / dimIt->y)  * (J * tPos) * J ;

					// test wether the new position is inside the cylinder

					
					if (!
						 (
							((destTPos * K) < dimIt->z)
							&& ((destTPos * K) > -dimIt->z)
							&& (destProjectedPos * destProjectedPos < 1.f) 
						 )
					   )
					{
						// now, detect the closest hit point (the smallest alpha, with alpha, the percent of the move vector
						// to reach the hit point)

						const float epsilon = 10E-6f ;

						float alphaTop, alphaBottom, alphaCyl ;

						const float denum = (dest - pos) * K ;

						// top plane

						if (fabs(denum) < epsilon)
						{
							alphaTop = (dimIt->z - (tPos * K)) / denum ;
							if (alphaTop < 0.f) alphaTop = 1.f ;
						}
						else
						{
							alphaTop = 1.f ;
						}

						// bottom plane

						if (fabs(denum) < epsilon)
						{
							alphaBottom = (- dimIt->z - (tPos * K)) / denum ;
							if (alphaBottom < 0.f) alphaBottom = 1.f ;
						}
						else
						{
							alphaBottom = 1.f ;
						}

						// cylinder

						//expressed the src and dest positions in the cylinder basis

						const float ox = tPos * I, oy = tPos * J, dx = (destTPos - tPos) * I, dy = (destTPos - tPos) * J ;

						// coefficients of the equation : a * alpha ^ 2 + b * alpha + c = 0
						const float a = (dx * dx) / (dimIt->x * dimIt->x)
								  + (dy * dy) / (dimIt->y * dimIt->y) ;
						const float b = 2.f * (ox * dx) / (dimIt->x * dimIt->x)
								  + (oy * dy) / (dimIt->y * dimIt->y) ;
						const float c = ox * ox + oy * oy - 1 ;

						// discriminant
						const float delta = b * b - 4.f * a * c ;

						if (delta < epsilon)
						{
							alphaCyl = 1.f ;
						}
						else
						{
							const float deltaRoot = sqrtf(delta) ;
							const float r1 = (- b - deltaRoot) / (2.f / a) ;
							const float r2 = (- b - deltaRoot) / (2.f / a) ;

							if (r1 < 0.f) alphaCyl = r2 ;
								else if (r2 < 0.f) alphaCyl = r1 ;
									else alphaCyl = r1 < r2 ? r1 : r2 ;
						}


						// now, choose the minimum positive dist
						
						if (alphaTop < alphaBottom && alphaTop < alphaCyl)
						{
							// collision with the top plane
							CVector startEnd = alphaTop * (dest - pos) ;
							ci.newPos = pos + startEnd + PSCollideEpsilon * K ;
							ci.dist = startEnd.norm() ;
							ci.newSpeed = (-2.f * (speed * K)) * K + speed ;
							ci.collisionZone = this ;
							
							(*it)->collisionUpdate(ci, k) ;
						}
						else
							if (alphaBottom < alphaCyl)
							{	
								// collision with the bottom plane
								CVector startEnd = alphaBottom * (dest - pos) ;
								ci.newPos = pos + startEnd - PSCollideEpsilon * K ;
								ci.dist = startEnd.norm() ;
								ci.newSpeed = (-2.f * (speed * K)) * K + speed ;
								ci.collisionZone = this ;
						
								
								(*it)->collisionUpdate(ci, k) ;
							}
							else
							{
								// collision with the cylinder

								CVector startEnd = alphaCyl * (dest - pos) ;

								// normal at the hit point. It is the gradient of the implicit equation x^2 / a^2 + y^2 / b^2 - R^ 2=  0
								// so we got unormalized n =  (2 x / a ^ 2, 2 y / b ^ 2, 0) in the basis of the cylinder
								// As we'll normalize it, we don't need  the 2 factor
								
								float px = ox + alphaCyl * dx ;
								float py = oy + alphaCyl * dy ;

								CVector normal = px / (dimIt->x * dimIt->x) * I + py / (dimIt->y * dimIt->y) * J ;
								normal.normalize() ;

								ci.newPos = pos + startEnd - PSCollideEpsilon * normal ;
								ci.dist = startEnd.norm() ;
								ci.newSpeed = (-2.f * (speed * normal)) * normal + speed ;
								ci.collisionZone = this ;

								(*it)->collisionUpdate(ci, k) ;
							}						

					}
				}									
			}
		}
	}
}
*/



void CPSZoneCylinder::performMotion(CAnimationTime ellapsedTime)
{
	TPSAttribVector::const_iterator dimIt = _Dim.begin() ;
	CPSAttrib<CPlaneBasis>::const_iterator basisIt = _Basis.begin() ;
	TPSAttribVector::const_iterator cylinderPosIt, cylinderPosEnd, targetPosIt, targetPosEnd ;
	CVector dest ;
	CPSCollisionInfo ci ;
	CVector startEnd ;
	uint32 k ;
	const TPSAttribVector *speedAttr ;



	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{

		speedAttr = &((*it)->getSpeed()) ;


		// cycle through the cylinders

		cylinderPosEnd = _Owner->getPos().end() ;
		for (cylinderPosIt = _Owner->getPos().begin() ; cylinderPosIt != cylinderPosEnd
				; ++cylinderPosIt, ++dimIt, ++basisIt)
		{
			
			// we must setup the cylinder in the good basis

			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner) ;

			

			// compute the new center pos
			CVector center = m * *cylinderPosIt ;

			// compute a basis for the cylinder
			CVector I = m.mulVector(basisIt->X) ;
			CVector J = m.mulVector(basisIt->Y) ;
			CVector K = m.mulVector(basisIt->X ^ basisIt->Y) ;

						
			// the pos projected (and scale) over the plane basis of the cylinder, the pos minus the center
			CVector projectedPos, tPos ;

			// the same, but with the final position
			CVector destProjectedPos, destTPos ;


			// deals with each particle
			targetPosEnd = (*it)->getPos().end() ;
			for (targetPosIt = (*it)->getPos().begin(), k = 0 ; targetPosIt != targetPosEnd ; ++targetPosIt, ++k)
			{	
				const CVector &speed = (*speedAttr)[k] ;
				const CVector &pos = *targetPosIt ;

				
				
				// check wether current pos was outside the cylinder


				tPos = pos - center ;
				projectedPos = (1 / dimIt->x) * (I * tPos) * I + (1 / dimIt->y)  * (J * tPos) * J ;
				
				if (!
					 (
						((tPos * K) < dimIt->z)
						&& ((tPos * K) > -dimIt->z)
						&& (projectedPos * projectedPos < 1.f) 
					 )
				   )
				{
					dest = pos + ellapsedTime *  speed ;
					destTPos = dest - center ;
					destProjectedPos = (1.f / dimIt->x) * (I * destTPos) * I + (1.f / dimIt->y)  * (J * destTPos) * J ;

					// test wether the new position is inside the cylinder

					
					if (
						 (
							((destTPos * K) < dimIt->z)
							&& ((destTPos * K) > -dimIt->z)
							&& (destProjectedPos * destProjectedPos < 1.f) 
						 )
					   )
					{
						// now, detect the closest hit point (the smallest alpha, with alpha, the percent of the move vector
						// to reach the hit point)

						const float epsilon = 10E-6f ;

						float alphaTop, alphaBottom, alphaCyl ;

						const float denum = (dest - pos) * K ;

						// top plane

						if (fabs(denum) < epsilon)
						{
							alphaTop = (dimIt->z - (tPos * K)) / denum ;
							if (alphaTop < 0.f) alphaTop = 1.f ;
						}
						else
						{
							alphaTop = 1.f ;
						}

						// bottom plane

						if (fabs(denum) < epsilon)
						{
							alphaBottom = (- dimIt->z - (tPos * K)) / denum ;
							if (alphaBottom < 0.f) alphaBottom = 1.f ;
						}
						else
						{
							alphaBottom = 1.f ;
						}

						// cylinder

						//expressed the src and dest positions in the cylinder basis

						const float ox = tPos * I, oy = tPos * J, dx = (destTPos - tPos) * I, dy = (destTPos - tPos) * J ;

						// coefficients of the equation : a * alpha ^ 2 + b * alpha + c = 0
						const float a = (dx * dx) / (dimIt->x * dimIt->x)
								  + (dy * dy) / (dimIt->y * dimIt->y) ;
						const float b = 2.f * ((ox * dx) / (dimIt->x * dimIt->x)
								  + (oy * dy) / (dimIt->y * dimIt->y)) ;
						const float c = (ox * ox) / (dimIt->x * dimIt->x) + (oy * oy) / (dimIt->y * dimIt->y)  - 1 ;

						// discriminant
						const float delta = b * b - 4.f * a * c ;

						if (delta < epsilon)
						{
							alphaCyl = 1.f ;
						}
						else
						{
							const float deltaRoot = sqrtf(delta) ;
							const float r1 = (- b + 2.f * deltaRoot) / (2.f * a) ;
							const float r2 = (- b - 2.f * deltaRoot) / (2.f * a) ;

							if (r1 < 0.f) alphaCyl = r2 ;
								else if (r2 < 0.f) alphaCyl = r1 ;
									else alphaCyl = r1 < r2 ? r1 : r2 ;

									if (alphaCyl < 0.f) alphaCyl = 1.f ;
						}


						// now, choose the minimum positive dist

					
						
						if (alphaTop < alphaBottom && alphaTop < alphaCyl)
						{
							// collision with the top plane
							CVector startEnd = alphaTop * (dest - pos) ;
							ci.newPos = pos + startEnd + PSCollideEpsilon * K ;
							ci.dist = startEnd.norm() ;
							ci.newSpeed = (-2.f * (speed * K)) * K + speed ;
							ci.collisionZone = this ;
							
							(*it)->collisionUpdate(ci, k) ;
						}
						else
							if (alphaBottom < alphaCyl)
							{	
								// collision with the bottom plane
								CVector startEnd = alphaBottom * (dest - pos) ;
								ci.newPos = pos + startEnd - PSCollideEpsilon * K ;
								ci.dist = startEnd.norm() ;
								ci.newSpeed = (-2.f * (speed * K)) * K + speed ;
								ci.collisionZone = this ;
						
								
								(*it)->collisionUpdate(ci, k) ;
							}
							else
							{
								// collision with the cylinder

								CVector startEnd = alphaCyl * (dest - pos) ;

								// normal at the hit point. It is the gradient of the implicit equation x^2 / a^2 + y^2 / b^2 - R^ 2=  0
								// so we got unormalized n =  (2 x / a ^ 2, 2 y / b ^ 2, 0) in the basis of the cylinder
								// As we'll normalize it, we don't need  the 2 factor
								
								float px = ox + alphaCyl * dx ;
								float py = oy + alphaCyl * dy ;

								CVector normal = px / (dimIt->x * dimIt->x) * I + py / (dimIt->y * dimIt->y) * J ;
								normal.normalize() ;

								ci.newPos = pos + startEnd + PSCollideEpsilon * normal ;
								ci.dist = startEnd.norm() ;
								ci.newSpeed = (-2.f * (speed * normal)) * normal + speed ;
								ci.collisionZone = this ;

								(*it)->collisionUpdate(ci, k) ;
							}						

					}
				}									
			}
		}
	}
}

void CPSZoneCylinder::show(CAnimationTime ellapsedTime)
{
	TPSAttribVector::const_iterator dimIt = _Dim.begin()
									,posIt = _Owner->getPos().begin()
									, endPosIt = _Owner->getPos().end() ;

	CPSAttrib<CPlaneBasis>::const_iterator basisIt = _Basis.begin() ;
									
	setupDriverModelMatrix() ;	
	CMatrix mat ;
	for ( ; posIt != endPosIt ; ++posIt, ++dimIt, ++basisIt) 
	{			
		mat.setRot(basisIt->X, basisIt->Y, basisIt->X ^ basisIt->Y) ;
		mat.setPos(CVector::Null) ;
		
		CPSUtil::displayCylinder(*getDriver(), *posIt, mat, *dimIt, 32) ; 

		mat.setPos(*posIt) ;
		CPSUtil::displayBasis(getLocatedMat(), mat, 1.f, *getFontGenerator(), *getFontManager()) ;				
		setupDriverModelMatrix() ;	
	
	}
}
	


void CPSZoneCylinder::applyMatrix(uint32 index, const CMatrix &m)
{
	CVector Z = _Basis[index].X ^ _Basis[index].Y ;
	// rescale dimensions
	CVector dim ;
	
	
	dim =  m.mulVector(_Dim[index].x * _Basis[index].X) ;
	_Dim[index].x = dim.norm() ;

	dim =  m.mulVector(_Dim[index].y * _Basis[index].Y) ;
	_Dim[index].y = dim.norm() ;

	dim =  m.mulVector(_Dim[index].z * Z) ;
	_Dim[index].z = dim.norm() ;


	// transform the basis
	
	_Basis[index].X = m.mulVector(_Basis[index].X) ;
	_Basis[index].X.normalize() ;

	_Basis[index].Y = m.mulVector(_Basis[index].Y) ;
	_Basis[index].Y.normalize() ;


	// compute new pos
	CVector &p = _Owner->getPos()[index] ;
	p = m * p ;
				 
}



CMatrix CPSZoneCylinder::getMatrix(uint32 index) const
{
	CMatrix m ;
	m.setRot(_Dim[index].x *  _Basis[index].X, _Dim[index].y *_Basis[index].Y 
			 , _Dim[index].z * (_Basis[index].X ^ _Basis[index].Y)) ;

	m.setPos(_Owner->getPos()[index]) ;	
	return m ;
}


void CPSZoneCylinder::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'ZCYL') ;
	CPSZone::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_Basis) ;		
	f.serial(_Dim)  ;
}



void CPSZoneCylinder::resize(uint32 size)
{
	_Basis.resize(size) ;
	_Dim.resize(size) ;
}

void CPSZoneCylinder::newElement(void)
{
	_Basis.insert(CPlaneBasis(CVector::K)) ;
	_Dim.insert(CVector(1, 1, 1)) ;
}

void CPSZoneCylinder::deleteElement(uint32 index)
{
	_Basis.remove(index) ;
	_Dim.remove(index) ;
}


//////////////////////////////////////////////
//	implementation of CPSZoneRectangle      //
//////////////////////////////////////////////





void CPSZoneRectangle::performMotion(CAnimationTime ellapsedTime)
{
	// for each target, we must check wether they are going through the rectangle
	// if so they must bounce
	TPSAttribVector::const_iterator rectanglePosIt, rectanglePosEnd, targetPosIt, targetPosEnd ;
	CPSAttrib<CPlaneBasis>::const_iterator basisIt = _Basis.begin() ;
	TPSAttribFloat::const_iterator widthIt = _Width.begin(), heightIt = _Height.begin() ;

	CVector dest ;
	CPSCollisionInfo ci ;
	CVector startEnd ;
	uint32 k ;
	const TPSAttribVector *speedAttr ;
	float posSide, negSide ;
	
	// alpha is the ratio that gives the percent of endPos - startPos that hit the rectangle
	float alpha ; 

	CVector center ;

	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{

		speedAttr = &((*it)->getSpeed()) ;
		// cycle through the rectangle

		rectanglePosEnd = _Owner->getPos().end() ;
		for (rectanglePosIt = _Owner->getPos().begin() ; rectanglePosIt != rectanglePosEnd
				; ++rectanglePosIt, ++basisIt, ++widthIt, ++heightIt)
		{
			
			// we must setup the rectangle in the good basis

			const CMatrix &m = CPSLocated::getConversionMatrix(*it, this->_Owner) ;

			

			NLMISC::CPlane p ;
			center = m * (*rectanglePosIt) ;

			const CVector X = m.mulVector(basisIt->X) ;
			const CVector Y = m.mulVector(basisIt->Y) ;

			p.make(X ^ Y, center) ;		
			
			// deals with each particle


			targetPosEnd = (*it)->getPos().end() ;
			for (targetPosIt = (*it)->getPos().begin(), k = 0 ; targetPosIt != targetPosEnd ; ++targetPosIt, ++k)
			{	
				const CVector &pos = *targetPosIt ;
				const CVector &speed = (*speedAttr)[k] ;
				// check whether the located is going through the rectangle
				dest = pos + ellapsedTime *  speed ;


				posSide = p * pos ;
				negSide = p * dest ;
								
				if ((posSide * negSide) < 0.0f)
				{									
						alpha = posSide / (posSide - negSide) ;
						startEnd = alpha * (dest - pos) ;					
						ci.dist = startEnd.norm() ;
						// we translate the particle from an epsilon so that it won't get hooked to the rectangle
						ci.newPos = pos + startEnd ;				
					

						

						if ( fabs( (pos - center) * X ) < *widthIt && fabs( (pos - center) * Y ) < *heightIt) // check collision against rectangle
						{
							ci.newPos += PSCollideEpsilon * p.getNormal() ;
							ci.newSpeed = _BounceFactor * (speed - 2.0f * (speed * p.getNormal()) * p.getNormal()) ;						
							ci.collisionZone = this ;						
							(*it)->collisionUpdate(ci, k) ;
						}
					
				}
			}	
		}
	}
}


void CPSZoneRectangle::show(CAnimationTime ellapsedTime)
{
	nlassert(_Owner) ;
	const uint size = _Owner->getSize() ;
	if (!size) return ;
	setupDriverModelMatrix() ;	
	CMatrix mat ;
	
	for (uint k = 0 ; k < size ; ++k) 
	{	
		const CVector &I = _Basis[k].X ;
		const CVector &J = _Basis[k].Y ;
		mat.setRot(I, J , I ^J) ;
		mat.setPos(_Owner->getPos()[k]) ;
		CPSUtil::displayBasis(getLocatedMat(), mat, 1.f, *getFontGenerator(), *getFontManager()) ;				
		setupDriverModelMatrix() ;	

		const CVector &pos = _Owner->getPos()[k] ;
		CPSUtil::display3DQuad(*getDriver(), pos + I * _Width[k] + J * _Height[k]
										   , pos + I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] + J * _Height[k]) ;
	}
}
	
void CPSZoneRectangle::applyMatrix(uint32 index, const CMatrix &m)
{
	nlassert(_Owner) ;

	_Owner->getPos()[index] = m * _Owner->getPos()[index] ;
	CVector w = _Width[index] * _Basis[index].X ;
	_Width[index] = (m.mulVector(w)).norm() ;

	CVector h = _Height[index] * _Basis[index].Y ;

	_Height[index] = (m.mulVector(h)).norm() ;

	_Basis[index].X = m.mulVector(_Basis[index].X) ;
	_Basis[index].X.normalize() ;

	_Basis[index].Y = m.mulVector(_Basis[index].Y) ;
	_Basis[index].Y.normalize() ;
}


CMatrix CPSZoneRectangle::getMatrix(uint32 index) const
{
	nlassert(_Owner) ;
	CMatrix m ;
	CVector N = _Basis[index].X ^ _Basis[index].Y ;
	CVector I = _Width[index] * _Basis[index].X ;
	CVector J = _Height[index] * _Basis[index].Y ;
	m.setRot(I, J, N) ;
	m.setPos(_Owner->getPos()[index]) ;
	return m ;
}

void CPSZoneRectangle::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'ZREC') ;
	CPSZone::serial(f) ;
	f.serialVersion(1) ;
	f.serial(_Basis) ;	
	f.serial(_Width) ;
	f.serial(_Height) ;
}


void CPSZoneRectangle::resize(uint32 size)
{
	_Basis.resize(size) ;
	_Width.resize(size) ;
	_Height.resize(size) ;
}

void CPSZoneRectangle::newElement(void)
{
	_Basis.insert(CPlaneBasis(CVector::K)) ;
	_Width.insert(1.f) ;
	_Height.insert(1.f) ;
}

void CPSZoneRectangle::deleteElement(uint32 index)
{
	_Basis.remove(index) ;
	_Width.remove(index) ;
	_Height.remove(index) ;
}



} // NL3D
