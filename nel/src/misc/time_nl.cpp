/** \file time_nl.cpp
 * CTime class
 *
 * $Id: time_nl.cpp,v 1.22 2007/03/09 09:49:30 boucher Exp $
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

#include <ctime>

#ifdef NL_OS_WINDOWS
#	include <windows.h>
#elif defined (NL_OS_UNIX)
#	include <sys/time.h>
#	include <unistd.h>
#endif

#include "nel/misc/time_nl.h"
#include "nel/misc/sstring.h"

namespace NLMISC
{

/* Return the number of second since midnight (00:00:00), January 1, 1970,
 * coordinated universal time, according to the system clock.
 * This values is the same on all computer if computers are synchronized (with NTP for example).
 */
uint32 CTime::getSecondsSince1970 ()
{
	return nl_time (NULL);
}

/** Return the number of second since midnight (00:00:00), January 1, 1970,
 * coordinated universal time, according to the system clock.
 * The time returned is UTC (aka GMT+0), ie it does not have the local time ajustement 
 * nor it have the daylight saving ajustement.
 * This values is the same on all computer if computers are synchronized (with NTP for example).
 */
uint32	CTime::getSecondsSince1970UTC ()
{
	// get the local time
	time_t nowLocal = time(NULL);
	// convert it to GMT time (UTC)
	struct tm * timeinfo;
	timeinfo = gmtime(&nowLocal);
	return nl_mktime(timeinfo);
}

/* Return the local time in milliseconds.
 * Use it only to measure time difference, the absolute value does not mean anything.
 * On Unix, getLocalTime() will try to use the Monotonic Clock if available, otherwise
 * the value can jump backwards if the system time is changed by a user or a NTP time sync process.
 * The value is different on 2 different computers; use the CUniTime class to get a universal
 * time that is the same on all computers.
 * \warning On Win32, the value is on 32 bits only. It wraps around to 0 every about 49.71 days.
 */
TTime CTime::getLocalTime ()
{

#ifdef NL_OS_WINDOWS

	//static bool initdone = false;
	//static bool byperfcounter;
	// Initialization
	//if ( ! initdone )
	//{
		//byperfcounter = (getPerformanceTime() != 0);
		//initdone = true;
	//}

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
		// This is not affected by system time changes. But it cycles every 49 days.
		return timeGetTime();
	//}

#elif defined (NL_OS_UNIX)

	static bool initdone = false;
	static bool isMonotonicClockSupported = false;
	if ( ! initdone )
	{

#ifdef _POSIX_TIMERS
#ifdef _POSIX_MONOTONIC_CLOCK

		/* Initialize the local time engine.
		* On Unix, this method will find out if the Monotonic Clock is supported
		* (seems supported by kernel 2.6, not by kernel 2.4). See getLocalTime().
		*/
		struct timespec tv;
		if ( (clock_gettime( CLOCK_MONOTONIC, &tv ) == 0) &&
			 (clock_getres( CLOCK_MONOTONIC, &tv ) == 0) )
		{
			nldebug( "Monotonic local time supported (resolution %.6f ms)", ((float)tv.tv_sec)*1000.0f + ((float)tv.tv_nsec)/1000000.0f );
			isMonotonicClockSupported = true;
		}
		else

#endif
#endif
		{
			nlwarning( "Monotonic local time not supported, caution with time sync" );
		}

		initdone = true;
	}

#ifdef _POSIX_TIMERS
#ifdef _POSIX_MONOTONIC_CLOCK

	if ( isMonotonicClockSupported )
	{
		struct timespec tv;
		// This is not affected by system time changes.
		if ( clock_gettime( CLOCK_MONOTONIC, &tv ) != 0 )
			nlerror ("Can't get clock time again");
	    return (TTime)tv.tv_sec * (TTime)1000 + (TTime)((tv.tv_nsec/*+500*/) / 1000000);
	}

#endif
#endif

	// This is affected by system time changes.
	struct timeval tv;
	if ( gettimeofday( &tv, NULL) != 0 )
		nlerror ("Can't get time of day");
	return (TTime)tv.tv_sec * (TTime)1000 + (TTime)tv.tv_usec / (TTime)1000;

#endif
}


/* Return the time in processor ticks. Use it for profile purpose.
 * If the performance time is not supported on this hardware, it returns 0.
 * \warning On a multiprocessor system, the value returned by each processor may
 * be different. The only way to workaround this is to set a processor affinity
 * to the measured thread.
 * \warning The speed of tick increase can vary (especially on laptops or CPUs with
 * power management), so profiling several times and computing the average could be
 * a wise choice.
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
#else // HAVE_X86
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


/* Convert a ticks count into second. If the performance time is not supported on this
 * hardware, it returns 0.0.
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


std::string	CTime::getHumanRelativeTime(sint32 nbSeconds)
{
	sint32 delta = nbSeconds;
	if (delta < 0)
		delta = -delta;

	// some constants of time duration in seconds
	const uint32 oneMinute = 60;
	const uint32 oneHour = oneMinute * 60;
	const uint32 oneDay = oneHour * 24;
	const uint32 oneWeek = oneDay * 7;
	const uint32 oneMonth = oneDay * 30; // aprox, a more precise value is 30.416666... but no matter
	const uint32 oneYear = oneDay * 365; // aprox, a more precise value is 365.26.. who care?

	uint32 year, month, week, day, hour, minute;
	year = month = week = day = hour = minute = 0;

	/// compute the different parts
	while (delta > oneYear)
	{
		++year;
		delta -= oneYear;
	}

	while (delta > oneMonth)
	{
		++month;
		delta -= oneMonth;
	}

	while (delta > oneWeek)
	{
		++week;
		delta -= oneWeek;
	}

	while (delta > oneDay)
	{
		++day;
		delta -= oneDay;
	}

	while (delta > oneHour)
	{
		++hour;
		delta -= oneHour;
	}

	while (delta > oneMinute)
	{
		++minute;
		delta -= oneMinute;
	}

	// compute the string
	CSString ret;

	if (year)
		ret << year << " years ";
	if (month)
		ret << month << " months ";
	if (week)
		ret << week << " weeks ";
	if (day)
		ret << day << " days ";
	if (hour)
		ret << hour << " hours ";
	if (minute)
		ret << minute << " minutes ";
	if (delta || ret.empty())
		ret << delta << " seconds ";

	return ret;
}


} // NLMISC
