/** \file time_nl.cpp
 * CTime class
 *
 * $Id: time_nl.cpp,v 1.3 2000/11/21 17:19:34 valignat Exp $
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

#ifdef NL_OS_WINDOWS
#include <windows.h>

#elif defined (NL_OS_UNIX)

#include <sys/time.h>
#include <unistd.h>

#endif

#include "nel/misc/time_nl.h"

namespace NLMISC
{

TTime CTime::getLocalTime ()
{
#ifdef NL_OS_WINDOWS
	return timeGetTime ();
#elif defined (NL_OS_UNIX)

	struct timeval tv;

	if ( gettimeofday( &tv, NULL) == -1 )
	{
		nlerror ("Cannot get time of day.");
	}

	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));

#endif
}

} // NLMISC
