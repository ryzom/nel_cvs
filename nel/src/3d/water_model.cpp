/** \file water_model.cpp
 * <File description>
 *
 * $Id: water_model.cpp,v 1.37 2003/05/28 10:06:07 vizerie Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/misc/vector_2f.h"
#include "nel/misc/vector_h.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/animation_time.h"
#include "3d/water_model.h"
#include "3d/water_shape.h"
#include "3d/water_pool_manager.h"
#include "3d/water_height_map.h"
#include "3d/dru.h"
#include "3d/scene.h"
#include "3d/driver.h"
#include "3d/render_trav.h"
#include "3d/anim_detail_trav.h"
#include "3d/texture_emboss.h"
#include "3d/texture_bump.h"




// to force the rendering of water with the simplest shader on all cards
//#define FORCE_SIMPLE_WATER_RENDER


namespace NL3D {

// for normal rendering
CMaterial CWaterModel::_WaterMat;
// for simple rendering
CMaterial CWaterModel::_SimpleWaterMat;
CVertexBuffer CWaterModel::_SimpleRenderVB;


//=======================================================================

CWaterModel::CWaterModel()
{
	setOpacity(false);
	setTransparency(true);
	setOrderingLayer(1);

	// RenderFilter: We are a SegRemanece
	_RenderFilterType= UScene::FilterWater;
}

//=======================================================================

void CWaterModel::registerBasic()
{
	CScene::registerModel(WaterModelClassId, TransformShapeId, CWaterModel::creator);	
}


//=======================================================================

ITrack* CWaterModel::getDefaultTrack (uint valueId)
{
	nlassert(Shape);
	CWaterShape *ws = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	switch (valueId)
	{
		case PosValue:			return ws->getDefaultPos(); break;		
		case ScaleValue:		return ws->getDefaultScale(); break;
		case RotQuatValue:		return ws->getDefaultRotQuat(); break;
		default: // delegate to parent
			return CTransformShape::getDefaultTrack(valueId);
		break;
	}
}

//=======================================================================

uint32	CWaterModel::getWaterHeightMapID() const
{
	CWaterShape *ws = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	return ws->_WaterPoolID;
}

//=======================================================================

float	CWaterModel::getHeightFactor() const
{
	CWaterShape *ws = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	return ws->_WaveHeightFactor;
}


//=======================================================================

float   CWaterModel::getHeight(const NLMISC::CVector2f &pos)
{
	CWaterShape *ws		 = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);	
	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(ws->_WaterPoolID);
	const float height   = whm.getHeight(pos);
	return height * ws->_WaveHeightFactor + this->getPos().z;
}

//=======================================================================

float   CWaterModel::getAttenuatedHeight(const NLMISC::CVector2f &pos, const NLMISC::CVector &viewer)
{	
	CWaterShape *ws		 = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);			
	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(ws->_WaterPoolID);
	const float maxDist = whm.getUnitSize() * (whm.getSize() >> 1);		
	const NLMISC::CVector planePos(pos.x, pos.y, this->getMatrix().getPos().z);
	const float userDist = (planePos - viewer).norm();

	if (userDist > maxDist)
	{
		return this->getMatrix().getPos().z;
	}
	else
	{
		const float height   = whm.getHeight(pos);
		return ws->_WaveHeightFactor * height * (1.f - userDist / maxDist) + this->getMatrix().getPos().z;
	}
}


//=======================================================================

// perform a bilinear on 4 values
//   0---1
//   |   |
//   3---2
static float inline BilinFilter(float v0, float v1, float v2, float v3, float u, float v)
{
	float g = v * v3 + (1.f - v) * v0;
	float h = v * v2 + (1.f - v) * v1;
	return u * h + (1.f - u) * g;
}



//=======================================================================

/// store a value in a water vertex buffer, and increment the pointer
static void inline FillWaterVB(uint8 *&vbPointer, float x, float y, float z, float nx, float ny)
{
	* (float *) vbPointer = x;
	((float *) vbPointer)[1] = y;
	((float *) vbPointer)[2] = z;
	*((float *) (vbPointer + 3 * sizeof(float))) = nx;
	*((float *) (vbPointer + 4 * sizeof(float))) = ny;
	vbPointer += 5 * sizeof(float);
}

//***************************************************************************************************************


/// this inline function setup one WaterPrev vertex
#ifdef NL_OS_WINDOWS
	__forceinline
#endif
static void SetupWaterVertex(  sint  qLeft,
							   sint  qRight,
							   sint  qUp,
							   sint  qDown,
							   sint  qSubLeft,
							   sint  qSubDown,
							   const NLMISC::CVector &inter,							   
							   float invWaterRatio,							   
							   sint  doubleWaterHeightMapSize,
							   CWaterHeightMap &whm,
							   uint8 *&vbPointer,
							   float offsetX,
							   float offsetY
							   )
{
	const float wXf = invWaterRatio * (inter.x + offsetX);
	const float wYf = invWaterRatio * (inter.y + offsetY);

	sint wx = (sint) floorf(wXf);
	sint wy = (sint) floorf(wYf);

	

	if (!
		 (wx >= qLeft && wx < qRight && wy < qUp &&  wy >= qDown)
	   )
	{	
		// no perturbation is visible
		FillWaterVB(vbPointer, inter.x, inter.y, 0, 0, 0);		
	}
	else
	{
		

		// filter height and gradient at the given point
		const sint stride = doubleWaterHeightMapSize;

		
		const uint xm	  = (uint) (wx - qSubLeft);
		const uint ym	  = (uint) (wy - qSubDown);
		const sint offset = xm + stride * ym;
		const float			  *ptWater     = whm.getPointer()	  + offset;

	/*	float epsilon = 10E-5f;
		if (ptWater[0] > epsilon			 || ptWater[0] < epsilon 
			|| ptWater[1] > epsilon			 || ptWater[1] < epsilon 
			|| ptWater[stride] > epsilon	 || ptWater[stride] < epsilon
			|| ptWater[stride + 1] > epsilon || ptWater[stride + 1] < epsilon
			)
		{*/

			float deltaU = wXf - wx;
			float deltaV = wYf - wy;
			nlassert(deltaU >= 0.f && deltaU <= 1.f  && deltaV >= 0.f && deltaV <= 1.f);
			const float			  *ptWaterPrev = whm.getPrevPointer()  + offset;



			float g0x, g1x, g2x, g3x;  // x gradient for current 
			float g0xp, g1xp, g2xp, g3xp;

			float gradCurrX, gradCurrY;

			float g0y, g1y, g2y, g3y; // y gradient for previous map
			float g0yp, g1yp, g2yp, g3yp;

			float gradPrevX, gradPrevY;

			/// curr gradient

			g0x = ptWater[ 1] - ptWater[ - 1];
			g1x = ptWater[ 2] - ptWater[ 0 ];
			g2x = ptWater[ 2 + stride] - ptWater[ stride];
			g3x = ptWater[ 1 + stride] - ptWater[ - 1 + stride];

			gradCurrX = BilinFilter(g0x, g1x, g2x, g3x, deltaU, deltaV);


			g0y = ptWater[ stride] - ptWater[ - stride];
			g1y = ptWater[ stride + 1] - ptWater[ - stride + 1];
			g2y = ptWater[ (stride << 1) + 1] - ptWater[ 1];
			g3y = ptWater[ (stride << 1)] - ptWater[0];

			gradCurrY = BilinFilter(g0y, g1y, g2y, g3y, deltaU, deltaV);

			/// prev gradient

			g0xp = ptWaterPrev[ 1] - ptWaterPrev[ - 1];
			g1xp = ptWaterPrev[ 2] - ptWaterPrev[ 0  ];
			g2xp = ptWaterPrev[ 2 + stride] - ptWaterPrev[ + stride];
			g3xp = ptWaterPrev[ 1 + stride] - ptWaterPrev[ - 1 + stride];

			gradPrevX = BilinFilter(g0xp, g1xp, g2xp, g3xp, deltaU, deltaV);


			g0yp = ptWaterPrev[ stride] - ptWaterPrev[ - stride];
			g1yp = ptWaterPrev[ stride + 1] - ptWaterPrev[ - stride + 1];
			g2yp = ptWaterPrev[ (stride << 1) + 1] - ptWaterPrev[ 1 ];
			g3yp = ptWaterPrev[ (stride << 1)] - ptWaterPrev[ 0 ];

			gradPrevY = BilinFilter(g0yp, g1yp, g2yp, g3yp, deltaU, deltaV);


			/// current height
			float h = BilinFilter(ptWater[ 0 ], ptWater[ + 1], ptWater[ 1 + stride], ptWater[stride], deltaU, deltaV);

			/// previous height
			float hPrev = BilinFilter(ptWaterPrev[ 0 ], ptWaterPrev[ 1], ptWaterPrev[ 1 + stride], ptWaterPrev[stride], deltaU, deltaV);
			

			float timeRatio = whm.getBufferRatio();


			FillWaterVB(vbPointer, inter.x, inter.y, timeRatio * h + (1.f - timeRatio) * hPrev,
						4.5f * (timeRatio * gradCurrX + (1.f - timeRatio) * gradPrevX),
						4.5f * (timeRatio * gradCurrY + (1.f - timeRatio) * gradPrevY)
					   );



			//NLMISC::CVector2f *ptGrad  = whm.getGradPointer() + offset;





		/*	float dh1 = deltaV * ptWater[stride] + (1.f - deltaV) *  ptWater[0];
			float dh2 = deltaV * ptWater[stride + 1] + (1.f - deltaV) *  ptWater[1];
			float h = deltaU * dh2 + (1.f - deltaU ) * dh1;

			
			float gR = deltaV * ptGrad[stride + 1].x + (1.f - deltaV) * ptGrad[1].x;
			float gL = deltaV * ptGrad[stride].x + (1.f - deltaV) * ptGrad[0].x;

			float grU = 4.5f * (deltaU *  gR + (1.f - deltaU) * gL);

			gR = deltaV * ptGrad[stride + 1].y + (1.f - deltaV) * ptGrad[1].y;
			gL = deltaV * ptGrad[stride].y + (1.f - deltaV) * ptGrad[0].y;

			float grV = 4.5f * (deltaU *  gR + (1.f - deltaU) * gL);

			vb.setValueFloat3Ex (Water_VB_POS, vbIndex, inter.x, inter.y, h);
			vb.setValueFloat2Ex (Water_VB_DX, vbIndex, grU, grV);
			*/
		/*}
		else
		{
			// no perturbation is visible
			FillWaterVB(vbPointer, inter.x, inter.y, 0, 0, 0);		
		}*/
	}
}

//*********************************************************
// compute a rotation matrix from the K and the up vector
static void ComputeUpMatrix(const NLMISC::CVector &J, NLMISC::CMatrix &dest, const NLMISC::CMatrix &defaultMat)
{
	// if the J vector is oriented toward K, we must use another vector to compute the matrix
	const float JdotK = J.z;
	if ( (1.0f - fabsf(J.z)) < 10E-6)
	{	
		dest.setRot(defaultMat.getRot());		
	}
	else
	{
		NLMISC::CVector up	  = (NLMISC::CVector::K - JdotK * J).normed();
		NLMISC::CVector right = J ^ up;
		dest.setRot(right, J, up);
	}
}

//*************************************************************
// draw a 2d polygon after a transformation

/*
static void DrawPoly2D(CVertexBuffer &vb, IDriver *drv, const NLMISC::CVector &pos, const NLMISC::CPolygon &p)
{
	uint k;

	for (k = 0; k < p.Vertices.size(); ++k)
	{
		vb.setValueFloat3Ex (WATER_VB_POS, k, p.Vertices[k].x + pos.x, p.Vertices[k].y + pos.y, p.Vertices[k].z + pos.z);
		vb.setValueFloat2Ex (WATER_VB_DX,  k, 0, 0);			
	}
	static std::vector<uint32> ib;
	ib.resize(3 * p.Vertices.size());

	for (k = 0; k < p.Vertices.size() - 2; ++k)
	{
		ib[ k * 3      ] = 0;
		ib[ k * 3  + 1 ] = k + 1;
		ib[ k * 3  + 2 ] = k + 2;
	}	
	drv->renderSimpleTriangles(&ib[0], p.Vertices.size() - 2);	
}
*/


static void DrawPoly2D(CVertexBuffer &vb, IDriver *drv, const NLMISC::CMatrix &mat, const NLMISC::CPolygon &p)
{
	uint k;

	for (k = 0; k < p.Vertices.size(); ++k)
	{
		NLMISC::CVector tPos = mat * NLMISC::CVector(p.Vertices[k].x, p.Vertices[k].y, 0);
		vb.setValueFloat3Ex (WATER_VB_POS, k, tPos.x, tPos.y, tPos.z);								 			
		vb.setValueFloat2Ex (WATER_VB_DX,  k, 0, 0);			
	}
	static std::vector<uint32> ib;
	ib.resize(3 * p.Vertices.size());

	for (k = 0; k < p.Vertices.size() - 2; ++k)
	{
		ib[ k * 3      ] = 0;
		ib[ k * 3  + 1 ] = k + 1;
		ib[ k * 3  + 2 ] = k + 2;
	}	
	drv->renderSimpleTriangles(&ib[0], p.Vertices.size() - 2);	
}




/*
static void DrawPoly2D(CVertexBuffer &vb, IDriver *drv, const NLMISC::CPolygon &p)
{
	uint k;

	for (k = 0; k < p.Vertices.size(); ++k)
	{		
		vb.setValueFloat3Ex (WATER_VB_POS, k, p.Vertices[k].x, p.Vertices[k].y, p.Vertices[k].z);
		vb.setValueFloat2Ex (WATER_VB_DX,  k, 0, 0);			
	}
	static std::vector<uint32> ib;
	ib.resize(3 * p.Vertices.size());

	for (k = 0; k < p.Vertices.size() - 2; ++k)
	{
		ib[ k * 3      ] = 0;
		ib[ k * 3  + 1 ] = k + 1;
		ib[ k * 3  + 2 ] = k + 2;
	}	
	drv->renderSimpleTriangles(&ib[0], p.Vertices.size() - 2);	
}
*/


//***************************************************************************************************************
void	CWaterModel::traverseRender()
{				
	H_AUTO( NL3D_Water_Render );

	CRenderTrav					&renderTrav		= getOwnerScene()->getRenderTrav();
	IDriver						*drv			= renderTrav.getDriver();	


	#ifndef FORCE_SIMPLE_WATER_RENDER
		if (!drv->isWaterShaderSupported()) 
	#endif
	{	
		doSimpleRender(drv);
		return;
	}

	CWaterShape					*shape			= NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);	


	if (shape->_GridSizeTouched)
	{
		shape->setupVertexBuffer();
	}
	
	// inverted object world matrix
	//NLMISC::CMatrix invObjMat = getWorldMatrix().inverted();

	// viewer pos in world space
	const NLMISC::CVector &obsPos = /*invObjMat **/ renderTrav.CamPos;

	// camera matrix in world space
	const NLMISC::CMatrix &camMat = renderTrav.CamMatrix;

	// view matrix (inverted cam matrix)
	const NLMISC::CMatrix &viewMat = renderTrav.ViewMatrix;	

	// compute the camera matrix such as there is no rotation around the y axis
	NLMISC::CMatrix camMatUp;
	ComputeUpMatrix(camMat.getJ(), camMatUp, camMat);
	camMatUp.setPos(camMat.getPos());

	const NLMISC::CMatrix matViewUp = camMatUp.inverted();

	// plane z pos in world
	const float zHeight =  getWorldMatrix().getPos().z;

	const sint numStepX = CWaterShape::getScreenXGridSize();
	const sint numStepY = CWaterShape::getScreenYGridSize();

	const float invNumStepX = 1.f / numStepX;
	const float invNumStepY = 1.f / numStepY;

	const uint rotBorderSize = (shape->_MaxGridSize + (shape->_XGridBorder << 1) - numStepX) >> 1;
	
	const sint isAbove = obsPos.z > zHeight ? 1 : 0;	
			

	const float transitionDist	= shape->_TransitionRatio   * renderTrav.Far;
	
	
	NLMISC::CMatrix modelMat;
	modelMat.setPos(NLMISC::CVector(obsPos.x, obsPos.y, zHeight));
	drv->setupModelMatrix(modelMat);

	


	//==================//
	// material setup   //
	//==================//

	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(shape->_WaterPoolID);
		
	setupMaterialNVertexShader(drv, shape, obsPos, isAbove > 0, whm.getUnitSize() * (whm.getSize() >> 1), zHeight);	
	
	
	drv->setupMaterial(CWaterModel::_WaterMat);

	

	sint numPass = drv->beginMaterialMultiPass();
	nlassert(numPass == 1); // for now, we assume water is always rendered in a single pass !
	drv->setupMaterialPass(0);
		

	//setAttenuationFactor(drv, false, obsPos, camMat.getJ(), farDist);
	//disableAttenuation(drv);

	
	//================================//
	//	Vertex buffer setup           //
	//================================//
		
	drv->activeVertexBuffer(shape->_VB);

	//================================//
	//	tesselated part of the poly   //
	//================================//

	if (_ClippedPoly.Vertices.size())
	{
		
		

		//======================================//
		// Polygon projection on the near plane //
		//======================================//

		static NLMISC::CPolygon2D projPoly; // projected poly
		projPoly.Vertices.resize(_ClippedPoly.Vertices.size());
		const float Near = renderTrav.Near;
		const float xFactor = (numStepX >> 1) * Near  / renderTrav.Right;
		const float xOffset = (float) (numStepX >> 1) + 0.5f;
		const float yFactor = - (numStepX >> 1) * Near  / renderTrav.Top;
		const float yOffset = (float) (numStepY >> 1) - 0.5f * isAbove;
		
		const NLMISC::CMatrix projMat =  matViewUp * getWorldMatrix();
		uint k;
		for (k = 0; k < _ClippedPoly.Vertices.size(); ++k)
		{
			// project points in the view
			NLMISC::CVector t = projMat * _ClippedPoly.Vertices[k];
			float invY = 1.f / t.y;
			projPoly.Vertices[k].set(xFactor * t.x * invY + xOffset, yFactor * t.z * invY + yOffset);
		}



		//=============================================//
		// compute borders of poly at a low resolution //
		//=============================================//

		NLMISC::CPolygon2D::TRasterVect rasters;		
		sint startY;
		projPoly.computeBorders(rasters, startY);

		if (rasters.size())
		{
			//===========================//
			// perform Water animation   //
			//===========================//			

			const float WaterRatio = whm.getUnitSize();
			const float invWaterRatio = 1.f / WaterRatio;
			const uint  WaterHeightMapSize = whm.getSize();
			const uint  doubleWaterHeightMapSize = (WaterHeightMapSize << 1);
						

			sint64 idate = getOwnerScene()->getHrcTrav().CurrentDate;


			
			if (idate != whm.Date)
			{
				whm.setUserPos((sint) (obsPos.x * invWaterRatio) - (WaterHeightMapSize >> 1),
					   (sint) (obsPos.y * invWaterRatio) - (WaterHeightMapSize >> 1)
					  );
				nlassert(getOwnerScene()); // this object should have been created from a CWaterShape!
				whm.animate((float) (getOwnerScene()->getEllapsedTime()));												
				whm.Date = idate;
			}
			
			//float startDate = (float) (1000.f * NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime()));

			//=====================================//
			//	compute heightmap useful area      //
			//=====================================//

			/** We don't store a heighmap for a complete Water area
			  * we just consider the height of Water columns that are near the observer
			  */
			/** Compute a quad in Water height field space that contains the useful heights
			  * This helps us to decide wether we should do a lookup in the height map
			  */

			sint mapPosX, mapPosY;

			/// get the pos used in the height map (may not be the same than our current pos, has it is taken in account
			/// every 'PropagationTime' second
 			whm.getUserPos(mapPosX, mapPosY);

			const uint mapBorder = 3;
			/*const sint qRight = (sint) mapPosX + (WaterHeightMapSize >> 1) - mapBorder;
				  sint qLeft  = (sint) mapPosX - (WaterHeightMapSize >> 1);
			const sint qUp    = (sint) mapPosY + (WaterHeightMapSize >> 1) - mapBorder;
				  sint qDown  = (sint) mapPosY - (WaterHeightMapSize >> 1); */


			const sint qRight = (sint) mapPosX + WaterHeightMapSize - mapBorder;
				  sint qLeft  = (sint) mapPosX;
			const sint qUp    = (sint) mapPosY + WaterHeightMapSize - mapBorder;
				  sint qDown  = (sint) mapPosY;

			/// Compute the origin of the area of Water covered by the height map. We use this to converted from object space to 2d map space
			const sint qSubLeft = qLeft - (uint)  qLeft % WaterHeightMapSize;
			const sint qSubDown = qDown - (uint)  qDown % WaterHeightMapSize;

			qLeft += mapBorder;
			qDown += mapBorder;

		

			
			//==============================================//
			// setup rays to be traced, and their increment //
			//==============================================//
			

			// compute  camera rays in world space
			NLMISC::CVector currHV = renderTrav.Left * camMatUp.getI() + renderTrav.Near * camMatUp.getJ() + renderTrav.Top * camMatUp.getK(); // current border vector, incremented at each line
			NLMISC::CVector currV; // current ray vector
			NLMISC::CVector xStep = (renderTrav.Right - renderTrav.Left) * invNumStepX * camMatUp.getI();	   // xStep for the ray vector
			NLMISC::CVector yStep = (renderTrav.Bottom - renderTrav.Top) * invNumStepY * camMatUp.getK();    // yStep for the ray vector
			


			//===============================================//
			//				perform display                  //
			//===============================================//

			// scale currHV at the top of the poly
			currHV += (startY - 0.5f  * isAbove) * yStep;

			// current index buffer used. We swap each time a row has been drawn
			std::vector<uint32> *currIB = &CWaterShape::_IBUpDown, *otherIB = &CWaterShape::_IBDownUp;

					
			sint vIndex = 0; // index in vertices	

			// current raster position
			sint oldStartX, oldEndX, realStartX, realEndX;	
			//float invNearWidth = numStepX / (renderTrav.Right - renderTrav.Left);

				//nlinfo("size = %d, maxSize = ", rasters.size(), numStepY);


			const uint wqHeight = rasters.size();
			if (wqHeight)
			{
				// denominator of the intersection equation
				const float denom = - obsPos.z + zHeight; 
				// test the upper raster
				// if it is above the horizon, we modify it to reach the correct location
				const float horizonEpsilon = 10E-4f; // we must be a little below the horizon

				// distance from the viewer along the traced ray
				float t;

				


				NLMISC::CPolygon2D::TRasterVect::const_iterator it = rasters.begin();
				for (uint l = 0; l <= wqHeight; ++l)
				{		
					//nlinfo("start = %d, end = %d", it->first, it->second);				
					const sint startX = it->first;
					const sint endX   = (it->second + 1);

					nlassert(startX >= - (sint) rotBorderSize);
					nlassert(endX  <= (sint) (numStepX + rotBorderSize));

					if (l != 0)
					{
						realStartX = std::min(startX, oldStartX);
						realEndX = std::max(endX, oldEndX);
					}
					else
					{
						realStartX = startX;
						realEndX =   endX;
					}

					
					// current view vector
					currV   = currHV + (realStartX - 0.5f) * xStep;
					
					if (l == 0)
					{
						if (isAbove)
						{
							// test wether the first row is out of horizon.
							// if this is the case, we make a correction
							if (denom * currV.z <= 0)
							{					
								// correct for the first line only by adding a y offset
								currV += yStep * ((denom > 0 ? horizonEpsilon : - horizonEpsilon)   - currV.z) / yStep.z;
							}

							// now, for the transition, check wether the first raster does not go over the transition dist
						
							t = denom / currV.z;
							const float VJ = camMat.getJ() * currV;
							if ( t * VJ >  transitionDist)
							{							
								float delta = (1.f / yStep.z) * ( denom * VJ / transitionDist - currV.z);
								// correct the first line to reach that position
								currV += delta * yStep;
							}
						}
					}

					
					uint8 *vbPointer = (uint8 *) shape->_VB.getVertexCoordPointer() + shape->_VB.getVertexSize() * (vIndex + realStartX + rotBorderSize);												
						

					for (sint k = realStartX; k <= realEndX; ++k)
					{	
						t =   denom / currV.z;						
						// compute intersection with plane											
						NLMISC::CVector inter = t * currV;
						inter.z += obsPos.z;
						SetupWaterVertex(qLeft, qRight, qUp, qDown, qSubLeft, qSubDown, inter, invWaterRatio, doubleWaterHeightMapSize, whm, vbPointer, obsPos.x, obsPos.y);
						currV += xStep;						
					}

					if (l != 0) // 2 line of the ib done ?
					{						
						sint count = oldEndX - oldStartX;
						if (count > 0)
						{												
							drv->renderSimpleTriangles(&((*currIB)[(oldStartX + rotBorderSize) * 6]),
												 2 * count );							
						}		
					}

					oldStartX = startX;
					oldEndX   = endX;
					currHV    += yStep;
					vIndex    = (numStepX + 2 * rotBorderSize + 1) - vIndex; // swap first row and second row
					std::swap(currIB, otherIB);
					if (l < (wqHeight - 1))
					{
 						++it;
					}
					else
					{
						if (!isAbove)
						{
							// last line
							// test wether we are out of horizon
							if (denom * currHV.z <= 0)
							{						
								// correct for the first line only by adding a y offset
								currHV += yStep * ((denom > 0 ? horizonEpsilon : - horizonEpsilon)  - currHV.z) / yStep.z;
							}

							// now, for the transition, check wether the first raster does not go over the transition dist
						
							t = denom / currHV.z;
							const float VJ = camMat.getJ() * currHV;
							if ( t * VJ >  transitionDist)
							{							
								float delta = (1.f / yStep.z) * ( denom * VJ / transitionDist - currHV.z);
								// correct the first line to reach that position
								currHV += delta * yStep;
							}
						}

					}
				}
					
			}			
			//nlinfo("display: %f ms", (float) (1000.f * NLMISC::CTime::ticksToSecond(NLMISC::CTime::getPerformanceTime()) - startDate));
		}
	}

	//=========================================//
	//			display end poly               //
	//=========================================//

	if (_EndClippedPoly.Vertices.size() != 0)
	{					

		CMatrix xform = _WorldMatrix;
		xform.movePos(NLMISC::CVector(- obsPos.x, - obsPos.y, _WorldMatrix.getPos().z));	
		DrawPoly2D(shape->_VB, drv, xform, _EndClippedPoly);
	}

	drv->endMaterialMultiPass();	


	drv->activeVertexProgram(NULL);
	
}

//***********************
// Water MATERIAL SETUP //
//***********************


void CWaterModel::setupMaterialNVertexShader(IDriver *drv, CWaterShape *shape, const NLMISC::CVector &obsPos, bool above, float maxDist, float zHeight)
{			
	static bool matSetupped = false;
	if (!matSetupped)
	{	
		_WaterMat.setLighting(false);
		_WaterMat.setDoubleSided(true);	
		_WaterMat.setColor(NLMISC::CRGBA::White);
		_WaterMat.setBlend(true);
		_WaterMat.setSrcBlend(CMaterial::srcalpha);
		_WaterMat.setDstBlend(CMaterial::invsrcalpha);
		_WaterMat.setZWrite(true);
		_WaterMat.setShader(CMaterial::Water);
	}
	
	
	const uint cstOffset = 4; // 4 places for the matrix
	NLMISC::CVectorH cst[13];
	
	
	//=========================//
	//	setup Water material   //
	//=========================//
		
	CWaterModel::_WaterMat.setTexture(0, shape->_BumpMap[0]);
	CWaterModel::_WaterMat.setTexture(1, shape->_BumpMap[1]);
	CWaterModel::_WaterMat.setTexture(3, shape->_ColorMap);

	if (!above && shape->_EnvMap[1])
	{
		CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[1]);				
	}
	else
	{
		CWaterModel::_WaterMat.setTexture(2, shape->_EnvMap[0]);
	}


	shape->envMapUpdate();

	const uint alphaMapStage = 3;
	if (shape->_ColorMap)
	{			
		//WaterMat.setTexture(alphaMapStage, shape->_ColorMap);
		//if (shape->_ColorMap->supportSharing()) nlinfo(shape->_ColorMap->getShareName().c_str());


		// setup 2x3 matrix for lookup in diffuse map
		float px = obsPos.x - getWorldMatrix().getPos().x;
		float py = obsPos.y - getWorldMatrix().getPos().y;

		cst[13 - cstOffset].set(shape->_ColorMapMatColumn0.x, shape->_ColorMapMatColumn1.x, 0, shape->_ColorMapMatColumn0.x * px + shape->_ColorMapMatColumn1.x * py + shape->_ColorMapMatPos.x); 
		cst[14 - cstOffset].set(shape->_ColorMapMatColumn0.y, shape->_ColorMapMatColumn1.y, 0, shape->_ColorMapMatColumn0.y * px + shape->_ColorMapMatColumn1.y * py + shape->_ColorMapMatPos.y);						
	}
	else
	{
		cst[13 - cstOffset].set(0, 0, 0, 0);
		cst[14 - cstOffset].set(0, 0, 0, 0);			
	}

	cst[16 - cstOffset].set(0.1f, 0.1f, 0.1f, 0.1f); // used to avoid imprecision when performing a RSQ to get distance from the origin
	// cst[16 - cstOffset].set(0.0f, 0.0f, 0.0f, 0.0f); // used to avoid imprecision when performing a RSQ to get distance from the origin

				


	cst[5  - cstOffset].set(0.f, 0.f, 0.f, 0.f); // claping negative values to 0

	// slope of attenuation of normal / height with distance		
	const float invMaxDist = shape->_WaveHeightFactor / maxDist;
	cst[6  - cstOffset].set(invMaxDist, shape->_WaveHeightFactor, 0, 0);		

	/*cst[6  - cstOffset].set(invMaxDist, invMaxDist, invMaxDist, invMaxDist); // upcoming light vectorshape->_WaveHeightFactor		
	cst[15  - cstOffset].set(shape->_WaveHeightFactor, shape->_WaveHeightFactor, shape->_WaveHeightFactor, shape->_WaveHeightFactor);
	*/


			

	/// set matrix		
	drv->setConstantMatrix(0, IDriver::ModelViewProjection, IDriver::Identity);

	// retrieve current time
	float date  = 0.001f * (NLMISC::CTime::getLocalTime() & 0xffffff); // must keep some precision.
	// set bumpmaps pos
	cst[9  - cstOffset].set(fmodf(obsPos.x * shape->_HeightMapScale[0].x, 1.f) + fmodf(date * shape->_HeightMapSpeed[0].x, 1.f), fmodf(shape->_HeightMapScale[0].y * obsPos.y, 1.f) + fmodf(date * shape->_HeightMapSpeed[0].y, 1.f), 0.f, 1.f); // bump map 0 offset
	cst[10  - cstOffset].set(shape->_HeightMapScale[0].x, shape->_HeightMapScale[0].y, 0, 1); // bump map 0 scale
	cst[11  - cstOffset].set(fmodf(shape->_HeightMapScale[1].x * obsPos.x, 1.f) + fmodf(date * shape->_HeightMapSpeed[1].x, 1.f), fmodf(shape->_HeightMapScale[1].y * obsPos.y, 1.f) + fmodf(date * shape->_HeightMapSpeed[1].y, 1.f), 0.f, 0.f); // bump map 1 offset
	cst[12  - cstOffset].set(shape->_HeightMapScale[1].x, shape->_HeightMapScale[1].y, 0, 1); // bump map 1 scale

			
	

	cst[4  - cstOffset].set(1.f, 1.f, 1.f, 1.f); // use with min man, and to get the 1 constant		
	cst[7  - cstOffset].set(0, 0, obsPos.z - zHeight, 1.f);
	cst[8  - cstOffset].set(0.5f, 0.5f, 0.f, 0.f); // used to scale reflected ray into the envmap


	

	/// set all our constants in one call
	drv->setConstant(4, sizeof(cst) / sizeof(cst[0]), (float *) &cst[0]);

	shape->initVertexProgram();		
	bool result;
	/*
	if (useBumpedVersion)
	{
		if (!useEMBM)
		{			
			result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramBump2Diffuse).get())
											: drv->activeVertexProgram((shape->_VertexProgramBump2).get());
		}
		else
		{
			result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramBump1Diffuse).get())
											: drv->activeVertexProgram((shape->_VertexProgramBump1).get());
		}
	}
	else
	{
		result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramNoBumpDiffuse).get())
									: drv->activeVertexProgram((shape->_VertexProgramNoBump).get());
	}
	*/
	
	

	result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramBump2Diffuse).get())
											: drv->activeVertexProgram((shape->_VertexProgramBump2).get());
			
	//
	if (!result) nlwarning("no vertex program setupped");							
}

//=======================================================================================
//							wave maker implementation
//=======================================================================================


CWaveMakerModel::CWaveMakerModel() : _Time(0)
{
	// AnimDetail behavior: Must be traversed in AnimDetail, even if no channel mixer registered
	CTransform::setIsForceAnimDetail(true);
}

//================================================

void CWaveMakerModel::registerBasic()
{
	CScene::registerModel(WaveMakerModelClassId, TransformShapeId, CWaveMakerModel::creator);
}

//================================================

ITrack* CWaveMakerModel::getDefaultTrack (uint valueId)
{
	nlassert(Shape);
	CWaveMakerShape *ws = NLMISC::safe_cast<CWaveMakerShape *>((IShape *) Shape);
	switch (valueId)
	{
		case PosValue:			return ws->getDefaultPos(); break;	
		default: // delegate to parent
			return CTransformShape::getDefaultTrack(valueId);
		break;
	}
}

//================================================
void	CWaveMakerModel::traverseAnimDetail()
{
	CTransformShape::traverseAnimDetail();
	nlassert(getOwnerScene());
	/// get the shape
	CWaveMakerShape *wms = NLMISC::safe_cast<CWaveMakerShape *>((IShape *) Shape);
	const NLMISC::CVector	worldPos = getWorldMatrix().getPos();
	const NLMISC::CVector2f pos2d(worldPos.x, worldPos.y);
	/// get the water height map
	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(wms->_PoolID);
	// get the time delta 
	const TAnimationTime deltaT  = std::min(getOwnerScene()->getEllapsedTime(), (TAnimationTime) whm.getPropagationTime());
	_Time += deltaT;
	if (!wms->_ImpulsionMode)
	{
		whm.perturbate(pos2d, wms->_Intensity * cosf(2.f / wms->_Period * (float) NLMISC::Pi * _Time), wms->_Radius);
	}
	else
	{
		if (_Time > wms->_Period)
		{
			_Time -= wms->_Period;
			whm.perturbate(pos2d, wms->_Intensity, wms->_Radius);
		}
	}
}

//================================================
void CWaterModel::computeSimpleClippedPoly()
{	
	CWaterShape	*shape = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	CClipTrav	&clipTrav		= getOwnerScene()->getClipTrav();
	const std::vector<CPlane>	&worldPyramid   = getOwnerScene()->getClipTrav().WorldFrustumPyramid;	
	
	_ClippedPoly.Vertices.resize(shape->_Poly.Vertices.size());
	uint k;
	for (k = 0; k < shape->_Poly.Vertices.size(); ++k)
	{
		_ClippedPoly.Vertices[k].set(shape->_Poly.Vertices[k].x, 
									shape->_Poly.Vertices[k].y,
									0
								   );
	}
	
	static std::vector<CPlane> xformPyram;
	const uint numPlanes = (uint) worldPyramid.size();
	xformPyram.resize(numPlanes);	
	for (k = 0; k < numPlanes; ++k)
	{
		xformPyram[k] = worldPyramid[k] * getWorldMatrix(); // put the plane in object space
	}
	// do the clip
	_ClippedPoly.clip(xformPyram);	
}

//================================================
void CWaterModel::computeClippedPoly()
{
	CWaterShape	*shape = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	CClipTrav	&clipTrav		= getOwnerScene()->getClipTrav();
	const std::vector<CPlane>	&worldPyramid   = getOwnerScene()->getClipTrav().WorldFrustumPyramid;	
	
	_ClippedPoly.Vertices.resize(shape->_Poly.Vertices.size());
	uint k;
	for (k = 0; k < shape->_Poly.Vertices.size(); ++k)
	{
		_ClippedPoly.Vertices[k].set(shape->_Poly.Vertices[k].x, 
									shape->_Poly.Vertices[k].y,
									0
								   );
	}
	
	_EndClippedPoly = _ClippedPoly;

	const sint numStepX = CWaterShape::getScreenXGridSize();
	const sint numStepY = CWaterShape::getScreenYGridSize();
	const NLMISC::CMatrix &viewMat = clipTrav.ViewMatrix;

	// Build the view pyramid. We need to rebuild it because we use a wider one to avoid holes on the border of the screen

	NLMISC::CPlane plvect[6];

	const float borderFactor = 0.67f; // we must avoid numerical imprecision as well as the rotation case (must divide by sqrt(2))
	const float fRight = clipTrav.Right * (2.f * borderFactor * (float) CWaterShape::_XGridBorder +  (float) numStepX) / numStepX;
	const float fTop   = clipTrav.Top   * (2.f * borderFactor * (float) CWaterShape::_YGridBorder +  (float) numStepY) / numStepY;

	// build pyramid corners
	const float nearDist	    = clipTrav.Near;
	const float farDist			= clipTrav.Far;	
	const float transitionDist	= shape->_TransitionRatio   * farDist;

	const NLMISC::CVector		pfoc(0,0,0);
	const NLMISC::CVector		lb(-fRight,  nearDist, - fTop );
	const NLMISC::CVector		lt(-fRight,  nearDist, fTop  );
	const NLMISC::CVector		rb( fRight,  nearDist, -fTop );
	const NLMISC::CVector		rt(fRight,	nearDist, fTop  );

	const NLMISC::CVector		lbfarDist(-fRight, transitionDist, -fTop);
	const NLMISC::CVector		ltfarDist(-fRight, transitionDist, fTop );
	const NLMISC::CVector		rtfarDist(fRight , transitionDist, fTop  );

	
	plvect[0].make(lt, lb, rt);							// near plane
	plvect[1].make(lbfarDist, ltfarDist, rtfarDist);    // far plane

	plvect[2].make(pfoc, lt, lb);
	plvect[3].make(pfoc, rt, lt);
	plvect[4].make(pfoc, rb, rt);
	plvect[5].make(pfoc, lb, rb);

		
	const NLMISC::CMatrix pyramidMat = viewMat * getWorldMatrix();
	for (k = 0; k < worldPyramid.size(); ++k)
	{
		plvect[k] = plvect[k] * pyramidMat; // put the plane in object space
	}

	_ClippedPoly.clip(plvect, 6);	// get the tesselated part of the poly 

	// modify the pyramid to get the transition part of the poly (no tesselated)
	plvect[0].d   += (transitionDist - nearDist);
	plvect[1].d   -= (farDist - transitionDist);	
	_EndClippedPoly.clip(plvect, 6);	
}


//***********************************************************************************************************
bool CWaterModel::clip()
{
	H_AUTO( NL3D_Water_Render );
	CClipTrav			&clipTrav= getOwnerScene()->getClipTrav();

	if(Shape)
	{
		IDriver *drv = getOwnerScene()->getDriver();
		
		#ifndef FORCE_SIMPLE_WATER_RENDER
			if (drv->isWaterShaderSupported())
			{		
				computeClippedPoly();
				if (_ClippedPoly.Vertices.empty() && _EndClippedPoly.Vertices.empty()) return false;
			}
			else
		#endif
			{

				computeSimpleClippedPoly();
				if (_ClippedPoly.Vertices.empty()) 
				{
					return false;
				}
			}

		return true;
	}
	else
		return false; 
}

// struct used to build vertices for the simple shader
struct CSimpleVertexInfo
{
	NLMISC::CVector XFormPos;
	NLMISC::CUV     UV;
};

//***********************************************************************************************************
void CWaterModel::doSimpleRender(IDriver *drv)
{
	if (_ClippedPoly.Vertices.empty()) return;
	// rendering of water when no vertex / pixel shaders are available
	CWaterShape	*shape = NLMISC::safe_cast<CWaterShape *>((IShape *) Shape);
	CRenderTrav	&renderTrav		= getOwnerScene()->getRenderTrav();
	static bool init = false;
	if (!init)
	{
		// setup the material, no special shader is used here
		_SimpleWaterMat.setLighting(false);
		_SimpleWaterMat.setDoubleSided(true);	
		_SimpleWaterMat.setColor(NLMISC::CRGBA::White);

		_SimpleWaterMat.setBlend(true);
		_SimpleWaterMat.setSrcBlend(CMaterial::srcalpha);
		_SimpleWaterMat.setDstBlend(CMaterial::invsrcalpha);
		_SimpleWaterMat.setZWrite(true);
		_SimpleWaterMat.setShader(CMaterial::Normal);

		// stage 0
		_SimpleWaterMat.texEnvOpRGB(0, CMaterial::Replace);
		_SimpleWaterMat.texEnvOpAlpha(0, CMaterial::Replace);
		_SimpleWaterMat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);

		// stage 1
		_SimpleWaterMat.texEnvOpRGB(1, CMaterial::Modulate);
		_SimpleWaterMat.texEnvOpAlpha(1, CMaterial::Modulate);
		_SimpleWaterMat.texEnvArg0RGB(0, CMaterial::Texture, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg0Alpha(0, CMaterial::Texture, CMaterial::SrcAlpha);
		_SimpleWaterMat.texEnvArg1RGB(0, CMaterial::Previous, CMaterial::SrcColor);
		_SimpleWaterMat.texEnvArg1Alpha(0, CMaterial::Previous, CMaterial::SrcAlpha);

		// setup the vb : one position & two tex coords
		_SimpleRenderVB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag | CVertexBuffer::TexCoord1Flag);		
		init = true;
	}	
	
	const NLMISC::CMatrix &worldMatrix = getWorldMatrix();
	const NLMISC::CVector &obsPos = renderTrav.CamPos;

	// setup the material
	bool isAbove = obsPos.z > worldMatrix.getPos().z;	

	// envmap is always present and is in stage 0
	if (!isAbove && shape->_EnvMap[1])
	{
		_SimpleWaterMat.setTexture(0, shape->_EnvMap[1]);				
	}
	else
	{
		_SimpleWaterMat.setTexture(0, shape->_EnvMap[0]);
	}
	//	
	static std::vector<CSimpleVertexInfo> verts;	
	static std::vector<uint32> indices;
	//
	NLMISC::CPolygon2D &poly = shape->_Poly;
	uint numVerts = poly.Vertices.size();
	uint k;
	//
	if (shape->_ColorMap == NULL)
	{
		// version with no color map
		if (!_EmbossTexture)
		{
			_EmbossTexture = new CTextureEmboss;
			_EmbossTexture->setSlopeFactor(4.f);			
			/*te->setAmbient(CRGBA(127, 127, 127));
			te->setDiffuse(CRGBA(127, 127, 127)); */
		}
		if (shape->_BumpMap[1] && shape->_BumpMap[1]->isBumpMap())
		{
			CTextureBump *bm = static_cast<CTextureBump *>((ITexture *) shape->_BumpMap[1]);
			if (bm->getHeightMap())
			{		
				_EmbossTexture->setHeightMap(bm->getHeightMap());
			}
		}
		_SimpleWaterMat.setTexture(1, _EmbossTexture);		
		_SimpleRenderVB.setNumVertices(numVerts);
		// retrieve current time
		float date  = 0.001f * (NLMISC::CTime::getLocalTime() & 0xffffff); // must keep some precision.
		/** Compute tex coordinates for emboss first.
		  * On some 3D chip, textures coords can't grow too mush or texture filtering loose accuracy.
		  * So we must keep texCoord as low as possible.
		  */			
		verts.resize(numVerts);		
		for(k = 0; k < numVerts; ++k)
		{
			verts[k].XFormPos = worldMatrix * NLMISC::CVector(poly.Vertices[k].x, poly.Vertices[k].y ,0.f);
			verts[k].UV.U = shape->_HeightMapScale[0].x * verts[k].XFormPos.x + date * shape->_HeightMapSpeed[0].x;
			verts[k].UV.V = shape->_HeightMapScale[0].y * verts[k].XFormPos.y + date * shape->_HeightMapSpeed[0].y;
		}		
		// get min tex coords
		float minU = verts[0].UV.U;
		float minV = verts[0].UV.V;
		for(k = 1; k < numVerts; ++k)
		{
			minU = std::min(minU, verts[k].UV.U);
			minV = std::min(minV, verts[k].UV.V);
		}
		//
		minU = floorf(minU);
		minV = floorf(minV);		
		//
		uint8 *data = (uint8 *) _SimpleRenderVB.getVertexCoordPointer();
		for(k = 0; k < numVerts; ++k)
		{
			((NLMISC::CVector *) data)->set(poly.Vertices[k].x, poly.Vertices[k].y, 0.f);			
			data += sizeof(NLMISC::CVector);
			// texture coord 0 is reflected vector into envmap
			// xform position in world space to compute the reflection			
			CVector surfToEye = (obsPos - verts[k].XFormPos).normed();
			// we assume that normal is (0, 0, 1)						
			* (float *) data = 0.5f - 0.5f * surfToEye.x;
			((float *) data)[1] = 0.5f  - 0.5f * surfToEye.y;
			data += sizeof(float[2]);
			// texture coord 1 is the embossed map
			* (float *) data = verts[k].UV.U - minU;
			((float *) data)[1] = verts[k].UV.V - minV;
			data += sizeof(float[2]);			
		}		
	}
	else
	{
		// version with a color map : it remplace the emboss texture						
		_SimpleWaterMat.setTexture(1, shape->_ColorMap);		
		_SimpleRenderVB.setNumVertices(numVerts);
		//
		uint8 *data = (uint8 *) _SimpleRenderVB.getVertexCoordPointer();
		for(k = 0; k < numVerts; ++k)
		{
			* (NLMISC::CVector *) data = poly.Vertices[k];
			data += sizeof(CVector);
			// texture coord 0 is reflected vector into envmap
			// xform position in world space to compute the reflection
			NLMISC::CVector xformPos = worldMatrix * poly.Vertices[k];
			NLMISC::CVector surfToEye = (obsPos - xformPos).normed();
			// we assume that normal is (0, 0, 1)						
			* (float *) data = 0.5f - 0.5f * surfToEye.x;
			((float *) data)[1] = 0.5f * - 0.5f * surfToEye.y;
			data += sizeof(float[2]);
			// texture coord 1 is the color map
			* (float *) data = shape->_ColorMapMatColumn0.x * xformPos.x + shape->_ColorMapMatColumn1.x * xformPos.y + shape->_ColorMapMatPos.x;
			((float *) data)[1] = shape->_ColorMapMatColumn0.y * xformPos.x + shape->_ColorMapMatColumn1.y * xformPos.y + shape->_ColorMapMatPos.y;
			data += sizeof(float[2]);			
		}
	}
			
	drv->activeVertexProgram(NULL);	
	drv->setupModelMatrix(worldMatrix);	
	drv->activeVertexBuffer(_SimpleRenderVB);		
	
	// create an index buffer to do the display	
	indices.resize((numVerts - 2) * 3);	
	for(k = 0; k < (numVerts - 2); ++k)
	{
		
		indices[ k * 3      ] = 0;
		indices[ k * 3  + 1 ] = k + 1;
		indices[ k * 3  + 2 ] = k + 2;		
	}	
	drv->setupMaterial(_SimpleWaterMat);
	drv->renderSimpleTriangles(&indices[0], numVerts - 2);	
}


} // NL3D
