/** \file string_mapper.cpp
 *
 * $Id: string_mapper.h,v 1.3 2003/03/04 13:12:54 corvazier Exp $
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

#ifndef STRING_MAPPER_H
#define STRING_MAPPER_H

#include "nel/misc/types_nl.h"
#include <hash_map>
#include <map>
#include <vector>

namespace NLMISC
{

#if defined(NL_DEBUG)
class CStringMapper;
/** This class is a debug version for string indentifier.
 *	In debug mode, the string identifier include the string value.
 *	This greatly facilitate debug of stuff using mapped string.
 *	
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2003
 */
struct TStringId
{
	/// Default constructor. Do nothing
	TStringId()
	{
	}

	/// Constructor from an uint value. You need to specifie the mapper to use or NULL for the global one
	inline TStringId(uint stringId, CStringMapper *mapper = NULL);

	/// Conversion to uint
	operator uint() const
	{
		return _StringId;
	}

	/// comparison with uint
	bool operator ==(uint value) const
	{
		return _StringId == value;
	}

	bool operator != (uint value) const
	{
		return !(operator==(value));
	}

	bool operator == (const TStringId &other) const
	{
		return _StringId == other._StringId;
	}
	
	bool operator <(const TStringId &other) const
	{
		return _StringId < other._StringId;
	}

private:

	friend class CStringMapper;

	// A special constructor used by the string mapper only.
	TStringId(uint stringId, const std::string &stringValue)
		: _StringId(stringId),
		_StringValue(stringValue)
	{}
	/// The integer string mapping.
	uint			_StringId;
	/** The mapped string value.
	 *	NB : Do not give access in any manner to this value !
	 *	It is here ONLY for debug purpose (in fact, to be read during debug).
	*/
	std::string		_StringValue;
};

}// namespace NLMISC
// declare a hash function for the TStringId type. This is to have the same
// behavior betrween the debug and release version of TStringId.
#if __SGI_STL_PORT <= 0x400
__STL_BEGIN_NAMESPACE
__STL_TEMPLATE_NULL struct hash<NLMISC::TStringId>
{
  size_t operator()(NLMISC::TStringId stringId) const { return uint(stringId); }
};
__STL_END_NAMESPACE
#else
_STLP_BEGIN_NAMESPACE
_STLP_TEMPLATE_NULL struct hash<NLMISC::TStringId>
{
	size_t operator()(const NLMISC::TStringId &stringId) const { return uint(stringId); }
};
_STLP_END_NAMESPACE
#endif // __SGI_STL_PORT > 0x400

namespace NLMISC
{
#else
/// In release mode, TStringId is just a typedef on uint. Operations on TStringId are very light.
typedef uint TStringId;
#endif

/** A static class that map string to integer and vice-versa
 * Each different string is tranformed into an unique integer identifier.
 * If the same string is submited twice, the same id is returned.
 * The class can also return the string associated with an id.
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2003
 */
class CStringMapper
{
	struct CStringRef
	{
		const std::string		*String;

		CStringRef(const std::string *str)
			: String(str)
		{}

		bool operator ==(const CStringRef &other) const
		{
			return *String == *(other.String);
		}

		bool operator < (const CStringRef &other) const
		{
			return *String < *(other.String);
		}
	};

/*	struct CHashStringRef : public std::unary_function<CStringRef, size_t>
	{
		size_t operator() (const CStringRef &stringRef) const
		{
			std::hash<std::string>	hasher;

			return hasher(*stringRef.String);
		}
	};
*/
//	typedef std::hash_map<CStringRef, uint, CHashStringRef> TStringRefHashMap;
	typedef std::map<CStringRef, uint>				TStringRefMap;
	
	/// The map stringRef to id
	TStringRefMap								_StringMap;
	/// The linear storage for reverse mapping (id to string)
	std::vector<CStringRef>							_StringTable;

#if defined(NL_DEBUG)
	friend struct TStringId;
	// a special unmaping function
	static const std::string &unmap(uint stringId);
	const std::string &localUnmap(uint stringId);
#endif

	static	CStringMapper	_GlobalMapper;
	TStringId				_EmptyId;

	// private constructor.
	CStringMapper();

public:
	/// Globaly map a string into a unique Id
	static TStringId			map(const std::string &str);
	/// Globaly unmap a string
	static const std::string	&unmap(const TStringId &stringId);
	/// Return the global id for the empty string (helper function)
	static TStringId			emptyId();

	/// Create a local mapper. You can dispose of it by deleting it.
	static CStringMapper	*createLocalMapper();
	/// Localy map a string into a unique Id
	TStringId			localMap(const std::string &str);
	/// Localy unmap a string
	const std::string	&localUnmap(const TStringId &stringId);
	/// Return the local id for the empty string (helper function)
	TStringId			localEmptyId()	{return _EmptyId;}

};

#if defined(NL_DEBUG)
TStringId::TStringId(uint stringId, CStringMapper *mapper)
{
	_StringId = stringId;
	if (mapper)
		_StringValue = mapper->localUnmap(_StringId);
	else
		_StringValue = CStringMapper::unmap(_StringId);
}
#endif

} //namespace NLMISC

#endif // STRING_MAPPER_H