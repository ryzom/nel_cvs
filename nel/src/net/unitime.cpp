/** \file unitime.cpp
 * CUniTime class
 *
 * $Id: unitime.cpp,v 1.19 2001/02/23 10:58:12 lecroart Exp $
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

#include "nel/misc/debug.h"
#include "nel/net/msg_socket.h"
#include "nel/net/naming_client.h"

#include "nel/misc/common.h"
#include "nel/misc/time_nl.h"
#include "nel/net/unitime.h"

using namespace NLMISC;
using namespace std;

namespace NLNET
{

TTime CUniTime::_SyncUniTime = 0;
TTime CUniTime::_SyncLocalTime = 0;
bool CUniTime::Sync = false;

TTime CUniTime::getUniTime ()
{
	if (!Sync)
	{
		nlerror ("called getUniTime before calling syncUniTimeFromServer");
	}
	return getLocalTime () - (_SyncLocalTime - _SyncUniTime);
}

void CUniTime::syncUniTimeFromService (const CInetAddress *addr)
{
	uint16 validitytime;
	CSocket server;
	if (addr != NULL)
	{
		server.connect(*addr);
	}
	else
	{
		CNamingClient::lookupAndConnect( "TS", server, validitytime );
	}

	if (server.connected ())
	{
		sint attempt = 0;
		TTime bestdelta = 60000;	// 1 minute
		while (attempt < 10)
		{
			CMessage msgout( "" );
			msgout.setType( 0 ); // we don't listen for incoming replies, therefore we must not use a type as string. 0 is the default action for CLogService : "LOG"

			// send the message
			server.send( msgout );

			// before time
			TTime before = CTime::getLocalTime ();

			// receive the answer
			CMessage msgin( "", true );
			server.receive ( msgin );

			TTime after = CTime::getLocalTime (), delta = after - before;

			if (delta < 10 || delta < bestdelta)
			{
				bestdelta = delta;

				TTime time;
				msgin.serial (time);
				CUniTime::setUniTime (time, (before+after)/2);

				if (delta < 10) break;
			}
			attempt++;
		}
		server.close();
		nlinfo ("Universal time is %"NL_I64"dms with a mean error of %"NL_I64"dms", CUniTime::getUniTime(), bestdelta/2);
	}
	else
	{
		nlwarning ("TS not found, can't synchronize universal time");
	}
}

const char *CUniTime::getStringUniTime ()
{
	return getStringUniTime(CUniTime::getUniTime());
}

const char *CUniTime::getStringUniTime (TTime ut)
{
	static char str[512];

	uint32 ms = (uint32) (ut % 1000); // time in ms 1000ms dans 1s
	ut /= 1000;

	uint32 s = (uint32) (ut % 60); // time in seconds 60s dans 1mn
	ut /= 60;

	uint32 m = (uint32) (ut % 60); // time in minutes 60m dans 1h
	ut /= 60;

	uint32 h = (uint32) (ut % 9); // time in hours 9h dans 1j
	ut /= 9;

	uint32 day = (uint32) (ut % (8*4)); // time in days 8day dans 1month
	ut /= 8;

	uint32 week = (uint32) (ut % 4); // time in weeks 4week dans 1month
	ut /= 4;

	uint32 month = (uint32) (ut % 12); // time in months 12month dans 1year
	ut /= 12;

	uint  year =  (uint32) ut;	// time in years

	smprintf (str, 512, "%02d/%02d/%04d (week %d) %02d:%02d:%02d.%03d", day+1, month+1, year+1, week+1, h, m, s, ms);
	return str;
}


} // NLNET
