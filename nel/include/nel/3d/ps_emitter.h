/** \file ps_emitter.h
 * <File description>
 *
 * $Id: ps_emitter.h,v 1.9 2001/05/31 12:16:11 vizerie Exp $
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
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/ps_edit.h"
#include "nel/3d/ps_plane_basis.h"

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


	/// return this bindable type
	uint32 getType(void) const { return PSEmitter ; }


	/// return priority for forces
	virtual uint32 getPriority(void) const { return 500 ; }
	
	/**
	* Process the emissions.
	* The standard behaviuour will call "emit" each time is needed.
	* So you don't need to redefine this most of the time
	*
	*/
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;
	


	/// set thetype of located to be emitted. THIS MUST BE CALLED
	void setEmittedType(CPSLocated *et) { _EmittedType = et ; }

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
	void setGenNbScheme(CPSAttribMaker<sint32> *scheme) ;

	/// Retrieve the GenNb scheme, or null, if there'isnt
	CPSAttribMaker<sint32> *getGenNbScheme(void) { return _GenNbScheme ; }

	/// Retrieve the GenNb scheme, or null, if there'isnt (const version)
	const CPSAttribMaker<sint32> *getGenNbScheme(void) const  { return _GenNbScheme ; }

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
	inline void processEmit(uint32 index, uint nbToGenerate) ;


	/** This method is called each time one (and only one) located must be emitted.
	 *  DERIVERS MUST DEFINE THIS
	 *  \param index the index of the emitter in the tab that generated a located	 
	 *  \param pos the resulting pos of the particle, expressed in the emitter basis
	 *  \param speed the reulting speed of the emitter, expressed in the emitter basis
	 */
	virtual void emit(uint32 index, CVector &pos, CVector &speed) = 0 ;

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
	virtual void newElement(void) ;
	
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
	CPSAttribMaker<sint32> *_GenNbScheme ;	
};


//////////////////////////////////////
// implementation of inline methods //
//////////////////////////////////////


inline void CPSEmitter::processEmit(uint32 index, uint nbToGenerate)
{
	static CVector speed, pos ;
	
	if (!_SpeedBasisEmission)
	{
		if (_SpeedInheritanceFactor == 0.f)
		{		
			while (nbToGenerate --)
			{
				emit(index, pos, speed) ;
				_EmittedType->newElement(pos, speed, this) ;
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(index, pos, speed) ;
				_EmittedType->newElement(pos, speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this) ;
			}
		}
	}
	else
	{
		CMatrix m = CPSUtil::buildSchmidtBasis(_Owner->getSpeed()[index]) ;
		if (_SpeedInheritanceFactor == 0.f)
		{		
			while (nbToGenerate --)
			{
				emit(index, pos, speed) ;
				_EmittedType->newElement(pos, m * speed, this) ;
			}
		}
		else
		{
			while (nbToGenerate --)
			{
				emit(index, pos, speed) ;
				_EmittedType->newElement(pos, m * speed + _SpeedInheritanceFactor * _Owner->getSpeed()[index], this) ;
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
		float _EmitteeSpeed ;
		CPSAttribMaker<float> *_EmitteeSpeedScheme ;
} ; 



/// emit in one direction. This can be the 0, 0, 0 vector
class CPSEmitterDirectionnal : public CPSEmitter, public CPSModulatedEmitter
{
	
public:


	CPSEmitterDirectionnal() : _Dir(CVector::K)
	{}

	/// serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	
	NLMISC_DECLARE_CLASS(CPSEmitterDirectionnal) ;
	
	virtual void emit(uint32 index, CVector &pos, CVector &speed) ;

	void setDir(const CVector &v) { _Dir = v ; }

	CVector getDir(void) const { return _Dir ; }


protected:

	CVector _Dir ;

} ; 


/// emit randomly in all direction
class CPSEmitterOmni : public CPSEmitter, public CPSModulatedEmitter
{
	
public:


	/// serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
	NLMISC_DECLARE_CLASS(CPSEmitterOmni) ;


	/// emission of located
	virtual void emit(uint32 index, CVector &pos, CVector &speed) ;

} ; 


/// emit directionnally in a rectangle (useful to produce snow, drop of water ...)

class CPSEmitterRectangle : public CPSEmitter, public CPSModulatedEmitter, public IPSMover
{
	public:

		// ctor

		CPSEmitterRectangle() : _Dir(-CVector::K)
		{
		}

		/// serialisation
 		virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
		NLMISC_DECLARE_CLASS(CPSEmitterRectangle) ;


		/// emission of located

		virtual void emit(uint32 index, CVector &pos, CVector &speed) ;

		virtual void setDir(const CVector &v) { _Dir = v ; }

		CVector getDir(void) const { return _Dir ; }


	
		virtual bool supportNonOrthonormalBasis(void) const { return true ; }


	
		virtual bool supportUniformScaling(void) const { return true ; }
		virtual bool supportScaling(void) const { return true ; }
	
		// left multiply the current matrix by the given one. No valid index -> assert
		virtual void applyMatrix(uint32 index, const CMatrix &m) ;
		// return a matrix of the system. No valid index -> assert
		virtual CMatrix getMatrix(uint32 index) const ;

	



	protected:

		
		CPSAttrib<CPlaneBasis> _Basis ;				

		//  width
		TPSAttribFloat _Width ;

		//  Height
		TPSAttribFloat _Height ;

		// direction of emission (in each plane basis)
		CVector _Dir ;

		/**	Generate a new element for this bindable. They are generated according to the propertie of the class		 
		 */
			virtual void newElement(void) ;
	
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


	/// serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
	NLMISC_DECLARE_CLASS(CPSEmitterConic) ;


	/// emission of located
	virtual void emit(uint32 index, CVector &pos, CVector &speed) ;

	/// set a new radius for emission
	void setRadius(float r) { _Radius = r ; }

	/// get the emission radius
	float getRadius(void) const { return _Radius ; }

	/// set the direction for emission
	virtual void setDir(const CVector &v) ;

protected:


	
	// tangent to emission plane
	CVector _EI ;

	// 2nd tangent to emission plane
	CVector _EJ ;

	// the radius for emission
	float _Radius ;
 
} ;


} // NL3D


#endif // NL_PS_EMITTER_H

/* End of ps_emitter.h */
