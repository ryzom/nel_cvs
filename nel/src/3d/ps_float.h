/** \file ps_size.h
 * <File description>
 *
 * $Id: ps_float.h,v 1.6 2001/09/13 14:26:19 vizerie Exp $
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

#ifndef NL_PS_FLOAT_H
#define NL_PS_FLOAT_H

#include "nel/misc/types_nl.h"
#include "3d/ps_attrib_maker_template.h"
#include "3d/ps_attrib_maker_bin_op.h"
#include "nel/3d/animation_time.h"
#include <algorithm>
#include <nel/misc/vector_h.h>

namespace NL3D {


const char *CPSAttribMaker<float>::getType() { return "float"; }

/// these are some attribute makers for float
/// This is a float blender class. It just blend between 2 values

class CPSFloatBlender : public CPSValueBlender<float>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatBlender);
	CPSFloatBlender(float startFloat = 0.1f , float endFloat = 1.f, float nbCycles = 1.0f) : CPSValueBlender<float>(nbCycles)
	{
		_F.setValues(startFloat, endFloat);
	}
	CPSAttribMakerBase *clone() const { return new CPSFloatBlender(*this); }
	// F is serialized by base classes...

};


/// This is a float gradient class
class CPSFloatGradient : public CPSValueGradient<float>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatGradient);

	/**	
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSFloatGradient(const float *floatTab = CPSFloatGradient::_DefaultGradient
						, uint32 nbValues = 2, uint32 nbStages = 16, float nbCycles = 1.0f);

	
	static float _DefaultGradient[];
	CPSAttribMakerBase *clone() const { return new CPSFloatGradient(*this); }
	// F is serialized by base classes...	
};

/** this memorize float by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSFloatMemory : public CPSAttribMakerMemory<float>
{
public:
	CPSFloatMemory() { setDefaultValue(0.f); }
	NLMISC_DECLARE_CLASS(CPSFloatMemory);
	CPSAttribMakerBase *clone() const { return new CPSFloatMemory(*this); }

};

/** An attribute maker whose output if the result of a binary op on floats
  *
  */
class CPSFloatBinOp : public CPSAttribMakerBinOp<float>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatBinOp);
	CPSAttribMakerBase *clone() const { return new CPSFloatBinOp(*this); }
};

/// this functor produce float based on a hermite curb
class CPSFloatLagrangeFunctor
{
	public:
		/// ctor. The default is a cst function whose value is 1
		CPSFloatLagrangeFunctor();

		/** set a control point. The first and last controls point dates must be 0 and 1
		  * \param index renge from 0 to 3
		  */
		void							setControlPoint(uint index, float date, float value);

		/** get a control point.
		  * \return a <date, value> std::pair
		  */
		const std::pair<float, float>  &getControlPoint(uint index) const;

		/// set the number of samples used with this curb
		void							setNumSamples(uint32 numSamples);

		/// get the numer of samples used with this curb
		uint32							getNumSamples(void) const;

		/** This return a sampled value from the hermite curb. The more steps there are, the more accurate it is
		  * You can also get an 'exact value'.
		  * This must be called between beginFastFloor() and endFastFloor() statements
		  * \see getValue
		  */
		#ifdef NL_OS_WINDOWS
			__forceinline
		#endif
		float operator()(CAnimationTime time) const
		{
			return _Tab[OptFastFloor(time * _NumSamples)];
		}

		/// compute an exact value at the given date, which must be in [0, 1[
		float							getValue(float date) const;

		/// serialization
		void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	protected:
		/// update the value tab
		void							updateTab(void);

		/// coeff of the interpolator poly must be recalculated
		void							touchCoeffs(void) const { _CoeffsTouched = true; }

		// recompute the polynom coefficients
		void							updateCoeffs(void) const;

		std::pair<float, float>		_CtrlPoints[4];
		uint32						_NumSamples;
		std::vector<float>			_Tab;
		mutable bool				_CoeffsTouched;
		/// the polynom used for interpolation
		mutable NLMISC::CVectorH	_Coeffs;
};


class CPSFloatLagrange : public CPSAttribMakerT<float, CPSFloatLagrangeFunctor>
{
public:
	CPSFloatLagrange() : CPSAttribMakerT<float, CPSFloatLagrangeFunctor>(1) {}
	NLMISC_DECLARE_CLASS(CPSFloatLagrange);
	CPSAttribMakerBase *clone() const { return new CPSFloatLagrange(*this); }
};



} // NL3D


#endif // NL_PS_FLOAT_H

/* End of ps_size.h */
