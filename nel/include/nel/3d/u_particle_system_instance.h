/** \file u_particle_system_instance.h
 * <File description>
 *
 * $Id: u_particle_system_instance.h,v 1.9 2003/08/18 14:30:35 vizerie Exp $
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
	class CAABBox;
}

namespace NL3D {


/**
 * Interface to manipulate a particle system. Particle system are created from a UScene.
 * A system can be tuned by its user params (when it makes use of them). It has several states
 * invalid : the system is invalid, this tells the user that he can destroy this instance
 * 
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
	  *   mySystem->setUserParam(0, 0.5f); 
	  * }
	  *
	  * \see isValid()	  
	  */
	virtual bool		isSystemPresent		(void) const = 0;

	/** Get the bounding box of the system, when it is present.
	  * You should call this instead of UInstance::getShapeAABBox() because the bbox may change over time, and thusn its shape
	  * doesn't hold any info on it...
	  * \param bbox a reference to the bbox to fill
	  * \return true if the bbox has been filled
	  * \see isPresent()
	  */
	virtual	bool		getSystemBBox(NLMISC::CAABBox &bbox)   = 0;


	
	/// \name System parameters
	//@{

		/** Set a user param of the system. Each user param must be >= 0 and <= 1		  
		  * \param index the index of the user param to modify. For now it ranges from 0 to 3
		  * \value the new value of the parameter
		  * \see isSystemPresent()
		  */
		virtual void		setUserParam(uint index, float value) = 0;

		/// Set a global user param value. User param in a system can mirror global values, which are identified by their name
		static  void        setGlobalUserParamValue(const std::string &name, float value);
		static  float       getGlobalUserParamValue(const std::string &name);
		/** Set a global vector value in the system.
		  * Some object in the system can bind their parameters to such a global value
		  * Example : direction of wind could be stored in the global variable 'WIND'
		  */
		static  void			 setGlobalVectorValue(const std::string &name, const NLMISC::CVector &v);
		static  NLMISC::CVector  getGlobalVectorValue(const std::string &name);

		/** Get the value of a user param		  
		  * \param index the index of the user param to get. For now it ranges from 0 to 3
		  * \return the value of the user param (>= 0 and <= 1)
		  * \see isSystemPresent()
		  */
		virtual float		getUserParam(uint index) const = 0;

		// bypass the update of a user param from a global value if there is one
		virtual void        bypassGlobalUserParamValue(uint userParamIndex, bool byPass = true) = 0;
		virtual bool        isGlobalUserParamValueBypassed(uint userParamIndex) const = 0;
	//@}

	///\name System validity
	//@{
		/** Test if the system is valid. A system is invalid when it should be destroyed.
		  * It's then up to the system user to destroy it (thus avoiding invalid pointers...)
		  * This usually happens when the system has been created with the flag 
		  * 'destroy when no more particles' or 'destroy when no more emitter and no more particles'
		  * , or when it is out of range.
		  * Of course, an invalid system will always return false when isSystemPresent() is called...	  
		  * \return true if the system has been invalidated. You can remove this object from the scene then...
		  */
		virtual bool		isValid				(void) const =  0;

		/** This is a struct is used by observers that want to be notified of the invalidation of the system.
		  * \see isValid()
		  * \see registerPSObserver()	  
		  */
		  struct			IPSObserver
		  {	 
			/** called when the system has been destroyed
			  * \param system the system that has been destroyed
			  */
	  		virtual void	systemDestroyed		(UParticleSystemInstance *system) = 0;	    
		  };

		/** Register an observer that will be notified when the system becomes invalid
		  * nlassert(!isPSObserver(oberver));
		  * \see isPSObserver()
		  * \see removePSObserver()
		  */
		virtual void		registerPSObserver	(IPSObserver *observer) = 0;

		/** test whether 'observer' is an observer of this system
		  * \see removePSObserver()
		  * \see registerPSObserver()
		  */
		virtual bool		isPSObserver		(IPSObserver *observer) = 0;

		/** remove 'observer' from the observers of this system. Not an observer => nlassert
		  * \see registerPSObserver()
		  * \see isPSObserver()
		  */
		virtual void		removePSObserver	(IPSObserver *observer) = 0;
	 //@}

	 //@{
			/** All the emitters that have the given ID emit their target.
			  * \return false if the id is invalid, or if it isn't an emitter ID
			  */
			virtual bool	emit(uint32 id, uint quantity = 1) = 0;
			/** All the object with the given id are removed.
			  * \return false if the id is invalid.
			  */
			virtual bool   removeByID(uint32 id) = 0;
			/// Return the number of objects in the system that are flagged with an ID, or 0 if the system is not present			  
			virtual uint   getNumID() const = 0;
			/// Get the nth ID, or 0 if index is invalid.			  
			virtual uint32 getID(uint index) const = 0;
			/** Get all the IDs in the system. 
			  * \warning As IDs are not stored in a vector, it is faster than several calls to getID
			  *
			  */
			virtual bool   getIDs(std::vector<uint32> &dest) const = 0;

			// Deactivate an object with the given ID
			virtual bool   setActive(uint32 id, bool active) = 0;			
			/** special : Activate / Deactivate all emitters.
			  * NB : the system do not need to be present for this to be called.
			  * If the system isn't instanciated, then emitters will be deactivated the next time it is
			  */
			virtual void   activateEmitters(bool active) = 0;			
			// test if there are active emitters in the system
			virtual bool  hasActiveEmitters() const = 0;
	 //@}

	 //@{
			// Test if there are particles left. Always return false if the system is not present.
			virtual bool   hasParticles() const = 0;
			// Test if there are emitters left. Always return false if the system is not present.
			virtual bool   hasEmmiters() const = 0;
	 //@}

	 // Test if the system is shared
	 virtual bool   isShared() const = 0;

};


} // NL3D


#endif // NL_U_PARTICLE_SYSTEM_INSTANCE_H

/* End of u_particle_system_instance.h */
