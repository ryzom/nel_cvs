/** \file ps_particle.h
 * <File description>
 *
 * $Id: ps_particle2.h,v 1.1 2001/09/26 17:45:09 vizerie Exp $
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

#ifndef NL_PS_PARTICLE2_H
#define NL_PS_PARTICLE2_H


#include "3d/ps_particle_basic.h"
#include "3d/vertex_buffer.h"

namespace NL3D {



/** A ribbon look at particle. It is like a ribbon, but textured (with no animation), and it always faces the user
  */
class CPSRibbonLookAt : public  CPSParticle, public CPSSizedParticle, public CPSColoredParticle,
						public  CPSMaterial, public CPSTailParticle, public CPSTexturedParticleNoAnim
{
public:	
	///\name Object
	///@{
		/// ctor
		CPSRibbonLookAt();
		/// dtor
		~CPSRibbonLookAt();
		/// serialisation. Derivers must override this, and call their parent version
		virtual void		serial(NLMISC::IStream &f) throw(NLMISC::EStream);
		//
		NLMISC_DECLARE_CLASS(CPSRibbonLookAt);
	///@}

	///\name Texture
	///@{
		/// set the texture for this particle
		void						setTexture(CSmartPtr<ITexture> tex);
		/// get the texture used for this particle
		ITexture *getTexture(void) { return _Tex;}
		const ITexture		*getTexture(void) const { return _Tex;}
	///@}

	///\name Geometry
	///@{
		/// set the number of segments used with this particle. In this case, it can't be lower than 2
		void				setTailNbSeg(uint32 nbSegs);
		/// get the number of segments used with this particle
		uint32				getTailNbSeg(void) const { return _NbSegs; }
		/// set how many seconds need a seg to be traversed. Long times will create lonegr ribbons. Default is 0.02
		void				setSegDuration(CAnimationTime ellapsedTime);
		/// set how many seconds need a seg to be traversed. Long times will create lonegr ribbons.
		CAnimationTime		getSegDuration(void) const;
	///@}

	///\name Behaviour
	///@{
			/** (de)activate color fading
			* when its done, colors fades to black along the tail
			*/
			virtual void setColorFading(bool onOff = true) {}

			/// test wether color fading is activated
			virtual bool getColorFading(void) const { return false; }

			/** tells in which basis is the tail
			*  It requires one transform per particle if it is not the same as the located that hold that particle
	  	    *  The default is false. With that you can control if a rotation of the system will rotate the tail
			*/
			virtual void setSystemBasis(bool yes) {}
		
			/// return true if the tails are in the system basis
			virtual bool isInSystemBasis(void) const { return true; }
		
		//void setPersistAfterDeath(bool persit = true);

		/** return true if the ribbon light persist after death 
		 *  \see _PersistAfterDeath()
		 */
		//bool getPersistAfterDeath(void) const { return _DyingRibbons != NULL; }
		
	///@}

	/// inherited from CPSParticle
	virtual void			step(TPSProcessPass pass, CAnimationTime ellapsedTime);

	/// return true if there are transparent faces in the object
	virtual bool hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool hasOpaqueFaces(void);

	virtual uint32 getMaxNumFaces(void) const;

protected:
	uint32									_NbSegs;
	CAnimationTime							_SegDuration;
	bool									_Parametric; // if this is set to true, then the owner has activated parametric motion, 
														 // so we don't need to memorize previous position

	//////////
	typedef std::vector<NLMISC::CVector>	TPointVect;
	struct CRibbons 
	{		
		TPointVect					    _Pts;		// several arrays of vectors packed in a single one. Contains previous pos for each ribbons
		std::vector<CAnimationTime>		_Times;     // ellapsed time for each ribbon		
		void reset(void) 
		{ 
			NLMISC::contReset(_Pts);
			NLMISC::contReset(_Times);
		}
	};		
	CRibbons						_Ribbons;
	CRibbons						*_DyingRibbons;
	uint32							_NbDyingRibbons;
	/////////


	CVertexBuffer					_VB;		// the vb used to draw the ribbons
	std::vector<uint32>				_IB;		// an index buffer, should be removed when strips are availables	
	CSmartPtr<ITexture>				_Tex;

	// the number of dying ribbons that are present
	//uint32							_NbDyingRibbons;
	// a counter to tell how much frame is left for each ribbon
	//std::vector<uint32>				_DyingRibbonsLifeLeft;
	/// inherited from CPSLocatedBindable
	virtual void					newElement(CPSLocated *emitterLocated, uint32 emitterIndex) ;
	/// inherited from CPSLocatedBindable
	virtual void					deleteElement(uint32 index);
	/// inherited from CPSLocatedBindable	
	virtual void					resize(uint32 size);
	virtual CPSLocated				*getSizeOwner(void) { return _Owner; }
	virtual CPSLocated				*getColorOwner(void) { return _Owner; }	
	/** resize and setup Vb (and index buffer also)	 
	 */
	void resizeVb(void);
	/// setup the initial colors in the whole vb : black or a precomputed gradient for constant color
	void setupColor();
	/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void					updateMatAndVbForColor(void);

	/// copy one ribbon to another one
	void							dup(CRibbons &src, CRibbons &dest, uint32 srcIndex, uint32 dstIndex);
	/// resize ribbons
	void							resizeRibbons(CRibbons &r, uint32 size);
	/// performs alive ribbons motion
	void							performMotion(CAnimationTime ellapsedTime);
	/** Perform motion of one ribbon portion by decaling of the given amount.
	  * \param alpha The amount to move one point toward the next in the list.
	  * \param ribbonIndex The index of the ribbon that is being modified.
	  * \param startIndex  First point of the ribbon to change
	  * \param numPoints   Number of points to move. As the next point is needed, numPoints + 1 position are read
	  */
	void							performSubMotion(float alpha, uint32 ribbonIndex, uint32 startIndex, uint32 numPoints); 

	/// display a set of ribbons
	void							displayRibbons(CRibbons &r, uint32 nbRibbons);
	/// init one ribbon at the given pos. At startup, all points of the ribbons are at the same pos
	void							initRibbon(CRibbons &r, uint32 index, const NLMISC::CVector &pos, const NLMISC::CVector &speed);
	/// create ribbon with a null lenght from the positions given by the owner
	void							reinitFromOwner(void);
	/// dump a ribbon content, for debug purpose
	void							dumpRibbon(uint32 index);

	/// called when the motion type has changed, this allow us to draw smoother ribbons when parametric anim is used
	virtual	void	    motionTypeChanged(bool parametric);
};



} // NL3D


#endif // NL_PS_PARTICLE_H

/* End of ps_particle.h */
