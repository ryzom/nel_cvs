/* camera.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: camera.cpp,v 1.1 2000/10/06 16:43:58 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/3d/camera.h"


namespace	NL3D
{


// ***************************************************************************
static	IModel	*creatorCamera() {return new CCamera;}
void	CCamera::registerBasic()
{
	CMOT::registerModel(CameraModelId, TransformModelId, creatorCamera);
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

