/** \file edge_collide.h
 * Collisions against edge in 2D.
 *
 * $Id: edge_collide.h,v 1.2 2001/05/16 15:17:12 berenguier Exp $
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

#ifndef NL_EDGE_COLLIDE_H
#define NL_EDGE_COLLIDE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2f.h"


namespace NLPACS 
{


using	NLMISC::CVector2f;


// ***************************************************************************
/**
 * Collisions against edge in 2D.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CEdgeCollide
{
public:
	CVector2f		P0;
	CVector2f		P1;
	CVector2f		Dir, Norm;
	float			A0, A1;
	float			C;

public:

	void		make(const CVector2f &p0, const CVector2f &p1);

	/// return 1 either if the point moves away from the line, or no collision occurs. Else return a [0,1[ interval.
	float		testPoint(const CVector2f &start, const CVector2f &delta, float borderEpsilon=0.01f);
	/** return 1 either if the circle moves away from the line, or no collision occurs. Else return a [0,1[ interval.
	 * If collision occurs (ie return<1), return in "normal" the normal of the collision.
	 * It may be normal of edge (+-), or normal against a point of the edge.
	 */
	float		testCircle(const CVector2f &start, const CVector2f &delta, float radius, CVector2f &normal);
	// TODO_BBOX: float		testBBox();
};



} // NLPACS


#endif // NL_EDGE_COLLIDE_H

/* End of edge_collide.h */
