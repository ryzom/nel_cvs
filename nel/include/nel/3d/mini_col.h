/** \file mini_col.h
 * <File description>
 *
 * $Id: mini_col.h,v 1.3 2001/01/02 10:21:43 berenguier Exp $
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

#ifndef NL_MINI_COL_H
#define NL_MINI_COL_H


#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/plane.h"
#include "nel/3d/quad_tree.h"
#include "nel/3d/triangle.h"
#include "nel/3d/landscape.h"


namespace NL3D 
{


using	NLMISC::CVector;
using	NLMISC::CPlane;


// ***************************************************************************
/**
 * This is a TEMPORARY collision "system".
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMiniCol
{
public:

	/// Constructor
	CMiniCol();

	/** Init the size of the collision system, and init it with the landscape.
	 * \param size the radius of the area.
	 */
	void			init(CLandscape *land, float radius= 100);


	/** Reset the center of interset of the collision system.
	 */
	void			setCenter(const CVector& center);


	/** This function test if a move is OK, by snaping it to ground, and test if angle is<45°.
	 * If !OK, cur is set to prec, and false is returned.
	 */
	bool			testMove(const CVector &prec, CVector &cur);


	/** This function snap a position on the current set of faces.
	 * hbot and hup are the margin where pos.z can't change. (the pos can't move higher than +hup and lower than -hbot)
	 */
	bool			snapToGround(CVector &pos, float hup=0.5, float hbot=1000);


// *****************************
private:
	struct	CNode
	{
		CTriangle	Face;
		CPlane		Plane;
	};
	CQuadTree<CNode>	_QuadTree;

	float				_Radius;
	CRefPtr<CLandscape>	_Landscape;
	bool				_Inited;
	CVector				_Center;

	/// add faces to the collision system.
	void			addFaces(const std::vector<CTriangle> &faces);

	/** add landscape/patch faces to the collision system. 
	 * \param size the diameter (2*radius) of the area.
	 */
	void			addLandscapePart(CLandscape &land, float size);
};


} // NL3D


#endif // NL_MINI_COL_H

/* End of mini_col.h */
