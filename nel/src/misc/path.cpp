/** \file path.cpp
 * Utility class for searching files in differents paths.
 *
 * $Id: path.cpp,v 1.24 2002/04/02 16:48:08 vizerie Exp $
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


#include "stdmisc.h"

#include <fstream>

#include "nel/misc/path.h"

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#	include <dirent.h>
#   include <unistd.h>
#endif // NL_OS_WINDOWS

using namespace std;

namespace NLMISC {

//
// Macros
//

// Use this define if you want to display info about the CPath.
//#define	NL_DEBUG_PATH

#ifdef	NL_DEBUG_PATH
#define	NL_DISPLAY_PATH	nlinfo
#else 
#define	NL_DISPLAY_PATH if(false)
#endif


//
// Variables
//

CPath *CPath::_Instance = NULL;


//
// Functions
//

CPath *CPath::getInstance ()
{
	if (_Instance == NULL)
	{
		_Instance = new CPath;
	}
	return _Instance;
}

void CPath::clearMap ()
{
	CPath *inst = CPath::getInstance();
	inst->_Files.clear ();
	NL_DISPLAY_PATH("CPath::clearMap(): map directory cleared");
}

sint CPath::findExtension (const string &ext1, const string &ext2)
{
	CPath *inst = CPath::getInstance();
	for (uint i = 0; i < inst->_Extensions.size (); i++)
	{
		if (inst->_Extensions[i].first == ext1 && inst->_Extensions[i].second == ext2)
		{
			return i;
		}
	}
	return -1;
}

void CPath::remapExtension (const string &ext1, const string &ext2, bool substitute)
{
	CPath *inst = CPath::getInstance();

	string ext1lwr = strlwr (ext1);
	string ext2lwr = strlwr (ext2);

	if (ext1lwr.empty() || ext2lwr.empty())
	{
		nlwarning ("CPath::remapExtension(%s, %s, %d): can't remap empty extension", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}

	if (!substitute)
	{
		// remove the mapping from the mapping list
		sint n = inst->findExtension (ext1lwr, ext2lwr);
		nlassert (n != -1);
		inst->_Extensions.erase (inst->_Extensions.begin() + n);

		// remove mapping in the map
		map<string, CFileEntry>::iterator it = inst->_Files.begin();
		map<string, CFileEntry>::iterator nit = it;
		while (it != inst->_Files.end ())
		{
			nit++;
			if ((*it).second.Remapped && (*it).second.Extension == ext2lwr)
			{
				inst->_Files.erase (it);
			}
			it = nit;
		}
		NL_DISPLAY_PATH("CPath::remapExtension(%s, %s, %d): extension removed", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}
	else
	{
		sint n = inst->findExtension (ext1lwr, ext2lwr);
		if (n != -1)
		{
			nlwarning ("CPath::remapExtension(%s, %s, %d): remapping already set", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
			return;
		}

		// adding mapping into the mapping list
		inst->_Extensions.push_back (make_pair (ext1lwr, ext2lwr));

		// adding mapping into the map
		vector<string> newFiles;
		map<string, CFileEntry>::iterator it = inst->_Files.begin();
		while (it != inst->_Files.end ())
		{
			if (!(*it).second.Remapped && (*it).second.Extension == ext1lwr)
			{
				// find if already exist
				sint pos = (*it).first.find_last_of (".");
				if (pos != string::npos)
				{
					string file = (*it).first.substr (0, pos + 1);
					file += ext2lwr;

					map<string, CFileEntry>::iterator nit = inst->_Files.find (file);
					if (nit != inst->_Files.end())
					{
						nlwarning ("CPath::remapExtension(%s, %s): The file '%s' is in conflict with the remapping file '%s', skip it", ext1lwr.c_str(), ext2lwr.c_str(), file.c_str(), (*it).first.c_str());
					}
					else
					{
// TODO perhaps a problem because I insert in the current map that i parcours
						insertFileInMap (file, (*it).second.Path, true, ext2lwr);
					}
				}
			}
			it++;
		}
		NL_DISPLAY_PATH("CPath::remapExtension(%s, %s, %d): extension added", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}
}

string CPath::lookup (const string &filename, bool throwException, bool displayWarning, bool lookupInLocalDirectory)
{
	// Try to find in the current directory
	if ( lookupInLocalDirectory && CFile::fileExists(filename) )
	{
		NL_DISPLAY_PATH("CPath::lookup(%s): found in the current directory: '%s'", filename.c_str(), filename.c_str());
		return filename;
	}

	// Try to find in the map directories
	CPath *inst = CPath::getInstance();
	string str = strlwr (filename);
	map<string, CFileEntry>::iterator it = inst->_Files.find (str);
	// If found in the map, returns it
	if (it != inst->_Files.end())
	{
		NL_DISPLAY_PATH("CPath::lookup(%s): found in the map directory: '%s'", filename.c_str(), (*it).second.Path.c_str());
		return (*it).second.Path;
	}
	

	// Try to find in the alternative directories
	for (uint i = 0; i < inst->_AlternativePaths.size(); i++)
	{
		string s = inst->_AlternativePaths[i] + filename;
		if ( CFile::fileExists(s) )
		{
			NL_DISPLAY_PATH("CPath::lookup(%s): found in the alternative directory: '%s'", filename.c_str(), s.c_str());
			return s;
		}
		
		// try with the remapping
		for (uint j = 0; j < inst->_Extensions.size(); j++)
		{
			string rs = inst->_AlternativePaths[i] + CFile::getFilenameWithoutExtension (filename) + "." + inst->_Extensions[j].first;
			if ( CFile::fileExists(rs) )
			{
				NL_DISPLAY_PATH("CPath::lookup(%s): found in the alternative directory: '%s'", filename.c_str(), rs.c_str());
				return rs;
			}
		}
	}


	// Not found
	if (displayWarning)
		nlwarning ("CPath::lookup(%s): file not found", filename.c_str());

	if (throwException)
		throw EPathNotFound (filename);

	return "";
}

string CPath::standardizePath (const string &path, bool addFinalSlash)
{
	string newPath;

	// check empty path
	if (path.empty()) return "";

	// don't transform the first \\ for windows network path
/*	if (path.size() >= 2 && path[0] == '\\' && path[1] == '\\')
	{
		newPath += "\\\\";
		i = 2;
	}
*/	
	for (uint i = 0; i < path.size(); i++)
	{
		// don't transform the first \\ for windows network path
		if (path[i] == '\\')
			newPath += '/';
		else
			newPath += path[i];
	}

	// add terminal slash
	if (addFinalSlash && newPath[path.size()-1] != '/')
		newPath += '/';

	return newPath;
}

#ifdef NL_OS_WINDOWS
#	define dirent	WIN32_FIND_DATA
#	define DIR		void

static string sDir;
static char sDirBackup[512];
static WIN32_FIND_DATA findData;
static HANDLE hFind;

DIR *opendir (const char *path)
{
	nlassert (path != NULL);
	nlassert (path[0] != '\0');

	nlassert (sDirBackup[0] == '\0');
	if (GetCurrentDirectory (512, sDirBackup) == 0)
	{
		// failed
		return NULL;
	}

	if (!CFile::isDirectory(path))
	{
		// failed
		return NULL;
	}
	
	sDir = path;

	hFind = NULL;
	
	return (void *)1;
}

int closedir (DIR *dir)
{
	nlassert (sDirBackup[0] != '\0');
	sDirBackup[0] = '\0';
	return 0;
}

dirent *readdir (DIR *dir)
{
	// set the current path
	nlassert (!sDir.empty());
	if (SetCurrentDirectory (sDir.c_str()) == 0)
	{
		// failed
		return NULL;
	}

	if (hFind == NULL)
	{
		hFind = FindFirstFile ("*", &findData);
	}
	else
	{
		if (!FindNextFile (hFind, &findData))
		{
			nlassert (sDirBackup[0] != '\0');
			SetCurrentDirectory (sDirBackup);
			return NULL;
		}
	}

	// restore the current path
	nlassert (sDirBackup[0] != '\0');
	SetCurrentDirectory (sDirBackup);

	return &findData;
}

#endif // NL_OS_WINDOWS

bool isdirectory (dirent *de)
{
	nlassert (de != NULL);
#ifdef NL_OS_WINDOWS
	return (de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
	return (de->d_type & DT_DIR) != 0;
#endif // NL_OS_WINDOWS
}

bool isfile (dirent *de)
{
	nlassert (de != NULL);
#ifdef NL_OS_WINDOWS
	return (de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
	return (de->d_type & DT_DIR) == 0;
#endif // NL_OS_WINDOWS
}

string getname (dirent *de)
{
	nlassert (de != NULL);
#ifdef NL_OS_WINDOWS
	return de->cFileName;
#else
	return de->d_name;
#endif // NL_OS_WINDOWS
}

void CPath::getPathContent (const string &path, bool recurse, bool wantDir, bool wantFile, vector<string> &result)
{
	DIR *dir = opendir (path.c_str());
	if (dir == NULL)
	{
		NL_DISPLAY_PATH("CPath::getPathContent(%s, %d, %d, %d): could not open the directory", path.c_str(), recurse, wantDir, wantFile);
		return;
	}

	// contains path that we have to recurs into
	vector<string> recursPath;

	while (true)
	{
		dirent *de = readdir(dir);
		if (de == NULL)
		{
			NL_DISPLAY_PATH("CPath::getPathContent(%s, %d, %d, %d): end of directory", path.c_str(), recurse, wantDir, wantFile);
			break;
		}

		// skip . and ..
		if (getname (de) == "." || getname (de) == "..")
			continue;

		if (isdirectory(de))
		{
			string stdName = standardizePath(path + getname(de));
			if (recurse)
			{
				NL_DISPLAY_PATH("CPath::getPathContent(%s, %d, %d, %d): need to recurse into '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
				recursPath.push_back (stdName);
			}

			if (wantDir)
			{
				NL_DISPLAY_PATH("CPath::getPathContent(%s, %d, %d, %d): adding path '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
				result.push_back (stdName);
			}
		}
		if (wantFile && isfile(de))
		{
			#ifdef NL_OS_WINDOWS
				string stdName = path + "\\" + getname(de);
			#else
				string stdName = path + "/" + getname(de);
			#endif
			NL_DISPLAY_PATH("CPath::getPathContent(%s, %d, %d, %d): adding file '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
			result.push_back (stdName);
		}
	}

	closedir (dir);

	// let s recurse
	for (uint i = 0; i < recursPath.size (); i++)
		getPathContent (recursPath[i], recurse, wantDir, wantFile, result);
}

void CPath::removeAllAlternativeSearchPath ()
{
	CPath *inst = CPath::getInstance();
	inst->_AlternativePaths.clear ();
	NL_DISPLAY_PATH("CPath::RemoveAllAternativeSearchPath(): removed");
}


void CPath::addSearchPath (const string &path, bool recurse, bool alternative)
{
	CPath *inst = CPath::getInstance();
	string newPath = standardizePath(path);

	// check empty directory
	if (newPath.empty())
	{
		nlwarning ("CPath::addSearchPath(%s, %d, %d): can't add empty directory, skip it", path.c_str(), recurse, alternative);
		return;
	}

	// check if it s a directory
	if (!CFile::isExists (newPath))
	{
		nlwarning ("CPath::addSearchPath(%s, %d, %d): '%s' is not found, skip it", path.c_str(), recurse, alternative, newPath.c_str());
		return;
	}

	// check if it s a directory
	if (!CFile::isDirectory (newPath))
	{
		nlwarning ("CPath::addSearchPath(%s, %d, %d): '%s' is not a directory, skip it", path.c_str(), recurse, alternative, newPath.c_str());
		return;
	}

	NL_DISPLAY_PATH("CPath::addSearchPath(%s, %d, %d): try to add '%s'", path.c_str(), recurse, alternative, newPath.c_str());

	if (alternative)
	{
		vector<string> pathsToProcess;

		// add the current path
		pathsToProcess.push_back (newPath);

		if (recurse)
		{
			// find all path and subpath
			getPathContent (newPath, recurse, true, false, pathsToProcess);
		}

		for (uint p = 0; p < pathsToProcess.size(); p++)
		{
			// check if the path not already in the vector
			uint i;
			for (i = 0; i < inst->_AlternativePaths.size(); i++)
			{
				if (inst->_AlternativePaths[i] == pathsToProcess[p])
					break;
			}
			if (i == inst->_AlternativePaths.size())
			{
				// add them in the alternative directory
				inst->_AlternativePaths.push_back (pathsToProcess[p]);
				NL_DISPLAY_PATH("CPath::addSearchPath(%s, %d, %d): path '%s' added", newPath.c_str(), recurse, alternative, pathsToProcess[p].c_str());
			}
			else
			{
				nlwarning ("CPath::addSearchPath(%s, %d, %d): path '%s' already added", newPath.c_str(), recurse, alternative, pathsToProcess[p].c_str());
			}
		}
	}
	else
	{
		vector<string> filesToProcess;
		// find all files in the path and subpaths
		getPathContent (newPath, recurse, false, true, filesToProcess);

		// add them in the map
		for (uint f = 0; f < filesToProcess.size(); f++)
		{
			string filename = CFile::getFilename (filesToProcess[f]);
			string filepath = CFile::getPath (filesToProcess[f]);
//			insertFileInMap (filename, filepath, false, CFile::getExtension(filename));
			addSearchFile (filesToProcess[f]);
		}
	}
}

void CPath::addSearchFile (const string &file, bool remap, const string &virtual_ext)
{
	CPath *inst = CPath::getInstance();
	string newFile = standardizePath(file, false);

	// check empty file
	if (newFile.empty())
	{
		nlwarning ("CPath::addSearchFile(%s, %d, %s): can't add empty file, skip it", file.c_str(), remap, virtual_ext.c_str());
		return;
	}

	// check if the file exists
	if (!CFile::isExists (newFile))
	{
		nlwarning ("CPath::addSearchFile(%s, %d, %s): '%s' is not found, skip it", file.c_str(), remap, virtual_ext.c_str(), newFile.c_str());
		return;
	}

	// check if it s a file
	if (CFile::isDirectory (newFile))
	{
		nlwarning ("CPath::addSearchFile(%s, %d, %s): '%s' is not a file, skip it", file.c_str(), remap, virtual_ext.c_str(), newFile.c_str());
		return;
	}

	string filenamewoext = CFile::getFilenameWithoutExtension (newFile);
	string filename, ext;
	
	if (virtual_ext.empty())
	{
		filename = CFile::getFilename (newFile);
		ext = CFile::getExtension (filename);
	}
	else
	{
		filename = filenamewoext + "." + virtual_ext;
		ext = virtual_ext;
	}

	map<string, CFileEntry>::iterator it = inst->_Files.find (filename);
	if (it == inst->_Files.end ())
	{
		// ok, the room is empty, let s add it
		insertFileInMap (filename, newFile, remap, ext);
	}
	else
	{
		if (remap)
			nlwarning ("CPath::addSearchFile(%s, %d, %s): remapped file '%s' already inserted in the map directory (location: %s)", file.c_str(), remap, virtual_ext.c_str(), filename.c_str(), (*it).second.Path.c_str());
		else
			nlwarning ("CPath::addSearchFile(%s, %d, %s): file '%s' already inserted in the map directory (location: %s)", file.c_str(), remap, virtual_ext.c_str(), filename.c_str(), (*it).second.Path.c_str());
	}

	if (!remap && !ext.empty())
	{
		// now, we have to see extension and insert in the map the remapped files
		for (uint i = 0; i < inst->_Extensions.size (); i++)
		{
			if (inst->_Extensions[i].first == strlwr(ext))
			{
				// need to remap
				addSearchFile (newFile, true, inst->_Extensions[i].second);
			}
		}
	}
}

void CPath::addSearchListFile (const string &filename, bool recurse, bool alternative)
{
	CPath *inst = CPath::getInstance();

	// check empty file
	if (filename.empty())
	{
		nlwarning ("CPath::addSearchListFile(%s, %d, %d): can't add empty file, skip it", filename.c_str(), recurse, alternative);
		return;
	}

	// check if the file exists
	if (!CFile::isExists (filename))
	{
		nlwarning ("CPath::addSearchListFile(%s, %d, %d): '%s' is not found, skip it", filename.c_str(), recurse, alternative, filename.c_str());
		return;
	}

	// check if it s a file
	if (CFile::isDirectory (filename))
	{
		nlwarning ("CPath::addSearchListFile(%s, %d, %d): '%s' is not a file, skip it", filename.c_str(), recurse, alternative, filename.c_str());
		return;
	}

	// TODO lire le fichier et ajouter les fichiers qui sont dedans

}


void CPath::addSearchBigFile (const string &filename, bool recurse, bool alternative)
{
	// TODO & CHECK
	nlwarning ("CPath::addSearchBigFile(): not impremented");
}

void CPath::insertFileInMap (const string &filename, const string &filepath, bool remap, const string &extension)
{
	CPath *inst = CPath::getInstance();

	// find if the file already exist
	map<string, CFileEntry>::iterator it = inst->_Files.find (strlwr(filename));
	if (it != inst->_Files.end ())
	{
		nlwarning ("CPath::insertFileInMap(%s, %s, %d, %s): already inserted from '%s', skip it", filename.c_str(), filepath.c_str(), remap, extension.c_str(), (*it).second.Path.c_str());
	}
	else
	{
		inst->_Files.insert (make_pair (strlwr(filename), CFileEntry (filepath, remap, strlwr(extension))));
		NL_DISPLAY_PATH("CPath::insertFileInMap(%s, %s, %d, %s): added", strlwr(filename).c_str(), filepath.c_str(), remap, strlwr(extension).c_str());
	}
}

void CPath::display ()
{
	CPath *inst = CPath::getInstance ();
	nlinfo ("Contents of the map:");
	nlinfo ("%-25s %-5s %-5s %s", "filename", "ext", "remap", "full path");
	nlinfo ("----------------------------------------------------");
	for (map<string, CFileEntry>::iterator it = inst->_Files.begin(); it != inst->_Files.end (); it++)
	{
		nlinfo ("%-25s %-5s %-5d %s", (*it).first.c_str(), (*it).second.Extension.c_str(), (*it).second.Remapped, (*it).second.Path.c_str());
	}
	nlinfo ("");
	nlinfo ("Contents of the alternative directory:");
	for (uint i = 0; i < inst->_AlternativePaths.size(); i++)
	{
		nlinfo ("'%s'", inst->_AlternativePaths[i].c_str ());
	}
	nlinfo ("");
	nlinfo ("Contents of the remapped entension table:");
	for (uint j = 0; j < inst->_Extensions.size(); j++)
	{
		nlinfo ("'%s' -> '%s'", inst->_Extensions[j].first.c_str (), inst->_Extensions[j].second.c_str ());
	}
	nlinfo ("End of display");
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

int CFile::getLastSeparator (const string &filename)
{
	int pos = filename.find_last_of ('/');
	if (pos == string::npos)
	{
		pos = filename.find_last_of ('\\');
	}
	return pos;
}

string CFile::getFilename (const string &filename)
{
	int pos = CFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (pos + 1);
	else
		return filename;
}

string CFile::getFilenameWithoutExtension (const string &filename)
{
	string filename2 = getFilename (filename);
	int pos = filename2.find_last_of ('.');
	if (pos == string::npos)
		return filename2;
	else
		return filename2.substr (0, pos);
}

string CFile::getExtension (const string &filename)
{
	int pos = filename.find_last_of ('.');
	if (pos == string::npos)
		return "";
	else
		return filename.substr (pos + 1);
}

string CFile::getPath (const string &filename)
{
	int pos = CFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (0, pos + 1);
	else
		return filename;
}

bool CFile::isDirectory (const string &filename)
{
#ifdef NL_OS_WINDOWS
	DWORD res = GetFileAttributes(filename.c_str());
	nlassert (res != -1);
	return (res & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else // NL_OS_WINDOWS
	struct stat buf;
	int result = stat (filename.c_str (), &buf);
	nlassert (result == 0);
	return (buf.st_mode & S_IFDIR) != 0;
#endif // NL_OS_WINDOWS
}

bool CFile::isExists (const string &filename)
{
#ifdef NL_OS_WINDOWS
	return (GetFileAttributes(filename.c_str()) != -1);
#else // NL_OS_WINDOWS
	struct stat buf;
	return stat (filename.c_str (), &buf) == 0;
#endif // NL_OS_WINDOWS
}

bool CFile::fileExists (const string& filename)
{
	return ! ! fstream( filename.c_str(), ios::in );
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


uint32	CFile::getFileModificationDate(const std::string &filename)
{
#if defined (NL_OS_WINDOWS)
	struct _stat buf;
	int result = _stat (filename.c_str (), &buf);
#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = stat (filename.c_str (), &buf);
#endif

	if (result != 0) return 0;
	else return buf.st_mtime;
}


uint32	CFile::getFileCreationDate(const std::string &filename)
{
#if defined (NL_OS_WINDOWS)
	struct _stat buf;
	int result = _stat (filename.c_str (), &buf);
#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = stat (filename.c_str (), &buf);
#endif

	if (result != 0) return 0;
	else return buf.st_ctime;
}




} // NLMISC
