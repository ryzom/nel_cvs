/** \file ps_emitter.h
 * <File description>
 *
 * $Id: ps_emitter.h,v 1.3 2001/04/27 09:32:27 vizerie Exp $
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


namespace NL3D {


/**
 * this struct is used for describing the frequency of emission 

 */
struct CPSFrequency
{
	// the frequency for emission
	enum TFreqType { regular = 0, onDeath = 1,  once = 2} _FreqType ;
	// the period of emission (fill that when needed)
	float _Period ;	
	// the number of located to generate each time an emission occurs
	uint32 _GenNb ;
	/// ctor an emitter that emit as much as possible 5each frame)
	CPSFrequency() : _FreqType(regular), _Period(0.0f), _GenNb(1)
	{
	}
	void serial(NLMISC::IStream &f) ;
} ;



/**
 * base class for all emitters in a particle system
 * Derivers should at least define the emit method which is called each time an emission is needed (and the getClassName method)
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

	/**	Generate a new element for this bindable. They are generated according to the propertie of the class	
	 * \return true if it could be added
	 */
	virtual bool newElement(void) ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) ;

	/// Resize the bindable attributes containers. DERIVERS SHOULD CALL THEIR PARENT VERSION
	virtual void resize(uint32 size) ;


	/// set thetype of located to be emitted. THIS MUST BE CALLED
	void setEmittedType(CPSLocated *et) { _EmittedType = et ; }

	/// get emitted type
	CPSLocated *getEmittedType(void) { return _EmittedType ; }
	/// get const ptr on emitted type
	const CPSLocated *getEmittedType(void) const { return _EmittedType ; }

	/// the emission frequency
	/// fill this struct with the desired params 
	CPSFrequency _Freq ;


	/// Serialization
	virtual void serial(NLMISC::IStream &f) ;

protected:

	/** This method is called each time one (and only one) located must be emitted.
	 *  DERIVERS MUST DEFINE THIS
	 *  \param index the index of the emitter in the tab that generated a located
	 */
	virtual void emit(uint32 index) = 0 ;

	/// a pointer on the type to be emitted
	CPSLocated *_EmittedType ;

	/** the phase (  0 < phase  < period of emission)
     *  its usage depends on the freq member
	 */
	TPSAttribFloat _Phase ; 
};


class CPSEmitterOmni : public CPSEmitter
{
	
public:


	/// serialisation
 	virtual	void serial(NLMISC::IStream &f) ;

	
	NLMISC_DECLARE_CLASS(CPSEmitterOmni) ;


	/// emission of located
	virtual void emit(uint32 index) ;

} ; 


} // NL3D


#endif // NL_PS_EMITTER_H

/* End of ps_emitter.h */
