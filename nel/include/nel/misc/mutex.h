/** \file mutex.h
 * OS independant class for the mutex management with Windows and Posix implementation
 * Classes CMutex, CSynchronized (using mutex) and CFairlySynchronized (using semaphore)
 *
 * $Id: mutex.h,v 1.9 2001/09/10 13:40:38 cado Exp $
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

#ifndef NL_MUTEX_H
#define NL_MUTEX_H

#include "nel/misc/types_nl.h"
#include <map>

#ifdef NL_OS_UNIX
#include <pthread.h>
#include <semaphore.h>
#endif

#undef MUTEX_DEBUG


namespace NLMISC {


/**
 * Classic mutex implementation
 * Don't assume the mutex are recursive (ie don't call enter() several times
 * on the same mutex from the same thread without having called leave()) ;
 * and don't assume either the threads are woken-up in the same order as they
 * were put to sleep !
 *
 *\code
 CMutex m;
 m.enter ();
 // do critical stuffs
 m.leave ();
 *\endcode
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CMutex
{
public:

	/// Constructor
	CMutex();

	/// Destructor
	~CMutex();

	void enter ();
	void leave ();

private:

#ifdef NL_OS_WINDOWS

	void *Mutex;

#elif defined NL_OS_UNIX

	pthread_mutex_t mutex;

#endif // NL_OS_WINDOWS

};


/*
 * Debug info
 */
#ifdef MUTEX_DEBUG

struct TMutexLocks
{
	TMutexLocks( bool locked=false, uint32 m=0 ) : Time(0), Nb(0), Locked(locked), MutexNum(m) {}
	
	uint32	Time;     // cumulated time blocking on enter
	uint32	Nb;       // number of calls of enter
	bool	Locked;   // thread is still locked
    uint32  MutexNum; // identifying a mutex
};

/// Inits the "mutex debugging info system"
void initAcquireTimeMap();

/// Gets the debugging info for all mutexes (call it evenly)
std::map<CMutex*,TMutexLocks>	getNewAcquireTimes();

/// The number of created mutexes (does not take in account the destroyed mutexes)
extern uint32 NbMutexes;

#endif // MUTEX_DEBUG






#ifdef NL_OS_WINDOWS

/**
 * Trick to avoid including <windows.h> !
 * winbase.h: typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
 * The original RTL_CRITICAL_SECTION is in winnt.h.
 */
struct TNelRtlCriticalSection {
    void *DebugInfo;
    long LockCount;
    long RecursionCount;
    void *OwningThread;        // from the thread's ClientId->UniqueThread
    void *LockSemaphore;
    unsigned long SpinCount;
};

#endif


/**
 * Kind of "fair" mutex (implemented by semaphore on Unix, critical section on Windows)
 */
class CCriticalSection
{
public:

	/// Constructor
	CCriticalSection();

	/// Destructor
	~CCriticalSection();

	void enter ();
	void leave ();

private:

#ifdef NL_OS_WINDOWS

	/// Windows critical section
	TNelRtlCriticalSection	_Cs;

#elif defined NL_OS_UNIX

	/// Unix semaphore
	volatile sem_t			_Sem;

#endif // NL_OS_WINDOWS

};




/**
 * This class ensure that the Value is accessed by only one thread. First you have to create a CSynchronized class with you type.
 * Then, if a thread want to modify or do anything on it, you create a CAccessor in a \b sub \b scope. You can modify the value
 * of the CSynchronized using the value() function \b until the end of the scope. So you have to put the smaller scope as you can.
 *
 * Internally, this class uses a CMutex object (see CMutex for programming considerations).
 *
 *\code
 // the value that you want to be thread safe.
 CSynchronized<int> val;
 { // create a new scope for the access
   // get an access to the value
   CSynchronized<int>::CAccessor acces(&val);
   // now, you have a thread safe access until the end of the scope, so you can do whatever you want. for example, change the value
   acces.value () = 10;
 } // end of the access
 *\endcode
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
template <class T>
class CMutexSynchronized
{
public:

	/**
	 * This class give you a thread safe access to the CSynchronized Value. Look at the example in the CSynchronized.
	 */
	class CAccessor
	{
		CMutexSynchronized<T> *Synchronized;
	public:

		/// get the mutex or wait
		CAccessor(CMutexSynchronized<T> *cs)
		{
			Synchronized = cs;
			const_cast<CMutex&>(Synchronized->_Mutex).enter();
		}

		/// release the mutex
		~CAccessor()
		{
			const_cast<CMutex&>(Synchronized->_Mutex).leave();
		}

		/// access to the Value
		T &value()
		{
			return const_cast<T&>(Synchronized->_Value);
		}
	};

private:

	friend class CMutexSynchronized::CAccessor;

	/// The mutex of the synchronized value.
	volatile CMutex		_Mutex;

	/// The synchronized value.
	volatile T			_Value;
};


/**
 * This class is similar to CMutexSynchronized, but it ensures that the threads
 * are woken-up in the same order as they were put to sleep.
 * Internally, it uses a CCriticalSection object instead of a CMutex object.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
template <class T>
class CFairlySynchronized
{
public:

	/**
	 * This class give you a thread safe access to the CFairlySynchronized Value. Look at the example in CSynchronized.
	 */
	class CAccessor
	{
		CFairlySynchronized<T> *Synchronized;
	public:

		/// get the mutex or wait
		CAccessor(CFairlySynchronized<T> *cs)
		{
			Synchronized = cs;
			const_cast<CCriticalSection&>(Synchronized->_Cs).enter();
		}

		/// release the mutex
		~CAccessor()
		{
			const_cast<CCriticalSection&>(Synchronized->_Cs).leave();
		}

		/// access to the Value
		T &value()
		{
			return const_cast<T&>(Synchronized->_Value);
		}
	};

private:

	friend class CFairlySynchronized::CAccessor;

	/// The mutex of the synchronized value.
	volatile CCriticalSection	_Cs;

	/// The synchronized value.
	volatile T					_Value;
};


// Shortcut
#define CSynchronized CFairlySynchronized



} // NLMISC


#endif // NL_MUTEX_H

/* End of mutex.h */
