/** \file ps_face_look_at.cpp
 * Face look at particles.
 *
 * $Id: ps_face_look_at.cpp,v 1.3 2002/08/21 09:39:53 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "3d/ps_face_look_at.h"
#include "3d/ps_macro.h"
#include "3d/driver.h"
#include "3d/fast_floor.h"
#include "3d/ps_iterator.h"


namespace NL3D 
{



//////////////////////////////////
// CPSFaceLookAt implementation //
//////////////////////////////////


/** Well, we could have put a method template in CPSFaceLookAt, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSFaceLookAtHelper
{
public:
	template <class T>	
	static void drawLookAt(T it, T speedIt, CPSFaceLookAt &la, uint size, uint32 srcStep)
	{
		PARTICLES_CHECK_MEM;	
		nlassert(la._Owner);			
		IDriver *driver = la.getDriver();

		la.updateMatBeforeRendering(driver);
		
		CVertexBuffer &vb = la.getNeededVB();
		la._Owner->incrementNbDrawnParticles(size); // for benchmark purpose	
		la.setupDriverModelMatrix();
		driver->activeVertexBuffer(vb);	
		const CVector I = la.computeI();
		const CVector J = la.computeJ();
		const CVector K = la.computeK();		
		const float *rotTable = CPSRotated2DParticle::getRotTable();	
		// for each the particle can be constantly rotated or have an independant rotation for each particle
		// number of face left, and number of face to process at once
		uint32 leftToDo = size, toProcess;
		float pSizes[CPSQuad::quadBufSize]; // the sizes to use	
		float pSecondSizes[CPSQuad::quadBufSize]; // the second sizes to use
		float *currentSize; 
		uint32 currentSizeStep = la._SizeScheme ? 1 : 0;
		// point the vector part in the current vertex
		uint8 *ptPos;
		// strides to go from one vertex to another one
		const uint32 stride = vb.getVertexSize(), stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride << 2;	
		if (!la._Angle2DScheme)
		{
			// constant rotation case
			do
			{			
				// restart at the beginning of the vertex buffer
				ptPos = (uint8 *) vb.getVertexCoordPointer();
				toProcess = leftToDo <= CPSQuad::quadBufSize ? leftToDo : CPSQuad::quadBufSize;

				if (la._SizeScheme)
				{
					currentSize = (float *) la._SizeScheme->make(la._Owner, size- leftToDo, pSizes, sizeof(float), toProcess, true, srcStep);
				}
				else
				{
					currentSize = &la._ParticleSize;
				}
				
				la.updateVbColNUVForRender(vb, size - leftToDo, toProcess, srcStep);					
				T endIt = it + toProcess;
				if (la._MotionBlurCoeff == 0.f)
				{
					if (!la._IndependantSizes)
					{						
						const uint32 tabIndex = (((uint32) la._Angle2D) & 0xff) << 2;
						const CVector v1 = rotTable[tabIndex] * I + rotTable[tabIndex + 1] * K;
						const CVector v2 = rotTable[tabIndex + 2] * I + rotTable[tabIndex + 3] * K;
						if (currentSizeStep)
						{
							while (it != endIt)
							{
								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x;  			
								((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y;  			
								((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  + *currentSize * v2.x;  			
								((CVector *) ptPos)->y = (*it).y  + *currentSize * v2.y;  			
								((CVector *) ptPos)->z = (*it).z  + *currentSize * v2.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x;  			
								((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y;  			
								((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - *currentSize * v2.x;  			
								((CVector *) ptPos)->y = (*it).y  - *currentSize * v2.y;  			
								((CVector *) ptPos)->z = (*it).z  - *currentSize * v2.z;  			
								ptPos += stride;							

								++it;
								currentSize += currentSizeStep;					
							}
						}
						else
						{
							// constant size
							const CVector myV1 = *currentSize * v1;
							const CVector myV2 = *currentSize * v2;

							while (it != endIt)
							{
								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  + myV1.x;  			
								((CVector *) ptPos)->y = (*it).y  + myV1.y;  			
								((CVector *) ptPos)->z = (*it).z  + myV1.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  + myV2.x;  			
								((CVector *) ptPos)->y = (*it).y  + myV2.y;  			
								((CVector *) ptPos)->z = (*it).z  + myV2.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - myV1.x;  			
								((CVector *) ptPos)->y = (*it).y  - myV1.y;  			
								((CVector *) ptPos)->z = (*it).z  - myV1.z;  			
								ptPos += stride;

								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - myV2.x;  			
								((CVector *) ptPos)->y = (*it).y  - myV2.y;  			
								((CVector *) ptPos)->z = (*it).z  - myV2.z;  			
								ptPos += stride;							
								++it;											
							}
						}
					}
					else // independant sizes
					{			
						const CVector v1 = CPSUtil::getCos((sint32) la._Angle2D) * I  + CPSUtil::getSin((sint32) la._Angle2D) * K;
						const CVector v2 = - CPSUtil::getSin((sint32) la._Angle2D) * I + CPSUtil::getCos((sint32) la._Angle2D) * K;

						float *currentSize2;
						float secondSize;
						uint32 currentSizeStep2;
						if (la._SecondSize.getSizeScheme())
						{
							currentSize2 = (float *) la._SecondSize.getSizeScheme()->make(la._Owner, size- leftToDo, pSecondSizes, sizeof(float), toProcess, true, srcStep);
							currentSizeStep2 = 1;
						}
						else
						{	
							secondSize = la._SecondSize.getSize();
							currentSize2 = &secondSize;
							currentSizeStep2 = 0;
						}
						 

						while (it != endIt)
						{
							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x + *currentSize2 * v2.x;  			
							((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y + *currentSize2 * v2.y;  			
							((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z + *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x + *currentSize2 * v2.x;  			
							((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y + *currentSize2 * v2.y;  			
							((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z + *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x - *currentSize2 * v2.x;  			
							((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y - *currentSize2 * v2.y;  			
							((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z - *currentSize2 * v2.z;
							ptPos += stride;

							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x - *currentSize2 * v2.x;  			
							((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y - *currentSize2 * v2.y;  			
							((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z - *currentSize2 * v2.z;
							ptPos += stride;
							++it;
							currentSize += currentSizeStep;
							currentSize2 += currentSizeStep2;
						}					
					}
				}
				else
				{
					// perform motion, blur, we need an iterator on speed
					// independant sizes and rotation not supported for now with motion blur				
					const CVector v1 = I + K;
					const CVector v2 = K - I;								
					CVector startV, endV, mbv1, mbv1n, mbv12, mbv2;
					// norme of the v1 vect
					float n;
					const float epsilon  = 10E-5f;
					const float normEpsilon  = 10E-6f;
					
					CMatrix tMat =  la._Owner->isInSystemBasis() ? la.getViewMat()  *  la.getSysMat()
															  : la.getViewMat();
			
					while (it != endIt)
					{
						// project the speed in the projection plane
						// this give us the v1 vect
						startV = tMat * *it ;						
						endV = tMat * (*it + *speedIt);													
						if (startV.y > epsilon || endV.y > epsilon)																																		
						{	
							if (startV.y < epsilon)
							{
								if (fabsf(endV.y - startV.y) > normEpsilon)
								{
									startV = endV + (endV.y - epsilon) / (endV.y - startV.y) * (startV - endV);
								}
								startV.y = epsilon;
							}
							else if (endV.y < epsilon)
							{	
								if (fabsf(endV.y - startV.y) > normEpsilon)
								{
									endV = startV + (startV.y - epsilon) / (startV.y - endV.y) * (endV - startV);
								}
								endV.y = epsilon;
							}														

							mbv1 = (startV.x / startV.y - endV.x / endV.y) * I
											+ (startV.z / startV.y - endV.z / endV.y) * K ;						
						
							n = mbv1.norm();
							if (n > la._Threshold)
							{
								mbv1 *= la._Threshold / n;
								n = la._Threshold;								
							}																										
							if (n > normEpsilon)												
							{																							
								mbv1n = mbv1 / n;														
								mbv2 = *currentSize * (J ^ mbv1n);
								mbv12 = -*currentSize * mbv1n;							
								mbv1 *= *currentSize * (1 + la._MotionBlurCoeff * n * n) / n;
							
								*(CVector *) ptPos = *it - mbv2;												
								*(CVector *) (ptPos + stride) = *it  + mbv1;  			
								*(CVector *) (ptPos + stride2) = *it + mbv2;	
								*(CVector *) (ptPos + stride3) = *it + mbv12;
							
							
								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - mbv2.x;  			
								((CVector *) ptPos)->y = (*it).y  - mbv2.y;
								((CVector *) ptPos)->z = (*it).z  - mbv2.z;
								
								CHECK_VERTEX_BUFFER(vb, ptPos + stride);
								((CVector *) (ptPos + stride))->x = (*it).x  + mbv1.x;  			
								((CVector *) (ptPos + stride))->y = (*it).y  + mbv1.y;  			
								((CVector *) (ptPos + stride))->z = (*it).z  + mbv1.z;  			

								CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
								((CVector *) (ptPos + stride2))->x = (*it).x  + mbv2.x;  			
								((CVector *) (ptPos + stride2))->y = (*it).y  + mbv2.y;  			
								((CVector *) (ptPos + stride2))->z = (*it).z  + mbv2.z;  			


								CHECK_VERTEX_BUFFER(vb, ptPos + stride3);
								((CVector *) (ptPos + stride3))->x = (*it).x  + mbv12.x;  			
								((CVector *) (ptPos + stride3))->y = (*it).y  + mbv12.y;  			
								((CVector *) (ptPos + stride3))->z = (*it).z  + mbv12.z;  												
								
							}
							else // speed too small, we must avoid imprecision
							{
								CHECK_VERTEX_BUFFER(vb, ptPos);
								((CVector *) ptPos)->x = (*it).x  - *currentSize * v2.x;  			
								((CVector *) ptPos)->y = (*it).y  - *currentSize * v2.y;  			
								((CVector *) ptPos)->z = (*it).z  - *currentSize * v2.z;  			
								
								CHECK_VERTEX_BUFFER(vb, ptPos + stride);
								((CVector *) (ptPos + stride))->x = (*it).x  + *currentSize * v1.x;  			
								((CVector *) (ptPos + stride))->y = (*it).y  + *currentSize * v1.y;  			
								((CVector *) (ptPos + stride))->z = (*it).z  + *currentSize * v1.z;  			

								CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
								((CVector *) (ptPos + stride2))->x = (*it).x  + *currentSize * v2.x;  			
								((CVector *) (ptPos + stride2))->y = (*it).y  + *currentSize * v2.y;  			
								((CVector *) (ptPos + stride2))->z = (*it).z  + *currentSize * v2.z;  			


								CHECK_VERTEX_BUFFER(vb, ptPos + stride3);
								((CVector *) (ptPos + stride3))->x = (*it).x  - *currentSize * v1.x;  			
								((CVector *) (ptPos + stride3))->y = (*it).y  - *currentSize * v1.y;  			
								((CVector *) (ptPos + stride3))->z = (*it).z  - *currentSize * v1.z;  							
							}
						}
						else
						{

							CHECK_VERTEX_BUFFER(vb, ptPos);
							((CVector *) ptPos)->x = (*it).x  - *currentSize * v2.x;  			
							((CVector *) ptPos)->y = (*it).y  - *currentSize * v2.y;  			
							((CVector *) ptPos)->z = (*it).z  - *currentSize * v2.z;  			
							
							CHECK_VERTEX_BUFFER(vb, ptPos + stride);
							((CVector *) (ptPos + stride))->x = (*it).x  + *currentSize * v1.x;  			
							((CVector *) (ptPos + stride))->y = (*it).y  + *currentSize * v1.y;  			
							((CVector *) (ptPos + stride))->z = (*it).z  + *currentSize * v1.z;  			

							CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
							((CVector *) (ptPos + stride2))->x = (*it).x  + *currentSize * v2.x;  			
							((CVector *) (ptPos + stride2))->y = (*it).y  + *currentSize * v2.y;  			
							((CVector *) (ptPos + stride2))->z = (*it).z  + *currentSize * v2.z;  			


							CHECK_VERTEX_BUFFER(vb, ptPos + stride3);
							((CVector *) (ptPos + stride3))->x = (*it).x  - *currentSize * v1.x;  			
							((CVector *) (ptPos + stride3))->y = (*it).y  - *currentSize * v1.y;  			
							((CVector *) (ptPos + stride3))->z = (*it).z  - *currentSize * v1.z;  													
						}
					
						ptPos += stride4;
						++it;
						++speedIt;
						currentSize += currentSizeStep;					
					}

				}			
				driver->renderQuads(la._Mat, 0, toProcess);			
				leftToDo -= toProcess;				
			}
			while (leftToDo);
		}
		else
		{		
			float pAngles[CPSQuad::quadBufSize]; // the angles to use
			float *currentAngle;		
			do
			{			
				// restart at the beginning of the vertex buffer
				ptPos = (uint8 *) vb.getVertexCoordPointer();
				toProcess = leftToDo <= CPSQuad::quadBufSize ? leftToDo : CPSQuad::quadBufSize;
				if (la._SizeScheme)
				{
					currentSize = (float *) la._SizeScheme->make(la._Owner, size - leftToDo, pSizes, sizeof(float), toProcess, true, srcStep);
				}
				else
				{
					currentSize = &la._ParticleSize;
				}
				currentAngle = (float *) la._Angle2DScheme->make(la._Owner, size - leftToDo, pAngles, sizeof(float), toProcess, true, srcStep);
				la.updateVbColNUVForRender(vb, size - leftToDo, toProcess, srcStep);					
				T endIt = it + toProcess;
				CVector v1, v2;
				OptFastFloorBegin();
				if (!la._IndependantSizes)
				{				
					while (it != endIt)
					{
						const uint32 tabIndex = ((OptFastFloor(*currentAngle)) & 0xff) << 2;			
						// lets avoid some ctor calls
						v1.x = *currentSize * (rotTable[tabIndex] * I.x + rotTable[tabIndex + 1] * K.x);
						v1.y = *currentSize * (rotTable[tabIndex] * I.y + rotTable[tabIndex + 1] * K.y);
						v1.z = *currentSize * (rotTable[tabIndex] * I.z + rotTable[tabIndex + 1] * K.z);

						v2.x = *currentSize * (rotTable[tabIndex + 2] * I.x + rotTable[tabIndex + 3] * K.x);
						v2.y = *currentSize * (rotTable[tabIndex + 2] * I.y + rotTable[tabIndex + 3] * K.y);
						v2.z = *currentSize * (rotTable[tabIndex + 2] * I.z + rotTable[tabIndex + 3] * K.z);
						
						CHECK_VERTEX_BUFFER(vb, ptPos);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride2);
						CHECK_VERTEX_BUFFER(vb, ptPos + stride3);				
					
						((CVector *) ptPos)->x  = (*it).x  + v1.x;		
						((CVector *) ptPos)->y  = (*it).y  + v1.y;
						((CVector *) ptPos)->z = (*it).z  + v1.z;  			
						ptPos += stride;

						((CVector *) ptPos)->x  = (*it).x  + v2.x;		
						((CVector *) ptPos)->y  = (*it).y  + v2.y;
						((CVector *) ptPos)->z = (*it).z  + v2.z;  			
						ptPos += stride;

						((CVector *) ptPos)->x  = (*it).x  - v1.x;		
						((CVector *) ptPos)->y  = (*it).y  - v1.y;
						((CVector *) ptPos)->z = (*it).z  - v1.z;  			
						ptPos += stride;

						((CVector *) ptPos)->x  = (*it).x  - v2.x;		
						((CVector *) ptPos)->y  = (*it).y  - v2.y;
						((CVector *) ptPos)->z = (*it).z  - v2.z;  					
						ptPos += stride;
						
						++it;
						currentSize += currentSizeStep;
						++currentAngle;					
					}
				}
				else // independant size, and non-constant rotation
				{
					
					float *currentSize2;
					float secondSize;
					uint32 currentSizeStep2;
					if (la._SecondSize.getSizeScheme())
					{
						currentSize2 = (float *) la._SecondSize.getSizeScheme()->make(la._Owner, size- leftToDo, pSecondSizes, sizeof(float), toProcess, true, srcStep);
						currentSizeStep2 = 1;
					}
					else
					{	
						secondSize = la._SecondSize.getSize();
						currentSize2 = &secondSize;
						currentSizeStep2 = 0;
					}

					float cosAngle, sinAngle;
					while (it != endIt)
					{
						cosAngle = CPSUtil::getCos((sint32) *currentAngle);
						sinAngle = CPSUtil::getSin((sint32) *currentAngle);
						v1 = cosAngle * I  + sinAngle * K;
						v2 = - sinAngle * I + cosAngle * K;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x + *currentSize2 * v2.x;  			
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y + *currentSize2 * v2.y;  			
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x + *currentSize2 * v2.x;  			
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y + *currentSize2 * v2.y;  			
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z + *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  + *currentSize * v1.x - *currentSize2 * v2.x;  			
						((CVector *) ptPos)->y = (*it).y  + *currentSize * v1.y - *currentSize2 * v2.y;  			
						((CVector *) ptPos)->z = (*it).z  + *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;

						CHECK_VERTEX_BUFFER(vb, ptPos);
						((CVector *) ptPos)->x = (*it).x  - *currentSize * v1.x - *currentSize2 * v2.x;  			
						((CVector *) ptPos)->y = (*it).y  - *currentSize * v1.y - *currentSize2 * v2.y;  			
						((CVector *) ptPos)->z = (*it).z  - *currentSize * v1.z - *currentSize2 * v2.z;
						ptPos += stride;
						++it;
						++currentAngle;
						currentSize  += currentSizeStep;
						currentSize2 += currentSizeStep2;			
					}
				}				
				OptFastFloorEnd();							
				driver->renderQuads(la._Mat, 0, toProcess);			
				leftToDo -= toProcess;
			}
			while (leftToDo);
		}
		PARTICLES_CHECK_MEM;
	}
};

///===========================================================================================
void CPSFaceLookAt::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;	
	if (!_Owner->getSize()) return;	

	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);	
	if (!numToProcess) return;
	

	if (step == (1 << 16))
	{
		CPSFaceLookAtHelper::drawLookAt(_Owner->getPos().begin(),
				   _Owner->getSpeed().begin(),
				   *this,
				    numToProcess,
					step
			   );
	}
	else
	{		
		CPSFaceLookAtHelper::drawLookAt(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),
				   TIteratorVectStep1616(_Owner->getSpeed().begin(), 0, step),
				   *this,
				   numToProcess,
				   step				
				  );
	}
	
	PARTICLES_CHECK_MEM;
}

///===========================================================================================
CPSFaceLookAt::CPSFaceLookAt(CSmartPtr<ITexture> tex) : CPSQuad(tex), _MotionBlurCoeff(0.f)
														, _Threshold(0.5f), _IndependantSizes(false)
{	
	_SecondSize.Owner = this;
	_Name = std::string("LookAt");
}

///===========================================================================================
void CPSFaceLookAt::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	CPSQuad::newElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
}

///===========================================================================================
void CPSFaceLookAt::deleteElement(uint32 index)
{
	CPSQuad::deleteElement(index);
	deleteAngle2DElement(index);
}

///===========================================================================================
void CPSFaceLookAt::resize(uint32 capacity)
{
	nlassert(capacity < (1 << 16));
	CPSQuad::resize(capacity);
	resizeAngle2D(capacity);
}


///===========================================================================================
void CPSFaceLookAt::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(2);
	CPSQuad::serial(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);	
	f.serial(_MotionBlurCoeff);
	if (_MotionBlurCoeff != 0)
	{
		f.serial(_Threshold);
	}
	if (ver > 1)
	{
		f.serial(_IndependantSizes);
		if (_IndependantSizes)
		{
			_SecondSize.serialSizeScheme(f);
		}
	}
	if (f.isReading())
	{
		init();		
	}
}

} // NL3D
