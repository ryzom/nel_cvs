/** \file ps_located.h
 * <File description>
 *
 * $Id: ps_located.h,v 1.28 2003/11/18 13:57:30 vizerie Exp $
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

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "3d/particle_system_process.h"
#include "3d/ps_attrib.h" // an attribute template container
#include "3d/ps_lod.h"
#include "nel/misc/stream.h"

namespace NLMISC
{
	class CAABBox;
	class CMatrix;	
}






namespace NL3D 
{



template <class T> class CPSAttribMaker;


class CPSLocatedBindable;
class CPSTargetLocatedBindable;
class CPSZone;
class CPSForce;
class IDriver;
class CFontManager;
class CFontGenerator;
class CScene;


const uint32 DefaultMaxLocatedInstance = 1; // the default value for a located container



/// This structure helps to perform the collision step, by telling which collisionner is the nearest if there are several candidate
/// a distance of -1 indicates that no collisions occured

struct CPSCollisionInfo
{

	float   TimeSliceRatio; /** Tells how mush of this time step is being used.
							   * This is usually 1.0, unless. The object was just emitted.
							   * In this case, this can range from 0 to 1 (When several emission
							   * occured within the same time step)
							   */
	// distance to the collisionner along the speed vector
	float   dist;	
	// new pos and speed, valid if a collision occured
	NLMISC::CVector newPos, newSpeed;

	/** the zone on which the bounce occured...
	 *  can be useful to check the behaviour in case of collision
	 */

	CPSZone *collisionZone;

	CPSCollisionInfo()
	{
		reset();
	}
	void reset(void)
	{
		dist			= -1;
		TimeSliceRatio = 1.0f;
	}

	 void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	 {
		f.serialVersion(1);
		f.serial(dist, newPos, newSpeed);
	 }
};


/// a container of collision infos
typedef CPSAttrib<CPSCollisionInfo> TPSAttribCollisionInfo;





/**
 * this class is a located : it belongs to a particle system, and it represents
 * any kind of object that has a position in the world.
 * A located don't do anything by itself. You must bind objects to it, such as a particle, 
 * a force and so on.
 * It is important to remember that a located holds all instance of object of
 * one type (force, emitter, particles or both...), not only one.
 * Not sharable accross systems
 */
 
class CPSLocated : public CParticleSystemProcess
{
public:
	/// Constructor
	CPSLocated();

	/// dtor

	virtual ~CPSLocated();

	// from CParticleSystemProcess
	virtual bool isLocated() const { return true; }

	/** attach a bindable object to this located, such as a force or a particle
	  * a bindable must be attached only once (-> nlassert)
	  * The bindable is then owned by the system and will be deleted by it.
	  * \return true if the operation could be performed. It can fail when this cause the system the system to last forever,
	  *              which is incompatible with the 'BypassMaxNumIntegrationSteps' in CParticleSystem
	  */
	bool bind(CPSLocatedBindable *lb);

	/** Detach a bindable object from this located. Ownership is transferred to the caller	 
	  * Any reference the object may have in the system is lost (targets..)
	  * After that is may be inserted an other system.	  
	  */
	CPSLocatedBindable *unbind(uint index);

	/// test wether a located bindable is attached to that object
	bool isBound(const CPSLocatedBindable *lb) const;

	/** Get the index of a located bindable that is bound to that object.
	  * If it isn't bound, an assertion is raised
	  */
	uint getIndexOf(const CPSLocatedBindable *lb) const;

	/** remove a bound object from the located
	*  if the object doesnt exist -> nlassert
	*  it is deleted
	*/
	void remove(const CPSLocatedBindable *lb);

	/** From CParticleSystemProcess.
	  * Release any reference this located may have on the given process.
	  * For example, this is used when detaching a located of a system.
	  */
	virtual	void			 releaseRefTo(const CParticleSystemProcess *other);

	/** From CParticleSystemProcess.
	  * Release any reference this located may have to other process of the system
	  * For example, this is used when detaching a process of a system.
	  */
	virtual void			 releaseAllRef();


	/**
	* count the number of bound objects
	*/
	uint32 getNbBoundObjects(void) const { return _LocatedBoundCont.size(); }

	/**
	* get a pointer to a bound object (const version)
	*/
	const CPSLocatedBindable *getBoundObject(uint32 index) const 
	{
		nlassert(index < _LocatedBoundCont.size());
		return _LocatedBoundCont[index];
	}


	/**
	* get a pointer to a bound object
	*/
	CPSLocatedBindable *getBoundObject(uint32 index)
	{
		nlassert(index < _LocatedBoundCont.size());
		return _LocatedBoundCont[index];
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
	* \param basisConversionForSpeed : if false, the speed vector is taken as if even if emitter and emittee basis are differents.
	* \param ellapsedTime
	*/

	sint32 newElement(const NLMISC::CVector &pos,
		              const NLMISC::CVector &speed,
					  CPSLocated *emitterLocated,
					  uint32 indexInEmitter,
					  TPSMatrixMode speedCoordSystem,
					  TAnimationTime ellapsedTime);


	/**
	* Delete one located in the container
	* not present -> nlassert
	*/

	void deleteElement(uint32 index);


	/// shortcut to get the scene
	CScene *getScene(void);

	/// shortcut to the same method of the owning particle system
	void getLODVect(NLMISC::CVector &v, float &offset, TPSMatrixMode matrixMode);


	/**  Shorcut to increase the particle counter (number of particle drawn, for benchmark purpose )
	  *  should be called only by bound object that display particles
	  */
	void incrementNbDrawnParticles(uint num);


	/**	 
	* Get the index of the new element that is created
	* Valid only after the newElement method (overridable) of a LocatedBindable is called 
	*: you get the index of the located being generated, if you need its pos, speed, or mass. 
	*/

	uint32 getNewElementIndex(void) const { return _Size; }


	/** Compute the aabbox of this located, (expressed in world basis
	*  \return true if there is any aabbox
	*  \param aabbox a ref to the result box
	*/

	bool computeBBox(NLMISC::CAABBox &aabbox) const;



	/** Set the duration of locateds.
	 *  Any previous call to setLastForever() is discarded
	 *  Any previous scheme for lifetime is dicarded	 
	 */
	void setInitialLife(TAnimationTime lifeTime);

	/** Set a scheme (allocated by new, released by that object) that generate the duration of locateds.
	 *  Such a scheme can't own its memory.
	 *  Any previous call to setLastForever() is discarded
	 *  Any previous scheme for lifetime is discarded	 
	 */
	void setLifeScheme(CPSAttribMaker<float> *scheme);

	/// get the life of created particles (valid if they have a limited life time)
	TAnimationTime getInitialLife(void) const { return _InitialLife; }

	/// get the life scheme of created particle, null if none (valid if they have a limited life time)
	CPSAttribMaker<float> *getLifeScheme(void) { return _LifeScheme; }
	const CPSAttribMaker<float> *getLifeScheme(void) const { return _LifeScheme; }


	/** Set the mass of locateds.	 
	 *  Any previous scheme for Mass is dicarded	 
	 */
	void setInitialMass(float mass);

	/** Set a scheme (allocated by new, released by that object) that generate the mass of locateds.	 
	 *  Such a scheme can't own its memory.
	 *  Any previous scheme for Mass is discarded	 
	 */
	void  setMassScheme(CPSAttribMaker<float> *scheme);

	/// get the mass of created particle
	float getInitialMass(void) const { return _InitialMass; }

	/// get the scheme that compute mass of created particles, null if none
	CPSAttribMaker<float> *getMassScheme(void) { return _MassScheme; }
	const CPSAttribMaker<float> *getMassScheme(void) const { return _MassScheme; }

	

	/** set immortality for located
	  * \see setInitialLife
	  * \return true if the operation could be performed. It can fail when this cause the system the system to last forever,
	  *              which is incompatible with the 'BypassMaxNumIntegrationSteps' in CParticleSystem 
	  */
	bool setLastForever(); 
	/// retrieve immortality for locateds
	bool getLastForever(void) const { return _LastForever; }

	/// get mass inverse attrib ref
	TPSAttribFloat &getInvMass(void) { return _InvMass; }
	/// get mass inverse attrib const ref
	const TPSAttribFloat &getInvMass(void) const { return _InvMass; }

	/// get Pos attrib ref
	TPSAttribVector &getPos(void) { return _Pos; }
	/// get Pos attrib const ref
	const TPSAttribVector &getPos(void) const { return _Pos; }

	/// get Speed attrib ref
	TPSAttribVector &getSpeed(void) { return _Speed; }
	/// get Speed attrib const ref
	const TPSAttribVector &getSpeed(void) const { return _Speed; }

	/// get Time attrib ref
	TPSAttribTime &getTime(void) { return _Time; }
	/// get Time attrib const ref
	const TPSAttribTime &getTime(void) const { return _Time; }

	/// get TotalTime attrib ref
	TPSAttribTime &getTimeIncrement(void) { return _TimeIncrement; }
	/// get TotalTime attrib const ref
	const TPSAttribTime &getTimeIncrement(void) const { return _TimeIncrement; }

	/**
	* process the system
	*/
	virtual void step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt);



	/// get the current number of instance in this located container
	uint32 getSize(void) const 
	{ 
		return _Size; 
	}

	/** get the max number of instance in this located container
	 *	\see resize()
	 */
	uint32 getMaxSize(void) const 
	{ 
		return _MaxSize; 
	}

	/** set the Refresh Rate of this located. Default is motion every frame (frameToSkip = 0)
	 *  The drawing process will still occur every frame anyway...
	 *  It's a speed / quality tradeof
	 */

	 void setFrameRate(uint32 nbFramesToSkip = 0) { _NbFramesToSkip = nbFramesToSkip; }

	 // retrieve the frame rate
	 uint32 getFrameRate(void) const { return _NbFramesToSkip; }


	/**
	* Resize the located container, in order to accept more instances
	*/
	void resize(uint32 newSize);

	/// serialization	 
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// Shortcut to get an instance of the 3d driver
	IDriver *getDriver() const;
	
	/// shorcut to get a user param that was set in the owner system
	float getUserParam(uint numParam) const;



	NLMISC_DECLARE_CLASS(CPSLocated); 

	/// Setup the driver model matrix. It is set accrodingly to the basis of the located
	void setupDriverModelMatrix(void) ;

	/** Compute a vector that will map to (1 0 0) after view and model transform.
	*  This allow to  have object that always faces the user, whatever basis they are in
	*/
	NLMISC::CVector computeI(void) const ;

	/** Compute a vector that will map to (0 1 0) after view and model transform.
	*  This allow to  have object that always faces the user, whatever basis they are in
	*/
	NLMISC::CVector computeJ(void) const ;

	/** Compute a vector that will map to (0 0 1) after view and model transform.
	*  This allow to  have object that always faces the user, whatever basis they are in
	*/
	NLMISC::CVector computeK(void) const ;

	/** call this if you need collision infos.
	*  The collide info attribute is not included by default to save memory.
	*  The first call will create the attribute, and others will add references.
	*  You can then access the infos by calling getCollisioInfo
	*  You must call releaseCollideInfo after use.
	*/

	void queryCollisionInfo(void);

	/// Release the collideInfos attribute

	void releaseCollisionInfo(void);

	/// test wether this located has collision infos
	bool hasCollisionInfos() const { return _CollisionInfo != NULL; }

	/// get a ref to the collision infos
	TPSAttribCollisionInfo &getCollisionInfo(void)
	{
		nlassert(_CollisionInfo);
		return *_CollisionInfo;
	}

	/// get a const ref to the collision infos
	const TPSAttribCollisionInfo &getCollisionInfo(void) const
	{
		nlassert(_CollisionInfo);
		return *_CollisionInfo;
	}


	/** A collider must call this when a collision occurs
	*  If the collider was nearer that another one it will be taken in account
	*  \index the index of instance that collided
	*/

	inline void collisionUpdate(const CPSCollisionInfo &ci, uint32 index);

	// get a conversion matrix between 2 matrix modes		
	static const NLMISC::CMatrix &getConversionMatrix(const CParticleSystem &ps, TPSMatrixMode to, TPSMatrixMode from);

	/** get a matrix that helps to express located B coordinate in located A basis
	*  A and B must belong to the same system
	*/
	static const NLMISC::CMatrix &getConversionMatrix(const CPSLocated *A, const CPSLocated *B);
	
	
	const NLMISC::CMatrix &getLocalToWorldMatrix() const;

	const NLMISC::CMatrix &getWorldToLocalMatrix() const;
	
	 
	/** Register a dtor observer; (that derives from CPSLocatedBindable)
	*  Each observer will be called when this object dtor is called (call of method notifyTargetRemoved() )
	*  This allow for objects that hold this as a target to know when it is suppressed
	*  (example : collision objects hold located as targets)
	*  When an observer is detroyed, it MUST call unregisterDtorObserver,
	*  The same observer can only register once, otherwise, an assertion occurs	
	*/

	void registerDtorObserver(CPSLocatedBindable *observer);	


	/** remove a dtor observer (not present -> nlassert)
	 *  see register dtor observer
	 */
	void unregisterDtorObserver(CPSLocatedBindable *anObserver);


	 
		 /// set the located bindable name (edition purpose)
	void setName(const std::string &name) { _Name = name; }

	/// get the located bindable name (edition purpose)
	std::string getName(void) const { return _Name; }


	/// tells wether there are alive entities / particles in the system
	virtual bool hasParticles(void) const;

	/// tells wether there are alive emitters / particles in the system
	virtual bool hasEmitters(void) const;

	/** Enable the to force LOD degradation. This will suppress instances immediately, (during the motion pass)  so that
	  * there won't be more than maxNbInstance * dist / maxDist instances. This may not be desirable
	  * every time since particle dissapear on screen, which may be noticeable.	
	  */

	void forceLODDegradation(bool enable = true) { _LODDegradation = enable; }

	/** Test whether LOD degradation was activated
	  * \see forceLODDegradation()
	  */
	bool hasLODDegradation(void) const { return _LODDegradation; }


	/// for the CPSLocated to reevaluate the max number of faces it may need
	void notifyMaxNumFacesChanged(void);

	/// ask for the max number of faces the located wants (for LOD balancing)
	virtual uint querryMaxWantedNumFaces(void);

	// Inherited from CParticlesystemProcess. Change the coord system for thta system.
	virtual void setMatrixMode(TPSMatrixMode matrixMode);	

	/// Test wether this located support parametric motion
	bool         supportParametricMotion(void) const;

	/** When set to true, this tells the system to use parametric motion. This is needed in a few case only,
	  * and can only work if all the forces that apply to the system are integrable. An assertion happens otherwise
	  */
	void		 enableParametricMotion(bool enable = true);

	/// test wether parametric motion is enabled
	bool		 isParametricMotionEnabled(void) const { return _ParametricMotion;}

	/// inherited from CParticlesystemProcess perform parametric motion for this located to reach the given date
	virtual void performParametricMotion(TAnimationTime date, TAnimationTime ellapsedTime, TAnimationTime realEllapsedTime);

	/// make the particle older of the given amount. Should not be called directly, as it is called by the system during its step method
	virtual void updateLife(TAnimationTime ellapsedTime);

	/** Compute the trajectory of the given located.
	  * NB : only works with object that have parametric trajectories
	  */
	void integrateSingle(float startDate, float deltaT, uint numStep,						 
						 uint32 indexInLocated,
						 NLMISC::CVector *destPos,						
						uint posStride = sizeof(NLMISC::CVector));

	/// Enable a trigger on death. It is used to create emission on an emitter with a given ID
	void				enableTriggerOnDeath(bool enable = true) { _TriggerOnDeath = enable; }

	/// Test wether a trigger on death has been enabled
	bool                isTriggerOnDeathEnabled(void) const { return _TriggerOnDeath; }

	/// Set an ID for the emitter to be triggered on death
	void				setTriggerEmitterID(uint32 id) 
	{ 
		nlassert(_TriggerOnDeath);
		_TriggerID = id; 
	}

	/// Get the ID for the emitter to be triggered on death
	uint32				getTriggerEmitterID(void) const 
	{ 
		nlassert(_TriggerOnDeath);
		return _TriggerID; 
	}

	/** eval max duration of the located (if no scheme is used, this is the lifetime)
      * nb : return -1 if located last for ever
	  */
	float				evalMaxDuration() const;

	// from CParticleSystemProcess
	 virtual uint	getFatherSkelMatrixUsageCount() const;	
	
protected:


	friend class CPSForce; // this is intended only for integrable forces that want to use
						   // registerIntegrableForce, and removeIntegrableForce

	/// cache the max number of faces this located may want
	uint32 _MaxNumFaces;

	std::string _Name;
	
	// number of frame to skip between motion ...
	uint32 _NbFramesToSkip;

	// container of all object that are bound to a located
	typedef std::vector< CPSLocatedBindable *> TLocatedBoundCont;


	// the list of all located
	TLocatedBoundCont _LocatedBoundCont;

	// max number of instance in the container
	uint32 _MaxSize;

	// current number of instances in the container

	uint32 _Size;

	
	//  = true if the located can't die (gravity for instance...)
	bool _LastForever;

	// needed atributes for a located

	// a container of masses. the inverse for mass are used in order to speed up forces computation
	TPSAttribFloat		_InvMass; 
	TPSAttribVector		_Pos ;
	TPSAttribVector		_Speed;
	TPSAttribTime		_Time ;
	TPSAttribTime		_TimeIncrement ;


	public:;

	/** WARNING : private use by forces only. This struct is used for parametric trajectory. These kind of trajectory can only be computed in a few case,
	  * but are useful in some cases.
	  */
	struct CParametricInfo
	{
		CParametricInfo() {}
		CParametricInfo(NLMISC::CVector pos, NLMISC::CVector speed, float date)
			: Pos(pos), Speed(speed), Date(date)
		{
		}
		NLMISC::CVector	Pos;   // the inital pos of emission
		NLMISC::CVector	Speed; // the inital direction of emission
		TAnimationTime  Date;  // the initial date of emission
	};

	/// WARNING : private use by forces only
	typedef CPSAttrib<CParametricInfo> TPSAttribParametricInfo;

	/** WARNING : private use by forces only. this vector is only used if parametric motion is achievable and enabled, because of the extra storage space
	  *
	  */
	CPSAttrib<CParametricInfo>  _PInfo;

	protected:

	/** Used to solve collision detection
	 *  it is not always instanciated
	 */
	TPSAttribCollisionInfo *_CollisionInfo;



	// nb of users of the _CollisionInfo field
	uint32 _CollisionInfoNbRef;

		
	// the life to use, or a scheme that generate it
	// if the scheme if null, initial life is used instead
	float _InitialLife;
	CPSAttribMaker<float> *_LifeScheme;

	// the mass to use, or a scheme that generate it
	// if the scheme if null, initial mass is used instead
	float _InitialMass;
	CPSAttribMaker<float> *_MassScheme;

	


	/// used internally to record the request of creation of new posted located
	struct CPostNewElementRequestInfo
	{
		NLMISC::CVector _Pos;
		NLMISC::CVector _Speed;
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serial(_Pos, _Speed);
		}			
		CPostNewElementRequestInfo(const NLMISC::CVector &pos = NLMISC::CVector::Null, const NLMISC::CVector &speed = NLMISC::CVector::Null) : _Pos(pos), _Speed(speed) {}		
	};

	typedef std::stack<CPostNewElementRequestInfo> TNewElementRequestStack;

	/// this stack is used after each update to generate located
	TNewElementRequestStack _RequestStack;

	/// generate the located that were posted
	void updateNewElementRequestStack(void);


	/**
	 * this is used to tell wether a newElement or a deleteElement if being performed
	 * During these method, creation should use postNewElement to generate new elements
	 * Because of the data structures (each located bindable are updated one after each other)
	 * inconsistency can happen, when a located generated another located of the same type
	 * when he's deleted.
	 */

	 bool _UpdateLock;	

	 /**
	 * Post a request for the generation of a new located.
	 * it is called by newElement when _UpdateLock is set
	 * (when called during newElement or deleteElement)
	 */

	 void postNewElement(const NLMISC::CVector &pos = NLMISC::CVector::Null					
		, const NLMISC::CVector &speed = NLMISC::CVector::Null);


	 /// this prepare the located ofr collision tests
	 void resetCollisionInfo(void);
	
	 typedef std::vector<CPSLocatedBindable *>			TDtorObserversVect;
	 TDtorObserversVect									_DtorObserversVect;

	 /// true when LOD degradation apply to this located
	 bool												_LODDegradation;

	 /** number of force, and zones etc. that are not integrable over time. If this is not 0, then the trajectory
	   * cannot be computed at any time. A force that is integrable must be in the same basis than the located.
	   */
	 uint16												_NonIntegrableForceNbRefs;
	 /// number of forces that apply on that located that have the same basis that this one (required for parametric animation)
	 uint16												_NumIntegrableForceWithDifferentBasis;
	 /// a vector of integrable forces that apply on this located
	 typedef std::vector<CPSForce *>					TForceVect;
	 TForceVect											_IntegrableForces;
	 bool												_TriggerOnDeath;
	uint32												_TriggerID;

	 /// When set to true, this tells the system to use parametric motion. Only parametric forces must have been applied.
	 bool _ParametricMotion;

	 /// allocate parametric infos
	 void allocateParametricInfos(void);

	 /// release paametric infos
	 void releaseParametricInfos(void);

	 /// notify the attached object that we have switch between parametric / incremental motion
	 void notifyMotionTypeChanged(void);

	 // for debug : check that system integrity is ok, otherwise -> assert
	 void checkIntegrity() const;

public:
	 /// PRIVATE USE: register a force that is integrable on this located. It must have been registered only once
	 void registerIntegrableForce(CPSForce *f);

	 /// PRIVATE USE: unregister a force that is integrable with this located
	 void unregisterIntegrableForce(CPSForce *f);

	 /// PRIVATE USE: says that an integrable force basis has changed, and says which is the right basis
	 void integrableForceBasisChanged(TPSMatrixMode basis);

	 /// PRIVATE USE: add a reference count that says that non-integrable forces have been added
	 void addNonIntegrableForceRef(void);

	 /// PRIVATE USE: decrease the reference count to say that a non-integrable force has been removed.
	 void releaseNonIntegrableForceRef(void);

	 /// PRIVATE USE : access to parametric infos
	 TPSAttribParametricInfo &getParametricInfos() { return _PInfo; }

	 /// PRIVATE USE : called by the system when its date has been manually changed
	 virtual void	systemDateChanged();
};



///////////////////////////////////////
// IMPLEMENTATION OF INLINE METHODS  //
///////////////////////////////////////

inline void CPSLocated::collisionUpdate(const CPSCollisionInfo &ci, uint32 index)
{
	nlassert(_CollisionInfo);
	CPSCollisionInfo  &firstCi = (*_CollisionInfo)[index];
	if (firstCi.dist == -1 || ci.dist < firstCi.dist)
	{
		firstCi = ci;
	}
}




//******************************************************************************************
//******************************************************************************************
//******************************************************************************************



// kind of bindable objects

const uint32 PSForce = 0   ;
const uint32 PSParticle = 1;
const uint32 PSEmitter = 2;
const uint32 PSLight = 3;
const uint32 PSZone  = 4;
const uint32 PSSound = 5;


/**
* an instance of these class can be bound to a particle system located
* this include forces, particle, and so on...
*/

class CPSLocatedBindable : public NLMISC::IStreamable
{
public:	
	///\name Object
	//@{
		/// ctor	
		CPSLocatedBindable();	
		/// serialization
		virtual void		serial(NLMISC::IStream &f) throw(NLMISC::EStream);
		/** this should be called before to delete any bindable inserted in a system, but this is done
		  * by the system, so you should never need calling it. This has been introduced because calls in dtor are not polymorphic
		  * to derived class (which are already destroyed anyway), and some infos are needed in some dtor. The default behaviour does nothing
		  */
		virtual void		finalize(void);
		/// dtor
		virtual ~CPSLocatedBindable();		
	//@}
	/// Activate / Deactivate this object. When not active, the owning system won't try to call the 'step' method
		void					setActive(bool active) { _Active = active; }
		bool					isActive() const { return _Active; }				
  /**
	*  Gives the type for this bindable.
	*  types are encoded as constant uint32
	*/
	virtual uint32			getType(void) const = 0;
	/**
	* Get the priority of the bindable
	* The more high it is, the earlier it is dealt with
	*/
	virtual uint32			getPriority(void) const = 0;			
	/// process one pass for this bindable
	virtual void			step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt) = 0;
	/** Can be used by located bindable that have located as targets (emitter, collision zone, forces)
     *	to be notified that one of their target has been removed.
	 *  To do this :
	 *  The object that focus the target must call registerDTorObserver on the target, with himself as a parameter
	 *  When the target is removed, this target will call this method for all registered CPSLocated
	 *	The default behaviour remove this object as an observer 
	 *   
	 *  \see CPSLocated::registerDTorObserver()
	 */
	virtual void			notifyTargetRemoved(CPSLocated *ptr);	

	/** Release any reference this obj may have on the given process.
	  * For example, this is used when detaching a located bindable from a system.  	 
	  */
	virtual	void			 releaseRefTo(const CParticleSystemProcess *other) {}
	 
	/** Release any reference this obj may have to other process of the system
	  * For example, this is used when detaching a located bindable from a system.	  
	  */
	virtual void			 releaseAllRef();
	/***
	* The following is used to complete an aabbox that was computed using the located positions
	* You may not need to do anything with that, unless your bindable has a space extents. For exAmple,
	* with a particle which has a radius of 2, you must enlarge the bbox to get the correct one.
	* The default behaviour does nothing
	* \return true if you modified the bbox
	*/	
	virtual bool			completeBBox(NLMISC::CAABBox &box) const  { return false ;}
	/***
	 * Override the following to say that you don't want to be part of a bbox computation
	 */
	virtual bool			doesProduceBBox(void) const { return true; }
	/// shortcut to get an instance of the driver
	 IDriver				*getDriver() const 
	 { 
		 nlassert(_Owner);
		 nlassert(_Owner->getDriver());
		 return _Owner->getDriver();
	 }		
	/// Shortcut to get the font generator if one was set
	 CFontGenerator			*getFontGenerator(void)
	 {
		nlassert(_Owner);
		return _Owner->getFontGenerator();
	 }

	 /// Shortcut to get the font generator if one was set (const version)
	 const CFontGenerator	*getFontGenerator(void) const
	 {
		nlassert(_Owner);
		return _Owner->getFontGenerator();
	 }

 	/// Shortcut to get the font manager if one was set
	CFontManager			*getFontManager(void);
	 
	/// Shortcut to get the font manager if one was set (const version)
	const CFontManager		*getFontManager(void) const;	 

	/// Shortcut to get the matrix of the system		 
	const NLMISC::CMatrix	&getSysMat(void) const;	
	/// Shortcut to get the local to world matrix
	const NLMISC::CMatrix	&getLocalToWorldMatrix() const;
	/// shortcut to get the inverted matrix of the system	
	const NLMISC::CMatrix	&getInvertedSysMat(void) const;		
	/// shortcut to get the view matrix
	const NLMISC::CMatrix	&getViewMat(void) const;	
	/// shortcut to get the inverted view matrix
	const NLMISC::CMatrix	&getInvertedViewMat(void) const;	
	/// shortcut to setup the model matrix (system basis or world basis)
	void					setupDriverModelMatrix(void);	
	/** Compute a vector that will map to (1 0 0) after view and model transform.
	 *  This allow to  have object that always faces the user, whatever basis they are in
	 */
	inline NLMISC::CVector	computeI(void)  const { return _Owner->computeI(); }

	/** Compute a vector that will map to (0 1 0) after view and model transform.
	 *  This allow to  have object that always faces the user, whatever basis they are in
	 */
	inline NLMISC::CVector	computeJ(void)  const { return _Owner->computeJ(); }
	 /** Compute a vector that will map to (0 0 1) after view and model transform.
	 *  This allow to  have object that always faces the user, whatever basis they are in
	 */
 	 inline NLMISC::CVector computeK(void)  const { return _Owner->computeK(); }
	 /// get the located that owns this bindable
	 CPSLocated				*getOwner(void) { return _Owner; }
	 /// get the located that owns this bindable (const version)
 	 const CPSLocated		*getOwner(void) const { return _Owner; }		
	 /// set the located bindable name (edition purpose)
	 void					setName(const std::string &name) { _Name = name; }
	/// get the located bindable name (edition purpose)
	std::string				getName(void) const { return _Name; }	
	/** set the LODs that apply to that object (warning : it is based on the position of the system, and don't act on a per instance basis ...)
      * To have per instance precision, you must use an attribute maker that has LOD as its input
	  */
	void					setLOD(TPSLod lod) { _LOD = lod; }
	/// get the valid lods for that object
	TPSLod					getLOD(void) const { return _LOD; }
	/// tells wether there are alive entities / particles
	virtual bool			hasParticles(void) const { return false; }
	/// tells wether there are alive emitters
	virtual bool			hasEmitters(void) const { return false; }
	/** set the extern ID of this located bindable. 0 means no extern access. The map of ID-locatedBindable. Is in th
	  * particle system, so this located bindable must have been attached to a particle system, otherwise an assertion is raised
	  */
	void					setExternID(uint32 id);
	/// get the extern ID of this located bindable
	uint32					getExternID(void) const { return _ExternID; }
	/** Called when the basis of the owner changed. the default behaviour does nothing
	  * \param newBasis : True if in the system basis, false for the world basis.
	  */
	virtual void			basisChanged(TPSMatrixMode systemBasis) {}

	/// called when a located has switch between incrmental / parametric motion. The default does nothing
	virtual	void			motionTypeChanged(bool parametric) {}
	
	// returns the number of sub-objects (including this one, that requires the father skeleton matrix for its computations)
	virtual bool			getFatherSkelMatrixUsageCount() const { return 0; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	friend class CPSLocated;

	/**	Generate a new element for this bindable. They are generated according to the propertie of the class		 
	 */
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) = 0;


	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable (pos etc...) are still accessible 
	 *  for the given index.
	 *  index out of range -> nl_assert
	 */

	virtual void deleteElement(uint32 index) = 0;

	/** Resize the bindable attributes containers
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size) = 0;



	/** a bounce occured, so some action could be done. The default behaviour does nothing
	 *  \param index the index of the element that bounced
	 */
	virtual void bounceOccured(uint32 index) {}

	/** show an drawing to represent the object, and in red if it is selected
	 *  \param tab : a table of 2 * nbSeg vector. only the x and y coordinates are used
	 *  \param nbSeg : the number of segment 
	 *  \param scale  : the scale to use for drawing
	 */

	void displayIcon2d(const NLMISC::CVector tab[], uint nbSegs, float scale);


	/// set the located that hold this located bindable
	virtual void setOwner(CPSLocated *psl);	

protected:
	CPSLocated  *_Owner;
	uint32		_ExternID;
	/// tells when this object must be dealt with
	TPSLod		_LOD; 
	// Name for this bindable
	std::string _Name;
	//
	bool        _Active; // Say if this bindable is active. If not active, the owning system won't try to call 'step' on that object. True by default
public:
	/** PRIVATE USE : called by the system when its date has been manually changed.
	  * This his usually for object that expect time to be always increasing, so that they can reset their datas
	  */
	virtual void			systemDateChanged() {}
};




/**
* less operator on located bindable. They're sorted in decreasing priority order
*/

inline bool operator<(const CPSLocatedBindable &lhs, const CPSLocatedBindable &rhs)
{
	return rhs.getPriority() > lhs.getPriority();
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
	/** Add a new type of located for this to apply on. nlassert if already present.
	 *  You should only call this if this object and the target are already inserted in a system.
	 *  By overriding this and calling the CPSTargetLocatedBindable version,
	 *  you can also send some notificiation to the object that's being attached.
	 */
	virtual void		attachTarget(CPSLocated *ptr);
	/** remove a target
	 *  \see attachTarget
	 */
	void				detachTarget(CPSLocated *ptr)
	{
		notifyTargetRemoved(ptr);
	}
	/** From CPSLocatedBindable.
	  * Release any reference this obj may have on the given process.
	  * For example, this is used when detaching a located of a system.
	  */
	virtual	void			 releaseRefTo(const CParticleSystemProcess *other);
	/** From CPSLocatedBindable
	  * Release any reference this obj may have to other process of the system
	  * For example, this is used when detaching a located bindable from a system.	  
	  */
	virtual void			 releaseAllRef();
	/// return the number of targets
	uint32				getNbTargets(void) const { return _Targets.size(); }
	/// Return a ptr on a target. Invalid range -> nlassert
	CPSLocated			*getTarget(uint32 index) 
	{
		nlassert(index < _Targets.size());
		return _Targets[index];
	}
	/// Return a const ptr on a target. Invalid range -> nlassert
	const CPSLocated	*getTarget(uint32 index) const
	{
		nlassert(index < _Targets.size());
		return _Targets[index];
	}		
	/** it is called when a target is destroyed or detached
	 *  Override this if you allocated resources from the target (to release them)
	 *  NOTE : as objects are no polymorphic while being destroyed, this class
	 *  can't call your releaseTargetRsc override in its destructor, it does it in its finalize method,
	 *  which is called by the particle system
	 */
	virtual void		releaseTargetRsc(CPSLocated *target) {};
	/// Seralization, must be called by derivers
	void				serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	/// Finalize this object : the default is to call releaseTargetRsc on targets
	virtual void		finalize(void);
	virtual				~CPSTargetLocatedBindable();
protected:
	/** Inherited from CPSLocatedBindable. A target has been remove If not present -> assert
	 * This also call releaseTargetRsc for clean up
	 */		
	virtual void		notifyTargetRemoved(CPSLocated *ptr);
	typedef std::vector<CPSLocated *> TTargetCont;
	TTargetCont _Targets;	

};


/////////////
// INLINES //
/////////////

//*****************************************************************************************************
inline const NLMISC::CMatrix &CPSLocated::getConversionMatrix(const CPSLocated *A,const CPSLocated *B)
{
	nlassert(A);
	nlassert(B);
	nlassert(A->_Owner == B->_Owner); // conversion must be made between entity of the same system
	const CParticleSystem *ps = A->_Owner;
	nlassert(ps);
	return getConversionMatrix(*ps, A->getMatrixMode(), B->getMatrixMode());
}

//*****************************************************************************************************
inline const NLMISC::CMatrix &CPSLocatedBindable::getLocalToWorldMatrix() const
{
	nlassert(_Owner);
	return _Owner->getLocalToWorldMatrix();
}


} // NL3D


#endif // NL_PARTICLE_SYSTEM_LOCATED_H

/* End of particle_system_located.h */
