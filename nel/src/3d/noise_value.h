/** \file noise_value.h
 * <File description>
 *
 * $Id: noise_value.h,v 1.1 2001/10/31 10:19:40 berenguier Exp $
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

#ifndef NL_NOISE_VALUE_H
#define NL_NOISE_VALUE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/stream.h"
#include "nel/misc/rgba.h"


namespace NL3D 
{


using NLMISC::CVector;


// ***************************************************************************
/**
 * A class which generate noisy value, according to a position
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CNoiseValue
{
public:
	float	Abs;
	float	Rand;
	float	Frequency;

public:
	/// Default to 0, 1, 1.
	CNoiseValue();
	CNoiseValue(float abs, float rand, float freq);

	/// return Abs + Rand* noise(Pos*Frequency).  with noise() E [0..1]
	float	eval(const CVector &posInWorld) const;

	/// same as eval, but eval just one random level for noise() => act much more like a random.
	float	evalOneLevelRandom(const CVector &posInWorld) const;


	void	serial(NLMISC::IStream &f);

// *******************
private:
	/// pos scale is in [0..1]
	float	noise(const CVector &pos) const;
	float	evalRandom(const CVector &pos) const;
};


// ***************************************************************************
/**
 * a gradient of color
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CColorGradient
{
public:
	NLMISC::CRGBAF		Col0, Col1;

	CColorGradient() {}
	CColorGradient(NLMISC::CRGBAF col0, NLMISC::CRGBAF col1) : Col0(col0), Col1(col1) {}

public:
	/// factor==0 => Col0, factor==1 => Col1. no clamp.
	void	interpolate(float factor, NLMISC::CRGBAF &result) const;

	void	serial(NLMISC::IStream &f);
};


// ***************************************************************************
/**
 * A noisy color generator
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class	CNoiseColorGradient
{
public:
	/// Abs and Rand should be 0 and 1 here. If not, some colors may not be generated...
	CNoiseValue					NoiseValue;
	std::vector<CColorGradient>	Gradients;


public:
	/** Use NoiseValue to compute a PerlinNoise E [0..1], and peek in Gradients, with linear interpolation.
	 *	result unmodified if no gradients.
	 */
	void	eval(const CVector &posInWorld, NLMISC::CRGBAF &result) const;

	void	serial(NLMISC::IStream &f);
};



} // NL3D


#endif // NL_NOISE_VALUE_H

/* End of noise_value.h */
