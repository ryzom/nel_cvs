/** \file ps_tail_dot.h
 * Tail dot particles.
 *
 * $Id: ps_tail_dot.h,v 1.2 2002/02/20 11:20:10 vizerie Exp $
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

#ifndef NL_PS_TAIL_DOT_H
#define NL_PS_TAIL_DOT_H

#include "3d/ps_particle_basic.h"
#include "3d/vertex_buffer.h"
#include "3d/primitive_block.h"


namespace NL3D 
{

/**
 *  These particle are like dot, but a tail is following them. The number of segments in the tails can be tuned.
 *  Note that it doesn't derive from CPSRibbonBase, but it may be the case
 *  later... (ribbon base class has been designed to get CPSRibbonLookAt working properly, and was written after this class).
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
	virtual void step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt);		

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

} // NL3D


#endif // NL_PS_TAIL_DOT_H

/* End of ps_taildot.h */
