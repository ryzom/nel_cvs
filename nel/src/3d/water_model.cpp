/** \file water_model.cpp
 * <File description>
 *
 * $Id: water_model.cpp,v 1.9 2001/11/16 16:46:12 vizerie Exp $
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

#include "3d/water_model.h"
#include "3d/water_shape.h"
#include "3d/water_pool_manager.h"
#include "3d/water_height_map.h"
#include "3d/dru.h"
#include "3d/scene.h"
#include "nel/misc/vector_2d.h"




namespace NL3D {


CWaterModel::CWaterModel() : _Scene(NULL)
{
	setOpacity(false);
	setTransparency(true);
}


void CWaterModel::registerBasic()
{
	CMOT::registerModel(WaterModelClassId, TransformShapeId, CWaterModel::creator);	
	CMOT::registerObs(RenderTravId, WaterModelClassId, CWaterRenderObs::creator);	
}


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



//***************************************************************************************************************


/// this inline function setup one water vertex
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
							   CVertexBuffer &vb,
							   float invWaterRatio,
							   uint  vbIndex,
							   sint  doubleWaterHeightMapSize,
							   CWaterHeightMap &whm
							   )
{
	const float wXf = invWaterRatio * inter.x;
	const float wYf = invWaterRatio * inter.y;

	sint wx = (sint) floorf(wXf);
	sint wy = (sint) floorf(wYf);

	

	if (!
		 (wx >= qLeft && wx < qRight && wy < qUp &&  wy >= qDown)
	   )
	{	
		// no perturbation is visible
		vb.setValueFloat3Ex (WATER_VB_POS, vbIndex, inter.x, inter.y, 0);
		vb.setValueFloat2Ex (WATER_VB_DX, vbIndex, 0, 0);
	}
	else
	{
		// filter height and gradient at the given point
		const sint stride = doubleWaterHeightMapSize;

		float deltaU = wXf - wx;
		float deltaV = wYf - wy;
		nlassert(deltaU >= 0.f && deltaU <= 1.f  && deltaV >= 0.f && deltaV <= 1.f);
		const uint xm	  = (uint) (wx - qSubLeft);
		const uint ym	  = (uint) (wy - qSubDown);
		const sint offset = xm + stride * ym;
		float			  *ptWater = whm.getPointer()	  + offset;
		NLMISC::CVector2f *ptGrad  = whm.getGradPointer() + offset;

		float dh1 = deltaV * ptWater[stride] + (1.f - deltaV) *  ptWater[0];
		float dh2 = deltaV * ptWater[stride + 1] + (1.f - deltaV) *  ptWater[1];
		float h = deltaU * dh2 + (1.f - deltaU ) * dh1;

		
		float gR = deltaV * ptGrad[stride + 1].x + (1.f - deltaV) * ptGrad[1].x;
		float gL = deltaV * ptGrad[stride].x + (1.f - deltaV) * ptGrad[0].x;

		float grU = 4.5f * (deltaU *  gR + (1.f - deltaU) * gL);

		gR = deltaV * ptGrad[stride + 1].y + (1.f - deltaV) * ptGrad[1].y;
		gL = deltaV * ptGrad[stride].y + (1.f - deltaV) * ptGrad[0].y;

		float grV = 4.5f * (deltaU *  gR + (1.f - deltaU) * gL);

		vb.setValueFloat3Ex (WATER_VB_POS, vbIndex, inter.x, inter.y, h);
		vb.setValueFloat2Ex (WATER_VB_DX, vbIndex, grU, grV);
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

//***************************************************************************************************************

void	CWaterRenderObs::traverse(IObs *caller)
{		

	CRenderTrav					*trav			= NLMISC::safe_cast<CRenderTrav *>(Trav);
	CWaterModel					*m				= NLMISC::safe_cast<CWaterModel *>(Model);
	CWaterShape					*shape			= NLMISC::safe_cast<CWaterShape *>((IShape *) m->Shape);
	IDriver						*drv			= trav->getDriver();
	const std::vector<CPlane>	&worldPyramid   = ((NLMISC::safe_cast<CClipTrav *>(ClipObs->Trav))->WorldFrustumPyramid);	

	if (shape->_GridSizeTouched)
	{
		shape->setupVertexBuffer();
	}


	// inverted object world matrix
	//NLMISC::CMatrix invObjMat = HrcObs->WorldMatrix.inverted();

	// viewer pos in world space
	const NLMISC::CVector &obsPos = /*invObjMat **/ trav->CamPos;

	// camera matrix in world space
	const NLMISC::CMatrix &camMat = trav->CamMatrix;

	// view matrix (inverted cam matrix)
	const NLMISC::CMatrix &viewMat = trav->ViewMatrix;

	// compute the camera matrix such as there is no rotation around the y axis
	NLMISC::CMatrix camMatUp;
	ComputeUpMatrix(camMat.getJ(), camMatUp, camMat);
	camMatUp.setPos(camMat.getPos());

	const NLMISC::CMatrix matViewUp = camMatUp.inverted();

	// compute the y rot matrix needed to put the grid in the right orientation
	const NLMISC::CMatrix rotYMat = camMat * matViewUp;


	// compute right and top after the rotation around y



	// plane z pos in world
	const float zHeight =  HrcObs->WorldMatrix.getPos().z;

	const sint numStepX = CWaterShape::getScreenXGridSize();
	const sint numStepY = CWaterShape::getScreenYGridSize();

	const float invNumStepX = 1.f / numStepX;
	const float invNumStepY = 1.f / numStepY;

	const uint rotBorderSize = (shape->_MaxGridSize + (shape->_XGridBorder << 1) - numStepX) >> 1;
	
	const sint isAbove = obsPos.z > zHeight ? 1 : 0;

		

	//==================//
	// polygon clipping //
	//==================//

	uint k;
	static NLMISC::CPolygon clippedPoly, endClippedPoly;	
	clippedPoly.Vertices.resize(shape->_Poly.Vertices.size());
	for (k = 0; k < shape->_Poly.Vertices.size(); ++k)
	{
		clippedPoly.Vertices[k].set(shape->_Poly.Vertices[k].x, 
									shape->_Poly.Vertices[k].y,
									0
								   );
	}
	
	endClippedPoly = clippedPoly;


	// Build the view pyramid. We need to rebuild it because we use a wider one to avoid holes on the border of the screen

	static NLMISC::CPlane plvect[6];

	const float borderFactor = 0.67f; // we must avoid numerical imprecision as well as the rotation case (must divide by sqrt(2))
	const float fRight = trav->Right * (2.f * borderFactor * (float) CWaterShape::_XGridBorder +  (float) numStepX) / numStepX;
	const float fTop   = trav->Top   * (2.f * borderFactor * (float) CWaterShape::_YGridBorder +  (float) numStepY) / numStepY;

	// build pyramid corners
	const float nearDist	    = trav->Near;
	const float farDist			= trav->Far;	
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

		
	const NLMISC::CMatrix pyramidMat = viewMat * HrcObs->WorldMatrix;
	for (k = 0; k < worldPyramid.size(); ++k)
	{
		plvect[k] = plvect[k] * pyramidMat; // put the plane in object space
	}

	clippedPoly.clip(plvect, 6);	// get the tesselated part of the poly (with alpah transition)

	// modify the pyramid to get the transition part of the poly (no tesselated, alpha transition)	
	plvect[0].d   += (transitionDist - nearDist);
	plvect[1].d   -= (farDist - transitionDist);
	endClippedPoly.clip(plvect, 6);



	/// anything to show ?
	if (clippedPoly.Vertices.size() == 0 && endClippedPoly.Vertices.size() == 0)
	{
		return;	
	}



	
	NLMISC::CMatrix modelMat;
	modelMat.setPos(NLMISC::CVector(0, 0, zHeight));
	drv->setupModelMatrix(modelMat);
	

	//==================//
	// material setup   //
	//==================//

	CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(shape->_WaterPoolID);
	setupMaterialNVertexShader(drv, shape, obsPos, isAbove > 0, whm.getUnitSize() * (whm.getSize() >> 1), zHeight);

	//setAttenuationFactor(drv, false, obsPos, camMat.getJ(), farDist);
	//disableAttenuation(drv);

	
	//================================//
	//	Vertex buffer setup           //
	//================================//
		
	drv->activeVertexBuffer(shape->_VB);

	//================================//
	//	tesselated part of the poly   //
	//================================//

	if (clippedPoly.Vertices.size())
	{
		
		//======================================//
		// Polygon projection on the near plane //
		//======================================//

		static NLMISC::CPolygon2D projPoly; // projected poly
		projPoly.Vertices.resize(clippedPoly.Vertices.size());
		const float Near = trav->Near;
		const float xFactor = (numStepX >> 1) * Near  / trav->Right;
		const float xOffset = (float) (numStepX >> 1) + 0.5f;
		const float yFactor = - (numStepX >> 1) * Near  / trav->Top;
		const float yOffset = (float) (numStepY >> 1) - 0.5f * isAbove;
		
		const NLMISC::CMatrix projMat =  matViewUp * HrcObs->WorldMatrix;
		for (k = 0; k < clippedPoly.Vertices.size(); ++k)
		{
			// project points in the view
			NLMISC::CVector t = projMat * clippedPoly.Vertices[k];
			float invY = 1.f / t.y;
			projPoly.Vertices[k].set(xFactor * t.x * invY + xOffset, yFactor * t.z * invY + yOffset);
			//nlinfo(" x = %f, y = %f", projPoly.Vertices[k].x, projPoly.Vertices[k].y);
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
			// perform water animation   //
			//===========================//			

			const float waterRatio = whm.getUnitSize();
			const float invWaterRatio = 1.f / waterRatio;
			const uint  waterHeightMapSize = whm.getSize();
			const uint  doubleWaterHeightMapSize = (waterHeightMapSize << 1);
						

			sint64 idate = (NLMISC::safe_cast<CHrcTrav *>(HrcObs->Trav))->CurrentDate;
			if (idate != whm.Date)
			{
				whm.setUserPos((sint) (obsPos.x * invWaterRatio) - (waterHeightMapSize >> 1),
					   (sint) (obsPos.y * invWaterRatio) - (waterHeightMapSize >> 1)
					  );
				nlassert(m->_Scene); // this object should have been created from a CWaterShape!
				whm.swapBuffers((float) (m->_Scene->getEllapsedTime()));
				whm.propagate();
				whm.filterNStoreGradient();
				whm.Date = idate;
			}

			//=====================================//
			//	compute heightmap useful area      //
			//=====================================//

			/** We don't store a heighmap for a complete water area
			  * we just consider the height of water columns that are near the observer
			  */
			/** Compute a quad in water height field space that contains the useful heights
			  * This helps us to decide wether we should do a lookup in the height map
			  */
			const sint qRight = (sint) (obsPos.x * invWaterRatio) + (waterHeightMapSize >> 1) - 2;
				  sint qLeft  = (sint) (obsPos.x * invWaterRatio) - (waterHeightMapSize >> 1);
			const sint qUp    = (sint) (obsPos.y * invWaterRatio) + (waterHeightMapSize >> 1) - 2;
				  sint qDown  = (sint) (obsPos.y * invWaterRatio) - (waterHeightMapSize >> 1);

			/// Compute the origin of the area of water covered by the height map. We use this to converted from object space to 2d map space
			const sint qSubLeft = qLeft - (uint)  qLeft % waterHeightMapSize;
			const sint qSubDown = qDown - (uint)  qDown % waterHeightMapSize;

			qLeft += 2;
			qDown += 2;

		

			
			//==============================================//
			// setup rays to be traced, and their increment //
			//==============================================//
			

			// compute  camera rays in world space
			CVector currHV = trav->Left * camMatUp.getI() + trav->Near * camMatUp.getJ() + trav->Top * camMatUp.getK(); // current border vector, incremented at each line
			CVector currV; // current ray vector
			CVector xStep = (trav->Right - trav->Left) * invNumStepX * camMatUp.getI();	   // xStep for the ray vector
			CVector yStep = (trav->Bottom - trav->Top) * invNumStepY * camMatUp.getK();    // yStep for the ray vector
			


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
			//float invNearWidth = numStepX / (trav->Right - trav->Left);

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

					
					sint index = vIndex + realStartX + rotBorderSize;												

						

					for (sint k = realStartX; k <= realEndX; ++k)
					{	
						t =   denom / currV.z;
						
		
						// compute intersection with plane			
						
		//				nlassert(t >= 0);
						CVector inter = obsPos + t * currV;		
						SetupWaterVertex(qLeft, qRight, qUp, qDown, qSubLeft, qSubDown, inter, shape->_VB, invWaterRatio, index, doubleWaterHeightMapSize, whm);

					
								
						currV += xStep;
						++index;
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
		}
	}

	//=========================================//
	//			display end poly               //
	//=========================================//

	if (endClippedPoly.Vertices.size() != 0)
	{				
		//setAttenuationFactor(drv, true, obsPos, camMat.getJ(), farDist);				
		DrawPoly2D(shape->_VB, drv, HrcObs->WorldMatrix, endClippedPoly);
	}

	/*if (endTransitionClippedPoly.Vertices.size() != 0)
	{				
		disableAttenuation(drv);				
		DrawPoly2D(shape->_VB, drv, HrcObs->WorldMatrix, endTransitionClippedPoly);
	}*/


	if (drv->isVertexProgramSupported())
	{
		bool result =  drv->activeVertexProgram(NULL);
		if (!result) nlwarning("no vertex program setupped");
	}
	

	this->traverseSons();
}

//***********************
// WATER MATERIAL SEUP //
//***********************

void CWaterRenderObs::setupMaterialNVertexShader(IDriver *drv, CWaterShape *shape, const NLMISC::CVector &obsPos, bool above, float maxDist, float zHeight)
{
	CMaterial waterMat;
		waterMat.setLighting(false);
		waterMat.setDoubleSided(true);	
		waterMat.setColor(NLMISC::CRGBA::White);
	
		waterMat.setBlend(true);
		waterMat.setSrcBlend(CMaterial::srcalpha);
		waterMat.setDstBlend(CMaterial::invsrcalpha);
		waterMat.setZWrite(false);
		if (drv->isVertexProgramSupported())
		{

		//=========================//
		//	setup water material   //
		//=========================//
	
		uint alphaMapStage, envMapStage;	
		waterMat.setColor(NLMISC::CRGBA::White);				
		if (drv->getNbTextureStages() < 4)
		{						
			waterMat.texEnvOpRGB(0, CMaterial::Modulate);
			alphaMapStage = 1;
			envMapStage  = 0;
		}
		else
		{
			// setup bump proj matrix
			static const float idMat[] = {0.25f, 0, 0, 0.25f};	
			drv->setMatrix2DForTextureOffsetAddrMode(0, idMat);
			drv->setMatrix2DForTextureOffsetAddrMode(1, idMat);

			waterMat.setTexture(0, shape->_BumpMap[0]);
			waterMat.setTexture(1, shape->_BumpMap[1]);			
			waterMat.texEnvOpRGB(2, CMaterial::Replace);
			/*waterMat.texEnvOpRGB(1, CMaterial::Replace);
			waterMat.texEnvOpRGB(1, CMaterial::Replace);*/
			waterMat.enableTexAddrMode();
			waterMat.setTexAddressingMode(0, CMaterial::FetchTexture);		
			waterMat.setTexAddressingMode(1, CMaterial::OffsetTexture);
			waterMat.setTexAddressingMode(2, CMaterial::OffsetTexture);
			waterMat.setTexAddressingMode(3, shape->_ColorMap ? CMaterial::FetchTexture : CMaterial::TextureOff);
			alphaMapStage = 3;
			envMapStage   = 2;
		}
		

		if (!above && shape->_EnvMap[1])
		{
			waterMat.setTexture(envMapStage, shape->_EnvMap[1]);	
		}
		else
		{
			waterMat.setTexture(envMapStage, shape->_EnvMap[0]);
		}
		shape->envMapUpdate();

		
		if (shape->_ColorMap)
		{			
			waterMat.setTexture(alphaMapStage, shape->_ColorMap);

			// setup 2x3 matrix for lookup in diffuse map
			drv->setConstant(15, shape->_ColorMapMatColumn0.x, shape->_ColorMapMatColumn1.x, 0, shape->_ColorMapMatPos.x); 
			drv->setConstant(16, shape->_ColorMapMatColumn0.y, shape->_ColorMapMatColumn1.y, 0, shape->_ColorMapMatPos.y);
			drv->setConstant(17, 0.1f, 0.1f, 0.1f, 0.1f); // used to avoid imprecision when performing a RSQ to get distance from the origin
			waterMat.texEnvOpRGB(alphaMapStage, CMaterial::Modulate);
			waterMat.texEnvOpAlpha(alphaMapStage, CMaterial::Modulate);
		}

		
		
		//================================//
		// setup vertex program contants  //
		//================================//


		drv->setConstant(5, 0.f, 0.f, 0.f, 0.f); // claping negative values to 0

		// slope of attenuation of normal / height with distance		
		const float invMaxDist = shape->_WaveHeightFactor / maxDist;
		drv->setConstant(6, invMaxDist, invMaxDist, invMaxDist, invMaxDist); // upcoming light vector
		drv->setConstant(17, shape->_WaveHeightFactor, shape->_WaveHeightFactor, shape->_WaveHeightFactor, shape->_WaveHeightFactor);

				

		/// set bumpmap matrix		
		drv->setConstantMatrix(0, IDriver::ModelViewProjection, IDriver::Identity);

		// retrieve current time
		float date  = 0.001f * NLMISC::CTime::getLocalTime();
		// set bumpmaps pos
		drv->setConstant(11, date * shape->_HeightMapSpeed[0].x, date * shape->_HeightMapSpeed[0].y, 0.f, 0.f); // bump map 0 offset
		drv->setConstant(12, shape->_HeightMapScale[0].x, shape->_HeightMapScale[0].y, 0, 0); // bump map 0 scale
		drv->setConstant(13, date * shape->_HeightMapSpeed[1].x, date * shape->_HeightMapSpeed[0].y, 0.f, 0.f); // bump map 1 offset
		drv->setConstant(14, shape->_HeightMapScale[1].x, shape->_HeightMapScale[1].y, 0, 0); // bump map 1 scale

				
			
		


		drv->setConstant(4, 1.f, 1.f, 1.f, 1.f); // use with min man, and to get the 1 constant		
		drv->setConstant(7, obsPos.x, obsPos.y, obsPos.z - zHeight, 0.f);
		drv->setConstant(8, 0.5f, 0.5f, 0.f, 0.f); // used to scale reflected ray into the envmap

	
		shape->initVertexProgram();		
		bool result;
		if (drv->getNbTextureStages() >= 4)
		{
			result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgramAlpha).get())
											: drv->activeVertexProgram((shape->_VertexProgram).get());
		}
		else
		{
			result = shape->getColorMap() ? drv->activeVertexProgram((shape->_VertexProgram2StagesAlpha).get())
										: drv->activeVertexProgram((shape->_VertexProgram2Stages).get());
		}
		if (!result) nlwarning("no vertex program setupped");
	}
	else
	{		
		waterMat.setColor(NLMISC::CRGBA(0, 32, 190, 128));	
	}
	drv->setupMaterial(waterMat);
}


/*
void CWaterRenderObs::setAttenuationFactor(IDriver *drv, bool reversed, const NLMISC::CVector &obsPos, const NLMISC::CVector &cameraJ, float farDist)
{
	
	CWaterModel		  *m				= NLMISC::safe_cast<CWaterModel *>(Model);
	const CWaterShape *shape = NLMISC::safe_cast<CWaterShape *>((IShape *) m->Shape);
	if (!reversed)
	{
		/// compute attenuation
		const float attenSlope = 1.f / (farDist *  (shape->_TransitionStartRatio - shape->_TransitionEndRatio));			
		drv->setConstant(5, attenSlope * cameraJ.x, attenSlope * cameraJ.y, attenSlope * cameraJ.z, 1.f - farDist * shape->_TransitionStartRatio * attenSlope);
	}
	else
	{
		/// compute attenuation
		const float attenSlope = 1.f / (farDist *  (shape->_TransitionEndRatio - shape->_TransitionStartRatio));			
		drv->setConstant(5, attenSlope * cameraJ.x, attenSlope * cameraJ.y, attenSlope * cameraJ.z, - farDist * shape->_TransitionStartRatio * attenSlope);
	}		
}

void CWaterRenderObs::disableAttenuation(IDriver *drv)
{
	drv->setConstant(5, 0.f, 0.f, 0.f, 1.f);
}
*/

} // NL3D
