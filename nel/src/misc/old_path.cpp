#if 0


/** \file path.cpp
 * CPath
 *
 * $Id: old_path.cpp,v 1.4 2002/04/15 08:43:25 lecroart Exp $
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

#include "nel/misc/path.h"
#include "nel/misc/debug.h"
//#include <stdio.h>
#include <fstream>

using namespace std;


// Use this define if you want to display the absolute paths in the console.
//#define	NL_DEBUG_PATH

#ifdef	NL_DEBUG_PATH
#define	NL_DISPLAY_PATH(_x_)	nlinfo("Path: %s", _x_.c_str())
#else 
#define	NL_DISPLAY_PATH(_x_)	NULL
#endif


namespace NLMISC {


CStringVector CPath::_SearchPaths;



/*
 * Adds a search path
 */
void CPath::addSearchPath( const string& path )
{
	if ( path == "" )
	{
		return;
	}
	string s = path;
	const char slash = '/';

	// Add an ending slash if necessary
	if ( path[path.size()-1] != slash )
	{
		s += slash;
	}

	// Add path to the search paths
	_SearchPaths.push_back( s );
}


/* Returns the long name (path and filename) for the specified file, using search paths
 * stored by addSearchPath.
 */
string CPath::lookup( const string& filename, bool throwException )
{
	if(!filename.empty())
	{
		if ( CFile::fileExists(filename) )
		{
			NL_DISPLAY_PATH(filename);
			return filename;
		}
		CStringVector::iterator isv;
		string s;
		for ( isv=CPath::_SearchPaths.begin(); isv!=CPath::_SearchPaths.end(); ++isv )
		{
			s = *isv + filename;
			if ( CFile::fileExists(s) )
			{
				NL_DISPLAY_PATH(s);
				return s;
			}
		}
	}

	if (throwException)
		throw EPathNotFound( filename );

	return "";
}

//********************************* CFile

int CFile::getLastSeparator (const std::string &filename)
{
	int pos = filename.find_last_of ('/');
	if (pos == string::npos)
	{
		pos = filename.find_last_of ('\\');
	}
	return pos;
}

std::string CFile::getFilename (const std::string &filename)
{
	int pos = CFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (pos + 1);
	else
		return filename;
}

std::string CFile::getPath (const std::string &filename)
{
	int pos = CFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (0, pos + 1);
	else
		return filename;
}

bool CFile::isDirectory (const std::string &filename)
{
	return (CFile::getLastSeparator(filename) == string::npos);
}


bool CFile::fileExists (const string& filename)
{
	/*FILE *f;
	if ( (f = fopen( filename.c_str(), "r" )) == NULL )
	{
		return false;
	}
	else
	{
		fclose( f );
		return true;
	}*/
	return ! ! fstream( filename.c_str(), ios::in ); // = ! fstream(...).fail()
}

string CFile::findNewFile (const string &filename)
{
	int pos = filename.find_last_of ('.');
	if (pos == string::npos)
		return filename;
	
	string start = filename.substr (0, pos);
	string end = filename.substr (pos);

	uint num = 0;
	char numchar[4];
	string npath;
	do
	{
		npath = start;
		smprintf(numchar,4,"%03d",num++);
		npath += numchar;
		npath += end;
		if (!CFile::fileExists(npath)) break;
	}
	while (num<999);
	return npath;
}

} // NLMISC
#endif
