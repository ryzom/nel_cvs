/** \file ps_particle2.h
 * Some more particles
 *
 * $Id: ps_particle2.h,v 1.3 2002/01/28 14:37:43 vizerie Exp $
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
#include "3d/primitive_block.h"


namespace NL3D {

/** Base class for ribbons. If can be used to compute ribbons trajectory. 
  * It can perform lagrange or linear interpolation.
  * to get the ribbon shape. It can also be used to have fixed size ribbons.
  * NB : Ribbons that don't herit from this are deprecated but may be kept for compatibility.
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CPSRibbonBase : public CPSParticle, public CPSTailParticle
{
public:		
	enum TRibbonMode		{ VariableSize = 0, FixedSize, RibbonModeLast };
	enum TInterpolationMode { Linear = 0, Hermitte, InterpModeLast };

	///\name Object
	///@{
		CPSRibbonBase();
		/// serialisation. Derivers must override this, and call their parent version
		virtual void			serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	///@}

	///\name Behaviour
	///@{
		/// NB : a fixed size isn't applied with parametric motion.
		void					setRibbonMode(TRibbonMode mode);
		TRibbonMode				getRibbonMode() const { return _RibbonMode; }
		void					setInterpolationMode(TInterpolationMode mode);
		TInterpolationMode		getInterpolationMode() const { return _InterpolationMode; }
	///@}	

	///\name Geometry
	///@{
		/// set the number of segments used with this particle. In this case, it can't be lower than 2
		void				setTailNbSeg(uint32 nbSegs);
		/// get the number of segments used with this particle
		uint32				getTailNbSeg(void) const { return _NbSegs; }
		/** Set how many seconds need a seg to be traversed. Long times will create longer ribbons. Default is 0.02.
		  * It gives the sampling rate for each type of ribbon
		  */
		void				setSegDuration(TAnimationTime ellapsedTime);		
		TAnimationTime		getSegDuration(void) const { return _SegDuration; }

		/** The the length in meter of the ribbon. This is used only if the ribbon mode is set to FixedSize.
		* These kind of ribbon are usually slower than variable size ribbons.
		* The default is one metter.
		*/
		void					setRibbonLength(float length);
		float			        getRibbonLength() const { return _RibbonLength; }
	///@}
		
	/** Allow degradation of ribbons with distance of the system (may not be suited when theit paths have wicked angles)
	  * \param percent 1 mean no degradation, 0 mean nothing will be draw when the system is at its max dist. 1 is the default
	  */
	void					setLODDegradation(float percent)
	{ 
		nlassert(percent >= 0 && percent <= 1)
		_LODDegradation = percent;
	}
	float					getLODDegradation() const { return _LODDegradation; }

protected:
	uint32							  _NbSegs;
	TAnimationTime					  _SegDuration;
	bool							  _Parametric; // if this is set to true, then the owner has activated parametric motion.	
	
	/// inherited from CPSLocatedBindable
	virtual void					newElement(CPSLocated *emitterLocated, uint32 emitterIndex) ;
	/// inherited from CPSLocatedBindable
	virtual void					deleteElement(uint32 index);
	/// inherited from CPSLocatedBindable	
	virtual void					resize(uint32 size);
	/// called when the motion type has changed, this allow us to draw smoother ribbons when parametric anim is used
	virtual	void					motionTypeChanged(bool parametric);
	
	/** Get position of the i-th ribbon and store them in a table of vector.	  
	  * It uses the interpolation setting of this object. 
	  * The dest tab must have at least nbSegs + 1 entries.
	  */
	void							computeRibbon( uint index,
												   NLMISC::CVector *dest,
												   uint stride = sizeof(NLMISC::CVector)
												  );

	/// Called each time the time of the system change in order to update the ribbons positions
	void							updateGlobals();

	/// must be called for the lod to apply (updates UsedNbSegs)
	void                            updateLOD();

	
	/// value to use after lod computation
	uint32							  _UsedNbSegs;
	TAnimationTime					  _UsedSegDuration;
	float							  _UsedSegLength;

private:
	typedef std::vector<NLMISC::CVector> TPosVect;
	typedef	std::vector<float>			 TFloatVect; // all positions for each ribbons packed in a single vector

	TPosVect						  _Ribbons;	
	TFloatVect					      _SamplingDate;
	uint							  _RibbonIndex;  // indicate which is the first index for the ribbons head	
	TAnimationTime					  _LastUpdateDate;	
	TRibbonMode						  _RibbonMode;
	TInterpolationMode				  _InterpolationMode;
	float							  _RibbonLength; // used if _RibbonMode == FixedSize
	float							  _SegLength;
	float							  _LODDegradation;

	



	void					initDateVect();
	void					resetSingleRibbon(uint index, const NLMISC::CVector &pos);
	void					resetFromOwner();	

	/// copy datas from one ribbon to another
	void					dupRibbon(uint dest, uint src);	

	/// Compute the ribbon points using linear interpolation between each sampling point.
	void					computeLinearRibbon( uint index,
											     NLMISC::CVector *dest,
										         uint stride = sizeof(NLMISC::CVector)
										       );
	/// The same as compute linear ribbon but try to make its length constant
	void					computeLinearCstSizeRibbon( uint index,
											     NLMISC::CVector *dest,
										         uint stride = sizeof(NLMISC::CVector)
										       );
	/// Compute the ribbon points using hermitte splines between each sampling point.
	void					computeHermitteRibbon( uint index,
											     NLMISC::CVector *dest,
										         uint stride = sizeof(NLMISC::CVector)
										       );

	/** Compute the ribbon points using hermitte splines between each sampling point,
	  * and make a rough approximation to get a constant lenght
	  */
	void					computeHermitteCstSizeRibbon( uint index,
											     NLMISC::CVector *dest,
										         uint stride = sizeof(NLMISC::CVector)
										       );
};


/** A ribbon look at particle. It is like a ribbon, but textured (with no animation), and it always faces the user
  */
class CPSRibbonLookAt : public  CPSRibbonBase, public CPSSizedParticle, public CPSColoredParticle,
						public  CPSMaterial, public CPSTexturedParticleNoAnim
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

	///\name Behaviour
	///@{
			/** (de)activate color fading
			* when its done, colors fades to black along the tail.
			* NOT SUPPORTED FOR NOW
			*/
			virtual void setColorFading(bool onOff = true) {}

			/** Test wether color fading is activated.
			  * NOT SUPPORTED FOR NOW
			  */
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
	virtual void			step(TPSProcessPass pass, TAnimationTime ellapsedTime);

	/// return true if there are transparent faces in the object
	virtual bool			hasTransparentFaces(void);

	/// return true if there are Opaque faces in the object
	virtual bool			hasOpaqueFaces(void);

	virtual uint32			getMaxNumFaces(void) const;

protected:				
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


private:	

	/// update the material and the vb so that they match the color scheme. Inherited from CPSColoredParticle
	virtual void					updateMatAndVbForColor(void);	

	/// display a set of ribbons
	void							displayRibbons(uint32 nbRibbons);

	/**\name Vertex buffers & their corresponding index buffers. We keep a map of pretextured vertex buffer (with or without colors).
	  * Vb for ribbons that have the same size are shared.
	  */
		
	//@{
			/// a struct containing a vertex buffer and a primitive block
			struct CVBnPB
			{
				CVertexBuffer		VB;
				CPrimitiveBlock		PB;
			};
			typedef std::map<uint, CVBnPB> TVBMap;

			static TVBMap					_VBMap;			  // index buffers with no color
			static TVBMap					_ColoredVBMap;    // index buffer + colors			

			/// get a vertex buffer and a primitive suited for the current ribbon
			CVBnPB &getVBnPB();

			/// get the number of ribbons contained in a vb for a given length. (e.g the number of ribbons that can be batched)
			uint	getNumRibbonsInVB() const;
	//@}		
};



} // NL3D


#endif // NL_PS_PARTICLE_H

/* End of ps_particle.h */
