/** \file common.cpp
 * Common functions
 *
 * $Id: common.cpp,v 1.6 2001/05/02 08:44:44 lecroart Exp $
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

#include "nel/misc/debug.h"

#include "nel/misc/common.h"

#ifdef NL_OS_WINDOWS
#include <windows.h>
#elif defined NL_OS_UNIX
#include <unistd.h>
#endif

using namespace std;


namespace	NLMISC
{

/*
 * Portable Sleep() function that suspends the execution of the calling thread for a number of milliseconds.
 * Note: the resolution of the timer is system-dependant and may be more than 1 millisecond.
 */
void nlSleep( uint32 ms )
{
#ifdef NL_OS_WINDOWS

// Yoyo: TODODO: BUG WITH DEBUG/_CONSOLE!!!! a Sleep(0) "block" the other thread!!!
// console suxxxx
#ifdef NL_DEBUG
	ms= std::max(ms, (uint32)1);
#endif

	Sleep( ms );
#elif defined NL_OS_UNIX
	usleep( ms*1000 );
#endif
}


/*
 * Returns Thread Id (note: on Linux, Process Id is the same as the Thread Id)
 */
uint getThreadId()
{
#ifdef NL_OS_WINDOWS
	return GetCurrentThreadId();
#elif defined NL_OS_UNIX
	return getpid();
#endif

}


/*
 * Returns a readable string from a vector of bytes. '\0' are replaced by ' '
 */
string stringFromVector( const vector<uint8>& v )
{
	string s;

	// Copy contents
	s.resize( v.size() );
	memcpy( &*s.begin(), &*v.begin(), v.size() );

	// Replace '\0' characters
	string::iterator is;
	for ( is=s.begin(); is!=s.end(); ++is )
	{
		if ( ! isprint((*is)) )
		{
			(*is) = ' ';
		}
	}

	return s;
}

Exception::Exception() : _Reason("Unknown Exception")
{
	nlwarning("Exception will be launch: %s", _Reason.c_str());
}

Exception::Exception(const std::string &reason) : _Reason(reason)
{
	nlwarning("Exception will be launch: %s", _Reason.c_str());
}

Exception::Exception(const char *format, ...)
{
	NLMISC_CONVERT_VARGS (_Reason, format, NLMISC::MaxCStringSize);
	nlwarning("Exception will be launch: %s", _Reason.c_str());
}

const char	*Exception::what() const throw()
{
	return _Reason.c_str();
}


} // NLMISC