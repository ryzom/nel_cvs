/** \file ps_particle.h
 * <File description>
 *
 * $Id: ps_particle.h,v 1.2 2001/04/26 08:46:34 vizerie Exp $
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

#ifndef NL_PS_PARTICLE_H
#define NL_PS_PARTICLE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/material.h"
#include "nel/3d/ps_util.h"


namespace NL3D {


/**
 * This is the base class for all particle
 * A deriver must provide a drawing method for his particle
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSParticle : public CPSLocatedBindable
{
public:

	/// Constructor
	CPSParticle();


	/// return this bindable type
	uint32 getType(void) const { return PSParticle ; }


	/// return priority
	virtual uint32 getPriority(void) const { return 1000 ; }
	
	/**
	* process one pass for the particle	
	*/
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) = 0 ;

	/**	Generate a new element for this bindable. They are generated according to the propertie of the class	
	 * \return true if it could be added
	 */
	virtual bool newElement(void) = 0 ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) = 0 ;

	/// Resize the bindable attributes containers DERIVERS MUST CALL THEIR PARENT VERSION
	virtual void resize(uint32 size) = 0 ;

	/// serialisation. Derivers must override this
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) { CPSLocatedBindable::serial(f) ; }


};



/**
 *	this is just a coloured dot that fade to black during its life
 */

class CPSDot : public CPSParticle
{
	public:
		/**
		* process one pass for the particle	
		*/
		virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;

		void resize(uint32) {}
		bool newElement(void) { return true ; }
		void deleteElement(uint32) {}
	
		
		NLMISC_DECLARE_CLASS(CPSDot) ;
		// ctor
		CPSDot(const CRGBA &color = CRGBA(255, 255, 255)) ;


		/// set the color of the dot
		void setColor(const CRGBA &c) { _Color = c ; }

		///serialisation
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	protected:
		void init(void) ;
		CRGBA _Color ;
		CMaterial _Mat ;
} ;


/**
 * A FaceLookAt particle
 */



const float FaceLookAtSize = 0.3f ;

class CPSFaceLookAt : public CPSDot
{
public:
	/// create the face look at by giving a texture and an optionnal color
	CPSFaceLookAt(CSmartPtr<ITexture> tex, const CRGBA &c = CRGBA(255, 255, 255)) ;
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime) ;
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
	NLMISC_DECLARE_CLASS(CPSFaceLookAt) ;

	virtual bool completeBBox(NLMISC::CAABBox &box) const  
	{ 
		CPSUtil::addRadiusToAABBox(box, FaceLookAtSize) ;
		return true  ;	
	}


	CPSFaceLookAt() {}
protected:
	
	CSmartPtr<ITexture> _Tex ;
} ;






} // NL3D


#endif // NL_PS_PARTICLE_H

/* End of ps_particle.h */
