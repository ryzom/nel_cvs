/** \file move_primitive.cpp
 * Description of movables primitives
 *
 * $Id: move_primitive.cpp,v 1.4 2001/05/22 08:47:21 berenguier Exp $
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

#include "nel/pacs/move_primitive.h"
#include "nel/pacs/collision_desc.h"
#include "nel/pacs/move_element.h"


namespace NLPACS 
{

// ***************************************************************************

CMovePrimitive::CMovePrimitive ()
{
	_Flags=0;
	_BBXMin=-FLT_MAX;
	_BBXMax=-FLT_MAX;
	_BBYMin=-FLT_MAX;
	_BBYMax=-FLT_MAX;
	_RootOTInfo=NULL;
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

bool CMovePrimitive::evalCollision (CMovePrimitive& other, CCollisionDesc& desc, float timeMin, float timeMax)
{
	// Bounding box test
	if (evalCollisionBB (other))
	{
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
					return evalCollisionOBoverOB (other, desc, timeMin, timeMax);

				// Static box over movable cylinder
				case _2DOrientedCylinder:
					// Make the test
					return evalCollisionOBoverOC (other, desc, timeMin, timeMax);

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
						bool collid=other.evalCollisionOBoverOC (*this, desc, timeMin, timeMax);
						if (collid)
							desc.XChgContactNormals ();
						return collid;
					}

				// Static box over movable cylinder
				case _2DOrientedCylinder:
					// Make the test
					return evalCollisionOCoverOC (other, desc, timeMin, timeMax);

				default:
				// Should not go here
				nlstop;
				}
			}

		default:
			// Should not go here
			nlstop;
		}	
	}

	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionOBoverOB (CMovePrimitive& other, CCollisionDesc& desc, float timeMin, float timeMax)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedBox);

	// Find a collision
	bool find=false;

	// Best time
	desc.ContactTime=FLT_MAX;

	// Timemin
	float _timeMin=-FLT_MAX;

	// Check movable points over the edge
	uint pt;
	uint seg;
	for (pt=0; pt<4; pt++)
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if (evalCollisionPoverS (other, d, pt, seg, _timeMin, timeMax))
		{
			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;

				// New time min
				_timeMin=desc.ContactTime;

				// Find
				find=true;
			}
		}
	}

	// Check static points over the movable box
	for (pt=0; pt<4; pt++)
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if (other.evalCollisionPoverS (*this, d, pt, seg, _timeMin, timeMax))
		{
			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;
				desc.XChgContactNormals ();

				// New time min
				_timeMin=desc.ContactTime;

				// Find
				find=true;
			}
		}
	}

	// Clip min
	if (_timeMin<timeMin)
		find=false;

	// Return if we have found a collision
	return find;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionOBoverOC (CMovePrimitive& other, CCollisionDesc& desc, float timeMin, float timeMax)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedCylinder);

	// Find a collision
	bool find=false;

	// Best time
	desc.ContactTime=FLT_MAX;

	// time min clip
	float _timeMin=-FLT_MAX;

	// Check movable points over the cylinder
	uint pt;
	for (pt=0; pt<4; pt++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if (evalCollisionPoverOC (other, d, pt, _timeMin, timeMax))
		{
			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;

				// New time min
				_timeMin=desc.ContactTime;

				// Found
				find=true;
			}
		}
	}

	// Check static points over the movable box
	uint seg;
	for (seg=0; seg<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc d;
		if (evalCollisionSoverOC (other, d, seg, _timeMin, timeMax))
		{
			// Best time ?
			if (d.ContactTime<desc.ContactTime)
			{
				// This is the new descriptor
				desc=d;

				// New time min
				_timeMin=desc.ContactTime;

				// Found
				find=true;
			}
		}
	}

	// Clip min
	if (_timeMin<timeMin)
		find=false;

	// Return if we have found a collision
	return find;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionPoverS (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, uint numSeg, float timeMin, float timeMax)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedBox);

	// Some constants
	const float normalSegX=other._OBData.EdgeDirectionY[numSeg];
	const float normalSegY=-other._OBData.EdgeDirectionX[numSeg];

	// Relative speed
	const float speedX=other._Speed.x-_Speed.x;
	const float speedY=other._Speed.y-_Speed.y;

	// Dot product with the plan tangeante
	float dotProd= speedX*normalSegX + speedY*normalSegY;
	if ( dotProd > 0 )
	{
		// Time of the collision
		float time= (normalSegX*(other._OBData.PointPosX[numPoint] - other._OBData.PointPosX[numSeg]) + 
			normalSegY*(other._OBData.PointPosY[numPoint] - other._OBData.PointPosY[numSeg])) / dotProd;

		// Clip time
		if ((timeMin<time)&&(time<timeMax))
		{
			// Position of segment point at collision time
			const float segPosX= other._OBData.PointPosX[numSeg] + other._Speed.x*time;
			const float segPosY= other._OBData.PointPosY[numSeg] + other._Speed.y*time;

			// Position of the point at collision time
			const float ptPosX= _OBData.PointPosX[numPoint] + _Speed.x*time;
			const float ptPosY= _OBData.PointPosY[numPoint] + _Speed.y*time;

			// Direction of the collision on the segment
			const float dirX= ptPosX - segPosX;
			const float dirY= ptPosY - segPosY;

			// Length of this vector
			const float length= dirY*normalSegX - dirX*normalSegY;

			// Included ?
			if ( ( length >= 0 ) && ( length <= other._OBData.Length[numSeg&1] ) )
			{
				// 2d Collid checked... Now check height
				
				// Pos Z
				const float pointSegZ=other._Position.z;
				const float segPosZ= pointSegZ + other._Speed.z*time;

				// Some constants
				const float pointZ=_Position.z;
				const float ptPosZ= pointZ + _Speed.z*time;

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

					// Point box normal
					uint previous=(numPoint+3)&3;
					desc.ContactNormal0.x=_OBData.EdgeDirectionY[numPoint] + _OBData.EdgeDirectionY[previous];
					desc.ContactNormal0.y=-_OBData.EdgeDirectionX[numPoint] - _OBData.EdgeDirectionX[previous];
					desc.ContactNormal0.z=0;
					desc.ContactNormal0.normalize ();

					// Seg box normal
					desc.ContactNormal1.x=normalSegX;
					desc.ContactNormal1.y=normalSegY;
					desc.ContactNormal1.z=0;

					// End
					return true;
				}
			}
		}
	}

	// No collision
	return false;
}

// ***************************************************************************

inline uint secondDegree (float a, float b, float c, float& s0, float& s1)
{
	float d=b*b-4.f*a*c;
	if (d>0)
	{
		// sqrt d
		d=(float)sqrt (d);

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

bool CMovePrimitive::evalCollisionPoverOC (CMovePrimitive& other, CCollisionDesc& desc, uint numPoint, float timeMin, float timeMax)
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
	const float _Ax = _OBData.PointPosX[numPoint] - other._Position.x;
	const float _Ay = _OBData.PointPosY[numPoint] - other._Position.y;
	const float _Bx = _Speed.x - other._Speed.x;
	const float _By = _Speed.y - other._Speed.y;

	// Eval system
	float s0, s1;
	uint numSolution=secondDegree (_Bx*_Bx+_By*_By, 2.f*(_Ax*_Bx+_Ay*_By), _Ax*_Ax+_Ay*_Ay-other._OCData.SquareRadius, s0, s1);
	if (numSolution!=0)
	{
		// time
		float time;

		// Collision time
		if (numSolution==1)
			time=s0;
		else
			time=std::min (s0, s1);

		// Clip time
		if ((timeMin<time)&&(time<timeMax))
		{
			// Pos Z
			const float pointCylZ=other._Position.z;
			const float cylPosZ= pointCylZ + other._Speed.z*time;

			// Some constants
			const float pointZ=_Position.z;
			const float ptPosZ= pointZ + _Speed.z*time;

			// Z Included ?
			if ( (ptPosZ <= cylPosZ + other._Height) && (ptPosZ + _Height >= cylPosZ) )
			{
				// Ok Collision, fill the result
				
				// Time
				desc.ContactTime=time;

				// Point position
				const float ptPosX= _OBData.PointPosX[numPoint] + _Speed.x*time;
				const float ptPosY= _OBData.PointPosY[numPoint] + _Speed.y*time;

				// Cylinder position
				const float cylPosX= other._Position.x + other._Speed.x*time;
				const float cylPosY= other._Position.y + other._Speed.y*time;

				// Position
				desc.ContactPosition.x=ptPosX;
				desc.ContactPosition.y=ptPosY;
				desc.ContactPosition.z=std::max (cylPosZ, ptPosZ);

				// Box normal
				uint previous=(numPoint+3)&3;
				desc.ContactNormal0.x=_OBData.EdgeDirectionY[numPoint] + _OBData.EdgeDirectionY[previous];
				desc.ContactNormal0.y=-_OBData.EdgeDirectionX[numPoint] - _OBData.EdgeDirectionX[previous];
				desc.ContactNormal0.z=0;
				desc.ContactNormal0.normalize ();

				// Cylinder normal
				desc.ContactNormal1.x=ptPosX-cylPosX;
				desc.ContactNormal1.y=ptPosY-cylPosY;
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

bool CMovePrimitive::evalCollisionSoverOC (CMovePrimitive& other, CCollisionDesc& desc, uint numSeg, float timeMin, float timeMax)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==_2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==_2DOrientedCylinder);

	// Some constants
	const float normalSegX=_OBData.EdgeDirectionY[numSeg];
	const float normalSegY=-_OBData.EdgeDirectionX[numSeg];

	// Relative speed
	const float speedX=other._Speed.x-_Speed.x;
	const float speedY=other._Speed.y-_Speed.y;

	// Dot product with the plan tangeante
	float dotProd= speedX*normalSegX + speedY*normalSegY;
	if ( dotProd < 0 )
	{
		// Time of the collision
		float time= (other._OCData.Radius + normalSegX*(_OBData.PointPosX[numSeg] - other._Position.x ) + 
			normalSegY*(_OBData.PointPosY[numSeg] - other._Position.y ) ) / dotProd;

		// Clip time
		if ((timeMin<time)&&(time<timeMax))
		{
			// Position of segment point at collision time
			const float segPosX= _OBData.PointPosX[numSeg] + _Speed.x*time;
			const float segPosY= _OBData.PointPosY[numSeg] + _Speed.y*time;

			// Position of the cylinder at collision time
			const float cylPosX= other._Position.x + _Speed.x*time;
			const float cylPosY= other._Position.y + _Speed.y*time;

			// Position de contact
			const float contactX= cylPosX - normalSegX*other._OCData.Radius;
			const float contactY= cylPosY - normalSegY*other._OCData.Radius;

			// Direction of the collision on the segment
			const float dirX= contactX - segPosX;
			const float dirY= contactY - segPosY;

			// Length of this vector
			const float length= dirY*normalSegX - dirX*normalSegY;

			// Included ?
			if ( ( length >= 0 ) && ( length <= _OBData.Length[numSeg&1] ) )
			{
				// 2d Collid checked... Now check height
				
				// Pos Z
				const float segPosZ= _Position.z + _Speed.z*time;

				// Some constants
				const float cylPosZ= other._Position.z + other._Speed.z*time;

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
	}

	// No collision
	return false;
}


// ***************************************************************************

bool CMovePrimitive::evalCollisionOCoverOC (CMovePrimitive& other, CCollisionDesc& desc, float timeMin, float timeMax)
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
	const float _Ax = _Position.x - other._Position.x;
	const float _Ay = _Position.y - other._Position.y;
	const float _Bx = _Speed.x - other._Speed.x;
	const float _By = _Speed.y - other._Speed.y;

	// Eval system
	float s0, s1;
	uint numSolution=secondDegree (_Bx*_Bx+_By*_By, 2.f*(_Ax*_Bx+_Ay*_By), _Ax*_Ax+_Ay*_Ay-other._OCData.SquareRadius, s0, s1);
	if (numSolution!=0)
	{
		// time
		float time;
		// Collision time
		if (numSolution==1)
			time=s0;
		else
			time=std::min (s0, s1);

		// Clip time
		if ((timeMin<time)&&(time<timeMax))
		{
			// Some constants
			const float cyl0Time= time;
			const float pointCyl0Z=_Position.z;
			const float cyl0PosZ= pointCyl0Z + _Speed.z*cyl0Time;

			// Pos Z
			const float cyl1Time= time;
			const float pointCyl1Z=other._Position.z;
			const float cyl1PosZ= pointCyl1Z + other._Speed.z * cyl1Time;

			// Z Included ?
			if ( (cyl0PosZ <= cyl1PosZ + other._Height) && (cyl0PosZ + _Height >= cyl1PosZ) )
			{
				// Ok Collision, fill the result
				
				// Time
				desc.ContactTime=time;

				// Cylinder 0 position
				const float cyl0PosX= _Position.x + _Speed.x*cyl0Time;
				const float cyl0PosY= _Position.y + _Speed.y*cyl0Time;

				// Cylinder 1 position
				const float cyl1PosX= other._Position.x + other._Speed.x*cyl1Time;
				const float cyl1PosY= other._Position.y + other._Speed.y*cyl1Time;

				// First cylinder normal
				desc.ContactNormal0.x= cyl1PosX - cyl0PosX;
				desc.ContactNormal0.y= cyl1PosY - cyl0PosY;
				desc.ContactNormal0.y= 0;
				desc.ContactPosition.normalize ();

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
		float cosinus=(float)cos(_OBData.Orientation);
		float sinus=(float)sin(_OBData.Orientation);

		// Size
		float halfWidth=_OBData.Length[0]/2;
		float halfDepth=_OBData.Length[1]/2;

		// First point
		_OBData.PointPosX[0]=cosinus*(-halfWidth)-sinus*(-halfDepth)+_Position.x;
		_OBData.PointPosY[0]=sinus*(-halfWidth)+cosinus*(-halfDepth)+_Position.y;

		// Second point
		_OBData.PointPosX[1]=cosinus*halfWidth-sinus*(-halfDepth)+_Position.x;
		_OBData.PointPosY[1]=sinus*halfWidth+cosinus*(-halfDepth)+_Position.y;
		
		// Third point
		_OBData.PointPosX[2]=cosinus*halfWidth-sinus*halfDepth+_Position.x;
		_OBData.PointPosY[2]=sinus*halfWidth+cosinus*halfDepth+_Position.y;
		
		// Fourth point
		_OBData.PointPosX[3]=cosinus*(-halfWidth)-sinus*halfDepth+_Position.x;
		_OBData.PointPosY[3]=sinus*(-halfWidth)+cosinus*halfDepth+_Position.y;

		// Direction
		float oneOverLength[2]= { 1 / _OBData.Length[0], 1 / _OBData.Length[1] };

		// Direction
		uint i;
		for (i=0; i<4; i++)
		{
			// Next index
			uint next=(i+1)&3;
			float oneOver=oneOverLength[i&1];

			// New direction
			_OBData.EdgeDirectionX[i]=(_OBData.PointPosX[next] - _OBData.PointPosX[i])*oneOver;
			_OBData.EdgeDirectionY[i]=(_OBData.PointPosY[next] - _OBData.PointPosY[i])*oneOver;
		}
	}
	else
	{
		// Should be a cylinder
		nlassert (type==_2DOrientedCylinder);

		// Square radius
		_OCData.SquareRadius= _OCData.Radius * _OCData.Radius;
	}
}

// ***************************************************************************

void CMovePrimitive::precalcBB (float beginTime, float endTime)
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
		nlassert (orient<2);

		// Compute coordinates
		_BBXMin= _OBData.PointPosX[minX[orient]] + _Speed.x*beginTime;
		_BBXMin= std::min (_BBXMin, _OBData.PointPosX[minX[orient]] + _Speed.x*endTime);

		_BBYMin= _OBData.PointPosY[minY[orient]] + _Speed.y*beginTime;
		_BBYMin= std::min (_BBYMin, _OBData.PointPosY[minY[orient]] + _Speed.y*endTime);

		_BBXMax= _OBData.PointPosX[maxX[orient]] + _Speed.x*beginTime;
		_BBXMax= std::max (_BBXMax, _OBData.PointPosX[maxX[orient]] + _Speed.x*endTime);

		_BBYMax= _OBData.PointPosY[maxY[orient]] + _Speed.y*beginTime;
		_BBYMax= std::max (_BBYMax, _OBData.PointPosY[maxY[orient]] + _Speed.y*endTime);
	}
	else
	{
		// Should be a cylinder
		nlassert (type==_2DOrientedCylinder);

		// Compute X coordinates
		_BBXMin= _Position.x + _Speed.x*beginTime;
		_BBXMax= _Position.x + _Speed.x*endTime;
		if (_BBXMin>_BBXMax)
		{
			float tmp=_BBXMin;
			_BBXMin=_BBXMax;
			_BBXMax=_BBXMin;
		}
		_BBXMin-=_OCData.Radius;
		_BBXMax+=_OCData.Radius;

		// Compute Y coordinates
		_BBYMin= _Position.y + _Speed.y*beginTime;
		_BBYMax= _Position.y + _Speed.y*endTime;
		if (_BBYMin>_BBYMax)
		{
			float tmp=_BBYMin;
			_BBYMin=_BBYMax;
			_BBYMax=_BBYMin;
		}
		_BBYMin-=_OCData.Radius;
		_BBYMax+=_OCData.Radius;
	}
}

// ***************************************************************************

void CMovePrimitive::addMoveElement (CMoveCell& cell, float centerX, float centerY)
{
	// Find a free place
	uint slot;
	for (slot=0; slot<4; slot++)
	{
		// Empty ?
		if (_MoveElement[slot]==NULL)
		{
			// Primitive center
			float cx=(_BBXMin+_BBXMax)/2.f;
			float cy=(_BBYMin+_BBYMax)/2.f;

			// Allocate move element
			_MoveElement[slot]=_Container->allocateMoveElement ();

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
			cell.updateSortedLists (_MoveElement[slot]);
		}
	}
}

// ***************************************************************************

void CMovePrimitive::removeMoveElement (uint i)
{
	// Check
	nlassert ((i>=0)||(i<4));
	nlassert (_MoveElement[i]!=NULL);

	// Free the move element
	_Container->freeMoveElement (_MoveElement[i]);

	// Set to NULL
	_MoveElement[i]=NULL;
}

// ***************************************************************************

void CMovePrimitive::removeCollisionOTInfo (float beginTime)
{
	// Protected
	if ((_Flags&(uint32)BeingRemovedFromTOTFlag) == 0)
	{
		// Put a flag to say we are removing the list
		_Flags|=(uint32)BeingRemovedFromTOTFlag;

		// For each element in the list
		CCollisionOTInfo	*element=_RootOTInfo;
		CCollisionOTInfo	*previousPointer=NULL;
		while (element)
		{
			// Contact time
			float contactTime=element->getCollisionDesc ().ContactTime;

			// Time to remove ?
			if (contactTime >= beginTime)
			{
				// Remove the other primitive
				if (element->getFirstPrimitive ()==this)
					// Remove in the others
					element->getSecondPrimitive ()->removeCollisionOTInfo ( std::max (contactTime, beginTime) );
				else
				{
					// Check
					nlassert (element->getSecondPrimitive ()==this);

					// Remove in the others
					element->getFirstPrimitive ()->removeCollisionOTInfo ( std::max (contactTime, beginTime) );
				}
			}
			else
			{
				// Relink element because we keep it
				if (previousPointer)
					previousPointer->primitiveLink (this, element);

				// New previous pointer
				previousPointer=element;
			}

			// Next element
			element=element->getNext (this);
		}

		// Clear the flag
		_Flags&=~(uint32)BeingRemovedFromTOTFlag;
	}
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
				nlassert (_MoveElement[i]->PreviousX->Primitive->_BBXMin <= _BBXMin);
			if (_MoveElement[i]->NextX)
				nlassert (_BBXMin <= _MoveElement[i]->NextX->Primitive->_BBXMin);
			if (_MoveElement[i]->PreviousY)
				nlassert (_MoveElement[i]->PreviousY->Primitive->_BBYMin <= _BBYMin);
			if (_MoveElement[i]->NextY)
				nlassert (_BBYMin <= _MoveElement[i]->NextY->Primitive->_BBYMin);
		}
	}
}

// ***************************************************************************

} // NLPACS
