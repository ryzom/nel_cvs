/** \file shared_memory.cpp
 * Encapsulation of shared memory APIs
 *
 * $Id: shared_memory.cpp,v 1.1 2002/08/08 10:18:21 cado Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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
#include "nel/misc/shared_memory.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#else
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
#endif

using namespace std;


namespace NLMISC {


#ifdef NL_OS_WINDOWS
	// Storage for file handles, necessary to close the handles
	map<void*,HANDLE>	AccessAddressesToHandles;
#endif


/*
 * Create a shared memory segment
 */
void			*CSharedMemory::createSharedMemory( TSharedMemId sharedMemId, uint32 size )
{
#ifdef NL_OS_WINDOWS

	// Create a file mapping backed by the virtual memory swap file (not a data file)
	HANDLE hMapFile = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, sharedMemId );
	if ( (hMapFile == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS) )
		return NULL;

	// Map the file into memory address space
	void *accessAddress = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	AccessAddressesToHandles.insert( make_pair( accessAddress, hMapFile ) );
	return accessAddress;

#else

	// Create a shared memory segment
	int shmid = shmget( sharedMemId, size, IPC_CREATE | 0666 );
	if ( shmid == -1 )
		return NULL;

	// Map the segment into memory address space
	char *accessAddress = shmat( shmid, 0, 0 );
	if ( accessAddress == -1 )
		return NULL;
	else
	{
		// Set the segment to auto-destroying (when the last process detaches)
		shmctl( shmid, IPC_RMID, 0 );
		return accessAddress;
	}

#endif
}


/*
 * Get access to an existing shared memory segment
 */
void			*CSharedMemory::accessSharedMemory( TSharedMemId sharedMemId )
{
#ifdef NL_OS_WINDOWS

	// Open the existing file mapping by name
	HANDLE hMapFile = OpenFileMapping( FILE_MAP_ALL_ACCESS, false, sharedMemId );
	if ( hMapFile == NULL )
		return NULL;

	// Map the file into memory address space
	void *accessAddress = MapViewOfFile( hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	AccessAddressesToHandles.insert( make_pair( accessAddress, hMapFile ) );
	return accessAddress;
	
#else

	// Open an existing shared memory segment
	int shmid = shmget( sharedMemId, size, 0666 );
	if ( shmid == -1 )
		return NULL;

	// Map the segment into memory address space
	char *accessAddress = shmat( shmid, 0, 0 );
	if ( accessAddress == -1 )
		return NULL;
	else
		return accessAddress;

#endif
}


/*
 * Close a shared memory segment
 */
bool			CSharedMemory::closeSharedMemory( void *accessAddress )
{
#ifdef NL_OS_WINDOWS

	// Unmap the file from memory address space
	if ( UnmapViewOfFile( accessAddress ) == 0 )
		return false;

	// Close the corresponding handle
	map<void*,HANDLE>::iterator im = AccessAddressesToHandles.find( accessAddress );
	if ( im != AccessAddressesToHandles.end() )
	{
		CloseHandle( (*im).second );
		return true;
	}
	else
	{
		return false;
	}

#else

	// Detach the shared memory segment
	return ( shmdt( accessAddress ) != -1 );
	
#endif
}


} // NLMISC
