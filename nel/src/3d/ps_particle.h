/** \file ps_particle.h
 * <File description>
 *
 * $Id: ps_particle.h,v 1.21 2002/01/29 13:34:05 vizerie Exp $
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

#include "3d/ps_particle_basic.h"
#include "3d/ps_util.h"
#include "3d/vertex_buffer.h"
#include "3d/primitive_block.h"
#include "3d/mesh.h"
#include "3d/scene.h"


namespace NL3D {


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *	this is just a coloured dot
 */

class CPSDot : public CPSParticle, public CPSColoredParticle, public CPSMaterial
{
public:					
	/// ctor
	CPSDot() { init(); _Name = std::string("point");}

	NLMISC_DECLARE_CLASS(CPSDot);

	///serialisation
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getMaxNumFaces(void) const;

	/// init the vertex buffers
	static void initVertexBuffers();

protected:				
	virtual void draw(bool opaque);
	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	void	init(void);				
	static CVertexBuffer _DotVb;
	static CVertexBuffer _DotVbColor;

	/// update the material and the vb so that they match the color scheme
	virtual void updateMatAndVbForColor(void);
	
	/** Set the nax number of dot		
	*/
	void resize(uint32 size);

	/// we don't save datas so it does nothing for now
	void newElement(CPSLocated *emitterLocated, uint32 emitterIndex);

	/// we don't save datas so it does nothing for now
	void deleteElement(uint32);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** This abstract class holds what is needed with quad particles (CPSFaceLookAt, CPSFace) e.g 
 *  Index and vertex buffer and method to setup them
 *  Material, and method to setup them 
 */

 class CPSQuad : public CPSParticle
	           , public CPSColoredParticle
			   , public CPSTexturedParticle
			   , public CPSMultiTexturedParticle
			   , public CPSSizedParticle
			   , public CPSMaterial
 {
public:
	/** create the quad by giving a texture. This can't be a CTextureGrouped (for animation)
	* animation must be set later by using setTextureScheme
	*/
	CPSQuad(CSmartPtr<ITexture> tex = NULL);


	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);		

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getMaxNumFaces(void) const;

	/// init the vertex buffers
	static void initVertexBuffers();

protected:		
	// dtor
	virtual ~CPSQuad();

	/// initialisations
	virtual void init(void);	

	/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void updateMatAndVbForColor(void);

	/// update the material and the vb so that they match the texture scheme.
	virtual void updateMatAndVbForTexture(void);

	/// this is inlined to save cost of call by derived class
	void newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
	{
		newColorElement(emitterLocated, emitterIndex);
		newSizeElement(emitterLocated, emitterIndex);
		newTextureIndexElement(emitterLocated, emitterIndex);
	}

	/// this is inlined to save cost of call by derived class
	void deleteElement(uint32 index)
	{
		deleteColorElement(index);
		deleteSizeElement(index);
		deleteTextureIndexElement(index);
	}

	void resize(uint32 capacity);

	/// complete the bbox depending on the size of particles
	virtual bool completeBBox(NLMISC::CAABBox &box) const;

	/** calculate current color and texture coordinate before any rendering
	 *  size can't be higher that quadBufSize ...
	 */
	void updateVbColNUVForRender(CVertexBuffer &vb, uint32 startIndex, uint32 numQuad);

	/// DERIVERS MUST CALL this		 
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);	
	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getTextureIndexOwner(void) { return _Owner; }
	
	enum VBType
	{
		VBCol			= 0x0001, // the vb has colors
		VBTex			= 0x0002, // the vb has textures coordinates
		VBTexAnimated	= 0x0004, // the texture coordinate can be animated (not precomputed)
		VBTex2		    = 0x0008, // the vb has second texture coordinates 
		VBTex2Animated  = 0x0010, // the second texture coordinates can be animated (not precomputed)
	};

	/// the various kind of vertex buffers we need
	static CVertexBuffer _VBPos;
	static CVertexBuffer _VBPosCol;
	static CVertexBuffer _VBPosTex1;
	static CVertexBuffer _VBPosTex1Col;				
	static CVertexBuffer _VBPosTex1Anim;
	static CVertexBuffer _VBPosTex1AnimCol;
	//==========
	static CVertexBuffer _VBPosTex1Tex2;
	static CVertexBuffer _VBPosTex1ColTex2;	
	static CVertexBuffer _VBPosTex1AnimTex2;
	static CVertexBuffer _VBPosTex1AnimColTex2;
	//==========
	static CVertexBuffer _VBPosTex1Tex2Anim;
	static CVertexBuffer _VBPosTex1ColTex2Anim;	
	static CVertexBuffer _VBPosTex1AnimTex2Anim;
	static CVertexBuffer _VBPosTex1AnimColTex2Anim;

	/// get the vertex buffer that is needed for drawing
	CVertexBuffer &getNeededVB();

	/// setup the texture shaders 2d matrix usef for EMBM to the identity (when needed)
	void	setupTexShaderMatrix(IDriver *drv);

	/// used to get a pointer on the right vb dependant on its type (cf. values of VBType)
	static CVertexBuffer    * const CPSQuad::_VbTab[];
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * A FaceLookAt particle
 *  These particles can have 2 different size (width and height) when activated
 */


class CPSFaceLookAt :   public CPSQuad
					  , public CPSRotated2DParticle
					
{
public:
	/** create the face look at by giving a texture. This can't be a CTextureGrouped (for animation)
     * animation must be set later by using setTextureScheme
	 */
	CPSFaceLookAt(CSmartPtr<ITexture> tex = NULL);
	
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
	NLMISC_DECLARE_CLASS(CPSFaceLookAt);
			
	/** activate 'fake' motion blur (its done by deforming the quad)
	 *  This is slower, however. This has no effect with rotated particles.
	 *  \param coeff a coefficient for motion blur (too high value may give unrealistic result)
	 *         0 deactivate the motion blur
	 *  \param threshold : speed will be clamped below this value	 
	 */
	void activateMotionBlur(float coeff = 1.f, float threshold = 1.f)
	{
		_MotionBlurCoeff = coeff;
		_Threshold = threshold;
	}

	/// set the motion blur coeff (0 = none)
	void setMotionBlurCoeff(float coeff) { _MotionBlurCoeff = coeff; }

	/// set the motion blur threshold
	void setMotionBlurThreshold(float threshold) { _Threshold = threshold; }



	/** return the motion blur coeff (0.f means none)
	 *  \see  activateMotionBlur()
	 */
	float getMotionBlurCoeff(void) const { return _MotionBlurCoeff; }

	/// get the motion blur threshold
	float getMotionBlurThreshold(void) const { return _Threshold; }

	/** Setting this to true allows to have independant height and width for these particles.
	  * The interface to manage the second size can be obtained from getSecondSize(), which correspond to the height of particles.
	  * The default is to not have independant sizes
	  */
	void setIndependantSizes(bool enable  = true) { _IndependantSizes = enable; }

	/// test wether independant sizes are activated
	bool hasIndependantSizes(void) const { return _IndependantSizes; }

	/// retrieve an interface to set the second size
	CPSSizedParticle &getSecondSize(void) 
	{ 
		nlassert(_IndependantSizes);
		return _SecondSize;
	}

	/// retrieve an interface to set the second size const version
	const CPSSizedParticle &getSecondSize(void) const
	{ 
		nlassert(_IndependantSizes);
		return _SecondSize;
	}
	
protected:

	float					_MotionBlurCoeff;	
	float					_Threshold; // motion blur theshold
	// in this struct we defines the getSizeOwner method, which is abstract in the CPSSizedParticle clas
	struct					CSecondSize : public CPSSizedParticle
	{
		CPSFaceLookAt *Owner;
		virtual CPSLocated *getSizeOwner(void) { return Owner->getOwner(); }
	} _SecondSize;
	bool					_IndependantSizes;		
	virtual void			draw(bool opaque);
	void					newElement(CPSLocated *emitterLocated, uint32 emitterIndex);	
	void					deleteElement(uint32);
	void					resize(uint32);
	virtual CPSLocated *getAngle2DOwner(void) { return _Owner; }

};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * A fan light particle
 */
class CPSFanLight : public CPSParticle, public CPSColoredParticle
				  , public CPSSizedParticle, public CPSRotated2DParticle
				  , public CPSMaterial, public CPSTexturedParticleNoAnim
{
public:	
	NLMISC_DECLARE_CLASS(CPSFanLight);

	virtual bool		completeBBox(NLMISC::CAABBox &box) const  ;

	///\name Object
	//@{
	/// Ctor, with the numbers of fans to draw (minimum is 3, maximum is 128)
	CPSFanLight(uint32 nbFans = 7);
	/// Dtor
	~CPSFanLight();
	void				serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	//@}

	// Set the number of fans used for drawing (minimum is 3, maximum is 128)
	void				setNbFans(uint32 nbFans);

	/** Set the smoothness of phases. The default is 0 which means no smoothness.
	  * n mean that the phase will be linearly interpolated between each n + 1 fans
	  * It ranges from 0 to 31
	  */
	void				setPhaseSmoothness(uint32 smoothNess) 
	{ 
		nlassert(smoothNess < 32);
		_PhaseSmoothness = smoothNess; 
	}

	/// retrieve the phase smoothness
	uint32				getPhaseSmoothness(void) const { return _PhaseSmoothness;}

	/// set the intensity of fan movement. Default is 1.5
	void				setMoveIntensity(float intensity) { _MoveIntensity = intensity; }

	/// get the intensity of fans movement
	float				getMoveIntensity(void) const      { return _MoveIntensity; }

	// Get the number of fans used for drawing
	uint32				getNbFans(void) const
	{
		return _NbFans;
	}

	/** Set the speed for phase
	 *	If the located holding this particle as a limited lifetime, it gives how many 0-2Pi cycle it'll do during its life
	 *  Otherwise it gives how many cycle there are in a second
	 */
	void				setPhaseSpeed(float multiplier);

	/// get the speed for phase
	float				getPhaseSpeed(void) const { return _PhaseSpeed / 256.0f; }

	// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void		updateMatAndVbForColor(void);


	/// must call this at least if you intend to use fanlight
	static void			initFanLightPrecalc(void);
	
	/// return true if there are transparent faces in the object
	virtual bool		hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool		hasOpaqueFaces(void);


	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32		getMaxNumFaces(void) const;

	/// Set a texture. NULL remove it
	void setTexture(CSmartPtr<ITexture> tex)
	{
		_Tex = tex;		
		updateMatAndVbForColor();
	}
	
	/// get the texture used
	ITexture *getTexture(void)
	{
		return _Tex; 
	}
	const ITexture *getTexture(void) const
	{
		return _Tex; 
	}
protected:
	
	/// initialisations
	virtual void init(void);

	uint32						_NbFans;
	uint32						_PhaseSmoothness;
	CVertexBuffer				_Vb;
	float						_MoveIntensity;	
	uint32						*_IndexBuffer;
	NLMISC::CSmartPtr<ITexture> _Tex;
	static uint8				_RandomPhaseTab[32][128];		
	float						_PhaseSpeed;

	//#ifdef NL_DEBUG		
		static bool _RandomPhaseTabInitialized;
	//#endif
	
	virtual void		draw(bool opaque);
	void				newElement(CPSLocated *emitterLocated, uint32 emitterIndex);
	void				deleteElement(uint32);

	/// Set the max number of fanlights		
	virtual void resize(uint32 size); 

	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getAngle2DOwner(void) { return _Owner; }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *  These particle are like dot, but a tail is following them. The number of line in the tails can be tuned
 */

class CPSTailDot : public CPSParticle, public CPSColoredParticle
				 , public CPSTailParticle, public CPSMaterial
{
public:			
	/** (de)activate color fading
	 * when its done, colors fades to black along the tail
	 */
	void setColorFading(bool onOff = true) 
	{ 
		_ColorFading = onOff; 
		setupColor();
	}

	/// test wether color fading is activated
	bool getColorFading(void) const { return _ColorFading; }
	
	/// ctor. It tells how many segments there are in the tail. 255 is the maximum
	CPSTailDot(uint32 nbSegmentInTail = 4);

	/** set the number of segments in the tail		
	 * 255 is the maximum
	 */
	void setTailNbSeg(uint32 nbSeg);

	// get the number of segments in the tail
	uint32 getTailNbSeg(void) const { return _TailNbSeg; }

	NLMISC_DECLARE_CLASS(CPSTailDot);

	///serialisation
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
	/** tells in which basis is the tail
	 *  It requires one transform per particle if it is not the same as the located that hold that particle
	 *  The default is false. With that you can control if a rotation of the system will rotate the tail
	 */
	void setSystemBasis(bool yes) { _SystemBasisEnabled = yes; }

	
	/// return true if the tails are in the system basis
	bool isInSystemBasis(void) const { return _SystemBasisEnabled; }

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getMaxNumFaces(void) const;

protected:
	/// process one pass for the particle			
	virtual void step(TPSProcessPass pass, TAnimationTime ellapsedTime);		

	/// draw (but don't animate) particles
	virtual void draw(bool opaque);		
	
	void init(void);		
	
	CVertexBuffer _Vb;	
	// a set of lines to draw
	CPrimitiveBlock _Pb; 

	// number of segments in the tail
	uint32 _TailNbSeg;

	
	// true if the tail is in the system basis, false otherwise
	bool _SystemBasisEnabled;
	
	/// true if the tail color must fade to black
	bool _ColorFading;
								
	// resize the vertex buffer to keep old datas
	void CPSTailDot::resizeVb(uint32 oldTailNbSeg, uint32 size);

	// setup the initial colors in the whole vb : black or a precomputed gradient for constant color
	void setupColor(void);

	virtual CPSLocated *getColorOwner(void) { return _Owner; }

	/// update the material and the vb so that they match the color scheme
	virtual void updateMatAndVbForColor(void);

	/// Set the max number of TailDot				
	void resize(uint32 size);		
	void newElement(CPSLocated *emitterLocated, uint32 emitterIndex);		
	void deleteElement(uint32 index);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 'Ribbon' particle : a shape is extruded while it follows the particle, thus
 *  giving a ribbon effect
 *  It inherit the CPSSizedParticle class, to that, the size of the shape can be controled
 *  It inherit the CPSRotated2DParticle class, to that, the orientation of the shape can be controled
 *  Supports texture, but no texture animation
 */

class CPSRibbon : public CPSParticle, public CPSSizedParticle
			    , public CPSColoredParticle, public CPSRotated2DParticle
				, public CPSTailParticle, public CPSMaterial, public CPSTexturedParticleNoAnim
{
public:
	NLMISC_DECLARE_CLASS(CPSRibbon);

	virtual bool hasParticles(void) const 
	{ nlassert(_Owner); return _Owner->getSize() || _NbDyingRibbons; }
			
	/** (de)activate color fading
	 * when its done, colors fades to black along the ribbon
	 */
	void setColorFading(bool onOff = true) 
	{ 
		_ColorFading = onOff; 
		setupColor(_AliveRibbons);
	}

	/** When this is set to true, the ribbon light remains after it is 'destroyed'
	 *  The default is false
	 */
	void setPersistAfterDeath(bool persit = true);

	/** return true if the ribbon light persist after death 
	 *  \see _PersistAfterDeath()
	 */
	bool getPersistAfterDeath(void) const { return _DyingRibbons != NULL; }



	/// test wether color fading is activated
	bool getColorFading(void) const { return _ColorFading; }
	
	/** ctor.
	 * \param nbSegmentInTail the number of segment the ribbon has in its tail
	 *  \param shape pointer to a shape that will be extruded along the ribbon. It must have a unit size
	 *         and be located in the x-y plane. This will be copied
	 *  \param nbPointsInShape : the number of points in the shape
	 */
	CPSRibbon(uint32 nbSegmentInTail = 256
			  , const CVector *shape = Losange
			  , uint32 nbPointsInShape = NbVerticesInLosange);

	// dtor
	~CPSRibbon();

	// set the number of segments in the tail		
	void setTailNbSeg(uint32 nbSeg);

	// get the number of segments in the tail
	uint32 getTailNbSeg(void) const { return _TailNbSeg; }

	/** set a new shape for the ribbon
	 * If the number of vertices in the shape has changed, the previous ribbon will be desroyed
	 * In fact, their radius will be 0 all along the tail
	 *  \param shape pointer to a shape that will be extruded along the ribbon. It must have a unit size
	 *         and be located in the x-y plane (z can be used for effects) . This will be copied
	 *  \param nbPointsInShape : the number of points in the shape
	 */
	void setShape(const CVector *shape, uint32 nbPointsInShape);

	/// get the number of vertice in the shape used for ribbons
	uint32 getNbVerticesInShape(void) const { return _ShapeNbSeg; }
	
	/** Get a copy of the shape used for ribbon
	 *  \param dest a table of cvector that has the right size, it will be filled with vertices
	 *  \see getNbVerticesInShape()
	 */
	void getShape(CVector *shape) const;		

	///serialisation
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	
	/** tells in which basis is the tail
	 *  It requires one transform per particle if it is not the same as the located that hold that particle
	 *  The default is false. With that you can control if a rotation of the system will rotate the tail
	 */
	void setSystemBasis(bool yes) { _SystemBasisEnabled = yes; }

	
	/// return true if the tails are in the system basis
	bool isInSystemBasis(void) const { return _SystemBasisEnabled; }

	/// set a texture
	void setTexture(CSmartPtr<ITexture> tex)
	{
		_Tex = tex;
		updateMatAndVbForTexture();
	}
	/// Set texture factors.
	void setTexFactor(float uFactor = 1.f, float vFactor = 1.f)
	{			
		_UFactor = uFactor;
		_VFactor = vFactor;
		updateMatAndVbForTexture();
	}

	// get the u-factor for texture mapping
	float getUFactor(void) const { return _UFactor; }

	// get the v-factor for texture mapping
	float getVFactor(void) const { return _VFactor; }

	/// get the texture used
	ITexture *getTexture(void)
	{
		return _Tex; 
	}
	const ITexture *getTexture(void) const
	{
		return _Tex; 
	}

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);


	/// Predefined shape : a regular losange shape
	static const CVector  Losange[];
	/// number of vertices in the losange
	static const uint32 NbVerticesInLosange;

	/// Predefined shape : height sides
	static const CVector  HeightSides[];
	/// number of vertices in the height side (must be 8 ... :)  )
	static const uint32 NbVerticesInHeightSide;

	/// Predifined shape : pentagram
	static const CVector Pentagram[];
	static const uint32 NbVerticesInPentagram;

	/// Predifined shape : triangle
	static const CVector Triangle[];
	static const uint32 NbVerticesInTriangle;				

	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getMaxNumFaces(void) const;

protected:	
	virtual void step(TPSProcessPass pass, TAnimationTime ellapsedTime);
	void draw(bool opaque);		
	void init(void);		
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
		CVertexBuffer _Vb;	

		// an index buffer
		std::vector<uint32> _Ib; 
	
		/** used to memorize the last positions in all ribbon. Position are stored consecutively		 
		 */

		std::vector<CVector> _Pos;

		// color are stored sequentially for each trail
		std::vector<CRGBA> _ColTab;
	};
	CRibbonsDesc _AliveRibbons;
	CRibbonsDesc *_DyingRibbons;

	// the number of dying ribbons that are present
	uint32 _NbDyingRibbons;

	// a counter to tell how much frame is left for each ribbon
	std::vector<uint32> _DyingRibbonsLifeLeft;

	//  texture, if used
	CSmartPtr<ITexture> _Tex;

	// number of segments in the tail
	uint32 _TailNbSeg;

	// number of segments in shape
	uint32 _ShapeNbSeg;

	// true if the tails are in the system basis
	bool _SystemBasisEnabled;

	// for texture use only
	float _UFactor, _VFactor;

	// the shape that is used
	std::vector<CVector> _Shape;

	// ratio pos : it is used to determine what is the ratio size between 2 slice in a ribbon
	std::vector<float> _SliceRatioTab;				
	
	/// true if the tail color must fade to black
	bool _ColorFading;
								
	/** resize and stup Vb (and index buffer also)
	 *  \param rb a reference to the set of ribbons to setup
	 */
	void resizeVb(CRibbonsDesc &rb);

	/// setup the initial colors in the whole vb : black or a precomputed gradient for constant color
	void setupColor(CRibbonsDesc &rb);

	/// update the material and the vb so that they match the color scheme
	virtual void updateMatAndVbForColor(void);

	/// init a ribbon : at start, all vertices have the same pos
	void initRibbonPos(CRibbonsDesc &rb, uint32 index, const CVector &pos);

	/// call when a texture has changed
	virtual void updateMatAndVbForTexture(void);

	/// setup the uvs
	void setupUV(CRibbonsDesc &rb);

	/// recompute slice of the ribbons, but not the last
	void decalRibbons(CRibbonsDesc &rb, const uint32 size);	

	/// compute last slice of the ribbons
	void computeLastSlice(CRibbonsDesc &rb, const uint32 size);

	/// render a ribbon desc
	void render(CRibbonsDesc &rb, const uint32 size);

	/// init a ribbon desc
	void init(CRibbonsDesc &rb);

	/// stup the Vb, when textures are on / off
	void setupVertexFormatNMat(CRibbonsDesc &rb);

	/// delete an element in the specified ribbon desc
	void deleteElement(CRibbonsDesc &rb, uint32 index, const uint32 size);

	/// duplicate a ribbon between ribbonDesc
	void copyElement(CRibbonsDesc &rbSrc, uint32 srcIndex, CRibbonsDesc &rbDest, uint32 destIndex);

	/// Set the max number of Ribbon				
	void resize(uint32 size);
	
	/// add a nex ribbon
	void newElement(CPSLocated *emitterLocated, uint32 emitterIndex);
	
	/// delete a ribbon given its index
	void deleteElement(uint32 index);

	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getAngle2DOwner(void) { return _Owner; }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
					, public CPSHintParticleRotateTheSame
{
public:
	/** Create the face 
	 *  you can give a non-animated texture here
	 */
	CPSFace(CSmartPtr<ITexture> tex = NULL);

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	NLMISC_DECLARE_CLASS(CPSFace);
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
						  );

	/** disable the hint 'hintRotateTheSame'
	 *  The previous set scheme for roation is used
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */
	void disableHintRotateTheSame(void)
	{
		hintRotateTheSame(0);
	}

	/** check wether a call to hintRotateTheSame was performed
	 *  \return 0 if the hint is disabled, the number of configurations else
	 *  \see hintRotateTheSame(), CPSRotated3dPlaneParticle
	 */

	uint32 checkHintRotateTheSame(float &min, float &max) const
	{
		min = _MinAngularVelocity;
		max = _MaxAngularVelocity;
		return _PrecompBasis.size(); 
	}


protected:
	virtual void step(TPSProcessPass pass, TAnimationTime ellapsedTime);	
	virtual void	newElement(CPSLocated *emitterLocated, uint32 emitterIndex);	
	virtual void	deleteElement(uint32 index);
	virtual void	resize(uint32 size);
	/// fill _IndexInPrecompBasis with index in the range [0.. nb configurations[
	void fillIndexesInPrecompBasis(void);
	virtual CPSLocated *getPlaneBasisOwner(void) { return _Owner; }
		
	// we must store them for serialization
	float			_MinAngularVelocity;
	float			_MaxAngularVelocity;
	struct CPlaneBasisPair
	{		
		CPlaneBasis Basis;
		CVector Axis; // an axis for rotation
		float AngularVelocity; // an angular velocity
	};

	/// a set of precomp basis, before and after transfomation in world space, used if the hint 'RotateTheSame' has been called
	std::vector< CPlaneBasisPair > _PrecompBasis;

	/// this contain an index in _PrecompBasis for each particle
	std::vector<uint32> _IndexInPrecompBasis;	
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CPSShockWave : public CPSParticle, public CPSSizedParticle
					, public CPSColoredParticle, public CPSTexturedParticle
					, public CPSRotated3DPlaneParticle, public CPSRotated2DParticle
					, public CPSMaterial
{
public:
	/** ctor
	 *  \param nbSeg : number of seg for the circonference of the shockwave. must be >= 3 and <= 64.
	 *  \param radiusCut : indicate how much to subtract to the outter radius to get the inner radius
	 *  \param  tex : the texture that must be applied to the shockwave
	 */
	CPSShockWave(uint nbSeg = 9, float radiusCut = 0.8f , CSmartPtr<ITexture> tex = NULL);

	/** set a new number of seg (mus be >= 3 and <= 64)
	 *  \see CPSShockWave()
	 */
	void setNbSegs(uint nbSeg);

	/// retrieve the number of segs
	uint getNbSegs(void) const { return _NbSeg; }

	/** set a new radius cut
	 *  \see CPSShockWave()
	 */
	void setRadiusCut(float aRatio);

	/// get the radius ratio
	float getRadiusCut(void) const { return _RadiusCut; }


	/// serialisation. Derivers must override this, and call their parent version
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	NLMISC_DECLARE_CLASS(CPSShockWave);

	/// complete the bbox depending on the size of particles
	virtual bool completeBBox(NLMISC::CAABBox &box) const  ;

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);
	
	/// return the max number of faces needed for display. This is needed for LOD balancing
	virtual uint32 getMaxNumFaces(void) const;

	/// set the U factor for textures
	void			setUFactor(float value);

	/// get the U factor for textures
	float			getUFactor(void) const { return _UFactor; }

protected:
	virtual void draw(bool opaque);

	/// initialisations
	virtual void	init(void);	

	/** calculate current color and texture coordinate before any rendering
	 *  size can't be higher that shockWaveBufSize ...
	 */
	void			updateVbColNUVForRender(uint32 startIndex, uint32 size)	;	

	/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void	updateMatAndVbForColor(void);

	/// update the material and the vb so that they match the texture scheme.
	virtual void	updateMatAndVbForTexture(void);

	/**	Generate a new element for this bindable. They are generated according to the properties of the class		 
	 */
	virtual void	newElement(CPSLocated *emitterLocated, uint32 emitterIndex) ;
	
	/** Delete an element given its index
	 *  Attributes of the located that hold this bindable are still accessible for the index given
	 *  index out of range -> nl_assert
	 */
	virtual void	deleteElement(uint32 index) ;

	/** Resize the bindable attributes containers. Size is the max number of element to be contained. DERIVERS MUST CALL THEIR PARENT VERSION
	 * should not be called directly. Call CPSLocated::resize instead
	 */
	virtual void	resize(uint32 size) ;	

	// the number of seg in the shockwave
	uint32 _NbSeg; 

	// ratio to get the inner circle radius from the outter circle radius
	float _RadiusCut;

	// a vertex buffer
	CVertexBuffer _Vb;

	// an index buffer
	CPrimitiveBlock _Pb;

	// texture factor
	float		 _UFactor;

	virtual CPSLocated *getColorOwner(void) { return _Owner; }
	virtual CPSLocated *getSizeOwner(void) { return _Owner; }
	virtual CPSLocated *getAngle2DOwner(void) { return _Owner; }
	virtual CPSLocated *getPlaneBasisOwner(void) { return _Owner; }
	virtual CPSLocated *getTextureIndexOwner(void) { return _Owner; }
};




} // NL3D


#endif // NL_PS_PARTICLE_H

/* End of ps_particle.h */
