/** \file random.h
 * Simple random generator
 * $Id: random.h,v 1.1 2002/11/14 17:38:46 vizerie Exp $
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
	CRandom();
	// generate a random value
	sint32  rand();
	// set a new seed for the random generator
	void   srand(sint32 seed);
	// generate a floating point random value in [0, mod]
	float  frand(float mod);
private:
	sint32 _Seed;
};




}

#endif


