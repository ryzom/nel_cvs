/** \file misc/dynloadlib.h
 * class for dynamic library loading
 *
 * $Id: dynloadlib.h,v 1.2 2004/08/31 17:40:29 boucher Exp $
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

//typedef int (CALLBACK *NL_FARPROC);
//typedef int (FAR WINAPI *NL_FARPROC)();

// Currently this class supports Windows only
#ifdef NL_OS_WINDOWS

#include <windows.h>
#undef max
#undef min



#include "nel/misc/types_nl.h"
namespace NLMISC {

/*
 *
 * \author Jean-Baptiste Cardouat
 * \author Nevrax France
 * \date 2004
 */
class CLibrary
{
	HMODULE lib;	
public:
	CLibrary();
	CLibrary(HMODULE newLib);
	virtual ~CLibrary();
	static HMODULE loadLibrary(std::string& libName);
	static FARPROC loadFunction(HMODULE&,std::string&);
	static BOOL closeLib(HMODULE& hm);

	void setLib(const HMODULE& libVal);
	FARPROC loadFunction(std::string&);
	BOOL closeLib();

};

inline HMODULE CLibrary::loadLibrary(std::string& libName)
{
	HMODULE hm = LoadLibrary(libName.c_str());

	if (hm == NULL)
	{
		nlwarning("CLibrary::loadLibrary: failed to load the module '%s'", libName.c_str());
	}

	return hm;
}

inline FARPROC CLibrary::loadFunction(HMODULE& hm,std::string& functionName)
{
	return GetProcAddress( hm,functionName.c_str());
}

inline BOOL CLibrary::closeLib(HMODULE& hm)
{
	return FreeLibrary(hm);
}
} // NLMISC

#endif //NL_OS_WINDOWS

#endif // NL_DYNLIBLOAD_H

/* End of thread.h */


