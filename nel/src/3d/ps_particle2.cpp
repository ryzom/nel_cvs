/** \file ps_particle.cpp
 * <File description>
 *
 * $Id: ps_particle2.cpp,v 1.4 2001/11/22 15:34:14 corvazier Exp $
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

#include "3d/ps_particle2.h"
#include "3d/driver.h"
#include "3d/ps_attrib_maker.h"
#include "3d/ps_util.h"
#include "3d/dru.h"
#include "3d/particle_system.h"


#include <algorithm>


#ifdef NL_DEBUG
	#define CHECK_VERTEX_BUFFER(vb, pt) nlassert((uint8 *) (pt) >= (uint8 *) (vb).getVertexCoordPointer()  \
										&& (uint8 *) (pt) < ((uint8 *) (vb).getVertexCoordPointer() + (vb).getVertexSize() * (vb).getNumVertices()));
#else
	#define CHECK_VERTEX_BUFFER
#endif


// this macro check the memory integrity (windows platform for now). It may be useful after violent vb access
#if defined(NL_DEBUG) && defined(NL_OS_WINDOWS)
	#include <crtdbg.h>
	#define PARTICLES_CHECK_MEM 
    //nlassert(_CrtCheckMemory());
#else
	#define PARTICLES_CHECK_MEM
#endif

// display a vector using nlinfo
#define DUMP_VECT(v) nlinfo(#v "= %f, %f, %f", v.x, v.y, v.z);

namespace NL3D {




////////////////////////////////////
// CPSRibbonLookAt implementation //
////////////////////////////////////

CPSRibbonLookAt::CPSRibbonLookAt() : _DyingRibbons(NULL), _SegDuration(0.02f), 
									 _NbSegs(8), _NbDyingRibbons(0), _Parametric(false)
{
}

CPSRibbonLookAt::~CPSRibbonLookAt()
{
	delete _DyingRibbons;
}

void CPSRibbonLookAt::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(3);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);	
	CPSSizedParticle::serialSizeScheme(f);		
	serialMaterial(f);
	f.serial(_SegDuration, _NbSegs, _NbDyingRibbons);
	ITexture *tex = NULL;

	if (ver > 2)
	{
		f.serial(_Parametric);
	}

	if (!f.isReading())
	{		
		tex = _Tex;
		f.serialPolyPtr(tex);	
	}
	else
	{		
		f.serialPolyPtr(tex);
		setTexture(tex);
		setTailNbSeg(_NbSegs); // force to build the vb
	}	
}

void CPSRibbonLookAt::setTexture(CSmartPtr<ITexture> tex)
{
	_Tex = tex;
	updateMatAndVbForColor();
}

void CPSRibbonLookAt::setTailNbSeg(uint32 nbSegs)
{
	nlassert(nbSegs >= 2);
	_NbSegs = nbSegs;
	if (!_Owner) return; // no attachement occured yet ...
	const uint32 size = _Owner->getMaxSize();
	resize(size);
	reinitFromOwner();
}
		
void CPSRibbonLookAt::step(TPSProcessPass pass, TAnimationTime ellapsedTime)
{		
	if (pass == PSPostdynamic) // performs motion by using the speed obtained after forces integration
	{		
		performMotion(ellapsedTime);
	}
	if (
		(pass == PSBlendRender && hasTransparentFaces())
		|| (pass == PSSolidRender && hasOpaqueFaces())
		)
	{
		nlassert(_Owner);		
		displayRibbons(_Ribbons, _Owner->getSize());
		if (_DyingRibbons) displayRibbons(*_DyingRibbons, _NbDyingRibbons);
	}
	else 
	if (pass == PSToolRender) // edition mode only
	{			
		//showTool();
	}	
}

void CPSRibbonLookAt::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newColorElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
	if (_Parametric) return;
	nlassert(_Owner);	
	const uint32 index =  _Owner->getNewElementIndex();
	initRibbon(_Ribbons, index , _Owner->getPos()[index],  _Owner->getSpeed()[index]);
}

void CPSRibbonLookAt::deleteElement(uint32 index)
{
	deleteColorElement(index);
	deleteSizeElement(index);
	if (_Parametric) return;
	nlassert(_Owner);
	const uint32 size = _Owner->getSize();
	if(index == (size - 1)) return;
	dup(_Ribbons, _Ribbons, size - 1, index);
}

void CPSRibbonLookAt::resize(uint32 size)
{
	resizeVb();
	if (_Parametric) return;
	resizeColor(size);
	resizeSize(size);		
	resizeRibbons(_Ribbons, size);
	if (_DyingRibbons)
	{
		resizeRibbons(*_DyingRibbons, size);
	}
}


void CPSRibbonLookAt::resizeRibbons(CRibbons &r, uint32 size)
{
	nlassert(!_Parametric);
	r._Pts.resize(size * (_NbSegs + 1));
	r._Times.resize(size);	
}

void CPSRibbonLookAt::updateMatAndVbForColor(void)
{
	_Mat.setTexture(0, _Tex);
}


	
void CPSRibbonLookAt::dup(CRibbons &src, CRibbons &dest, uint32 srcIndex, uint32 dstIndex)
{
	nlassert(!_Parametric);
	// copy pos & size
	std::copy(src._Pts.begin() + (srcIndex * (_NbSegs + 1)), src._Pts.begin() + ((srcIndex + 1) * (_NbSegs + 1))
			  , dest._Pts.begin() + (dstIndex * (_NbSegs + 1))
			  );	
	dest._Times[dstIndex] = src._Times[srcIndex];		
}


/// build some hermite spline value, with the given points and tangents
static inline void BuildHermiteVector(const NLMISC::CVector P0,
							   const NLMISC::CVector P1,
							   const NLMISC::CVector T0,
							   const NLMISC::CVector T1,
									 NLMISC::CVector *dest,									 
							   float lambda,
							   float lambdaStep,
							   sint32 numValues
							   )
{
	for (sint32 k = 0; k < numValues; ++k)
	{
		const float lambda2 = lambda * lambda;
		const float lambda3 = lambda2 * lambda;
		const float h1 = 2 * lambda3 - 3 * lambda2 + 1; 
		const float h2 = - 2 * lambda3 + 3 * lambda2; 
		const float h3 = lambda3 - 2 * lambda2 + lambda; 
		const float h4 = lambda3 - lambda2; 
		*dest = h1 * P0 + h2 * P1 + h3 * T0 + h4 * T1;
		++dest;
	}
}

	
void CPSRibbonLookAt::performMotion(TAnimationTime ellapsedTime)
{
	if (ellapsedTime == 0) return;
	nlassert(_Owner);
	const uint32 nbRibbons = _Owner->getSize();
	if (nbRibbons == 0) return ;
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin();
	TPSAttribVector::const_iterator speedIt = _Owner->getSpeed().begin();


	
	

	if (ellapsedTime <= _SegDuration)
	{				
		for (uint k = 0; k < nbRibbons; ++k)
		{			
			const float newTime = _Ribbons._Times[k] + ellapsedTime;			
		/*	const float delta = _SegDuration - _Ribbons._Times[k];			
			const float alpha = ellapsedTime / (delta > 10E-2f ? delta : 10E-2f); // too low value leads to numerical instability
			performSubMotion(alpha, k, 0, _NbSegs);*/
							
			
			if (newTime >= _SegDuration)
			{
				_Ribbons._Times[k] = newTime - _SegDuration;
				std::copy(_Ribbons._Pts.begin() + (_NbSegs + 1) * k + 1,
						 _Ribbons._Pts.begin() + (_NbSegs + 1) * (k + 1) ,
						 _Ribbons._Pts.begin() + (_NbSegs + 1) * k);				
			}
			else
			{
				_Ribbons._Times[k] = newTime;				
			}
			// set last point		
			_Ribbons._Pts[k * (_NbSegs + 1) + _NbSegs] = *posIt;			
			
			
			
			++posIt;
			++speedIt;
		}

	}
	else // lag situation, must approximate previous position by using hermite interpolation
	{
		TPSAttribVector::const_iterator speedIt = _Owner->getSpeed().begin();
		uint numStep = (uint) (ellapsedTime / _SegDuration);
		uint numStepInc;
		const TAnimationTime fracTime = ellapsedTime - numStep * _SegDuration;
		// use hermite interpolation to create intermediate values
		NLMISC::CVector T0, T1, P0, P1;

	//	setupDriverModelMatrix();
		for (uint k = 0; k < nbRibbons; ++k)
		{	
			//dumpRibbon(k);
			const float newTime = _Ribbons._Times[k] + fracTime;
			if (newTime >= _SegDuration)
			{
				_Ribbons._Times[k] = newTime - _SegDuration;
				numStepInc = numStep + 1;
			}
			else
			{
				_Ribbons._Times[k] = newTime;
				numStepInc = numStep;
			}
			
			if (numStepInc < _NbSegs) 
			{				
				/// decal positions
				std::copy(_Ribbons._Pts.begin() + (_NbSegs + 1) * k + numStepInc,
							 _Ribbons._Pts.begin() + (_NbSegs + 1) * (k + 1),
							 _Ribbons._Pts.begin() + (_NbSegs + 1) * k);
				//dumpRibbon(k);				

				const float lambdaStep = 1.f / numStepInc;
				float lambda = lambdaStep;

				

				TPointVect::iterator pos = _Ribbons._Pts.begin() + (_NbSegs + 1) * k + (_NbSegs + 1 - numStepInc);

				P1 = *posIt;
				P0 = *(pos - 1);
				NLMISC::CVector tg = _SegDuration * (P1 - P0) / ellapsedTime;

				T0 =  ellapsedTime * 0.5f * ( tg + *(pos - 1) - *(pos - 2) ) /*/ _SegDuration*/;
				T1 =  ellapsedTime * 0.5f * ( tg + _SegDuration * *speedIt) /*/ _SegDuration*/;

			/*	CPSUtil::displayArrow(getDriver(), P0, T0, 1.5f, NLMISC::CRGBA::White, NLMISC::CRGBA::Red);		
				CPSUtil::displayArrow(getDriver(), P1, T1, 1.5f, NLMISC::CRGBA::White, NLMISC::CRGBA::Red);		*/

				// compute with hermite spline
				BuildHermiteVector(P0, P1, T0, T1, &*pos, lambda, lambdaStep, numStepInc);
							   
				/// perform sub motion
			/*	const float delta = _SegDuration - _Ribbons._Times[k];			
				const float alpha = fracTime / (delta > 10E-2f ? delta : 10E-2f); // too low value leads to numerical instability
				performSubMotion(alpha, k, 0, _NbSegs - numStepInc - 1);*/
								
			}
			else // all position are new
			{			
				TPointVect::iterator pos = _Ribbons._Pts.begin() + (_NbSegs + 1) * (k + 1);
				P1 = *posIt;
				NLMISC::CVector tg = _SegDuration * (P1 - P0) / ellapsedTime;
				T0 =  ellapsedTime * 0.5f * ( tg + *(pos - 1) - *(pos - 2) ) / _SegDuration;
				T1 =  ellapsedTime * 0.5f * ( tg + _SegDuration * *speedIt) / _SegDuration;
				const float lambdaStep = _SegDuration * ellapsedTime;
				float lambda = 1.f - (_NbSegs + 1) * lambdaStep;
				pos = pos - (_NbSegs + 1);
			
				BuildHermiteVector(P0, P1, T0, T1, &*pos, lambda, lambdaStep, _NbSegs + 1);
			}

			++posIt;
			++speedIt;
		}
	}
}



const float ZEpsilon = 10E-3f;
const float NormEpsilon = 10E-8f;


struct CVectInfo
{
	NLMISC::CVector Interp;
	NLMISC::CVector Proj;	
};
typedef std::vector<CVectInfo> TRibbonVect;
	

static inline void MakeProj(NLMISC::CVector &dest, const NLMISC::CVector &src)
{		
	if (fabsf(src.y) > NormEpsilon * NormEpsilon)
	{
		dest.x = src.x / src.y;
		dest.z = src.z / src.y;
		dest.y = src.y;	
	}	
}


// build one slice of the ribbon

static inline void BuildSlice(const NLMISC::CMatrix &mat, CVertexBuffer &vb, uint8 *currVert, uint32 vertexSize,
							  const NLMISC::CVector &I, 
							  const NLMISC::CVector &K, 
  							  TRibbonVect::iterator  pos,
							  TRibbonVect::iterator  prev,
							  TRibbonVect::iterator  next,
							  float ribSize
							  )
{
	CHECK_VERTEX_BUFFER(vb, currVert);
	CHECK_VERTEX_BUFFER(vb, currVert);
	NLMISC::CVector tangent;

	float invTgNorm; // inverse of the' norm of the projected segment
	float tgNorm;

	if (prev->Proj.y > ZEpsilon && next->Proj.y > ZEpsilon) // the 2 points are in front of the camera
	{
		tangent = next->Proj - prev->Proj;
		tangent.y = 0;
		tgNorm = tangent.norm();
		if (fabs(tgNorm) > 10E-8)
		{
			invTgNorm = 1.f / tgNorm;
		}
		else
		{
			invTgNorm = 1.f;
		}
		// build orthogonals vectors to tangent
		
		*(NLMISC::CVector *) currVert = pos->Interp + ribSize * invTgNorm * (tangent.x * K - tangent.z * I);						
		*(NLMISC::CVector *) (currVert + vertexSize) = pos->Interp + ribSize * invTgNorm * (- tangent.x * K + tangent.z * I);			
	}
	else if (prev->Proj.y > ZEpsilon) // second point cross the near plane
	{
		// compute intersection point
		NLMISC::CVector inter;
		NLMISC::CVector tInter;		
		if (fabsf(prev->Proj.y - next->Proj.y) > NormEpsilon)
		{
				float lambda = (next->Proj.y - ZEpsilon) / (next->Proj.y - prev->Proj.y);
				inter = lambda * prev->Interp + (1.f - lambda) * next->Interp;				
				MakeProj(tInter, mat * inter);
		}
		else // 
		{
			*(NLMISC::CVector *) currVert = pos->Interp;
			*(NLMISC::CVector *) (currVert + vertexSize) = pos->Interp;			
			return;
		}
		
		tangent = tInter - prev->Proj;		
		tangent.y = 0;

		tgNorm = tangent.norm();
		if (fabs(tgNorm > 10E-8))
		{
			invTgNorm = 1.f / tgNorm;			
		}
		else
		{
			invTgNorm = 1.f;
		}
		// build orthogonals vectors to tangent
		
		*(NLMISC::CVector *) currVert = inter + ribSize *  invTgNorm * (tangent.x * K - tangent.z * I);		
		*(NLMISC::CVector *) (currVert + vertexSize) = inter + ribSize * invTgNorm * (- tangent.x * K + tangent.z * I);
	}
	else if (next->Proj.y > ZEpsilon) // first point cross the near plane
	{
			// compute intersection point
		// compute intersection point
		NLMISC::CVector inter;
		NLMISC::CVector tInter;
		if (fabsf(prev->Proj.y - next->Proj.y) > NormEpsilon)
		{
				float lambda = (next->Proj.y - ZEpsilon) / (next->Proj.y - prev->Proj.y);
				inter = lambda * prev->Interp + (1.f - lambda) * next->Interp;				
				MakeProj(tInter, mat * inter);
		}
		else // 
		{
			*(NLMISC::CVector *) currVert = pos->Interp;
			*(NLMISC::CVector *) (currVert + vertexSize) = pos->Interp;
			return;
		}
		
		tangent = next->Proj - tInter;		
		tangent.y = 0;
		tgNorm = tangent.norm();
		if (fabs(tgNorm > 10E-8))
		{
			invTgNorm = 1.f / tgNorm;			
		}
		else
		{
			invTgNorm = 1.f;
		}
		// build orthogonals vectors to tangent
		
		*(NLMISC::CVector *) currVert = inter + ribSize * invTgNorm * (tangent.x * K - tangent.z * I);		
		*(NLMISC::CVector *) (currVert + vertexSize) = inter + ribSize * invTgNorm * (- tangent.x * K + tangent.z * I);

	}
	else // two points are not visible
	{
		*(NLMISC::CVector *) currVert = pos->Interp;
		*(NLMISC::CVector *) (currVert + vertexSize) = pos->Interp;
	}

	
	
	
}


	
void CPSRibbonLookAt::displayRibbons(CRibbons &r, uint32 nbRibbons)
{	
	if (!nbRibbons) return;
	nlassert(_Owner);	
	const float date = _Owner->getOwner()->getSystemDate();
	uint8						*currVert;
	const uint32				vertexSize  = _VB.getVertexSize();
	const uint32				vertexSizeX2  = vertexSize << 1;
	const NLMISC::CVector       I = _Owner->computeI();
	const NLMISC::CVector       K = _Owner->computeK();
	TPointVect::const_iterator posIt;	
	if (!_Parametric) posIt = r._Pts.begin();

	static TRibbonVect				   currRibbon;
	static std::vector<float>		   sizes;
	static std::vector<NLMISC::CRGBA>  colors;

	float	*ptCurrSize;
	uint32  ptCurrSizeIncrement;	
	if (_SizeScheme)
	{
		sizes.resize(nbRibbons);
		_SizeScheme->make(this->_Owner, 0, &sizes[0], sizeof(float), nbRibbons);
		ptCurrSize = &sizes[0];
		ptCurrSizeIncrement = 1;
	}
	else
	{
		ptCurrSize = &_ParticleSize;
		ptCurrSizeIncrement = 0;
	}


	NLMISC::CRGBA	*ptCurrColor;
	uint32  ptCurrColorIncrement;	
	if (_ColorScheme)
	{
		colors.resize(nbRibbons);
		ptCurrColor = (NLMISC::CRGBA *) _ColorScheme->make(this->_Owner, 0, &colors[0], sizeof(NLMISC::CRGBA), nbRibbons, true);
		ptCurrColorIncrement = 1;
	}
	else
	{
		ptCurrColor = &_Color;
		ptCurrColorIncrement = 0;
	}


	currRibbon.resize(_NbSegs + 1);




	CMatrix mat =  _Owner->isInSystemBasis() ? getViewMat()  *  getSysMat()
															  : getViewMat();

	IDriver *drv = this->getDriver();
	setupDriverModelMatrix();
	drv->activeVertexBuffer(_VB);
	_Owner->incrementNbDrawnParticles(nbRibbons); // for benchmark purpose		

	for (uint k = 0; k < nbRibbons; ++k)
	{			
		currVert = (uint8 *) _VB.getVertexCoordPointer();		

		
		TRibbonVect::iterator rIt = currRibbon.begin(), rItEnd = currRibbon.end(), rItEndMinusOne = rItEnd - 1;

		////////////////////////////////////
		// interpolate and project points //
		////////////////////////////////////
	
			if (!_Parametric)
			{

				//////////////////////
				// INCREMENTAL CASE //
				//////////////////////								
				// we use the previous position to build the curve				

				// get the lambda to decal ribbon slices
				float lambda = r._Times[k] / _SegDuration;
				float oneMinusLambda = 1.f - lambda;

				do
				{
					rIt->Interp = lambda * *(posIt + 1) + oneMinusLambda * *posIt;			
					MakeProj(rIt->Proj, mat * rIt->Interp);
					++rIt;
					++posIt;						
				}
				while (rIt != rItEndMinusOne);

				rIt->Interp = *posIt;		
				MakeProj(rIt->Proj, mat * rIt->Interp);
				++posIt;
			}
			else
			{
				//////////////////////
				// PARAMETRIC  CASE //
				//////////////////////
				// we compute each pos thanks to the parametric curve
				_Owner->integrateSingle(date - _SegDuration * (_NbSegs + 1), _SegDuration, _NbSegs + 1, k,
									 &rIt->Interp, sizeof(CVectInfo) );
				// project each position now
				do
				{					
					MakeProj(rIt->Proj, mat * rIt->Interp); // we don't multiply by mat, this was done during integarteSingle
					++rIt;				
				}
				while (rIt != rItEnd);			

			}

			rIt = currRibbon.begin();

		////////////////////////////
		// deals with first point //
		////////////////////////////		
		BuildSlice(mat, _VB, currVert, vertexSize, I, K, rIt, rIt, rIt + 1, *ptCurrSize); 
		currVert += vertexSizeX2;		
		++rIt;

		/////////////////////////////
		// deals with other points //
		/////////////////////////////

		for (;;) // we assume at least 2 segments, so we must have a middle point		
		{						
			// build 2 vertices with the right tangent. /* to project 2 */ is old projected point
			BuildSlice(mat, _VB, currVert, vertexSize, I, K, rIt, rIt - 1, rIt + 1, *ptCurrSize); 
			// next position		
			++rIt;
			if (rIt == rItEndMinusOne) break;					
			// next vertex			
			currVert += vertexSizeX2;		
		}
		currVert += vertexSizeX2;
		// last point.
		BuildSlice(mat, _VB, currVert, vertexSize, I, K, rIt , rIt - 1, rIt, *ptCurrSize); 			
		
		_Mat.setColor(*ptCurrColor);

		// display the result
		drv->renderTriangles(_Mat, &_IB[0], _NbSegs << 1);

		ptCurrSize += ptCurrSizeIncrement;
		ptCurrColor += ptCurrColorIncrement;
	}
}
	
void CPSRibbonLookAt::initRibbon(CRibbons &r, uint32 index, const NLMISC::CVector &pos, const NLMISC::CVector &speed)
{
	std::fill(r._Pts.begin() + (index * (_NbSegs + 1)), r._Pts.begin() + (index + 1) * (_NbSegs + 1), pos );	
	r._Times[index] = 0.f;	
	
}

void CPSRibbonLookAt::resizeVb(void)
{
	_VB.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag /*| CVertexBuffer::PrimaryColorFlag*/);
	_VB.setNumVertices((_NbSegs + 1) * 2);
	_IB.resize(_NbSegs * 6);
	for (uint k = 0; k < _NbSegs; ++k)
	{
	/*	_VB.setColor(2 * k, NLMISC::CRGBA::White);
		_VB.setColor(2 * k + 1, NLMISC::CRGBA::White);*/

		_VB.setTexCoord(2 * k, 0, CUV(k / (float) _NbSegs, 0));
		_VB.setTexCoord(2 * k + 1, 0, CUV(k / (float) _NbSegs, 1));		
		_IB[6 * k] = k * 2;
		_IB[6 * k + 1] = k * 2 + 1;
		_IB[6 * k + 2] = k * 2 + 2;

		_IB[6 * k + 3] = k * 2 + 2;
		_IB[6 * k + 4] = k * 2 + 1;
		_IB[6 * k + 5] = k * 2 + 3;
	}
	_VB.setTexCoord(2 * _NbSegs, 0, CUV(1, 0));
	_VB.setTexCoord(2 * _NbSegs + 1, 0, CUV(1, 1));		
/*	_VB.setColor(2 * _NbSegs, NLMISC::CRGBA::White);
	_VB.setColor(2 * _NbSegs + 1, NLMISC::CRGBA::White);*/
	
}
/*
void CPSRibbonLookAt::setupColor(CRibbons &rb)
{

} 
*/

bool CPSRibbonLookAt::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}
bool CPSRibbonLookAt::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

uint32 CPSRibbonLookAt::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize() * _NbSegs * 2;	
}

void CPSRibbonLookAt::reinitFromOwner(void)
{
	if (_Parametric) return;
	if (!_Owner) return;
	const uint32 size = _Owner->getMaxSize();
	resize(size);
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt = _Owner->getPos().end();
	TPSAttribVector::const_iterator speedIt = _Owner->getSpeed().begin();

	for (uint32 k = 0; posIt != endPosIt; ++posIt, ++k, ++speedIt)
	{
		initRibbon(_Ribbons, k, *posIt, *speedIt);
	}
}

void CPSRibbonLookAt::setSegDuration(TAnimationTime ellapsedTime)
{
	_SegDuration = ellapsedTime;
}

TAnimationTime	CPSRibbonLookAt::getSegDuration(void) const
{
	return _SegDuration;
}


void	CPSRibbonLookAt::dumpRibbon(uint32 index)
{
	nlassert(!_Parametric);
	TPointVect::iterator posIt = _Ribbons._Pts.begin() + (_NbSegs + 1) * index;
	TPointVect::iterator endPosIt = posIt + _NbSegs + 1;
	nlinfo("**** RIBBON ****");
	while (posIt != endPosIt)
	{
		nlinfo("(%f, %f, %f)", posIt->x, posIt->y, posIt->z);
		++posIt;
	}
}


void	    CPSRibbonLookAt::motionTypeChanged(bool parametric)
{
	_Parametric = parametric;
	if (parametric)
	{
		_Ribbons.reset();
	}
	else
	{
		reinitFromOwner();
	}

}

} // NL3D
