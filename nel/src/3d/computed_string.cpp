/** \file computed_string.cpp
 * Computed string
 *
 * $Id: computed_string.cpp,v 1.7 2000/12/21 13:39:31 corvazier Exp $
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

#include "nel/3d/computed_string.h"
#include "nel/3d/font_generator.h"
#include "nel/3d/texture.h"
#include "nel/3d/primitive_block.h"
#include "nel/3d/material.h"
#include "nel/3d/nelu.h"

#include "nel/misc/smart_ptr.h"
#include "nel/misc/debug.h"

#include "nel/misc/file.h"


namespace NL3D {



/*------------------------------------------------------------------*\
							render2D()
\*------------------------------------------------------------------*/
void CComputedString::render2D (IDriver& driver,
								float x, float z,
								THotSpot hotspot,
								float scaleX, float scaleZ,
								float rotateY
								)
{
	//x*=ResX/ResY;
	x*=(float)4/3;
	driver.setFrustum(0, 4.0f/3.0f, 0, 1, -1, 1, false);  // resX/resY

	// Computing hotspot translation vector
	CVector hotspotVector(0,0,0);

	if(hotspot==MiddleLeft)
		hotspotVector = CVector(0,0,-StringHeight/2);
	
	if(hotspot==TopLeft)
		hotspotVector = CVector(0,0,StringHeight);
	
	if(hotspot==MiddleBottom)
		hotspotVector = CVector(-StringWidth/2,0,0);
	
	if(hotspot==MiddleMiddle)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight/2);
	
	if(hotspot==MiddleTop)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight);
	
	if(hotspot==BottomRight)
		hotspotVector = CVector(-StringWidth,0,0);
	
	if(hotspot==MiddleRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight/2);
	
	if(hotspot==TopRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight);
	
	
	// tansformation matrix initialized to identity
	CMatrix matrix;
	matrix.identity();
		
	// view matrix <-> identity
	driver.setupViewMatrix(matrix);

	// model matrix :
	// centering to hotspot, then scaling, rotating, and translating.
	matrix.translate(CVector(x,0,z));
	matrix.rotateY(rotateY);
	matrix.scale(CVector(scaleX,1,scaleZ));
	matrix.translate(hotspotVector);
	driver.setupModelMatrix(matrix);
	
	driver.activeVertexBuffer(Vertices);
	
	// rendering each primitives 
	for(uint32 i=0; i<Primitives.size(); i++)
	{
		driver.render(Primitives[i], Materials[i]);
	}
}


/*------------------------------------------------------------------*\
							render3D()
\*------------------------------------------------------------------*/
void CComputedString::render3D (CVector pos)
{
	CMatrix matrix;
	matrix.translate(pos);
	CNELU::Driver->setupModelMatrix(matrix);
	CNELU::Driver->activeVertexBuffer(Vertices);
	
	// rendering each primitives 
	for(uint32 i=0; i<Primitives.size(); i++)
	{
		CNELU::Driver->render(Primitives[i], Materials[i]);
	}
}



} // NL3D