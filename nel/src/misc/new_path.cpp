/** \file path.cpp
 * Utility class for searching files in differents paths.
 *
 * $Id: new_path.cpp,v 1.1 2001/11/22 11:08:27 lecroart Exp $
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

#include "nel/misc/debug.h"
#include <fstream>

#include "nel/misc/new_path.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;

namespace NLMISC {

//
// Macros
//

// Use this define if you want to display info about the CPath.
#define	NL_DEBUG_PATH

#ifdef	NL_DEBUG_PATH
#define	NL_DISPLAY_PATH	nlinfo
#else 
#define	NL_DISPLAY_PATH if(false)
#endif


//
// Variables
//

CNewPath *CNewPath::_Instance = NULL;


//
// Functions
//

CNewPath *CNewPath::getInstance ()
{
	if (_Instance == NULL)
	{
		_Instance = new CNewPath;
	}
	return _Instance;
}

void CNewPath::clearMap ()
{
	CNewPath *inst = CNewPath::getInstance();
	inst->_Files.clear ();
	NL_DISPLAY_PATH("CNewPath::clearMap(): map directory cleared");
}

sint CNewPath::findExtension (const string &ext1, const string &ext2)
{
	CNewPath *inst = CNewPath::getInstance();
	for (uint i = 0; i < inst->_Extensions.size (); i++)
	{
		if (inst->_Extensions[i].first == ext1 && inst->_Extensions[i].second == ext2)
		{
			return i;
		}
	}
	return -1;
}

void CNewPath::remapExtension (const string &ext1, const string &ext2, bool substitute)
{
	CNewPath *inst = CNewPath::getInstance();

	if (ext1.empty() || ext2.empty())
	{
		nlwarning ("CNewPath::remapExtension(%s, %s, %d): can't remap empty extension", ext1.c_str(), ext2.c_str(), substitute);
	}

	if (!substitute)
	{
		// remove the mapping from the mapping list
		sint n = inst->findExtension (ext1, ext2);
		nlassert (n != -1);
		inst->_Extensions.erase (inst->_Extensions.begin() + n);

		// remove mapping in the map
		std::map<std::string, CNewFileEntry>::iterator it = inst->_Files.begin();
		std::map<std::string, CNewFileEntry>::iterator nit = it;
		while (it != inst->_Files.end ())
		{
			nit++;
			if ((*it).second.Remapped && (*it).second.Extension == ext2)
			{
				inst->_Files.erase (it);
			}
			it = nit;
		}
		NL_DISPLAY_PATH("CNewPath::remapExtension(%s, %s, %d): extension removed", ext1.c_str(), ext2.c_str(), substitute);
	}
	else
	{
		sint n = inst->findExtension (ext1, ext2);
		if (n != -1)
		{
			nlwarning ("CNewPath::remapExtension(%s, %s, %d): remapping already set", ext1.c_str(), ext2.c_str(), substitute);
			return;
		}

		// adding mapping into the mapping list
		inst->_Extensions.push_back (make_pair (ext1, ext2));

		// adding mapping into the map
		vector<string> newFiles;
		std::map<std::string, CNewFileEntry>::iterator it = inst->_Files.begin();
		while (it != inst->_Files.end ())
		{
			if (!(*it).second.Remapped && (*it).second.Extension == ext1)
			{
				// find if already exist
				sint pos = (*it).first.find (".");
				if (pos != string::npos)
				{
					string file = (*it).first.substr (0, pos + 1);
					file += ext2;

					std::map<std::string, CNewFileEntry>::iterator nit = inst->_Files.find (file);
					if (nit != inst->_Files.end())
					{
						nlwarning ("CNewPath::remapExtension(%s, %s): The file '%s' is in conflict with the remapping file '%s', skip it", ext1.c_str(), ext2.c_str(), file.c_str(), (*it).first.c_str());
					}
					else
					{
// TODO perhaps a problem because I insert in the current map that i parcours
						insertFileInMap (file, (*it).second.Path, true, ext2);
					}
				}
			}
			it++;
		}
		NL_DISPLAY_PATH("CNewPath::remapExtension(%s, %s, %d): extension added", ext1.c_str(), ext2.c_str(), substitute);
	}
}

string CNewPath::lookup (const string &filename)
{
	// Try to find in the map directories
	CNewPath *inst = CNewPath::getInstance();
	std::map<std::string, CNewFileEntry>::iterator it = inst->_Files.find (filename);

	// If found in the map, returns it
	if (it != inst->_Files.end())
	{
		NL_DISPLAY_PATH("CNewPath::lookup(%s): found in the map directory: '%s'", filename.c_str(), (*it).second.Path.c_str());
		return (*it).second.Path;
	}
	
	// Try to find in the alternative directories
	for (uint i = 0; i < inst->_AlternativePaths.size(); i++)
	{
		string s = inst->_AlternativePaths[i] + filename;
		if ( CNewFile::fileExists(s) )
		{
			NL_DISPLAY_PATH("CNewPath::lookup(%s): found in the alternative directory: '%s'", filename.c_str(), s.c_str());
			return s;
		}
		
		// try with the remapping
		for (uint j = 0; j < inst->_Extensions.size(); j++)
		{
			string rs = inst->_AlternativePaths[i] + CNewFile::getFilenameWithoutExtension (filename) + "." + inst->_Extensions[j].first;
			if ( CNewFile::fileExists(rs) )
			{
				NL_DISPLAY_PATH("CNewPath::lookup(%s): found in the alternative directory: '%s'", filename.c_str(), rs.c_str());
				return s;
			}
		}
	}

	// Not found
	NL_DISPLAY_PATH("CNewPath::lookup(%s): file not found", filename.c_str());
	return "";
}

string CNewPath::standardizePath (const string &path)
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
	if (newPath[path.size()-1] != '/')
		newPath += '/';

	return newPath;
}

#define dirent WIN32_FIND_DATA
#define DIR	void

static string sDir;
static char sDirBackup[512];
static WIN32_FIND_DATA findData;
static HANDLE hFind;

DIR *opendir (const string &path)
{
	nlassert (!path.empty());

	nlassert (sDirBackup[0] == '\0');
	if (GetCurrentDirectory (512, sDirBackup) == 0)
	{
		// failed
		return NULL;
	}

	if (!CNewFile::isDirectory(path))
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

bool isdirectory (dirent *de)
{
	nlassert (de != NULL);
	return (de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool isfile (dirent *de)
{
	nlassert (de != NULL);
	return (de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

string getname (dirent *de)
{
	nlassert (de != NULL);
	return de->cFileName;
}

void CNewPath::getPathContent (const string &path, bool recurse, bool wantDir, bool wantFile, vector<string> &result)
{
	DIR *dir = opendir (path);
	if (dir == NULL)
	{
		NL_DISPLAY_PATH("CNewPath::getPathContent(%s, %d, %d, %d): could not open the directory", path.c_str(), recurse, wantDir, wantFile);
		return;
	}

	// contains path that we have to recurs into
	vector<string> recursPath;

	while (true)
	{
		dirent *de = readdir(dir);
		if (de == NULL)
		{
			NL_DISPLAY_PATH("CNewPath::getPathContent(%s, %d, %d, %d): end of directory", path.c_str(), recurse, wantDir, wantFile);
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
				NL_DISPLAY_PATH("CNewPath::getPathContent(%s, %d, %d, %d): need to recurse into '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
				recursPath.push_back (stdName);
			}

			if (wantDir)
			{
				NL_DISPLAY_PATH("CNewPath::getPathContent(%s, %d, %d, %d): adding path '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
				result.push_back (stdName);
			}
		}
		if (wantFile && isfile(de))
		{
			string stdName = path + getname(de);
			NL_DISPLAY_PATH("CNewPath::getPathContent(%s, %d, %d, %d): adding file '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
			result.push_back (stdName);
		}
	}

	closedir (dir);

	// let s recurse
	for (uint i = 0; i < recursPath.size (); i++)
		getPathContent (recursPath[i], recurse, wantDir, wantFile, result);
}

void CNewPath::addSearchPath (const std::string &path, bool recurse, bool alternative)
{
	CNewPath *inst = CNewPath::getInstance();

	// check empty directory
	if (path.empty())
	{
		nlwarning ("CNewPath::addSearchPath(%s, %d, %d): can't add empty directory, skip it", path.c_str(), recurse, alternative);
		return;
	}

	// check if it s a directory
	if (!CNewFile::isExists (path))
	{
		nlwarning ("CNewPath::addSearchPath(%s, %d, %d): '%s' is not found, skip it", path.c_str(), recurse, alternative, path.c_str());
		return;
	}

	// check if it s a directory
	if (!CNewFile::isDirectory (path))
	{
		nlwarning ("CNewPath::addSearchPath(%s, %d, %d): '%s' is not a directory, skip it", path.c_str(), recurse, alternative, path.c_str());
		return;
	}

	string newPath = standardizePath(path);
	NL_DISPLAY_PATH("CNewPath::addSearchPath(%s, %d, %d): try to add '%s'", newPath.c_str(), recurse, alternative, newPath.c_str());

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
				NL_DISPLAY_PATH("CNewPath::addSearchPath(%s, %d, %d): path '%s' added", newPath.c_str(), recurse, alternative, pathsToProcess[p].c_str());
			}
			else
			{
				nlwarning ("CNewPath::addSearchPath(%s, %d, %d): path '%s' already added", newPath.c_str(), recurse, alternative, pathsToProcess[p].c_str());
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
			string filename = CNewFile::getFilename (filesToProcess[f]);
			string filepath = CNewFile::getPath (filesToProcess[f]);
//			insertFileInMap (filename, filepath, false, CNewFile::getExtension(filename));
			addSearchFile (filesToProcess[f]);
		}
	}
}

void CNewPath::addSearchFile (const std::string &file, bool remap, const string &virtual_ext)
{
	CNewPath *inst = CNewPath::getInstance();

	// check empty file
	if (file.empty())
	{
		nlwarning ("CNewPath::addSearchFile(%s, %d, %s): can't add empty file, skip it", file.c_str(), remap, virtual_ext.c_str());
		return;
	}

	// check if the file exists
	if (!CNewFile::isExists (file))
	{
		nlwarning ("CNewPath::addSearchFile(%s, %d, %s): '%s' is not found, skip it", file.c_str(), remap, virtual_ext.c_str(), file.c_str());
		return;
	}

	// check if it s a file
	if (CNewFile::isDirectory (file))
	{
		nlwarning ("CNewPath::addSearchFile(%s, %d, %s): '%s' is not a file, skip it", file.c_str(), remap, virtual_ext.c_str(), file.c_str());
		return;
	}

	string filenamewoext = CNewFile::getFilenameWithoutExtension (file);
	string filename, ext;
	
	if (virtual_ext.empty())
	{
		filename = CNewFile::getFilename (file);
		ext = CNewFile::getExtension (file);
	}
	else
	{
		filename = filenamewoext + "." + virtual_ext;
		ext = virtual_ext;
	}

	std::map<std::string, CNewFileEntry>::iterator it = inst->_Files.find (filename);
	if (it == inst->_Files.end ())
	{
		// ok, the room is empty, let s add it
		insertFileInMap (filename, file, remap, ext);
	}
	else
	{
		if (remap)
			nlwarning ("CNewPath::addSearchPath(%s, %d, %s): remapped file '%s' already inserted in the map directory", file.c_str(), remap, virtual_ext.c_str(), filename.c_str());
		else
			nlwarning ("CNewPath::addSearchPath(%s, %d, %s): file '%s' already inserted in the map directory", file.c_str(), remap, virtual_ext.c_str(), filename.c_str());
	}

	if (!remap && !ext.empty())
	{
		// now, we have to see extension and insert in the map the remapped files
		for (uint i = 0; i < inst->_Extensions.size (); i++)
		{
			if (inst->_Extensions[i].first == ext)
			{
				// need to remap
				addSearchFile (file, true, inst->_Extensions[i].second);
			}
		}
	}
}

void CNewPath::addSearchListFile (const std::string &filename, bool recurse, bool alternative)
{
	CNewPath *inst = CNewPath::getInstance();

	// check empty file
	if (filename.empty())
	{
		nlwarning ("CNewPath::addSearchListFile(%s, %d, %d): can't add empty file, skip it", filename.c_str(), recurse, alternative);
		return;
	}

	// check if the file exists
	if (!CNewFile::isExists (filename))
	{
		nlwarning ("CNewPath::addSearchListFile(%s, %d, %d): '%s' is not found, skip it", filename.c_str(), recurse, alternative, filename.c_str());
		return;
	}

	// check if it s a file
	if (CNewFile::isDirectory (filename))
	{
		nlwarning ("CNewPath::addSearchListFile(%s, %d, %d): '%s' is not a file, skip it", filename.c_str(), recurse, alternative, filename.c_str());
		return;
	}

	// TODO lire le fichier et ajouter les fichiers qui sont dedans

}


void CNewPath::addSearchBigFile (const std::string &filename, bool recurse, bool alternative)
{
	// TODO & CHECK
	nlwarning ("CNewPath::addSearchBigFile(): not impremented");
}

void CNewPath::insertFileInMap (const string &filename, const string &filepath, bool remap, const string &extension)
{
	CNewPath *inst = CNewPath::getInstance();

	// find if the file already exist
	std::map<string, CNewFileEntry>::iterator it = inst->_Files.find (filename);
	if (it != inst->_Files.end ())
	{
		nlwarning ("CNewPath::insertFileInMap(%s, %s, %d, %s): already inserted from '%s', skip it", filename.c_str(), filepath.c_str(), remap, extension.c_str(), (*it).second.Path);
	}
	else
	{
		inst->_Files.insert (make_pair (filename, CNewFileEntry (filepath, remap, extension)));
		NL_DISPLAY_PATH("CNewPath::insertFileInMap(%s, %s, %d, %s): added", filename.c_str(), filepath.c_str(), remap, extension.c_str());
	}
}

void CNewPath::display ()
{
	CNewPath *inst = CNewPath::getInstance ();
	nlinfo ("Contents of the map:");
	nlinfo ("%-25s %-5s %-5s %s", "filename", "ext", "remap", "full path");
	nlinfo ("----------------------------------------------------");
	for (std::map<std::string, CNewFileEntry>::iterator it = inst->_Files.begin(); it != inst->_Files.end (); it++)
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
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
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
string CPath::lookup (const string &filename, bool throwException)
{
	if(!filename.empty())
	{
		if ( CNewFile::fileExists(filename) )
		{
			NL_DISPLAY_PATH(filename);
			return filename;
		}
		CStringVector::iterator isv;
		string s;
		for ( isv=CPath::_SearchPaths.begin(); isv!=CPath::_SearchPaths.end(); ++isv )
		{
			s = *isv + filename;
			if ( CNewFile::fileExists(s) )
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

#endif

//********************************* CNewFile

int CNewFile::getLastSeparator (const std::string &filename)
{
	int pos = filename.find_last_of ('/');
	if (pos == string::npos)
	{
		pos = filename.find_last_of ('\\');
	}
	return pos;
}

std::string CNewFile::getFilename (const std::string &filename)
{
	int pos = CNewFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (pos + 1);
	else
		return filename;
}

std::string CNewFile::getFilenameWithoutExtension (const std::string &filename)
{
	string filename2 = getFilename (filename);
	int pos = filename2.find_last_of ('.');
	if (pos == string::npos)
		return filename2;
	else
		return filename2.substr (0, pos);
}

std::string CNewFile::getExtension (const std::string &filename)
{
	int pos = filename.find_last_of ('.');
	if (pos == string::npos)
		return "";
	else
		return filename.substr (pos + 1);
}

std::string CNewFile::getPath (const std::string &filename)
{
	int pos = CNewFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (0, pos + 1);
	else
		return filename;
}

bool CNewFile::isDirectory (const std::string &filename)
{
	DWORD res = GetFileAttributes(filename.c_str());
	nlassert (res != -1);
	return (res & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool CNewFile::isExists (const string& filename)
{
	return (GetFileAttributes(filename.c_str()) != -1);
}

bool CNewFile::fileExists (const string& filename)
{
	return ! ! fstream( filename.c_str(), ios::in );
}

string CNewFile::findNewFile (const string &filename)
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
		if (!CNewFile::fileExists(npath)) break;
	}
	while (num<999);
	return npath;
}

} // NLMISC
