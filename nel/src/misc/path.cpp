/** \file path.cpp
 * Utility class for searching files in differents paths.
 *
 * $Id: path.cpp,v 1.60 2002/10/02 15:50:38 lecroart Exp $
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

#include "nel/misc/big_file.h"
#include "nel/misc/path.h"
#include "nel/misc/hierarchical_timer.h"

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <direct.h>
#else
#   include <sys/types.h>
#   include <sys/stat.h>
#	include <dirent.h>
#   include <unistd.h>
#   include <errno.h>
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
#ifdef __GNUC__
#define	NL_DISPLAY_PATH(format, args...)
#else // __GNUC__
#define	NL_DISPLAY_PATH if(false)
#endif // __GNUC__
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

	if (ext1lwr == "bnp" || ext2lwr == "bnp")
	{
		nlwarning ("CPath::remapExtension(%s, %s, %d): you can't remap a big file", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
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
				uint32 pos = (*it).first.find_last_of (".");
				if (pos != string::npos)
				{
					string file = (*it).first.substr (0, pos + 1);
					file += ext2lwr;

// TODO perhaps a problem because I insert in the current map that i parcours
					insertFileInMap (file, (*it).second.Path, true, ext2lwr);
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

	// If the file already contains a @, it means that a lookup already proceed and returning a big file, do nothing
	if (filename.find ("@") != string::npos)
	{
		NL_DISPLAY_PATH("CPath::lookup(%s):	already found", filename.c_str());
		return filename;
	}

	// Try to find in the map directories
	CPath *inst = CPath::getInstance();
	string str = strlwr (filename);

	// Remove end spaces
	while ((!str.empty()) && (str[str.size()-1] == ' '))
	{
		str.resize (str.size()-1);
	}

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
			if (strlwr(CFile::getExtension (filename)) == inst->_Extensions[j].second)
			{
				string rs = inst->_AlternativePaths[i] + CFile::getFilenameWithoutExtension (filename) + "." + inst->_Extensions[j].first;
				if ( CFile::fileExists(rs) )
				{
					NL_DISPLAY_PATH("CPath::lookup(%s): found in the alternative directory: '%s'", filename.c_str(), rs.c_str());
					return rs;
				}
			}
		}
	}


	// Not found
	if (displayWarning)
	{
		nlwarning ("CPath::lookup(%s): file not found", filename.c_str());
	}

	if (throwException)
		throw EPathNotFound (filename);

	return "";
}

bool CPath::exists (const std::string &filename)
{
	// Try to find in the map directories
	CPath *inst = CPath::getInstance();
	string str = strlwr (filename);

	// Remove end spaces
	while ((!str.empty()) && (str[str.size()-1] == ' '))
	{
		str.resize (str.size()-1);
	}

	map<string, CFileEntry>::iterator it = inst->_Files.find (str);
	// If found in the map, returns it
	if (it != inst->_Files.end())
	{
		return true;
	}

	return false;
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

// remplace / wiht \ and put all in lower case
std::string	CPath::standardizeDosPath (const std::string &path)
{
	string newPath;

	for (uint i = 0; i < path.size(); i++)
	{
		if (path[i] == '/')
			newPath += '\\';
		else if (isupper(path[i]))
			newPath += tolower(path[i]);
		else
			newPath += path[i];
	}

	if (CFile::isDirectory(path) && newPath[newPath.size()-1] != '\\')
		newPath += '\\';

	return newPath;
}


std::string CPath::getCurrentPath ()
{
	char buffer [10000];

#ifdef NL_OS_WINDOWS
	return _getcwd(buffer, 10000);
#else
	return getcwd(buffer, 10000);
#endif
}

std::string CPath::getFullPath (const std::string &path, bool addFinalSlash)
{
	string currentPath = standardizePath (getCurrentPath ());
	string sPath = standardizePath (path, addFinalSlash);

	// current path
	if (path.empty())
	{
		return currentPath;
	}

	// windows full path
	if (path.size() > 2 && path[1] == ':')
	{
		return sPath;
	}

	// from root
	if (path [0] == '/' || path[0] == '\\')
	{
		if (currentPath.size() > 2 && currentPath[1] == ':')
		{
			return currentPath.substr(0,3) + sPath.substr(1);
		}
		else
		{
			return sPath;
		}
	}

	// default case
	return currentPath + sPath;
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
		sDirBackup[0] = 0;
		return NULL;
	}


	if (!CFile::isDirectory(path))
	{
		// failed
		sDirBackup[0] = 0;
		return NULL;
	}
	
	sDir = path;

	hFind = NULL;
	
	return (void *)1;
}

int closedir (DIR *dir)
{
	nlassert (sDirBackup[0] != '\0');
	FindClose(hFind);
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

#ifndef NL_OS_WINDOWS
string BasePathgetPathContent;
#endif

bool isdirectory (dirent *de)
{
	nlassert (de != NULL);
#ifdef NL_OS_WINDOWS
	return ((de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) && ((de->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0);
#else
	//nlinfo ("isdirectory filename %s -> 0x%08x", de->d_name, de->d_type);
	// we can't use "de->d_type & DT_DIR" because it s always NULL on libc2.1
	//return (de->d_type & DT_DIR) != 0;

	return CFile::isDirectory (BasePathgetPathContent + de->d_name);

#endif // NL_OS_WINDOWS
}

bool isfile (dirent *de)
{
	nlassert (de != NULL);
#ifdef NL_OS_WINDOWS
	return ((de->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) && ((de->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == 0);
#else
	// we can't use "de->d_type & DT_DIR" because it s always NULL on libc2.1
	//return (de->d_type & DT_DIR) == 0;

	return !CFile::isDirectory (BasePathgetPathContent + de->d_name);

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
#ifndef NL_OS_WINDOWS
	BasePathgetPathContent = CPath::standardizePath (path);
#endif

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

		string fn = getname (de);

		// skip . and ..
		if (fn == "." || fn == "..")
			continue;

		if (isdirectory(de))
		{
			// skip CVS directory
			if (fn == "CVS")
			{
				NL_DISPLAY_PATH("CPath::getPathContent(%s, %d, %d, %d): skip CVS directory", path.c_str(), recurse, wantDir, wantFile);
				continue;
			}

			string stdName = standardizePath(standardizePath(path) + fn);
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
			if (fn.size() >= 4 && fn.substr (fn.size()-4) == ".log")
			{
				NL_DISPLAY_PATH("CPath::getPathContent(%s, %d, %d, %d): skip *.log files (%s)", path.c_str(), recurse, wantDir, wantFile, fn.c_str());
				continue;
			}

/*			int lastSep = CFile::getLastSeparator(path);
			#ifdef NL_OS_WINDOWS
				char sep = lastSep == std::string::npos ? '\\'
													    : path[lastSep];
			#else
				char sep = lastSep == std::string::npos ? '/'
														: path[lastSep];
			#endif
*/			
			string stdName = standardizePath(path) + getname(de);
			
				
			NL_DISPLAY_PATH("CPath::getPathContent(%s, %d, %d, %d): adding file '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
			result.push_back (stdName);
		}
	}

	closedir (dir);

#ifndef NL_OS_WINDOWS
	BasePathgetPathContent = "";
#endif

	// let s recurse
	for (uint i = 0; i < recursPath.size (); i++)
	{		
		getPathContent (recursPath[i], recurse, wantDir, wantFile, result);
	}
}

void CPath::removeAllAlternativeSearchPath ()
{
	CPath *inst = CPath::getInstance();
	inst->_AlternativePaths.clear ();
	NL_DISPLAY_PATH("CPath::RemoveAllAternativeSearchPath(): removed");
}


void CPath::addSearchPath (const string &path, bool recurse, bool alternative)
{
	H_AUTO_INST(addSearchPath);

	CPath *inst = CPath::getInstance();

	// check empty directory
	if (path.empty())
	{
		nlwarning ("CPath::addSearchPath(%s, %d, %d): can't add empty directory, skip it", path.c_str(), recurse, alternative);
		return;
	}

	// check if it s a directory
	if (!CFile::isDirectory (path))
	{
		nlinfo ("CPath::addSearchPath(%s, %d, %d): '%s' is not a directory, I'll call addSearchFile()", path.c_str(), recurse, alternative, path.c_str());
		addSearchFile (path);
		return;
	}

	string newPath = standardizePath(path);

	// check if it s a directory
	if (!CFile::isExists (newPath))
	{
		nlwarning ("CPath::addSearchPath(%s, %d, %d): '%s' is not found, skip it", path.c_str(), recurse, alternative, newPath.c_str());
		return;
	}

	nlinfo ("CPath::addSearchPath(%s, %d, %d): adding the path '%s'", path.c_str(), recurse, alternative, newPath.c_str());

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

	// check if it s a big file
	if (CFile::getExtension(newFile) == "bnp")
	{
		NL_DISPLAY_PATH ("CPath::addSearchFile(%s, %d, %s): '%s' is a big file, add it", file.c_str(), remap, virtual_ext.c_str(), newFile.c_str());
		addSearchBigFile(file, false, false);
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

	insertFileInMap (filename, newFile, remap, ext);

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

// WARNING : recurse is not used
void CPath::addSearchBigFile (const string &sBigFilename, bool recurse, bool alternative)
{
	// Check if filename is not empty
	if (sBigFilename.empty())
	{
		nlwarning ("CPath::addSearchBigFile(%s, %d, %d): can't add empty file, skip it", sBigFilename.c_str(), recurse, alternative);
		return;
	}
	// Check if the file exists
	if (!CFile::isExists (sBigFilename))
	{
		nlwarning ("CPath::addSearchBigFile(%s, %d, %d): '%s' is not found, skip it", sBigFilename.c_str(), recurse, alternative, sBigFilename.c_str());
		return;
	}
	// Check if it s a file
	if (CFile::isDirectory (sBigFilename))
	{
		nlwarning ("CPath::addSearchBigFile(%s, %d, %d): '%s' is not a file, skip it", sBigFilename.c_str(), recurse, alternative, sBigFilename.c_str());
		return;
	}
	// Open and read the big file header
	CPath *inst = CPath::getInstance();

	FILE *Handle = fopen (sBigFilename.c_str(), "rb");
	if (Handle == NULL)
	{
		nlwarning ("CPath::addSearchBigFile(%s, %d, %d): can't open file, skip it", sBigFilename.c_str(), recurse, alternative);
		return;
	}

	// add the link with the CBigFile singleton
	CBigFile::getInstance().add (sBigFilename, BF_ALWAYS_OPENED | BF_CACHE_FILE_ON_OPEN);

	// parse the big file to add file in the map
	fseek (Handle, 0, SEEK_END);
	uint32 nFileSize = ftell (Handle);
	fseek (Handle, nFileSize-4, SEEK_SET);
	uint32 nOffsetFromBegining;
	fread (&nOffsetFromBegining, sizeof(uint32), 1, Handle);
	fseek (Handle, nOffsetFromBegining, SEEK_SET);
	uint32 nNbFile;
	fread (&nNbFile, sizeof(uint32), 1, Handle);
	for (uint32 i = 0; i < nNbFile; ++i)
	{
		char FileName[256];
		uint8 nStringSize;
		fread (&nStringSize, 1, 1, Handle);
		fread (FileName, 1, nStringSize, Handle);
		FileName[nStringSize] = 0;
		uint32 nFileSize;
		fread (&nFileSize, sizeof(uint32), 1, Handle);
		uint32 nFilePos;
		fread (&nFilePos, sizeof(uint32), 1, Handle);
		string sTmp = strlwr(string(FileName));
		if (sTmp.empty())
		{
			nlwarning ("CPath::addSearchBigFile(%s, %d, %d): can't add empty file, skip it", sBigFilename.c_str(), recurse, alternative);
			continue;
		}
		string bigfilenamealone = CFile::getFilename (sBigFilename);
		string filenamewoext = CFile::getFilenameWithoutExtension (sTmp);
		string ext = strlwr(CFile::getExtension(sTmp));

		insertFileInMap (sTmp, bigfilenamealone + "@" + sTmp, false, ext);

		for (uint j = 0; j < inst->_Extensions.size (); j++)
		{
			if (inst->_Extensions[j].first == ext)
			{
				// need to remap
				insertFileInMap (filenamewoext+"."+inst->_Extensions[j].second, 
								bigfilenamealone + "@" + sTmp, 
								true, 
								inst->_Extensions[j].second);
			}
		}

	}
	fclose (Handle);
}

void CPath::insertFileInMap (const string &filename, const string &filepath, bool remap, const string &extension)
{
	CPath *inst = CPath::getInstance();

	// find if the file already exist
	map<string, CFileEntry>::iterator it = inst->_Files.find (strlwr(filename));
	if (it != inst->_Files.end ())
	{
		if ((*it).second.Path.find("@") != string::npos && filepath.find("@") == string::npos)
		{
			// if there's a file in a big file and a file in a path, the file in path wins
			// remplace with the new one
			nlinfo ("CPath::insertFileInMap(%s, %s, %d, %s): already inserted from '%s' but special case so overide it", filename.c_str(), filepath.c_str(), remap, extension.c_str(), (*it).second.Path.c_str());
			(*it).second.Path = filepath;
			(*it).second.Remapped = remap;
			(*it).second.Extension = extension;
		}
		else
		{
			nlwarning ("CPath::insertFileInMap(%s, %s, %d, %s): already inserted from '%s', skip it", filename.c_str(), filepath.c_str(), remap, extension.c_str(), (*it).second.Path.c_str());
		}
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
	uint32 pos = filename.find_last_of ('/');
	if (pos == string::npos)
	{
		pos = filename.find_last_of ('\\');
		if (pos == string::npos)
		{
			pos = filename.find_last_of ('@');
		}
	}
	return pos;
}

string CFile::getFilename (const string &filename)
{
	uint32 pos = CFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (pos + 1);
	else
		return filename;
}

string CFile::getFilenameWithoutExtension (const string &filename)
{
	string filename2 = getFilename (filename);
	uint32 pos = filename2.find_last_of ('.');
	if (pos == string::npos)
		return filename2;
	else
		return filename2.substr (0, pos);
}

string CFile::getExtension (const string &filename)
{
	uint32 pos = filename.find_last_of ('.');
	if (pos == string::npos)
		return "";
	else
		return filename.substr (pos + 1);
}

string CFile::getPath (const string &filename)
{
	uint32 pos = CFile::getLastSeparator(filename);
	if (pos != string::npos)
		return filename.substr (0, pos + 1);
	else
		return "";
}

bool CFile::isDirectory (const string &filename)
{
#ifdef NL_OS_WINDOWS
	DWORD res = GetFileAttributes(filename.c_str());
	if (res == -1)
	{
		nlwarning ("%s is not a valid file / directory name", filename);
		return false;
	}
	return (res & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else // NL_OS_WINDOWS
	struct stat buf;
	int res = stat (filename.c_str (), &buf);
	if (res == -1)
	{
		nlwarning ("can't stat '%s' error %d '%s'", filename.c_str(), errno, strerror(errno));
		return false;
	}
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
	uint32 pos = filename.find_last_of ('.');
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

// \warning doesn't work with big file
uint32	CFile::getFileSize (const std::string &filename)
{
	FILE *fp = fopen (filename.c_str(), "rb");
	if (fp == NULL) return 0;
	fseek (fp, 0, SEEK_END);
	uint32 size = ftell (fp);
	fclose (fp);
	return size;
}

uint32	CFile::getFileModificationDate(const std::string &filename)
{
	uint pos;
	string fn;
	if ((pos=filename.find('@')) != string::npos)
	{
		fn = filename.substr (0, pos);
	}
	else
	{
		fn = filename;
	}

#if defined (NL_OS_WINDOWS)
	struct _stat buf;
	int result = _stat (fn.c_str (), &buf);
#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = stat (fn.c_str (), &buf);
#endif

	if (result != 0) return 0;
	else return buf.st_mtime;
}


uint32	CFile::getFileCreationDate(const std::string &filename)
{
	uint pos;
	string fn;
	if ((pos=filename.find('@')) != string::npos)
	{
		fn = filename.substr (0, pos);
	}
	else
	{
		fn = filename;
	}

#if defined (NL_OS_WINDOWS)
	struct _stat buf;
	int result = _stat (fn.c_str (), &buf);
#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = stat (fn.c_str (), &buf);
#endif

	if (result != 0) return 0;
	else return buf.st_ctime;
}

struct CFileEntry
{
	CFileEntry (const string &filename, void (*callback)(const string &filename)) : FileName (filename), Callback (callback)
	{
		LastModified = CFile::getFileModificationDate(filename);
	}
	string FileName;
	void (*Callback)(const string &filename);
	uint32 LastModified;
};

static vector <CFileEntry> FileToCheck;


void CFile::addFileChangeCallback (const std::string &filename, void (*cb)(const string &filename))
{
	nlinfo ("CFile::addFileChangeCallback: I'll check the modification date for this file '%s'", CPath::lookup(filename).c_str());
	FileToCheck.push_back(CFileEntry(CPath::lookup(filename), cb));
}

void CFile::checkFileChange (TTime frequency)
{
	static TTime lastChecked = CTime::getLocalTime();

	if (CTime::getLocalTime() > lastChecked + frequency)
	{
		for (uint i = 0; i < FileToCheck.size(); i++)
		{
			if(CFile::getFileModificationDate(FileToCheck[i].FileName) != FileToCheck[i].LastModified)
			{
				// need to reload it
				if(FileToCheck[i].Callback != NULL)
					FileToCheck[i].Callback(FileToCheck[i].FileName);

				FileToCheck[i].LastModified = CFile::getFileModificationDate(FileToCheck[i].FileName);
			}
		}

		lastChecked = CTime::getLocalTime();
	}
}


static bool CopyMoveFile(const char *dest, const char *src, bool copyFile, bool failIfExists = false)
{
	if (!dest || !src) return false;
	if (!strlen(dest) || !strlen(src)) return false;	
#ifdef NL_OS_WINDOWS
	std::string dosdest = CPath::standardizeDosPath(dest);
	std::string dossrc = CPath::standardizeDosPath(src);

	return copyFile  ? CopyFile(dossrc.c_str(), dosdest.c_str(), failIfExists) != FALSE
					 : MoveFile(dossrc.c_str(), dosdest.c_str()) != FALSE;
#else
	nlstop; // not implemented yet
	return false;
#endif	
}

bool CFile::copyFile(const char *dest, const char *src, bool failIfExists /*=false*/)
{
	return CopyMoveFile(dest, src, true, failIfExists);
}

bool CFile::moveFile(const char *dest,const char *src)
{
	return CopyMoveFile(dest, src, false);
}


bool CFile::createDirectory(const std::string &filename)
{
#ifdef NL_OS_WINDOWS
	return _mkdir(filename.c_str())==0;
#else
	// Set full permissions....
	return mkdir(filename.c_str(), 0xFFFF)==0;
#endif
}


} // NLMISC
