/** \file script_debug_source.cpp
 * <File description>
 *
 * $Id: script_debug_source.cpp,v 1.4 2002/05/17 13:46:34 chafik Exp $
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

#include "nel/ai/script/script_debug_source.h"


namespace NLAISCRIPT {

//*********************
//IScriptDebugSource
//*********************

/*
 * Constructor
 */
IScriptDebugSource::IScriptDebugSource(const char* sourceName):
_SourceName(sourceName)
{
}

/// Return a name for the script source
std::string IScriptDebugSource::getSourceName () const
{
	return _SourceName;
}

//*********************
//CScriptDebugSourceFile
//*********************

/*
 * Constructor
 */
CScriptDebugSourceFile::CScriptDebugSourceFile(const char* sourceName, bool file): 
	IScriptDebugSource((file) ? sourceName : "memoryScript"), isMemory(!file)
{
	if(!file) MemoryScript = sourceName;
}

/// Return the entire source buffer.
std::string CScriptDebugSourceFile::getSourceBuffer() const
{
	if(!isMemory)
	{
		FILE* f;
		sint32 size;
		char* buf;
		std::string ret;

		// Read the file
		f = fopen(_SourceName.c_str(),"rb");
		fseek(f,0,SEEK_END);
		size = ftell(f);
		rewind(f);
		buf = new char [size + 4];
		fread(buf+1, sizeof( char ), size, f);
		fclose(f);
		buf[0] = ' ';
		buf[size+1] = '\n';
		buf[size+2] = 0;

		ret = buf;
		delete[] buf;
		
		return ret;
	}
	else return MemoryScript;
}

///	Save the class in a stream.
void CScriptDebugSourceFile::save(NLMISC::IStream &os)
{
	os.serial(_SourceName);
}

///Load the class from a stream.		
void CScriptDebugSourceFile::load(NLMISC::IStream &is)
{
	is.serial(_SourceName);
}

const NLAIC::CIdentType& CScriptDebugSourceFile::getType() const
{		
	return IdScriptDebugSourceFile;
}

const NLAIC::IBasicType* CScriptDebugSourceFile::clone() const 
{
	NLAIC::IBasicType *x = new CScriptDebugSourceFile(*this);
	return x;
}

const NLAIC::IBasicType *CScriptDebugSourceFile::newInstance() const 
{
	NLAIC::IBasicType *x = new CScriptDebugSourceFile("");
	return x;
}

void CScriptDebugSourceFile::getDebugString(std::string &text) const
{
	text += "CScriptDebugSourceFile<";
	text += _SourceName;
	text += ">";
}

//*********************
//CScriptDebugSourceMemory
//*********************

/*
 * Constructor
 */
CScriptDebugSourceMemory::CScriptDebugSourceMemory(const char* sourceName, const char* code):
IScriptDebugSource(sourceName)
{
	_Code = code;
}

/// Return the entire source buffer.
std::string CScriptDebugSourceMemory::getSourceBuffer() const
{
	return _Code;
}

///	Save the class in a stream.
void CScriptDebugSourceMemory::save(NLMISC::IStream &os)
{
	os.serial(_SourceName);
	os.serial(_Code);
}

///Load the class from a stream.		
void CScriptDebugSourceMemory::load(NLMISC::IStream &is)
{
	is.serial(_SourceName);
	is.serial(_Code);
}

const NLAIC::CIdentType& CScriptDebugSourceMemory::getType() const
{		
	return IdScriptDebugSourceMemory;
}

const NLAIC::IBasicType* CScriptDebugSourceMemory::clone() const 
{
	NLAIC::IBasicType *x = new CScriptDebugSourceMemory(*this);
	return x;
}

const NLAIC::IBasicType *CScriptDebugSourceMemory::newInstance() const 
{
	NLAIC::IBasicType *x = new CScriptDebugSourceMemory("","");
	return x;
}

void CScriptDebugSourceMemory::getDebugString(std::string &text) const
{
	text += "CScriptDebugSourceMemory<";
	text += _SourceName;
	text += ">";
	text += "\nValue : ";
	text += _Code;
}

} // NLAISCRIPT
