/** \file enumeration.h
 *
 * $Id: enumeration.h,v 1.1 2004/12/01 15:31:16 vizerie Exp $
 */

/* Copyright, 2000-2004 Nevrax Ltd.
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


#ifndef NL_ENUMERATION_H
#define NL_ENUMERATION_H


namespace NLMISC
{


// Enumeration of the values of a Map of type T  
template <class T>
class CMapValueEnumeration 
{
public:
	// Build an enumeration on the map 'map'
	CMapEnumeration(T &map) : _Map(map), _Iter(map.begin()) {}
	/** Return next available value in the enumeration. Availability should have been checked with 'hasNext' or an assertion is raised.	  
	  */
	typename T::value_type &next() { typename T::value_type &val = _Iter->second; ++_Iter; return val; }
	// Check that a new value is available for retrieval with next(). Returns false if the enumeration is finished
	bool					hasNext() const { nlassert(_Iter != _Map.end()); }
private:
	typename T::iterator _Iter;
	T                   &_Map;

};


// Enumeration of the values of a Map of type T  (const version)
template <class T>
class CMapValueEnumerationConst
{
public:
	// Build an enumeration on the map 'map'
	CMapEnumeration(const T &map) : _Map(map), _Iter(map.begin()) {}
	/** Return next available value in the enumeration. Availability should have been checked with 'hasNext' or an assertion is raised.	  
	  */
	typename const T::value_type &next() { typename const T::value_type &val = _Iter->second; ++_Iter; return val; }
	// Check that a new value is available for retrieval with next(). Returns false if the enumeration is finished
	bool					hasNext() const { nlassert(_Iter != _Map.end()); }
private:
	typename T::const_iterator _Iter;
	const    T                 &_Map;

};




}









#endif
