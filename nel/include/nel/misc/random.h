/** \file random.h
 * Simple random generator
 * $Id: random.h,v 1.3 2005/08/29 16:12:12 boucher Exp $
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


#ifndef NL_RANDOM_H
#define NL_RANDOM_H

#include "types_nl.h"

namespace NLMISC
{

/** A simple, os-independant random generator.  
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CRandom
{
public:
	// The max value that can be returned by the random generator
	enum { RandMax = 0x7fff };
public:
	// ctor
	inline CRandom();
	// generate a random value in [0, RandMax]
	inline sint32  rand();
	// generate a random value in [0, mod]
	inline sint32  rand(uint16 mod);
	// generate a floating point random value in [0, mod]
	inline float  frand(double mod=1.0f);
	// generate a random value in [-mod, mod]
	inline sint32  randPlusMinus(uint16 mod);
	// generate a random value in [-mod, mod]
	inline float  frandPlusMinus(double mod=1.0);
	// set a new seed for the random generator
	inline void   srand(sint32 seed);
private:
	sint32 _Seed;
};


//===========================================================================
inline CRandom::CRandom() : _Seed(1)
{
}

//===========================================================================
// NB : In fact this random generator has the same behaviour than the VC6 one
inline sint32 CRandom::rand()
{
	return ((_Seed = _Seed * 214013L + 2531011L) >> 16) & RandMax;	
}

//===========================================================================
inline sint32 CRandom::rand(uint16 mod)
{
	sint32 m=mod;
	return rand()*(m+1)/(sint32(RandMax)+1);
}

//===========================================================================
inline sint32 CRandom::randPlusMinus(uint16 mod)
{
	sint32 m=mod;
	return m - rand()*(2*m+1)/(sint32(RandMax)+1);
}

//===========================================================================
inline float CRandom::frand(double mod)
{
	double	r = (double) rand();
	r /= (double) RandMax;
	return (float)(r * mod);	
}

//===========================================================================
inline float CRandom::frandPlusMinus(double mod)
{
	return frand(2*mod)-(float)mod;
}

//===========================================================================
inline void CRandom::srand(sint32 seed)
{
	_Seed = seed;
}



}

#endif


