/** \file ps_ribbon.h
 * Ribbons particles.
 *
 * $Id: ps_ribbon.h,v 1.2 2002/02/20 11:19:37 vizerie Exp $
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

#ifndef NL_PS_RIBBON_H
#define NL_PS_RIBBON_H

#include "3d/ps_particle_basic.h"
#include "3d/vertex_buffer.h"

namespace NL3D 
{


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 'Ribbon' particle : a shape is extruded while it follows the particle, thus
 *  giving a ribbon effect. Note that it doesn't derive from CPSRibbonBase, but it may be the case
 *  later... (ribbon base class has been designed to get CPSRibbonLookAt working properly, and was written after this class).
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
	virtual void step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt);
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



} // NL3D


#endif // NL_PS_RIBBON_H

/* End of ps_ribbon.h */
