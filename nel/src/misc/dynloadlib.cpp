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

using namespace NLMISC;
using namespace std;

CLibrary::CLibrary()
{
}

CLibrary::CLibrary(HMODULE newLib)
{
	lib=newLib;
}

CLibrary::~CLibrary()
{
}
FARPROC CLibrary::loadFunction(string& functionName)
{
	return GetProcAddress( lib,functionName.c_str());
}

BOOL CLibrary::closeLib()
{
	return FreeLibrary(lib);
}

void CLibrary::setLib(const HMODULE& libVal)
{
	lib=libVal;
}

#endif //NL_OS_WINDOWS