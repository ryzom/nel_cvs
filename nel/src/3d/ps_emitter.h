/** \file ps_emitter.h
 * <File description>
 *
 * $Id: ps_emitter.h,v 1.9 2001/07/17 15:54:08 vizerie Exp $
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

#ifndef NL_PS_EMITTER_H
#define NL_PS_EMITTER_H

#include "nel/misc/types_nl.h"
#include "3d/ps_located.h"
#include "3d/ps_attrib_maker.h"
#include "3d/ps_edit.h"
#include "3d/ps_plane_basis.h"
#include "3d/ps_direction.h"


namespace NL3D {


/**
 * this struct is used for describing the frequency of emission 
 */
 
/*struct CPSFrequency
{
	// the frequency for emission.
	enum TFreqType { regular = 0, onDeath = 1,  once = 2, onBounce = 3 } _FreqType ;
	// the period of emission (fill that when needed)
	float _Period ;	
	// the number of located to generate each time an emission occurs
	uint32 _GenNb ;

	/// ctor an emitter that emit one particle each frame
	CPSFrequency() : _FreqType(regular), _Period(0.0f), _GenNb(1)
	{
	}
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
} ;*/






/**
 * base class for all emitters in a particle system
 * Derivers should at least define the emit method which is called each time an emission is needed
 * Not sharable.
 * 
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSEmitter : public CPSLocatedBindable
{
public:

	/// Constructor
	CPSEmitter();


	// dtor
	virtual ~CPSEmitter() ;

	/// return this bindable type
	uint32 getType(void) const { return PSEmitter ; }


	/// return priority for forces
	virtual uint32 getPriority(void) const { return 500 ; }

	/// return true if this located bindable derived class holds alive emitters
	virtual bool hasEmitters(void) { nlassert(_Owner) ; return _Owner->getSize() != 0 ; }
	
	/**
	* Process the emissions.
	* The standard behaviuour will call "emit" each time is needed.
	* So you don't need to redefine this most of the time
	*
	*/
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;
	

	/// display the emitter in edition mode
	virtual void showTool(void)  ;

	/// set thetype of located to be emitted. THIS MUST BE CALLED
	void setEmittedType(CPSLocated *et) ;

	/** Inherited from CPSLocatedBindable
	 * we register to the emitted type, so, this, this will be called when it is detroyed
	 */
	virtual void notifyTargetRemoved(CPSLocated *ptr)  ;

	/// get emitted type
	CPSLocated *getEmittedType(void) { return _EmittedType ; }
	/// get const ptr on emitted type
	const CPSLocated *getEmittedType(void) const { return _EmittedType ; }




	/** the type of emission emission.
	 *  regular means use Period, and generation number
	 *  onDeath : when the particle is destroyed
	 *  once : when the particle is created
	 *  onBounce : when the particle bounce
	 */
	enum TEmissionType { regular = 0, onDeath = 1,  once = 2, onBounce = 3 } ;

	/// set the frequency type
	void setEmissionType(TEmissionType freqType) { _EmissionType = freqType ; }

	/// get the frequency type
	TEmissionType getEmissionType(void) const { return _EmissionType ; }


	/** set a constant period for emission (expressed in second)
	 *  any previous period scheme is discarded
	 */
	void setPeriod(float period) ;

	/// retrieve the period for emission, valid only if a period scheme is used
	float getPeriod(void) const { return _Period ; }

	/// indicate whether a period scheme is used or not
	bool usePeriodScheme(void) { return _PeriodScheme != NULL ; }

	/// set a period scheme
	void setPeriodScheme(CPSAttribMaker<float> *scheme) ;

	// Retrieve the period scheme, or null, if there'isnt
	CPSAttribMaker<float> *getPeriodScheme(void) { return _PeriodScheme ; }

	// Retrieve the period scheme, or null, if there'isnt (const version)
	const CPSAttribMaker<float> *getPeriodScheme(void) const  { return _PeriodScheme ; }


	/** set a constant number of particle to be generated at once
	 *  any previous scheme is discarded
	 */
	void setGenNb(uint32 GenNb) ;

	/// retrieve the GenNb for emission, valid only if a GenNb scheme is used
	uint getGenNb(void) const { return _GenNb ; }

	/// indicate whether a GenNb scheme is used or not
	bool useGenNbScheme(void) { return _GenNbScheme != NULL ; }

	/// set a GenNb scheme
	void setGenNbScheme(CPSAttribMaker<uint32> *scheme) ;

	/// Retrieve the GenNb scheme, or null, if there'isnt
	CPSAttribMaker<uint32> *getGenNbScheme(void) { return _GenNbScheme ; }

	/// Retrieve the GenNb scheme, or null, if there'isnt (const version)
	const CPSAttribMaker<uint32> *getGenNbScheme(void) const  { return _GenNbScheme ; }

	/// serialization
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;


	/** set a factor, to add the emitter speed to the emittee creation speed. this can be < 0
	 *  The default is 0
	 */	 
	void setSpeedInheritanceFactor(float fact)
	{
		_SpeedInheritanceFactor = fact ;
	}

	/// get the speed Inheritance factor 
	float getSpeedInheritanceFactor(void) const
	{
		return _SpeedInheritanceFactor ;
	}

	/// this use the speed of the emitter create a basis (like with cameras), and express the emission speed in it
	void enableSpeedBasisEmission(bool enabled = true)
	{
		_SpeedBasisEmission  = enabled ;
	}

	/** check if the speed basis emission is enabled
	 *  \see enableSpeedBasisEmission()
	 */
	bool isSpeedBasisEmissionEnabled(void) const { return _SpeedBasisEmission ; }
	

protected:

	/// this will call emit, and will add additionnal features (speed addition and so on)
	inline void processEmit(uint32 index, sint nbToGenerate) ;


	/** This method is called each time one (and only one) located must be emitted.
	 *  DERIVERS MUST DEFINE THIS
	 *  \param index the index of the emitter in the tab that generated a located	 
	 *  \param pos the resulting pos of the particle, expressed in the emitter basis
	 *  \param speed the reulting speed of the emitter, expressed in the emitter basis
	 */
	virtual void emit(uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed) = 0 ;

	/// a pointer on the type to be emitted
	CPSLocated *_EmittedType ;

	/** the phase (  0 < phase  < period of emission)
     *  its usage depends on the freq member
	 */
	TPSAttribFloat _Phase ; 


	float _SpeedInheritanceFactor ;

	bool _SpeedBasisEmission ;

	/**	Generate a new element for this bindable. They are generated according to the propertie of the class		 
	 */
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) ;

	/** Resize the bindable attributes containers. DERIVERS SHOULD CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size) ;


	virtual void bounceOccured(uint32 index) ;	

	TEmissionType _EmissionType ;
	float _Period ;
	CPSAttribMaker<float> *_PeriodScheme ;			
	uint32 _GenNb ;
	CPSAttribMaker<uint32> *_GenNbScheme ;	



};


//////////////////////////////////////
// implementation of inline methods //
//////////////////////////////////////


inline void CPSEmitter::processEmit(uint32 index, sint nbToGenerate)
{
	if (!_EmittedType) return ;

	static NLMISC::CVector speed, pos ;
	
	if (!_SpeedBasisEmission)
	{
		if (_SpeedInheritanceFactor == 0.f)
		{		
			while (nbToGenerate > 0)
			{
				nbToGenerate -- ;
				emit(index, pos, speed) ;
				_EmittedType->newElement(pos, speed, this->_Owner) ;
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(index, pos, speed) ;
				_EmittedType->newElement(pos, speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this->_Owner) ;
			}
		}
	}
	else
	{
		NLMISC::CMatrix m = CPSUtil::buildSchmidtBasis(_Owner->getSpeed()[index]) ;
		if (_SpeedInheritanceFactor == 0.f)
		{		
			while (nbToGenerate > 0)
			{
				nbToGenerate -- ;
				emit(index, pos, speed) ;
				_EmittedType->newElement(pos, m * speed, this->_Owner) ;
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(index, pos, speed) ;
				_EmittedType->newElement(pos, m * speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this->_Owner) ;
			}
		}
	}
}


/** this class helps tuning the emission strenght.
 *  It modulate the speed of emitted particle by a coeeficient produced by an attribute maker
 */

class CPSModulatedEmitter
{
	public:

		/// ctor : the default doesn't alter speed
		CPSModulatedEmitter() : _EmitteeSpeedScheme(NULL), _EmitteeSpeed(1.f)
		{
		}

		/// dtor
		~CPSModulatedEmitter()
		{
			delete _EmitteeSpeedScheme ;
		}

		/** Set a new scheme for speed modulation.
		 *  It must have been allocated with new, and will be destriyed by this object
		 */
		void setEmitteeSpeedScheme(CPSAttribMaker<float> *scheme)
		{
			delete _EmitteeSpeedScheme ;
			_EmitteeSpeedScheme = scheme ;
			if (getModulatedEmitterOwner() && scheme->hasMemory()) 
				scheme->resize(getModulatedEmitterOwner()->getMaxSize(), getModulatedEmitterOwner()->getSize()) ;
		}

		/// set a constant speed modulation for emittee
		void setEmitteeSpeed(float speed)
		{
			delete _EmitteeSpeedScheme ;
			_EmitteeSpeedScheme = NULL ;
			_EmitteeSpeed = speed ;
		
		}

		/// get the modulation speed (valid only if no scheme is used)
		float getEmitteeSpeed(void) const { return _EmitteeSpeed ; }

		/// get the speed modulation shceme, or NULL if no one is set
		CPSAttribMaker<float> *getEmitteeSpeedScheme(void) { return _EmitteeSpeedScheme ; }

		/// get the speed modulation shceme, or NULL if no one is set (const version)
		const CPSAttribMaker<float> *getEmitteeSpeedScheme(void) const { return _EmitteeSpeedScheme ; }

		/// check wether a speed modulation scheme is being used
		bool useEmitteeSpeedScheme(void) const { return _EmitteeSpeedScheme != NULL ; }

		/// serialization 
		void serialEmitteeSpeedScheme(NLMISC::IStream &f) throw(NLMISC::EStream)
		{

			bool useScheme ;			
			if (!f.isReading())
			{
				useScheme = useEmitteeSpeedScheme() ;
			}
			f.serial(useScheme) ;
			if (useScheme)
			{
				f.serialPolyPtr(_EmitteeSpeedScheme) ;
			}
			else
			{
				f.serial(_EmitteeSpeed) ;
			}
		}

	protected:

		// emitter must define this in order to allow this class to access the located owner
		virtual CPSLocated *getModulatedEmitterOwner(void) = 0 ;

		void newEmitteeSpeedElement(CPSLocated *emitter, uint32 emitterIndex)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->newElement(emitter, emitterIndex) ;
		}

		void deleteEmitteeSpeedElement(uint32 index)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->deleteElement(index) ;
		}

		void resizeEmitteeSpeed(uint32 capacity)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->resize(capacity, getModulatedEmitterOwner()->getSize()) ;
		}
	

		float _EmitteeSpeed ;
		CPSAttribMaker<float> *_EmitteeSpeedScheme ;
} ; 






/// emit in one direction. This can be the 0, 0, 0 vector
class CPSEmitterDirectionnal : public CPSEmitter, public CPSModulatedEmitter
							   ,public CPSDirection
{
	
public:


	CPSEmitterDirectionnal() : _Dir(NLMISC::CVector::K)
	{ _Name = std::string("DirectionnalEmitter") ; }

	/// serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	
	NLMISC_DECLARE_CLASS(CPSEmitterDirectionnal) ;
	
	virtual void emit(uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed) ;

	void setDir(const NLMISC::CVector &v) { _Dir = v ; }

	NLMISC::CVector getDir(void) const { return _Dir ; }


protected:

	NLMISC::CVector _Dir ;

	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner ; }
	virtual void newElement(CPSLocated *emitter, uint32 emitterIndex) ;
	virtual void deleteElement(uint32 index) ;
	virtual void resize(uint32 capacity) ;
} ; 


/// emit randomly in all direction
class CPSEmitterOmni : public CPSEmitter, public CPSModulatedEmitter
{
	
public:

	CPSEmitterOmni()
	{
		_Name = std::string("EmitterOmni") ;
	}

	/// serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
	NLMISC_DECLARE_CLASS(CPSEmitterOmni) ;


	/// emission of located
	virtual void emit(uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed) ;
protected:
	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner ; }
	virtual void newElement(CPSLocated *emitter, uint32 emitterIndex) ;
	virtual void deleteElement(uint32 index) ;
	virtual void resize(uint32 capacity) ;


} ; 


/// emit directionnally in a rectangle (useful to produce snow, drop of water ...)

class CPSEmitterRectangle : public CPSEmitter, public CPSModulatedEmitter, public IPSMover
							, public CPSDirection
{
	public:

		// ctor

		CPSEmitterRectangle() : _Dir(-NLMISC::CVector::K)
		{
			_Name = std::string("EmitterRectangle") ;
		}

		/// serialisation
 		virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
		NLMISC_DECLARE_CLASS(CPSEmitterRectangle) ;


		/// emission of located

		virtual void emit(uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed) ;

		virtual void setDir(const NLMISC::CVector &v) { _Dir = v ; }

		NLMISC::CVector getDir(void) const { return _Dir ; }

	
		void showTool(void) ;

	

		// inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true ; }
		virtual bool supportNonUniformScaling(void) const { return true ; }			
		virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m) ;	
		virtual NLMISC::CMatrix getMatrix(uint32 index) const ;			
		virtual void setScale(uint32 index, float scale) ;	
		virtual void setScale(uint32 index, const NLMISC::CVector &s) ;
		NLMISC::CVector getScale(uint32 index) const ;

	



	protected:

		virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner ; }
		
		CPSAttrib<CPlaneBasis> _Basis ;				

		//  width
		TPSAttribFloat _Width ;

		//  Height
		TPSAttribFloat _Height ;

		// direction of emission (in each plane basis)
		NLMISC::CVector _Dir ;

		/**	Generate a new element for this bindable. They are generated according to the propertie of the class		 
		 */
		virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) ;
	
		/** Delete an element given its index
		 *  Attributes of the located that hold this bindable are still accessible for of the index given
		 *  index out of range -> nl_assert
		 */
		virtual void deleteElement(uint32 index) ;

		/** Resize the bindable attributes containers. DERIVERS SHOULD CALL THEIR PARENT VERSION
		 * should not be called directly. Call CPSLocated::resize instead
		 */
		virtual void resize(uint32 size) ;
} ; 


/// se same as a directionnel emitter, but you can also specify the radius for emission

class CPSEmitterConic : public CPSEmitterDirectionnal					
{	
public:

	CPSEmitterConic() : _Radius(1.f)
	{
		_Name = std::string("EmitterConic") ;
	}

	/// serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
	NLMISC_DECLARE_CLASS(CPSEmitterConic) ;


	/// emission of located
	virtual void emit(uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed) ;

	/// set a new radius for emission
	void setRadius(float r) { _Radius = r ; }

	/// get the emission radius
	float getRadius(void) const { return _Radius ; }

	/// set the direction for emission
	virtual void setDir(const NLMISC::CVector &v) ;

protected:
	

	// the radius for emission
	float _Radius ;
 
} ;


// a spherical emitter
class CPSSphericalEmitter : public CPSEmitter, public CPSModulatedEmitter, public IPSMover
{
public:
	// ctor

	CPSSphericalEmitter()
	{
		_Name = std::string("spherical emitter") ;
	}

	/// serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	NLMISC_DECLARE_CLASS(CPSSphericalEmitter) ;


	/// emission of located

	virtual void emit(uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed) ;

	

	void showTool(void) ;



	// inherited from IPSMover
	virtual bool supportUniformScaling(void) const { return true ; }
	virtual bool supportNonUniformScaling(void) const { return false ; }			
	virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m) ;	
	virtual NLMISC::CMatrix getMatrix(uint32 index) const ;			
	virtual void setScale(uint32 index, float scale) { _Radius[index] = scale ; }		
	NLMISC::CVector getScale(uint32 index) const { return NLMISC::CVector(_Radius[index], _Radius[index], _Radius[index]) ; }





protected:

	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner ; }
	
	TPSAttribFloat _Radius ;
	
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex) ;	
	virtual void deleteElement(uint32 index) ;
	virtual void resize(uint32 size) ;
} ;


} // NL3D


#endif // NL_PS_EMITTER_H

/* End of ps_emitter.h */
