/** \file mutex.cpp
 * <File description>
 *
 * $Id: mutex.cpp,v 1.1 2000/12/15 14:51:53 lecroart Exp $
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
#include <windows.h>
#endif // NL_OS_WINDOWS

namespace NLMISC {


/*
 * Constructor
 */
CMutex::CMutex()
{
	// Create a mutex with no initial owner.
	Mutex = (void *) CreateMutex (NULL, FALSE, NULL);
	nlassert (Mutex != NULL)
}

void CMutex::enter ()
{
#ifdef NL_OS_WINDOWS
    // Request ownership of mutex during 10s
	DWORD dwWaitResult = WaitForSingleObject (Mutex, 10000);
	switch (dwWaitResult)
	{
	// The thread got mutex ownership.
	case WAIT_OBJECT_0:		break;
	// Cannot get mutex ownership due to time-out.
	case WAIT_TIMEOUT:		nlerror ("Dead lock in a mutex (or more that 5s for the critical section");
	// Got ownership of the abandoned mutex object.
	case WAIT_ABANDONED:	nlerror ("A thread forgot to release the mutex");
    }
#endif // NL_OS_WINDOWS
}

void CMutex::leave ()
{
#ifdef NL_OS_WINDOWS
	nlverify (ReleaseMutex(Mutex));
#endif // NL_OS_WINDOWS
}


} // NLMISC
