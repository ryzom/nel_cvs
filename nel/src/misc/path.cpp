/** \file path.cpp
 * CPath
 *
 * $Id: path.cpp,v 1.3 2000/12/22 11:47:18 cado Exp $
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
//#include <stdio.h>
#include <fstream>

using namespace std;


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
		return filename;
	}
	CStringVector::iterator isv;
	string s;
	for ( isv=CPath::_SearchPaths.begin(); isv!=CPath::_SearchPaths.end(); ++isv )
	{
		s = *isv + filename;
		if ( fileExists(s) )
		{
			return s;
		}
	}
	throw EPathNotFound( filename );
	return "";
}


} // NLMISC
