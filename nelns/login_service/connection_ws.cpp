/** \file login_service.cpp
 * Login Service (LS)
 *
 * $Id: connection_ws.cpp,v 1.7 2002/03/11 10:42:49 lecroart Exp $
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

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <map>

#include "nel/misc/types_nl.h"

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

#include "nel/net/service.h"
#include "nel/net/net_manager.h"
#include "nel/net/login_cookie.h"
#include "login_service.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;



sint findShard (TSockId senderId)
{
	for (sint i = 0; i < (sint) Shards.size (); i++)
	{
		if (Shards[i].SockId == senderId)
		{
			return i;
		}
	}
	// shard not found
	return -1;
}


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// CONNECTION TO THE WELCOME SERVICES /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

static void cbWSConnection (const string &serviceName, TSockId from, void *arg)
{
	const CInetAddress &ia = CNetManager::getNetBase("WSLS")->hostAddress (from);

	nldebug("new potential shard: %s", ia.asString ().c_str ());

	// first, check if it an authorized shard
	for (sint32 i = 0; i < (sint32) Shards.size (); i++)
	{
		if (!Shards[i].Online && Shards[i].WSAddr == ia.ipAddress ())
		{
			// new shard connected
			Shards[i].WSAddr = ia.asIPString (); // put addr+port to detect different shard on the same address
			Shards[i].Online = true;
			Shards[i].SockId = from;
			nlinfo("Shard with ip '%s' is online!", Shards[i].WSAddr.c_str ());
			return;
		}
	}
	if (AcceptExternalShard)
	{
		// New externam shard connected, add it in the file
		Shards.push_back (CShard(ia));
		sint32 pos = Shards.size()-1;
		Shards[pos].WSAddr = ia.asIPString (); // put addr+port to detect different shard on the same address
		Shards[pos].Online = true;
		Shards[pos].SockId = from;
		nlinfo("External shard with ip '%s' is online!", Shards[pos].WSAddr.c_str ());
		writePlayerDatabase ();
	}
	else
	{
		nlwarning("It's not a authorized shard, disconnect it");
		CNetManager::getNetBase("WSLS")->disconnect(from);
	}
}

static void cbWSDisconnection (const string &serviceName, TSockId from, void *arg)
{
	CCallbackNetBase *cnb = CNetManager::getNetBase("WSLS");
	const CInetAddress &ia = cnb->hostAddress (from);

	nldebug("shard disconnection: %s", ia.asString ().c_str ());

	for (sint32 i = 0; i < (sint32) Shards.size (); i++)
	{
		if (Shards[i].SockId == from)
		{
			if (Shards[i].Online)
			{
				// shard disconnected
				nlinfo("Shard with ip '%s' is offline!", Shards[i].WSAddr.c_str());
				Shards[i].Online = false;
				Shards[i].SockId = NULL;
				Shards[i].WSAddr = Shards[i].WSAddr.substr (0, Shards[i].WSAddr.find (":"));

				// put users connected on this shard offline
				for (sint32 u = 0; u < (sint32) Users.size (); u++)
				{
					if (Users[u].ShardId == from)
					{
						disconnectClient (Users[u], true, false);
					}
				}
			}
			else
			{
				nlwarning("Shard with ip '%s' goes offline but wasn't online!", Shards[i].WSAddr.c_str ());
			}
			return;
		}
	}
	nlwarning("Unknown Shard goes offline but wasn't online!");
}

static void cbWSShardChooseShard (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	CLoginCookie cookie;
	string reason;
	string addr;

	//
	// S10: receive "SCS" message from WS
	//

	CMessage msgout (CNetManager::getNetBase("LS")->getSIDA (), "SCS");

	msgin.serial (reason);
	msgout.serial (reason);
	
	msgin.serial (cookie);
	msgout.serial (cookie);

	if (reason.empty())
	{
		msgin.serial (addr);
		msgout.serial (addr);
	}

	// find the user
	for (vector<CUser>::iterator it = Users.begin (); it != Users.end (); it++)
	{
		if ((*it).State == CUser::Authorized && (*it).Cookie == cookie)
		{
			CNetManager::send ("LS", msgout, (*it).SockId);

			if (reason.empty())
			{
				// Now we wait the message from WS saying that the client is well connected to the shard
				(*it).State = CUser::Awaiting;
				(*it).Cookie.clear ();
			}
			else
			{
				// the WS haven't accepted the client
				disconnectClient (*it, true, false);
			}

			return;
		}
	}
	nlwarning ("The user is not connected");
}


/** Shard accepted the new user, so warn the user that he could connect to the shard now */
/*void cbClientShardAcceptedTheUser (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	uint8 res;
	string IP;
	uint32 Key, Id;
	msgin.serial (IP);
	msgin.serial (Key);
	msgin.serial (Id);
	msgin.serial (res);

	CMessage msgout (netbase.getSIDA (), "ACC");
	msgout.serial (res);

	if(res)
	{
		// the shard accept the user
		msgout.serial (IP);
		msgout.serial (Key);
	}
	else
	{
		// the shard don't want him!
		string reason;
		msgin.serial (reason);
		msgout.serial (reason);
	}

	// find the user
	for (vector<CUser>::iterator it = Users.begin (); it != Users.end (); it++)
	{
		if ((*it).Authorized && (*it).Key == Key)
		{
			// send the answer to the user
			netbase.send (msgout, (*it).SockId);

			(*it).Authorized = false;
			return;
		}
	}
}
*/


/*
void cbShardComesIn (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	const CInetAddress &ia = netbase.hostAddress (from);

	// at this time, it could be a new shard or a ne client.

	nldebug("new potential shard: %s", ia.asString ().c_str ());

	// first, check if it an authorized shard
	for (sint32 i = 0; i < (sint32) Shards.size (); i++)
	{
		if (Shards[i].Address.ipAddress () == ia.ipAddress ())
		{
			if (Shards[i].Online)
			{
				nlwarning("Shard with ip '%s' is already online! Disconnect the new one", ia.asString().c_str ());
				netbase.disconnect (from);
			}
			else
			{
				// new shard connected
				Shards[i].Address = ia;
				Shards[i].Online = true;
				Shards[i].SockId = from;
				nlinfo("Shard with ip '%s' is online!", Shards[i].Address.asString().c_str ());
			}
			return;
		}
	}
#if ACCEPT_EXTERNAL_SHARD
	// New externam shard connected, add it in the file
	Shards.push_back (CShard(ia));
	sint32 pos = Shards.size()-1;
	Shards[pos].Online = true;
	Shards[pos].SockId = from;
	nlinfo("External shard with ip '%s' is online!", Shards[pos].Address.asString().c_str ());
	writeConfigFile ();
#else
	nlwarning("It's not a authorized shard, disconnect it");
	netbase.close (from);
#endif
}
*/

// 
static void cbWSIdentification (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	const CInetAddress &ia = netbase.hostAddress (from);

	string shardName;
	msgin.serial(shardName);
	nldebug("new potential identification: %s", shardName.c_str());

	// first, check if it an authorized shard
	for (sint32 i = 0; i < (sint32) Shards.size (); i++)
	{
		if (Shards[i].WSAddr == ia.asIPString ())
		{
			if (Shards[i].Online)
			{
				nlinfo("Shard with ip '%s' is identified to '%s'!", Shards[i].WSAddr.c_str (), shardName.c_str());
				Shards[i].ShardName = shardName;
			}
			else
			{
				nlwarning("Shard with ip '%s' is not online! Disconnect this intruder", ia.asString().c_str ());
				netbase.disconnect(from);
			}
			return;
		}
	}
}

static void cbWSClientConnected (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S16: Receive "CC" message from WS
	//

	// a WS tells me that a player is connected or disconnected
	// find the user
	uint32 Id;
	uint8 con;
	msgin.serial (Id);
	msgin.serial (con);	// con=1 means a client is connected on the shard, 0 means a client disconnected

	sint pos = findUser (Id);
	if (pos == -1)
	{
		nlwarning ("Id %d doesn't exist", Id);
		Output.displayNL ("###: %3d UId doesn't exist", Id);
		return;
	}

	if (con == 1 && Users[pos].State != CUser::Awaiting)
	{
		nlwarning ("Id %d isn't waiting a connection to a shard", Id);
		Output.displayNL ("###: %3d User isn't waiting a connection to a shard", Id);
		return;
	}
	else if (con == 0 && Users[pos].State != CUser::Online)
	{
		nlwarning ("Id %d wasn't connected on a shard", Id);
		Output.displayNL ("###: %3d User wasn't connected on a shard", Id);
		return;
	}

	static uint recordNbPlayer = 0;
	static uint nbPlayer = 0;

	if (con == 1)
	{
		// new client on the shard
		Users[pos].State = CUser::Online;
		Users[pos].ShardId = from;

		sint ShardPos = findShard(from);
		if (ShardPos != -1)
			Shards[ShardPos].NbPlayers++;
		else
			nlwarning ("user connected shard isn't in the shard list");
		
		nldebug ("Id %d is connected on the shard", Id);
		Output.displayNL ("###: %3d User connected to the shard (%s)", Id, Shards[ShardPos].Name.c_str());

		nbPlayer++;
		if (nbPlayer > recordNbPlayer)
		{
			recordNbPlayer = nbPlayer;
			beep (2000, 1, 100, 0);
			nlwarning("New player number record!!! %d players online on all shards", recordNbPlayer);
		}
	}
	else
	{
		// client removed from the shard (true is for potential other client with the same id that wait for a connection)
		disconnectClient (Users[pos], true, false);

		sint ShardPos = findShard(from);
		if (ShardPos != -1)
			Shards[ShardPos].NbPlayers--;
		else
			nlwarning ("user disconnected shard isn't in the shard list");
		
		nldebug ("Id %d is disconnected from the shard", Id);
		Output.displayNL ("###: %3d User disconnected from the shard (%s)", Id, Shards[ShardPos].Name.c_str());

		nbPlayer--;
	}
}


static const TCallbackItem WSCallbackArray[] =
{
	{ "SCS", cbWSShardChooseShard },

	{ "CC", cbWSClientConnected },

	{ "WS_IDENT", cbWSIdentification },
};

void connectionWSInit (uint16 port)
{
	CNetManager::addServer ("WSLS", port);
	CNetManager::addCallbackArray ("WSLS", WSCallbackArray, sizeof(WSCallbackArray)/sizeof(WSCallbackArray[0]));
	CNetManager::setConnectionCallback ("WSLS", cbWSConnection, NULL);
	CNetManager::setDisconnectionCallback ("WSLS", cbWSDisconnection, NULL);

	nlinfo ("Set the server connection for welcome service to port %hu", port);
}
