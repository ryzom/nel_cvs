/** \file camera.cpp
 * <File description>
 *
 * $Id: camera.cpp,v 1.7 2001/02/28 14:28:57 berenguier Exp $
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
void	CCamera::registerBasic()
{
	CMOT::registerModel(CameraId, TransformId, CCamera::creator);
}


// ***************************************************************************
CCamera::CCamera()
{
	setFrustum(1.0f, 1.0f, 0.01f, 1.0f);
}
// ***************************************************************************
void		CCamera::setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective)
{
	_Frustum.init( left, right,bottom, top, znear, zfar, perspective);
}
// ***************************************************************************
void		CCamera::setFrustum(float width, float height, float znear, float zfar, bool perspective)
{
	_Frustum.init(width, height, znear, zfar, perspective);
}
// ***************************************************************************
void		CCamera::setPerspective(float fov, float aspectRatio, float znear, float zfar)
{
	_Frustum.initPerspective(fov, aspectRatio, znear, zfar);
}
// ***************************************************************************
void		CCamera::getFrustum(float &left, float &right, float &bottom, float &top, float &znear, float &zfar) const
{
	left= _Frustum.Left;
	right= _Frustum.Right;
	bottom=	_Frustum.Bottom;
	top= _Frustum.Top;
	znear= _Frustum.Near;
	zfar= _Frustum.Far;
}
// ***************************************************************************
bool		CCamera::isOrtho() const
{
	return !_Frustum.Perspective;
}
// ***************************************************************************
bool		CCamera::isPerspective() const
{
	return _Frustum.Perspective;
}


}

