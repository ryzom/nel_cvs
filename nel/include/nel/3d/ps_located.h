/** \file particle_system_located.h
 * <File description>
 *
 * $Id: ps_located.h,v 1.6 2001/05/08 13:37:08 vizerie Exp $
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

#include <stack>
#include <memory>

#include "nel/misc/types_nl.h"
#include "nel/3d/particle_system.h"
#include "nel/3d/ps_attrib.h" // an attribute template container
#include "nel/3d/nelu.h"
#include "nel/misc/vector.h"
#include "nel/misc/stream.h"
#include "nel/misc/aabbox.h"
 

namespace NL3D 
{


using NLMISC::CVector ;
class CPSLocatedBindable ;
class CPSTargetLocatedBindable ;
class CParticleSystem ;
class CPSZone ;




const uint32 DefaultMaxLocatedInstance = 1 ; // the default value for a located container



/// This structure helps to perform the collision step, by telling which collisionner is the nearest if there are several candidate
/// a distance of -1 indicates that no collisions occured

struct CPSCollisionInfo
{
	// distance to the collisionner along the speed vector
	float   dist ;	
	// new pos and speed, valid if a collision occured
	CVector newPos, newSpeed ;

	CPSCollisionInfo()
	{
		reset() ;
	}
	void reset(void)
	{
		dist = -1 ;
	}

	 void serial(NLMISC::IStream &f)
	 {
		f.serialVersion(1) ;
		f.serial(dist, newPos, newSpeed) ;
	 }
} ;


/// a container of collision infos
typedef CPSAttrib<CPSCollisionInfo> TPSAttribCollisionInfo ;



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

	/// dtor

	virtual ~CPSLocated() ;

	/**
	* attach a bindable object to this located, such as a force or a particle
	* a bindable must be attached only once (-> nlassert)
	* the bindable is then owned by the system
	* and will be deleted by it
	*/
	void bind(CPSLocatedBindable *lb) ;

	/** remove a bound object from the located
	*  if the object doesnt exist -> nlassert
	*  it is deleted
	*/

	void remove(const CPSLocatedBindable *lb) ;

	/**
	* count the number of bound objects
	*/
	uint32 getNbBoundObjects(void) const { return _LocatedBoundCont.size() ; }

	/**
	* get a reference to a bound object
	*/
	const CPSLocatedBindable *getBoundObject(uint32 index) const 
	{
		nlassert(index < _LocatedBoundCont.size()) ;
		return _LocatedBoundCont[index] ;
	}


	/**
	* Generate one more instance in a located.
	* The coordinate are given in the chosen basis for the located.
	* If the emitterLocated ptr is not null, then the coordinate are taken from the emitterLocated basis 
	* and are expressed in this located basis. 
	* other attributes are generated according to other properties of this class
	* Will succeed only if it hasn't reach the max number of allowed instances
	* return will be -1 if call failed or an index to the created object.
	* Index is only valid after creation. Any processing pass on the system will make it invalid.
	* It can be used with any attribute modification method of located and located bindable
	* \param indexInEmitter The index of the emitter (in the emitterLocated object)
	*/

	sint32 newElement(const NLMISC::CVector &pos = CVector::Null					
	, const CVector &speed = CVector::Null, CPSLocatedBindable *emitterLocated = NULL, uint32 indexInEmitter = 0) ;					


	/**
	* Delete one located in the container
	* not present -> nlassert
	*/

	void deleteElement(uint32 index) ;


	/**	 
	* Get the index of the new element that is created
	* Valid only after the newElement method (overridable) of a LocatedBindable is called 
	*: you get the index of the located being generated, if you need its pos, speed, or mass. 
	*/

	uint32 getNewElementIndex(void) const { return _Size ; }


	/** Compute the aabbox of this located, (expressed in world basis
	*  \return true if there is any aabbox
	*  \param aabbox a ref to the result box
	*/

	bool computeBBox(NLMISC::CAABBox &aabbox) const ;

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
	 *  Any previous call to setLastForever() is discraded
	 *  They must not be immortal -> nlassert
	 */

	void setLifeTime(CAnimationTime min, CAnimationTime max)
	{
		_LastForever = false ;
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

	/// set immortality for located
	void setLastForever(void) 
	{ 
		_LastForever = true ; 
	}
	/// retrieve immortality for locateds
	bool getLastForever(void) const { return _LastForever ; }

	/// get mass inverse attrib ref
	TPSAttribFloat &getInvMass(void) { return _InvMass ; }
	/// get mass inverse attrib const ref
	const TPSAttribFloat &getInvMass(void) const { return _InvMass ; }

	/// get Pos attrib ref
	TPSAttribVector &getPos(void) { return _Pos ; }
	/// get Pos attrib const ref
	const TPSAttribVector &getPos(void) const { return _Pos ; }

	/// get Speed attrib ref
	TPSAttribVector &getSpeed(void) { return _Speed ; }
	/// get Speed attrib const ref
	const TPSAttribVector &getSpeed(void) const { return _Speed ; }

	/// get Time attrib ref
	TPSAttribTime &getTime(void) { return _Time ; }
	/// get Time attrib const ref
	const TPSAttribTime &getTime(void) const { return _Time ; }

	/// get TotalTime attrib ref
	TPSAttribTime &getTimeIncrement(void) { return _TimeIncrement ; }
	/// get TotalTime attrib const ref
	const TPSAttribTime &getTimeIncrement(void) const { return _TimeIncrement ; }

	/**
	* process the system
	*/
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;



	/// get the current number of instance in this located container
	uint32 getSize(void) const 
	{ 
		return _Size ; 
	}

	/** get the max number of instance in this located container
	 *	\see resize()
	 */
	uint32 getMaxSize(void) const 
	{ 
		return _MaxSize ; 
	}

	/**
	* Resize the located container, in order to accept more instances
	*/
	void resize(uint32 newSize) ;

	/// serialization	 
	void serial(NLMISC::IStream &f) ;

	/// Shortcut to get an instance of the 3d driver
	IDriver *getDriver() const { return CNELU::Driver ;  }



	NLMISC_DECLARE_CLASS(CPSLocated) ; 

	/// Setup the driver model matrix. It is set accrodingly to the basis of the located
	void setupDriverModelMatrix(void)  ;

	/** Compute a vector that will map to (1 0 0) after view and model transform.
	*  This allow to  have object that always faces the user, whatever basis they are in
	*/
	inline CVector computeI(void) const  ;
	/** Compute a vector that will map to (0 0 1) after view and model transform.
	*  This allow to  have object that always faces the user, whatever basis they are in
	*/
	inline CVector computeK(void) const  ;

	/** call this if you need collision infos.
	*  The collide info attribute is not included by default to save memory.
	*  The first call will create the attribute, and others will add references.
	*  You can then access the infos by calling getCollisioInfo
	*  You must call releaseCollideInfo after use.
	*/

	void queryCollisionInfo(void) ;

	/// Release the collideInfos attribute

	void releaseCollisionInfo(void) ;


	/// get a ref to the collision infos
	TPSAttribCollisionInfo &getCollisionInfo(void)
	{
		nlassert(_CollisionInfo) ;
		return *_CollisionInfo ;
	}

	/// get a const ref to the collision infos
	const TPSAttribCollisionInfo &getCollisionInfo(void) const
	{
		nlassert(_CollisionInfo) ;
		return *_CollisionInfo ;
	}


	/** A collider must call this when a collision occurs
	*  If the collider was nearer that another one it will be taken in account
	*  \index the index of instance that collided
	*/

	inline void collisionOccured(const CPSCollisionInfo &ci, uint32 index) ;

	/** get a matrix that helps to express located B coordinate in located A basis
	*  A and B must belong to the same system
	*/
	static inline const CMatrix &getConversionMatrix(const CPSLocated *A, const CPSLocated *B) ;



	 
	/** Register a dtor observer; (that derives from CPSTargetLocatedBindable)
	*  Each observer will be called when this object dtor is called
	*  This allow for objects that hold this as a target to know when the located is suppressed
	*  (example : collision objects hold located as targets)
	*  When an observer is detroyed, it MUST call removeDtorObserver, unless the located has been destroyed
	*  which will be notified by the call of releaseTargetRsc in CPSTargetLocatedBindable
	*  The same observer can only register once, otherwise, an assertion occurs	
	*/

	void registerDtorObserver(CPSTargetLocatedBindable *observerInterface) ;	


	/// remove a dtor observer (not present -> nlassert)
	void unregisterDtorObserver(CPSTargetLocatedBindable *anObserver) ;



	  

protected:		

	// container of all object that are bound to a located
	typedef std::vector< CPSLocatedBindable *> TLocatedBoundCont ;


	// the list of all located
	TLocatedBoundCont _LocatedBoundCont ;

	// max number of instance in the container
	uint32 _MaxSize ;

	// current number of instances in the container

	uint32 _Size ;

	
	//  = true if the located can't die (gravity for instance...)
	bool _LastForever ;

	// needed atributes for a located

	// a container of masses. the inverse for mass are used in order to speed up forces computation
	TPSAttribFloat _InvMass ; 
	TPSAttribVector  _Pos  ;
	TPSAttribVector  _Speed ;
	TPSAttribTime  _Time  ;
	TPSAttribTime  _TimeIncrement  ;
	/** Used to solve collision detection
	 *  it is not always instanciated
	 */
	TPSAttribCollisionInfo *_CollisionInfo ;



	// nb of users of the _CollisionInfo field
	uint32 _CollisionInfoNbRef ;

	// the ratio _Time / _TimeIncrement give us the ability to retrieve a life percentage ;
	// _Time won't increase in case of located with unlimited life time, so the ratio will always be 0

	float _MinMass, _MaxMass ;
	float _MaxLife, _MinLife ;


	/// used internally to record the request of creation of new posted located
	struct CPostNewElementRequestInfo
	{
		CVector _Pos ;
		CVector _Speed ;
		void serial(NLMISC::IStream &f)
		{
			f.serial(_Pos, _Speed) ;
		}			
		CPostNewElementRequestInfo(const CVector &pos = CVector::Null, const CVector &speed = CVector::Null) : _Pos(pos), _Speed(speed) {}		
	} ;

	typedef std::stack<CPostNewElementRequestInfo> TNewElementRequestStack ;

	/// this stack is used after each update to generate located
	TNewElementRequestStack _RequestStack ;

	/// generate the located that were posted
	void updateNewElementRequestStack(void) ;


	/**
	 * this is used to tell wether a newElement or a deleteElement if being performed
	 * During these method, creation should use postNewElement to generate new elements
	 * Because of the data structures (each located bindable are updated one after each other)
	 * inconsistency can happen, when a located generated another located of the same type
	 * when he's deleted.
	 */

	 bool _UpdateLock ;	

	 /**
	 * Post a request for the generation of a new located.
	 * it is called by newElement when _UpdateLock is set
	 * (when called during newElement or deleteElement)
	 */

	 void postNewElement(const NLMISC::CVector &pos = CVector::Null					
		, const CVector &speed = CVector::Null) ;


	 /// this prepare the located ofr collision tests
	 void resetCollisionInfo(void) ;
	
	 typedef std::vector<CPSTargetLocatedBindable *> TDtorObserversVect ;

	TDtorObserversVect _DtorObserversVect ;
} ;



///////////////////////////////////////
// IMPLEMENTATION OF INLINE METHODS  //
///////////////////////////////////////


inline CVector CPSLocated::computeI(void) const 
{
	if (!_SystemBasisEnabled)
	{
		return _Owner->getViewMat().getI() ;
	}
	else
	{
		// we must express the I vector in the system basis, so we need to multiply it by the inverted matrix of the system
		return _Owner->getInvertedSysMat().mulVector(_Owner->getViewMat().getI()) ;
	}
}


inline CVector CPSLocated::computeK(void) const
{
	if (!_SystemBasisEnabled)
	{
		return _Owner->getViewMat().getK() ;
	}
	else
	{
		// we must express the K vector in the system basis, so we need to multiply it by the inverted matrix of the system
		return _Owner->getInvertedSysMat().mulVector(_Owner->getViewMat().getK()) ;
	}
}


inline void CPSLocated::collisionOccured(const CPSCollisionInfo &ci, uint32 index)
{
	nlassert(_CollisionInfo) ;
	CPSCollisionInfo  &firstCi = (*_CollisionInfo)[index] ;
	if (firstCi.dist == -1 || ci.dist < firstCi.dist)
	{
		firstCi = ci ;
	}
}

/// get a matrix that helps to express located B coordinate in located A basis
inline const CMatrix &CPSLocated::getConversionMatrix(const CPSLocated *A, const CPSLocated *B)
{
	nlassert(A->_Owner == B->_Owner) ; // conversion must be made between entity of the same system
	if (A->_SystemBasisEnabled == B->_SystemBasisEnabled)
	{
		return CMatrix::Identity ;
	}
	else
	{
		if (B->_SystemBasisEnabled)
		{
			return B->_Owner->getSysMat() ;
		}
		else
		{
			return A->_Owner->getInvertedSysMat() ;
		}


	}
}




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

class CPSLocatedBindable : public NLMISC::IStreamable
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
	
	/// ctor
	CPSLocatedBindable() : _Owner(NULL) {}


	/// serialization
	virtual void serial(NLMISC::IStream &f) ;

	/// dtor

	virtual ~CPSLocatedBindable() {}

	/// process one pass for this bindable
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) = 0 ;

	
	/***
	* The following is used to complete an aabbox that was computed using the located positions
	* You may not need to do anything with that, unless your bindable has a space extents. For exAmple,
	* with a particle which has a radius of 2, you must enlarge the bbox to get the correct one.
	* The default behaviour does nothing
	* \return true if you modified the bbox
	*/


	
	
	virtual bool completeBBox(NLMISC::CAABBox &box) const  { return false  ;}

	/***
	 * Override the following to say that you don't want to be part of a bbox computation
	 */

	virtual bool doesProduceBBox(void) const { return true ; }




	/// shortcut to get an instance of the driver
	 IDriver *getDriver() const { return CNELU::Driver ;  }		



	/// Shortcut to get the font generator if one was set
	 CFontGenerator *getFontGenerator(void)
	 {
		nlassert(_Owner) ;
		return _Owner->getFontGenerator() ;
	 }

	 /// Shortcut to get the font generator if one was set (const version)
	 const CFontGenerator *getFontGenerator(void) const
	 {
		nlassert(_Owner) ;
		return _Owner->getFontGenerator() ;
	 }

 	/// Shortcut to get the font manager if one was set
	 CFontManager *getFontManager(void)
	 {
		nlassert(_Owner) ;
		return _Owner->getFontManager() ;
	 }

	 /// Shortcut to get the font manager if one was set (const version)
	 const CFontManager *getFontManager(void) const
	 {
		nlassert(_Owner) ;
		return _Owner->getFontManager() ;
	 }


	/// Shortcut to get the matrix of the system	
	 

	const CMatrix &getSysMat(void) const 
	{
		nlassert(_Owner) ;		
		return _Owner->getOwner()->getSysMat() ;
	}

	/// shortcut to get the inverted matrix of the system
	
	const CMatrix &getInvertedSysMat(void) const 
	{
		nlassert(_Owner) ;
			return _Owner->getOwner()->getInvertedSysMat() ;
	
	}

	/** Get the matrix applied to this set of located bindable
	 *  It may be the identity or the system matrix
	 */
	const CMatrix &getLocatedMat(void) const
	{
		nlassert(_Owner) ;
		if (_Owner->isInSystemBasis())
		{
			return _Owner->getOwner()->getSysMat() ;
		}
		else
		{
			return CMatrix::Identity ;
		}
	}

	/** Get the matrix applied to this set of located bindable
	 *  It may be the identity or the inverted system matrix
	 */
	const CMatrix &getInvertedLocatedMat(void) const
	{
		nlassert(_Owner) ;
		if (_Owner->isInSystemBasis())
		{
			return _Owner->getOwner()->getInvertedSysMat() ;
		}
		else
		{
			return CMatrix::Identity ;
		}
	}


	/// shortcut to get the view matrix (trasposed)
	const CMatrix &getViewMat(void) const 
	{
		nlassert(_Owner) ;
		return _Owner->getOwner()->getViewMat() ;	
	}	


	/// shortcut to setup the model matrix (system basis or world basis)
	void setupDriverModelMatrix(void)  
	{
		nlassert(_Owner) ;
		_Owner->setupDriverModelMatrix() ;
	}

	/** Compute a vector that will map to (1 0 0) after view and model transform.
	 *  This allow to  have object that always faces the user, whatever basis they are in
	 */
	inline CVector computeI(void)  const { return _Owner->computeI() ; }
	 /** Compute a vector that will map to (0 0 1) after view and model transform.
	 *  This allow to  have object that always faces the user, whatever basis they are in
	 */
 	 inline CVector computeK(void)  const { return _Owner->computeK() ; }


	 /// get the located that owns this bindable
	 CPSLocated *getOwner(void) { return _Owner ; }

	 /// get the located that owns this bindable (const version)
 	 const CPSLocated *getOwner(void) const { return _Owner ; }
	


protected:    

	friend class CPSLocated ;

	/**	Generate a new element for this bindable. They are generated according to the propertie of the class	
	 * \return true if it could be added
	 */
	virtual bool newElement(void) = 0 ;


	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable (pos etc...) are still accessible 
	 *  for of the index given
	 *  index out of range -> nl_assert
	 */

	virtual void deleteElement(uint32 index) = 0 ;

	/// Resize the bindable attributes containers
	virtual void resize(uint32 size) = 0 ;


	/// set the located that hold this located bindable
	virtual void setOwner(CPSLocated *psl) 
	{ 
		nlassert(psl) ; _Owner = psl ; 
	}

	CPSLocated  *_Owner ;

} ;




/**
* less operator on located bindable. They're sorted in decreasing priority order
*/

inline bool operator<(const CPSLocatedBindable &lhs, const CPSLocatedBindable &rhs)
{
	return rhs.getPriority() > lhs.getPriority() ;
}



//******************************************************************************************
//******************************************************************************************
//******************************************************************************************


/** This class is a located bindable that can focus on several target
 *  Can be inherited by bindable like forces or collision zones
 */

class CPSTargetLocatedBindable : public CPSLocatedBindable
{
	public:

		/** Add a new type of located for this force to apply on. nlassert if present
		 *  By overriding this and calling the CPSTargetLocatedBindable version,
		 *  you can also send some notficiation to the object that's being attached
		 */

		virtual void attachTarget(CPSLocated *ptr) ;

		/** Detach a target. If not present -> assert
		 * This also call releaseTargetRsc for clean up
		 */		
		void detachTarget(CPSLocated *ptr) ;

		/// return the number of targets
		uint32 getNbTargets(void) const { return _Targets.size() ; }

		/// Return a ptr on a target. Invalid range -> nlassert
		CPSLocated *getTarget(uint32 index) 
		{
			nlassert(index < _Targets.size()) ;
			return _Targets[index] ;
		}

		/// Return a const ptr on a target. Invalid range -> nlassert
		const CPSLocated *getTarget(uint32 index) const
		{
			nlassert(index < _Targets.size()) ;
			return _Targets[index] ;
		}


		
		/** it is called when a target is destroyed or detached
		 *  Override this if you allocated resources from the target (to release them)
		 *  IMPORTANT : as objects are no polymorphic while being destroyed, this class
		 *  can't call your releaseTargetRsc override in its destructor. You must
		 *  must redefine the destructor and call this method yourself for all target
		 *  see ps_zone.cpp for an example
		 */
		virtual void releaseTargetRsc(CPSLocated *target) {} ;


		/// Seralization, must be called by derivers
		void serial(NLMISC::IStream &f) ;


		virtual ~CPSTargetLocatedBindable() ;
	protected:

		typedef std::vector< CPSLocated *> TTargetCont ;
		TTargetCont _Targets ;	

} ;



} // NL3D


#endif // NL_PARTICLE_SYSTEM_LOCATED_H

/* End of particle_system_located.h */
