/** \file random.cpp
 *
 * $Id: random.cpp,v 1.1 2002/11/14 17:38:55 vizerie Exp $
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

#include "stdmisc.h"
#include "nel/misc/random.h"

namespace NLMISC
{

//===========================================================================
CRandom::CRandom() : _Seed(1)
{
}

//===========================================================================
// NB : In fact this random generator has the same behaviour than the VC6 one
sint32 CRandom::rand()
{
	return ((_Seed = _Seed * 214013L + 2531011L) >> 16) & RandMax;	
}

//===========================================================================
void CRandom::srand(sint32 seed)
{
	_Seed = seed;
}

//===========================================================================
float CRandom::frand(float mod)
{
	double	r = (double) rand();
	r /= (double) RandMax;
	return (float)(r * mod);	
}

}