/** \file ps_force.h
 * <File description>
 *
 * $Id: ps_force.h,v 1.2 2001/04/26 08:46:34 vizerie Exp $
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


namespace NL3D {



/** This struct describe where 
 * 


/**
 * All forces in the system derive from this class
 * It all a list all located on which the force can apply
 * Only the motion and toolRender passes are supported for a force
 * The corresponding virtual calls are done
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSForce : public CPSLocatedBindable
{
public:

	/// Constructor
	CPSForce();

	/// Add a new type of located for this force to apply on. nlassert if present
	virtual void attachTarget(CSmartPtr<CPSLocated> ptr) ;

	/// Detach a target. If not present -> assert
	virtual void detachTarget(CSmartPtr<CPSLocated> ptr) ;

	/// return the number of targets
	uint32 getNbTargets(void) const { return _Targets.size() ; }

	/// Return a ref on a target. Invalid range -> nlassert
	CSmartPtr<CPSLocated> getTarget(uint32 index) 
	{
		nlassert(index < _Targets.size()) ;
		return _Targets[index] ;
	}

	/// Return a const ref on a target. Invalid range -> nlassert
	const CSmartPtr<CPSLocated> getTarget(uint32 index) const
	{
		nlassert(index < _Targets.size()) ;
		return _Targets[index] ;
	}

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


	virtual bool newElement(void) = 0 ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for of the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) = 0 ;

	/// Resize the bindable attributes containers DERIVERS SHOULD CALL THEIR PARENT VERSION
	virtual void resize(uint32 size) = 0 ;

	

protected:
	typedef std::vector<CSmartPtr<CPSLocated> > TTargetCont ;
	TTargetCont _Targets ;

};


// example of gravity class for test
class CPSGravity : public CPSForce
{
public:
	/// Compute the force on the targets
	virtual void performMotion(CAnimationTime ellapsedTime)  ;

	/// Show the force (edition mode)
	virtual void show(CAnimationTime ellapsedTime)  ;


	NLMISC_DECLARE_CLASS(CPSGravity) ; 


	virtual bool newElement(void) { return true ; } ;	
	virtual void deleteElement(uint32 index) {} ;	
	virtual void resize(uint32 size) {} ;

} ;




} // NL3D


#endif // NL_PS_FORCE_H

/* End of ps_force.h */
