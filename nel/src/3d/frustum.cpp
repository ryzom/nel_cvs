/** \file frustum.cpp
 * <File description>
 *
 * $Id: frustum.cpp,v 1.3 2001/02/05 10:10:09 coutelas Exp $
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

#include "nel/3d/frustum.h"
#include "nel/misc/matrix.h"
#include <math.h>


using namespace NLMISC;

namespace NL3D 
{


void			CFrustum::init(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	Left= left;
	Right= right;
	Bottom=	bottom;
	Top= top;
	Near= znear;
	Far= zfar;
	Perspective= perspective;
}

void			CFrustum::init(float width, float height, float znear, float zfar, bool perspective)
{
	init(-width/2, width/2, -height/2, height/2, znear, zfar, perspective);
}
void			CFrustum::initPerspective(float fov, float aspectRatio, float znear, float zfar)
{
	float	w,h;
	w= 2*znear*(float)tan(fov/2);
	h= w/aspectRatio;
	init(w,h,znear,zfar,true);
}
void			CFrustum::getValues(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const
{
	left= Left;
	right= Right;
	bottom=	Bottom;
	top= Top;
	znear= Near;
	zfar= Far;
}


// ***************************************************************************
CVector			CFrustum::project(const CVector &vec) const
{
	CVector		ret;
	float		decalX, decalY;
	float		w, h;
	float		OOw, OOh;

	// Fast transform to openGL like axis, but with Z to front (and not back).
	CVector		pt;
	pt.x= vec.x;
	pt.y= vec.z;
	pt.z= vec.y;	// => this is a left hand axis.

	decalX= (Right+Left);
	decalY= (Top+Bottom);
	w= Right-Left;
	h= Top-Bottom;
	OOw= 1.0f/w;
	OOh= 1.0f/h;

	// project to -1..+1.
	if(Perspective)
	{
		ret.x= (2*Near*pt.x + decalX*pt.z)*OOw;
		ret.x/= pt.z;
		ret.y= (2*Near*pt.y + decalY*pt.z)*OOh;
		ret.y/= pt.z;
	}
	else
	{
		ret.x= (2-decalX)*OOw;
		ret.y= (2-decalY)*OOh;
	}

	// Map it to 0..1.
	ret.x= 0.5f*(ret.x+1);
	ret.y= 0.5f*(ret.y+1);
	ret.z= 0;

	return ret;
}


} // NL3D
