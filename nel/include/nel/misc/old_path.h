#if 0
/** \file path.h
 * Abstract system directory architecture and file management
 *
 * $Id: old_path.h,v 1.3 2005/02/22 10:14:12 besson Exp $
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

#ifndef NL_PATH_H
#define NL_PATH_H

#include "types_nl.h"
#include "common.h"
#include <string>
#include <vector>


namespace NLMISC {


/// Vectors of strings
typedef std::vector<std::string> CStringVector;


// EPathNotFound
struct EPathNotFound : public Exception
{
	EPathNotFound( const std::string& filename ) : Exception( "Path not found for " + filename ) {}
};


/**
 * Utility class for search paths
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CPath
{
public:

	/// Adds a search path. The separator for directories is '/'.
	static void			addSearchPath( const std::string& path );

	/** Returns the long name (path and filename) for the specified file, trying first the local path, then 
	 * using search paths stored by addSearchPath in the same order as they were added.
	 * If no path is found where path/file exists, an exception EPathNotFound is raised if the boolean is true.
	 */
	static std::string	lookup( const std::string& filename, bool throwException = true );

private:

	static CStringVector	_SearchPaths;
};


/**
 * Utility class for file manipulation
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
struct CFile
{
	/**
	 * Retrieve the associated file name.
	 * An empty string is returned if the path is invalid
	 */
	static std::string getFilename (const std::string &filename);

	/**
	 * Retrieve the associated file path with the trailing slash.
	 * Returns an empty string if the path is invalid
	 */
	static std::string getPath (const std::string &filename);

	/**
	 * Just to know if it is a directory.
	 * _FileName empty and path not !!!
	 */
	static bool isDirectory (const std::string &filename);

	/**
	 * Return true if the file exists.
	 * Warning: this test will also tell that the file does not
	 * exist if you don't have the rights to read it (Unix).
	 */
	static bool fileExists (const std::string &filename);

	/**
	 * Return a new filename that doesn't exists. It's used for screenshot filename for example.
	 * example: findNewFile("foobar.tga");
	 * will try foobar001.tga, if the file exists, try foobar002.tga and so on until it finds an unexistant file.
	 */
	static std::string findNewFile (const std::string &filename);

	/**
	 * Return the position between [begin,end[ of the last separator between path and filename ('/' or '\').
	 * If there's no separator, it returns string::npos.
	 */
	static int getLastSeparator (const std::string &filename);
};


} // NLMISC


#endif // NL_PATH_H

/* End of path.h */
#endif
