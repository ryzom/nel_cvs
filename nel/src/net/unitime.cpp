/** \file unitime.cpp
 * CUniTime class
 *
 * $Id: unitime.cpp,v 1.1 2000/11/08 15:54:35 lecroart Exp $
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

#include <windows.h>
#include <time.h>

#include "nel/misc/debug.h"
#include "nel/net/msg_socket.h"
#include "nel/net/naming_client.h"

#include "nel/misc/time_nl.h"
#include "nel/net/unitime.h"

using namespace std;

namespace NLNET
{

uint64 CUniTime::_SyncUniTime = 0;
uint64 CUniTime::_SyncLocalTime = 0;
bool CUniTime::Sync = false;

uint64 CUniTime::getUniTime ()
{
	if (!Sync)
	{
		nlwarning ("called getUniTime before calling syncUniTimeFromServer");
		return 0;
	}
	return getLocalTime () - (_SyncLocalTime - _SyncUniTime);
}

void CUniTime::syncUniTimeFromService ()
{
	CInetAddress servaddr;
	uint16 validitytime;
	if ( CNamingClient::lookup( "CTS", servaddr, validitytime ) )
	{
		CSocket server(true,false);
		server.connect( servaddr );

		sint attempt = 0;
		uint64 bestdelta = 0;
		while (attempt < 10)
		{
			CMessage msgout( "" );
			msgout.setType( 0 ); // we don't listen for incoming replies, therefore we must not use a type as string. 0 is the default action for CLogService : "LOG"

			// send the message
			server.send( msgout );

			// before time
			uint64 before = CTime::getLocalTime ();

			// receive the answer
			CMessage msgin( "", true );
			server.receive ( msgin );

			uint64 after = CTime::getLocalTime (), delta = after - before;

			nldebug ("*****  delta: %I64dms (best is %I64dms)", delta, bestdelta);

			if (delta < 10 || delta < bestdelta)
			{
				bestdelta = delta;

				uint64 time;
				msgin.serial (time);
				CUniTime::setUniTime (time, (before+after)/2);

				if (delta < 10) break;
			}
			attempt++;
		}
		server.close();
	}
	else
	{
		nlwarning ("CTS not found, can't syncronize universal time");
	}
}


} // NLNET
