/** \file connection_web.cpp
 * 
 *
 * $Id: connection_web.cpp,v 1.1 2002/09/16 14:50:07 lecroart Exp $
 *
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "nel/misc/types_nl.h"

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <map>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

#include "nel/net/buf_server.h"
#include "nel/net/login_cookie.h"
#include "login_service.h"

#define CRYPT_PASSWORD 1

#if defined(NL_OS_UNIX) && CRYPT_PASSWORD
extern "C" char *crypt (const char *__key, const char *__salt);
#endif

using namespace std;
using namespace NLMISC;
using namespace NLNET;

CBufServer *WebServer = NULL;

// uint32 is the hostid to the web connection
map<uint32, CLoginCookie> TempCookies;

static void cbWSShardChooseShard (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	nlassert(WebServer != NULL);

	//
	// S10: receive "SCS" message from WS
	//

	CMemStream msgout;
	uint32 fake = 0;
	msgout.serial(fake);

	string reason;
	msgin.serial (reason);
	msgout.serial (reason);
	
	CLoginCookie cookie;
	msgin.serial (cookie);

	// search the cookie
	map<uint32, CLoginCookie>::iterator it = TempCookies.find (cookie.getUserAddr ());

	if (it == TempCookies.end ())
	{
		// not found in TempCookies, can't do anything
		nlwarning ("Receive an answer from welcome service but no connection waiting");
		return;
	}

	if (reason.empty())
	{
		string str = cookie.setToString ();
		msgout.serial (str);

		string addr;
		msgin.serial (addr);
		msgout.serial (addr);
	}

	WebServer->send (msgout, (TSockId)cookie.getUserAddr ());
}

static const TCallbackItem WSCallbackArray[] =
{
	{ "SCS", cbWSShardChooseShard },
};

void cb( TSockId from, void *arg )
{
	nlinfo ("cool");
/*	CMemStream msgout(false);
	uint32 fake = 0;
	msgout.serial(fake);
	string reason = "ACE";
	msgout.serial(reason);
	WebServer->send (msgout, from);
*/}

void connectionWebInit ()
{
	nlassert(WebServer == NULL);

	WebServer = new CBufServer ();
	nlassert(WebServer != NULL);

	WebServer->init (49990);
	WebServer->setConnectionCallback (cb, NULL);

	CNetManager::addCallbackArray ("WSLS", WSCallbackArray, sizeof(WSCallbackArray)/sizeof(WSCallbackArray[0]));

	nlinfo ("Set the server connection for web to port %hu", 49990);
}

void connectionWebUpdate ()
{
	nlassert(WebServer != NULL);

	try
	{
		WebServer->update ();

		while (WebServer->dataAvailable ())
		{
			// create a string mem stream to easily communicate with web server
			NLMISC::CMemStream msgin (true);
			TSockId host;
			try
			{
				WebServer->receive (msgin, &host);
				uint32 fake = 0;
				msgin.serial(fake);
			}
			catch (Exception &e)
			{
				nlwarning ("Error during receiving: '%s'", e.what ());
			}

			sint32 shardId;
			sint32 userId;
			msgin.serial (shardId);
			msgin.serial (userId);

			nlinfo ("Web wants to add userid %d to the shardid %d, send request to the shard", userId, shardId);

			uint32 i;
			for (i = 0; i < Shards.size (); i++)
			{
				if (Shards[i].ShardId == shardId)
				{
					// generate a cookie
					CLoginCookie Cookie ((uint32)host, userId);

					// send message to the welcome service to see if it s ok and know the front end ip
					CMessage msgout (CNetManager::getNetBase("WSLS")->getSIDA (), "CS");
					msgout.serial (Cookie);
					CNetManager::send("WSLS", msgout, Shards[i].SockId);
					beep (1000, 1, 100, 100);

					// add the connection in temp cookie
					TempCookies.insert(make_pair(Cookie.getUserAddr(), Cookie));
					break;
				}
			}
			if(i == Shards.size ())
			{
				// the shard is not available, denied the user
				nlwarning("ShardId %d is not available, can't add the userid %d", shardId, userId);

				CMemStream msgout;
				uint32 fake = 0;
				msgout.serial(fake);
				string reason = "Selected shard is not available";
				msgout.serial (reason);
				WebServer->send (msgout, host);
			}
		}
	}
	catch (Exception &e)
	{
		nlwarning ("Error during update: '%s'", e.what ());
	}
}
