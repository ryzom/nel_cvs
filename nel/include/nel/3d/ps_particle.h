/** \file ps_particle.h
 * <File description>
 *
 * $Id: ps_particle.h,v 1.5 2001/05/02 11:49:50 vizerie Exp $
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
#include "nel/3d/vertex_buffer.h"




namespace NL3D {

template <typename T> struct CPSAttribMaker ;

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
	* process one pass for the particles. The default behaviour shows the particles
	*/
	virtual void step(TPSProcessPass pass, CAnimationTime ellapsedTime)
	{
		if (pass == PSBlendRender)
		{
			draw() ;
		}
	}


	/// draw the particles

	virtual void draw(void) = 0 ;

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
	virtual void serial(NLMISC::IStream &f) { CPSLocatedBindable::serial(f) ; }
	
};






/// this class adds tunable color to a particle. Must be added using public inheritance
class CPSColoredParticle
{
	public:

		/** Set an attribute maker that produce a color
		 *  It must have been allocated by new
		 * It will be deleted by this object
		 */
		void setColorScheme(CPSAttribMaker<CRGBA> *col) ;

		/// Set a constant color for the particles
		void setColor(const NLMISC::CRGBA col) ;

		/// ctor : default are white particles (constant color)
		CPSColoredParticle() ;

		/// dtor
		~CPSColoredParticle() ;

		/// serialization
		void serialColorScheme(NLMISC::IStream &f) ;

	protected:		
		/// if this is false, constant color will be used instead of a scheme
		bool _UseColorScheme ;

		CRGBA _Color ;

		// used only if _UseColorScheme is set to true

		CPSAttribMaker<CRGBA> *_ColorScheme ; 						

		/// Update the material and the vb and the like so that they match the color scheme
		virtual void updateMatAndVb(void) = 0 ;
} ;


/// this class adds tunable size to a particle. Must be added using public inheritance
class CPSSizedParticle
{
	public:

		/** Set an attribute maker that produce a size
		 *  It must have been allocated by new
		 *  It will be deleted by this object
		 */
		virtual void setSizeScheme(CPSAttribMaker<float> *size) ;

		/// Set a constant size for the particles
		virtual void setSize(float size) ;

		/// ctor : default are 0.1f particles
		CPSSizedParticle() ;

		/// dtor
		~CPSSizedParticle() ;

		/// serialization
		void serialSizeScheme(NLMISC::IStream &f) ;

	protected:		
		/// if this is false, constant size will be used instead of a scheme
		bool _UseSizeScheme ;
		float _ParticleSize ;
		CPSAttribMaker<float> *_SizeScheme ; // used only if _UseSizeScheme is set to true				
} ;

/// this class adds tunable 2D rotation to a particle





/**
 *	this is just a coloured dot that fade to black during its life
 */

class CPSDot : public CPSParticle, public CPSColoredParticle
{
	public:
		/**
		* process one pass for the particle	
		*/
		virtual void draw(void) ;

		void resize(uint32 size) { _Vb.setNumVertices(size) ;}

		/// we don't save datas so it does nothing for now
		bool newElement(void) { return true ; }

		/// we don't save datas so it does nothing for now
		void deleteElement(uint32) {}
	
		
		/// ctor

		CPSDot() { init() ; }

		NLMISC_DECLARE_CLASS(CPSDot) ;

		///serialisation
		void serial(NLMISC::IStream &f) ;
	protected:
		
		void init(void) ;		
		CMaterial _Mat ;
		CVertexBuffer _Vb ;

		/// update the material and the vb so that they match the color scheme
		virtual void updateMatAndVb(void) ;
} ;


/**
 * A FaceLookAt particle
 */


class CPSFaceLookAt : public CPSParticle, public CPSColoredParticle, public CPSSizedParticle
{
public:
	/// create the face look at by giving a texture and an optionnal color
	CPSFaceLookAt(CSmartPtr<ITexture> tex) ;
	virtual void draw(void) ;
	void serial(NLMISC::IStream &f) ;
	
	NLMISC_DECLARE_CLASS(CPSFaceLookAt) ;

	virtual bool completeBBox(NLMISC::CAABBox &box) const   ;

	virtual void resize(uint32 size) ; 
		
	/// we don't save datas so it does nothing for now
	bool newElement(void) { return true ; }

	/// we don't save datas so it does nothing for now
	void deleteElement(uint32) {}

	// default ctor

	CPSFaceLookAt() ;

	//dtor

	~CPSFaceLookAt() ;
protected:

	/// initialisations
	virtual void init(void) ;
	
	CSmartPtr<ITexture> _Tex ;

	// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void updateMatAndVb(void) ;

	CMaterial _Mat ;
	CVertexBuffer _Vb ;

	// an index buffer used for drawing
	uint32 *_IndexBuffer ;

	
} ;






} // NL3D


#endif // NL_PS_PARTICLE_H

/* End of ps_particle.h */
