/** \file ps_emitter.h
 * <File description>
 *
 * $Id: ps_emitter.h,v 1.23 2003/04/10 09:22:34 vizerie Exp $
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
 * Base class for all emitters in a particle system.
 * Derivers should at least define the emit method which is called each time an emission is needed.
 * Not sharable accross systems.
 * 
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSEmitter : public CPSLocatedBindable
{
public:

	/// \name Object
	//@{
		/// Constructor
		CPSEmitter();

		// dtor
		virtual ~CPSEmitter();
	//@}

	/// Return this bindable type
	uint32							getType(void) const { return PSEmitter; }


	/// Return priority for emitters
	virtual uint32					getPriority(void) const { return 500; }

	/// Return true if this located bindable derived class holds alive emitters
	virtual bool					hasEmitters(void) { nlassert(_Owner); return _Owner->getSize() != 0; }
		
	/**
	* Process the emissions.
	* The standard behaviuour will call "emit" each time is needed.
	* So you don't need to redefine this most of the time
	*
	*/
	virtual void					step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEllapsedTime);
	

	/// Display the emitter in edition mode
	virtual void					showTool(void);

	/// Set the type of located to be emitted. The default is NULL which mean that no emission will occur
	void							setEmittedType(CPSLocated *et);

	/** Inherited from CPSLocatedBindable
	 *  We register to the emitted type, so, this, this will be called when it is destroyed
	 */
	virtual void					notifyTargetRemoved(CPSLocated *ptr);

	/// Get emitted type.
	CPSLocated						*getEmittedType(void) { return _EmittedType; }
	/// Get const ptr on emitted type
	const CPSLocated				*getEmittedType(void) const { return _EmittedType; }


	/** The type of emission.
	 *  regular     : means use Period, and generation number (the number of particle to generate when an emission occurs)
	 *  onDeath     : emit when the emitter is destroyed
	 *  once        : emit when the emitter is created
	 *  onBounce    : emit when the emitter bounce
	 *  externEmit  : emitted explicitly by the system user. A 4 letter id must be used to identify this kind of emitters
	 *                the default ID is NONE
	 */
	enum TEmissionType { regular = 0, onDeath = 1,  once = 2, onBounce = 3, externEmit = 4 };

	/** Set the frequency type. Please note that if the type is externEmit, this located need to have been attached to the system (the system is holding the ID-Located map)
	  */
	void							setEmissionType(TEmissionType freqType);

	/// Get the frequency type
	TEmissionType					getEmissionType(void) const { return _EmissionType; }

	/** Set a constant period for emission (expressed in second)
	 *  any previous period scheme is discarded
	 */
	void							setPeriod(float period);

	/// Retrieve the period for emission, valid only if a period scheme is used
	float							getPeriod(void) const { return _Period; }

	/// Indicate whether a period scheme is used or not
	bool							usePeriodScheme(void) { return _PeriodScheme != NULL; }

	/// Set a period scheme
	void							setPeriodScheme(CPSAttribMaker<float> *scheme);

	// Retrieve the period scheme, or null, if there'isnt
	CPSAttribMaker<float>			*getPeriodScheme(void) { return _PeriodScheme; }

	// Retrieve the period scheme, or null, if there'isnt (const version)
	const CPSAttribMaker<float>		*getPeriodScheme(void) const  { return _PeriodScheme; }

	/// Set a delay in seconds before the first emission (regular emitter only)
	void							setEmitDelay(float delay) { _EmitDelay = delay; }

	/// Get the delay in seconds before the first emission (regular emitter only)
	float							getEmitDelay() const { return _EmitDelay; }

	/// Set a max. number of particle emission (0 means no limit and is the default). Applies with regular emitter only.
	void							setMaxEmissionCount(uint8 count);

	/// Get the max. number of particle emission (0 means no limit and is the default). Applies with regular emitter only.
	uint8							getMaxEmissionCount() const { return _MaxEmissionCount; }

	/** Set a constant number of particle to be generated at once
	 *  any previous scheme is discarded
	 */
	void							setGenNb(uint32 GenNb);

	/// Retrieve the GenNb for emission, valid only if a GenNb scheme is used
	uint							getGenNb(void) const { return _GenNb; }

	/// Indicate whether a GenNb scheme is used or not
	bool							useGenNbScheme(void) { return _GenNbScheme != NULL; }

	/// Set a GenNb scheme
	void							setGenNbScheme(CPSAttribMaker<uint32> *scheme);

	/// Retrieve the GenNb scheme, or null, if there'isnt
	CPSAttribMaker<uint32>			*getGenNbScheme(void) { return _GenNbScheme; }

	/// Retrieve the GenNb scheme, or null, if there'isnt (const version)
	const CPSAttribMaker<uint32>	*getGenNbScheme(void) const  { return _GenNbScheme; }

	/// Serialization
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	///\name Speed vector options
	//@{
		/** Set a factor, to add the emitter speed to the emittee creation speed. this can be < 0
		 *  The default is 0
		 */	 
		void							setSpeedInheritanceFactor(float fact)
		{
			_SpeedInheritanceFactor = fact;
		}

		/// Get the speed Inheritance factor 
		float							getSpeedInheritanceFactor(void) const
		{
			return _SpeedInheritanceFactor;
		}

		/// This use the speed of the emitter to create a basis (like with cameras), and express the emission speed in it
		void							enableSpeedBasisEmission(bool enabled = true)
		{
			_SpeedBasisEmission  = enabled;
		}

		/** Check if the speed basis emission is enabled
		 *  \see enableSpeedBasisEmission()
		 */
		bool							isSpeedBasisEmissionEnabled(void) const { return _SpeedBasisEmission; }


		/** Set the basis for the direction of emission. When set to true, 
		  *	the direction is in the emitter basis (this is the default), 
		  * otherwise it is used directly as the world basis speed vector if the emitter it is the system basis, 
		  * and vice-versa. 
		  * NB : if 'speedBasisEmission' is enabled , or if the speed inheritance factor is not null, it has no effect.
		  **/
		void							setSpeedVectorInEmitterBasis(bool enable) { _EmitDirBasis = enable;}

		/// Test wether the emission direction is in the system basis
		bool							isSpeedVectorInEmitterBasis(void) const   { return _EmitDirBasis;}
	//@}
	
	/// Process a single emission. For external use (in the user interface layer)
	void							singleEmit(uint32 index, uint quantity);

	/** Enable consistent emission. The default is false. This try to keep the number of emitted particle constant, by allowing
	  * more than one emission cycle per iteration. This is useful to deal with poor frmerate. This has several drawbacks though :
	  * - collisions are not properly supported in this case (may be resolved later).
	  * - The motion is in straight lines.
	  * - It assumes that emitter has no motion (for now).
	  * In fact, this should be used when there can't be collisions with the emitted particles, and with main emitters only.	  
	  * NB : this has no effect if the emission period is 0 (which mean emit at each frame)
	  */
	void							enableConsistenEmission(bool enable) { _ConsistentEmission = enable; }

	bool						    isConsistentEmissionEnabled() const { return _ConsistentEmission; }

	/** Release any reference this obj may have on the given process.
	  * For example, this is used when detaching a located bindable from a system.  	 
	  */
	virtual	void			 releaseRefTo(const CParticleSystemProcess *other);
	 
	/** Release any reference this obj may have to other process of the system
	  * For example, this is used when detaching a located bindable from a system.	  
	  */
	virtual void			 releaseAllRef();

	// bypass the auto-LOD : no auto-LOD will be applied to that emitter
	void					 setBypassAutoLOD(bool bypass) { _BypassAutoLOD = bypass; }
	bool					 getBypassAutoLOD() const {	return _BypassAutoLOD; }

	/** For edition only : avoid that a call to CPSLocated::deleteElement() causes emitters flagged with 'emitOnDeath' to emit
	  */
	static void					 setBypassEmitOnDeath(bool bypass) { _BypassEmitOnDeath = bypass; }	

protected:	

	/// This will call emit, and will add additionnal features (speed addition and so on)
	void						processEmit(uint32 index, sint nbToGenerate);

	/// The same as processEmit, but can also add a time delta
	void						processEmitConsistent(const NLMISC::CVector &emitterPos,
													  uint32 emitterIndex,
													  sint nbToGenerate,
													  TAnimationTime deltaT,
													  TAnimationTime ellapsedTime,
													  float realEllapsedTimeRatio
													 );

	/// Regular emission processing
	void							processRegularEmission(TAnimationTime ellapsedTime, float emitLOD);
	void							processRegularEmissionWithNoLOD(TAnimationTime ellapsedTime);

	/** Regular emission processing, with low-framrate compensation
	  */
	void							processRegularEmissionConsistent(TAnimationTime ellapsedTime, float realEllapsedTimeRatio, float emitLOD, float inverseEmitLOD);
	void							processRegularEmissionConsistentWithNoLOD(TAnimationTime ellapsedTime, float realEllapsedTimeRatio);





	/** This method is called each time one (and only one) located must be emitted.
	 *  DERIVERS MUST DEFINE THIS
	 *  \param srcPos the source position of the emitter (with eventually a correction)
	 *  \param index the index of the emitter in the tab that generated a located	 
	 *  \param pos the resulting pos of the particle, expressed in the emitter basis
	 *  \param speed the reulting speed of the emitter, expressed in the emitter basis
	 */
	virtual void					emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed) = 0;

	/**	Generate a new element for this bindable. They are generated according to the propertie of the class		 
	 */
	virtual void					newElement(CPSLocated *emitterLocated, uint32 emitterIndex);
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */
	virtual void					deleteElement(uint32 index);

	/** Resize the bindable attributes containers. DERIVERS SHOULD CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void					resize(uint32 size);
	virtual void					bounceOccured(uint32 index);	
	void							updateMaxCountVect();



	/// A pointer on the type to be emitted.
	CPSLocated						*_EmittedType;

	/** The phase (  0 < phase  < period of emission). This is the time ellapsed since the last emission    
	 */
	TPSAttribFloat					_Phase;
	TPSAttribUInt8					_NumEmission; // used only if MaxEmissionCount is != 0

	float							_SpeedInheritanceFactor;	
	TEmissionType					_EmissionType;
	float _Period;
	CPSAttribMaker<float>			*_PeriodScheme;			
	uint32 _GenNb;
	CPSAttribMaker<uint32>			*_GenNbScheme;
	float							_EmitDelay;
	uint8							_MaxEmissionCount;
	bool							_SpeedBasisEmission;
	bool							_EmitDirBasis; // true when emission direction is in the emitter basis
	bool							_ConsistentEmission; 
	bool							_BypassAutoLOD;
	static bool						_BypassEmitOnDeath; // for edition only
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** This class helps tuning the emission strenght.
 *  It modulate the speed of emitted particle by a coeeficient produced by an attribute maker
 */

class CPSModulatedEmitter
{
	public:

		/// ctor : the default doesn't alter speed
		CPSModulatedEmitter() : _EmitteeSpeed(1.f), _EmitteeSpeedScheme(NULL)
		{
		}

		/// dtor
		virtual ~CPSModulatedEmitter()
		{
			delete _EmitteeSpeedScheme;
		}

		/** Set a new scheme for speed modulation.
		 *  It must have been allocated with new, and will be destriyed by this object
		 */
		void setEmitteeSpeedScheme(CPSAttribMaker<float> *scheme)
		{
			delete _EmitteeSpeedScheme;
			_EmitteeSpeedScheme = scheme;
			if (getModulatedEmitterOwner() && scheme->hasMemory()) 
				scheme->resize(getModulatedEmitterOwner()->getMaxSize(), getModulatedEmitterOwner()->getSize());
		}

		/// set a constant speed modulation for emittee
		void setEmitteeSpeed(float speed)
		{
			delete _EmitteeSpeedScheme;
			_EmitteeSpeedScheme = NULL;
			_EmitteeSpeed = speed;
		
		}

		/// get the modulation speed (valid only if no scheme is used)
		float getEmitteeSpeed(void) const { return _EmitteeSpeed; }

		/// get the speed modulation shceme, or NULL if no one is set
		CPSAttribMaker<float> *getEmitteeSpeedScheme(void) { return _EmitteeSpeedScheme; }

		/// get the speed modulation shceme, or NULL if no one is set (const version)
		const CPSAttribMaker<float> *getEmitteeSpeedScheme(void) const { return _EmitteeSpeedScheme; }

		/// check wether a speed modulation scheme is being used
		bool useEmitteeSpeedScheme(void) const { return _EmitteeSpeedScheme != NULL; }

		/// serialization 
		void serialEmitteeSpeedScheme(NLMISC::IStream &f) throw(NLMISC::EStream);
		
	protected:

		// emitter must define this in order to allow this class to access the located owner
		virtual CPSLocated *getModulatedEmitterOwner(void) = 0;

		void newEmitteeSpeedElement(CPSLocated *emitter, uint32 emitterIndex)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->newElement(emitter, emitterIndex);
		}

		void deleteEmitteeSpeedElement(uint32 index)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->deleteElement(index);
		}

		void resizeEmitteeSpeed(uint32 capacity)
		{
			if (_EmitteeSpeedScheme && _EmitteeSpeedScheme->hasMemory()) _EmitteeSpeedScheme->resize(capacity, getModulatedEmitterOwner()->getSize());
		}
	

		float _EmitteeSpeed;
		CPSAttribMaker<float> *_EmitteeSpeedScheme;
}; 



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Emit in one direction. This can be the 0, 0, 0 vector
class CPSEmitterDirectionnal : public CPSEmitter, public CPSModulatedEmitter
							   ,public CPSDirection
{
	
public:


	CPSEmitterDirectionnal() : _Dir(NLMISC::CVector::K)
	{ _Name = std::string("DirectionnalEmitter"); }

	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	
	NLMISC_DECLARE_CLASS(CPSEmitterDirectionnal);
	
	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);

	void setDir(const NLMISC::CVector &v) { _Dir = v; }

	NLMISC::CVector getDir(void) const { return _Dir; }


protected:

	NLMISC::CVector _Dir;

	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner; }
	virtual void newElement(CPSLocated *emitter, uint32 emitterIndex);
	virtual void deleteElement(uint32 index);
	virtual void resize(uint32 capacity);
}; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// A radial emitter. The direction gives the normal to the plane of emission
class CPSRadialEmitter : public CPSEmitterDirectionnal
{
	public:
	CPSRadialEmitter()
	{ _Name = std::string("RadialEmitter"); }
	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);	
	NLMISC_DECLARE_CLASS(CPSRadialEmitter);
	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Emit randomly in all direction
class CPSEmitterOmni : public CPSEmitter, public CPSModulatedEmitter
{
	
public:

	CPSEmitterOmni()
	{
		_Name = std::string("EmitterOmni");
	}

	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
	NLMISC_DECLARE_CLASS(CPSEmitterOmni);


	/// Emission of located
	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);
protected:
	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner; }
	virtual void newElement(CPSLocated *emitter, uint32 emitterIndex);
	virtual void deleteElement(uint32 index);
	virtual void resize(uint32 capacity);


}; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Emit directionnally in a rectangle (useful to produce snow, drop of water ...)
class CPSEmitterRectangle : public CPSEmitter, public CPSModulatedEmitter, public IPSMover
							, public CPSDirection
{
	public:

		// Ctor

		CPSEmitterRectangle() : _Dir(-NLMISC::CVector::K)
		{
			_Name = std::string("EmitterRectangle");
		}

		/// Serialisation
 		virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
		NLMISC_DECLARE_CLASS(CPSEmitterRectangle);


		/// Emission of located

		virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);

		virtual void setDir(const NLMISC::CVector &v) { _Dir = v; }

		NLMISC::CVector getDir(void) const { return _Dir; }

	
		void showTool(void);

	

		// Inherited from IPSMover
		virtual bool supportUniformScaling(void) const { return true; }
		virtual bool supportNonUniformScaling(void) const { return true; }			
		virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);	
		virtual NLMISC::CMatrix getMatrix(uint32 index) const;			
		virtual void setScale(uint32 index, float scale);	
		virtual void setScale(uint32 index, const NLMISC::CVector &s);
		NLMISC::CVector getScale(uint32 index) const;

	protected:

		virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner; }
		
		CPSAttrib<CPlaneBasis> _Basis;				

		//  Width
		TPSAttribFloat _Width;

		//  Height
		TPSAttribFloat _Height;

		// Direction of emission (in each plane basis)
		NLMISC::CVector _Dir;

		/**	Generate a new element for this bindable. They are generated according to the propertie of the class		 
		 */
		virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex);
	
		/** Delete an element given its index
		 *  Attributes of the located that hold this bindable are still accessible for of the index given
		 *  index out of range -> nl_assert
		 */
		virtual void deleteElement(uint32 index);

		/** Resize the bindable attributes containers. DERIVERS SHOULD CALL THEIR PARENT VERSION
		 * should not be called directly. Call CPSLocated::resize instead
		 */
		virtual void resize(uint32 size);
}; 


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// The same as a directionnel emitter, but you can also specify the radius for emission
class CPSEmitterConic : public CPSEmitterDirectionnal					
{	
public:

	CPSEmitterConic() : _Radius(1.f)
	{
		_Name = std::string("EmitterConic");
	}

	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
	NLMISC_DECLARE_CLASS(CPSEmitterConic);


	/// Emission of located
	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);

	/// Set a new radius for emission
	void setRadius(float r) { _Radius = r; }

	/// Get the emission radius
	float getRadius(void) const { return _Radius; }

	/// Set the direction for emission
	virtual void setDir(const NLMISC::CVector &v);

protected:	

	// The radius for emission
	float _Radius;
 
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A spherical emitter
class CPSSphericalEmitter : public CPSEmitter, public CPSModulatedEmitter, public IPSMover
{
public:
	// Ctor

	CPSSphericalEmitter()
	{
		_Name = std::string("spherical emitter");
	}

	/// Serialisation
 	virtual	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	NLMISC_DECLARE_CLASS(CPSSphericalEmitter);


	/// Emission of located

	virtual void emit(const NLMISC::CVector &srcPos, uint32 index, NLMISC::CVector &pos, NLMISC::CVector &speed);

	

	void showTool(void);



	// Inherited from IPSMover
	virtual bool supportUniformScaling(void) const { return true; }
	virtual bool supportNonUniformScaling(void) const { return false; }			
	virtual void setMatrix(uint32 index, const NLMISC::CMatrix &m);	
	virtual NLMISC::CMatrix getMatrix(uint32 index) const;			
	virtual void setScale(uint32 index, float scale) { _Radius[index] = scale; }		
	NLMISC::CVector getScale(uint32 index) const { return NLMISC::CVector(_Radius[index], _Radius[index], _Radius[index]); }

protected:
	virtual CPSLocated *getModulatedEmitterOwner(void) { return _Owner; }	
	TPSAttribFloat _Radius;	
	virtual void newElement(CPSLocated *emitterLocated, uint32 emitterIndex);	
	virtual void deleteElement(uint32 index);
	virtual void resize(uint32 size);
};


} // NL3D


#endif // NL_PS_EMITTER_H

/* End of ps_emitter.h */
