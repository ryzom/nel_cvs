/** \file ps_emitter.cpp
 * <File description>
 *
 * $Id: ps_emitter.cpp,v 1.51 2003/11/04 09:42:06 vizerie Exp $
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

#include <stdlib.h>
#include "3d/ps_emitter.h"
#include "3d/material.h"
#include "nel/misc/line.h"
#include "3d/dru.h"
#include "3d/particle_system.h"

namespace NL3D {




static const uint  EMITTER_BUFF_SIZE = 512;			   // number of emitter to be processed at once
static const float EMIT_PERIOD_THRESHOLD = 1.f / 75.f; // assuming the same behaviour than with a 75 hz rendering
bool CPSEmitter::_BypassEmitOnDeath = false;


//////////////////////
// STATIC FUNCTIONS //
//////////////////////
/** In an arrey of float, all value that are 0.f are replaced by EMIT_PERIOD_THRESHOLD
  * A period of 0 is allowed for emitter and means "emit at each frame"
  * This is deprecated now, and this helps to avoid that behaviour
  */
static void replaceNullPeriodsByThreshold(float *tab, uint numElem)
{
	const float *endTab = tab + numElem;
	while (tab != endTab)
	{
		if (*tab == 0.f) *tab = EMIT_PERIOD_THRESHOLD;
		++ tab;
	}
}




///////////////////////////////
// CPSEmitter implementation //
///////////////////////////////
CPSEmitter::CPSEmitter() : _EmittedType(NULL),
						   _SpeedInheritanceFactor(0.f),
						   _EmissionType(regular),
						   _Period(0.02f),
						   _PeriodScheme(NULL), 
						   _GenNb(1), 
						   _GenNbScheme(NULL), 
						   _EmitDelay(0),
						   _MaxEmissionCount(0),
						   _SpeedBasisEmission(false),
						   _EmitDirBasis(true),
						   _ConsistentEmission(true),
						   _BypassAutoLOD(false)
{
}


///==========================================================================
CPSEmitter::~CPSEmitter()
{
	delete _PeriodScheme;
	delete _GenNbScheme;
	// if a located is emitted, unregister us as an observer
	if (_EmittedType)
	{
		_EmittedType->unregisterDtorObserver(this);
	}
}

///==========================================================================
void CPSEmitter::releaseRefTo(const CParticleSystemProcess *other)
{
	if (_EmittedType == other)
	{
		setEmittedType(NULL);
	}
}

void CPSEmitter::releaseAllRef()
{
	setEmittedType(NULL);
}


///==========================================================================
void CPSEmitter::setOwner(CPSLocated *psl)
{
	CPSLocatedBindable::setOwner(psl);
	updateMaxCountVect();
}


///==========================================================================
inline void CPSEmitter::processEmit(uint32 index, sint nbToGenerate)
{	
	static NLMISC::CVector speed, pos;
	
	if (!_SpeedBasisEmission)
	{
		if (_SpeedInheritanceFactor == 0.f)
		{		
			if (_EmitDirBasis)
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(_Owner->getPos()[index], index, pos, speed);
					_EmittedType->newElement(pos, speed, this->_Owner, index);
				}
			}
			else
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(_Owner->getPos()[index], index, pos, speed);
					_EmittedType->newElement(pos, speed, this->_Owner, index, false);
				}
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->newElement(pos, speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this->_Owner);
			}
		}
	}
	else
	{
		NLMISC::CMatrix m;
		CPSUtil::buildSchmidtBasis(_Owner->getSpeed()[index], m);
		if (_SpeedInheritanceFactor == 0.f)
		{		
			while (nbToGenerate > 0)
			{
				nbToGenerate --;
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->newElement(pos, m * speed, this->_Owner, index);
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(_Owner->getPos()[index], index, pos, speed);
				_EmittedType->newElement(pos, m * speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this->_Owner, index);
			}
		}
	}
}

/** Compensate  particle position and age by using the given deltaT. We need this in the case of low-framerate
  * to get correct behaviour. Note that this can be deactivated (speed-quality tradeoff)
  */
static inline void CompensateEmission(CPSLocated *emittedType, uint emittedIndex, TAnimationTime deltaT, TAnimationTime ellapsedTime, float realEllapsedTimeRatio)
{	
	// now, update emitted element life time	
	if (emittedType->getLastForever())
	{		
		emittedType->getTime()[emittedIndex] += realEllapsedTimeRatio * deltaT;

		if (emittedType->isParametricMotionEnabled())
		{
			emittedType->getParametricInfos()[emittedIndex].Date -= realEllapsedTimeRatio * deltaT;
		}
		else
		{
			if (!emittedType->hasCollisionInfos())
			{
				// no collision case
				emittedType->getPos()[emittedIndex] += deltaT * emittedType->getSpeed()[emittedIndex];
			}
			else // if there are collisions, we delay the step to the next iteration to ensure correct collisions
			{
				emittedType->getCollisionInfo()[emittedIndex].TimeSliceRatio = deltaT / ellapsedTime;				
			}
		}
	}
	else
	{		
		TAnimationTime &destTime = emittedType->getTime()[emittedIndex];
		if (emittedType->getLifeScheme() != NULL)
		{
			destTime = realEllapsedTimeRatio * deltaT * emittedType->getTimeIncrement()[emittedIndex];
		}
		else
		{				
			if (emittedType->getInitialLife() != 0.f)
			{
				destTime = realEllapsedTimeRatio * deltaT / emittedType->getInitialLife();
			}
		}
		if (destTime >= 1.f)
		{					
			emittedType->deleteElement(emittedIndex);			
		}
		else
		{	
			if (emittedType->isParametricMotionEnabled())
			{
				emittedType->getParametricInfos()[emittedIndex].Date -= realEllapsedTimeRatio * deltaT;
			}
			else
			{
				if (!emittedType->hasCollisionInfos())
				{
					// no collision case
					emittedType->getPos()[emittedIndex] += deltaT * emittedType->getSpeed()[emittedIndex];
				}
				else // if there are collisions, we delay the step to the next iteration to ensure correct collisions
				{
					emittedType->getCollisionInfo()[emittedIndex].TimeSliceRatio = deltaT / ellapsedTime;				
				}
			}
		}
	}
}

///==========================================================================
inline void CPSEmitter::processEmitConsistent(const NLMISC::CVector &emitterPos,
											  uint32 index,
											  sint nbToGenerate,
											  TAnimationTime deltaT,
											  TAnimationTime ellapsedTime,
											  float realEllapsedTimeRatio
											 )
{		
	static NLMISC::CVector speed, pos; /// speed and pos of emittee
	sint emittedIndex;
	if (!_SpeedBasisEmission)
	{
		if (_SpeedInheritanceFactor == 0.f)
		{		
			if (_EmitDirBasis)
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(emitterPos, index, pos, speed);
					emittedIndex = _EmittedType->newElement(pos, speed, this->_Owner, index, true, deltaT);
					if (emittedIndex != - 1) CompensateEmission(_EmittedType, emittedIndex, deltaT, ellapsedTime, realEllapsedTimeRatio);
					else break;
				}
			}
			else
			{
				while (nbToGenerate > 0)
				{
					nbToGenerate --;
					emit(emitterPos, index, pos, speed);
					emittedIndex = _EmittedType->newElement(pos, speed, this->_Owner, index, false, deltaT);
					if (emittedIndex != - 1) CompensateEmission(_EmittedType, emittedIndex, deltaT, ellapsedTime, realEllapsedTimeRatio);
					else break;
				}
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(emitterPos, index, pos, speed);
				emittedIndex = _EmittedType->newElement(pos, speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this->_Owner, index, true, deltaT);
				if (emittedIndex != - 1) CompensateEmission(_EmittedType, emittedIndex, deltaT, ellapsedTime, realEllapsedTimeRatio);
					else break;
			}
		}
	}
	else
	{
		NLMISC::CMatrix m;
		CPSUtil::buildSchmidtBasis(_Owner->getSpeed()[index], m);
		if (_SpeedInheritanceFactor == 0.f)
		{		
			while (nbToGenerate > 0)
			{
				nbToGenerate --;
				emit(emitterPos, index, pos, speed);
				emittedIndex = _EmittedType->newElement(pos, m * speed, this->_Owner, index, true, deltaT);
				if (emittedIndex != - 1) CompensateEmission(_EmittedType, emittedIndex, deltaT, ellapsedTime, realEllapsedTimeRatio);
					else break;
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(emitterPos, index, pos, speed);
				emittedIndex = _EmittedType->newElement(pos, m * speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this->_Owner, index, true, deltaT);
				if (emittedIndex != - 1) CompensateEmission(_EmittedType, emittedIndex, deltaT, ellapsedTime, realEllapsedTimeRatio);
					else break;
			}
		}
	}
}


///==========================================================================
bool	CPSEmitter::setEmissionType(TEmissionType freqType)
{ 
	if (_Owner && _Owner->getOwner())
	{
		CParticleSystem *ps = _Owner->getOwner();
		if (ps->getBypassMaxNumIntegrationSteps())
		{
			if (!_Owner)
			{
				nlwarning("<CPSEmitter::setEmissionType> The emitter should be inserted in a CPSLocated instance");
				nlassert(0);
			}
			// check if the new value is valid
			TEmissionType oldType = _EmissionType;
			_EmissionType = freqType;
			if (testEmitForever() == true)
			{
				_EmissionType = oldType;
				std::string mess = "<CPSEmitter::setEmissionType> can't set emission type to '" + 
					               NLMISC::toString(freqType) +
								  "' with the current configuration : the system has been flagged with \
								   'BypassMaxNumIntegrationSteps', and should have a finite duration.  \
								   The flag is not set";
				nlwarning(mess.c_str());
				return false;

			}			
		}
		ps->systemDurationChanged();
	}
	_EmissionType = freqType; 
	return true;
}

///==========================================================================
bool CPSEmitter::setEmittedType(CPSLocated *et) 
{	
	if (_EmittedType)
	{
		_EmittedType->unregisterDtorObserver(this);		
	}
	if (et)
	{
		et->registerDtorObserver(this);
	}	
	CPSLocated *oldType = _EmittedType;
	_EmittedType = et;
	if (_Owner && _Owner->getOwner())
	{
		CParticleSystem *ps = _Owner->getOwner();
		if (_EmittedType)
		{		
			if (ps->getBypassMaxNumIntegrationSteps())
			{
				if (!ps->canFinish())
				{
					setEmittedType(oldType);
					nlwarning("<CPSLocated::setEmittedType> Can't set new emitted type : this causes the system to last forever, and it has been flagged with 'BypassMaxNumIntegrationSteps'. New emitted type is not set");
					return false;
				}
			}
		}
		ps->systemDurationChanged();
	}
	return true;
}

///==========================================================================
void CPSEmitter::notifyTargetRemoved(CPSLocated *ptr)
{
	nlassert(ptr == _EmittedType && _EmittedType);
	setEmittedType(NULL);	
}

///==========================================================================
void CPSEmitter::setPeriod(float period)
{
	if (_PeriodScheme)
	{
		delete _PeriodScheme;
		_PeriodScheme = NULL;
	}
	_Period = period;
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->systemDurationChanged();
	}
}

///==========================================================================
void CPSEmitter::setPeriodScheme(CPSAttribMaker<float> *scheme)
{	
	delete _PeriodScheme;	
	_PeriodScheme = scheme;
	if (_Owner && scheme->hasMemory()) scheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->systemDurationChanged();
	}
}

///==========================================================================
void CPSEmitter::setGenNb(uint32 genNb)
{
	if (_GenNbScheme)
	{
		delete _GenNbScheme;
		_GenNbScheme = NULL;
	}
	_GenNb = genNb;	
}

///==========================================================================
void CPSEmitter::setGenNbScheme(CPSAttribMaker<uint32> *scheme)
{
	delete _GenNbScheme;	
	_GenNbScheme = scheme;
	if (_Owner && scheme->hasMemory()) scheme->resize(_Owner->getMaxSize(), _Owner->getSize());
}

///==========================================================================
void CPSEmitter::showTool(void) 
{
	uint32 size = _Owner->getSize();
	if (!size) return;		
	setupDriverModelMatrix();	

	const CVector I = computeI();
	const CVector K = computeK();

	// ugly slow code, but not for runtime
	for (uint  k = 0; k < size; ++k)
	{
		// center of the current particle
		const CVector p = _Owner->getPos()[k];
		const float sSize =0.1f;	
		std::vector<NLMISC::CLine> lines;
		NLMISC::CLine l;
		l.V0 = p - sSize * I; l.V1 =  p + sSize * I; lines.push_back(l);
		l.V0 = p - sSize * K; l.V1 =  p + sSize * K; lines.push_back(l);
		l.V0 = p - sSize * (I + K); l.V1 = p + sSize * (I + K); lines.push_back(l);
		l.V0 = p - sSize * (I - K); l.V1 = p + sSize * (I - K); lines.push_back(l);
																		
		CMaterial mat;
		mat.setBlendFunc(CMaterial::one, CMaterial::one);
		mat.setZWrite(false);
		mat.setLighting(false);
		mat.setBlend(true);
		mat.setZFunc(CMaterial::less);
		
	
		CPSLocated *loc;		
		uint32 index;
		CPSLocatedBindable *lb;
		_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);

		mat.setColor((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));
		

		CDRU::drawLinesUnlit(lines, mat, *getDriver() );
	}
}

///==========================================================================
void CPSEmitter::singleEmit(uint32 index, uint quantity)
{
	nlassert(_Owner);
	const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner,0) : _GenNb;		
	processEmit(index, quantity * nbToGenerate);
}



///==========================================================================
void CPSEmitter::processRegularEmissionWithNoLOD(TAnimationTime ellapsedTime)
{
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	//
	const bool emitThreshold = _Owner->getOwner()->isEmitThresholdEnabled();
	//
	const uint size = _Owner->getSize();
	uint leftToDo = size, toProcess;
	float emitPeriod[EMITTER_BUFF_SIZE]; 
	const float *currEmitPeriod;
	uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0;			
	sint32 nbToGenerate;

	TPSAttribTime::iterator phaseIt = _Phase.begin(), endPhaseIt; 
	TPSAttribUInt8::iterator numEmitIt = _NumEmission.begin(); 

	// we don't use an iterator here
	// because it could be invalidated if size change (a located could generate itself)	
	do
	{
		toProcess = leftToDo < EMITTER_BUFF_SIZE ? leftToDo : EMITTER_BUFF_SIZE;


		if (_PeriodScheme)
		{
			currEmitPeriod = (float *) (_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess, true));				
			if (emitThreshold)
			{
				
				/** Test if 'make' filled our buffer. If this is not the case, we assume that values where precomputed, and that
				  * all null period have already been replaced by the threshold
				  */
				if (currEmitPeriod == emitPeriod) 
				{
					// if there possibility to have 0 in the scheme ?
					if (_PeriodScheme->getMinValue() <= 0.f && _PeriodScheme->getMaxValue() >= 0.f)
					{											
						replaceNullPeriodsByThreshold(emitPeriod, toProcess);
					}
				}				
			}
		}
		else
		{
			if (_Period != 0.f || !emitThreshold)
			{			
				currEmitPeriod = &_Period;
			}
			else
			{
				currEmitPeriod = &EMIT_PERIOD_THRESHOLD;
			}
		}

		endPhaseIt = phaseIt + toProcess;

		if (_MaxEmissionCount == 0) // no emission count limit
		{
			/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do			
				{
					*phaseIt += ellapsedTime;
					if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
					{					
						if (*currEmitPeriod != 0)
						{
							*phaseIt -= ::floorf(*phaseIt / *currEmitPeriod) * *currEmitPeriod;
						}
						const uint32 k = phaseIt - (_Phase.begin());
						nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;					
						processEmit(k, nbToGenerate);										
					}	
					
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do			
				{
					*phaseIt += ellapsedTime;
					if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							*phaseIt -= ::floorf((*phaseIt - _EmitDelay)  / *currEmitPeriod) * *currEmitPeriod;
						}
						const uint32 k = phaseIt - (_Phase.begin());
						nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;					
						processEmit(k, nbToGenerate);										
					}	
					
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
		}
		else // there's an emission count limit
		{
				/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do			
				{
					if (*numEmitIt < _MaxEmissionCount)
					{
						*phaseIt += ellapsedTime;
						if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								*phaseIt -= ::floorf(*phaseIt / *currEmitPeriod) * *currEmitPeriod;
							}
							const uint32 k = phaseIt - (_Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;					
							processEmit(k, nbToGenerate);
							++*numEmitIt;
						}	
					}	
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;							
					++ numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do			
				{
					if (*numEmitIt < _MaxEmissionCount)
					{
						*phaseIt += ellapsedTime;
						if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								*phaseIt -= ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod) * *currEmitPeriod;
							}
							const uint32 k = phaseIt - (_Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;					
							processEmit(k, nbToGenerate);
							++*numEmitIt;
						}
					}							
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
		}

		leftToDo -= toProcess;
	}
	while (leftToDo);
}


///==========================================================================
void CPSEmitter::processRegularEmission(TAnimationTime ellapsedTime, float emitLOD)
{
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	//
	const bool emitThreshold = _Owner->getOwner()->isEmitThresholdEnabled();
	//
	const uint size = _Owner->getSize();
	uint leftToDo = size, toProcess;
	float emitPeriod[EMITTER_BUFF_SIZE]; 
	const float *currEmitPeriod;
	uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0;			
	sint32 nbToGenerate;

	TPSAttribTime::iterator phaseIt = _Phase.begin(), endPhaseIt; 
	TPSAttribUInt8::iterator numEmitIt = _NumEmission.begin(); 	

	float ellapsedTimeLOD = emitLOD * ellapsedTime;
	uint maxEmissionCountLOD = (uint8) (_MaxEmissionCount * emitLOD);
	maxEmissionCountLOD = std::max(1u, maxEmissionCountLOD);

	// we don't use an iterator here
	// because it could be invalidated if size change (a located could generate itself)	
	do
	{
		toProcess = leftToDo < EMITTER_BUFF_SIZE ? leftToDo : EMITTER_BUFF_SIZE;


		if (_PeriodScheme)
		{
			currEmitPeriod = (float *) (_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess, true));				
			if (emitThreshold)
			{
				
				/** Test if 'make' filled our buffer. If this is not the case, we assume that values where precomputed, and that
				  * all null period have already been replaced by the threshold
				  */
				if (currEmitPeriod == emitPeriod) 
				{
					// if there possibility to have 0 in the scheme ?
					if (_PeriodScheme->getMinValue() <= 0.f && _PeriodScheme->getMaxValue() >= 0.f)
					{											
						replaceNullPeriodsByThreshold(emitPeriod, toProcess);
					}
				}				
			}
		}
		else
		{
			if (_Period != 0.f || !emitThreshold)
			{			
				currEmitPeriod = &_Period;
			}
			else
			{
				currEmitPeriod = &EMIT_PERIOD_THRESHOLD;
			}
		}

		endPhaseIt = phaseIt + toProcess;

		if (_MaxEmissionCount == 0) // no emission count limit
		{
			/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do			
				{
					*phaseIt += ellapsedTimeLOD;
					if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
					{					
						if (*currEmitPeriod != 0)
						{
							*phaseIt -= ::floorf(*phaseIt / *currEmitPeriod) * *currEmitPeriod;
						}
						const uint32 k = phaseIt - (_Phase.begin());
						nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
						if (nbToGenerate)
						{						
							nbToGenerate = (sint32) (emitLOD * nbToGenerate);
							if (!nbToGenerate) nbToGenerate = 1;
							processEmit(k, nbToGenerate);
						}
					}	
					
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do			
				{
					if (*phaseIt < _EmitDelay)
					{					
						*phaseIt += ellapsedTime;
						if (*phaseIt < _EmitDelay)
						{
							++phaseIt;
							currEmitPeriod += currEmitPeriodPtrInc;
							continue;
						}
						else
						{
							*phaseIt = 	(*phaseIt - _EmitDelay) * emitLOD + _EmitDelay;
						}
					}
					else
					{
						*phaseIt += ellapsedTimeLOD;
					}

					if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
					{
						if (*currEmitPeriod != 0)
						{
							*phaseIt -= ::floorf((*phaseIt - _EmitDelay)  / *currEmitPeriod) * *currEmitPeriod;
						}
						const uint32 k = phaseIt - (_Phase.begin());
						nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;
						if (nbToGenerate)
						{						
							nbToGenerate = (sint32) (emitLOD * nbToGenerate);
							if (!nbToGenerate) nbToGenerate = 1;
							processEmit(k, nbToGenerate);
						}						
					}	
					
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
		}
		else // there's an emission count limit
		{
				/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do			
				{
					if (*numEmitIt < maxEmissionCountLOD)
					{
						*phaseIt += ellapsedTimeLOD;
						if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								*phaseIt -= ::floorf(*phaseIt / *currEmitPeriod) * *currEmitPeriod;
							}
							const uint32 k = phaseIt - (_Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;					
							if (nbToGenerate)
							{						
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								processEmit(k, nbToGenerate);
							}
							++*numEmitIt;
						}	
					}
					else
					{
						*numEmitIt = _MaxEmissionCount;
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;							
					++ numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do			
				{
					if (*numEmitIt < maxEmissionCountLOD)
					{
						if (*phaseIt < _EmitDelay)
						{					
							*phaseIt += ellapsedTime;
							if (*phaseIt < _EmitDelay)
							{
								++phaseIt;
								currEmitPeriod += currEmitPeriodPtrInc;
								++numEmitIt;
								currEmitPeriod += currEmitPeriodPtrInc;
								continue;
							}
							else
							{
								*phaseIt = 	(*phaseIt - _EmitDelay) * emitLOD + _EmitDelay;
							}
						}
						else
						{
							*phaseIt += ellapsedTimeLOD;
						}

						if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								*phaseIt -= ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod) * *currEmitPeriod;
							}
							const uint32 k = phaseIt - (_Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, k) : _GenNb;					
							if (nbToGenerate)
							{						
								nbToGenerate = (sint32) (nbToGenerate * emitLOD);
								if (!nbToGenerate) nbToGenerate = 1;
								processEmit(k, nbToGenerate);
							}
							++*numEmitIt;
						}
					}
					else
					{
						*numEmitIt = _MaxEmissionCount;
					}
					++phaseIt;
					currEmitPeriod += currEmitPeriodPtrInc;
					++numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
		}

		leftToDo -= toProcess;
	}
	while (leftToDo);
}

/// private : generate the various position of an emitter in the given tab for the given slice of time,
//  depending on wether its motion is parametric or incremental. This is used to create emittees at the right position

static inline uint GenEmitterPositions(CPSLocated *emitter,
									   CPSLocated *emittee,
									   uint emitterIndex,
									   uint numStep,
									   TAnimationTime deltaT, /* fraction of time needed to reach the first emission */
									   TAnimationTime step,
									   std::vector<NLMISC::CVector> &dest
									  )
{
	const uint toProcess = std::max(1U, std::min(numStep, (uint) emittee->getMaxSize()));	
	dest.resize(toProcess);
	
	
	if (!emitter->isParametricMotionEnabled()) // standard case : take current pos and integrate
	{
		if (toProcess == 1) // only one emission -> takes current pos
		{
			dest[0] = emitter->getPos()[emitterIndex] - deltaT * emitter->getSpeed()[emitterIndex];
		}
		else
		{
			std::vector<NLMISC::CVector>::iterator outIt = dest.end();
			std::vector<NLMISC::CVector>::iterator endIt = dest.begin();
			NLMISC::CVector pos = emitter->getPos()[emitterIndex] - deltaT * emitter->getSpeed()[emitterIndex];
			NLMISC::CVector speed = step * emitter->getSpeed()[emitterIndex];
			do
			{
				-- outIt;
				*outIt = pos;
				pos -= speed;				
			}
			while (outIt != endIt);
		}
	}
	else // compute parametric trajectory
	{
		emitter->integrateSingle(emitter->getOwner()->getSystemDate() - deltaT - step * toProcess,
								 step,
								 toProcess,									 
								 emitterIndex,
								 &dest[0]
								);				
	}
	
	return toProcess;
}


/** The same as GenEmitterPositions, but with LOD taken in account. 
  */
static inline uint GenEmitterPositionsWithLOD(CPSLocated *emitter,
									   CPSLocated *emittee,
									   uint emitterIndex,
									   uint numStep,
									   TAnimationTime deltaT, /* fraction of time needed to reach the first emission */
									   TAnimationTime step,
									   float invLODRatio,
									   std::vector<NLMISC::CVector> &dest
									  )
{
	const uint toProcess = std::max(1U, std::min(numStep, (uint) emittee->getMaxSize()));	
	dest.resize(toProcess);
	
	
	if (!emitter->isParametricMotionEnabled()) // standard case : take current pos and integrate
	{
		if (toProcess == 1) // only one emission -> takes current pos
		{
			dest[0] = emitter->getPos()[emitterIndex] - deltaT * emitter->getSpeed()[emitterIndex];
		}
		else
		{
			std::vector<NLMISC::CVector>::iterator outIt = dest.end();
			std::vector<NLMISC::CVector>::iterator endIt = dest.begin();
			NLMISC::CVector pos = emitter->getPos()[emitterIndex] - deltaT * emitter->getSpeed()[emitterIndex];
			NLMISC::CVector speed = step * invLODRatio * emitter->getSpeed()[emitterIndex];
			do
			{
				-- outIt;
				*outIt = pos;
				pos -= speed;				
			}
			while (outIt != endIt);
		}
	}
	else // compute parametric trajectory
	{
		emitter->integrateSingle(emitter->getOwner()->getSystemDate() - deltaT - step * toProcess,
								 step,
								 toProcess,									 
								 emitterIndex,
								 &dest[0]
								);				
	}
	
	return toProcess;
}

///==========================================================================
void CPSEmitter::processRegularEmissionConsistent(TAnimationTime ellapsedTime, float realEllapsedTimeRatio, float emitLOD, float inverseEmitLOD)
{
	/// hmm some code factorisation would do no harm, but we want to keep tests outside the loops as much as possible...

	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	//
	const bool emitThreshold = _Owner->getOwner()->isEmitThresholdEnabled();
	//


	static std::vector<NLMISC::CVector> emitterPositions; 
	// Positions for the emitter. They are computed by using a parametric trajectory or by using integration

	const uint size = _Owner->getSize();
	uint leftToDo = size, toProcess;
	float emitPeriod[EMITTER_BUFF_SIZE]; 
	const float *currEmitPeriod;
	uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0;			
	sint32 nbToGenerate;


	TPSAttribTime::iterator phaseIt = _Phase.begin(), endPhaseIt; 
	TPSAttribUInt8::iterator numEmitIt = _NumEmission.begin(); 

	float ellapsedTimeLOD = ellapsedTime * emitLOD;
	uint maxEmissionCountLOD = (uint8) (_MaxEmissionCount * emitLOD);
	maxEmissionCountLOD = std::max(1u, maxEmissionCountLOD);

	// we don't use an iterator here
	// because it could be invalidated if size change (a located could generate itself)	
	do
	{
		toProcess = leftToDo < EMITTER_BUFF_SIZE ? leftToDo : EMITTER_BUFF_SIZE;


		if (_PeriodScheme)
		{
			currEmitPeriod = (float *) (_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess, true));				
			if (emitThreshold)
			{
				
				/** Test if 'make' filled our buffer. If this is not the case, we assume that values where precomputed, and that
				  * all null period have already been replaced by the threshold
				  */
				if (currEmitPeriod == emitPeriod) 
				{
					// if there possibility to have 0 in the scheme ?
					if (_PeriodScheme->getMinValue() <= 0.f && _PeriodScheme->getMaxValue() >= 0.f)
					{											
						replaceNullPeriodsByThreshold(emitPeriod, toProcess);
					}
				}				
			}
		}
		else
		{
			if (_Period != 0.f || !emitThreshold)
			{			
				currEmitPeriod = &_Period;
			}
			else
			{
				currEmitPeriod = &EMIT_PERIOD_THRESHOLD;
			}
		}

		endPhaseIt = phaseIt + toProcess;

		if (_MaxEmissionCount == 0) // no emission count limit
		{
			/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do			
				{
					*phaseIt += ellapsedTimeLOD;
					if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
					{									
						if (*currEmitPeriod != 0)
						{		
							/** Must ensure phase is valid if period decrease over time
							  */
							*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTimeLOD);
							//
							/// compute the number of emissions
							uint numEmissions = (uint) ::floorf(*phaseIt / *currEmitPeriod);
							*phaseIt -= *currEmitPeriod * numEmissions;
														
							uint emitterIndex = phaseIt - _Phase.begin();
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							if (nbToGenerate)
							{
								float deltaT = std::max(0.f, *phaseIt);								

								/// compute the position of the emitter for the needed dates
								numEmissions = GenEmitterPositionsWithLOD(_Owner,
																		  _EmittedType,
																		  emitterIndex,
																		  numEmissions,
																		  deltaT,
																		  *currEmitPeriod,
																		  inverseEmitLOD,
																		  emitterPositions
																		 );

								/// process each emission at the right pos at the right date																				
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								uint k = numEmissions;							
								float deltaTInc = *currEmitPeriod * inverseEmitLOD;
								do
								{	
									--k;
									processEmitConsistent(emitterPositions[k],
														  emitterIndex,
														  nbToGenerate,
														  deltaT,
														  ellapsedTime,
														  realEllapsedTimeRatio);
									deltaT += deltaTInc;								
								}
								while (k);
							}
						}
						else
						{
							const uint32 emitterIndex = phaseIt - (_Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							if (nbToGenerate)
							{
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								processEmit(emitterIndex, nbToGenerate);
							}
						}
					}	
					
					++phaseIt;					
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
			else // thhere's an emission delay
			{
				do			
				{
					if (*phaseIt < _EmitDelay)
					{					
						*phaseIt += ellapsedTime;
						if (*phaseIt < _EmitDelay)
						{
							++phaseIt;
							currEmitPeriod += currEmitPeriodPtrInc;
							continue;
						}
						else
						{
							*phaseIt = 	(*phaseIt - _EmitDelay) * emitLOD + _EmitDelay;
						}
					}
					else
					{
						*phaseIt += ellapsedTimeLOD;
					}
										
					if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
					{						
						if (*currEmitPeriod != 0)
						{		
							/** Must ensure phase is valid if period decrease over time
							  */
							*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTimeLOD + _EmitDelay);
							//
							uint numEmissions = (uint) ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod);
							*phaseIt -= *currEmitPeriod * numEmissions;
							float deltaT = std::max(*phaseIt - _EmitDelay, 0.f);
							//nlassert(deltaT >= 0.f);
							/// process each emission at the right pos at the right date							
							uint emitterIndex = phaseIt - _Phase.begin();
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;																				
							if (nbToGenerate)
							{
								
								/// compute the position of the emitter for the needed date
								numEmissions = GenEmitterPositionsWithLOD(  _Owner,
																			_EmittedType,
																			emitterIndex,
																			numEmissions,
																			deltaT,
																			*currEmitPeriod,
																			inverseEmitLOD,
																			emitterPositions
																		   );
											
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								uint k = numEmissions;							
								float deltaTInc = *currEmitPeriod * inverseEmitLOD;
								do
								{	
									--k;
									processEmitConsistent(emitterPositions[k],
														  emitterIndex,
														  nbToGenerate,
														  deltaT,
														  ellapsedTime,
														  realEllapsedTimeRatio);
									deltaT += deltaTInc;								
								}
								while (k);
							}
						}
						else
						{		
							const uint32 emitterIndex = phaseIt - (_Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							if (nbToGenerate)
							{
								nbToGenerate = (sint32) (emitLOD * nbToGenerate);
								if (!nbToGenerate) nbToGenerate = 1;
								processEmit(emitterIndex, nbToGenerate);
							}
						}									
					}	
					
					++phaseIt;					
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
		}
		else // there's an emission count limit
		{
				/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do			
				{
					if (*numEmitIt < maxEmissionCountLOD)
					{
						*phaseIt += ellapsedTimeLOD;
						if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								/** Must ensure phase is valid if period decrease over time
								 */
								*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTimeLOD);
								//
								uint numEmissions = (uint) ::floorf(*phaseIt / *currEmitPeriod);
								*numEmitIt +=  numEmissions;								
								*phaseIt -= *currEmitPeriod * numEmissions;
								float deltaT = std::max(*phaseIt, 0.f);
								//nlassert(deltaT >= 0.f);
								uint emitterIndex = phaseIt - _Phase.begin();
								if (*numEmitIt > _MaxEmissionCount) // make sure we don't go over the emission limit
								{
									numEmissions -= *numEmitIt - _MaxEmissionCount;
									*numEmitIt = _MaxEmissionCount;
								}
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;								
								if (nbToGenerate)
								{
									/// compute the position of the emitter for the needed date
									numEmissions = GenEmitterPositionsWithLOD(_Owner,
																				_EmittedType,
																				emitterIndex,
																				numEmissions,
																				deltaT,
																				*currEmitPeriod,
																				inverseEmitLOD,
																				emitterPositions
																			 );
									uint k = numEmissions;								
									/// process each emission at the right pos at the right date
									nbToGenerate = (sint32) (emitLOD * nbToGenerate);
									if (!nbToGenerate) nbToGenerate = 1;
									float deltaTInc = *currEmitPeriod * inverseEmitLOD;
									do
									{
										--k;
										processEmitConsistent(emitterPositions[k],
															  emitterIndex,
															  nbToGenerate,
															  deltaT,
															  ellapsedTime,
															  realEllapsedTimeRatio);
										deltaT += deltaTInc;									
									}
									while (k);
								}
							}
							else
							{
								const uint32 emitterIndex = phaseIt - _Phase.begin();
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;								
								if (nbToGenerate)
								{
									nbToGenerate = (sint32) (emitLOD * nbToGenerate);
									if (!nbToGenerate) nbToGenerate = 1;
									processEmit(emitterIndex, nbToGenerate);								
									++*numEmitIt;
								}
							}
						}	
					}
					else
					{
						*numEmitIt = _MaxEmissionCount; // if the lod change, must ensure that the 
					}
					++phaseIt;					
					currEmitPeriod += currEmitPeriodPtrInc;							
					++ numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do			
				{
					if (*numEmitIt < maxEmissionCountLOD)
					{
						if (*phaseIt < _EmitDelay)
						{					
							*phaseIt += ellapsedTime;
							if (*phaseIt < _EmitDelay)
							{
								++phaseIt;					
								currEmitPeriod += currEmitPeriodPtrInc;
								++numEmitIt;
								continue;
							}
							else
							{
								*phaseIt = 	(*phaseIt - _EmitDelay) * emitLOD + _EmitDelay;
							}
						}
						else
						{
							*phaseIt += ellapsedTimeLOD;
						}
						//
						if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
						{							
							if (*currEmitPeriod != 0)
							{		
								/** Must ensure phase is valid if period decrease over time
								 */
								*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTimeLOD + _EmitDelay);
								//
								uint numEmissions = (uint) ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod);
								*numEmitIt +=  numEmissions;								
								*phaseIt -= *currEmitPeriod * numEmissions;
								float deltaT = std::max(*phaseIt - _EmitDelay, 0.f);
								//nlassert(deltaT >= 0.f);
								uint emitterIndex = phaseIt - _Phase.begin();
								if (*numEmitIt > _MaxEmissionCount) // make sure we don't go over the emission limit
								{
									numEmissions -= *numEmitIt - _MaxEmissionCount;
									*numEmitIt = _MaxEmissionCount;
								}
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;								
								if (nbToGenerate)
								{
									/// compute the position of the emitter for the needed date
									numEmissions = GenEmitterPositionsWithLOD(	_Owner,
																				_EmittedType,
																				emitterIndex,
																				numEmissions,
																				deltaT,
																				*currEmitPeriod,
																				inverseEmitLOD,
																				emitterPositions
																			 );
									uint k = numEmissions;							
									/// process each emission at the right pos at the right date
									nbToGenerate = (sint32) (emitLOD * nbToGenerate);
									if (!nbToGenerate) nbToGenerate = 1;
									float deltaTInc = *currEmitPeriod * inverseEmitLOD;
									do
									{	
										--k;
										processEmitConsistent(emitterPositions[k],
															  emitterIndex,
															  nbToGenerate,
															  deltaT,
															  ellapsedTime,
															  realEllapsedTimeRatio);
										deltaT += deltaTInc;									
									}
									while (k);
								}
							}
							else
							{
								const uint32 emitterIndex = phaseIt - (_Phase.begin());
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								if (nbToGenerate)
								{
									nbToGenerate = (sint32) (emitLOD * nbToGenerate);
									if (!nbToGenerate) nbToGenerate = 1;
									processEmit(emitterIndex, nbToGenerate);								
									++*numEmitIt;
								}
							}
						}
					}							
					else
					{
						*numEmitIt = _MaxEmissionCount; // if the lod change, must ensure that the 
					}
					++phaseIt;					
					currEmitPeriod += currEmitPeriodPtrInc;
					++numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
		}

		leftToDo -= toProcess;
	}
	while (leftToDo);
}

///==========================================================================
void CPSEmitter::processRegularEmissionConsistentWithNoLOD(TAnimationTime ellapsedTime, float realEllapsedTimeRatio)
{
	/// hmm some code factorisation would do no harm, but we want to keep tests outside the loops as much as possible...

	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	//
	const bool emitThreshold = _Owner->getOwner()->isEmitThresholdEnabled();
	//


	static std::vector<NLMISC::CVector> emitterPositions; 
	// Positions for the emitter. They are computed by using a parametric trajectory or by using integration

	const uint size = _Owner->getSize();
	uint leftToDo = size, toProcess;
	float emitPeriod[EMITTER_BUFF_SIZE]; 
	const float *currEmitPeriod;
	uint currEmitPeriodPtrInc = _PeriodScheme ? 1 : 0;			
	sint32 nbToGenerate;


	TPSAttribTime::iterator phaseIt = _Phase.begin(), endPhaseIt; 
	TPSAttribUInt8::iterator numEmitIt = _NumEmission.begin(); 

	// we don't use an iterator here
	// because it could be invalidated if size change (a located could generate itself)	
	do
	{
		toProcess = leftToDo < EMITTER_BUFF_SIZE ? leftToDo : EMITTER_BUFF_SIZE;


		if (_PeriodScheme)
		{
			currEmitPeriod = (float *) (_PeriodScheme->make(_Owner, size - leftToDo, emitPeriod, sizeof(float), toProcess, true));				
			if (emitThreshold)
			{
				
				/** Test if 'make' filled our buffer. If this is not the case, we assume that values where precomputed, and that
				  * all null period have already been replaced by the threshold
				  */
				if (currEmitPeriod == emitPeriod) 
				{
					// if there possibility to have 0 in the scheme ?
					if (_PeriodScheme->getMinValue() <= 0.f && _PeriodScheme->getMaxValue() >= 0.f)
					{											
						replaceNullPeriodsByThreshold(emitPeriod, toProcess);
					}
				}				
			}
		}
		else
		{
			if (_Period != 0.f || !emitThreshold)
			{			
				currEmitPeriod = &_Period;
			}
			else
			{
				currEmitPeriod = &EMIT_PERIOD_THRESHOLD;
			}
		}		

		endPhaseIt = phaseIt + toProcess;

		if (_MaxEmissionCount == 0) // no emission count limit
		{
			/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do			
				{
					*phaseIt += ellapsedTime;
					if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
					{									
						if (*currEmitPeriod != 0)
						{		
							/** Must ensure phase is valid if period decrease over time
							  */
							*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTime);
							//
							/// compute the number of emissions
							uint numEmissions = (uint) ::floorf(*phaseIt / *currEmitPeriod);
							*phaseIt -= *currEmitPeriod * numEmissions;
							float deltaT = std::max(0.f, *phaseIt);
							//nlassert(deltaT >= 0.f);
							uint emitterIndex = phaseIt - _Phase.begin();

							/// compute the position of the emitter for the needed dates
							numEmissions = GenEmitterPositions(_Owner,
																_EmittedType,
																emitterIndex,
																numEmissions,
																deltaT,
																*currEmitPeriod,
																emitterPositions
															   );

							/// process each emission at the right pos at the right date							
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							uint k = numEmissions;							
							do
							{	
								--k;
								processEmitConsistent(emitterPositions[k],
													  emitterIndex,
													  nbToGenerate,
													  deltaT,
													  ellapsedTime,
													  realEllapsedTimeRatio);
								deltaT += *currEmitPeriod;								
							}
							while (k);																	
						}
						else
						{
							const uint32 emitterIndex = phaseIt - (_Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							processEmit(emitterIndex, nbToGenerate);
						}
					}	
					
					++phaseIt;					
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
			else // thhere's an emission delay
			{
				do			
				{
					*phaseIt += ellapsedTime;
					if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
					{						
						if (*currEmitPeriod != 0)
						{		
							/** Must ensure phase is valid if period decrease over time
							  */
							*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTime + _EmitDelay);
							//
							uint numEmissions = (uint) ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod);
							*phaseIt -= *currEmitPeriod * numEmissions;
							float deltaT = std::max(*phaseIt - _EmitDelay, 0.f);
							//nlassert(deltaT >= 0.f);

							uint emitterIndex = phaseIt - _Phase.begin();
							/// compute the position of the emitter for the needed date
							numEmissions = GenEmitterPositions(_Owner,
										        _EmittedType,
												emitterIndex,
								                numEmissions,
												deltaT,
											    *currEmitPeriod,
											    emitterPositions
											   );
							/// process each emission at the right pos at the right date							
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;							
							uint k = numEmissions;							
							do
							{	
								--k;
								processEmitConsistent(emitterPositions[k],
													  emitterIndex,
													  nbToGenerate,
													  deltaT,
													  ellapsedTime,
													  realEllapsedTimeRatio);
								deltaT += *currEmitPeriod;								
							}
							while (k);													
						}
						else
						{		
							const uint32 emitterIndex = phaseIt - (_Phase.begin());
							nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
							processEmit(emitterIndex, nbToGenerate);							
						}									
					}	
					
					++phaseIt;					
					currEmitPeriod += currEmitPeriodPtrInc;
				}
				while (phaseIt != endPhaseIt);
			}
		}
		else // there's an emission count limit
		{
				/// is there an emission delay ?
			if (_EmitDelay == 0.f) // no emission delay
			{
				do			
				{
					if (*numEmitIt < _MaxEmissionCount)
					{
						*phaseIt += ellapsedTime;
						if ( *phaseIt >= *currEmitPeriod) // phase is greater than period -> must emit
						{
							if (*currEmitPeriod != 0)
							{
								/** Must ensure phase is valid if period decrease over time
								 */
								*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTime);
								//
								uint numEmissions = (uint) ::floorf(*phaseIt / *currEmitPeriod);
								*numEmitIt +=  numEmissions;								
								*phaseIt -= *currEmitPeriod * numEmissions;
								float deltaT = std::max(*phaseIt, 0.f);
								//nlassert(deltaT >= 0.f);
								uint emitterIndex = phaseIt - _Phase.begin();
								if (*numEmitIt > _MaxEmissionCount) // make sure we don't go over the emission limit
								{
									numEmissions -= *numEmitIt - _MaxEmissionCount;
									*numEmitIt = _MaxEmissionCount;
								}
								/// compute the position of the emitter for the needed date
								numEmissions = GenEmitterPositions(_Owner,
													_EmittedType,
													emitterIndex,
													numEmissions,
													deltaT,
													*currEmitPeriod,
													emitterPositions
												   );
								uint k = numEmissions;								
								/// process each emission at the right pos at the right date								
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;								
								do
								{
									--k;
									processEmitConsistent(emitterPositions[k],
														  emitterIndex,
														  nbToGenerate,
														  deltaT,
														  ellapsedTime,
														  realEllapsedTimeRatio);
									deltaT += *currEmitPeriod;									
								}
								while (k);														
							}
							else
							{
								const uint32 emitterIndex = phaseIt - _Phase.begin();
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								processEmit(emitterIndex, nbToGenerate);								
								++*numEmitIt;
							}
						}	
					}	
					++phaseIt;					
					currEmitPeriod += currEmitPeriodPtrInc;							
					++ numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
			else // there's an emission delay
			{
				do			
				{
					if (*numEmitIt < _MaxEmissionCount)
					{
						*phaseIt += ellapsedTime;
						if ( *phaseIt >= *currEmitPeriod + _EmitDelay) // phase is greater than period -> must emit
						{							
							if (*currEmitPeriod != 0)
							{		
								/** Must ensure phase is valid if period decrease over time
								 */
								*phaseIt = std::min(*phaseIt, *currEmitPeriod + ellapsedTime + _EmitDelay);
								//
								uint numEmissions = (uint) ::floorf((*phaseIt - _EmitDelay) / *currEmitPeriod);
								*numEmitIt +=  numEmissions;								
								*phaseIt -= *currEmitPeriod * numEmissions;
								float deltaT = std::max(*phaseIt - _EmitDelay, 0.f);
								//nlassert(deltaT >= 0.f);
								uint emitterIndex = phaseIt - _Phase.begin();
								if (*numEmitIt > _MaxEmissionCount) // make sure we don't go over the emission limit
								{
									numEmissions -= *numEmitIt - _MaxEmissionCount;
									*numEmitIt = _MaxEmissionCount;
								}
								/// compute the position of the emitter for the needed date
								numEmissions = GenEmitterPositions(_Owner,
																	_EmittedType,
																	emitterIndex,
																	numEmissions,
																	deltaT,
																	*currEmitPeriod,
																	emitterPositions
																   );
								uint k = numEmissions;								
								/// process each emission at the right pos at the right date								
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;								
								do
								{	
									--k;
									processEmitConsistent(emitterPositions[k],
														  emitterIndex,
														  nbToGenerate,
														  deltaT,
														  ellapsedTime,
														  realEllapsedTimeRatio);
									deltaT += *currEmitPeriod;									
								}
								while (k);								
							}
							else
							{
								const uint32 emitterIndex = phaseIt - (_Phase.begin());
								nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, emitterIndex) : _GenNb;
								processEmit(emitterIndex, nbToGenerate);								
								++*numEmitIt;
							}
						}
					}							
					++phaseIt;					
					currEmitPeriod += currEmitPeriodPtrInc;
					++numEmitIt;
				}
				while (phaseIt != endPhaseIt);
			}
		}

		leftToDo -= toProcess;
	}
	while (leftToDo);
}


///==========================================================================
void CPSEmitter::step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEllapsedTime)
{	
	if (pass == PSToolRender)
	{
		showTool();
		return;
	}
	if (pass != PSEmit || !_EmittedType) return;
	const uint32 size = _Owner->getSize();
	if (!size) return;

	if (ellapsedTime == 0.f) return; // do nothing when paused

	CParticleSystem *ps = _Owner->getOwner();	
	nlassert(ps);
	float emitLOD;
	if (ps->isAutoLODEnabled() && !ps->isSharingEnabled() && !_BypassAutoLOD)
	{
		// temp test for auto lod
		emitLOD = ps->getAutoLODEmitRatio();
	}
	else
	{
		emitLOD = 1.f; 
	}
	

	// our behaviour depend of the frequency
	switch (_EmissionType)
	{
		case CPSEmitter::once :
		{
			if (!_GenNbScheme && _GenNb == 0) return;
			TPSAttribTime::iterator timeIt = _Phase.begin(), timeEndIt = _Phase.end();

			if (emitLOD == 1.f)
			{			
				while (timeIt != timeEndIt)
				{
					if (*timeIt == 0.f)
					{
						const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, timeIt - _Phase.begin()) : _GenNb;		
						processEmit(timeIt - _Phase.begin(), nbToGenerate);		
						*timeIt = 1.f;
					}
					++timeIt;
				}
			}
			else
			{
				while (timeIt != timeEndIt)
				{
					if (*timeIt == 0.f)
					{
						uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, timeIt - _Phase.begin()) : _GenNb;
						if (nbToGenerate > 0)
						{				
							nbToGenerate = (sint32) (emitLOD * nbToGenerate);
							if (!nbToGenerate) nbToGenerate = 1;
							processEmit(timeIt - _Phase.begin(), nbToGenerate);
						}
						*timeIt = 1.f;
					}
					++timeIt;
				}

			}
		}
		break;
		case (CPSEmitter::regular):
		{
			if (!_ConsistentEmission)
			{
				if (emitLOD != 1.f)
				{
					processRegularEmission(ellapsedTime, emitLOD);
				}
				else
				{
					processRegularEmissionWithNoLOD(ellapsedTime);
				}
			}
			else
			{
				if (emitLOD != 1.f)
				{
					if (emitLOD != 0.f)
					{					
						processRegularEmissionConsistent(ellapsedTime, realEllapsedTime / ellapsedTime, emitLOD, 1.f / emitLOD);
					}
				}
				else
				{
					processRegularEmissionConsistentWithNoLOD(ellapsedTime, realEllapsedTime / ellapsedTime);
				}
			}
		}
		break;	
		default: break;
	}
}

///==========================================================================
void CPSEmitter::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{	
	nlassert(_Phase.getSize() != _Phase.getMaxSize());	

	_Phase.insert(0.f);
	if (_MaxEmissionCount != 0)
	{
		_NumEmission.insert(0);	
	}
	if (_PeriodScheme && _PeriodScheme->hasMemory()) _PeriodScheme->newElement(emitterLocated, emitterIndex);
	if (_GenNbScheme && _GenNbScheme->hasMemory()) _GenNbScheme->newElement(emitterLocated, emitterIndex);

}

///==========================================================================
void CPSEmitter::deleteElement(uint32 index)
{	

	if (_EmissionType == CPSEmitter::onDeath && _EmittedType)
	{
		if (!_BypassEmitOnDeath)
		{		
			const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, index) : _GenNb;		
			processEmit(index, nbToGenerate);
		}
	}		

	if (_PeriodScheme && _PeriodScheme->hasMemory()) _PeriodScheme->deleteElement(index);
	if (_GenNbScheme && _GenNbScheme->hasMemory()) _GenNbScheme->deleteElement(index);
	_Phase.remove(index);
	if (_MaxEmissionCount != 0)
	{
		_NumEmission.remove(index);
	}
}

///==========================================================================
void CPSEmitter::resize(uint32 size)
{
	nlassert(size < (1 << 16));
	if (_PeriodScheme && _PeriodScheme->hasMemory()) _PeriodScheme->resize(size, _Owner->getSize());
	if (_GenNbScheme && _GenNbScheme->hasMemory()) _GenNbScheme->resize(size, _Owner->getSize());
	_Phase.resize(size);
	if (_MaxEmissionCount != 0)
	{
		_NumEmission.resize(size);
	}
}

///==========================================================================
void CPSEmitter::bounceOccured(uint32 index)
{
	// TODO : avoid duplication with deleteElement
	if (_EmittedType && _EmissionType == CPSEmitter::onBounce)
	{
		const uint32 nbToGenerate = _GenNbScheme ? _GenNbScheme->get(_Owner, index) : _GenNb;		
		processEmit(index, nbToGenerate);
	}		
}

///==========================================================================
void CPSEmitter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{

	/// version 5  : added _BypassAutoLOD
	/// version 4  : added consistent emissions
	sint ver = f.serialVersion(4);	
	CPSLocatedBindable::serial(f);
	
	f.serialPolyPtr(_EmittedType);
	f.serial(_Phase);
	f.serial(_SpeedInheritanceFactor);
	f.serial(_SpeedBasisEmission);
	
	f.serialEnum(_EmissionType);

	// this is for use with serial
	bool trueB = true, falseB = false;

	if (!f.isReading())
	{
		switch (_EmissionType)
		{
			case CPSEmitter::regular:			
				if (_PeriodScheme)
				{
					f.serial(trueB);
					f.serialPolyPtr(_PeriodScheme);
				}
				else
				{
					 f.serial(falseB);
					 f.serial(_Period);
				}
				if (ver >= 3)
				{
					f.serial(_EmitDelay, _MaxEmissionCount);													
				}
			break;
			default:
			break;
		}
		if (_GenNbScheme)
		{
			f.serial(trueB);
			f.serialPolyPtr(_GenNbScheme);
		}
		else
		{
			 f.serial(falseB);
			 f.serial(_GenNb);
		}
	}
	else
	{
		bool useScheme;
		switch (_EmissionType)
		{
			case CPSEmitter::regular:
			{				
				f.serial(useScheme);
				if (useScheme)
				{
					delete _PeriodScheme;					
					f.serialPolyPtr(_PeriodScheme);
				}
				else
				{					 
					 f.serial(_Period);
				}
				if (ver >= 3)
				{
					f.serial(_EmitDelay, _MaxEmissionCount);
					updateMaxCountVect();
				}				
			}
			break;
			default:
			break;
		}

		f.serial(useScheme); 
		if (useScheme)
		{
			delete _GenNbScheme;
			f.serialPolyPtr(_GenNbScheme);
		}
		else
		{				
			 f.serial(_GenNb);
		}
	}
	if (ver > 1)
	{
		f.serial(_EmitDirBasis);
	}
	if (ver >= 4)
	{
		f.serial(_ConsistentEmission);
	}
	if (ver >= 5)
	{
		f.serial(_BypassAutoLOD);
	}
}

///==========================================================================
void	CPSEmitter::updateMaxCountVect()
{
	if (!_MaxEmissionCount || !_Owner)
	{
		_NumEmission.resize(0);
	}
	else
	{		
		_NumEmission.resize(_Owner->getMaxSize());
		while (_NumEmission.getSize() != 0) 
		{		
			_NumEmission.remove(0);
		}
		while (_NumEmission.getSize() != _Owner->getSize())
		{
			_NumEmission.insert(0);
		}		
	}
}

///==========================================================================
void CPSEmitter::setEmitDelay(float delay)
{
	_EmitDelay = delay;
	if (_Owner && _Owner->getOwner())
	{
		_Owner->getOwner()->systemDurationChanged();
	}
}

///==========================================================================
bool	CPSEmitter::setMaxEmissionCount(uint8 count)
{
	if (count == _MaxEmissionCount) return true;	
	nlassert(_Owner && _Owner->getOwner());
	CParticleSystem *ps = _Owner->getOwner();
	if (ps->getBypassMaxNumIntegrationSteps())
	{
		uint8 oldEmissiontCount = _MaxEmissionCount;
		// should check that the new value is valid
		_MaxEmissionCount = count;
		if (testEmitForever())
		{
			_MaxEmissionCount = oldEmissiontCount;
			nlwarning("<CPSEmitter::setMaxEmissionCount> can't set max emission count to %d  \
					   with the current configuration : the system has been flagged with     \
					   'BypassMaxNumIntegrationSteps', and should have a finite duration.    \
					   The new value is not set", (int) count);		
			return false;
		}
	}
	ps->systemDurationChanged();
	_MaxEmissionCount = count;
	updateMaxCountVect();
	return true;
}

///==========================================================================
bool CPSEmitter::checkLoop() const
{
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	if (!_EmittedType) return false;	
	std::set<const CPSLocated *> seenLocated; // the located we've already seen
	std::vector<const CPSLocated *> leftLoc(1);  // the located that are left to see
	leftLoc[0] = _EmittedType;
	do
	{
		const CPSLocated *curr = leftLoc.back();
		if (curr == this->_Owner) return true;
		leftLoc.pop_back();
		seenLocated.insert(curr);
		for(uint32 k = 0; k < curr->getNbBoundObjects(); ++k)
		{
			const CPSEmitter *emitter = dynamic_cast<const CPSEmitter *>(curr->getBoundObject(k));
			if (emitter && emitter->_EmittedType)
			{
				if (seenLocated.find(emitter->_EmittedType) == seenLocated.end()) // not already seen this one ?
				{
					leftLoc.push_back(emitter->_EmittedType);
				}
			}			
		}
	}
	while (!leftLoc.empty());
	return false;
}

///==========================================================================
bool CPSEmitter::testEmitForever() const
{				
	if (!_Owner)
	{
		nlwarning("<CPSEmitter::testEmitForever> The emitter should be inserted in a CPSLocated instance for this call to work.");
		nlassert(0);
		return true;
	}	
	if (!_Owner->getLastForever()) return false;
	switch(getEmissionType())
	{
		case CPSEmitter::onBounce:
		case CPSEmitter::externEmit:
		case CPSEmitter::regular:
			// it is ok only if a limited number of located is emitted
			if (getMaxEmissionCount() == 0) return true;
		break;
		case CPSEmitter::onDeath: return true; // the emitter never dies, so ..
		case CPSEmitter::once: return false;
		break;
		default:
			nlassert(0); // not a known type
		break;
	}
	return false;
}


////////////////////////////////////////////
// implementation of CPSModulatedEmitter  //
////////////////////////////////////////////

void CPSModulatedEmitter::serialEmitteeSpeedScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	bool useScheme;			
	if (!f.isReading())
	{
		useScheme = useEmitteeSpeedScheme();
	}
	f.serial(useScheme);
	if (useScheme)
	{
		f.serialPolyPtr(_EmitteeSpeedScheme);
	}
	else
	{
		f.serial(_EmitteeSpeed);
	}
}



////////////////////////////////////////////
// implementation of CPSEmitterOmni		  //
////////////////////////////////////////////

///==========================================================================
void CPSEmitterOmni::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	// TODO : verifier que ca marche si une particule s'emet elle-mem
	nlassert(_EmittedType);	
	
	CVector v( ((rand() % 1000) - 500) / 500.0f
				   , ((rand() % 1000) - 500) / 500.0f
				   , ((rand() % 1000) - 500) / 500.0f);
	v.normalize();
	v *= _EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed;		

	pos = srcPos;
	speed = v;	
}

///==========================================================================
void CPSEmitterOmni::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSEmitter::serial(f);
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f);
}

///==========================================================================
void CPSEmitterOmni::newElement(CPSLocated *emitter, uint32 emitterIndex)
{
	CPSEmitter::newElement(emitter, emitterIndex);
	newEmitteeSpeedElement(emitter, emitterIndex);
}

///==========================================================================
void CPSEmitterOmni::deleteElement(uint32 index)
{	
	CPSEmitter::deleteElement(index);
	deleteEmitteeSpeedElement(index);
}

///==========================================================================
void CPSEmitterOmni::resize(uint32 capacity)
{
	nlassert(capacity < (1 << 16));
	CPSEmitter::resize(capacity);
	resizeEmitteeSpeed(capacity);
}

///==========================================================================
void CPSEmitterDirectionnal::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	// TODO : verifier que ca marche si une particule s'emet elle-mem
	nlassert(_EmittedType);	
	

	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) * _Dir;		
	pos = srcPos;	
}

///==========================================================================
void CPSEmitterDirectionnal::newElement(CPSLocated *emitter, uint32 emitterIndex)
{
	CPSEmitter::newElement(emitter, emitterIndex);
	newEmitteeSpeedElement(emitter, emitterIndex);
}

///==========================================================================
void CPSEmitterDirectionnal::deleteElement(uint32 index)
{	
	CPSEmitter::deleteElement(index);
	deleteEmitteeSpeedElement(index);
}

///==========================================================================
void CPSEmitterDirectionnal::resize(uint32 capacity)
{
	nlassert(capacity < (1 << 16));
	CPSEmitter::resize(capacity);
	resizeEmitteeSpeed(capacity);
}

///==========================================================================
void CPSEmitterDirectionnal::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSEmitter::serial(f);
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f);
	f.serial(_Dir);
}

////////////////////////////////////////////
// implementation of CPSEmitterRectangle  //
////////////////////////////////////////////

///==========================================================================
void CPSEmitterRectangle::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSEmitter::serial(f);
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f);
	f.serial(_Basis);	
	f.serial(_Width);
	f.serial(_Height);
	f.serial(_Dir);

}

///==========================================================================
void CPSEmitterRectangle::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	CVector N = _Basis[index].X ^ _Basis[index].Y;
	pos = srcPos + ((rand() % 32000) * (1.f / 16000) - 1.f) *  _Width[index] *  _Basis[index].X 
				 + ((rand() % 32000) * (1.f / 16000) - 1.f) *  _Height[index] * _Basis[index].Y;
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) 
					* (_Dir.x * _Basis[index].X+ _Dir.y * _Basis[index].Y + _Dir.z *  N);
}

///==========================================================================
void CPSEmitterRectangle::setMatrix(uint32 index, const CMatrix &m)
{
	_Owner->getPos()[index] = m.getPos();

	
	 _Basis[index].X = m.getI();
	 _Basis[index].Y = m.getJ();
}

///==========================================================================
CMatrix CPSEmitterRectangle::getMatrix(uint32 index) const
{
	CMatrix m;
	m.setPos(_Owner->getPos()[index]);
	m.setRot(_Basis[index].X, _Basis[index].Y, _Basis[index].X ^ _Basis[index].Y, true);
	return m;
}

///==========================================================================
void CPSEmitterRectangle::setScale(uint32 index, float scale)
{
	_Width[index] = scale;
	_Height[index] = scale;
}

///==========================================================================
void CPSEmitterRectangle::setScale(uint32 index, const CVector &s)
{
	_Width[index] = s.x;
	_Height[index] = s.y;
}

///==========================================================================
CVector CPSEmitterRectangle::getScale(uint32 index) const
{	
	return CVector(_Width[index], _Height[index], 1.f); 
}

///==========================================================================
		void CPSEmitterRectangle::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	CPSEmitter::newElement(emitterLocated, emitterIndex);
	newEmitteeSpeedElement(emitterLocated, emitterIndex);
	_Basis.insert(CPlaneBasis(CVector::K));
	_Width.insert(1.f);
	_Height.insert(1.f);
}

///==========================================================================
void CPSEmitterRectangle::deleteElement(uint32 index)
{
	CPSEmitter::deleteElement(index);
	deleteEmitteeSpeedElement(index);
	_Basis.remove(index);
	_Width.remove(index);
	_Height.remove(index);
}

///==========================================================================
void CPSEmitterRectangle::resize(uint32 size)
{
	nlassert(size < (1 << 16));
	CPSEmitter::resize(size);
	resizeEmitteeSpeed(size);
	_Basis.resize(size);
	_Width.resize(size);
	_Height.resize(size);
}

///==========================================================================
void CPSEmitterRectangle::showTool(void)
{
	nlassert(_Owner);
	const uint size = _Owner->getSize();
	if (!size) return;
	setupDriverModelMatrix();	
	CMatrix mat;
	
	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);

	for (uint k = 0; k < size; ++k) 
	{	
		const CVector &I = _Basis[k].X;
		const CVector &J = _Basis[k].Y;
		mat.setRot(I, J , I ^J);
		mat.setPos(_Owner->getPos()[k]);
		CPSUtil::displayBasis(getDriver() ,getLocatedMat(), mat, 1.f, *getFontGenerator(), *getFontManager());				
		setupDriverModelMatrix();	

		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));
	


		const CVector &pos = _Owner->getPos()[k];
		CPSUtil::display3DQuad(*getDriver(), pos + I * _Width[k] + J * _Height[k]
										   , pos + I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] - J * _Height[k]
										   , pos - I * _Width[k] + J * _Height[k], col);
	}
}



////////////////////////////////////
// CPSEmitterconic implementation //
////////////////////////////////////

///==========================================================================
void CPSEmitterConic::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSEmitterDirectionnal::serial(f);
	f.serial(_Radius);	
}
	
///==========================================================================
void CPSEmitterConic::setDir(const CVector &v)
{
	CPSEmitterDirectionnal::setDir(v);
	
}

///==========================================================================
void CPSEmitterConic::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	// TODO : optimize that
	nlassert(_EmittedType);	
	
	// we choose a custom direction like with omnidirectionnal emitter
	// then we force the direction vect to have the unit size

	static const double divRand = (2.0 / RAND_MAX);

	CVector dir((float) (rand() * divRand - 1)
				, (float) (rand() * divRand - 1)
				, (float) (rand() * divRand - 1) );

	const float n =dir.norm();

	dir *= _Radius / n;

	dir -= (_Dir * dir) * _Dir;
	dir += _Dir;
	dir.normalize();
	
	
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) 
		    * dir;
	pos = srcPos;	
}

////////////////////////////////////////
// CPSSphericalEmitter implementation //
////////////////////////////////////////

///==========================================================================
void CPSSphericalEmitter::emit(const NLMISC::CVector &srcPos, uint32 index, CVector &pos, CVector &speed)
{
	static const double divRand = (2.0 / RAND_MAX);
	CVector dir((float) (rand() * divRand - 1), (float) (rand() * divRand - 1) , (float) (rand() * divRand - 1) );
	dir.normalize();
	pos = srcPos + _Radius[index] * dir;
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed)  * dir;	
}


///==========================================================================
void CPSSphericalEmitter::showTool(void)
{
	CPSLocated *loc;
	uint32 index;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);


	TPSAttribFloat::const_iterator radiusIt = _Radius.begin();
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPosIt = _Owner->getPos().end();
	setupDriverModelMatrix();	
	for (uint k = 0; posIt != endPosIt; ++posIt, ++radiusIt, ++k) 
	{	
		const CRGBA col = ((lb == NULL || this == lb) && loc == _Owner && index == k  ? CRGBA::Red : CRGBA(127, 127, 127));		
		CPSUtil::displaySphere(*getDriver(), *radiusIt, *posIt, 5, col);
	}
}


///==========================================================================
void CPSSphericalEmitter::setMatrix(uint32 index, const CMatrix &m)
{
	nlassert(index < _Radius.getSize());		
	// compute new pos
	_Owner->getPos()[index] = m.getPos();
	
}

///==========================================================================
CMatrix CPSSphericalEmitter::getMatrix(uint32 index) const
{
	nlassert(index < _Radius.getSize());
	CMatrix m;
	m.identity();
	m.translate(_Owner->getPos()[index]);	
	return m; 
}
		
///==========================================================================
void CPSSphericalEmitter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);	
	CPSEmitter::serial(f);
	CPSModulatedEmitter::serialEmitteeSpeedScheme(f);
	f.serial(_Radius);
}
		
///==========================================================================
void CPSSphericalEmitter::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	CPSEmitter::newElement(emitterLocated, emitterIndex);
	newEmitteeSpeedElement(emitterLocated, emitterIndex);	
	_Radius.insert(1.f);
}

///==========================================================================
void CPSSphericalEmitter::deleteElement(uint32 index)
{
	CPSEmitter::deleteElement(index);
	deleteEmitteeSpeedElement(index);	
	_Radius.remove(index);
}

///==========================================================================
void CPSSphericalEmitter::resize(uint32 size)
{
	nlassert(size < (1 << 16));
	CPSEmitter::resize(size);
	resizeEmitteeSpeed(size);	
	_Radius.resize(size);
}

/////////////////////////////////////
// CPSRadialEmitter implementation //
/////////////////////////////////////

///==========================================================================
void CPSRadialEmitter::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	CPSEmitterDirectionnal::serial(f);
}

///==========================================================================
void CPSRadialEmitter::emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed)
{
	// TODO : verifier que ca marche si une particule s'emet elle-mem
	nlassert(_EmittedType);	

	static const double divRand = (2.0 / RAND_MAX);
	CVector dir((float) (rand() * divRand - 1),
		        (float) (rand() * divRand - 1),
				(float) (rand() * divRand - 1) );
	dir -= (dir * _Dir) * _Dir; //keep tangential direction
	dir.normalize();
	
	speed = (_EmitteeSpeedScheme ? _EmitteeSpeedScheme->get(_Owner, index) : _EmitteeSpeed) * dir;		
	pos = srcPos;
}

} // NL3D

namespace NLMISC
{

std::string toString(NL3D::CPSEmitter::TEmissionType type)
{
	nlctassert(NL3D::CPSEmitter::numEmissionType == 5); // If this ct assertion is raised, the content of TEmissionType has changed, so should change this function !
	switch (type)
	{
		case NL3D::CPSEmitter::regular: return "regular";
		case NL3D::CPSEmitter::onDeath: return "onDeath";
		case NL3D::CPSEmitter::once: return "once";
		case NL3D::CPSEmitter::onBounce: return "onBounce";
		case NL3D::CPSEmitter::externEmit: return "externEmit";
		default:
			nlassert(0);
			return "";
		break;
	}
}

} // NLMISC

