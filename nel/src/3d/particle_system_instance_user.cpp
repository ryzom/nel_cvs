/** \file particle_system_instance_user.cpp
 * <File description>
 *
 * $Id: particle_system_instance_user.cpp,v 1.16 2002/10/10 13:30:26 vizerie Exp $
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
#include "3d/particle_system_shape.h"
#include "3d/ps_emitter.h"


namespace NL3D {


//===================================================================
CParticleSystemInstanceUser::CParticleSystemInstanceUser(CScene *scene, IModel *model) 
								: CTransformUser(scene, model), _Invalidated(false)
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	psm->registerPSModelObserver(this);
}

//===================================================================
void				CParticleSystemInstanceUser::getShapeAABBox(NLMISC::CAABBox &bbox) const
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	psm->getAABBox(bbox);
}

//===================================================================
CParticleSystemInstanceUser::~CParticleSystemInstanceUser()
{	
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	psm->removePSModelObserver(this);		
}

//===================================================================
bool		CParticleSystemInstanceUser::isSystemPresent(void) const
{
	if (_Invalidated) return false; // the system is not even valid
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	return psm->getPS() != NULL;
}

//===================================================================
bool		CParticleSystemInstanceUser::getSystemBBox(NLMISC::CAABBox &bbox)
{
	if (_Invalidated) return false;
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	if (!psm->getPS()) return false;
	psm->getPS()->computeBBox(bbox);
	return true;
}

//===================================================================
void		CParticleSystemInstanceUser::setUserParam(uint index, float value)
{	
	nlassert(index < MaxPSUserParam); // invalid parameter index
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	// psm->getPS()->setUserParam(index, value);
	IAnimatedValue *av = psm->getValue(CParticleSystemModel::PSParam0 + index);
	NLMISC::safe_cast<CAnimatedValueFloat *>(av)->Value = value;
	psm->touch(CParticleSystemModel::PSParam0 + index, CParticleSystemModel::OwnerBit);	
}

//===================================================================
float		CParticleSystemInstanceUser::getUserParam(uint index) const
{
	//nlassert(isSystemPresent()); // user : you forgot to check wether the system was present with isPresent() !!
	nlassert(index < MaxPSUserParam); // invalid parameter index
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	//return psm->getPS()->getUserParam(index);
	IAnimatedValue *av = psm->getValue(CParticleSystemModel::PSParam0 + index);
	return NLMISC::safe_cast<CAnimatedValueFloat *>(av)->Value;
}

//===================================================================
bool		CParticleSystemInstanceUser::isValid(void) const
{
	return !_Invalidated;
}

//===================================================================
void		CParticleSystemInstanceUser::registerPSObserver(IPSObserver *observer)
{
	nlassert(!isPSObserver(observer));
	_Observers.push_back(observer);
}

//===================================================================
bool		CParticleSystemInstanceUser::isPSObserver(IPSObserver *observer)
{
	return std::find(_Observers.begin(), _Observers.end(), observer) != _Observers.end();
}


//===================================================================
void		CParticleSystemInstanceUser::removePSObserver(IPSObserver *observer)
{
	nlassert(isPSObserver(observer));
	_Observers.erase(std::find(_Observers.begin(), _Observers.end(), observer));
}


//===================================================================
void		CParticleSystemInstanceUser::invalidPS(CParticleSystemModel *psm)
{
	// the instance pointer is invalid now
	_Invalidated = true;
	std::vector<IPSObserver *> obserCopy(_Observers.begin(), _Observers.end());
	for (std::vector<IPSObserver *>::iterator it = _Observers.begin(); it != _Observers.end(); ++it)
	{
		(*it)->systemDestroyed(this);
	}
}

//===================================================================
uint				CParticleSystemInstanceUser::getNumMaterials() const
{
	return 0;
}

//===================================================================
UInstanceMaterial	&CParticleSystemInstanceUser::getMaterial(uint materialId)
{
	nlassert(0); // no material for a particle system

	// return dummy object
	return *(UInstanceMaterial *) NULL;
}


//===================================================================
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

//===================================================================
bool	CParticleSystemInstanceUser::emit(uint32 anId, uint quantity)
{
	const uint32 id = IDToLittleEndian(anId);
	nlassert(isSystemPresent());
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	uint numLb  = ps->getNumLocatedBindableByExternID(id);
	if (numLb == 0) return false; // INVALID ID !!
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
	return true;
}



//===================================================================
bool CParticleSystemInstanceUser::removeByID(uint32 anId)
{
	const uint32 id = IDToLittleEndian(anId);
	if (!isSystemPresent()) return false;
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	uint numLb  = ps->getNumLocatedBindableByExternID(id);
	if (numLb == 0) return false; // INVALID ID !!
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
	return true;
}

//===================================================================
void		CParticleSystemInstanceUser::changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest)
{
	// no-op.
}


//===================================================================
uint CParticleSystemInstanceUser::getNumID() const
{
	if (!isSystemPresent()) return 0;
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	return ps->getNumID();
}

//===================================================================
uint32 CParticleSystemInstanceUser::getID(uint index) const
{
	if (!isSystemPresent()) return 0;
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	return ps->getID(index);
}

//===================================================================
bool CParticleSystemInstanceUser::getIDs(std::vector<uint32> &dest) const
{
	if (!isSystemPresent()) return false;
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	ps->getIDs(dest);
	return true;
}


//===================================================================
void		CParticleSystemInstanceUser::setShapeDistMax(float distMax)
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	if(psm && psm->Shape)
	{
		psm->Shape->setDistMax(distMax);
	}
}

//===================================================================
float		CParticleSystemInstanceUser::getShapeDistMax() const
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	if(psm && psm->Shape)
	{
		return psm->Shape->getDistMax();
	}
	else
		return -1;
}

//===================================================================
bool CParticleSystemInstanceUser::setActive(uint32 anId, bool active)
{
	const uint32 id = IDToLittleEndian(anId);
	if (!isSystemPresent()) return false;
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	uint numLb  = ps->getNumLocatedBindableByExternID(id);
	if (numLb == 0) return false; // INVALID ID !!
	for (uint k = 0; k < numLb; ++k)
	{
		CPSLocatedBindable *lb = ps->getLocatedBindableByExternID(id, k);
		lb->setActive(active);		
	}
	return true;
}


//===================================================================
bool CParticleSystemInstanceUser::activateEmitters(bool active)
{	
	if (!isSystemPresent()) return false;
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	for(uint k = 0; k < ps->getNbProcess(); ++k)
	{
		CPSLocated *loc = dynamic_cast<CPSLocated *>(ps->getProcess(k));
		if (loc)
		{
			for(uint l = 0; l < loc->getNbBoundObjects(); ++l)
			{
				if (loc->getBoundObject(l)->getType() == PSEmitter)	
					loc->getBoundObject(l)->setActive(active);
			}
		}
	}
	return true;
}

//===================================================================
bool CParticleSystemInstanceUser::hasParticles() const
{
	if (!isSystemPresent()) return false;
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	return ps->hasParticles();
}

//===================================================================
bool CParticleSystemInstanceUser::hasEmmiters() const
{
	if (!isSystemPresent()) return false;
	CParticleSystem *ps = (NLMISC::safe_cast<CParticleSystemModel *>(_Transform))->getPS();
	return ps->hasEmitters();
}
//===================================================================
bool CParticleSystemInstanceUser::isShared() const
{
	CParticleSystemModel *psm = NLMISC::safe_cast<CParticleSystemModel *>(_Transform);
	if(psm->Shape)
	{
		return NLMISC::safe_cast<CParticleSystemShape *>((IShape *) psm->Shape)->isShared();
	}	
	return false;
}




// ***************************************************************************
void		CParticleSystemInstanceUser::enableAsyncTextureMode(bool enable) 
{
}
bool		CParticleSystemInstanceUser::getAsyncTextureMode() const 
{
	return false;
}
void		CParticleSystemInstanceUser::startAsyncTextureLoading() 
{
}
bool		CParticleSystemInstanceUser::isAsyncTextureReady() 
{
	return true;
}


} // NL3D
