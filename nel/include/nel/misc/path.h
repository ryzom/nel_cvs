/** \file path.h
 * Utility class for searching files in differents paths.
 *
 * $Id: path.h,v 1.21 2002/04/26 10:00:15 lecroart Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/misc/types_nl.h"

#include <map>
#include <string>
#include <vector>

#include "nel/misc/common.h"

namespace NLMISC {

/// Exception throwed when a find is not found in a lookup() call
struct EPathNotFound : public Exception
{
	EPathNotFound (const std::string& filename) : Exception ("Path not found for " + filename) { }
};

/**
 * Utility class for searching files in differents paths.
 * \warning addSearchPath(), clearMap() and remapExtension() are not reentrant.
 * \warning all path and files are *case sensitive* on linux.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CPath
{
public:
	/** Adds a search path.
     * The path is a directory "c:/temp" all files in the directory will be included (and recursively if asked)
	 *
	 * Alternative directories are not precached (instead of non Alternative files) and will used when a file is not found in the standard directories.
	 * For example, local data will be in the cached directories and server repository files will be in the Alternative files. If a new file is not
	 * found in the local data, we'll try to find it on the repositrory.
	 *
	 * When Alternative is false, all added file names must be uniq or a warning will be display. In the Alternative directories, it could have
	 * more than one file with the same name.
	 *
	 * \warning the path you provide is case sensitive, you must be sure that the path name is exactly the same
	 *
	 * \param path the path name. The separator for directories could be '/' or '\' (bit '\' will be translate into '/' in the function).
	 * \param recurse true if you want the function recurse in sub-directories.
	 * \param Alternative true if you want to add the path in the Alternative directories.
	 */
	static void			addSearchPath (const std::string &path, bool recurse, bool alternative);

	/** Used only for compatibility with the old CPath. In this case, we don't use the map to have the same behavior as the old CPath */
	static void			addSearchPath (const std::string &path) { addSearchPath (path, false, true); }

	/** Same as AddSearchPath but with a file "c:/autoexec.bat" this file only will included. wildwards *doesn't* work */
	static void			addSearchFile (const std::string &file, bool remap = false, const std::string &virtual_ext = "");

	/** Same as AddSearchPath but with a path file "c:/test.pth" all files name contain in this file will be included (the extention is used to know that it's a path file) */
	static void			addSearchListFile (const std::string &filename, bool recurse, bool alternative);
	
	/** Same as AddSearchPath but with a big file "c:/test.nbf" all files name contain in the big file will be included  (the extention (Nel Big File) is used to know that it's a big file) */
	static void			addSearchBigFile (const std::string &filename, bool recurse, bool alternative);

	/** Remove all search path contains in the alternative directories */
	static void			removeAllAlternativeSearchPath ();

	/** Returns the long name (path + filename) for the specified file.
	 * The directory separator is always '/'.
	 * First, the lookup() lookups in standard directories (Alternative=false).
	 * If not found, it lookups in the Alternative directories.
	 * If not found the lookup() returns empty string "" (and generate an exception if throwException is true)
	 *
	 * The filename is not case sensitive so if the real filename is "FooBAR.Jpg" and you call lookup("fOOBar.jPg"), it'll
	 * return the real filename "FooBAR.Jpg"
	 *
	 * \param filename the file name you are seeking. (ex: "test.txt")
	 * \param throwException used for backward compatibility, set to true to generate an EPathNotFound.
	 * \param displayWarning set to false if you don't want the function displays a warning if the file is not found
	 * \param lookupInLocalDirectory if true, the lookup() will first try to open the file without path.
	 * \return empty string if file is not found or the full path + file name (ex: "c:/temp/test.txt");
	 */
	static std::string	lookup (const std::string &filename, bool throwException = true, bool displayWarning = true, bool lookupInLocalDirectory = true);

	/** Clears the map that contains all cached files (Use this function to take into account new files).
	 */
	static void clearMap ();

	/** Add a remapping function to allow file extension substitution.
	 * - eg remapExtension("dds", "tga", true) Where the boolean indicates whether
	 * the "dds" should replace a "tga" if one exists - again - a warning should
	 * be generated if the two are present.
	 *
	 * ie: If you have a file called pic.dds and you call remapExtension("dds", "tga", true),
	 *     if you call lookup("pic.tga"), it'll return "pic.dds"
	 *
	 */
	static void remapExtension (const std::string &ext1, const std::string &ext2, bool substitute);

	static void display ();

	/**	Take a path and put it in the portable format and add a terminated / if needed
	 * ie: "C:\\Game/dir1" will become "C:/Game/dir1/" or "C:/Game/dir1" if addFinalSlash is false
	 */
	static std::string	standardizePath (const std::string &path, bool addFinalSlash = true);

	/** List all files in a directory.
	 *	\param path path where files are scanned (MUST NOT BE EMPTY)
	 *	\param recurse true if want to recurs directories
	 *	\param wantDir true if want to add directorires in result
	 *	\param wantFile true if want to add files in result
	 *	\param result list of string where directories/files names are added.
	 */
	static void			getPathContent (const std::string &path, bool recurse, bool wantDir, bool wantFile, std::vector<std::string> &result);

	/** Get the full path based on a file/path and the current directory. Example, imagine that the current path is c:\temp and toto is a directory
	 * GetFullPath ("toto") returns "c:/temp/toto/"
	 * GetFullPath ("../toto") returns "c:/temp/../toto/"
	 * GetFullPath ("d:\dir\toto") returns "d:/dir/toto/"
	 * GetFullPath ("\toto") returns "c:/toto/"
	 * GetFullPath ("") returns "c:/temp/"
	 *
	 * \param path the path
	 * \return the full path
	 */
	static std::string getFullPath (const std::string &path, bool addFinalSlash = true);

	/** Returns the current path of the applicated.
	 */
	static std::string getCurrentPath ();

private:

	static CPath *getInstance ();

	static CPath *_Instance;

	// All path in this vector must have a terminated '/'
	std::vector<std::string> _AlternativePaths;

	struct CFileEntry
	{
		CFileEntry (std::string	path, bool remapped, std::string ext) : Path(path), Remapped(remapped), Extension(ext) { }
		std::string	Path;
		bool		Remapped;		// true if the file is remapped
		std::string	Extension;		// extention of the faile
	};

	/** first is the filename, second the full path for the filename.
	 * Due to the remapping, first and second.path could have different extention.
	 */
	std::map<std::string, CFileEntry> _Files;

	/// first ext1, second ext2 (ext1 could remplace ext2)
	std::vector<std::pair<std::string, std::string> > _Extensions;

	sint				findExtension (const std::string &ext1, const std::string &ext2);
	static void			insertFileInMap (const std::string &filename, const std::string &filepath, bool remap, const std::string &extension);
};



/**
 * Utility class for file manipulation
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
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
	 * Return true if the file OR directory exists.
	 * Warning: this test will also tell that the file does not
	 * exist if you don't have the rights to read it (Unix).
	 */
	static bool isExists (const std::string& filename);

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

	static std::string getFilenameWithoutExtension (const std::string &filename);
	static std::string getExtension (const std::string &filename);

	/**
	 *	Return Time of last modification of file. 0 if not found.
	 */
	static uint32	getFileModificationDate(const std::string &filename);

	/**
	 *	Return creation Time of the file. 0 if not found.
	 */
	static uint32	getFileCreationDate(const std::string &filename);


};




} // NLMISC


#endif // NL_PATH_H

/* End of path.h */
