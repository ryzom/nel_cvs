/** \file cubic_entity_interpolator.cpp
 * Cubic interpolation of entity
 *
 * $Id: cubic_entity_interpolator.cpp,v 1.3 2000/11/27 10:07:07 cado Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "nel/net/cubic_entity_interpolator.h"

using namespace NLMISC;

/* // tests
CVector P1(0,0,0);
CVector P2(0,0,0);
CVector P3(0,0,0);
CVector P4(0,0,0);
*/

namespace NLNET {


CMatrix	CCubicEntityInterpolator::_BezierMatrix;
bool	CCubicEntityInterpolator::_Initialized = false;

/// The 4x4 Bezier Matrix
const float BezierValues [16] =
{
	-1, 3, -3, 1, // column 1
	3, -6, 3, 0,  // column 2
	-3, 3, 0, 0,  // column 3
	1, 0, 0, 0    // column 4
};


/*
 * Constructor
 */
CCubicEntityInterpolator::CCubicEntityInterpolator() :
	IEntityInterpolator()
{
	if ( ! CCubicEntityInterpolator::_Initialized )
	{
		CCubicEntityInterpolator::_BezierMatrix.set( BezierValues );
		CCubicEntityInterpolator::_Initialized = true;
	}
}


// Tests
/*void setTV( CVector p1, CVector p2, CVector p3, CVector p4 )
{
	P1 = p1;
	P2 = p2; 
	P3 = p3;
	P4 = p4;
}*/


/*
 * Begins interpolation
 */
void CCubicEntityInterpolator::begin( const IMovingEntity& src, const IMovingEntity& dest, TDuration duration )
{
	_Dest = dest;
	_Duration = duration;
	_Elapsed = 0;
	_Active = true;

	_SrcHeading = src.bodyHeading();
	_DestHeading = dest.bodyHeading();

	// Precompute CubicMatrix
	CVector p1 = src.pos();
	CVector p2 = src.pos() + src.trajVector()/8.0f; // amplitude correction
	CVector p3 = dest.pos() - dest.trajVector()/8.0f; // amplitude correction
	CVector p4 = dest.pos();

	// Test
	//setTV( src.pos(), src.pos()+src.trajVector(), dest.pos(), dest.pos()-dest.trajVector() );

	CMatrix controlmatrix;
	controlmatrix.identity();
	controlmatrix.setRot( p1, p2, p3, true );
	controlmatrix.setPos( p4 );
	_CubicMatrix = controlmatrix * CCubicEntityInterpolator::_BezierMatrix;

}


/*
 * Returns the actual state; \e deltatime is the time between two updates.
 */
void CCubicEntityInterpolator::getNextState( IMovingEntity& es, TDuration deltatime )
{
	//es.setPos( _Dest.pos() );
	//_Active = false;
	_Elapsed += deltatime;
	float ratio = _Elapsed / _Duration;
	if ( ratio < 1.0 )
	{
		// Cubic interpolation of position
		CVectorH v( ratio*ratio*ratio, ratio*ratio, ratio, 1 );
		v = _CubicMatrix * v;
		es.setPos( v );
		
		// Linear interpolation of body heading
		es.setBodyHeading( _SrcHeading + (_DestHeading-_SrcHeading) * ratio );
	}
	else
	{
		es = _Dest;
		_Active = false;
	}

}




} // NLNET
