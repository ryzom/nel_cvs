/** \file common.h
 * <File description>
 *
 * $Id: common.h,v 1.2 2000/10/23 13:54:22 cado Exp $
 */

/** Copyright, 2000 Nevrax Ltd.
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

#ifndef	_COMMON_H_
#define	_COMMON_H_


#include "nel/misc/types_nl.h"
#include <stdlib.h>


namespace	NLMISC
{


// ============================================================================================
inline float	frand(float mod)
{
	double	r=rand();

	r/= RAND_MAX;

	return (float)(r*mod);
}


// ============================================================================================
template<class T, class U, class V>	inline void clamp(T &v, const U &min, const V &max)
{
	v= v<min? min: v;
	v= v>max? max: v;
}


// ============================================================================================
#ifndef SQR
#define	SQR(_x) ((_x)*(_x))
#endif

// ============================================================================================
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

// ============================================================================================
// MIN/MAX extended functions.

template<class T>	inline T minof(T a,  T b,  T c)
	{return min(min(a,b),c);}
template<class T>	inline T minof(T a,  T b,  T c,  T d)
	{return min(minof(a,b,c),d);}
template<class T>	inline T minof(T a,  T b,  T c,  T d,  T e)
	{return min(minof(a,b,c,d),e);}
template<class T>	inline T maxof(T a,  T b,  T c)
	{return max(max(a,b),c);}
template<class T>	inline T maxof(T a,  T b,  T c,  T d)
	{return max(maxof(a,b,c),d);}
template<class T>	inline T maxof(T a,  T b,  T c,  T d,  T e)
	{return max(maxof(a,b,c,d),e);}


}


#endif