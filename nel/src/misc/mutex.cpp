/** \file mutex.cpp
 * mutex and synchronization implementation
 *
 * $Id: mutex.cpp,v 1.30 2002/10/18 13:25:39 coutelas Exp $
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include "nel/misc/mutex.h"
#include "nel/misc/time_nl.h"

#ifdef MUTEX_DEBUG
#include <iostream>
#endif

using namespace std;

#ifndef MUTEX_DEBUG
#define debugCreateMutex() ;
#define debugBeginEnter() ;
#define debugEndEnter() ;
#define debugLeave() ;
#define debugDeleteMutex() ;
#endif


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

CUnfairMutex::CUnfairMutex()
{
	// Create a mutex with no initial owner.
	Mutex = (void *) CreateMutex (NULL, FALSE, NULL);
	nlassert (Mutex != NULL);
}


CUnfairMutex::CUnfairMutex(const std::string &name)
{
	// Create a mutex with no initial owner.
	Mutex = (void *) CreateMutex (NULL, FALSE, NULL);
	nlassert (Mutex != NULL);
}


/*
 * Windows version
 */
CUnfairMutex::~CUnfairMutex()
{
	CloseHandle( Mutex );
}


/*
 * Windows version
 */
void CUnfairMutex::enter()
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
void CUnfairMutex::leave()
{
	if (ReleaseMutex(Mutex) == 0)
	{
		nlerror ("error while releasing the mutex (0x%x %d), %p", GetLastError(), GetLastError(), Mutex);
	}
}


/*
 * Windows version
 */
CFairMutex::CFairMutex()
{
	Name = "unset mutex name";

	debugCreateMutex();

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


CFairMutex::CFairMutex(const string &name)
{
	Name = name;

#ifdef MUTEX_DEBUG
	debugCreateMutex();
#endif

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
CFairMutex::~CFairMutex()
{
	DeleteCriticalSection( (CRITICAL_SECTION*)&_Cs );

	debugDeleteMutex();
}


/*
 * Windows version
 */
void CFairMutex::enter()
{
	debugBeginEnter();

	EnterCriticalSection( (CRITICAL_SECTION*)&_Cs );

	debugEndEnter();
}


/*
 * Windows version
 */
void CFairMutex::leave()
{
	LeaveCriticalSection( (CRITICAL_SECTION*)&_Cs );

	debugLeave();
}








/*************
 * Unix code *
 *************/

#elif defined NL_OS_UNIX

#include <pthread.h>
#include <errno.h>
#include <unistd.h>


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
CUnfairMutex::CUnfairMutex(const std::string &name)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init( &attr );
	// Fast mutex. Note: on Windows all mutexes are recursive
	pthread_mutexattr_setkind_np( &attr, PTHREAD_MUTEX_ADAPTIVE_NP ); //PTHREAD_MUTEX_ERRORCHECK_NP );//PTHREAD_MUTEX_ADAPTIVE_NP );//PTHREAD_MUTEX_RECURSIVE_NP );
	pthread_mutex_init( &mutex, &attr );
	pthread_mutexattr_destroy( &attr );
}


/*
 * Unix version
 */
CUnfairMutex::~CUnfairMutex()
{
	pthread_mutex_destroy( &mutex );
}


/*
 * Unix version
 */
void CUnfairMutex::enter()
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
void CUnfairMutex::leave()
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
CFairMutex::CFairMutex() : _OwnerPid( ~0 )
{
	sem_init( const_cast<sem_t*>(&_Sem), 0, 1 );
}


CFairMutex::CFairMutex(	const std::string &name ) : _OwnerPid( ~0 )
{
	sem_init( const_cast<sem_t*>(&_Sem), 0, 1 );
}


/*
 * Unix version
 */
CFairMutex::~CFairMutex()
{
	sem_destroy( const_cast<sem_t*>(&_Sem) ); // needs that no thread is waiting on the semaphore
}


/*
 * Unix version
 */
void CFairMutex::enter()
{
	// Allow the call to be reentrant (do not wait if same thread)
	uint pid = getpid();
	if ( pid != _OwnerPid )
	{
		_OwnerPid = pid;
		sem_wait( const_cast<sem_t*>(&_Sem) );
	}
}


/*
 * Unix version
 */
void CFairMutex::leave()
{
	if ( getpid() == _OwnerPid )
	{
		sem_post( const_cast<sem_t*>(&_Sem) );
		_OwnerPid = ~0;
	}
}


#endif // NL_OS_WINDOWS/NL_OS_LINUX







/******************
 * Debugging code *
 ******************/
	
#ifdef MUTEX_DEBUG

map<CFairMutex*,TMutexLocks>	*AcquireTime = NULL;
uint32						NbMutexes = 0;
CFairMutex						*ATMutex = NULL;
bool						InitAT = true;


/// Inits the "mutex debugging info system"
void initAcquireTimeMap()
{
	if ( InitAT )
	{
		ATMutex = new CFairMutex("ATMutex");
		AcquireTime = new map<CFairMutex*,TMutexLocks>;
		InitAT = false;
	}
}


/// Gets the debugging info for all mutexes (call it evenly, e.g. once per second)
map<CFairMutex*,TMutexLocks>	getNewAcquireTimes()
{
	map<CMutex*,TMutexLocks>	m;
	ATMutex->enter();

	// Copy map
	m = *AcquireTime;

	// Reset map
/*	map<CMutex*,TMutexLocks>::iterator im;
	for ( im=AcquireTime->begin(); im!=AcquireTime->end(); ++im )
	{
		(*im).second.Time = 0;
		(*im).second.Nb = 0;
		(*im).second.Locked = false;
	}
*/
	ATMutex->leave();
	return m;
}


////////////////////////
////////////////////////

void CFairMutex::debugCreateMutex()
{
/*	if ( ! InitAT )
	{
		ATMutex->enter();
		AcquireTime->insert( make_pair( this, TMutexLocks(NbMutexes) ) );
		NbMutexes++;
		ATMutex->leave();
		char dbgstr [256];
		smprintf( dbgstr, 256, "MUTEX: Creating mutex %p %s (number %u)\n", this, Name.c_str(), NbMutexes-1 );
#ifdef NL_OS_WINDOWS
		if ( IsDebuggerPresent() )
			OutputDebugString( dbgstr );
#endif
		cout << dbgstr << endl;
	}
*/}

void CFairMutex::debugDeleteMutex()
{
	if ( (this!=ATMutex ) && (ATMutex!=NULL) )
	{
		ATMutex->enter();
		(*AcquireTime)[this].Dead = true;
		ATMutex->leave();
	}
}

void CFairMutex::debugBeginEnter()
{
	if ( (this!=ATMutex ) && (ATMutex!=NULL) )
	{
		TTicks t = CTime::getPerformanceTime();

		ATMutex->enter();
		std::map<CMutex*,TMutexLocks>::iterator it = (*AcquireTime).find (this);
		if (it == (*AcquireTime).end())
		{
			AcquireTime->insert( make_pair( this, TMutexLocks(NbMutexes++) ) );
			char dbgstr [256];
			smprintf( dbgstr, 256, "MUTEX: Creating mutex %p %s (number %u)\n", this, Name.c_str(), NbMutexes-1 );
#ifdef NL_OS_WINDOWS
			if ( IsDebuggerPresent() ) OutputDebugString( dbgstr );
#endif
			cout << dbgstr << endl;

			it = (*AcquireTime).find (this);
			(*it).second.MutexName = Name;
		}
		(*it).second.WaitingMutex++;
		(*it).second.BeginEnter = t;
		ATMutex->leave();
	}
}


void CFairMutex::debugEndEnter()
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
		TTicks t = CTime::getPerformanceTime();
		ATMutex->enter();
		if ((uint32)(t-(*AcquireTime)[this].BeginEnter) > (*AcquireTime)[this].TimeToEnter)
			(*AcquireTime)[this].TimeToEnter = (uint32)(t-(*AcquireTime)[this].BeginEnter);
		(*AcquireTime)[this].Nb += 1;
		(*AcquireTime)[this].WaitingMutex--;
		(*AcquireTime)[this].ThreadHavingTheMutex = getThreadId();
		(*AcquireTime)[this].EndEnter = t;
		ATMutex->leave();
	}
}


void CFairMutex::debugLeave()
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

	if ( ( this != ATMutex ) && ( ATMutex != NULL ) )
	{
		TTicks Leave = CTime::getPerformanceTime();
		ATMutex->enter();
		if ((uint32)(Leave-(*AcquireTime)[this].EndEnter) > (*AcquireTime)[this].TimeInMutex)
			(*AcquireTime)[this].TimeInMutex = (uint32)(Leave-(*AcquireTime)[this].EndEnter);
		(*AcquireTime)[this].Nb += 1;
		(*AcquireTime)[this].WaitingMutex = false;
		(*AcquireTime)[this].ThreadHavingTheMutex = 0xFFFFFFFF;
		ATMutex->leave();
	}

}

#endif // MUTEX_DEBUG

} // NLMISC
