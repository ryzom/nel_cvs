/** \file string_mapper.cpp
 * A static class that map string to integer and vice-versa
 * Each different string is tranformed into an unique integer identifier.
 * If the same string is submited twice, the same id is returned.
 * The class can also (but not in an optimized manner) return the
 * string associated with an id.
 *
 * $Id: string_mapper.cpp,v 1.1 2003/01/08 15:52:59 boucher Exp $
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

std::hash_map<std::string, uint>	CStringMapper::_StringMap;

uint CStringMapper::map(const std::string &str)
{
	std::hash_map<std::string, uint>::iterator it(_StringMap.find(str));

	if (it == _StringMap.end())
	{
		// create a new id
		uint id = _StringMap.size();
		_StringMap.insert(std::make_pair(str, id));
		return id;
	}
	else
		return it->second;
		
}

const std::string	&CStringMapper::unmap(uint id)
{
	const static std::string notFound;
	std::hash_map<std::string, uint>::iterator first(_StringMap.begin()), last(_StringMap.end());
	for (; first != last; ++first)
	{
		if (first->second == id)
			return first->first;
	}

	nlassertex(false, ("There are no string mapped to id %u", id));
	// not found ! return an empty string
	return notFound;
}

} // namespace NLMISC
