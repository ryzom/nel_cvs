/** \file sound_driver.cpp
 * ISoundDriver: sound driver interface
 *
 * $Id: sound_driver.cpp,v 1.14.2.1 2003/04/24 14:05:45 boucher Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "sound/driver/sound_driver.h"
#include "nel/misc/debug.h"


#ifdef NL_OS_WINDOWS

#include <windows.h>
#undef min
#undef max

#else // NL_OS_WINDOWS

#include <dlfcn.h>

#endif // NL_OS_WINDOWS


namespace NLSOUND
{


// Interface version
const uint32 ISoundDriver::InterfaceVersion = 0x08;

typedef ISoundDriver* (*ISDRV_CREATE_PROC)(bool, ISoundDriver::IStringMapperProvider *stringMapper); 
const char *IDRV_CREATE_PROC_NAME = "NLSOUND_createISoundDriverInstance";

typedef uint32 (*ISDRV_VERSION_PROC)(void); 
const char *IDRV_VERSION_PROC_NAME = "NLSOUND_interfaceVersion";



/*
 * The static method which builds the sound driver instance
 */
ISoundDriver	*ISoundDriver::createDriver(bool useEax, IStringMapperProvider *stringMapper)
{
	ISDRV_CREATE_PROC	createSoundDriver = NULL;
	ISDRV_VERSION_PROC	versionDriver = NULL;

#ifdef NL_OS_WINDOWS

	// WINDOWS code.
	HINSTANCE			hInst;

	hInst = LoadLibrary(NLSOUND_DLL_NAME);

	if (!hInst)
	{
		throw ESoundDriverNotFound();
	}

	char buffer[1024], *ptr;
	SearchPath (NULL, NLSOUND_DLL_NAME, NULL, 1023, buffer, &ptr);
	nlinfo ("Using the library '"NLSOUND_DLL_NAME"' that is in the directory: '%s'", buffer);

	createSoundDriver = (ISDRV_CREATE_PROC) GetProcAddress (hInst, IDRV_CREATE_PROC_NAME);
	if (createSoundDriver == NULL)
	{
		nlinfo( "Error: %u", GetLastError() );
		throw ESoundDriverCorrupted();
	}

	versionDriver = (ISDRV_VERSION_PROC) GetProcAddress (hInst, IDRV_VERSION_PROC_NAME);
	if (versionDriver != NULL)
	{
		if (versionDriver()<ISoundDriver::InterfaceVersion)
			throw ESoundDriverOldVersion();
		else if (versionDriver()>ISoundDriver::InterfaceVersion)
			throw ESoundDriverUnknownVersion();
	}

#elif defined (NL_OS_UNIX)

	// Unix code
	void *handle = dlopen(NLSOUND_DLL_NAME, RTLD_NOW);

	if (handle == NULL)
	{
		nlwarning ("when loading dynamic library '%s': %s", NLSOUND_DLL_NAME, dlerror());
		throw ESoundDriverNotFound();
	}

	/* Not ANSI. Might produce a warning */
	createSoundDriver = (ISDRV_CREATE_PROC) dlsym (handle, IDRV_CREATE_PROC_NAME);
	if (createSoundDriver == NULL)
	{
		nlwarning ("when getting function in dynamic library '%s': %s", NLSOUND_DLL_NAME, dlerror());
		throw ESoundDriverCorrupted();
	}

	versionDriver = (ISDRV_VERSION_PROC) dlsym (handle, IDRV_VERSION_PROC_NAME);
	if (versionDriver != NULL)
	{
		if (versionDriver()<ISoundDriver::InterfaceVersion)
			throw ESoundDriverOldVersion();
		else if (versionDriver()>ISoundDriver::InterfaceVersion)
			throw ESoundDriverUnknownVersion();
	}

#else // NL_OS_UNIX
#error "Dynamic DLL loading not implemented!"
#endif // NL_OS_UNIX

	ISoundDriver *ret = createSoundDriver(useEax, stringMapper);
	if ( ret == NULL )
	{
		throw ESoundDriverCantCreateDriver();
	}
	return ret;
}


} // NLSOUND
