/** \file ps_force.h
 * <File description>
 *
 * $Id: ps_force.h,v 1.11 2001/06/07 10:17:56 vizerie Exp $
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

#ifndef NL_PS_FORCE_H
#define NL_PS_FORCE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_util.h"


namespace NL3D {



/** This struct describe where 
 * 


/**
 * All forces in the system derive from this class
 * It all a list all located on which the force can apply
 * Only the motion and toolRender passes are supported for a force
 * The corresponding virtual calls are done
 * Not sharable.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSForce : public CPSTargetLocatedBindable
{
public:

	/// Constructor
	CPSForce();
	

	/// return this bindable type
	uint32 getType(void) const { return PSForce ; }


	/// return priority for forces

	virtual uint32 getPriority(void) const { return 4000 ; }

	/// Override of CPSLocatedBindable::doesProduceBBox. forces usually are not part of the bbox
	virtual bool doesProduceBBox(void) const { return false ; }

	/**
	 * process one pass for the force 
	 */
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;

	
	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime) = 0 ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  = 0 ;

	/// Serial the force definition. MUST be called by deriver during their serialisation
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

protected:

	virtual void newElement(void) = 0 ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) = 0 ;

	/** Resize the bindable attributes containers DERIVERS SHOULD CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLOcated::resize instead
	 */
	virtual void resize(uint32 size) = 0 ;

	
};


/** a helper class to create isotropic force : they are independant of the basis, and have no position 
 s*  (fluid friction for example)
 *  To use this class you should provide to it a functor class that define the () operator with 3 parameters
 *  param1 = a const reference to the position of the particle
 *  param2 = a reference to the position, that must be updated
 *  param3 =  a float giving the inverse of the mass
 *  param4 = the ellapsed time, in second (has the CAnimationTime type).
 *  Example of use :
 *  class MyForceFunctor
 *  {
 *    public:
 *      /// it is strongly recommended to have your operator inlined
 *      void operator() (const CVector &pos, CVector &speed, float invMass , CanimationTime ellapsedTime)
 *      {
 *			// perform the speed update there
 *		}
 *		
 *      // you must provide a serialization method
 *		void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
 *
 *    protected:
 *      ...
 *  } ;
 *
 *
 * because of the serialization process, you must proceed like the following. (but you don't need to redefine serial, which
 * will serilize the functor object you passed for you
 *
 *	class MyForce : public CHomogenousForceT<MyForceFunctor>
 *  {
 *		public:
 *         MyForce() ;
 *		   NLMISC_DECLARE_CLASS(Myforce) ;
 *
 *      protected:
 *			...
 *
 *  } ;
 */

template <class T> class CIsotropicForceT : public CPSForce
{
public: 

	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime) ;


/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialVersion(1) ;
		CPSForce::serial(f) ;
		f.serial(_F) ;
	}

	
	/** Show the force (edition mode). The default does nothing
	 *  TODO later
	 */

	 void show(CAnimationTime ellapsedTime)  {}

protected:
	
	/// the functor object
	T _F ;

	
		
	virtual void newElement(void) { } ;		
	virtual void deleteElement(uint32 index) {} ;	
	virtual void resize(uint32 size) {} ;


} ;

//////////////////////////////////////////////////////////////////////
// implementation of method of thetemplate class  CHomogenousForceT //
//////////////////////////////////////////////////////////////////////


template <class T> void CIsotropicForceT<T>::performMotion(CAnimationTime ellapsedTime)
{
	for (uint32 k = 0 ; k < _Owner->getSize() ; ++k)
	{	
		for (TTargetCont::iterator it = _Targets.begin() ; it != _Targets.end() ; ++it)
		{			
			
			TPSAttribVector::iterator speedIt = (*it)->getSpeed().begin(), endSpeedIt = (*it)->getSpeed().end() ;
			TPSAttribVector::const_iterator posIt = (*it)->getPos().begin() ;
			TPSAttribFloat::const_iterator invMassIt = (*it)->getInvMass().begin() ;
			
			for (; speedIt != endSpeedIt ; ++speedIt, ++posIt, ++invMassIt)
			{
				_F(*posIt, *speedIt, *invMassIt, ellapsedTime) ;				
			}
		}
	}
}


/// a gravity class

class CPSGravity : public CPSForce
{
public:
	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime)  ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  ;


	/// set the gravity strenght
	void setG(float g) { _G = g ; }

	/// get the gravity strenght
	float getG(void) const { return _G ; } 

	CPSGravity(float g = 9.8f) : _G(g)
	{ _Name = std::string("Gravity") ; }

	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


	NLMISC_DECLARE_CLASS(CPSGravity) ; 

protected:

	float _G ;

		/// Inhrited from CPSLocatedBindable. we don't store additionnal information so it does nothing
	virtual void newElement(void) { } ;	

	/// Inhrited from CPSLocatedBindable. we don't store additionnal information so it does nothing
	virtual void deleteElement(uint32 index) {} ;	

	/** Inhrited from CPSLocatedBindable. we don't store additionnal information so it does nothing
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size) {} ;

} ;


/// a spring class

class CPSSpring : public CPSForce
{
public:


	/// set the k coefficient of the spring
	void setK(float k) { _K = k ; } 

	/// get the k coeefficient of the dpring
	float getK(void) const { return _K ; }

	/// ctor : k is the coefficient of the spring

	CPSSpring(float k = 1.0f) : _K(1.0f)
	{ _Name = std::string("Spring") ; }


	/// serialization
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime)  ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  ;


	NLMISC_DECLARE_CLASS(CPSSpring) ; 


protected:

	float _K ;

	/// Inhrited from CPSLocatedBindable. we don't store additionnal information so it does nothing
	virtual void newElement(void) { } ;	

	/// Inhrited from CPSLocatedBindable. we don't store additionnal information so it does nothing
	virtual void deleteElement(uint32 index) {} ;	

	/** Inhrited from CPSLocatedBindable. we don't store additionnal information so it does nothing
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size) {} ;
} ;



/// a fluid friction functor, it is used by the fluid friction class
class CPSFluidFrictionFunctor
{
public:
	CPSFluidFrictionFunctor() : _K(.2f) {}

	 void operator() (const CVector &pos, CVector &speed, float invMass , CAnimationTime ellapsedTime)
	 {
		speed += (ellapsedTime * _K * invMass * speed)  ;
	 }

	 virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	 {
		f.serial(_K) ;
	 }

	 // get the friction coefficient
	 float getK(void) const { return -_K ; }

	 // set the friction coefficient usually, it is < 1
	 void setK(float coeff) { _K = -coeff ; }
protected:
	// the friction coeff
	float _K ;
} ;


// the fluid friction force

class CPSFluidFriction : public CIsotropicForceT<CPSFluidFrictionFunctor>
{
public:
	// create the force with a friction coefficient
	CPSFluidFriction(float frictionCoeff = .1f)
	{
		_F.setK(frictionCoeff) ;
		_Name = std::string("FluidFriction") ;
	}

	// get the friction coefficient
	 float getK(void) const { return _F.getK() ; }

	 // set the friction coefficient usually, it is < 1
	 float setK(float coeff) { _F.setK(coeff) ; }

	NLMISC_DECLARE_CLASS(CPSFluidFriction)
} ;


/// a turubulence force functor

struct CPSTurbulForceFunc
{	
	void operator() (const CVector &pos, CVector &speed, float invMass , CAnimationTime ellapsedTime)
	{
		nlassert(0) ;

		// TODO : complete that

	/*	static const CVector v1(1.235f, - 45.32f, 157.5f) ;
		static const CVector v2(-0.35f, 7.77f, 220.77f) ;


		speed += ellapsedTime * _Intensity 
			   * CVector(2.f * (-0.5f + CPSUtil::buildPerlinNoise(_Scale * pos, _NumOctaves))
						 , 2.f * (-0.5f +  CPSUtil::buildPerlinNoise(_Scale * (pos +  v1) , _NumOctaves))
						 , 2.f * (-0.5f +  CPSUtil::buildPerlinNoise(_Scale * (pos +  v2) , _NumOctaves))
						 ) ;
						 */
	}

	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	 {
		f.serial(_Scale, _Intensity, _NumOctaves) ;
	 }

	float _Scale ;
	float _Intensity ;
	uint32 _NumOctaves ;
} ;


// the turbulence force

class CPSTurbul : public CIsotropicForceT<CPSTurbulForceFunc>
{
public:
	// create the force with a friction coefficient
	CPSTurbul(float intensity = 1.f , float scale = 1.f , uint numOctaves = 4)
	{
		nlassert(numOctaves > 0) ;
		setScale(scale) ;
		setIntensity(intensity) ;
		setNumOctaves(numOctaves) ;
		_Name = std::string("Turbulence") ;
	}

	
	float getScale(void) const { return _F._Scale ; }
	void setScale(float scale) { _F._Scale = scale ; } 

	float getIntensity(void) const { return _F._Intensity ; }
	void setIntensity(float intensity) { _F._Intensity = intensity ; } 

	uint getNumOctaves(void) const { return _F._NumOctaves ; }
	void setNumOctaves(uint numOctaves) { _F._NumOctaves = numOctaves ; } 


	NLMISC_DECLARE_CLASS(CPSTurbul)
} ;









} // NL3D


#endif // NL_PS_FORCE_H

/* End of ps_force.h */
