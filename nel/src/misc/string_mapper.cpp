/** \file string_mapper.cpp
 * A static class that map string to integer and vice-versa
 * Each different string is tranformed into an unique integer identifier.
 * If the same string is submited twice, the same id is returned.
 * The class can also (but not in an optimized manner) return the
 * string associated with an id.
 *
 * $Id: string_mapper.cpp,v 1.2 2003/03/03 13:04:16 boucher Exp $
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

#include "stdmisc.h"
#include "nel/misc/string_mapper.h"

namespace NLMISC
{

//CStringMapper::TStringRefHashMap		CStringMapper::_StringMap;
//std::vector<CStringMapper::CStringRef>	CStringMapper::_StringTable;
CStringMapper	CStringMapper::_GlobalMapper;


CStringMapper::CStringMapper()
{
	_EmptyId = localMap("");
}

CStringMapper *CStringMapper::createLocalMapper()
{
	return new CStringMapper;
}

TStringId	CStringMapper::map(const std::string &str)
{
	return _GlobalMapper.localMap(str);
}
const std::string	&CStringMapper::unmap(const TStringId &stringId)
{
	return _GlobalMapper.localUnmap(stringId);

}
TStringId	CStringMapper::emptyId()
{
	return _GlobalMapper._EmptyId;
}


NLMISC::TStringId CStringMapper::localMap(const std::string &str)
{
	CStringRef	ref(&str);
	TStringRefHashMap::iterator it(_StringMap.find(ref));

	if (it == _StringMap.end())
	{
		// create a new id
		uint id = _StringTable.size();
		_StringTable.push_back(CStringRef(new std::string(str)));
		_StringMap.insert(std::make_pair(_StringTable.back(), id));
#if defined(_DEBUG)
		return TStringId(id, this);
#else
		return id;
#endif
	}
	else
	{
#if defined(_DEBUG)
		return TStringId(it->second, *(it->first.String));
#else
		return it->second;		
#endif
	}
		
}

const std::string	&CStringMapper::localUnmap(const NLMISC::TStringId &stringId)
{
	const static std::string notFound("** Invalid stringId unmapped ! **");
	
	if (uint(stringId) < _StringTable.size())
	{
		// ok, we have an unmap
		return *(_StringTable[uint(stringId)].String);
	}
	else
	{
		nlassertex(false, ("There are no string mapped to id %u", uint(stringId)));
		// not found ! return an empty string
		return notFound;
	}
}

#if defined(_DEBUG)
const std::string &CStringMapper::unmap(uint stringId)
{
	return _GlobalMapper.localUnmap(stringId);
}

const std::string &CStringMapper::localUnmap(uint stringId)
{
	const static std::string notFound;
	
	if (stringId < _StringTable.size())
	{
		// ok, we have an unmap
		return *(_StringTable[stringId].String);
	}
	else
	{
		nlassertex(false, ("There are no string mapped to id %u", uint(stringId)));
		// not found ! return an empty string
		return notFound;
	}
}

#endif

} // namespace NLMISC
