/** \file sheet_id.cpp
 * This class defines a sheet id
 * 
 * $Id: sheet_id.cpp,v 1.27 2003/11/06 12:51:33 besson Exp $
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

CSheetId::CChar CSheetId::_AllStrings;
CStaticMap<uint32,CSheetId::CChar> CSheetId::_SheetIdToName;
CStaticMap<CSheetId::CChar,uint32, CSheetId::CCharComp> CSheetId::_SheetNameToId;
//map<uint32,std::string> CSheetId::_SheetIdToName;
//map<std::string,uint32> CSheetId::_SheetNameToId;
map<uint32,std::string> CSheetId::_SheetIdToAlias;
map<std::string,uint32> CSheetId::_SheetAliasToId;
vector<std::string> CSheetId::_FileExtensions;
bool CSheetId::_Initialised=false;
bool CSheetId::_RemoveUnknownSheet=true;

const CSheetId CSheetId::Unknown(0);

void CSheetId::cbFileChange (const std::string &filename)
{
	nlinfo ("SHEETID: %s changed, reload it", filename.c_str());

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
		nlwarning("SHEETID: The sheet '%s' is not in sheet_id.bin, setting it to Unknown",sheetName.c_str());
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

	map<string,uint32>::const_iterator itIdAlias;
	
	itIdAlias = _SheetAliasToId.find( strlwr(sheetName) );
	if( itIdAlias != _SheetAliasToId.end() )
	{
		_Id.Id = (*itIdAlias).second;
		return true;
	}
	

	CStaticMap<CChar,uint32,CCharComp>::const_iterator itId;
	CChar c;
	c.Ptr = new char [sheetName.size()+1];
	strcpy(c.Ptr, sheetName.c_str());
	strlwr(c.Ptr);

	itId = _SheetNameToId.find (c);
	delete [] c.Ptr;
	if( itId != _SheetNameToId.end() )
	{
		_Id.Id = (*itId).second;
		return true;
	}
	return false;		
	
}

void CSheetId::loadSheetId ()
{
	// Open the sheet id to sheet file name association
	CIFile file;
	std::string path = CPath::lookup("sheet_id.bin", false, false);
	if(!path.empty() && file.open(path))
	{
		// clear entries
		_FileExtensions.clear ();
		_SheetIdToName.clear ();
		_SheetNameToId.clear ();

		// reserve space for the vector of file extensions
		_FileExtensions.resize(256);

		// Get the map from the file
		map<uint32,string> tempMap;
		contReset(tempMap);
		file.serialCont(tempMap);

		// Convert the map to one big string and 1 static map (id to name)
		{
			// Get the number and size of all strings
			vector<CChar> tempVec; // Used to initialise the first map
			uint32 nNb = 0;
			uint32 nSize = 0;
			map<uint32,string>::const_iterator it = tempMap.begin();
			while (it != tempMap.end())
			{
				nSize += it->second.size()+1;
				nNb++;
				it++;
			}

			// Make the big string (composed of all strings) and a vector referencing each string
			tempVec.resize(nNb);
			_AllStrings.Ptr = new char[nSize];
			it = tempMap.begin();
			nSize = 0;
			nNb = 0;
			while (it != tempMap.end())
			{
				tempVec[nNb].Ptr = _AllStrings.Ptr+nSize;
				strcpy(_AllStrings.Ptr+nSize, it->second.c_str());
				strlwr(_AllStrings.Ptr+nSize);
				nSize += it->second.size()+1;
				nNb++;
				it++;
			}

			// Finally build the static map (id to name)
			_SheetIdToName.reserve(tempVec.size());
			it = tempMap.begin();
			nNb = 0;
			while (it != tempMap.end())
			{
				_SheetIdToName.add(pair<uint32, CChar>::pair(it->first, CChar(tempVec[nNb])));

				nNb++;
				it++;
			}

			// The vector of all small string is not needed anymore we have all the info in
			// the static map and with the pointer AllStrings referencing the beginning.
		}


		// Close the file.
		file.close();

		if (_RemoveUnknownSheet)
		{
			uint32 removednbfiles = 0;
			uint32 nbfiles = _SheetIdToName.size();

			// now we remove all files that not available
			CStaticMap<uint32,CChar>::iterator itStr2;
			for( itStr2 = _SheetIdToName.begin(); itStr2 != _SheetIdToName.end(); )
			{
				if (CPath::exists ((*itStr2).second.Ptr))
				{
					++itStr2;
				}
				else
				{
					CStaticMap<uint32,CChar>::iterator olditStr = itStr2;
					//nldebug ("Removing file '%s' from CSheetId because the file not exists", (*olditStr).second.c_str ());
					itStr2++;
					_SheetIdToName.del (olditStr);
					removednbfiles++;
				}
			}

			nlinfo ("SHEETID: Removed %d files on %d from CSheetId because these files doesn't exists", removednbfiles, nbfiles);
		}

		// Build the invert map (Name to Id) & file extension vector
		{
			uint32 nSize = _SheetIdToName.size();
			_SheetNameToId.reserve(nSize);
			CStaticMap<uint32,CChar>::iterator itStr;
			for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
			{
				// add entry to the inverse map
				_SheetNameToId.add( make_pair((*itStr).second, (*itStr).first) );

				// work out the type value for this entry in the map
				TSheetId sheetId;
				sheetId.Id=(*itStr).first;
				uint8 type=	sheetId.IdInfos.Type;
 
				// check whether we need to add an entry to the file extensions vector
				if (_FileExtensions[type].empty())
				{
					// find the file extension part of the given file name
					_FileExtensions[type]=strlwr(CFile::getExtension((*itStr).second.Ptr));
				}
				nSize--;
			}
			_SheetNameToId.endAdd();
		}
	}
	else
	{
		nlerror("<CSheetId::init> Can't open the file sheet_id.bin");
	}
}

void CSheetId::loadSheetAlias ()
{
	// Open the sheet id to sheet alias association
	CIFile file;
	if(file.open(CPath::lookup("alias.packed_sheets", false, false)))
	{
		// clear entries
		_SheetIdToAlias.clear ();
		_SheetAliasToId.clear ();

		// Get the map.
		uint32 nbEntries;
		file.serial (nbEntries);
		file.setVersionException (false, false);
		uint ver = 1;
		file.serialVersion(ver);
		file.serialCont(_SheetIdToAlias);

		// Close the file.
		file.close();

		if (_RemoveUnknownSheet)
		{
		  //uint32 nbfiles = _SheetIdToAlias.size();

			// now we remove all files that not available
			map<uint32,string>::iterator itStr2;
			for( itStr2 = _SheetIdToAlias.begin(); itStr2 != _SheetIdToAlias.end(); )
			{
				if (CPath::exists (CSheetId((*itStr2).first).toString()))
				{
					++itStr2;
				}
				else
				{
					map<uint32,string>::iterator olditStr = itStr2;
					itStr2++;
					_SheetIdToAlias.erase (olditStr);
				}
			}
		}

		// build the invert map
		map<uint32,string>::iterator itStr;
		for( itStr = _SheetIdToAlias.begin(); itStr != _SheetIdToAlias.end(); ++itStr )
		{
			// add entry to the inverse map
			if( !(*itStr).second.empty() )
			{
				_SheetAliasToId.insert( make_pair(strlwr((*itStr).second),(*itStr).first) );
			}
			else
			{
				//nlwarning("<CSheetId::loadSheetAlias> The sheet %s doesn't have alias",CSheetId((*itStr).first).toString().c_str());
			}
		}
	}
	else
	{
		nlinfo("SHEETID: Can't open the file alias.packed_sheets");
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
	loadSheetAlias ();


} // init //


//-----------------------------------------------
//	uninit
//
//-----------------------------------------------
void CSheetId::uninit()
{
	delete [] _AllStrings.Ptr;
} // uninit //

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

	map<string,uint32>::const_iterator itIdAlias;
	
	itIdAlias = _SheetAliasToId.find( strlwr(sheetName) );
	if( itIdAlias != _SheetAliasToId.end() )
	{
		_Id.Id = (*itIdAlias).second;
		return *this;
	}
	

	CStaticMap<CChar,uint32,CCharComp>::const_iterator itId;
	CChar c;
	c.Ptr = new char [sheetName.size()+1];
	strcpy(c.Ptr, sheetName.c_str());
	strlwr(c.Ptr);

	itId = _SheetNameToId.find (c);
	delete [] c.Ptr;
	if( itId != _SheetNameToId.end() )
	{
		_Id.Id = (*itId).second;
		return *this;
	}
	*this = Unknown;
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

	CStaticMap<uint32,CChar>::const_iterator itStr = _SheetIdToName.find (_Id.Id);
	if( itStr != _SheetIdToName.end() )
	{
		return string((*itStr).second.Ptr);
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

	CStaticMap<uint32,CChar>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		//nlinfo("%d %s",(*itStr).first,(*itStr).second.c_str());
		nlinfo("SHEETID: (%08x %d) %s",(*itStr).first,(*itStr).first,(*itStr).second.Ptr);
	}

} // display //



//-----------------------------------------------
//	display
//
//-----------------------------------------------
void CSheetId::display(uint8 type)
{
	nlassert(_Initialised);

	CStaticMap<uint32,CChar>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		// work out the type value for this entry in the map
		TSheetId sheetId;
		sheetId.Id=(*itStr).first;
 
		// decide whether or not to dsiplay the entry
		if (type==sheetId.IdInfos.Type)
		{
			//nlinfo("%d %s",(*itStr).first,(*itStr).second.c_str());
			nlinfo("SHEETID: (%08x %d) %s",(*itStr).first,(*itStr).first,(*itStr).second.Ptr);
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

	CStaticMap<uint32,CChar>::const_iterator itStr;
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

	CStaticMap<uint32,CChar>::const_iterator itStr;
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

	CStaticMap<uint32,CChar>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		// work out the type value for this entry in the map
		TSheetId sheetId;
		sheetId.Id=(*itStr).first;
 
		// decide whether or not to use the entry
		if (type==sheetId.IdInfos.Type)
		{
			result.push_back( (CSheetId)sheetId.Id );
			resultFilenames.push_back( (*itStr).second.Ptr );
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

//-----------------------------------------------
//	build
//
//-----------------------------------------------
void	CSheetId::build(uint32 shortId, uint8 type)
{
	_Id.IdInfos.Id= shortId;
	_Id.IdInfos.Type= type;
}

} // NLMISC
