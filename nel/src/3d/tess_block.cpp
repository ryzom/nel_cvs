/** \file tess_block.cpp
 * <File description>
 *
 * $Id: tess_block.cpp,v 1.2 2001/06/15 16:24:45 corvazier Exp $
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

#include "3d/tess_block.h"


namespace NL3D 
{


// ***************************************************************************
void			CTessBlock::extendSphere(const CVector &vec)
{
	if(Empty)
	{
		Empty= false;
		BBox.setCenter(vec);
		BBox.setHalfSize(CVector::Null);
	}
	else
		BBox.extend(vec);

	BSphere.Center= BBox.getCenter();
	BSphere.Radius= BBox.getRadius();
}
// ***************************************************************************
void			CTessBlock::resetClip()
{
	Clipped= false;
	FullFar1= false;
	EmptyFar1= false;
}
// ***************************************************************************
void			CTessBlock::clip(const std::vector<CPlane>	&pyramid)
{
	Clipped= false;
	for(sint i=0;i<(sint)pyramid.size();i++)
	{
		// If entirely out.
		if(!BSphere.clipBack(pyramid[i]))
		{
			Clipped= true;
			break;
		}
	}
}
// ***************************************************************************
void			CTessBlock::clipFar(const CVector &refineCenter, float tileDistNear, float farTransition)
{
	float	r= (refineCenter-BSphere.Center).norm();
	if( (r-BSphere.Radius) > tileDistNear)
	{
		FullFar1= true;
	}
	else
	{
		if( (r+BSphere.Radius) < (tileDistNear-farTransition) )
			EmptyFar1= true;
	}
}



} // NL3D
