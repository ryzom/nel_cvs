/** \file move_primitive.cpp
 * Description of movables primitives
 *
 * $Id: move_primitive.cpp,v 1.7 2001/06/08 15:38:28 legros Exp $
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

#include "pacs/move_primitive.h"
#include "pacs/collision_desc.h"
#include "pacs/move_element.h"
#include "nel/misc/common.h"

using namespace NLMISC;

namespace NLPACS 
{

// ***************************************************************************

CMovePrimitive::CMovePrimitive (CMoveContainer* container)
{
	_CollisionMask=0xffffffff;
	_OcclusionMask=0xffffffff;
	_Attenuation=1;
	_Container=container;
	_Flags=0;
	_BBXMin=-FLT_MAX;
	_BBXMax=-FLT_MAX;
	_BBYMin=-FLT_MAX;
	_BBYMax=-FLT_MAX;
	_RootOTInfo=NULL;
	_LastTestTime=0xffffffff;
	for (uint i=0; i<4; i++)
		_MoveElement[i]=NULL;
}

// ***************************************************************************

CMovePrimitive::~CMovePrimitive ()
{
	for (uint i=0; i<4; i++)
		if (_MoveElement[i])
			removeMoveElement (i);
}

// ***************************************************************************

bool CMovePrimitive::evalCollision (CMovePrimitive& other, CCollisionDesc& desc, double timeMin, double timeMax, uint32 testTime, 
									uint32 maxTestIteration, double &firstContactTime, double &lastContactTime)
{
	// Mask test
	if (( (_CollisionMask & other._OcclusionMask)  == 0) && ( (_OcclusionMask & other._CollisionMask)  == 0))
		return false;

	// Test time
	if ( (!checkTestTime (testTime, maxTestIteration)) || (!other.checkTestTime (testTime, maxTestIteration)) )
		return false;

	// Clear time min time max
	firstContactTime=FLT_MAX;
	lastContactTime=-FLT_MAX;

	// Switch the good test
	switch (((uint32)_Flags)&PrimitiveMask)
	{

	// Static box over...
	case _2DOrientedBox:
		{
			// Switch second type
			switch (((uint32)other._Flags)&PrimitiveMask)
			{
			
			// Static box over movable box
			case _2DOrientedBox:
				// Make the test
				return evalCollisionOBoverOB (other, desc, timeMin, timeMax, firstContactTime, lastContactTime);

			// Static box over movable cylinder
			case _2DOrientedCylinder:
				// Make the test
				return evalCollisionOBoverOC (other, desc, timeMin, timeMax, firstContactTime, lastContactTime);

			default:
			// Should not go here
			nlstop;
			}
		}

	// Static box over...
	case _2DOrientedCylinder:
		{
			// Switch second type
			switch (((uint32)other._Flags)&PrimitiveMask)
			{
			
			// Static box over movable box
			case _2DOrientedBox:
				{
					// Make the test
					bool collid=other.evalCollisionOBoverOC (*this, desc, timeMin, timeMax, firstContactTime, lastContactTime);
					if (collid)
						desc.XChgContactNormals ();
					return collid;
				}

			// Static box over movable cylinder
			case _2DOrientedCylinder:
				// Make the test
				return evalCollisionOCoverOC (other, desc, timeMin, timeMax, firstContactTime, lastContactTime);

			default:
			// Should not go here
			nlstop;
			}
		}

	default:
		// Should not go here
		nlstop;
	}	

	return false;
}

// ***************************************************************************

const TCollisionSurfaceDescVector *CMovePrimitive::evalCollision (CGlobalRetriever &retriever, CCollisionSurfaceTemp& surfaceTemp, 
																  const CVector& delta, uint32 testTime, uint32 maxTestIteration)
{
	// Test time
	if (!checkTestTime (testTime, maxTestIteration))
		return NULL;

	// Switch the good test
	if ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox)
	{
		// Local I
		CVector locI ((float)(_OBData.EdgeDirectionX[0]*_OBData.Length[0]/2.0), (float)(_OBData.EdgeDirectionY[0]*_OBData.Length[1]/2.0), 0);

		// Local J
		CVector locJ ((float)(_OBData.EdgeDirectionX[1]*_OBData.Length[0]/2.0), (float)(_OBData.EdgeDirectionY[1]*_OBData.Length[1]/2.0), 0);

		// Test
		return &retriever.testBBoxMove (_Position.getGlobalPos (), delta, locI, locJ, surfaceTemp);
	}
	else
	{
		// Check
		nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedCylinder);

		// Test
		return &retriever.testCylinderMove (_Position.getGlobalPos (), delta, _OCData.Radius, surfaceTemp);
	}
}

// ***************************************************************************

void CMovePrimitive::doMove (CGlobalRetriever &retriever, CCollisionSurfaceTemp& surfaceTemp, double timeMax)
{
	// Delta time
	float deltaTime=(float)(timeMax-_InitTime);
	
	// Delta pos..
	CVector delta=_Speed;
	delta*=deltaTime;

	// Make the move
	_Position.setGlobalPos (retriever.doMove(_Position.getGlobalPos(), delta, 1, surfaceTemp, false), retriever);

	// Final position
	_InitTime=timeMax;
}

// ***************************************************************************

void CMovePrimitive::doMove (double timeMax)
{
	// Make the move
	_Position.setPos (_Position.getPos ()+_Speed*(timeMax-_InitTime));

	// Final position
	_InitTime=timeMax;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionOBoverOB (CMovePrimitive& other, CCollisionDesc& desc, double timeMin, double timeMax, 
											double &firstContactTime, double &lastContactTime)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedBox);

	// Find a collision
	bool find=false;

	// Best time
	desc.ContactTime=FLT_MAX;

	// Timemin
	double _timeMax=-FLT_MAX;

	// Check movable points over the edge
	uint pt;
	uint seg;
	for (pt=0; pt<4; pt++)
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if ( evalCollisionPoverS (other, d, pt, seg) )
		{
			// Find
			find=true;

			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
			}

			// Best max time ?
			if (d.ContactTime>_timeMax)
			{
				// This is the new max time
				_timeMax=d.ContactTime;
			}
		}
	}

	// Check static points over the movable box
	for (pt=0; pt<4; pt++)
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if (other.evalCollisionPoverS (*this, d, pt, seg))
		{
			// Find
			find=true;

			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
			}

			// Best max time ?
			if (d.ContactTime>_timeMax)
			{
				// This is the new max time
				_timeMax=d.ContactTime;
			}
		}
	}

	if (find)
	{
		// First last contact time
		firstContactTime=desc.ContactTime;
		lastContactTime=_timeMax;

		// Half time
		//double halfTime = (_timeMax+desc.ContactTime)/2.0;

		// Collision in the past ?
		//if (timeMin > halfTime)
		if (timeMin > _timeMax)
			// yes, abort
			return false;

		// Collision not in the future ?
		if (timeMax>desc.ContactTime)
		{
			// Clamp time
			if (desc.ContactTime<timeMin)
				desc.ContactTime=timeMin;

			// yes, found it
			return true;
		}
	}

	// No collision found
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionOBoverOC (CMovePrimitive& other, CCollisionDesc& desc, double timeMin, double timeMax, 
											double &firstContactTime, double &lastContactTime)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedCylinder);

	// Find a collision
	bool find=false;

	// Best time
	desc.ContactTime=FLT_MAX;

	// time min clip
	double _timeMax = -FLT_MAX;

	// Check movable points over the cylinder
	uint pt;
	for (pt=0; pt<4; pt++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		double firstContactTime;
		double lastContactTime;
		if (evalCollisionPoverOC (other, d, pt, firstContactTime, lastContactTime))
		{
			// Found
			find=true;

			// Best time ?
			if (firstContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
			}

			// Best max time ?
			if (lastContactTime>_timeMax)
			{
				// New max time
				_timeMax=lastContactTime;
			}
		}
	}

	// Check static points over the movable box
	uint seg;
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if (evalCollisionSoverOC (other, d, seg))
		{
			// Found
			find=true;

			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
			}

			// Best max time ?
			if (d.ContactTime>_timeMax)
			{
				// New max time
				_timeMax=d.ContactTime;
			}
		}
	}

	if (find)
	{
		// First last contact time
		firstContactTime=desc.ContactTime;
		lastContactTime=_timeMax;

		// Half time
		//double halfTime = (_timeMax+desc.ContactTime)/2.0;

		// Collision in the past ?
		//if (timeMin > halfTime)
		if (timeMin > _timeMax)
			// yes, abort
			return false;

		// Collision not in the future ?
		if (timeMax>desc.ContactTime)
		{
			// Clamp time
			if (desc.ContactTime<timeMin)
				desc.ContactTime=timeMin;

			// yes, found it
			return true;
		}
	}

	// No collision found
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionPoverS (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, uint numSeg)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedBox);

	// Some constants
	const double normalSegX=other._OBData.EdgeDirectionY[numSeg];
	const double normalSegY=-other._OBData.EdgeDirectionX[numSeg];

  	// Relative speed
	const double speedX=other._Speed.x-_Speed.x;
	const double speedY=other._Speed.y-_Speed.y;

	// Dot product with the plan tangeante
	double dotProd= speedX*normalSegX + speedY*normalSegY;
	//if ( dotProd > 0 )
	if ( dotProd != 0 )
	{
		// Time of the collision
		double time= (normalSegX*(_OBData.PointPosX[numPoint] - other._OBData.PointPosX[numSeg]) + 
			normalSegY*(_OBData.PointPosY[numPoint] - other._OBData.PointPosY[numSeg])) / dotProd;

		// Position of segment point at collision time
		const double segPosX= other._OBData.PointPosX[numSeg] + other._Speed.x*time;
		const double segPosY= other._OBData.PointPosY[numSeg] + other._Speed.y*time;

		// Position of the point at collision time
		const double ptPosX= _OBData.PointPosX[numPoint] + _Speed.x*time;
		const double ptPosY= _OBData.PointPosY[numPoint] + _Speed.y*time;

		// Direction of the collision on the segment
		const double dirX= ptPosX - segPosX;
		const double dirY= ptPosY - segPosY;

		// Length of this vector
		const double length= dirY*normalSegX - dirX*normalSegY;

		// Included ?
		if ( ( length >= 0 ) && ( length <= other._OBData.Length[numSeg&1] ) )
		{
			// 2d Collid checked... Now check height
			
			// Pos Z
			const double pointSegZ=other._3dInitPosition.z;
			const double segPosZ= pointSegZ + other._Speed.z*time;

			// Some constants
			const double pointZ=_3dInitPosition.z;
			const double ptPosZ= pointZ + _Speed.z*time;

			// Included ?
			if ( (ptPosZ <= segPosZ + other._Height) && (ptPosZ + _Height >= segPosZ) )
			{
				// Ok Collision, fill the result
				
				// Time
				desc.ContactTime=time;

				// Position
				desc.ContactPosition.x=ptPosX;
				desc.ContactPosition.y=ptPosY;
				desc.ContactPosition.z=std::max (segPosZ, ptPosZ);

				// Seg box normal
				desc.ContactNormal1.x=normalSegX;
				desc.ContactNormal1.y=normalSegY;
				desc.ContactNormal1.z=0;
				desc.ContactNormal0.x=-desc.ContactNormal1.x;
				desc.ContactNormal0.y=-desc.ContactNormal1.y;
				desc.ContactNormal0.z=0;

				// End
				return true;
			}
		}
	}

	// No collision
	return false;
}

// ***************************************************************************

inline uint secondDegree (double a, double b, double c, double& s0, double& s1)
{
	double d=b*b-4.f*a*c;
	if (d>0)
	{
		// sqrt d
		d=(double)sqrt (d);

		// 1 / 2a
		a=0.5f/a;

		// 2 solutions
		s0 = (-b-d)*a;
		s1 = (-b+d)*a;

		return 2;
	}
	else if (d<0)
	{
		// No solution
		return 0;
	}
	else
	{
		// 1 solution
		s0 = -b/(2.f*a);

		return 1;
	}
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionPoverOC (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, 
										   double &firstContactTime, double &lastContactTime)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedCylinder);

	/* Point Equ:
	 * p(t) = p0 + v0*(t - t0)
	 *
	 * Cylinder center Equ:
	 * p'(t) = p'0 + v'0*(t - t'0)
	 *
	 * Find t for this equation:
	 * R² = Norm² (p(t) - p'(t))
	 * R² = Norm² ( p0 + v0 ( t - t0 ) - p'0 - v'0 ( t - t'0 ) )
	 *
	 * A = p0 - v0*t0 - p'0 + v'0*t'0
	 * B = (v0 - v'0)
	 *
	 * Norm² (B)*t² + 2*(A.B)*t + Norm² (A) - R² = 0
	 *
	 * a = Norm² (B)
	 * b = 2*(A.B)
	 * c = Norm² (A) - R²
	 *
	 * a*t² + b*t + c = 0
	 */

	// Let's go
	const double _Ax = _OBData.PointPosX[numPoint] - other._3dInitPosition.x;
	const double _Ay = _OBData.PointPosY[numPoint] - other._3dInitPosition.y;
	const double _Bx = _Speed.x - other._Speed.x;
	const double _By = _Speed.y - other._Speed.y;

	// Eval system
	double s0, s1;
	double squareRadius=other._OCData.Radius*other._OCData.Radius;
	uint numSolution=secondDegree (_Bx*_Bx+_By*_By, 2.f*(_Ax*_Bx+_Ay*_By), _Ax*_Ax+_Ay*_Ay-squareRadius, s0, s1);
	if (numSolution!=0)
	{
		// time
		double time;

		// Collision time
		if (numSolution==1)
		{
			firstContactTime=s0;
			lastContactTime=s0;
		}
		else
		{
			// First and last time
			if (s0<s1)
			{
				firstContactTime=s0;
				lastContactTime=s1;
			}
			else
			{
				firstContactTime=s1;
				lastContactTime=s0;
			}
		}
		time=firstContactTime;

		// Pos Z
		const double pointCylZ=other._3dInitPosition.z;
		const double cylPosZ= pointCylZ + other._Speed.z*time;

		// Some constants
		const double pointZ=_3dInitPosition.z;
		const double ptPosZ= pointZ + _Speed.z*time;

		// Z Included ?
		if ( (ptPosZ <= cylPosZ + other._Height) && (ptPosZ + _Height >= cylPosZ) )
		{
			// Ok Collision, fill the result
			
			// Time
			desc.ContactTime=time;

			// Point position
			const double ptPosX= _OBData.PointPosX[numPoint] + _Speed.x*time;
			const double ptPosY= _OBData.PointPosY[numPoint] + _Speed.y*time;

			// Cylinder position
			const double cylPosX= other._3dInitPosition.x + other._Speed.x*time;
			const double cylPosY= other._3dInitPosition.y + other._Speed.y*time;

			// Position
			desc.ContactPosition.x=ptPosX;
			desc.ContactPosition.y=ptPosY;
			desc.ContactPosition.z=std::max (cylPosZ, ptPosZ);

			// Cylinder normal
			desc.ContactNormal1.x=ptPosX-cylPosX;
			desc.ContactNormal1.y=ptPosY-cylPosY;
			desc.ContactNormal1.z=0;
			desc.ContactNormal1.normalize ();
			desc.ContactNormal0.x=-desc.ContactNormal1.x;
			desc.ContactNormal0.y=-desc.ContactNormal1.y;
			desc.ContactNormal0.z=0;

			// End
			return true;
		}
	}

	// No collision
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionSoverOC (CMovePrimitive& other, CCollisionDesc& desc, uint numSeg)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedCylinder);

	// Some constants
	const double normalSegX=_OBData.EdgeDirectionY[numSeg];
	const double normalSegY=-_OBData.EdgeDirectionX[numSeg];

	// Relative speed
	const double speedX=other._Speed.x-_Speed.x;
	const double speedY=other._Speed.y-_Speed.y;

	// Dot product with the plan tangeante
	double dotProd= speedX*normalSegX + speedY*normalSegY;
	//if ( dotProd < 0 )
	if ( dotProd !=0 )
	{
		// Time of the collision
		double time= (other._OCData.Radius + normalSegX*(_OBData.PointPosX[numSeg] - other._3dInitPosition.x ) + 
			normalSegY*(_OBData.PointPosY[numSeg] - other._3dInitPosition.y ) ) / dotProd;

		// Position of segment point at collision time
		const double segPosX= _OBData.PointPosX[numSeg] + _Speed.x*time;
		const double segPosY= _OBData.PointPosY[numSeg] + _Speed.y*time;

		// Position of the cylinder at collision time
		const double cylPosX= other._3dInitPosition.x + _Speed.x*time;
		const double cylPosY= other._3dInitPosition.y + _Speed.y*time;

		// Position de contact
		const double contactX= cylPosX - normalSegX*other._OCData.Radius;
		const double contactY= cylPosY - normalSegY*other._OCData.Radius;

		// Direction of the collision on the segment
		const double dirX= contactX - segPosX;
		const double dirY= contactY - segPosY;

		// Length of this vector
		const double length= dirY*normalSegX - dirX*normalSegY;

		// Included ?
		if ( ( length >= 0 ) && ( length <= _OBData.Length[numSeg&1] ) )
		{
			// 2d Collid checked... Now check height
			
			// Pos Z
			const double segPosZ= _3dInitPosition.z + _Speed.z*time;

			// Some constants
			const double cylPosZ= other._3dInitPosition.z + other._Speed.z*time;

			// Included ?
			if ( (cylPosZ <= segPosZ + _Height) && (cylPosZ + other._Height >= segPosZ) )
			{
				// Ok Collision, fill the result
				
				// Time
				desc.ContactTime=time;

				// Position
				desc.ContactPosition.x=contactX;
				desc.ContactPosition.y=contactY;
				desc.ContactPosition.z=std::max (segPosZ, cylPosZ);

				// Segment normal
				desc.ContactNormal0.x=normalSegX;
				desc.ContactNormal0.y=normalSegY;
				desc.ContactNormal0.z=0;

				// Seg box normal
				desc.ContactNormal1.x=contactX-cylPosX;
				desc.ContactNormal1.y=contactY-cylPosY;
				desc.ContactNormal1.z=0;
				desc.ContactNormal1.normalize ();

				// End
				return true;
			}
		}
	}

	// No collision
	return false;
}


// ***************************************************************************

bool CMovePrimitive::evalCollisionOCoverOC (CMovePrimitive& other, CCollisionDesc& desc, double timeMin, double timeMax, 
											double &firstContactTime, double &lastContactTime)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedCylinder);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedCylinder);

	/* Cylinder0 center equ:
	 * p(t) = p0 + v0*(t - t0)
	 *
	 * Cylinder1 center equ:
	 * p'(t) = p'0 + v'0*(t - t'0)
	 *
	 * Find t for this equation:
	 * (R + R')² = Norm² (p(t) - p'(t))
	 * (R + R')² = Norm² ( p0 + v0 ( t - t0 ) - p'0 - v'0 ( t - t'0 ) )
	 *
	 * A = p0 - v0*t0 - p'0 + v'0*t'0
	 * B = (v0 - v'0)
	 *
	 * Norm² (B)*t² + 2*(A.B)*t + Norm² (A) - (R + R')² = 0
	 *
	 * a = Norm² (B)
	 * b = 2*(A.B)
	 * c = Norm² (A) - (R + R')²
	 *
	 * a*t² + b*t + c = 0
	 */

	// Let's go
	const double _Ax = _3dInitPosition.x - other._3dInitPosition.x;
	const double _Ay = _3dInitPosition.y - other._3dInitPosition.y;
	const double _Bx = _Speed.x - other._Speed.x;
	const double _By = _Speed.y - other._Speed.y;

	// Eval system
	double s0, s1;
	double radiusSquare=_OCData.Radius+other._OCData.Radius;
	radiusSquare*=radiusSquare;
	uint numSolution=secondDegree (_Bx*_Bx+_By*_By, 2.f*(_Ax*_Bx+_Ay*_By), _Ax*_Ax+_Ay*_Ay-radiusSquare, s0, s1);
	if (numSolution!=0)
	{
		// time
		double _timeMin, _timeMax;

		// Collision time
		if (numSolution==1)
		{
			_timeMin=s0;
			_timeMax=s0;
		}
		else
		{
			// Time min and max
			if (s0>s1)
			{
				_timeMin=s1;
				_timeMax=s0;
			}
			else
			{
				_timeMin=s0;
				_timeMax=s1;
			}
		}

		// half time
		//const double halfTime=(_timeMin+_timeMax)/2.0;

		// Conatct time
		firstContactTime=_timeMin;
		lastContactTime=_timeMax;

		// Clip time
		if ((timeMin<_timeMax)&&(_timeMin<timeMax))
		{
			// Some constants
			const double cyl0Time= _timeMin;
			const double pointCyl0Z=_3dInitPosition.z;
			const double cyl0PosZ= pointCyl0Z + _Speed.z*cyl0Time;

			// Pos Z
			const double cyl1Time= _timeMin;
			const double pointCyl1Z=other._3dInitPosition.z;
			const double cyl1PosZ= pointCyl1Z + other._Speed.z * cyl1Time;

			// Z Included ?
			if ( (cyl0PosZ <= cyl1PosZ + other._Height) && (cyl0PosZ + _Height >= cyl1PosZ) )
			{
				// Ok Collision, fill the result
				
				// Time
				desc.ContactTime=std::max (_timeMin, timeMin);

				// Cylinder 0 position
				const double cyl0PosX= _3dInitPosition.x + _Speed.x*cyl0Time;
				const double cyl0PosY= _3dInitPosition.y + _Speed.y*cyl0Time;

				// Cylinder 1 position
				const double cyl1PosX= other._3dInitPosition.x + other._Speed.x*cyl1Time;
				const double cyl1PosY= other._3dInitPosition.y + other._Speed.y*cyl1Time;

				// First cylinder normal
				desc.ContactNormal0.x= cyl1PosX - cyl0PosX;
				desc.ContactNormal0.y= cyl1PosY - cyl0PosY;
				desc.ContactNormal0.z= 0;
				desc.ContactNormal0.normalize ();

				// Contact position
				desc.ContactPosition.x= desc.ContactNormal0.x*_OCData.Radius + cyl0PosX;
				desc.ContactPosition.y= desc.ContactNormal0.y*_OCData.Radius + cyl0PosY;
				desc.ContactPosition.z= std::max (cyl0PosZ, cyl1PosZ);

				// Second cylinder normal
				desc.ContactNormal1.x= -desc.ContactNormal0.x;
				desc.ContactNormal1.y= -desc.ContactNormal0.y;
				desc.ContactNormal1.z= 0;

				// End
				return true;
			}
		}
	}

	// No collision
	return false;
}

// ***************************************************************************

void CMovePrimitive::precalcPos ()
{
	// Type of the primitive
	uint type=((uint32)_Flags)&PrimitiveMask;

	// Box ?
	if (type==_2DOrientedBox)
	{
		// Calc cosinus and sinus
		double cosinus=(double)cos(_OBData.Orientation);
		double sinus=(double)sin(_OBData.Orientation);

		// Size
		double halfWidth=_OBData.Length[0]/2;
		double halfDepth=_OBData.Length[1]/2;

		// First point
		_OBData.PointPosX[0]=cosinus*(-halfWidth)-sinus*(-halfDepth)+_3dInitPosition.x;
		_OBData.PointPosY[0]=sinus*(-halfWidth)+cosinus*(-halfDepth)+_3dInitPosition.y;

		// Second point
		_OBData.PointPosX[1]=cosinus*halfWidth-sinus*(-halfDepth)+_3dInitPosition.x;
		_OBData.PointPosY[1]=sinus*halfWidth+cosinus*(-halfDepth)+_3dInitPosition.y;
		
		// Third point
		_OBData.PointPosX[2]=cosinus*halfWidth-sinus*halfDepth+_3dInitPosition.x;
		_OBData.PointPosY[2]=sinus*halfWidth+cosinus*halfDepth+_3dInitPosition.y;
		
		// Fourth point
		_OBData.PointPosX[3]=cosinus*(-halfWidth)-sinus*halfDepth+_3dInitPosition.x;
		_OBData.PointPosY[3]=sinus*(-halfWidth)+cosinus*halfDepth+_3dInitPosition.y;

		// Direction
		double oneOverLength[2]= { 1 / _OBData.Length[0], 1 / _OBData.Length[1] };

		// Direction
		uint i;
		for (i=0; i<4; i++)
		{
			// Next index
			uint next=(i+1)&3;
			double oneOver=oneOverLength[i&1];

			// New direction
			_OBData.EdgeDirectionX[i]=(_OBData.PointPosX[next] - _OBData.PointPosX[i])*oneOver;
			_OBData.EdgeDirectionY[i]=(_OBData.PointPosY[next] - _OBData.PointPosY[i])*oneOver;
		}
	}
	else
	{
		// Should be a cylinder
		nlassert (type==_2DOrientedCylinder);
	}
}

// ***************************************************************************

void CMovePrimitive::precalcBB (double beginTime, double endTime)
{
	// Type of the primitive
	uint type=((uint32)_Flags)&PrimitiveMask;

	// Box ?
	if (type==_2DOrientedBox)
	{
		// Point index
		const static minX[4]={ 3, 2, 1, 0 };
		const static minY[4]={ 0, 3, 2, 1 };
		const static maxX[4]={ 1, 0, 3, 2 };
		const static maxY[4]={ 2, 1, 0, 3 };

		// Orientation index
		sint orient= (sint)(256.f*_OBData.Orientation/(2.f*NLMISC::Pi));
		orient&=0xff;
		orient>>=6;
		nlassert (orient>=0);
		nlassert (orient<4);

		// Compute coordinates
		_BBXMin=FLT_MAX;
		_BBYMin=FLT_MAX;
		_BBXMax=-FLT_MAX;
		_BBYMax=-FLT_MAX;

		for (uint i=0; i<4; i++)
		{
			if (_OBData.PointPosX[i]<_BBXMin)
				_BBXMin=_OBData.PointPosX[i];
			if (_OBData.PointPosX[i]>_BBXMax)
				_BBXMax=_OBData.PointPosX[i];
			if (_OBData.PointPosY[i]<_BBYMin)
				_BBYMin=_OBData.PointPosY[i];
			if (_OBData.PointPosY[i]>_BBYMax)
				_BBYMax=_OBData.PointPosY[i];
		}
		_BBXMin=std::min (std::min (_BBXMin, _BBXMin+endTime*_Speed.x), _BBXMin+beginTime*_Speed.x);
		_BBXMax=std::max (std::max (_BBXMax, _BBXMax+endTime*_Speed.x), _BBXMax+beginTime*_Speed.x);
		_BBYMin=std::min (std::min (_BBYMin, _BBYMin+endTime*_Speed.y), _BBYMin+beginTime*_Speed.y);
		_BBYMax=std::max (std::max (_BBYMax, _BBYMax+endTime*_Speed.y), _BBYMax+beginTime*_Speed.y);

/*		
		// This code is faster but buggy.. 
		_BBXMin= _OBData.PointPosX[minX[orient]] + _Speed.x*beginTime;
		_BBXMin= std::min (_BBXMin, _OBData.PointPosX[minX[orient]] + _Speed.x*endTime);

		_BBYMin= _OBData.PointPosY[minY[orient]] + _Speed.y*beginTime;
		_BBYMin= std::min (_BBYMin, _OBData.PointPosY[minY[orient]] + _Speed.y*endTime);

		_BBXMax= _OBData.PointPosX[maxX[orient]] + _Speed.x*beginTime;
		_BBXMax= std::max (_BBXMax, _OBData.PointPosX[maxX[orient]] + _Speed.x*endTime);

		_BBYMax= _OBData.PointPosY[maxY[orient]] + _Speed.y*beginTime;
		_BBYMax= std::max (_BBYMax, _OBData.PointPosY[maxY[orient]] + _Speed.y*endTime);*/
	}
	else
	{
		// Should be a cylinder
		nlassert (type==_2DOrientedCylinder);

		// Compute X coordinates
		_BBXMin= _3dInitPosition.x + _Speed.x*beginTime;
		_BBXMax= _3dInitPosition.x + _Speed.x*endTime;
		if (_BBXMin>_BBXMax)
		{
			double tmp=_BBXMin;
			_BBXMin=_BBXMax;
			_BBXMax=tmp;
		}
		_BBXMin-=_OCData.Radius;
		_BBXMax+=_OCData.Radius;

		// Compute Y coordinates
		_BBYMin= _3dInitPosition.y + _Speed.y*beginTime;
		_BBYMax= _3dInitPosition.y + _Speed.y*endTime;
		if (_BBYMin>_BBYMax)
		{
			double tmp=_BBYMin;
			_BBYMin=_BBYMax;
			_BBYMax=tmp;
		}
		_BBYMin-=_OCData.Radius;
		_BBYMax+=_OCData.Radius;
	}
}

// ***************************************************************************

void CMovePrimitive::addMoveElement (CMoveCell& cell, uint16 x, uint16 y, double centerX, double centerY)
{
	// Find a free place
	uint slot;
	for (slot=0; slot<4; slot++)
	{
		// Empty ?
		if (_MoveElement[slot]==NULL)
		{
			// Primitive center
			double cx=(_BBXMin+_BBXMax)/2.f;
			double cy=(_BBYMin+_BBYMax)/2.f;

			// Allocate move element
			_MoveElement[slot]=_Container->allocateMoveElement ();
			_MoveElement[slot]->Primitive=this;
			_MoveElement[slot]->X=x;
			_MoveElement[slot]->Y=y;

			// Insert in left or right ?
			if (cx<centerX)
				// In the left
				cell.linkFirstX (_MoveElement[slot]);
			else
				// In the right
				cell.linkLastX (_MoveElement[slot]);

			// Insert in left or right ?
			if (cy<centerY)
				// In the left
				cell.linkFirstY (_MoveElement[slot]);
			else
				// In the right
				cell.linkLastY (_MoveElement[slot]);

			// Move it
#ifndef TEST_CELL
			 cell.updateSortedLists (_MoveElement[slot]);
#endif // TEST_CELL

			// End
			break;
		}
	}
}

// ***************************************************************************

void CMovePrimitive::removeMoveElement (uint i)
{
	// Check
	nlassert ((i>=0)||(i<4));
	nlassert (_MoveElement[i]!=NULL);

	// Unlink the element
	_Container->unlinkMoveElement (_MoveElement[i]);

	// Free the move element
	_Container->freeMoveElement (_MoveElement[i]);

	// Set to NULL
	_MoveElement[i]=NULL;
}

// ***************************************************************************

void CMovePrimitive::removeCollisionOTInfo (CCollisionOTInfo *toRemove)
{
	// Should be ok
	CCollisionOTInfo	*previousElement=NULL;
	CCollisionOTInfo	*element=_RootOTInfo;
	nlassert (element);

	// Look for it
	while (element)
	{
		// Good one ?
		if (element==toRemove)
		{
			// If previous element, just link
			if (previousElement)
				previousElement->primitiveLink (this, element->getNext (this));
			else
				_RootOTInfo=element->getNext (this);

			// End
			break;
		}
		
		// Look for next
		previousElement=element;
		element=element->getNext (this);
	}

	// Should be found
	nlassert (element);
}

// ***************************************************************************

void CMovePrimitive::removeCollisionOTInfo ()
{
	// For each element in the list
	CCollisionOTInfo	*element=_RootOTInfo;
	while (element)
	{
		// Unlink from ot
		element->unlink ();

		// Remove collision ot info from other primitive
		CMovePrimitive *other=element->getOtherPrimitive (this);
		if (other)
		{
			// Remove it in the other element
			other->removeCollisionOTInfo (element);
		}

		// Next element
		element=element->getNext (this);
	}

	// Relink element because we keep it
	_RootOTInfo=NULL;
}

// ***************************************************************************

void CMovePrimitive::checkSortedList ()
{
	// For the 4 elements
	for (uint i=0; i<4; i++)
	{
		// element here ?
		if (_MoveElement[i])
		{
			if (_MoveElement[i]->PreviousX)
				nlassertonce (_MoveElement[i]->PreviousX->Primitive->_BBXMin <= _BBXMin);
			if (_MoveElement[i]->NextX)
				nlassertonce (_BBXMin <= _MoveElement[i]->NextX->Primitive->_BBXMin);
		}
	}
}

// ***************************************************************************

bool CMovePrimitive::isTriggered (CMovePrimitive& second, bool enter, bool exit)
{
	// Generate a trigger ?

	// Is the two are not triggers ?
	if ( ( (_Flags&TriggerMask) == NotATrigger ) && ( (second._Flags&TriggerMask) == NotATrigger ) )
		return false;

	// Is one of them is an enter trigger ?
	if ( enter && ( (_Flags&EnterTrigger) || (second._Flags&EnterTrigger) ) )
		return true;

	// Is one of them is an exit trigger ?
	if ( exit && ( (_Flags&ExitTrigger) || (second._Flags&ExitTrigger) ) )
		return true;

	// Is one of them is a trigger ?
	if ( (_Flags&OverlapTrigger) || (second._Flags&OverlapTrigger) )
		return true;

	return false;
}

// ***************************************************************************

void CMovePrimitive::reaction (CMovePrimitive& second, const CCollisionDesc& desc, CGlobalRetriever* retriever,
							   CCollisionSurfaceTemp& surfaceTemp, bool collision)
{
	// TODO: reaction for no collision must be made on the full deltaTime not only to CollisionTime

	// Get the two reaction codes
	TReaction firstReaction=(TReaction)(_Flags&ReactionMask);
	TReaction secondReaction=(TReaction)(second._Flags&ReactionMask);

	// Overide collsion 
	collision = collision && (isObstacle ()) && (second.isObstacle ());

	// Get the two mass
	float mass0 = getMass ();
	float mass1 = second.getMass ();

	// Energy sum
	double projSpeed0 = desc.ContactNormal1 * _Speed;
	double projSpeed1 = desc.ContactNormal0 * second._Speed;
	double energySum = (- mass0 * projSpeed0 - mass1 * projSpeed1 ) / 2.0;

	// Old position
	CVectorD collisionPosition=_3dInitPosition;
	collisionPosition+=_Speed*desc.ContactTime;

	// Calc new speed
	CVectorD newSpeed;
	
	// Obstacle ?
	if ( (!collision) || (firstReaction==DoNothing) )
		newSpeed=_Speed;
	else
	{
		switch (firstReaction)
		{
		case Slide:
			// Remove projected speed
			newSpeed=_Speed - projSpeed0 * desc.ContactNormal1;

			// Reflexion speed
			newSpeed+=( _Attenuation*energySum / mass0 ) * desc.ContactNormal1;
			break;
		case Reflexion:
			// Remove projected speed
			newSpeed=_Speed - projSpeed0 * desc.ContactNormal1;

			// Reflexion speed
			newSpeed+=( _Attenuation*energySum / mass0 ) * desc.ContactNormal1;
			break;
		case Stop:
			newSpeed.set (0,0,0);
			break;
		}
	}

	// Set new speed
	setSpeed (newSpeed);

	// New position at t=0
	if (retriever)
	{
		// Make a domove in the Ben data
		CGlobalRetriever::CGlobalPosition newPosition = retriever->doMove (_Position.getGlobalPos (), 
			collisionPosition-_Position.getPos (), 1, surfaceTemp, true);

		// Set the new position
		_Position.setGlobalPos (newPosition, *retriever);

		// Position at t=0
		_3dInitPosition = _Position.getPos() - newSpeed * desc.ContactTime;

		// New init time
		_InitTime = desc.ContactTime;
	}
	else
	{
		// No retriever used
		_Position.setPos (collisionPosition);

		// Position at t=0
		_3dInitPosition = collisionPosition - newSpeed * desc.ContactTime;

		// New init time
		_InitTime = desc.ContactTime;
	}

	// Dirt pos
	dirtPos ();

	// ****** Second object

	// Old position
	collisionPosition=second._3dInitPosition;
	collisionPosition+=second._Speed * desc.ContactTime;
	
	// Obstacle ?
	if ( (!collision) || (secondReaction==DoNothing) )
		newSpeed=second._Speed;
	else
	{
		switch (secondReaction)
		{
		case Slide:
			// Remove projected speed
			newSpeed=second._Speed - projSpeed1 * desc.ContactNormal0;

			// Reflexion speed
			newSpeed+=( second._Attenuation*energySum / mass1 ) * desc.ContactNormal1;
			break;
		case Reflexion:
			// Remove projected speed
			newSpeed=second._Speed - projSpeed1 * desc.ContactNormal0;

			// Reflexion speed
			newSpeed+=( second._Attenuation*energySum / mass1 ) * desc.ContactNormal0;
			break;
		case Stop:
			newSpeed.set (0,0,0);
			break;
		}
	}

	// Set new speed
	second.setSpeed (newSpeed);

	// New position at t=0
	if (retriever)
	{
		// Make a domove in the Ben data
		CGlobalRetriever::CGlobalPosition newPosition = retriever->doMove (second._Position.getGlobalPos (), 
			collisionPosition-second._Position.getPos (), 1, surfaceTemp, true);

		// Set the new position
		second._Position.setGlobalPos (newPosition, *retriever);

		// Position at t=0
		second._3dInitPosition = second._Position.getPos() - newSpeed * desc.ContactTime;

		// New init time
		second._InitTime = desc.ContactTime;
	}
	else
	{
		// No retriever used
		second._Position.setPos (collisionPosition);

		// Position at t=0
		second._3dInitPosition = collisionPosition - newSpeed * desc.ContactTime;

		// New init time
		second._InitTime = desc.ContactTime;
	}

	// Dirt pos
	second.dirtPos ();
}

// ***************************************************************************

void CMovePrimitive::reaction (const CCollisionSurfaceDesc&	surfaceDesc, const CGlobalRetriever::CGlobalPosition& globalPosition,
							   const CGlobalRetriever& retriever, double deltaTime)
{
	// Reaction type
	uint32 type=_Flags&ReactionMask;

	// Reaction to the collision: copy the CGlobalRetriever::CGlobalPosition
	_Position.setGlobalPos (globalPosition, retriever);
	
	// Relfexion or slide ?
	if ((type==Reflexion)||(type==Slide))
	{
		// Cut last delta from distance to the wall.
		_Speed*=(1.0-deltaTime);

		// Slide ?
		if (type==Slide)
		{
			// Project last delta on plane of collision.
			_Speed-= surfaceDesc.ContactNormal*(surfaceDesc.ContactNormal*_Speed-NELPACS_DIST_BACK);
		}

		// Reflexion ?
		if (type==Reflexion)
		{
			// Project last delta on plane of collision.
			double speedProj=surfaceDesc.ContactNormal*_Speed;
			_Speed-=surfaceDesc.ContactNormal*(speedProj+speedProj*_Attenuation-NELPACS_DIST_BACK);
		}
	}
	else
	{
		// Stop ?
		if (type==Stop)
		{
			_Speed.set (0,0,0);
		}
	}
}

// ***************************************************************************

void CMovePrimitive::setGlobalPosition (const NLMISC::CVectorD& pos, const UMoveContainer& container)
{
	// Cast type
	nlassert (dynamic_cast<const CMoveContainer*>(&container));
	const CMoveContainer *cont=(const CMoveContainer*)&container;

	// Get the retriever
	CGlobalRetriever *retriever=cont->getGlobalRetriever();

	// Use a global retriever
	if (retriever)
	{
		// Get a cvector
		CVector vect=pos;

		// Get global position
		CGlobalRetriever::CGlobalPosition globalPosition=retriever->retrievePosition (vect);

		// Set global position
		_Position.setGlobalPos (globalPosition, *retriever);
	}
	else
	{
		// Set the position
		_Position.setPos (pos);
	}

	// Precalc some values
	_3dInitPosition = _Position.getPos ();
	_InitTime = 0;

	// Speed NULL
	_Speed=CVector::Null;

	// Dirt BB
	dirtPos ();
}

// ***************************************************************************

void CMovePrimitive::move (const NLMISC::CVectorD& speed)
{
	// New speed
	setSpeed (speed);

	// Set initial position
	_3dInitPosition = _Position.getPos ();

	// Set initial time
	_InitTime = 0;

	// Dirt BB
	dirtPos ();
}

// ***************************************************************************

} // NLPACS
