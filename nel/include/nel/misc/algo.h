/** \file algo.h
 * Some common algorithms.
 *
 * $Id: algo.h,v 1.7 2003/02/26 16:45:13 legros Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_ALGO_H
#define NL_ALGO_H

#include "nel/misc/types_nl.h"
#include <vector>
#include <string>


namespace NLMISC 
{

// ***************************************************************************
/** bilinear of 4 values
  *  v3    v2
  *  +-----+
  *  |     |
  *  |     |
  *  +-----+
  *  v0    v1
  *
  *
  *  T
  *  ^
  *  |
  *  |
  *  +---> S  
  */
template <class T, class U>
T computeBilinear(const T &v0, const T &v1, const T &v2, const T &v3, const U &s, const U &t)
{
	T h0 = t * v3 + ((U) 1 - t) * v0;
	T h1 = t * v2 + ((U) 1 - t) * v1;
	return s * h1 + ((U) 1 - s) * h0;
}

// ***************************************************************************
/** Select all points crossed by the line [(x0,y0) ; (x1,y1)] 
 *  Not the same than brensenham
 */
void drawFullLine (float x0, float y0, float x1, float y1, std::vector<std::pair<sint, sint> > &result);

// ***************************************************************************
/** Select points on the line [(x0,y0) ; (x1,y1)]
 */
void drawLine (float x0, float y0, float x1, float y1, std::vector<std::pair<sint, sint> > &result);


// ***************************************************************************
/**	Search the lower_bound in a sorted array of Value, in growing order (0, 1, 2....).
 *	operator<= is used to perform the comparison.
 *	It return the first element such that array[id]<=key
 *	If not possible, 0 is returned
 *	NB: but 0 may still be a good value, so you must check wether or not 0 means "Not found", or "Id 0".
 */
template<class T>
uint		searchLowerBound(const T *array, uint arraySize, const T &key)
{
	uint	start=0;
	uint	end= arraySize;
	// find lower_bound by dichotomy
	while(end-1>start)
	{
		uint	pivot= (end+start)/2;
		// return the lower_bound, ie return first start with array[pivot]<=key
		if(array[pivot] <= key)
			start= pivot;
		else
			end= pivot;
	}

	return start;
}


// ***************************************************************************
/**	Search the lower_bound in a sorted array of Value, in growing order (0, 1, 2....).
 *	operator<= is used to perform the comparison.
 *	It return the first element such that array[id]<=key
 *	If not possible, 0 is returned
 *	NB: but 0 may still be a good value, so you must check wether or not 0 means "Not found", or "Id 0".
 */
template<class T>
uint		searchLowerBound(const std::vector<T> &array, const T &key)
{
	uint	size= array.size();
	if(size==0)
		return 0;
	else
		return searchLowerBound(&array[0], size, key);
}


// ***************************************************************************
/** Clamp a sint in 0..255. Avoid cond jump.
 */
static inline	void fastClamp8(sint &v)
{
#ifdef NL_OS_WINDOWS
	// clamp v in 0..255 (no cond jmp)
	__asm
	{
		mov		esi, v
		mov		eax, [esi]
		mov		ebx, eax
		// clamp to 0.
		add		eax, 0x80000000
		sbb		ecx, ecx
		not		ecx
		and		ebx, ecx
		// clamp to 255.
		add		eax, 0x7FFFFF00
		sbb		ecx, ecx
		and		ebx, 255
		and		ecx, 255
		or		ebx, ecx
		// store
		mov		[esi], ebx
	}
#else
	clamp(v, 0, 255);
#endif
}


// ***************************************************************************
/** return true if the string strIn verify the wildcard string wildCard.
 *	eg: 
 *		testWildCard("azert", "*")== true
 *		testWildCard("azert", "??er*")== true
 *		testWildCard("azert", "*er*")== true
 *		testWildCard("azert", "azert*")== true
 *	Undefined result if s has some '*', 
 *	return false if wildcard has some "**" or "*?"
 *	NB: case-sensitive
 */
bool		testWildCard(const char *strIn, const char *wildCard);


// ***************************************************************************
/** From a string with some separator, build a vector of string.
 *	eg: splitString("hello|bye|||bee", "|", list) return 3 string into list: "hello", "bye" and "bee".
 */
void		splitString(const std::string &str, const std::string &separator, std::vector<std::string> &retList);


} // NLMISC


#endif // NL_ALGO_H

/* End of algo.h */
