/** \file computed_string.cpp
 * Computed string
 *
 * $Id: computed_string.cpp,v 1.22 2002/08/22 13:38:45 besson Exp $
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

	if(hotspot==MiddleLeft)
		hotspotVector = CVector(0,0,-StringHeight/2);
	
	if(hotspot==TopLeft)
		hotspotVector = CVector(0,0,-StringHeight);
	
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

	return hotspotVector;
}


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
	if (Vertices.getNumVertices() == 0)
		return;
	//x*=ResX/ResY;
	x*=(float)4/3;

	driver.setFrustum(0, 4.0f/3.0f, 0, 1, -1, 1, false);  // resX/resY

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
	matrix.translate(hotspotVector);
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
	x *= 4.0f/3.0f;
	xmin *= 4.0f/3.0f;
	xmax *= 4.0f/3.0f;

	driver.setFrustum(0, 4.0f/3.0f, 0, 1, -1, 1, false);  // resX/resY

	// Computing hotspot translation vector
/*	CVector hotspotVector = getHotSpotVector(hotspot);

	x += hotspotVector.x;
	z += hotspotVector.z;
*/	
	// tansformation matrix initialized to identity
	CMatrix matrix;
	matrix.identity();
		
	// view matrix <-> identity
	driver.setupViewMatrix (matrix);

	// model matrix :
	// centering to hotspot, then scaling, rotating, and translating.
	/*matrix.translate(CVector(x,0,z));
	matrix.rotateY(rotateY);
	matrix.scale(CVector(scaleX,1,scaleZ));
	matrix.translate(hotspotVector);*/
	driver.setupModelMatrix (matrix);
	
	// rendering each primitives 
	Material->setZFunc (CMaterial::always);
	Material->setZWrite (false);
	Material->setColor (Color);

	// clipping
	VerticesClipped.setNumVertices (Vertices.getNumVertices());
	uint32 nNumQuad = 0;
	for (uint32 i = 0; i < (Vertices.getNumVertices()/4); ++i)
	{
		CVector *pPos0 = (CVector*)Vertices.getVertexCoordPointer (i*4+0);
		CVector *pPos2 = (CVector*)Vertices.getVertexCoordPointer (i*4+2);

		if (((x+pPos0->x) > xmax) || ((x+pPos2->x) < xmin) ||
			((z+pPos0->z) > zmax) || ((z+pPos2->z) < zmin))
		{
			// Totally clipped do nothing
		}
		else
		{
			NLMISC::CFastMem::memcpy (	VerticesClipped.getVertexCoordPointer (nNumQuad*4), 
								Vertices.getVertexCoordPointer (i*4), 
								Vertices.getVertexSize()*4 );
			CVector *pCPos0 = (CVector*)VerticesClipped.getVertexCoordPointer (nNumQuad*4+0);
			CVector *pCPos1 = (CVector*)VerticesClipped.getVertexCoordPointer (nNumQuad*4+1);
			CVector *pCPos2 = (CVector*)VerticesClipped.getVertexCoordPointer (nNumQuad*4+2);
			CVector *pCPos3 = (CVector*)VerticesClipped.getVertexCoordPointer (nNumQuad*4+3);
			pCPos0->x += x; pCPos1->x += x; pCPos2->x += x; pCPos3->x += x;
			pCPos0->z += z; pCPos1->z += z; pCPos2->z += z; pCPos3->z += z;
			if ((pCPos0->x >= xmin) && (pCPos0->z >= zmin) && (pCPos2->x <= xmax) && (pCPos2->z <= zmax))
			{
				// Not clipped
			}
			else
			{
				// Partially clipped
				CUV *pCuv0 = (CUV*)VerticesClipped.getTexCoordPointer (nNumQuad*4+0, 0);
				CUV *pCuv1 = (CUV*)VerticesClipped.getTexCoordPointer (nNumQuad*4+1, 0);
				CUV *pCuv2 = (CUV*)VerticesClipped.getTexCoordPointer (nNumQuad*4+2, 0);
				CUV *pCuv3 = (CUV*)VerticesClipped.getTexCoordPointer (nNumQuad*4+3, 0);
				float ratio;

				if (pCPos0->x < xmin)
				{
					ratio = ((float)(xmin - pCPos0->x))/((float)(pCPos1->x - pCPos0->x));
					pCPos3->x = pCPos0->x = xmin;
					pCuv0->U += ratio*(pCuv1->U - pCuv0->U);
					pCuv3->U += ratio*(pCuv2->U - pCuv3->U);
				}

				if (pCPos0->z < zmin)
				{
					ratio = ((float)(zmin - pCPos0->z))/((float)(pCPos3->z - pCPos0->z));
					pCPos1->z = pCPos0->z = zmin;
					pCuv0->V += ratio*(pCuv3->V - pCuv0->V);
					pCuv1->V += ratio*(pCuv2->V - pCuv1->V);
				}

				if (pCPos2->x > xmax)
				{
					ratio = ((float)(xmax - pCPos2->x))/((float)(pCPos3->x - pCPos2->x));
					pCPos2->x = pCPos1->x = xmax;
					pCuv2->U += ratio*(pCuv3->U - pCuv2->U);
					pCuv1->U += ratio*(pCuv0->U - pCuv1->U);
				}

				if (pCPos2->z > zmax)
				{
					ratio = ((float)(zmax - pCPos2->z))/((float)(pCPos1->z - pCPos2->z));
					pCPos2->z = pCPos3->z = zmax;
					pCuv2->V += ratio*(pCuv1->V - pCuv2->V);
					pCuv3->V += ratio*(pCuv0->V - pCuv3->V);
				}
			}
			++nNumQuad;
		}
	}

	driver.activeVertexBuffer(VerticesClipped);
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

	driver.setupModelMatrix(matrix);
	driver.activeVertexBuffer(Vertices);

	// Rendering each primitive blocks
	Material->setZFunc (CMaterial::lessequal);
	Material->setZWrite (true);
	Material->setColor (Color);
	driver.renderQuads (*Material,0,Vertices.getNumVertices()/4);
}


} // NL3D
