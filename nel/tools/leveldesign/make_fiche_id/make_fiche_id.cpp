/** \file make_fiche_id.cpp
 * 
 *
 * $Id: make_fiche_id.cpp,v 1.2 2002/02/12 10:33:56 coutelas Exp $
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


// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"

// std
#include <string>

#include <iostream>

#include "windows.h"

using namespace NLMISC;
using namespace std;



/**
 *	TFormId
 */
union TFormId
{
	uint32		Id;
	
	struct
	{
		uint32	Type	: 8;
		uint32	Id		: 24;
	} FormIDInfos;

	void serial(NLMISC::IStream &f) { f.serial(Id); };
};

bool operator<(const TFormId& fid1, const TFormId& fid2) { return fid1.Id<fid2.Id; }


map<string,TFormId> FormToId;
map<TFormId,string> IdToForm;
map<string,uint8> FileTypeToId;
map<uint8,string> IdToFileType;
map<uint8,uint32> TypeToLastId;
sint32 lastFileTypeId = -1;



//	getInputDirectoryList
void getInputDirectoryList( int argc, char **argv, list<string>& dirs );

//manageFile
void manageFile( WIN32_FIND_DATA& findFileData, list<string>& dirs, string& currentDir );

// addId
void addId( string fileName );

// getFileType
bool getFileType( string& fileName, string& fileType );

// displayHelp
void displayHelp();

// main
void main( int argc, char ** argv );





//-----------------------------------------------
//	displayHelp
//
//-----------------------------------------------
void displayHelp()
{
	cout<<"This tool associates an ID to the files in the given directory"<<endl<<endl;
	cout<<"MAKE_FICHE_ID  <output file> [<start directory>] [<directory2>] ..."<<endl;

} // displayHelp //



//-----------------------------------------------
//	getInputDirectoryList
//
//-----------------------------------------------
void getInputDirectoryList( int argc, char **argv, list<string>& dirs )
{
	if( argc < 3 )
	{
		dirs.push_back(".");
	}
	else
	{
		uint i;
		for( i = 2; i < (uint)argc; i++ )
		{
			dirs.push_back( string(argv[i]) );
		}
	}	

} // getInputDirectoryList //



//-----------------------------------------------
//	readFormId
//
//-----------------------------------------------
void readFormId( string& outputFileName )
{
	CIFile f;
	if( f.open( outputFileName ) )
	{
		f.serialCont( IdToForm );
	}

	// init FormToId (associates the form name to its id )
	map<TFormId,string>::iterator itIF;
	for( itIF = IdToForm.begin(); itIF != IdToForm.end(); ++itIF )
	{
		FormToId.insert( make_pair((*itIF).second,(*itIF).first) );
	}

	// init FileTypeToId (associates the form type to the form type id)
	for( itIF = IdToForm.begin(); itIF != IdToForm.end(); ++itIF )
	{
		// get the file type from form name
		TFormId fid = (*itIF).first;
		string fileType;
		if( getFileType( (*itIF).second, fileType ) )
		{
			// insert the association (file type/file type id)
			map<string,uint8>::iterator itFT = FileTypeToId.find(fileType);
			if( itFT != FileTypeToId.end() )
			{
				FileTypeToId.insert( make_pair(fileType,fid.FormIDInfos.Type) );
			}
		}
		else
		{
			nlinfo("Unknown file type for the file : %s",(*itIF).second.c_str());
		}
	}
	
	// init IdToFileType (associates the form type id to the form type name)
	map<string,uint8>::iterator itIFT;
	for( itIFT = FileTypeToId.begin(); itIFT != FileTypeToId.end(); ++itIFT )
	{
		IdToFileType.insert( make_pair((*itIFT).second,(*itIFT).first) );
	}

	// init TypeToLastId (associates the type id to the last index used for this type)
	for( itIF = IdToForm.begin(); itIF != IdToForm.end(); ++itIF )
	{
		uint8 type = (*itIF).first.FormIDInfos.Type;
		uint32 id = (*itIF).first.FormIDInfos.Id;
		map<uint8,uint32>::iterator itTLI = TypeToLastId.find( type );
		if( itTLI != TypeToLastId.end() )
		{
			if( (*itTLI).second < id )
			{
				(*itTLI).second = id;
			}
		}
		else
		{
			TypeToLastId.insert( make_pair(type,id) );
		}
	}

	// init lastFileTypeId
	if( IdToFileType.begin() != IdToFileType.end() )
	{
		lastFileTypeId = (*IdToFileType.rbegin()).first;
	}

} // readFormId //



//-----------------------------------------------
//	makeId
//
//-----------------------------------------------
void makeId( list<string>& dirs )
{
	list<string>::const_iterator itDir;
	for( itDir = dirs.begin(); itDir != dirs.end(); ++itDir )
	{
		WIN32_FIND_DATA findFileData;
		HANDLE hFind;
		cout<<"searching in directory "<<(*itDir).c_str()<<endl;
		hFind = FindFirstFile((*itDir+"\\*").c_str(), &findFileData );
		string currentDir(*itDir);

		if( hFind == INVALID_HANDLE_VALUE ) 
		{
    		cerr<<"Invalid File Handle"<<endl;
		} 
		else 
		{
			do
			{
				manageFile( findFileData, dirs, currentDir );
			}
    		while( FindNextFile( hFind, &findFileData ) );
			FindClose( hFind );
  		}
	}

} // makeId //



//-----------------------------------------------
//	manageFile
//
//-----------------------------------------------
void manageFile( WIN32_FIND_DATA& findFileData, list<string>& dirs, string& currentDir )
{
	if( strcmp(".",findFileData.cFileName) && strcmp("..",findFileData.cFileName) )
	{
		if( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			cout<<"(D)"<<findFileData.cFileName<<endl;
			dirs.push_back( currentDir + "\\" + string(findFileData.cFileName) );
		}
		else
		{
			cout<<"(F)"<<findFileData.cFileName<<endl;
			addId( string(findFileData.cFileName) );
		}
	}

} // manageFile //



//-----------------------------------------------
//	addId
//
//-----------------------------------------------
void addId( string fileName )
{
	
	// if the file is new
	map<string,TFormId>::iterator itFI = FormToId.find( fileName );
	if( itFI == FormToId.end() )
	{
		string fileType;
		if( getFileType( fileName, fileType ) )
		{
			map<string,uint8>::iterator itFTI = FileTypeToId.find( fileType );
			
			// if the type of this file is a new type
			if( itFTI == FileTypeToId.end() )
			{
				lastFileTypeId++;
				FileTypeToId.insert( make_pair(fileType,lastFileTypeId) );
				IdToFileType.insert( make_pair(lastFileTypeId,fileType) );
				TypeToLastId.insert( make_pair(lastFileTypeId,0) );

				TFormId fid;
				fid.FormIDInfos.Type = lastFileTypeId;
				fid.FormIDInfos.Id = 0;

				FormToId.insert( make_pair(fileName,fid) );
				IdToForm.insert( make_pair(fid,fileName) );
			}
			// else the file type already exist
			else
			{
				// id of the file type
				uint8 fileTypeId = (*itFTI).second;

				// last id used for this file type
				map<uint8,uint32>::iterator itTLI = TypeToLastId.find(fileTypeId);
				nlassert(itTLI != TypeToLastId.end());
				(*itTLI).second++;

				// add the new association
				TFormId fid;
				fid.FormIDInfos.Type = fileTypeId;
				fid.FormIDInfos.Id = (*itTLI).second;

				FormToId.insert( make_pair(fileName,fid) );
				IdToForm.insert( make_pair(fid,fileName) );
			}
		}
		else
		{
			nlinfo("Unknown file type for the file : %s",fileName.c_str());
		}
	}
	
} // addId //



//-----------------------------------------------
//	getFileType
//
//-----------------------------------------------
bool getFileType( string& fileName, string& fileType )
{
	sint idx = fileName.find_last_of(".");
	if( idx != -1 ) 
	{
		fileType = fileName.substr(idx+1,fileName.size()-1);
		return true;
	}
	else
	{
		return false;
	}

} // getFileType //



//-----------------------------------------------
//	display
//
//-----------------------------------------------
void display()
{
	cout<<"Output :"<<endl;
	map<TFormId,string>::iterator it1;
	for( it1 = IdToForm.begin(); it1 != IdToForm.end(); ++it1 )
	{
		cout<<"type: "<<(*it1).first.FormIDInfos.Type<<" id: "<<(*it1).first.FormIDInfos.Id<<" file: "<<(*it1).second<<endl;
	}

} // display //



//-----------------------------------------------
//	MAIN
//
//-----------------------------------------------
void main( int argc, char ** argv )
{
	// get the output filename
	if( argc < 2 )
	{
		displayHelp();
		return;
	}
	if( strcmp(argv[1],"/?") == 0 )
	{
		displayHelp();
		return;
	}
	string outputFileName(argv[1]);

	// get the input directories
	list<string> inputDirs;
	getInputDirectoryList( argc, argv, inputDirs);

	// get the current associations
	readFormId( outputFileName );
		
	// make the ids
	makeId( inputDirs );

	// save the new map
	COFile f( outputFileName );
	f.serialCont( IdToForm );

	// display the map
	display();

} // main //




