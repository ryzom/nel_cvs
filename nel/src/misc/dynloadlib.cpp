//file misc/dynloadlib.cpp
/*
 * <dynloadlib.cpp>
 *
 * $Id: dynloadlib.cpp
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

#include "stdmisc.h"

// Currently this class supports Windows only
#ifdef NL_OS_WINDOWS

#include "nel/misc/dynloadlib.h"
#include "nel/misc/path.h"

using namespace std;

namespace NLMISC
{

NL_LIB_HANDLE nlLoadLibrary(const std::string &libName)
{
#if defined NL_OS_WINDOWS
	return LoadLibrary(libName.c_str());
#elif defined (NL_OS_UNIX)
	return dlopen(libName.c_str(), RTLD_NOW);
#else
#error "You must define nlLoadLibrary for your platform"
#endif
}

bool nlFreeLibrary(NL_LIB_HANDLE libHandle)
{
#if defined NL_OS_WINDOWS
	return FreeLibrary(libHandle) > 0;
#elif defined (NL_OS_UNIX)
#error "this code is not terminated as I don't have the man right now !"
	dlclose(libHandle);
#else
#error "You must define nlFreeLibrary for your platform"
#endif
}

void *nlGetSymbolAddress(NL_LIB_HANDLE libHandle, const std::string &procName)
{
#if defined (NL_OS_WINDOWS)
	return GetProcAddress(libHandle, procName.c_str());
#elif defined (NL_OS_UNIX)
	return dlsym(libHandle, procName.c_str());
#else
#error "You must define nlGetProcAddress for your platform"
#endif
}

// Again some OS specifics stuff
#if defined (NL_OS_WINDOWS)
  const string	NL_LIB_PREFIXE;	// empty
  const string	NL_LIB_EXT(".dll");
#elif defined (NL_OS_UNIX)
  const string	NL_LIB_PREFIXE("lib");
  const string	NL_LIB_EXT(".so");
#else
#error "You must define the default dynamic lib extention"
#endif

// Compilation mode specific suffixes
#if defined (NL_OS_WINDOWS)
 #ifdef NL_DEBUG_INSTRUMENT
  const string	NL_LIB_SUFFIXE("_di");
 #elif defined (NL_DEBUG_FAST)
   const string	NL_LIB_SUFFIXE("_df");
 #elif defined (NL_DEBUG)
   const string	NL_LIB_SUFFIXE("_d");
 #elif defined (NL_RELEASE_DEBUG)
   const string	NL_LIB_SUFFIXE("_rd");
 #elif defined (NL_RELEASE)
   const string	NL_LIB_SUFFIXE("_r");
 #else
   #error "Unknown compilation mode, can't build suffixe"
 #endif
#elif defined (NL_OS_UNIX)
   const string	NL_LIB_SUFFIXE;	// empty
#else
 #error "Lib suffixe not defined for your platform"
#endif

std::vector<std::string>	CLibrary::_LibPaths;


std::string CLibrary::makeLibName(const std::string &baseName)
{
	return NL_LIB_PREFIXE+baseName+NL_LIB_SUFFIXE+NL_LIB_EXT;
}

void CLibrary::addLibPaths(const std::vector<std::string> &paths)
{
	for (uint i=0; i<paths.size(); ++i)
	{
		string newPath = CPath::standardizePath(paths[i]);

		// only add new path
		if (std::find(_LibPaths.begin(), _LibPaths.end(), newPath) == _LibPaths.end())
		{
			_LibPaths.push_back(newPath);
		}
	}
}

void CLibrary::addLibPath(const std::string &path)
{
	string newPath = CPath::standardizePath(path);

	// only add new path
	if (std::find(_LibPaths.begin(), _LibPaths.end(), newPath) == _LibPaths.end())
	{
		_LibPaths.push_back(newPath);
	}
}
	
CLibrary::CLibrary()
:	_LibHandle(NULL),
	_Ownership(true)
{
}

CLibrary::CLibrary(NL_LIB_HANDLE libHandle, bool ownership)
{
	_LibHandle = libHandle;
	_Ownership = ownership;
	_LibFileName = "unknown";
}

CLibrary::CLibrary(const std::string &libName, bool addNelSuffixe, bool tryLibPath, bool ownership)
{
	loadLibrary(libName, addNelSuffixe, tryLibPath, ownership);
	// Assert here !
	nlassert(_LibHandle);
}


CLibrary::~CLibrary()
{
	if (_LibHandle != NULL && _Ownership)
	{
		nlFreeLibrary(_LibHandle);
	}
}

bool CLibrary::loadLibrary(const std::string &libName, bool addNelSuffixe, bool tryLibPath, bool ownership)
{
	_Ownership = ownership;
	string libPath = libName;

	if (addNelSuffixe)
		libPath = makeLibName(libPath);

	if (tryLibPath)
	{
		// remove any directory spec
		string filename = CFile::getFilename(libPath);

		for (uint i=0; i<_LibPaths.size(); ++i)
		{
			string pathname = _LibPaths[i]+filename;
			if (CFile::isExists(pathname))
			{
				// we found it, replace libPath
				libPath = pathname;
				break;
			}
		}
	}

	// load the lib now
	_LibHandle = nlLoadLibrary(libPath);
	_LibFileName = libPath;

	return _LibHandle != NULL;
}

void CLibrary::freeLibrary()
{
	if (_LibHandle)
	{
		nlassert(_Ownership);
		nlFreeLibrary(_LibHandle);

		_LibHandle = NULL;
		_Ownership = false;
		_LibFileName = "";
	}
}

void *CLibrary::getSymbolAddress(const std::string &procName)
{
	nlassert(_LibHandle != NULL);

	return nlGetSymbolAddress(_LibHandle, procName);
}


}	// namespace NLMISC

#endif //NL_OS_WINDOWS