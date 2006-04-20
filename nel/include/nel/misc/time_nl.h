/** \file time_nl.h
 * OS independant time class provided system clock
 *
 * $Id: time_nl.h,v 1.11.20.1 2006/04/20 15:36:36 boucher Exp $
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

#ifndef NL_TIME_H
#define NL_TIME_H

#include "types_nl.h"

#ifdef NL_OS_WINDOWS
// automatically add the win multimedia library if you use CTime class
#	pragma comment(lib, "winmm.lib")
#endif

namespace NLMISC
{


/// New time types
typedef double TGameTime;		// Time according to the game (used for determining day, night...) (double in seconds)
typedef uint32 TGameCycle;		// Integer game cycle count from the game (in game ticks)
typedef double TLocalTime;		// Time according to the machine's local clock (double in seconds)
typedef sint64 TCPUCycle;		// Integer cycle count from the CPU (for profiling in cpu ticks)

/// Old time type
typedef sint64 TTime;
typedef sint64 TTicks;


/**
 * This class provide a independant local time system.
 * \author Vianney Lecroart, Olivier Cado
 * \author Nevrax France
 * \date 2000-2005
 */
class CTime
{
public:

	/** Return the number of second since midnight (00:00:00), January 1, 1970,
	 * coordinated universal time, according to the system clock.
	 * This values is the same on all computer if computers are synchronized (with NTP for example).
	 */
	static uint32	getSecondsSince1970 ();

	/** Return the local time in milliseconds.
	 * Use it only to measure time difference, the absolute value does not mean anything.
	 * On Unix, getLocalTime() will try to use the Monotonic Clock if available, otherwise
	 * the value can jump backwards if the system time is changed by a user or a NTP time sync process.
	 * The value is different on 2 different computers; use the CUniTime class to get a universal
	 * time that is the same on all computers.
	 * \warning On Win32, the value is on 32 bits only. It wraps around to 0 every about 49.71 days.
	 */
	static TTime	getLocalTime ();

	/** Return the time in processor ticks. Use it for profile purpose.
	 * If the performance time is not supported on this hardware, it returns 0.
	 * \warning On a multiprocessor system, the value returned by each processor may
	 * be different. The only way to workaround this is to set a processor affinity
	 * to the measured thread.
	 * \warning The speed of tick increase can vary (especially on laptops or CPUs with
	 * power management), so profiling several times and computing the average could be
	 * a wise choice.
	 */
	static TTicks	getPerformanceTime ();

	/** Convert a ticks count into second. If the performance time is not supported on this
	 * hardware, it returns 0.0.
	 */
	static double	ticksToSecond (TTicks ticks);

	/** Build a human readable string of a time difference in second.
	 *	The result will be of the form '1 years 2 months 2 days 10 seconds'
	 */
	static std::string	getHumanRelativeTime(sint32 nbSeconds);
};

} // NLMISC

#endif // NL_TIME_H

/* End of time_nl.h */
