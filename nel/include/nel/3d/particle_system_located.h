/** \file particle_system_located.h
 * <File description>
 *
 * $Id: particle_system_located.h,v 1.1 2001/04/25 08:39:08 vizerie Exp $
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

#ifndef NL_PARTICLE_SYSTEM_LOCATED_H
#define NL_PARTICLE_SYSTEM_LOCATED_H

#include "nel/misc/types_nl.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_attrib.h" // an attribute template container
#include "nel/misc/vector.h"


namespace NL3D 
{

using NLMISC::CSmartPtr ;
using NLMISC::CVector ;
class CPSLocatedBindable ;

/**
 * this class is a located : it belongs to a particle system, and it represents
 * any kind of object that has a position in the world.
 * A located don't do anything by itself. You must bind objects to it, such as a particle, 
 * a force and so on.
 * It is important to remember that a located holds all instance of object of
 * one type (force, emitter, particles or both...), not only one.
 */
 
class CPSLocated : public CParticleSystemProcess
{
public:
	/// Constructor
	CPSLocated() ;

	/**
	* return true if the located is in the particle system basis, false if it's in the world basis
	*/
	bool isInLocalBasis(void) const { return _LocalBasisEnabled ; }

	/**
	* attach a bindable object to this located, such as a force or a particle
	* a bindable must be attached only once (-> nlassert)
	*/
	void bind(CSmartPtr<CPSLocatedBindable> lb) ;

	/** remove a bound object from the located
	 *  if the object doesnt exist -> nlassert
	 */

	void unbind(const CSmartPtr<CPSLocatedBindable> &lb) ;

	/**
	* count the number of bound objects
	*/
	uint32 getNbBoundObjects(void) const { return _LocatedBoundCont.size() ; }

	/**
	* get a reference to a bound object
	*/
	const CSmartPtr<CPSLocatedBindable> &getBoundObject(uint32 index) const 
	{
		nlassert(index < _LocatedBoundCont.size()) ;
		return _LocatedBoundCont[index] ;
	}
	
	/**
	 * Generate one more located
	 * other attributes are generated according to other porperties of this class
	 * Will succeed only it hasn't reach the max number of allowed instances
	 */

	 bool newElement(const NLMISC::CVector &pos = CVector::Null					
		, const CVector &speed = CVector::Null) ;					
					
	/**
	 * Delete one located in the container
	 * not present -> nlassert
	 */

	 void deleteElement(uint32 index) ;
	
	 /// Set the maximum mass of located to be generated
	 void setMaxMass(float mass) { _MaxMass = mass ; }
	 /// Get the maximum mass of located to be generated	 
	 float getMaxMass(void) const { return _MaxMass ; }
     /// Set the minimum mass of located to be generated
	 void setMinMass(float mass) { _MinMass = mass ; }
	 /// Get the minimum mass of located to be generated	 
	 float getMinMass(void) const { return _MinMass ; }
	 /// Set both min and max mass
	 float setMass(float min, float max)
	 {
		 _MinMass = min ;
		 _MaxMass = max ;
	 }



	 /** Set the duration of locateds.
	  *  They must not be immortal -> nlassert
	  */

	 void setLifeTime(CAnimationTime min, CAnimationTime max)
	 {
		 nlassert(!_LastForever) ;
		 _MaxLife = max ;
		 _MinLife = min ;
	 }

	 /// Retrieve min duration of locateds. They must not be immortal -> nlassert
	 CAnimationTime getMinLife(void) const 
	 { 
		 nlassert(!_LastForever) ;
		 return _MinLife ; 	 
	 }
	 /// Retrieve max duration of locateds. They must not be immortal -> nlassert
	 CAnimationTime getMaxLife(void) const 
	 { 
		 nlassert(!_LastForever) ;
		 return _MaxLife ; 
	 }


	 /// set/unset immortality for located
	 void setLastForever(bool isImmortal = true) { _LastForever = isImmortal ; }
	 /// retrieve immortality for locateds
	 bool getLastForever(void) const { return _LastForever ; }

	 
	 


	/**
	 * process the system
	 */
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;

	virtual ~CPSLocated() {}


	/**
	 * Resize the located container, in order to accept more instances
	 */
	 void resize(uint32 newSize) ;

protected:	

	// container of all object that are bound to a located
	typedef std::vector< CSmartPtr<CPSLocatedBindable> > TLocatedBoundCont ;


	// the list of all located
	TLocatedBoundCont _LocatedBoundCont ;

	// max number of instance in the container
	uint32 _MaxSize ;

	// current number of instances in the container

	uint32 _Size ;

	// true if the system basis is used for display
	bool _LocalBasisEnabled ; 

	//  = true if the located can't die (gravity for instance...)
	bool _LastForever ;

	// needed atributes for a located

	TPSAttribFloat _Mass ;
	TPSAttribVector  _Pos  ;
	TPSAttribVector  _Speed ;
	TPSAttribTime  _Time  ;
	TPSAttribTime  _TotalTime  ;

	// the ratio _Time / _TotalTime give us the ability to retrieve a life percentage ;
	// _Time won't increase in case of located with unlimited life time, so the ratio will always be 0


	float _MinMass, _MaxMass ;
	float _MaxLife, _MinLife ;



} ;






//******************************************************************************************
//******************************************************************************************
//******************************************************************************************



// bindable types are not encoded as an enum in order to provide
// easy extensibility

const uint32 PSForce = 0    ;
const uint32 PSParticle = 1 ;
const uint32 PSEmitter = 2 ;
const uint32 PSLight = 3 ;
const uint32 PSZone  = 4 ;


/**
* an instance of these class can be bound to a particle system located
* this include forces, particle, and so on...
*/

class CPSLocatedBindable : public NLMISC::CRefCount
{
public:
	/**
	*  Gives the type for this bindable.
	*  types are encoded as constant uint32
	*/
	virtual uint32 getType(void) const = 0 ;

	/**
	* Get the priority of the bindable
	* The more high it is, the earlier it is dealt with
	*/
	virtual uint32 getPriority(void) const = 0 ;
	
	/// ctor. Need a ref to the object it is binded to
	CPSLocatedBindable(CPSLocated *psl) ;	
		
	
	
	/// process one pass for this bindable
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) = 0 ;


protected:    

	friend class CPSLocated ;

	/**	Generate a new element for this bindable. They are generated according to the propertie of the class	
	 * \return true if it could be added
	 */
	virtual bool newElement(void) = 0 ;


	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */

	virtual void deleteElement(uint32 index) = 0 ;

	/// Resize the bindable attributes containers
	virtual resize(uint32 size) = 0 ;
	
} ;


/**
* less operator on located bindable. They're sorted in decreasing priority order
*/

bool operator<(const CPSLocatedBindable &lhs, const CPSLocatedBindable &rhs)
{
	return rhs.getPriority() > lhs.getPriority() ;
}



} // NL3D


#endif // NL_PARTICLE_SYSTEM_LOCATED_H

/* End of particle_system_located.h */
