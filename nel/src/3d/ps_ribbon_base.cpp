/** \file ps_ribbon_base.cpp
 * Base class for (some) ribbons.
 *
 * $Id: ps_ribbon_base.cpp,v 1.2 2002/02/21 17:35:47 vizerie Exp $
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

#include "3d/ps_ribbon_base.h"
#include "3d/particle_system.h"

namespace NL3D 
{

////////////////////////////////////
//  CPSRibbonBase implementation  //
////////////////////////////////////


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


const uint EndRibbonStorage = 1;


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
void CPSRibbonBase::computeRibbon(uint index, NLMISC::CVector *dest, uint stride /* = sizeof(NLMISC::CVector)*/)
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
	nlassert(size < (1 << 16));
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
	float squaredRatio = ratio * ratio;
	float lodRatio = _LODDegradation + (1.f - _LODDegradation ) * squaredRatio * squaredRatio * squaredRatio;
	
	_UsedNbSegs = (uint) (_NbSegs * lodRatio);
	NLMISC::clamp(_UsedNbSegs, 0u, _NbSegs);
	const float epsilon = 10E-4f;	
	_UsedSegDuration =  _SegDuration / std::max(epsilon, lodRatio);
	_UsedSegLength   =  _SegLength / std::max(epsilon, lodRatio);

}


} // NL3D
