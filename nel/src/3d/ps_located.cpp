/** \file particle_system_located.cpp
 * <File description>
 *
 * $Id: ps_located.cpp,v 1.10 2001/05/23 15:18:01 vizerie Exp $
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



#include <algorithm>
#include "nel/3d/ps_located.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/ps_util.h"



namespace NL3D {




/**
 * Constructor
 */
CPSLocated::CPSLocated() : _MinMass(1), _MaxMass(1), _LastForever(true)
						 , _MaxLife(1.0f), _MinLife(1.0f), _Size(0)
						 , _MaxSize(DefaultMaxLocatedInstance), _UpdateLock(false)
						 , _CollisionInfo(NULL), _CollisionInfoNbRef(0)
						 , _NbFramesToSkip(0)
{		
}



/// dtor

CPSLocated::~CPSLocated()
{
	// we must do a copy, because the subsequent call can modify this vector
	TDtorObserversVect copyVect(_DtorObserversVect.begin(), _DtorObserversVect.end()) ;
	// call all the dtor observers
	for (TDtorObserversVect::iterator it = copyVect.begin() ; it != copyVect.end() ; ++it)
	{
		(*it)->detachTarget(this) ;
	}

	nlassert(_CollisionInfoNbRef == 0) ; //If this is not = 0, then someone didnt call releaseCollisionInfo
										 // If this happen, you can register with the CPSTargetLocatedBindable
										 // (observer pattern)
										 // and override releaseTarget to call releaseCollisionInfo
	nlassert(!_CollisionInfo) ;

	// delete all bindable

	for (TLocatedBoundCont::iterator it2 = _LocatedBoundCont.begin() ; it2 != _LocatedBoundCont.end() ; ++it2)
	{
		delete *it2 ;
	}
}



/**
* sorted insertion  (by decreasing priority order) of a bindable (particle e.g an aspect, emitter) in a located
*/
void CPSLocated::bind(CPSLocatedBindable *lb)
{
	nlassert(std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), lb) == _LocatedBoundCont.end()) ;	
	TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ;
	while (it != _LocatedBoundCont.end() && **it < *lb) // the "<" operator sort them correctly
	{
		++it ;
	}
	_LocatedBoundCont.insert(it, lb) ;
	lb->setOwner(this) ;
	lb->resize(_MaxSize) ;

	// any located bindable that is bound to us should have no element in it for now !!
	// we reisze the boundable, so that it has ne same number of elements as us
	for (uint32 k = 0 ; k < _Size ; ++k)
	{
		lb->newElement() ;
	}
}



void CPSLocated::remove(const CPSLocatedBindable *p)
{
	TLocatedBoundCont::iterator it = std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), p) ;
	nlassert(it != _LocatedBoundCont.end()) ;	
	delete *it ;
	_LocatedBoundCont.erase(it) ;
}


void CPSLocated::registerDtorObserver(CPSTargetLocatedBindable *anObserver)
{
	// check wether the observer wasn't registered twice
	nlassert(std::find(_DtorObserversVect.begin(), _DtorObserversVect.end(), anObserver) == _DtorObserversVect.end()) ;
	_DtorObserversVect.push_back(anObserver) ;
}

void CPSLocated::unregisterDtorObserver(CPSTargetLocatedBindable *anObserver)
{
	// check that it was registered
	TDtorObserversVect::iterator it = std::find(_DtorObserversVect.begin(), _DtorObserversVect.end(), anObserver) ;
	nlassert(it != _DtorObserversVect.end()) ;
	_DtorObserversVect.erase(it) ;
}





/**
 * new element generation
 */

sint32 CPSLocated::newElement(const CVector &pos, const CVector &speed, CPSLocatedBindable *emitter, uint32 indexInEmitter)
{	
	if (_UpdateLock)
	{
		postNewElement(pos, speed) ;
	}
	

	if (_CollisionInfo)
	{
		_CollisionInfo->insert() ;
	}

	sint32 creationIndex ;

	// get the convertion matrix  from the emitter basis to the emittee basis
	// if the emitter is null, we assume that the coordinate are given in the chosen basis for this particle type

	
	
	if (_MaxSize == _Size) return -1 ;

	const CMatrix &convMat = emitter ? CPSLocated::getConversionMatrix(this, emitter->getOwner()) 
							:  CMatrix::Identity ;
	

	creationIndex  =_Pos.insert(convMat * pos) ;
	nlassert(creationIndex != -1) ; // all attributs must contains the same number of elements
	_Speed.insert(convMat.mulVector(speed)) ;
	
	float delta ;
	
	delta = (rand() % 32767) * (1.0f / 32767.0f) ;
	_InvMass.insert((1.0f) / (delta * _MaxMass + (1.0f - delta) * _MinMass)) ;
	_Time.insert(0.0f) ;
	delta = (rand() % 32767) * (1.0f / 32767.0f) ;
	_TimeIncrement.insert(1.0f / (delta * _MaxLife + (1.0f - delta) * _MinLife)) ;

	// generate datas for all bound objects
	
	
	_UpdateLock = true ;	

	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		(*it)->newElement() ;
	}

	
	_UpdateLock = false ;
	

	++_Size ;	// if this is modified, you must also modify the getNewElementIndex in this class
				// because that method give the index of the element being created for overrider of the newElement method
				// of the CPSLocatedClass (which is called just above)

	return creationIndex ;
}

void CPSLocated::postNewElement(const CVector &pos, const CVector &speed)
{
	_RequestStack.push(CPostNewElementRequestInfo(pos, speed)) ;
}


/**
 * delete an element
 */

void CPSLocated::deleteElement(uint32 index)
{
	nlassert(index < _Size) ;
	
	// delete all bindable before : they may need our coordinate
	// to perform a destruction task
	
	_UpdateLock = true ;
	

	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		(*it)->deleteElement(index) ;
	}	

	
	_UpdateLock = false ;	

	// remove common located's attributes

	_InvMass.remove(index) ;
	_Pos.remove(index) ;
	_Speed.remove(index) ;
	_Time.remove(index) ;
	_TimeIncrement.remove(index) ;

	if (_CollisionInfo)
	{
		_CollisionInfo->remove(index) ;
	}

	--_Size ;
}


/// Resize the located container

void CPSLocated::resize(uint32 newSize)
{
	_MaxSize = newSize ;
	_InvMass.resize(newSize) ;
	_Pos.resize(newSize) ;
	_Speed.resize(newSize) ;
	_Time.resize(newSize) ;
	_TimeIncrement.resize(newSize) ;

	if (_CollisionInfo)
	{
		_CollisionInfo->resizeNFill(newSize) ;
	}

	// resize attributes for all bound objects
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		(*it)->resize(newSize) ;
	}
}


void CPSLocated::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// TODO : update this

	f.serialCheck((uint32) 'LOCA') ;
	
	CParticleSystemProcess::serial(f) ;

	f.serialVersion(1) ;

	
	f.serial(_InvMass) ;
	f.serial(_Pos) ;
	f.serial(_Speed) ;
	f.serial(_Time) ;
	f.serial(_TimeIncrement) ;

	f.serial(_Size) ; 
	f.serial(_MaxSize) ;

	f.serial(_LastForever) ;

	f.serialPtr(_CollisionInfo) ;
	f.serial(_CollisionInfoNbRef) ;
	f.serial(_MinMass, _MaxMass) ;
	f.serial(_MaxLife, _MinLife) ;

	f.serial(_NbFramesToSkip) ;

	f.serialContPolyPtr(_DtorObserversVect) ;

	if (f.isReading())
	{
		while(!_RequestStack.empty())
		{
			_RequestStack.pop() ;
		}
		uint32 size ;
		f.serial(size) ;
		for (uint32 k = 0 ; k < size ; ++k)
		{
			TNewElementRequestStack::value_type t ;
			f.serial(t) ;
			_RequestStack.push(t) ;
		}
	}
	else
	{
		// when writing the stack, we must make a copy because we can't access elements by their index
		// so the stack must be destroyed
		TNewElementRequestStack r2 ;
		uint32 size = (uint32) _RequestStack.size() ;
		f.serial(size) ;

		while(!_RequestStack.empty())
		{
			r2.push(_RequestStack.top()) ;
			_RequestStack.pop() ;
		}
		// now rebuild the stack while serializing it ;
		while (!r2.empty())
		{			
			f.serial(r2.top()) ;
			_RequestStack.push(r2.top()) ;
			r2.pop() ;
		}

	}

	
	f.serial(_UpdateLock) ;

	
	f.serialContPolyPtr(_LocatedBoundCont) ;	
}



void CPSLocated::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	if (pass == PSCollision)
	{
		
	}

	if (pass == PSMotion)
	{		
		
		// check if we must skip frames


		if (!_NbFramesToSkip || !( (uint32) _Owner->getDate() % (_NbFramesToSkip + 1)))
		{

			// update the located creation requests that may have been posted
			updateNewElementRequestStack() ;

			TPSAttribVector::iterator itPos = _Pos.begin() ;			

			// there are 2 integration steps : with and without collisions

			if (!_CollisionInfo) // no collisionner are used
			{
				TPSAttribVector::const_iterator itSpeed = _Speed.begin() ;		


				// unoptimized version for speed integration
				 for (uint k = 0 ; k < _Size ; ++k, ++itPos, ++itSpeed)
				{				
					// let's avoid a constructor call				
					itPos->x += ellapsedTime * itSpeed->x ;
					itPos->y += ellapsedTime * itSpeed->y ;
					itPos->z += ellapsedTime * itSpeed->z ;
				} 

				/* optimized version (in fact it doesn't change speed at all ;( )
				uint leftToDo = _Size ;

				while (leftToDo)			
				{	

					// let's use an ugly macro to unroll this loop more easily
					// note thta we perform the op directly to avoid a ctor call 
					#define INTEGRATE_SPEED_ONCE \
						itPos->x += ellapsedTime * itSpeed->x ; \
						itPos->y += ellapsedTime * itSpeed->y ; \
						itPos->z += ellapsedTime * itSpeed->z ; \
						++itPos ; ++itSpeed ;

					#define INTEGRATE_SPEED_FOUR INTEGRATE_SPEED_ONCE  INTEGRATE_SPEED_ONCE  INTEGRATE_SPEED_ONCE  INTEGRATE_SPEED_ONCE 
					#define INTEGRATE_SPEED_SIXTEEN INTEGRATE_SPEED_FOUR INTEGRATE_SPEED_FOUR INTEGRATE_SPEED_FOUR INTEGRATE_SPEED_FOUR
					#define INTEGRATE_SPEED_SIXTYFOUR INTEGRATE_SPEED_SIXTEEN INTEGRATE_SPEED_SIXTEEN INTEGRATE_SPEED_SIXTEEN INTEGRATE_SPEED_SIXTEEN
							

					if (leftToDo >= 64) // try to batch computations
					{
						INTEGRATE_SPEED_SIXTYFOUR ;
						leftToDo -= 64 ;
					}
					else
					{
						do
						{
							INTEGRATE_SPEED_ONCE ;
						}
						while (-- leftToDo) ;
						break ;
					}				
				}
				*/
			}
			else
			{
				// integration with collisions

				nlassert(_CollisionInfo) ;
				TPSAttribCollisionInfo::const_iterator itc = _CollisionInfo->begin() ;
				TPSAttribVector::iterator itSpeed = _Speed.begin() ;		
				for (uint k = 0 ; k < _Size ; ++k, ++itPos, ++itSpeed, ++itc)
				{
					if (itc->dist != -1)
					{
						(*itPos) = itc->newPos ;
						(*itSpeed) = itc->newSpeed ;
					}
					else
					{
						(*itPos) += ellapsedTime * (*itSpeed) ;
					}
				}

				
				// reset collision info for the next time

				resetCollisionInfo() ;
				
			}


			if (! _LastForever)
			{
				TPSAttribTime::iterator itTime = _Time.begin(), itTimeInc = _TimeIncrement.begin() ;
				for (uint32 k = 0 ; k < _Size ;)
				{
					*itTime += ellapsedTime * *itTimeInc ;
					if (*itTime >= 1.0f)
					{
						deleteElement(k) ;
					}
					else
					{
						++k ;
						++itTime ;
						++itTimeInc ;
					}
				}
			}
			else
			{
				// the time attribute gives the life in seconds
				TPSAttribTime::iterator itTime = _Time.begin(), endItTime = _Time.end() ;
				for (; itTime != endItTime ; ++itTime)
				{
					*itTime += ellapsedTime ;
				}
			}
		}
		else
		{
			return ; // we skip the frame...
		}
	}

	// apply the pass to all bound objects
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
	{
		(*it)->step(pass, ellapsedTime) ;
	}
}




void CPSLocated::updateNewElementRequestStack(void)
{
	while (!_RequestStack.empty())
	{
		newElement(_RequestStack.top()._Pos, _RequestStack.top()._Speed) ;
		_RequestStack.pop() ;
	}
}


bool CPSLocated::computeBBox(NLMISC::CAABBox &box) const
{
	if (!_Size) return false ; // something to compute ?


	TLocatedBoundCont::const_iterator it ;
	TPSAttribVector::const_iterator it2 ;

	// check whether any object bound to us need a bbox

	for (it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		if ((*it)->doesProduceBBox()) 
		{
			break ;
		}
	}

	if (it == _LocatedBoundCont.end()) 
	{
		return false ;
	}

	CVector min = _Pos[0], max = _Pos[0] ;

	for (it2 = _Pos.begin() ; it2 != _Pos.end() ; ++ it2)
	{
		const CVector &v = (*it2) ;
		min.minof(min, v) ;	
		max.maxof(max, v) ;		
	}

	box.setMinMax(min, max) ;	

	// we've considered that located had no extent in space
	// now, we must call each objects that are bound to the located in order
	// to complete the bbox if they have no null extent

	NLMISC::CAABBox tmpBox, startBox = box  ;

	for (it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		if ((*it)->doesProduceBBox())
		{
			tmpBox = startBox ;
			if ((*it)->completeBBox(tmpBox))
			{
				box = CPSUtil::computeAABBoxUnion(tmpBox, box) ;
			}
		}
	}

	return true ;
}


/// Setup the driver model matrix. It is set accordingly to the basis used for rendering	
void CPSLocated::setupDriverModelMatrix(void) 
{
	if (_SystemBasisEnabled)
	{
		getDriver()->setupModelMatrix(_Owner->getSysMat()) ;		
	}
	else
	{
		getDriver()->setupModelMatrix(CMatrix::Identity) ;
	}
}




void CPSLocated::queryCollisionInfo(void)
{
	if (_CollisionInfoNbRef)
	{
		++ _CollisionInfoNbRef ;
	}
	else
	{
		_CollisionInfo = new TPSAttribCollisionInfo ;
		_CollisionInfoNbRef = 1 ;
		_CollisionInfo->resizeNFill(_MaxSize) ;
		resetCollisionInfo() ;
	}
}

void CPSLocated::releaseCollisionInfo(void)
{
	nlassert(_CollisionInfoNbRef) ; // check whether queryCollisionInfo was called
									// so the number of refs must not = 0									
    --_CollisionInfoNbRef ;
	if (_CollisionInfoNbRef == 0)
	{
		delete _CollisionInfo ;
		_CollisionInfo = NULL ;
	}
}


void CPSLocated::resetCollisionInfo(void)
{
	nlassert(_CollisionInfo) ;

	TPSAttribCollisionInfo::iterator it = _CollisionInfo->begin(), endIt = _CollisionInfo->end() ;

	for (; it != endIt ; ++it)
	{
		it->reset() ;
	}
}


void CPSLocatedBindable::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCheck((uint32) 'BIND') ;
	f.serialVersion(1) ;
	f.serialPtr(_Owner) ;
}



void CPSTargetLocatedBindable::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1) ;	
	f.serialPtr(_Owner) ;
	if (f.isReading())
	{
		// delete previous attached bindables...
		for (TTargetCont::iterator it = _Targets.begin(); it != _Targets.end(); ++it)
		{
			delete (*it) ;
		}
		_Targets.clear() ;		
	}
	f.serialContPolyPtr(_Targets) ;
}


void CPSTargetLocatedBindable::attachTarget(CPSLocated *ptr)
{

	// a target can't be shared between different particle systems
	#ifdef NL_DEBUG
	if (_Owner)
	{
		nlassert(_Owner->getOwner() == ptr->getOwner()) ;
	}
	#endif

	// see wether this target has not been registered before 
	nlassert(std::find(_Targets.begin(), _Targets.end(), ptr) == _Targets.end()) ;
	_Targets.push_back(ptr) ;

	// we register us to be notified when the target disappear
	ptr->registerDtorObserver(this) ;
}

void CPSTargetLocatedBindable::detachTarget(CPSLocated *ptr)
{
	TTargetCont::iterator it = std::find(_Targets.begin(), _Targets.end(), ptr) ;
	nlassert(it != _Targets.end()) ;
	releaseTargetRsc(*it) ;	
	_Targets.erase(it) ;
	ptr->unregisterDtorObserver(this) ;

}

CPSTargetLocatedBindable::~CPSTargetLocatedBindable()
{
	// we unregister to all the targets
	for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
	{		
		(*it)->unregisterDtorObserver(this) ;
	}
}


} // NL3D
