/** \file algo.h
 * Some common algorithms.
 *
 * $Id: algo.h,v 1.1 2002/05/30 14:23:39 berenguier Exp $
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


namespace NLMISC 
{

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


} // NLMISC


#endif // NL_ALGO_H

/* End of algo.h */
