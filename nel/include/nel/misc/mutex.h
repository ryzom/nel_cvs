/** \file mutex.h
 * class CMutex
 *
 * $Id: mutex.h,v 1.5 2001/04/20 13:36:21 cado Exp $
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
#endif


namespace NLMISC {


/**
 * classic mutex implementation
 *\code
 CMutex m;
 m.enter ();
 // do critical stuffs
 m.leave ();
 *\endcode
 * \author Vianney Lecroart
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


// DEBUG
struct TMutexLocks
{
	TMutexLocks( bool locked=false ) : Time(0), Nb(0), Locked(locked) {}
	
	uint32	Time;
	uint32	Nb;
	bool	Locked;
};
void initAcquireTimeMap();
std::map<CMutex*,TMutexLocks>	getNewAcquireTimes();


/**
 * This class ensure that the Value is access by only one thread. First you have to create a CSynchronized class with you type.
 * Then, if a thread want to modify or do anything on it, you create a CAccessor in a \b sub \b scope. You can modify the value
 * of the CSynchronized using the value() function \b until the end of the scope. So you have to put the smaller scope as you can.
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
template <class T> class CSynchronized
{
public:

	/**
	 * This class give you a thread safe access to the CSynchronized Value. Look the example in the CSynchronized.
	 */
	class CAccessor
	{
		CSynchronized<T> *Synchronized;
	public:

		/// get the mutex or wait
		CAccessor(CSynchronized<T> *cs)
		{
			Synchronized = cs;
			const_cast<CMutex&>(Synchronized->Mutex).enter();
		}

		/// release the mutex
		~CAccessor()
		{
			const_cast<CMutex&>(Synchronized->Mutex).leave();
		}

		/// access to the Value
		T &value()
		{
			return const_cast<T&>(Synchronized->Value);
		}
	};

	/// The mutex of the synchronized value. \warning Don't access to the Mutex directly.
	volatile	CMutex	Mutex;
	/// The synchronized value. \warning Don't access to the Value directly.
	volatile	T		Value;
};



} // NLMISC


#endif // NL_MUTEX_H

/* End of mutex.h */
