/** \file time_nl.cpp
 * CTime class
 *
 * $Id: time_nl.cpp,v 1.15 2002/04/15 12:58:16 lecroart Exp $
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

#include <time.h>

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


	/* Now we have noticed that on all WinNT4 PC the getPerformanceTime can give us value that
	 * are less than previous
	 */

	//if ( byperfcounter )
	//{
	//	return (TTime)(ticksToSecond(getPerformanceTime()) * 1000.0f);
	//}
	//else
	//{
		return timeGetTime();
	//}

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

#ifdef HAVE_X86
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
#else HAVE_X86
	static bool firstWarn = true;
	if (firstWarn)
	{
		nlwarning ("TTicks CTime::getPerformanceTime () is not implemented for your processor, returning 0");
		firstWarn = false;
	}
	return 0;
#endif // HAVE_X86

#endif // NL_OS_WINDOWS
}
/*
#define GETTICKS(t) asm volatile ("push %%esi\n\t" "mov %0, %%esi" : : "r" (t)); \
                      asm volatile ("push %eax\n\t" "push %edx"); \
                      asm volatile ("rdtsc"); \
                      asm volatile ("movl %eax, (%esi)\n\t" "movl %edx, 4(%esi)"); \
                      asm volatile ("pop %edx\n\t" "pop %eax\n\t" "pop %esi"); 
*/


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
#endif // NL_OS_WINDOWS
	{
		static bool benchFrequency = true;
		static sint64 freq = 0;
		if (benchFrequency)
		{
			// try to have an estimation of the cpu frequency

			TTicks tickBefore = getPerformanceTime ();
			TTicks tickAfter = tickBefore;
			TTime timeBefore = getLocalTime ();
			TTime timeAfter = timeBefore;
			while (true)
			{
				if (timeAfter - timeBefore > 1000)
					break;
				timeAfter = getLocalTime ();
				tickAfter = getPerformanceTime ();
			}

			TTime timeDelta = timeAfter - timeBefore;
			TTicks tickDelta = tickAfter - tickBefore;

			freq = 1000 * tickDelta / timeDelta;
			benchFrequency = false;
		}

		return (double)(sint64)ticks/(double)freq;
	}
}


} // NLMISC
