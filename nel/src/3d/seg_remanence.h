/** \file seg_remanence.h
 * A segment that let a remanence on the screen (for sword trace)
 * $Id: seg_remanence.h,v 1.2 2002/07/04 10:35:39 vizerie Exp $
 */

/* Copyright, 2000, 2001, 2002 Nevrax Ltd.
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


#ifndef NL_SEG_REMANENCE_H
#define NL_SEG_REMANENCE_H

#include "nel/misc/vector.h"
#include "3d/transform_shape.h"
#include "3d/transform.h"
#include "3d/animated_material.h"

#include <deque>


namespace NL3D
{

class IDriver;
class CVertexBuffer;
class CPrimitiveBlock;
class CMaterial;
class CScene;



/** A segment (or a more complicated shape) that let a 'trace' when it is moved.
  * Can be used for fx of a sword stroke for example
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
	
class CSegRemanence : public CTransformShape
{
public:
	///\name object
	//@{		
		CSegRemanence();
		~CSegRemanence();
		CSegRemanence(CSegRemanence &other);		
		CSegRemanence &operator = (CSegRemanence &other);
	//@}
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void			registerBasic();
	/// to instanciate that model from a scene
	static IModel			*creator() { return new CSegRemanence; }	
	// Render this model with currently setupped material and matrix
	void					render(IDriver *drv, CVertexBuffer &vb, CPrimitiveBlock &pb, CMaterial &mat);
	// sample current position
	void					samplePos(float date);
	/** Setup from the shape (no effect if geometry described in the shape didn't change)	  
	  */ 
	void					setupFromShape();


	///\name From CTransformShape
	//@{
		virtual bool				canStartStop() { return true; }
		// start the fx (by default it is off).
		virtual void				start();
		/// Stop the fx and let it unroll		   
		virtual void				stop();
		/// Stop the fx with no unrolling
		virtual void				stopNoUnroll();
		// Test wether the fx is started
		virtual bool				isStarted() const { return _Started; }
		// Test if the fx is stopping (unrollinh)
		bool						isStopping() const { return _Stopping; }
		// Equivalent to a call to start, then stop
		void						restart();
	//@}

	/** Set an animated material. This is usually called by the 'shape' of this object at instanciation
	  * The material is owned by this object
	  */
	void					setAnimatedMaterial(CAnimatedMaterial *mat);
	//
	CAnimatedMaterial		*getAnimatedMaterial() const { return _AniMat; }	

	// Register to a channel mixer.
	void					registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);

	enum	TAnimValues
	{
		OwnerBit= CTransformShape::AnimValueLast, 

		AnimValueLast,
	};
	
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
private:	
	struct CSampledPos
	{
		CSampledPos(const NLMISC::CVector &pos = NLMISC::CVector::Null, float date = 0.f) : Pos(pos), SamplingDate(date)
		{
		}
		NLMISC::CVector Pos;
		float			SamplingDate;		
	};
	class CRibbon
	{
		public:
			CRibbon();
			void setNumSlices(uint numSlices);
			void samplePos(const NLMISC::CVector &pos, float date, float sliceDuration);
			void fillVB(uint8 *dest, uint stride, uint nbSegs, float sliceTime);
			void duplicateFirstPos();
		private:
			typedef std::deque<CSampledPos> TSampledPosVect;
			TSampledPosVect	_Ribbon;
			float			_LastSamplingDate;
	};	
	typedef std::vector<CRibbon> TRibbonVect;
private:	
	TRibbonVect			_Ribbons; // sampled positions at each extremities of segment
	uint				_NumSlice;
	uint				_NumCorners;
	bool				_Started;
	bool				_Stopping; // true if the effect is unrolling
	bool				_Restarted;
	float				_StartDate;
	float				_CurrDate;
	float				_UnrollRatio;	
	CAnimatedMaterial   *_AniMat;	
private:
	void		updateOpacityFromShape();
	void		copyFromOther(CSegRemanence &other);
public:
	// for anim detail obs
	void clearAnimatedMatFlag()
	{
		IAnimatable::clearFlag(OwnerBit);
	}
};

/** Hierarchical observer for seg remanence. It sample extremities of the segment in world space
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CSegRemanenceHrcObs : public CTransformHrcObs
{
public:
	CSegRemanenceHrcObs();
	virtual	void traverse (IObs *caller);
	static IObs	*creator () { return new CSegRemanenceHrcObs; }	
private:
	uint64				_LastSampleFrame;
};

/** Detail ani observer (for texture animation)
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CSegRemanenceAnimDetailObs : public CTransformAnimDetailObs
{
public:

	/** this do :
	 *  - call CTransformAnimDetailObs::traverse()
	 *  - update animated material if any
	 */
	virtual	void	traverse(IObs *caller);


public:
	static IObs	*creator() {return new CSegRemanenceAnimDetailObs;}
};




}

#endif


