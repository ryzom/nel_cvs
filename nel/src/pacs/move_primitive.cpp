/** \file move_primitive.cpp
 * <File description>
 *
 * $Id: move_primitive.cpp,v 1.2 2001/05/04 15:20:33 corvazier Exp $
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


namespace NLPACS 
{

// ***************************************************************************

#define NL_EPSILON (0.0001f)

// ***************************************************************************

bool CMovePrimitive::evalCollision (CMovePrimitive& other, CCollisionDesc& desc)
{
	// Update precalc
	update ();

	// Switch the good test
	switch (((uint32)_Flags)&PrimitiveMask)
	{

	// Static box over...
	case Static2DOrientedBox:
		{
			// Switch second type
			switch (((uint32)other._Flags)&PrimitiveMask)
			{
			
			// Static box over movable box
			case Movable2DOrientedBox:
				// Make the test
				return evalCollisionSOBoverMOB (other, desc);

			// Static box over movable cylinder
			case Movable2DOrientedCylinder:
				// Make the test
				return evalCollisionSOBoverMOC (other, desc);

			// Static box over static object
			case Static2DOrientedBox:
			case Static2DOrientedCylinder:
				// Nothing to do.
				return false;
			}
		}
	
	// Movable box over...
	case Movable2DOrientedBox:
		{
			// Switch second type
			switch (((uint32)other._Flags)&PrimitiveMask)
			{
			
			// Movable box over static box
			case Static2DOrientedBox:
				// Make the test
				return other.evalCollisionSOBoverMOB (*this, desc);

			// Movable box over movable box
			case Movable2DOrientedBox:
				// Make the test
				return evalCollisionMOBoverMOB (other, desc);

			// Movable box over static cylinder
			case Static2DOrientedCylinder:
				// Make the test
				return evalCollisionMOBoverSOC (other, desc);

			// Movable box over movable cylinder
			case Movable2DOrientedCylinder:
				// Make the test
				return evalCollisionMOBoverMOC (other, desc);
			}
		}

	// Static cylinder over...
	case Static2DOrientedCylinder:
		{
			// Switch second type
			switch (((uint32)other._Flags)&PrimitiveMask)
			{
			
			// Static cylinder over movable box
			case Movable2DOrientedBox:
				// Make the test
				return evalCollisionMOBoverSOC (other, desc);

			// Static cylinder over movable cylinder
			case Movable2DOrientedCylinder:
				// Make the test
				return evalCollisionSOCoverMOC (other, desc);

			// Static cylinder over static object
			case Static2DOrientedBox:
			case Static2DOrientedCylinder:
				// Nothing to do.
				return false;
			}
		}
	
	// Movable cylinder over...
	case Movable2DOrientedCylinder:
		{
			// Switch second type
			switch (((uint32)other._Flags)&PrimitiveMask)
			{
			
			// Movable cylinder over static box
			case Static2DOrientedBox:
				// Make the test
				return other.evalCollisionSOBoverMOC (*this, desc);

			// Movable cylinder over movable box
			case Movable2DOrientedBox:
				// Make the test
				return other.evalCollisionMOBoverMOC (*this, desc);

			// Movable cylinder over static cylinder
			case Static2DOrientedCylinder:
				// Make the test
				return other.evalCollisionSOCoverMOC (*this, desc);

			// Movable cylinder over movable cylinder
			case Movable2DOrientedCylinder:
				// Make the test
				return evalCollisionMOCoverMOC (other, desc);
			}
		}
	}

	// Should not go here
	nlstop;

	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionSOBoverMOB (CMovePrimitive& other, CCollisionDesc& desc)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Static2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Movable2DOrientedBox);

	// Find a collision
	bool find=false;

	// Best time
	CCollisionDesc best;
	best.ContactTime=FLT_MAX;

	// Check movable points over the static box
	uint pt;
	uint seg;
	for (pt=0; pt<4; pt++)
	for (seg=0; pt<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc desc;
		if (evalCollisionSSoverMP (other, desc, seg, pt))
		{
			// Best time ?
			if (desc.ContactTime<best.ContactTime)
				best=desc;

			// Find
			find=true;
		}
	}

	// Check static points over the movable box
	for (pt=0; pt<4; pt++)
	for (seg=0; pt<4; seg++)
	{
		// Get collision time of the point over the segment
		CCollisionDesc desc;
		if (other.evalCollisionMSoverSP (*this, desc, seg, pt))
		{
			// Best time ?
			if (desc.ContactTime<best.ContactTime)
				best=desc;

			// Find
			find=true;
		}
	}

	// Return if we have found a collision
	return find;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionSOBoverMOC (CMovePrimitive& other, CCollisionDesc& desc)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Static2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Movable2DOrientedCylinder);

	// Not implemented
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionMOBoverMOB (CMovePrimitive& other, CCollisionDesc& desc)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Movable2DOrientedBox);

	// Not implemented
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionMOBoverSOC (CMovePrimitive& other, CCollisionDesc& desc)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Static2DOrientedCylinder);

	// Not implemented
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionMOBoverMOC (CMovePrimitive& other, CCollisionDesc& desc)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Movable2DOrientedCylinder);

	// Not implemented
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionSOCoverMOC (CMovePrimitive& other, CCollisionDesc& desc)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Static2DOrientedCylinder);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Movable2DOrientedCylinder);

	// Not implemented
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionMOCoverMOC (CMovePrimitive& other, CCollisionDesc& desc)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedCylinder);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Movable2DOrientedCylinder);

	// Not implemented
	return false;
}

// ***************************************************************************

bool CMovePrimitive::evalCollisionSSoverMP (CMovePrimitive& other, CCollisionDesc& desc, uint numSeg, uint numPoint)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Static2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Movable2DOrientedBox);

	// Some constants
	const float edgeX=_OBData.EdgeDirectionX[numSeg];
	const float edgeY=_OBData.EdgeDirectionY[numSeg];
	const float speedX=other._MData.SpeedX;
	const float speedY=other._MData.SpeedY;

	// 2d Normal and speed in opposit direction ?
	float dotProd= speedX*edgeY - speedY*edgeX;
	if ( dotProd < 0 )
	{
		// Some constants
		const float pointX=other._OBData.PointPosX[numPoint];
		const float pointY=other._OBData.PointPosY[numPoint];

		// Time of the collision
		const float otherTime= (- _OBData.PlaneConst[numSeg] - edgeY*pointX + edgeX*pointY) / (dotProd);

		// Position of the collision
		const float posX= pointX + speedX*otherTime;
		const float posY= pointY + speedY*otherTime;

		// Direction of the collision on the segment
		const float dirX=posX-_OBData.PointPosX[numSeg];
		const float dirY=posY-_OBData.PointPosY[numSeg];

		// Length of this vector
		const float length=dirX*edgeX + dirY*edgeY;

		// Included ?
		if ( ( length >= 0 ) && ( length <= _OBData.Length[numSeg&1] ) )
		{
			// 2d Collid checked... Now check height
			
			// Pos Z
			const float posZ=other._Position.z + other._MData.SpeedZ*otherTime;

			// Included ?
			if ( (posZ <= _Position.z + _Height) && (posZ + other._Height >= _Position.z) )
			{
				// Ok Collision, fill the result
				
				// Time
				desc.ContactTime= otherTime + other._MData.Time;

				// Position
				desc.ContactPosition.x=posX;
				desc.ContactPosition.y=posY;
				desc.ContactPosition.z=std::max (posZ, other._Position.z);

				// Normal
				desc.ContactNormal.x=edgeY;
				desc.ContactNormal.y=-edgeX;
				desc.ContactNormal.z=0;

				// End
				return true;
			}
		}
	}

	// No collisions
	return false;
}

// ***************************************************************************

/*bool CMovePrimitive::evalCollisionMSoverSP (CMovePrimitive& other, CCollisionDesc& desc, uint numSeg, uint numPoint)
{
	// Checks
	nlassert ((((uint32)_Flags)&PrimitiveMask)==Movable2DOrientedBox);
	nlassert ((((uint32)other._Flags)&PrimitiveMask)==Static2DOrientedBox);

	// Some constants
	const float edgeX=_OBData.EdgeDirectionX[numSeg];
	const float edgeY=_OBData.EdgeDirectionY[numSeg];

	// 2d Normal and speed in same direction ?
	float oneOverSpeedPlaneConst=_MOBData.OneOverSpeedPlaneConst[numSeg];
	if ( oneOverSpeedPlaneConst > 0 )
	{
		// Some constants
		const float pointX=other._OBData.PointPosX[numPoint];
		const float pointY=other._OBData.PointPosY[numPoint];
		const float constPlane=_OBData.PlaneConst[numSeg];

		// Time of the collision
		float thisTime= (- edgeY*pointX + edgeX*pointY - constPlane ) * oneOverSpeedPlaneConst;

		// Position of segment point at collision time
		const float posX= _OBData.PointPosX[numSeg] + _MData.SpeedX[numSeg]*thisTime;
		const float posY= _OBData.PointPosY[numSeg] + _MData.SpeedY[numSeg]*thisTime;

		// Direction of the collision on the segment
		const float dirX= pointX - posX;
		const float dirY= pointY - posY;

		// Length of this vector
		const float length= dirX*edgeX + dirY*edgeY;

		// Included ?
		if ( ( length >= 0 ) && ( length <= _OBData.Length[numSeg&1] ) )
		{
			// 2d Collid checked... Now check height
			
			// Pos Z
			const float posZ=_Position.z + _MData.SpeedZ[numSeg]*thisTime;

			// Some constants
			const float pointZ=other._Position.z;

			// Included ?
			if ( (pointZ <= posZ + _Height) && (pointZ + other._Height >= posZ) )
			{
				// Ok Collision, fill the result
				
				// Time
				desc.ContactTime=thisTime + _MData.Time;

				// Position
				desc.ContactPosition.x=pointX;
				desc.ContactPosition.y=pointY;
				desc.ContactPosition.z=std::max (pointZ, posZ);

				// Normal
				desc.ContactNormal.x=edgeY;
				desc.ContactNormal.y=-edgeX;
				desc.ContactNormal.z=0;

				// End
				return true;
			}
		}
	}

	// Not implemented
	return false;
}*/

// ***************************************************************************

/*void CMovePrimitive::precalcPos ()
{
	// Type of the primitive
	uint type=((uint32)_Flags)&PrimitiveMask;

	// Box ?
	if ((type==Static2DOrientedBox)||(type==Movable2DOrientedBox))
	{
		// Calc cosinus and sinus
		float cosinus=(float)cos(Orientation);
		float sinus=(float)sin(Orientation);

		// Size
		float halfWidth=Length[0]/2;
		float halfDepth=Length[1]/2;

		// First point
		PointPosX[0]=cosinus*(-halfWidth)-sinus*(-halfDepth)+_Position.x;
		PointPosY[0]=sinus*(-halfWidth)+cosinus*(-halfDepth)+_Position.Y;

		// Second point
		PointPosX[1]=cosinus*halfWidth-sinus*(-halfDepth)+_Position.x;
		PointPosY[1]=sinus*halfWidth+cosinus*(-halfDepth)+_Position.y;
		
		// Third point
		PointPosX[2]=cosinus*halfWidth-sinus*halfDepth+_Position.x;
		PointPosY[2]=sinus*halfWidth+cosinus*halfDepth+_Position.y;
		
		// Fourth point
		PointPosX[3]=cosinus*(-halfWidth)-sinus*halfDepth+_Position.x;
		PointPosY[3]=sinus*(-halfWidth)+cosinus*halfDepth+_Position.y;

		// Direction
		float oneOverLength[2]= { 1/Length[0], 1/Length[1] };

		// Direction
		uint i;
		for (i=0; i<4; i++)
		{
			// Next index
			uint next=(i+1)&3;
			float oneOver=oneOverLength[i&1];

			// New direction
			EdgeDirectionX[i]=(PointPosX[next]-PointPosX[i])*oneOver;
			EdgeDirectionY[i]=(PointPosY[next]-PointPosY[i])*oneOver;
		}

		// Plane constant
		for (i=0; i<4; i++)
		{
			// Next index
			uint next=(i+1)&3;
			float oneOver=oneOverLength[i&1];

			// New plane constant
			EdgeDirectionX[i]=(PointPosX[next]-PointPosX[i])*oneOver;
			EdgeDirectionY[i]=(PointPosY[next]-PointPosY[i])*oneOver;
		}
	}
}*/

// ***************************************************************************

void CMovePrimitive::precalcSpeed ()
{
	
}

// ***************************************************************************

} // NLPACS
