/** \file string_mapper.cpp
 *
 * $Id: string_mapper.h,v 1.11.8.1 2004/10/22 09:15:38 berenguier Exp $
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

#include <vector>
#include <set>

#include "nel/misc/stream.h"
#include "nel/misc/mutex.h"

namespace NLMISC
{

// const string *  as  uint (the TStringId returned by CStringMapper is a pointer to a string object)
//#ifdef HAVE_X86_64
//typedef uint64 TStringId;
//#else
//typedef uint TStringId;
//#endif

typedef	const std::string * TStringId;

class CStringIdHasher
{
public:
	CStringIdHasher()
	{}
	inline	size_t	operator	()(const NLMISC::TStringId &stringId)	const
	{
		return	(size_t)stringId;
	}
protected:
private:
};

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

	class CAutoFastMutex
	{
		CFastMutex		*_Mutex;
	public:
		CAutoFastMutex(CFastMutex *mtx) : _Mutex(mtx)	{_Mutex->enter();}
		~CAutoFastMutex() {_Mutex->leave();}
	};

	// Local Data
	std::set<std::string*,CCharComp>	_StringTable;
	std::string*			_EmptyId;
	CFastMutex				_Mutex;		// Must be thread-safe (Called by CPortal/CCluster, each of them called by CInstanceGroup)
	
	// The 'singleton' for static methods
	static	CStringMapper	_GlobalMapper;

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
	static TStringId			emptyId() { return 0; }

	static void					clear() { _GlobalMapper.localClear(); }

	/// Create a local mapper. You can dispose of it by deleting it.
	static CStringMapper *	createLocalMapper();
	/// Localy map a string into a unique Id
	TStringId				localMap(const std::string &str);
	/// Localy unmap a string
	const std::string		&localUnmap(const TStringId &stringId) { return (stringId==0)?*_EmptyId:*((std::string*)stringId); }
	/// Return the local id for the empty string (helper function)
	TStringId				localEmptyId()	{ return 0; }

	void					localClear();

};

// linear from 0 (0 is empty string) (The TSStringId returned by CStaticStringMapper 
// is an index in the vector and begin at 0)
typedef uint TSStringId;

/** 
 * After endAdd you cannot add strings anymore or it will assert
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
	// Uncompress the map.
	void				memoryUncompress();
	/// Globaly unmap a string
	const char *		get(TSStringId stringId);
	/// Return the global id for the empty string (helper function)
	TSStringId			emptyId() { return 0; }

	void				clear();

	uint32 getCount() { return _IdCounter; }

	// helper serialize a string id as a string
	void				serial(NLMISC::IStream &f, TSStringId &strId) throw(EStream);

	// helper serialize a string id vector
	void				serial(NLMISC::IStream &f, std::vector<TSStringId> &strIdVect) throw(EStream);
	
};

} //namespace NLMISC

#endif // STRING_MAPPER_H

