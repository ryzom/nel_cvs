/** \file script_debug_source.h
 * <File description>
 *
 * $Id: script_debug_source.h,v 1.3 2002/05/17 13:46:24 chafik Exp $
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

#ifndef NL_SCRIPT_DEBUG_SOURCE_H
#define NL_SCRIPT_DEBUG_SOURCE_H

#include "nel/misc/types_nl.h"
#include "nel/ai/c/abstract_interface.h"
#include <stdio.h>

namespace NLAISCRIPT 
{
/**
 * IScriptDebugSource is an interface for the script source code.
 * It give methode to use it, wether it comes from a file or from the memory.
 * \author Gabriel ROBERT
 * \author Nevrax France
 * \date 2001
 */
class IScriptDebugSource : public NLAIC::IBasicInterface
{
public:

	/// Constructor
	IScriptDebugSource(const char* sourceName);

	/// Return a name for the script source
	virtual std::string getSourceName () const;

	/// Return the entire source buffer.
	virtual std::string getSourceBuffer() const =0;

	///	Save the class in a stream.
	virtual void save(NLMISC::IStream &) = 0;
	
	///Load the class from a stream.		
	virtual void load(NLMISC::IStream &) = 0;			

protected :
	std::string	_SourceName;
};


/**
 * CScriptDebugSourceFile is an IScriptDebugSource where the source code come from a file.
 * \author Gabriel ROBERT
 * \author Nevrax France
 * \date 2001
 */
class CScriptDebugSourceFile : public IScriptDebugSource
{
public :
	static const NLAIC::CIdentType IdScriptDebugSourceFile;

private:
	std::string MemoryScript;
	bool isMemory;

public:

	/// Constructor
	CScriptDebugSourceFile(const char* sourceName, bool file = true);	

	/// Return the entire source buffer.
	std::string getSourceBuffer() const;

	///	Save the class in a stream.
	void save(NLMISC::IStream &);
	
	///Load the class from a stream.		
	void load(NLMISC::IStream &);

	const NLAIC::CIdentType& getType() const;

	const NLAIC::IBasicType* clone() const;

	const NLAIC::IBasicType* newInstance() const;

	virtual void getDebugString(std::string &) const;
};


/**
 * CScriptDebugSourceMemory is an IScriptDebugSource where the source code come from memory.
 * \author Gabriel ROBERT
 * \author Nevrax France
 * \date 2001
 */
class CScriptDebugSourceMemory : public IScriptDebugSource
{
private :
	std::string _Code;
public :
	static const NLAIC::CIdentType IdScriptDebugSourceMemory;

public:

	/// Constructor
	CScriptDebugSourceMemory(const char* sourceName, const char* code);

	/// Return the entire source buffer.
	std::string getSourceBuffer() const;

	///	Save the class in a stream.
	void save(NLMISC::IStream &);
	
	///Load the class from a stream.		
	void load(NLMISC::IStream &);

	const NLAIC::CIdentType& getType() const;

	const NLAIC::IBasicType* clone() const;

	const NLAIC::IBasicType* newInstance() const;

	virtual void getDebugString(std::string &) const;
};

} // NLAISCRIPT


#endif // NL_SCRIPT_DEBUG_SOURCE_H

/* End of script_debug_source.h */
