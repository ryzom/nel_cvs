/** \file path_content.cpp
 *	list the path content with details on each files
 *
 * $Id: path_content_diff.cpp,v 1.2 2003/01/03 17:46:28 coutelas Exp $
 */

/* Copyright, 2003 Nevrax Ltd.
 *
 * This file is part of NEVRAX RYZOM.
 * NEVRAX RYZOM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX RYZOM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX RYZOM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/misc/path.h"
#include "nel/misc/debug.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;



//-----------------------------------------------
//	main
//
//-----------------------------------------------
sint main( sint argc, char ** argv )
{
	if( argc < 3 )
	{
		printf("\n");
		printf("Build a listing of the diff of two path contents\n\n");
		printf("PATH_CONTENT <ref path> <new path>\n");
		return 1;
	}
	
	// add ref path in search paths
	string refPath(argv[1]);
	CPath::addSearchPath(refPath, true, false);

	// content of ref path
	vector<string> refPathContent;
	CPath::getPathContent(refPath, true, true, true, refPathContent);

	// content of new path
	string newPath(argv[2]);
	vector<string> newPathContent;
	CPath::getPathContent(newPath, true, true, true, newPathContent);
	
	// open output file
	/*
	sint lastSeparator = CFile::getLastSeparator(newPath);
	if( lastSeparator != -1 )
	{
		newPath = newPath.substr(lastSeparator+1);
	}
	string outputFileName = newPath + ".txt";
	*/
	string outputFileName = "path_diff.txt";
	COFile output;
	if( !output.open(outputFileName,false,true) )
	{
		nlwarning("Can't open output file %s",outputFileName.c_str());
		return 1;
	}

	// get the list of new or modified files
	vector<string> differentFiles;
	vector<string>::const_iterator itFile;
	for( itFile = newPathContent.begin(); itFile != newPathContent.end(); ++itFile )
	{
		string newFileName = *itFile;
		string newFileNameShort = CFile::getFilename(newFileName);

		if( CFile::getExtension(newFileNameShort) == "bnp" )
		{
			nlwarning("%s is a big file, content of big files is not managed", newFileName.c_str());
		}
		
		bool keepIt = false;
		
		// check if file is new
		string refFileName = CPath::lookup(newFileNameShort, false, false, true);
		if( refFileName == "" )
		{
			keepIt = true;
		}

		uint32 refSize = CFile::getFileSize( refFileName );
		uint32 newSize = CFile::getFileSize( newFileName );
		if( refSize != newSize )
		{
			keepIt = true;
		}
		
		uint32 refModificationDate = CFile::getFileModificationDate( refFileName );
		uint32 newModificationDate = CFile::getFileModificationDate( newFileName );		
		if( newModificationDate > refModificationDate )
		{
			keepIt = true;
		}

		uint32 newCreationDate = CFile::getFileCreationDate( newFileName );

		if( keepIt )
		{
			differentFiles.push_back( newFileName );

			//string outputLine = newFileName + "\t\t"+toString(newSize) + "\t" + toString(newModificationDate) + "\t" + toString(newCreationDate) + "\n";
			//output.serialBuffer((uint8*)(const_cast<char*>(outputLine.data())),outputLine.size());
			output.serialBuffer((uint8*)(const_cast<char*>(newFileName.data())),newFileName.size());
		}
	}

	return 0;	
}

