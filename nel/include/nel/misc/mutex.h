/** \file mutex.h
 * OS independant class for the mutex management with Windows and Posix implementation
 * Classes CMutex, CSynchronized
 *
 * $Id: mutex.h,v 1.13 2002/08/23 15:40:12 cado Exp $
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
#include "nel/misc/time_nl.h"
#include <map>

#ifdef NL_OS_UNIX
#include <pthread.h>
#include <semaphore.h>
#endif

#undef MUTEX_DEBUG


namespace NLMISC {

// By default, all mutex use the CFairMutex class to avoid freeze problem.
#define CMutex CFairMutex

/**
 * Classic mutex implementation (not fairly)
 * Don't assume the mutex are recursive (ie don't call enter() several times
 * on the same mutex from the same thread without having called leave()) ;
 * and don't assume either the threads are woken-up in the same order as they
 * were put to sleep !
 *
 *\code
 CUnfairMutex m;
 m.enter ();
 // do critical stuffs
 m.leave ();
 *\endcode
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CUnfairMutex
{
public:

	/// Constructor
	CUnfairMutex();
	CUnfairMutex(const std::string &name);

	/// Destructor
	~CUnfairMutex();

	void enter ();
	void leave ();

private:

#ifdef NL_OS_WINDOWS
	void *Mutex;
#elif defined NL_OS_UNIX
	pthread_mutex_t mutex;
#else
#	error "No unfair mutex implementation for this OS"
#endif

};


#ifdef NL_OS_WINDOWS
/**
 * Trick to avoid including <windows.h> !
 * winbase.h: typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
 * The original RTL_CRITICAL_SECTION is in winnt.h.
 */
struct TNelRtlCriticalSection {
	void	*DebugInfo;
	long	 LockCount;
	long	 RecursionCount;
	void	*OwningThread;        // from the thread's ClientId->UniqueThread
	void	*LockSemaphore;
	uint32	 SpinCount;
};
#endif // NL_OS_WINDOWS


/**
 * Kind of "fair" mutex (implemented by semaphore on Unix, critical section on Windows)
 *
 *\code
 CUnfairMutex m;
 m.enter ();
 // do critical stuffs
 m.leave ();
 *\endcode
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 *
 *\code
 CFairMutex m;
 m.enter ();
 // do critical stuffs
 m.leave ();
 *\endcode
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CFairMutex
{
public:

	/// Constructor
	CFairMutex();
	CFairMutex(const std::string &name);

	/// Destructor
	~CFairMutex();

	void enter ();
	void leave ();

	std::string Name;

private:

#ifdef NL_OS_WINDOWS
	TNelRtlCriticalSection	_Cs;
#elif defined NL_OS_UNIX
	volatile sem_t			_Sem;
#else
#	error "No fair mutex implementation for this OS"
#endif


#ifdef MUTEX_DEBUG
	// debug stuffs
	void debugCreateMutex();
	void debugBeginEnter();
	void debugEndEnter();
	void debugLeave();
	void debugDeleteMutex();
#endif // MUTEX_DEBUG

};






/*
 * Debug info
 */
#ifdef MUTEX_DEBUG

struct TMutexLocks
{
	TMutexLocks(uint32 m=0) : TimeToEnter(0), TimeInMutex(0), Nb(0), WaitingMutex(0), MutexNum(m), ThreadHavingTheMutex(0xFFFFFFFF), Dead(false) {}
	
	uint32		TimeToEnter;			// cumulated time blocking on enter
	uint32		TimeInMutex;			// cumulated time between enter and leave
	uint32		Nb;						// number of calls of enter
	uint32		WaitingMutex;			// number of thread that waiting this mutex
    sint32		MutexNum;				// identifying a mutex
	uint		ThreadHavingTheMutex;	// thread id of the thread that is in this mutex (0xFFFFFFFF if no thread)
	bool		Dead;					// True if the mutex is dead (deleted)
	std::string	MutexName;				// Name of the mutex

	NLMISC::TTicks BeginEnter;
	NLMISC::TTicks EndEnter;
};

/// Inits the "mutex debugging info system"
void initAcquireTimeMap();

/// Gets the debugging info for all mutexes (call it evenly)
std::map<CMutex*,TMutexLocks>	getNewAcquireTimes();

/// The number of created mutexes (does not take in account the destroyed mutexes)
extern uint32 NbMutexes;

#endif // MUTEX_DEBUG





// By default, all synchronization use the CFairSynchronized class to avoid freeze problem.
#define CSynchronized CFairSynchronized

/**
 * This class ensure that the Value is accessed by only one thread. First you have to create a CSynchronized class with you type.
 * Then, if a thread want to modify or do anything on it, you create a CAccessor in a \b sub \b scope. You can modify the value
 * of the CUnfairSynchronized using the value() function \b until the end of the scope. So you have to put the smaller scope as you can.
 *
 * Internally, this class uses a CUnfairMutex object (see CUnfairMutex for programming considerations).
 *
 *\code
 // the value that you want to be thread safe.
 CUnfairSynchronized<int> val;
 { // create a new scope for the access
   // get an access to the value
   CUnfairSynchronized<int>::CAccessor acces(&val);
   // now, you have a thread safe access until the end of the scope, so you can do whatever you want. for example, change the value
   acces.value () = 10;
 } // end of the access
 *\endcode
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
template <class T>
class CUnfairSynchronized
{
public:

	CUnfairSynchronized (const std::string &name) : _Mutex(name) { }

	/**
	 * This class give you a thread safe access to the CSynchronized Value. Look at the example in the CSynchronized.
	 */
	class CAccessor
	{
		CUnfairSynchronized<T> *Synchronized;
	public:

		/// get the mutex or wait
		CAccessor(CUnfairSynchronized<T> *cs)
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

	friend class CUnfairSynchronized::CAccessor;

	/// The mutex of the synchronized value.
	volatile CUnfairMutex	_Mutex;

	/// The synchronized value.
	volatile T				_Value;
};


/**
 * This class is similar to CUnfairSynchronized, but it ensures that the threads
 * are woken-up in the same order as they were put to sleep.
 * Internally, it uses a CFairMutex object instead of a CUnfairMutex object.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
template <class T>
class CFairSynchronized
{
public:

	CFairSynchronized (const std::string &name) : _Cs(name) { }

	/**
	 * This class give you a thread safe access to the CFairSynchronized Value. Look at the example in CSynchronized.
	 */
	class CAccessor
	{
		CFairSynchronized<T> *Synchronized;
	public:

		/// get the mutex or wait
		CAccessor(CFairSynchronized<T> *cs)
		{
			Synchronized = cs;
			const_cast<CFairMutex&>(Synchronized->_Cs).enter();
		}

		/// release the mutex
		~CAccessor()
		{
			const_cast<CFairMutex&>(Synchronized->_Cs).leave();
		}

		/// access to the Value
		T &value()
		{
			return const_cast<T&>(Synchronized->_Value);
		}
	};

private:

	friend class CFairSynchronized::CAccessor;

	/// The mutex of the synchronized value.
	volatile CFairMutex	_Cs;

	/// The synchronized value.
	volatile T			_Value;
};

} // NLMISC


#endif // NL_MUTEX_H

/* End of mutex.h */
