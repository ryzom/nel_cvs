/** \file ps_located.cpp
 * <File description>
 *
 * $Id: ps_located.cpp,v 1.49 2002/08/21 09:39:53 lecroart Exp $
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



#include <algorithm>
#include "nel/misc/aabbox.h"
#include "nel/misc/matrix.h"
#include "3d/ps_util.h"
#include "3d/particle_system.h"
#include "3d/ps_zone.h"
#include "3d/driver.h"
#include "3d/material.h"
#include "3d/dru.h"
#include "3d/ps_located.h"
#include "3d/ps_particle.h"
#include "3d/ps_force.h"
#include "3d/ps_emitter.h"
#include "3d/ps_misc.h"

#include "nel/misc/line.h"
#include "nel/misc/system_info.h"
#include "nel/misc/common.h"

namespace NL3D {




/**
 * Constructor
 */
	CPSLocated::CPSLocated() : _MaxNumFaces(0),
							   _Name(std::string("located")),
							   _NbFramesToSkip(0),
							   _MaxSize(DefaultMaxLocatedInstance),
							   _Size(0),
							   _LastForever(true),
							   _CollisionInfo(NULL),
							   _CollisionInfoNbRef(0),
							   _InitialLife(1.f),
							   _LifeScheme(NULL),
							   _InitialMass(1.f),
							   _MassScheme(NULL),
							   _UpdateLock(false),
							   _LODDegradation(false),
							   _NonIntegrableForceNbRefs(0),
							   _NumIntegrableForceWithDifferentBasis(0),
							   _TriggerOnDeath(false),
							   _TriggerID((uint32) 'NONE'),
							   _ParametricMotion(false)
{		
}



///=============================================================================
void CPSLocated::releaseRefTo(const CParticleSystemProcess *other)
{
	// located bindables
	{	
		for(TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			(*it)->releaseRefTo(other);
		}
	}
	// dtor observers
	{	
				
		for(TDtorObserversVect::iterator it = _DtorObserversVect.begin(); it != _DtorObserversVect.end(); ++it)
		{	
			if ((*it)->getOwner() == other)
			{									
				CPSLocatedBindable *refMaker = *it;				
				refMaker->notifyTargetRemoved(this);
				break;
			}
		}
	}
}

///=============================================================================
void CPSLocated::releaseAllRef()
{
	 // located bindables
	{	
		for(TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			(*it)->releaseAllRef();
		}
	}

	// we must do a copy, because the subsequent call can modify this vector
	TDtorObserversVect copyVect(_DtorObserversVect.begin(), _DtorObserversVect.end());
	// call all the dtor observers
	for (TDtorObserversVect::iterator it = copyVect.begin(); it != copyVect.end(); ++it)
	{
		(*it)->notifyTargetRemoved(this);
	}
	_DtorObserversVect.clear();	
	
	nlassert(_CollisionInfoNbRef == 0); //If this is not = 0, then someone didnt call releaseCollisionInfo
										 // If this happen, you can register with the registerDTorObserver
										 // (observer pattern)
										 // and override notifyTargetRemove to call releaseCollisionInfo
	nlassert(_IntegrableForces.size() == 0);
	nlassert(_NonIntegrableForceNbRefs == 0);
	nlassert(!_CollisionInfo);
}


///=============================================================================
void CPSLocated::notifyMotionTypeChanged(void)
{
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->motionTypeChanged(_ParametricMotion);
	}
}


///=============================================================================
void CPSLocated::integrateSingle(float startDate, float deltaT, uint numStep,								
								uint32 indexInLocated,
								NLMISC::CVector *destPos,						
								uint stride /*= sizeof(NLMISC::CVector)*/)
{
	nlassert(supportParametricMotion() && _ParametricMotion);
	if (_IntegrableForces.size() != 0)
	{
		bool accumulate = false;
		for (TForceVect::iterator it = _IntegrableForces.begin(); it != _IntegrableForces.end(); ++it)
		{
			nlassert((*it)->isIntegrable());
			(*it)->integrateSingle(startDate, deltaT, numStep, this, indexInLocated, destPos, accumulate, stride);
			accumulate = true;
		}
	}
	else // no forces applied, just deduce position from date, initial pos and speed
	{
			#ifdef NL_DEBUG
				NLMISC::CVector *endPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride * numStep);
			#endif
			const CPSLocated::CParametricInfo &pi = _PInfo[indexInLocated];
			destPos = FillBufUsingSubdiv(pi.Pos, pi.Date, startDate, deltaT, numStep, destPos, stride);
			if (numStep != 0)
			{			
				float currDate = startDate - pi.Date;
				nlassert(currDate >= 0);
				do
				{
					#ifdef NL_DEBUG
						nlassert(destPos < endPos);
					#endif					
					destPos->x = pi.Pos.x + currDate * pi.Speed.x;
					destPos->y = pi.Pos.y + currDate * pi.Speed.y;
					destPos->z = pi.Pos.z + currDate * pi.Speed.z;
					currDate += deltaT;
					destPos = (NLMISC::CVector *) ( (uint8 *) destPos + stride);
				}
				while (--numStep);
			}	
	}
}

void CPSLocated::performParametricMotion(TAnimationTime date, TAnimationTime ellapsedTime, TAnimationTime realEllapsedTime)
{
	if (!_Size) return;	
	nlassert(supportParametricMotion() && _ParametricMotion);

	if (_IntegrableForces.size() != 0)
	{
		bool accumulate = false;
		for (TForceVect::iterator it = _IntegrableForces.begin(); it != _IntegrableForces.end(); ++it)
		{
			nlassert((*it)->isIntegrable());
			(*it)->integrate(date, this, 0, _Size, &_Pos[0], &_Speed[0], accumulate);
			accumulate = true;
		}
	}
	else
	{
		CPSLocated::TPSAttribParametricInfo::const_iterator it = _PInfo.begin(),
											endIt = _PInfo.end();
		TPSAttribVector::iterator posIt = _Pos.begin();
		float deltaT;
		do
		{
			deltaT = date - it->Date;
			posIt->x = it->Pos.x + deltaT * it->Speed.x;
			posIt->y = it->Pos.y + deltaT * it->Speed.y;
			posIt->z = it->Pos.z + deltaT * it->Speed.z;
			++posIt;
			++it;
		}
		while (it != endIt);
	}
	//step(PSEmit, ellapsedTime, realEllapsedTime);	
}

/// allocate parametric infos
void  CPSLocated::allocateParametricInfos(void)
{
	if (_ParametricMotion) return;
	nlassert(supportParametricMotion());
	nlassert(_Owner);
	const float date = _Owner->getSystemDate();
	_PInfo.resize(_MaxSize);
	// copy back infos from current position and speeds
	TPSAttribVector::const_iterator posIt = _Pos.begin(), endPosIt = _Pos.end();
	TPSAttribVector::const_iterator speedIt = _Speed.begin();
	while (posIt != endPosIt)
	{
		_PInfo.insert( CParametricInfo(*posIt, *speedIt, date) );
		++posIt;
	}
	_ParametricMotion = true;
	notifyMotionTypeChanged();
}

/// release parametric infos
void  CPSLocated::releaseParametricInfos(void)
{
	if (!_ParametricMotion) return;
	NLMISC::contReset(_PInfo);
	_ParametricMotion = false;
	notifyMotionTypeChanged();
}


/// Test wether this located support parametric motion
bool      CPSLocated::supportParametricMotion(void) const
{
	return _NonIntegrableForceNbRefs == 0 && _NumIntegrableForceWithDifferentBasis == 0;
}

/** When set to true, this tells the system to use parametric motion. This is needed in a few case only,
  * and can only work if all the forces that apply to the system are integrable
  */
void	CPSLocated::enableParametricMotion(bool enable /*= true*/)
{
	nlassert(supportParametricMotion());
	if (enable)
	{
		allocateParametricInfos();
	}
	else
	{
		releaseParametricInfos();
	}
}


void CPSLocated::setSystemBasis(bool sysBasis)
{			
	if (sysBasis != isInSystemBasis())
	{		
		for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			(*it)->basisChanged(sysBasis);
		}

		CParticleSystemProcess::setSystemBasis(sysBasis);

		for (TForceVect::iterator fIt = _IntegrableForces.begin(); fIt != _IntegrableForces.end(); ++fIt)
		{			
			integrableForceBasisChanged( (*fIt)->getOwner()->isInSystemBasis() );
		}				
	}			
}

void CPSLocated::notifyMaxNumFacesChanged(void)
{
	if (!_Owner) return;
	
	// we examine wether we have particle attached to us, and ask for the max number of faces they may want
	_MaxNumFaces  = 0;
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSParticle)
		{
			uint maxNumFaces = ((CPSParticle *) (*it))->getMaxNumFaces();
			///nlassertex(maxNumFaces < ((1 << 16) - 1), ("%s", (*it)->getClassName().c_str()));
			_MaxNumFaces += maxNumFaces;
		}
	}	
}


uint CPSLocated::querryMaxWantedNumFaces(void)
{
	return _MaxNumFaces;
}


/// tells wether there are alive entities / particles in the system
bool CPSLocated::hasParticles(void) const
{
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSParticle && (*it)->hasParticles()) return true;
	}
	return false;
}

/// tells wether there are alive emitters
bool CPSLocated::hasEmitters(void) const
{
	for (TLocatedBoundCont::const_iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->getType() == PSEmitter && (*it)->hasEmitters()) return true;
	}
	return false;
}


void CPSLocated::getLODVect(NLMISC::CVector &v, float &offset, bool systemBasis)
{
	nlassert(_Owner);
	_Owner->getLODVect(v, offset, systemBasis);
}



float CPSLocated::getUserParam(uint numParam) const
{
	nlassert(_Owner);
	return _Owner->getUserParam(numParam);
}

CScene *CPSLocated::getScene(void)
{
	nlassert(_Owner);
	return _Owner->getScene();
}


void CPSLocated::incrementNbDrawnParticles(uint num)
{
	CParticleSystem::NbParticlesDrawn += num; // for benchmark purpose	
}

void CPSLocated::setInitialLife(TAnimationTime lifeTime)
{
	_LastForever = false;
	_InitialLife = lifeTime;
	delete _LifeScheme;
	_LifeScheme = NULL;	

	/** Reset all particles current date to 0. This is needed because we do not check
	  * if particle life is over when the date of the system has not gone beyond the life duration of particles
	  */
	for (uint k = 0; k < _Size; ++k)
	{
		_Time[k] = 0.f;
	}

}
void CPSLocated::setLifeScheme(CPSAttribMaker<float> *scheme)
{
	nlassert(scheme);
	nlassert(!scheme->hasMemory()); // scheme with memory is invalid there !!
	_LastForever = false;
	delete _LifeScheme;
	_LifeScheme = scheme;
}
void CPSLocated::setInitialMass(float mass)
{
	_InitialMass = mass;
	delete _MassScheme;
	_MassScheme = NULL;	
}
void CPSLocated::setMassScheme(CPSAttribMaker<float> *scheme)
{
	nlassert(scheme);
	nlassert(!scheme->hasMemory()); // scheme with memory is invalid there !!
	delete _MassScheme;
	_MassScheme = scheme;	
}
	


/// get a matrix that helps to express located B coordinate in located A basis
const NLMISC::CMatrix &CPSLocated::getConversionMatrix(const CPSLocated *A, const CPSLocated *B)
{
	nlassert(A->_Owner == B->_Owner); // conversion must be made between entity of the same system
	if (A->_SystemBasisEnabled == B->_SystemBasisEnabled)
	{
		return NLMISC::CMatrix::Identity;
	}
	else
	{
		if (B->_SystemBasisEnabled)
		{
			return B->_Owner->getSysMat();
		}
		else
		{
			return A->_Owner->getInvertedSysMat();
		}


	}
}


NLMISC::CVector CPSLocated::computeI(void) const 
{
	if (!_SystemBasisEnabled)
	{
		return _Owner->getInvertedViewMat().getI();
	}
	else
	{
		// we must express the I vector in the system basis, so we need to multiply it by the inverted matrix of the system
		return _Owner->getInvertedSysMat().mulVector(_Owner->getInvertedViewMat().getI());
	}
}


NLMISC::CVector CPSLocated::computeJ(void) const 
{
	if (!_SystemBasisEnabled)
	{
		return _Owner->getInvertedViewMat().getJ();
	}
	else
	{
		// we must express the J vector in the system basis, so we need to multiply it by the inverted matrix of the system
		return _Owner->getInvertedSysMat().mulVector(_Owner->getInvertedViewMat().getJ());
	}
}



NLMISC::CVector CPSLocated::computeK(void) const
{
	if (!_SystemBasisEnabled)
	{
		return _Owner->getInvertedViewMat().getK();
	}
	else
	{
		// we must express the K vector in the system basis, so we need to multiply it by the inverted matrix of the system
		return _Owner->getInvertedSysMat().mulVector(_Owner->getInvertedViewMat().getK());
	}
}



IDriver *CPSLocated::getDriver() const 
{ 
	nlassert(_Owner);
	nlassert (_Owner->getDriver() ); // you haven't called setDriver on the system
	return _Owner->getDriver();
}

/// dtor

CPSLocated::~CPSLocated()
{
	// we must do a copy, because the subsequent call can modify this vector
	TDtorObserversVect copyVect(_DtorObserversVect.begin(), _DtorObserversVect.end());
	// call all the dtor observers
	for (TDtorObserversVect::iterator it = copyVect.begin(); it != copyVect.end(); ++it)
	{
		(*it)->notifyTargetRemoved(this);
	}

	nlassert(_CollisionInfoNbRef == 0); //If this is not = 0, then someone didnt call releaseCollisionInfo
										 // If this happen, you can register with the registerDTorObserver
										 // (observer pattern)
										 // and override notifyTargetRemove to call releaseCollisionInfo
	nlassert(_IntegrableForces.size() == 0);
	nlassert(_NonIntegrableForceNbRefs == 0);
	nlassert(!_CollisionInfo);

	// delete all bindable

	for (TLocatedBoundCont::iterator it2 = _LocatedBoundCont.begin(); it2 != _LocatedBoundCont.end(); ++it2)
	{					
		(*it2)->finalize();
		delete *it2;		
	} 


	delete _LifeScheme;
	delete _MassScheme;
}



/**
* sorted insertion  (by decreasing priority order) of a bindable (particle e.g an aspect, emitter) in a located
*/
void CPSLocated::bind(CPSLocatedBindable *lb)
{
	nlassert(std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), lb) == _LocatedBoundCont.end());	
	TLocatedBoundCont::iterator it = _LocatedBoundCont.begin();
	while (it != _LocatedBoundCont.end() && **it < *lb) // the "<" operator sort them correctly
	{
		++it;
	}

	_LocatedBoundCont.insert(it, lb);
	lb->setOwner(this);
	lb->resize(_MaxSize);

	// any located bindable that is bound to us should have no element in it for now !!
	// we resize it anyway...

	uint32 initialSize  = _Size;
	for (uint k = 0; k < initialSize; ++k)
	{
		_Size = k;
		lb->newElement(NULL, 0);
	}
	_Size = initialSize;


	if (_ParametricMotion) lb->motionTypeChanged(true);

	/// the max number of shapes may have changed
	notifyMaxNumFacesChanged();
}



void CPSLocated::remove(const CPSLocatedBindable *p)
{
	TLocatedBoundCont::iterator it = std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), p);
	nlassert(it != _LocatedBoundCont.end());	
	(*it)->finalize();
	delete *it;
	_LocatedBoundCont.erase(it);
}


void CPSLocated::registerDtorObserver(CPSLocatedBindable *anObserver)
{
	// check wether the observer wasn't registered twice
	nlassert(std::find(_DtorObserversVect.begin(), _DtorObserversVect.end(), anObserver) == _DtorObserversVect.end());
	_DtorObserversVect.push_back(anObserver);
}

void CPSLocated::unregisterDtorObserver(CPSLocatedBindable *anObserver)
{
	// check that it was registered
	TDtorObserversVect::iterator it = std::find(_DtorObserversVect.begin(), _DtorObserversVect.end(), anObserver);
	nlassert(it != _DtorObserversVect.end());
	_DtorObserversVect.erase(it);
}





/**
 * new element generation
 */

sint32 CPSLocated::newElement(const CVector &pos, const CVector &speed, CPSLocated *emitter, uint32 indexInEmitter, bool basisConversionForSpeed)
{	
	if (_UpdateLock)
	{
		postNewElement(pos, speed);
		return -1;
	}
	

	if (_CollisionInfo)
	{
		_CollisionInfo->insert();
	}

	sint32 creationIndex;

	// get the convertion matrix  from the emitter basis to the emittee basis
	// if the emitter is null, we assume that the coordinate are given in the chosen basis for this particle type

	
	
	if (_MaxSize == _Size) return -1;

	const CMatrix &convMat = emitter ? CPSLocated::getConversionMatrix(this, emitter) 
							:  CMatrix::Identity;
	

	creationIndex  =_Pos.insert(convMat * pos);
	nlassert(creationIndex != -1); // all attributs must contains the same number of elements
	
	_Speed.insert(basisConversionForSpeed ? convMat.mulVector(speed) : speed);
	

			
	_InvMass.insert(1.f / ((_MassScheme && emitter) ? _MassScheme->get(emitter, indexInEmitter) : _InitialMass ) );
	_Time.insert(0.0f);	
	const float lifeTime = (_LifeScheme && emitter) ?  _LifeScheme->get(emitter, indexInEmitter) : _InitialLife ;
	_TimeIncrement.insert( lifeTime ? 1.f / lifeTime : 10E6f);

	// test wether parametric motion is used, and generate the infos that are needed then
	if (_ParametricMotion)
	{
		_PInfo.insert( CParametricInfo(_Pos[creationIndex], _Speed[creationIndex], _Owner->getSystemDate() ) );
	}


	///////////////////////////////////////////
	// generate datas for all bound objects  //
	///////////////////////////////////////////
	_UpdateLock = true;	

	
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->newElement(emitter, indexInEmitter);
	}

	
	_UpdateLock = false;	
	++_Size;	// if this is modified, you must also modify the getNewElementIndex in this class
				// because that method give the index of the element being created for overrider of the newElement method
				// of the CPSLocatedClass (which is called just above)

	

	return creationIndex;
}

void CPSLocated::postNewElement(const CVector &pos, const CVector &speed)
{
	_RequestStack.push(CPostNewElementRequestInfo(pos, speed));
}


		
static inline uint32 IDToLittleEndian(uint32 input)
{
	#ifdef NL_LITTLE_ENDIAN
		return input;
	#else
		return ((input & (0xff<<24))>>24)
				|| ((input & (0xff<<16))>>8)
				|| ((input & (0xff<<8))<<8)
				|| ((input & 0xff)<<24);
	#endif
}

/**
 * delete an element
 */

void CPSLocated::deleteElement(uint32 index)
{
	nlassert(index < _Size);
	
	// delete all bindable before : they may need our coordinate
	// to perform a destruction task
	
	_UpdateLock = true;
	

	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->deleteElement(index);
	}	

	
	_UpdateLock = false;	

	// remove common located's attributes

	_InvMass.remove(index);
	_Pos.remove(index);
	_Speed.remove(index);
	_Time.remove(index);
	_TimeIncrement.remove(index);

	if (_CollisionInfo)
	{
		_CollisionInfo->remove(index);
	}

	if (_ParametricMotion)
	{
		_PInfo.remove(index);
	}

	--_Size;

	if (_TriggerOnDeath)
	{
		const uint32 id = IDToLittleEndian(_TriggerID);
		nlassert(_Owner);		
		uint numLb  = _Owner->getNumLocatedBindableByExternID(id);		
		for (uint k = 0; k < numLb; ++k)
		{
			CPSLocatedBindable *lb = _Owner->getLocatedBindableByExternID(id, k);		
			if (lb->getType() == PSEmitter)
			{
				CPSEmitter *e = NLMISC::safe_cast<CPSEmitter *>(lb);
				nlassert(e->getOwner());
				uint nbInstances = e->getOwner()->getSize();				
				for (uint l = 0; l < nbInstances; ++l)
				{
					e->singleEmit(l, 1);
				}
			}
		}
	}
}


/// Resize the located container

void CPSLocated::resize(uint32 newSize)
{
	nlassert(newSize < (1 << 16));
	if (newSize < _Size)
	{
		for (uint32 k = _Size - 1; k >= newSize; --k)
		{
			deleteElement(k);
			
			if (k == 0) break; // we're dealing with unsigned quantities
		}
		_Size = newSize;
	}


	_MaxSize = newSize;
	_InvMass.resize(newSize);
	_Pos.resize(newSize);
	_Speed.resize(newSize);
	_Time.resize(newSize);
	_TimeIncrement.resize(newSize);

	if (_ParametricMotion)
	{
		_PInfo.resize(newSize);
	}	

	if (_CollisionInfo)
	{
		_CollisionInfo->resize(newSize);
	}

	

	// resize attributes for all bound objects
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->resize(newSize);
	}


	/// compute the new max number of faces
	notifyMaxNumFacesChanged();
}


void CPSLocated::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(4);
	CParticleSystemProcess::serial(f);
	
	f.serial(_Name);

	f.serial(_InvMass);
	f.serial(_Pos);
	f.serial(_Speed);
	f.serial(_Time);
	f.serial(_TimeIncrement);

	f.serial(_Size); 
	f.serial(_MaxSize);

	f.serial(_LastForever);

	f.serialPtr(_CollisionInfo);
	f.serial(_CollisionInfoNbRef);

	
	if (f.isReading())
	{
		delete _LifeScheme;
		delete _MassScheme;

		bool useScheme;
		f.serial(useScheme);
		if (useScheme)
		{
			f.serialPolyPtr(_LifeScheme);
		}
		else
		{
			f.serial(_InitialLife);
			_LifeScheme = NULL;
		}

		f.serial(useScheme);
		if (useScheme)
		{
			f.serialPolyPtr(_MassScheme);
		}
		else
		{
			f.serial(_InitialMass);
			nlassert(_InitialMass > 0);
			_MassScheme = NULL;
		}
	}
	else
	{
		bool bFalse = false, bTrue = true;
		if (_LifeScheme)
		{
			f.serial(bTrue);
			f.serialPolyPtr(_LifeScheme);
		}
		else
		{
			f.serial(bFalse);
			f.serial(_InitialLife);
		}
		if (_MassScheme)
		{
			f.serial(bTrue);
			f.serialPolyPtr(_MassScheme);
		}
		else
		{
			f.serial(bFalse);
			nlassert(_InitialMass > 0);
			f.serial(_InitialMass);			
		}
	}


	f.serial(_NbFramesToSkip);

	f.serialContPolyPtr(_DtorObserversVect);

	if (f.isReading())
	{
		while(!_RequestStack.empty())
		{
			_RequestStack.pop();
		}
		uint32 size;
		f.serial(size);
		for (uint32 k = 0; k < size; ++k)
		{
			TNewElementRequestStack::value_type t;
			f.serial(t);
			_RequestStack.push(t);
		}
	}
	else
	{
		// when writing the stack, we must make a copy because we can't access elements by their index
		// so the stack must be destroyed
		TNewElementRequestStack r2;
		uint32 size = (uint32) _RequestStack.size();
		f.serial(size);

		while(!_RequestStack.empty())
		{
			r2.push(_RequestStack.top());
			_RequestStack.pop();
		}
		// now rebuild the stack while serializing it;
		while (!r2.empty())
		{			
			f.serial(r2.top());
			_RequestStack.push(r2.top());
			r2.pop();
		}

	}

	


	f.serial(_UpdateLock);	

	f.serialContPolyPtr(_LocatedBoundCont);
	
	if (ver > 1)
	{
		f.serial(_LODDegradation);
	}

	if (ver > 2)
	{
		f.serial(_ParametricMotion);
	}

	if (f.isReading())
	{
		// evaluate our max number of faces
		notifyMaxNumFacesChanged();

		if (_ParametricMotion)
		{
			allocateParametricInfos();			
		}
	}

	if (ver > 3)
	{
		f.serial(_TriggerOnDeath, _TriggerID);
	}
}


// integrate speed of particles. Makes eventually use of SSE instructions when present
static void IntegrateSpeed(uint count, float *src1, const float *src2 ,float ellapsedTime)
{	
	#if 0 // this works, but is not enabled for now. The precision is not that good...
		#ifdef NL_OS_WINDOWS



		if (NLMISC::CCpuInfo::hasSSE()
			&& ((uint) src1 & 15) == ((uint) src2 & 15)
			&& ! ((uint) src1 & 3)
			&& ! ((uint) src2 & 3)				
		   )   // must must be sure that memory alignment is identical	   
		{

			// compute first datas in order to align to 16 byte boudary

			uint alignCount =  ((uint) src1 >> 2) & 3; // number of float to process

			while (alignCount --)
			{
				*src1++ += ellapsedTime * *src2 ++;
			}



			count -= alignCount;
			if (count > 3)
			{
				float et[4] = { ellapsedTime, ellapsedTime, ellapsedTime, ellapsedTime};
				// sse part of computation
				__asm
				{
						mov  ecx, count
						shr  ecx, 2
						

						xor   edx, edx

						mov    eax, src1			
						mov    ebx, src2			
						movups  xmm0, et[0]
					myLoop:
						movaps xmm2, [ebx + 8 * edx]
						movaps xmm1, [eax + 8 * edx]
						mulps  xmm2, xmm0			
						addps  xmm1, xmm2
						movaps [eax + 8 * edx], xmm1			
						add edx, 2					
						dec ecx
						jne myLoop				
				}
			}
			// proceed with left float
			count &= 3;

			if (count)
			{
				src1 += alignCount;	
				src2 += alignCount;
				do
				{
					*src1 += ellapsedTime * *src2;

					++src1;
					++src2;
				}
				while (--count);
			}

		}
		else
		#endif
	#endif
	{
		// standard version	
		
	/*	for (float *src1End = src1 + count; src1 != src1End; ++src1, ++src2)
		{				
			*src1 += ellapsedTime * *src2;			
		} */


		// standard version	
		uint countDiv8 = count>>3;
		count &= 7; // takes count % 8

		while (countDiv8 --)
		{		
			src1[0] += ellapsedTime * src2[0];
			src1[1] += ellapsedTime * src2[1];
			src1[2] += ellapsedTime * src2[2];
			src1[3] += ellapsedTime * src2[3];

			src1[4] += ellapsedTime * src2[4];
			src1[5] += ellapsedTime * src2[5];
			src1[6] += ellapsedTime * src2[6];
			src1[7] += ellapsedTime * src2[7];

			src2 += 8;
			src1 += 8;
		}
		
		while (count--)
		{
			*src1++ += ellapsedTime * *src2++;			
		}


	}
}


void CPSLocated::step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt)
{
	if (!_Size) return;	


	if (pass == PSMotion)
	{		

		
		// check wether we must perform LOD degradation
		if (_LODDegradation)
		{
			if (ellapsedTime > 0)
			{
				nlassert(_Owner);
				// compute the number of particles to show
				const uint maxToHave = (uint) (_MaxSize * _Owner->getOneMinusCurrentLODRatio());
				if (_Size > maxToHave) // too much instances ?
				{
					// choose a random element to start at, and a random step
					// this will avoid a pulse effect when the system is far away
					
					uint pos = maxToHave ? rand() % maxToHave : 0;
					uint step  = maxToHave ? rand() % maxToHave : 0;

					do
					{
						deleteElement(pos);
						pos += step;
						if (pos >= maxToHave) pos -= maxToHave;
					}
					while (_Size !=maxToHave);				
				}
			}
		}


		// check if we must skip frames
		if (!_NbFramesToSkip || !( (uint32) _Owner->getDate() % (_NbFramesToSkip + 1)))
		{
			

			// update the located creation requests that may have been posted
			updateNewElementRequestStack();


			// there are 2 integration steps : with and without collisions

			if (!_CollisionInfo) // no collisionner are used
			{
				if (_Size != 0) // avoid referencing _Pos[0] if there's no size, causes STL vectors to assert...
					IntegrateSpeed(_Size * 3, &_Pos[0].x, &_Speed[0].x, ellapsedTime);
			}
			else
			{
				// integration with collisions

				nlassert(_CollisionInfo);
				TPSAttribCollisionInfo::iterator itc = _CollisionInfo->begin();
				TPSAttribVector::iterator itSpeed = _Speed.begin();		
				TPSAttribVector::iterator itPos = _Pos.begin();		

				for (uint k = 0; k < _Size;)
				{
					if (itc->dist != -1)
					{
						(*itPos) = itc->newPos;
						(*itSpeed) = itc->newSpeed;
						// notify each located bindable that a bounce occured ...
						for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
						{	
							(*it)->bounceOccured(k);
						}
						switch(itc->collisionZone->getCollisionBehaviour())
						{
							case CPSZone::bounce:
								itc->reset();
								++k, ++itPos, ++itSpeed, ++itc;
							break;
							case CPSZone::destroy:
								deleteElement(k);
							break;
						}
					}
					else
					{
						(*itPos) += ellapsedTime * (*itSpeed) * itc->TimeSliceRatio;
						itc->reset();
						++k, ++itPos, ++itSpeed, ++itc;
					}
				}

				
				// reset collision info for the next time => done during the traversal
				/// resetCollisionInfo();
				
			}		
		}
		else
		{
			return; // we skip the frame...
		}
	}

	if (pass != PSMotion)
	{
		// apply the pass to all bound objects
		for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			if ((*it)->getLOD() == PSLod1n2 || _Owner->getLOD() == (*it)->getLOD()) // has this object the right LOD ?
			{
				(*it)->step(pass, ellapsedTime, realEt);
			}
		}
	}
	else
	{
		for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{		
			(*it)->step(pass, ellapsedTime, realEt);		
		}

	}
}

void CPSLocated::updateLife(TAnimationTime ellapsedTime)
{
	if (!_Size) return;
	if (! _LastForever)
	{
		if (_LifeScheme != NULL)
		{			
			TPSAttribTime::iterator itTime = _Time.begin(), itTimeInc = _TimeIncrement.begin();			
			for (uint32 k = 0; k < _Size;)
			{
				*itTime += ellapsedTime * *itTimeInc;
				if (*itTime >= 1.0f)
				{
					deleteElement(k);
				}
				else
				{
					++k;
					++itTime;
					++itTimeInc;
				}
			}
		}
		else /// all particles have the same lifetime
		{
			if (_InitialLife != 0)
			{
				nlassert(_Owner);
				float timeInc = ellapsedTime * 1.f / _InitialLife;
				if (_Owner->getSystemDate() >= (_InitialLife - ellapsedTime))
				{
					TPSAttribTime::iterator itTime = _Time.begin();
					for (uint32 k = 0; k < _Size;)
					{
						*itTime += timeInc;
						if (*itTime >= 1.0f)
						{
							deleteElement(k);
						}
						else
						{
							++k;
							++itTime;					
						}
					}
				}
				else
				{
					TPSAttribTime::iterator itTime = _Time.begin(), itEndTime = _Time.end();
					do
					{
						*itTime += timeInc;
						++itTime;
					}
					while (itTime != itEndTime);
				}
			}
			else
			{
				uint size = _Size;
				do
				{
					deleteElement(0);
				}
				while (--size);				
			}
		}
	}
	else
	{
		// the time attribute gives the life in seconds
		TPSAttribTime::iterator itTime = _Time.begin(), endItTime = _Time.end();
		for (; itTime != endItTime; ++itTime)
		{
			*itTime += ellapsedTime;
		}
	}
}




void CPSLocated::updateNewElementRequestStack(void)
{
	while (!_RequestStack.empty())
	{
		newElement(_RequestStack.top()._Pos, _RequestStack.top()._Speed);
		_RequestStack.pop();
	}
}


bool CPSLocated::computeBBox(NLMISC::CAABBox &box) const
{
	if (!_Size) return false; // something to compute ?


	TLocatedBoundCont::const_iterator it;
	TPSAttribVector::const_iterator it2;

	// check whether any object bound to us need a bbox

	for (it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->doesProduceBBox()) 
		{
			break;
		}
	}

	if (it == _LocatedBoundCont.end()) 
	{
		return false;
	}

	CVector min = _Pos[0], max = _Pos[0];

	for (it2 = _Pos.begin(); it2 != _Pos.end(); ++ it2)
	{
		const CVector &v = (*it2);
		min.minof(min, v);	
		max.maxof(max, v);		
	}

	box.setMinMax(min, max);	

	// we've considered that located had no extent in space
	// now, we must call each objects that are bound to the located in order
	// to complete the bbox if they have no null extent

	NLMISC::CAABBox tmpBox, startBox = box;

	for (it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		if ((*it)->doesProduceBBox())
		{
			tmpBox = startBox;
			if ((*it)->completeBBox(tmpBox))
			{
				box = NLMISC::CAABBox::computeAABBoxUnion(tmpBox, box);
			}
		}
	}

	return true;
}


/// Setup the driver model matrix. It is set accordingly to the basis used for rendering	
void CPSLocated::setupDriverModelMatrix(void) 
{
	if (_SystemBasisEnabled)
	{
		getDriver()->setupModelMatrix(_Owner->getSysMat());		
	}
	else
	{
		getDriver()->setupModelMatrix(CMatrix::Identity);
	}
}




void CPSLocated::queryCollisionInfo(void)
{
	if (_CollisionInfoNbRef)
	{
		++ _CollisionInfoNbRef;
	}
	else
	{
		_CollisionInfo = new TPSAttribCollisionInfo;
		_CollisionInfoNbRef = 1;
		_CollisionInfo->resize(_MaxSize);		

		for(uint k = 0; k < _Size; ++k)
		{
			_CollisionInfo->insert();
		}		
	}
}

void CPSLocated::releaseCollisionInfo(void)
{
	nlassert(_CollisionInfoNbRef); // check whether queryCollisionInfo was called
									// so the number of refs must not = 0									
    --_CollisionInfoNbRef;
	if (_CollisionInfoNbRef == 0)
	{
		delete _CollisionInfo;
		_CollisionInfo = NULL;
	}
}


void CPSLocated::resetCollisionInfo(void)
{
	nlassert(_CollisionInfo);

	TPSAttribCollisionInfo::iterator it = _CollisionInfo->begin(), endIt = _CollisionInfo->end();

	for (; it != endIt; ++it)
	{
		it->reset();
	}
}

void CPSLocated::registerIntegrableForce(CPSForce *f)
{	
	nlassert(std::find(_IntegrableForces.begin(), _IntegrableForces.end(), f) == _IntegrableForces.end()); // force registered twice
	_IntegrableForces.push_back(f);
	if (_SystemBasisEnabled != f->getOwner()->isInSystemBasis())
	{
		++_NumIntegrableForceWithDifferentBasis;
		releaseParametricInfos();
	}
}


void CPSLocated::unregisterIntegrableForce(CPSForce *f)
{
	nlassert(f->getOwner()); // f must be attached to a located
	std::vector<CPSForce *>::iterator it = std::find(_IntegrableForces.begin(), _IntegrableForces.end(), f);
	nlassert(it != _IntegrableForces.end() );	
	_IntegrableForces.erase(it);
	if (_SystemBasisEnabled != f->getOwner()->isInSystemBasis())
	{
		--_NumIntegrableForceWithDifferentBasis;
	}
}

void CPSLocated::addNonIntegrableForceRef(void)
{
	++_NonIntegrableForceNbRefs;
	releaseParametricInfos();
}

void CPSLocated::releaseNonIntegrableForceRef(void)
{
	nlassert(_NonIntegrableForceNbRefs != 0);
	--_NonIntegrableForceNbRefs;
}


void CPSLocated::integrableForceBasisChanged(bool basis)
{	
	if (_SystemBasisEnabled != basis)
	{
		++_NumIntegrableForceWithDifferentBasis;
		releaseParametricInfos();
	}
	else
	{
		--_NumIntegrableForceWithDifferentBasis;
	}
}


///=============================================================================
CPSLocatedBindable *CPSLocated::unbind(uint index)
{	
	nlassert(index < _LocatedBoundCont.size());
	CPSLocatedBindable *lb = _LocatedBoundCont[index];		
	lb->setOwner(NULL);	
	_LocatedBoundCont.erase(_LocatedBoundCont.begin() + index);
	return lb;
}

///=============================================================================
bool CPSLocated::isBound(const CPSLocatedBindable *lb) const
{
	TLocatedBoundCont::const_iterator it = std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), lb);
	return it != _LocatedBoundCont.end();
}

///=============================================================================
uint CPSLocated::getIndexOf(const CPSLocatedBindable *lb) const
{
	for(uint k = 0; k < _LocatedBoundCont.size(); ++k)
	{
		if (_LocatedBoundCont[k] == lb) return k;
	}
	nlassert(0);
	return 0;
}



///////////////////////////////////////
// CPSLocatedBindable implementation //
///////////////////////////////////////


///=============================================================================
CPSLocatedBindable::CPSLocatedBindable() : _LOD(PSLod1n2), _Owner(NULL), _ExternID(0)
{
	_Owner = NULL;
}

void CPSLocatedBindable::setOwner(CPSLocated *psl)
{ 
	if (psl == NULL)
	{
		releaseAllRef();
		if (_Owner)
		{
			// empty this located bindable. Need to be empty if it must be rebound to another located.
			for (uint k = 0; k < _Owner->getSize(); ++k)
			{
				deleteElement(0);
			}
		}
	}
	_Owner = psl; 
}

///=============================================================================
CPSLocatedBindable::~CPSLocatedBindable()
{
	if (_ExternID)
	{
		if (_Owner && _Owner->getOwner())
		{
			_Owner->getOwner()->unregisterLocatedBindableExternID(this);
		}
	}
}

///=============================================================================
const NLMISC::CMatrix &CPSLocatedBindable::getLocatedMat(void) const
{
	nlassert(_Owner);
	if (_Owner->isInSystemBasis())
	{
		return _Owner->getOwner()->getSysMat();
	}
	else
	{
		return NLMISC::CMatrix::Identity;
	}
}


///=============================================================================
void CPSLocatedBindable::notifyTargetRemoved(CPSLocated *ptr)
{
	ptr->unregisterDtorObserver(this);
}

///=============================================================================
void CPSLocatedBindable::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(4);
	f.serialPtr(_Owner);
	if (ver > 1) f.serialEnum(_LOD);
	if (ver > 2) f.serial(_Name);
	if (ver > 3) 
	{
		if (f.isReading())
		{
			uint32 id;
			f.serial(id);
			setExternID(id);
		}
		else
		{
			f.serial(_ExternID);
		}
	}
	
}

///=============================================================================
void CPSLocatedBindable::displayIcon2d(const CVector tab[], uint nbSegs, float scale)
{
	uint32 size = _Owner->getSize();
	if (!size) return;		
	setupDriverModelMatrix();	

	const CVector I = computeI();
	const CVector K = computeK();

	static std::vector<NLMISC::CLine> lines;
	
	lines.clear();

	// ugly slow code, but not for runtime
	for (uint  k = 0; k < size; ++k)
	{
		// center of the current particle
		const CVector p = _Owner->getPos()[k];
		
		

		for (uint l = 0; l < nbSegs; ++l)
		{
			NLMISC::CLine li;
			li.V0 = p + scale * (tab[l << 1].x * I + tab[l << 1].y * K);
			li.V1 = p + scale * (tab[(l << 1) + 1].x * I + tab[(l << 1) + 1].y * K);
			lines.push_back(li);
		}
	
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

///=============================================================================
CFontManager *CPSLocatedBindable::getFontManager(void)
{
	nlassert(_Owner);
	return _Owner->getFontManager();
}

///=============================================================================
 /// Shortcut to get the font manager if one was set (const version)
const CFontManager *CPSLocatedBindable::getFontManager(void) const
{
	nlassert(_Owner);
	return _Owner->getFontManager();
}


///=============================================================================
// Shortcut to get the matrix of the system	
 const NLMISC::CMatrix &CPSLocatedBindable::getSysMat(void) const 
{
	nlassert(_Owner);		
	return _Owner->getOwner()->getSysMat();
}

 ///=============================================================================
/// shortcut to get the inverted matrix of the system
const NLMISC::CMatrix &CPSLocatedBindable::getInvertedSysMat(void) const 
{
	nlassert(_Owner);
		return _Owner->getOwner()->getInvertedSysMat();

}

///=============================================================================
const NLMISC::CMatrix &CPSLocatedBindable::getInvertedLocatedMat(void) const
{
	nlassert(_Owner);
	if (_Owner->isInSystemBasis())
	{
		return _Owner->getOwner()->getInvertedSysMat();
	}
	else
	{
		return NLMISC::CMatrix::Identity;
	}
}

///=============================================================================
/// shortcut to get the view matrix
const NLMISC::CMatrix &CPSLocatedBindable::getViewMat(void) const 
{
	nlassert(_Owner);
	return _Owner->getOwner()->getViewMat();	
}	


///=============================================================================
/// shortcut to get the inverted view matrix
const NLMISC::CMatrix &CPSLocatedBindable::getInvertedViewMat(void) const 
{
	nlassert(_Owner);
	return _Owner->getOwner()->getInvertedViewMat();	
}	

///=============================================================================
/// shortcut to setup the model matrix (system basis or world basis)
void CPSLocatedBindable::setupDriverModelMatrix(void)  
{
	nlassert(_Owner);
	_Owner->setupDriverModelMatrix();
}

///=============================================================================
void	CPSLocatedBindable::setExternID(uint32 id)
{
	if (id == _ExternID) return;
	nlassert(_Owner && _Owner->getOwner()); // to call this method, this locatedBindable must be inserted in a system!! 
											// (e.g not standalone)
	_Owner->getOwner()->unregisterLocatedBindableExternID(this);
	if (id != 0)
	{	
		_Owner->getOwner()->registerLocatedBindableExternID(id, this);
		_ExternID = id;
	}	
}

///=============================================================================
void CPSLocatedBindable::releaseAllRef()
{
}

	 



/////////////////////////////////////////////
// CPSTargetLocatedBindable implementation //
/////////////////////////////////////////////

///=============================================================================
void CPSTargetLocatedBindable::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	(void)f.serialVersion(1);	
	f.serialPtr(_Owner);
	f.serial(_Name);
	if (f.isReading())
	{
		// delete previous attached bindables...
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{
			delete (*it);
		}
		_Targets.clear();		
	}
	f.serialContPolyPtr(_Targets);	
}


///=============================================================================
void CPSTargetLocatedBindable::attachTarget(CPSLocated *ptr)
{

	// a target can't be shared between different particle systems
	#ifdef NL_DEBUG
	if (_Owner)
	{
		nlassert(_Owner->getOwner() == ptr->getOwner());
	}
	#endif

	// see wether this target has not been registered before 
	nlassert(std::find(_Targets.begin(), _Targets.end(), ptr) == _Targets.end());
	_Targets.push_back(ptr);

	// we register us to be notified when the target disappear
	ptr->registerDtorObserver(this);
}


///=============================================================================
void CPSTargetLocatedBindable::notifyTargetRemoved(CPSLocated *ptr) 
{	
	TTargetCont::iterator it = std::find(_Targets.begin(), _Targets.end(), ptr);
	nlassert(it != _Targets.end());
	releaseTargetRsc(*it);	
	_Targets.erase(it);

	CPSLocatedBindable::notifyTargetRemoved(ptr);	
}



// dtor

///=============================================================================
void CPSTargetLocatedBindable::finalize(void)
{	
	/** Release the collisionInfos we've querried. We can't do it in the dtor, as calls to releaseTargetRsc wouldn't be polymorphics for derived class!
	  * And the behaviour of releaseTergetRsc is implemented in derived class
	  */
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{		
		releaseTargetRsc(*it);
	}
}

///=============================================================================
CPSTargetLocatedBindable::~CPSTargetLocatedBindable()
{	
	// we unregister to all the targets
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{		
		(*it)->unregisterDtorObserver(this);
	}
}

///=============================================================================
void CPSTargetLocatedBindable::releaseRefTo(const CParticleSystemProcess *other)
{
	TTargetCont::iterator it = std::find(_Targets.begin(), _Targets.end(), other);
	if (it == _Targets.end()) return;
	releaseTargetRsc(*it);
	(*it)->unregisterDtorObserver(this);
	_Targets.erase(it);
	nlassert(std::find(_Targets.begin(), _Targets.end(), other) == _Targets.end());
}

///=============================================================================
void CPSTargetLocatedBindable::releaseAllRef()
{
	for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
	{
		releaseTargetRsc(*it);
		(*it)->unregisterDtorObserver(this);		
	}
	_Targets.clear();
	CPSLocatedBindable::releaseAllRef();
}




} // NL3D
