/** \file ps_zone.cpp
 * <File description>
 *
 * $Id: ps_zone.cpp,v 1.1 2001/04/25 08:45:37 vizerie Exp $
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


namespace NL3D {


/*
 * Constructor
 */
CPSZone::CPSZone()
{
}


/** Add a new type of located for this zone to apply on. 
* We override this to queery the target to allocate the CollisionInfo attribute
*/
void CPSZone::attachTarget(CSmartPtr<CPSLocated> ptr)
{
	CPSForce::attachTarget(ptr) ;
	ptr->queryCollisionInfo() ;
}


/** Detach a target.
* We override this to queery the target to deallocate the CollisionInfo attribute
*/

void CPSZone::detachTarget(CSmartPtr<CPSLocated> ptr)
{
	ptr->releaseCollisionInfo() ;
	CPSForce::detachTarget(ptr) ;
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


/// this compute a new speed vector, so that the located will reach the correct position at the next speed integration
/// this create the illusion of collision
void CPSZone::bounce(uint32 locatedIndex, const CVector &bouncePoint, const CVector &surfNormal, float elasticity, CAnimationTime ellapsedTime)
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
}


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
		CMatrix planeMat ;
		planeMat.translate((*it)) ;

		driver->setupModelMatrix(getLocatedMat() * planeMat  * CPSUtil::buildSchmidtBasis(_Normal[k]) ) ;
		driver->activeVertexBuffer(vb) ;
		driver->render(pb, mat) ;	
	}
	

}



void CPSZonePlane::resize(uint32 size)
{
	_Normal.resize(size) ;
}


bool CPSZonePlane::newElement(void)
{
	return _Normal.insert(CVector(0, 0, 1)) ;
}


void CPSZonePlane::deleteElement(uint32 index)
{
	_Normal.remove(index) ;
}



void CPSZonePlane::performMotion(CAnimationTime ellapsedTime)
{

}



} // NL3D
