/** \file time_nl.h
 * CTime class
 *
 * $Id: time_nl.h,v 1.3 2000/11/22 11:17:06 lecroart Exp $
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

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
// automatically add the win multimedia library if you use CTime class
#pragma comment(lib, "winmm.lib")
#endif

namespace NLMISC
{

/// Time type
typedef uint64 TTime;


/**
 * This class provide a independant local time system.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CTime
{
public:

	/** get the time in millisecond. It's a \b local time, it means that the value
	 * is \b different on 2 differents computers. Use the CUniTime to get a universal
	 * time that is the same on all computers.
	 * \warning On Windows, the value is on 32 bits only.
	 */
	static TTime	getLocalTime ();
};

} // NLMISC

#endif // NL_TIME_H

/* End of time_nl.h */
