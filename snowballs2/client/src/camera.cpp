/** \file camera.cpp
 * Camera management
 *
 * $Id: camera.cpp,v 1.3 2001/07/12 14:36:33 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <math.h>
#include <nel/misc/vectord.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_3d_mouse_listener.h>
#include <nel/3d/u_instance.h>

#include "client.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

UCamera				*Camera = NULL;

float				ViewLagBehind = 3.0f;
float				ViewHeight = 2.0f;

UInstance			*Snow = NULL;

void	initCamera()
{
	Camera = Scene->getCam();
	Camera->setTransformMode (UTransformable::DirectMatrix);
	Camera->setPerspective ((float)Pi/2.f, 1.33f, 0.1f, 1000);
	Camera->lookAt (CVector(ConfigFile.getVar("StartPoint").asFloat(0),
							ConfigFile.getVar("StartPoint").asFloat(1),
							ConfigFile.getVar("StartPoint").asFloat(2)),
							CVectorD (0,0,0));


	Snow = Scene->createInstance("snow.ps");
	Snow->setScale(1.0f, 1.0f, 1.0f);
	Snow->setPivot(0.0f, 0.0f, 0.0f);
	Snow->show();

}

void	updateCamera()
{
	CMatrix	mat = MouseListener->getViewMatrix();
	mat.translate(CVector(0.0f, -ViewLagBehind, ViewHeight));
	float	alpha = (float)atan(ViewHeight/ViewLagBehind);
	mat.rotateX(-alpha);
	Camera->setMatrix(mat);

	Snow->setMatrix (mat);
}

void	releaseCamera()
{
}