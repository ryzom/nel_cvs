/** \file path.cpp
 * CPath
 *
 * $Id: path.cpp,v 1.5 2001/01/18 14:14:52 berenguier Exp $
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
#define	NL_DISPLAY_PATH(_x_)	nlinfo("Patch: %s", _x_.c_str())
#else 
#define	NL_DISPLAY_PATH(_x_) NULL
#endif


namespace NLMISC {


CStringVector CPath::_SearchPaths;


/*
 * fileExists. Warning: this test will also tell that the file does not
 * exist if you don't have the rights to read it (Unix).
 */
bool fileExists( const string& filename )
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
string CPath::lookup( const string& filename )
{
	if ( fileExists(filename) )
	{
		NL_DISPLAY_PATH(filename);
		return filename;
	}
	CStringVector::iterator isv;
	string s;
	for ( isv=CPath::_SearchPaths.begin(); isv!=CPath::_SearchPaths.end(); ++isv )
	{
		s = *isv + filename;
		if ( fileExists(s) )
		{
			NL_DISPLAY_PATH(s);
			return s;
		}
	}
	throw EPathNotFound( filename );
	return "";
}

string CPath::findNewFile (const string &path)
{
	string start="", end="";
	char *dotpos = strrchr (path.c_str(), '.');
	if (dotpos == NULL) return path;
	uint pos= dotpos-path.c_str();

	uint i;
	for (i=0; i<pos; i++) start += path[i];
	for (i=0; i<path.size()-pos; i++) end += path[pos+i];

	uint num = 0;
	char numchar[4];
	string npath;
	do
	{
		npath = start;
		sprintf(numchar,"%03d",num++);
		npath += numchar;
		npath += end;
		if (!fileExists(npath)) return npath;
	}
	while (num<999);
	return npath;
}

} // NLMISC
