/** \file u_particle_system_instance.h
 * <File description>
 *
 * $Id: u_particle_system_instance.h,v 1.1 2001/07/25 10:20:50 vizerie Exp $
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

#ifndef NL_U_PARTICLE_SYSTEM_INSTANCE_H
#define NL_U_PARTICLE_SYSTEM_INSTANCE_H

#include "nel/misc/types_nl.h"
#include "u_instance.h"


namespace NLMISC
{
	class CAABBox ;
}

namespace NL3D {


/**
 * Interface to manipulate a particle system. Particle system are created from a UScene.
 * A system can be tuned by its user params (when it makes use of them). It has sevral states
 * invalid : the system has been destroyed \see isValid(). 
 *           once it at been detroyed, you should not try to call method inherited from the Utransform interface 
 * present : the system is available for modification. This may not be the case when the system has been temporarily remove because
 *           it is not visible anymore.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class UParticleSystemInstance : public UInstance
{
public:
	/** Tell wether the system is currently instanciated. This may not be the case when the system is not visible	  	  
	  * You must check this before you use any method that access the system.
	  * If you don't, there will be an assertion when you'll try to access it
	  * example :
	  * UParticleSystemInstance *mySystem = ...
	  * ...
	  * if (mySystem->isSystemPresent())
	  * {
	  *   /// perform modification on the system here
	  *   mySystem->setUserParam(0, 0.5f) ; 
	  * }
	  *
	  * \see isValid()	  
	  */
	virtual bool		isSystemPresent		(void) const = 0 ;

	/** Get the bounding box of the system, when it is present.
	  * \param bbox a reference to the bbox to fill
	  * \return true if the bbox has been filled
	  * \see isPresent()
	  */
	virtual	bool		getBBox(NLMISC::CAABBox &bbox)   = 0 ;


	
	/// \name System parameters
	// @{

		/** Set a user param of the system. Each user param must be >= 0 and < 1
		  * The system must be present otherwise => nlassert	  
		  * \param index the index of the user param to modify. For now it ranges from 0 to 3
		  * \value the new value of the parameter
		  * \see isSystemPresent()
		  */
		virtual void		setUserParam		(uint index, float value) = 0 ;

		/** Get the value of a user param
		  * The system must be present, otherwise => nlassert
		  * \param index the index of the user param to get. For now it ranges from 0 to 3
		  * \return the value of the user param (>= 0 and < 1)
		  * \see isSystemPresent()
		  */
		virtual float		getUserParam		(uint index) const = 0 ;
	// @}

	/** Test if the system is valid. A system is invalid when it as been destroyed.
	  * This object remains but if references nothing anymore. This usually happens
	  * when the system has been created with the flag 'detroy when no more particles' or 
	  * 'destroy when no more emitter and no more particles', or when it is detroyed when out of range
	  * . Of course, an invalid system
	  * will always return false when isSystemPresent() is called...	  
	  * \return true if the system has been destroyed and is thus invalid. You can remove this object from the scene then...
	  */
	virtual bool		isValid				(void) const =  0 ;

	/** This is a struct is used by observers that want to be notified of the destruction of the system.
	  * \see isValid()
	  * \see registerPSObserver()	  
	  */
	  struct			IPSObserver
	  {	 
		/** called when the system has been destroyed
		  * \param system the system that has been destroyed
		  */
	  	virtual void	systemDestroyed		(UParticleSystemInstance *system) = 0 ;	    
	  } ;

	/** Register an observer that will be notified when the system becomes invalid
	  * nlassert(!isPSObserver(oberver)) ;
	  * \see isPSObserver()
	  * \see removePSObserver()
	  */
	virtual void		registerPSObserver	(IPSObserver *observer) = 0 ;

	/** test whether 'observer' is an observer of this system
	  * \see removePSObserver()
	  * \see registerPSObserver()
	  */
	virtual bool		isPSObserver		(IPSObserver *observer) = 0 ;

	/** remove 'observer' from the observers of this system. Not an observer => nlassert
	  * \see registerPSObserver()
	  * \see isPSObserver()
	  */
	virtual void		removePSObserver		(IPSObserver *observer) = 0 ;
	 
};


} // NL3D


#endif // NL_U_PARTICLE_SYSTEM_INSTANCE_H

/* End of u_particle_system_instance.h */
