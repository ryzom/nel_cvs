/** \file string_mapper.cpp
 *
 * $Id: string_mapper.h,v 1.5 2003/11/06 12:50:03 besson Exp $
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
#include <set>

namespace NLMISC
{

// const string *  as  uint
typedef uint TStringId;

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
	class CCharComp
	{
	public:
		bool operator()(std::string *x, std::string *y) const
		{
			return (*x) < (*y);
		}
	};

	std::set<std::string*,CCharComp>	_StringTable;

	static	CStringMapper	_GlobalMapper;
	std::string*			_EmptyId;

	// private constructor.
	CStringMapper();

public:

	~CStringMapper()
	{
		localClear();
	}

	/// Globaly map a string into a unique Id
	static TStringId			map(const std::string &str) { return _GlobalMapper.localMap(str); }
	/// Globaly unmap a string
	static const std::string	&unmap(const TStringId &stringId) { return _GlobalMapper.localUnmap(stringId); }
	/// Return the global id for the empty string (helper function)
	static TStringId			emptyId() { return NULL; }

	static void					clear() { _GlobalMapper.localClear(); }

	/// Create a local mapper. You can dispose of it by deleting it.
	static CStringMapper *	createLocalMapper();
	/// Localy map a string into a unique Id
	TStringId				localMap(const std::string &str);
	/// Localy unmap a string
	const std::string		&localUnmap(const TStringId &stringId) { return (stringId==NULL)?*_EmptyId:*((std::string*)stringId); }
	/// Return the local id for the empty string (helper function)
	TStringId				localEmptyId()	{ return NULL; }

	void					localClear();

};

// linear from 0 (0 is empty string)
typedef uint TSStringId;

/** 
 * After endAdd you cannot add strings anymore
 * \author Matthieu Besson
 * \author Nevrax France
 * \date November 2003
 */
class CStaticStringMapper
{

	std::map<std::string, TSStringId>	_TempStringTable;
	std::map<TSStringId, std::string>	_TempIdTable;

	uint32	_IdCounter;
	char	*_AllStrings;
	std::vector<char*>	_IdToStr;
	bool _MemoryCompressed; // If false use the 2 maps

public:

	CStaticStringMapper()
	{
		_IdCounter = 0;
		_AllStrings = NULL;
		_MemoryCompressed = false;
		add("");
	}

	~CStaticStringMapper()
	{
		clear();
	}

	/// Globaly map a string into a unique Id
	TSStringId			add(const std::string &str);

	void				memoryCompress();
	/// Globaly unmap a string
	const char *		get(TSStringId stringId);
	/// Return the global id for the empty string (helper function)
	TSStringId			emptyId() { return 0; }

	void				clear();

	uint32 getCount() { return _IdCounter; }
	
};

} //namespace NLMISC

#endif // STRING_MAPPER_H

