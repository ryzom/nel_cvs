/** \file path.cpp
 * Utility class for searching files in differents paths.
 *
 * $Id: path.cpp,v 1.110 2004/09/22 18:22:41 distrib Exp $
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
#include "nel/misc/progress_callback.h"
#include "nel/misc/file.h"

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <direct.h>
#	include <io.h>
#	include <fcntl.h>
#	include <sys/types.h>
#	include <sys/stat.h>
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

void CPath::getFileList(const std::string &extension, std::vector<std::string> &filenames)
{
	CPath *inst = getInstance();

	if (!inst->_MemoryCompressed)
	{
		std::map<std::string, CFileEntry>::iterator first(inst->_Files.begin()), last(inst->_Files.end());

		if( !extension.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = inst->SSMext.get(first->second.idExt);
				if (ext == extension)
				{
					filenames.push_back(first->first);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
			{
				filenames.push_back(first->first);
			}
		}
	}
	else
	{
		// compressed memory version
		std::vector<CPath::CMCFileEntry>::iterator first(inst->_MCFiles.begin()), last(inst->_MCFiles.end());

		if( !extension.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = inst->SSMext.get(first->idExt);
				if (ext == extension)
				{
					filenames.push_back(first->Name);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
			{
				filenames.push_back(first->Name);
			}
		}
	}
}

#ifndef NL_DONT_USE_EXTERNAL_CODE
void CPath::getFileListByName(const std::string &extension, const std::string &name, std::vector<std::string> &filenames)
{
	CPath *inst = getInstance();
	if (!inst->_MemoryCompressed)
	{
		std::map<std::string, CFileEntry>::iterator first(inst->_Files.begin()), last(inst->_Files.end());

		if( !name.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = inst->SSMext.get(first->second.idExt);
				if (first->first.find(name) != string::npos && (ext == extension || extension.empty()))
				{
					filenames.push_back(first->first);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
			{
				filenames.push_back(first->first);
			}
		}
	}
	else
	{
		// compressed memory version
		std::vector<CPath::CMCFileEntry>::iterator first(inst->_MCFiles.begin()), last(inst->_MCFiles.end());

		if( !name.empty() )
		{
			for (; first != last; ++ first)
			{
				string ext = inst->SSMext.get(first->idExt);
				if (strstr(first->Name, name.c_str()) != NULL && (ext == extension || extension.empty()))
				{
					filenames.push_back(first->Name);
				}
			}
		}
		// if extension is empty we keep all files
		else
		{
			for (; first != last; ++ first)
		{
				filenames.push_back(first->Name);
			}
		}
	}
}
#endif // NL_DONT_USE_EXTERNAL_CODE


CPath *CPath::getInstance ()
{
	if (_Instance == NULL)
	{
#undef new
		_Instance = new CPath;
#define new NL_NEW
	}
	return _Instance;
}

void CPath::clearMap ()
{
	CPath *inst = CPath::getInstance();
	nlassert(!inst->_MemoryCompressed);
	inst->_Files.clear ();
	CBigFile::getInstance().removeAll ();
	NL_DISPLAY_PATH("PATH: CPath::clearMap(): map directory cleared");
}

CPath::CMCFileEntry *CPath::MCfind (const std::string &filename)
{
	CPath *inst = CPath::getInstance();
	nlassert(inst->_MemoryCompressed);
	vector<CMCFileEntry>::iterator it;
	it = lower_bound(inst->_MCFiles.begin(), inst->_MCFiles.end(), filename.c_str(), CMCFileComp());
	if (it != inst->_MCFiles.end())
	{
		CMCFileComp FileComp;
		if (FileComp.specialCompare(*it, filename.c_str()) == 0)
			return &(*it);
	}
	return NULL;
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
	NL_ALLOC_CONTEXT (MiPath);
	CPath *inst = CPath::getInstance();
	nlassert(!inst->_MemoryCompressed);

	string ext1lwr = toLower(ext1);
	string ext2lwr = toLower(ext2);

	if (ext1lwr.empty() || ext2lwr.empty())
	{
		nlwarning ("PATH: CPath::remapExtension(%s, %s, %d): can't remap empty extension", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}

	if (ext1lwr == "bnp" || ext2lwr == "bnp")
	{
		nlwarning ("PATH: CPath::remapExtension(%s, %s, %d): you can't remap a big file", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
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
			string ext = inst->SSMext.get((*it).second.idExt);
			if ((*it).second.Remapped && ext == ext2lwr)
			{
				inst->_Files.erase (it);
			}
			it = nit;
		}
		NL_DISPLAY_PATH("PATH: CPath::remapExtension(%s, %s, %d): extension removed", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}
	else
	{
		sint n = inst->findExtension (ext1lwr, ext2lwr);
		if (n != -1)
		{
			nlwarning ("PATH: CPath::remapExtension(%s, %s, %d): remapping already set", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
			return;
		}

		// adding mapping into the mapping list
		inst->_Extensions.push_back (make_pair (ext1lwr, ext2lwr));

		// adding mapping into the map
		vector<string> newFiles;
		map<string, CFileEntry>::iterator it = inst->_Files.begin();
		while (it != inst->_Files.end ())
		{
			string ext = inst->SSMext.get((*it).second.idExt);
			if (!(*it).second.Remapped && ext == ext1lwr)
			{
				// find if already exist
				uint32 pos = (*it).first.find_last_of (".");
				if (pos != string::npos)
				{
					string file = (*it).first.substr (0, pos + 1);
					file += ext2lwr;

// TODO perhaps a problem because I insert in the current map that i parcours
					string path = inst->SSMpath.get((*it).second.idPath);
					insertFileInMap (file, path+file, true, ext1lwr);
				}
			}
			it++;
		}
		NL_DISPLAY_PATH("PATH: CPath::remapExtension(%s, %s, %d): extension added", ext1lwr.c_str(), ext2lwr.c_str(), substitute);
	}
}

// ***************************************************************************
void CPath::remapFile (const std::string &file1, const std::string &file2)
{
	NL_ALLOC_CONTEXT (MiPath);
	CPath *inst = CPath::getInstance();
	if (file1.empty()) return;
	if (file2.empty()) return;
	inst->_RemappedFiles[toLower(file1)] = toLower(file2);
}

// ***************************************************************************
static void removeAllUnusedChar(string &str)
{
	uint32 i = 0;
	while (!str.empty() && (i != str.size()))
	{
		if ((str[i] == ' ' || str[i] == '\t' || str[i] == '\r' || str[i] == '\n'))
			str.erase(str.begin()+i);
		else
			i++;
	}
}

// ***************************************************************************
void CPath::loadRemappedFiles (const std::string &file)
{
	NL_ALLOC_CONTEXT (MiPath);
	string fullName = lookup(file, false, true, true);
	CIFile f;
	f.setCacheFileOnOpen (true);

	if (!f.open (fullName))
		return;

	char sTmp[514];
	string str;
	
	while (!f.eof())
	{
		f.getline(sTmp, 512);
		str = sTmp;
		if (str.find(','))
		{
			removeAllUnusedChar(str);
			if (!str.empty())
				remapFile( str.substr(0,str.find(',')), str.substr(str.find(',')+1, str.size()) );
		}
	}
}

// ***************************************************************************
string CPath::lookup (const string &filename, bool throwException, bool displayWarning, bool lookupInLocalDirectory)
{
	// If the file already contains a @, it means that a lookup already proceed and returning a big file, do nothing
	if (filename.find ("@") != string::npos)
	{
		NL_DISPLAY_PATH("PATH: CPath::lookup(%s):	already found", filename.c_str());
		return filename;
	}

	// Try to find in the map directories
	CPath *inst = CPath::getInstance();
	string str = toLower(filename);

	// Remove end spaces
	while ((!str.empty()) && (str[str.size()-1] == ' '))
	{
		str.resize (str.size()-1);
	}

	map<string, string>::iterator itss = inst->_RemappedFiles.find(str);
	if (itss != inst->_RemappedFiles.end())
		str = itss->second;

	if (inst->_MemoryCompressed)
	{
		CMCFileEntry *pMCFE = MCfind(str);
		// If found in the map, returns it
		if (pMCFE != NULL)
		{
			string fname, path = inst->SSMpath.get(pMCFE->idPath);
			if (pMCFE->Remapped)
				fname = CFile::getFilenameWithoutExtension(pMCFE->Name) + "." + inst->SSMext.get(pMCFE->idExt);
			else
				fname = pMCFE->Name;

			NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the map directory: '%s'", fname.c_str(), path.c_str());
			return path + fname;
		}
	}
	else // NOT memory compressed
	{
		map<string, CFileEntry>::iterator it = inst->_Files.find (str);
		// If found in the map, returns it
		if (it != inst->_Files.end())
		{
			string fname, path = inst->SSMpath.get((*it).second.idPath);
			if (it->second.Remapped)
				fname = CFile::getFilenameWithoutExtension((*it).second.Name) + "." + inst->SSMext.get((*it).second.idExt);
			else
				fname = (*it).second.Name;

			NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the map directory: '%s'", fname.c_str(), path.c_str());
			return path + fname;
		}
	}
	

	// Try to find in the alternative directories
	for (uint i = 0; i < inst->_AlternativePaths.size(); i++)
	{
		string s = inst->_AlternativePaths[i] + filename;
		if ( CFile::fileExists(s) )
		{
			NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the alternative directory: '%s'", filename.c_str(), s.c_str());
			return s;
		}
		
		// try with the remapping
		for (uint j = 0; j < inst->_Extensions.size(); j++)
		{
			if (toLower(CFile::getExtension (filename)) == inst->_Extensions[j].second)
			{
				string rs = inst->_AlternativePaths[i] + CFile::getFilenameWithoutExtension (filename) + "." + inst->_Extensions[j].first;
				if ( CFile::fileExists(rs) )
				{
					NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the alternative directory: '%s'", filename.c_str(), rs.c_str());
					return rs;
				}
			}
		}
	}

	// Try to find in the current directory
	if ( lookupInLocalDirectory && CFile::fileExists(filename) )
	{
		NL_DISPLAY_PATH("PATH: CPath::lookup(%s): found in the current directory: '%s'", filename.c_str(), filename.c_str());
		return filename;
	}

	// Not found
	if (displayWarning)
	{
		nlwarning ("PATH: CPath::lookup(%s): file not found", filename.c_str());
	}

	if (throwException)
		throw EPathNotFound (filename);

	return "";
}

bool CPath::exists (const std::string &filename)
{
	// Try to find in the map directories
	CPath *inst = CPath::getInstance();
	string str = toLower(filename);

	// Remove end spaces
	while ((!str.empty()) && (str[str.size()-1] == ' '))
	{
		str.resize (str.size()-1);
	}


	if (inst->_MemoryCompressed)
	{
		CMCFileEntry *pMCFE = MCfind(str);
		// If found in the vector, returns it
		if (pMCFE != NULL)
			return true;
	}
	else
	{
		map<string, CFileEntry>::iterator it = inst->_Files.find (str);
		// If found in the map, returns it
		if (it != inst->_Files.end())
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
//	if (path.size() >= 2 && path[0] == '\\' && path[1] == '\\')
//	{
//		newPath += "\\\\";
//		i = 2;
//	}
	
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
		// Yoyo: supress toLower. Not usefull!?!
		/*else if (isupper(path[i]))
			newPath += tolower(path[i]);*/
		else
			newPath += path[i];
	}

	if (CFile::isExists(path) && CFile::isDirectory(path) && newPath[newPath.size()-1] != '\\')
		newPath += '\\';

	return newPath;
}


std::string CPath::getCurrentPath ()
{
	char buffer [10000];

#ifdef NL_OS_WINDOWS
	return standardizePath(_getcwd(buffer, 10000), false);
#else
	return standardizePath(getcwd(buffer, 10000), false);
#endif
}

bool CPath::setCurrentPath (const char *newDir)
{
#ifdef NL_OS_WINDOWS
	return _chdir(newDir) == 0;
#else
	// todo : check this compiles under linux. Thanks (Hulud)
	return chdir(newDir) == 0;
#endif
}

std::string CPath::getFullPath (const std::string &path, bool addFinalSlash)
{
	string currentPath = standardizePath (getCurrentPath ());
	string sPath = standardizePath (path, addFinalSlash);

	// current path
	if (path.empty() || sPath == "." || sPath == "./")
	{
		return currentPath;
	}

	// windows full path
	if (path.size() >= 2 && path[1] == ':')
	{
		return sPath;
	}

	if (path.size() >= 2 && (path[0] == '/' || path[0] == '\\') && (path[1] == '/' || path[1] == '\\'))
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

void CPath::getPathContent (const string &path, bool recurse, bool wantDir, bool wantFile, vector<string> &result, class IProgressCallback *progressCallBack, bool showEverything)
{			
	if(	path.empty() )
	{
		NL_DISPLAY_PATH("PATH: CPath::getPathContent(): Empty input Path");
		return;
	}

#ifndef NL_OS_WINDOWS
	BasePathgetPathContent = CPath::standardizePath (path);
#endif

	DIR *dir = opendir (path.c_str());

	if (dir == NULL)
	{
		NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): could not open the directory", path.c_str(), recurse, wantDir, wantFile);
		return;
	}

	// contains path that we have to recurs into
	vector<string> recursPath;

	while (true)
	{
		dirent *de = readdir(dir);
		if (de == NULL)
		{
			NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): end of directory", path.c_str(), recurse, wantDir, wantFile);
			break;
		}

		string fn = getname (de);

		// skip . and ..
		if (fn == "." || fn == "..")
			continue;

		if (isdirectory(de))
		{
			// skip CVS directory
			if ((!showEverything) && (fn == "CVS"))
			{
				NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): skip CVS directory", path.c_str(), recurse, wantDir, wantFile);
				continue;
			}

			string stdName = standardizePath(standardizePath(path) + fn);
			if (recurse)
			{
				NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): need to recurse into '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
				recursPath.push_back (stdName);
			}

			if (wantDir)
			{
				NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): adding path '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
				result.push_back (stdName);
			}
		}
		if (wantFile && isfile(de))
		{
			if ( (!showEverything) && (fn.size() >= 4 && fn.substr (fn.size()-4) == ".log"))
			{
				NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): skip *.log files (%s)", path.c_str(), recurse, wantDir, wantFile, fn.c_str());
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
			
				
			NL_DISPLAY_PATH("PATH: CPath::getPathContent(%s, %d, %d, %d): adding file '%s'", path.c_str(), recurse, wantDir, wantFile, stdName.c_str());
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
		// Progress bar
		if (progressCallBack)
		{
			progressCallBack->progress ((float)i/(float)recursPath.size ());
			progressCallBack->pushCropedValues ((float)i/(float)recursPath.size (), (float)(i+1)/(float)recursPath.size ());
		}

		getPathContent (recursPath[i], recurse, wantDir, wantFile, result, progressCallBack, showEverything);

		// Progress bar
		if (progressCallBack)
		{
			progressCallBack->popCropedValues ();
		}
	}
}

void CPath::removeAllAlternativeSearchPath ()
{
	CPath *inst = CPath::getInstance();
	inst->_AlternativePaths.clear ();
	NL_DISPLAY_PATH("PATH: CPath::RemoveAllAternativeSearchPath(): removed");
}


void CPath::addSearchPath (const string &path, bool recurse, bool alternative, class IProgressCallback *progressCallBack)
{
	NL_ALLOC_CONTEXT (MiPath);
	H_AUTO_INST(addSearchPath);

	CPath *inst = CPath::getInstance();
	nlassert(!inst->_MemoryCompressed);

	// check empty directory
	if (path.empty())
	{
		nlwarning ("PATH: CPath::addSearchPath(%s, %d, %d): can't add empty directory, skip it", path.c_str(), recurse, alternative);
		return;
	}

	// check if it s a directory
	if (!CFile::isDirectory (path))
	{
		nlinfo ("PATH: CPath::addSearchPath(%s, %d, %d): '%s' is not a directory, I'll call addSearchFile()", path.c_str(), recurse, alternative, path.c_str());
		addSearchFile (path, false, "", progressCallBack);
		return;
	}

	string newPath = standardizePath(path);

	// check if it s a directory
	if (!CFile::isExists (newPath))
	{
		nlwarning ("PATH: CPath::addSearchPath(%s, %d, %d): '%s' is not found, skip it", path.c_str(), recurse, alternative, newPath.c_str());
		return;
	}

	nlinfo ("PATH: CPath::addSearchPath(%s, %d, %d): adding the path '%s'", path.c_str(), recurse, alternative, newPath.c_str());

	NL_DISPLAY_PATH("PATH: CPath::addSearchPath(%s, %d, %d): try to add '%s'", path.c_str(), recurse, alternative, newPath.c_str());

	if (alternative)
	{
		vector<string> pathsToProcess;

		// add the current path
		pathsToProcess.push_back (newPath);

		if (recurse)
		{
			// find all path and subpath
			getPathContent (newPath, recurse, true, false, pathsToProcess, progressCallBack);
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
				NL_DISPLAY_PATH("PATH: CPath::addSearchPath(%s, %d, %d): path '%s' added", newPath.c_str(), recurse, alternative, pathsToProcess[p].c_str());
			}
			else
			{
				nlwarning ("PATH: CPath::addSearchPath(%s, %d, %d): path '%s' already added", newPath.c_str(), recurse, alternative, pathsToProcess[p].c_str());
			}
		}
	}
	else
	{
		vector<string> filesToProcess;

		// Progree bar
		if (progressCallBack)
		{
			progressCallBack->progress (0);
			progressCallBack->pushCropedValues (0, 0.5f);
		}

		// find all files in the path and subpaths
		getPathContent (newPath, recurse, false, true, filesToProcess, progressCallBack);

		// Progree bar
		if (progressCallBack)
		{
			progressCallBack->popCropedValues ();
			progressCallBack->progress (0.5);
			progressCallBack->pushCropedValues (0.5f, 1);
		}

		// add them in the map
		for (uint f = 0; f < filesToProcess.size(); f++)
		{
			// Progree bar
			if (progressCallBack)
			{
				progressCallBack->progress ((float)f/(float)filesToProcess.size());
				progressCallBack->pushCropedValues ((float)f/(float)filesToProcess.size(), (float)(f+1)/(float)filesToProcess.size());
			}

			string filename = CFile::getFilename (filesToProcess[f]);
			string filepath = CFile::getPath (filesToProcess[f]);
//			insertFileInMap (filename, filepath, false, CFile::getExtension(filename));
			addSearchFile (filesToProcess[f], false, "", progressCallBack);

			// Progree bar
			if (progressCallBack)
			{
				progressCallBack->popCropedValues ();
			}
		}

		// Progree bar
		if (progressCallBack)
		{
			progressCallBack->popCropedValues ();
		}
	}
}

void CPath::addSearchFile (const string &file, bool remap, const string &virtual_ext, NLMISC::IProgressCallback *progressCallBack)
{
	NL_ALLOC_CONTEXT (MiPath);
	CPath *inst = CPath::getInstance();
	nlassert(!inst->_MemoryCompressed);

	string newFile = standardizePath(file, false);

	// check empty file
	if (newFile.empty())
	{
		nlwarning ("PATH: CPath::addSearchFile(%s, %d, %s): can't add empty file, skip it", file.c_str(), remap, virtual_ext.c_str());
		return;
	}

	// check if the file exists
	if (!CFile::isExists (newFile))
	{
		nlwarning ("PATH: CPath::addSearchFile(%s, %d, %s): '%s' is not found, skip it", file.c_str(), remap, virtual_ext.c_str(), newFile.c_str());
		return;
	}

	// check if it s a file
	if (CFile::isDirectory (newFile))
	{
		nlwarning ("PATH: CPath::addSearchFile(%s, %d, %s): '%s' is not a file, skip it", file.c_str(), remap, virtual_ext.c_str(), newFile.c_str());
		return;
	}

	// check if it s a big file
	if (CFile::getExtension(newFile) == "bnp")
	{
		NL_DISPLAY_PATH ("PATH: CPath::addSearchFile(%s, %d, %s): '%s' is a big file, add it", file.c_str(), remap, virtual_ext.c_str(), newFile.c_str());
		addSearchBigFile(file, false, false, progressCallBack);
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
		ext = CFile::getExtension (newFile);
	}

	insertFileInMap (filename, newFile, remap, ext);

	if (!remap && !ext.empty())
	{
		// now, we have to see extension and insert in the map the remapped files
		for (uint i = 0; i < inst->_Extensions.size (); i++)
		{
			if (inst->_Extensions[i].first == toLower(ext))
			{
				// need to remap
				addSearchFile (newFile, true, inst->_Extensions[i].second, progressCallBack);
			}
		}
	}
}

void CPath::addSearchListFile (const string &filename, bool recurse, bool alternative)
{
	NL_ALLOC_CONTEXT (MiPath);
	// check empty file
	if (filename.empty())
	{
		nlwarning ("PATH: CPath::addSearchListFile(%s, %d, %d): can't add empty file, skip it", filename.c_str(), recurse, alternative);
		return;
	}

	// check if the file exists
	if (!CFile::isExists (filename))
	{
		nlwarning ("PATH: CPath::addSearchListFile(%s, %d, %d): '%s' is not found, skip it", filename.c_str(), recurse, alternative, filename.c_str());
		return;
	}

	// check if it s a file
	if (CFile::isDirectory (filename))
	{
		nlwarning ("PATH: CPath::addSearchListFile(%s, %d, %d): '%s' is not a file, skip it", filename.c_str(), recurse, alternative, filename.c_str());
		return;
	}

	// TODO lire le fichier et ajouter les fichiers qui sont dedans

}

// WARNING : recurse is not used
void CPath::addSearchBigFile (const string &sBigFilename, bool recurse, bool alternative, NLMISC::IProgressCallback *progressCallBack)
{
 //#ifndef NL_OS_WINDOWS
  //	nlerror( "BNP currently not supported on Unix" ); // test of BNP failed on Linux
  //#endif

	// Check if filename is not empty
	if (sBigFilename.empty())
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): can't add empty file, skip it", sBigFilename.c_str(), recurse, alternative);
		return;
	}
	// Check if the file exists
	if (!CFile::isExists (sBigFilename))
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): '%s' is not found, skip it", sBigFilename.c_str(), recurse, alternative, sBigFilename.c_str());
		return;
	}
	// Check if it s a file
	if (CFile::isDirectory (sBigFilename))
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): '%s' is not a file, skip it", sBigFilename.c_str(), recurse, alternative, sBigFilename.c_str());
		return;
	}
	// Open and read the big file header
	CPath *inst = CPath::getInstance();
	nlassert(!inst->_MemoryCompressed);

	FILE *Handle = fopen (sBigFilename.c_str(), "rb");
	if (Handle == NULL)
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): can't open file, skip it", sBigFilename.c_str(), recurse, alternative);
		return;
	}

	// add the link with the CBigFile singleton
	if (CBigFile::getInstance().add (sBigFilename, BF_ALWAYS_OPENED | BF_CACHE_FILE_ON_OPEN))
	{
		// also add the bigfile name in the map to retreive the full path of a .bnp when we want modification date of the bnp for example
		insertFileInMap (CFile::getFilename (sBigFilename), sBigFilename, false, CFile::getExtension(sBigFilename));

		// parse the big file to add file in the map
		uint32 nFileSize=CFile::getFileSize (Handle);
		//nlfseek64 (Handle, 0, SEEK_END);
		//uint32 nFileSize = ftell (Handle);
		nlfseek64 (Handle, nFileSize-4, SEEK_SET);
		uint32 nOffsetFromBegining;
		fread (&nOffsetFromBegining, sizeof(uint32), 1, Handle);
		nlfseek64 (Handle, nOffsetFromBegining, SEEK_SET);
		uint32 nNbFile;
		fread (&nNbFile, sizeof(uint32), 1, Handle);
		for (uint32 i = 0; i < nNbFile; ++i)
		{
			// Progress bar
			if (progressCallBack)
			{
				progressCallBack->progress ((float)i/(float)nNbFile);
				progressCallBack->pushCropedValues ((float)i/(float)nNbFile, (float)(i+1)/(float)nNbFile);
			}

			char FileName[256];
			uint8 nStringSize;
			fread (&nStringSize, 1, 1, Handle);
			fread (FileName, 1, nStringSize, Handle);
			FileName[nStringSize] = 0;
			uint32 nFileSize2;
			fread (&nFileSize2, sizeof(uint32), 1, Handle);
			uint32 nFilePos;
			fread (&nFilePos, sizeof(uint32), 1, Handle);
			string sTmp = toLower(string(FileName));
			if (sTmp.empty())
			{
				nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): can't add empty file, skip it", sBigFilename.c_str(), recurse, alternative);
				continue;
			}
			string bigfilenamealone = CFile::getFilename (sBigFilename);
			string filenamewoext = CFile::getFilenameWithoutExtension (sTmp);
			string ext = toLower(CFile::getExtension(sTmp));

			insertFileInMap (sTmp, bigfilenamealone + "@" + sTmp, false, ext);

			for (uint j = 0; j < inst->_Extensions.size (); j++)
			{
				if (inst->_Extensions[j].first == ext)
				{
					// need to remap
					insertFileInMap (filenamewoext+"."+inst->_Extensions[j].second, 
									bigfilenamealone + "@" + sTmp, 
									true, 
									inst->_Extensions[j].first);
				}
			}

			// Progress bar
			if (progressCallBack)
			{
				progressCallBack->popCropedValues ();
			}
		}
	}
	else
	{
		nlwarning ("PATH: CPath::addSearchBigFile(%s, %d, %d): can't add the big file", sBigFilename.c_str(), recurse, alternative);
	}

	fclose (Handle);
}

void CPath::addIgnoredDoubleFile(const std::string &ignoredFile)
{
	NL_ALLOC_CONTEXT (MiPath);
	CPath::getInstance ()->IgnoredFiles.push_back(ignoredFile);
}

void CPath::insertFileInMap (const string &filename, const string &filepath, bool remap, const string &extension)
{
	NL_ALLOC_CONTEXT (MiPath);
	CPath *inst = CPath::getInstance();
	nlassert(!inst->_MemoryCompressed);
	// find if the file already exist
	map<string, CFileEntry>::iterator it = inst->_Files.find (toLower(filename));
	if (it != inst->_Files.end ())
	{
		string path = inst->SSMpath.get((*it).second.idPath);
		if (path.find("@") != string::npos && filepath.find("@") == string::npos)
		{
			// if there's a file in a big file and a file in a path, the file in path wins
			// replace with the new one
			nlinfo ("PATH: CPath::insertFileInMap(%s, %s, %d, %s): already inserted from '%s' but special case so overide it", filename.c_str(), filepath.c_str(), remap, extension.c_str(), path.c_str());
			string sTmp = filepath.substr(0,filepath.rfind('/')+1);
			(*it).second.idPath = inst->SSMpath.add(sTmp);
			(*it).second.Remapped = remap;
			(*it).second.idExt = inst->SSMext.add(extension);
			(*it).second.Name = filename;
		}
		else
		{
			for(uint i = 0; i < inst->IgnoredFiles.size(); i++)
			{
				// if we don't want to display a warning, skip it
				if(filename == inst->IgnoredFiles[i])
					return;
			}
			// if the path is the same, don't warn
			string path2 = inst->SSMpath.get((*it).second.idPath);
			string sPathOnly;
			if(filepath.rfind('@') != string::npos)
				sPathOnly = filepath.substr(0,filepath.rfind('@')+1);
			else
				sPathOnly = CFile::getPath(filepath);

			if (path2 == sPathOnly)
				return;
			nlwarning ("PATH: CPath::insertFileInMap(%s, %s, %d, %s): already inserted from '%s', skip it\n%s\n%s", filename.c_str(), filepath.c_str(), remap, extension.c_str(), path2.c_str(),filepath.c_str(),path2.c_str());
		}
	}
	else
	{
		CFileEntry fe;
		fe.idExt = inst->SSMext.add(extension);
		fe.Remapped = remap;
		string sTmp;
		if (filepath.find("@") == string::npos)
			sTmp = filepath.substr(0,filepath.rfind('/')+1);
		else
			sTmp = filepath.substr(0,filepath.rfind('@')+1);

		fe.idPath = inst->SSMpath.add(sTmp);
		fe.Name = filename;

		inst->_Files.insert (make_pair(toLower(filename), fe));
		NL_DISPLAY_PATH("PATH: CPath::insertFileInMap(%s, %s, %d, %s): added", toLower(filename).c_str(), filepath.c_str(), remap, toLower(extension).c_str());
	}
}

void CPath::display ()
{
	CPath *inst = CPath::getInstance ();
	nlinfo ("PATH: Contents of the map:");
	nlinfo ("PATH: %-25s %-5s %-5s %s", "filename", "ext", "remap", "full path");
	nlinfo ("PATH: ----------------------------------------------------");
	if (inst->_MemoryCompressed)
	{
		for (uint i = 0; i < inst->_MCFiles.size(); ++i)
		{
			const CMCFileEntry &fe = inst->_MCFiles[i];
			string ext = inst->SSMext.get(fe.idExt);
			string path = inst->SSMpath.get(fe.idPath);
			nlinfo ("PATH: %-25s %-5s %-5d %s", fe.Name, ext.c_str(), fe.Remapped, path.c_str());
		}
	}
	else
	{
		for (map<string, CFileEntry>::iterator it = inst->_Files.begin(); it != inst->_Files.end (); it++)
		{
			string ext = inst->SSMext.get((*it).second.idExt);
			string path = inst->SSMpath.get((*it).second.idPath);
			nlinfo ("PATH: %-25s %-5s %-5d %s", (*it).first.c_str(), ext.c_str(), (*it).second.Remapped, path.c_str());
		}
	}
	nlinfo ("PATH: ");
	nlinfo ("PATH: Contents of the alternative directory:");
	for (uint i = 0; i < inst->_AlternativePaths.size(); i++)
	{
		nlinfo ("PATH: '%s'", inst->_AlternativePaths[i].c_str ());
	}
	nlinfo ("PATH: ");
	nlinfo ("PATH: Contents of the remapped entension table:");
	for (uint j = 0; j < inst->_Extensions.size(); j++)
	{
		nlinfo ("PATH: '%s' -> '%s'", inst->_Extensions[j].first.c_str (), inst->_Extensions[j].second.c_str ());
	}
	nlinfo ("PATH: End of display");
}

void CPath::memoryCompress()
{ 
	NL_ALLOC_CONTEXT (MiPath);
	CPath *inst = CPath::getInstance();

	inst->SSMext.memoryCompress();
	inst->SSMpath.memoryCompress();
	uint nDbg = inst->_Files.size();
	nlinfo ("PATH: Number of file : %d", nDbg);
	nDbg = inst->SSMext.getCount();
	nlinfo ("PATH: Number of different extension : %d", nDbg);
	nDbg = inst->SSMpath.getCount();
	nlinfo ("PATH: Number of different path : %d", nDbg);

	// Convert from _Files to _MCFiles
	uint nSize = 0, nNb = 0;
	map<string,CFileEntry>::iterator it = inst->_Files.begin();
	while (it != inst->_Files.end())
	{
		string sTmp = inst->SSMpath.get(it->second.idPath);
		if ((sTmp.find('@') != string::npos) && !it->second.Remapped)
		{
			// This is a file included in a bigfile (so the name is in the bigfile manager)
		}
		else
		{
			nSize += it->second.Name.size() + 1;
		}
		nNb++;
		it++;
	}

	inst->_AllFileNames = new char[nSize];
	memset(inst->_AllFileNames, 0, nSize);
	inst->_MCFiles.resize(nNb);

	it = inst->_Files.begin();
	nSize = 0;
	nNb = 0;
	while (it != inst->_Files.end())
	{
		CFileEntry &rFE = it->second;
		string sTmp = inst->SSMpath.get(rFE.idPath);
		if ((sTmp.find('@') != string::npos) && !rFE.Remapped)
		{
			// This is a file included in a bigfile (so the name is in the bigfile manager)
			sTmp = sTmp.substr(0, sTmp.size()-1);
			inst->_MCFiles[nNb].Name = CBigFile::getInstance().getFileNamePtr(rFE.Name, sTmp);
			nlassert(inst->_MCFiles[nNb].Name != NULL);
		}
		else
		{
			strcpy(inst->_AllFileNames+nSize, rFE.Name.c_str());
			inst->_MCFiles[nNb].Name = inst->_AllFileNames+nSize;
			nSize += rFE.Name.size() + 1;
		}
		
		inst->_MCFiles[nNb].idExt = rFE.idExt;
		inst->_MCFiles[nNb].idPath = rFE.idPath;
		inst->_MCFiles[nNb].Remapped = rFE.Remapped;

		nNb++;
		it++;
	}

	contReset(inst->_Files);
	inst->_MemoryCompressed = true;
}

void CPath::memoryUncompress()
{
	CPath *inst = CPath::getInstance ();
	inst->SSMext.memoryUncompress(); 
	inst->SSMpath.memoryUncompress(); 	
	inst->_MemoryCompressed = false;
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
	if (res == ~0U)
	{
		nlwarning ("PATH: '%s' is not a valid file or directory name", filename.c_str ());
		return false;
	}
	return (res & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else // NL_OS_WINDOWS
	struct stat buf;
	int res = stat (filename.c_str (), &buf);
	if (res == -1)
	{
		nlwarning ("PATH: can't stat '%s' error %d '%s'", filename.c_str(), errno, strerror(errno));
		return false;
	}
	return (buf.st_mode & S_IFDIR) != 0;
#endif // NL_OS_WINDOWS
}

bool CFile::isExists (const string &filename)
{
#ifdef NL_OS_WINDOWS
	return (GetFileAttributes(filename.c_str()) != ~0U);
#else // NL_OS_WINDOWS
	struct stat buf;
	return stat (filename.c_str (), &buf) == 0;
#endif // NL_OS_WINDOWS
}

bool CFile::fileExists (const string& filename)
{
	H_AUTO(FileExists);
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
/*	FILE *fp = fopen (filename.c_str(), "rb");
	if (fp == NULL) return 0;
	nlfseek64 (fp, 0, SEEK_END);
	uint32 size = ftell (fp);
	fclose (fp);
	return size;*/

/*	const char *s = filename.c_str();
	int h = _open (s, _O_RDONLY | _O_BINARY);
	_lseek (h, 0, SEEK_END);
	uint32 size = _tell (h);
	_close (h);
	return size;
*/

	if (filename.find('@') != string::npos)
	{
		uint32 fs = 0, bfo;
		bool c, d;
		CBigFile::getInstance().getFile (filename, fs, bfo, c, d);
		return fs;
	}
	else
	{
#if defined (NL_OS_WINDOWS)
		struct _stat buf;
		int result = _stat (filename.c_str (), &buf);
#elif defined (NL_OS_UNIX)
		struct stat buf;
		int result = stat (filename.c_str (), &buf);
#endif
		if (result != 0) return 0;
		else return buf.st_size;
	}
}

uint32	CFile::getFileSize (FILE *f)
{
#if defined (NL_OS_WINDOWS)
	struct _stat buf;
	int result = _fstat (fileno(f), &buf);
#elif defined (NL_OS_UNIX)
	struct stat buf;
	int result = fstat (fileno(f), &buf);
#endif
	if (result != 0) return 0;
	else return buf.st_size;
}

uint32	CFile::getFileModificationDate(const std::string &filename)
{
	uint pos;
	string fn;
	if ((pos=filename.find('@')) != string::npos)
	{
		fn = CPath::lookup(filename.substr (0, pos));
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
		fn = CPath::lookup(filename.substr (0, pos));
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

void CFile::removeFileChangeCallback (const std::string &filename)
{
	string fn = CPath::lookup(filename, false, false);
	if (fn.empty())
	{
		fn = filename;
	}
	for (uint i = 0; i < FileToCheck.size(); i++)
	{
		if(FileToCheck[i].FileName == fn)
		{
			nlinfo ("PATH: CFile::removeFileChangeCallback: '%s' is removed from checked files modification", fn.c_str());
			FileToCheck.erase(FileToCheck.begin()+i);
			return;
		}
	}
}

void CFile::addFileChangeCallback (const std::string &filename, void (*cb)(const string &filename))
{
	string fn = CPath::lookup(filename, false, false);
	if (fn.empty())
	{
		fn = filename;
	}
	nlinfo ("PATH: CFile::addFileChangeCallback: I'll check the modification date for this file '%s'", fn.c_str());
	FileToCheck.push_back(CFileEntry(fn, cb));
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
	std::string sdest = CPath::standardizePath(dest,false);
	std::string ssrc = CPath::standardizePath(src,false);

	if(copyFile)
	  {
		FILE *fp1 = fopen(ssrc.c_str(), "rb");
		if (fp1 == NULL)
		  {
			nlwarning ("PATH: CopyMoveFile error: can't fopen in read mode '%s'", ssrc.c_str());
			return false;
		  }
		FILE *fp2 = fopen(sdest.c_str(), "wb");
		if (fp2 == NULL)
		  {
			nlwarning ("PATH: CopyMoveFile error: can't fopen in read write mode '%s'", sdest.c_str());
			return false;
		  }
		static char buffer [1000];
		int s;
		for(s = fread(buffer, 1, sizeof(buffer), fp1); s > 0 && (s = fread(buffer, 1, sizeof(buffer), fp1)) ; fwrite(buffer, 1, s, fp2));
		fclose(fp1);
		fclose(fp2);
	  }
	else
	  {
		if (link (ssrc.c_str(), sdest.c_str()) == -1)
		  {
			nlwarning ("PATH: CopyMoveFile error: can't link '%s' into '%s'", ssrc.c_str(), sdest.c_str());
			return false;
		  }

		if (unlink (ssrc.c_str()) == -1)
		  {
			nlwarning ("PATH: CopyMoveFile error: can't unlink '%s'", ssrc.c_str());
			return false;
		  }
	  }
	  return true;
#endif	
}

bool CFile::copyFile(const char *dest, const char *src, bool failIfExists /*=false*/)
{
	return CopyMoveFile(dest, src, true, failIfExists);
}

bool CFile::quickFileCompare(const std::string &fileName0, const std::string &fileName1)
{
	// make sure the files both exist
	if (!fileExists(fileName0.c_str()) || !fileExists(fileName1.c_str()))
		return false;

	// compare time stamps
	if (getFileModificationDate(fileName0.c_str()) != getFileModificationDate(fileName1.c_str()))
		return false;

	// compare file sizes
	if (getFileSize(fileName0.c_str()) != getFileSize(fileName1.c_str()))
		return false;

	// everything matched so return true
	return true;
}

bool CFile::thoroughFileCompare(const std::string &fileName0, const std::string &fileName1,uint32 maxBufSize)
{
	// make sure the files both exist
	if (!fileExists(fileName0.c_str()) || !fileExists(fileName1.c_str()))
		return false;

	// setup the size variable from file length of first file
	uint32 fileSize=getFileSize(fileName0.c_str());

	// compare file sizes
	if (fileSize != getFileSize(fileName1.c_str()))
		return false;

	// allocate a couple of data buffers for our 2 files
	uint32 bufSize= maxBufSize/2;
	nlassert(sint32(bufSize)>0);
	std::vector<uint8> buf0(bufSize);
	std::vector<uint8> buf1(bufSize);

	// open the two files for input
	CIFile file0(fileName0);
	CIFile file1(fileName1);

	for (uint32 i=0;i<fileSize;i+=bufSize)
	{
		// for the last block in the file reduce buf size to represent the amount of data left in file
		if (i+bufSize>fileSize)
		{
			bufSize= fileSize-i;
			buf0.resize(bufSize);
			buf1.resize(bufSize);
		}

		// read in the next data block from disk
		file0.serialBuffer(&buf0[0], bufSize);
		file1.serialBuffer(&buf1[0], bufSize);

		// compare the contents of hte 2 data buffers
		if (buf0!=buf1)
			return false;
	}

	// everything matched so return true
	return true;
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

bool CFile::createDirectoryTree(const std::string &filename)
{
	bool lastResult=true;
	uint32 i=0;

	// skip dos drive name eg "a:"
	if (filename.size()>1 && filename[1]==':')
		i=2;

	// iterate over the set of directories in the routine's argument
	while (i<filename.size())
	{
		// skip passed leading slashes
		for (;i<filename.size();++i)
			if (filename[i]!='\\' && filename[i]!='/')
				break;

		// if the file name ended with a '/' then there's no extra directory to create
		if (i==filename.size())
			break;

		// skip forwards to next slash
		for (;i<filename.size();++i)
			if (filename[i]=='\\' || filename[i]=='/')
				break;

		// try to create directory
		std::string s= filename.substr(0,i);
		lastResult= createDirectory(s);
	}

	return lastResult;
}

bool CPath::makePathRelative (const char *basePath, std::string &relativePath)
{
	// Standard path with final slash
	string tmp = standardizePath (basePath, true);
	string src = standardizePath (relativePath, true);
	string prefix;

	while (1)
	{
		// Compare with relativePath
		if (strncmp (tmp.c_str (), src.c_str (), tmp.length ()) == 0)
		{
			// Troncate
			uint size = tmp.length ();

			// Same path ?
			if (size == src.length ())
			{
				relativePath = ".";
				return true;
			}

			relativePath = prefix+relativePath.substr (size, relativePath.length () - size);
			return true;
		}

		// Too small ?
		if (tmp.length ()<2)
			break;

		// Remove last directory
		uint lastPos = tmp.rfind ('/', tmp.length ()-2);
		uint previousPos = tmp.find ('/');
		if ((lastPos == previousPos) || (lastPos == string::npos))
			break;

		// Troncate
		tmp = tmp.substr (0, lastPos+1);

		// New prefix
		prefix += "../";
	}
	
	return false;
}

bool CFile::setRWAccess(const std::string &filename)
{
#ifdef NL_OS_WINDOWS
	// if the file exists and there's no write access
	if (_access (filename.c_str(), 00) == 0 && _access (filename.c_str(), 06) == -1)
	{
		// try to set the read/write access
		if (_chmod (filename.c_str(), _S_IREAD | _S_IWRITE) == -1)
		{
			nlwarning ("PATH: Can't set RW access to file '%s': %d %s", filename.c_str(), errno, strerror(errno));
			return false;
		}
	}
#else
	// if the file exists and there's no write access
	if (access (filename.c_str(), F_OK) == 0)
	{
		// try to set the read/write access
		if (chmod (filename.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH) == -1)
		{
			nlwarning ("PATH: Can't set RW access to file '%s': %d %s", filename.c_str(), errno, strerror(errno));
			return false;
		}
	}
	else
	{
		nlwarning("PATH: Can't access to file '%s'", filename.c_str());
	}
#endif
	return true;
}


#ifdef NL_OS_WINDOWS
#define unlink _unlink
#endif

bool CFile::deleteFile(const std::string &filename)
{
	setRWAccess(filename);
	int res = unlink (filename.c_str());
	if (res == -1)
	{
		nlwarning ("PATH: Can't delete file '%s': %d %s", filename.c_str(), errno, strerror(errno));
		return false;
	}
	return true;
}

void CFile::getTemporaryOutputFilename (const std::string &originalFilename, std::string &tempFilename)
{
	uint i = 0;
	do
		tempFilename = originalFilename+".tmp"+toString (i++);
	while (CFile::isExists(tempFilename));
}

} // NLMISC
