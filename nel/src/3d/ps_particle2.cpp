/** \file ps_particle.cpp
 * <File description>
 *
 * $Id: ps_particle2.cpp,v 1.6 2002/01/28 15:47:37 vizerie Exp $
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

/// build some hermite spline value, with the given points and tangents
static inline void BuildHermiteVector(const NLMISC::CVector &P0,
							   const NLMISC::CVector &P1,
							   const NLMISC::CVector &T0,
							   const NLMISC::CVector &T1,
									 NLMISC::CVector &dest,									 
							   float lambda
							   )
{	
	const float lambda2 = lambda * lambda;
	const float lambda3 = lambda2 * lambda;
	const float h1 = 2 * lambda3 - 3 * lambda2 + 1; 
	const float h2 = - 2 * lambda3 + 3 * lambda2; 
	const float h3 = lambda3 - 2 * lambda2 + lambda; 
	const float h4 = lambda3 - lambda2;
	/// just avoid some ctor calls here...
	dest.set (h1 * P0.x + h2 * P1.x + h3 * T0.x + h4 * T1.x,
			  h1 * P0.y + h2 * P1.y + h3 * T0.y + h4 * T1.y,
			  h1 * P0.z + h2 * P1.z + h3 * T0.z + h4 * T1.z);

}

/// for test
static inline void BuildLinearVector(const NLMISC::CVector &P0,
									 const NLMISC::CVector &P1,							   
									 NLMISC::CVector &dest,									 
									 float lambda,
									 float oneMinusLambda
							        )
{		
	dest.set (lambda * P1.x + oneMinusLambda * P0.x,
			  lambda * P1.y + oneMinusLambda * P0.y,
			  lambda * P1.z + oneMinusLambda * P0.z);			  		
}


const uint EndRibbonStorage = 2;

////////////////////////////////////
//  CPSRibbonBase implementation  //
////////////////////////////////////

//=======================================================
CPSRibbonBase::CPSRibbonBase() : _NbSegs(8),
								 _SegDuration(0.02f),
								 _RibbonIndex(0),
								 _LastUpdateDate(0),
								 _Parametric(false),
								 _RibbonMode(VariableSize),
								 _InterpolationMode(Hermitte),
								 _RibbonLength(1),
								 _SegLength(_RibbonLength / _NbSegs),
								 _LODDegradation(1)

{
	initDateVect();
}

//=======================================================
void	CPSRibbonBase::setRibbonLength(float length)
{ 
	nlassert(length > 0.f);
	_RibbonLength = length;
	_SegLength = length / _NbSegs;
}

//=======================================================
void	CPSRibbonBase::setRibbonMode(TRibbonMode mode)
{
	nlassert(mode < RibbonModeLast);
	_RibbonMode = mode;
}


//=======================================================
void	CPSRibbonBase::setInterpolationMode(TInterpolationMode mode)
{
	nlassert(mode < InterpModeLast);
	_InterpolationMode = mode;
}

//=======================================================
void	CPSRibbonBase::setTailNbSeg(uint32 nbSegs)
{
	nlassert(nbSegs >= 1);	
	_NbSegs = nbSegs;
	nlassert(_Owner);
	_RibbonIndex = 0;
	resize(_Owner->getMaxSize());	
	initDateVect();
}


//=======================================================	
void	CPSRibbonBase::setSegDuration(TAnimationTime ellapsedTime)
{
	_SegDuration = ellapsedTime;

}

//=======================================================	
void	CPSRibbonBase::updateGlobals()
{
	nlassert(!_Parametric);
	nlassert(_Owner);
	const uint size = _Owner->getSize();
	if (!size) return;
	const TAnimationTime currDate = _Owner->getOwner()->getSystemDate();
	if (currDate  - _LastUpdateDate >= _SegDuration)
	{
		if (_RibbonIndex == 0) _RibbonIndex = _NbSegs + EndRibbonStorage;
		else --_RibbonIndex;
		
		/// decal date
		::memmove(&_SamplingDate[1], &_SamplingDate[0], sizeof(float) * (_NbSegs + EndRibbonStorage));
		_LastUpdateDate = currDate;
	}
	
	/// save current date
	_SamplingDate[0] = currDate;

	/// updating ribbons positions	
	TPSAttribVector::iterator posIt = _Owner->getPos().begin();
	TPosVect::iterator currIt = _Ribbons.begin() + _RibbonIndex;
	uint k = size;
	for (;;)
	{
		*currIt = *posIt;
		--k;
		if (!k) break;
		++posIt;
		currIt += (_NbSegs + 1 + EndRibbonStorage);
	}
}


//=======================================================	
void	CPSRibbonBase::computeHermitteRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{	
	nlassert(!_Parametric);	
	TPosVect::iterator startIt = _Ribbons.begin() + (_NbSegs + 1 + EndRibbonStorage) * index;
	TPosVect::iterator endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	TPosVect::iterator currIt  = startIt + _RibbonIndex;	
	TPosVect::iterator nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	TPosVect::iterator nextNextIt = nextIt + 1;	
	if (nextNextIt == endIt) nextNextIt = startIt;	
	float *date = &_SamplingDate[0];			

	NLMISC::CVector t0 = (*currIt - *nextIt);
	NLMISC::CVector t1 = 0.5f * (*nextNextIt - *currIt);

	uint leftToDo = _UsedNbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;

	for (;;)
	{		
		float dt = date[0] - date[1];

		if (dt < 10E-6f) // we reached the start of ribbon
		{

			do
			{
				*dest = *currIt;
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			}
			while (--leftToDo);			
			return;
		}

		float newLambdaStep = _UsedSegDuration / dt;
		// readapt lambda
		lambda *= newLambdaStep / lambdaStep;
		lambdaStep = newLambdaStep;
		for(;;)
		{
			if (lambda >= 1.f) break;
			/// compute a location
			BuildHermiteVector(*currIt, *nextIt, t0, t1, *dest, lambda);
			dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			-- leftToDo;
			if (!leftToDo) return;												
			lambda += lambdaStep;			
		}

		++date;
		lambda -= 1.f;

		// Start new segment and compute new tangents
		t0 = t1;
		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;
		t1 = 0.5f * (*nextNextIt - *currIt);		
	}
}

//=======================================================	
void CPSRibbonBase::computeLinearRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{
	/// well we could avoid code duplication with computeHermitteRibbon, but this is simply easier than using templates for now...
	nlassert(!_Parametric);	
	TPosVect::iterator startIt = _Ribbons.begin() + (_NbSegs + 1 + EndRibbonStorage) * index;
	TPosVect::iterator endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	TPosVect::iterator currIt  = startIt + _RibbonIndex;	
	TPosVect::iterator nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	TPosVect::iterator nextNextIt = nextIt + 1;	
	if (nextNextIt == endIt) nextNextIt = startIt;	
	float *date = &_SamplingDate[0];				

	uint leftToDo = _NbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;

	for (;;)
	{		
		float dt = date[0] - date[1];

		if (dt < 10E-6f) // we reached the start of ribbon
		{

			do
			{
				*dest = *currIt;
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			}
			while (--leftToDo);			
			return;
		}

		float newLambdaStep = _UsedSegDuration / dt;
		// readapt lambda
		lambda *= newLambdaStep / lambdaStep;
		lambdaStep = newLambdaStep;

		float oneMinusLambda = 1.f - lambda;
		for(;;)
		{
			if (lambda >= 1.f) break;
			/// compute a location
			BuildLinearVector(*currIt, *nextIt, *dest, lambda, oneMinusLambda);
			dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			-- leftToDo;
			if (!leftToDo) return;												
			lambda += lambdaStep;
			oneMinusLambda -= lambdaStep;
		}

		++date;
		lambda -= 1.f;
		
		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;		
	}
}



//=======================================================	
void CPSRibbonBase::computeLinearCstSizeRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{	
	nlassert(!_Parametric);	
	TPosVect::iterator startIt = _Ribbons.begin() + (_NbSegs + 1 + EndRibbonStorage) * index;
	TPosVect::iterator endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	TPosVect::iterator currIt  = startIt + _RibbonIndex;	
	TPosVect::iterator firstIt = currIt;	
	TPosVect::iterator nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	TPosVect::iterator nextNextIt = nextIt + 1;	
	if (nextNextIt == endIt) nextNextIt = startIt;		

	uint leftToDo = _UsedNbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;	

	
	/// Our goal here is to match the length of the ribbon, But if it isn't moving fast enough, we must truncate it
	for (;;)
	{	
		/// compute length between the 2 sampling points
		const float sampleLength = (*nextIt - *currIt).norm();		
		if (sampleLength > 10E-6f)
		{					
			/// compute lambda so that it match the length needed for each segment
			float newLambdaStep = _UsedSegLength / sampleLength;
			// readapt lambda
			lambda *= newLambdaStep / lambdaStep;
			lambdaStep = newLambdaStep;

			float oneMinusLambda = 1.f - lambda;
			for(;;)
			{
				if (lambda >= 1.f) break;
				/// compute a location
				BuildLinearVector(*currIt, *nextIt, *dest, lambda, oneMinusLambda);
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
				-- leftToDo;
				if (!leftToDo) return;												
				lambda += lambdaStep;
				oneMinusLambda -= lambdaStep;
			}
			lambda -= 1.f;
		}
		
		/// go to next sampling pos
		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;
		if (nextNextIt == firstIt)
		{
			// The length of the sampling curve is too short
			// must truncate the ribbon.
			NLMISC::CVector &toDup = *nextIt;
			while (leftToDo --)
			{
				*dest = toDup;
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			}
			return;
		}
	}
}

//=======================================================	
void CPSRibbonBase::computeHermitteCstSizeRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{	
	nlassert(!_Parametric);	
	TPosVect::iterator startIt = _Ribbons.begin() + (_NbSegs + 1 + EndRibbonStorage) * index;
	TPosVect::iterator endIt   = startIt + (_NbSegs + 1 + EndRibbonStorage);
	TPosVect::iterator currIt  = startIt + _RibbonIndex;	
	TPosVect::iterator firstIt = currIt;	
	TPosVect::iterator nextIt  = currIt + 1;
	if (nextIt == endIt) nextIt = startIt;
	TPosVect::iterator nextNextIt = nextIt + 1;	
	if (nextNextIt == endIt) nextNextIt = startIt;

	NLMISC::CVector t0 = (*currIt - *nextIt);
	NLMISC::CVector t1 = 0.5f * (*nextNextIt - *currIt);

	uint leftToDo = _UsedNbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;	

	
	/// Our goal here is to match the length of the ribbon, But if it isn't moving fast enough, we must truncate it
	/// Having a constant speed over a hermite curve is expensive, so we make a (very) rough approximation...
	for (;;)
	{	
		/// compute length between the 2 sampling points
		const float sampleLength = (*nextIt - *currIt).norm();		
		if (sampleLength > 10E-6f)
		{					
			/// compute lambda so that it match the length needed for each segment
			float newLambdaStep = _UsedSegLength / sampleLength;
			// readapt lambda
			lambda *= newLambdaStep / lambdaStep;
			lambdaStep = newLambdaStep;
			
			for(;;)
			{
				if (lambda >= 1.f) break;
				/// compute a location
				BuildHermiteVector(*currIt, *nextIt, t0, t1, *dest, lambda);
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
				-- leftToDo;
				if (!leftToDo) return;												
				lambda += lambdaStep;				
			}
			lambda -= 1.f;
		}
		
		/// go to next sampling pos
		currIt = nextIt;
		nextIt = nextNextIt;
		++nextNextIt;
		if (nextNextIt == endIt) nextNextIt = startIt;
		if (nextNextIt == firstIt)
		{
			// The length of the sampling curve is too short
			// must truncate the ribbon.
			NLMISC::CVector &toDup = *nextIt;
			while (leftToDo --)
			{
				*dest = toDup;
				dest  = (NLMISC::CVector *) ((uint8 *) dest + stride);
			}
			return;
		}
		/// update tangents
		t0 = t1;
		t1 = 0.5f * (*nextNextIt - *currIt);
	}
}


//=======================================================	
inline void CPSRibbonBase::computeRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
{
	switch (_InterpolationMode)
	{
		case Linear:
			if (_RibbonMode == VariableSize)
			{
				computeLinearRibbon(index, dest, stride);
			}
			else
			{
				computeLinearCstSizeRibbon(index, dest, stride);
			}
		break;
		case Hermitte:
			if (_RibbonMode == VariableSize)
			{
				computeHermitteRibbon(index, dest, stride);	
			}
			else
			{
				computeHermitteCstSizeRibbon(index, dest, stride);	
			}
		break;
		default:
			nlassert(0);
		break;
	}
}


//=======================================================	
void	CPSRibbonBase::dupRibbon(uint dest, uint src)
{
	nlassert(!_Parametric);
	nlassert(_Owner);
	const uint size = _Owner->getSize();
	nlassert(dest < size && src < size);
	::memcpy(&_Ribbons[dest * (_NbSegs + EndRibbonStorage + 1)], &_Ribbons[src * (_NbSegs + EndRibbonStorage + 1)], sizeof(NLMISC::CVector) * (_NbSegs + 1 + EndRibbonStorage));
}

//=======================================================	
void	CPSRibbonBase::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	if (_Parametric) return;
	/// dump the same pos for all pos of the ribbon
	const uint index = _Owner->getNewElementIndex();
	const NLMISC::CVector &pos = _Owner->getPos()[index]; // get the pos of the new element;
	resetSingleRibbon(index, pos);
}

//=======================================================	
void	CPSRibbonBase::deleteElement(uint32 index)
{
	if (_Parametric) return;
	const uint32 size = _Owner->getSize();
	if(index == (size - 1)) return; // was the last element, no permutation needed.
	dupRibbon(index, size - 1);
}

//=======================================================	
void	CPSRibbonBase::resize(uint32 size)
{
	if (_Parametric) return;
	_Ribbons.resize(size * (_NbSegs + 1 + EndRibbonStorage));
	resetFromOwner();
}


//=======================================================	
void CPSRibbonBase::resetSingleRibbon(uint index, const NLMISC::CVector &pos)
{
	nlassert(!_Parametric);
	TPosVect::iterator it = _Ribbons.begin() + (index * (_NbSegs + 1 + EndRibbonStorage));
	std::fill(it, it + (_NbSegs + 1 + EndRibbonStorage), pos);
}



//=======================================================	
void CPSRibbonBase::resetFromOwner()
{
	nlassert(!_Parametric);
	const uint32 size = _Owner->getSize();
	TPSAttribVector::iterator posIt = _Owner->getPos().begin();
	TPSAttribVector::iterator endPosIt = _Owner->getPos().end();
	for (uint k = 0; posIt != endPosIt; ++posIt, ++k)
	{
		resetSingleRibbon(k, *posIt);
	}
}

//=======================================================	
void CPSRibbonBase::motionTypeChanged(bool parametric)
{
	_Parametric = parametric;
	if (parametric)
	{
		NLMISC::contReset(_Ribbons); // kill the vector		
	}
	else
	{
		nlassert(_Owner);
		resize(_Owner->getMaxSize());
		initDateVect();		
		resetFromOwner();		
	}	
}


//=======================================================	
void CPSRibbonBase::initDateVect()
{
	_SamplingDate.resize( _NbSegs + 1 + EndRibbonStorage);
	std::fill(_SamplingDate.begin(), _SamplingDate.begin() + (_NbSegs + 1 + EndRibbonStorage), 0.f);
}


//=======================================================	
void CPSRibbonBase::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSParticle::serial(f);
	sint ver = f.serialVersion(1);
	f.serialEnum(_RibbonMode);
	f.serialEnum(_InterpolationMode);
	f.serial(_NbSegs, _SegDuration);	
	if (_RibbonMode == FixedSize)
	{
		f.serial(_RibbonLength);
		if (f.isReading())
		{
			_SegLength = _RibbonLength / _NbSegs;
		}
	}

	if (f.isReading())
	{
		nlassert(_Owner);
		resize(_Owner->getMaxSize());
		initDateVect();		
		resetFromOwner();		
	}

	if (ver >= 1)
	{
		f.serial(_LODDegradation);
	}
}


//=======================================================	
void CPSRibbonBase::updateLOD()
{
	nlassert(_Owner)
	float ratio = _Owner->getOwner()->getOneMinusCurrentLODRatio();
	float lodRatio = _LODDegradation + (1.f - _LODDegradation ) * ratio * ratio;
	
	_UsedNbSegs = (uint) (_NbSegs * lodRatio);
	NLMISC::clamp(_UsedNbSegs, 0u, _NbSegs);
	const float epsilon = 10E-4f;	
	_UsedSegDuration =  _SegDuration / std::max(epsilon, lodRatio);
	_UsedSegLength   =  _SegLength / std::max(epsilon, lodRatio);

}

////////////////////////////////////
// CPSRibbonLookAt implementation //
////////////////////////////////////

const float ZEpsilon = 10E-3f;
const float NormEpsilon = 10E-8f;


struct CVectInfo
{
	NLMISC::CVector Interp;
	NLMISC::CVector Proj;	
};
typedef std::vector<CVectInfo> TRibbonVect; // a vector used for intermediate computations

CPSRibbonLookAt::TVBMap		CPSRibbonLookAt::_VBMap;			// index buffers with no color
CPSRibbonLookAt::TVBMap		CPSRibbonLookAt::_ColoredVBMap;  // index buffer + colors	

//=======================================================	
CPSRibbonLookAt::CPSRibbonLookAt()
{
}

//=======================================================	
CPSRibbonLookAt::~CPSRibbonLookAt()
{
//	delete _DyingRibbons;
}

//=======================================================	
void CPSRibbonLookAt::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/** Version 4 : added CPSRibbonBase has a base class instead of CPSParticle
	  * 
	  */
	sint ver = f.serialVersion(4);
	if (ver > 3)
	{
		CPSRibbonBase::serial(f);
	}
	else
	{
		CPSParticle::serial(f);
	}
	CPSColoredParticle::serialColorScheme(f);	
	CPSSizedParticle::serialSizeScheme(f);		
	serialMaterial(f);
	uint32 dummy = 0; /* _NbDyingRibbons */
	if (ver <= 3)
	{
		f.serial(_SegDuration, _NbSegs, dummy /*_NbDyingRibbons*/);
	}
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


//=======================================================	
void CPSRibbonLookAt::setTexture(CSmartPtr<ITexture> tex)
{
	_Tex = tex;
	updateMatAndVbForColor();
}


//=======================================================	
void CPSRibbonLookAt::step(TPSProcessPass pass, TAnimationTime ellapsedTime)
{	
	if (pass == PSPostdynamic) // performs motion by using the speed obtained after forces integration
	{		
		updateGlobals();		
	}
	else
	if (
		(pass == PSBlendRender && hasTransparentFaces())
		|| (pass == PSSolidRender && hasOpaqueFaces())
		)
	{
		nlassert(_Owner);		
		displayRibbons(_Owner->getSize());
	//	if (_DyingRibbons) displayRibbons(*_DyingRibbons, _NbDyingRibbons);
	}
	else 
	if (pass == PSToolRender) // edition mode only
	{			
		//showTool();
	}	
}


//=======================================================	
void CPSRibbonLookAt::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	CPSRibbonBase::newElement(emitterLocated, emitterIndex);
	newColorElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);	
}


//=======================================================	
void CPSRibbonLookAt::deleteElement(uint32 index)
{
	CPSRibbonBase::deleteElement(index);
	deleteColorElement(index);
	deleteSizeElement(index);	
}


//=======================================================	
void CPSRibbonLookAt::resize(uint32 size)
{
	CPSRibbonBase::resize(size);	
	resizeColor(size);
	resizeSize(size);			
}

//=======================================================	
void CPSRibbonLookAt::updateMatAndVbForColor(void)
{
	_Mat.setTexture(0, _Tex);
}

//=======================================================	
static inline void MakeProj(NLMISC::CVector &dest, const NLMISC::CVector &src)
{		
	if (fabsf(src.y) > NormEpsilon * NormEpsilon)
	{
		dest.x = src.x / src.y;
		dest.z = src.z / src.y;
		dest.y = src.y;	
	}	
}

static inline void BuildSlice(const NLMISC::CMatrix &mat, CVertexBuffer &vb, uint8 *currVert, uint32 vertexSize,
							  const NLMISC::CVector &I, 
							  const NLMISC::CVector &K, 
  							  TRibbonVect::iterator  pos,
							  TRibbonVect::iterator  prev,
							  TRibbonVect::iterator  next,
							  float ribSize)
/// TODO: some optimisation to get a better speed
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


//==========================================================================	
void CPSRibbonLookAt::displayRibbons(uint32 nbRibbons)
{	
	if (!nbRibbons) return;
	nlassert(_Owner);	
	CPSRibbonBase::updateLOD();
	if (_UsedNbSegs < 2) return;

	const float date = _Owner->getOwner()->getSystemDate();
	uint8						*currVert;

	CVBnPB						&VBnPB = getVBnPB(); // get the appropriate vb (build it if needed)
	CVertexBuffer				&VB = VBnPB.VB;
	CPrimitiveBlock				&PB = VBnPB.PB;

	const uint32				vertexSize  = VB.getVertexSize();
	uint						colorOffset;
	const uint32				vertexSizeX2  = vertexSize << 1;
	const NLMISC::CVector       I = _Owner->computeI();
	const NLMISC::CVector       K = _Owner->computeK();

	
	CMatrix mat =  _Owner->isInSystemBasis() ? getViewMat()  *  getSysMat()
																  : getViewMat();
	IDriver *drv = this->getDriver();
	setupDriverModelMatrix();
	drv->activeVertexBuffer(VB);
	_Owner->incrementNbDrawnParticles(nbRibbons); // for benchmark purpose	
	
	const uint numRibbonBatch = getNumRibbonsInVB(); // number of ribons to process at once
	

	static TRibbonVect				   currRibbon;
	static std::vector<float>		   sizes;
	static std::vector<NLMISC::CRGBA>  colors;


	
	

	if (_UsedNbSegs == 0) return;

	currRibbon.resize(_UsedNbSegs + 1);
	sizes.resize(numRibbonBatch);
	if (_ColorScheme)
	{
		colorOffset = VB.getColorOff();
		colors.resize(numRibbonBatch);
		_Mat.setColor(NLMISC::CRGBA::White);
	}
	else
	{
		_Mat.setColor(_Color);
	}
	
	uint toProcess;
	uint ribbonIndex = 0; // index of the first ribbon in the batch being processed

	do
	{
		toProcess = std::min((uint) (nbRibbons - ribbonIndex) /* = left to do */, numRibbonBatch);
	

		/// setup sizes
		const float	*ptCurrSize;
		uint32  ptCurrSizeIncrement;
		if (_SizeScheme)
		{			
			ptCurrSize = (float *) _SizeScheme->make(this->_Owner, ribbonIndex, &sizes[0], sizeof(float), toProcess, true);			
			ptCurrSizeIncrement = 1;
		}
		else
		{
			ptCurrSize = &_ParticleSize;
			ptCurrSizeIncrement = 0;
		}

		/// setup colors
		NLMISC::CRGBA	*ptCurrColor;		
		if (_ColorScheme)
		{
			colors.resize(nbRibbons);
			ptCurrColor = (NLMISC::CRGBA *) _ColorScheme->make(this->_Owner, ribbonIndex, &colors[0], sizeof(NLMISC::CRGBA), toProcess, true);			
		}		
		
		currVert = (uint8 *) VB.getVertexCoordPointer();
		for (uint k = ribbonIndex; k < ribbonIndex + toProcess; ++k)
		{			
			
			TRibbonVect::iterator rIt = currRibbon.begin(), rItEnd = currRibbon.end(), rItEndMinusOne = rItEnd - 1;

			////////////////////////////////////
			// interpolate and project points //
			////////////////////////////////////
		
				if (!_Parametric)
				{

					//////////////////////
					// INCREMENTAL CASE //
					//////////////////////

					// the parent class has a method to get the ribbons positions
					computeRibbon(k, &rIt->Interp, sizeof(CVectInfo));				
					do
					{					
						MakeProj(rIt->Proj, mat * rIt->Interp); 
						++rIt;				
					}
					while (rIt != rItEnd);			
				}
				else
				{
					//////////////////////
					// PARAMETRIC  CASE //
					//////////////////////
					// we compute each pos thanks to the parametric curve				
					_Owner->integrateSingle(date - _UsedSegDuration * (_UsedNbSegs + 1), _UsedSegDuration, _UsedNbSegs + 1, k,
											 &rIt->Interp, sizeof(CVectInfo) );				
					// project each position now
					do
					{					
						MakeProj(rIt->Proj, mat * rIt->Interp); 
						++rIt;				
					}
					while (rIt != rItEnd);			
				}

				rIt = currRibbon.begin();

		
				// setup colors
				if (_ColorScheme)
				{
					uint8 *currColVertex = currVert + colorOffset;
					uint colCount = (_UsedNbSegs + 1) << 1;
					do
					{
						* (CRGBA *) currColVertex = *ptCurrColor;
						currColVertex += vertexSize;
					}
					while (--colCount);

					++ptCurrColor;			
				}

				/// build the ribbon in vb				
				// deals with first point				
				BuildSlice(mat, VB, currVert, vertexSize, I, K, rIt, rIt, rIt + 1, *ptCurrSize);				
				currVert += vertexSizeX2;		
				++rIt;

				
				// deals with other points				
				for (;;) // we assume at least 2 segments, so we must have a middle point		
				{						
					// build 2 vertices with the right tangent. /* to project 2 */ is old projected point
					BuildSlice(mat, VB, currVert, vertexSize, I, K, rIt, rIt - 1, rIt + 1, *ptCurrSize); 
					// next position		
					++rIt;
					if (rIt == rItEndMinusOne) break;					
					// next vertex			
					currVert += vertexSizeX2;		
				}
				currVert += vertexSizeX2;
				// last point.
				BuildSlice(mat, VB, currVert, vertexSize, I, K, rIt , rIt - 1, rIt, *ptCurrSize);										
				ptCurrSize += ptCurrSizeIncrement;
				currVert += vertexSizeX2;
		}

		PB.setNumTri((_UsedNbSegs << 1) * toProcess);
		// display the result
		drv->render(PB, _Mat);
		ribbonIndex += toProcess;		
	}
	while (ribbonIndex != nbRibbons);
}	

//==========================================================================	
bool CPSRibbonLookAt::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}


//==========================================================================	
bool CPSRibbonLookAt::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

//==========================================================================	
uint32 CPSRibbonLookAt::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize() * _NbSegs * 2;	
}



//==========================================================================	
CPSRibbonLookAt::CVBnPB &CPSRibbonLookAt::getVBnPB()
{
	TVBMap &map = _ColorScheme ? _VBMap : _ColoredVBMap;	
	TVBMap::iterator it = map.find(_UsedNbSegs + 1);
	if (it != map.end())
	{
		return it->second;
	}
	else	// must create this vb
	{
		const uint numRibbonInVB = getNumRibbonsInVB();
		CVBnPB &VBnPB = map[_UsedNbSegs + 1]; // make an entry

		/// set the vb format & size
		CVertexBuffer &vb = VBnPB.VB;
		vb.setVertexFormat(CVertexBuffer::PositionFlag |
						   CVertexBuffer::TexCoord0Flag | 
						   (_ColorScheme ? CVertexBuffer::PrimaryColorFlag : 0));
		vb.setNumVertices(2 * (_UsedNbSegs + 1) * numRibbonInVB );

		// set the primitive block size
		CPrimitiveBlock &pb = VBnPB.PB;
		pb.setNumTri((_UsedNbSegs << 1) * numRibbonInVB);
		/// Setup the pb and vb parts. Not very fast but executed only once
		uint vbIndex = 0;
		uint pbIndex = 0; 
		for (uint i = 0; i < numRibbonInVB; ++i)
		{
			for (uint k = 0; k < (_UsedNbSegs + 1); ++k)
			{				
				vb.setTexCoord(vbIndex, 0, CUV((1.f - k / (float) _UsedNbSegs), 0)); /// top vertex
				vb.setTexCoord(vbIndex + 1, 0, CUV((1.f - k / (float) _UsedNbSegs), 1)); /// bottom vertex
				if (k != _UsedNbSegs)
				{
					/// add 2 tri in the primitive block
					pb.setTri(pbIndex ++, vbIndex + 1, vbIndex + 2, vbIndex);
					pb.setTri(pbIndex ++, vbIndex + 1, vbIndex + 3, vbIndex + 2);
				}
				vbIndex += 2;
			}
		}
		return VBnPB;
	}
}

//==========================================================================	
uint	CPSRibbonLookAt::getNumRibbonsInVB() const
{
	/// approximation of the max number of vertices we want in a vb
	const uint vertexInVB = 256;	
	return std::max(1u, (uint) (vertexInVB / (_UsedNbSegs + 1)));
}

} // NL3D
