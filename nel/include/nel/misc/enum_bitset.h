/** \file misc/enum_bitset.h
 * utility class to build bit field from enum value vithout need
 * to do explicit cast by hands.
 *
 * $Id: enum_bitset.h,v 1.2 2005/02/23 09:19:00 guignot Exp $
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


#ifndef NL_ENUM_BIYSET_H
#define NL_ENUM_BIYSET_H

#include "types_nl.h"

namespace NLMISC
{

/** Utility to build 'ored' bit set from a 2 powered enum.
 *	The class give to user a conprehensive interface for
 *	dealing with 'ored' enum value.
 *	
 *	The class not strictly check that the enum only contains
 *	power of 2 values because the enum can eventualy 
 *	contains pre 'ored' values.
 *	For each access, the size in bit of the enumerated value
 *	passed to the class is checked to not oversize the
 *	BitsetType capacity.
 *	By default, the BitsetType is set to 32 bits, but you
 *	can provide your own type to narrow or expand the 
 *	capacity.
 *	
 *	usage:
 *	enum foo
 *	{
 *		value1 = 1,
 *		value2 = 2,
 *		value3 = 4,
 *		value4 = 8
 *	};
 *
 *	CEnumBitSet<foo>	myset;
 *	myset.addEnumValue(value1);
 *	myset.addEnumValue(value4);
 *
 *	myset.checkEnumValue(value1);	// return true
 *	myset.checkEnumValue(value2);	// return false
 */
template <class EnumType, typename BitsetType = uint32, BitsetType maxValue = UINT_MAX>
struct CEnumBitset
{
	// Default contructor with no flag set
	CEnumBitset()
		: Bitset(0)
	{
	}

	// Constructor with one flag set
	CEnumBitset(EnumType value)
		: Bitset(0)
	{
		addEnumValue(value);
	}

	/// Add an ent
	void addEnumValue(EnumType value)
	{
		nlassert(value < maxValue);
		Bitset |= value;
	}

	bool checkEnumValue(EnumType value)
	{
		return (Bitset & BitsetType(value)) == BitsetType(value);
	}

	BitsetType		Bitset;	
};


} // namespace NLMISC


#endif // NL_ENUM_BIYSET_H
