/** \file sheet_id.cpp
 * This class defines a sheet id
 * 
 * $Id: sheet_id.cpp,v 1.19 2002/10/21 14:41:40 cado Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

/* This class is case unsensitive. It means that you can call build() and
 * buildIdVector() with string with anycase, it'll work.
 */

#include "stdmisc.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "nel/misc/sheet_id.h"
#include "nel/misc/common.h"

using namespace std;

namespace NLMISC {

map<uint32,std::string> CSheetId::_SheetIdToName;
map<std::string,uint32> CSheetId::_SheetNameToId;
vector<std::string> CSheetId::_FileExtensions;
bool CSheetId::_Initialised=false;
bool CSheetId::_RemoveUnknownSheet=true;

const CSheetId CSheetId::Unknown(0);

void CSheetId::cbFileChange (const std::string &filename)
{
	nlinfo ("%s changed, reload it", filename.c_str());

	loadSheetId();
}

//-----------------------------------------------
//	CSheetId
//
//-----------------------------------------------
CSheetId::CSheetId( const string& sheetName )
{
	if (!build(sheetName))
	{
		nlwarning("<CSheetId::CSheetId> The sheet %s is not in sheet_id.bin, setting it to Unknown",sheetName.c_str());
		*this = Unknown;
	}

} // CSheetId //


//-----------------------------------------------
//	Build
//
//-----------------------------------------------
bool CSheetId::build(const std::string& sheetName)
{
	nlassert(_Initialised);

	map<string,uint32>::const_iterator itId = _SheetNameToId.find( strlwr(sheetName) );
	if( itId != _SheetNameToId.end() )
	{
		_Id.Id = (*itId).second;
		return true;
	}
	else
	{
		return false;		
	}
}

void CSheetId::loadSheetId ()
{
	// Open the sheet id to sheet file name association
	CIFile file;
	if(file.open(CPath::lookup("sheet_id.bin", false, false)))
	{
		// clear entries
		_FileExtensions.clear ();
		_SheetIdToName.clear ();
		_SheetNameToId.clear ();

		// reserve space for the vector of file extensions
		_FileExtensions.resize(256);

		// Get the map.
		file.serialCont(_SheetIdToName);

		// Close the file.
		file.close();

		if (_RemoveUnknownSheet)
		{
			uint32 removednbfiles = 0;
			uint32 nbfiles = _SheetIdToName.size();

			// now we remove all files that not available
			map<uint32,string>::iterator itStr2;
			for( itStr2 = _SheetIdToName.begin(); itStr2 != _SheetIdToName.end(); )
			{
				if (CPath::exists ((*itStr2).second))
				{
					++itStr2;
				}
				else
				{
					map<uint32,string>::iterator olditStr = itStr2;
					//nldebug ("Removing file '%s' from CSheetId because the file not exists", (*olditStr).second.c_str ());
					itStr2++;
					_SheetIdToName.erase (olditStr);
					removednbfiles++;
				}
			}

			nlinfo ("Removed %d files on %d from CSheetId because these files doesn't exists", removednbfiles, nbfiles);
		}

		// build the invert map & file extension vector
		map<uint32,string>::iterator itStr;
		for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
		{
			// add entry to the inverse map
			_SheetNameToId.insert( make_pair(strlwr((*itStr).second),(*itStr).first) );

			// work out the type value for this entry in the map
			TSheetId sheetId;
			sheetId.Id=(*itStr).first;
			uint8 type=	sheetId.IdInfos.Type;
 
			// check whether we need to add an entry to the file extensions vector
			if (_FileExtensions[type].empty())
			{
				// find the file extension part of the given file name
				_FileExtensions[type]=strlwr(CFile::getExtension((*itStr).second));
			}
		}
	}
	else
	{
		nlerror("<CSheetId::init> Can't open the file sheet_id.bin");
	}
}

//-----------------------------------------------
//	init
//
//-----------------------------------------------
void CSheetId::init(bool removeUnknownSheet)
{
	// allow multiple calls to init in case libraries depending on sheetid call this init from their own
	if (_Initialised)
		return;

	CFile::addFileChangeCallback ("sheet_id.bin", cbFileChange);

	_RemoveUnknownSheet = removeUnknownSheet;

	loadSheetId ();

	_Initialised=true;
} // init //




//-----------------------------------------------
//	operator=
//
//-----------------------------------------------
CSheetId& CSheetId::operator=( const CSheetId& sheetId )
{
	nlassert(_Initialised);

	if(this == &sheetId)
	{
		return *this;
	}

	_Id.Id = sheetId.asInt();

    return *this;


} // operator= //


//-----------------------------------------------
//	operator=
//
//-----------------------------------------------
CSheetId& CSheetId::operator=( const string& sheetName )
{
	nlassert(_Initialised);

	map<string,uint32>::const_iterator itId = _SheetNameToId.find( strlwr(sheetName) );
	if( itId != _SheetNameToId.end() )
	{
		_Id.Id = (*itId).second;
	}
	else
	{
		nlwarning("<CSheetId::operator=> The sheet %s is not in sheet_id.bin, setting it to Unknown",sheetName.c_str());
		*this = Unknown;
	}
	
	return *this;

} // operator= //


//-----------------------------------------------
//	operator=
//
//-----------------------------------------------
CSheetId& CSheetId::operator=( uint32 sheetRef )
{
	nlassert(_Initialised);

	_Id.Id = sheetRef;
	
	return *this;

} // operator= //



//-----------------------------------------------
//	operator<
//
//-----------------------------------------------
bool CSheetId::operator < (const CSheetId& sheetRef ) const
{
	nlassert(_Initialised);

	if (_Id.Id < sheetRef.asInt())
	{
		return true;
	}
			
	return false;

} // operator< //



//-----------------------------------------------
//	toString
//
//-----------------------------------------------
string CSheetId::toString() const
{
	nlassert(_Initialised);

	map<uint32,string>::const_iterator itStr = _SheetIdToName.find( _Id.Id );
	if( itStr != _SheetIdToName.end() )
	{
		return (*itStr).second;
	}
	else
	{
		// This nlwarning is commented out because the loggers are mutexed, therefore
		// you couldn't use toString() within a nlwarning().
		//nlwarning("<CSheetId::toString> The sheet %08x is not in sheet_id.bin",_Id.Id);
		return NLMISC::toString( "<Sheet %d not found in sheet_id.bin>", _Id.Id );
	}

} // toString //



//-----------------------------------------------
//	display
//
//-----------------------------------------------
void CSheetId::display()
{
	nlassert(_Initialised);

	map<uint32,string>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		//nlinfo("%d %s",(*itStr).first,(*itStr).second.c_str());
		nlinfo("(%08x %d) %s",(*itStr).first,(*itStr).first,(*itStr).second.c_str());
	}

} // display //



//-----------------------------------------------
//	display
//
//-----------------------------------------------
void CSheetId::display(uint8 type)
{
	nlassert(_Initialised);

	map<uint32,string>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		// work out the type value for this entry in the map
		TSheetId sheetId;
		sheetId.Id=(*itStr).first;
 
		// decide whether or not to dsiplay the entry
		if (type==sheetId.IdInfos.Type)
		{
			//nlinfo("%d %s",(*itStr).first,(*itStr).second.c_str());
			nlinfo("(%08x %d) %s",(*itStr).first,(*itStr).first,(*itStr).second.c_str());
		}
	}

} // display //



//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result)
{
	nlassert(_Initialised);

	map<uint32,string>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		result.push_back( (CSheetId)(*itStr).first );
	}

} // buildIdVector //


//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result,uint8 type)
{
	nlassert(_Initialised);

	map<uint32,string>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		// work out the type value for this entry in the map
		TSheetId sheetId;
		sheetId.Id=(*itStr).first;
 
		// decide whether or not to use the entry
		if (type==sheetId.IdInfos.Type)
		{
			result.push_back( (CSheetId)sheetId.Id );
		}
	}

} // buildIdVector //

//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result, std::vector <std::string> &resultFilenames,uint8 type)
{
	nlassert(_Initialised);

	map<uint32,string>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		// work out the type value for this entry in the map
		TSheetId sheetId;
		sheetId.Id=(*itStr).first;
 
		// decide whether or not to use the entry
		if (type==sheetId.IdInfos.Type)
		{
			result.push_back( (CSheetId)sheetId.Id );
			resultFilenames.push_back( (*itStr).second );
		}
	}

} // buildIdVector //

//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result,const std::string &fileExtension)
{
	uint32 type=typeFromFileExtension(fileExtension);
	if (type!=(uint32)~0)
		buildIdVector(result,(uint8)type);

} // buildIdVector //

//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result, std::vector <std::string> &resultFilenames,const std::string &fileExtension)
{
	uint32 type=typeFromFileExtension(fileExtension);
	if (type!=(uint32)~0)
		buildIdVector(result,resultFilenames, (uint8)type);

} // buildIdVector //


//-----------------------------------------------
//	typeFromFileExtension
//
//-----------------------------------------------
uint32 CSheetId::typeFromFileExtension(const std::string &fileExtension)
{
	nlassert(_Initialised);

	unsigned i;
	for (i=0;i<_FileExtensions.size();i++)
		if (strlwr(fileExtension)==_FileExtensions[i])
			return i;

	return ~0;
} // typeFromFileExtension //


//-----------------------------------------------
//	fileExtensionFromType
//
//-----------------------------------------------
const std::string &CSheetId::fileExtensionFromType(uint8 type)
{
	nlassert(_Initialised);
	nlassert(type<256);

	return _FileExtensions[type];

} // fileExtensionFromType //

} // NLMISC
