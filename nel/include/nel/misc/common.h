/** \file common.h
 * common algorithms, constants and functions
 *
 * $Id: common.h,v 1.11 2000/11/10 11:18:51 lecroart Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef	NL_COMMON_H
#define	NL_COMMON_H


#include "nel/misc/types_nl.h"
#include <stdlib.h>
#include <algorithm>

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
template<class T>	inline T sqr(const T &v)
{
	return v*v;
}


// ============================================================================================
const	double	Pi= 3.1415926535897932384626433832795;

/* il faut trouver comment detecter stlport...
#ifdef NL_OS_WINDOWS
namespace std
{
#ifdef min
#undef min
#endif // min
template <class _Tp>
inline const _Tp& min(const _Tp& __a, const _Tp& __b) {
  return __b < __a ? __b : __a;
}

#ifdef max
#undef max
#endif // max
template <class _Tp>
inline const _Tp& max(const _Tp& __a, const _Tp& __b) {
  return  __a < __b ? __b : __a;
}
}
#endif // NL_OS_WINDOWS
*/

// ============================================================================================
// Force v to be inside the interval [min,max]
template<class T, class U, class V>	inline void clamp(T &v, const U &min, const V &max)
{
	v = (v < min) ? min : v;
	v = (v > max) ? max : v;
}

// ============================================================================================
// MIN/MAX extended functions.

template<class T>	inline T minof(const T& a,  const T& b,  const T& c)
	{return std::min(std::min(a,b),c);}
template<class T>	inline T minof(const T& a,  const T& b,  const T& c,  const T& d)
	{return std::min(minof(a,b,c),d);}
template<class T>	inline T minof(const T& a,  const T& b,  const T& c,  const T& d,  const T& e)
	{return std::min(minof(a,b,c,d),e);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c)
	{return std::max(std::max(a,b),c);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c,  const T& d)
	{return std::max(maxof(a,b,c),d);}
template<class T>	inline T maxof(const T& a,  const T& b,  const T& c,  const T& d,  const T& e)
	{return std::max(maxof(a,b,c,d),e);}

// ============================================================================================
/** 
  * contReset take a container like std::vector or std::deque and put his size to 0 like clear but free all buffers.
  * This function is usefull because resize, clear, erase or reserve methods never realloc when the array size come down.
  * \param a is the container to reset.
  */
template<class T>	inline contReset (T& a)
{
	a.~T();
	new (&a) T;
}

// ============================================================================================
/// Return the value maximized to the next power of 2 of v.   eg: raiseToNextPowerOf2(8)==8.  raiseToNextPowerOf2(5)==8.
inline uint	raiseToNextPowerOf2(uint v)
{
	uint	res=1;
	while(res<v)
		res<<=1;
	
	return res;
}

/// Return the power of 2 of v.   eg: getPowerOf2(8)==3.  getPowerOf2(5)==3.
inline uint	getPowerOf2(uint v)
{
	uint	res=1;
	uint	ret=0;
	while(res<v)
	{
		ret++;
		res<<=1;
	}
	
	return ret;
}

}	// NLMISC


#endif	// NL_COMMON_H