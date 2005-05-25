/** \file misc/dynloadlib.h
 * class for dynamic library loading
 *
 * $Id: dynloadlib.h,v 1.4 2005/05/25 12:16:32 boucher Exp $
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

#ifndef NL_DYNLIBLOAD_H
#define NL_DYNLIBLOAD_H

#include "types_nl.h"
#include <string>
#include <vector>

#ifdef NL_OS_WINDOWS
#include <windows.h>
#undef max
#undef min
#else
#include <dlfcn.h>
#endif

namespace NLMISC 
{

/// Define the os specific type for dynamic library module handler
#if defined (NL_OS_WINDOWS)
typedef HMODULE		NL_LIB_HANDLE;
#elif defined (NL_OS_UNIX)
typedef void*		NL_LIB_HANDLE;
#else
# error "You must define the module type on this platform"
#endif

#ifdef NL_OS_WINDOWS
#define NL_LIB_EXPORT	__declspec(dllexport)
#define NL_LIB_IMPORT	__declspec(dllimport)
#else
#define NL_LIB_EXPORT
#define NL_LIB_IMPORT
#endif

/// Generic dynamic library loading function.
NL_LIB_HANDLE	nlLoadLibrary(const std::string &libName);
/// Generic dynamic library unloading function.
bool			nlFreeLibrary(NL_LIB_HANDLE libHandle);
/// Generic dynamic library symbol address lookup function.
void			*nlGetSymbolAddress(NL_LIB_HANDLE libHandle, const std::string &symbolName);


// Utility macro to export a module entry point as a C pointer to a C++ class or function
#define NL_LIB_EXPORT_SYMBOL(symbolName, classOrFunctionName, instancePointer) \
extern "C"\
{\
	NL_LIB_EXPORT classOrFunctionName	*symbolName = instancePointer;\
};

/*
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2004
 */
class CLibrary
{
	/// Dynamic library handle
	NL_LIB_HANDLE	_LibHandle;
	/// Loaded library name
	std::string		_LibFileName;

	/** When a module hanmdle is assigned to the instance, this
	 *	flag state whether the CLibrary will free the library or not
	*/
	bool			_Ownership;

	/// Lib paths
	static std::vector<std::string>	_LibPaths;

	/// Private copy constructor, not authorized
	CLibrary (const CLibrary &other)
	{
		// Nothing to do has it is forbiden.
		// Allowing copy require to manage reference count from CLibrary to the module resource.
		nlassert(false);
	}

	// Private assignement operator
	CLibrary &operator =(const CLibrary &other)
	{
		// Nothing to do has it is forbiden.
		// Allowing assignemnt require to manage reference count from CLibrary to the module resource.
		nlassert(false);
		return *this;
	}
	
public:
	CLibrary();
	/// Assign a existing module handler to a new dynamic library instance
	CLibrary(NL_LIB_HANDLE libHandle, bool ownership);
	/// Load the specified library and take ownership
	CLibrary(const std::string &libName, bool addNelSuffixe, bool tryLibPath, bool ownership = true);
	/// Destructor, free the library is the object have ownership
	virtual ~CLibrary();

	/** Load the specified library.
	*	The method assert if a module is already assigned or loaded
	*	If addNelSuffixe is true, the standard nel suffixe and library extention are 
	*	appended to the lib name (with is just a base name).
	*	If tryLibPath is true, then the method will try to find the required 
	*	library in the added library files (in order of addition).
	*	Return true if the library load ok.
	*/
	bool loadLibrary(const std::string &libName, bool addNelSuffixe, bool tryLibPath, bool ownership = true);

	/** Unload (free) the assigned/loaded library.
	*	The object must have ownership over the library or the call will assert.
	*	After this call, you can recall loadLibrary.
	*/
	void freeLibrary();

	/** Get the address a the specified procedure in the library
	*	Return NULL is the proc is not found,
	*	Assert if the library is not load or assigned.
	*/
	void *getSymbolAddress(const std::string &symbolName);

	/// Get the name of the loaded library
	std::string getLibFileName()
	{
		return _LibFileName;
	}

	/// Build a NeL standard library name according to platform and compilation mode setting.
	static std::string makeLibName(const std::string &baseName);
	/// Add a list of library path
	static void addLibPaths(const std::vector<std::string> &paths);
	/// Add a library path
	static void addLibPath(const std::string &path);

};

} // NLMISC

#endif // NL_DYNLIBLOAD_H

