/** \file compass.cpp
 * Snowballs 2 specific code for managing the compass.
 * This code was taken from Snowballs 1.
 *
 * $Id: compass.cpp,v 1.1 2001/07/18 16:06:34 lecroart Exp $
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

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

#include <nel/3d/u_material.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_texture.h>

#include "mouse_listener.h"
#include "camera.h"
#include "client.h"

//
// Namespaces
//

using namespace NLMISC;
using namespace NL3D;
using namespace std;

//
// Variables
//

static NL3D::UMaterial *CompassMaterial = NULL;

//
// Functions
//

void initCompass ()
{
	CompassMaterial = Driver->createMaterial ();
	CompassMaterial->initUnlit ();
//	CompassMaterial->setBlendFunc (UMaterial::srcalpha, UMaterial::invsrcalpha);
//	CompassMaterial->setBlend(true);
//	CompassMaterial->setColor(CRGBA(50,255,255,150));
}

void updateCompass ()
{
//	float x = 0.9f*4.f/3.f;
//	float y = 0.1f;
	float x = 0.5f;
	float y = 0.5f;
	float radius = 0.015f;

	// tri
	CTriangle tri;
	tri.V0.set (-radius, 0, 0);
	tri.V1.set (radius, 0, 0);
	tri.V2.set (0, 3*radius, 0);

	CQuad quad;
	// quad
	quad.V0.set (-radius, -radius, 0);
	quad.V1.set ( radius, -radius, 0);
	quad.V2.set ( radius,  radius, 0);
	quad.V3.set (-radius,  radius, 0);
	
	Driver->setMatrixMode2D11 ();

	CMatrix mtx;
	mtx.identity();
	Driver->setViewMatrix (mtx);

	// up
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateY(MouseListener->getOrientation() - (float)Pi/2);
	mtx.translate(CVector(0,radius,0));
	Driver->setViewMatrix (mtx);
	Driver->drawTriangle (tri,  *CompassMaterial);

	// down
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateY(MouseListener->getOrientation() + (float)Pi/2);
	mtx.translate(CVector(0,radius,0));
	Driver->setViewMatrix (mtx);
	Driver->drawTriangle (tri,  *CompassMaterial);

	// left
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateY(MouseListener->getOrientation());
	mtx.translate(CVector(0,radius,0));
	Driver->setViewMatrix (mtx);
	Driver->drawTriangle (tri,  *CompassMaterial);

	// right
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateY(MouseListener->getOrientation() - (float)Pi);
	mtx.translate(CVector(0,radius,0));
	Driver->setViewMatrix (mtx);
	Driver->drawTriangle (tri,  *CompassMaterial);

	// center
	mtx.identity();
	mtx.translate(CVector(x,y,0));
	mtx.rotateY(MouseListener->getOrientation());
	Driver->setViewMatrix (mtx);
	Driver->drawQuad (quad,  *CompassMaterial);
}

void releaseCompass ()
{
	Driver->deleteMaterial (CompassMaterial);
	CompassMaterial = NULL;
}
