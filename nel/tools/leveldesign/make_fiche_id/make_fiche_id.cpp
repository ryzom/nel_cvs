/** \file make_fiche_id.cpp
 * 
 *
 * $Id: make_fiche_id.cpp,v 1.1 2002/02/07 16:30:35 coutelas Exp $
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



//	getInputDirectoryList
void getInputDirectoryList( int argc, char **argv, list<string>& dirs );

//	getInputDirectoryList
void readFicheId( string& outputFileName, map<string,uint32>& ficheIds, uint32& lastId );

// makeId
void makeId( list<string>& dirs, map<string,uint32>& ficheIds, uint32& lastId );

//manageFile
void manageFile( WIN32_FIND_DATA& findFileData, list<string>& dirs, string& currentDir, map<string,uint32>& ficheIds, uint32& lastId );

// addId
void addId( string fileName, map<string,uint32>& ficheIds, uint32& lastId );

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
//	readFicheId
//
//-----------------------------------------------
void readFicheId( string& outputFileName, map<string,uint32>& ficheIds, uint32& lastId )
{
	CIFile f;
	if( f.open( outputFileName ) )
	{
		f.serial( lastId );
		f.serialCont( ficheIds );
	}
	else
	{
		lastId = 0;
	}

} // readFicheId //



//-----------------------------------------------
//	makeId
//
//-----------------------------------------------
void makeId( list<string>& dirs, map<string,uint32>& ficheIds, uint32& lastId )
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
				manageFile( findFileData, dirs, currentDir, ficheIds, lastId );
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
void manageFile( WIN32_FIND_DATA& findFileData, list<string>& dirs, string& currentDir, map<string,uint32>& ficheIds, uint32& lastId )
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
			addId( string(findFileData.cFileName), ficheIds, lastId );
		}
	}

} // manageFile //



//-----------------------------------------------
//	addId
//
//-----------------------------------------------
void addId( string fileName, map<string,uint32>& ficheIds, uint32& lastId )
{
	map<string,uint32>::iterator itFch = ficheIds.find( fileName );
	if( itFch == ficheIds.end() )
	{
		lastId++;
		ficheIds.insert( make_pair(fileName,lastId) );
	}

} // addId //





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
	map<string,uint32> ficheIds;
	uint32 lastId = 0;
	readFicheId(outputFileName, ficheIds, lastId);
	
	// make the ids
	makeId( inputDirs, ficheIds, lastId );

	// save the new map
	COFile f( outputFileName );
	f.serial( lastId );
	f.serialCont( ficheIds );

} // main //




