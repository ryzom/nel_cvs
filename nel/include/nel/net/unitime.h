/** \file unitime.h
 * CUniTime class
 *
 * $Id: unitime.h,v 1.2 2000/11/10 16:58:35 cado Exp $
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

#ifndef NL_UNITIME_H
#define NL_UNITIME_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"

namespace NLNET
{

/**
 * This class provide a independant universal time system.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CUniTime : public NLMISC::CTime
{
public:

	/// Return the time in millisecond. This time is the same on all computers at the \b same moment.
	static NLMISC::TTime	getUniTime ();

	/** You need to call this function before calling getUniTime or an assert will occured.
	 * This function will connect to the time service and synchronize your computer.
	 */
	static void				syncUniTimeFromService ();

	/// \internal used by the time service to set the universal time the first time
	static void				setUniTime (NLMISC::TTime uTime, NLMISC::TTime lTime) { Sync = true; _SyncUniTime = uTime; _SyncLocalTime = lTime; }
	/// \internal
	static void				setUniTime (NLMISC::TTime uTime) { setUniTime (uTime, getLocalTime ()); }

	static bool				Sync;				// true if the synchronization occured
private:

	static NLMISC::TTime	_SyncUniTime;		// time in millisecond when the universal time received
	static NLMISC::TTime	_SyncLocalTime;		// time in millisecond when the syncro with universal time occured
};

} // NLNET

#endif // NL_UNITIME_H

/* End of unitime.h */
