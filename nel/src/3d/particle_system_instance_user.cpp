/** \file particle_system_instance_user.cpp
 * <File description>
 *
 * $Id: particle_system_instance_user.cpp,v 1.8 2002/03/29 14:19:55 berenguier Exp $
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

#include "3d/particle_system_instance_user.h"
#include "3d/particle_system.h"
#include "3d/ps_emitter.h"


namespace NL3D {

CParticleSystemInstanceUser::CParticleSystemInstanceUser(CScene *scene, IModel *model) 
								: CTransformUser(scene, model), _Invalidated(false)
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform) ;
	psm->registerPSModelObserver(this) ;
}

void				CParticleSystemInstanceUser::getShapeAABBox(NLMISC::CAABBox &bbox) const
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform) ;
	psm->getAABBox(bbox);
}

CParticleSystemInstanceUser::~CParticleSystemInstanceUser()
{	
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform) ;
	psm->removePSModelObserver(this) ;		
}

bool		CParticleSystemInstanceUser::isSystemPresent(void) const
{
	if (_Invalidated) return false ; // the system is not even valid
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform) ;
	return psm->getPS() != NULL ;
}

bool		CParticleSystemInstanceUser::getSystemBBox(NLMISC::CAABBox &bbox)
{
	if (_Invalidated) return false ;
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform) ;
	if (!psm->getPS()) return false ;
	psm->getPS()->computeBBox(bbox) ;
	return true ;
}

void		CParticleSystemInstanceUser::setUserParam(uint index, float value)
{
	nlassert(isSystemPresent())      ; // user : you forgot to check wether the system was present with isPresent() !!
	nlassert(index < MaxPSUserParam) ; // invalid parameter index
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform) ;
	psm->getPS()->setUserParam(index, value) ;
}

float		CParticleSystemInstanceUser::getUserParam(uint index) const
{
	nlassert(isSystemPresent())      ; // user : you forgot to check wether the system was present with isPresent() !!
	nlassert(index < MaxPSUserParam) ; // invalid parameter index
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform) ;
	return psm->getPS()->getUserParam(index) ;

}

bool		CParticleSystemInstanceUser::isValid(void) const
{
	return !_Invalidated ;
}

void		CParticleSystemInstanceUser::registerPSObserver(IPSObserver *observer)
{
	nlassert(!isPSObserver(observer)) ;
	_Observers.push_back(observer) ;
}

bool		CParticleSystemInstanceUser::isPSObserver(IPSObserver *observer)
{
	return std::find(_Observers.begin(), _Observers.end(), observer) != _Observers.end() ;
}

void		CParticleSystemInstanceUser::removePSObserver(IPSObserver *observer)
{
	nlassert(isPSObserver(observer)) ;
	_Observers.erase(std::find(_Observers.begin(), _Observers.end(), observer)) ;
}


void		CParticleSystemInstanceUser::invalidPS(CParticleSystemModel *psm)
{
	// the instance pointer is invalid now
	_Invalidated = true ;
	std::vector<IPSObserver *> obserCopy(_Observers.begin(), _Observers.end()) ;
	for (std::vector<IPSObserver *>::iterator it = _Observers.begin(); it != _Observers.end() ; ++it)
	{
		(*it)->systemDestroyed(this) ;
	}
}



uint				CParticleSystemInstanceUser::getNumMaterials() const
{
	return 0 ;
}

UInstanceMaterial	&CParticleSystemInstanceUser::getMaterial(uint materialId)
{
	nlassert(0) ; // no material for a particle system

	// return dummy object
	return *(UInstanceMaterial *) NULL ;
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

void	CParticleSystemInstanceUser::emit(uint32 anId, uint quantity)
{
	const uint32 id = IDToLittleEndian(anId);
	nlassert(isSystemPresent());
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	uint numLb  = ps->getNumLocatedBindableByExternID(id);
	nlassert(numLb != 0); // INVALID ID !!
	for (uint k = 0; k < numLb; ++k)
	{
		CPSLocatedBindable *lb = ps->getLocatedBindableByExternID(id, k);		
		if (lb->getType() == PSEmitter)
		{
			CPSEmitter *e = NLMISC::safe_cast<CPSEmitter *>(lb);
			nlassert(e->getOwner());
			uint nbInstances = e->getOwner()->getSize();
			for (uint l = 0; l < nbInstances; ++l)
			{
				e->singleEmit(l, quantity);
			}
		}
	}
}

void CParticleSystemInstanceUser::removeByID(uint32 anId)
{
	const uint32 id = IDToLittleEndian(anId);
	nlassert(isSystemPresent());
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	uint numLb  = ps->getNumLocatedBindableByExternID(id);
	nlassert(numLb != 0); // INVALID ID !!
	for (uint k = 0; k < numLb; ++k)
	{
		CPSLocatedBindable *lb = ps->getLocatedBindableByExternID(id, k);
		CPSLocated *owner = lb->getOwner();
		nlassert(owner);
		uint nbInstances  =  owner->getSize();
		for (uint l = 0; l < nbInstances; ++l)
		{
			owner->deleteElement(0);
		}		
	}

}

void		CParticleSystemInstanceUser::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	// no-op.
}


} // NL3D
