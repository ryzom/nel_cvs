/** \file particle_system_instance_user.h
 * <File description>
 *
 * $Id: particle_system_instance_user.h,v 1.12 2002/07/03 09:06:57 vizerie Exp $
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

#ifndef NL_PARTICLE_SYSTEM_INSTANCE_USER_H
#define NL_PARTICLE_SYSTEM_INSTANCE_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/u_particle_system_instance.h"
#include "3d/particle_system_model.h"
#include "3d/transform_user.h"

#include <vector>


namespace NL3D {


/**
 * This is the implementation of the user class UParticleSystemInstance
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CParticleSystemInstanceUser : virtual public UParticleSystemInstance,
								    virtual public CTransformUser,
								    public CParticleSystemModel::IPSModelObserver
{
public:	
	// \name Object
	// @{		
		CParticleSystemInstanceUser(CScene *scene, IModel *model);
		~CParticleSystemInstanceUser();
	// @}


	// \name inherited from uinstance
	// @{						
		virtual	uint				getNumMaterials() const;	
		virtual	UInstanceMaterial	&getMaterial(uint materialId);
		virtual void				selectTextureSet(uint id) { nlassert(0); /* not supported for particle systems */ }
		virtual void				getShapeAABBox(NLMISC::CAABBox &bbox) const;		
		virtual bool				canStartStop() { return false; }
		virtual void				start() {}
		virtual void				stop()  {}	
		virtual bool				isStarted() const { return true; }
	// @}

	// \name inherited from UParticleSystemInstance
	// @{
		virtual bool		emit(uint32 id, uint quantity);
		virtual bool		removeByID(uint32 id);
		virtual bool		isSystemPresent		(void) const;		
		virtual bool		getSystemBBox(NLMISC::CAABBox &bbox);
		virtual void		setUserParam		(uint index, float value);
		virtual float		getUserParam		(uint index) const;
		virtual bool		isValid				(void) const;
		virtual void		registerPSObserver	(IPSObserver *observer);
		virtual bool		isPSObserver		(IPSObserver *observer);
		virtual void		removePSObserver	(IPSObserver *observer);		
		virtual uint		getNumID() const;		
		virtual uint32		getID(uint index) const;		
		virtual bool		getIDs(std::vector<uint32> &dest) const;		  
	// @}

	// \name inherited from  CParticleSystemModel::IPSModelObserver
	// @{
		virtual void invalidPS(CParticleSystemModel *psm); 
	// @}

	// No-op for ParticleSystem
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);
	// change/get Shape distMax.
	virtual void		setShapeDistMax(float distMax);
	virtual float		getShapeDistMax() const;

private:
	/// list of observers
	std::vector<IPSObserver *> _Observers;
	/// validity flag
	bool _Invalidated;

};


} // NL3D


#endif // NL_PARTICLE_SYSTEM_INSTANCE_USER_H

/* End of particle_system_instance_user.h */
