/** \file mutex.cpp
 * <File description>
 *
 * $Id: mutex.cpp,v 1.5 2001/03/09 14:57:00 cado Exp $
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

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/mutex.h"

#ifdef NL_OS_WINDOWS

// these defines is for IsDebuggerPresent(). it'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on windows 95
#define _WIN32_WINDOWS	0x0410
#define WINVER			0x0400
#include <windows.h>

#elif defined NL_OS_UNIX

#include <pthread.h>

#endif // NL_OS_WINDOWS


namespace NLMISC {


#ifdef NL_OS_UNIX

/*
 * Clanlib authors say: "We need to do this because the posix threads library
 * under linux obviously suck:"
 */
extern "C"
{
	int pthread_mutexattr_setkind_np( pthread_mutexattr_t *attr, int kind );
}

#endif // NL_OS_UNIX


/*
 * Constructor
 */
CMutex::CMutex()
{
#ifdef NL_OS_WINDOWS

	// Create a mutex with no initial owner.
	Mutex = (void *) CreateMutex (NULL, FALSE, NULL);
	nlassert (Mutex != NULL);

#elif defined NL_OS_UNIX

	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_setkind_np( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
	pthread_mutex_init( &mutex, &attr );
	pthread_mutexattr_destroy( &attr );

#endif // NL_OS_WINDOWS
}


/*
 * Destructor
 */
CMutex::~CMutex()
{
#ifdef NL_OS_UNIX
	pthread_mutex_destroy( &mutex );
#endif // NL_OS_UNIX
}


/*
 * enter
 */
void CMutex::enter ()
{
#ifdef NL_OS_WINDOWS

#ifdef NL_DEBUG
	DWORD timeout;
	if ( IsDebuggerPresent() )
		timeout = INFINITE;
	else
		timeout = 10000;

    // Request ownership of mutex
	DWORD dwWaitResult = WaitForSingleObject (Mutex, timeout);
#else
    // Request ownership of mutex during 10s
	DWORD dwWaitResult = WaitForSingleObject (Mutex, 10000);
#endif
	switch (dwWaitResult)
	{
	// The thread got mutex ownership.
	case WAIT_OBJECT_0:		break;
	// Cannot get mutex ownership due to time-out.
	case WAIT_TIMEOUT:		nlerror ("Dead lock in a mutex (or more that 10s for the critical section");
	// Got ownership of the abandoned mutex object.
	case WAIT_ABANDONED:	nlerror ("A thread forgot to release the mutex");
    }

#elif defined NL_OS_UNIX

	if ( pthread_mutex_lock( &mutex ) != 0 )
	{
		nlerror( "Error locking a mutex" );
	}

#endif // NL_OS_WINDOWS
}


/*
 * leave
 */
void CMutex::leave ()
{
#ifdef NL_OS_WINDOWS

	nlverify (ReleaseMutex(Mutex));

#elif defined NL_OS_UNIX

	if ( pthread_mutex_unlock( &mutex ) != 0 )
	{
		nlerror( "Error unlocking a mutex" );
	}

#endif // NL_OS_WINDOWS
}


} // NLMISC
