/** \file time_nl.cpp
 * CTime class
 *
 * $Id: time_nl.cpp,v 1.7 2001/09/24 14:13:50 cado Exp $
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

#include <time.h>

#include "nel/misc/types_nl.h"

#include "nel/misc/debug.h"


#ifdef NL_OS_WINDOWS
#	include <windows.h>
#elif defined (NL_OS_UNIX)
#	include <sys/time.h>
#	include <unistd.h>
#endif

#include "nel/misc/time_nl.h"

namespace NLMISC
{

/*
 *
 */
uint32 CTime::getSecondsSince1970 ()
{
	return (uint32) time (NULL);
}



/*
 *
 */
TTime CTime::getLocalTime ()
{
#ifdef NL_OS_WINDOWS

	static bool byperfcounter;
	static bool initdone = false;

	// Initialization
	if ( ! initdone )
	{
		byperfcounter = (getPerformanceTime() != 0);
		initdone = true;
	}

	/* Retrieve time is ms
     * Why do we prefer getPerformanceTime() to timeGetTime() ? Because on one dual-processor Win2k
	 * PC, we have noticed that timeGetTime() slows down when the client is running !!!
	 */
	if ( byperfcounter )
	{
		return (TTime)(ticksToSecond(getPerformanceTime()) * 1000.0f);
	}
	else
	{
		return timeGetTime();
	}

#elif defined (NL_OS_UNIX)

	struct timeval tv;

	if ( gettimeofday( &tv, NULL) == -1 )
	{
		nlerror ("Cannot get time of day.");
	}
	return (TTime)tv.tv_sec * (TTime)1000 + (TTime)tv.tv_usec / (TTime)1000;

#endif
}


/*
 *
 */
TTicks CTime::getPerformanceTime ()
{
#ifdef NL_OS_WINDOWS
	LARGE_INTEGER ret;
	if (QueryPerformanceCounter (&ret))
		return ret.QuadPart;
	else
		return 0;
#else // NL_OS_WINDOWS
	return 0;
#endif // NL_OS_WINDOWS
}


/*
 *
 */
double CTime::ticksToSecond (TTicks ticks)
{
#ifdef NL_OS_WINDOWS
	LARGE_INTEGER ret;
	if (QueryPerformanceFrequency(&ret))
	{
		return (double)(sint64)ticks/(double)ret.QuadPart;
	}
	else
		return 0.0;
#else // NL_OS_WINDOWS
	return 0.0;
#endif // NL_OS_WINDOWS
}


} // NLMISC
