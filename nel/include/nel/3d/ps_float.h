/** \file ps_size.h
 * <File description>
 *
 * $Id: ps_float.h,v 1.1 2001/05/02 11:49:50 vizerie Exp $
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
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/tmp/animation_time.h"
#include <algorithm>

namespace NL3D {


/**
 * This functor blend between 2 float
 * It is used by CPSFloatBlend
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
class CPSFloatBlendFunc
{
public:
	/// this produce floats
	float operator()(CAnimationTime time) const
	{
		return time * _FloatRatio + _CstFloat ;		
	}

	/// restrieve the start and end float

	void getFloats(float &startFloat, float &endFloat) const
	{
		startFloat = (*this)(0) ;
		endFloat = (*this)(1) ;
	}	

	/// set the floats

	void setFloats(float startFloat, float endFloat)
	{
		_CstFloat = startFloat ;
		_FloatRatio = endFloat - startFloat ;
	}

	/// ctor
	CPSFloatBlendFunc() {}

	/// serialization
	void serial(NLMISC::IStream &f) ;

	float getMaxFloat(void) const
	{
		return std::max((*this)(0), (*this)(1)) ;
	}

protected:
	float _CstFloat ;
	float _FloatRatio ;
};



/// This is a float blender class
class CPSFloatBlender : public CPSAttribMakerT<float, CPSFloatBlendFunc>
{
public:
	NLMISC_DECLARE_CLASS(CPSFloatBlender) ;
	CPSFloatBlender(float startFloat = 0.1f , float endFloat = 0.0f)
	{
		_F.setFloats(startFloat, endFloat) ;
	}

	virtual float getMaxValue(void) const { return _F.getMaxFloat() ; }

} ;



} // NL3D


#endif // NL_PS_FLOAT_H

/* End of ps_size.h */
