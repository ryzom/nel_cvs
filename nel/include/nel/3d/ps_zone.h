/** \file ps_zone.h
 * <File description>
 *
 * $Id: ps_zone.h,v 1.1 2001/04/25 08:44:43 vizerie Exp $
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
#include "ps_force.h"



namespace NL3D {



using NLMISC::CPlane ;

/**
 * This class hold any entity that has an effect over located : a sink, a bouncing zone etc
 * This is a kind a specialized force, and it has an attached list of the targets
 * 
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSZone : public CPSForce
{
public:

	/// Constructor
	CPSZone();

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
	virtual void attachTarget(CSmartPtr<CPSLocated> ptr) ;

	/// Detach a target. If not present -> assert
	virtual void detachTarget(CSmartPtr<CPSLocated> ptr) ;

protected:

	/**
	 * This set speed of a located so that it looks like bouncing on a surface
	 * \param locatedIndex the index 
	 * \param bouncePoint the position where the collision occured
	 * \param surfNormal  the normal of the surface at the collision point (this must be a unit vector)
	 * \elasticity  1 = full bounce, 0 = no bounce (contact)
	 * \ellapsedTime the time ellapsed
	 */

	void bounce(uint32 locatedIndex, const CVector &bouncePoint, const CVector &surfNormal, float elasticity, float ellapsedTime) ;




};


/** A plane over which particle bounce
 *
 */

class CPSZonePlane : public CPSZone
{
	public:
		virtual void performMotion(CAnimationTime ellapsedTime) ;
		virtual void show(CAnimationTime ellapsedTime)  ;
		virtual void resize(uint32 size) ;
		virtual bool newElement(void) ;
		virtual void deleteElement(uint32 index) ;
		virtual std::string getClassName(void) { return std::string("NL3D::CPSZonePlane") ; }
	protected:
		TPSAttribVector _Normal ;		
} ;

} // NL3D


#endif // NL_PS_ZONE_H

/* End of ps_zone.h */
