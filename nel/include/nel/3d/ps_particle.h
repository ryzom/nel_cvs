/** \file ps_particle.h
 * <File description>
 *
 * $Id: ps_particle.h,v 1.9 2001/05/11 17:17:22 vizerie Exp $
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
#include "nel/3d/primitive_block.h"




namespace NL3D {

template <typename T> class CPSAttribMaker ;
class CTextureGrouped ;

/**
 * This is the base class for all particle
 * A deriver must provide a drawing method for his particle
 * Not sharable.
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

	/**	Generate a new element for this bindable. They are generated according to the properties of the class	
	 * \return true if it could be added
	 */
	virtual void newElement(void) = 0 ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index) = 0 ;

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size) = 0 ;

	/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{ 
		CPSLocatedBindable::serial(f) ; 
	}
	
};






/// this class adds tunable color to a particle. Can be added using public multiple inheritance
class CPSColoredParticle
{
	public:

		/** Set an attribute maker that produce a color
		 *  It must have been allocated by new
		 * It will be deleted by this object
		 */
		void setColorScheme(CPSAttribMaker<CRGBA> *col) ;

		/// Set a constant color for the particles. remove any previous scheme
		void setColor(const NLMISC::CRGBA col) ;

		/// ctor : default are white particles (constant color)
		CPSColoredParticle() ;

		/// dtor
		~CPSColoredParticle() ;

		/// serialization. 
		void serialColorScheme(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	protected:		
		/// if this is false, constant color will be used instead of a scheme
		bool _UseColorScheme ;

		CRGBA _Color ;

		// used only if _UseColorScheme is set to true

		CPSAttribMaker<CRGBA> *_ColorScheme ; 						

		/// Update the material and the vb and the like so that they match the color scheme
		virtual void updateMatAndVbForColor(void) = 0 ;
} ;


/// this class adds tunable size to a particle. Can be added using public multiple inheritance
class CPSSizedParticle
{
	public:

		/** Set an attribute maker that produce a size
		 *  It must have been allocated by new
		 *  It will be deleted by this object
		 */
		void setSizeScheme(CPSAttribMaker<float> *size) ;

		/// Set a constant size for the particles
		void setSize(float size) ;

		/// ctor : default are 0.1f particles
		CPSSizedParticle() ;

		/// dtor
		~CPSSizedParticle() ;

		/// serialization. We choose a different name because of multiple-inheritance
		void serialSizeScheme(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	protected:		
		/// if this is false, constant size will be used instead of a scheme
		bool _UseSizeScheme ;
		float _ParticleSize ;
		CPSAttribMaker<float> *_SizeScheme ; // used only if _UseSizeScheme is set to true				
} ;



/// this class adds tunable 2D rotation to a particle, it can be used by public multiple inheritance

class CPSRotated2DParticle
{
	public:

		/** Set an attribute maker that produce a float
		 *  It must have been allocated by new
		 *  It will be deleted by this object
		 *  Output angles must range from 0.0f to 256.0f
		 */
		void setAngle2DScheme(CPSAttribMaker<float> *size) ;

		/** Set a constant angle for the particle. Angles range from  0.0f to 256.0f (2 pi)
		 *	This discrad any previous scheme
		 * \see setAngle2DScheme()
		 */
		void setAngle2D(float angle) ;

		/// ctor : default are unrotated particles (angle = 0.0f)
		CPSRotated2DParticle() ;

		/// dtor
		~CPSRotated2DParticle() ;

		/// serialization. We choose a different name because of multiple-inheritance
		void serialAngle2DScheme(NLMISC::IStream &f) throw(NLMISC::EStream) ;



		/** this return a float table used to speed up rotations of face look at and the like
		 * for each angle, there are 4 float : 2 couple of float : a1, b1, a2, b2
		 * a1 * I + b1 * K = up left corner, a2 * I + b2 * K = up right corner, 
		 * This table must have been initialized with initRotTable
		 */
		static inline const float *getRotTable(void)
		{
			nlassert(_InitializedRotTab) ; // must have called initRotTable at the start of the apply
			return _RotTable ;
		}

		/// init the rotation table

		static void initRotTable(void) ;


	protected:		
		/// if this is false, constant size will be used instead of a scheme
		bool _UseAngle2DScheme ;
		float _Angle2D ;
		CPSAttribMaker<float> *_Angle2DScheme ; // used only if _UseSizeScheme is set to true				
		static float _RotTable[4 * 256] ;

		#ifdef NL_DEBUG
			/// it is true if the table has been initialized, for debug purposes
			static bool _InitializedRotTab ;
		#endif
} ;

/// this class adds a texture to a particle. The texture can be animated or not. it can be used by public multiple inheritance
 

class CPSTexturedParticle
{
	public:

		/** Set an attribute maker that produce a sint32
		 *  It must have been allocated by new
		 *  It will be deleted by this object		
		 *  The integer is used as an index in a grouped texture. It tells which frame to use
		 */
		void setTextureScheme(CSmartPtr<CTextureGrouped> textureGroup, CPSAttribMaker<sint32> *animOrder) ;

		/** Set a constant texture for the particle
		 *	This discard any previous scheme
		 * \see setTextureScheme()
		 */
		void setTexture(CSmartPtr<ITexture> tex) ;

		/// ctor : default have no texture. You must set it, otherwise you'll get an assertion when it's drawn


		CPSTexturedParticle() ;

		/// dtor
		~CPSTexturedParticle() ;

		/// serialization. We choose a different name because of multiple-inheritance

		void serialTextureScheme(NLMISC::IStream &f) throw(NLMISC::EStream) ;		


	protected:		
		/// if this is false, constant size will be used instead of a scheme

		bool _UseTextureScheme ;

		// a single texture
		CSmartPtr<ITexture> _Tex ;

		// a grouped texture
		CSmartPtr<CTextureGrouped> _TexGroup ;		
		
		CPSAttribMaker<sint32> *_TextureScheme ; // used only if _UseSizeScheme is set to true							


		/// Update the material so that it match the texture scheme
		virtual void updateMatAndVbForTexture(void) = 0 ;
} ;





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

		/** Set the nax number of dot
		* should not be called directly. Call CPSLocated::resize instead
	    */
		void resize(uint32 size) { _Vb.setNumVertices(size) ;}

		/// we don't save datas so it does nothing for now
		void newElement(void) {}

		/// we don't save datas so it does nothing for now
		void deleteElement(uint32) {}
	
		
		/// ctor

		CPSDot() { init() ; }

		NLMISC_DECLARE_CLASS(CPSDot) ;

		///serialisation
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	protected:
		
		void init(void) ;		
		CMaterial _Mat ;
		CVertexBuffer _Vb ;

		/// update the material and the vb so that they match the color scheme
		virtual void updateMatAndVbForColor(void) ;
} ;


/**
 * A FaceLookAt particle
 */


class CPSFaceLookAt : public CPSParticle, public CPSColoredParticle
					, public CPSSizedParticle, public CPSRotated2DParticle
					, public CPSTexturedParticle
{
public:
	/// create the face look at by giving a texture and an optionnal color
	CPSFaceLookAt(CSmartPtr<ITexture> tex = NULL) ;
	virtual void draw(void) ;
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
	NLMISC_DECLARE_CLASS(CPSFaceLookAt) ;

	virtual bool completeBBox(NLMISC::CAABBox &box) const   ;

	/** Set the max number of faceLookAt. 
	* should not be called directly. Call CPSLocated::resize instead
	*/
	virtual void resize(uint32 size) ; 
		
	/// we don't save datas so it does nothing for now
	void newElement(void) {}

	/// we don't save datas so it does nothing for now
	void deleteElement(uint32) {}

	//dtor

	~CPSFaceLookAt() ;
protected:

	/// initialisations
	virtual void init(void) ;	

	// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void updateMatAndVbForColor(void) ;

	// update the material and the vb so that they match the texture scheme.
	virtual void updateMatAndVbForTexture(void) ;

	CMaterial _Mat ;
	CVertexBuffer _Vb ;

	// an index buffer used for drawing
	uint32 *_IndexBuffer ;

	
} ;


/**
 * A fan light particle
 */



class CPSFanLight : public CPSParticle, public CPSColoredParticle, public CPSSizedParticle, public CPSRotated2DParticle
{
public:
	virtual void draw(void) ;

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	NLMISC_DECLARE_CLASS(CPSFanLight) ;

	virtual bool completeBBox(NLMISC::CAABBox &box) const   ;

	/** Set the max number of fanlights
	* should not be called directly. Call CPSLocated::resize instead
	*/
	virtual void resize(uint32 size) ; 
	
	/// we don't save datas so it does nothing for now
	void newElement(void) {}

	/// we don't save datas so it does nothing for now
	void deleteElement(uint32) {}


	/// Ctor, with the numbers of fans to draw (minimum is 3, maximum is 128)
	CPSFanLight(uint32 nbFans = 7) ;

	// Set the number of fans used for drawing (minimum is 3, maximum is 128)
	void setNbFans(uint32 nbFans) ;

	// Get the number of fans used for drawing
	uint32 gteNbFans(void) const
	{
		return _NbFans ;
	}

	/** Set the speed for phase
	 *	If the located holding this particle as a limited lifetime, it gives how many 0-2Pi cycle it'll do during its life
	 *  Otherwise it gives how many cycle there are in a second
	 */
	void setPhaseSpeed(float multiplier) ;

	// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void updateMatAndVbForColor(void) ;


	/// must call this at least if you intend to use fanlight
	static void initFanLightPrecalc(void) ;

	/// dtor
	~CPSFanLight() ;

protected:
	/// initialisations
	virtual void init(void) ;

	uint32 _NbFans ;
	CVertexBuffer _Vb ;
	CMaterial _Mat ;
	uint32   *_IndexBuffer ;

	static uint8 _RandomPhaseTab[128] ;

	float _PhaseSpeed ;

	#ifdef NL_DEBUG		
		static bool _RandomPhaseTabInitialized ;
	#endif
} ;


/**
 *  These particle are like dot, but a tail is following them. The number of line in the tails can be tuned
 */

class CPSTailDot : public CPSParticle, public CPSColoredParticle
{
	public:
		/// process one pass for the particle	
		
		virtual void draw(void) ;

		/** Set the max number of TailDot
		* should not be called directly. Call CPSLocated::resize instead
		*/
		void resize(uint32 size) ;
		
		void newElement(void) ;
		
		void deleteElement(uint32 index) ;
	
		/** (de)activate color fading
		 * when its done, colors fades to black along the tail
		 */
		void setColorFading(bool onOff = true) 
		{ 
			_ColorFading = onOff ; 
			setupColor() ;
		}

		/// test wether color fading is activated
		bool getColorFading(void) const { return _ColorFading ; }
		
		/// ctor. It tells how many segments there are in the tail
		CPSTailDot(uint32 nbSegmentInTail = 4) ;

		// set the number of segments in the tail		
		void setTailNbSeg(uint32 nbSeg) ;

		// get the number of segments in the tail
		uint getTailNbSeg(uint32 getNbSeg) const { return _TailNbSeg ; }

		NLMISC_DECLARE_CLASS(CPSTailDot) ;

		///serialisation
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
		
		/** tells in which basis is the tail
		 *  It requires one transform per particle if it is not the same as the located that hold that particle
		 *  The default is false. With that you can control if a rotation of the system will rotate the tail
		 */
		void setSystemBasis(bool yes) ;

		
		/// return true if the tails are in the system basis
		bool isInSystemBasis(void) const { return _SystemBasisEnabled ; }


	protected:
		
		void init(void) ;		
		CMaterial _Mat ;
		CVertexBuffer _Vb ;	
		// a set of lines to draw
		CPrimitiveBlock _Pb ; 

		// number of segments in the tail
		uint32 _TailNbSeg ;

		
		// true if the rail is in the system basis, false otherwise
		bool _SystemBasisEnabled ;



		
		/// true if the tail color must fade to black
		bool _ColorFading ;
							
		
		// resize the vertex buffer to keep old datas
		void CPSTailDot::resizeVb(uint32 oldTailNbSeg, uint32 size) ;

		// setup the initial colors in the whole vb : black or a precomputed gradient for constant color
		void setupColor(void) ;



		/// update the material and the vb so that they match the color scheme

		virtual void updateMatAndVbForColor(void) ;
} ;






} // NL3D


#endif // NL_PS_PARTICLE_H

/* End of ps_particle.h */
