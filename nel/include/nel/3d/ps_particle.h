/** \file ps_particle.h
 * <File description>
 *
 * $Id: ps_particle.h,v 1.17 2001/06/08 08:30:43 vizerie Exp $
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
#include "nel/3d/ps_plane_basis.h"
#include "nel/3d/material.h"
#include "nel/3d/ps_util.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/primitive_block.h"
#include "nel/3d/mesh.h"



namespace NL3D {





////////////////////////////////
// class forward declarations //
////////////////////////////////


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


	/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{ 
		CPSLocatedBindable::serial(f) ; 
	}
protected:
		/**	Generate a new element for this bindable. They are generated according to the properties of the class		 
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
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// this class adds tunable color to a particle. Can be added using public multiple inheritance
class CPSColoredParticle
{
	public:

		/** Set an attribute maker that produce a color
		 *  It must have been allocated by new
		 * It will be deleted by this object
		 */
		void setColorScheme(CPSAttribMaker<CRGBA> *col) ;

		/// get the color scheme (NULL if none)
		CPSAttribMaker<CRGBA> *getColorScheme(void) { return _ColorScheme ; }

		/// get the color scheme (NULL if none) const version
		const CPSAttribMaker<CRGBA> *getColorScheme(void) const { return _ColorScheme ; }

		/// Set a constant color for the particles. remove any previous scheme
		void setColor(NLMISC::CRGBA col) ;

		/// Get the color
		NLMISC::CRGBA getColor(NLMISC::CRGBA col) const { return _Color ; }

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



		/// get the size scheme (NULL if none)
		CPSAttribMaker<float> *getSizeScheme(void) { return _SizeScheme ; }

		/// get the size scheme (NULL if none) const version
		const CPSAttribMaker<float> *getSizeScheme(void) const { return _SizeScheme ; }

		/// Set a constant size for the particles
		void setSize(float size) ;

		/// get the constant size
		float getSize(void) const { return _ParticleSize ; }

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
		void setAngle2DScheme(CPSAttribMaker<float> *scheme) ;

		/// get the angle 2D scheme (NULL if none)
		CPSAttribMaker<float> *getAngle2DScheme(void) { return _Angle2DScheme ; }

		/// get the angle 2D scheme (NULL if none) const version
		const CPSAttribMaker<float> *getAngle2DScheme(void) const { return _Angle2DScheme ; }


		/** Set a constant angle for the particle. Angles range from  0.0f to 256.0f (2 pi)
		 *	This discrad any previous scheme
		 * \see setAngle2DScheme()
		 */
		void setAngle2D(float angle) ;

		/// get the constant 
		float getAngle2D(void) const { return _Angle2D ; }

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

/** this class adds a texture to a particle. The texture can be animated or not. it can be used by public multiple inheritance.
 *  The frame animation are all stored in the same texture for optimisation so it's not suited for large anim...
 */
class CPSTexturedParticle
{
	public:

		/** Set an attribute maker that produce a sint32
		 *  It must have been allocated by new
		 *  It will be deleted by this object		
		 *  The integer is used as an index in a grouped texture. It tells which frame to use
		 */
		void setTextureScheme(CSmartPtr<CTextureGrouped> textureGroup, CPSAttribMaker<sint32> *animOrder) ;

		/// get the texture scheme (null if none) const version
		CPSAttribMaker<sint32> *getTextureScheme(void) { return _TextureScheme ; }

		/// get the texture scheme (null if none) const version
		const CPSAttribMaker<sint32> *getTextureScheme(void) const { return _TextureScheme ; }

		/// get the texture group used if there's a texture scheme
		CTextureGrouped *getTextureGroup(void) { nlassert(getTextureScheme()) ; return _TexGroup ; }

		/// get the texture group used if there's a texture scheme (const version)
		const CTextureGrouped *getTextureGroup(void) const { nlassert(getTextureScheme()) ; return _TexGroup ; }


		/** Set a constant texture for the particle
		 *	This discard any previous scheme
		 * \see setTextureScheme()
		 */
		void setTexture(CSmartPtr<ITexture> tex) ;

		/// get the constant texture
		const ITexture *getTexture(void) const { return _Tex ; }

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




 


/** this class adds tunable 3D rotation to a PLANE particle, it can be used by public multiple inheritance
 *  It must just produce 2 vectors that give the x and y vector of the local basis.
 */
class CPSRotated3DPlaneParticle
{
	public:

		/** Set an attribute maker that produce a basis
		 *  It must have been allocated by new
		 *  It will be deleted by this object				
		 */
		void setPlaneBasisScheme(CPSAttribMaker<CPlaneBasis> *basisMaker) ;

		/** Set a constant basis for all particles
		 * \see setPlaneBasisSchemeScheme()
		 */

		/// get the plane basis scheme, (NULL if none)
		CPSAttribMaker<CPlaneBasis> *getPlaneBasisScheme(void) { return _PlaneBasisScheme ; }

		/// get the plane basis scheme, (NULL if none) const version
		const CPSAttribMaker<CPlaneBasis> *getPlaneBasisScheme(void) const { return _PlaneBasisScheme ; }

		void setPlaneBasis(const CPlaneBasis &basis) ;

		/// get the constant basis
		CPlaneBasis getPlaneBasis(void) const { return _PlaneBasis ; }

		/// ctor : default have constant basis that map to the I & J vector (e.g identity)
		CPSRotated3DPlaneParticle() ;

		/// dtor
		~CPSRotated3DPlaneParticle() ;

		/// serialization. We choose a different name because of multiple-inheritance

		void serialPlaneBasisScheme(NLMISC::IStream &f) throw(NLMISC::EStream) ;		


	protected:		
		/// if this is false, constant size will be used instead of a scheme

		bool _UsePlaneBasisScheme ;
		
		CPSAttribMaker<CPlaneBasis> *_PlaneBasisScheme ; // used only if _UseSizeScheme is set to true							
		
		CPlaneBasis _PlaneBasis ; // constant basis..
} ;



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	
	
		
		/// ctor

		CPSDot() { init() ; _Name = std::string("point") ;}

		NLMISC_DECLARE_CLASS(CPSDot) ;

		///serialisation
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	protected:
		
		void init(void) ;		
		CMaterial _Mat ;
		CVertexBuffer _Vb ;

		/// update the material and the vb so that they match the color scheme
		virtual void updateMatAndVbForColor(void) ;
		
		/** Set the nax number of dot		
	    */
		void resize(uint32 size) ;

		/// we don't save datas so it does nothing for now
		void newElement(void) {}

		/// we don't save datas so it does nothing for now
		void deleteElement(uint32) {}
} ;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** This abstract class holds what is needed with quad particles (CPSFaceLookAt, CPSFace) e.g 
 *  Index and vertex buffer and method to setup them
 *  Material, and method to setup them
 */

 class CPSQuad : public CPSParticle
	           , public CPSColoredParticle
			   , public CPSTexturedParticle
			   , public CPSSizedParticle
 {

	protected:

		/** create the quad by giving a texture. This can't be a CTextureGrouped (for animation)
		* animation must be set later by using setTextureScheme
		*/
		CPSQuad(CSmartPtr<ITexture> tex = NULL) ;

		// dtor
		virtual ~CPSQuad() ;

		/// initialisations
		virtual void init(void) ;	

		/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
		virtual void updateMatAndVbForColor(void) ;

		/// update the material and the vb so that they match the texture scheme.
		virtual void updateMatAndVbForTexture(void) ;

		/// we don't save datas so it does nothing for now
		void newElement(void) {}

		/// we don't save datas so it does nothing for now
		void deleteElement(uint32) {}
	
		/// complete the bbox depending on the size of particles
		virtual bool completeBBox(NLMISC::CAABBox &box) const   ;

		/** calculate current color and texture coordinate before any rendering
		 *  size can't be higher that quadBufSize ...
		 */
		void updateVbColNUVForRender(uint32 startIndex, uint32 numQuad)	 ;	



		CMaterial _Mat ;
		CVertexBuffer _Vb ;

		/// an index buffer used for drawing the quads
		CPrimitiveBlock _Pb ;

		/// DERIVER MUST CALL this		 
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;	

		/** Set the max number of particles. 
		 * should not be called directly. Call CPSLocated::resize instead
		 */
		virtual void resize(uint32 size) ; 
 } ;



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * A FaceLookAt particle
 */


class CPSFaceLookAt :   public CPSQuad
					  , public CPSRotated2DParticle
					
{
public:
	/** create the face look at by giving a texture. This can't be a CTextureGrouped (for animation)
     * animation must be set later by using setTextureScheme
	 */
	CPSFaceLookAt(CSmartPtr<ITexture> tex = NULL) ;
	virtual void draw(void) ;
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
	
	NLMISC_DECLARE_CLASS(CPSFaceLookAt) ;
	
		
	/** activate 'fake' motion blur (its done by deforming the quad)
	 *  This is slower, however. This has no effect with rotated particles.
	 *  \param coeff a coefficient for motion blur (too high value may give unrealistic result)
	 *         0 deactivate the motion blur
	 *  \param threshold : speed will be clamped below this value	 
	 */
	void activateMotionBlur(float coeff = 1.f, float threshold = 1.f)
	{
		_MotionBlurCoeff = coeff ;
		_Threshold = threshold ;
	}

	/** return the motion blur coeff (0.f means none)
	 *  \see  activateMotionBlur()
	 */
	float getMotionBlurCoeff(void) const { return _MotionBlurCoeff ; }

	/// we don't save datas so it does nothing for now
	void newElement(void) {}

	/// we don't save datas so it does nothing for now
	void deleteElement(uint32) {}

protected:

	float _MotionBlurCoeff ;

	// threshold for the motion blur
	float _Threshold ;

} ;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

	/// Ctor, with the numbers of fans to draw (minimum is 3, maximum is 128)
	CPSFanLight(uint32 nbFans = 7) ;

	// Set the number of fans used for drawing (minimum is 3, maximum is 128)
	void setNbFans(uint32 nbFans) ;

	// Get the number of fans used for drawing
	uint32 getNbFans(void) const
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
	
	/// we don't save datas so it does nothing for now
	void newElement(void) {}

	/// we don't save datas so it does nothing for now
	void deleteElement(uint32) {}

	/// Set the max number of fanlights		
	virtual void resize(uint32 size) ; 

} ;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *  These particle are like dot, but a tail is following them. The number of line in the tails can be tuned
 */

class CPSTailDot : public CPSParticle, public CPSColoredParticle
{
	public:
		/// process one pass for the particle	
		
		virtual void draw(void) ;
		
	
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
		
		/// ctor. It tells how many segments there are in the tail. 255 is the maximum
		CPSTailDot(uint32 nbSegmentInTail = 4) ;

		/** set the number of segments in the tail		
		 * 255 is the maximum
	     */
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
		void setSystemBasis(bool yes) { _SystemBasisEnabled = yes ; }

		
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

		
		// true if the tail is in the system basis, false otherwise
		bool _SystemBasisEnabled ;



		
		/// true if the tail color must fade to black
		bool _ColorFading ;
							
		
		// resize the vertex buffer to keep old datas
		void CPSTailDot::resizeVb(uint32 oldTailNbSeg, uint32 size) ;

		// setup the initial colors in the whole vb : black or a precomputed gradient for constant color
		void setupColor(void) ;



		/// update the material and the vb so that they match the color scheme

		virtual void updateMatAndVbForColor(void) ;

		/// Set the max number of TailDot				
		void resize(uint32 size) ;
		
		void newElement(void) ;
		
		void deleteElement(uint32 index) ;
} ;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** 'Ribbon' particle : a shape is extruded while it follows the particle, thus
 *  giving a ribbon effect
 *  It inherit the CPSSizedParticle class, to that, the size of the shape can be controled
 *  It inherit the CPSRotated2DParticle class, to that, the orientation of the shape can be controled
 *  Supports texture, but no texture animation
 */

class CPSRibbon : public CPSParticle, public CPSSizedParticle, public CPSColoredParticle, public CPSRotated2DParticle
{
		public:
		/// process one pass for the particle	
		
		virtual void draw(void) ;
	
	
		/** (de)activate color fading
		 * when its done, colors fades to black along the ribbon
		 */
		void setColorFading(bool onOff = true) 
		{ 
			_ColorFading = onOff ; 
			setupColor(_AliveRibbons) ;
		}

		/** When this is set to true, the ribbon light remains after it is 'destroyed'
		 *  The default is false
		 */
		void setPersistAfterDeath(bool persit = true) ;

		/** return true if the ribbon light persist after death 
		 *  \see _PersistAfterDeath()
		 */
		bool getPersistAfterDeath(void) const { return _DyingRibbons != NULL ; }



		/// test wether color fading is activated
		bool getColorFading(void) const { return _ColorFading ; }
		
		/** ctor.
		 * \param nbSegmentInTail the number of segment the ribbon has in its tail
		 *  \param shape pointer to a shape that will be extruded along the ribbon. It must have a unit size
		 *         and be located in the x-y plane. This will be copied
		 *  \param nbPointsInShape : the number of points in the shape
		 */
		CPSRibbon(uint32 nbSegmentInTail = 256
				  , const CVector *shape = Losange
				  , uint32 nbPointsInShape = NbVerticesInLosange) ;


		// dtor
		~CPSRibbon() ;

		// set the number of segments in the tail		
		void setTailNbSeg(uint32 nbSeg) ;

		// get the number of segments in the tail
		uint32 getTailNbSeg(uint32 getNbSeg) const { return _TailNbSeg ; }

		/** set a new shape for the ribbon
		 * If the number of vertices in the shape has changed, the previous ribbon will be desroyed
		 * In fact, their radius will be 0 all along the tail
		 *  \param shape pointer to a shape that will be extruded along the ribbon. It must have a unit size
		 *         and be located in the x-y plane (z can be used for effects) . This will be copied
		 *  \param nbPointsInShape : the number of points in the shape
		 */
		void setShape(const CVector *shape, uint32 nbPointsInShape) ;

		/// get the number of vertice in the shape used for ribbons
		uint32 getNbVerticesInShape(void) const { return _ShapeNbSeg ; }
		
		/** Get a copy of the shape used for ribbon
		 *  \param dest a table of cvector that has the right size, it will be filled with vertices
		 *  \see getNbVerticesInShape()
		 */

		void getShape(CVector *shape, uint32 nbPointsInShape) const ;


		NLMISC_DECLARE_CLASS(CPSRibbon) ;

		///serialisation
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;
		
		/** tells in which basis is the tail
		 *  It requires one transform per particle if it is not the same as the located that hold that particle
		 *  The default is false. With that you can control if a rotation of the system will rotate the tail
		 */
		void setSystemBasis(bool yes) { _SystemBasisEnabled = yes ; }

		
		/// return true if the tails are in the system basis
		bool isInSystemBasis(void) const { return _SystemBasisEnabled ; }


		/// Set a texture. NULL remove it
		void setTexture(CSmartPtr<ITexture> tex, float uFactor = 1.f, float vFactor = 1.f)
		{
			_Tex = tex ;
			_UFactor = uFactor ;
			_VFactor = vFactor ;
			updateMatAndVbForTexture() ;
		}

		/// get the texture used
		ITexture *getTexture(void)
		{
			return _Tex ; 
		}
	

		/// Predefined shape : a regular losange shape
		static const CVector  Losange[] ;
		/// number of vertices in the losange
		static const uint32 NbVerticesInLosange ;

		/// Predefined shape : height sides
		static const CVector  HeightSides[] ;
		/// number of vertices in the height side (must be 8 ... :)  )
		static const uint32 NbVerticesInHeightSide ;

		/// Predifined shape : pentagram
		static const CVector Pentagram[] ;
		static const uint32 NbVerticesInPentagram ;

		


	protected:
		
		void init(void) ;		


		/** used to describe ribbons
		 * we group it in a struct to
		 * separate alive ribbon from 
		 * ribbon that dissapear beacuse the particle has dissapear
		 * this is needed for long ribbons because it could
		 * be shocking to have then disappearing at once
		 * so theyr light remains for a while
		 */

		struct CRibbonsDesc
		{		

			// vertex buffer containing vertices
			CVertexBuffer _Vb ;	

			// an index buffer
			uint32 *_Ib ; 
		
			/** used to memorize the last positions in all ribbon. Position are stored consecutively		 
			 */

			std::vector<CVector> _Pos ;

			// color are stored sequentially for each tail
			std::vector<CRGBA> _ColTab ;
		} ;


		// material for ribbons
		CMaterial _Mat ;

		CRibbonsDesc _AliveRibbons ;
		CRibbonsDesc *_DyingRibbons ;


		// the number of dying ribbons that are present
		uint32 _NbDyingRibbons ;

		// a counter to tell how much frame is left for each ribbon
		std::vector<uint32> _DyingRibbonsLifeLeft ;

		//  texture, if used

		CSmartPtr<ITexture> _Tex ;

		// number of segments in the tail
		uint32 _TailNbSeg ;

		// number of segments in shape
		uint32 _ShapeNbSeg ;

	

		// true if the tails are in the system basis
		bool _SystemBasisEnabled ;

		// for texture use only
		float _UFactor, _VFactor ;

		// the shape that is used
		std::vector<CVector> _Shape ;

		// ratio pos : it is used to determine what is the ratio size between 2 slice in a ribbon
		std::vector<float> _SliceRatioTab ;				
		
		/// true if the tail color must fade to black
		bool _ColorFading ;
							
		
		/** resize and stup Vb (and index buffer also)
		 *  \param rb a reference to the set of ribbons to setup
		 */
		void resizeVb(CRibbonsDesc &rb) ;

		/// setup the initial colors in the whole vb : black or a precomputed gradient for constant color
		void setupColor(CRibbonsDesc &rb) ;

		/// update the material and the vb so that they match the color scheme
		virtual void updateMatAndVbForColor(void) ;

		/// init a ribbon : at start, all vertices have the same pos

		void initRibbonPos(CRibbonsDesc &rb, uint32 index, const CVector &pos) ;


		/// call when a texture has changed
		virtual void updateMatAndVbForTexture(void) ;

		/// setup the uvs
		void setupUV(CRibbonsDesc &rb) ;


		/// recompute slice of the ribbons, but not the last
		void decalRibbons(CRibbonsDesc &rb, const uint32 size) ;	

		/// compute last slice of the ribbons
		void computeLastSlice(CRibbonsDesc &rb, const uint32 size);

		/// render a ribbon desc
		void render(CRibbonsDesc &rb, const uint32 size) ;


		/// init a ribbon desc
		void init(CRibbonsDesc &rb) ;


		/// stup the Vb, when textures are on / off
		void setupVertexFormatNMat(CRibbonsDesc &rb) ;

		/// delete an element in the specified ribbon desc
		void deleteElement(CRibbonsDesc &rb, uint32 index, const uint32 size) ;

		/// duplicate a ribbon between ribbonDesc
		void copyElement(CRibbonsDesc &rbSrc, uint32 srcIndex, CRibbonsDesc &rbDest, uint32 destIndex) ;


		/// Set the max number of Ribbon				
		void resize(uint32 size) ;
		
		/// add a nex ribbon
		void newElement(void) ;
		
		/// delete a ribbon given its index
		void deleteElement(uint32 index) ;
} ;



/**
 * A face particle
 * Unlike FaceLookAt, these particle can have an orientation in space.
 * They are drawn with an angle bias of 45° in theyr local basis (for optimisation purpose)
 *
 *          ^ y
 *          |
 *          0
 *         / \
 *        /   \
 *       3     1--> x
 *        \   /
 *         \ /
 *          2
 * If all particle must rotate the same, but with a rotattionnal bias, a hint can be provided, so that
 * there are batch of particle that share the same orientation. The users must give the number of various phase
 * This is the fastest.
 * Other cases need an attribute maker that produce a couple of vectors
 * , giving the x & y direction of the local basis (plane particle) 
 * 
 */


class CPSFace       : public CPSQuad					
					, public CPSRotated3DPlaneParticle
{

public:

	/** Create the face 
	 *  you can give a non-animated texture here
	 */
	CPSFace(CSmartPtr<ITexture> tex = NULL) ;

	virtual void draw(void) ;

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	NLMISC_DECLARE_CLASS(CPSFace) ;


	/** Tells that all faces are turning in the same manner, and only have a rotationnal bias
	 *  This is faster then other method. Any previous set scheme for 3d rotation is kept.
	 *	\param: the number of rotation configuration we have. The more high it is, the slower it'll be
	 *          If this is too low, a lot of particles will have the same orientation	           	 
	 *          If it is 0, then the hint is disabled
 	 *  \param  minAngularVelocity : the maximum angular velocity for particle rotation	 
	 *  \param  maxAngularVelocity : the maximum angular velocity for particle rotation	 
	 *  \see    CPSRotated3dPlaneParticle
	 */
	void hintRotateTheSame(uint32 nbConfiguration
							, float minAngularVelocity = NLMISC::Pi
							, float maxAngularVelocity = NLMISC::Pi
						  ) ;

	/** disable the hint 'hintRotateTheSame'
	 *  The previous set scheme for roation is used
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */
	void disableHintRotateTheSame(void)
	{
		hintRotateTheSame(0) ;
	}

	/** check wether a call to hintRotateTheSame was performed
	 *  \return 0 if the hint is disabled, the number of configurations else
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */

	uint32 checkHintRotateTheSame(void) const
	{
		return _PrecompBasis.size() ; 
	}


protected:
	
	virtual void newElement(void) ;
	
	
	virtual void deleteElement(uint32 index) ;
	
	virtual void resize(uint32 size) ;
		


	// we must store them for serialization
	float _MinAngularVelocity ;
	float _MaxAngularVelocity ;

	struct CPlaneBasisPair
	{		
		CPlaneBasis Basis ;
		CVector Axis ; // an axis for rotation
		float AngularVelocity ; // an angular velocity
	} ;

	/// a set of precomp basis, before and after transfomation in world space, used if the hint 'RotateTheSame' has been called
	std::vector< CPlaneBasisPair > _PrecompBasis ;

	/// this contain an index in _PrecompBasis for each particle
	std::vector<uint32> _IndexInPrecompBasis ;

	/// fill _IndexInPrecompBasis with index in the range [0.. nb configurations[
	void fillIndexesInPrecompBasis(void) ;

} ;


class CPSShockWave : public CPSParticle, public CPSSizedParticle, public CPSColoredParticle
					, public CPSTexturedParticle, public CPSRotated3DPlaneParticle, public CPSRotated2DParticle
{
public:

	/** ctor
	 *  \param nbSeg : number of seg for the circonference of the shockwave. must be >= 3 and <= 64.
	 *  \param radiusCut : indicate how much to subtract to the outter radius to get the inner radius
	 *  \param  tex : the texture that must be applied to the shockwave
	 */
	CPSShockWave(uint nbSeg = 9, float radiusCut = 0.8f , CSmartPtr<ITexture> tex = NULL) ;

	/** set a new number of seg (mus be >= 3 and <= 64)
	 *  \see CPSShockWave()
	 */
	void setNbSegs(uint nbSeg) ;

	/// retrieve the number of segs
	uint getNbSegs(void) const { return _NbSeg ; }

	/** set a new radius cut
	 *  \see CPSShockWave()
	 */
	void setRadiusCut(float aRatio) ;

	/// get the radius ratio
	float getRadiusCut(void) const { return _RadiusCut ; }


	/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	NLMISC_DECLARE_CLASS(CPSShockWave) ;

	virtual void draw(void) ;


	/// complete the bbox depending on the size of particles
	virtual bool completeBBox(NLMISC::CAABBox &box) const   ;

	
	
protected:
	/// initialisations
	virtual void init(void) ;	


	/** calculate current color and texture coordinate before any rendering
	 *  size can't be higher that shockWaveBufSize ...
	 */
	void updateVbColNUVForRender(uint32 startIndex, uint32 size)	 ;	


	/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void updateMatAndVbForColor(void) ;

	/// update the material and the vb so that they match the texture scheme.
	virtual void updateMatAndVbForTexture(void) ;




	/**	Generate a new element for this bindable. They are generated according to the properties of the class		 
	 */
	virtual void newElement(void)  ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index)  ;

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size)  ;	


	// the number of seg in the shockwave
	uint32 _NbSeg ; 

	// ratio to get the inner circle radius from the outter circle radius
	float _RadiusCut ;


	// material
	CMaterial _Mat ;

	// a vertex buffer
	CVertexBuffer _Vb ;

	// an index buffer
	CPrimitiveBlock _Pb ;


} ;


/** This class is for mesh handling. It operates with any mesh, but it must insert them in the scene...
 *  It is not very adapted for lots of little meshs..
 *  To create the mesh basis, we use CPlaneBasis here. It give us the I and J vector of the basis for each mesh 
 *  and compute K ( K =  I ^ J)
 */

class CPSMesh : public  CPSParticle, public CPSSizedParticle
				, public CPSRotated3DPlaneParticle, public CPSRotated2DParticle
{
public:
	/// construct the system by using the given shape for mesh
	CPSMesh(const std::string &shape = "") : _Invalidated(false)
	{
		_Shape = shape ;
		_Name = std::string("Mesh") ;
	}

	/// set a new shape for that kind of particles
	void setShape(const std::string &shape) { _Shape = shape ; }

	/// get the shape used for thos particles
	
	const std::string &getShape(void) const { return _Shape ; }

		/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	virtual ~CPSMesh() ;

	NLMISC_DECLARE_CLASS(CPSMesh) ;


	/** invalidate the transformShapes that were inserted in the scene, so they need to be rebuilt
	 *  during the next rendering. This is useful for clipping, or when the system has been loaded
	 */

	void invalidate() 
	{ 
		_Invalidated = true ; 
		_Instances.clear() ;
	}

protected:
	/**	Generate a new element for this bindable. They are generated according to the properties of the class		 
	 */
	virtual void newElement(void)  ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index)  ;


	virtual void draw(void) ;

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size)  ;	

	//CSmartPtr<IShape> _Shape ;

	std::string _Shape ;

	// a container for mesh instances
	typedef CPSAttrib<CTransformShape *> TInstanceCont ;

	TInstanceCont _Instances ;

	// this is set to true when the transformed shape have to be recerated

	bool _Invalidated ;
} ; 


/** This class is for mesh that have very simple geometry. The constraint is that they can only have one matrix block. 
 *  They got a hint for constant rotation scheme. With little meshs, this is the best to draw a maximum of them
 */

class CPSConstraintMesh : public  CPSParticle, public CPSSizedParticle
				, public CPSRotated3DPlaneParticle
{
public:	
	CPSConstraintMesh() : _ModelShape(NULL), _ModelVb(NULL), _ModelBank(NULL), _Touched(false)
	{		
		_Name = std::string("ConstraintMesh") ;
	}

	virtual ~CPSConstraintMesh() ;

	/** construct the mesh by using the given mesh shape file	
	 */
	void build(const std::string meshFileName) ;

	/** Tells that all meshs are turning in the same manner, and only have a rotationnal bias
	 *  This is a lot faster then other method. Any previous set scheme for 3d rotation is kept.
	 *	\param: the number of rotation configuration we have. The more high it is, the slower it'll be
	 *          If this is too low, a lot of particles will have the same orientation	           	 
	 *          If it is 0, then the hint is disabled	
 	 *  \param  minAngularVelocity : the maximum angular velocity for particle rotation	 
	 *  \param  maxAngularVelocity : the maximum angular velocity for particle rotation	 
	 *  \see    CPSRotated3dPlaneParticle
	 */
	void hintRotateTheSame(uint32 nbConfiguration
							, float minAngularVelocity = NLMISC::Pi
							, float maxAngularVelocity = NLMISC::Pi
						  ) ;

	/** disable the hint 'hintRotateTheSame'
	 *  The previous set scheme for roation is used
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */
	void disableHintRotateTheSame(void)
	{
		hintRotateTheSame(0) ;
	}

	/** check wether a call to hintRotateTheSame was performed
	 *  \return 0 if the hint is disabled, the number of configurations else
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */

	uint32 checkHintRotateTheSame(void) const
	{
		return _PrecompBasis.size() ; 
	}


		/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream) ;

	
	NLMISC_DECLARE_CLASS(CPSConstraintMesh) ;
	

protected:
	/**	Generate a new element for this bindable. They are generated according to the properties of the class		 
	 */
	virtual void newElement(void)  ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void deleteElement(uint32 index)  ;


	virtual void draw(void) ;

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void resize(uint32 size)  ;	
	

	/// build the mesh data
	void update(void) ;

	

	/// a rendering pass
	struct RdrPass
	{
		CMaterial Mat ;
		CPrimitiveBlock Pb ;
		
	} ;

	// name of the mesh shape  it was generated from
	std::string _MeshShapeFileName ;

	// A new mesh has been set, so we must reconstruct it when needed
	bool _Touched ;

	// the shape bank containing the shape
	CShapeBank  *_ModelBank ;

	//  the shape we're using
	IShape  *_ModelShape ;

	typedef std::vector<RdrPass> TRdrPassVect ;
	// the rendering passes
	TRdrPassVect _RdrPasses ;

	
	//  the only vertex buffer for the model mesh 5if points the vb of the mesh used as a model)
	const CVertexBuffer *_ModelVb ;


	// the vertex buffer for pre-rotated meshs (it is computed before each new mesh is shown)
	// it does only contains position and normal when present
	CVertexBuffer  _PreRotatedMeshVb ;


	/** the vertex buffer for a batch of mesh
	 *  If contains all vertices data, and only position and normal (when used)	 are uspdated
	 *  By using datas from _PreRotatedMeshVb
	 */
	CVertexBuffer _MeshBatchVb ;


	/** the primitive block for a batch of meshs. It is only computed once when build() is called.
	 *  It contains duplication of the original mesh pb
	 */
	CPrimitiveBlock _MeshBatchPb ;


	
	// we must store them for serialization
	float _MinAngularVelocity ;
	float _MaxAngularVelocity ;


	// use for rotation of precomputed meshs
	struct CPlaneBasisPair
	{		
		CPlaneBasis Basis ;
		CVector Axis ; // an axis for rotation
		float AngularVelocity ; // an angular velocity
	} ;

	/// a set of precomp basis, before and after transfomation in world space, used if the hint 'RotateTheSame' has been called
	std::vector< CPlaneBasisPair > _PrecompBasis ;

	/// this contain an index in _PrecompBasis for each particle
	std::vector<uint32> _IndexInPrecompBasis ;

	/// fill _IndexInPrecompBasis with index in the range [0.. nb configurations[
	void fillIndexesInPrecompBasis(void) ;

	/// when pre-rotated mesh are used, this setup the vb and ib used for copy
	void setupPreRotatedVb(sint vertexFlags) ;

	// when a new model mesh is set, this setup the Vb used for drawing
	void setupVb(sint vertexFlags) ;


	// release the model shape (dtor, or before loading)
	void clean(void) ;



} ; 





} // NL3D


#endif // NL_PS_PARTICLE_H

/* End of ps_particle.h */
