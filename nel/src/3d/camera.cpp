/** \file camera.cpp
 * <File description>
 *
 * $Id: camera.cpp,v 1.4 2000/11/23 15:51:12 berenguier Exp $
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

#include "nel/3d/camera.h"


namespace	NL3D
{


// ***************************************************************************
static	IModel	*creatorCamera() {return new CCamera;}
void	CCamera::registerBasic()
{
	CMOT::registerModel(CameraId, TransformId, creatorCamera);
}


// ***************************************************************************
CCamera::CCamera()
{
	setFrustum(1.0f, 1.0f, 0.01f, 1.0f);
}
// ***************************************************************************
void		CCamera::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	Left= left;
	Right= right;
	Bottom=	bottom;
	Top= top;
	Near= znear;
	Far= zfar;
	Perspective= perspective;
}
// ***************************************************************************
void		CCamera::setFrustum(float width, float height, float znear, float zfar, bool perspective)
{
	setFrustum(-width/2, width/2, -height/2, height/2, znear, zfar, perspective);
}
// ***************************************************************************
void		CCamera::setPerspective(float fov, float aspectRatio, float znear, float zfar)
{
	float	w,h;
	w= 2*znear*(float)tan(fov/2);
	h= w/aspectRatio;
	setFrustum(w,h,znear,zfar,true);
}
// ***************************************************************************
void		CCamera::getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar)
{
	left= Left;
	right= Right;
	bottom=	Bottom;
	top= Top;
	znear= Near;
	zfar= Far;
}
// ***************************************************************************
bool		CCamera::isOrtho()
{
	return !Perspective;
}
// ***************************************************************************
bool		CCamera::isPerspective()
{
	return Perspective;
}


}

