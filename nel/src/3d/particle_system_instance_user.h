/** \file particle_system_instance_user.h
 * <File description>
 *
 * $Id: particle_system_instance_user.h,v 1.29 2004/04/09 14:28:48 vizerie Exp $
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
#include "3d/instance_user.h"

#include <vector>

#define NL3D_MEM_PS_INSTANCE						NL_ALLOC_CONTEXT( 3dPS )

namespace NL3D {


/**
 * This is the implementation of the user class UParticleSystemInstance
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CParticleSystemInstanceUser : public UParticleSystemInstance,
								    virtual public CInstanceUser,
								    public CParticleSystemModel::IPSModelObserver
{
public:	
	// \name Object
	// @{		
		CParticleSystemInstanceUser(CScene *scene, CTransform *model, bool deleteIt /*= true*/);
		~CParticleSystemInstanceUser();
	// @}


	// \name inherited from uinstance
	// @{						
		virtual	uint				getNumMaterials() const;	
		virtual	UInstanceMaterial	&getMaterial(uint materialId);
		virtual void				selectTextureSet(uint id) 
		{ 
			NL3D_MEM_PS_INSTANCE			
			// no-op
		}
		virtual void				getShapeAABBox(NLMISC::CAABBox &bbox) const;		
		virtual bool				canStartStop() 
		{ 
			NL3D_MEM_PS_INSTANCE			
			return false; 
		}
		virtual void				start() 
		{
			NL3D_MEM_PS_INSTANCE			
		}
		virtual void				stop()  
		{
			NL3D_MEM_PS_INSTANCE			
		}	
		virtual bool				isStarted() const 
		{ 
			NL3D_MEM_PS_INSTANCE			
			return true; 
		}
	// @}

	// \name inherited from UParticleSystemInstance
	// @{
		virtual void			setUserColor(NLMISC::CRGBA userColor);
		virtual NLMISC::CRGBA	getUserColor() const;
		virtual bool			emit(uint32 id, uint quantity);
		virtual bool			removeByID(uint32 id);
		virtual bool			isSystemPresent		(void) const;		
		virtual bool			getSystemBBox(NLMISC::CAABBox &bbox);
		virtual void			setUserParam(uint index, float value);
		virtual float			getUserParam(uint index) const;
		virtual void			bypassGlobalUserParamValue(uint userParamIndex, bool byPass = true);
		virtual bool			isGlobalUserParamValueBypassed(uint userParamIndex) const;
		virtual bool			isValid(void) const;
		virtual void			registerPSObserver(IPSObserver *observer);
		virtual bool			isPSObserver(IPSObserver *observer);
		virtual void			removePSObserver(IPSObserver *observer);		
		virtual uint			getNumID() const;		
		virtual uint32			getID(uint index) const;		
		virtual bool			getIDs(std::vector<uint32> &dest) const;
		virtual bool			setActive(uint32 id, bool active);
		virtual void			activateEmitters(bool active);		
		virtual bool			hasActiveEmitters() const;
	// @}

	// \name inherited from  CParticleSystemModel::IPSModelObserver
	// @{
		virtual void invalidPS(CParticleSystemModel *psm); 
	// @}

	// @{
		virtual bool		   hasParticles() const;
		virtual bool		   hasEmmiters() const;
	// @}
		virtual bool		   isShared() const;

	// force instanciation of system
	virtual void   forceInstanciate();


	// No-op for ParticleSystem
	virtual void		changeMRMDistanceSetup(float distanceFinest, float distanceMiddle, float distanceCoarsest);
	// change/get Shape distMax.
	virtual void		setShapeDistMax(float distMax);
	virtual float		getShapeDistMax() const;

	// No-op for ParticleSystem
	virtual	void		enableAsyncTextureMode(bool enable) ;
	virtual	bool		getAsyncTextureMode() const ;
	virtual	void		startAsyncTextureLoading() ;
	virtual	bool		isAsyncTextureReady() ;
	virtual	void		setAsyncTextureDistance(float dist) ;
	virtual	float		getAsyncTextureDistance() const ;
	virtual	void		setAsyncTextureDirty(bool flag);
	virtual	bool		isAsyncTextureDirty() const;

	// currently these methods have no effects for FXs...
	virtual float               getDistMax() const { return -1.f; }	
	virtual void                setDistMax(float distMax) {}	
	virtual void                setCoarseMeshDist(float dist) {}
	virtual float               getCoarseMeshDist() const { return -1.f; }

	virtual void		setUserMatrix(const NLMISC::CMatrix &userMat);
	virtual void		forceSetUserMatrix(const NLMISC::CMatrix &userMat);
	

	// no-op for slice time
	virtual void		setSliceTime(float duration) {}
	virtual float		getSliceTime() const { return 0.f; }

	virtual void		setZBias(float value);	

	virtual	bool		supportMaterialRendering(UDriver &drv) { return true; /* rendering of ps always supported */ }	

private:
	/// list of observers
	std::vector<IPSObserver *> _Observers;
	/// validity flag
	bool _Invalidated;

};


} // NL3D


#endif // NL_PARTICLE_SYSTEM_INSTANCE_USER_H

/* End of particle_system_instance_user.h */
