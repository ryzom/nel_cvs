/** \file ps_zone.h
 * <File description>
 *
 * $Id: ps_zone.h,v 1.5 2001/05/11 17:17:22 vizerie Exp $
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

#ifndef NL_PS_ZONE_H
#define NL_PS_ZONE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_edit.h"



namespace NL3D {


/** This epsilon is in meter and give a thickness to surfaces for tests. It must be above above 0
* for the system to work correctly
*/
const float PSCollideEpsilon = 10E-5f ;



/**
 * This class hold any entity that has an effect over located : a sink, a bouncing zone etc
 * This is a kind a specialized force, and it has an attached list of the targets
 * Not sharable.
 * 
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSZone : public CPSTargetLocatedBindable
{
public:

	/// Constructor
	CPSZone();

	// dtor

	~CPSZone() ;

	/**
	*  Gives the type for this bindable.	
	*/
	virtual uint32 getType(void) const { return PSZone ; }

	/**
	* Get the priority of the bindable
	* The more high it is, the earlier it is dealt with
	*/
	virtual uint32 getPriority(void) const { return 3500 ; }
	
	/**
	 * Process one pass for the zone
	 * The default behaviour call performMotion or show depending on the pass being processed
	 */
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;

	
	/// This is usually called by the step method for the pass dedicated to zone
	virtual void performMotion(CAnimationTime ellapsedTime) = 0 ;

	/// Show the zone (edition mode). This is usually called 
	virtual void show(CAnimationTime ellapsedTime) = 0 ;
		

	/// Add a new type of located for this zone to apply on. nlassert if present	
	virtual void attachTarget(CPSLocated *ptr) ;


	
	/// serialisation, DERIVER must override this
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) 
	{ 
		CPSTargetLocatedBindable::serial(f) ; 
	}


	/** Inherited from CPSTargetLocatedBindable. Its called when one of the targets has been detroyed or detached
	 *  The default behaviour, release collision infos from the located
	 */
	
	virtual void releaseTargetRsc(CPSLocated *target) ;


protected:

	/**
	 * This set speed of a located so that it looks like bouncing on a surface
	 * \param locatedIndex the index 
	 * \param bouncePoint the position where the collision occured
	 * \param surfNormal  the normal of the surface at the collision point (this must be a unit vector)
	 * \elasticity  1 = full bounce, 0 = no bounce (contact)
	 * \ellapsedTime the time ellapsed
	 */

//	void bounce(uint32 locatedIndex, const CVector &bouncePoint, const CVector &surfNormal, float elasticity, float ellapsedTime) ;




};


/** A plane over which particles bounce
 * It has an interface to move each plane individually
 */

class CPSZonePlane : public CPSZone, public IPSMover
{
	public:
		virtual void performMotion(CAnimationTime ellapsedTime) ;
		virtual void show(CAnimationTime ellapsedTime)  ;
		virtual void resize(uint32 size) ;
		virtual void newElement(void) ;
		virtual void deleteElement(uint32 index) ;

		NLMISC_DECLARE_CLASS(CPSZonePlane) ;


		// left multiply the current matrix by the given one. No valid index -> assert
		virtual void applyMatrix(uint32 index, const CMatrix &m) ;
		// return a matrix of the system. No valid index -> assert
		virtual CMatrix getMatrix(uint32 index) const ;

		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	protected:
		TPSAttribVector _Normal ;
		CMatrix buildBasis(uint32 index) const ;
} ;

} // NL3D


#endif // NL_PS_ZONE_H

/* End of ps_zone.h */
