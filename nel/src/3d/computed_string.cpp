/** \file computed_string.cpp
 * Computed string
 *
 * $Id: computed_string.cpp,v 1.25 2002/11/21 15:55:06 berenguier Exp $
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

#include "std3d.h"

#include "3d/computed_string.h"
#include "3d/texture.h"
#include "3d/primitive_block.h"
#include "3d/material.h"

#include "nel/misc/smart_ptr.h"
#include "nel/misc/debug.h"

#include "nel/misc/file.h"
#include "nel/misc/fast_mem.h"

using namespace std;

namespace NL3D {



/*------------------------------------------------------------------*\
							getHotSpotVector()
\*------------------------------------------------------------------*/
CVector CComputedString::getHotSpotVector(THotSpot hotspot)
{
	CVector hotspotVector(0,0,0);

	if (hotspot==MiddleLeft)
		hotspotVector = CVector(0,0,-StringHeight/2);
	
	if (hotspot==TopLeft)
		hotspotVector = CVector(0,0,-StringHeight);
	
	if (hotspot==MiddleBottom)
		hotspotVector = CVector(-StringWidth/2,0,0);
	
	if (hotspot==MiddleMiddle)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight/2);
	
	if (hotspot==MiddleTop)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight);
	
	if (hotspot==BottomRight)
		hotspotVector = CVector(-StringWidth,0,0);
	
	if (hotspot==MiddleRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight/2);
	
	if (hotspot==TopRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight);

	return hotspotVector;
}


/*------------------------------------------------------------------*\
							render2D()
\*------------------------------------------------------------------*/
void CComputedString::render2D (IDriver& driver,
								float x, float z,
								THotSpot hotspot,
								float scaleX, float scaleZ,
								float rotateY,
								bool useScreenAR43, bool roundToNearestPixel
								)
{
	if (Vertices.getNumVertices() == 0)
		return;

	// get window size
	uint32	wndWidth, wndHeight;
	driver.getWindowSize(wndWidth, wndHeight);
	// scale to window size.
	x*= wndWidth;
	z*= wndHeight;

	driver.setFrustum(0, (float)wndWidth, 0, (float)wndHeight, -1, 1, false);  // resX/resY

	// Computing hotspot translation vector
	CVector hotspotVector = getHotSpotVector(hotspot);

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
	// scale the string to follow window aspect Ratio
	if(useScreenAR43)
	{
		matrix.scale(CVector((3.0f*wndWidth)/(4.0f*wndHeight),1,1));
	}
	matrix.translate(hotspotVector);
	// if roundToNearestPixel, then snap the position to the nearest pixel
	if( roundToNearestPixel)
	{
		CVector	pos= matrix.getPos();
		pos.x= (float)floor(pos.x+0.5f);
		pos.z= (float)floor(pos.z+0.5f);
		matrix.setPos(pos);
	}
	// setup the matrix
	driver.setupModelMatrix(matrix);
	
	driver.activeVertexBuffer(Vertices);

	// rendering each primitives 
	Material->setZFunc (CMaterial::always);
	Material->setZWrite (false);
	Material->setColor (Color);
	driver.renderQuads (*Material, 0, Vertices.getNumVertices()/4);
}

/*------------------------------------------------------------------*\
							render2DClip()
\*------------------------------------------------------------------*/
void CComputedString::render2DClip (IDriver& driver, 
					float x, float z,
					float xmin, float zmin, float xmax, float zmax)
{
	if (Vertices.getNumVertices() == 0)
		return;

	// get window size
	uint32	wndWidth, wndHeight;
	driver.getWindowSize(wndWidth, wndHeight);
	// scale to window size.
	x*= wndWidth;
	z*= wndHeight;
	xmin*= wndWidth;
	xmax*= wndWidth;
	zmin*= wndHeight;
	zmax*= wndHeight;

	driver.setFrustum(0, (float)wndWidth, 0, (float)wndHeight, -1, 1, false);  // resX/resY

	// tansformation matrix initialized to identity
	CMatrix matrix;
	matrix.identity();
		
	// view matrix <-> identity
	driver.setupViewMatrix (matrix);
	driver.setupModelMatrix (matrix);
	
	// rendering each primitives 
	Material->setZFunc (CMaterial::always);
	Material->setZWrite (false);
	Material->setColor (Color);

	// clipping
	VerticesClipped.setNumVertices (Vertices.getNumVertices());
	uint32 nNumQuad = 0;
	CVector *pIniPos0 = (CVector*)Vertices.getVertexCoordPointer (0);
	CVector *pIniPos2 = (CVector*)(((char*)pIniPos0) + Vertices.getVertexSize()*2);
	CVector *pClipPos0 = (CVector*)VerticesClipped.getVertexCoordPointer (0);
	CVector *pClipPos1 = (CVector*)(((char*)pClipPos0) + Vertices.getVertexSize());
	CVector *pClipPos2 = (CVector*)(((char*)pClipPos1) + Vertices.getVertexSize());
	CVector *pClipPos3 = (CVector*)(((char*)pClipPos2) + Vertices.getVertexSize());
	CUV *pClipUV0 = (CUV*)VerticesClipped.getTexCoordPointer (0, 0);
	CUV *pClipUV1 = (CUV*)(((char*)pClipUV0) + Vertices.getVertexSize());
	CUV *pClipUV2 = (CUV*)(((char*)pClipUV1) + Vertices.getVertexSize());
	CUV *pClipUV3 = (CUV*)(((char*)pClipUV2) + Vertices.getVertexSize());
	float ratio;
	for (uint32 i = 0; i < Vertices.getNumVertices(); i+=4)
	{
		if (((x+pIniPos0->x) > xmax) || ((x+pIniPos2->x) < xmin) ||
			((z+pIniPos0->z) > zmax) || ((z+pIniPos2->z) < zmin))
		{
			// Totally clipped do nothing
		}
		else
		{
			memcpy (pClipPos0, pIniPos0, Vertices.getVertexSize()*4);

			pClipPos0->x += x; pClipPos1->x += x; pClipPos2->x += x; pClipPos3->x += x;
			pClipPos0->z += z; pClipPos1->z += z; pClipPos2->z += z; pClipPos3->z += z;
			if ((pClipPos0->x >= xmin) && (pClipPos0->z >= zmin) && (pClipPos2->x <= xmax) && (pClipPos2->z <= zmax))
			{
				// Not clipped
			}
			else
			{
				// Partially clipped

				if (pClipPos0->x < xmin)
				{
					ratio = ((float)(xmin - pClipPos0->x))/((float)(pClipPos1->x - pClipPos0->x));
					pClipPos3->x = pClipPos0->x = xmin;
					pClipUV0->U += ratio*(pClipUV1->U - pClipUV0->U);
					pClipUV3->U += ratio*(pClipUV2->U - pClipUV3->U);
				}

				if (pClipPos0->z < zmin)
				{
					ratio = ((float)(zmin - pClipPos0->z))/((float)(pClipPos3->z - pClipPos0->z));
					pClipPos1->z = pClipPos0->z = zmin;
					pClipUV0->V += ratio*(pClipUV3->V - pClipUV0->V);
					pClipUV1->V += ratio*(pClipUV2->V - pClipUV1->V);
				}

				if (pClipPos2->x > xmax)
				{
					ratio = ((float)(xmax - pClipPos2->x))/((float)(pClipPos3->x - pClipPos2->x));
					pClipPos2->x = pClipPos1->x = xmax;
					pClipUV2->U += ratio*(pClipUV3->U - pClipUV2->U);
					pClipUV1->U += ratio*(pClipUV0->U - pClipUV1->U);
				}

				if (pClipPos2->z > zmax)
				{
					ratio = ((float)(zmax - pClipPos2->z))/((float)(pClipPos1->z - pClipPos2->z));
					pClipPos2->z = pClipPos3->z = zmax;
					pClipUV2->V += ratio*(pClipUV1->V - pClipUV2->V);
					pClipUV3->V += ratio*(pClipUV0->V - pClipUV3->V);
				}
			}
			++nNumQuad;
			pClipPos0 = (CVector*)(((char*)pClipPos0) + Vertices.getVertexSize()*4);
			pClipPos1 = (CVector*)(((char*)pClipPos0) + Vertices.getVertexSize());
			pClipPos2 = (CVector*)(((char*)pClipPos1) + Vertices.getVertexSize());
			pClipPos3 = (CVector*)(((char*)pClipPos2) + Vertices.getVertexSize());
			pClipUV0 = (CUV*)( ((char*)pClipUV0) + Vertices.getVertexSize()*4 );
			pClipUV1 = (CUV*)(((char*)pClipUV0) + Vertices.getVertexSize());
			pClipUV2 = (CUV*)(((char*)pClipUV1) + Vertices.getVertexSize());
			pClipUV3 = (CUV*)(((char*)pClipUV2) + Vertices.getVertexSize());
		}
		pIniPos0 = (CVector*)(((char*)pIniPos0) + Vertices.getVertexSize()*4);
		pIniPos2 = (CVector*)(((char*)pIniPos0) + Vertices.getVertexSize()*2);
	}
	//VerticesClipped.setNumVertices (4*nNumQuad);
	driver.activeVertexBuffer (VerticesClipped);
	driver.renderQuads (*Material, 0, nNumQuad);
}

/*------------------------------------------------------------------*\
							render3D()
\*------------------------------------------------------------------*/
void CComputedString::render3D (IDriver& driver,CMatrix matrix,THotSpot hotspot)
{
	if (Vertices.getNumVertices() == 0)
		return;
	// Computing hotspot translation vector
	CVector hotspotVector = getHotSpotVector(hotspot);
	matrix.translate(hotspotVector);

	// get window size
	uint32	wndWidth, wndHeight;
	driver.getWindowSize(wndWidth, wndHeight);
	// scale according to window height (backward compatibility)
	matrix.scale(1.0f/wndHeight);

	// render
	driver.setupModelMatrix(matrix);
	driver.activeVertexBuffer(Vertices);

	// Rendering each primitive blocks
	Material->setZFunc (CMaterial::lessequal);
	Material->setZWrite (true);
	Material->setColor (Color);
	driver.renderQuads (*Material,0,Vertices.getNumVertices()/4);
}


} // NL3D
