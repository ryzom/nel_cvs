/** \file mutex.cpp
 * Class CMutex
 *
 * $Id: mutex.cpp,v 1.17 2001/09/10 13:42:50 cado Exp $
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

#define _GNU_SOURCE


#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/mutex.h"
#include "nel/misc/time_nl.h"

#ifdef MUTEX_DEBUG
#include <iostream>
#endif

using namespace std;


/****************
 * Windows code *
 ****************/

#ifdef NL_OS_WINDOWS

// these defines are for IsDebuggerPresent(). It'll not compile on windows 95
// just comment this and the IsDebuggerPresent to compile on windows 95
#define _WIN32_WINDOWS	0x0410
#define WINVER			0x0400
#include <windows.h>


namespace NLMISC {


/*
 * Windows version
 */
CMutex::CMutex()
{
	// Create a mutex with no initial owner.
	Mutex = (void *) CreateMutex (NULL, FALSE, NULL);
	nlassert (Mutex != NULL);
}


/*
 * Windows version
 */
CMutex::~CMutex()
{
	CloseHandle( Mutex );
}


/*
 * Windows version
 */
void CMutex::enter()
{
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
#endif // NL_DEBUG
	switch (dwWaitResult)
	{
	// The thread got mutex ownership.
	case WAIT_OBJECT_0:		break;
	// Cannot get mutex ownership due to time-out.
	case WAIT_TIMEOUT:		nlerror ("Dead lock in a mutex (or more that 10s for the critical section");
	// Got ownership of the abandoned mutex object.
	case WAIT_ABANDONED:	nlerror ("A thread forgot to release the mutex");
    }
}


/*
 * Windows version
 */
void CMutex::leave()
{
	if (ReleaseMutex(Mutex) == 0)
	{
		nlerror ("error while releasing the mutex (0x%x %d), %p", GetLastError(), GetLastError(), Mutex);
	}
}


/*
 * Windows version
 */
CCriticalSection::CCriticalSection()
{
	// Check that the CRITICAL_SECTION structure has not changed
	nlassert( sizeof(TNelRtlCriticalSection)==sizeof(CRITICAL_SECTION) );

#if (_WIN32_WINNT >= 0x0500)
	DWORD dwSpinCount = 0x80000000; // set high-order bit to use preallocation
	if ( ! InitializeCriticalSectionAndSpinCount( &_Cs, dwSpinCount ) )
	{
		nlerror( "Error entering critical section" );
	}
#else
	InitializeCriticalSection( (CRITICAL_SECTION*)&_Cs );
#endif
}



/*
 * Windows version
 */
CCriticalSection::~CCriticalSection()
{
	DeleteCriticalSection( (CRITICAL_SECTION*)&_Cs );
}


/*
 * Windows version
 */
void CCriticalSection::enter()
{
	EnterCriticalSection( (CRITICAL_SECTION*)&_Cs );
}


/*
 * Windows version
 */
void CCriticalSection::leave()
{
	LeaveCriticalSection( (CRITICAL_SECTION*)&_Cs );
}








/*************
 * Unix code *
 *************/

#elif defined NL_OS_UNIX

#include <pthread.h>
#include <errno.h>


/*
 * Clanlib authors say: "We need to do this because the posix threads library
 * under linux obviously suck:"
 */
extern "C"
{
	int pthread_mutexattr_setkind_np( pthread_mutexattr_t *attr, int kind );
}


namespace NLMISC {


/*
 * Unix version
 */
CMutex::CMutex()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	// Fast mutex. Note: on Windows all mutexes are recursive
	pthread_mutexattr_setkind_np( &attr, PTHREAD_MUTEX_FAST_NP ); //PTHREAD_MUTEX_ERRORCHECK_NP );//PTHREAD_MUTEX_ADAPTIVE_NP );//PTHREAD_MUTEX_RECURSIVE_NP );
	pthread_mutex_init( &mutex, &attr );
	pthread_mutexattr_destroy( &attr );
}


/*
 * Unix version
 */
CMutex::~CMutex()
{
	pthread_mutex_destroy( &mutex );
}


/*
 * Unix version
 */
void CMutex::enter()
{
	//cout << getpid() << ": Locking " << &mutex << endl;
	if ( pthread_mutex_lock( &mutex ) != 0 )
	{
	  //cout << "Error locking a mutex " << endl;
		nlerror( "Error locking a mutex" );
	}
	/*else
	{
	  cout << getpid() << ": Owning " << &mutex << endl;
	}*/
}


/*
 * Unix version
 */
void CMutex::leave()
{
	//int errcode;
	//cout << getpid() << ": Unlocking " << &mutex << endl;
	if ( (/*errcode=*/pthread_mutex_unlock( &mutex )) != 0 )
	{
	 /* switch ( errcode )
	    {
	    case EINVAL: cout << "INVAL" << endl; break;
	    case EPERM: cout << "PERM" << endl; break;
	    default: cout << "OTHER" << endl;
	    }
	  */
	  //cout << "Error unlocking a mutex " /*<< &mutex*/ << endl;
		nlerror( "Error unlocking a mutex" );
	}
	/*else
	{
	  cout << getpid() << ": Released " << &mutex << endl;
	}*/
}


/*
 * Unix version
 */
CCriticalSection::CCriticalSection()
{
	sem_init( const_cast<sem_t*>(&_Sem), 0, 1 );
}


/*
 * Unix version
 */
CCriticalSection::~CCriticalSection()
{
	sem_destroy( const_cast<sem_t*>(&_Sem) ); // needs that no thread is waiting on the semaphore
}


/*
 * Unix version
 */
void CCriticalSection::enter()
{
	sem_wait( const_cast<sem_t*>(&_Sem) );
}


/*
 * Unix version
 */
void CCriticalSection::leave()
{
	sem_post( const_cast<sem_t*>(&_Sem) );
}


#endif // NL_OS_WINDOWS/NL_OS_LINUX







/******************
 * Debugging code *
 ******************/
	
#ifdef MUTEX_DEBUG

map<CMutex*,TMutexLocks>	*AcquireTime = NULL;
uint32						NbMutexes = 0;
CMutex						*ATMutex = NULL;
bool						InitAT = true;


/// Inits the "mutex debugging info system"
void initAcquireTimeMap()
{
	if ( InitAT )
	{
		ATMutex = new CMutex();
		AcquireTime = new map<CMutex*,TMutexLocks>;
		InitAT = false;
	}
}


/// Gets the debugging info for all mutexes (call it evenly, e.g. once per second)
map<CMutex*,TMutexLocks>	getNewAcquireTimes()
{
	map<CMutex*,TMutexLocks>	m;
	ATMutex->enter();

	// Copy map
	m = *AcquireTime;

	// Reset map
	map<CMutex*,TMutexLocks>::iterator im;
	for ( im=AcquireTime->begin(); im!=AcquireTime->end(); ++im )
	{
		(*im).second.Time = 0;
		(*im).second.Nb = 0;
		(*im).second.Locked = false;
	}

	ATMutex->leave();
	return m;
}


void debugCreateMutex()
{
	if ( ! InitAT )
	{
		ATMutex->enter();
		AcquireTime->insert( make_pair( this, TMutexLocks(false,NbMutexes) ) );
		NbMutexes++;
		ATMutex->leave();
		char dbgstr [256];
		smprintf( dbgstr, 256, "MUTEX: Creating mutex %p (number %u)\n", this, NbMutexes-1 );
#ifdef NL_OS_WINDOWS
		if ( IsDebuggerPresent() )
			OutputDebugString( dbgstr );
#endif
		cout << dbgstr << endl;
	}
}


void debugBeginEnter()
{
	TTime before;
	
	if ( ( this != ATMutex ) && ( ATMutex != NULL ) )
	{
		ATMutex->enter();
		(*AcquireTime)[this].Locked = true;
		ATMutex->leave();
		before = CTime::getLocalTime();
	}
}


void debugEndEnter()
{
//	printf("1");
/*	char str[1024];
	sprintf(str, "enter %8p %8p %8p\n", this, Mutex, getThreadId ());
	if (Mutex == (void*)0x88)
	{
		OutputDebugString (str);
		if (entered) __asm int 3;
		entered = true;
	}
*/
	if ( ( this != ATMutex ) && ( ATMutex != NULL ) )
	{
		TTime diff = CTime::getLocalTime()-before;
		ATMutex->enter();
		(*AcquireTime)[this].Time += (uint32)diff;
		(*AcquireTime)[this].Nb += 1;
		(*AcquireTime)[this].Locked = false;
		ATMutex->leave();
	}
}


void debugLeave()
{
//	printf( "0" );
/*	char str[1024];
	sprintf(str, "leave %8p %8p %8p\n", this, Mutex, getThreadId ());
	if (Mutex == (void*)0x88)
	{
		OutputDebugString (str);
		if (!entered) __asm int 3;
		entered = false;
	}
*/
}

#endif // MUTEX_DEBUG


} // NLMISC
