/** \file particle_system_located.cpp
 * <File description>
 *
 * $Id: ps_located.cpp,v 1.1 2001/04/25 08:46:19 vizerie Exp $
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
{
}



/// dtor

CPSLocated::~CPSLocated()
{
	nlassert(_CollisionInfoNbref == 0) ; //If this is not null, then someone didnt call releaseCollisionInfo
	nlassert(!_CollideInfo) ;
}



/**
* sorted insertion  (by decreasing priority order) of a bindable (particle e.g an aspect, emitter) in a located
*/
void CPSLocated::bind(CSmartPtr<CPSLocatedBindable> lb)
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
}



void CPSLocated::unbind(const CSmartPtr<CPSLocatedBindable> &p)
{
	TLocatedBoundCont::iterator it = std::find(_LocatedBoundCont.begin(), _LocatedBoundCont.end(), p) ;
	nlassert(it != _LocatedBoundCont.end()) ;	
	_LocatedBoundCont.erase(it) ;
}




/**
 * new element generation
 */

bool CPSLocated::newElement(const CVector &pos, const CVector &speed, CPSLocatedBindable *emitter, uint32 indexInEmitter)
{	
	if (_UpdateLock)
	{
		postNewElement(pos, speed) ;
	}
	

	if (_CollisionInfo)
	{
		_CollisionInfo->insert() ;
	}

	// if the emitter is null, we assume that the coordinate are given in the chosen basis for this particle type

	if ((!emitter) || _SystemBasisEnabled == emitter->getOwner()->_SystemBasisEnabled)
	{
		_Pos.insert(pos) ;
		_Speed.insert(speed) ;
	}
	else
	{
		if (emitter->getOwner()->_SystemBasisEnabled) // convert from system basis to world basis
		{
			_Pos.insert(_Owner->getSysMat() * pos) ;
			_Speed.insert(_Owner->getSysMat().mulVector(speed)) ;
		}
		else  // convert from world basis to system basis
		{
			_Pos.insert(_Owner->getInvertedSysMat() * pos) ;
			_Speed.insert(_Owner->getSysMat().mulVector(speed)) ;
		}
	}

	if (_MaxSize == _Size) return false ;

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

	return true ;
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

	// resize attributes for all bound objects
	for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin() ; it != _LocatedBoundCont.end() ; ++it)
	{
		(*it)->resize(newSize) ;
	}
}


void CPSLocated::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// TODO : update this


	f.serialVersion(1) ;

	f.serial(_InvMass) ;
	f.serial(_Pos) ;
	f.serial(_Speed) ;
	f.serial(_Time) ;
	f.serial(_TimeIncrement) ;

	uint32 size ;	
	if (f.isReading())
	{
		_LocatedBoundCont.clear() ;
		f.serial(size) ;
		for (uint32 k = 0 ; k < size ; ++k)
		{
			CPSLocatedBindable *pt = NULL ;
			f.serialPolyPtr(pt) ;
			_LocatedBoundCont.push_back(CSmartPtr<CPSLocatedBindable>(pt)) ;
		}
	}
	else
	{
		size = _LocatedBoundCont.size() ;
		f.serial(size) ;
		for (TLocatedBoundCont::iterator it = _LocatedBoundCont.begin(); it != _LocatedBoundCont.end(); ++it)
		{
			CPSLocatedBindable *pt = (*it) ;
			f.serialPolyPtr(pt) ;
		}
	}
}



void CPSLocated::step(TPSProcessPass pass, CAnimationTime ellapsedTime)
{
	if (pass == PSCollision)
	{
		if (_CollisionInfo)  // are there any collisionner for us ?
		{
			resetCollisionInfo() ;
		}
	}

	if (pass == PSMotion)
	{		
		
		// update the located creation requests that may have been posted
		updateNewElementRequestStack() ;

		TPSAttribVector::iterator itPos = _Pos.begin() ;
		
		TPSAttribFloat::const_iterator itm = _InvMass.begin() ;



		// there are 2 integration steps : with and without collisions

		if (!_CollisionInfo) // no collisionner are used
		{
			TPSAttribVector::const_iterator itSpeed = _Speed.begin() ;		
			for (uint k = 0 ; k < _Size ; ++k, ++itPos, ++itSpeed, ++itm)
			{
				(*itPos) += ellapsedTime * (*itm) * (*itSpeed) ;
			}
		}
		else
		{
			// integration with collisions

			TPSAttribCollisionInfo::const_iterator itc = _CollisionInfo->begin() ;
			TPSAttribVector::iterator itSpeed = _Speed.begin() ;		
			for (uint k = 0 ; k < _Size ; ++k, ++itPos, ++itSpeed, ++itm, ++itc)
			{
				if (itc->dist != -1)
				{
					(*itPos) = itc->newPos ;
					(*itSpeed) = itc->newSpeed ;
				}
				else
				{
					(*itPos) += ellapsedTime * (*itm) * (*itSpeed) ;
				}
			}
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

	box.setCenter(0.5f * (max + min)) ;
	box.setHalfSize(0.5f * (max - min)) ;

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



} // NL3D
