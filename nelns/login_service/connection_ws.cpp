/** \file login_service.cpp
 * Login Service (LS)
 *
 * $Id: connection_ws.cpp,v 1.9 2002/09/16 14:50:07 lecroart Exp $
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

sint32 findShard (sint32 shardId)
{
	for (sint i = 0; i < (sint) Shards.size (); i++)
	{
		if (Shards[i].ShardId == shardId)
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

	// if we accept external shard, don't need to check if address is valid
	if(IService::getInstance ()->ConfigFile.getVar("AcceptExternalShards").asInt () == 1)
		return;

	string query = "select * from shard where WSAddr='"+ia.ipAddress()+"'";
	int ret = mysql_query (DatabaseConnection, query.c_str ());
	if (ret != 0)
	{
		nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}

	MYSQL_RES *res = mysql_store_result(DatabaseConnection);
	if (res == NULL)
	{
		nlwarning ("mysql_store_result () failed from query '%s': %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}

	if (mysql_num_rows(res) == 0)
	{
		// if we are here, it s that the shard have not a valid wsaddr in the database
		// we can't accept unknown shard

		nlwarning("The shard is not in the database (WSAddr %s is not in the base), disconnecting it", ia.ipAddress ().c_str ());
		CNetManager::getNetBase("WSLS")->disconnect(from);
	}
}

static void cbWSDisconnection (const string &serviceName, TSockId from, void *arg)
{
	CCallbackNetBase *cnb = CNetManager::getNetBase("WSLS");
	const CInetAddress &ia = cnb->hostAddress (from);

	nldebug("shard disconnection: %s", ia.asString ().c_str ());

	for (uint32 i = 0; i < Shards.size (); i++)
	{
		if (Shards[i].SockId == from)
		{
			// shard disconnected
			nlinfo("ShardId %d with ip '%s' is offline!", Shards[i].ShardId, ia.asString ().c_str());

			string query = "update shard set Online=Online-1, NbPlayers=NbPlayers-"+toString(Shards[i].NbPlayer)+" where ShardId="+toString(Shards[i].ShardId);
			int ret = mysql_query (DatabaseConnection, query.c_str ());
			if (ret != 0)
			{
				nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			}

			// put users connected on this shard offline
			
			/// \todo ace: remove all users from this shard from the database

			Shards.erase (Shards.begin () + i);

			return;
		}
	}
	nlwarning("Shard %s goes offline but wasn't online!", ia.asString ().c_str ());
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
	const CInetAddress &ia = CNetManager::getNetBase("WSLS")->hostAddress (from);

	sint32 shardId;
	msgin.serial(shardId);
	nldebug("shard identification, It says to be ShardId %d, let's check that!", shardId);

	string query = "select * from shard where ShardId="+toString(shardId);
	int ret = mysql_query (DatabaseConnection, query.c_str ());
	if (ret != 0)
	{
		nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}

	MYSQL_RES *res = mysql_store_result(DatabaseConnection);
	if (res == NULL)
	{
		nlwarning ("mysql_store_result () failed from query '%s': %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}

	int nbrow = mysql_num_rows(res);
	if (nbrow == 0)
	{
		if(IService::getInstance ()->ConfigFile.getVar("AcceptExternalShards").asInt () == 1)
		{
			// we accept new shard, add it
			query = "insert into shard (ShardId, WsAddr, Online) values ("+toString(shardId)+", '"+ia.ipAddress ()+"', 1)";
			int ret = mysql_query (DatabaseConnection, query.c_str ());
			if (ret != 0)
			{
				nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			}
			else
			{
				nlinfo("The ShardId %d with ip '%s' was inserted in the database and is online!", shardId, ia.ipAddress ().c_str ());
			}
			return;
		}
		else
		{
			// can't accept new shard
			nlwarning ("Bad shard identification, Can't accept new shard, disconnecting it");
			CNetManager::getNetBase("WSLS")->disconnect(from);
			return;
		}
	}
	else if (nbrow == 1)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		if (row == NULL)
		{
			nlwarning ("mysql_fetch_row (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			return;
		}

		// check that the ip is ok
		if (row[1] != ia.ipAddress ())
		{
			// good shard id but from a bad computer address
			nlwarning ("Bad shard identification, ShardId %d should come from '%s' and come from '%s'", shardId, row[1], ia.ipAddress ().c_str ());
			CNetManager::getNetBase("WSLS")->disconnect(from);
			return;
		}

		sint32 s = findShard (shardId);
		if (s != -1)
		{
			// the shard should be already online, bad!
			nlwarning ("Bad shard identification, ShardId %d is already online", shardId);
			CNetManager::getNetBase("WSLS")->disconnect(from);
			return;
		}

		string query = "update shard set Online=Online+1 where ShardId="+toString(shardId);
		int ret = mysql_query (DatabaseConnection, query.c_str ());
		if (ret != 0)
		{
			nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			return;
		}

		Shards.push_back (CShard (shardId, from));

		// ok, the shard is identified correctly
		nlinfo("ShardId %d with ip '%s' is online!", shardId, ia.ipAddress ().c_str ());
		return;
	}
	else
	{
		nlwarning ("There's more than 1 shard with the shardId %d", shardId);
	}

	// bad shardId
	nlwarning ("Bad shard identification, ShardId %d is not in the database", shardId);
	CNetManager::getNetBase("WSLS")->disconnect(from);
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


	string query = "select * from user where UId="+toString(Id);
	int ret = mysql_query (DatabaseConnection, query.c_str ());
	if (ret != 0)
	{
		nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}
	MYSQL_RES *res = mysql_store_result(DatabaseConnection);
	if (res == NULL)
	{
		nlwarning ("mysql_store_result () failed from query '%s': %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}
	int nbrow = mysql_num_rows(res);
	MYSQL_ROW row = mysql_fetch_row(res);
	if (row == NULL)
	{
		nlwarning ("mysql_fetch_row (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		return;
	}

	if(nbrow == 0)
	{
		nlwarning ("Id %d doesn't exist", Id);
		Output.displayNL ("###: %3d UId doesn't exist", Id);
		return;
	}
	else if (nbrow > 1)
	{
		nlerror ("Id %d have more than one entry!!!", Id);
		return;
	}

	// row[4] = State
	if (con == 1 && row[4] != "Waiting")
	{
		nlwarning ("Id %d isn't waiting a connection to a shard", Id);
		Output.displayNL ("###: %3d User isn't waiting a connection to a shard", Id);
		return;
	}
	else if (con == 0 && row[4] != "Online")
	{
		nlwarning ("Id %d wasn't connected on a shard", Id);
		Output.displayNL ("###: %3d User wasn't connected on a shard", Id);
		return;
	}

	static uint recordNbPlayer = 0;
	static uint nbPlayer = 0;

	sint ShardPos = findShard(from);

	if (con == 1)
	{
		// new client on the shard


		string query = "update user set State='Online', ShardId="+toString(Shards[ShardPos].ShardId)+" where UId="+toString(Id);
		int ret = mysql_query (DatabaseConnection, query.c_str ());
		if (ret != 0)
		{
			nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			return;
		}

		if (ShardPos != -1)
		{
			Shards[ShardPos].NbPlayer++;

			string query = "update shard set NbPlayers=NbPlayers+1 where ShardId="+toString(Shards[ShardPos].ShardId);
			int ret = mysql_query (DatabaseConnection, query.c_str ());
			if (ret != 0)
			{
				nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
				return;
			}
		}
		else
			nlwarning ("user connected shard isn't in the shard list");

		nldebug ("Id %d is connected on the shard", Id);
		Output.displayNL ("###: %3d User connected to the shard (%d)", Id, Shards[ShardPos].ShardId);

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
	//		disconnectClient (Users[pos], true, false);

		string query = "update user set State='Offline', ShardId=-1 where UId="+toString(Id);
		int ret = mysql_query (DatabaseConnection, query.c_str ());
		if (ret != 0)
		{
			nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
			return;
		}

		if (ShardPos != -1)
		{
			Shards[ShardPos].NbPlayer--;

			string query = "update shard set NbPlayers=NbPlayers-1 where ShardId="+toString(Shards[ShardPos].ShardId);
			int ret = mysql_query (DatabaseConnection, query.c_str ());
			if (ret != 0)
			{
				nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
				return;
			}
		}
		else
			nlwarning ("user disconnected shard isn't in the shard list");
			
		nldebug ("Id %d is disconnected from the shard", Id);
		Output.displayNL ("###: %3d User disconnected from the shard (%d)", Id, Shards[ShardPos].ShardId);

		nbPlayer--;
	}
}


static const TCallbackItem WSCallbackArray[] =
{
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

void connectionWSRelease ()
{
	// remove all online and player because this LS going down
	for (sint i = 0; i < (sint) Shards.size (); i++)
	{
		string query = "update shard set Online=Online-1, NbPlayers=NbPlayers-"+toString(Shards[i].NbPlayer)+" where ShardId="+toString(Shards[i].ShardId);
		int ret = mysql_query (DatabaseConnection, query.c_str ());
		if (ret != 0)
		{
			nlwarning ("mysql_query (%s) failed: %s", query.c_str (),  mysql_error(DatabaseConnection));
		}
	}
}
